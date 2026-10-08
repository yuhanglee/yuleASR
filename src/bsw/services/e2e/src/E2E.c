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

/**
 * @file E2E.c
 * @brief End-to-End Protection (AUTOSAR Classic BSW Module)
 * @details 模块级初始化/反初始化实现。
 *          E2E_Init 是 AUTOSAR SWS E2E 标准 API (E2E_Init(const void* ConfigPtr)),
 *          由本 Classic 库导出; host 版 (src/autosar/e2e/e2e_protection.c) 的
 *          同名无参 E2E_Init(void) 已于 2026-08-08 更名为 E2E_Protection_Init,
 *          避免同名不同签名符号同时链接 (P2-4)。
 * @author  AutoSAR Team
 * @version 1.0.0
 */

#include "E2E.h"
#include "E2E_Cfg.h"
#include "Det.h"

/*=============================================================================*
 * Module State
 *=============================================================================*/
static boolean E2E_ModuleInitialized = FALSE;

/*=============================================================================*
 * Development Error Tracer (DET)
 *=============================================================================*/
#define E2E_INSTANCE_ID                     (0x00U)

/* Service IDs */
#define E2E_SID_INIT                        (0x01U)
#define E2E_SID_DEINIT                      (0x02U)

/* DET error codes (module-specific, not part of the public API result set) */
#define E2E_E_DET_ALREADY_INITIALIZED       (0x0AU)
#define E2E_E_DET_NOT_INITIALIZED           (0x0BU)

#if (E2E_DEV_ERROR_DETECT == STD_ON)
    #define E2E_DET_REPORT_ERROR(ApiId, ErrorId) \
        (void)Det_ReportError(E2E_MODULE_ID, E2E_INSTANCE_ID, (ApiId), (ErrorId))
#else
    #define E2E_DET_REPORT_ERROR(ApiId, ErrorId)
#endif

/*=============================================================================*
 * Function Implementations
 *=============================================================================*/

/** @req SWS_E2E_00001 */
/**
 * @brief 初始化 E2E 模块 (AUTOSAR 标准 API)
 * @param ConfigPtr E2E 配置指针 (当前实现不依赖模块级配置)
 * @return E_OK 初始化成功 (重复初始化保持幂等, 同时上报 DET);
 *         E2E_E_INPUTERR_NULL 配置指针为空
 */
Std_ReturnType E2E_Init(const void* ConfigPtr)
{
    if (ConfigPtr == NULL_PTR)
    {
        E2E_DET_REPORT_ERROR(E2E_SID_INIT, E2E_E_INPUTERR_NULL);
        return E2E_E_INPUTERR_NULL;
    }

#if (E2E_DEV_ERROR_DETECT == STD_ON)
    /* 重复初始化: 上报 DET 错误, 保持幂等语义 (返回 E_OK) */
    if (E2E_ModuleInitialized == TRUE)
    {
        E2E_DET_REPORT_ERROR(E2E_SID_INIT, E2E_E_DET_ALREADY_INITIALIZED);
    }
#endif

    /* 重置模块状态并设置初始化标志 */
    E2E_ModuleInitialized = TRUE;

    return E_OK;
}

/** @req SWS_E2E_00002 */
/**
 * @brief 反初始化 E2E 模块
 * @return E_OK 始终成功 (幂等; 未初始化调用时上报 DET)
 */
Std_ReturnType E2E_DeInit(void)
{
#if (E2E_DEV_ERROR_DETECT == STD_ON)
    /* 未初始化调用: 上报 DET 错误, 保持幂等语义 (返回 E_OK) */
    if (E2E_ModuleInitialized == FALSE)
    {
        E2E_DET_REPORT_ERROR(E2E_SID_DEINIT, E2E_E_DET_NOT_INITIALIZED);
    }
#endif

    /* 清除模块状态, 恢复未初始化标志 */
    E2E_ModuleInitialized = FALSE;

    return E_OK;
}
