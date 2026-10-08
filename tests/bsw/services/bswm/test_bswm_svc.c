/**
 * @file test_bswm_svc.c
 * @brief BswM Unit Tests — Substantiated (service behavior focus)
 * @version 1.0.0
 * @date 2026-08-25
 *
 * Substantiation: rewritten against the real BswM API (BswM.h). All
 * assertions verify return values, latched vs. applied mode state and
 * exact DET mock arguments (module/service/error IDs).
 */

// @tests src/bsw/services/bswm/src/BswM.c  @tests src/bsw/services/bswm/include/BswM.h

#include "unity.h"
#include "BswM.h"
#include "Com.h"
#include "EcuM.h"

/* Mock Det_ReportError — records full argument set for verification */
static uint8 mock_DetLastApiId = 0xFFU;
static uint8 mock_DetLastErrorId = 0xFFU;
static uint16 mock_DetLastModuleId = 0U;
static uint8 mock_DetCallCount = 0U;

static void mock_Det_Reset(void) {
    mock_DetLastApiId = 0xFFU;
    mock_DetLastErrorId = 0xFFU;
    mock_DetLastModuleId = 0U;
    mock_DetCallCount = 0U;
}

Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId) {
    (void)InstanceId;
    mock_DetLastModuleId = ModuleId;
    mock_DetLastApiId = ApiId;
    mock_DetLastErrorId = ErrorId;
    mock_DetCallCount++;
    return E_OK;
}

/* Mocks for the module dependencies referenced by the BswM action engine
 * (Com_IpduGroupControl / EcuM shutdown coordination). service_com /
 * service_ecum are not linked into this test binary. */
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

/* Test config */
static BswM_ConfigType testConfig;
static void test_BswM_SetupDefaultConfig(void) {
    testConfig.NumModeRequestPorts = 0U;
    testConfig.NumRules = 0U;
    testConfig.NumActionLists = 0U;
    testConfig.ModeRequestPorts = NULL_PTR;
    testConfig.Rules = NULL_PTR;
    testConfig.ActionLists = NULL_PTR;
}

static void mock_ActionReset(void);

void setUp(void) {
    /* Force a known UNINIT state before every test (BswM_State is file-static).
     * DeInit runs the configured reverse-order cleanup and may fire action
     * lists of rules the previous test left latched TRUE, so the mock
     * counters are cleared AFTERWARDS to keep every test at a zero baseline. */
    BswM_DeInit();
    mock_Det_Reset();
    mock_ActionReset();
    test_BswM_SetupDefaultConfig();
}

void tearDown(void) {
}

/** @req SWS_BswM_00001 */
void test_BswM_Init_NullPtr_ShouldSelectDefaultConfig(void) {
    /* Pre-compile configuration: NULL activates the BswM_Config object
     * generated in BswM_Lcfg.c — no development error is reported. */
    BswM_Init(NULL_PTR);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL(E_OK, BswM_RequestMode(BSWM_ECUM_REQUEST, BSWM_MODE_VALUE_RUN));
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, BswM_GetRequestedMode());
    /* Default arbitration tables (see BswM_Lcfg.c). */
    TEST_ASSERT_EQUAL_UINT8(16U, BswM_Config.NumModeRequestPorts);
    TEST_ASSERT_EQUAL_UINT16(6U, BswM_Config.NumExpressions);
    TEST_ASSERT_EQUAL_UINT8(3U, BswM_Config.NumRules);
    TEST_ASSERT_EQUAL_UINT8(4U, BswM_Config.NumActionLists);
}

/** @req SWS_BswM_00001 */
void test_BswM_Init_ValidConfig_ShouldSucceed(void) {
    BswM_Init(&testConfig);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetRequestedMode());
}

/** @req SWS_BswM_00001 */
void test_BswM_Init_DoubleInit_ShouldResetRequestedMode(void) {
    BswM_Init(&testConfig);
    (void)BswM_RequestMode(0U, BSWM_MODE_VALUE_RUN);
    /* Production Init is not guarded: re-init silently resets the state. */
    BswM_Init(&testConfig);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetRequestedMode());
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

