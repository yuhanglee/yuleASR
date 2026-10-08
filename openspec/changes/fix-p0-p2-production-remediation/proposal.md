# 变更提案：P0-P2 量产整改（需求→设计→实施）

> **变更 ID**: fix-p0-p2-production-remediation
> **状态**: In Progress
> **优先级**: P0/P1/P2
> **负责人**: BSW 量产收敛 Track
> **创建日期**: 2026-10-08
> **目标版本**: v2.0.0
> **估计工时**: 120h (15d)

## 背景

《BSW 量产就绪性再评估（2026-10-08）》在 8 阶段 P0/P1 修复后重新扫描全栈，识别出 **3 个 P0 + 13 个 P1 + 9 个 P2** 遗留问题。再评估评分从 C+ (3.0) 提升至 B- (3.5)，但仍存在阻断量产的关键差距：

| 维度 | 当前评分 | 量产目标 | 差距 |
|:-----|:---------|:---------|:-----|
| 模块唯一性 | 2.5 | 5.0 | 4 对新重复模块未收敛 |
| 安全证据完整性 | 2.0 | 4.0 | .osh/evidence/reviews/ 为空 |
| 测试覆盖率 | 3.0 | 4.5 | 130+ TEST_IGNORE stub，门禁 40%/25% 过低 |
| CI 完整性 | 2.5 | 5.0 | 6 个 job 永久跳过 |
| 配置生成覆盖 | 2.0 | 4.0 | 仅 17/104 模块 (16%) |
| 标准合规 | 3.0 | 4.5 | ISO 21434 Clauses 7/8/11 缺失 |

本变更按用户要求执行 **文档先行** 工作流：需求文档 (proposal.md) → 设计文档 (design.md) → 实施修复。

## 目标

### P0（阻断量产，3 项）

1. **P0-1**: 收敛 4 对重复模块（services/canm vs ecual/canNm, services/linsm vs ecual/linSM, mcal/fee vs ecual/fee, services/ramtst vs mcal/ramtst），采用 tombstone shim 模式
2. **P0-2**: 实现 SecOC Freshness 值的 NvM 持久化，确保 Master-Slave 同步状态跨重启保持
3. **P0-3**: 填充 `.osh/evidence/reviews/` 目录，为 13 个 ASIL-B 模块生成标准化评审记录

### P1（影响量产质量，13 项）

4. **P1-1**: 修复 51 个 MISRA error-severity 违规（ComM.c, CanNm.c）
5. **P1-2**: 实现 130+ TEST_IGNORE stub 测试的真实逻辑（分 3 批：ECUAL→Services→MCAL）
6. **P1-3**: 提升覆盖率门禁至 ISO 26262 SWE.4.BP1 要求（行覆盖 80%，分支覆盖 70%）
7. **P1-4**: 恢复 6 个永久跳过的 CI job（编写缺失的测试源文件）
8. **P1-5**: 创建量产 MCU MCAL 桩文件集（S32K312，11 个驱动）
9. **P1-6**: 扩展配置生成覆盖至全部 104 模块（当前 17/104 = 16%）
10. **P1-7**: SecOC Freshness 持久化不完整（与 P0-2 合并处理）
11. **P1-8**: 为 90+ 公共 API header 添加 Doxygen 注释骨架
12. **P1-9**: 实现 MC/DC 覆盖率测量（基于 gcov 条件/决策覆盖数据）
13. **P1-10**: 修复需求追溯矩阵不一致（运行 trace_requirements.py 并修正）
14. **P1-11**: 补充 ISO 21434 Clauses 7/8/11（distributed activities, continuous monitoring, vulnerability management）
15. **P1-12**: 编写 DoIP 全链集成测试（DoIP→SoAd→EthIf→Eth）
16. **P1-13**: 完善 CAN FD 支持（Can 驱动 FD 配置 + CanIf BRS 位处理）

### P2（改善工程成熟度，9 项）

17. **P2-1**: Com 模块返回类型统一为 Std_ReturnType（当前 uint8）
18. **P2-2**: Can 模块返回类型统一为 Std_ReturnType（当前 Can_ReturnType）
19. **P2-3**: 补充 examples/ 目录下的集成示例代码
20. **P2-4**: 添加 release 脚本（semver tag + SHA256 校验 + GPG 签名）
21. **P2-5**: 添加 Doxyfile + CI job 自动生成 API 文档
22. **P2-6**: 添加 ROM/RAM 占用追踪（arm-none-eabi-size + map 文件解析）
23. **P2-7**: 建立性能基准回归基线（JSON baseline + CI ±10% 阈值对比）
24. **P2-8**: 清理已收敛模块的空 shim 目录
25. **P2-9**: 审计 docs/ 目录，合并/删除重复/过期文档

