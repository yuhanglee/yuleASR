# Acceptance Matrix

> Generated: 2026-08-25 | Updated: 2026-10-07
> Version: 0.2.0 — Phase 7 安全合规证据加固 (Task 8)
>
> 变更说明 (v0.1.0 → v0.2.0):
> - 补齐 30 个未关联 SHALL 的验证证据 (测试文件路径 + 匹配方式 + 置信度)
> - NFR 类 SHALL 以静态分析 / 覆盖率分析报告作为验证证据 (非单元测试)
> - 置信度说明: **High** = 专属测试直接验证; **Medium** = 间接证据 (相邻模块测试 / 报告级证据)
> - 后续可用 `tools/traceability/trace_requirements.py` 自动化复核本矩阵

| Req ID | Requirement | SHALL | 验证方法 | 测试文件 | 匹配方式 | 置信度 | 状态 |
|:------:|:-----------|:------|:---------|:--------|:--------|:------:|:----:|
| MCAL-SHALL-001 | MCAL-SHALL-001 | MCAL SHALL 提供标准 AUTOSAR API (如 Adc_Init, Can_Write) | Unit Test | tests/unit/test_mcal_api_contracts.c::test_MCAL001_Can_Init | API 契约测试 (test_MCAL001_* 覆盖 Adc/Can/Dio/Port/Wdg/Spi/Pwm/Gpt/Icu/Mcu/Lin 共 11 组标准 API) | High | ✅ |
| MCAL-SHALL-002 | MCAL-SHALL-002 | 所有 MCAL 模块 SHALL 支持同步和中断两种操作模式 | Unit Test | tests/unit/test_mcal_api_contracts.c::test_MCAL002_Spi_Sync + test_MCAL002_Spi_Async | API 契约测试 (test_MCAL002_* 覆盖 Spi 同步/异步、Adc 触发、Can MainFunction 中断驱动) | High | ✅ |
| MCAL-SHALL-003 | MCAL-SHALL-003 | MCAL SHALL 使用 MISRA C:2023 合规编码风格 | Static Analysis | docs/misra_compliance_report.md | cppcheck MISRA 扫描报告 (Required=0, Advisory=0, 320 源文件全量覆盖) | High | ✅ |
| ECUAL-SHALL-001 | ECUAL-SHALL-001 | ECUAL SHALL 使用 MCAL API, 不直接操作硬件寄存器 | Unit Test | tests/unit/autosar/ecual/test_CanIf.c::test_CanIf_Transmit | CanIf 测试经 mock Can_* (MCAL API) 验证 — ECUAL 层仅通过 MCAL 标准接口访问硬件 | Medium | ✅ |
| ECUAL-SHALL-002 | ECUAL-SHALL-002 | 看门狗管理器 SHALL 在超时前刷新 | Unit Test | tests/unit/autosar/services/WdgM_Test.c::test_TC014_CheckpointReached_Valid | Checkpoint 监督测试 (超时前 WdgM_CheckpointReached 刷新 + test_TC042_FailureThreshold 超时阈值) | High | ✅ |
| SVC-SHALL-001 | SVC-SHALL-001 | OS SHALL 提供符合 OSEK/AUTOSAR OS 的调度服务 | Integration Test | tests/qemu_full_stack/p1a_os_schedule/main_os_schedule.c | QEMU 全栈集成测试 (多任务优先级调度) + tests/unit/test_os_timing.c (时序保护) | High | ✅ |
| SVC-SHALL-002 | SVC-SHALL-002 | 通信栈 (CAN/以太网) SHALL 实现 PDU 路由 | Unit Test | tests/unit/pdur/test_pdur.c::test_PduR_ComTransmit | PduR 路由测试 (ComTransmit/CanIfRxIndication/CanIfTxConfirmation 路由路径) | High | ✅ |
| SVC-SHALL-003 | SVC-SHALL-003 | 诊断事件管理器 (Dem) SHALL 记录并上报 DTC | Unit Test | tests/unit/autosar/services/test_Dem.c::test_Dem_SetEventStatus_Failed | Dem 事件状态测试 (SetEventStatus_Failed 记录 + test_Dem_DTCConstants_Exist DTC 分组) | High | ✅ |
| NFR-SHALL-001 | NFR-SHALL-001 | 代码 MISRA C:2023 合规 | Static Analysis | .yuleosh/reports/misra-report.md | CI L1 MISRA 报告 (Required=0) — 与 docs/misra_compliance_report.md 交叉一致 | High | ✅ |
| NFR-SHALL-002 | NFR-SHALL-002 | 单元测试行覆盖率 | Coverage Analysis | .yuleosh/reports/c-coverage.json | CI 覆盖率实测数据 (line_rate=91.57%, 批E 2026-08-07) + .yuleosh/reports/branch-coverage-report.md (复现: tools/run_branch_coverage.sh) | High | ✅ |
| NFR-SHALL-003 | NFR-SHALL-003 | 条件覆盖率 | Coverage Analysis | .yuleosh/reports/branch-coverage-report.md | 分支覆盖实测 (branch_rate=79.79%, BRDA/lcov 实测; MC/DC 未测量 — 诚实标注见 docs/safety/VERIFICATION_REPORT.md §1) | Medium | ✅ |
| NFR-SHALL-004 | NFR-SHALL-004 | 静态分析 (cppcheck) | Static Analysis | .yuleosh/reports/misra-raw-output.txt | cppcheck 原始输出 + misra-report.json (CI L1 自动化) | High | ✅ |
| None | None | SHALL 使用 MISRA C:2023 `safety` 配置 | Configuration Review | .misra_config | MISRA safety 配置文件 (misra_c2023_texts.txt 基线, .yuleosh/misra-addon-config.json 扩展) | Medium | ✅ |
| SWR-001.1-01 | SWR-001.1-01 | **SWR-001.1-01**: SHALL support AUTOSAR Classic Platform 4.4.0 standard | Unit Test | tests/unit/autosar/services/Dcm/test_Dcm.c::test_Dcm_Init_ValidConfig | 标准命名/API 契约 | High | ✅ |
| SWR-001.1-02 | SWR-001.1-02 | **SWR-001.1-02**: SHALL implement MCAL abstraction layer covering 21 microcontroller driver modules | Unit Test | tests/unit/autosar/mcal/test_ADC.c::test_init_deinit | 模块覆盖测试 (mcal/ 下 21 个驱动测试文件) | High | ✅ |
| SWR-001.1-03 | SWR-001.1-03 | **SWR-001.1-03**: SHALL implement ECUAL abstraction layer covering 29 ECU hardware driver modules | Unit Test | tests/unit/autosar/ecual/test_CanIf.c::test_CanIf_Init_ValidConfig | 模块覆盖测试 (ecual/ 下 29 个抽象层测试文件) | High | ✅ |
| SWR-001.1-04 | SWR-001.1-04 | **SWR-001.1-04**: SHALL implement BSW Services layer covering 44 service modules | Unit Test | tests/unit/autosar/services/Dcm/test_Dcm.c::test_Dcm_MainFunction_Initialized | 模块覆盖测试 (services/ 下 44 个服务模块测试文件) | High | ✅ |
| SWR-001.1-05 | SWR-001.1-05 | **SWR-001.1-05**: SHALL target NXP S32K312 microcontroller platform | Unit Test | tests/unit/autosar/mcal/test_mcu.c::mcu_init_valid_config | MCU 平台测试 (S32K312 时钟/配置) | High | ✅ |
| SWR-001.1-06 | SWR-001.1-06 | **SWR-001.1-06**: SHALL support RTE generation for SWC-to-BSW communication | Tool Verification | tools/code_generators/rte/tests/test_rte_generator.py::test_map_uint8 | RTE 生成器单元测试 (ARXML→RTE C 代码) + scripts/rte_generation.sh CI 管线 | High | ✅ |
| SWR-002.1-01 | SWR-002.1-01 | **SWR-002.1-01**: SHALL implement E2E communication protection for safety-critical signals | Unit Test | tests/unit/autosar/services/E2E/test_E2E.c::test_E2E_P01_Protect_Check_RoundTrip | E2E P01 Profile 往返测试 | High | ✅ |
| SWR-002.1-02 | SWR-002.1-02 | **SWR-002.1-02**: SHALL support HSM-based cryptographic operations via Crypto module | Unit Test | tests/unit/autosar/mcal/test_Crypto.c::test_init_deinit | Crypto 模块测试 (S32K312 HSM + @req SWS_Crypto_00100+ 注解) | High | ✅ |
| SWR-002.1-03 | SWR-002.1-03 | **SWR-002.1-03**: SHALL provide RAM safety and lockstep monitoring for ASIL-D decomposition | Unit Test | tests/unit/autosar/services/RamSafety_test.c::test_TC005_StartupTest_Normal | RAM 安全测试 + WdgM_Test.c::test_TC035_LockstepIntegration (锁步) + tests/qemu_full_stack/p3a_ram_ecc/main_ram_ecc.c (QEMU ECC 注入) | High | ✅ |
| SWR-002.1-04 | SWR-002.1-04 | **SWR-002.1-04**: SHALL implement secure boot mechanism | Unit Test | src/bootloader/tests/test_sbl_main.c::test_sbl_boot_verify_pass | SBL 安全启动测试 (verify_pass/hash_fail/rollback) + tools/signing/sign_tool.py (ECDSA P-256 签名) | High | ✅ |
| SWR-003.1-01 | SWR-003.1-01 | **SWR-003.1-01**: SHALL implement CAN communication (Can, CanIf, CanTp, CanNm, CanSm) | Unit Test | tests/unit/autosar/mcal/test_CAN.c::test_init | CAN 栈测试 (mcal/test_CAN + ecual/test_CanIf/test_CanTp/test_canNm/test_canSm) | High | ✅ |
| SWR-003.1-02 | SWR-003.1-02 | **SWR-003.1-02**: SHALL implement LIN communication (Lin, LinIf, LinTp, LinNm, LinSM) | Unit Test | tests/unit/autosar/mcal/test_LIN.c::test_init_deinit | LIN 栈测试 (mcal/test_LIN + ecual/test_linif/test_linTp/test_linNm/test_linSM) | High | ✅ |
| SWR-003.1-03 | SWR-003.1-03 | **SWR-003.1-03**: SHALL implement Ethernet communication (Eth, EthIf, EthSm, SoAd, SomeIp, SomeIpSd) | Unit Test | tests/unit/autosar/mcal/test_ETH.c::test_init_deinit | Ethernet 栈测试 (test_ETH + test_ethif + test_ethSm + services/test_soad + test_someip + ecual/test_someipsd) | High | ✅ |
| SWR-003.1-04 | SWR-003.1-04 | **SWR-003.1-04**: SHALL implement DCM diagnostic communication manager | Unit Test | tests/unit/autosar/services/Dcm/test_Dcm.c::test_Dcm_MainFunction_Uninit | DCM 测试 (services/Dcm/test_Dcm.c 全套 UDS 服务) | High | ✅ |
| SWR-003.1-05 | SWR-003.1-05 | **SWR-003.1-05**: SHALL implement DoIP diagnostic over IP | Unit Test | tests/unit/autosar/ecual/test_doIP.c::test_DoIP_Init_ValidConfig | DoIP 测试 (ecual/test_doIP + tests/unit/test_doip.c 协议头解析/车辆识别) | High | ✅ |
| SWR-004.1-01 | SWR-004.1-01 | **SWR-004.1-01**: SHALL implement NVRAM manager (NvM) for persistent storage | Unit Test | tests/unit/autosar/services/Nvm/test_Nvm.c::test_NvM_Init_ValidConfig | NvM 测试 (CRC 校验/写计数器, tests/unit/nvm/ 补充) | High | ✅ |
| SWR-004.1-02 | SWR-004.1-02 | **SWR-004.1-02**: SHALL implement Flash EEPROM emulation (Fee) | Unit Test | tests/unit/fee/test_fee_init.c::test_Fee_Init_ValidConfig | Fee 测试 (fee/test_fee_init + test_fee_read + test_fee_write + @req SWS_Fee_* 注解) | High | ✅ |
| SWR-004.1-03 | SWR-004.1-03 | **SWR-004.1-03**: SHALL implement internal/external EEPROM driver | Unit Test | tests/unit/autosar/mcal/test_eep.c::test_eep_init_valid_config | EEPROM 测试 (mcal/test_eep 内部 + ecual/test_ea 外部) | High | ✅ |
| SWR-004.1-04 | SWR-004.1-04 | **SWR-004.1-04**: SHALL implement memory abstraction interface (MemIf) | Unit Test | tests/unit/autosar/services/test_memif.c::test_memif_Init_should_initialize | MemIf 测试 (services/test_memif + ecual/test_memif) | High | ✅ |
| SWR-004.1-05 | SWR-004.1-05 | **SWR-004.1-05**: SHALL support flash driver for S32K312 on-chip flash | Unit Test | tests/unit/autosar/mcal/test_flash.c::test_flash_init | Flash 测试 (mcal/test_flash + tests/unit/flash/test_flash_init 读写擦除) | High | ✅ |
| SWR-005.1-01 | SWR-005.1-01 | **SWR-005.1-01**: SHALL implement ECU state manager (EcuM) | Unit Test | tests/unit/services/test_ecum.c::ecum_init_startup_state | EcuM 测试 (+ tests/unit/ecum/test_ecum.c @req SWS_EcuM_* 注解) | High | ✅ |
| SWR-005.1-02 | SWR-005.1-02 | **SWR-005.1-02**: SHALL implement BSW scheduler (BswM) with mode management | Unit Test | tests/unit/autosar/services/test_bswm.c::test_BswM_Init_ValidConfig | BswM 测试 (模式管理/规则求值, 43 个测试用例) | High | ✅ |
| SWR-005.1-03 | SWR-005.1-03 | **SWR-005.1-03**: SHALL implement Watchdog manager (WdgM) | Unit Test | tests/unit/autosar/mcal/test_wdg.c::test_wdg_Init_should_initialize_successfully | Wdg 测试 (+ WdgM_Test.c 43 个监督测试 + tests/unit/wdgm/test_wdgm.c) | High | ✅ |
| SWR-005.1-04 | SWR-005.1-04 | **SWR-005.1-04**: SHALL implement Default Error Tracer (Det) | Unit Test | tests/unit/det/Det_Test.c::Test_Det_Init_Valid | Det 测试 (RegisterHooks/ReportError/Start, tests/unit/det/Det_Test.c) | High | ✅ |
| SWR-005.1-05 | SWR-005.1-05 | **SWR-005.1-05**: SHALL implement Diagnostic Event Manager (Dem) | Unit Test | tests/unit/autosar/services/test_Dem.c::test_Dem_Init_ValidConfig | Dem 测试 (冻结帧/DTCSetting/操作循环控制) | High | ✅ |
| SWR-005.1-06 | SWR-005.1-06 | **SWR-005.1-06**: SHALL implement Function Inhibition Manager (FiM) | Unit Test | tests/unit/fim/test_fim.c::test_FiM_Init | FiM 测试 (GetFunctionPermission/SetFunctionAvailable/MainFunction) | High | ✅ |
| SWR-005.1-07 | SWR-005.1-07 | **SWR-005.1-07**: SHALL implement CRC calculator | Unit Test | tests/unit/crc/Crc_test.c::test_Crc_CalculateCRC8_KnownValue | CRC 测试 (CRC8 已知向量 + services/test_crc + libs/test_lib_crc AES/CRC 库测试) | High | ✅ |
| SWR-005.1-08 | SWR-005.1-08 | **SWR-005.1-08**: SHALL implement OS (AUTOSAR SC4 compliant) | Unit Test | tests/unit/test_os_timing.c::test_Os_Timing_Execution_Budget | OS 时序保护测试 (Execution Budget/Lock/InterArrival — SC4 特性) | High | ✅ |
| SWR-005.1-09 | SWR-005.1-09 | **SWR-005.1-09**: SHALL support DLT (Diagnostic Log and Trace) | Unit Test | tests/unit/dlt/test_dlt.c::test_Dlt_Init | DLT 测试 (SendLogMessage/SendTraceMessage/MainFunction) | High | ✅ |
| SWR-006.1-01 | SWR-006.1-01 | **SWR-006.1-01**: SHALL implement 21 MCAL modules (ADC, CAN, Crypto, DIO, EEP, ETH, FEE, Flash, FLS, GPT, I2C, ICU, LIN, MCU, OCU, PORT, PWM, RAMTST, SPI, UART, WDG) | Unit Test | tests/unit/autosar/mcal/test_gpt.c::test_init_valid | 21 模块测试 (mcal/ 下 test_ADC/test_CAN/test_Crypto/test_dio/test_eep/test_ETH/test_flash/test_fls/test_gpt/test_i2c/test_icu/test_LIN/test_mcu/test_ocu/test_port/test_pwm/test_ramtst/test_spi/test_uart/test_wdg + test_fls) | High | ✅ |
| SWR-006.1-02 | SWR-006.1-02 | **SWR-006.1-02**: SHALL provide standardized AUTOSAR interface macros (SchM, Det, MemMap) | Unit Test | tests/unit/autosar/services/test_schm.c::test_schm_Init_should_initialize | 标准宏测试 (test_schm + test_det + MemMap 分区见 tests/unit/mem/test_mem.c) | Medium | ✅ |
| SWR-007.1-01 | SWR-007.1-01 | **SWR-007.1-01**: SHALL support ASW components: CommunicationManager, DiagnosticManager, EngineControl, IOControl, ModeManager, StorageManager, VehicleDynamics, WatchdogManager | Unit Test | tests/unit/services/test_comm.c::comm_init_valid_config | ASW 组件测试 (8 组件通信接口) | High | ✅ |
| SWR-007.1-02 | SWR-007.1-02 | **SWR-007.1-02**: SHALL implement RTE for component communication | Unit Test | tests/unit/rte/test_rte_cs_operations.c::test_Rte_Call_EngineControl_SetTargetRPM_normal | RTE CS 操作测试 (SWC 间通信: EngineControl/DiagnosticManager/WatchdogManager) | High | ✅ |
| SWR-008.1-01 | SWR-008.1-01 | **SWR-008.1-01**: SHALL integrate micro DDS middleware for inter-ECU communication | Unit Test | tests/unit/test_dds_qualification.c::test_dds_init_and_config | DDS 资格测试 (init/config/topic/pub-sub) | High | ✅ |

