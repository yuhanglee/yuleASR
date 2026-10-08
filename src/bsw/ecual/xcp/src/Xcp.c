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
 * Xcp previously existed in two layers (ecual + services) with divergent
 * implementations. Per the Phase 2 convergence decision matrix, the canonical
 * implementation is retained in:
 *
 *     src/bsw/services/xcp/          (target: service_xcp, ~2086 LOC)
 *
 * The services version is the superset implementation (channel model, DAQ/
 * STIM, PGM commands, seed & unlock, resource protection, TriggerTransmit).
 *
 * This file is intentionally emptied of all function and data definitions so
 * that the duplicate target compiles no symbols and cannot collide with the
 * canonical library at link time. Removed conflicting symbols: Xcp_Init,
 * Xcp_DeInit, Xcp_MainFunction, Xcp_RxIndication, Xcp_TxConfirmation,
 * Xcp_ProcessCommand, Xcp_SendResponse, Xcp_SendError, Xcp_DaqProcessor and
 * the Xcp_Cmd* command handlers (the two sides even used incompatible
 * signatures for Xcp_Init/Xcp_RxIndication/Xcp_TxConfirmation).
 *
 * Unique API disposition:
 *   - Ecual-only helpers Xcp_MtaSet/Xcp_MtaRead/Xcp_MtaWrite,
 *     Xcp_SetCalPage/Xcp_GetCalPage/Xcp_CopyCalPage,
 *     Xcp_DaqTrigger/Xcp_SendDaqPacket had no consumers outside this
 *     directory (verified across src/ and tests/). They are recorded here as
 *     pending canonical-side work if the MTA memory-access abstraction or
 *     ecual-style calibration page routing is ever required.
 *
 * The include/ directory and the CMake target name are kept for
 * compatibility (see src/bsw/ecual/CMakeLists.txt, ECUAL_SHIMMED_MODULES).
 *================================================================================================*/