/** @req SWS_BswM_00002 */
void test_BswM_DeInit_Uninit_ShouldNotReportDet(void) {
    /* Production BswM_DeInit() has no DET guard — it silently resets the
     * (already uninitialized) state. Assert the actual behavior. */
    BswM_DeInit();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

/** @req SWS_BswM_00002 */
void test_BswM_DeInit_ValidCall_ShouldReturnToUninit(void) {
    BswM_Init(&testConfig);
    BswM_DeInit();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL(E_NOT_OK, BswM_RequestMode(0U, BSWM_MODE_VALUE_RUN));
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(0x03U, mock_DetLastApiId);   /* BSWM_SID_REQUEST_MODE */
    TEST_ASSERT_EQUAL_UINT8(0x20U, mock_DetLastErrorId); /* BSWM_E_UNINIT */
}

/** @req SWS_BswM_00030 */
void test_BswM_GetVersionInfo_NullPtr_ShouldReportError(void) {
    BswM_GetVersionInfo(NULL_PTR);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(0x06U, mock_DetLastApiId);   /* BSWM_SID_GETVERSIONINFO */
    TEST_ASSERT_EQUAL_UINT8(0x10U, mock_DetLastErrorId); /* BSWM_E_PARAM_POINTER */
}

/** @req SWS_BswM_00030 */
void test_BswM_GetVersionInfo_ValidPtr_ShouldSucceed(void) {
    Std_VersionInfoType info = {0U, 0U, 0U, 0U, 0U};
    BswM_GetVersionInfo(&info);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT16(BSWM_VENDOR_ID, info.vendorID);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODULE_ID, info.moduleID);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SW_MAJOR_VERSION, info.sw_major_version);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SW_MINOR_VERSION, info.sw_minor_version);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SW_PATCH_VERSION, info.sw_patch_version);
}

/** @req SWS_BswM_00020 */
void test_BswM_MainFunction_Uninit_ShouldSilentlyReturn(void) {
    /* Production BswM_MainFunction() has no DET guard: it returns early. */
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

/** @req SWS_BswM_00020 */
void test_BswM_MainFunction_ValidCall_ShouldApplyRequestedMode(void) {
    BswM_Init(&testConfig);
    (void)BswM_RequestMode(0U, BSWM_MODE_VALUE_SLEEP);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_SLEEP, BswM_GetCurrentMode());
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_SLEEP, BswM_GetRequestedMode());
}

/** @req SWS_BswM_00010 */
void test_BswM_RequestMode_Uninit_ShouldReportError(void) {
    Std_ReturnType ret = BswM_RequestMode(0U, BSWM_MODE_VALUE_RUN);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(0x03U, mock_DetLastApiId);   /* BSWM_SID_REQUEST_MODE */
    TEST_ASSERT_EQUAL_UINT8(0x20U, mock_DetLastErrorId); /* BSWM_E_UNINIT */
}

/** @req SWS_BswM_00010 */
void test_BswM_RequestMode_ValidCall_ShouldLatchMode(void) {
    BswM_Init(&testConfig);
    Std_ReturnType ret = BswM_RequestMode(0x55U, BSWM_MODE_VALUE_POST_RUN);
    /* SwCompositionId is ignored by production code; any value succeeds. */
    TEST_ASSERT_EQUAL(E_OK, ret);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_POST_RUN, BswM_GetRequestedMode());
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

/** @req SWS_BswM_00011 */
void test_BswM_GetCurrentMode_Uninit_ShouldReturnOffWithoutDet(void) {
    /* Production BswM_GetCurrentMode() never reports DET; the static state
     * is OFF before the first successful init. */
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_BswM_00011 */
void test_BswM_GetCurrentMode_AfterRequestAndMain_ShouldReturnAppliedMode(void) {
    BswM_Init(&testConfig);
    (void)BswM_RequestMode(0U, BSWM_MODE_VALUE_SHUTDOWN);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_SHUTDOWN, BswM_GetCurrentMode());
}

/** @req SWS_BswM_00012 */
void test_BswM_GetRequestedMode_AfterTwoRequests_ShouldReturnLatest(void) {
    BswM_Init(&testConfig);
    (void)BswM_RequestMode(0U, BSWM_MODE_VALUE_RUN);
    (void)BswM_RequestMode(0U, BSWM_MODE_VALUE_WAKEUP);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_WAKEUP, BswM_GetRequestedMode());
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/*==================================================================================================
*                            Rule / ActionList engine tests
*================================================================================================*/

static uint8 mock_ActionCalls = 0U;
static BswM_ModeType mock_ActionLastParam = BSWM_MODE_VALUE_OFF;

static void mock_ActionRecord(BswM_ModeType Mode) {
    mock_ActionCalls++;
    mock_ActionLastParam = Mode;
}

static void mock_ActionReset(void) {
    mock_ActionCalls = 0U;
    mock_ActionLastParam = BSWM_MODE_VALUE_OFF;
}

/* Ports: 0 = EcuM requests, 1 = ComM requests */
static const BswM_ModeRequestPortType enginePorts[] = {
    BSWM_ECUM_REQUEST,
    BSWM_COMM_REQUEST
};

/* 0: port0 == RUN, 1: port1 == RUN, 2: AND(0,1) */
static const BswM_ExpressionConfigType engineAndExpressions[] = {
    { BSWM_EXPR_MODE_EQUALS, 0U, BSWM_MODE_VALUE_RUN, 0U, 0U },
    { BSWM_EXPR_MODE_EQUALS, 1U, BSWM_MODE_VALUE_RUN, 0U, 0U },
    { BSWM_EXPR_LOGICAL_AND, 0U, BSWM_MODE_VALUE_OFF, 0U, 1U }
};

