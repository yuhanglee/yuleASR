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
 *                     PHASE 2 DUPLICATE-MODULE CONVERGENCE - FORWARDING SHIM
 *==================================================================================================
 * FiM previously existed in two layers (ecual + services) with divergent
 * implementations. Per the Phase 2 convergence decision matrix and AUTOSAR
 * layering (FiM belongs to the Services layer), the canonical implementation
 * is retained in:
 *
 *     src/bsw/services/fim/          (target: service_fim)
 *
 * This file is intentionally emptied of all function definitions so that the
 * duplicate target compiles no symbols and cannot collide with the canonical
 * library at link time. Removed conflicting symbols: FiM_Init, FiM_DeInit,
 * FiM_GetFunctionPermission, FiM_SetFunctionAvailable (note: the two sides
 * used different signatures for FiM_Init and FiM_GetFunctionPermission).
 *
 * Unique API disposition:
 *   - FiM_DemTriggerOnMonitorStatus / FiM_DemTriggerOnEventStatus (ecual,
 *     1-arg EventId only) diverge from the canonical services FiM.h
 *     declarations, which take (Dem_EventIdType, Dem_EventStatusType /
 *     Dem_UdsStatusByteType pairs). Keeping the 1-arg definitions alive in a
 *     shim would have violated the canonical header contract (silent ABI
 *     mismatch for future callers), so they were removed.
 *   - The canonical FiM.h also declares FiM_DemTriggerOnEventStatusUds,
 *     FiM_MainFunction and FiM_GetVersionInfo, which the canonical FiM.c
 *     does not yet define. No production callers exist today (Dem does not
 *     trigger FiM yet; fim_svc_test explicitly does not exercise them).
 *     Implementing these Dem-trigger entry points against the canonical
 *     2-arg signatures is recorded as pending canonical-side work.
 *================================================================================================*/
