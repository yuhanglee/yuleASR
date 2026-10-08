/**
 * @file test_comstack_benchmark.c
 * @brief P1 Phase 8 — communication-stack performance benchmark (host build).
 *
 * Runs the REAL production CanIf.c and Com.c (plus the real service_pdur
 * library) end-to-end and measures, with clock_gettime(CLOCK_MONOTONIC)
 * statistics (min/max/avg/p95/p99 via perf_stats.h):
 *
 *   1. CanIf_RxIndication dispatch latency at different PDU counts:
 *      the benchmark compiles CanIf.c against a 64-entry Rx table (shadow
 *      CanIf_Cfg.h, -include) and measures the dispatch cost for a match at
 *      table entry 0 / 31 / 63 (1 / 32 / 64 entries scanned in the Hoh
 *      bucket) plus a no-match (full scan + DET).
 *   2. Com signal pack/unpack latency for 8/16/32-bit signals in both
 *      endiannesses (Com_SendSignal PENDING = pack-only, Com_ReceiveSignal =
 *      unpack) — this exercises the P1 Phase 8 byte-aligned memcpy fast path
 *      for the byte-aligned signals used here.
 *   3. PduR_Transmit routing latency (COM source -> CanIf destination).
 *   4. Full transmit chain: Com_SendSignal TRIGGERED = signal pack +
 *      PduR_Transmit + CanIf_Transmit + Can_Write.
 *
 * Configuration summary:
 *   - CanIf: 1 controller, 4 Tx PDUs, 64 Rx PDUs (all on HRH 0, CAN ids
 *     0x150..0x18F, DLCCheck off), RxIndication enabled.
 *   - PduR:  4 routing paths — CanIf Rx PDU 0/31/63 -> Com IPDU 0 (immediate,
 *     real up-link into Com_RxIndication) and COM PDU 0 -> CanIf PDU 0.
 *   - Com:   6 signals on I-PDU 0: 8-bit LE @0, 16-bit LE @16, 32-bit LE @32,
 *     16-bit BE @64, 32-bit BE @80 (all PENDING), plus an 8-bit TRIGGERED
 *     signal @96. Byte-aligned positions exercise the memcpy fast path.
 */

#include "unity.h"
#include "CanIf.h"
#include "Can.h"
#include "PduR.h"
#include "Com.h"
#include "perf_stats.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* Dependency stubs (Can driver, CanTrcv, CanSM, Dcm, PduR up-link)    */
/* Signatures per Can.h / PduR.h — mirrors tests/bsw/ecual/canif and   */
/* tests/bsw/services/pdur. CanIf/Com symbols come from the real       */
/* production modules compiled into this executable.                   */
/* ------------------------------------------------------------------ */

static uint32_t mock_DetCalls = 0u;

Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId)
{
    (void)ModuleId;
    (void)InstanceId;
    (void)ApiId;
    (void)ErrorId;
    mock_DetCalls++;
    return E_OK;
}

static uint32_t mock_CanWriteCalls = 0u;

Can_ReturnType Can_SetControllerMode(uint8 Controller, Can_ControllerStateType Transition)
{
    (void)Controller;
    (void)Transition;
    return CAN_OK;
}

Can_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType* PduInfo)
{
    (void)Hth;
    (void)PduInfo;
    mock_CanWriteCalls++;
    return CAN_OK;
}

Std_ReturnType Can_CheckWakeup(uint8 Controller)
{
    (void)Controller;
    return E_OK;
}

/* CanTrcv stub: CanTrcv.h is intentionally not included, the mode type is
 * mirrored with the same constants/values as CanTrcv_TrcvModeType. */
typedef enum {
    CANTRCV_TRCVMODE_NORMAL = 0u,
    CANTRCV_TRCVMODE_STANDBY = 1u,
    CANTRCV_TRCVMODE_SLEEP = 2u
} CanTrcv_TrcvModeType;