/* 0: port0 == RUN, 1: port1 == SLEEP, 2: OR(0,1), 3: NOT(2) */
static const BswM_ExpressionConfigType engineLogicExpressions[] = {
    { BSWM_EXPR_MODE_EQUALS,   0U, BSWM_MODE_VALUE_RUN,   0U, 0U },
    { BSWM_EXPR_MODE_EQUALS,   1U, BSWM_MODE_VALUE_SLEEP, 0U, 0U },
    { BSWM_EXPR_LOGICAL_OR,    0U, BSWM_MODE_VALUE_OFF,   0U, 1U },
    { BSWM_EXPR_LOGICAL_NOT,   0U, BSWM_MODE_VALUE_OFF,   2U, 0U }
};

/* 0: port0 != RUN */
static const BswM_ExpressionConfigType engineNeqExpressions[] = {
    { BSWM_EXPR_MODE_NOT_EQUALS, 0U, BSWM_MODE_VALUE_RUN, 0U, 0U }
};

static const BswM_ActionType engineRunActions[]   = { { mock_ActionRecord, BSWM_MODE_VALUE_RUN } };
static const BswM_ActionType engineSleepActions[] = { { mock_ActionRecord, BSWM_MODE_VALUE_SLEEP } };

static const BswM_ActionListType engineActionLists[] = {
    { 1U, engineRunActions },
    { 1U, engineSleepActions }
};

static const BswM_RuleType engineRuleAnd[] = {
    { 0U, 2U, 0U, 1U, BSWM_RULE_STATE_FALSE, TRUE }
};

static const BswM_RuleType engineRuleInitialTrue[] = {
    { 0U, 2U, 0U, 1U, BSWM_RULE_STATE_TRUE, TRUE }
};

static const BswM_RuleType engineRuleDisabled[] = {
    { 0U, 2U, 0U, 1U, BSWM_RULE_STATE_FALSE, FALSE }
};

static const BswM_RuleType engineRuleBadIndices[] = {
    { 0U, 99U, BSWM_ACTION_LIST_NONE, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_TRUE, TRUE }
};

static const BswM_RuleType engineRuleLogic[] = {
    { 0U, 3U, 0U, 1U, BSWM_RULE_STATE_FALSE, TRUE }
};

static const BswM_RuleType engineRuleNeq[] = {
    { 0U, 0U, 0U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE }
};

static const BswM_ConfigType engineAndConfig = {
    2U, enginePorts, 3U, engineAndExpressions, 1U, engineRuleAnd, 2U, engineActionLists
};

static const BswM_ConfigType engineInitialTrueConfig = {
    2U, enginePorts, 3U, engineAndExpressions, 1U, engineRuleInitialTrue, 2U, engineActionLists
};

static const BswM_ConfigType engineDisabledConfig = {
    2U, enginePorts, 3U, engineAndExpressions, 1U, engineRuleDisabled, 2U, engineActionLists
};

static const BswM_ConfigType engineBadIndexConfig = {
    2U, enginePorts, 3U, engineAndExpressions, 1U, engineRuleBadIndices, 2U, engineActionLists
};

static const BswM_ConfigType engineLogicConfig = {
    2U, enginePorts, 4U, engineLogicExpressions, 1U, engineRuleLogic, 2U, engineActionLists
};

static const BswM_ConfigType engineNeqConfig = {
    2U, enginePorts, 1U, engineNeqExpressions, 1U, engineRuleNeq, 2U, engineActionLists
};

/** @req SWS_BswM_00020 */
void test_BswM_Engine_AndCondition_ShouldRequireBothPorts(void) {
    BswM_Init(&engineAndConfig);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls); /* both ports unset */

    (void)BswM_RequestMode(BSWM_ECUM_REQUEST, BSWM_MODE_VALUE_RUN);
    BswM_MainFunction();
    /* AND(port0==RUN, port1==RUN) still FALSE: ComM has not requested RUN. */
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls);

    (void)BswM_RequestMode(BSWM_COMM_REQUEST, BSWM_MODE_VALUE_RUN);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, mock_ActionLastParam);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_BswM_00020 */
