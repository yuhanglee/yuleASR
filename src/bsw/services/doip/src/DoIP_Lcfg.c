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
 * Canonical DoIP implementation: src/bsw/ecual/doIP/ (target: ecual_doIP).
 * See DoIP.c in this directory for the full convergence rationale.
 *
 * This Lcfg previously defined the services-variant configuration data
 * (DoIP_Vin, DoIP_Eid, DoIP_Gid, DoIP_EntityLogicalAddress,
 * DoIP_GeneralConfig, DoIP_State) and the Dcm_DoIP* notification callbacks
 * (Dcm_DoIPRxIndication, Dcm_DoIPTxConfirmation, Dcm_DoIPRoutingActivation).
 * None of these symbols had consumers outside this module (the canonical DCM
 * wires its DoIP interaction through the ecual DoIP path), so all definitions
 * were removed. If a future DCM integration needs Dcm_DoIP* callbacks, port
 * them against the canonical ecual/doIP design.
 *================================================================================================*/