## Summary

- Total SHALL statements: 47
- Covered by tests / evidence: **47 (100%)**
- Uncovered: **0**
- Threshold: 100% → ✅ **PASS**

### 证据类型分布

| 验证方法 | 数量 | 说明 |
|:---------|:----:|:-----|
| Unit Test | 37 | 单元测试直接验证 (tests/unit/, tests/unit/autosar/, src/bootloader/tests/) |
| Integration Test | 1 | QEMU 全栈集成 (OS 调度 SVC-SHALL-001) |
| Static Analysis | 4 | MISRA 合规报告 + cppcheck 原始输出 (MCAL-SHALL-003, NFR-SHALL-001/004) |
| Coverage Analysis | 2 | GCov 覆盖率报告 (NFR-SHALL-002/003) |
| Configuration Review | 1 | .misra_config 安全配置审查 |
| Tool Verification | 1 | RTE 生成器 Python 单测 (SWR-001.1-06) |

### 置信度分布

| 置信度 | 数量 | 需求 |
|:------:|:----:|:-----|
| High | 43 | 专属测试/报告直接验证 |
| Medium | 4 | ECUAL-SHALL-001 (间接), NFR-SHALL-003 (分支覆盖受 GCov 限制), SWR-006.1-02 (MemMap 无专属测试), None-MISRA 配置 (仅配置审查) |

### 遗留说明 (v0.2.0)

1. **测试深度**: 本矩阵证明"证据关联完整性" (每条 SHALL 均有可定位的验证工件)。
   审查记录 (`.osh/evidence/review-log-summary.md`) 指出部分测试为 smoke 级 —
   行为级验证 (状态机/超时/错误注入) 应在后续迭代加深, 详见 Safety Case J-2/J-4 论据。
2. **自动化复核**: 运行 `python3 tools/traceability/trace_requirements.py` 可自动
   校验本矩阵中测试文件的存在性与 `@req SWS_*` 注解覆盖率, 输出 JSON + 控制台摘要。
3. **来源一致性**: 需求全集 (47 条) 与 `.osh/evidence/traceability-matrix.json`
   (`requirements` 数组, len=47) 保持一致。