## 范围

### 包含内容

**P0 批次（3 项）**:
- `src/bsw/services/canm/src/CanNm.c` — 替换为 tombstone（保留 ecual/canNm）
- `src/bsw/services/linsm/src/LinSM.c` — 替换为 tombstone（保留 ecual/linSM）
- `src/bsw/mcal/fee/src/Fee.c` — 替换为 tombstone（保留 ecual/fee）
- `src/bsw/services/ramtst/src/RamTst.c` — 替换为 tombstone（保留 mcal/ramtst）
- `src/bsw/services/CMakeLists.txt` — 添加 canm, linsm, ramtst 到 SERVICES_SHIMMED_MODULES
- `src/bsw/mcal/CMakeLists.txt` — 添加 fee 到 MCAL_SHIMMED_MODULES（新建）
- `tests/CMakeLists.txt` — 更新 canm/linsm/ramtst 测试 include 路径指向保留方
- `src/bsw/services/secoc/src/SecOC.c` — 添加 NvM_ReadBlock/NvM_WriteBlock 调用
- `src/bsw/services/secoc/include/SecOC.h` — 添加 NvM block ID 配置宏
- `.osh/evidence/reviews/TEMPLATE.md` — 创建评审记录模板
- `.osh/evidence/reviews/*.md` — 为 13 个 ASIL-B 模块填充评审记录

**P1 批次（13 项）**:
- `src/bsw/services/comM/src/ComM.c` — 修复 MISRA 违规
- `src/bsw/ecual/canNm/src/CanNm.c` — 修复 MISRA 违规
- `tests/unit/autosar/ecual/*.c` — 实现 30+ TEST_IGNORE stub（ECUAL 层）
- `tests/unit/autosar/services/*.c` — 实现 50+ TEST_IGNORE stub（Services 层）
- `tests/unit/autosar/mcal/*.c` — 实现 50+ TEST_IGNORE stub（MCAL 层）
- `.yuleosh/ci-config.yaml` — 覆盖率阈值 40%/25% → 60%/50%（Phase 1）→ 80%/70%（Phase 2）
- `tests/e2e/test_e2e_dds.c`, `test_e2e_secoc.c`, `test_e2e_e2e.c` — E2E 测试
- `tests/e2e/test_e2e_uds.c` — UDS 测试
- `tests/e2e/test_e2e_ota.c` — OTA 测试
- `tests/performance/test_latency.c`, `test_throughput.c`, `test_memory.c` — 性能测试
- `tests/security/test_security.c` — 安全测试
- `src/bsw/mcal/production/` — 创建 S32K312 MCAL 桩文件集（11 个驱动）
- `config/bsw_config.json` — 扩展至 104 模块
- `config/templates/*.j2` — 添加缺失模块的 Jinja2 模板
- `src/bsw/*/include/*.h` — 为 90+ header 添加 Doxygen 注释
- `coverage_run/mcdc_measure.sh` — MC/DC 测量脚本
- `docs/cybersecurity/cybersecurity-case.md` — 补充 Clauses 7/8/11
- `tests/integration/test_doip_chain.c` — DoIP 全链集成测试
- `src/bsw/mcal/can/src/Can.c` — CAN FD 配置字段 + DLC 映射
- `src/bsw/ecual/canif/src/CanIf.c` — BRS 位处理

**P2 批次（9 项）**:
- `src/bsw/services/com/src/Com.c` — 返回类型统一
- `src/bsw/mcal/can/src/Can.c` — 返回类型统一
- `examples/` — 集成示例代码
- `tools/release/create_release.sh` — Release 脚本
- `Doxyfile` — Doxygen 配置
- `.github/workflows/ci.yml` — 添加 doxygen job
- `tools/analysis/track_memory.sh` — ROM/RAM 追踪脚本
- `tools/performance/baseline.json` — 性能基线
- `docs/` — 文档审计与清理

### 不包含内容

