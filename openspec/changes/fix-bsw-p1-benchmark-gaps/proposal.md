# Change: fix-bsw-p1-benchmark-gaps

## Status
Draft — 2026-09-28

## Why
对标复查（`references/benchmark-compare-2026-09-28.md`）确认 P0 已全部关闭，剩余差距为 P1「扩展联动面」。本变更按该清单逐模块落地，使 BswM/LinIf/CanIf/CanSM/CanTp 的公开 API 面与 EasyXMen 命名对齐，并补齐多 LIN 子节点车窗控制器场景所需的收发器/节点配置能力。

范围纪律：仅落地报告中列明的 P1 面；EMX 内部助手（`BswM_DetChk*`、`CanIf_Validate*`、`CanTp_MemCpy` 等）与 SD/J1939/SoAd 回调族不在本次范围；符号面 ≠ 功能符合度，落地后需逐模块测试佐证。

## What Changes
- **BswM**（services/bswm）：新增联动通知回调族 `BswM_ComM_CurrentMode/CurrentPNCMode`、`BswM_Dcm_CommunicationMode_CurrentState/ApplicationUpdated`、`BswM_Nm_StateChangeNotification/CarWakeUpIndication`、`BswM_CanSM/EthSM/FrSM/LinSM_CurrentState`、`BswM_LinSM_CurrentSchedule`、`BswM_LinTp_RequestMode`、`BswM_EcuM_RequestedState`、`BswM_BswMPartitionRestarted`、`BswM_EthIf_PortGroupLinkStateChg`；端口表扩展（既有索引 0-2 不变），通知状态以原始值写入端口、不受仲裁模式域 0..7 约束。
- **LinIf**（ecual/linif）：新增收发器控制族 `LinIf_SetTrcvMode/GetTrcvMode/SetTrcvWakeupMode/GetTrcvWakeupReason`（委托 LinTrcv）、`LinIf_CheckWakeup`、节点配置族 `LinIf_SetConfiguredNAD/GetConfiguredNAD/SetPIDTable/GetPIDTable`、Tp 能力查询 `LinIf_IsSupportTpTransmit`。
- **CanIf**（ecual/canif + mcal/can）：先扩展 `mcal/Can` 错误面 `Can_GetControllerErrorState/RxErrorCounter/TxErrorCounter`，再在 CanIf 落地 `CanIf_GetControllerErrorState/RxErrorCounter/TxErrorCounter`、`CanIf_ReadRxNotifStatus/ReadTxNotifStatus`（通知状态跟踪）、`CanIf_TriggerTransmit`、`CanIf_ConfirmPnAvailability`、`CanIf_CheckTrcvWakeFlag(+Indication)`、`CanIf_ClearTrcvWufFlag(+Indication)`、`CanIf_ControllerModeIndication/TrcvModeIndication` 上层回调、`CanIf_CanIdToMetaData/MetaDataToCanId`。
- **CanSM**（services/cansm）：新增 `CanSM_StartWakeupSource/StopWakeupSource`、`CanSM_SetEcuPassive`、`CanSM_SetNetworkPassive`、`CanSM_TransceiverModeIndication`、`CanSM_CheckTransceiverWakeFlagIndication`、`CanSM_TxTimeoutException`，接入既有 BSM。
- **CanTp**（ecual/cantp）：新增 Rx 帧队列（RxIndication 入队、MainFunction 出队处理，ISR 安全）、MetaData 保存/构造助手（MIXED/NORMALFIXED 寻址）、FF/SF-DL 校验助手，全部内部静态实现 + 行为测试。

## Impact
- 受影响代码：`src/bsw/{services/bswm,ecual/linif,ecual/canif,services/cansm,ecual/cantp,ecual/cantp, mcal/can}` 的 include/src 与 `tests/bsw` 对应测试。
- 非破坏性：既有公开 API 签名/语义不变；BswM 端口索引 0-2 与既有规则保持兼容。
- 测试：五模块新增单元测试；全量 `ctest` 回归必须全绿后方可评审。
