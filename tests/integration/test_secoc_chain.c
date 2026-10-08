/**
 * @file test_secoc_chain.c
 * @brief SecOC + Csm + Crypto + KeyM full-chain integration test (P1 Phase 8)
 * @req SWS_SecOC, SWS_Csm, SWS_KeyM
 *
 * Chain under test — real production code on both sides of every interface:
 *
 *   KeyM_SetKey (key provisioning)
 *        |
 *        v  (ECU-integration mapping, see injectKey())
 *   Csm keystore (Csm_KeyElementSet + Csm_KeySetValid)
 *        |
 *        v
 *   SecOC_IfTransmit ──► SecOC_MainFunctionTx
 *        |                    |  HMAC-SHA256 MAC via real Csm job pipeline
 *        |                    v  (mbedTLS backend, Csm_Cfg_HwService)
 *        |              PduR_SecOCTransmit (captured "bus" frame)
 *        |                    |
 *        v                    v
 *   SecOC_IfRxIndication ──► SecOC_MainFunctionRx
 *        |                    |  MAC verification + freshness reconstruction
 *        v                    v
 *   PduR_SecOCRxIndication (verified payload forwarded to the upper layer)
 *
 * Test doubles are limited to the PduR boundary (frame capture), the Det
 * recorder and the SchM interrupt-lock primitives. The crypto backend is the
 * real mbedTLS HMAC-SHA256; the key material flows through the real KeyM and
 * Csm key stores.
 *
 * Secured frame layout produced/consumed by this configuration:
 *   [ payload (8B) ][ truncated freshness (2B, big-endian) ][ MAC (16B) ]
 * with MAC input = dataId(1B) + full freshness (4B, big-endian) + payload.
 */

#include "unity.h"
#include "SecOC.h"
#include "Csm.h"
#include "Csm_Cfg.h"    /* CSM_KEY_ID_MASTER / CSM_KEY_ELEMENT_ID_SECRET */
#include "KeyM.h"
#include <string.h>

/*============================================================================*/
/*                              Test doubles                                  */
/*============================================================================*/

#define CHAIN_BUF_LEN     64u
#define CHAIN_DATA_LEN    8u
/* [data 8][freshness TX 16bit = 2B][MAC SECOC_AUTH_INFO_LENGTH = 16B] */
#define CHAIN_SECURED_LEN (CHAIN_DATA_LEN + (SECOC_FRESHNESS_VALUE_TX_LENGTH / 8u) \
                           + SECOC_AUTH_INFO_LENGTH)

/* ---- Det recorder (shared by SecOC, Csm and KeyM) ---- */
static uint32 detCalls;
static uint16 detLastModuleId;
static uint8  detLastApiId;
static uint8  detLastErrorId;

Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId,
                               uint8 ApiId, uint8 ErrorId)
{
    (void)InstanceId;
    detLastModuleId = ModuleId;
    detLastApiId = ApiId;
    detLastErrorId = ErrorId;
    detCalls++;
    return E_OK;
}

/* ---- NvM stubs (SecOC freshness persistence, P0-2) ---- */
Std_ReturnType NvM_ReadBlock(uint16 BlockId, void* DstPtr) {
    (void)BlockId;
    (void)DstPtr;
    return E_NOT_OK;
}

Std_ReturnType NvM_WriteBlock(uint16 BlockId, const void* SrcPtr) {
    (void)BlockId;
    (void)SrcPtr;
    return E_OK;
}

/* ---- PduR boundary: the "bus" between the Tx and Rx SecOC instances ---- */
static uint8         txFrame[CHAIN_BUF_LEN];   /* secured frame on the bus  */
static PduLengthType txFrameLen;
static PduIdType     txFramePduId;
static uint32        txFrameCount;

static uint8         rxPayload[CHAIN_BUF_LEN]; /* verified payload upstream */
static PduLengthType rxPayloadLen;
static PduIdType     rxPayloadPduId;
static uint32        rxPayloadCount;

