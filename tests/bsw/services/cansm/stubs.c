/*
 * Test stubs for CanSM unit tests.
 *
 * CanSm.c (single translation unit, static-lib link model) references the
 * CanIf controller/PDU/transceiver mode APIs from its state-machine helpers.
 * The real CanIf static library is not linked into this test, so minimal
 * stubs with the exact prototypes from CanIf.h are provided here. The stubs
 * record the last request arguments so tests can verify the CanSM ->
 * CanIf interactions (mode requests of the state machine).
 */
#include "CanIf.h"

/* Recorded CanIf interactions (reset via CanSM_TestStubs_Reset) */
uint8 CanIfStub_SetControllerMode_CallCount = 0U;
uint8 CanIfStub_SetControllerMode_LastControllerId = 0xFFU;
CanIf_ControllerModeType CanIfStub_SetControllerMode_LastMode = CANIF_CS_UNINIT;

uint8 CanIfStub_SetPduMode_CallCount = 0U;
uint8 CanIfStub_SetPduMode_LastControllerId = 0xFFU;
CanIf_PduModeType CanIfStub_SetPduMode_LastMode = CANIF_OFFLINE;

uint8 CanIfStub_SetTrcvMode_CallCount = 0U;
uint8 CanIfStub_SetTrcvMode_LastTransceiverId = 0xFFU;
CanIf_TransceiverModeType CanIfStub_SetTrcvMode_LastMode = CANIF_TRCV_MODE_SLEEP;

void CanSM_TestStubs_Reset(void)
{
    CanIfStub_SetControllerMode_CallCount = 0U;
    CanIfStub_SetControllerMode_LastControllerId = 0xFFU;
    CanIfStub_SetControllerMode_LastMode = CANIF_CS_UNINIT;

    CanIfStub_SetPduMode_CallCount = 0U;
    CanIfStub_SetPduMode_LastControllerId = 0xFFU;
    CanIfStub_SetPduMode_LastMode = CANIF_OFFLINE;

    CanIfStub_SetTrcvMode_CallCount = 0U;
    CanIfStub_SetTrcvMode_LastTransceiverId = 0xFFU;
    CanIfStub_SetTrcvMode_LastMode = CANIF_TRCV_MODE_SLEEP;
}

Std_ReturnType CanIf_SetControllerMode(uint8 ControllerId, CanIf_ControllerModeType ControllerMode)
{
    CanIfStub_SetControllerMode_CallCount++;
    CanIfStub_SetControllerMode_LastControllerId = ControllerId;
    CanIfStub_SetControllerMode_LastMode = ControllerMode;
    return E_OK;
}

Std_ReturnType CanIf_SetPduMode(uint8 ControllerId, CanIf_PduModeType PduModeRequest)
{
    CanIfStub_SetPduMode_CallCount++;
    CanIfStub_SetPduMode_LastControllerId = ControllerId;
    CanIfStub_SetPduMode_LastMode = PduModeRequest;
    return E_OK;
}

Std_ReturnType CanIf_SetTrcvMode(uint8 TransceiverId, CanIf_TransceiverModeType TransceiverMode)
{
    CanIfStub_SetTrcvMode_CallCount++;
    CanIfStub_SetTrcvMode_LastTransceiverId = TransceiverId;
    CanIfStub_SetTrcvMode_LastMode = TransceiverMode;
    return E_OK;
}
