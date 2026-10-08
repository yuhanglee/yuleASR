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
 * LinSM previously existed in two layers (services + ecual) with divergent
 * implementations. Per the Phase 3 convergence decision matrix, the canonical
 * implementation is retained in:
 *
 *     src/bsw/ecual/linSM/           (target: ecual_linSM)
 *
 * This file is intentionally emptied of all function and data definitions so
 * that the duplicate target compiles no symbols and cannot collide with the
 * canonical library at link time. The include/ directory and the CMake target
 * name are kept for compatibility (see src/bsw/services/CMakeLists.txt,
 * SERVICES_SHIMMED_MODULES).
 *
 * Convergence rationale:
 *   - ecual/linSM has standard AUTOSAR ComM/EcuM integration with proper
 *     schedule table management and LinIf lower-layer interface.
 *   - services/linsm was a self-contained implementation with non-standard
 *     types (STATIC, LinSM_ScheduleStatusType) and no ComM integration.
 *   - ecual/linSM is the AUTOSAR-conformant implementation.
 *
 * Unique API disposition (diffed against the canonical version before this
 * change):
 *   - Conflicting symbols removed: LinSM_Init, LinSM_DeInit,
 *     LinSM_GetVersionInfo, LinSM_MainFunction.
 *   - Services-only APIs: LinSM_ScheduleRequest, LinSM_GetCurrentSchedule,
 *     LinSM_RequestComMode, LinSM_GetCurrentComMode,
 *     LinSM_ScheduleConfirmation, LinSM_WakeUpConfirmation,
 *     LinSM_GotoSleepConfirmation — these are standard AUTOSAR LinSM APIs
 *     already present in the canonical ecual/linSM implementation.
 *
 * Note: Test files under tests/bsw/services/linsm/ are updated to include
 * the canonical ecual/linSM headers via CMake include path redirection.
 *================================================================================================*/
