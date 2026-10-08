# 任务台账：P0-P2 量产整改

> **变更 ID**: fix-p0-p2-production-remediation
> **关联提案**: `proposal.md` | **设计文档**: `design.md`
> **创建日期**: 2026-10-08
> **总预估工时**: 120h (15d)

---

## 任务状态图例

| 状态 | 含义 |
|:-----|:-----|
| [ ] | 待开始 |
| [~] | 进行中 |
| [x] | 已完成 |
| [!] | 阻塞 |

---

## Phase 1: 需求与设计文档

| # | 任务 | 负责人 | 工时 | 状态 | 证据 |
|:--|:-----|:-------|:-----|:-----|:-----|
| 1.1 | 创建 proposal.md | — | 2h | [x] | `openspec/changes/fix-p0-p2-production-remediation/proposal.md` |
| 1.2 | 创建 design.md | — | 4h | [x] | `openspec/changes/fix-p0-p2-production-remediation/design.md` |
| 1.3 | 创建 tasks.md | — | 1h | [~] | 本文档 |

---

## Phase 2: P0 批次（3 天）

### P0-1: 4 对重复模块收敛

| # | 任务 | 负责人 | 工时 | 状态 | 依赖 | 证据 |
|:--|:-----|:-------|:-----|:-----|:-----|:-----|
| 2.1.1 | services/canm → tombstone shim | — | 1h | [ ] | — | `services/canm/src/CanNm.c` 替换为墓碑注释 |
| 2.1.2 | services/linsm → tombstone shim | — | 1h | [ ] | — | `services/linsm/src/LinSM.c` 替换为墓碑注释 |
| 2.1.3 | services/ramtst → tombstone shim | — | 1h | [ ] | — | `services/ramtst/src/RamTst.c` 替换为墓碑注释 |
| 2.1.4 | mcal/fee → tombstone shim | — | 1h | — | `mcal/fee/src/Fee.c` 替换为墓碑注释 |
| 2.1.5 | 更新 services/CMakeLists.txt | — | 0.5h | [ ] | 2.1.1-3 | `SERVICES_SHIMMED_MODULES` 追加 canm, linsm, ramtst |
| 2.1.6 | 更新 mcal/CMakeLists.txt | — | 0.5h | [ ] | 2.1.4 | 新增 `MCAL_SHIMMED_MODULES` 列表，追加 fee |
| 2.1.7 | 更新 canm 测试 include 路径 | — | 0.5h | [ ] | 2.1.1 | `tests/bsw/services/canm/CMakeLists.txt` → ecual/canNm/include |
| 2.1.8 | 更新 linsm 测试 include 路径 | — | 0.5h | [ ] | 2.1.2 | `tests/bsw/services/linsm/CMakeLists.txt` → ecual/linSM/include |
| 2.1.9 | 更新 ramtst 测试 include 路径 | — | 0.5h | [ ] | 2.1.3 | `tests/bsw/services/ramtst/CMakeLists.txt` → mcal/ramtst/include |
| 2.1.10 | 全量编译验证 | — | 0.5h | [ ] | 2.1.5-9 | `cmake --build build-native` exit 0 |
| 2.1.11 | 全量测试验证 | — | 0.5h | [ ] | 2.1.10 | `ctest --test-dir build-native` 114/114 零失败 |

**P0-1 小计**: 8h

### P0-2: SecOC Freshness NvM 持久化

| # | 任务 | 负责人 | 工时 | 状态 | 依赖 | 证据 |
|:--|:-----|:-------|:-----|:-----|:-----|:-----|
| 2.2.1 | 定义 SecOC_PersistedDataType 结构 | — | 1h | [ ] | — | SecOC.c 中添加结构体定义 |
| 2.2.2 | 实现 SecOC_PersistFreshness() | — | 2h | [ ] | 2.2.1 | SecOC.c 中添加 NvM_WriteBlock 调用 |
| 2.2.3 | 实现 Init 时 NvM 恢复逻辑 | — | 2h | [ ] | 2.2.1 | SecOC_Init 中添加 NvM_ReadBlock + CRC 验证 |
| 2.2.4 | 添加写入频率控制（100 次节流） | — | 1h | [ ] | 2.2.2 | SecOC_IncrementFreshness 中添加计数器 |
| 2.2.5 | 定义 NvM block 配置宏 | — | 0.5h | [ ] | — | SecOC_Cfg.h 中添加 SECOC_FRESHNESS_NVM_BLOCK_ID |
| 2.2.6 | 编写单元测试 | — | 3h | [ ] | 2.2.1-5 | test_secoc_freshness_persistence (4 个用例) |
| 2.2.7 | 全量测试验证 | — | 0.5h | [ ] | 2.2.6 | ctest 通过，新增 4 个用例 |