Std_ReturnType PduR_SecOCTransmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr)
{
    txFramePduId = TxPduId;
    if ((PduInfoPtr != NULL_PTR) && (PduInfoPtr->SduDataPtr != NULL_PTR) &&
        (PduInfoPtr->SduLength <= CHAIN_BUF_LEN)) {
        (void)memcpy(txFrame, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
        txFrameLen = PduInfoPtr->SduLength;
    }
    txFrameCount++;
    return E_OK;
}

void PduR_SecOCRxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr)
{
    rxPayloadPduId = RxPduId;
    if ((PduInfoPtr != NULL_PTR) && (PduInfoPtr->SduDataPtr != NULL_PTR) &&
        (PduInfoPtr->SduLength <= CHAIN_BUF_LEN)) {
        (void)memcpy(rxPayload, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
        rxPayloadLen = PduInfoPtr->SduLength;
    }
    rxPayloadCount++;
}

/* ---- SchM_SecOC exclusive-area primitives ---- */
void Mcal_DisableAllInterrupts(void) {}
void Mcal_EnableAllInterrupts(void) {}

/* ---- CryIf stubs (same precedent as tests/bsw/services/csm) ----
 * Csm.c references the CryIf object-oriented key services; they are outside
 * the SecOC chain scope (which uses Csm_KeyElementSet/Csm_KeySetValid only),
 * so minimal fail-closed stubs keep the executable linkable without pulling
 * in the CryIf/Crypto library stack. */
Std_ReturnType CryIf_KeyDerive(uint32 keyId, uint32 targetKeyId)
{
    (void)keyId; (void)targetKeyId;
    return E_NOT_OK;
}
Std_ReturnType CryIf_KeyExchangeCalcPubValue(uint32 keyId)
{
    (void)keyId;
    return E_NOT_OK;
}
Std_ReturnType CryIf_KeyExchangeCalcSecret(uint32 keyId, const uint8* partnerPublicValuePtr,
                                           uint32 partnerPublicValueLength)
{
    (void)keyId; (void)partnerPublicValuePtr; (void)partnerPublicValueLength;
    return E_NOT_OK;
}
Std_ReturnType CryIf_KeyGenerate(uint32 keyId)
{
    (void)keyId;
    return E_NOT_OK;
}

/*============================================================================*/
/*                          Chain configuration                               */
/*============================================================================*/

/* NOTE: SecOC derives the MAC dataId from the PDU id itself (SecOC_ProcessTxPdu
 * / SecOC_ProcessRxPdu use (uint8)pduId), so the loopback requires TxPduId ==
 * RxPduId == 0 and the configured dataId documents that same value. */
static const SecOC_AuthBuildConfigType chainAuthConfig = {
    SECOC_HMAC_SHA256,          /* algorithm                                */
    SECOC_AUTH_INFO_LENGTH,     /* authInfoLength: 16B truncated MAC        */
    0x00u                       /* dataId == PDU id (loopback constraint)   */
};

static const SecOC_FreshnessValueConfigType chainFreshnessConfig = {
    SECOC_COUNTER,                          /* type                         */
    0x00u,                                  /* freshnessValueId             */
    SECOC_FRESHNESS_VALUE_LENGTH,           /* 32 bits total freshness      */
    SECOC_FRESHNESS_VALUE_TX_LENGTH         /* 16 bits transmitted on wire  */
};

static const SecOC_PduConfigType chainTxPdu = {
    0x00u,                  /* pduId                                             */
    0x00u,                  /* lowerLayerPduId                                   */
    SECOC_IFPDU,            /* pduType                                           */
    chainAuthConfig,
    chainFreshnessConfig,
    FALSE,                  /* useCryptographicPdu                               */
    0x00u,                  /* dataToAuthOffset                                  */
    CHAIN_DATA_LEN,         /* dataToAuthLength                                  */
    (uint16)CHAIN_SECURED_LEN /* authPduLength: data + freshness + MAC          */
};

static const SecOC_PduConfigType chainRxPdu = {
    0x00u, 0x00u, SECOC_IFPDU,
    chainAuthConfig,
    chainFreshnessConfig,
    FALSE, 0x00u, CHAIN_DATA_LEN,
    (uint16)CHAIN_SECURED_LEN
};

static const SecOC_ConfigType chainConfig = {
    &chainTxPdu, 1u,
    &chainRxPdu, 1u,
    10u, 10u,               /* mainFunctionPeriodRx/Tx [ms]                      */
    TRUE,                   /* devErrorDetect                                    */
    TRUE,                   /* versionInfoApi                                    */
    TRUE                    /* overrideStatusAllowed                             */
};

/* KeyM_Init only stores the config pointer; a zeroed config is sufficient. */
static KeyM_ConfigType keyMConfig;

/* 256-bit HMAC-SHA256 key material */
static uint8 chainKeyA[32] = {
    0xA5, 0x5A, 0x3C, 0xC3, 0x69, 0x96, 0xF0, 0x0F,
    0x1E, 0xE1, 0x2D, 0xD2, 0x4B, 0xB4, 0x87, 0x78,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x0F, 0xF0
};
static uint8 chainKeyB[32] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
    0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x23, 0x45, 0x67,
    0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98
};

