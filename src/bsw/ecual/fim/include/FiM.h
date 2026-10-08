/*==================================================================================================
* Project              : YuleTech AutoSAR BSW
* Platform             : NXP i.MX8M Mini
* Dependencies         : ...
*
* Copyright (c) 2026 Shanghai Yule Electronics Technology Co., Ltd.
* All rights reserved.
*
* SPDX-License-Identifier: MIT
*
*================================================================================================*/

/*
 * FiM.h - Function Inhibition Manager
 * AUTOSAR_SWS_FunctionInhibitionManager implementation
 */

#ifndef FIM_H
#define FIM_H

/**
 * @file FiM.h
 * @brief FIM module public API
 */


#include "Std_Types.h"
#include "FiM_Cfg.h"
#include "ModuleId.h"

/* Module / vendor identification */
#define FIM_VENDOR_ID               (0x0055U)
#define FIM_MODULE_ID               (MODULE_ID_FIM_ECUAL) /* Fixed: was 0x71, conflict with SD/SomeIpSd — see ModuleId.h */
#define FIM_INSTANCE_ID             (0x00U)

/* AUTOSAR Version Information */
#define FIM_AR_RELEASE_MAJOR_VERSION    4u
#define FIM_AR_RELEASE_MINOR_VERSION    4u
#define FIM_AR_RELEASE_REVISION_VERSION 0u

/* Software Version Information */
#define FIM_SW_MAJOR_VERSION            1u
#define FIM_SW_MINOR_VERSION            0u
#define FIM_SW_PATCH_VERSION            0u

/* Service IDs for error reporting */
#define FIM_SID_INIT                        0x00u
#define FIM_SID_DEINIT                      0x01u
#define FIM_SID_GETFUNCTIONPERMISSION       0x01u
#define FIM_SID_SETFUNCTIONAVAILABLE        0x02u
#define FIM_SID_DEMTRIGGERONMONITORSTATUS   0x03u
#define FIM_SID_DEMTRIGGERONEVENTSTATUS     0x04u
#define FIM_SID_GETVERSIONINFO              0x05u
#define FIM_SID_MAINFUNCTION                0x06u

/* Error Codes */
#define FIM_E_NO_ERROR                      0x00u
#define FIM_E_UNINIT                        0x01u
#define FIM_E_FID_OUT_OF_RANGE              0x02u
#define FIM_E_EVENTID_OUT_OF_RANGE          0x03u
#define FIM_E_PARAM_POINTER                 0x04u
#define FIM_E_NOT_INITIALIZED               0x11u
#define FIM_E_INIT_FAILED                   0x12u
#define FIM_E_PARAM_CONFIG                  0x13u

/* Function Inhibition Manager Status */
typedef enum {
    FIM_UNINIT = 0,
    FIM_INIT
} FiM_StateType;

/* Type Definitions (FiM_FunctionIdType is defined in FiM_Cfg.h) */
typedef uint32 FiM_MaskedEventsType;

/* External declarations for configuration data */
extern const uint16 FiM_NumFids;
extern const uint16 FiM_NumEvents;
extern const uint8 FiM_InhibitionConfiguration[];
extern const uint16 FiM_EventIdFidMap[];
extern const uint8 FiM_EventFidInhibitionMask[];

/* Function prototypes */
/**
 * @brief Initialize the module
 * @param[in] configPtr Configuration reference
 */
extern void FiM_Init(const void* configPtr);
/**
 * @brief De-initialize the module
 * @param[in] configPtr Configuration reference
 */
extern void FiM_DeInit(void);
/**
 * @brief Get requested information
 * @param[in] FID Identifier
 * @param[in] Permission Permission value
 * @return Operation status
 */
extern Std_ReturnType FiM_GetFunctionPermission(FiM_FunctionIdType FID, boolean* Permission);
/**
 * @brief Set configuration value
 * @param[in] FID Identifier
 * @param[in] Availability Availability value
 * @return Operation status
 */
extern Std_ReturnType FiM_SetFunctionAvailable(FiM_FunctionIdType FID, boolean Availability);
/**
 * @brief dem trigger on monitor status
 * @param[in] EventId Identifier
 */
extern void FiM_DemTriggerOnMonitorStatus(uint16 EventId);
/**
 * @brief dem trigger on event status
 * @param[in] EventId Identifier
 */
extern void FiM_DemTriggerOnEventStatus(uint16 EventId);
/**
 * @brief Process periodic tasks
 * @param[in] EventId Identifier
 */
extern void FiM_MainFunction(void);

#if (FIM_VERSION_INFO_API == STD_ON)
/**
 * @brief Get module version information
 * @param[in] versioninfo Version info
 */
extern void FiM_GetVersionInfo(Std_VersionInfoType* versioninfo);
#endif

/* Callback functions for Dem integration */
/**
 * @brief dem trigger on event status uds
 * @param[in] EventId Identifier
 * @param[in] EventStatusByteOld State value
 * @param[in] EventStatusByteNew State value
 */
extern void FiM_DemTriggerOnEventStatusUds(uint16 EventId, uint8 EventStatusByteOld, uint8 EventStatusByteNew);

#endif /* FIM_H */