**P0-2 小计**: 10h

### P0-3: Safety Case 模块评审证据

| # | 任务 | 负责人 | 工时 | 状态 | 依赖 | 证据 |
|:--|:-----|:-------|:-----|:-----|:-----|:-----|
| 2.3.1 | 创建评审记录模板 | — | 1h | [ ] | — | `.osh/evidence/reviews/TEMPLATE.md` |
| 2.3.2 | 填充 Can 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/can_review.md` |
| 2.3.3 | 填充 CanIf 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/canif_review.md` |
| 2.3.4 | 填充 CanSM 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/cansm_review.md` |
| 2.3.5 | 填充 CanTp 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/cantp_review.md` |
| 2.3.6 | 填充 Com 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/com_review.md` |
| 2.3.7 | 填充 PduR 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/pdur_review.md` |
| 2.3.8 | 填充 NvM 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/nvm_review.md` |
| 2.3.9 | 填充 Dcm 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/dcm_review.md` |
| 2.3.10 | 填充 Dem 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/dem_review.md` |
| 2.3.11 | 填充 E2E 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/e2e_review.md` |
| 2.3.12 | 填充 WdgM 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/wdgm_review.md` |
| 2.3.13 | 填充 SecOC 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/secoc_review.md` |
| 2.3.14 | 填充 BswM 评审记录 | — | 0.5h | [ ] | 2.3.1 | `.osh/evidence/reviews/bswm_review.md` |
| 2.3.15 | 更新 safety-case.md [R12] 引用 | — | 0.5h | [ ] | 2.3.2-14 | 确认 [R12] 指向已填充的评审记录 |

**P0-3 小计**: 8h

---

## Phase 3: P1 批次（7 天）

### P1-1: MISRA Error-Severity 违规修复

| # | 任务 | 负责人 | 工时 | 状态 | 依赖 | 证据 |
|:--|:-----|:-------|:-----|:-----|:-----|:-----|
| 3.1.1 | 运行静态分析获取违规列表 | — | 0.5h | [ ] | — | `python tools/analysis/static_analysis.py` 输出 |
| 3.1.2 | 分类违规（真阳性/误报） | — | 2h | [ ] | 3.1.1 | 违规分类清单 |
| 3.1.3 | 修复真阳性违规 | — | 4h | [ ] | 3.1.2 | 代码修改 |
| 3.1.4 | 更新 deviation permit（误报） | — | 1h | [ ] | 3.1.2 | `.yuleosh/ci-config.yaml` 更新 |
| 3.1.5 | 验证违规数清零 | — | 0.5h | [ ] | 3.1.3-4 | `static_analysis.py` errors = 0 |

**P1-1 小计**: 8h

### P1-2: TEST_IGNORE Stub 实现

| # | 任务 | 负责人 | 工时 | 状态 | 依赖 | 证据 |
|:--|:-----|:-------|:-----|:-----|:-----|:-----|
| 3.2.1 | Batch 1: ECUAL 层 30 个 stub | — | 8h | [ ] | — | `tests/unit/autosar/ecual/*.c` |
| 3.2.2 | Batch 2: Services 层 50 个 stub | — | 12h | [ ] | — | `tests/unit/autosar/services/*.c` |
| 3.2.3 | Batch 3: MCAL 层 50 个 stub | — | 12h | [ ] | — | `tests/unit/autosar/mcal/*.c` |
| 3.2.4 | 验证 TEST_IGNORE 数 < 20 | — | 1h | [ ] | 3.2.1-3 | `grep -r "TEST_IGNORE" tests/unit/ | wc -l` < 20 |