- FreeRTOS 安全认证版替换评估（需产品级决策，另变更处理）
- S32K312 真实硬件 HIL 验证（Track C，需硬件设备）
- 第三方工具链集成（Green Hills, Lauterbach TRACE32）
- ASPICE CL2/CL3 过程域全面合规（需组织级过程改进）
- ISO 26262 Part 10 (ISO 26262-10) 工具认证（需独立工具鉴定）

## 验收标准

### P0 批次

- [ ] **S1**: 4 对重复模块收敛完成，全量 ctest 零失败
  - 证据：`services/canm`, `services/linsm`, `mcal/fee`, `services/ramtst` 的 `.c` 文件替换为 tombstone 注释；CMake 构建通过；`ctest --test-dir build-native` 114/114 零失败
- [ ] **S2**: SecOC Freshness 持久化功能验证通过
  - 证据：单元测试 `test_secoc_freshness_persistence` 验证 Init 时从 NvM 恢复 freshness counter，Verify 成功后写入 NvM；重启模拟测试验证跨重启状态保持
- [ ] **S3**: 13 个 ASIL-B 模块评审记录填充完成
  - 证据：`.osh/evidence/reviews/` 目录包含 13 个 `.md` 文件（Can, CanIf, CanSM, CanTp, Com, PduR, NvM, Dcm, Dem, E2E, WdgM, SecOC, BswM），每个文件包含评审日期、评审人、代码审查结果、覆盖率数据、MISRA 偏差清单

### P1 批次

- [ ] **S4**: MISRA error-severity 违规清零
  - 证据：`python tools/analysis/static_analysis.py` 输出 errors = 0（当前 51）
- [ ] **S5**: TEST_IGNORE stub 数量降至 20 以下
  - 证据：`grep -r "TEST_IGNORE" tests/unit/ | wc -l` < 20（当前 130+）
- [ ] **S6**: 覆盖率门禁提升至 80%/70%
  - 证据：`.yuleosh/ci-config.yaml` 中 `line_coverage_threshold: 80`, `branch_coverage_threshold: 70`；CI 运行通过
- [ ] **S7**: 6 个 CI job 恢复运行
  - 证据：`.github/workflows/ci.yml` 中 `e2e-dds-tests`, `uds-tests`, `ota-tests`, `multi-ecu-tests`, `performance-tests`, `security-tests` 不再被 guard-artifacts 跳过
- [ ] **S8**: 量产 MCU MCAL 桩文件集创建完成
  - 证据：`src/bsw/mcal/production/` 目录包含 Mcu/Port/Dio/Can/Spi/Gpt/Pwm/Adc/Wdg/Icu/Lin 11 个 `.c` 文件，每个文件标注 `#error "Production MCAL — replace with vendor IP"`
- [ ] **S9**: 配置生成覆盖提升至 80%+
  - 证据：`config/bsw_config.json` 包含 84+ 模块节点；`python config/tools/generate_bsw_config.py --validate-only` 通过
- [ ] **S10**: 90+ 公共 API header 包含 Doxygen 注释
  - 证据：`find src/bsw -name "*.h" -exec grep -l "@brief" {} \; | wc -l` >= 90
- [ ] **S11**: MC/DC 覆盖率可测量
  - 证据：`coverage_run/mcdc_measure.sh` 输出条件覆盖率和决策覆盖率数据
- [ ] **S12**: 需求追溯矩阵一致性验证通过
  - 证据：`python tools/traceability/trace_requirements.py` 输出 "Consistency check: PASS"
- [ ] **S13**: ISO 21434 Clauses 7/8/11 章节补充完成
  - 证据：`docs/cybersecurity/cybersecurity-case.md` 包含 "Distributed Activities", "Continuous Monitoring", "Vulnerability Management" 章节
- [ ] **S14**: DoIP 全链集成测试通过
  - 证据：`tests/integration/test_doip_chain.c` 编译并运行通过，验证 DoIP→SoAd→EthIf→Eth 链路
- [ ] **S15**: CAN FD 支持完整
  - 证据：Can 驱动包含 FD 配置字段（`Can_EnableFD`, `Can_SetBaudRate`）；CanIf 包含 BRS 位处理逻辑

### P2 批次

- [ ] **S16**: Com/Can 返回类型统一为 Std_ReturnType
  - 证据：`grep -r "uint8 Com_" src/bsw/services/com/include/` 无匹配；`grep -r "Can_ReturnType" src/bsw/mcal/can/include/` 无匹配
