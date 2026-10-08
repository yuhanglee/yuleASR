/**
 * @file test_cansm.c
 * @brief CanSM (CAN State Manager) Unit Tests
 * @req SWS_CanSM
 *
 * Scope note: the production sources (src/bsw/services/cansm/src/CanSm.c)
 * implement the module lifecycle (CanSM_Init / CanSM_DeInit), the controller
 * mode indication path (CanSM_ControllerModeIndication), the internal state
 * observation hook (CanSM_GetCurrentInternalState), the partial networking
 * services (CanSM_ConfirmPnAvailability / CanSM_ClearTrcvWufFlagIndication),
 * CanSm_GetVersionInfo (guarded by CANSM_VERSION_INFO_API == STD_ON), the
 * mode request path (CanSM_RequestComMode) with the MainFunction-driven BSM
 * (CanSM_MainFunction, incl. CANSM_BSM_S_SILENTCOM_BOR processing), the
 * BusOff callback (CanSM_ControllerBusOff), the wakeup source services
 * (CanSM_StartWakeupSource / CanSM_StopWakeupSource with the
 * CANSM_BSM_S_CHECKWAKEUP validation path), the passive mode services
 * (CanSM_SetEcuPassive / CanSM_SetNetworkPassive) and the transceiver
 * indication callbacks (CanSM_TransceiverModeIndication /
 * CanSM_CheckTransceiverWakeFlagIndication). The SUT reports development
 * errors via Det_ReportError (CANSM_DEV_ERROR_DETECT == STD_ON); a local
 * mock captures the report arguments. The CanIf stubs (stubs.c) return E_OK
 * for all controller/PDU/transceiver mode requests and record the last
 * request arguments, which lets the mode indication path reach FULLCOM.
 */

// @tests src/bsw/services/cansm/src/CanSm.c  @tests src/bsw/services/cansm/include/CanSm.h
#include "unity.h"
#include "CanSm.h"

/* SUT header/source name mismatch: CanSm.h declares CanSM_GetVersionInfo
 * (capital SM) but CanSm.c defines CanSm_GetVersionInfo. The implemented
 * symbol is what we test; production sources must not be modified. */
extern void CanSm_GetVersionInfo(Std_VersionInfoType* versioninfo);

/* CanIf stub interaction records (stubs.c) */
extern uint8 CanIfStub_SetControllerMode_CallCount;
extern uint8 CanIfStub_SetControllerMode_LastControllerId;
extern CanIf_ControllerModeType CanIfStub_SetControllerMode_LastMode;
extern uint8 CanIfStub_SetPduMode_CallCount;
extern uint8 CanIfStub_SetPduMode_LastControllerId;
extern CanIf_PduModeType CanIfStub_SetPduMode_LastMode;
extern uint8 CanIfStub_SetTrcvMode_CallCount;
extern uint8 CanIfStub_SetTrcvMode_LastTransceiverId;
extern CanIf_TransceiverModeType CanIfStub_SetTrcvMode_LastMode;
extern void CanSM_TestStubs_Reset(void);

/* Mock Det_ReportError — captures report arguments (CANSM_DEV_ERROR_DETECT = STD_ON) */
static uint16 mock_DetLastModuleId = 0xFFFFU;
static uint8 mock_DetLastInstanceId = 0xFFU;
static uint8 mock_DetLastApiId = 0xFFU;
static uint8 mock_DetLastErrorId = 0xFFU;
static uint8 mock_DetCallCount = 0U;

static void mock_Det_Reset(void) {
    mock_DetLastModuleId = 0xFFFFU;
    mock_DetLastInstanceId = 0xFFU;
    mock_DetLastApiId = 0xFFU;
    mock_DetLastErrorId = 0xFFU;
    mock_DetCallCount = 0U;
}

Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId) {
    mock_DetLastModuleId = ModuleId;
    mock_DetLastInstanceId = InstanceId;
    mock_DetLastApiId = ApiId;
    mock_DetLastErrorId = ErrorId;
    mock_DetCallCount++;
    return E_OK;
}

void setUp(void) {
    mock_Det_Reset();
    CanSM_TestStubs_Reset();
}

void tearDown(void) {
}

/* Drives network 0 to CANSM_BSM_S_FULLCOM through the implemented path:
 * CanSM_Init(NULL_PTR) puts the network in CANSM_BSM_S_NOCOM, then a CanIf
 * controller mode indication with CANIF_CS_STARTED is processed by
 * CanSm_HandleModeConfirmation -> CanSm_TransitionToFullCom (the CanIf
 * stubs return E_OK for every mode request). */