**P1-2 小计**: 33h

### P1-3: 覆盖率门禁提升

| # | 任务 | 负责人 | 工时 | 状态 | 依赖 | 证据 |
|:--|:-----|:-------|:-----|:-----|:-----|:-----|
| 3.3.1 | 提升阈值至 60%/50% | — | 0.5h | [ ] | — | `.yuleosh/ci-config.yaml` 更新 |
| 3.3.2 | 验证 CI 通过 | — | 1h | [ ] | 3.3.1 | CI 运行通过 |
| 3.3.3 | 提升阈值至 80%/70% | — | 0.5h | [ ] | 3.3.2, P1-2 | `.yuleosh/ci-config.yaml` 更新 |
| 3.3.4 | 验证 CI 通过 | — | 1h | [ ] | 3.3.3 | CI 运行通过 |

**P1-3 小计**: 3h

### P1-4: CI Job 恢复

| # | 任务 | 负责人 | 工时 | 状态 | 依赖 | 证据 |
|:--|:-----|:-------|:-----|:-----|:-----|:-----|
| 3.4.1 | 编写 test_e2e_dds.c | — | 2h | [ ] | — | `tests/e2e/test_e2e_dds.c` |
| 3.4.2 | 编写 test_e2e_secoc.c | — | 2h | [ ] | — | `tests/e2e/test_e2e_secoc.c` |
| 3.4.3 | 编写 test_e2e_e2e.c | — | 2h | [ ] | — | `tests/e2e/test_e2e_e2e.c` |
| 3.4.4 | 编写 test_e2e_uds.c | — | 2h | [ ] | — | `tests/e2e/test_e2e_uds.c` |
| 3.4.5 | 编写 test_e2e_ota.c | — | 2h | [ ] | — | `tests/e2e/test_e2e_ota.c` |
| 3.4.6 | 编写 test_latency.c | — | 2h | [ ] | — | `tests/performance/test_latency.c` |
| 3.4.7 | 编写 test_throughput.c | — | 2h | [ ] | — | `tests/performance/test_throughput.c` |
| 3.4.8 | 编写 test_memory.c | — | 2h | [ ] | — | `tests/performance/test_memory.c` |
| 3.4.9 | 编写 test_security.c | — | 2h | [ ] | — | `tests/security/test_security.c` |
| 3.4.10 | 更新 CI workflow 移除 guard-artifacts 跳过 | — | 1h | [ ] | 3.4.1-9 | `.github/workflows/ci.yml` 更新 |
| 3.4.11 | 验证 6 个 job 恢复运行 | — | 1h | [ ] | 3.4.10 | CI 运行日志 |

**P1-4 小计**: 20h

### P1-5 ~ P1-13（独立任务）

| # | 任务 | 工时 | 状态 | 证据 |
|:--|:-----|:-----|:-----|:-----|
| 3.5 | P1-5: 创建 S32K312 MCAL 桩文件集（11 个） | 4h | [ ] | `src/bsw/mcal/production/*.c` |
| 3.6 | P1-6: 扩展 bsw_config.json 至 84+ 模块 | 6h | [ ] | `config/bsw_config.json` + Jinja2 模板 |
| 3.7 | P1-8: 90+ header Doxygen 注释骨架 | 8h | [ ] | `find src/bsw -name "*.h" -exec grep -l "@brief" {} \;` >= 90 |
| 3.8 | P1-9: MC/DC 覆盖率测量脚本 | 3h | [ ] | `coverage_run/mcdc_measure.sh` |
| 3.9 | P1-10: 需求追溯矩阵修复 | 2h | [ ] | `trace_requirements.py` PASS |
| 3.10 | P1-11: ISO 21434 Clauses 7/8/11 补充 | 4h | [ ] | `docs/cybersecurity/cybersecurity-case.md` |
| 3.11 | P1-12: DoIP 全链集成测试 | 4h | [ ] | `tests/integration/test_doip_chain.c` |
| 3.12 | P1-13: CAN FD 支持完善 | 4h | [ ] | Can 驱动 FD 配置 + CanIf BRS 处理 |

