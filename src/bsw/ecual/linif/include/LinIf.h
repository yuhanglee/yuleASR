/**
 * @file LinIf.h
 * @brief LIN Interface - AUTOSAR ECUAL Module
 * @version 2.2.0
 * @date 2026-09-28
 * @author YuleTech
 *
 * @implements AUTOSAR_SWS_LINInterface.pdf
 */

#ifndef LINIF_H
#define LINIF_H

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "LinIf_Cfg.h"

#define LINIF_AR_RELEASE_MAJOR_VERSION   4U
#define LINIF_AR_RELEASE_MINOR_VERSION   4U
#define LINIF_AR_RELEASE_REVISION_VERSION 0U
#define LINIF_SW_MAJOR_VERSION           1U
#define LINIF_SW_MINOR_VERSION           2U
#define LINIF_SW_PATCH_VERSION           0U
#define LINIF_MODULE_ID             0x27U
#define LINIF_VENDOR_ID             0x0055U

#define LINIF_UNCONDITIONAL_FRAME   0x00U
#define LINIF_EVENT_TRIGGERED_FRAME 0x01U
#define LINIF_SPORADIC_FRAME        0x02U
#define LINIF_DIAGNOSTIC_FRAME      0x03U

#define LINIF_NULL_SCHEDULE         0x00U
#define LINIF_Normal                0x01U

/* Channel State Machine */
typedef uint8 LinIf_ChannelStateType;
#define LINIF_CHANNEL_UNINIT        0x00U
#define LINIF_CHANNEL_INIT          0x01U
#define LINIF_CHANNEL_ONLINE        0x02U
#define LINIF_CHANNEL_SLEEP         0x03U

/* Service IDs for Error Tracing - transceiver control & node configuration */
#define LINIF_SID_SETTRCVMODE           0x20U
#define LINIF_SID_GETTRCVMODE           0x21U
#define LINIF_SID_SETTRCVWAKEUPMODE     0x22U
#define LINIF_SID_GETTRCVWAKEUPREASON   0x23U
#define LINIF_SID_CHECKWAKEUP           0x24U
#define LINIF_SID_SETCONFIGUREDNAD      0x25U
#define LINIF_SID_GETCONFIGUREDNAD      0x26U
#define LINIF_SID_SETPIDTABLE           0x27U
#define LINIF_SID_GETPIDTABLE           0x28U
#define LINIF_SID_ISSUPPORTTPTRANSMIT   0x29U

/* Development error codes. LINIF_E_PARAM_POINTER/LINIF_E_PARAM_CHANNEL were
 * private to LinIf.c and are exported here for the new API family.
 * NOTE: the requested 0x40 for LINIF_E_PARAM_CHANNEL conflicts with the
 * long-standing private LINIF_E_PARAM_SCHEDULE (0x40, still in LinIf.c), so
 * the established 0x50 value is kept; LINIF_E_PARAM_POINTER keeps its
 * established 0x10 instead of introducing a duplicate at 0x41. */
#define LINIF_E_PARAM_POINTER       0x10U
#define LINIF_E_PARAM_CHANNEL       0x50U
#define LINIF_E_PARAM_VALUE         0x42U

/* LIN transceiver operation modes - semantics aligned with LinTrcv_OpmodeType */
typedef uint8 LinIf_TrcvModeType;
#define LINIF_TRCV_MODE_NORMAL      0x00U
#define LINIF_TRCV_MODE_STANDBY     0x01U
#define LINIF_TRCV_MODE_SLEEP       0x02U

/* LIN transceiver wake-up modes - semantics aligned with LinTrcv_WakeupModeType */
typedef uint8 LinIf_TrcvWakeupModeType;
#define LINIF_TRCV_WU_ENABLE        0x00U
#define LINIF_TRCV_WU_DISABLE       0x01U
#define LINIF_TRCV_WU_CLEAR         0x02U