static void reach_fullcom(void) {
    CanSM_Init(NULL_PTR);
    CanSM_ControllerModeIndication((uint8)CANSM_CONTROLLER_CAN0, CANIF_CS_STARTED);
}

/** @req SWS_CanSM_00003 */
void test_CanSm_GetVersionInfo_ValidPtr_FillsVendorId(void) {
    Std_VersionInfoType info = {0U, 0U, 0U, 0U, 0U};
    CanSm_GetVersionInfo(&info);
    TEST_ASSERT_EQUAL_UINT16(CANSM_VENDOR_ID, info.vendorID);
}

/** @req SWS_CanSM_00003 */
void test_CanSm_GetVersionInfo_ValidPtr_FillsModuleId(void) {
    Std_VersionInfoType info = {0U, 0U, 0U, 0U, 0U};
    CanSm_GetVersionInfo(&info);
    TEST_ASSERT_EQUAL_UINT16(CANSM_MODULE_ID, info.moduleID);
}

/** @req SWS_CanSM_00003 */
void test_CanSm_GetVersionInfo_ValidPtr_FillsSwVersion(void) {
    Std_VersionInfoType info = {0U, 0U, 0U, 0U, 0U};
    CanSm_GetVersionInfo(&info);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SW_MAJOR_VERSION, info.sw_major_version);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SW_MINOR_VERSION, info.sw_minor_version);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SW_PATCH_VERSION, info.sw_patch_version);
}

/** @req SWS_CanSM_00003 */
void test_CanSm_GetVersionInfo_ValidPtr_DoesNotReportDet(void) {
    Std_VersionInfoType info;
    CanSm_GetVersionInfo(&info);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_CanSM_00003 */
void test_CanSm_GetVersionInfo_NullPtr_ReportsDetWithExactIds(void) {
    CanSm_GetVersionInfo(NULL_PTR);
    /* SUT hardcodes ApiId 0x02 in the NULL-pointer guard (not CANSM_SID_GETVERSIONINFO) */
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT16(CANSM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_INSTANCE_ID, mock_DetLastInstanceId);
    TEST_ASSERT_EQUAL_UINT8(0x02U, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_POINTER, mock_DetLastErrorId);
}

/** @req SWS_CanSM_00003 */
void test_CanSm_GetVersionInfo_NullThenValid_RecoversCleanly(void) {
    CanSm_GetVersionInfo(NULL_PTR);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    Std_VersionInfoType info = {0U, 0U, 0U, 0U, 0U};
    CanSm_GetVersionInfo(&info);
    /* Valid call after the NULL report: no new report, version still filled */
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT16(CANSM_VENDOR_ID, info.vendorID);
}

/** @req SWS_CanSM_00003 */
void test_CanSm_GetVersionInfo_RepeatedCalls_KeepSilentDet(void) {
    Std_VersionInfoType info;
    CanSm_GetVersionInfo(&info);
    CanSm_GetVersionInfo(&info);
    CanSm_GetVersionInfo(&info);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_CanSM_00001 */
void test_CanSm_Init_NullPtr_EntersNoComWithoutDet(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_NOCOM, state);
}

/** @req SWS_CanSM_00001 */
void test_CanSm_Init_AfterDeInit_EntersNoComAgain(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);
    CanSM_DeInit();
    mock_Det_Reset();

    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_NOCOM, state);
}

/** @req SWS_CanSM_00002 */
void test_CanSm_DeInit_ReportsNetworkNotInitialized(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOCOM;

    CanSM_Init(NULL_PTR);
    mock_Det_Reset();

    CanSM_DeInit();

    /* DeInit leaves no observable state behind except the uninitialized flag */
    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_GETCURRENTINTERNALSTATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM_00012 */
void test_CanSm_GetCurrentInternalState_BeforeInit_ReportsNotInitialized(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOCOM;

    CanSM_DeInit();

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_GETCURRENTINTERNALSTATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM_00012 */
void test_CanSm_GetCurrentInternalState_NullPtr_ReportsParamPointer(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, NULL_PTR));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_GETCURRENTINTERNALSTATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_POINTER, mock_DetLastErrorId);
}

/** @req SWS_CanSM_00012 */
void test_CanSm_GetCurrentInternalState_InvalidNetwork_ReportsParamNetwork(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOCOM;

    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NUM_NETWORKS, &state));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_GETCURRENTINTERNALSTATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_NETWORK, mock_DetLastErrorId);
}

/** @req SWS_CanSM_00007 */
void test_CanSm_ControllerModeIndication_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    CanSM_ControllerModeIndication((uint8)CANSM_CONTROLLER_CAN0, CANIF_CS_STARTED);

    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_CONTROLLERMODEINDICATION, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM_00007 */
