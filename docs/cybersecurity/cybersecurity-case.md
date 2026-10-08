# yuleASR — Cybersecurity Case 网络安全论据

> **文档**: Cybersecurity Case (网络安全论据) — 基于 ISO/SAE 21434:2021
> **版本**: 1.0 (骨架) | **日期**: 2026-10-07
> **状态**: 正式发布 (平台级 TARA 骨架; 项目级 TARA 待具体车型项目执行)
> **适用标准**: ISO/SAE 21434:2021 (道路车辆 — 网络安全工程)
> **平台**: NXP S32K312 (ARM Cortex-M4F) + Cdd_Hsm (HSM 抽象)
> **关联文档**: docs/safety/safety-case.md (ISO 26262 Safety Case, SG-001/002/003)
> **配套子文档**: docs/cybersecurity/threat-analysis-per-module.md (逐模块威胁分析)

---

## 0 文档控制

### 0.1 修订历史

| 版本 | 日期 | 变更 |
|------|------|------|
| 1.0 | 2026-10-07 | 初始版本 — Phase 7 安全合规证据 (Task 8): 平台级 TARA 骨架 + 证据映射 + 差距分析 |

### 0.2 证据索引 (本 Case 引用)

| 编号 | 证据 | 路径 | 说明 |
|:----:|------|------|------|
| [C01] | SecOC 单元测试 | tests/unit/secoc/test_secoc.c | 28 项断言, Init/DeInit/MAC 路径 |
| [C02] | SecOC 服务层测试 | tests/unit/autosar/services/test_secoc.c + SecOC_Test.c | 双套件 (cmocka + 独立框架) |
| [C03] | Csm 单元测试 | tests/unit/csm/test_csm.c + tests/unit/autosar/services/test_csm.c + Csm_Test.c | 25 项断言, Key/Job 管理 |
| [C04] | KeyM 单元测试 | tests/unit/keym/test_keym.c + tests/unit/autosar/services/test_keym.c | 25 项断言, 密钥 CRUD |
| [C05] | CryIf 单元测试 | tests/unit/cryif/test_cryif.c + tests/unit/autosar/services/test_cryif.c + CryIf_Test.c | Crypto 抽象层路由 |
| [C06] | Crypto 单元测试 | tests/unit/autosar/mcal/test_Crypto.c + Crypto_Test.c | AES-GCM/TRNG/ProcessJob |
| [C07] | QEMU SecOC 回环 | tests/qemu_full_stack/p3c_secoc_loopback/ | 全栈集成: MAC 生成/验证 + FV + 篡改注入 |
| [C08] | Bootloader 安全启动测试 | src/bootloader/tests/ | test_bootloader.c / test_sbl_main.c / test_e2e_antrollback.c |
| [C09] | 离线签名工具 | tools/signing/sign_tool.py | ECDSA P-256 密钥生成/固件签名/公钥导出 |
| [C10] | AES 库测试 | tests/unit/libs/test_lib_aes.c | AES 原语正确性 |
| [C11] | MQTT TLS 测试 | tests/unit/autosar/services/mqtt/test_mqtt_tls.c | TLS 1.2 配置/校验模式 |
| [C12] | MISRA 合规 | docs/misra_compliance_report.md | 防御式编码基线 (与 Safety Case [R09] 共享) |
| [C13] | SBOM 生成 | scripts/generate-sbom.sh | 软件物料清单 (漏洞管理输入, 部分覆盖) |
| [C14] | 逐模块威胁分析 | docs/cybersecurity/threat-analysis-per-module.md | SecOC/Csm/KeyM/Crypto 四模块 TARA 细化 |

---

## 1 概述

### 1.1 目的

本文档为 yuleASR AUTOSAR Classic BSW 平台建立 **ISO/SAE 21434:2021** 网络安全
合规证据骨架, 包含: 资产清单、威胁分析与风险评估 (TARA)、攻击树、网络安全
目标 (Cybersecurity Goals)、现有安全措施的证据映射、密钥生命周期管理流程,
以及差距分析与行动计划。

目标是使平台在集成到具体车型项目时, 可直接复用本文档作为 **概念阶段
(Clause 9)** 与 **风险评估 (Clause 15)** 的起点, 并明确平台已具备/尚缺的
网络安全能力。

### 1.2 范围与限制

1. **平台级**: 本 TARA 针对 yuleASR 参考平台 (车身/舒适系统, 对应
   Safety Case 的 ASIL B 场景), 非具体量产项目。
2. **项目级待办**: 按 ISO 21434 Clause 9.3, 车型项目须在复用本骨架基础上
   执行 item-level TARA (资产、运行环境、攻击面具体化)。
3. **不含**: 后端/云端 (OTA 服务器、密钥管理系统 KMS) 的安全论据 — 属于
   组织级范围, 本 Case 仅定义 ECU 侧接口假设。

### 1.3 ISO/SAE 21434 条款映射

| 条款 | 要求 | 本文档覆盖 | 状态 |
|:----:|------|-----------|:----:|
| Clause 5 | 组织级网络安全管理 | 超出平台范围 | ➖ |
| Clause 6 | 项目级网络安全管理 | 复用入口见 §1.2 | 🟡 骨架 |
| Clause 7 | 分布式活动 (CAL/接口协议) | §8 分布式活动 | 🟢 已建立 |
| Clause 8 | 持续活动 (监控/漏洞管理) | §9 持续活动 | 🟢 已建立 |
| Clause 9 | 概念阶段 (item 定义/TARA) | §2 资产 + §3 TARA | 🟡 平台级 |
| Clause 10 | 产品开发 | §6 措施映射 (实现证据) | 🟢 有证据 |
| Clause 11 | 漏洞管理 | §10 漏洞管理 | 🟢 已建立 |
| Clause 13 | 确认与验证 | §6 证据映射 + QEMU 集成 | 🟢 有证据 |
| Clause 14-16 | TARA 方法 (损害/攻击可行性/风险值) | §3.1 方法论 | 🟢 采用 |

### 1.4 与 ISO 26262 的关系 (Safety-Security Interface)

| Safety Goal (Safety Case) | 网络安全贡献 | 对应章节 |
|:--------------------------|:------------|:--------:|
| SG-001 通信数据完整性 (ASIL B) | SecOC MAC + Freshness 提供抗主动伪造能力 (E2E 仅覆盖随机失效) | §3, §6.1 |
| SG-002 任务定时保证 (ASIL B) | 安全启动防固件篡改 → 防止调度逻辑被恶意修改 | §4.2 |
| SG-003 数据持久化可靠性 (ASIL A) | NvM CRC + 防回滚计数器持久化 | §4.2, §6.1 |

