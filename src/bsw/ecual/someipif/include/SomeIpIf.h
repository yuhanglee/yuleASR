/**
 * @file SomeIpIf.h
 * @brief SOME/IP Interface - AUTOSAR ECUAL Module
 * @version 2.0.0
 * @date 2026-07-19
 * @author YuleTech
 *
 * @implements AUTOSAR_SWS_SOMEIPTransformer.pdf
 */

#ifndef SOMEIPIF_H
#define SOMEIPIF_H

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "ModuleId.h"

#define SOMEIPIF_AR_RELEASE_MAJOR_VERSION   4U
#define SOMEIPIF_AR_RELEASE_MINOR_VERSION   4U
#define SOMEIPIF_AR_RELEASE_REVISION_VERSION 0U
#define SOMEIPIF_SW_MAJOR_VERSION           1U
#define SOMEIPIF_SW_MINOR_VERSION           0U
#define SOMEIPIF_SW_PATCH_VERSION           0U
#define SOMEIPIF_MODULE_ID          MODULE_ID_SOMEIPIF   /* Fixed: was 0x82, conflict with CDD_Lockstep — see ModuleId.h */
#define SOMEIPIF_VENDOR_ID          0x0055U

/* Connection Types */
#define SOMEIP_CONNECTION_TCP       0x00U
#define SOMEIP_CONNECTION_UDP       0x01U

/* Endpoint Type */
typedef struct {
    uint32 IpAddress;
    uint16 Port;
    uint8  ConnectionType;
} SomeIpIf_EndpointType;

/* Service Config Type */
typedef struct {
    uint16 ServiceId;
    uint16 InstanceId;
    SomeIpIf_EndpointType Endpoint;
    boolean IsReliable;
} SomeIpIf_ServiceConfigType;

typedef struct {
    uint8  ChannelId;
    uint32 LocalIp;
    uint32 SubnetMask;
    uint16 UdpPort;
    uint16 TcpPort;
    uint8  NumRxFilters;
    const uint16* RxServiceIds;
} SomeIpIf_ChannelConfigType;

typedef struct {
    uint8  NumChannels;
    const SomeIpIf_ChannelConfigType* Channels;
    uint16 NumServices;
    const SomeIpIf_ServiceConfigType* Services;
    uint16 NumEndpoints;
    const SomeIpIf_EndpointType* Endpoints;
} SomeIpIf_ConfigType;

/** @req SWS_SomeIpIf_00001 */
/**
 * @brief Initialize the module
 * @param[in] ConfigPtr Configuration reference
 */
void SomeIpIf_Init(const SomeIpIf_ConfigType* ConfigPtr);
/** @req SWS_SomeIpIf_00002 */
/**
 * @brief De-initialize the module
 * @param[in] ConfigPtr Configuration reference
 */
void SomeIpIf_DeInit(void);
/** @req SWS_SomeIpIf_00005 */
/**
 * @brief Transmit data
 * @param[in] TxPduId Identifier
 * @param[in] PduInfoPtr Pointer reference
 * @return Operation status
 */
Std_ReturnType SomeIpIf_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr);
/** @req SWS_SomeIpIf_00006 */
/**
 * @brief Receive data
 * @param[in] RxPduId Identifier
 * @param[in] PduInfoPtr Pointer reference
 */
void SomeIpIf_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr);
/** @req SWS_SomeIpIf_00004 */
/**
 * @brief Process periodic tasks
 * @param[in] RxPduId Identifier
 * @param[in] PduInfoPtr Pointer reference
 */
void SomeIpIf_MainFunction(void);
/** @req SWS_SomeIpIf_00007 */
/**
 * @brief Set configuration value
 * @param[in] ChannelId Channel identifier
 * @param[in] Online Online value
 * @return Operation status
 */
Std_ReturnType SomeIpIf_SetState(uint8 ChannelId, boolean Online);
/** @req SWS_SomeIpIf_00003 */
/**
 * @brief Get module version information
 * @param[in] versioninfo Version info
 */
void SomeIpIf_GetVersionInfo(Std_VersionInfoType* versioninfo);

#endif /* SOMEIPIF_H */