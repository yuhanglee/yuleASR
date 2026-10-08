# yuleASR — Safety Case 证据文档核查报告

> **文档**: Evidence Verification (证据文档核查) — Safety Case 引用完整性审计
> **版本**: 1.0 | **日期**: 2026-10-07
> **核查对象**: `docs/safety/safety-case.md` §0.2 参考文档 (R01-R14, 共 14 项)
> **核查方法**: 逐项路径存在性检查 + 章节级抽查 + 数据一致性交叉核对
> **结论**: 14/14 文档路径存在 ✅；发现 **5 项内容级缺口** → 生成 TODO 记录 (见 §3)

---

## 1 核查结果总览

| 编号 | 文档 | 路径 | 存在 | 规模 | 备注 |
|:----:|------|------|:----:|:----:|------|
| [R01] | HARA 分析 | docs/safety/HARA_ANALYSIS.md | ✅ | 88 行 | 安全引用正常 |
| [R02] | 安全架构 (ASIL B + FMEDA) | docs/safety/safety-architecture.md | ✅ | 441 行 | §2 ASIL 分解 / §3 Safe State / §4 FMEDA / §5 安全机制矩阵 均存在 |
| [R03] | DFA 分析 | docs/safety/dfa-analysis.md | ✅ | 390 行 | §5 FFI 评估 / §6 安全偏差 / §7 CCF 分析 均存在; D-FFI 偏差 8 处引用一致 |
| [R04] | FMEA/FTA 量化分析 | docs/safety/fmea-fta-quantitative-analysis.md | ✅ | 609 行 | 存在; 定量结论背书问题见 TODO-04 |
| [R05] | MPU 分区方案 | docs/safety/mpu-partition.md | ✅ | 256 行 | 正常 |
| [R06] | 验证报告 | docs/safety/VERIFICATION_REPORT.md | ✅ | 75 行 | 批E (2026-08-07) 重写版; 与 Safety Case 数字不一致见 TODO-03 |
| [R07] | 安全手册 | docs/safety/SAFETY_MANUAL.md | ✅ | 44 行 | 正常 |
| [R08] | ASIL 分解报告 | docs/safety/asil-decomposition-report.md | ✅ | 529 行 | 正常 |
| [R09] | MISRA 合规报告 | docs/misra_compliance_report.md | ✅ | 303 行 | 正常 |
| [R10] | MISRA 偏差报告 | docs/misra_deviations.md | ✅ | 326 行 | 正常 |
| [R11] | 需求追溯矩阵 | docs/requirement-traceability-matrix.md | ✅ | 26 行 | 仅 17 条 SHALL 行, 内容滞后见 TODO-02 |
| [R12] | 模块审查证据 | .osh/evidence/ | ⚠️ | 目录存在 | `reviews/` 子目录为空, 见 TODO-01 |
| [R13] | CI 层报告 | .yuleosh/reports/ | ✅ | 23+ 文件 | L1/L2/L3 报告齐全 (json/md/xlsx) |
| [R14] | 硬件安全分析 | docs/safety/safety-architecture.md §4 | ✅ | 同 R02 | §4 FMEDA 精细分析 (v1.4.0) 存在, 含 PMHF |

**路径级结论**: 14/14 存在 → Safety Case §0.2 证据索引无死链。

---

## 2 章节级抽查 (Safety Case GSN 引用 → 实际章节)

| Safety Case 断言 | 引用 | 实际核查 | 结果 |
|:-----------------|:----:|:---------|:----:|
| Sn-12~14 SPFM/LFM/PMHF (R02 §4.3-4.5) | [R02] | safety-architecture.md §4 FMEDA 精细分析 (L150) | ✅ |
| Sn-19~21 Safe State/FTTI (R02 §3) | [R02] | §3 Safe State 定义 (L104) | ✅ |
| C-05 安全机制矩阵 12 项 (R02 §5) | [R02] | §5 安全机制矩阵 (L313) | ✅ |
| Sn-16~18 CCF/FFI (R03 §5-§7) | [R03] | §5 FFI (L206) / §6 偏差 (L267) / §7 CCF (L314) | ✅ |
| D-FFI-001/002 偏差 (R03 §6) | [R03] | §6 安全偏差, D-FFI 关键字 8 处 | ✅ |
| R14 = R02 §4 FMEDA + PMHF | [R02] | §4 FMEDA 精细分析 (v1.4.0) (L150) | ✅ |
| Sn-22~24 覆盖率 (R06) | [R06] | §1 实测数据存在, 但数字不一致 | ⚠️ TODO-03 |
| Sn-26 MISRA Required=0 (R09) | [R09] | misra_compliance_report.md 存在 | ✅ |
| Sn-30 追溯矩阵 (R11) | [R11] | 存在但仅 17 行, 与断言规模不符 | ⚠️ TODO-02 |
| Sn-29 模块审查证据 (R12) | [R12] | review-log.json 17 条为过程级审查, 非模块级 | ⚠️ TODO-01 |

