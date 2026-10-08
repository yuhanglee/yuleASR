/**
 * @file test_dem_dtc_matrix.c
 * @brief Dem DTC matrix tests - P1 Phase 8 (P1 test infrastructure)
 *
 * Covers the Dem-side equivalents of the UDS 0x19/0x14 diagnostic flows:
 *  - DTC status bitmask combinations (reportDTCByStatusMask equivalent)
 *  - ClearDTC: single / all / functional group (UDS 0x14 mask semantics)
 *  - Freeze frame (snapshot) auto-store on confirmation, prestore, read, clear
 *  - Counter-based debounce (PREFAILED/PREPASSED progression vs per-event thresholds)
 *  - Time-based debounce (threshold accumulation + MainFunction commit)
 *  - Filtered DTC iteration (SelectDTC + GetNumberOfFilteredDTC + GetNextFilteredDTC)
 *  - DTC aging
 *
 * White-box access to Dem_InternalState via Dem_Int.h follows the existing
 * tests/bsw/services/dem/test_dem.c pattern.
 *
 * @tests src/bsw/services/dem/src/Dem.c
 * @tests src/bsw/services/dem/src/Dem_Int.c
 */
#include "unity.h"
#include "Dem.h"
#include "Dem_Int.h"
#include <string.h>

/* ---- DET recorder (test-local mock; tests/mocks/mock_det.c is NOT linked) ---- */
static uint8 mock_DetCalls = 0;
static uint8 mock_lastApiId = 0;
static uint8 mock_lastErrorId = 0;
Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId)
{
    (void)ModuleId;
    (void)InstanceId;
    mock_DetCalls++;
    mock_lastApiId = ApiId;
    mock_lastErrorId = ErrorId;
    return E_OK;
}

/* ---- ClearDTC notification callbacks ---- */
static uint8 clearStartCalls = 0;
static uint8 clearFinishCalls = 0;
static void matrix_ClearStartCb(void)  { clearStartCalls++; }
static void matrix_ClearFinishCb(void) { clearFinishCalls++; }

/* ---- DTC / event identifiers ---- */
#define EVT_A   (1U)                  /* DTC 0x123456 - no debounce, conf = 1 */
#define EVT_B   (2U)                  /* DTC 0x234567 - counter debounce 3 / -3 */
#define EVT_C   (3U)                  /* DTC 0x345678 - time debounce 100ms / 100ms */
#define DTC_A   (0x123456UL)
#define DTC_B   (0x234567UL)
#define DTC_C   (0x345678UL)
#define DTC_UNKNOWN (0x999999UL)
/* Functional groups (mask semantics): (dtc & group) == group */
#define GRP_A   (0x120000UL)          /* byte2 0x12: matches only DTC_A */
#define GRP_B   (0x230000UL)          /* byte2 0x23: matches only DTC_B */

/* ---- expected UDS status byte combinations ---- */
#define ST_PENDING  (DEM_UDS_STATUS_TNCSLC | DEM_UDS_STATUS_TNCTOC)                 /* 0x50 */
#define ST_FAILED   (DEM_UDS_STATUS_TF | DEM_UDS_STATUS_TFTOC |                     \
                     DEM_UDS_STATUS_PDTC | DEM_UDS_STATUS_CDTC |                    \
                     DEM_UDS_STATUS_TFSLC)                                          /* 0x2F */
#define ST_PASSED   (DEM_UDS_STATUS_TFTOC | DEM_UDS_STATUS_CDTC |                   \
                     DEM_UDS_STATUS_TFSLC)                                          /* 0x2A */
#define ST_AGED     (DEM_UDS_STATUS_TFTOC | DEM_UDS_STATUS_TFSLC)                   /* 0x22 */

/* ---- configuration ----
 * Dem_IntProcessDebounceMainFunction() iterates DEM_NUM_EVENTS config entries
 * without a NumEvents bound, so the array MUST be DEM_NUM_EVENTS sized.
 * Remaining entries are zero-initialized (DEM_DEBOUNCE_ALGORITHM_NONE == 0). */
static const Dem_EventParameterType matrix_EventParams[DEM_NUM_EVENTS] = {
    /* EventId, Dtc,    Prio, Avail, Report, FailCyc, ConfThr, Algorithm,  cb, tb, mi,  failThr, passThr, tFailMs, tPassMs */
    {  EVT_A,   DTC_A,   0U,  TRUE,  TRUE,   0U,      1U,  DEM_DEBOUNCE_ALGORITHM_NONE,
       FALSE, FALSE, FALSE, 127, -128, 0U, 0U },
    {  EVT_B,   DTC_B,   0U,  TRUE,  TRUE,   0U,      1U,  DEM_DEBOUNCE_ALGORITHM_COUNTER,
       FALSE, FALSE, FALSE,   3,   -3, 0U, 0U },
    {  EVT_C,   DTC_C,   0U,  TRUE,  TRUE,   0U,      1U,  DEM_DEBOUNCE_ALGORITHM_TIME,
       FALSE, FALSE, FALSE, 127, -128, 100U, 100U }
};

static const Dem_DtcParameterType matrix_DtcParams[3] = {
    { DTC_A, 0U, 0U, DEM_DTC_ORIGIN_PRIMARY_MEMORY, TRUE, TRUE, 2U, FALSE },
    { DTC_B, 0U, 0U, DEM_DTC_ORIGIN_PRIMARY_MEMORY, TRUE, TRUE, 2U, FALSE },
    { DTC_C, 0U, 0U, DEM_DTC_ORIGIN_PRIMARY_MEMORY, TRUE, TRUE, 2U, FALSE }
};

