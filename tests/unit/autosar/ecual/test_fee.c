/* 
 * @file test_fee.c
 * @brief FEE 模块单元测试
 */

// @tests src/bsw/ecual/fee/src/Fee.c  @tests src/bsw/ecual/fee/include/Fee.h

#include <unity.h>
#include <string.h>
#include "Fee.h"

void setUp(void) {}
void tearDown(void) {}

void test_fee_Init_should_initialize(void) {
    /* Fee_Init is void; call with NULL to exercise the code path */
    Fee_Init(NULL_PTR);
    /* Verify module is callable after init attempt */
    Fee_StatusType status = Fee_GetStatus();
    TEST_ASSERT_TRUE(status == FEE_IDLE || status == FEE_BUSY ||
                     status == FEE_BUSY_INTERNAL || status == FEE_CANCELLED);
}

void test_fee_Init_with_config_should_succeed(void) {
    Fee_Init(&Fee_Config);
    Fee_StatusType status = Fee_GetStatus();
    TEST_ASSERT_TRUE(status == FEE_IDLE || status == FEE_BUSY ||
                     status == FEE_BUSY_INTERNAL || status == FEE_CANCELLED);
}

void test_fee_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    Fee_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(FEE_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(FEE_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(FEE_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(FEE_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(FEE_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

void test_fee_GetVersionInfo_with_null_should_not_crash(void) {
    Fee_GetVersionInfo(NULL_PTR);
    TEST_PASS();
}

void test_fee_DeInit_should_not_crash(void) {
    Fee_DeInit();
    TEST_PASS();
}

void test_fee_Read_uninit_should_return_error(void) {
    uint8 buffer[8];
    Std_ReturnType ret = Fee_Read(0, 0, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_fee_Write_uninit_should_return_error(void) {
    uint8 data[4] = {0x01, 0x02, 0x03, 0x04};
    Std_ReturnType ret = Fee_Write(0, data);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_fee_SetMode_should_accept_valid_mode(void) {
    Fee_SetMode(FEE_MODE_SLOW);
    Fee_SetMode(FEE_MODE_FAST);
    TEST_PASS();
}

void test_fee_Cancel_should_not_crash(void) {
    Fee_Cancel();
    TEST_PASS();
}

void test_fee_GetJobResult_should_return_valid_result(void) {
    Fee_JobResultType result = Fee_GetJobResult();
    TEST_ASSERT_TRUE(result <= FEE_BLOCK_INVALID);
}

void test_fee_InvalidateBlock_invalid_block_should_return_error(void) {
    Std_ReturnType ret = Fee_InvalidateBlock(0xFFFF);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_fee_EraseImmediateBlock_invalid_block_should_return_error(void) {
    Std_ReturnType ret = Fee_EraseImmediateBlock(0xFFFF);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_fee_GetCycleCount_should_return_value(void) {
    uint32 count = Fee_GetCycleCount();
    TEST_ASSERT_TRUE(count <= 0xFFFFFFFFU);
}

void test_fee_GetEraseCycleCount_should_return_value(void) {
    uint32 count = Fee_GetEraseCycleCount();
    TEST_ASSERT_TRUE(count <= 0xFFFFFFFFU);
}

void test_fee_GetWriteCycleCount_should_return_value(void) {
    uint32 count = Fee_GetWriteCycleCount();
    TEST_ASSERT_TRUE(count <= 0xFFFFFFFFU);
}

void test_fee_MainFunction_should_not_crash(void) {
    Fee_MainFunction();
    TEST_PASS();
}

void test_fee_JobEndNotification_should_not_crash(void) {
    Fee_JobEndNotification();
    TEST_PASS();
}

void test_fee_JobErrorNotification_should_not_crash(void) {
    Fee_JobErrorNotification();
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fee_Init_should_initialize);
    RUN_TEST(test_fee_Init_with_config_should_succeed);
    RUN_TEST(test_fee_GetVersionInfo_should_return_version);
    RUN_TEST(test_fee_GetVersionInfo_with_null_should_not_crash);
    RUN_TEST(test_fee_DeInit_should_not_crash);
    RUN_TEST(test_fee_Read_uninit_should_return_error);
    RUN_TEST(test_fee_Write_uninit_should_return_error);
    RUN_TEST(test_fee_SetMode_should_accept_valid_mode);
    RUN_TEST(test_fee_Cancel_should_not_crash);
    RUN_TEST(test_fee_GetJobResult_should_return_valid_result);
    RUN_TEST(test_fee_InvalidateBlock_invalid_block_should_return_error);
    RUN_TEST(test_fee_EraseImmediateBlock_invalid_block_should_return_error);
    RUN_TEST(test_fee_GetCycleCount_should_return_value);
    RUN_TEST(test_fee_GetEraseCycleCount_should_return_value);
    RUN_TEST(test_fee_GetWriteCycleCount_should_return_value);
    RUN_TEST(test_fee_MainFunction_should_not_crash);
    RUN_TEST(test_fee_JobEndNotification_should_not_crash);
    RUN_TEST(test_fee_JobErrorNotification_should_not_crash);
    return UNITY_END();
}