/* LIN transceiver wake-up reasons - semantics aligned with LinTrcv_WakeupReasonType */
typedef uint8 LinIf_TrcvWakeupReasonType;
#define LINIF_TRCV_WU_ERROR         0x00U
#define LINIF_TRCV_WU_BY_BUS        0x01U
#define LINIF_TRCV_WU_BY_PIN        0x02U
#define LINIF_TRCV_WU_INTERNALLY    0x03U
#define LINIF_TRCV_WU_NOT_SUPPORTED 0x04U
#define LINIF_TRCV_WU_POWER_ON      0x05U
#define LINIF_TRCV_WU_RESET         0x06U
#define LINIF_TRCV_WU_BY_SYSERR     0x07U

typedef uint8 LinIf_ScheduleTableType;

/* Frame Type */
typedef struct {
    uint8    FrameIdx;
    uint8    Pid;
    uint8    Dlc;
    uint8    FrameType;
    boolean  IsPublish;
} LinIf_FrameConfigType;

/* Schedule Entry */
typedef struct {
    uint16   DelayMs;
    uint8    FrameIdx;
} LinIf_ScheduleEntryType;

/* Schedule Table Config */
typedef struct {
    uint8    Schedule;
    uint8    EntryCount;
    const LinIf_ScheduleEntryType* Entries;
} LinIf_ScheduleTableConfigType;

/* Channel Config */
typedef struct {
    uint8    ChannelId;
    uint8    NumFrames;
    uint8    NumSchedules;
    const LinIf_FrameConfigType* Frames;
    const LinIf_ScheduleTableConfigType* Schedules;
} LinIf_ChannelConfigType;

/* PDU Type */
typedef struct {
    uint8 Id;
    uint8 Dlc;
    const uint8* DataPtr;
} LinIf_PduType;

/* TX PDU to frame mapping */
typedef struct {
    uint8 Channel;
    uint8 FrameIdx;
} LinIf_TxPduMapType;

/* Top Config */
typedef struct {
    uint8    NumChannels;
    const LinIf_ChannelConfigType* Channels;
    uint8    NumTxPdus;
    const LinIf_TxPduMapType* TxPduMap;
} LinIf_ConfigType;

extern const LinIf_ConfigType LinIf_Config;

/**
 * @brief Initialize the module
 * @param[in] ConfigPtr Configuration reference
 */
void LinIf_Init(const LinIf_ConfigType* ConfigPtr);
/**
 * @brief De-initialize the module
 * @param[in] ConfigPtr Configuration reference
 */
void LinIf_DeInit(void);
/**
 * @brief Transmit data
 * @param[in] TxPduId Identifier
 * @param[in] PduInfoPtr Pointer reference
 * @return Operation status
 */
Std_ReturnType LinIf_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr);
/**
 * @brief Set configuration value
 * @param[in] ScheduleTableId Identifier
 * @return Operation status
 */
Std_ReturnType LinIf_SetSchedule(uint8 ScheduleTableId);
/**
 * @brief schedule request
 * @param[in] Channel Channel identifier
 * @param[in] ScheduleTable ScheduleTable value
 * @return Operation status
 */
Std_ReturnType LinIf_ScheduleRequest(uint8 Channel, LinIf_ScheduleTableType ScheduleTable);
/**
 * @brief wake up
 * @param[in] Channel Channel identifier
 * @return Operation status
 */
Std_ReturnType LinIf_WakeUp(uint8 Channel);
/**
 * @brief goto sleep
 * @param[in] Channel Channel identifier
 * @return Operation status
 */
Std_ReturnType LinIf_GotoSleep(uint8 Channel);
/**
 * @brief Receive data
 * @param[in] LinChannel Channel identifier
 * @param[in] PduInfoPtr Pointer reference
 */
void LinIf_RxIndication(uint8 LinChannel, const LinIf_PduType* PduInfoPtr);
/**
 * @brief Process periodic tasks
 * @param[in] LinChannel Channel identifier
 * @param[in] PduInfoPtr Pointer reference
 */
void LinIf_MainFunction(void);
/**
 * @brief Get module version information
 * @param[in] versioninfo Version info
 */
void LinIf_GetVersionInfo(Std_VersionInfoType* versioninfo);