static const Dem_ConfigType matrix_Config = {
    matrix_EventParams, 3U,
    matrix_DtcParams, 3U,
    NULL_PTR, 0U,               /* freeze frame records */
    NULL_PTR, 0U,               /* extended data records */
    NULL_PTR, 0U,               /* indicators */
    TRUE, TRUE, TRUE, FALSE,
    0xFFU,
    FALSE, FALSE, FALSE, FALSE,
    NULL_PTR, matrix_ClearStartCb, matrix_ClearFinishCb
};

/* big scratch buffer kept off the stack */
static uint8 ffBuf[DEM_FREEZE_FRAME_MAX_SIZE];

void setUp(void)
{
    Dem_Init(&matrix_Config);
    mock_DetCalls = 0;
    mock_lastApiId = 0;
    mock_lastErrorId = 0;
    clearStartCalls = 0;
    clearFinishCalls = 0;
}

void tearDown(void)
{
}

/* ---- helpers ---- */
static Dem_UdsStatusByteType statusOf(Dem_DtcType dtc)
{
    Dem_UdsStatusByteType status = 0xFFU;
    (void)Dem_GetStatusOfDTC(dtc, DEM_DTC_ORIGIN_PRIMARY_MEMORY, &status);
    return status;
}

static Std_ReturnType report(Dem_EventIdType event, Dem_EventStatusType status)
{
    return Dem_SetEventStatus(event, status);
}

/*==================================================================================*
 *  A. DTC status bitmask combinations (reportDTCByStatusMask equivalent)          *
 *==================================================================================*/

/** Init state of a configured DTC: only the "test not completed" bits are set */
void test_StatusMask_AfterInit_PendingBitsOnly(void)
{
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_A));
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_B));

    TEST_ASSERT_EQUAL(DEM_UDS_STATUS_TNCSLC, statusOf(DTC_A) & DEM_UDS_STATUS_TNCSLC);
    TEST_ASSERT_EQUAL(DEM_UDS_STATUS_TNCTOC, statusOf(DTC_A) & DEM_UDS_STATUS_TNCTOC);
    TEST_ASSERT_EQUAL(0, statusOf(DTC_A) & DEM_UDS_STATUS_TF);
}

/** Single FAILED report (conf=1) sets TF|TFTOC|PDTC|CDTC|TFSLC and clears pending bits */
void test_StatusMask_SingleFailed_AllFailBitsSet(void)
{
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_A));

    TEST_ASSERT_EQUAL(DEM_UDS_STATUS_TF,    statusOf(DTC_A) & DEM_UDS_STATUS_TF);
    TEST_ASSERT_EQUAL(DEM_UDS_STATUS_TFTOC, statusOf(DTC_A) & DEM_UDS_STATUS_TFTOC);
    TEST_ASSERT_EQUAL(DEM_UDS_STATUS_PDTC,  statusOf(DTC_A) & DEM_UDS_STATUS_PDTC);
    TEST_ASSERT_EQUAL(DEM_UDS_STATUS_CDTC,  statusOf(DTC_A) & DEM_UDS_STATUS_CDTC);
    TEST_ASSERT_EQUAL(DEM_UDS_STATUS_TFSLC, statusOf(DTC_A) & DEM_UDS_STATUS_TFSLC);
    TEST_ASSERT_EQUAL(0, statusOf(DTC_A) & DEM_UDS_STATUS_TNCSLC);
    TEST_ASSERT_EQUAL(0, statusOf(DTC_A) & DEM_UDS_STATUS_TNCTOC);
}

/** PASSED after FAILED clears the active bits but keeps history (CDTC, TFSLC, TFTOC) */
void test_StatusMask_PassedAfterFailed_KeepsHistoryBits(void)
{
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_PASSED));
    TEST_ASSERT_EQUAL_UINT8(ST_PASSED, statusOf(DTC_A));
    TEST_ASSERT_EQUAL(0, statusOf(DTC_A) & DEM_UDS_STATUS_TF);
    TEST_ASSERT_EQUAL(0, statusOf(DTC_A) & DEM_UDS_STATUS_PDTC);
}

/** Status bits are tracked per DTC: failing A leaves B and C untouched */
void test_StatusMask_IndependentPerDtc(void)
{
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_A));
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_B));
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_C));
}

/** Unconfigured DTC yields DEM_STATUS_WRONG_DTC and a zeroed status */
void test_StatusMask_UnconfiguredDtc_ReturnsWrongDtc(void)
{
    Dem_UdsStatusByteType status = 0xFFU;
    TEST_ASSERT_EQUAL(DEM_STATUS_WRONG_DTC,
        Dem_GetStatusOfDTC(DTC_UNKNOWN, DEM_DTC_ORIGIN_PRIMARY_MEMORY, &status));
    TEST_ASSERT_EQUAL_UINT8(0U, status);
}

/** Occurrence counter increments per FAILED report; confirmation at threshold */
void test_StatusMask_OccurrenceCounter_IncrementsPerFailure(void)
{
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL_UINT32(1U, Dem_InternalState.DTCEntries[0].OccurrenceCounter);
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL_UINT32(2U, Dem_InternalState.DTCEntries[0].OccurrenceCounter);
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_A));
}

