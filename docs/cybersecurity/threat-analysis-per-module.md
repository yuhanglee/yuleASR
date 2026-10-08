# yuleASR — 逐模块威胁分析 (Threat Analysis per Module)

> **文档**: Threat Analysis per Module — SecOC / Csm / KeyM / Crypto (+支撑模块)
> **版本**: 1.0 | **日期**: 2026-10-07
> **适用标准**: ISO/SAE 21434:2021 Clause 15 (损害场景) + Clause 16 (攻击可行性)
> **方法**: STRIDE 威胁分类 × 模块攻击面 × 代码级脆弱性核查
> **上级文档**: docs/cybersecurity/cybersecurity-case.md (§3 TARA, §8 差距)
> **核查基线**: 源码 HEAD (2026-10-07) 实读, 脆弱性均给出代码位置

---

## 0 文档控制

| 版本 | 日期 | 变更 |
|------|------|------|
| 1.0 | 2026-10-07 | 初始版本 — Phase 7 安全合规证据 (Task 8) |

**分析范围**: 四个核心密码服务模块 (SecOC / Csm / KeyM / Crypto) +
三个支撑模块 (Cdd_Hsm / CryIf / Bootloader 安全启动链)。

**符号约定**: 残余风险评级沿用 cybersecurity-case.md §3.1 矩阵
(风险值 1-5; ≥4 高, 3 中, ≤2 低)。

---

## 1 分析方法

1. **资产定位**: 每模块识别密码资产与安全属性 (见 cybersecurity-case §2.2);
2. **STRIDE 威胁枚举**: Spoofing (仿冒) / Tampering (篡改) / Repudiation (抵赖) /
   Information disclosure (信息泄露) / Denial of service (拒绝服务) /
   Elevation of privilege (提权);
3. **脆弱性核查**: 逐条对照源码与测试证据, 标注代码位置 — 不做假设性推测;
4. **对策映射**: 现有实现 + 测试证据 (编号沿用 cybersecurity-case §0.2 [C01]-[C14]);
5. **残余风险**: 对策后按影响 × 攻击可行性重估。

---

## 2 SecOC 模块威胁分析

### 2.1 模块概述与资产

- **源码**: src/bsw/services/secoc/ (SecOC.c, SecOc_Lcfg.c; 头文件 SecOC.h /
  SecOC_Cfg.h / SecOC_MemMap.h / SchM_SecOC.h)
- **职责**: 为安全相关 PDU 附加认证信息 (截断 MAC) 与新鲜度值 (截断 FV),
  接收侧验证 (SecOC_VerifyStatusOverride 提供验证状态覆盖接口)。
- **MAC 算法 profile**: SECOC_AES_MAC (AES-CMAC) / SECOC_HMAC_SHA256 /
  SECOC_HMAC_SHA512 (SecOC.h L130-132)。
- **错误码**: SECOC_E_FRESHNESS_FAILURE (0x02) 等 (SecOC.h L97)。

| 资产 | 属性 | 说明 |
|------|------|------|
| SecOC 对称密钥 (经 Csm keyId 引用) | Confidentiality | MAC 生成/验证根 |
| 截断 MAC (默认 4 字节, QEMU 配置) | Integrity/Authenticity | PDU 认证器 |
| 截断新鲜度值 FV (默认 4 字节) | Integrity/Authenticity | 防重放计数器 |
| SecOC 配置 (SecOc_Lcfg.c) | Integrity | PduId→密钥/算法映射 |

### 2.2 威胁识别 (STRIDE)

| ID | 威胁 (STRIDE 类) | 场景 | 影响 |
|:--:|-----------------|------|------|
| ST-1 | Tampering + Spoofing | 攻击者无密钥构造 PDU, 尝试伪造 MAC | 安全信号被伪造 (T02) |
| ST-2 | Spoofing (重放) | 截获合法 PDU 后原样/稍后重发 | 重放窗口攻击 (T01) |
| ST-3 | Information disclosure | SecOC 密钥经调试/读取路径泄露 | 认证体系失守 (T06) |
| ST-4 | Tampering | 配置 (PduId→keyId 映射) 被篡改为弱配置 | 验证被降级/旁路 |
| ST-5 | Denial of service | 总线 MAC 错误风暴 → 验证层持续拒绝 | 安全功能降级 (可用性) |
| ST-6 | Tampering (计数器) | FV 计数器被重置/回绕 | 重放窗口重开 (T14) |

### 2.3 脆弱性分析 (代码级核查)

