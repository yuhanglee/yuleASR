/**
 * @file test_bswm.c
 * @brief BswM (BSW Manager) Unit Tests — Substantiated
 * @req SWS_BswM
 *
 * Substantiation: every assertion checks observable behavior of the
 * production BswM implementation (src/bsw/services/bswm/src/BswM.c):
 * return values (E_OK/E_NOT_OK), internal mode state before/after
 * BswM_MainFunction, version info fields and DET mock call arguments.
 */

// @tests src/bsw/services/bswm/src/BswM.c  @tests src/bsw/services/bswm/include/BswM.h
#include "unity.h"
#include "BswM.h"
#include "Com.h"
#include "EcuM.h"

/* Service IDs and error codes from BswM.c */
#define BSWM_SID_INIT               0x00U
#define BSWM_SID_REQUEST_MODE       0x03U
#define BSWM_E_PARAM_POINTER        0x10U
#define BSWM_E_UNINIT               0x20U

static uint8 mock_DetCalls = 0U;
static uint16 mock_DetLastModuleId = 0U;
static uint8 mock_DetLastApiId = 0xFFU;
static uint8 mock_DetLastErrorId = 0xFFU;

Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId) {
    (void)InstanceId;
    mock_DetLastModuleId = ModuleId;
    mock_DetLastApiId = ApiId;
    mock_DetLastErrorId = ErrorId;
    mock_DetCalls++;
    return E_OK;
}

/* Mocks for the module dependencies referenced by the BswM action engine
 * (Com_IpduGroupControl / EcuM shutdown coordination). The production
 * library service_bswm now calls them; the real implementations live in
 * service_com / service_ecum which are NOT linked into this test binary. */
static uint8   mock_ComIpduGroupControlCalls = 0U;
static boolean mock_ComLastInitialize = FALSE;
static Com_IpduGroupVector mock_ComLastVector;

void Com_IpduGroupControl(Com_IpduGroupVector IpduGroupVector, boolean Initialize) {
    uint8 i;
    for (i = 0U; i < (uint8)sizeof(Com_IpduGroupVector); i++) {
        mock_ComLastVector[i] = IpduGroupVector[i];
    }
    mock_ComLastInitialize = Initialize;
    mock_ComIpduGroupControlCalls++;
}

static uint8 mock_EcuMGoSleepCalls = 0U;
static uint8 mock_EcuMGoHaltCalls = 0U;
static uint8 mock_EcuMShutdownCalls = 0U;
static EcuM_ShutdownTargetType mock_EcuMLastTarget = 0xFFU;
static uint8 mock_EcuMLastTargetMode = 0xFFU;

void EcuM_GoSleep(void) { mock_EcuMGoSleepCalls++; }
void EcuM_GoHalt(void)  { mock_EcuMGoHaltCalls++; }
void EcuM_Shutdown(void) { mock_EcuMShutdownCalls++; }

Std_ReturnType EcuM_SelectShutdownTarget(EcuM_ShutdownTargetType target, uint8 mode) {
    mock_EcuMLastTarget = target;
    mock_EcuMLastTargetMode = mode;
    return E_OK;
}

static BswM_ConfigType testConfig;

void setUp(void) {
    mock_DetCalls = 0U;
    mock_DetLastModuleId = 0U;
    mock_DetLastApiId = 0xFFU;
    mock_DetLastErrorId = 0xFFU;
    testConfig.NumModeRequestPorts = 0U;
    testConfig.NumRules = 0U;
    testConfig.NumActionLists = 0U;
    testConfig.ModeRequestPorts = NULL_PTR;
    testConfig.Rules = NULL_PTR;
    testConfig.ActionLists = NULL_PTR;
    /* Force a known UNINIT state before every test (BswM_State is file-static). */
    BswM_DeInit();
}

void tearDown(void) {}

/** @req SWS_BswM_00001 */
void test_BswM_Init_NullPtr_ShouldSelectDefaultConfig(void) {
    /* Pre-compile configuration: NULL selects the BswM_Config object of
     * BswM_Lcfg.c instead of reporting a parameter error. */
    BswM_Init(NULL_PTR);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    TEST_ASSERT_EQUAL(E_OK, BswM_RequestMode(BSWM_ECUM_REQUEST, BSWM_MODE_VALUE_RUN));
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, BswM_GetRequestedMode());
}

