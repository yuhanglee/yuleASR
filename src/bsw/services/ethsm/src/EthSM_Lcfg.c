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
 * Canonical EthSM implementation: src/bsw/ecual/ethsm/ (target: ecual_ethsm).
 * See EthSM.c in this directory for the full convergence rationale.
 *
 * This Lcfg previously defined `const EthSM_ConfigType EthSM_Config` (a
 * config OBJECT), which collided by name with the canonical
 * `const EthSM_ConfigType* const EthSM_Config` (a config POINTER) defined by
 * ecual/ethsm/src/EthSM_Lcfg.c. No consumer referenced the services variant,
 * so the definition was removed. Keep this asymmetry in mind when porting
 * config access to the canonical side: the canonical EthSM_Config is a
 * pointer.
 *================================================================================================*/