- [ ] **S17**: examples/ 目录包含 3+ 集成示例
  - 证据：`examples/` 目录包含 `basic_can_comm/`, `nvm_persistence/`, `diagnostic_session/` 示例
- [ ] **S18**: Release 脚本功能完整
  - 证据：`tools/release/create_release.sh --help` 输出使用说明；执行后生成 SHA256SUMS 和 GPG 签名
- [ ] **S19**: Doxygen API 文档自动生成
  - 证据：`doxygen Doxyfile` 生成 HTML 文档至 `docs/api/`；CI job `api-docs` 运行通过
- [ ] **S20**: ROM/RAM 占用可追踪
  - 证据：`tools/analysis/track_memory.sh` 输出 .text/.data/.bss 段大小统计
- [ ] **S21**: 性能基准回归检测启用
  - 证据：`tools/performance/baseline.json` 包含基线数据；CI job 对比当前值与基线，偏差 >10% 时告警
- [ ] **S22**: 空 shim 目录清理完成
  - 证据：`find src/bsw -type d -empty` 无匹配
- [ ] **S23**: 文档审计完成，重复/过期文档清理
  - 证据：`docs/` 目录无重复文件；过期文档移至 `docs/archived/`

## 风险评估

| 风险 | 概率 | 影响 | 缓解措施 |
|:-----|:-----|:-----|:---------|
| P0-1 模块收敛破坏现有测试 | 中 | 高 | 保留 CMake target，仅替换 .c 文件内容为 tombstone；测试 include 路径指向保留方 |
| P0-2 SecOC NvM 集成引入循环依赖 | 低 | 高 | SecOC 通过 NvM API 接口调用，不直接 #include NvM.h；使用 NvM 抽象层 |
| P0-3 评审记录内容不准确 | 中 | 中 | 评审记录基于实际覆盖率数据和 MISRA 扫描结果，非人工编造 |
| P1-2 TEST_IGNORE 实现工作量超预期 | 高 | 中 | 分批实施（ECUAL→Services→MCAL），每批 20-30 个；优先实现高频调用模块 |
| P1-3 覆盖率提升导致 CI 失败 | 中 | 中 | 分两阶段提升（60/50→80/70），给代码修复留出时间窗口 |
| P1-5 量产 MCAL 桩文件与现有测试冲突 | 低 | 中 | 桩文件使用 `#error` 阻止意外编译；测试继续使用现有 MCAL 实现 |
| P1-6 配置扩展遗漏模块 | 中 | 中 | 使用 `find src/bsw -name "*.c"` 自动扫描模块列表，交叉验证 bsw_config.json |

## 交付物与交叉引用

- **需求文档**: `openspec/changes/fix-p0-p2-production-remediation/proposal.md`（本文档）
- **设计文档**: `openspec/changes/fix-p0-p2-production-remediation/design.md`
- **任务台账**: `openspec/changes/fix-p0-p2-production-remediation/tasks.md`
- **规范与验收用例**: `openspec/changes/fix-p0-p2-production-remediation/specs/`
- **实施证据**: 各验收标准中的证据项

## 实施计划

### Phase 1: 需求文档（本文档）

- [x] 创建 proposal.md，覆盖全部 P0-P2 问题
- [x] 定义验收标准和证据要求
- [x] 风险评估和缓解措施

### Phase 2: 设计文档

- [ ] 创建 design.md，详细描述每项修复的技术方案
- [ ] P0-1 模块收敛设计（tombstone shim 模式）
- [ ] P0-2 SecOC Freshness 持久化设计（NvM 集成接口）
- [ ] P0-3 Safety 评审证据设计（模板 + 13 个模块记录）
- [ ] P1 各项设计要点

### Phase 3: 任务拆解

- [ ] 创建 tasks.md，拆解为可执行任务项
- [ ] 每项任务指定负责人、预估工时、依赖关系

### Phase 4: 实施

- [ ] P0 批次实施（3 天）
- [ ] P1 批次实施（7 天）
- [ ] P2 批次实施（3 天）

### Phase 5: 验证与归档

- [ ] 全量验证（编译、测试、静态分析、覆盖率、追溯）
- [ ] 更新 acceptance-matrix.md
- [ ] 归档至 `openspec/changes/archive/`
