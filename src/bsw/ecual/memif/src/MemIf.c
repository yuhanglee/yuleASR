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
 * MemIf previously existed in two layers (ecual + services) with divergent
 * implementations. Per the Phase 2 convergence decision matrix, the canonical
 * implementation is retained in:
 *
 *     src/bsw/services/memif/        (target: service_memif)
 *
 * The services version is canonical because it contains the MemMap/Rte
 * integration and the full device-state model (MemIf_DeviceState,
 * MemIf_ModuleInitialized, block-number validation).
 *
 * This file is intentionally emptied of all function and data definitions so
 * that the duplicate target compiles no symbols and cannot collide with the
 * canonical library at link time.
 *
 * Unique API disposition:
 *   - MemIf_EraseImmediateBlock was the only ecual-only API. It is declared
 *     by the canonical services MemIf.h and consumed by NvM.c, and was
 *     MERGED into the canonical services/memif/src/MemIf.c as part of this
 *     convergence (the ecual implementation's dispatch relied on ecual-only
 *     static state that would have gone dead in a shim).
 *   - All remaining ecual APIs (MemIf_Init/Read/Write/Cancel/GetStatus/
 *     GetJobResult/InvalidateBlock/GetVersionInfo/SetMode) were duplicates
 *     of the canonical API surface and were removed.
 *
 * The include/ directory and the CMake target name are kept for
 * compatibility (see src/bsw/ecual/CMakeLists.txt, ECUAL_SHIMMED_MODULES;
 * s0_smoke_test keeps linking ecual_memif as an empty archive).
 *================================================================================================*/
