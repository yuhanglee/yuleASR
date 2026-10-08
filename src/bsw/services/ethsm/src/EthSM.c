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
 * EthSM previously existed in two layers (ecual + services) with divergent
 * implementations. Per the Phase 2 convergence decision matrix, the canonical
 * implementation is retained in:
 *
 *     src/bsw/ecual/ethsm/           (target: ecual_ethsm, ~853 LOC)
 *
 * The ecual version is canonical because it is a real network state machine
 * (per-network trcv/ctrl handling, TcpIp mode indications, ComM request
 * routing), while the services version was a ~162 LOC stub. The P0-A dual
 * network regression test (tests/unit/ethsm) compiles the canonical
 * ecual/ethSm/src/EthSM.c directly.
 *
 * This file is intentionally emptied of all function and data definitions so
 * that the duplicate target compiles no symbols and cannot collide with the
 * canonical library at link time. Removed conflicting symbols: EthSM_Init,
 * EthSM_DeInit, EthSM_GetVersionInfo, EthSM_MainFunction.
 *
 * Unique API disposition:
 *   - EthSM_SetState / EthSM_Start / EthSM_Stop / EthSM_GetState had no
 *     consumers outside this directory. They are recorded here as pending
 *     canonical-side work if a simplified start/stop control surface is ever
 *     required (implement against the canonical EthSM.h contract).
 *
 * The include/ directory and the CMake target name are kept for
 * compatibility (see src/bsw/services/CMakeLists.txt, SERVICES_SHIMMED_MODULES).
 *================================================================================================*/