/** @req SWS_BswM_00001 */
void test_BswM_Init_NullPtr_DefaultConfigShape_ShouldMatchLcfg(void) {
    /* The default config must expose the arbitration tables of BswM_Lcfg.c. */
    TEST_ASSERT_EQUAL_UINT8(16U, BswM_Config.NumModeRequestPorts);
    TEST_ASSERT_EQUAL_UINT16(6U, BswM_Config.NumExpressions);
    TEST_ASSERT_EQUAL_UINT8(3U, BswM_Config.NumRules);
    TEST_ASSERT_EQUAL_UINT8(4U, BswM_Config.NumActionLists);
    TEST_ASSERT_NOT_NULL(BswM_Config.ModeRequestPorts);
    TEST_ASSERT_NOT_NULL(BswM_Config.Expressions);
    TEST_ASSERT_NOT_NULL(BswM_Config.Rules);
    TEST_ASSERT_NOT_NULL(BswM_Config.ActionLists);
    /* Port bindings of the expanded table (indices 0-2 keep the legacy
     * EcuM/ComM/NM semantics; 3..15 are the notification-fed ports). */
    TEST_ASSERT_EQUAL_UINT8(BSWM_ECUM_REQUEST, BswM_Config.ModeRequestPorts[0]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_COMM_REQUEST, BswM_Config.ModeRequestPorts[1]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_NM_REQUEST, BswM_Config.ModeRequestPorts[2]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_DCM_REQUEST, BswM_Config.ModeRequestPorts[3]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_CANSM_REQUEST, BswM_Config.ModeRequestPorts[4]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_ETHSM_REQUEST, BswM_Config.ModeRequestPorts[5]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_FRSM_REQUEST, BswM_Config.ModeRequestPorts[6]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_LINSM_REQUEST, BswM_Config.ModeRequestPorts[7]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_COMM_PNC_REQUEST, BswM_Config.ModeRequestPorts[8]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_ECUM_REQUESTED, BswM_Config.ModeRequestPorts[9]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_PARTITION_REQUEST, BswM_Config.ModeRequestPorts[10]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_ETHIF_REQUEST, BswM_Config.ModeRequestPorts[11]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_NM_CARWAKEUP_REQUEST, BswM_Config.ModeRequestPorts[12]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_DCM_APPUPDATED_REQUEST, BswM_Config.ModeRequestPorts[13]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_LINSM_SCHEDULE_REQUEST, BswM_Config.ModeRequestPorts[14]);
    TEST_ASSERT_EQUAL_UINT8(BSWM_LINTP_REQUEST, BswM_Config.ModeRequestPorts[15]);
}

/** @req SWS_BswM_00210 */
void test_BswM_DefaultConfig_EcuMShutdown_ShouldEnterShutdown(void) {
    BswM_Init(NULL_PTR);
    BswM_EcuM_CurrentState(ECUM_STATE_SHUTDOWN);
    /* Request latched, rule 2 (EcuM SHUTDOWN) evaluated once MainFunction runs. */
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_SHUTDOWN, BswM_GetCurrentMode());
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_SHUTDOWN, BswM_GetRequestedMode());
}

/** @req SWS_BswM_00210 */
void test_BswM_DefaultConfig_UnmappedEcuMState_ShouldBeIgnored(void) {
    BswM_Init(NULL_PTR);
    /* 0x02 is not a defined ECUM_STATE_* value: no mode request is issued. */
    BswM_EcuM_CurrentState((EcuM_StateType)0x02U);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetRequestedMode());
}