/** Availability mask is 0xFF (all 8 UDS status bits supported) */
void test_StatusMask_AvailabilityMask_Is0xFF(void)
{
    uint8 mask = 0U;
    TEST_ASSERT_EQUAL(E_OK, Dem_GetDTCStatusAvailabilityMask(&mask));
    TEST_ASSERT_EQUAL_UINT8(0xFFU, mask);
}

/*==================================================================================*
 *  B. ClearDTC: single / all / functional group                                   *
 *==================================================================================*/

/** Single DTC clear resets its status and occurrence counter, keeps other DTCs */
void test_Clear_SingleDtc_ResetsStatusKeepsOthers(void)
{
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL(E_OK, report(EVT_B, DEM_EVENT_STATUS_FAILED));

    TEST_ASSERT_EQUAL(E_OK,
        Dem_ClearDTC(DTC_A, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));

    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_A));
    TEST_ASSERT_EQUAL_UINT32(0U, Dem_InternalState.DTCEntries[0].OccurrenceCounter);
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_B));
    TEST_ASSERT_EQUAL(1, clearStartCalls);
    TEST_ASSERT_EQUAL(1, clearFinishCalls);
    TEST_ASSERT_EQUAL(0, mock_DetCalls);
}

/** Clear all (DEM_DTC_GROUP_ALL) resets every entry */
void test_Clear_AllDtcs_ClearsEverything(void)
{
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL(E_OK, report(EVT_B, DEM_EVENT_STATUS_FAILED));
    /* EVT_C is time-debounced: a direct FAILED report never commits DTC
     * status - drive the PREFAILED counting path through Dem_MainFunction */
    TEST_ASSERT_EQUAL(E_OK, report(EVT_C, DEM_EVENT_STATUS_PREFAILED));
    Dem_IntProcessTimeBasedDebounce(EVT_C, DEM_EVENT_STATUS_PREFAILED, 60U);
    Dem_IntProcessTimeBasedDebounce(EVT_C, DEM_EVENT_STATUS_PREFAILED, 60U);
    Dem_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_C));

    TEST_ASSERT_EQUAL(E_OK,
        Dem_ClearDTC(DEM_DTC_GROUP_ALL, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));

    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_A));
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_B));
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_C));
    TEST_ASSERT_EQUAL(TRUE, Dem_InternalState.DTCEntries[0].IsDeleted);
    TEST_ASSERT_EQUAL(TRUE, Dem_InternalState.DTCEntries[1].IsDeleted);
    TEST_ASSERT_EQUAL(TRUE, Dem_InternalState.DTCEntries[2].IsDeleted);
    TEST_ASSERT_EQUAL(1, clearStartCalls);
    TEST_ASSERT_EQUAL(1, clearFinishCalls);
}

/** Functional group clear (UDS 0x14 mask semantics) clears only matching DTCs */
void test_Clear_FunctionalGroup_ClearsOnlyMatchingDtcs(void)
{
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL(E_OK, report(EVT_B, DEM_EVENT_STATUS_FAILED));
    /* EVT_C is time-debounced: a direct FAILED report never commits DTC
     * status - drive the PREFAILED counting path through Dem_MainFunction */
    TEST_ASSERT_EQUAL(E_OK, report(EVT_C, DEM_EVENT_STATUS_PREFAILED));
    Dem_IntProcessTimeBasedDebounce(EVT_C, DEM_EVENT_STATUS_PREFAILED, 60U);
    Dem_IntProcessTimeBasedDebounce(EVT_C, DEM_EVENT_STATUS_PREFAILED, 60U);
    Dem_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_C));

    /* group byte2 = 0x12 matches only DTC_A (0x123456) */
    TEST_ASSERT_EQUAL(1U, Dem_IntCountDTCGroupMatches(GRP_A));
    TEST_ASSERT_EQUAL(E_OK,
        Dem_ClearDTC(GRP_A, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_A));
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_B));
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_C));
    TEST_ASSERT_EQUAL(TRUE, Dem_InternalState.DTCEntries[0].IsDeleted);
    TEST_ASSERT_EQUAL(FALSE, Dem_InternalState.DTCEntries[1].IsDeleted);

    /* group byte2 = 0x23 matches only DTC_B (0x234567) */
    TEST_ASSERT_EQUAL(1U, Dem_IntCountDTCGroupMatches(GRP_B));
    TEST_ASSERT_EQUAL(E_OK,
        Dem_ClearDTC(GRP_B, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_B));
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_C));
}

/** Unknown DTC with no group match reports DET and does not run notifications */
void test_Clear_UnknownDtcNoGroupMatch_ReportsDet(void)
{
    TEST_ASSERT_EQUAL(E_NOT_OK,
        Dem_ClearDTC(DTC_UNKNOWN, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));
    TEST_ASSERT_EQUAL(1, mock_DetCalls);
    TEST_ASSERT_EQUAL(DEM_SID_CLEARDTC, mock_lastApiId);
    TEST_ASSERT_EQUAL(DEM_E_PARAM_DATA, mock_lastErrorId);
    TEST_ASSERT_EQUAL(0, clearStartCalls);
    TEST_ASSERT_EQUAL(0, clearFinishCalls);
}