Std_ReturnType CanTrcv_SetOpMode(uint8 Transceiver, CanTrcv_TrcvModeType OpMode)
{
    (void)Transceiver;
    (void)OpMode;
    return E_OK;
}

/* Can driver error-state getter stubs */
static Can_ErrorStateType mock_CanGetErrState_Value = CAN_ERRORSTATE_ACTIVE;

Std_ReturnType Can_GetControllerErrorState(uint8 Controller, Can_ErrorStateType* ErrorStatePtr)
{
    (void)Controller;
    if (ErrorStatePtr != NULL) { *ErrorStatePtr = mock_CanGetErrState_Value; }
    return E_OK;
}

Std_ReturnType Can_GetControllerRxErrorCounter(uint8 Controller, uint8* RxErrorCounterPtr)
{
    (void)Controller;
    if (RxErrorCounterPtr != NULL) { *RxErrorCounterPtr = 0u; }
    return E_OK;
}

Std_ReturnType Can_GetControllerTxErrorCounter(uint8 Controller, uint8* TxErrorCounterPtr)
{
    (void)Controller;
    if (TxErrorCounterPtr != NULL) { *TxErrorCounterPtr = 0u; }
    return E_OK;
}

/* CanSM indication stubs */
void CanSM_CheckTransceiverWakeFlagIndication(uint8 NetworkHandle)
{
    (void)NetworkHandle;
}

Std_ReturnType CanSM_ClearTrcvWufFlagIndication(uint8 NetworkHandle)
{
    (void)NetworkHandle;
    return E_OK;
}

void CanSM_TransceiverModeIndication(uint8 NetworkHandle, CanIf_TransceiverModeType TransceiverMode)
{
    (void)NetworkHandle;
    (void)TransceiverMode;
}

void CanSM_ControllerModeIndication(uint8 ControllerId, CanIf_ControllerModeType ControllerMode)
{
    (void)ControllerId;
    (void)ControllerMode;
}

/* PduR up-link (PduR_RxIndication / PduR_TxConfirmation) comes from the REAL
 * libservice_pdur: CanIf_RxIndication dispatches into the real routing engine,
 * which forwards to the real Com_RxIndication — the benchmark therefore times
 * the genuine CanIf -> PduR -> Com receive path. */

/* Dcm stubs (referenced by PduR.c for DCM-targeted routing) */
static uint32_t mock_DcmRxIndCalls = 0u;

void Dcm_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr)
{
    (void)RxPduId;
    (void)PduInfoPtr;
    mock_DcmRxIndCalls++;
}

void Dcm_TxConfirmation(PduIdType TxPduId, Std_ReturnType result)
{
    (void)TxPduId;
    (void)result;
}

Std_ReturnType Dcm_TriggerTransmit(PduIdType TxPduId, PduInfoType* PduInfoPtr)
{
    (void)TxPduId;
    (void)PduInfoPtr;
    return E_OK;
}

/* ------------------------------------------------------------------ */
/* Configurations                                                      */
/* ------------------------------------------------------------------ */

#define BENCH_NUM_RX_PDUS   (64u)   /* == shadow CANIF_NUM_RX_PDUS */
#define BENCH_FIRST_CANID   (0x150u)

