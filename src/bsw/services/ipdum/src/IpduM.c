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
 * IpduM previously existed in two layers (ecual + services) with divergent
 * implementations. Per the Phase 2 convergence decision matrix, the canonical
 * implementation is retained in:
 *
 *     src/bsw/ecual/ipdum/           (target: ecual_ipdum, ~448 LOC)
 *
 * The ecual version is canonical because it is a real multiplexing
 * implementation (Tx mux, Rx mux, static/dynamic part handling), while the
 * services version was a ~130 LOC stub.
 *
 * This file is intentionally emptied of all function and data definitions so
 * that the duplicate target compiles no symbols and cannot collide with the
 * canonical library at link time. Removed conflicting symbols: IpduM_Init,
 * IpduM_DeInit, IpduM_GetVersionInfo, IpduM_MainFunction (note: the two
 * sides even disagreed on the IpduM_DeInit signature: Std_ReturnType here
 * vs. void in the canonical version).
 *
 * Unique API disposition:
 *   - IpduM_SetIpduMode / IpduM_GetIpduMode had no consumers anywhere in the
 *     repository and are not declared by the canonical IpduM.h. They are
 *     recorded here as pending canonical-side work if IPdu mode switching
 *     support is ever required (implement against the canonical IpduM.h
 *     contract).
 *
 * The include/ directory and the CMake target name are kept for
 * compatibility (see src/bsw/services/CMakeLists.txt, SERVICES_SHIMMED_MODULES).
 *================================================================================================*/