/** ClearDTC also removes the stored freeze frame of the cleared DTC */
void test_Clear_RemovesFreezeFrame(void)
{
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL(TRUE, Dem_InternalState.FreezeFrames[0].IsValid);

    TEST_ASSERT_EQUAL(E_OK,
        Dem_ClearDTC(DTC_A, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));
    TEST_ASSERT_EQUAL(FALSE, Dem_InternalState.FreezeFrames[0].IsValid);
}

/*==================================================================================*
 *  C. Freeze frame (snapshot) store / read / clear                                *
 *==================================================================================*/

/** Confirmation (occurrence >= conf threshold) auto-stores a freeze frame */
void test_FreezeFrame_AutoStoreOnConfirmation(void)
{
    uint16 j;

    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));

    TEST_ASSERT_EQUAL(TRUE,  Dem_InternalState.FreezeFrames[0].IsValid);
    TEST_ASSERT_EQUAL_UINT16(0U,    Dem_InternalState.FreezeFrames[0].DtcIndex);
    TEST_ASSERT_EQUAL_UINT16(DEM_FREEZE_FRAME_MAX_SIZE, Dem_InternalState.FreezeFrames[0].Length);
    /* sample pattern: Data[j] = j + DtcIndex + slotIndex = j + 0 + 0 */
    for (j = 0U; j < DEM_FREEZE_FRAME_MAX_SIZE; j++)
    {
        TEST_ASSERT_EQUAL_UINT8((uint8)j, Dem_InternalState.FreezeFrames[0].Data[j]);
    }
}

/** Public freeze-frame read returns the stored snapshot; robust against bad args */
void test_FreezeFrame_PublicRead_RoundTrip(void)
{
    uint16 j;
    uint16 size = sizeof(ffBuf);

    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));

    /* happy path */
    TEST_ASSERT_EQUAL(E_OK,
        Dem_GetFreezeFrameDataByDTC(DTC_A, DEM_DTC_ORIGIN_PRIMARY_MEMORY, 0U, ffBuf, &size));
    TEST_ASSERT_EQUAL_UINT16(DEM_FREEZE_FRAME_MAX_SIZE, size);
    for (j = 0U; j < DEM_FREEZE_FRAME_MAX_SIZE; j++)
    {
        TEST_ASSERT_EQUAL_UINT8((uint8)j, ffBuf[j]);
    }

    /* buffer too small */
    size = 8U;
    TEST_ASSERT_EQUAL(E_NOT_OK,
        Dem_GetFreezeFrameDataByDTC(DTC_A, DEM_DTC_ORIGIN_PRIMARY_MEMORY, 0U, ffBuf, &size));

    /* NULL pointers report DET */
    TEST_ASSERT_EQUAL(0, mock_DetCalls);
    (void)Dem_GetFreezeFrameDataByDTC(DTC_A, DEM_DTC_ORIGIN_PRIMARY_MEMORY, 0U, NULL_PTR, &size);
    TEST_ASSERT_EQUAL(1, mock_DetCalls);
    TEST_ASSERT_EQUAL(DEM_SID_GETFREEZEFRAMEDATABYDTC, mock_lastApiId);
    TEST_ASSERT_EQUAL(DEM_E_PARAM_POINTER, mock_lastErrorId);

    /* unknown DTC */
    size = sizeof(ffBuf);
    TEST_ASSERT_EQUAL(DEM_GET_FREEZEFRAME_WRONG_DTC,
        Dem_GetFreezeFrameDataByDTC(DTC_UNKNOWN, DEM_DTC_ORIGIN_PRIMARY_MEMORY, 0U, ffBuf, &size));
}

/** Explicit prestore round trip: prestore EVT_B, read pattern, clear prestore */
void test_FreezeFrame_PrestoreAndClearPrestore(void)
{
    uint16 j;
    uint16 size = sizeof(ffBuf);

    /* no failure reported: entry B already exists from config, slot 0 is free */
    TEST_ASSERT_EQUAL(E_OK, Dem_PrestoreFreezeFrame(EVT_B));
    TEST_ASSERT_EQUAL(TRUE, Dem_InternalState.FreezeFrames[0].IsValid);
    TEST_ASSERT_EQUAL_UINT16(1U, Dem_InternalState.FreezeFrames[0].DtcIndex);
    /* sample pattern: Data[j] = j + DtcIndex(1) + slotIndex(0) = j + 1 */
    TEST_ASSERT_EQUAL(E_OK,
        Dem_GetFreezeFrameDataByDTC(DTC_B, DEM_DTC_ORIGIN_PRIMARY_MEMORY, 0U, ffBuf, &size));
    for (j = 0U; j < DEM_FREEZE_FRAME_MAX_SIZE; j++)
    {
        TEST_ASSERT_EQUAL_UINT8((uint8)(j + 1U), ffBuf[j]);
    }

    /* clear prestore removes it */
    TEST_ASSERT_EQUAL(E_OK, Dem_ClearPrestoredFreezeFrame(EVT_B));
    TEST_ASSERT_EQUAL(FALSE, Dem_InternalState.FreezeFrames[0].IsValid);
    size = sizeof(ffBuf);
    TEST_ASSERT_EQUAL(E_NOT_OK,
        Dem_GetFreezeFrameDataByDTC(DTC_B, DEM_DTC_ORIGIN_PRIMARY_MEMORY, 0U, ffBuf, &size));
}