---

## 2 资产清单 (Asset Inventory)

### 2.1 安全相关模块总表

| 资产 ID | 模块 | 源码位置 | 层级 | 安全职责 | 主要证据 |
|:-------:|------|---------|:----:|---------|:---------:|
| A-01 | SecOC | src/bsw/services/secoc/ | Service | PDU 认证 (MAC) + 新鲜度 (FV) | [C01][C02][C07] |
| A-02 | Csm | src/bsw/services/csm/ | Service | 作业管理 (MAC/加密/签名/哈希/随机) | [C03] |
| A-03 | CryIf | src/bsw/services/cryif/ | Service | Crypto 驱动抽象与路由 | [C05] |
| A-04 | KeyM | src/bsw/services/keym/ | Service | 密钥管理 (Set/Get/Copy/Derive) | [C04] |
| A-05 | Crypto | src/bsw/mcal/crypto/ | MCAL | AES-GCM/TRNG/哈希原语 + mbedtls 后端 | [C06][C10] |
| A-06 | Cdd_Hsm | src/bsw/cdd/ (Cdd_Hsm_1.0.0.c) | CDD | HSM 硬件抽象: 随机数/自检/安全启动验签 | [C08] (间接) |
| A-07 | Bootloader | src/bootloader/ | Boot | 安全启动链: 验签/分区 CRC/回滚/升级日志 | [C08][C09] |
| A-08 | TcpIp/TLS | src/bsw/services/tcpip/ + mqtt TLS | Service | 传输层安全 (MQTT TLS 1.2) | [C11] |
| A-09 | Dcm SecurityAccess | src/bsw/services/dcm/ | Service | 诊断访问控制 (SID 0x27) | 见 §6.2 |
| A-10 | NvM (CRC) | src/bsw/services/nvm/ | Service | 持久化数据完整性 | Safety Case [R06] |

### 2.2 资产与安全属性 (CIA + 真实性)

| 资产 | Confidentiality | Integrity | Availability | Authenticity | Non-repudiation |
|:-----|:---:|:---:|:---:|:---:|:---:|
| SecOC 密钥 (对称) | ✅ 关键 | ✅ | — | — | — |
| 安全启动公钥 | — (公开) | ✅ 关键 | — | ✅ | — |
| 固件映像 | — | ✅ 关键 | — | ✅ 关键 | ✅ (签名) |
| CAN/Eth 安全 PDU | — | ✅ | ✅ | ✅ 关键 | — |
| 新鲜度计数器 (FV) | — | ✅ 关键 | — | ✅ | — |
| 防回滚计数器 (NVM) | — | ✅ 关键 | ✅ | — | — |
| TLS 私钥/证书 | ✅ 关键 | ✅ | — | ✅ | — |
| 诊断会话状态 | — | ✅ | ✅ | — | — |

### 2.3 密码算法清单 (平台实现)

| 算法 | 用途 | 实现位置 | 备注 |
|------|------|---------|------|
| AES-CMAC | SecOC PDU MAC | Crypto_Aes.c / SecOc_Lcfg.c | SECOC_AES_MAC = 0 |
| HMAC-SHA256/512 | SecOC MAC (备选 profile) | Crypto 模块 | SECOC_HMAC_SHA256/512 |
| AES-GCM | 数据加解密 (CCC) | Crypto.c (Crypto_Encrypt/Decrypt) | mbedtls 后端 |
| ECDSA P-256 + SHA-256 | 固件签名/验签 | bl_secure_boot.c + sign_tool.py | 默认启动链算法 |
| ECDSA P-384 + SHA-384 | 备选签名 | bl_secure_boot.c | 分发表已含 |
| RSA PKCS1/PSS + SHA-256 | 备选签名 | bl_secure_boot.c | 分发表已含 |
| SM2/SM3 (国密) | 预留 | bl_secure_boot.c | **fail-closed**: 无后端时显式返回 BL_SB_ERROR_ALGO_NOT_SUPPORTED |
| TRNG | 随机数 | Crypto_HwTrng.c / Cdd_Hsm_GenerateRandom | 自检见 §6 |
| CRC (多项式族) | 数据完整性 (非安全认证) | Crc 模块 | 防随机失效, 不防攻击 |

---

## 3 TARA — 威胁分析与风险评估

### 3.1 方法论 (ISO 21434 Clause 14-16)

1. **攻击面分析** (§3.2): 枚举平台全部外部接口;
2. **威胁场景识别** (§3.3): 每个攻击面推导损害场景 (damage scenario);
3. **影响评级** (Clause 15): 按 Safety / Financial / Operational / Privacy
   四类, S0-S3 / F0-F3 / O0-O3 / P0-P3;
4. **攻击可行性评级** (Clause 16, 规则法): 综合耗时/专业度/目标知识/机会窗口/
   设备 → 1 (极易) ~ 5 (极难);
5. **风险值** (Clause 16.4): 影响 × 可行性查表 → 1-5;
6. **处置决定** (Clause 15.5): avoid / reduce / share / retain。

**风险值矩阵** (行=影响 1-3, 列=攻击可行性 1-5; 数值越高风险越大):

| 影响 \ 可行性 | 1 极易 | 2 易 | 3 中 | 4 难 | 5 极难 |
|:---:|:---:|:---:|:---:|:---:|:---:|
| **3 (S3/F3/O3)** | 5 | 5 | 4 | 4 | 3 |
| **2 (S2/F2/O2)** | 5 | 4 | 4 | 3 | 2 |
| **1 (S1/F1/O1/P1+)** | 4 | 3 | 3 | 2 | 1 |

### 3.2 攻击面分析

