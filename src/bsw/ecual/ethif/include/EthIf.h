/**
 * @file EthIf.h
 * @brief Ethernet Interface - AUTOSAR ECUAL Module
 * @version 2.0.0
 * @date 2026-07-19
 * @author YuleTech
 *
 * @implements AUTOSAR_SWS_EthernetInterface.pdf
 */

#ifndef ETHIF_H
#define ETHIF_H

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "EthIf_Cfg.h"
#include "ModuleId.h"

#define ETHIF_AR_RELEASE_MAJOR_VERSION   4U
#define ETHIF_AR_RELEASE_MINOR_VERSION   4U
#define ETHIF_AR_RELEASE_REVISION_VERSION 0U
#define ETHIF_SW_MAJOR_VERSION           1U
#define ETHIF_SW_MINOR_VERSION           0U
#define ETHIF_SW_PATCH_VERSION           0U
#define ETHIF_MODULE_ID             MODULE_ID_ETHIF   /* Fixed: was 0x70, conflict with RTE/CSM/SomeIp — see ModuleId.h */
#define ETHIF_VENDOR_ID             0x0055U
#define ETHIF_MAX_CONTROLLERS       4U
#define ETHIF_MAX_VLANS             4U

#define ETH_MODE_DOWN               0x00U
#define ETH_MODE_ACTIVE             0x01U
#define ETH_MODE_SLEEP              0x02U

/* Controller mode aliases used by EthSM */
#define ETHIF_MODE_DOWN             ETH_MODE_DOWN
#define ETHIF_MODE_ACTIVE           ETH_MODE_ACTIVE
#define ETHIF_MODE_SLEEP            ETH_MODE_SLEEP

typedef enum {
    ETHIF_CS_STOPPED = 0,
    ETHIF_CS_STARTED,
    ETHIF_CS_SLEEP
} EthIf_ControllerMode;

/* Transceiver link state */
typedef enum {
    ETHIF_LINK_STATE_DOWN = 0,
    ETHIF_LINK_STATE_ACTIVE,
    ETHIF_LINK_STATE_INVALID
} EthIf_LinkStateType;

typedef struct {
    uint8  MacAddress[6];
    uint16 EtherType;
    uint8* SduPtr;
    uint16 SduLength;
} EthIf_PduType;

typedef void (*EthIf_RxCallback)(uint8 ControllerId, const EthIf_PduType* PduInfoPtr);

typedef enum {
    ETHIF_FILTER_MAC = 0,
    ETHIF_FILTER_ETHERTYPE,
    ETHIF_FILTER_VLAN
} EthIf_FilterType;

typedef struct {
    EthIf_FilterType FilterType;
    uint8 ControllerId;
    uint8 MacAddress[6];
    uint16 EtherType;
    uint16 VlanId;
    EthIf_RxCallback RxCallback;
} EthIf_RxFilterType;

typedef struct {
    uint8 CtrlIdx;
    uint8 VlanId;
    uint8 CtrlMode;
    uint8 MacAddress[6];
    uint32 ControllerHandle;
} EthIf_ControllerConfigType;

typedef struct {
    uint16 VlanId;
    uint8  Priority;
} EthIf_VlanConfigType;

typedef struct {
    uint8 NumControllers;
    const EthIf_ControllerConfigType* Controllers;
    uint8 NumVlans;
    const EthIf_VlanConfigType* Vlans;
    uint8 NumRxFilters;
    const EthIf_RxFilterType* RxFilters;
} EthIf_ConfigType;

/**
 * @brief Initialize the module
 * @param[in] ConfigPtr Configuration reference
 */
void EthIf_Init(const EthIf_ConfigType* ConfigPtr);
/**
 * @brief De-initialize the module
 * @param[in] ConfigPtr Configuration reference
 */
void EthIf_DeInit(void);
/**
 * @brief Transmit data
 * @param[in] ControllerId Identifier
 * @param[in] BufferHandle Data buffer
 * @param[in] PduInfoPtr Pointer reference
 * @return Operation status
 */
Std_ReturnType EthIf_Transmit(uint8 ControllerId, uint32 BufferHandle, const EthIf_PduType* PduInfoPtr);
/**
 * @brief Set configuration value
 * @param[in] ControllerId Identifier
 * @param[in] Mode Operation mode
 * @return Operation status
 */
Std_ReturnType EthIf_SetControllerMode(uint8 ControllerId, EthIf_ControllerMode Mode);
/**
 * @brief Get requested information
 * @param[in] ControllerId Identifier
 * @return Operation result
 */
EthIf_ControllerMode EthIf_GetControllerMode(uint8 ControllerId);
/**
 * @brief Receive data
 * @param[in] ControllerId Identifier
 * @param[in] PduInfoPtr Pointer reference
 */
void EthIf_RxIndication(uint8 ControllerId, const EthIf_PduType* PduInfoPtr);
/**
 * @brief Transmit data
 * @param[in] ControllerId Identifier
 * @param[in] BufferHandle Data buffer
 */
void EthIf_TxConfirmation(uint8 ControllerId, uint32 BufferHandle);
/**
 * @brief Process periodic tasks
 * @param[in] ControllerId Identifier
 * @param[in] BufferHandle Data buffer
 */
void EthIf_MainFunction(void);
/**
 * @brief Get module version information
 * @param[in] versioninfo Version info
 */
void EthIf_GetVersionInfo(Std_VersionInfoType* versioninfo);
/**
 * @brief Get current module state
 * @param[in] TrcvIdx Index value
 * @param[in] LinkStatePtr State value
 * @return Operation status
 */
Std_ReturnType EthIf_GetTransceiverLinkState(uint8 TrcvIdx, EthIf_LinkStateType* LinkStatePtr);

#endif /* ETHIF_H */