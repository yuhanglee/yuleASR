/**
 * @file BswM_Lcfg.c
 * @brief BswM Configuration Tables — window controller ECU arbitration rules
 *
 * Default (pre-compile) configuration. Mode arbitration policy:
 *  - RUN entry:    EcuM RUN && ComM RUN requested        -> switch to RUN
 *  - SLEEP entry:  NM SLEEP requested, cleared -> RUN    -> switch to SLEEP / RUN
 *  - Shutdown:     EcuM SHUTDOWN requested               -> switch to SHUTDOWN
 * Integrators extend the action lists with user callouts as needed.
 */
/* @req SWS_BswM_00001 @req SWS_BswM_00002 @req SWS_BswM_00010 */

#include "BswM.h"
#include "BswM_Cfg.h"

/* Mode Request Ports: 0 EcuM, 1 ComM, 2 NM feed the existing arbitration
 * rules; 3..15 are the notification-fed ports of the联动回调族 (raw module
 * state semantics). The first three entries keep their legacy indices. */
static const BswM_ModeRequestPortType BswM_ModeRequestPorts[BSWM_MAX_MODE_REQUEST_PORTS] = {
    /*  0 */ BSWM_ECUM_REQUEST,
    /*  1 */ BSWM_COMM_REQUEST,
    /*  2 */ BSWM_NM_REQUEST,
    /*  3 */ BSWM_DCM_REQUEST,
    /*  4 */ BSWM_CANSM_REQUEST,
    /*  5 */ BSWM_ETHSM_REQUEST,
    /*  6 */ BSWM_FRSM_REQUEST,
    /*  7 */ BSWM_LINSM_REQUEST,
    /*  8 */ BSWM_COMM_PNC_REQUEST,
    /*  9 */ BSWM_ECUM_REQUESTED,
    /* 10 */ BSWM_PARTITION_REQUEST,
    /* 11 */ BSWM_ETHIF_REQUEST,
    /* 12 */ BSWM_NM_CARWAKEUP_REQUEST,
    /* 13 */ BSWM_DCM_APPUPDATED_REQUEST,
    /* 14 */ BSWM_LINSM_SCHEDULE_REQUEST,
    /* 15 */ BSWM_LINTP_REQUEST
};

/* Condition expressions */
static const BswM_ExpressionConfigType BswM_Expressions[BSWM_MAX_EXPRESSIONS] = {
    /* 0 */ { BSWM_EXPR_MODE_EQUALS, 0U, BSWM_MODE_VALUE_STARTUP,  0U, 0U },
    /* 1 */ { BSWM_EXPR_MODE_EQUALS, 0U, BSWM_MODE_VALUE_RUN,      0U, 0U },
    /* 2 */ { BSWM_EXPR_MODE_EQUALS, 1U, BSWM_MODE_VALUE_RUN,      0U, 0U },
    /* 3 */ { BSWM_EXPR_LOGICAL_AND, 0U, BSWM_MODE_VALUE_OFF,      1U, 2U },
    /* 4 */ { BSWM_EXPR_MODE_EQUALS, 2U, BSWM_MODE_VALUE_SLEEP,    0U, 0U },
    /* 5 */ { BSWM_EXPR_MODE_EQUALS, 0U, BSWM_MODE_VALUE_SHUTDOWN, 0U, 0U }
};

/* Action lists */
static const BswM_ActionType BswM_ActionListRun[] = {
    { BswM_ActionSwitchMode, BSWM_MODE_VALUE_RUN }
};

static const BswM_ActionType BswM_ActionListSleep[] = {
    { BswM_ActionSwitchMode, BSWM_MODE_VALUE_SLEEP }
};

static const BswM_ActionType BswM_ActionListShutdown[] = {
    { BswM_ActionSwitchMode, BSWM_MODE_VALUE_SHUTDOWN }
};

/* Rules (Priority: 0 = highest, evaluated first; Shutdown preempts)
 * Field order: RuleId, ConditionIndex, TrueList, FalseList, InitialState,
 * IsEnabled, Priority */
static const BswM_RuleType BswM_Rules[BSWM_MAX_RULES] = {
    /* RunEntry: EcuM RUN && ComM RUN -> RUN */
    { 0U, 3U, 0U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE, 1U },
    /* SleepEntry: NM SLEEP -> SLEEP, cleared -> RUN */
    { 1U, 4U, 1U, 2U, BSWM_RULE_STATE_FALSE, TRUE, 2U },
    /* Shutdown: EcuM SHUTDOWN -> SHUTDOWN (highest priority) */
    { 2U, 5U, 3U, BSWM_ACTION_LIST_NONE, BSWM_RULE_STATE_FALSE, TRUE, 0U }
};

static const BswM_ActionListType BswM_ActionLists[BSWM_MAX_ACTION_LISTS] = {
    { 1U, BswM_ActionListRun },
    { 1U, BswM_ActionListSleep },
    { 1U, BswM_ActionListRun },
    { 1U, BswM_ActionListShutdown }
};

/* Configuration */
const BswM_ConfigType BswM_Config = {
    .NumModeRequestPorts = 16U,
    .ModeRequestPorts    = BswM_ModeRequestPorts,
    .NumExpressions      = 6U,
    .Expressions         = BswM_Expressions,
    .NumRules            = 3U,
    .Rules               = BswM_Rules,
    .NumActionLists      = 4U,
    .ActionLists         = BswM_ActionLists
};