static uint8 chainPayload[CHAIN_DATA_LEN] = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
};

/*============================================================================*/
/*                               Helpers                                      */
/*============================================================================*/

/**
 * @brief Provision the chain key end-to-end.
 *
 * ECU-integration mapping: KeyM is the provisioning owner (KeyM_SetKey stores
 * the material in the KeyM key database), while the SecOC MAC jobs read the
 * Csm keystore slot CSM_KEY_ID_MASTER / CSM_KEY_ELEMENT_ID_SECRET (fixed by
 * the Csm_Cfg_HwService backend). This codebase has no automatic KeyM->Csm
 * bridge, so the ECU integration layer mirrors the provisioned key into the
 * Csm keystore and marks it valid — exactly what this helper performs and
 * what a real BswM/EcuM bootstrap sequence would do.
 */
static Std_ReturnType injectKey(const uint8* key)
{
    Std_ReturnType ret;

    ret = KeyM_SetKey(KEYM_KEY_ID_HMAC_SHA256, key, 32u, KEYM_KEY_FORMAT_RAW);
    if (ret != E_OK) {
        return ret;
    }
    ret = Csm_KeyElementSet(CSM_KEY_ID_MASTER, CSM_KEY_ELEMENT_ID_SECRET,
                            key, 32u);
    if (ret != E_OK) {
        return ret;
    }
    return Csm_KeySetValid(CSM_KEY_ID_MASTER);
}

static void resetRecorders(void)
{
    detCalls = 0u;
    detLastModuleId = 0u;
    detLastApiId = 0xFFu;
    detLastErrorId = 0xFFu;
    (void)memset(txFrame, 0, sizeof(txFrame));
    txFrameLen = 0u;
    txFramePduId = 0xFFFFu;
    txFrameCount = 0u;
    (void)memset(rxPayload, 0, sizeof(rxPayload));
    rxPayloadLen = 0u;
    rxPayloadPduId = 0xFFFFu;
    rxPayloadCount = 0u;
}

/* Tx leg: submit the payload to SecOC and run one MainFunctionTx cycle.
 * Post: exactly one secured frame captured from PduR_SecOCTransmit. */
static void chainTransmit(const uint8* payload, PduLengthType len)
{
    PduInfoType pduInfo;

    pduInfo.SduDataPtr = (uint8*)payload;   /* SecOC buffers the data */
    pduInfo.SduLength = len;
    pduInfo.MetaDataPtr = NULL_PTR;

    txFrameCount = 0u;
    TEST_ASSERT_EQUAL(E_OK, SecOC_IfTransmit(0x00u, &pduInfo));
    SecOC_MainFunctionTx();
    TEST_ASSERT_EQUAL_UINT32(1u, txFrameCount);
}

/* Rx leg: feed a captured bus frame back into SecOC and run one
 * MainFunctionRx cycle (verification + upstream forwarding). */
static void chainReceive(const uint8* securedFrame, PduLengthType len)
{
    PduInfoType pduInfo;

    pduInfo.SduDataPtr = (uint8*)securedFrame;
    pduInfo.SduLength = len;
    pduInfo.MetaDataPtr = NULL_PTR;

    rxPayloadCount = 0u;
    SecOC_IfRxIndication(0x00u, &pduInfo);
    SecOC_MainFunctionRx();
}

/*============================================================================*/
/*                             Test lifecycle                                 */
/*============================================================================*/

void setUp(void)
{
    /* Bootstrap the chain in integration order: Csm -> KeyM -> SecOC.
     * DeInit of an uninitialized module reports DET and returns early, which
     * is harmless here — the subsequent Init always reaches a clean state.
     * Recorders are cleared after bootstrap so any DET from the re-init
     * sequence is not charged to the individual test. */
    (void)Csm_DeInit();
    TEST_ASSERT_EQUAL(E_OK, Csm_Init(&Csm_Config));

    KeyM_DeInit();
    KeyM_Init(&keyMConfig);

    SecOC_DeInit();
    SecOC_Init(&chainConfig);

    TEST_ASSERT_EQUAL(E_OK, injectKey(chainKeyA));

    resetRecorders();
}