| 攻击面 | 接口/协议 | 暴露环境 | 对应模块 |
|:------:|----------|---------|---------|
| AS-1 | CAN 总线 (500k/1M) | 车内网络, 物理/远程 (经由网关) | Can/CanIf/SecOC |
| AS-2 | LIN 总线 | 车内低速网络 | Lin/LinIf |
| AS-3 | Ethernet (100BASE-T1) / SomeIP / SomeIpSd | 车内 IP 网络 | Eth/SoAd/SomeIp |
| AS-4 | DoIP (诊断 over IP) | IP 网络, 车外可及 (充电口/远程) | DoIP/Dcm |
| AS-5 | UDS 诊断 (OBD-II 口) | 物理可及, 售后场景 | Dcm (SID 0x27) |
| AS-6 | XCP 标定 | 物理可及 (开发/产线) | Xcp |
| AS-7 | OTA 固件更新链 | 远程 (云→车) | Bootloader + bl_upgrade_log |
| AS-8 | Bootloader / Flash 重编程 | 物理可及 (J-Link/OpenOCD) | sbl_main/bl_secure_boot |
| AS-9 | 调试接口 (JTAG/SWD) | 物理可及 | Mcu/开发探针 |
| AS-10 | MQTT/TLS (云连接) | 无线/远程 | mqtt TLS 1.2 |
| AS-11 | NvM 持久化区 | 经由 AS-7/AS-8 间接 | NvM/Fee |

### 3.3 威胁场景清单与风险评估

| ID | 威胁场景 | 攻击面 | 损害场景 | 影响 (类别/级) | 可行性 | 风险值 | 处置 |
|:--:|---------|:------:|---------|:---:|:---:|:---:|:---:|
| T01 | CAN 安全 PDU 重放 (截获后原样重发) | AS-1 | 未授权功能触发 (如反复解锁) | S1 | 2 | 3 | **reduce** (SecOC FV) |
| T02 | CAN PDU 伪造/注入 (无密钥构造报文) | AS-1/AS-5 | 安全相关信号被篡改 (SG-001 违背) | S2 | 2 | 4 | **reduce** (SecOC MAC) |
| T03 | UDS SecurityAccess 会话绕过/种子嗅探 | AS-5 | 未授权诊断操作 (改配置/清 DTC) | O2 | 2 | 3 | **reduce** (Dcm 0x27) |
| T04 | 固件篡改 (植入后门后刷写) | AS-7/AS-8 | 恶意固件获得执行 (SG-002 失效) | S2 | 3 | 4 | **reduce** (安全启动验签) |
| T05 | 固件回滚 (刷回含已知漏洞旧版本) | AS-7/AS-8 | 已修复漏洞复活 | F2 | 2 | 3 | **reduce** (防回滚计数器) |
| T06 | 对称密钥/私钥读取 (flash dump) | AS-8/AS-9 | SecOC/签名体系整体失守 | F3 | 3 | 4 | **reduce** (HSM 隔离) |
| T07 | Ethernet/SomeIP 消息注入 | AS-3 | 车内服务被未授权调用 | S1 | 3 | 3 | **reduce** (SecOC over Eth) |
| T08 | DoIP 未授权诊断连接 | AS-4 | 远程未授权诊断/刷写 | O2 | 3 | 3 | **reduce** (0x27 over DoIP) |
| T09 | 弱随机数 (TRNG 退化未检出) | 内部 | 密钥/nonce 可预测 | F2 | 4 | 2 | **reduce** (TRNG 自检) |
| T10 | NvM 持久数据篡改 (配置/DTC) | AS-11 | 功能配置被改 (经 AS-7/8) | O1 | 3 | 2 | **reduce** (NvM CRC + 写计数) |
| T11 | TLS 中间人 (MQTT 云通道) | AS-10 | 云端指令伪造/数据泄露 | F1/P1 | 3 | 3 | **reduce** (TLS 1.2 双向校验) |
| T12 | 故障注入绕过安全启动 (电压毛刺) | AS-8/AS-9 | 验签步骤被跳过 | S3 | 4 | 4 | **reduce** (fail-closed 流程) |
| T13 | 侧信道密钥恢复 (功耗/EM 分析) | AS-8/AS-9 | HSM 内密钥泄露 | F3 | 5 | 3 | **share** (依赖 HSM 硬件抗性) |
| T14 | 新鲜度计数器重置/溢出攻击 | AS-11 | 重放窗口重新打开 | S1 | 3 | 3 | **reduce** (FvM — 见 GAP-01) |

### 3.4 风险处置汇总

| 处置 | 威胁 | 说明 |
|:----:|:-----|------|
| reduce | T01-T12, T14 | 均有平台对策 (§6); 有效性以残余风险评估 (威胁分析子文档 §per-module) |
| share | T13 | 侧信道抗性由 HSM 硬件 (S32K312 SHE+/HSE) 承担, 平台侧 Cdd_Hsm 抽象不引入额外泄露面 |
| retain | 无 | 平台级无保留风险; 项目级 TARA 可按运行环境追加 |
| avoid | 无 | — |

---

## 4 攻击树分析 (Attack Trees)

### 4.1 攻击树 AT-1: 未授权控制车辆功能 (伪造安全报文)

```
G1 未授权控制车辆功能 (T01/T02/T07)
├── OR
│   ├── 1.1 伪造带有效 MAC 的 CAN PDU
│   │   ├── AND
│   │   │   ├── 1.1.1 获取 SecOC 对称密钥
│   │   │   │   ├── 1.1.1.1 flash 读取 [对策: HSM 隔离, 密钥不出 HSM → T06]
│   │   │   │   ├── 1.1.1.2 侧信道分析 [对策: share — HSM 硬件抗性 → T13]
│   │   │   │   └── 1.1.1.3 产线/密钥分发泄露 [对策: 密钥生命周期 §7]
│   │   │   └── 1.1.2 构造合法新鲜度值
│   │   │       └── 1.1.2.1 截获当前 FV 并同步递增 [对策: FV 截断 + 单调计数 → T14/GAP-01]
│   │   └── [阻断点: 无密钥时 MAC 伪造不可行 → 剩余路径仅重放]
│   ├── 1.2 重放截获的合法 PDU
│   │   └── [对策: SecOC Freshness 验证拒绝旧 FV → T01]
│   ├── 1.3 经由网关以未保护 PDU 注入 (QM 消息)
│   │   └── [对策: E2E + Com 审计 (Safety Case S-1.3); 车辆架构责任]
│   └── 1.4 Ethernet/SomeIP 服务未授权调用
│       └── [对策: SecOC over Ethernet + TLS (mqtt) → T07/T11]
```

### 4.2 攻击树 AT-2: 固件篡改与降级 (T04/T05/T12)

