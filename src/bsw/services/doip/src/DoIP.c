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
 * DoIP previously existed in two layers (ecual + services) with divergent
 * implementations. Per the Phase 2 convergence decision matrix, the canonical
 * implementation is retained in:
 *
 *     src/bsw/ecual/doIP/            (target: ecual_doIP, ~1191 LOC)
 *
 * This file is intentionally emptied of all function and data definitions so
 * that the duplicate target compiles no symbols and cannot collide with the
 * canonical library at link time. The include/ directory and the CMake target
 * name are kept for compatibility (see src/bsw/services/CMakeLists.txt,
 * SERVICES_SHIMMED_MODULES).
 *
 * Unique API disposition (diffed against the canonical version before this
 * change):
 *   - Conflicting symbols removed: DoIP_Init, DoIP_DeInit,
 *     DoIP_GetVersionInfo, DoIP_MainFunction.
 *   - Services-only APIs (DoIP_IfTransmit, DoIP_IfRxIndication,
 *     DoIP_ActivateRouting, DoIP_CloseConnection, DoIP_VehicleAnnouncement,
 *     DoIP_RequestEntityStatus, DoIP_GetPowerMode, DoIP_SetPowerMode,
 *     DoIP_HandleAliveCheckTimeout, DoIP_SoAdTxConfirmation,
 *     DoIP_SoConModeChg, DoIP_TriggerTransmit, DoIP_TpRxIndication,
 *     DoIP_TpTxConfirmation) had no production consumers (verified: no
 *     callers outside this directory; the referencing tests under
 *     tests/ are unbuilt @ANCHOR3 placeholders). If a future consumer
 *     needs them, merge into the canonical DoIP.c against the canonical
 *     DoIP.h signatures.
 *
 * Note: DoIp_test.c in this directory is left untouched; it is excluded from
 * the service_doip target by the *_test.c source filter and is self-contained
 * (provides its own mocks).
 *================================================================================================*/
