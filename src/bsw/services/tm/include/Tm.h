/**
 * @file Tm.h
 * @brief Time Manager (Tm) — AUTOSAR BSW Module
 *
 * AUTOSAR R21-11 §12.15: Tm provides centralized time management
 * services for BSW modules including time base synchronization,
 * time conversion, and timing event scheduling.
 */
#ifndef TM_H
#define TM_H

#include "Std_Types.h"

/* Module ID */
#define TM_MODULE_ID             0x0CUL

/* Time base types */
typedef uint64 Tm_TimeBaseType;
typedef uint32 Tm_DurationType;

/* Time base status */
typedef enum {
    TM_STATUS_RUNNING,
    TM_STATUS_STOPPED,
    TM_STATUS_SYNCHRONIZED,
    TM_STATUS_FREE_RUNNING,
    TM_STATUS_ERROR
} Tm_StatusType;

/* Time base information */
typedef struct {
    Tm_TimeBaseType currentValue;
    Tm_DurationType resolution;
    boolean isSynchronized;
    Tm_StatusType status;
} Tm_TimeBaseInfoType;

/* Global time */
typedef struct {
    uint32 secondsHigh;
    uint32 secondsLow;
    uint32 nanoseconds;
} Tm_GlobalTimeType;

/* Configuration */
typedef struct {
    uint8 numTimeBases;
    Tm_DurationType defaultResolution;
    boolean enableSync;
} Tm_ConfigType;

/** @req SWS_Tm_00001 */
/* Initialization */
/**
 * @brief Initialize the module
 * @param[in] config Configuration reference
 * @return Operation status
 */
Std_ReturnType Tm_Init(const Tm_ConfigType* config);
/** @req SWS_Tm_00002 */
/**
 * @brief De-initialize the module
 */
void Tm_DeInit(void);

/** @req SWS_Tm_00003 */
/* Main function */
/**
 * @brief Process periodic tasks
 */
void Tm_MainFunction(void);

/** @req SWS_Tm_00004 */
/* Time base operations */
/**
 * @brief Get requested information
 * @param[in] timeBaseId Identifier
 * @param[in] value Parameter value
 * @return Operation status
 */
Std_ReturnType Tm_GetTimeBaseValue(uint8 timeBaseId, Tm_TimeBaseType* value);
/** @req SWS_Tm_00005 */
/**
 * @brief Set configuration value
 * @param[in] timeBaseId Identifier
 * @param[in] value Parameter value
 * @return Operation status
 */
Std_ReturnType Tm_SetTimeBaseValue(uint8 timeBaseId, Tm_TimeBaseType value);
/** @req SWS_Tm_00006 */
/**
 * @brief Get requested information
 * @param[in] timeBaseId Identifier
 * @param[in] info Information pointer
 * @return Operation status
 */
Std_ReturnType Tm_GetTimeBaseInfo(uint8 timeBaseId, Tm_TimeBaseInfoType* info);

/** @req SWS_Tm_00007 */
/* Global time */
/**
 * @brief Get requested information
 * @param[in] time time value
 * @return Operation status
 */
Std_ReturnType Tm_GetGlobalTime(Tm_GlobalTimeType* time);
/** @req SWS_Tm_00008 */
/**
 * @brief Set configuration value
 * @param[in] time time value
 * @return Operation status
 */
Std_ReturnType Tm_SetGlobalTime(const Tm_GlobalTimeType* time);

/** @req SWS_Tm_00009 */
/* Synchronization */
/**
 * @brief sync time base
 * @param[in] sourceId Identifier
 * @param[in] targetId Identifier
 * @return Operation status
 */
Std_ReturnType Tm_SyncTimeBase(uint8 sourceId, uint8 targetId);

/* Duration */
/**
 * @brief Get requested information
 * @param[in] timeBaseId Identifier
 * @param[in] since since value
 * @return Operation result
 */
Tm_DurationType Tm_GetElapsedDuration(uint8 timeBaseId, Tm_TimeBaseType since);

#endif /* TM_H */
