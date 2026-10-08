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
 * J1939Tp previously existed in two layers (ecual + services) with divergent
 * implementations. Per the Phase 2 convergence decision matrix, the canonical
 * implementation is retained in:
 *
 *     src/bsw/services/j1939tp/      (target: service_j1939tp)
 *
 * The services version is canonical because it NULL-checks ConfigPtr and
 * reports DET errors, and additionally implements J1939Tp_DeInit,
 * J1939Tp_GetVersionInfo, J1939Tp_CancelTransmit, J1939Tp_CancelReceive and
 * J1939Tp_ChangeParameter.
 *
 * This file is intentionally emptied of all function definitions so that the
 * duplicate target compiles no symbols and cannot collide with the canonical
 * library at link time. Removed conflicting symbols: J1939Tp_Init,
 * J1939Tp_Transmit, J1939Tp_RxIndication, J1939Tp_TxConfirmation,
 * J1939Tp_MainFunction. No unique APIs existed on the ecual side.
 *
 * The include/ directory and the CMake target name are kept for
 * compatibility (see src/bsw/ecual/CMakeLists.txt, ECUAL_SHIMMED_MODULES).
 *================================================================================================*/
