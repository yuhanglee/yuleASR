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
 * Canonical J1939Tp implementation: src/bsw/services/j1939tp/
 * (target: service_j1939tp). See J1939Tp.c in this directory for the full
 * convergence rationale.
 *
 * This Lcfg previously defined J1939Tp_NSduConfig[J1939TP_NUM_NSDUS], which
 * had no consumers outside the removed ecual J1939Tp.c (the canonical module
 * drives its configuration through J1939Tp_Config in its own Lcfg), so the
 * definition was removed.
 *================================================================================================*/