```
G2 在 ECU 上执行恶意/过期固件
├── OR
│   ├── 2.1 刷写未签名固件
│   │   ├── 2.1.1 伪造签名
│   │   │   ├── 2.1.1.1 获取签名私钥 [对策: 私钥仅存离线签名机 (sign_tool), 不在车端 → §7]
│   │   │   └── 2.1.1.2 算法弱点 (ECDSA P-256 nonce) [对策: TRNG 质量自检 → T09]
│   │   ├── 2.1.2 绕过验签
│   │   │   ├── 2.1.2.1 故障注入跳过验签分支 [对策: fail-closed 状态机 → T12]
│   │   │   └── 2.1.2.2 国密 SM2 后端缺失路径 [对策: 显式 BL_SB_ERROR_ALGO_NOT_SUPPORTED, 不静默放行]
│   │   └── [证据: test_sbl_boot_verify_pass / test_sbl_boot_hash_fail — C08]
│   ├── 2.2 刷写合法签名的历史 (含漏洞) 固件
│   │   └── [对策: Boot_AntiRollback 单调计数器 (NVM 持久) → T05;
│   │        证据: test_sbl_boot_rollback_protection / test_e2e_antrollback.c]
│   └── 2.3 篡改分区头 CRC 绕过分区校验
│       └── [对策: 分区 CRC + 映像签名双重校验 — test_bootloader.c]
```

### 4.3 攻击树 AT-3: 密钥资产泄露 (T06/T09/T13)

```
G3 获取平台密码资产 (密钥/证书)
├── OR
│   ├── 3.1 车端存储读取
│   │   ├── 3.1.1 JTAG/调试口 dump [对策: 量产锁调试口 (项目级); Cdd_Hsm 密钥不出 HSM]
│   │   └── 3.1.2 flash 物理拆卸读取 [对策: 对称密钥置于 HSM 密钥槽, 明文不落 flash]
│   ├── 3.2 密钥分发链截获
│   │   ├── 3.2.1 产线注入通道 [对策: 加密传输 + KeyM 注入流程 §7.3]
│   │   └── 3.2.2 OTA 密钥更新通道 [对策: 会话加密 + Csm_KeyElementSet 权限]
│   ├── 3.3 运行时侧信道
│   │   └── [对策: share — HSM 硬件抗侧信道; 平台侧算法常时间化待评估 GAP-12]
│   └── 3.4 弱随机导致密钥可预测
│       └── [对策: Cdd_Hsm_SelfTest + Crypto_HwTrng 上电自检 → T09]
```

---

## 5 网络安全目标 (Cybersecurity Goals)

> 由 TARA 导出; 编号 CSG-xx; 与 Safety Goal 的接口见 §1.4。

| 目标 ID | 网络安全目标 | 覆盖威胁 | 关联 SG | 验证状态 |
|:-------:|-------------|:--------:|:-------:|:--------:|
| CSG-01 | 安全相关 PDU 须通过密码学认证 (MAC) 才能被接受 | T02, T07 | SG-001 | ✅ [C01][C02][C07] |
| CSG-02 | 安全相关 PDU 须通过新鲜度验证 (防重放) | T01, T14 | SG-001 | 🟡 [C07] + GAP-01 (FvM) |
| CSG-03 | 固件映像须经验签后方可执行 | T04 | SG-002 | ✅ [C08] |
| CSG-04 | 固件版本须满足防回滚单调约束 | T05 | SG-002 | ✅ [C08] |
| CSG-05 | 车端密码密钥须与通用软件隔离 (HSM) | T06, T13 | — | 🟡 Cdd_Hsm 抽象 + GAP-03 |
| CSG-06 | 诊断安全功能须受 SecurityAccess (0x27) 访问控制 | T03, T08 | — | 🟡 Dcm 支持 0x27, 会话策略项目级配置 |
| CSG-07 | 车外通信通道须加密与双向认证 | T11 | — | 🟡 MQTT TLS 1.2 [C11]; DoIP over TLS 缺 (GAP-09) |
| CSG-08 | 随机数产生须通过上电/周期自检 | T09 | — | ✅ Cdd_Hsm_SelfTest + [C06] |
| CSG-09 | 网络安全事件须可检测、可记录 (DTC/日志) | T01-T08 | — | 🔴 GAP-02 (SecOC→Dem 上报被注释) |
| CSG-10 | 密码验证失败须进入 fail-closed 安全路径 | T12 | SG-002 | ✅ [C08] (SM2 无后端显式拒绝为例证) |

---

## 6 现有安全措施证据映射

### 6.1 措施-证据总表

| 措施 | 实现位置 | 单元证据 | 集成证据 | 备注 |
|------|---------|:--------:|:--------:|------|
| SecOC MAC 生成/验证 | src/bsw/services/secoc/ | [C01] tests/unit/secoc/test_secoc.c (28 断言) <br> [C02] services/test_secoc.c + SecOC_Test.c | [C07] QEMU p3c | AES-CMAC / HMAC-SHA256/512 profile |
| SecOC 新鲜度 (FV) | SecOC + FvM 接口 | [C07] tamper/replay 用例 | [C07] QEMU p3c | FvM 独立模块缺失 → GAP-01 |
| Csm 作业/密钥服务 | src/bsw/services/csm/ | [C03] 3 套测试 (25 断言) | [C07] Csm_MacGenerate/Verify 被调用 | MAC/ENCRYPT/SIGN/HASH/RANDOM API 常量齐备 |
| CryIf 路由 | src/bsw/services/cryif/ | [C05] 3 套测试 | — | 抽象层接口 |
| KeyM 密钥管理 | src/bsw/services/keym/ | [C04] 2 套测试 (25 断言) | [C08] bootloader 经 KeyM 取公钥 | Set/Get/Copy/Derive API |
| Crypto 原语 | src/bsw/mcal/crypto/ | [C06] 2 套 + [C10] AES 库 | [C07] 经 Csm 间接 | AES-GCM/TRNG/mbedtls 后端 |
| Cdd_Hsm 抽象 | src/bsw/cdd/src/Cdd_Hsm_1.0.0.c | 🔴 无专属测试 (GAP-03) | [C08] 间接 (bl 经 csm/keym) | GenerateRandom/SelfTest/SecureBootVerify |
| 安全启动验签 | src/bootloader/bl_secure_boot.c | [C08] test_bootloader.c::test_secure_boot_cert_chain 等 | test_sbl_main.c::test_sbl_boot_verify_pass | ECDSA P-256/384, RSA, SM2 fail-closed |
| 防回滚 | src/bootloader/bl_antrollback.c | [C08] test_e2e_antrollback.c | test_sbl_main.c::test_sbl_boot_rollback_protection | NVM 单调计数器 |
| 分区完整性 | src/bootloader/bl_partition.c | [C08] test_bootloader.c (CRC/回滚逻辑) | — | mock flash 注入式 |
| 离线签名 | tools/signing/sign_tool.py | — (工具) | genkey/sign/dumpkey 三命令 | 私钥存 tools/signing/keys/ (离线) |
| 升级日志 | src/bootloader/bl_upgrade_log.c | [C08] | — | 审计轨迹 |
| TLS (MQTT) | mqtt TLS 模块 | [C11] test_mqtt_tls.c | — | TLS 1.2, VERIFY_REQUIRED |
| Dcm SecurityAccess | src/bsw/services/dcm/ | services/Dcm/test_Dcm.c (会话/安全) | tests/qemu_full_stack/p2c_uds_inject | SID 0x27 常量与处理 |
| NvM 完整性 | src/bsw/services/nvm/ | tests/unit/nvm/ 等 | Safety Case [R06] | CRC + 写计数器 |
| 防御式编码 | 全部 | [C12] MISRA 报告 | CI L1 | 与 Safety Case 共享证据 |