void test_CanSm_ControllerModeIndication_UnknownController_ReportsParamController(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);

    CanSM_ControllerModeIndication((uint8)0x42U, CANIF_CS_STARTED);

    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_CONTROLLERMODEINDICATION, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_CONTROLLER, mock_DetLastErrorId);
    /* No matching controller: the network state must stay untouched */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_NOCOM, state);
}

/** @req SWS_CanSM_00007 */
void test_CanSm_ControllerModeIndication_StartedFromNoCom_ReachesFullCom(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    reach_fullcom();

    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_FULLCOM, state);
}

/** @req SWS_CanSM_00007 */
void test_CanSm_ControllerModeIndication_StoppedFromFullCom_ReachesSilentCom(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    reach_fullcom();

    CanSM_ControllerModeIndication((uint8)CANSM_CONTROLLER_CAN0, CANIF_CS_STOPPED);

    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_SILENTCOM, state);
}

/** @req SWS_CanSM_00007 */
void test_CanSm_ControllerModeIndication_StoppedThenStarted_RoundTripToFullCom(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    reach_fullcom();
    CanSM_ControllerModeIndication((uint8)CANSM_CONTROLLER_CAN0, CANIF_CS_STOPPED);
    CanSM_ControllerModeIndication((uint8)CANSM_CONTROLLER_CAN0, CANIF_CS_STARTED);

    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_FULLCOM, state);
}

/** @req SWS_CanSM */
void test_CanSm_ConfirmPnAvailability_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_ConfirmPnAvailability((NetworkHandleType)CANSM_NETWORK_CAN0));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT16(CANSM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_INSTANCE_ID, mock_DetLastInstanceId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_CONFIRMPNAVAILABILITY, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_ConfirmPnAvailability_InvalidNetwork_ReportsParamNetwork(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_ConfirmPnAvailability((NetworkHandleType)CANSM_NUM_NETWORKS));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_CONFIRMPNAVAILABILITY, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_NETWORK, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_ConfirmPnAvailability_NoCom_ReportsInvalidNetworkMode(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_ConfirmPnAvailability((NetworkHandleType)CANSM_NETWORK_CAN0));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_CONFIRMPNAVAILABILITY, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_INVALID_NETWORK_MODE, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_ConfirmPnAvailability_FullCom_ReturnsOkWithoutDet(void) {
    reach_fullcom();
    mock_Det_Reset();

    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_ConfirmPnAvailability((NetworkHandleType)CANSM_NETWORK_CAN0));
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/* --- Phase 4: PN sleep-availability (PNSA) state machine --- */

/** @req SWS_CanSM */
void test_CanSm_SetPnRequest_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_SetPnRequest((NetworkHandleType)CANSM_NETWORK_CAN0, TRUE));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT16(CANSM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_INSTANCE_ID, mock_DetLastInstanceId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_SETPNREQUEST, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_SetPnRequest_InvalidNetwork_ReportsParamNetwork(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_SetPnRequest((NetworkHandleType)CANSM_NUM_NETWORKS, TRUE));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_SETPNREQUEST, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_NETWORK, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_GetPnState_BeforeInit_ReportsNotInitialized(void) {
    CanSm_PnStateType pnState = CANSM_PNSA_PN_AVAILABLE;
    CanSM_DeInit();

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_GETPNSTATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_GetPnState_NullPtr_ReportsParamPointer(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, NULL));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_GETPNSTATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_POINTER, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_PnState_DefaultIsNoPnAfterInit(void) {
    CanSm_PnStateType pnState = CANSM_PNSA_PN_AVAILABLE;
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_NO_PN, pnState);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_CanSM */
void test_CanSm_SetPnRequest_ShouldPromoteToRequestedInMainFunction(void) {
    CanSm_PnStateType pnState = CANSM_PNSA_NO_PN;
    reach_fullcom();
    /* Align the ComM request with the reached state: reach_fullcom() bypasses
     * RequestComMode, and MainFunction would otherwise tear FULLCOM down to
     * NOCOM (ProcessFullComState follows RequestedComMMode). */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_RequestComMode((ComM_UserHandleType)CANSM_NETWORK_CAN0, COMM_FULL_COMMUNICATION));

    /* Latched only: no promotion before the MainFunction cycle */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_SetPnRequest((NetworkHandleType)CANSM_NETWORK_CAN0, TRUE));
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_NO_PN, pnState);

    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_PN_REQUESTED, pnState);
}