/** Prestore for an unknown event fails */
void test_FreezeFrame_PrestoreUnknownEvent_Fails(void)
{
    TEST_ASSERT_EQUAL(E_NOT_OK, Dem_PrestoreFreezeFrame(99U));
    TEST_ASSERT_EQUAL(E_NOT_OK, Dem_ClearPrestoredFreezeFrame(99U));
}

/*==================================================================================*
 *  D. Counter-based debounce                                                      *
 *==================================================================================*/

/** PREFAILED increments the counter 1,2,3; DTC status crosses only at threshold */
void test_DebounceCounter_PrefailedProgression_CrossesAtConfiguredThreshold(void)
{
    sint8 fdc = 0;

    TEST_ASSERT_EQUAL(E_OK, report(EVT_B, DEM_EVENT_STATUS_PREFAILED));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_B, &fdc));
    TEST_ASSERT_EQUAL_INT8(1, fdc);
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_B));

    TEST_ASSERT_EQUAL(E_OK, report(EVT_B, DEM_EVENT_STATUS_PREFAILED));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_B, &fdc));
    TEST_ASSERT_EQUAL_INT8(2, fdc);
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_B));

    /* third PREFAILED reaches the configured failed threshold (3) -> TF */
    TEST_ASSERT_EQUAL(E_OK, report(EVT_B, DEM_EVENT_STATUS_PREFAILED));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_B, &fdc));
    TEST_ASSERT_EQUAL_INT8(3, fdc);
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_B));
}

/** PREPASSED decrements back down; status clears only when the passed threshold is hit */
void test_DebounceCounter_PrepassedProgression_ClearsAtConfiguredThreshold(void)
{
    sint8 fdc = 0;

    /* drive to failed first (counter = 3) */
    (void)report(EVT_B, DEM_EVENT_STATUS_PREFAILED);
    (void)report(EVT_B, DEM_EVENT_STATUS_PREFAILED);
    (void)report(EVT_B, DEM_EVENT_STATUS_PREFAILED);
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_B));

    /* 3 -> 2 (no crossing yet) */
    TEST_ASSERT_EQUAL(E_OK, report(EVT_B, DEM_EVENT_STATUS_PREPASSED));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_B, &fdc));
    TEST_ASSERT_EQUAL_INT8(2, fdc);
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_B));

    /* 2 -> 1 -> 0 -> -1 -> -2 -> -3 (crossing on the 6th PREPASSED) */
    (void)report(EVT_B, DEM_EVENT_STATUS_PREPASSED);
    (void)report(EVT_B, DEM_EVENT_STATUS_PREPASSED);
    (void)report(EVT_B, DEM_EVENT_STATUS_PREPASSED);
    (void)report(EVT_B, DEM_EVENT_STATUS_PREPASSED);
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_B));

    TEST_ASSERT_EQUAL(E_OK, report(EVT_B, DEM_EVENT_STATUS_PREPASSED));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_B, &fdc));
    TEST_ASSERT_EQUAL_INT8(-3, fdc);
    TEST_ASSERT_EQUAL_UINT8(ST_PASSED, statusOf(DTC_B));
}

/** Direct FAILED/PASSED reports jump the counter straight to the threshold */
void test_DebounceCounter_DirectReports_JumpToThresholdAndCross(void)
{
    sint8 fdc = 0;

    TEST_ASSERT_EQUAL(E_OK, report(EVT_B, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_B, &fdc));
    TEST_ASSERT_EQUAL_INT8(3, fdc);
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_B));

    TEST_ASSERT_EQUAL(E_OK, report(EVT_B, DEM_EVENT_STATUS_PASSED));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_B, &fdc));
    TEST_ASSERT_EQUAL_INT8(-3, fdc);
    TEST_ASSERT_EQUAL_UINT8(ST_PASSED, statusOf(DTC_B));
}

/*==================================================================================*
 *  E. Time-based debounce                                                         *
 *==================================================================================*/

/** PREFAILED accumulates elapsed time; MainFunction commits the result at threshold */
void test_DebounceTime_Prefailed_ThresholdCommittedByMainFunction(void)
{
    sint8 fdc = 0;
    const uint16 idx = EVT_C - 1U;

    /* public API starts the counting window (elapsed = 0) */
    TEST_ASSERT_EQUAL(E_OK, report(EVT_C, DEM_EVENT_STATUS_PREFAILED));
    TEST_ASSERT_EQUAL_UINT8(DEM_TIME_DEBOUNCE_COUNTING_UP, Dem_InternalState.TimeDebounceStates[idx].State);
    TEST_ASSERT_EQUAL_UINT32(0U, Dem_InternalState.TimeDebounceStates[idx].ElapsedTimeMs);
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_C));

    /* MainFunction with threshold not reached: no status change */
    Dem_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_C));

    /* +60ms: below the 100ms threshold */
    Dem_IntProcessTimeBasedDebounce(EVT_C, DEM_EVENT_STATUS_PREFAILED, 60U);
    TEST_ASSERT_EQUAL_UINT32(60U, Dem_InternalState.TimeDebounceStates[idx].ElapsedTimeMs);
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_C, &fdc));
    TEST_ASSERT_EQUAL_INT8(0, fdc);

    /* +60ms again: 120ms >= 100ms -> FDC saturates, commit still pending */
    Dem_IntProcessTimeBasedDebounce(EVT_C, DEM_EVENT_STATUS_PREFAILED, 60U);
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_C, &fdc));
    TEST_ASSERT_EQUAL_INT8(DEM_DEBOUNCE_COUNTER_FAILED_THRESHOLD, fdc);
    TEST_ASSERT_EQUAL(TRUE, Dem_InternalState.TimeDebounceStates[idx].ThresholdReached);
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, statusOf(DTC_C));

    /* MainFunction commits the debounce result to the DTC status */
    Dem_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_C));
    TEST_ASSERT_EQUAL_UINT8(DEM_TIME_DEBOUNCE_IDLE, Dem_InternalState.TimeDebounceStates[idx].State);
    TEST_ASSERT_EQUAL(FALSE, Dem_InternalState.TimeDebounceStates[idx].ThresholdReached);
}