| ID | 脆弱性 | 代码位置 | 严重性 |
|:--:|-------|---------|:------:|
| SV-1 | **验证失败不上报 Dem**: `Dem_ReportErrorStatus(SECOC_E_CRYPTO_AUTH_FAILED, ...)` 被注释 | SecOC.c L364 | 高 (CSG-09 缺失, GAP-02) |
| SV-2 | **FvM 非产品实现**: 新鲜度接口 (FvM_GetTx/RxFreshnessValue, FvM_UpdateCounter) 在 QEMU 测试中由 secoc_crypto_stub.c 提供 | tests/qemu_full_stack/p3c_secoc_loopback/secoc_crypto_stub.c | 高 (GAP-01) |
| SV-3 | **截断 MAC 4 字节**: 撞库空间 2^32; 总线速率限制下的在线暴力可行性受协议周期约束 | QEMU p3c 配置 (SECOC_CMAC_LEN 4U) | 中 (可配置, 项目级应评估 6-8 字节) |
| SV-4 | 截断 FV 4 字节: 计数器回绕周期与重放窗口取决于 FvM 策略 | 同上 | 中 (依赖 GAP-01 关闭) |

### 2.4 现有对策与证据

| 对策 | 实现 | 证据 |
|------|------|------|
| MAC 生成/验证路径 | SecOC_Init/DeInit/主功能 + Csm_MacGenerate/MacVerify 集成 | [C01] tests/unit/secoc/test_secoc.c (28 断言); [C02] test_secoc.c + SecOC_Test.c |
| 防重放 (新鲜度) | FV 附加 + SECOC_E_FRESHNESS_FAILURE 错误路径 | [C07] QEMU p3c: tamper 注入 → s_rejected |
| 全栈攻击模拟 | 回环 + 数据位篡改 | [C07] @req SWS_SecOC_00010/00011/00020/00021 |
| 配置静态化 | SecOc_Lcfg.c 编译期绑定 (运行时不可改) | 代码结构 |
| MISRA 防御式编码 | Required=0 | [C12] |

### 2.5 残余风险评估

| 威胁 | 对策后可行性 | 残余风险值 | 说明 |
|:----:|:---:|:---:|------|
| ST-1 伪造 MAC | 4 (需先破密钥) | 2 | 无密钥时不可行 |
| ST-2 重放 | 3 (受 FV 保护; SV-2/SV-4 弱化) | **3** | FvM 产品化前保持中等 |
| ST-3 密钥泄露 | 3 (经 Csm/Cdd_Hsm 隔离) | 3 | 见 §3/§6 联动 |
| ST-4 配置篡改 | 4 (编译期静态) | 1 | — |
| ST-5 DoS | 2 (拒绝即预期行为) | 2 | Fail-closed 语义正确; 检测缺 (SV-1) |
| ST-6 计数器回绕 | 3 (SV-2) | **3** | 与 GAP-01 同源 |

**模块结论**: 认证主路径有完整测试证据; **新鲜度管理 (FvM) 与事件可见性
(Dem 上报) 是本模块两个高严重性脆弱性**, 对应 GAP-01/GAP-02 (P0)。

---

## 3 Csm 模块威胁分析

### 3.1 模块概述与资产

- **源码**: src/bsw/services/csm/ (Csm.c, Csm_Cfg.c)
- **职责**: 加密服务管理器 — 作业 (Job) 调度与密钥元素管理, 向上提供
  MAC/加密/签名/哈希/随机服务, 向下经 CryIf 路由至 Crypto 驱动。
- **服务 API 常量** (Csm.h L78-84): CSM_API_HASH (0x30),
  CSM_API_MAC_GENERATE (0x40), CSM_API_MAC_VERIFY (0x41),
  CSM_API_ENCRYPT (0x50), CSM_API_SIGNATURE_GENERATE (0x60),
  CSM_API_SIGNATURE_VERIFY (0x61)。
- **密钥 API**: Csm_KeyElementSet/Get/Copy, Csm_KeyCopy,
  Csm_KeyElementIdsGet, Csm_KeyGenerate, Csm_KeyDerive, Csm_KeySetValid。

| 资产 | 属性 | 说明 |
|------|------|------|
| 密钥元素 (keyId → 元素) | Confidentiality | 全平台密码资产入口 |
| 作业队列/回调状态 | Integrity | 异步作业结果完整性 |
| Csm 配置 (Csm_Cfg.c) | Integrity | jobId→算法/密钥绑定 |

