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
 * Canonical Xcp implementation: src/bsw/services/xcp/ (target: service_xcp).
 * See Xcp.c in this directory for the full convergence rationale.
 *
 * This Lcfg previously held the ecual XCP data model (Xcp_DaqEntryPool,
 * Xcp_Odts, Xcp_DaqLists, Xcp_StimLists, Xcp_Segments, Xcp_Seed/Xcp_Key,
 * Xcp_Config, ...). All non-static symbols were consumed only by the removed
 * ecual Xcp.c (the canonical module carries its own static configuration),
 * so every definition was removed.
 *================================================================================================*/