void test_BswM_Engine_ActionLists_ShouldRunOnTransitionsOnly(void) {
    BswM_Init(&engineAndConfig);
    (void)BswM_RequestMode(BSWM_ECUM_REQUEST, BSWM_MODE_VALUE_RUN);
    (void)BswM_RequestMode(BSWM_COMM_REQUEST, BSWM_MODE_VALUE_RUN);

    BswM_MainFunction(); /* FALSE -> TRUE : true action list */
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, mock_ActionLastParam);

    BswM_MainFunction(); /* unchanged condition: no action */
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);

    /* Clearing ComM's request breaks the AND: TRUE -> FALSE action list. */
    (void)BswM_RequestMode(BSWM_COMM_REQUEST, BSWM_MODE_VALUE_OFF);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(2U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_SLEEP, mock_ActionLastParam);

    BswM_MainFunction(); /* already FALSE: no repeat */
    TEST_ASSERT_EQUAL_UINT8(2U, mock_ActionCalls);
}

/** @req SWS_BswM_00020 */
void test_BswM_Engine_InitialStateTrue_ShouldNotRunActionList(void) {
    BswM_Init(&engineInitialTrueConfig);
    (void)BswM_RequestMode(BSWM_ECUM_REQUEST, BSWM_MODE_VALUE_RUN);
    (void)BswM_RequestMode(BSWM_COMM_REQUEST, BSWM_MODE_VALUE_RUN);

    BswM_MainFunction(); /* condition TRUE and initial state already TRUE */
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls);

    (void)BswM_RequestMode(BSWM_COMM_REQUEST, BSWM_MODE_VALUE_OFF);
    BswM_MainFunction(); /* TRUE -> FALSE */
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_SLEEP, mock_ActionLastParam);
}

/** @req SWS_BswM_00020 */
void test_BswM_Engine_DisabledRule_ShouldNeverRunActionList(void) {
    BswM_Init(&engineDisabledConfig);
    (void)BswM_RequestMode(BSWM_ECUM_REQUEST, BSWM_MODE_VALUE_RUN);
    (void)BswM_RequestMode(BSWM_COMM_REQUEST, BSWM_MODE_VALUE_RUN);

    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls);

    (void)BswM_RequestMode(BSWM_COMM_REQUEST, BSWM_MODE_VALUE_OFF);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls);
}

/** @req SWS_BswM_00020 */
void test_BswM_Engine_OutOfRangeIndices_ShouldBeIgnored(void) {
    /* Condition index 99 is outside the expression table; both action list
     * indices are BSWM_ACTION_LIST_NONE. The rule must be evaluated as FALSE
     * without executing anything and without reporting a development error. */
    BswM_Init(&engineBadIndexConfig);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

/** @req SWS_BswM_00020 */
void test_BswM_Engine_OrNotExpressions_ShouldEvaluate(void) {
    BswM_Init(&engineLogicConfig);
    BswM_MainFunction();
    /* NOT(OR(FALSE, FALSE)) == TRUE -> true action list */
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, mock_ActionLastParam);

    (void)BswM_RequestMode(BSWM_COMM_REQUEST, BSWM_MODE_VALUE_SLEEP);
    BswM_MainFunction();
    /* port1 == SLEEP makes the OR TRUE, so the NOT becomes FALSE */
    TEST_ASSERT_EQUAL_UINT8(2U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_SLEEP, mock_ActionLastParam);
}

/** @req SWS_BswM_00020 */
void test_BswM_Engine_NotEqualsAndNoneActionList_ShouldBeHandled(void) {
    BswM_Init(&engineNeqConfig);
    BswM_MainFunction();
    /* port0 has no request yet (0xFF) which is != RUN -> TRUE */
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, mock_ActionLastParam);

    (void)BswM_RequestMode(BSWM_ECUM_REQUEST, BSWM_MODE_VALUE_RUN);
    BswM_MainFunction();
    /* TRUE -> FALSE transition, but the false action list is NONE. */
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

/** @req SWS_BswM_00020 */
void test_BswM_RequestMode_InvalidMode_ShouldReportParamModeDet(void) {
    BswM_Init(&testConfig);
    Std_ReturnType ret = BswM_RequestMode(0U, 0xFFU);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(0x03U, mock_DetLastApiId);   /* BSWM_SID_REQUEST_MODE */
    TEST_ASSERT_EQUAL_UINT8(0x30U, mock_DetLastErrorId); /* BSWM_E_PARAM_MODE */
}

void test_BswM_ActionSwitchMode_Uninit_ShouldReportDet(void) {
    BswM_ActionSwitchMode(BSWM_MODE_VALUE_RUN);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(0x07U, mock_DetLastApiId);   /* BSWM_SID_SWITCH_MODE */
    TEST_ASSERT_EQUAL_UINT8(0x20U, mock_DetLastErrorId); /* BSWM_E_UNINIT */
}

void test_BswM_ActionSwitchMode_InvalidMode_ShouldReportDet(void) {
    BswM_Init(&testConfig);
    BswM_ActionSwitchMode(0xFFU);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(0x07U, mock_DetLastApiId);   /* BSWM_SID_SWITCH_MODE */
    TEST_ASSERT_EQUAL_UINT8(0x30U, mock_DetLastErrorId); /* BSWM_E_PARAM_MODE */
}