/** @req SWS_BswM_00211 */
void test_BswM_DefaultConfig_ValidatedWakeup_ShouldRequestWakeupMode(void) {
    BswM_Init(NULL_PTR);
    BswM_EcuM_CurrentWakeup(0x01U, ECUM_WKSTATUS_VALIDATED);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_WAKEUP, BswM_GetRequestedMode());
    /* A non-validated wakeup status must not change the requested mode. */
    BswM_Init(NULL_PTR);
    BswM_EcuM_CurrentWakeup(0x01U, ECUM_WKSTATUS_PENDING);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetRequestedMode());
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
}

/** @req SWS_BswM_00001 */
void test_BswM_Init_ValidConfig_ShouldSucceed(void) {
    BswM_Init(&testConfig);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    /* Fresh init resets both mode registers to OFF. */
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetRequestedMode());
}

/** @req SWS_BswM_00002 */
void test_BswM_DeInit_AfterInit_ShouldReturnToUninit(void) {
    BswM_Init(&testConfig);
    BswM_DeInit();
    TEST_ASSERT_EQUAL(E_NOT_OK, BswM_RequestMode(0U, BSWM_MODE_VALUE_RUN));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_REQUEST_MODE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

/** @req SWS_BswM_00010 */
void test_BswM_RequestMode_AfterInit_ShouldSucceed(void) {
    BswM_Init(&testConfig);
    Std_ReturnType ret = BswM_RequestMode(0U, BSWM_MODE_VALUE_RUN);
    TEST_ASSERT_EQUAL(E_OK, ret);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    /* Request is latched but not applied until BswM_MainFunction runs. */
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, BswM_GetRequestedMode());
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

/** @req SWS_BswM_00011 */
void test_BswM_GetCurrentMode_AfterInit_ShouldReturnOff(void) {
    BswM_Init(&testConfig);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
}

/** @req SWS_BswM_00012 */
void test_BswM_GetRequestedMode_AfterRequest_ShouldReturnMode(void) {
    BswM_Init(&testConfig);
    TEST_ASSERT_EQUAL(E_OK, BswM_RequestMode(0U, BSWM_MODE_VALUE_RUN));
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, BswM_GetRequestedMode());
}

/** @req SWS_BswM_00020 */
void test_BswM_MainFunction_AfterRequest_ShouldApplyMode(void) {
    BswM_Init(&testConfig);
    (void)BswM_RequestMode(0U, BSWM_MODE_VALUE_RUN);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, BswM_GetCurrentMode());
    /* Request mask is consumed: a second run must not change the mode. */
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, BswM_GetCurrentMode());
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
}

/** @req SWS_BswM_00030 */
void test_BswM_GetVersionInfo_ValidPtr_ShouldSucceed(void) {
    Std_VersionInfoType info = {0U, 0U, 0U, 0U, 0U};
    BswM_GetVersionInfo(&info);
    TEST_ASSERT_EQUAL_UINT16(BSWM_VENDOR_ID, info.vendorID);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODULE_ID, info.moduleID);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SW_MAJOR_VERSION, info.sw_major_version);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SW_MINOR_VERSION, info.sw_minor_version);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SW_PATCH_VERSION, info.sw_patch_version);
}

/** @req SWS_BswM_00030 */
void test_BswM_GetVersionInfo_NullPtr_ShouldReportDet(void) {
    BswM_GetVersionInfo(NULL_PTR);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_GETVERSIONINFO, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_PARAM_POINTER, mock_DetLastErrorId);
}