### 3.2 威胁识别 (STRIDE)

| ID | 威胁 | 场景 | 影响 |
|:--:|------|------|------|
| CT-1 | Information disclosure | Csm_KeyElementGet 被未授权调用方用于导出密钥明文 | 密钥泄露 (T06) |
| CT-2 | Tampering | 作业参数 (jobId/数据指针) 被篡改 → 错误密钥/算法绑定 | 认证降级 |
| CT-3 | Elevation of privilege | BSW 内部任意模块可直接调用 Csm 服务 (无调用方鉴权) | 越权密码运算 |
| CT-4 | Denial of service | 作业队列被填满/回调不返回 | 密码服务不可用 (SecOC 连锁) |
| CT-5 | Repudiation | 密钥操作 (Set/Copy/Derive) 无审计记录 | 事后不可追溯 |

### 3.3 脆弱性分析 (代码级核查)

| ID | 脆弱性 | 核查结果 | 严重性 |
|:--:|-------|---------|:------:|
| CV-1 | 无调用方访问控制 (AUTOSAR 语义固有) | Csm API 无 capability/调用方校验; 依赖 BSW 分区隔离 (Safety Case D-FFI-001: 无 MPU → 软件层隔离弱) | 中 |
| CV-2 | KeyElementGet 明文返回 | 设计使然 (AUTOSAR KeyM/Csm 语义); 风险取决于调用方范围 | 中 |
| CV-3 | 密钥操作无审计钩子 | 仅 KeyM 有 SetNotificationCallback; Csm 无操作日志 | 低 (GAP-05 联动) |
| CV-4 | 异步作业回调上下文 | test_csm.c 覆盖 callback 路径 (test_callback); 嵌套/重入未见测试 | 低 |

### 3.4 现有对策与证据

| 对策 | 实现 | 证据 |
|------|------|------|
| Init/DeInit/重复初始化防护 | Csm_Init_Success / NullConfig / AlreadyInitialized / DeInit_NotInitialized | [C03] tests/unit/csm/test_csm.c (25 断言) + services/test_csm.c + Csm_Test.c |
| 密钥元素 CRUD | KeyElementSet/Get/Copy + KeyGenerate/KeyDerive/KeySetValid 测试 | [C03] |
| 作业回调完整性 | test_callback 路径 | [C03] |
| 全栈 MAC 通路 | QEMU: Csm_MacGenerate/MacVerify 被 SecOC 回环真实调用 | [C07] |
| Bootloader 集成 | bl_secure_boot 经 csm/keym 上下文 (test_bootloader.c L724 bl_secure_boot_init(&ctx, &cfg, csm, keym)) | [C08] |

### 3.5 残余风险评估

| 威胁 | 对策后可行性 | 残余风险值 | 说明 |
|:----:|:---:|:---:|------|
| CT-1 密钥导出 | 3 (需先获 BSW 执行权) | 3 | 与 CV-1/CV-2 联动; MPU (D-FFI-001) 为根因 |
| CT-2 作业参数篡改 | 4 | 2 | 配置静态绑定 |
| CT-3 越权调用 | 3 | 3 | AUTOSAR 固有; 项目级以分区/MPU 缓解 |
| CT-4 DoS | 3 | 2 | 队列状态机有测试 |
| CT-5 不可追溯 | 4 | 1 | 审计为流程性需求 (GAP-05) |

**模块结论**: 功能正确性证据充分 (3 套测试 + QEMU 全栈); **调用方鉴权为
AUTOSAR 架构固有缺口**, 靠项目级 MPU 分区 (Safety Case 偏差修复方向) 收敛。

---

## 4 KeyM 模块威胁分析

### 4.1 模块概述与资产

- **源码**: src/bsw/services/keym/ (KeyM.c)
- **职责**: 密钥管理器 — 密钥的设置/读取/复制/派生与通知回调, 密钥槽与
  Csm 协同 (密钥数据实际经 Csm 密钥元素存储)。
- **API**: KeyM_Init / KeyM_GetVersionInfo / KeyM_SetKey / KeyM_GetKey /
  KeyM_SetNotificationCallback 等。

| 资产 | 属性 | 说明 |
|------|------|------|
| 密钥槽引用与密钥描述 | Integrity/Confidentiality | keyId 映射 |
| 密钥更新通知回调 | Integrity | 轮换事件完整性 |
| TLS 证书链 (经 KeyM 存储) | Integrity/Authenticity | 云通道信任根 |