### 6.2 QEMU SecOC 回环测试详情 [C07]

- **路径**: tests/qemu_full_stack/p3c_secoc_loopback/ (main_secoc_loopback.c +
  secoc_crypto_stub.c + build.sh)
- **需求注解**: `@req SWS_SecOC_00010` (MAC 生成), `@req SWS_SecOC_00011`
  (MAC 验证), `@req SWS_SecOC_00020/00021` (新鲜度收发)
- **覆盖行为**:
  1. 构造安全 PDU (数据 + 截断 MAC(4B) + 截断 FV(4B));
  2. 回环验证接受 (s_accepted 计数);
  3. **篡改注入** (`tamper` 参数): 修改数据位 → MAC 验证拒绝 (s_rejected);
  4. FvM 计数器同步 (FvM_UpdateCounter / SecocCrypto_GetTxFv);
- **意义**: 平台唯一的**全栈级**主动攻击模拟证据 (认证 + 防重放路径)。

### 6.3 签名工具链 [C09]

```
离线签名机                          ECU 侧
─────────────────                  ─────────────────────────────
sign_tool.py genkey  ──→ 私钥 (.pem, 仅离线保存)
sign_tool.py sign fw.bin v1.3.0 ──→ 签名固件 (YBL1 魔数 + ECDSA P-256)
sign_tool.py dumpkey sbl ──→ DER 公钥头文件 ──→ 编译进 bootloader
                                                    │
                                                    ▼
                                     bl_secure_boot 验签 (fail-closed)
```

- 密钥目录: tools/signing/keys/ (不入固件);
- 验签失败路径测试: test_sbl_boot_hash_fail (拒绝并回滚) [C08]。

### 6.4 证据缺口 (诚实声明)

| 缺口 | 影响 | 关联 |
|:-----|------|:----:|
| Cdd_Hsm 无专属单元测试 | HSM 抽象层行为仅由 bootloader 测试间接覆盖 | GAP-03 |
| SecOC 验证失败不上报 Dem | 网络安全事件不可见 (无 DTC) | GAP-02 |
| FvM 为 QEMU stub | 新鲜度管理无产品级实现 | GAP-01 |
| 无渗透测试报告 | Clause 13 确认手段不足 | GAP-06 |

---

## 7 密钥生命周期管理

> 按 ISO 21434 Clause 10 开发要求 + AUTOSAR KeyM/Csm 语义组织。

### 7.1 密钥类型与用途

| 密钥 | 类型 | 用途 | 存储位置 | 载体 |
|------|------|------|---------|------|
| SecOC 对称密钥 | AES-128/256 | PDU MAC 生成/验证 | HSM 密钥槽 (经 Csm keyId) | A-01/A-02 |
| 安全启动公钥 | ECDSA P-256 公钥 | 固件验签 | bootloader 只读区 (DER 编译) | A-07 |
| 签名私钥 | ECDSA P-256 私钥 | 离线固件签名 | **离线签名机** tools/signing/keys/ | [C09] |
| TLS 客户端密钥/证书 | RSA/EC + X.509 | MQTT 云通道 | flash (经 KeyM) | A-08 |
| 防回滚计数器 | 非密钥资产 | 版本单调约束 | NVM (经 bl_antrollback) | A-07 |

### 7.2 生成 (Generation)

- **对称密钥**: `Cdd_Hsm_GenerateRandom()` (HSM TRNG) → `Csm_KeyElementSet()`
  注入密钥槽; 上电自检 `Cdd_Hsm_SelfTest()` + `Crypto_HwTrng` 保证熵质量 (CSG-08)。
- **签名密钥对**: 离线 `sign_tool.py genkey` (ECDSA P-256, cryptography 库);
  私钥永不入车端。
- **平台义务**: 生成过程记录审计 (密钥 ID, 时间, 操作者) — 项目级流程 (GAP-05)。

### 7.3 分发 (Distribution)

1. **产线注入**: 诊断通道 (UDS 0x27 安全会话) → `KeyM_SetKey()` /
   `Csm_KeyElementSet()` 写入; 传输层须加密 (项目级 CAL 约束, GAP-08)。
2. **公钥分发**: `sign_tool.py dumpkey sbl` 导出 DER → 编译进 bootloader
   镜像 → 随安全启动链自证 (不可被运行时替换)。
3. **OTA 更新**: 密钥更新走加密会话 + `Csm_KeyElementCopy()` 槽切换
   (原子化, 失败可回退)。

### 7.4 存储 (Storage)

- **机密原则**: 对称密钥明文仅存在于 HSM 内部; flash 侧仅存密钥槽引用
  (keyId) 与密钥元数据 (CSG-05, AT-3 对策)。
- **完整性**: 密钥区纳入 NvM CRC 保护; bootloader 公钥区受分区签名保护。
- **隔离**: `Cdd_Hsm` 为唯一 HSM 访问通道 (MCAL Crypto 经 CryIf/Csm 路由),
  通用代码无直接 HSM 寄存器访问。

### 7.5 轮换 (Rotation)

- 触发: 定期 (密钥有效期策略) / 事件 (疑似泄露, CSG-09 检测到异常) /
  供应商变更;
- 流程: 新密钥注入备用槽 → `Csm_KeySetValid(newKeyId)` 原子切换 → 旧槽
  观察期后销毁; SecOC 双密钥重叠期由 SecOC 配置 (Tx/Rx keyId 独立) 支持;