/** @req SWS_CanSM */
void test_CanSm_ConfirmPnAvailability_ShouldPromoteToAvailable(void) {
    CanSm_PnStateType pnState = CANSM_PNSA_NO_PN;
    reach_fullcom();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_RequestComMode((ComM_UserHandleType)CANSM_NETWORK_CAN0, COMM_FULL_COMMUNICATION));

    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_SetPnRequest((NetworkHandleType)CANSM_NETWORK_CAN0, TRUE));
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_PN_REQUESTED, pnState);

    /* Transceiver confirms PN availability: REQUESTED -> AVAILABLE */
    mock_Det_Reset();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_ConfirmPnAvailability((NetworkHandleType)CANSM_NETWORK_CAN0));
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_PN_AVAILABLE, pnState);

    /* Stable in FULLCOM: further MainFunction cycles keep AVAILABLE */
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_PN_AVAILABLE, pnState);
}

/** @req SWS_CanSM */
void test_CanSm_ConfirmPnAvailability_WithoutRequest_ShouldSetAvailable(void) {
    CanSm_PnStateType pnState = CANSM_PNSA_NO_PN;
    reach_fullcom();

    /* Transceiver-driven confirmation without an explicit PN request */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_ConfirmPnAvailability((NetworkHandleType)CANSM_NETWORK_CAN0));
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_PN_AVAILABLE, pnState);
}

/** @req SWS_CanSM */
void test_CanSm_SetPnRequest_Cancel_ShouldNotDemoteRequestedState(void) {
    CanSm_PnStateType pnState = CANSM_PNSA_NO_PN;
    reach_fullcom();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_RequestComMode((ComM_UserHandleType)CANSM_NETWORK_CAN0, COMM_FULL_COMMUNICATION));

    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_SetPnRequest((NetworkHandleType)CANSM_NETWORK_CAN0, TRUE));
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_PN_REQUESTED, pnState);

    /* Cancel before confirmation: already-reached REQUESTED state is kept */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_SetPnRequest((NetworkHandleType)CANSM_NETWORK_CAN0, FALSE));
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_PN_REQUESTED, pnState);
}

/** @req SWS_CanSM */
void test_CanSm_PnState_ShouldResetOnNoCom(void) {
    CanSm_PnStateType pnState = CANSM_PNSA_NO_PN;
    reach_fullcom();

    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_SetPnRequest((NetworkHandleType)CANSM_NETWORK_CAN0, TRUE));
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_PN_REQUESTED, pnState);

    /* FULLCOM -> NOCOM: the PN sleep availability is released */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_RequestComMode((ComM_UserHandleType)CANSM_NETWORK_CAN0, COMM_NO_COMMUNICATION));
    CanSM_MainFunction(); /* BSM enters NOCOM */
    CanSM_MainFunction(); /* PN state block runs after the NOCOM entry */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetPnState((NetworkHandleType)CANSM_NETWORK_CAN0, &pnState));
    TEST_ASSERT_EQUAL_INT(CANSM_PNSA_NO_PN, pnState);
}