/* Transceiver control - delegated to the LinTrcv driver (channel IDs map 1:1) */
/**
 * @brief Set configuration value
 * @param[in] Channel Channel identifier
 * @param[in] Mode Operation mode
 * @return Operation status
 */
Std_ReturnType LinIf_SetTrcvMode(uint8 Channel, LinIf_TrcvModeType Mode);
/**
 * @brief Get requested information
 * @param[in] Channel Channel identifier
 * @param[in] ModePtr Operation mode
 * @return Operation status
 */
Std_ReturnType LinIf_GetTrcvMode(uint8 Channel, LinIf_TrcvModeType* ModePtr);
/**
 * @brief Set configuration value
 * @param[in] Channel Channel identifier
 * @param[in] Mode Operation mode
 * @return Operation status
 */
Std_ReturnType LinIf_SetTrcvWakeupMode(uint8 Channel, LinIf_TrcvWakeupModeType Mode);
/**
 * @brief Get requested information
 * @param[in] Channel Channel identifier
 * @param[in] ReasonPtr Pointer reference
 * @return Operation status
 */
Std_ReturnType LinIf_GetTrcvWakeupReason(uint8 Channel, LinIf_TrcvWakeupReasonType* ReasonPtr);
/**
 * @brief Check condition
 * @param[in] Channel Channel identifier
 * @return Operation status
 */
Std_ReturnType LinIf_CheckWakeup(uint8 Channel);

/* Node configuration - per-channel runtime state (not static config tables) */
/**
 * @brief Set configuration value
 * @param[in] Channel Channel identifier
 * @param[in] NAD NAD value
 * @return Operation status
 */
Std_ReturnType LinIf_SetConfiguredNAD(uint8 Channel, uint8 NAD);
/**
 * @brief Get requested information
 * @param[in] Channel Channel identifier
 * @param[in] NADPtr Pointer reference
 * @return Operation status
 */
Std_ReturnType LinIf_GetConfiguredNAD(uint8 Channel, uint8* NADPtr);
/**
 * @brief Set configuration value
 * @param[in] Channel Channel identifier
 * @param[in] PidTable Identifier
 * @param[in] NumFrames NumFrames value
 * @return Operation status
 */
Std_ReturnType LinIf_SetPIDTable(uint8 Channel, const uint8* PidTable, uint8 NumFrames);
/**
 * @brief Get requested information
 * @param[in] Channel Channel identifier
 * @param[in] PidTable Identifier
 * @param[in] NumFramesPtr Pointer reference
 * @return Operation status
 */
Std_ReturnType LinIf_GetPIDTable(uint8 Channel, uint8* PidTable, uint8* NumFramesPtr);
/**
 * @brief is support tp transmit
 * @param[in] Channel Channel identifier
 * @return Result flag
 */
boolean LinIf_IsSupportTpTransmit(uint8 Channel);

/* Upper layer callbacks. LinIf.c provides weak no-op defaults; the
 * integrating ECU overrides them (LinNm defines TxConfirmation and
 * ScheduleRequestConfirmation). */
/**
 * @brief Transmit data
 * @param[in] Channel Channel identifier
 * @param[in] LinTxPduId Identifier
 */
void LinIf_TxConfirmation(uint8 Channel, uint8 LinTxPduId);
/**
 * @brief schedule request confirmation
 * @param[in] Channel Channel identifier
 * @param[in] ScheduleIndex Index value
 */
void LinIf_ScheduleRequestConfirmation(uint8 Channel, uint8 ScheduleIndex);
/**
 * @brief wake up confirmation
 * @param[in] Channel Channel identifier
 */
void LinIf_WakeUpConfirmation(uint8 Channel);
/**
 * @brief goto sleep confirmation
 * @param[in] Channel Channel identifier
 */
void LinIf_GotoSleepConfirmation(uint8 Channel);
/**
 * @brief Receive data
 * @param[in] Channel Channel identifier
 * @param[in] PduInfoPtr Pointer reference
 */
void LinIf_RxCallback(uint8 Channel, const LinIf_PduType* PduInfoPtr);

#endif /* LINIF_H */
