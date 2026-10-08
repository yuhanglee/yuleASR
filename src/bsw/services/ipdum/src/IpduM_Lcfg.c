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
 * Canonical IpduM implementation: src/bsw/ecual/ipdum/ (target: ecual_ipdum).
 * See IpduM.c in this directory for the full convergence rationale.
 *
 * This Lcfg previously defined IpduM_Config (IpduM_StaticParts + mapping
 * table). The symbol had no consumers outside this module (the canonical
 * ecual implementation builds its mux configuration from its own
 * IpduM_Lcfg.c), so the definition was removed.
 *================================================================================================*/
