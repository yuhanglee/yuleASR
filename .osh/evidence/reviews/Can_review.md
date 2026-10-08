# Module Review Record — Can

> **说明**: 本文件由工具自动生成，待人工评审确认。

---

## 基本信息

| 字段 | 值 |
|------|-----|
| **模块名称** | `Can` |
| **ASIL 等级** | ASIL-B |
| **评审日期** | 2026-10-08 |
| **评审人** | Auto-generated — pending human review |
| **模块版本** | v1.0.0 (AR 4.4.0) |
| **源码路径** | `src/bsw/mcal/can/` |

## 评审范围

### 代码审查

- [ ] 功能完整性（对照 SWS 规范）
- [ ] MISRA C:2012 合规性
- [ ] 圈复杂度 ≤ 10
- [ ] 无未初始化变量、空指针解引用
- [ ] 错误处理路径完整

### 测试覆盖

- [ ] 单元测试覆盖率 ≥ 80%
- [ ] 边界条件测试
- [ ] 错误注入测试
- [ ] 集成测试通过

### 配置与追溯

- [ ] 配置参数与 bsw_config.json 一致
- [ ] 需求追溯矩阵完整
- [ ] 偏差许可记录完整

## 评审结果

### 代码审查结果

| 指标 | 值 | 备注 |
|------|-----|------|
| MISRA 违规数 | 0 (pending scan) | 待执行 cppcheck/MISRA 扫描 |
| 圈复杂度 (max) | ≤ 10 (pending scan) | 待执行 complexity 分析 |
| 代码行数 (LOC) | 1104 | .c: 711, .h: 393 |
| 函数数量 | — (pending scan) | 待统计 |

### 测试覆盖结果

| 指标 | 值 | 备注 |
|------|-----|------|
| 行覆盖率 | ≥ 80% (pending gcov run) | 测试文件: tests/bsw/mcal/can/ |
| 分支覆盖率 | ≥ 70% (pending gcov run) | gcov 数据 |
| MC/DC 覆盖率 | N/A | 或提供测量数据 |
| 测试用例数 | 1 (pending count) | 待统计 |

### 遗留问题

| ID | 描述 | 严重度 | 状态 | 计划解决日期 |
|----|------|--------|------|-------------|
| OOS-1 | 覆盖率数据待填充 | Medium | Open | 2026-10-15 |
| OOS-2 | MISRA 扫描待执行 | Medium | Open | 2026-10-15 |

## 评审结论

- [ ] **通过** — 模块满足 ASIL-B 要求，可纳入安全基线
- [x] **有条件通过** — 需在指定日期前解决遗留问题
- [ ] **不通过** — 需重新评审

**结论说明**: 有条件通过 — 需填充实际覆盖率数据

## 签名

| 角色 | 姓名 | 日期 | 签名 |
|------|------|------|------|
| 评审人 | Auto-generated — pending human review | 2026-10-08 | — |
| 安全经理 | | | |
| 质量经理 | | | |

---

**关联文档**:
- [Safety Case](../../safety-case.md) — GSN Goal 引用
- [MISRA Deviation Permits](../misra-deviations.md)
- [Test Coverage Report](../coverage/)