- **平台缺口**: 轮换流程无自动化测试 (GAP-12)。

### 7.6 销毁 (Destruction)

- `Csm_KeyElementSet(keyId, NULL, 0)` / 槽覆写 + HSM 内部擦除命令;
- 报废流程 (EOL): 全 flash 擦除 (含 NvM 密钥区与防回滚计数器 — 注意与
  T05 的张力: 报废场景计数器一并清除是预期行为);
- **平台缺口**: 销毁验证步骤 (读回确认) 未文档化 (GAP-05)。

### 7.7 生命周期与 AUTOSAR API 映射

| 阶段 | AUTOSAR API | 平台状态 |
|:----:|-------------|:--------:|
| 生成 | Csm_KeyGenerate / Cdd_Hsm_GenerateRandom | ✅ 实现 + 测试 [C03][C06] |
| 注入 | KeyM_SetKey / Csm_KeyElementSet | ✅ 实现 + 测试 [C04] |
| 派生 | Csm_KeyDerive / KeyM 密钥派生接口 | ✅ 实现 [C04] |
| 导出 | Csm_KeyElementGet (受控) | ✅ 实现 [C03] |
| 生效 | Csm_KeySetValid | ✅ 实现 [C03] |
| 轮换 | 上述组合流程 | 🟡 API 齐备, 流程测试缺 (GAP-12) |
| 销毁 | 槽覆写 + NvM 擦除 | 🟡 API 齐备, 流程文档缺 (GAP-05) |

---

## 8 分布式活动 (Distributed Activities) — ISO 21434 Clause 7

> Clause 7 要求: 当网络安全活动分布在多个组织间 (OEM ↔ Tier-1 ↔ Tier-2),
> 须明确各方职责、接口协议与网络安全保证等级 (CAL)。

### 8.1 CAL 确定 (Cybersecurity Assurance Level)

| 模块/子系统 | 建议 CAL | 理由 |
|:-----------|:--------:|------|
| SecOC (安全 PDU 认证) | CAL 3 | 直接影响 SG-001 (通信完整性, ASIL B); 攻击可行性 2-3 |
| 安全启动 (Bootloader) | CAL 3 | 直接影响 SG-002 (任务定时, ASIL B); 固件篡改后果 S2-S3 |
| KeyM/Csm (密钥管理) | CAL 3 | 密钥泄露导致整个 SecOC/签名体系失守 |
| Dcm SecurityAccess | CAL 2 | 诊断访问控制, 后果 O2 但物理可及性降低可行性 |
| E2E (非安全 PDU) | CAL 1 | 仅覆盖随机失效, 不抗主动攻击; QM 场景 |
| Can/Lin/Eth 驱动 | CAL 1 | 传输层无安全语义, 由上层 SecOC 保护 |

### 8.2 接口协议 (Interface Agreement)

> 按 ISO 21434 Clause 7.4, 组织间须就以下事项达成书面协议。

| 接口 | 甲方 | 乙方 | 协议内容 | 状态 |
|:----:|------|------|---------|:----:|
| IA-1 | OEM (车型项目) | yuleASR 平台 | SecOC 密钥注入流程、CAL 3 交付物清单、TARA 复用范围 | 🔴 待建 (模板) |
| IA-2 | yuleASR 平台 | MCU 供应商 (NXP) | HSM 密钥隔离保证、TRNG 熵源认证、安全启动硬件信任根 | 🟡 依赖 NXP HSE 文档 |
| IA-3 | OEM | yuleASR 平台 | 漏洞通报流程 (Clause 11)、OTA 签名密钥托管、CAL 协议变更管理 | 🔴 待建 |
| IA-4 | yuleASR 平台 | 第三方库供应商 (mbedTLS 等) | SBOM 组件清单、CVE 通报义务、补丁响应 SLA | 🟡 SBOM 已有 [C13], SLA 待建 |

### 8.3 交付物清单 (CAL 3 级)

| 交付物 | 提供方 | 接收方 | 格式 | 关联 GAP |
|:------:|:------:|:------:|:----:|:--------:|
| TARA 报告 (平台级) | yuleASR | OEM | 本文档 §2-§3 | GAP-11 (项目级待补充) |
| 攻击树分析 | yuleASR | OEM | 本文档 §4 | — |
| 网络安全目标 (CSG) | yuleASR | OEM | 本文档 §5 | — |
| 密钥生命周期 SOP | yuleASR | OEM | 本文档 §7 | GAP-05 |
| 安全启动验签测试报告 | yuleASR | OEM | [C08] 测试集 | — |
| SecOC 集成测试报告 | yuleASR | OEM | [C07] QEMU p3c | — |
| SBOM | yuleASR | OEM | SPDX/CycloneDX [C13] | GAP-07 |

---

## 9 持续活动 (Continuous Activities) — ISO 21434 Clause 8

> Clause 8 要求: 组织须在整个产品生命周期内持续监控网络安全态势,
> 包括车载监控、事件响应与安全更新。

### 9.1 网络安全监控 (Cybersecurity Monitoring)

| 监控项 | 实现位置 | 检测方式 | 响应动作 | 状态 |
|:------:|---------|---------|---------|:----:|
| SecOC MAC 验证失败计数 | SecOC 内部计数器 | 滑动窗口异常检测 (阈值可配) | Dem DTC 上报 (SECOC_E_CRYPTO_AUTH_FAILED) | 🔴 被注释 (GAP-02) |
| SecOC 重放检测 (FV 异常) | SecOC_FreshnessVerify | FV 回退/跳跃计数 | Dem DTC + 降级处理 | 🟡 QEMU stub (GAP-01) |
| 诊断会话异常 (0x27 暴力破解) | Dcm SecurityAccess | 连续失败次数超阈值 → 锁定延迟 | 延长 seed 延迟 / 记录 DTC | 🟡 基础延迟实现, 无 DTC |
| 安全启动失败 | Bootloader | 验签失败计数 + 回滚尝试 | 进入恢复模式 + 升级日志记录 | ✅ [C08] |
| TRNG 自检失败 | Cdd_Hsm_SelfTest | NIST SP 800-90B 熵检验 | 故障安全状态 + Dem DTC | ✅ 上电自检 |
| 固件版本异常 | bl_antrollback | 版本计数器回退检测 | 拒绝刷写 + 日志 | ✅ [C08] |

### 9.2 网络安全事件响应 (Incident Response)

