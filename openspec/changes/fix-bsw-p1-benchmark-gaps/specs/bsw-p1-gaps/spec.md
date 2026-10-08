# BSW P1 对标差距补齐（Delta）

## ADDED Requirements

### Requirement: BswM 联动通知回调族
BswM MUST 提供 ComM/Dcm/Nm/CanSM/EthSM/FrSM/LinSM/LinTp/EthIf/EcuM 联动通知回调与分区重启通知，将模块状态写入绑定的模式请求端口（原始值语义），未初始化时上报 DET 并忽略。

#### Scenario: ComM 模式通知写入端口
- **WHEN** BswM 已初始化且 ComM 端口已绑定
- **AND** 调用 `BswM_ComM_CurrentMode(Network, COMM_FULL_COMMUNICATION)`
- **THEN** ComM 端口值为 2，MainFunction 后匹配规则触发

#### Scenario: 未初始化通知被忽略并上报 DET
- **WHEN** BswM 未初始化
- **AND** 调用任一通知回调
- **THEN** 上报 `BSWM_E_UNINIT`，端口值不变

### Requirement: LinIf 收发器与节点配置
LinIf MUST 提供收发器控制族（委托 LinTrcv）、通道唤醒检查与 NAD/PID 节点配置，参数非法时返回 E_NOT_OK 并按需上报 DET。

#### Scenario: 收发器模式委托
- **WHEN** 通道已初始化
- **AND** 调用 `LinIf_SetTrcvMode(0, LINTRCV_TRCV_MODE_NORMAL)`
- **THEN** 委托 `LinTrcv_SetOpMode` 且返回 E_OK

#### Scenario: NAD 设置后回读一致
- **WHEN** 调用 `LinIf_SetConfiguredNAD(0, 0x60)` 成功
- **THEN** `LinIf_GetConfiguredNAD(0, &nad)` 返回 E_OK 且 nad == 0x60

### Requirement: CanIf 错误面/PN/Trcv 唤醒标志/通知状态
CanIf MUST 提供控制器错误状态/错误计数（委托 Can 驱动）、Tx/Rx 通知状态读取、TriggerTransmit、PN 可用性确认、Trcv 唤醒标志检查/清除及其上层指示回调；MetaData 与 CAN ID 可互转。

#### Scenario: 错误计数委托
- **WHEN** CanIf 已初始化
- **AND** 调用 `CanIf_GetControllerTxErrorCounter(0, &cnt)`
- **THEN** 返回 E_OK 且 cnt 来自 Can 驱动当前值

#### Scenario: Tx 通知状态在确认后置位
- **WHEN** `ReadTxPduNotifyStatusApi` 使能
- **AND** 对应 Tx PDU 完成 TxConfirmation
- **THEN** `CanIf_ReadTxNotifStatus` 返回 `CANIF_TX_RX_NOTIFICATION`

### Requirement: CanSM 唤醒源与被动网络
CanSM MUST 提供唤醒源启动/停止、ECU/网络被动模式、发送超时异常与收发器指示回调，并接入既有 BSM。

#### Scenario: 被动模式下降级请求
- **WHEN** `CanSM_SetEcuPassive(TRUE)` 已生效
- **AND** 请求 FULL_COMMUNICATION
- **THEN** 网络进入 SILENTCOM 而非 FULLCOM

#### Scenario: 唤醒源启动进入唤醒验证
- **WHEN** 网络处于 NOCOM
- **AND** 调用 `CanSM_StartWakeupSource(Network)`
- **THEN** MainFunction 驱动后网络进入唤醒验证态（控制器 STOPPED + 收发器 NORMAL）

### Requirement: CanTp Rx 队列与 MetaData
CanTp RxIndication MUST 将帧入队由 MainFunction 出队处理（队列满运行时报错并丢弃）；MIXED/NORMALFIXED 寻址支持 MetaData 保存与构造；SF/FF/CF/FC 长度校验经统一助手。

#### Scenario: 队列满丢弃并报运行时错误
- **WHEN** Rx 队列已满
- **AND** 再次收到 RxIndication
- **THEN** 上报 `CANTP_E_RX_COM` 且帧被丢弃，既有接收不受影响

#### Scenario: MetaData 往返一致
- **WHEN** NORMALFIXED 寻址 NSDU 携带 MetaData 接收
- **AND** 同通道发送路径构造 MetaData
- **THEN** 构造值与保存值一致