/** @req SWS_CanSM */
void test_CanSm_ClearTrcvWufFlagIndication_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_ClearTrcvWufFlagIndication((NetworkHandleType)CANSM_NETWORK_CAN0));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT16(CANSM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_INSTANCE_ID, mock_DetLastInstanceId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_CLEARTRCVWUFFLAGINDICATION, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_ClearTrcvWufFlagIndication_InvalidNetwork_ReportsParamNetwork(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_ClearTrcvWufFlagIndication((NetworkHandleType)CANSM_NUM_NETWORKS));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_CLEARTRCVWUFFLAGINDICATION, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_NETWORK, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_ClearTrcvWufFlagIndication_NoCom_ReportsInvalidNetworkMode(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_ClearTrcvWufFlagIndication((NetworkHandleType)CANSM_NETWORK_CAN0));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_CLEARTRCVWUFFLAGINDICATION, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_INVALID_NETWORK_MODE, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_ClearTrcvWufFlagIndication_FullCom_ReturnsOkWithoutDet(void) {
    reach_fullcom();
    mock_Det_Reset();

    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_ClearTrcvWufFlagIndication((NetworkHandleType)CANSM_NETWORK_CAN0));
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_CanSM */
void test_CanSm_StartWakeupSource_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_StartWakeupSource((NetworkHandleType)CANSM_NETWORK_CAN0));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_STARTWAKEUPSOURCE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_StartWakeupSource_InvalidNetwork_ReportsParamNetwork(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_StartWakeupSource((NetworkHandleType)CANSM_NUM_NETWORKS));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_STARTWAKEUPSOURCE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_NETWORK, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_StartWakeupSource_MainFunction_ConsumesFlag_EntersCheckWakeup(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;
    uint8 ctrlModeCalls;

    CanSM_Init(NULL_PTR);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_StartWakeupSource((NetworkHandleType)CANSM_NETWORK_CAN0));

    /* Request is latched only: no state change before the MainFunction cycle */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_NOCOM, state);
    TEST_ASSERT_EQUAL_UINT8(0U, CanIfStub_SetControllerMode_CallCount);

    CanSM_MainFunction();

    /* Flag consumed: wakeup validation entered (controller STOPPED +
     * transceiver NORMAL requested through CanIf) */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_CHECKWAKEUP, state);
    TEST_ASSERT_EQUAL_UINT8((uint8)CANSM_CONTROLLER_CAN0, CanIfStub_SetControllerMode_LastControllerId);
    TEST_ASSERT_EQUAL_UINT8(CANIF_CS_STOPPED, CanIfStub_SetControllerMode_LastMode);
    TEST_ASSERT_EQUAL_UINT8(1U, CanIfStub_SetTrcvMode_CallCount);
    TEST_ASSERT_EQUAL_UINT8((uint8)CANSM_TRANSCEIVER_CAN0, CanIfStub_SetTrcvMode_LastTransceiverId);
    TEST_ASSERT_EQUAL_INT(CANIF_TRCV_MODE_NORMAL, CanIfStub_SetTrcvMode_LastMode);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);

    /* The latch is cleared: a further cycle must not re-enter / re-request */
    ctrlModeCalls = CanIfStub_SetControllerMode_CallCount;
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(ctrlModeCalls, CanIfStub_SetControllerMode_CallCount);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_CHECKWAKEUP, state);
}

/** @req SWS_CanSM */
void test_CanSm_StartWakeupSource_ValidationCompletes_ReachesFullCom(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);
    (void)CanSM_StartWakeupSource((NetworkHandleType)CANSM_NETWORK_CAN0);
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_CHECKWAKEUP, state);

    /* Controller STOPPED confirmed, but the transceiver has not indicated
     * NORMAL yet: validation must stay pending */
    CanSM_ControllerModeIndication((uint8)CANSM_CONTROLLER_CAN0, CANIF_CS_STOPPED);
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_CHECKWAKEUP, state);

    /* Transceiver NORMAL indicated, but the wakeup-flag check is pending */
    CanSM_TransceiverModeIndication((uint8)CANSM_TRANSCEIVER_CAN0, CANIF_TRCV_MODE_NORMAL);
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_CHECKWAKEUP, state);

    /* Wakeup-flag check completion clears the pending validation step */
    CanSM_CheckTransceiverWakeFlagIndication((uint8)CANSM_TRANSCEIVER_CAN0);
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_FULLCOM, state);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_CanSM */
void test_CanSm_StopWakeupSource_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_StopWakeupSource((NetworkHandleType)CANSM_NETWORK_CAN0));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_STOPWAKEUPSOURCE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_StopWakeupSource_InvalidNetwork_ReportsParamNetwork(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_StopWakeupSource((NetworkHandleType)CANSM_NUM_NETWORKS));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_STOPWAKEUPSOURCE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_NETWORK, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_StopWakeupSource_BeforeMainFunction_CancelsPendingRequest(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);
    (void)CanSM_StartWakeupSource((NetworkHandleType)CANSM_NETWORK_CAN0);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_StopWakeupSource((NetworkHandleType)CANSM_NETWORK_CAN0));

    CanSM_MainFunction();

    /* The cancelled request was consumed as a no-op: still NOCOM, no
     * controller/transceiver mode request issued */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_NOCOM, state);
    TEST_ASSERT_EQUAL_UINT8(0U, CanIfStub_SetControllerMode_CallCount);
    TEST_ASSERT_EQUAL_UINT8(0U, CanIfStub_SetTrcvMode_CallCount);
}

/** @req SWS_CanSM */
void test_CanSm_StopWakeupSource_FromCheckWakeup_ReturnsToNoCom(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);
    (void)CanSM_StartWakeupSource((NetworkHandleType)CANSM_NETWORK_CAN0);
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_CHECKWAKEUP, state);

    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_StopWakeupSource((NetworkHandleType)CANSM_NETWORK_CAN0));

    /* Back to the regular NOCOM flow (controller sleep requested) */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_NOCOM, state);
    TEST_ASSERT_EQUAL_UINT8(CANIF_CS_SLEEP, CanIfStub_SetControllerMode_LastMode);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_CanSM */