/** PREPASSED after failed counts down and clears the active bits via MainFunction */
void test_DebounceTime_Prepassed_ClearsActiveBitsViaMainFunction(void)
{
    sint8 fdc = 0;
    const uint16 idx = EVT_C - 1U;

    /* drive to failed via the time path */
    (void)report(EVT_C, DEM_EVENT_STATUS_PREFAILED);
    Dem_IntProcessTimeBasedDebounce(EVT_C, DEM_EVENT_STATUS_PREFAILED, 60U);
    Dem_IntProcessTimeBasedDebounce(EVT_C, DEM_EVENT_STATUS_PREFAILED, 60U);
    Dem_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, statusOf(DTC_C));

    /* pre-passed starts the counting-down window */
    TEST_ASSERT_EQUAL(E_OK, report(EVT_C, DEM_EVENT_STATUS_PREPASSED));
    TEST_ASSERT_EQUAL_UINT8(DEM_TIME_DEBOUNCE_COUNTING_DOWN, Dem_InternalState.TimeDebounceStates[idx].State);

    Dem_IntProcessTimeBasedDebounce(EVT_C, DEM_EVENT_STATUS_PREPASSED, 50U);
    TEST_ASSERT_EQUAL_UINT32(50U, Dem_InternalState.TimeDebounceStates[idx].ElapsedTimeMs);
    Dem_IntProcessTimeBasedDebounce(EVT_C, DEM_EVENT_STATUS_PREPASSED, 60U);
    TEST_ASSERT_EQUAL_UINT32(110U, Dem_InternalState.TimeDebounceStates[idx].ElapsedTimeMs);
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_C, &fdc));
    TEST_ASSERT_EQUAL_INT8(DEM_DEBOUNCE_COUNTER_PASSED_THRESHOLD, fdc);

    /* commit */
    Dem_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(ST_PASSED, statusOf(DTC_C));
    TEST_ASSERT_EQUAL_UINT8(DEM_TIME_DEBOUNCE_IDLE, Dem_InternalState.TimeDebounceStates[idx].State);
}

/** Immediate PASSED report sets the FDC saturation value right away */
void test_DebounceTime_ImmediatePassed_SetsFdcImmediately(void)
{
    sint8 fdc = 0;

    TEST_ASSERT_EQUAL(E_OK, report(EVT_C, DEM_EVENT_STATUS_PASSED));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_C, &fdc));
    TEST_ASSERT_EQUAL_INT8(DEM_DEBOUNCE_COUNTER_PASSED_THRESHOLD, fdc);
    TEST_ASSERT_EQUAL_UINT8(DEM_TIME_DEBOUNCE_IDLE, Dem_InternalState.TimeDebounceStates[EVT_C - 1U].State);
}

/*==================================================================================*
 *  F. Filtered DTC iteration (SelectDTC + GetNextFilteredDTC)                     *
 *==================================================================================*/