void test_BswM_ActionSwitchMode_ValidMode_ShouldSetBothModes(void) {
    BswM_Init(&testConfig);
    BswM_ActionSwitchMode(BSWM_MODE_VALUE_POST_RUN);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_POST_RUN, BswM_GetCurrentMode());
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_POST_RUN, BswM_GetRequestedMode());
}

/** @req SWS_BswM_00210 */
void test_BswM_EcuM_CurrentState_Uninit_ShouldReportDet(void) {
    BswM_EcuM_CurrentState(ECUM_STATE_RUN);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(0x03U, mock_DetLastApiId);   /* BSWM_SID_REQUEST_MODE */
    TEST_ASSERT_EQUAL_UINT8(0x20U, mock_DetLastErrorId); /* BSWM_E_UNINIT */
}

/** @req SWS_BswM_00210 */
void test_BswM_EcuM_CurrentState_ShouldRequestMappedMode(void) {
    BswM_Init(NULL_PTR);
    BswM_EcuM_CurrentState(ECUM_STATE_RUN);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_RUN, BswM_GetRequestedMode());
}

/*==================================================================================================
*                      Notification callbacks — rule triggering (raw writes)
*==================================================================================================
* notifyConfig binds one port per notification composition. Each expression
* compares its port against the expected raw value; each rule's action list
* records that raw value, so a triggered action both proves the port write
* and identifies the callback under test.
*================================================================================================*/

/* Expected raw values per port (also the action parameters). */
#define NOTIFY_RAW_COMM          2U   /* ComM full communication mode */
#define NOTIFY_RAW_PNC           1U   /* boolean TRUE                  */
#define NOTIFY_RAW_DCM           3U
#define NOTIFY_RAW_NM            4U
#define NOTIFY_RAW_XSM           5U   /* CanSM/EthSM/FrSM/LinSM state  */
#define NOTIFY_RAW_SCHEDULE      7U
#define NOTIFY_RAW_LINTP         2U
#define NOTIFY_RAW_ECU_REQUESTED BSWM_MODE_VALUE_RUN
#define NOTIFY_RAW_PARTITION     0x34U

static const BswM_ModeRequestPortType notifyPorts[] = {
    BSWM_COMM_REQUEST,            /* 0  */
    BSWM_COMM_PNC_REQUEST,        /* 1  */
    BSWM_DCM_REQUEST,             /* 2  */
    BSWM_DCM_APPUPDATED_REQUEST,  /* 3  */
    BSWM_NM_REQUEST,              /* 4  */
    BSWM_NM_CARWAKEUP_REQUEST,    /* 5  */
    BSWM_CANSM_REQUEST,           /* 6  */
    BSWM_ETHSM_REQUEST,           /* 7  */
    BSWM_FRSM_REQUEST,            /* 8  */
    BSWM_LINSM_REQUEST,           /* 9  */
    BSWM_LINSM_SCHEDULE_REQUEST,  /* 10 */
    BSWM_LINTP_REQUEST,           /* 11 */
    BSWM_ECUM_REQUESTED,          /* 12 */
    BSWM_PARTITION_REQUEST,       /* 13 */
    BSWM_ETHIF_REQUEST            /* 14 */
};

static const BswM_ExpressionConfigType notifyExpressions[] = {
    { BSWM_EXPR_MODE_EQUALS,  0U, NOTIFY_RAW_COMM,          0U, 0U },
    { BSWM_EXPR_MODE_EQUALS,  1U, NOTIFY_RAW_PNC,           0U, 0U },
    { BSWM_EXPR_MODE_EQUALS,  2U, NOTIFY_RAW_DCM,           0U, 0U },
    { BSWM_EXPR_MODE_EQUALS,  3U, NOTIFY_RAW_PNC,           0U, 0U },
    { BSWM_EXPR_MODE_EQUALS,  4U, NOTIFY_RAW_NM,            0U, 0U },
    { BSWM_EXPR_MODE_EQUALS,  5U, NOTIFY_RAW_PNC,           0U, 0U },
    { BSWM_EXPR_MODE_EQUALS,  6U, NOTIFY_RAW_XSM,           0U, 0U },
    { BSWM_EXPR_MODE_EQUALS,  7U, NOTIFY_RAW_XSM,           0U, 0U },
    { BSWM_EXPR_MODE_EQUALS,  8U, NOTIFY_RAW_XSM,           0U, 0U },
    { BSWM_EXPR_MODE_EQUALS,  9U, NOTIFY_RAW_XSM,           0U, 0U },
    { BSWM_EXPR_MODE_EQUALS, 10U, NOTIFY_RAW_SCHEDULE,      0U, 0U },
    { BSWM_EXPR_MODE_EQUALS, 11U, NOTIFY_RAW_LINTP,         0U, 0U },
    { BSWM_EXPR_MODE_EQUALS, 12U, NOTIFY_RAW_ECU_REQUESTED, 0U, 0U },
    { BSWM_EXPR_MODE_EQUALS, 13U, NOTIFY_RAW_PARTITION,     0U, 0U },
    { BSWM_EXPR_MODE_EQUALS, 14U, NOTIFY_RAW_PNC,           0U, 0U }
};

