# 主流开源 AUTOSAR 对标复查（2026-09-28）

> **前置状态**：P0 批次（fix-bsw-production-gaps）已全部落地并归档（2026-09-27）；上次对标见 `bsw-depth-analysis-2026-09-26.md`（深度）与 `gap-matrix-2026-09-26.md`（目录矩阵 52/52）。
> **本次口径**：① 主流开源项目全景刷新；② P0 后本地基线重扫；③ P0 五模块对 EasyXMen 头文件符号面复算。目录级覆盖沿用 09-26 结论（52/52），P0 删除项（classic/com、legacy、废弃测试树）均系我方多出物/债务，不影响覆盖结论。
> **判定纪律**：头文件符号面 ≠ 功能符合度；基线 .c 未下载，不评实现质量；MCAL 缺失与许可证差异在结论中提示（技能口径）。

---

## 0. 一句话结论

P0 五项落地后：**BswM/LinIf 从"显著差距（最小实现）"收敛为"引擎主干对齐"，CAN 栈差距面收窄且新增 API 与 EasyXMen 命名对齐**；**P1 代码可落地项已于当日全部关闭（change fix-bsw-p1-benchmark-gaps，106/106 回归）**，剩余 `theirs_only` 以内部助手与需求驱动面为主（CanSM 符号面归零）。在主流开源全景中，**"全栈 BSW + 全自研 MCAL + 成体系测试"的组合仍仅我方具备**——活跃基线 EasyXMen 缺 MCAL 与公开测试，OpenBSW 明确放弃 AUTOSAR 符合性，openAUTOSAR/Trampoline 已停滞或仅覆盖 OS。

---

## 1. 主流开源项目全景（2026-09-28 核实）

| 项目 | 许可证 | 最新版本 | 最近活跃 | AUTOSAR 定位 | MCAL | 配置工具 | 测试资产 |
|:-----|:-------|:---------|:---------|:-------------|:-----|:---------|:---------|
| **EasyXMen（普华）** | LGPL-2.1＋链接例外 | V25.10（2025-10-24）＋V2510_patch（2026-04-30） | 2026-04-30 | CP 全栈（57 模块，中国规范口径） | **不开源**（Drivers/ 空，随芯片 Demo 分发） | 注册制图形配置器（BswCfg/SwcCfg/McalCfg） | Test/UT 目录为空（随工具分发，**未核实**） |
| **Eclipse OpenBSW**（Accenture/EB/BMW） | Apache-2.0 | tag v0.1.0；CHANGELOG 0.2.0 | **2026-09-24（非常活跃）** | **非 AUTOSAR 实现**（README 0 处 AUTOSAR；code-first C++ BSW） | 无（自有 bsp） | 开源 bazel/cmake | googletest＋pytest HIL＋覆盖率 CI |
| **openAUTOSAR/classic-platform** | GPL-2.0（**禁商用**） | tag v2.18.0（2014-11-28） | master pushed 2024-08-06 | Arctic Core 分叉（AUTOSAR 3.1/4.0 时代） | 部分通用驱动（Eep/Fls/Wdg/Fr 等） | 无现代配置器（tools/ 仅调试脚本） | testCommon（有限） |
| **Arctic Core / ArcCore** | GPL-2.0 | — | 官方 GitHub 组织已消失，官网 522 | 谱系存续于 openAUTOSAR 与镜像 | — | — | — |
| **Trampoline** | GPL-2.0 | 无 release；master 2025-09-12 | 2025-09 | **仅 OS**：AUTOSAR OS 4.2 API 对齐（SC2–SC4 特性，未声明认证） | — | goil（OIL/ARXML） | tests/ |
| 边缘注记：ERIKA3 | GPLv2＋例外 | — | 活跃 | 现为 OSEK 认证 RTOS，AUTOSAR OS 化自述"未来方向" | — | RT-Druid | — |

**三点结论性提示**：
1. 真正"活"且活跃维护的仅 **EasyXMen 与 OpenBSW**，路线相反：前者 AUTOSAR 协议栈＋封闭配置工具，后者开源工具链＋明确不做 AUTOSAR 符合。
2. Arctic Core 官方渠道已消失，采样对象须以 openAUTOSAR/classic-platform 为准并注明其 tag 停留 2014 年。
3. 开源 CP 领域**不存在"开源配置器＋完整 MCAL＋完整 BSW"的全栈组合**——MCAL 与配置工具是各项目共同短板（OpenBSW 以放弃 AUTOSAR 符合性换取工程现代化）。

---

## 2. 本地基线刷新（P0 后，2026-09-28 重扫）