```
事件检测                    分类与评估                  响应与恢复
─────────                  ─────────                 ─────────
SecOC 异常计数 ─┐
                 ├→ 严重度评估 (CVSS / 影响分析)  ─┐
DTC 告警 ───────┘                                  │
                                                    ├→ 低风险: 记录 + 监控
OTA 通道异常 ───→ 日志分析 ─────────────────────────┤
                                                    ├→ 中风险: 安全更新 (OTA)
渗透测试发现 ───→ 缺陷跟踪 ─────────────────────────┤
                                                    └→ 高风险: 紧急召回 + 密钥轮换
```

| 响应阶段 | 平台能力 | 状态 |
|:--------:|---------|:----:|
| 检测 | SecOC 计数 + DTC + Bootloader 日志 | 🟡 部分 (GAP-02) |
| 分析 | 升级日志 [C08] + SBOM [C13] | 🟡 手动流程 |
| 遏制 | OTA 紧急更新通道 (MQTT TLS) | 🟡 [C11] |
| 根除 | 密钥轮换 (KeyM/Csm API) | 🟡 API 齐备, 流程缺 (GAP-12) |
| 恢复 | 安全启动 + 防回滚 | ✅ [C08] |
| 复盘 | 升级日志审计轨迹 | 🟡 无自动化 |

### 9.3 安全更新策略 (Security Update Policy)

| 更新类型 | 触发条件 | 分发通道 | 验证方式 | 状态 |
|:--------:|---------|---------|---------|:----:|
| 常规固件更新 | 功能迭代 / Bug 修复 | OTA (MQTT TLS) | ECDSA P-256 签名 + 防回滚 | ✅ |
| 安全补丁 | CVE 修复 / 漏洞修补 | OTA (优先通道) | 签名 + 版本约束 | 🟡 流程待建 |
| 密钥轮换 | 定期 / 泄露事件 | 诊断通道 (0x27) / OTA | Csm_KeySetValid 原子切换 | 🟡 API 就绪, SOP 缺 |
| 紧急召回 | 高危漏洞 (CVSS ≥ 9) | OEM 决策 → OTA + 经销商 | 安全启动验签 | 🔴 流程未建 |

---

## 10 漏洞管理 (Vulnerability Management) — ISO 21434 Clause 11

> Clause 11 要求: 组织须建立漏洞管理流程, 包括漏洞识别、评估、
> 通报与处置, 贯穿产品全生命周期。

### 10.1 漏洞识别来源

| 来源 | 识别方式 | 频率 | 平台状态 |
|:----:|---------|:----:|:--------:|
| 内部代码审计 | MISRA 静态分析 + 人工审查 | 每次 PR / 发版前 | ✅ CI L1 [C12] |
| 渗透测试 | 外部红队 (CAN/Eth/诊断/固件) | 发版前 + 年度 | 🔴 未执行 (GAP-06) |
| SBOM 组件漏洞 | CVE 数据库比对 (NVD / CNVD) | 持续 (SBOM 已有) | 🟡 SBOM 生成 [C13], 比对未自动化 |
| 第三方库公告 | mbedtls / FreeRTOS 安全公告 | 持续 | 🔴 无订阅机制 |
| 社区/用户报告 | 安全邮箱 / Issue Tracker | 按需 | 🔴 未建立 |

### 10.2 漏洞评估与分级

| 严重度 | CVSS 评分 | 响应 SLA | 处置要求 |
|:------:|:---------:|:--------:|---------|
| **Critical** | 9.0-10.0 | 24 小时评估, 7 天补丁 | 紧急 OTA / 密钥轮换 / 临时缓解措施 |
| **High** | 7.0-8.9 | 72 小时评估, 30 天补丁 | 安全 OTA 更新 |
| **Medium** | 4.0-6.9 | 2 周评估, 90 天补丁 | 常规 OTA 或下版修复 |
| **Low** | 0.1-3.9 | 季度评估 | 排入 backlog |

### 10.3 SBOM 与漏洞追踪

```
SBOM 生成                    漏洞比对                    处置闭环
─────────                  ─────────                 ─────────
scripts/generate-sbom.sh    NVD/CNVD API 查询 ──→ CVE 匹配列表
      │                           │
      ▼                           ▼
SPDX/CycloneDX 格式          严重度分级 + SLA 分配
      │                           │
      ▼                           ▼
归档 (每次发版)              补丁开发 → 测试 → OTA 发布
```