void tearDown(void)
{
}

/*============================================================================*/
/*                                 Tests                                      */
/*============================================================================*/

/**
 * Positive end-to-end chain: provisioned key -> authenticated Tx frame ->
 * verified Rx payload. Proves the key material actually reaches the mbedTLS
 * MAC backend and that the receiver reconstructs the same MAC input.
 */
static void test_Chain_Positive_TxAuthRxVerify_FullLoop(void)
{
    SecOC_VerificationResultType result = SECOC_NO_VERIFICATION;

    /* --- Tx leg --- */
    chainTransmit(chainPayload, CHAIN_DATA_LEN);

    /* Secured frame: [data 8][freshness 2][MAC 16] = 26 bytes */
    TEST_ASSERT_EQUAL_UINT32(CHAIN_SECURED_LEN, txFrameLen);
    TEST_ASSERT_EQUAL_UINT8(0x00u, txFramePduId);
    /* payload prefix must be untouched */
    TEST_ASSERT_EQUAL_MEMORY(chainPayload, txFrame, CHAIN_DATA_LEN);
    /* first transmission uses freshness counter 1 (incremented before build) */
    TEST_ASSERT_EQUAL_UINT8(0x00u, txFrame[CHAIN_DATA_LEN]);
    TEST_ASSERT_EQUAL_UINT8(0x01u, txFrame[CHAIN_DATA_LEN + 1u]);
    /* MAC must be non-trivial (a dead crypto path would leave zeros) */
    TEST_ASSERT_TRUE((txFrame[CHAIN_DATA_LEN + 2u] != 0u) ||
                     (txFrame[CHAIN_DATA_LEN + 3u] != 0u));

    /* --- Rx leg: loop the frame back through the receiver --- */
    chainReceive(txFrame, txFrameLen);

    TEST_ASSERT_EQUAL_UINT8(SECOC_VERIFICATIONSUCCESS_STATUS,
                            SecOC_GetVerificationStatus(0x00u));
    TEST_ASSERT_EQUAL(E_OK, SecOC_GetVerificationResult(0x00u, &result));
    TEST_ASSERT_EQUAL(SECOC_VERIFICATIONSUCCESS, result);

    /* verified payload forwarded exactly once, with original content */
    TEST_ASSERT_EQUAL_UINT32(1u, rxPayloadCount);
    TEST_ASSERT_EQUAL_UINT32(CHAIN_DATA_LEN, rxPayloadLen);
    TEST_ASSERT_EQUAL_UINT8(0x00u, rxPayloadPduId);
    TEST_ASSERT_EQUAL_MEMORY(chainPayload, rxPayload, CHAIN_DATA_LEN);

    /* the clean chain must not raise any DET */
    TEST_ASSERT_EQUAL_UINT32(0u, detCalls);
}

/** Tampered payload on the bus must fail verification and never be forwarded. */
static void test_Chain_TamperedPayload_FailsVerification(void)
{
    uint8 frame[CHAIN_SECURED_LEN];
    SecOC_VerificationResultType result = SECOC_NO_VERIFICATION;

    chainTransmit(chainPayload, CHAIN_DATA_LEN);
    (void)memcpy(frame, txFrame, CHAIN_SECURED_LEN);

    frame[3] ^= 0xFFu;      /* flip one payload byte in transit */

    chainReceive(frame, CHAIN_SECURED_LEN);

    TEST_ASSERT_EQUAL_UINT8(SECOC_VERIFICATIONFAILURE_STATUS,
                            SecOC_GetVerificationStatus(0x00u));
    TEST_ASSERT_EQUAL(E_OK, SecOC_GetVerificationResult(0x00u, &result));
    TEST_ASSERT_EQUAL(SECOC_VERIFICATIONFAILURE, result);
    TEST_ASSERT_EQUAL_UINT32(0u, rxPayloadCount);   /* nothing forwarded */
    TEST_ASSERT_EQUAL_UINT32(0u, detCalls);         /* runtime failure, no DET */
}

/**
 * Manipulated (stale/forged) truncated freshness changes the MAC input at the
 * receiver — verification must fail and the frame must be dropped.
 */