| 层 | 09-26 LOC | 09-28 LOC | 变化说明 |
|:---|----------:|----------:|:---------|
| mcal | 47.8k | 46.4k | crypto legacy 4 文件删除 |
| ecual | 37.6k | 38.2k | LinIf/CanIf/CanTp/CanTrcv 加厚（+），classic 移出后净增 |
| services | 116.0k | 85.0k | **P0-5 删 legacy 75 文件**：Dcm 22.7k→4.1k、Dem 12.6k→5.1k 为剔除 legacy 死代码后的活跃面 |
| os | 3.5k | 3.5k | — |
| cdd | 5.1k | 5.1k | — |
| classic | 6.3k | 0 | P0-3 双 Com 收敛（src/bsw/classic/com 删除） |

- 测试：`ctest --test-dir build-native` **104/104 零失败**（09-28 复核）；`tests/unit` 证死子集已清理。
- 口径说明：新旧值同为 `scan-module-metrics.js` 脚本口径（.c+.h 行数；services 旧值含 vendored mbedtls）。

---

## 3. P0 模块符号面复算（vs EasyXMen，头文件面）

| 模块 | 09-26 我方 | 09-28 我方 | EMX独/我独（09-26 → 09-28） | 判定变化 |
|:-----|:-----------|:-----------|:----------------------------|:---------|
| BswM | 292 LOC / 5 API | 650 LOC | 115/4 → 113/5 → **98/5** | **显著差距 → 主干对齐 → P1 联动面关闭**（15 联动回调 + 端口 3→16） |
| LinIf | 361 LOC / 9 API | 741 LOC | 133/4 → 132/8 → **122/8** | **显著差距 → 主干对齐 → P1 Trcv/NAD/PID 关闭**（Master/Slave 细分与 LinTp 转 P2） |
| CanIf | 1.4k | 1.5k | 76/2 → 74/2 → **59/2** | 中等-显著 → 中等 → 收窄（错误面/PN/Trcv/NotifStatus/TriggerTransmit/MetaData 落地；遗留 ReadRxPduData 转 P2） |
| CanSM | 0.9k | 1.2k | 6/6 → 7/2 → **0/2** | 相当-略缺 → **符号面归零**（唤醒源/被动网络/TxTimeout/Trcv 指示落地） |
| CanTp | 1.5k | 1.6k | 35/1 → 35/1 → **35/1** | 中等（Rx 队列/MetaData/统一 DL 校验落地；剩余全为内部助手/宏，公开面已对齐） |
| MCAL Can | 968 LOC | 968 LOC | EMX 无 MCAL 开源 | 开源无对标物，**我方独有资产** |

剩余 `theirs_only` 归类（区分"公开 API 缺口"与"实现风格差异"，技能口径）：

- **内部助手（非缺口）**：`BswM_DetChk*`/`CanIf_Validate*`/`CanTp_MemCpy`/各类索引器——EMX 的 DET 守卫与内部工具函数。
- **扩展联动面（P1 已关闭，2026-09-28）**：
  - BswM：ComM/Dcm/Nm/CanSM/EthSM/FrSM/LinSM/LinTp 联动回调族 + 分区端口已落地；SD/J1939/SoAd/Swc 回调按需求定档（维持排除）；
  - LinIf：Trcv 控制 + NAD/PID 运行时面已落地；Master/Slave 细分、LinTp 整模块转 P2；
  - CanIf：PN、Trcv 唤醒标志、MetaData 转换、通知状态读取族已落地；遗留 ReadRxPduData 转 P2；
  - CanSM：唤醒源、被动网络、TxTimeout 已落地，符号面归零；
  - CanTp：Rx 队列、MetaData 收发、FF/SF-DL 校验已落地，剩余为内部助手。

---

## 4. 维度矩阵（我方 vs 主流）

| 维度 | yuleASR（本仓） | EasyXMen | OpenBSW | openAUTOSAR | Trampoline |
|:-----|:-----|:---------|:--------|:------------|:-----------|
| BSW 广度 | 52/52 目录（R22-11 口径声明） | 57 模块（CP 栈） | 组件族（非 AUTOSAR 命名） | 4.0 时代全套 | 无（仅 OS） |
| MCAL | **21 模块全自研 ≈46k LOC** | 不开源 | 无（自有 bsp） | 部分通用驱动 | — |
| 配置工具 | ConfigGenerator 模板（MCAL 5＋BSW 5） | 注册制图形配置器 | 开源 bazel/cmake | 无现代配置器 | goil |
| 测试 | **106 ctest＋Unity 体系** | UT 空（随工具分发） | gtest＋pytest HIL＋覆盖率 CI | testCommon（有限） | tests/ |
| 芯片目标 | S32K312 / i.MX8M Mini | TC397 / S32K148 / RH850 U2A16 | POSIX / S32K148 / STM32 | 20+ 老板卡（MPC5xx 等） | 多架构（无 TriCore） |
| 许可证 | 自有 | LGPL-2.1＋例外（量产静态链接需合规评估） | Apache-2.0 | **GPL-2.0（禁商用）** | GPL-2.0 |
| 活跃度 | 活跃（本变更 25/25） | 中（2026-04） | 高（2026-09） | 停滞（tag 2014） | 低（2025-09） |

