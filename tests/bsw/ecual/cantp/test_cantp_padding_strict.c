/**
 * @file test_cantp_padding_strict.c
 * @brief CanTp Rx padding validation - strict rejection build
 * @req SWS_CanTp
 *
 * Compiled with -DCANTP_REJECT_INVALID_PADDING=STD_ON: SF / CF frames whose
 * padding bytes deviate from CANTP_PADDING_BYTE_VALUE are reported (runtime
 * error) AND dropped instead of being forwarded to PduR. FC frames stay
 * exempt (their padding is sender-specific); classic FF frames carry no
 * padding area (2 PCI + 6 data bytes fill the frame exactly).
 */

// @tests src/bsw/ecual/cantp/src/CanTp.c  @tests src/bsw/ecual/cantp/include/CanTp.h
#include "unity.h"
#include "CanTp.h"
#include "CanTp_Cfg.h"

/* Guard: this binary must really be the strict variant */
#ifndef CANTP_REJECT_INVALID_PADDING
#error "test_cantp_padding_strict requires CANTP_REJECT_INVALID_PADDING to be defined"
#endif
#if (CANTP_REJECT_INVALID_PADDING != STD_ON)
#error "test_cantp_padding_strict must be compiled with CANTP_REJECT_INVALID_PADDING=STD_ON"
#endif

/* Lower-layer stubs. Signatures match CanIf.h / PduR.h / Det.h. */
static uint8 mock_DetCalls = 0;
static uint8 mock_RteCalls = 0;
static uint8 mock_RteLastApiId = 0;
static uint8 mock_RteLastErrorId = 0;
static uint8 mock_CanIfCalls = 0;
static PduIdType mock_CanIfPduId = 0xFFFFU;
static uint8 mock_CanIfFrame[CANTP_CAN_FRAME_LENGTH];
static uint8 mock_PduRRxCalls = 0;
static PduLengthType mock_PduRRxLens[8];
static uint8 mock_PduRRxFirstByte[8];
static uint8 mock_PduRLastData[16];
static PduLengthType mock_PduRLastLen = 0;

Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId) {
    (void)ModuleId;(void)InstanceId;(void)ApiId;(void)ErrorId;
    mock_DetCalls++; return E_OK;
}

Std_ReturnType Det_ReportRuntimeError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId) {
    (void)ModuleId;(void)InstanceId;
    mock_RteCalls++;
    mock_RteLastApiId = ApiId;
    mock_RteLastErrorId = ErrorId;
    return E_OK;
}

Std_ReturnType CanIf_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr) {
    mock_CanIfCalls++;
    mock_CanIfPduId = TxPduId;
    for (uint8 i = 0U; i < CANTP_CAN_FRAME_LENGTH; i++) {
        mock_CanIfFrame[i] = (i < PduInfoPtr->SduLength) ? PduInfoPtr->SduDataPtr[i] : 0U;
    }
    return E_OK;
}

void PduR_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr) {
    (void)RxPduId;
    if (mock_PduRRxCalls < 8U) {
        mock_PduRRxLens[mock_PduRRxCalls] = PduInfoPtr->SduLength;
        mock_PduRRxFirstByte[mock_PduRRxCalls] =
            (PduInfoPtr->SduLength > 0U) ? PduInfoPtr->SduDataPtr[0] : 0U;
    }
    for (PduLengthType i = 0U; (i < PduInfoPtr->SduLength) && (i < 16U); i++) {
        mock_PduRLastData[i] = PduInfoPtr->SduDataPtr[i];
    }
    mock_PduRLastLen = PduInfoPtr->SduLength;
    mock_PduRRxCalls++;
}

void PduR_TxConfirmation(PduIdType TxPduId, Std_ReturnType result) {
    (void)TxPduId;(void)result;
}