static const BswM_RuleType notifyRules[] = {
    { 0U,  0U,  0U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 1U,  1U,  1U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 2U,  2U,  2U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 3U,  3U,  3U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 4U,  4U,  4U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 5U,  5U,  5U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 6U,  6U,  6U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 7U,  7U,  7U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 8U,  8U,  8U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 9U,  9U,  9U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 10U, 10U, 10U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 11U, 11U, 11U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 12U, 12U, 12U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 13U, 13U, 13U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE },
    { 14U, 14U, 14U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE }
};

static const BswM_ActionType notifyAction0[]  = { { mock_ActionRecord, NOTIFY_RAW_COMM } };
static const BswM_ActionType notifyAction1[]  = { { mock_ActionRecord, NOTIFY_RAW_PNC } };
static const BswM_ActionType notifyAction2[]  = { { mock_ActionRecord, NOTIFY_RAW_DCM } };
static const BswM_ActionType notifyAction3[]  = { { mock_ActionRecord, NOTIFY_RAW_PNC } };
static const BswM_ActionType notifyAction4[]  = { { mock_ActionRecord, NOTIFY_RAW_NM } };
static const BswM_ActionType notifyAction5[]  = { { mock_ActionRecord, NOTIFY_RAW_PNC } };
static const BswM_ActionType notifyAction6[]  = { { mock_ActionRecord, NOTIFY_RAW_XSM } };
static const BswM_ActionType notifyAction7[]  = { { mock_ActionRecord, NOTIFY_RAW_XSM } };
static const BswM_ActionType notifyAction8[]  = { { mock_ActionRecord, NOTIFY_RAW_XSM } };
static const BswM_ActionType notifyAction9[]  = { { mock_ActionRecord, NOTIFY_RAW_XSM } };
static const BswM_ActionType notifyAction10[] = { { mock_ActionRecord, NOTIFY_RAW_SCHEDULE } };
static const BswM_ActionType notifyAction11[] = { { mock_ActionRecord, NOTIFY_RAW_LINTP } };
static const BswM_ActionType notifyAction12[] = { { mock_ActionRecord, NOTIFY_RAW_ECU_REQUESTED } };
static const BswM_ActionType notifyAction13[] = { { mock_ActionRecord, NOTIFY_RAW_PARTITION } };
static const BswM_ActionType notifyAction14[] = { { mock_ActionRecord, NOTIFY_RAW_PNC } };

static const BswM_ActionListType notifyActionLists[] = {
    { 1U, notifyAction0 },
    { 1U, notifyAction1 },
    { 1U, notifyAction2 },
    { 1U, notifyAction3 },
    { 1U, notifyAction4 },
    { 1U, notifyAction5 },
    { 1U, notifyAction6 },
    { 1U, notifyAction7 },
    { 1U, notifyAction8 },
    { 1U, notifyAction9 },
    { 1U, notifyAction10 },
    { 1U, notifyAction11 },
    { 1U, notifyAction12 },
    { 1U, notifyAction13 },
    { 1U, notifyAction14 }
};

static const BswM_ConfigType notifyConfig = {
    15U, notifyPorts, 15U, notifyExpressions, 15U, notifyRules, 15U, notifyActionLists
};

void test_BswM_Notify_ComM_CurrentMode_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_ComM_CurrentMode(0x55U, (BswM_ComMModeType)NOTIFY_RAW_COMM);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls); /* evaluated on MainFunction only */
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_COMM, mock_ActionLastParam);
}

void test_BswM_Notify_ComM_CurrentMode_NonMatchingValue_ShouldNotTrigger(void) {
    BswM_Init(&notifyConfig);
    BswM_ComM_CurrentMode(0x55U, 1U);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

void test_BswM_Notify_ComM_CurrentPNCMode_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_ComM_CurrentPNCMode(0U, TRUE);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_PNC, mock_ActionLastParam);
}

void test_BswM_Notify_Dcm_CommunicationMode_CurrentState_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_Dcm_CommunicationMode_CurrentState(0U, (BswM_DcmCommunicationModeType)NOTIFY_RAW_DCM);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_DCM, mock_ActionLastParam);
}

void test_BswM_Notify_Dcm_ApplicationUpdated_ShouldWriteFixedOne(void) {
    BswM_Init(&notifyConfig);
    BswM_Dcm_ApplicationUpdated(0U);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_PNC, mock_ActionLastParam); /* fixed value 1 */
}