/* 64 Rx L-PDUs, all on HRH 0 -> one bucket of 64 entries to scan */
static const CanIf_RxPduConfigType benchRxPdus[BENCH_NUM_RX_PDUS] = {
    {  0u, 0x150u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    {  1u, 0x151u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    {  2u, 0x152u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    {  3u, 0x153u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    {  4u, 0x154u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    {  5u, 0x155u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    {  6u, 0x156u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    {  7u, 0x157u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    {  8u, 0x158u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    {  9u, 0x159u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 10u, 0x15Au, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 11u, 0x15Bu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 12u, 0x15Cu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 13u, 0x15Du, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 14u, 0x15Eu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 15u, 0x15Fu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 16u, 0x160u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 17u, 0x161u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 18u, 0x162u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 19u, 0x163u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 20u, 0x164u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 21u, 0x165u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 22u, 0x166u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 23u, 0x167u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 24u, 0x168u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 25u, 0x169u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 26u, 0x16Au, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 27u, 0x16Bu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 28u, 0x16Cu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 29u, 0x16Du, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 30u, 0x16Eu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 31u, 0x16Fu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 32u, 0x170u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 33u, 0x171u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 34u, 0x172u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 35u, 0x173u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 36u, 0x174u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 37u, 0x175u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 38u, 0x176u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 39u, 0x177u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 40u, 0x178u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 41u, 0x179u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 42u, 0x17Au, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 43u, 0x17Bu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 44u, 0x17Cu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 45u, 0x17Du, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 46u, 0x17Eu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 47u, 0x17Fu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 48u, 0x180u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 49u, 0x181u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 50u, 0x182u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 51u, 0x183u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 52u, 0x184u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 53u, 0x185u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 54u, 0x186u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 55u, 0x187u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 56u, 0x188u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 57u, 0x189u, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 58u, 0x18Au, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 59u, 0x18Bu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 60u, 0x18Cu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 61u, 0x18Du, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 62u, 0x18Eu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE },
    { 63u, 0x18Fu, 0x7FFu, 0u, 0u, 0u, 8u, TRUE }
};

/* Tx side: PDU 0 is the PduR routing destination (CanId 0x100 via HTH 0) */
static const CanIf_TxPduConfigType benchTxPdus[CANIF_NUM_TX_PDUS] = {
    { 0U, 0x100U, 0U, 0U, 0U, 8U, TRUE,  FALSE, FALSE },
    { 1U, 0x200U, 0U, 0U, 0U, 8U, FALSE, FALSE, FALSE },
    { 2U, 0x300U, 0U, 1U, 0U, 8U, FALSE, FALSE, FALSE },
    { 3U, 0x700U, 0U, 1U, 0U, 8U, TRUE,  FALSE, FALSE }
};

static const CanIf_ControllerConfigType benchControllers[CANIF_NUM_CONTROLLERS] = {
    { 0U, 500000U, 0U, CANIF_CS_STOPPED, FALSE, FALSE, TRUE, FALSE }
};

static const CanIf_ConfigType benchCanIfConfig = {
    benchControllers, CANIF_NUM_CONTROLLERS,
    NULL_PTR, 0U,
    NULL_PTR, 0U,
    benchTxPdus, CANIF_NUM_TX_PDUS,
    benchRxPdus, BENCH_NUM_RX_PDUS,
    TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE
};

/* Com: byte-aligned signals on I-PDU 0 (18 bytes) — exercises the P1 Phase 8
 * byte-aligned memcpy fast path in Com_PackSignal/Com_UnpackSignal. */
static const Com_SignalConfigType benchSignals[6] = {
    { 0U,  0U,  8U, COM_LITTLE_ENDIAN, COM_PENDING,  COM_ALWAYS, 0U, 0U, 0U },
    { 1U, 16U, 16U, COM_LITTLE_ENDIAN, COM_PENDING,  COM_ALWAYS, 0U, 0U, 0U },
    { 2U, 32U, 32U, COM_LITTLE_ENDIAN, COM_PENDING,  COM_ALWAYS, 0U, 0U, 0U },
    { 3U, 64U, 16U, COM_BIG_ENDIAN,    COM_PENDING,  COM_ALWAYS, 0U, 0U, 0U },
    { 4U, 80U, 32U, COM_BIG_ENDIAN,    COM_PENDING,  COM_ALWAYS, 0U, 0U, 0U },
    { 5U, 96U,  8U, COM_LITTLE_ENDIAN, COM_TRIGGERED, COM_ALWAYS, 0U, 0U, 0U }
};

static const Com_IPduConfigType benchIPdus[1] = {
    { 0U, 18U, FALSE, 0U, 0U, 0U, 0U }
};

static const Com_ConfigType benchComConfig = { benchSignals, 6U, benchIPdus, 1U };

/* PduR: 3 receive paths (CanIf Rx PDU 0/31/63 -> Com IPDU 0, immediate) so
 * the benchmarked Rx dispatches reach the real Com_RxIndication, plus 1
 * transmit path (COM PDU 0 -> CanIf PDU 0) for the routing/full-chain
 * benchmarks. */
static const PduR_DestPduConfigType benchPduRRxDest =
    { 0U, PDUR_MODULE_COM, PDUR_DESTPDU_PROCESSING_IMMEDIATE, 0U };
static const PduR_DestPduConfigType benchPduRTxDest =
    { 0U, PDUR_MODULE_CANIF, PDUR_DESTPDU_PROCESSING_IMMEDIATE, 0U };

static const PduR_RoutingPathConfigType benchPduRPaths[4] = {
    { {  0U, PDUR_MODULE_CANIF, 8U }, &benchPduRRxDest, 1U, PDUR_ROUTING_PATH_DIRECT, FALSE },
    { { 31U, PDUR_MODULE_CANIF, 8U }, &benchPduRRxDest, 1U, PDUR_ROUTING_PATH_DIRECT, FALSE },
    { { 63U, PDUR_MODULE_CANIF, 8U }, &benchPduRRxDest, 1U, PDUR_ROUTING_PATH_DIRECT, FALSE },
    { {  0U, PDUR_MODULE_COM,   8U }, &benchPduRTxDest, 1U, PDUR_ROUTING_PATH_DIRECT, FALSE }
};

static const PduR_ConfigType benchPduRConfig =
    { benchPduRPaths, 4U, NULL_PTR, 0U, TRUE, TRUE };

/* ------------------------------------------------------------------ */
/* Campaigns                                                           */
/* ------------------------------------------------------------------ */

static perf_stats_t g_rx_first;
static perf_stats_t g_rx_mid;
static perf_stats_t g_rx_last;
static perf_stats_t g_rx_miss;
static perf_stats_t g_pack8;
static perf_stats_t g_pack16;
static perf_stats_t g_pack32;
static perf_stats_t g_pack16be;
static perf_stats_t g_pack32be;
static perf_stats_t g_unpack8;
static perf_stats_t g_unpack16;
static perf_stats_t g_unpack32;
static perf_stats_t g_unpack16be;
static perf_stats_t g_unpack32be;
static perf_stats_t g_pdur_route;
static perf_stats_t g_full_tx_chain;

/* Received-frame payload: 18 bytes so every configured signal (up to bit
 * 103) round-trips through Com_RxIndication's IPDU-buffer copy. */
static uint8_t benchRxData[18];

/* Byte 0 = the 8-bit signal 0 payload used by the end-to-end Rx checks. */
#define BENCH_RX_SIGNAL0_BYTE   (0x5Au)

static void bench_stack_init(void)
{
    CanIf_Init(&benchCanIfConfig);
    /* CanIf_Transmit requires a STARTED controller and non-OFFLINE PDU mode. */
    (void)CanIf_SetControllerMode(0U, CANIF_CS_STARTED);
    (void)CanIf_SetPduMode(0U, CANIF_ONLINE);
    PduR_Init(&benchPduRConfig);
    Com_Init(&benchComConfig);
}

static void bench_make_pdu(PduInfoType* pdu)
{
    pdu->SduDataPtr = benchRxData;
    pdu->SduLength = 18U;
    pdu->MetaDataPtr = NULL_PTR;
}

void setUp(void)
{
    uint32_t i;

    mock_DetCalls = 0u;
    mock_CanWriteCalls = 0u;
    mock_DcmRxIndCalls = 0u;

    for (i = 0u; i < (uint32_t)sizeof(benchRxData); i++) {
        benchRxData[i] = (uint8_t)(0x10u + i);
    }
    benchRxData[0] = BENCH_RX_SIGNAL0_BYTE;

    bench_stack_init();
}

void tearDown(void)
{
}

/* --- CanIf Rx dispatch at different PDU counts --------------------- */

/** Shared timing body: dispatch the given CAN id 20000 times. */
static void bench_rx_dispatch(Can_IdType canId, perf_stats_t* stats)
{
    const uint32_t iterations = 20000u;
    Can_HwType mbox;
    PduInfoType pdu;
    uint32_t i;

    mbox.CanId = canId;
    mbox.Hoh = 0u;          /* HRH 0 -> the 64-entry bucket */
    mbox.ControllerId = 0u;
    bench_make_pdu(&pdu);

    for (i = 0u; i < iterations; i++) {
        uint64_t t0 = perf_now_ns();
        CanIf_RxIndication(&mbox, &pdu);
        uint64_t t1 = perf_now_ns();
        perf_record(stats, t1 - t0);
    }
}

void test_00_CanIfRxDispatch_MatchFirstOf64(void)
{
    uint8_t rx8 = 0u;

    /* correctness pre-check: entry 0 routes CanIf -> PduR -> Com and signal 0
     * (byte 0) round-trips end to end */
    {
        Can_HwType mbox;
        PduInfoType pdu;
        mbox.CanId = BENCH_FIRST_CANID;
        mbox.Hoh = 0u;
        mbox.ControllerId = 0u;
        bench_make_pdu(&pdu);
        CanIf_RxIndication(&mbox, &pdu);
        TEST_ASSERT_EQUAL_UINT8(E_OK, Com_ReceiveSignal(0u, &rx8));
        TEST_ASSERT_EQUAL_UINT8(BENCH_RX_SIGNAL0_BYTE, rx8);
    }

    bench_rx_dispatch(BENCH_FIRST_CANID, &g_rx_first);
    perf_report("CanIf_RxIndication dispatch | 64-entry table, match @ entry 0  (1 PDU scanned)", &g_rx_first);
    TEST_ASSERT_EQUAL_UINT8(BENCH_RX_SIGNAL0_BYTE, rx8);
}

void test_01_CanIfRxDispatch_MatchMidOf64(void)
{
    uint8_t rx8 = 0u;

    bench_rx_dispatch(BENCH_FIRST_CANID + 31u, &g_rx_mid);
    perf_report("CanIf_RxIndication dispatch | 64-entry table, match @ entry 31 (32 PDUs scanned)", &g_rx_mid);
    /* entry 31 -> CanIf PduId 31 -> PduR Rx path -> Com IPDU 0 */
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_ReceiveSignal(0u, &rx8));
    TEST_ASSERT_EQUAL_UINT8(BENCH_RX_SIGNAL0_BYTE, rx8);
}

void test_02_CanIfRxDispatch_MatchLastOf64(void)
{
    uint8_t rx8 = 0u;

    bench_rx_dispatch(BENCH_FIRST_CANID + 63u, &g_rx_last);
    perf_report("CanIf_RxIndication dispatch | 64-entry table, match @ entry 63 (64 PDUs scanned)", &g_rx_last);
    /* entry 63 -> CanIf PduId 63 -> PduR Rx path -> Com IPDU 0 */
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_ReceiveSignal(0u, &rx8));
    TEST_ASSERT_EQUAL_UINT8(BENCH_RX_SIGNAL0_BYTE, rx8);
}

void test_03_CanIfRxDispatch_MissFullScanDet(void)
{
    uint8_t rx8 = 0u;

    bench_rx_dispatch(0x7AAu, &g_rx_miss);
    perf_report("CanIf_RxIndication dispatch | 64-entry table, no match     (64 PDUs scanned, DET)", &g_rx_miss);
    /* nothing was routed into Com (Com_Init cleared the IPDU buffer):
     * byte 0 never became the marker value */
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_ReceiveSignal(0u, &rx8));
    TEST_ASSERT_EQUAL_UINT8(0u, rx8);
    TEST_ASSERT_TRUE(mock_DetCalls > 0u);
}

/* --- Com signal pack/unpack (8/16/32-bit, both endiannesses) ------- */

void test_10_Com_PackUnpack_8_16_32(void)
{
    const uint32_t iterations = 10000u;
    uint8_t v8;
    uint16_t v16;
    uint32_t v32;
    uint8_t r8;
    uint16_t r16;
    uint32_t r32;
    uint32_t i;

    /* correctness first: one full round trip per signal type */
    v8 = 0xA5u;
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_SendSignal(0u, &v8));
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_ReceiveSignal(0u, &r8));
    TEST_ASSERT_EQUAL_UINT8(0xA5u, r8);

    v16 = 0x1234u;
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_SendSignal(1u, &v16));
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_ReceiveSignal(1u, &r16));
    TEST_ASSERT_EQUAL_UINT16(0x1234u, r16);

    v32 = 0xDEADBEEFu;
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_SendSignal(2u, &v32));
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_ReceiveSignal(2u, &r32));
    TEST_ASSERT_EQUAL_UINT32(0xDEADBEEFu, r32);

    v16 = 0xCAFEu;
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_SendSignal(3u, &v16));
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_ReceiveSignal(3u, &r16));
    TEST_ASSERT_EQUAL_UINT16(0xCAFEu, r16);

    v32 = 0x0DDBA115u;
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_SendSignal(4u, &v32));
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_ReceiveSignal(4u, &r32));
    TEST_ASSERT_EQUAL_UINT32(0x0DDBA115u, r32);

    /* timing: pack-only (PENDING property, no transmission) */
    for (i = 0u; i < iterations; i++) {
        uint64_t t0;
        uint64_t t1;
        v8 = (uint8_t)(0x10u + i);
        t0 = perf_now_ns();
        (void)Com_SendSignal(0u, &v8);
        t1 = perf_now_ns();
        perf_record(&g_pack8, t1 - t0);

        v16 = (uint16_t)(0x1000u + i);
        t0 = perf_now_ns();
        (void)Com_SendSignal(1u, &v16);
        t1 = perf_now_ns();
        perf_record(&g_pack16, t1 - t0);

        v32 = 0x10000000u + (uint32_t)i;
        t0 = perf_now_ns();
        (void)Com_SendSignal(2u, &v32);
        t1 = perf_now_ns();
        perf_record(&g_pack32, t1 - t0);

        v16 = (uint16_t)(0xB000u + i);
        t0 = perf_now_ns();
        (void)Com_SendSignal(3u, &v16);
        t1 = perf_now_ns();
        perf_record(&g_pack16be, t1 - t0);

        v32 = 0xB0000000u + (uint32_t)i;
        t0 = perf_now_ns();
        (void)Com_SendSignal(4u, &v32);
        t1 = perf_now_ns();
        perf_record(&g_pack32be, t1 - t0);
    }

    /* timing: unpack */
    for (i = 0u; i < iterations; i++) {
        uint64_t t0;
        uint64_t t1;
        t0 = perf_now_ns();
        (void)Com_ReceiveSignal(0u, &r8);
        t1 = perf_now_ns();
        perf_record(&g_unpack8, t1 - t0);

        t0 = perf_now_ns();
        (void)Com_ReceiveSignal(1u, &r16);
        t1 = perf_now_ns();
        perf_record(&g_unpack16, t1 - t0);

        t0 = perf_now_ns();
        (void)Com_ReceiveSignal(2u, &r32);
        t1 = perf_now_ns();
        perf_record(&g_unpack32, t1 - t0);

        t0 = perf_now_ns();
        (void)Com_ReceiveSignal(3u, &r16);
        t1 = perf_now_ns();
        perf_record(&g_unpack16be, t1 - t0);

        t0 = perf_now_ns();
        (void)Com_ReceiveSignal(4u, &r32);
        t1 = perf_now_ns();
        perf_record(&g_unpack32be, t1 - t0);
    }

    perf_report("Com_SendSignal pack    |  8-bit LE, byte-aligned (memcpy fast path)", &g_pack8);
    perf_report("Com_SendSignal pack    | 16-bit LE, byte-aligned (memcpy fast path)", &g_pack16);
    perf_report("Com_SendSignal pack    | 32-bit LE, byte-aligned (memcpy fast path)", &g_pack32);
    perf_report("Com_SendSignal pack    | 16-bit BE, byte-aligned (memcpy fast path)", &g_pack16be);
    perf_report("Com_SendSignal pack    | 32-bit BE, byte-aligned (memcpy fast path)", &g_pack32be);
    perf_report("Com_ReceiveSignal unpack |  8-bit LE, byte-aligned (memcpy fast path)", &g_unpack8);
    perf_report("Com_ReceiveSignal unpack | 16-bit LE, byte-aligned (memcpy fast path)", &g_unpack16);
    perf_report("Com_ReceiveSignal unpack | 32-bit LE, byte-aligned (memcpy fast path)", &g_unpack32);
    perf_report("Com_ReceiveSignal unpack | 16-bit BE, byte-aligned (memcpy fast path)", &g_unpack16be);
    perf_report("Com_ReceiveSignal unpack | 32-bit BE, byte-aligned (memcpy fast path)", &g_unpack32be);
}