---

## 5. 差距清单更新（承接 09-26 分级）

- **P0：全部关闭**（fix-bsw-production-gaps 25/25 归档，104/104 回归）。
- **P1：代码可落地项已关闭**（change `fix-bsw-p1-benchmark-gaps`，21/21 任务，全量回归 106/106 通过，2026-09-28）：
  1. ~~BswM 扩展联动面~~ **已落地**：ComM/Dcm/Nm/CanSM/EthSM/FrSM/LinSM/LinTp 等 15 个联动回调 + 端口表 3→16（原始模块状态经 `BswM_WritePortByComposition` 直写端口，绕过 0..7 模式域校验）；剩余 theirs_only 98 均为 DetChk*/Init*RequestPorts 内部助手与 SD/J1939/SoAd/Swc 排除项（`BswM_DeInit` 为 DeInit/Deinit 命名变体，已实现并声明）。
  2. ~~LinIf 收发器/NAD/PID~~ **已落地**：Trcv 族 5 API 委托 LinTrcv + NAD/PID 运行时 4 API + IsSupportTpTransmit；Master/Slave 细分与 LinTp 整模块仍为缺口（多 LIN 子节点量产场景触发，转 P2 观察）。
  3. ~~CanIf PN/Trcv 唤醒标志/MetaData/通知状态族 + CanSM 唤醒源/被动网络~~ **已落地**：CanIf 15 API（含 mcal/Can 错误面 3 getter 前置）；CanSM 7 API；CanSM theirs_only 归零。遗留 1 个公开 API `CanIf_ReadRxPduData`（转 P2）。
  4. ~~CanTp Rx 队列与 MetaData~~ **已落地**：Rx 环形队列深 4（ISR 安全）+ MIXED/NORMALFIXED MetaData + FF/SF-DL 统一校验；剩余 theirs_only 35 全为内部助手/宏。
  5. **维持定档**：Dem 按 OEM 诊断规范定档、Crypto/Csm AEAD 服务族、ComM/StbM/Nm 增强（09-26 已列，需求驱动非对标驱动）。
- **P2（新增自 P1 降级/遗留）**：CanIf_ReadRxPduData、LinIf Master/Slave 细分、LinTp 模块、Dem/Crypto/ComM/StbM/Nm 定档类；另维持：跨层重复模块收敛、tests/unit 存量树（tools/docs 引用）、MISRA 存量、命名/版本宏统一。
- **结构性（维持）**：OS 依赖 vendored FreeRTOS V11.1.0 非安全认证版，功能安全场景需另行评估。

## 6. 我方领先项（复查确认维持）

- **MCAL 全自研 21 模块 ≈46k LOC**：所有开源基线均不开放 MCAL，核心资产不变；
- **测试资产**：104 ctest 全通过（EMX 开源仓 UT 为空、OpenBSW 有 gtest 但非 AUTOSAR 面）；
- **Dcm/NvM/存储栈**深度、安全 CDD（Hsm/Lockstep/RamEcc）、**广度**（J1939/FlexRay/SOME-IP/MQTT/DoIP/XCP）均维持 09-26 结论。

---

## 7. 数据与复现

- 原始数据（本目录）：`local-metrics-2026-09-28.txt`（本地分层扫描）、`api-symbol-diff-p0-2026-09-28.txt`（5 模块符号面全量）；EMX 44 个头文件当时存于 `/tmp/emx_hdrs_20260928/`（Gitee 源，GitHub API 当日限流）。
- 复现：
  ```bash
  for L in mcal ecual services os cdd; do node .qoder/skills/autosar-benchmark-compare/scripts/scan-module-metrics.js src/bsw/$L; done
  curl -sL "https://gitee.com/api/v5/repos/easyxmen/XMen/git/trees/master?recursive=1" -o /tmp/emx_tree_gitee.json
  node .qoder/skills/autosar-benchmark-compare/scripts/api-surface-diff.js <我方模块目录> /tmp/emx_hdrs_20260928/<EMX模块>
  ```
- 主流项目事实来源（2026-09-28 获取）：GitHub/Gitee/GitCode REST API、各仓 README/LICENSE/NOTICE/EXCEPTION/CHANGELOG、Eclipse OpenBSW 文档站、Trampoline wiki、ERIKA 官网；未核实项见调研汇总（EasyXMen UT 实际分发内容、OpenBSW STM32 型号、Trampoline SC 认证声明、ERIKA3 仓址）。