void setUp(void) {
    mock_DetCalls = 0;
    mock_RteCalls = 0;
    mock_RteLastApiId = 0;
    mock_RteLastErrorId = 0;
    mock_CanIfCalls = 0;
    mock_CanIfPduId = 0xFFFFU;
    mock_PduRRxCalls = 0;
    for (uint8 i = 0U; i < 8U; i++) { mock_PduRRxLens[i] = 0; mock_PduRRxFirstByte[i] = 0U; }
    for (uint8 i = 0U; i < 16U; i++) { mock_PduRLastData[i] = 0U; }
    mock_PduRLastLen = 0;
    for (uint8 i = 0U; i < CANTP_CAN_FRAME_LENGTH; i++) { mock_CanIfFrame[i] = 0U; }
}
void tearDown(void) {}

/* Strict mode: an SF whose padding area deviates from CANTP_PADDING_BYTE_VALUE
 * is reported AND dropped. */
/** @req SWS_CanTp_00009 */
void test_Strict_RxSfInvalidPadding_ShouldDropFrame(void) {
    uint8 sfData[8] = {0x02U, 0xAAU, 0xBBU, 0U, 0U, 0U, 0U, 0U}; /* tail 0x00 != 0xCC */
    PduInfoType sfPdu;
    sfPdu.SduDataPtr = sfData; sfPdu.MetaDataPtr = NULL_PTR; sfPdu.SduLength = 8U;

    CanTp_Init(&CanTp_Config);

    CanTp_RxIndication(CANTP_RX_DIAG_PHYSICAL, &sfPdu);
    CanTp_MainFunction();

    TEST_ASSERT_EQUAL(1, mock_RteCalls);
    TEST_ASSERT_EQUAL_HEX8(CANTP_SID_RXINDICATION, mock_RteLastApiId);
    TEST_ASSERT_EQUAL_HEX8(CANTP_E_RX_UNEXP_PADDING, mock_RteLastErrorId);
    TEST_ASSERT_EQUAL(0, mock_PduRRxCalls); /* dropped, not forwarded to PduR */
    TEST_ASSERT_EQUAL(0, mock_DetCalls);    /* runtime error, not a development error */
}

/* Strict mode: correctly padded SF frames are delivered as usual. */
/** @req SWS_CanTp_00009 */
void test_Strict_RxSfValidPadding_ShouldDeliver(void) {
    uint8 sfData[8] = {0x02U, 0xAAU, 0xBBU, 0xCCU, 0xCCU, 0xCCU, 0xCCU, 0xCCU};
    PduInfoType sfPdu;
    sfPdu.SduDataPtr = sfData; sfPdu.MetaDataPtr = NULL_PTR; sfPdu.SduLength = 8U;

    CanTp_Init(&CanTp_Config);

    CanTp_RxIndication(CANTP_RX_DIAG_PHYSICAL, &sfPdu);
    CanTp_MainFunction();

    TEST_ASSERT_EQUAL(0, mock_RteCalls);
    TEST_ASSERT_EQUAL(1, mock_PduRRxCalls);
    TEST_ASSERT_EQUAL(2, mock_PduRLastLen);
    TEST_ASSERT_EQUAL_HEX8(0xAAU, mock_PduRRxFirstByte[0]);
}

/* Strict mode: a frame with no padding area (SF_DL = 7 fills the frame) is
 * delivered even though the non-data bytes... there are none. */
/** @req SWS_CanTp_00009 */
void test_Strict_RxSfFullFrame_NoPaddingArea_ShouldDeliver(void) {
    uint8 sfData[8] = {0x07U, 0x11U, 0x22U, 0x33U, 0x44U, 0x55U, 0x66U, 0x77U};
    PduInfoType sfPdu;
    sfPdu.SduDataPtr = sfData; sfPdu.MetaDataPtr = NULL_PTR; sfPdu.SduLength = 8U;

    CanTp_Init(&CanTp_Config);

    CanTp_RxIndication(CANTP_RX_DIAG_PHYSICAL, &sfPdu);
    CanTp_MainFunction();

    TEST_ASSERT_EQUAL(0, mock_RteCalls);
    TEST_ASSERT_EQUAL(1, mock_PduRRxCalls);
    TEST_ASSERT_EQUAL(7, mock_PduRLastLen);
    TEST_ASSERT_EQUAL_HEX8(0x77U, mock_PduRLastData[6]);
}