---

## 3 TODO 记录 (内容级缺口)

> 以下缺口不影响"文档存在性", 但影响 Safety Case 论据的数据可信度。
> 按 ISO 26262 审核视角排序 (P0 = 审核必问, P1 = 应修复, P2 = 建议改进)。

### TODO-01 [P0] R12 模块级审查证据缺失

- **现象**: Safety Case Sn-29 声称 "11 模块审查证据: E2E/WdgM/NvM/Can/Com/Dcm/
  Lin/SecOC/CryIf/RamSafety/Dem/Det/EcuM"; §0.2 [R12] 指向 `.osh/evidence/`。
- **核查结果**: `.osh/evidence/reviews/` 子目录**为空** (0 文件)。
  `review-log.json` 有 17 条记录, 但均为**过程级**审查 (AUTOSAR专家终审-v1.3.0、
  复审-小克/小马、综合修复计划-v2.0 等), 不含上述 13 个模块的逐模块审查文件。
- **影响**: Sn-29 论据 (G-3 → S-3.3 证据链完整) 证据不足; OEM 审核抽查模块
  审查记录时将无法提供。
- **TODO**:
  - [ ] 补充/恢复 13 个模块 (E2E/WdgM/NvM/Can/Com/Dcm/Lin/SecOC/CryIf/
    RamSafety/Dem/Det/EcuM) 的审查记录至 `.osh/evidence/reviews/`;
  - [ ] 或修订 Safety Case Sn-29, 将 [R12] 引用改为 `review-log.json` 实际
    覆盖的过程级审查并如实描述范围;
  - [ ] 在 GSN 覆盖矩阵 (§7.1) 中将 S-3.3 的状态降级为 ⚠️ 直到证据补齐。

### TODO-02 [P1] R11 需求追溯矩阵内容滞后

- **现象**: Safety Case Sn-30 声称 "需求追溯矩阵: 127 SHALL → 127 测试,
  138 mapping 全部一致"; §0.2 [R11] 指向 `docs/requirement-traceability-matrix.md`。
- **核查结果**: 该文件仅 **26 行 / 17 条 SHALL→测试行**。当前权威数据源为
  `.osh/evidence/traceability-matrix.json` (47 条需求) 与
  `.yuleosh/audit/acceptance-matrix.md` (v0.2.0, 47/47 关联)。
  "127 SHALL" 的口径出处无法在本仓库复核。
- **影响**: S-3.3 论据的数字与被引文档不符 → 数据可信度问题 (审查记录中
  专家已指出类似风险: "无法被独立复现的数字比缺陷本身更危险")。
- **TODO**:
  - [ ] 以 `.yuleosh/audit/acceptance-matrix.md` v0.2.0 为准重生成
    `docs/requirement-traceability-matrix.md` (47 需求);
  - [ ] 修订 Safety Case Sn-30 的数字口径 (47 或核实 127 的来源);
  - [ ] 纳入 `tools/traceability/trace_requirements.py` 自动校验, 防止再漂移。

### TODO-03 [P0] R06 覆盖率数字与 Safety Case 断言不一致

- **现象**: Safety Case (2026-07-30) 引用 [R06] 声称:
  - Sn-22 语句覆盖率 96.2%
  - Sn-23 MC/DC 覆盖率 E2E 90% / OS Timing 88% / NVM 89%
  - Sn-24 函数覆盖率 95.7% (268/280 函数)
  - J-2 "分支覆盖当前 0.0% (GCov 限制)"
- **核查结果**: `docs/safety/VERIFICATION_REPORT.md` 批E 重写版 (2026-08-07,
  晚于 Safety Case) 实测数据为:
  - 语句 **91.57%** (2638/2881) ≠ 96.2%
  - 分支 **79.79%** (995/1247) ≠ 0.0% (批E 已用 lcov BRDA 实测)
  - 函数 **96.48%** (192/199) ≠ 95.7% (268/280)
  - **MC/DC: 未测量, 无数据** — 与 Sn-23 的 88-90% 断言直接冲突
  - 数据源 `.yuleosh/reports/c-coverage.json` (line_rate=91.57, branch_rate=79.79)
    与 `.yuleosh/reports/branch-coverage-report.md` 一致
