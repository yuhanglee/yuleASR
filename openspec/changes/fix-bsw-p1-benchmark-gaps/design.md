# Design: fix-bsw-p1-benchmark-gaps

## 背景与依据
- 对标来源：`.qoder/skills/autosar-benchmark-compare/references/benchmark-compare-2026-09-28.md` §5 P1 清单与 `api-symbol-diff-p0-2026-09-28.txt` 全量 theirs_only。
- 判定纪律：头文件符号面 ≠ 功能符合度；落地以「API 存在 + 行为测试通过」为完成标准。

## AD1：BswM 通知回调写入模式端口（原始值语义）
- 现有引擎：`BswM_RequestMode(SwCompositionId, Mode)` 仅接受 0..7 仲裁模式，按 composition 绑定写入 `portValue[]`，MainFunction 评估规则。
- 决策：通知回调以 **原始模块状态** 写入端口（如 ComM mode 0..2、Nm_StateType 0..16），不走 `BswM_RequestMode` 的 mode 域校验；新增内部助手 `BswM_WritePortByComposition(CompositionId, RawValue)`，UNINIT 时上报 DET 并忽略。
- 端口表：`BswM_Lcfg.c` 在既有索引 0=EcuM/1=ComM/2=NM 后追加 DCM、CANSM、ETHSM、FRSM、LINSM、LINSM_SCHEDULE、LINTP、COMM_PNC、ECUM_REQUESTED、PARTITION、ETHIF、NM_CARWAKEUP、DCM_APPUPDATED 端口，`NumModeRequestPorts` 3→16；既有表达式/规则索引不动。
- 未匹配端口：composition 未绑定任何端口时静默丢弃（与 `BswM_RequestMode` 对未匹配 composition 的锁存语义不同——通知是广播事实，非请求）。
- 类型镜像：沿用 `BswM_EcuMStateType` 先例，新增 `BswM_ComMModeType/BswM_DcmCommunicationModeType/BswM_NmStateType/BswM_CanSmStateType/BswM_EthSmStateType/BswM_FrSmStateType/BswM_LinSmStateType/BswM_LinTpModeType` 及 `NetworkHandleType`（若 ComStack_Types 已提供则复用）。

## AD2：LinIf 收发器与节点配置委托
- LinIf 不实现收发器逻辑，`LinIf_*Trcv*` 族直接委托 `LinTrcv_SetOpMode/GetOpMode/SetWakeupMode/GetWakeupReason/CheckWakeup`，通道号映射沿用 LinIf 的 ChannelId。
- 节点配置：NAD/PID 为通道运行时状态（非静态配置表），存于 LinIf 通道运行时结构；`SetPIDTable` 拷贝至内部表并校验帧数 ≤ 通道 NumFrames。
- `LinIf_IsSupportTpTransmit`：读取通道配置中的诊断/Tp 标志（无该标志时按配置默认 FALSE）。

## AD3：CanIf 错误面下沉到 Can 驱动
- `mcal/Can` 新增 `Can_GetControllerErrorState(uint8, Can_ErrorStateType*)`、`Can_GetControllerRxErrorCounter(uint8, uint8*)`、`Can_GetControllerTxErrorCounter(uint8, uint8*)`，驱动内维护错误计数运行时状态（BUSOFF/ERROR_ACTIVE/PASSIVE 迁移）。
- CanIf 侧做 UNINIT/参数 DET 守卫后委托；通知状态（`CanIf_ReadRxNotifStatus/ReadTxNotifStatus`）由 CanIf 在 RxIndication/TxConfirmation 路径维护，受配置位 `ReadRxPduNotifyStatusApi/ReadTxPduNotifyStatusApi` 门控。
- Trcv 唤醒标志与 PN：依赖底层 CanTrcv 能力，CanIf 维护「请求挂起/确认」状态并在 Indication 回调中通知上层；底层不支持时返回 E_NOT_OK + DET `CANIF_E_OPER_NOT_SUPPORTED`（复用现有错误码族）。
- MetaData：实现 `CanIf_CanIdToMetaData/MetaDataToCanId`（uint32 CAN ID ↔ MetaData 高 16 位 SID/低 16 位 位置语义，与 SWS CanIf MetaData 布局一致）。

## AD4：CanSM 新 API 接入既有 BSM
- `StartWakeupSource/StopWakeupSource`：请求锁存 + MainFunction 驱动，进入/退出「控制器 STOPPED + 收发器 NORMAL」的唤醒验证态（对应 AUTOSAR wakeup source 时序的简化实现），与 CHECKWAKEUP 态复用。
- `SetEcuPassive(boolean)`：全局被动标志；被动期间 `RequestComMode(FULL)` 降级为 SILENTCOM。`SetNetworkPassive(network, boolean)` 为每网络覆盖标志（优先于全局）。
- `TxTimeoutException`：按 AUTOSAR 语义触发 BOR 路径（进入 SILENTCOM_BOR）。
- `TransceiverModeIndication/CheckTransceiverWakeFlagIndication`：更新运行时收发器模式/清除挂起标志。

## AD5：CanTp Rx 队列与 MetaData（内部机制）
- Rx 帧队列：静态环形队列（深度 4，单通道规模），`CanTp_RxIndication` 仅入队（队列满 → `Det_ReportRuntimeError(CANTP_E_RX_COM)` 并丢弃），`CanTp_MainFunction` 出队后走既有处理路径——保证 ISR 上下文不做协议处理。
- MetaData：MIXED/NORMALFIXED 寻址时从 `PduInfoPtr->MetaDataPtr` 提取目标地址保存至 NSDU 运行时，发送路径按同布局构造。
- FF/SF-DL 校验：抽取 `CanTp_CheckRxFrameDL` 静态助手统一 SF/FF/CF/FC 长度校验（CAN FD 长度表 + padding 规则），替换分散的长度检查。

## 测试策略
- 每模块在既有测试文件中新增用例（Unity 框架、自带 Det mock 约定不变）：新 API 的 DET 守卫、委托交互（mock/stub 底层）、状态机迁移、队列满/空边界、MetaData 往返。
- 完成标准：`ctest --test-dir build-native` 全绿（基线 104 + 新增）。
