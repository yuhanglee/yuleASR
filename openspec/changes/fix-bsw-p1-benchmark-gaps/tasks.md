# Tasks: fix-bsw-p1-benchmark-gaps

> 依据：`references/benchmark-compare-2026-09-28.md` §5 P1 清单。完成标准：每模块测试新增并通过；全量 `ctest --test-dir build-native` 全绿。

## 1. BswM 联动回调族（services/bswm）
- [x] 1.1 BswM.h：镜像类型 + 12 个新回调声明 + SID/错误码
- [x] 1.2 BswM.c：`BswM_WritePortByComposition` 内部助手 + 回调实现（UNINIT DET 守卫）
- [x] 1.3 BswM_Lcfg.c：端口表 3→16（追加 composition，索引 0-2 不变）
- [x] 1.4 测试：每回调端口写入 + DET 守卫 + 规则触发

## 2. LinIf 收发器/节点配置（ecual/linif）
- [x] 2.1 LinIf.h：Trcv 族 + NAD/PID 族 + IsSupportTpTransmit 声明
- [x] 2.2 LinIf.c：委托 LinTrcv + 通道运行时 NAD/PID 状态
- [x] 2.3 测试：委托交互 + 参数校验 + PID 表边界

## 3. CanIf 错误面/PN/Trcv/NotifStatus（ecual/canif + mcal/can）
- [x] 3.1 mcal/Can：Can_GetControllerErrorState/RxErrorCounter/TxErrorCounter + 运行时错误状态
- [x] 3.2 CanIf：错误计数三 API（委托 + DET 守卫）
- [x] 3.3 CanIf：ReadRx/TxNotifStatus + 通知状态跟踪
- [x] 3.4 CanIf：TriggerTransmit + ConfirmPnAvailability + CheckTrcvWakeFlag(+Ind) + ClearTrcvWufFlag(+Ind)
- [x] 3.5 CanIf：ControllerModeIndication/TrcvModeIndication 上层回调 + CanIdToMetaData/MetaDataToCanId
- [x] 3.6 测试：上述全部 + 既有用例不回退

## 4. CanSM 唤醒源/被动网络（services/cansm）
- [x] 4.1 CanSM_StartWakeupSource/StopWakeupSource（BSM 接入）
- [x] 4.2 CanSM_SetEcuPassive/SetNetworkPassive（被动降级语义）
- [x] 4.3 CanSM_TxTimeoutException（BOR 路径）
- [x] 4.4 CanSM_TransceiverModeIndication/CheckTransceiverWakeFlagIndication
- [x] 4.5 测试：迁移 + 标志语义

## 5. CanTp Rx 队列/MetaData（ecual/cantp）
- [x] 5.1 Rx 帧队列（RxIndication 入队 / MainFunction 出队）+ 队列满运行时报错
- [x] 5.2 MetaData 保存/构造（MIXED/NORMALFIXED）
- [x] 5.3 FF/SF-DL 校验助手统一
- [x] 5.4 测试：队列边界 + MetaData 往返 + 长度校验

## 6. 收尾
- [x] 6.1 全量回归（ctest 全绿）：/tmp/build-p1-full，106/106 通过（含新增 bswm/linif/canif/cansm/cantp 用例）；s0_smoke_test 补链 service_cansm 后链接通过
- [x] 6.2 符号面复算（api-surface-diff vs /tmp/emx_hdrs_20260928）并更新对标报告 §5 P1 状态
- [x] 6.3 proposal/design/tasks 证据回填，提交评审

## 进度统计
- 总任务数：6 组 / 21 项
- 已完成：21
- 完成率：100%

## 证据索引
- 全量回归：`ctest --test-dir /tmp/build-p1-full` → 100% tests passed, 0 failed out of 106（36.39s）
- 符号面（theirs_only 余量）：CanSM 0；CanTp 35（全为内部助手/宏）；CanIf 59（含 1 个公开 API CanIf_ReadRxPduData，非 P1 范围）；BswM 98（含 BswM_DeInit 命名变体 DeInit/Deinit，其余为 DetChk*/Init*RequestPorts 内部助手与 SD/J1939/SoAd/Swc 排除项）；LinIf 122（LinTp_* 为整模块缺口，其余为 Master/Slave 内部助手）
- 模块测试：tests/bsw/services/bswm、tests/bsw/ecual/linif、tests/bsw/ecual/canif、tests/bsw/mcal/can、tests/bsw/services/cansm、tests/bsw/ecual/cantp 全绿