### 4.2 威胁识别 (STRIDE)

| ID | 威胁 | 场景 | 影响 |
|:--:|------|------|------|
| KT-1 | Tampering | KeyM_SetKey 被未授权调用 → 密钥被替换为攻击者密钥 | 认证体系劫持 (最严重) |
| KT-2 | Spoofing | 伪造密钥更新通知 → 下游使用未生效密钥 | 认证错乱 |
| KT-3 | Information disclosure | KeyM_GetKey 导出 | 密钥泄露 |
| KT-4 | Tampering | 证书链被替换 (TLS 信任根劫持) | 云通道 MITM (T11) |
| KT-5 | Denial of service | 密钥槽耗尽/非法 keyId 风暴 | 密码服务降级 |

### 4.3 脆弱性分析 (代码级核查)

| ID | 脆弱性 | 核查结果 | 严重性 |
|:--:|-------|---------|:------:|
| KV-1 | SetKey 无来源认证 | 信任边界 = BSW 内部 (与 CV-1 同根因); 产线注入通道鉴权属流程层 (GAP-05) | 高 (流程未文档化) |
| KV-2 | 错误路径覆盖 | test_keym.c 含 run_error_tests 专项 (25 断言) — 非法参数有验证 | 低 |
| KV-3 | 证书解析/校验深度 | KeyM 证书处理以 TLS 模块 (test_mqtt_tls.c) 间接验证; 独立 X.509 链校验测试未见 | 中 |

### 4.4 现有对策与证据

| 对策 | 实现 | 证据 |
|------|------|------|
| 基础/错误路径测试 | run_basic_tests + run_error_tests | [C04] tests/unit/keym/test_keym.c (25 断言) + services/test_keym.c |
| Bootloader 取钥集成 | bl_secure_boot_init(ctx, cfg, csm, keym) → 验签公钥经 KeyM | [C08] test_bootloader.c |
| 通知回调 | KeyM_SetNotificationCallback API + 测试 | [C04] |
| MISRA 编码基线 | Required=0 | [C12] |

### 4.5 残余风险评估

| 威胁 | 对策后可行性 | 残余风险值 | 说明 |
|:----:|:---:|:---:|------|
| KT-1 未授权 SetKey | 3 (需 BSW 执行权 + 流程缺口) | **4** | 密钥注入 SOP (GAP-05) 关闭前保持高 |
| KT-2 伪造通知 | 4 | 1 | 回调由 BSW 内部触发 |
| KT-3 密钥导出 | 3 | 3 | 同 CT-1 |
| KT-4 证书替换 | 3 | 3 | KV-3; TLS verify 模式 REQUIRED 缓解 |
| KT-5 DoS | 3 | 2 | 错误路径已测 |

**模块结论**: 功能测试完备; **密钥注入信任链 (KT-1) 是全平台最高残余风险**,
必须以产线 SOP + 加密注入通道 (GAP-05, P1) 关闭。

---

## 5 Crypto 模块 (MCAL) 威胁分析

### 5.1 模块概述与资产

- **源码**: src/bsw/mcal/crypto/ (Crypto.c, Crypto_Aes.c, Crypto_HwTrng.c,
  Crypto_Hsm.c, Crypto_S32K312_Hsm.c, Crypto_MbedTLS.c, Crypto_MbedTLS_Mem.c,
  Crypto_Cfg.c)
- **职责**: 密码驱动 — 作业执行 (Crypto_ProcessJob), AES-GCM 加解密
  (Crypto_Encrypt/Decrypt, CCC 语义), 随机数生成与播种, TRNG 硬件访问,
  S32K312 HSM 后端与 mbedtls 主机后端双实现。
- **需求注解**: @req SWS_Crypto_00100~00135 (Crypto_S32K312_Hsm.c 等, 共 36 处)。

| 资产 | 属性 | 说明 |
|------|------|------|
| TRNG 熵源 | Integrity | 全平台随机性根 |
| HSM 会话/密钥槽访问 | Confidentiality | 硬件信任根接口 |
| AES-GCM 原语正确性 | Integrity | 加密数据保护基础 |
| mbedtls 后端 (主机开发态) | Integrity | 不得混入量产固件 |

### 5.2 威胁识别 (STRIDE)