static void test_Chain_TamperedFreshness_FailsVerification(void)
{
    uint8 frame[CHAIN_SECURED_LEN];
    SecOC_VerificationResultType result = SECOC_NO_VERIFICATION;

    chainTransmit(chainPayload, CHAIN_DATA_LEN);
    (void)memcpy(frame, txFrame, CHAIN_SECURED_LEN);

    frame[CHAIN_DATA_LEN + 1u] ^= 0x01u;    /* freshness 0x0001 -> 0x0000 */

    chainReceive(frame, CHAIN_SECURED_LEN);

    TEST_ASSERT_EQUAL_UINT8(SECOC_VERIFICATIONFAILURE_STATUS,
                            SecOC_GetVerificationStatus(0x00u));
    TEST_ASSERT_EQUAL(E_OK, SecOC_GetVerificationResult(0x00u, &result));
    TEST_ASSERT_EQUAL(SECOC_VERIFICATIONFAILURE, result);
    TEST_ASSERT_EQUAL_UINT32(0u, rxPayloadCount);
}

/**
 * Receiver rollback attack: the frame is captured from the bus, then the
 * sender's key is rotated before the receiver verifies. The old MAC was built
 * with key A, the receiver computes with key B — verification must fail.
 */
static void test_Chain_WrongKey_FailsVerification(void)
{
    uint8 frame[CHAIN_SECURED_LEN];
    SecOC_VerificationResultType result = SECOC_NO_VERIFICATION;

    chainTransmit(chainPayload, CHAIN_DATA_LEN);
    (void)memcpy(frame, txFrame, CHAIN_SECURED_LEN);

    /* rotate the key after the frame was captured (mirror into KeyM + Csm) */
    TEST_ASSERT_EQUAL(E_OK, injectKey(chainKeyB));

    chainReceive(frame, CHAIN_SECURED_LEN);

    TEST_ASSERT_EQUAL_UINT8(SECOC_VERIFICATIONFAILURE_STATUS,
                            SecOC_GetVerificationStatus(0x00u));
    TEST_ASSERT_EQUAL(E_OK, SecOC_GetVerificationResult(0x00u, &result));
    TEST_ASSERT_EQUAL(SECOC_VERIFICATIONFAILURE, result);
    TEST_ASSERT_EQUAL_UINT32(0u, rxPayloadCount);
}

