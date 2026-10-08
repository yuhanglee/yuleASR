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
 * RamTst previously existed in two layers (services + mcal) with divergent
 * implementations. Per the Phase 3 convergence decision matrix, the canonical
 * implementation is retained in:
 *
 *     src/bsw/mcal/ramtst/           (target: mcal_ramtst, ~v2.0.0)
 *
 * This file is intentionally emptied of all function and data definitions so
 * that the duplicate target compiles no symbols and cannot collide with the
 * canonical library at link time. The include/ directory and the CMake target
 * name are kept for compatibility (see src/bsw/services/CMakeLists.txt,
 * SERVICES_SHIMMED_MODULES).
 *
 * Convergence rationale:
 *   - mcal/ramtst is v2.0.0 with 6 algorithms (March C, March C-, March B,
 *     March A, Checkerboard, Walking 1/0), error record tracking, and
 *     Lcfg-based configuration.
 *   - services/ramtst was v1.0.0 with only March C / March C- (2 algorithms),
 *     no error records, and simpler configuration.
 *   - mcal/ramtst is the superset implementation.
 *
 * Unique API disposition (diffed against the canonical version before this
 * change):
 *   - Conflicting symbols removed: RamTst_Init, RamTst_DeInit,
 *     RamTst_GetVersionInfo, RamTst_RunTest, RamTst_GetResult,
 *     RamTst_Abort, RamTst_MainFunction.
 *   - Services-only APIs: none — services/ramtst exposed only the standard
 *     RamTst API subset already present in mcal/ramtst.
 *
 * Note: Test files under tests/bsw/services/ramtst/ are updated to include
 * the canonical mcal/ramtst headers via CMake include path redirection.
 *================================================================================================*/
