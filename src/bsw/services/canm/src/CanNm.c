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
 * CanNm previously existed in two layers (services + ecual) with divergent
 * implementations. Per the Phase 3 convergence decision matrix, the canonical
 * implementation is retained in:
 *
 *     src/bsw/ecual/canNm/           (target: ecual_canNm, ~1409 LOC)
 *
 * This file is intentionally emptied of all function and data definitions so
 * that the duplicate target compiles no symbols and cannot collide with the
 * canonical library at link time. The include/ directory and the CMake target
 * name are kept for compatibility (see src/bsw/services/CMakeLists.txt,
 * SERVICES_SHIMMED_MODULES).
 *
 * Convergence rationale:
 *   - ecual/canNm has full CanIf/ComM integration with 2-channel Lcfg.
 *   - services/canm (~507 LOC) was a standalone OSEK NM implementation with
 *     no CanIf/ComM integration and non-standard type usage.
 *   - ecual/canNm is the AUTOSAR-conformant implementation.
 *
 * Unique API disposition (diffed against the canonical version before this
 * change):
 *   - Conflicting symbols removed: CanNm_Init, CanNm_GetVersionInfo,
 *     CanNm_MainFunction (all present in both implementations).
 *   - Services-only APIs: none — the services/canm implementation exposed
 *     only the standard CanNm API subset.
 *
 * Note: Test files under tests/bsw/services/canm/ are updated to include
 * the canonical ecual/canNm headers via CMake include path redirection.
 *================================================================================================*/