**P1-5~13 小计**: 39h

---

## Phase 4: P2 批次（3 天）

| # | 任务 | 工时 | 状态 | 证据 |
|:--|:-----|:-----|:-----|:-----|
| 4.1 | P2-1: Com 返回类型统一 | 2h | [ ] | `grep -r "uint8 Com_" src/bsw/services/com/include/` 无匹配 |
| 4.2 | P2-2: Can 返回类型统一 | 2h | [ ] | `grep -r "Can_ReturnType" src/bsw/mcal/can/include/` 无匹配 |
| 4.3 | P2-3: 集成示例代码 | 4h | [ ] | `examples/` 目录 3+ 示例 |
| 4.4 | P2-4: Release 脚本 | 2h | [ ] | `tools/release/create_release.sh` |
| 4.5 | P2-5: Doxyfile + CI job | 2h | [ ] | `Doxyfile` + `.github/workflows/ci.yml` doxygen job |
| 4.6 | P2-6: ROM/RAM 追踪脚本 | 2h | [ ] | `tools/analysis/track_memory.sh` |
| 4.7 | P2-7: 性能基线 JSON | 2h | [ ] | `tools/performance/baseline.json` |
| 4.8 | P2-8: 空 shim 目录清理 | 1h | [ ] | `find src/bsw -type d -empty` 无匹配 |
| 4.9 | P2-9: 文档审计与清理 | 3h | [ ] | `docs/` 无重复/过期文档 |

**P2 小计**: 20h

---

## Phase 5: 验证与归档

| # | 任务 | 工时 | 状态 | 证据 |
|:--|:-----|:-----|:-----|:-----|
| 5.1 | 全量验证（编译+测试+静态分析+覆盖率+追溯） | 4h | [ ] | 5 项验证全部 PASS |
| 5.2 | 更新 acceptance-matrix.md | 2h | [ ] | 47+ SHALL requirements 全部 covered |
| 5.3 | 归档至 openspec/changes/archive/ | 1h | [ ] | `openspec/changes/archive/2026-10-XX-fix-p0-p2-production-remediation/` |

**Phase 5 小计**: 7h

---

## 工时汇总

| Phase | 工时 | 天数 |
|:------|:-----|:-----|
| Phase 1: 需求与设计 | 7h | 1d |
| Phase 2: P0 批次 | 26h | 3d |
| Phase 3: P1 批次 | 103h | 13d |
| Phase 4: P2 批次 | 20h | 3d |
| Phase 5: 验证与归档 | 7h | 1d |
| **总计** | **163h** | **21d** |

> **注**: 原预估 120h (15d) 偏乐观。细化后实际约 163h (21d)。可并行化 P1-5~13 和 P2 批次以压缩至 15d。

---

## 关键里程碑

| 里程碑 | 目标日期 | 状态 |
|:-------|:---------|:-----|
| M1: 需求文档完成 | 2026-10-08 | [x] |
| M2: 设计文档完成 | 2026-10-08 | [x] |
| M3: P0 批次完成 | 2026-10-11 | [ ] |
| M4: P1 批次完成 | 2026-10-24 | [ ] |
| M5: P2 批次完成 | 2026-10-27 | [ ] |
| M6: 验证与归档完成 | 2026-10-28 | [ ] |

---

## 风险登记

| ID | 风险 | 概率 | 影响 | 缓解措施 | 状态 |
|:---|:-----|:-----|:-----|:---------|:-----|
| R1 | P0-1 模块收敛破坏现有测试 | 中 | 高 | 保留 CMake target，仅替换 .c 内容 | [ ] 监控 |
| R2 | P0-2 SecOC NvM 循环依赖 | 低 | 高 | 通过 API 接口调用，不直接 #include | [ ] 监控 |
| R3 | P1-2 TEST_IGNORE 实现超工作量 | 高 | 中 | 分批实施，优先核心模块 | [ ] 已识别 |
| R4 | P1-3 覆盖率提升导致 CI 失败 | 中 | 中 | 分两阶段提升 | [ ] 监控 |