void test_BswM_Init_DoubleInit_ShouldNotCrash(void) {
    BswM_Init(&testConfig);
    BswM_Init(&testConfig);
    /* Production code does not guard double init: no DET, state stays INIT. */
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

void test_BswM_DeInit_BeforeInit_ShouldNotCrash(void) {
    /* setUp() already forced UNINIT; DeInit on an uninitialized module is a
     * no-op in production and must not report a development error. */
    BswM_DeInit();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    TEST_ASSERT_EQUAL(E_NOT_OK, BswM_RequestMode(0U, BSWM_MODE_VALUE_RUN));
}

void test_BswM_RequestMode_BeforeInit_ShouldFail(void) {
    Std_ReturnType ret = BswM_RequestMode(0U, BSWM_MODE_VALUE_RUN);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_REQUEST_MODE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

/*==================================================================================================
*                            Notification callbacks — UNINIT DET guards
*================================================================================================*/

/* setUp() leaves the module UNINIT: every notification must report
 * BSWM_E_UNINIT with its own service id and leave the state untouched. */

void test_BswM_ComM_CurrentMode_Uninit_ShouldReportDet(void) {
    BswM_ComM_CurrentMode(0U, 2U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_COMM_CURRENT_MODE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetRequestedMode());
}

void test_BswM_ComM_CurrentPNCMode_Uninit_ShouldReportDet(void) {
    BswM_ComM_CurrentPNCMode(0U, TRUE);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_COMM_CURRENT_PNC_MODE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_Dcm_CommunicationMode_CurrentState_Uninit_ShouldReportDet(void) {
    BswM_Dcm_CommunicationMode_CurrentState(0U, 3U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_DCM_COMM_MODE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_Dcm_ApplicationUpdated_Uninit_ShouldReportDet(void) {
    BswM_Dcm_ApplicationUpdated(0U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_DCM_APP_UPDATED, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_Nm_StateChangeNotification_Uninit_ShouldReportDet(void) {
    BswM_Nm_StateChangeNotification(0U, 4U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_NM_STATE_CHANGE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_Nm_CarWakeUpIndication_Uninit_ShouldReportDet(void) {
    BswM_Nm_CarWakeUpIndication(0U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_NM_CAR_WAKEUP, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_CanSM_CurrentState_Uninit_ShouldReportDet(void) {
    BswM_CanSM_CurrentState(0U, 5U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_CANSM_CURRENT_STATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_EthSM_CurrentState_Uninit_ShouldReportDet(void) {
    BswM_EthSM_CurrentState(0U, 5U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_ETHSM_CURRENT_STATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_FrSM_CurrentState_Uninit_ShouldReportDet(void) {
    BswM_FrSM_CurrentState(0U, 5U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_FRSM_CURRENT_STATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_LinSM_CurrentState_Uninit_ShouldReportDet(void) {
    BswM_LinSM_CurrentState(0U, 5U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_LINSM_CURRENT_STATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_LinSM_CurrentSchedule_Uninit_ShouldReportDet(void) {
    BswM_LinSM_CurrentSchedule(0U, 7U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_LINSM_CURRENT_SCHEDULE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_LinTp_RequestMode_Uninit_ShouldReportDet(void) {
    BswM_LinTp_RequestMode(0U, 2U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_LINTP_REQUEST_MODE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_EcuM_RequestedState_Uninit_ShouldReportDet(void) {
    BswM_EcuM_RequestedState(ECUM_STATE_RUN);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_ECUM_REQUESTED_STATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_BswMPartitionRestarted_Uninit_ShouldReportDet(void) {
    BswM_BswMPartitionRestarted(0x1234U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_PARTITION_RESTARTED, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_EthIf_PortGroupLinkStateChg_Uninit_ShouldReportDet(void) {
    BswM_EthIf_PortGroupLinkStateChg(0U, TRUE);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_ETHIF_PORTGROUP_LINKSTATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

/*==================================================================================================
*                   Notification callbacks — default config regression
*================================================================================================**/

void test_BswM_DefaultConfig_NmStateChangeNotification_ShouldDriveSleepRule(void) {
    /* Raw Nm state 4 (BSWM_MODE_VALUE_SLEEP) written to port 2 must trigger
     * the SleepEntry rule exactly like the legacy BswM_RequestMode path. */
    BswM_Init(NULL_PTR);
    BswM_Nm_StateChangeNotification(0U, 4U);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    /* Notifications broadcast facts: they do not latch the requested mode. */
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetRequestedMode());
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_SLEEP, BswM_GetCurrentMode());

    /* Any other raw state clears the rule again (TRUE -> FALSE action runs
     * BswM_ActionSwitchMode(RUN), which also updates the requested mode —
     * same semantics as the legacy EcuMShutdown regression test). */
    BswM_Nm_StateChangeNotification(0U, 3U);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, BswM_GetCurrentMode());
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, BswM_GetRequestedMode());
}

void test_BswM_DefaultConfig_ComMCurrentMode_ShouldNotLatchRequest(void) {
    /* Port 1 (ComM RUN) alone cannot satisfy the RunEntry rule which also
     * requires the EcuM port; the notification must not latch anything. */
    BswM_Init(NULL_PTR);
    BswM_ComM_CurrentMode(0U, BSWM_MODE_VALUE_RUN);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetRequestedMode());
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

void test_BswM_DefaultConfig_EcuMRequestedState_ShouldNotAffectLegacyPorts(void) {
    /* The mapped state lands on port 9 (ECUM_REQUESTED), which no default
     * expression references: the legacy rules must stay untouched. */
    BswM_Init(NULL_PTR);
    BswM_EcuM_RequestedState(ECUM_STATE_SHUTDOWN);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetRequestedMode());
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

void test_BswM_DefaultConfig_UnboundNotification_ShouldSilentlyDrop(void) {
    /* Ports 4 (CANSM), 11 (ETHIF) and 10 (PARTITION) are bound in the default
     * table, but no default expression references them: the notifications are
     * accepted silently and no rule fires. */
    BswM_Init(NULL_PTR);
    BswM_CanSM_CurrentState(0U, 5U);
    BswM_EthIf_PortGroupLinkStateChg(0U, TRUE);
    BswM_BswMPartitionRestarted(0x34U);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCalls);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_BswM_Init_NullPtr_ShouldSelectDefaultConfig);
    RUN_TEST(test_BswM_Init_NullPtr_DefaultConfigShape_ShouldMatchLcfg);
    RUN_TEST(test_BswM_DefaultConfig_EcuMShutdown_ShouldEnterShutdown);
    RUN_TEST(test_BswM_DefaultConfig_UnmappedEcuMState_ShouldBeIgnored);
    RUN_TEST(test_BswM_DefaultConfig_ValidatedWakeup_ShouldRequestWakeupMode);
    RUN_TEST(test_BswM_Init_ValidConfig_ShouldSucceed);
    RUN_TEST(test_BswM_DeInit_BeforeInit_ShouldNotCrash);
    RUN_TEST(test_BswM_DeInit_AfterInit_ShouldReturnToUninit);
    RUN_TEST(test_BswM_RequestMode_BeforeInit_ShouldFail);
    RUN_TEST(test_BswM_Init_DoubleInit_ShouldNotCrash);
    RUN_TEST(test_BswM_RequestMode_AfterInit_ShouldSucceed);
    RUN_TEST(test_BswM_GetCurrentMode_AfterInit_ShouldReturnOff);
    RUN_TEST(test_BswM_GetRequestedMode_AfterRequest_ShouldReturnMode);
    RUN_TEST(test_BswM_MainFunction_AfterRequest_ShouldApplyMode);
    RUN_TEST(test_BswM_GetVersionInfo_ValidPtr_ShouldSucceed);
    RUN_TEST(test_BswM_GetVersionInfo_NullPtr_ShouldReportDet);
    RUN_TEST(test_BswM_ComM_CurrentMode_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_ComM_CurrentPNCMode_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_Dcm_CommunicationMode_CurrentState_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_Dcm_ApplicationUpdated_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_Nm_StateChangeNotification_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_Nm_CarWakeUpIndication_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_CanSM_CurrentState_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_EthSM_CurrentState_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_FrSM_CurrentState_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_LinSM_CurrentState_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_LinSM_CurrentSchedule_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_LinTp_RequestMode_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_EcuM_RequestedState_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_BswMPartitionRestarted_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_EthIf_PortGroupLinkStateChg_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_DefaultConfig_NmStateChangeNotification_ShouldDriveSleepRule);
    RUN_TEST(test_BswM_DefaultConfig_ComMCurrentMode_ShouldNotLatchRequest);
    RUN_TEST(test_BswM_DefaultConfig_EcuMRequestedState_ShouldNotAffectLegacyPorts);
    RUN_TEST(test_BswM_DefaultConfig_UnboundNotification_ShouldSilentlyDrop);

    return UnityEnd();
}