| ID | 威胁 | 场景 | 影响 |
|:--:|------|------|------|
| PT-1 | Tampering | TRNG 退化/被固定 → 密钥与 nonce 可预测 | 密钥体系根失效 (T09) |
| PT-2 | Information disclosure | HSM 后端调用序列泄露密钥材料 (API 误用) | 密钥泄露 |
| PT-3 | Tampering | AES-GCM 实现错误 (nonce 重用/标签绕过) | 机密性/完整性失效 |
| PT-4 | Tampering | 开发态 mbedtls 后端误编入量产目标 | 算法实现未经硬件安全评估 |
| PT-5 | Denial of service | ProcessJob 交叉调用/共享会话竞争 | 密码服务死锁 |

### 5.3 脆弱性分析 (代码级核查)

| ID | 脆弱性 | 核查结果 | 严重性 |
|:--:|-------|---------|:------:|
| PV-1 | TRNG 自检深度 | Crypto_HwTrng.c 21 处 @req 注解, 上电自检存在; 连续性/健康检测的周期性策略未见文档化 | 中 |
| PV-2 | 双后端选择机制 | Crypto_Cfg.c 编译期选择; 误配风险由构建配置评审控制 (4 处 @req) | 低 |
| PV-3 | nonce 管理 | AES-GCM 由调用方提供 IV; QEMU/单测用固定 IV — 量产配置须强制随机 IV | 中 (项目级) |
| PV-4 | 常时间性 | 软件路径 (mbedtls/AES) 未做常时间化声明 | 低 (HSM 路径承担) |

### 5.4 现有对策与证据

| 对策 | 实现 | 证据 |
|------|------|------|
| 驱动生命周期 | Crypto_Init/DeInit + test_init_deinit | [C06] tests/unit/autosar/mcal/test_Crypto.c + Crypto_Test.c |
| AES 原语 | AES 已知向量 | [C10] tests/unit/libs/test_lib_aes.c |
| 随机数接口 | GenerateRandom/SeedRandom API | [C06] |
| HSM 后端需求追溯 | SWS_Crypto_00100+ 注解 | 源码 36 处 (trace_requirements.py 可复核) |
| 全栈 MAC 通路 | 经 Csm → CryIf → Crypto | [C07] |
| mbedtls 后端隔离 | 主机构建目标独立 | 构建配置 (PV-2) |

### 5.5 残余风险评估

| 威胁 | 对策后可行性 | 残余风险值 | 说明 |
|:----:|:---:|:---:|------|
| PT-1 TRNG 退化 | 4 (自检存在; 周期策略缺) | 2 | PV-1 |
| PT-2 HSM 误用 | 3 | 3 | 经 Csm 抽象收敛 |
| PT-3 GCM 错误 | 5 (mbedtls 成熟实现 + 向量测试) | 1 | — |
| PT-4 后端误配 | 3 (构建评审) | 2 | PV-2, 量产发布清单应含检查项 |
| PT-5 并发竞争 | 3 | 2 | 作业状态机测试 |

**模块结论**: 双后端架构清晰、需求注解完备 (2113 个 SWS 注解中 Crypto 占
113); 主要残余风险在 **TRNG 周期性健康策略与项目级 nonce 管理**。

---

## 6 支撑模块分析 (简表)

### 6.1 Cdd_Hsm (HSM 抽象)

- **源码**: src/bsw/cdd/src/Cdd_Hsm_1.0.0.c (+ Cdd_Hsm.h)
- **API**: Cdd_Hsm_Init/DeInit/IsAvailable/GetStatus/GenerateRandom/SelfTest/
  SecureBootVerify。
- **威胁要点**: HSM 会话劫持 (I); 自检绕过 (T); 抽象层寄存器误用 (T)。
- **脆弱性**: **无专属单元测试** (tests/ 下无 Cdd_Hsm 匹配; 仅 bootloader
  测试间接覆盖) → GAP-03 (P1)。
- **现有对策**: 状态机 (Cdd_Hsm_StateType/StatusType); fail-closed 验签
  (SecureBootVerify)。
- **残余风险**: **3 (中)** — 测试缺口使其成为链条上最薄弱一环。

### 6.2 CryIf (Crypto 接口)

- **源码**: src/bsw/services/cryif/ (CryIf.c)
- **威胁要点**: 通道/驱动路由被篡改 → 密码作业被重定向 (T)。
- **脆弱性**: 路由表为编译期配置 (低); 无专属渗透场景。
- **现有对策**: [C05] 三套单元测试 (tests/unit/cryif/ + services/test_cryif.c +
  CryIf_Test.c)。
- **残余风险**: 1 (低)。

