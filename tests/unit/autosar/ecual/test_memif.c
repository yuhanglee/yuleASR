/* 
 * @file test_memif.c
 * @brief MEMIF 模块单元测试
 */

// @tests src/bsw/ecual/memif/src/MemIf.c  @tests src/bsw/ecual/memif/include/MemIf.h

#include <unity.h>
#include <string.h>
#include "MemIf.h"

void setUp(void) {}
void tearDown(void) {}

void test_memif_Init_should_initialize(void) {
    /* MemIf_Init is void; call with NULL to exercise the code path */
    MemIf_Init(NULL_PTR);
    /* Verify module is callable after init attempt */
    MemIf_StatusType status = MemIf_GetStatus(0);
    TEST_ASSERT_TRUE(status == MEMIF_IDLE || status == MEMIF_BUSY || status == MEMIF_BUSY_INTERNAL);
}

void test_memif_Init_with_config_should_succeed(void) {
    MemIf_Init(&MemIf_Config);
    MemIf_StatusType status = MemIf_GetStatus(0);
    TEST_ASSERT_TRUE(status == MEMIF_IDLE || status == MEMIF_BUSY || status == MEMIF_BUSY_INTERNAL);
}

void test_memif_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    MemIf_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(MEMIF_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(MEMIF_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(MEMIF_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(MEMIF_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(MEMIF_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

void test_memif_GetVersionInfo_with_null_should_not_crash(void) {
    MemIf_GetVersionInfo(NULL_PTR);
    TEST_PASS();
}

void test_memif_Read_uninit_should_return_error(void) {
    uint8 buffer[8];
    Std_ReturnType ret = MemIf_Read(0, 0, 0, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_memif_Write_uninit_should_return_error(void) {
    uint8 data[4] = {0x01, 0x02, 0x03, 0x04};
    Std_ReturnType ret = MemIf_Write(0, 0, data);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_memif_Cancel_should_not_crash(void) {
    MemIf_Cancel(0);
    TEST_PASS();
}

void test_memif_GetJobResult_should_return_valid_result(void) {
    MemIf_JobResultType result = MemIf_GetJobResult(0);
    TEST_ASSERT_TRUE(result <= MEMIF_BLOCK_INVALID);
}

void test_memif_InvalidateBlock_invalid_should_return_error(void) {
    Std_ReturnType ret = MemIf_InvalidateBlock(0, 0xFFFF);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_memif_EraseImmediateBlock_invalid_should_return_error(void) {
    Std_ReturnType ret = MemIf_EraseImmediateBlock(0, 0xFFFF);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_memif_SetMode_should_accept_valid_mode(void) {
    MemIf_SetMode(0, MEMIF_MODE_SLOW);
    MemIf_SetMode(0, MEMIF_MODE_FAST);
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_memif_Init_should_initialize);
    RUN_TEST(test_memif_Init_with_config_should_succeed);
    RUN_TEST(test_memif_GetVersionInfo_should_return_version);
    RUN_TEST(test_memif_GetVersionInfo_with_null_should_not_crash);
    RUN_TEST(test_memif_Read_uninit_should_return_error);
    RUN_TEST(test_memif_Write_uninit_should_return_error);
    RUN_TEST(test_memif_Cancel_should_not_crash);
    RUN_TEST(test_memif_GetJobResult_should_return_valid_result);
    RUN_TEST(test_memif_InvalidateBlock_invalid_should_return_error);
    RUN_TEST(test_memif_EraseImmediateBlock_invalid_should_return_error);
    RUN_TEST(test_memif_SetMode_should_accept_valid_mode);
    return UNITY_END();
}
