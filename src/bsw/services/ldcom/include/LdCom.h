/**
 * @file LdCom.h
 * @brief Large Data Communication (LdCom) — AUTOSAR BSW Module
 *
 * AUTOSAR R21-11 §12.11: LdCom provides segmentation and reassembly
 * for large PDUs that exceed a single CAN/LIN/Ethernet frame MTU.
 */
#ifndef LDCOM_H
#define LDCOM_H

#include "Std_Types.h"
#include "Com_Types.h"

/* Module ID */
#define LDCOM_MODULE_ID          0x0BUL

/* LdCom PDU direction */
typedef enum {
    LDCOM_DIR_TX,
    LDCOM_DIR_RX
} LdCom_DirectionType;

/* LdCom segmentation status */
typedef enum {
    LDCOM_SEG_IDLE,
    LDCOM_SEG_IN_PROGRESS,
    LDCOM_SEG_COMPLETE,
    LDCOM_SEG_ABORTED,
    LDCOM_SEG_ERROR
} LdCom_SegmentStatusType;

/* LdCom configuration */
typedef struct {
    PduIdType pduId;
    uint16 maxSegmentSize;
    uint16 interSegmentInterval;
    LdCom_DirectionType direction;
} LdCom_ConfigType;

/** @req SWS_LdCom_00001 */
/* Initialization */
/**
 * @brief Initialize the module
 * @param[in] config Configuration reference
 * @return Operation status
 */
Std_ReturnType LdCom_Init(const LdCom_ConfigType* config);
/** @req SWS_LdCom_00002 */
/**
 * @brief De-initialize the module
 */
void LdCom_DeInit(void);

/** @req SWS_LdCom_00003 */
/* Main function */
/**
 * @brief Process periodic tasks
 */
void LdCom_MainFunction(void);

/** @req SWS_LdCom_00004 */
/* Transmission */
/**
 * @brief Transmit data
 * @param[in] pduId Identifier
 * @param[in] pduInfo PDU reference
 * @return Operation status
 */
Std_ReturnType LdCom_Transmit(PduIdType pduId, const PduInfoType* pduInfo);
/** @req SWS_LdCom_00005 */
/**
 * @brief Cancel pending operation
 * @param[in] pduId Identifier
 * @return Operation status
 */
Std_ReturnType LdCom_CancelTransmit(PduIdType pduId);

/** @req SWS_LdCom_00006 */
/* Reception */
/**
 * @brief Receive data
 * @param[in] pduId Identifier
 * @param[in] pduInfo PDU reference
 * @return Operation status
 */
Std_ReturnType LdCom_RxIndication(PduIdType pduId, const PduInfoType* pduInfo);

/** @req SWS_LdCom_00007 */
/* Status */
/**
 * @brief Get current status
 * @param[in] pduId Identifier
 * @param[in] status State value
 * @return Operation status
 */
Std_ReturnType LdCom_GetSegmentStatus(PduIdType pduId, LdCom_SegmentStatusType* status);
/** @req SWS_LdCom_00008 */
/**
 * @brief Get requested information
 * @param[in] pduId Identifier
 * @param[in] bytesSent bytesSent value
 * @param[in] totalBytes totalBytes value
 * @return Operation status
 */
Std_ReturnType LdCom_GetProgress(PduIdType pduId, uint16* bytesSent, uint16* totalBytes);

/** @req SWS_LdCom_00009 */
/* Trigger transmit */
/**
 * @brief Trigger action
 * @param[in] pduId Identifier
 * @param[in] pduInfo PDU reference
 * @return Operation status
 */
Std_ReturnType LdCom_TriggerTransmit(PduIdType pduId, PduInfoType* pduInfo);

#endif /* LDCOM_H */