/** Frames received before module initialization must raise DET (E_UNINIT). */
static void test_Chain_IfRxIndication_BeforeInit_ReportsDet(void)
{
    PduInfoType pduInfo;
    uint8 frame[CHAIN_SECURED_LEN];

    SecOC_DeInit();     /* initialized: clean shutdown, no DET */
    resetRecorders();

    pduInfo.SduDataPtr = frame;
    pduInfo.SduLength = CHAIN_SECURED_LEN;
    pduInfo.MetaDataPtr = NULL_PTR;

    SecOC_IfRxIndication(0x00u, &pduInfo);

    TEST_ASSERT_EQUAL_UINT32(1u, detCalls);
    TEST_ASSERT_EQUAL_UINT16(SECOC_MODULE_ID, detLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(SECOC_SID_IFRXINDICATION, detLastApiId);
    TEST_ASSERT_EQUAL_UINT8(SECOC_E_UNINIT, detLastErrorId);
    TEST_ASSERT_EQUAL_UINT32(0u, rxPayloadCount);
}

/**
 * After the lower layer confirms the transmission, the main function must not
 * retransmit the consumed buffer (no duplicate frames on the bus).
 */
static void test_Chain_TxConfirmation_PreventsRetransmission(void)
{
    chainTransmit(chainPayload, CHAIN_DATA_LEN);
    TEST_ASSERT_EQUAL_UINT32(1u, txFrameCount);

    SecOC_TxConfirmation(0x00u, E_OK);
    SecOC_MainFunctionTx();     /* buffer already consumed */

    TEST_ASSERT_EQUAL_UINT32(1u, txFrameCount); /* no duplicate frame */
    TEST_ASSERT_EQUAL_UINT32(0u, detCalls);
}

/**
 * Freshness counter must advance monotonically across successive frames and
 * the receiver must track each value (low-16-bit monotonic path).
 */
static void test_Chain_FreshnessCounter_MonotonicAcrossFrames(void)
{
    SecOC_VerificationResultType result = SECOC_NO_VERIFICATION;

    /* frame 1: freshness 1 */
    chainTransmit(chainPayload, CHAIN_DATA_LEN);
    TEST_ASSERT_EQUAL_UINT8(0x00u, txFrame[CHAIN_DATA_LEN]);
    TEST_ASSERT_EQUAL_UINT8(0x01u, txFrame[CHAIN_DATA_LEN + 1u]);
    chainReceive(txFrame, txFrameLen);
    TEST_ASSERT_EQUAL_UINT8(SECOC_VERIFICATIONSUCCESS_STATUS,
                            SecOC_GetVerificationStatus(0x00u));

    /* frame 2: freshness 2 (same key, counter advanced) */
    chainTransmit(chainPayload, CHAIN_DATA_LEN);
    TEST_ASSERT_EQUAL_UINT8(0x00u, txFrame[CHAIN_DATA_LEN]);
    TEST_ASSERT_EQUAL_UINT8(0x02u, txFrame[CHAIN_DATA_LEN + 1u]);
    chainReceive(txFrame, txFrameLen);
    TEST_ASSERT_EQUAL_UINT8(SECOC_VERIFICATIONSUCCESS_STATUS,
                            SecOC_GetVerificationStatus(0x00u));
    TEST_ASSERT_EQUAL(E_OK, SecOC_GetVerificationResult(0x00u, &result));
    TEST_ASSERT_EQUAL(SECOC_VERIFICATIONSUCCESS, result);
    /* chainReceive() resets the capture counter per receive: frame 2 was
     * forwarded exactly once (frame 1's forward was already asserted above) */
    TEST_ASSERT_EQUAL_UINT32(1u, rxPayloadCount);
    TEST_ASSERT_EQUAL_UINT32(0u, detCalls);
}

/**
 * Verification status override (overrideStatusAllowed = TRUE): an authorized
 * diagnostic can override a failure, and the next authentic frame recovers
 * the PDU to a genuine SUCCESS state. Override on an invalid PDU is rejected.
 */
static void test_Chain_VerifyStatusOverride_AndRecovery(void)
{
    uint8 frame[CHAIN_SECURED_LEN];

    /* drive a genuine failure first */
    chainTransmit(chainPayload, CHAIN_DATA_LEN);
    (void)memcpy(frame, txFrame, CHAIN_SECURED_LEN);
    frame[0] ^= 0xFFu;
    chainReceive(frame, CHAIN_SECURED_LEN);
    TEST_ASSERT_EQUAL_UINT8(SECOC_VERIFICATIONFAILURE_STATUS,
                            SecOC_GetVerificationStatus(0x00u));

    /* override the status (diagnostic authority) */
    TEST_ASSERT_EQUAL(E_OK, SecOC_VerifyStatusOverride(0x00u, SECOC_VERIFICATIONOVERRIDE));
    TEST_ASSERT_EQUAL_UINT8(SECOC_VERIFICATIONOVERRIDE,
                            SecOC_GetVerificationStatus(0x00u));

    /* invalid PDU id must be rejected */
    TEST_ASSERT_EQUAL(E_NOT_OK,
                      SecOC_VerifyStatusOverride(99u, SECOC_VERIFICATIONOVERRIDE));

    /* recovery: the authentic frame verifies again and wins over the override */
    chainReceive(txFrame, CHAIN_SECURED_LEN);
    TEST_ASSERT_EQUAL_UINT8(SECOC_VERIFICATIONSUCCESS_STATUS,
                            SecOC_GetVerificationStatus(0x00u));
    TEST_ASSERT_EQUAL_UINT32(1u, rxPayloadCount);
}

/*============================================================================*/
/*                                 Main                                       */
/*============================================================================*/

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Chain_Positive_TxAuthRxVerify_FullLoop);
    RUN_TEST(test_Chain_TamperedPayload_FailsVerification);
    RUN_TEST(test_Chain_TamperedFreshness_FailsVerification);
    RUN_TEST(test_Chain_WrongKey_FailsVerification);
    RUN_TEST(test_Chain_IfRxIndication_BeforeInit_ReportsDet);
    RUN_TEST(test_Chain_TxConfirmation_PreventsRetransmission);
    RUN_TEST(test_Chain_FreshnessCounter_MonotonicAcrossFrames);
    RUN_TEST(test_Chain_VerifyStatusOverride_AndRecovery);
    return UnityEnd();
}