/* Strict mode: a CF with bad padding is dropped, the reassembled message is
 * never delivered to PduR. The FF itself was accepted (FC CTS emitted). */
/** @req SWS_CanTp_00009 */
void test_Strict_RxCfInvalidPadding_ShouldDropMessage(void) {
    uint8 ffData[8] = {0x10U, 0x0AU, 0xA0U, 0xA1U, 0xA2U, 0xA3U, 0xA4U, 0xA5U}; /* FF, DL = 10 */
    uint8 cfData[8] = {0x21U, 0xA6U, 0xA7U, 0xA8U, 0xA9U, 0U, 0U, 0U};          /* CF SN = 1, bad padding */
    PduInfoType ffPdu;
    PduInfoType cfPdu;
    ffPdu.SduDataPtr = ffData; ffPdu.MetaDataPtr = NULL_PTR; ffPdu.SduLength = 8U;
    cfPdu.SduDataPtr = cfData; cfPdu.MetaDataPtr = NULL_PTR; cfPdu.SduLength = 8U;

    CanTp_Init(&CanTp_Config);

    CanTp_RxIndication(CANTP_RX_DIAG_PHYSICAL, &ffPdu);
    CanTp_MainFunction();
    TEST_ASSERT_EQUAL(1, mock_CanIfCalls); /* FC CTS: FF exempt from padding */

    CanTp_RxIndication(CANTP_RX_DIAG_PHYSICAL, &cfPdu);
    CanTp_MainFunction();

    TEST_ASSERT_EQUAL(1, mock_RteCalls);
    TEST_ASSERT_EQUAL_HEX8(CANTP_E_RX_UNEXP_PADDING, mock_RteLastErrorId);
    TEST_ASSERT_EQUAL(0, mock_PduRRxCalls); /* message not delivered */
}

/* Strict mode: a correctly padded CF completes the multi-frame reception. */
/** @req SWS_CanTp_00009 */
void test_Strict_RxCfValidPadding_ShouldDeliverMessage(void) {
    uint8 ffData[8] = {0x10U, 0x0AU, 0xA0U, 0xA1U, 0xA2U, 0xA3U, 0xA4U, 0xA5U}; /* FF, DL = 10 */
    uint8 cfData[8] = {0x21U, 0xA6U, 0xA7U, 0xA8U, 0xA9U, 0xCCU, 0xCCU, 0xCCU}; /* CF SN = 1 */
    PduInfoType ffPdu;
    PduInfoType cfPdu;
    ffPdu.SduDataPtr = ffData; ffPdu.MetaDataPtr = NULL_PTR; ffPdu.SduLength = 8U;
    cfPdu.SduDataPtr = cfData; cfPdu.MetaDataPtr = NULL_PTR; cfPdu.SduLength = 8U;

    CanTp_Init(&CanTp_Config);

    CanTp_RxIndication(CANTP_RX_DIAG_PHYSICAL, &ffPdu);
    CanTp_MainFunction();

    CanTp_RxIndication(CANTP_RX_DIAG_PHYSICAL, &cfPdu);
    CanTp_MainFunction();

    TEST_ASSERT_EQUAL(0, mock_RteCalls);
    TEST_ASSERT_EQUAL(1, mock_PduRRxCalls);
    TEST_ASSERT_EQUAL(10, mock_PduRLastLen);
    for (uint8 i = 0U; i < 10U; i++) {
        TEST_ASSERT_EQUAL_HEX8((uint8)(0xA0U + i), mock_PduRLastData[i]);
    }
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_Strict_RxSfInvalidPadding_ShouldDropFrame);
    RUN_TEST(test_Strict_RxSfValidPadding_ShouldDeliver);
    RUN_TEST(test_Strict_RxSfFullFrame_NoPaddingArea_ShouldDeliver);
    RUN_TEST(test_Strict_RxCfInvalidPadding_ShouldDropMessage);
    RUN_TEST(test_Strict_RxCfValidPadding_ShouldDeliverMessage);

    return UnityEnd();
}