/** After SelectDTC the iteration returns all configured, non-deleted DTCs in order */
void test_Filter_SelectAndIterate_ReturnsAllConfiguredDtcs(void)
{
    Dem_DtcType dtc = 0U;
    Dem_UdsStatusByteType status = 0U;
    uint16 count = 0xFFFFU;

    TEST_ASSERT_EQUAL(E_OK, Dem_SelectDTC(DTC_A, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetNumberOfFilteredDTC(&count));
    TEST_ASSERT_EQUAL_UINT16(3U, count);

    TEST_ASSERT_EQUAL(DEM_FILTERED_OK, Dem_GetNextFilteredDTC(&dtc, &status));
    TEST_ASSERT_EQUAL_UINT32(DTC_A, dtc);
    TEST_ASSERT_EQUAL_UINT8(ST_PENDING, status);

    TEST_ASSERT_EQUAL(DEM_FILTERED_OK, Dem_GetNextFilteredDTC(&dtc, &status));
    TEST_ASSERT_EQUAL_UINT32(DTC_B, dtc);

    TEST_ASSERT_EQUAL(DEM_FILTERED_OK, Dem_GetNextFilteredDTC(&dtc, &status));
    TEST_ASSERT_EQUAL_UINT32(DTC_C, dtc);

    TEST_ASSERT_EQUAL(DEM_FILTERED_NO_MATCHING_ELEMENT, Dem_GetNextFilteredDTC(&dtc, &status));
}

/** The iteration reports live status bytes (failed DTC carries TF) */
void test_Filter_Iteration_ReflectsFailedStatus(void)
{
    Dem_DtcType dtc = 0U;
    Dem_UdsStatusByteType status = 0U;

    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL(E_OK, Dem_SelectDTC(DTC_A, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));

    TEST_ASSERT_EQUAL(DEM_FILTERED_OK, Dem_GetNextFilteredDTC(&dtc, &status));
    TEST_ASSERT_EQUAL_UINT32(DTC_A, dtc);
    TEST_ASSERT_EQUAL_UINT8(ST_FAILED, status);
}

/** Cleared DTCs are excluded from the iteration and the filtered count */
void test_Filter_ClearedDtc_ExcludedFromIteration(void)
{
    Dem_DtcType dtc = 0U;
    Dem_UdsStatusByteType status = 0U;
    uint16 count = 0xFFFFU;

    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL(E_OK, Dem_ClearDTC(DTC_A, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));

    TEST_ASSERT_EQUAL(E_OK, Dem_SelectDTC(DTC_A, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetNumberOfFilteredDTC(&count));
    TEST_ASSERT_EQUAL_UINT16(2U, count);

    TEST_ASSERT_EQUAL(DEM_FILTERED_OK, Dem_GetNextFilteredDTC(&dtc, &status));
    TEST_ASSERT_EQUAL_UINT32(DTC_B, dtc);
    TEST_ASSERT_EQUAL(DEM_FILTERED_OK, Dem_GetNextFilteredDTC(&dtc, &status));
    TEST_ASSERT_EQUAL_UINT32(DTC_C, dtc);
    TEST_ASSERT_EQUAL(DEM_FILTERED_NO_MATCHING_ELEMENT, Dem_GetNextFilteredDTC(&dtc, &status));
}

/** Clear-all leaves an empty iteration */
void test_Filter_ClearAll_EmptiesIteration(void)
{
    Dem_DtcType dtc = 0U;
    Dem_UdsStatusByteType status = 0U;
    uint16 count = 0xFFFFU;

    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    TEST_ASSERT_EQUAL(E_OK, Dem_ClearDTC(DEM_DTC_GROUP_ALL, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));

    TEST_ASSERT_EQUAL(E_OK, Dem_SelectDTC(DTC_A, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetNumberOfFilteredDTC(&count));
    TEST_ASSERT_EQUAL_UINT16(0U, count);
    TEST_ASSERT_EQUAL(DEM_FILTERED_NO_MATCHING_ELEMENT, Dem_GetNextFilteredDTC(&dtc, &status));
}

/** SelectDTC with an unknown DTC fails; NULL pointers report DET */
void test_Filter_BadArguments_FailOrReportDet(void)
{
    Dem_DtcType dtc = 0U;
    Dem_UdsStatusByteType status = 0U;

    TEST_ASSERT_EQUAL(E_NOT_OK, Dem_SelectDTC(DTC_UNKNOWN, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY));

    TEST_ASSERT_EQUAL(E_NOT_OK, Dem_GetNextFilteredDTC(NULL_PTR, &status));
    TEST_ASSERT_EQUAL(1, mock_DetCalls);
    TEST_ASSERT_EQUAL(DEM_SID_GETNEXTFILTEREDDTC, mock_lastApiId);
    TEST_ASSERT_EQUAL(DEM_E_PARAM_POINTER, mock_lastErrorId);

    TEST_ASSERT_EQUAL(E_NOT_OK, Dem_GetNextFilteredDTC(&dtc, NULL_PTR));
    TEST_ASSERT_EQUAL(2, mock_DetCalls);
}

/*==================================================================================*
 *  G. DTC aging                                                                   *
 *==================================================================================*/

/** Confirmed DTC without TF ages out after the configured number of MainFunction calls */
void test_Aging_ConfirmedAndPassed_DtcAgesAfterThreshold(void)
{
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));  /* confirmed, FF stored */
    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_PASSED));  /* TF cleared, CDTC kept */
    TEST_ASSERT_EQUAL_UINT8(ST_PASSED, statusOf(DTC_A));

    /* aging threshold = 2 MainFunction calls */
    Dem_MainFunction();
    TEST_ASSERT_EQUAL_UINT32(1U, Dem_InternalState.DTCEntries[0].AgingCounter);
    TEST_ASSERT_EQUAL(DEM_UDS_STATUS_CDTC, statusOf(DTC_A) & DEM_UDS_STATUS_CDTC);

    Dem_MainFunction();
    TEST_ASSERT_EQUAL(TRUE, Dem_InternalState.DTCEntries[0].IsAged);
    TEST_ASSERT_EQUAL_UINT8(ST_AGED, statusOf(DTC_A));
    TEST_ASSERT_EQUAL(0, statusOf(DTC_A) & DEM_UDS_STATUS_CDTC);
    /* aging also drops the freeze frame */
    TEST_ASSERT_EQUAL(FALSE, Dem_InternalState.FreezeFrames[0].IsValid);
}

/** While the test is still failed the DTC does not age */
void test_Aging_NotTriggeredWhileTestFailed(void)
{
    uint8 i;

    TEST_ASSERT_EQUAL(E_OK, report(EVT_A, DEM_EVENT_STATUS_FAILED));
    for (i = 0U; i < 3U; i++)
    {
        Dem_MainFunction();
    }
    TEST_ASSERT_EQUAL_UINT32(0U, Dem_InternalState.DTCEntries[0].AgingCounter);
    TEST_ASSERT_EQUAL(DEM_UDS_STATUS_CDTC, statusOf(DTC_A) & DEM_UDS_STATUS_CDTC);
    TEST_ASSERT_EQUAL(DEM_UDS_STATUS_TF, statusOf(DTC_A) & DEM_UDS_STATUS_TF);
}