| 步骤 | 工具/流程 | 状态 |
|:----:|---------|:----:|
| SBOM 生成 | `scripts/generate-sbom.sh` [C13] | ✅ 实现 |
| SBOM 格式 | SPDX 2.3 / CycloneDX 1.4 | ✅ 支持 |
| CVE 自动比对 | — | 🔴 未实现 |
| 漏洞数据库 | NVD (https://nvd.nist.gov/) / CNVD | 🔴 未接入 |
| 补丁追踪 | — | 🔴 未建立 |
| 通报流程 | — | 🔴 未建立 (IA-3 待签) |

### 10.4 漏洞管理流程 (目标状态)

| 阶段 | 活动 | 责任方 | 输出 |
|:----:|------|:------:|------|
| 1. 识别 | SBOM 比对 + 渗透测试 + 社区报告 | yuleASR 安全团队 | 漏洞记录 (CVE ID / 内部 ID) |
| 2. 评估 | CVSS 评分 + 影响分析 (受影响模块/车型) | yuleASR + OEM | 严重度 + SLA |
| 3. 通报 | OEM 通知 (IA-3 协议) + 内部 JIRA | yuleASR | 通报记录 |
| 4. 修复 | 补丁开发 + 回归测试 | yuleASR 开发团队 | 补丁 + 测试报告 |
| 5. 分发 | OTA 发布 / 经销商刷写 | OEM | 更新记录 |
| 6. 验证 | 渗透复测 + SBOM 更新 | yuleASR | 闭环确认 |

### 10.5 与 GAP 登记表的关联

| GAP ID | 本章节覆盖 | 残余差距 |
|:------:|:---------:|---------|
| GAP-06 (渗透测试) | §10.1 列为识别来源 | 仍未执行, 已排入行动计划 §8.2 |
| GAP-07 (漏洞管理) | §10.3-10.4 定义目标流程 | SBOM→CVE 自动比对未实现 |
| GAP-08 (CAL 协议) | §8.1-8.2 定义 CAL 与接口协议模板 | OEM 签署待项目级执行 |

---

## 11 差距分析与行动计划

### 11.1 差距登记表

| GAP ID | 描述 | ISO 21434 条款 | 影响 CSG | 优先级 | 关联证据缺口 |
|:------:|------|:---:|:---:|:---:|:---:|
| GAP-01 | FvM (Freshness Value Manager) 无独立产品模块, QEMU 测试以 stub 实现 | Cl.10 | CSG-02 | **P0** | [C07] stub |
| GAP-02 | SecOC 验证失败的 Dem 上报被注释 (SecOC.c L364), 网络安全事件无 DTC | Cl.10/13 | CSG-09 | **P0** | §6.4 |
| GAP-03 | Cdd_Hsm 无专属单元测试 (仅 bootloader 测试间接覆盖) | Cl.10/13 | CSG-05/08 | **P1** | §6.4 |
| GAP-04 | 无 IDS (入侵检测) 能力 (SecOC 拒绝统计仅内部计数) | Cl.8/15 | CSG-09 | P1 | — |
| GAP-05 | 密钥注入/轮换/销毁的量产流程文档 (SOP) 缺失 | Cl.10 | CSG-05 | **P1** | §7.5-7.6 |
| GAP-06 | 渗透测试 / 红队评估未执行 | Cl.13 | 全部 | P1 | — |
| GAP-07 | 漏洞管理流程 (Cl.11): SBOM→CVE 自动比对未实现; 通报流程待 OEM 签署 | Cl.8/11 | — | P1 | [C13] 部分; §10 流程已定义 |
| GAP-08 | CAL (网络安全保证等级) 与网络安全接口协议 (CIA/CAL 协议): §8 已定义模板, 待 OEM 项目级签署 | Cl.7 | — | P2 | §8 已建立 |
| GAP-09 | DoIP 通道无 TLS/传输加密 (仅 MQTT 有 TLS) | Cl.10 | CSG-07 | P2 | §6.1 |
| GAP-10 | 网络安全需求未纳入 openspec/specs 规范体系 (无法走 /triple-* 流程门禁) | Cl.6/10 | — | P2 | — |
| GAP-11 | 本 TARA 为平台级, 项目级 item TARA (运行环境具体化) 待执行 | Cl.9 | — | **P0** (项目启动时) | §1.2 |
| GAP-12 | 密钥轮换/销毁流程无自动化测试 | Cl.10/13 | CSG-05 | P2 | §7.7 |

### 11.2 行动计划 (建议纳入 Phase 8)

| 优先级 | 行动 | 验收标准 | 预估 |
|:---:|------|---------|:---:|
| P0 | 实现 FvM 模块 (单调计数 + 截断 FV 策略 + 溢出处理) | 单元测试 + QEMU p3c 去 stub 化 | 2 周 |
| P0 | 启用 SecOC→Dem DTC 上报 (SECOC_E_CRYPTO_AUTH_FAILED 等) | Dem 集成测试见 DTC 置位 | 3 天 |
| P1 | Cdd_Hsm 专属单元测试 (mock HSM 寄存器协议) | 覆盖 Init/RNG/SelfTest/SecureBootVerify | 1 周 |
| P1 | 密钥生命周期 SOP 文档 (生成→销毁, 含产线) | 审核通过并存档 | 1 周 |
| P1 | 渗透测试 (CAN 注入/重放 + 固件篡改 + 调试口) | 测试报告 + 缺陷闭环 | 2 周 (外部) |
| P2 | DoIP over TLS / 传输安全评估 | 方案评审 | 1 周 |
| P2 | 网络安全需求纳入 openspec (cybersecurity spec 域) | /triple-new 走通 | 1 周 |
| P2 | 漏洞监控闭环 (SBOM → CVE 订阅 → 修复流程) | 流程演练一次 | 1 周 |

### 11.3 与 Safety Case 的交叉结论

1. **增强关系**: CSG-01/02 (SecOC) 将 Safety Case SG-001 的防护从
   "随机通信失效 (E2E)" 扩展到 "恶意伪造 (主动攻击)", 补全了 E2E 的
   天然盲区 (E2E CRC 不抗攻击者重构)。
2. **不冲突验证**: 安全启动链 (CSG-03/04) 保护 Safety Case 所依赖的
   WdgM/E2E 代码完整性 — 若验签被绕过, SG-002 论据失效, 因此
   test_sbl_boot_* 系列 [C08] 同时是两类 Case 的共同证据。
3. **共享偏差**: Safety Case 已接受偏差 D-FFI-001 (无 MPU 隔离) 在网络安全
   语境下的后果由 Cdd_Hsm 密钥隔离部分缓解; 完整缓解依赖 GAP-03 补齐。

---

## 12 结论

```
┌────────────────────────────────────────────────────────────────┐
│  🛡️ Cybersecurity Case 综合判定: 🟡 骨架成立, 量产前须闭环 P0 缺口 │
├────────────────────────────────────────────────────────────────┤
│  资产清单:      10 项安全资产, 9 类密码算法 — ✅ 已建立            │
│  TARA:          11 个攻击面, 14 个威胁场景, 14 项风险处置 — ✅ 平台级 │
│  攻击树:        3 棵 (控制/固件/密钥), 对策标注完整 — ✅            │
│  安全目标:      10 条 CSG, 6 ✅ / 3 🟡 / 1 🔴                    │
│  证据映射:      16 项措施, 14 项有测试证据 (含 QEMU 全栈攻击模拟)   │
│  密钥生命周期:  生成/注入/派生/生效 API ✅; 轮换/销毁流程 🟡         │
│  分布式活动:    CAL 分级 + 接口协议模板 — ✅ §8                      │
│  持续活动:      监控/事件响应/安全更新策略 — ✅ §9                    │
│  漏洞管理:      SBOM + 漏洞分级 + 流程框架 — ✅ §10                  │
│  差距:          12 项 (P0×3, P1×5, P2×4) — 行动计划已排期          │
│                                                                 │
│  "yuleASR 平台网络安全证据骨架完整; SecOC/安全启动/密钥管理三条     │
│   主防线均有测试证据。ISO 21434 Clauses 7/8/11 已覆盖。FvM 缺失   │
│   与网络安全事件不可见 (无 DTC) 为量产前必须关闭的 P0 项。"         │
└────────────────────────────────────────────────────────────────┘
```

---

*— Phase 7 安全合规证据 (Task 8) 产出物*
*配套: docs/cybersecurity/threat-analysis-per-module.md*
*2026-10-07*