void test_CanSm_RequestComMode_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_RequestComMode((ComM_UserHandleType)CANSM_NETWORK_CAN0, COMM_FULL_COMMUNICATION));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_REQUESTCOMMODE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_RequestComMode_InvalidNetwork_ReportsParamNetwork(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_RequestComMode((ComM_UserHandleType)CANSM_NUM_NETWORKS, COMM_FULL_COMMUNICATION));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_REQUESTCOMMODE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_NETWORK, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_RequestComMode_InvalidMode_ReportsInvalidCommRequest(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_RequestComMode((ComM_UserHandleType)CANSM_NETWORK_CAN0, (ComM_ModeType)0x55));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_REQUESTCOMMODE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_INVALID_COMM_REQUEST, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_SetEcuPassive_True_DegradesFullRequestToSilentCom(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_SetEcuPassive(TRUE));
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_RequestComMode((ComM_UserHandleType)CANSM_NETWORK_CAN0, COMM_FULL_COMMUNICATION));

    CanSM_MainFunction();

    /* FULL request degraded to SILENTCOM on the passive network */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_SILENTCOM, state);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_CanSM */
void test_CanSm_SetEcuPassive_False_FullRequestReachesFullCom(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_SetEcuPassive(FALSE));
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_RequestComMode((ComM_UserHandleType)CANSM_NETWORK_CAN0, COMM_FULL_COMMUNICATION));

    CanSM_MainFunction();
    CanSM_ControllerModeIndication((uint8)CANSM_CONTROLLER_CAN0, CANIF_CS_STARTED);

    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_FULLCOM, state);
}

/** @req SWS_CanSM */
void test_CanSm_SetNetworkPassive_True_ForcesPassiveWhenEcuActive(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_SetEcuPassive(FALSE));
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_SetNetworkPassive((NetworkHandleType)CANSM_NETWORK_CAN0, TRUE));
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_RequestComMode((ComM_UserHandleType)CANSM_NETWORK_CAN0, COMM_FULL_COMMUNICATION));

    CanSM_MainFunction();

    /* Network override (passive) wins over the active ECU mode */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_SILENTCOM, state);
}

/** @req SWS_CanSM */
void test_CanSm_SetNetworkPassive_False_OverridesEcuPassive_ReelevatesToFullCom(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);
    (void)CanSM_SetEcuPassive(TRUE);
    (void)CanSM_RequestComMode((ComM_UserHandleType)CANSM_NETWORK_CAN0, COMM_FULL_COMMUNICATION);
    CanSM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_SILENTCOM, state);

    /* Explicit non-passive network overrides the ECU-wide passive mode:
     * the pending FULL request is served on the next cycle */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_SetNetworkPassive((NetworkHandleType)CANSM_NETWORK_CAN0, FALSE));
    CanSM_MainFunction();

    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_FULLCOM, state);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_CanSM */
void test_CanSm_SetNetworkPassive_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_SetNetworkPassive((NetworkHandleType)CANSM_NETWORK_CAN0, TRUE));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_SETNETWORKPASSIVE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_SetNetworkPassive_InvalidNetwork_ReportsParamNetwork(void) {
    CanSM_Init(NULL_PTR);

    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, CanSM_SetNetworkPassive((NetworkHandleType)CANSM_NUM_NETWORKS, TRUE));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_SETNETWORKPASSIVE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_NETWORK, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_TxTimeoutException_EntersSilentComBor(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;

    CanSM_Init(NULL_PTR);
    CanSM_TxTimeoutException((NetworkHandleType)CANSM_NETWORK_CAN0);

    /* Bus-off recovery entry: SILENTCOM_BOR, controller STOPPED requested */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_SILENTCOM_BOR, state);
    TEST_ASSERT_EQUAL_UINT8((uint8)CANSM_CONTROLLER_CAN0, CanIfStub_SetControllerMode_LastControllerId);
    TEST_ASSERT_EQUAL_UINT8(CANIF_CS_STOPPED, CanIfStub_SetControllerMode_LastMode);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_CanSM */
void test_CanSm_TxTimeoutException_BorRecoveryCompletes_ReachesFullCom(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;
    uint16 cycle;

    CanSM_Init(NULL_PTR);
    CanSM_TxTimeoutException((NetworkHandleType)CANSM_NETWORK_CAN0);

    /* Confirm STOPPED, run the L1/L2 recovery timers (10 + 100 ticks at a
     * 10 ms main function period), then confirm the restart */
    CanSM_ControllerModeIndication((uint8)CANSM_CONTROLLER_CAN0, CANIF_CS_STOPPED);
    for (cycle = 0U; cycle < 115U; cycle++) {
        CanSM_MainFunction();
    }
    TEST_ASSERT_EQUAL_UINT8(CANIF_CS_STARTED, CanIfStub_SetControllerMode_LastMode);

    CanSM_ControllerModeIndication((uint8)CANSM_CONTROLLER_CAN0, CANIF_CS_STARTED);
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_FULLCOM, state);
}

