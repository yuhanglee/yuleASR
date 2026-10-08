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
 * Canonical FiM implementation: src/bsw/services/fim/ (target: service_fim).
 * See FiM.c in this directory for the full convergence rationale.
 *
 * This Lcfg previously held the ecual FiM configuration model
 * (FiM_NumFids, FiM_NumEvents, FiM_FidConfigTable, FiM_EventConfigTable,
 * FiM_InhibitionConfigTable, FiM_EventFidMapTable, FiM_EventFidInhibitionMask
 * and friends). These tables were consumed only by the removed ecual FiM.c;
 * the canonical module is driven through its own FiM_Config/FiM_ConfigPtr
 * contract (services/fim/src/FiM_Lcfg.c), so all definitions were removed.
 *================================================================================================*/
