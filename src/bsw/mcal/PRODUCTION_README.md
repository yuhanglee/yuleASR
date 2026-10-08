# MCAL 层实现说明

## 当前状态

本目录包含 AUTOSAR MCAL（Microcontroller Abstraction Layer）驱动的实现文件。

**重要提示**：当前实现为**参考桩实现（Reference Stub Implementation）**，用于：
- 开发阶段的接口验证
- 单元测试和集成测试
- 静态代码分析和 MISRA 合规性检查

## 量产要求

**量产部署前必须替换为芯片厂商提供的 MCAL IP**：

| 芯片平台 | 厂商 | MCAL 来源 |
|---------|------|----------|
| S32K312 | NXP | S32K3 RTD (Real-Time Drivers) |
| i.MX8M Mini | NXP | i.MX 8M MCAL |
| TC3xx | Infineon | AURIX MCAL |

## 替换流程

1. 获取厂商 MCAL 包（通常需要 NDA）
2. 替换对应驱动目录下的 `.c` 和 `.h` 文件
3. 更新 `CMakeLists.txt` 中的源文件路径
4. 验证配置生成器（`config/templates/`）与厂商 MCAL 配置结构一致
5. 运行全量测试套件验证接口兼容性

## 接口兼容性

所有桩实现严格遵循 AUTOSAR SWS 规范定义的 API 签名，确保：
- 上层代码（ECUAL/Services）无需修改即可对接厂商 MCAL
- 配置生成器输出的 `*_Cfg.h` 文件可直接被厂商 MCAL 使用

## 驱动清单

| 驱动 | AUTOSAR 规范 | 桩实现状态 | 备注 |
|------|------------|-----------|------|
| Mcu | SWS_McuDriver | ✓ 桩实现 | 时钟/复位/电源模式 |
| Port | SWS_PortDriver | ✓ 桩实现 | 端口引脚配置 |
| Dio | SWS_DioDriver | ✓ 桩实现 | 数字 IO |
| Can | SWS_CanDriver | ✓ 桩实现 | CAN 控制器 |
| Spi | SWS_SpiDriver | ✓ 桩实现 | SPI 通信 |
| Gpt | SWS_GptDriver | ✓ 桩实现 | 通用定时器 |
| Pwm | SWS_PwmDriver | ✓ 桩实现 | PWM 输出 |
| Adc | SWS_AdcDriver | ✓ 桩实现 | ADC 转换 |
| Wdg | SWS_WdgDriver | ✓ 桩实现 | 看门狗 |
| Icu | SWS_IcuDriver | ✓ 桩实现 | 输入捕获 |
| Lin | SWS_LinDriver | ✓ 桩实现 | LIN 通信 |
| Eth | SWS_EthDriver | ✓ 桩实现 | 以太网控制器 |
| Fls | SWS_FlsDriver | ✓ 桩实现 | Flash 驱动 |
| Eep | SWS_EepDriver | ✓ 桩实现 | EEPROM 驱动 |
| I2c | SWS_I2cDriver | ✓ 桩实现 | I2C 通信 |
| Ocu | SWS_OcuDriver | ✓ 桩实现 | 输出比较 |
| Crypto | SWS_CryptoDriver | ✓ 桩实现 | 加密加速 |

## 安全等级

所有 MCAL 驱动按 **ASIL-B** 等级开发（ISO 26262），支持 ASIL 分解：
- 系统级 ASIL-D → 子系统 ASIL-B(D)
- 满足 SWE.1-SWE.6 全流程要求
- MISRA C:2012 合规
- 单元测试覆盖率 ≥ 80%

## 联系信息

如需获取量产 MCAL 支持，请联系：
- NXP S32K: https://www.nxp.com/design/automotive-software-and-tools/s32-design-studio
- 予乐技术支持: support@yuletech.com
