/**
 * @file IpduM.h
 * @brief I-PDU Multiplexer - AUTOSAR Services Module
 * @version 1.0.0
 * @date 2026-07-19
 */

#ifndef IPDUM_H
#define IPDUM_H

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "IpduM_Cfg.h"

#define IPDUM_AR_RELEASE_MAJOR_VERSION   4U
#define IPDUM_AR_RELEASE_MINOR_VERSION   4U
#define IPDUM_AR_RELEASE_REVISION_VERSION 0U
#define IPDUM_SW_MAJOR_VERSION           1U
#define IPDUM_SW_MINOR_VERSION           0U
#define IPDUM_SW_PATCH_VERSION           0U
#define IPDUM_MODULE_ID             0x38U
#define IPDUM_VENDOR_ID             0x0055U

typedef enum {
    IPDUM_IPDU_MODE_OFF = 0,
    IPDUM_IPDU_MODE_ON,
    IPDUM_IPDU_MODE_ALTERNATE
} IpduM_IpduModeType;

typedef struct {
    uint16 SourcePduId;
    uint16 DestPduId;
    uint8  SelectorPosition;
} IpduM_StaticPartType;

typedef struct {
    uint16 IpduId;
    PduIdType SourcePduId;
    PduIdType DestPduId;
    void (*RoutingCallback)(PduIdType SourceId, PduIdType DestId);
} IpduM_IpduMappingType;

typedef struct {
    uint16 NumStaticParts;
    const IpduM_StaticPartType* StaticParts;
    uint16 NumIpduMappings;
    const IpduM_IpduMappingType* IpduMapping;
} IpduM_ConfigType;

/**
 * @brief Initialize the module
 * @param[in] ConfigPtr Configuration reference
 */
void IpduM_Init(const IpduM_ConfigType* ConfigPtr);
/**
 * @brief De-initialize the module
 */
void IpduM_DeInit(void);
/**
 * @brief Set configuration value
 * @param[in] IpduId Identifier
 * @param[in] Mode Operation mode
 * @return Operation status
 */
Std_ReturnType IpduM_SetIpduMode(uint16 IpduId, IpduM_IpduModeType Mode);
/**
 * @brief Get requested information
 * @param[in] IpduId Identifier
 * @return Operation result
 */
IpduM_IpduModeType IpduM_GetIpduMode(uint16 IpduId);
/**
 * @brief Process periodic tasks
 */
void IpduM_MainFunction(void);
/**
 * @brief Get module version information
 * @param[in] versioninfo Version info
 */
void IpduM_GetVersionInfo(Std_VersionInfoType* versioninfo);

#endif /* IPDUM_H */