### 6.3 Bootloader 安全启动链

- **源码**: src/bootloader/ (bl_secure_boot.c, bl_antrollback.c,
  bl_partition.c, bl_rollback.c, bl_upgrade_log.c, main/sbl_main.c)
- **威胁要点**: 见 cybersecurity-case §4.2 攻击树 AT-2 (T04/T05/T12)。
- **关键对策**:
  - 多算法验签分发表 (ECDSA P-256/384, RSA PKCS1/PSS; **SM2/SM3 无后端时
    显式返回 BL_SB_ERROR_ALGO_NOT_SUPPORTED — fail-closed 设计**, 
    bl_secure_boot.h L77/L105-107);
  - Boot_AntiRollback NVM 单调计数器 (Read/Write/Increment/Stage/
    NotifySuccessfulBoot);
  - 升级日志审计 (bl_upgrade_log.c)。
- **证据**: [C08] test_bootloader.c (分区 CRC + 回滚 + test_secure_boot_cert_chain),
  test_sbl_main.c (verify_pass / hash_fail / rollback_protection /
  rollback_execute), test_e2e_antrollback.c。
- **残余风险**: 2 (低) — 验签被绕过的故障注入路径 (T12) 依赖硬件手段
  (锁定调试口, 项目级)。

---

## 7 残余风险汇总

| 模块 | 最高残余风险 | 主导威胁 | 对应 GAP | 处置建议 |
|------|:---:|---------|:---:|---------|
| SecOC | 3 (中) | ST-2 重放 / ST-6 计数器 | GAP-01, GAP-02 | FvM 产品化 + Dem 上报 (P0) |
| Csm | 3 (中) | CT-1/CT-3 密钥导出 | GAP-05 (联动) | MPU 分区 + 注入通道加密 |
| KeyM | **4 (高)** | KT-1 未授权 SetKey | GAP-05 | 产线密钥 SOP + 鉴权通道 (P1) |
| Crypto | 3 (中) | PT-2 HSM 误用 | — | 经 Csm 收敛 + 评审 |
| Cdd_Hsm | 3 (中) | 测试缺口 | GAP-03 | 专属单元测试 (P1) |
| CryIf | 1 (低) | — | — | — |
| Bootloader | 2 (低) | T12 故障注入 | — | 量产锁调试口 (项目级) |

**平台级残余风险结论**: 无风险值 5 项; 唯一 4 级项 (KeyM KT-1) 为流程性
缺口, 关闭路径明确 (GAP-05 SOP); 其余 ≤3 级且均有工程化处置方向。

---

## 8 复核与维护

```bash
# 1. 本文档引用的测试证据存在性复核
ls tests/unit/secoc/ tests/unit/csm/ tests/unit/keym/ tests/unit/cryif/ \
   tests/unit/libs/test_lib_aes.c tests/qemu_full_stack/p3c_secoc_loopback/

# 2. SecOC Dem 上报缺口 (SV-1) 现场核查
grep -n "Dem_ReportErrorStatus" src/bsw/services/secoc/src/SecOC.c   # → 注释态

# 3. Crypto 需求注解覆盖率复核 (SWS_Crypto_*)
python3 tools/traceability/trace_requirements.py | grep -A2 "\[3\]"
```

- **维护触发**: GAP-01/02/03/05 关闭时更新对应模块残余风险;
- **重估周期**: 随平台版本 (当前 v1.3.0) 或 CVE 事件触发 (GAP-07 流程建立后)。

---

## 9 结论

1. 四个核心模块均建立了 **资产 → 威胁 (STRIDE) → 代码级脆弱性 → 对策/证据 →
   残余风险** 的完整分析链, 脆弱性全部落点到具体代码位置, 可独立复核。
2. **平台最薄弱点**排序: KeyM 密钥注入信任链 (残余 4) > SecOC 新鲜度/可见性
   (GAP-01/02) > Cdd_Hsm 测试缺口 (GAP-03)。
3. 三个支撑模块中 Bootloader 安全启动链证据最厚 (fail-closed 多算法验签 +
   防回滚 + 三套测试), 是平台当前最强的安全资产。
4. 本文档与 cybersecurity-case.md §8 行动计划一一对应: P0 项 (FvM/Dem 上报)
   直接来自 §2.3 的 SV-1/SV-2 脆弱性。

---

*— Phase 7 安全合规证据 (Task 8) 产出物*
*上级文档: docs/cybersecurity/cybersecurity-case.md*
*2026-10-07*