/** @req SWS_CanSM */
void test_CanSm_TxTimeoutException_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    CanSM_TxTimeoutException((NetworkHandleType)CANSM_NETWORK_CAN0);

    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_TXTIMEOUTEXCEPTION, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_TxTimeoutException_InvalidNetwork_ReportsParamNetwork(void) {
    CanSM_Init(NULL_PTR);

    CanSM_TxTimeoutException((NetworkHandleType)CANSM_NUM_NETWORKS);

    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_TXTIMEOUTEXCEPTION, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_PARAM_NETWORK, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_ControllerBusOff_ThresholdReached_EntersSilentComBor(void) {
    CanSm_BsmStateType state = CANSM_BSM_S_NOTINITIALIZED;
    uint8 event;

    CanSM_Init(NULL_PTR);
    for (event = 0U; event < (uint8)CANSM_BUSOFF_THRESHOLD; event++) {
        CanSM_ControllerBusOff((uint8)CANSM_CONTROLLER_CAN0);
    }

    /* Same BOR entry as TxTimeoutException once the threshold is hit */
    TEST_ASSERT_EQUAL_UINT8(E_OK, CanSM_GetCurrentInternalState((uint8)CANSM_NETWORK_CAN0, &state));
    TEST_ASSERT_EQUAL_INT(CANSM_BSM_S_SILENTCOM_BOR, state);
    TEST_ASSERT_EQUAL_UINT8(CANIF_CS_STOPPED, CanIfStub_SetControllerMode_LastMode);
}

/** @req SWS_CanSM */
void test_CanSm_TransceiverModeIndication_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    CanSM_TransceiverModeIndication((uint8)CANSM_TRANSCEIVER_CAN0, CANIF_TRCV_MODE_NORMAL);

    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_TRANSCEIVERMODEINDICATION, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_TransceiverModeIndication_UnknownTransceiver_IgnoredWithoutDet(void) {
    CanSM_Init(NULL_PTR);

    CanSM_TransceiverModeIndication((uint8)0x42U, CANIF_TRCV_MODE_NORMAL);

    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_CanSM */
void test_CanSm_CheckTransceiverWakeFlagIndication_BeforeInit_ReportsNotInitialized(void) {
    CanSM_DeInit();

    CanSM_CheckTransceiverWakeFlagIndication((uint8)CANSM_TRANSCEIVER_CAN0);

    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(CANSM_SID_CHECKTRCVWAKEFLAGINDICATION, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(CANSM_E_NOT_INITIALIZED, mock_DetLastErrorId);
}

/** @req SWS_CanSM */
void test_CanSm_CheckTransceiverWakeFlagIndication_UnknownTransceiver_IgnoredWithoutDet(void) {
    CanSM_Init(NULL_PTR);

    CanSM_CheckTransceiverWakeFlagIndication((uint8)0x42U);

    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_CanSm_GetVersionInfo_ValidPtr_FillsVendorId);
    RUN_TEST(test_CanSm_GetVersionInfo_ValidPtr_FillsModuleId);
    RUN_TEST(test_CanSm_GetVersionInfo_ValidPtr_FillsSwVersion);
    RUN_TEST(test_CanSm_GetVersionInfo_ValidPtr_DoesNotReportDet);
    RUN_TEST(test_CanSm_GetVersionInfo_NullPtr_ReportsDetWithExactIds);
    RUN_TEST(test_CanSm_GetVersionInfo_NullThenValid_RecoversCleanly);
    RUN_TEST(test_CanSm_GetVersionInfo_RepeatedCalls_KeepSilentDet);

    RUN_TEST(test_CanSm_Init_NullPtr_EntersNoComWithoutDet);
    RUN_TEST(test_CanSm_Init_AfterDeInit_EntersNoComAgain);
    RUN_TEST(test_CanSm_DeInit_ReportsNetworkNotInitialized);
    RUN_TEST(test_CanSm_GetCurrentInternalState_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_GetCurrentInternalState_NullPtr_ReportsParamPointer);
    RUN_TEST(test_CanSm_GetCurrentInternalState_InvalidNetwork_ReportsParamNetwork);
    RUN_TEST(test_CanSm_ControllerModeIndication_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_ControllerModeIndication_UnknownController_ReportsParamController);
    RUN_TEST(test_CanSm_ControllerModeIndication_StartedFromNoCom_ReachesFullCom);
    RUN_TEST(test_CanSm_ControllerModeIndication_StoppedFromFullCom_ReachesSilentCom);
    RUN_TEST(test_CanSm_ControllerModeIndication_StoppedThenStarted_RoundTripToFullCom);
    RUN_TEST(test_CanSm_ConfirmPnAvailability_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_ConfirmPnAvailability_InvalidNetwork_ReportsParamNetwork);
    RUN_TEST(test_CanSm_ConfirmPnAvailability_NoCom_ReportsInvalidNetworkMode);
    RUN_TEST(test_CanSm_ConfirmPnAvailability_FullCom_ReturnsOkWithoutDet);
    RUN_TEST(test_CanSm_SetPnRequest_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_SetPnRequest_InvalidNetwork_ReportsParamNetwork);
    RUN_TEST(test_CanSm_GetPnState_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_GetPnState_NullPtr_ReportsParamPointer);
    RUN_TEST(test_CanSm_PnState_DefaultIsNoPnAfterInit);
    RUN_TEST(test_CanSm_SetPnRequest_ShouldPromoteToRequestedInMainFunction);
    RUN_TEST(test_CanSm_ConfirmPnAvailability_ShouldPromoteToAvailable);
    RUN_TEST(test_CanSm_ConfirmPnAvailability_WithoutRequest_ShouldSetAvailable);
    RUN_TEST(test_CanSm_SetPnRequest_Cancel_ShouldNotDemoteRequestedState);
    RUN_TEST(test_CanSm_PnState_ShouldResetOnNoCom);
    RUN_TEST(test_CanSm_ClearTrcvWufFlagIndication_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_ClearTrcvWufFlagIndication_InvalidNetwork_ReportsParamNetwork);
    RUN_TEST(test_CanSm_ClearTrcvWufFlagIndication_NoCom_ReportsInvalidNetworkMode);
    RUN_TEST(test_CanSm_ClearTrcvWufFlagIndication_FullCom_ReturnsOkWithoutDet);

    RUN_TEST(test_CanSm_StartWakeupSource_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_StartWakeupSource_InvalidNetwork_ReportsParamNetwork);
    RUN_TEST(test_CanSm_StartWakeupSource_MainFunction_ConsumesFlag_EntersCheckWakeup);
    RUN_TEST(test_CanSm_StartWakeupSource_ValidationCompletes_ReachesFullCom);
    RUN_TEST(test_CanSm_StopWakeupSource_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_StopWakeupSource_InvalidNetwork_ReportsParamNetwork);
    RUN_TEST(test_CanSm_StopWakeupSource_BeforeMainFunction_CancelsPendingRequest);
    RUN_TEST(test_CanSm_StopWakeupSource_FromCheckWakeup_ReturnsToNoCom);

    RUN_TEST(test_CanSm_RequestComMode_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_RequestComMode_InvalidNetwork_ReportsParamNetwork);
    RUN_TEST(test_CanSm_RequestComMode_InvalidMode_ReportsInvalidCommRequest);

    RUN_TEST(test_CanSm_SetEcuPassive_True_DegradesFullRequestToSilentCom);
    RUN_TEST(test_CanSm_SetEcuPassive_False_FullRequestReachesFullCom);
    RUN_TEST(test_CanSm_SetNetworkPassive_True_ForcesPassiveWhenEcuActive);
    RUN_TEST(test_CanSm_SetNetworkPassive_False_OverridesEcuPassive_ReelevatesToFullCom);
    RUN_TEST(test_CanSm_SetNetworkPassive_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_SetNetworkPassive_InvalidNetwork_ReportsParamNetwork);

    RUN_TEST(test_CanSm_TxTimeoutException_EntersSilentComBor);
    RUN_TEST(test_CanSm_TxTimeoutException_BorRecoveryCompletes_ReachesFullCom);
    RUN_TEST(test_CanSm_TxTimeoutException_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_TxTimeoutException_InvalidNetwork_ReportsParamNetwork);
    RUN_TEST(test_CanSm_ControllerBusOff_ThresholdReached_EntersSilentComBor);

    RUN_TEST(test_CanSm_TransceiverModeIndication_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_TransceiverModeIndication_UnknownTransceiver_IgnoredWithoutDet);
    RUN_TEST(test_CanSm_CheckTransceiverWakeFlagIndication_BeforeInit_ReportsNotInitialized);
    RUN_TEST(test_CanSm_CheckTransceiverWakeFlagIndication_UnknownTransceiver_IgnoredWithoutDet);

    return UnityEnd();
}