/*==================================================================================*
 *  H. Misc API surface                                                            *
 *==================================================================================*/

/** ResetEventStatus clears debounce counter, FDC and time-debounce state */
void test_ResetEventStatus_ClearsDebounceState(void)
{
    sint8 fdc = 0;

    (void)report(EVT_B, DEM_EVENT_STATUS_PREFAILED);
    (void)report(EVT_B, DEM_EVENT_STATUS_PREFAILED);
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_B, &fdc));
    TEST_ASSERT_EQUAL_INT8(2, fdc);

    TEST_ASSERT_EQUAL(E_OK, Dem_ResetEventStatus(EVT_B));
    TEST_ASSERT_EQUAL(E_OK, Dem_GetFaultDetectionCounter(EVT_B, &fdc));
    TEST_ASSERT_EQUAL_INT8(0, fdc);
    TEST_ASSERT_EQUAL_INT16(0, Dem_InternalState.EventStates[EVT_B - 1U].DebounceCounter);
    TEST_ASSERT_EQUAL_UINT8(DEM_TIME_DEBOUNCE_IDLE,
                            Dem_InternalState.TimeDebounceStates[EVT_B - 1U].State);
}

/** Disable/Enable DTC setting toggles the internal gate */
void test_DisableEnableDTCSetting_TogglesFlag(void)
{
    TEST_ASSERT_EQUAL(E_OK, Dem_DisableDTCSetting(DEM_DTC_GROUP_ALL, DEM_DTC_KIND_ALL_DTCS));
    TEST_ASSERT_EQUAL(TRUE, Dem_InternalState.DTCSettingDisabled);
    TEST_ASSERT_EQUAL(E_OK, Dem_EnableDTCSetting(DEM_DTC_GROUP_ALL, DEM_DTC_KIND_ALL_DTCS));
    TEST_ASSERT_EQUAL(FALSE, Dem_InternalState.DTCSettingDisabled);
}

int main(void)
{
    UNITY_BEGIN();

    /* A. status bitmask combinations */
    RUN_TEST(test_StatusMask_AfterInit_PendingBitsOnly);
    RUN_TEST(test_StatusMask_SingleFailed_AllFailBitsSet);
    RUN_TEST(test_StatusMask_PassedAfterFailed_KeepsHistoryBits);
    RUN_TEST(test_StatusMask_IndependentPerDtc);
    RUN_TEST(test_StatusMask_UnconfiguredDtc_ReturnsWrongDtc);
    RUN_TEST(test_StatusMask_OccurrenceCounter_IncrementsPerFailure);
    RUN_TEST(test_StatusMask_AvailabilityMask_Is0xFF);

    /* B. ClearDTC */
    RUN_TEST(test_Clear_SingleDtc_ResetsStatusKeepsOthers);
    RUN_TEST(test_Clear_AllDtcs_ClearsEverything);
    RUN_TEST(test_Clear_FunctionalGroup_ClearsOnlyMatchingDtcs);
    RUN_TEST(test_Clear_UnknownDtcNoGroupMatch_ReportsDet);
    RUN_TEST(test_Clear_RemovesFreezeFrame);

    /* C. freeze frame */
    RUN_TEST(test_FreezeFrame_AutoStoreOnConfirmation);
    RUN_TEST(test_FreezeFrame_PublicRead_RoundTrip);
    RUN_TEST(test_FreezeFrame_PrestoreAndClearPrestore);
    RUN_TEST(test_FreezeFrame_PrestoreUnknownEvent_Fails);

    /* D. counter debounce */
    RUN_TEST(test_DebounceCounter_PrefailedProgression_CrossesAtConfiguredThreshold);
    RUN_TEST(test_DebounceCounter_PrepassedProgression_ClearsAtConfiguredThreshold);
    RUN_TEST(test_DebounceCounter_DirectReports_JumpToThresholdAndCross);

    /* E. time debounce */
    RUN_TEST(test_DebounceTime_Prefailed_ThresholdCommittedByMainFunction);
    RUN_TEST(test_DebounceTime_Prepassed_ClearsActiveBitsViaMainFunction);
    RUN_TEST(test_DebounceTime_ImmediatePassed_SetsFdcImmediately);

    /* F. filter iteration */
    RUN_TEST(test_Filter_SelectAndIterate_ReturnsAllConfiguredDtcs);
    RUN_TEST(test_Filter_Iteration_ReflectsFailedStatus);
    RUN_TEST(test_Filter_ClearedDtc_ExcludedFromIteration);
    RUN_TEST(test_Filter_ClearAll_EmptiesIteration);
    RUN_TEST(test_Filter_BadArguments_FailOrReportDet);

    /* G. aging */
    RUN_TEST(test_Aging_ConfirmedAndPassed_DtcAgesAfterThreshold);
    RUN_TEST(test_Aging_NotTriggeredWhileTestFailed);

    /* H. misc */
    RUN_TEST(test_ResetEventStatus_ClearsDebounceState);
    RUN_TEST(test_DisableEnableDTCSetting_TogglesFlag);

    return UnityEnd();
}
