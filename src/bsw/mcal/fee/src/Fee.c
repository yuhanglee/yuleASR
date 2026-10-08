/*==================================================================================================
* Project              : YuleTech AutoSAR BSW
* Platform             : NXP i.MX8M Mini
*
* Copyright (c) 2026 Shanghai Yule Electronics Technology Co., Ltd.
* All rights reserved.
*
* SPDX-License-Identifier: MIT
*
*================================================================================================*/

/*==================================================================================================
 *                     PHASE 3 DUPLICATE-MODULE CONVERGENCE - FORWARDING SHIM
 *==================================================================================================
 * Fee (MCAL layer) previously existed in two layers (mcal + ecual) with
 * divergent implementations. Per the Phase 3 convergence decision matrix,
 * the canonical implementation is retained in:
 *
 *     src/bsw/ecual/fee/             (target: ecual_fee)
 *
 * This file is intentionally emptied of all function and data definitions so
 * that the duplicate target compiles no symbols and cannot collide with the
 * canonical library at link time. The include/ directory and the CMake target
 * name are kept for compatibility (see src/bsw/mcal/CMakeLists.txt,
 * MCAL_SHIMMED_MODULES).
 *
 * Convergence rationale:
 *   - ecual/fee provides a block-based API (Fee_Read/Fee_Write with
 *     block numbers) that correctly matches the AUTOSAR MemIf contract
 *     and the standard Fee specification (SWS_Fee).
 *   - mcal/fee was an address-based flash driver (Fee_Read/Fee_Write with
 *     raw flash addresses) — this is the wrong abstraction level for the
 *     AUTOSAR Fee module. Address-based access belongs in Fls/Flash drivers,
 *     not in Fee.
 *   - ecual/fee is the AUTOSAR-conformant implementation.
 *
 * Unique API disposition (diffed against the canonical version before this
 * change):
 *   - Conflicting symbols removed: Fee_Init, Fee_DeInit, Fee_GetVersionInfo,
 *     Fee_MainFunction, Fee_Read, Fee_Write, Fee_GetStatus, Fee_GetJobResult,
 *     Fee_Cancel, Fee_SetMode.
 *   - MCAL-only APIs (address-based): Fee_Erase, Fee_Compare,
 *     Fee_BlankCheck, Fee_Suspend, Fee_Resume, Fee_JobEndNotification,
 *     Fee_JobErrorNotification, Fee_GetNextState, Fee_IsStateTransitionValid,
 *     Fee_UpdateWearLeveling, Fee_GetPreferredPageForGc, Fee_GetBlockConfig,
 *     Fee_GetPageConfig — these are low-level flash operations that belong
 *     in the Fls/Flash driver layer, not in Fee. If a future consumer needs
 *     them, they should be implemented in the Fls driver.
 *================================================================================================*/