void test_BswM_Notify_Nm_StateChangeNotification_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_Nm_StateChangeNotification(0U, (BswM_NmStateType)NOTIFY_RAW_NM);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_NM, mock_ActionLastParam);
}

void test_BswM_Notify_Nm_CarWakeUpIndication_ShouldWriteFixedOne(void) {
    BswM_Init(&notifyConfig);
    BswM_Nm_CarWakeUpIndication(0U);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_PNC, mock_ActionLastParam); /* fixed value 1 */
}

void test_BswM_Notify_CanSM_CurrentState_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_CanSM_CurrentState(0U, (BswM_CanSmStateType)NOTIFY_RAW_XSM);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_XSM, mock_ActionLastParam);
}

void test_BswM_Notify_EthSM_CurrentState_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_EthSM_CurrentState(0U, (BswM_EthSmStateType)NOTIFY_RAW_XSM);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_XSM, mock_ActionLastParam);
}

void test_BswM_Notify_FrSM_CurrentState_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_FrSM_CurrentState(0U, (BswM_FrSmStateType)NOTIFY_RAW_XSM);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_XSM, mock_ActionLastParam);
}

void test_BswM_Notify_LinSM_CurrentState_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_LinSM_CurrentState(0U, (BswM_LinSmStateType)NOTIFY_RAW_XSM);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_XSM, mock_ActionLastParam);
}

void test_BswM_Notify_LinSM_CurrentSchedule_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_LinSM_CurrentSchedule(0U, NOTIFY_RAW_SCHEDULE);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_SCHEDULE, mock_ActionLastParam);
}

void test_BswM_Notify_LinTp_RequestMode_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_LinTp_RequestMode(0U, (BswM_LinTpModeType)NOTIFY_RAW_LINTP);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_LINTP, mock_ActionLastParam);
}

void test_BswM_Notify_EcuM_RequestedState_ShouldMapAndTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_EcuM_RequestedState(ECUM_STATE_RUN);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_ECU_REQUESTED, mock_ActionLastParam);
}

void test_BswM_Notify_EcuM_RequestedState_Unmapped_ShouldBeIgnored(void) {
    BswM_Init(&notifyConfig);
    /* 0x02 is not a defined ECUM_STATE_* value: no port write, no DET. */
    BswM_EcuM_RequestedState((BswM_EcuMStateType)0x02U);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

void test_BswM_Notify_BswMPartitionRestarted_ShouldWritePartitionIdLowByte(void) {
    BswM_Init(&notifyConfig);
    BswM_BswMPartitionRestarted(0x1234U);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_PARTITION, mock_ActionLastParam);
}