/* --- PduR routing --------------------------------------------------- */
void test_20_PduR_TransmitRouting(void)
{
    const uint32_t iterations = 20000u;
    PduInfoType pdu;
    uint8_t data[8];
    uint32_t i;
    Std_ReturnType ret;

    (void)memset(data, 0x5A, sizeof(data));
    pdu.SduDataPtr = data;
    pdu.SduLength = 8U;
    pdu.MetaDataPtr = NULL_PTR;

    /* correctness pre-check: COM PDU 0 routes to CanIf PDU 0 -> Can_Write */
    mock_CanWriteCalls = 0u;
    ret = PduR_Transmit(0u, &pdu);
    TEST_ASSERT_EQUAL(E_OK, ret);
    TEST_ASSERT_EQUAL_UINT32(1u, mock_CanWriteCalls);

    for (i = 0u; i < iterations; i++) {
        uint64_t t0 = perf_now_ns();
        (void)PduR_Transmit(0u, &pdu);
        uint64_t t1 = perf_now_ns();
        perf_record(&g_pdur_route, t1 - t0);
    }

    perf_report("PduR_Transmit routing | COM -> CanIf, 1 dest, immediate", &g_pdur_route);
}

/* --- full transmit chain (pack + PduR + CanIf + Can) ----------------- */
void test_30_Com_FullTxChain(void)
{
    const uint32_t iterations = 10000u;
    uint8_t v8;
    uint32_t i;

    /* correctness pre-check: TRIGGERED send reaches Can_Write */
    mock_CanWriteCalls = 0u;
    v8 = 0x77u;
    TEST_ASSERT_EQUAL_UINT8(E_OK, Com_SendSignal(5u, &v8));
    TEST_ASSERT_EQUAL_UINT32(1u, mock_CanWriteCalls);

    for (i = 0u; i < iterations; i++) {
        uint64_t t0;
        uint64_t t1;
        v8 = (uint8_t)i;
        t0 = perf_now_ns();
        (void)Com_SendSignal(5u, &v8);
        t1 = perf_now_ns();
        perf_record(&g_full_tx_chain, t1 - t0);
    }

    perf_report("Com_SendSignal TRIGGERED full chain | pack + PduR + CanIf + Can_Write", &g_full_tx_chain);
}

/* ------------------------------------------------------------------ */
/* Runner                                                              */
/* ------------------------------------------------------------------ */

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_00_CanIfRxDispatch_MatchFirstOf64);
    RUN_TEST(test_01_CanIfRxDispatch_MatchMidOf64);
    RUN_TEST(test_02_CanIfRxDispatch_MatchLastOf64);
    RUN_TEST(test_03_CanIfRxDispatch_MissFullScanDet);
    RUN_TEST(test_10_Com_PackUnpack_8_16_32);
    RUN_TEST(test_20_PduR_TransmitRouting);
    RUN_TEST(test_30_Com_FullTxChain);

    return UNITY_END();
}