- **影响**: Sn-22/23/24 与 J-2 四处论据基于过期数据; MC/DC 88-90% 的声明
  在被引文档中被明确否认 ("无数据, 如实标注")。这属于 Safety Case 完整性
  的实质性缺口。
- **TODO**:
  - [ ] 以批E 实测数据重写 Safety Case Sn-22/23/24 与 J-2:
    语句 91.57%、分支 79.79%、函数 96.48%、MC/DC 未测量 (诚实标注);
  - [ ] J-2 中 "MC/DC > 90% 边界达标" 的达标声明撤回, 改为缺口声明
    (需 VectorCAST/gcov 扩展或 MC/DC 手动枚举矩阵作为替代证据);
  - [ ] SWE.4.BP1 门禁结论保留: 语句 ≥80% ✅ / 分支 ≥70% ✅ (双轴达标);
  - [ ] 若存在 96.2%/MC/DC 88-90% 的独立测量来源, 补充其出处与复现命令。

### TODO-04 [P1] R04 FTA 定量结论缺乏独立背书

- **现象**: Safety Case Sn-15 引用 [R04] 声称 P(SG-001)=2.72×10⁻¹⁶/h 等定量结论;
  §9.2 定量底线表同样引用。
- **核查结果**: `docs/safety/VERIFICATION_REPORT.md` §2 明确声明
  "FTA 顶层事件概率 < 10⁻⁸/h 的定量结论**本报告不背书** — 该数字的计算输入
  (失效率库、共因因子) 需独立评审证据; 当前无独立评审记录"。
- **影响**: G-2 → S-2.2 论据的定量部分依赖未被内部验证报告背书的数据。
- **TODO**:
  - [ ] 为 [R04] 的失效率库 (NXP AN13475 / IEC TR 62380) 与共因因子建立
    独立评审记录 (专家签核);
  - [ ] 或在 Safety Case Sn-15/§9.2 处补充限定语 "定量结论待独立评审"。

### TODO-05 [P2] .osh/evidence/code-coverage-report.md 为空壳

- **现象**: `.osh/evidence/code-coverage-report.md` 内容为
  "No coverage data available — run CI Layer 1 first."
- **影响**: 若审核员抽查该文件会得到"无数据"印象, 与实际存在的批E 实测数据
  (`.yuleosh/reports/`) 形成反差; R12 证据目录内含空壳报告削弱整体可信度。
- **TODO**:
  - [ ] 重新运行 CI Layer 1 或将 `.yuleosh/reports/c-coverage.json` 的实测
    数据同步至该文件;
  - [ ] 验收矩阵 v0.2.0 中 NFR-SHALL-002 证据已改指
    `.yuleosh/reports/c-coverage.json` (有实测数据), 不受此空壳影响。

---

## 4 核查方法与复现

```bash
# 1. 路径存在性 (14 项)
ls docs/safety/HARA_ANALYSIS.md docs/safety/safety-architecture.md \
   docs/safety/dfa-analysis.md docs/safety/fmea-fta-quantitative-analysis.md \
   docs/safety/mpu-partition.md docs/safety/VERIFICATION_REPORT.md \
   docs/safety/SAFETY_MANUAL.md docs/safety/asil-decomposition-report.md \
   docs/misra_compliance_report.md docs/misra_deviations.md \
   docs/requirement-traceability-matrix.md .yuleosh/reports/

# 2. R12 审查目录
ls -la .osh/evidence/reviews/          # → 空目录 (TODO-01)

# 3. R06 覆盖率数据交叉核对
python3 -c "import json; print(json.load(open('.yuleosh/reports/c-coverage.json'))['line_rate'])"
# → 91.57 (与 VERIFICATION_REPORT.md §1 批E 一致, 与 Safety Case 96.2% 不一致)

# 4. 需求追溯自动化复核 (见 TODO-02)
python3 tools/traceability/trace_requirements.py
```

---

## 5 结论

1. **路径完整性**: Safety Case §0.2 的 14 项证据引用 **无死链** (14/14 存在)。
2. **内容完整性**: 存在 5 项内容级缺口 (TODO-01~05), 其中 2 项 P0
   (模块审查证据缺失、覆盖率数字不一致) 直接影响 G-3 论据可信度,
   建议在下一版 Safety Case (v1.1) 中修复。
3. **交叉改进**: 验收矩阵 (`.yuleosh/audit/acceptance-matrix.md` v0.2.0) 已将
   NFR 覆盖率证据指向有实测数据的 `.yuleosh/reports/` 工件, 并提供
   `tools/traceability/trace_requirements.py` 自动化复核能力。

---

*— Phase 7 安全合规证据 (Task 8) 产出物*
*2026-10-07*