void test_BswM_Notify_EthIf_PortGroupLinkStateChg_ShouldTriggerRule(void) {
    BswM_Init(&notifyConfig);
    BswM_EthIf_PortGroupLinkStateChg(2U, TRUE);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(1U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(NOTIFY_RAW_PNC, mock_ActionLastParam);
}

void test_BswM_Notify_EthIf_PortGroupLinkStateChg_LinkDown_ShouldNotTrigger(void) {
    BswM_Init(&notifyConfig);
    BswM_EthIf_PortGroupLinkStateChg(2U, FALSE);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
}

void test_BswM_Notify_UnboundComposition_ShouldSilentlyDrop(void) {
    /* testConfig binds no ports at all: the notification is dropped without
     * a development error and no rule can fire. */
    BswM_Init(&testConfig);
    BswM_CanSM_CurrentState(0U, (BswM_CanSmStateType)NOTIFY_RAW_XSM);
    BswM_Nm_StateChangeNotification(0U, (BswM_NmStateType)NOTIFY_RAW_NM);
    BswM_BswMPartitionRestarted(0x1234U);
    TEST_ASSERT_EQUAL_UINT8(0U, mock_DetCallCount);
    BswM_MainFunction();
    TEST_ASSERT_EQUAL_UINT8(0U, mock_ActionCalls);
    TEST_ASSERT_EQUAL_UINT8(BSWM_MODE_VALUE_OFF, BswM_GetCurrentMode());
}

void test_BswM_Notify_ComM_CurrentMode_Uninit_ShouldReportDet(void) {
    BswM_ComM_CurrentMode(0U, (BswM_ComMModeType)NOTIFY_RAW_COMM);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_COMM_CURRENT_MODE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_Notify_EcuM_RequestedState_Uninit_ShouldReportDet(void) {
    BswM_EcuM_RequestedState(ECUM_STATE_RUN);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_ECUM_REQUESTED_STATE, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

void test_BswM_Notify_BswMPartitionRestarted_Uninit_ShouldReportDet(void) {
    BswM_BswMPartitionRestarted(0x1234U);
    TEST_ASSERT_EQUAL_UINT8(1U, mock_DetCallCount);
    TEST_ASSERT_EQUAL_UINT16(BSWM_MODULE_ID, mock_DetLastModuleId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_SID_PARTITION_RESTARTED, mock_DetLastApiId);
    TEST_ASSERT_EQUAL_UINT8(BSWM_E_UNINIT, mock_DetLastErrorId);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_BswM_Init_NullPtr_ShouldSelectDefaultConfig);
    RUN_TEST(test_BswM_Init_ValidConfig_ShouldSucceed);
    RUN_TEST(test_BswM_Init_DoubleInit_ShouldResetRequestedMode);
    RUN_TEST(test_BswM_DeInit_Uninit_ShouldNotReportDet);
    RUN_TEST(test_BswM_DeInit_ValidCall_ShouldReturnToUninit);
    RUN_TEST(test_BswM_GetVersionInfo_NullPtr_ShouldReportError);
    RUN_TEST(test_BswM_GetVersionInfo_ValidPtr_ShouldSucceed);
    RUN_TEST(test_BswM_MainFunction_Uninit_ShouldSilentlyReturn);
    RUN_TEST(test_BswM_MainFunction_ValidCall_ShouldApplyRequestedMode);
    RUN_TEST(test_BswM_RequestMode_Uninit_ShouldReportError);
    RUN_TEST(test_BswM_RequestMode_ValidCall_ShouldLatchMode);
    RUN_TEST(test_BswM_GetCurrentMode_Uninit_ShouldReturnOffWithoutDet);
    RUN_TEST(test_BswM_GetCurrentMode_AfterRequestAndMain_ShouldReturnAppliedMode);
    RUN_TEST(test_BswM_GetRequestedMode_AfterTwoRequests_ShouldReturnLatest);
    RUN_TEST(test_BswM_Engine_AndCondition_ShouldRequireBothPorts);
    RUN_TEST(test_BswM_Engine_ActionLists_ShouldRunOnTransitionsOnly);
    RUN_TEST(test_BswM_Engine_InitialStateTrue_ShouldNotRunActionList);
    RUN_TEST(test_BswM_Engine_DisabledRule_ShouldNeverRunActionList);
    RUN_TEST(test_BswM_Engine_OutOfRangeIndices_ShouldBeIgnored);
    RUN_TEST(test_BswM_Engine_OrNotExpressions_ShouldEvaluate);
    RUN_TEST(test_BswM_Engine_NotEqualsAndNoneActionList_ShouldBeHandled);
    RUN_TEST(test_BswM_RequestMode_InvalidMode_ShouldReportParamModeDet);
    RUN_TEST(test_BswM_ActionSwitchMode_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_ActionSwitchMode_InvalidMode_ShouldReportDet);
    RUN_TEST(test_BswM_ActionSwitchMode_ValidMode_ShouldSetBothModes);
    RUN_TEST(test_BswM_EcuM_CurrentState_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_EcuM_CurrentState_ShouldRequestMappedMode);
    RUN_TEST(test_BswM_Notify_ComM_CurrentMode_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_ComM_CurrentMode_NonMatchingValue_ShouldNotTrigger);
    RUN_TEST(test_BswM_Notify_ComM_CurrentPNCMode_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_Dcm_CommunicationMode_CurrentState_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_Dcm_ApplicationUpdated_ShouldWriteFixedOne);
    RUN_TEST(test_BswM_Notify_Nm_StateChangeNotification_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_Nm_CarWakeUpIndication_ShouldWriteFixedOne);
    RUN_TEST(test_BswM_Notify_CanSM_CurrentState_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_EthSM_CurrentState_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_FrSM_CurrentState_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_LinSM_CurrentState_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_LinSM_CurrentSchedule_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_LinTp_RequestMode_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_EcuM_RequestedState_ShouldMapAndTriggerRule);
    RUN_TEST(test_BswM_Notify_EcuM_RequestedState_Unmapped_ShouldBeIgnored);
    RUN_TEST(test_BswM_Notify_BswMPartitionRestarted_ShouldWritePartitionIdLowByte);
    RUN_TEST(test_BswM_Notify_EthIf_PortGroupLinkStateChg_ShouldTriggerRule);
    RUN_TEST(test_BswM_Notify_EthIf_PortGroupLinkStateChg_LinkDown_ShouldNotTrigger);
    RUN_TEST(test_BswM_Notify_UnboundComposition_ShouldSilentlyDrop);
    RUN_TEST(test_BswM_Notify_ComM_CurrentMode_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_Notify_EcuM_RequestedState_Uninit_ShouldReportDet);
    RUN_TEST(test_BswM_Notify_BswMPartitionRestarted_Uninit_ShouldReportDet);

    return UnityEnd();
}
