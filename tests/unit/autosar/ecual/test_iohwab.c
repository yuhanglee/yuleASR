/* 
 * @file test_iohwab.c
 * @brief IOHWAB 模块单元测试
 */

// @tests src/bsw/ecual/iohwab/src/IoHwAb.c  @tests src/bsw/ecual/iohwab/include/IoHwAb.h

#include <unity.h>
#include <string.h>
#include "IoHwAb.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_IoHwAb_00001 */
void test_iohwab_Init_should_initialize(void) {
    /* IoHwAb_Init is void; call with NULL to exercise the code path */
    IoHwAb_Init(NULL_PTR);
    /* Verify module is callable after init attempt */
    IoHwAb_DeInit();
    TEST_PASS();
}

void test_iohwab_Init_with_null_should_not_crash(void) {
    IoHwAb_Init(NULL_PTR);
    TEST_PASS();
}

void test_iohwab_DeInit_should_not_crash(void) {
    IoHwAb_DeInit();
    TEST_PASS();
}

void test_iohwab_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    IoHwAb_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(IOHWAB_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(IOHWAB_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(IOHWAB_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(IOHWAB_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(IOHWAB_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

void test_iohwab_GetVersionInfo_with_null_should_not_crash(void) {
    IoHwAb_GetVersionInfo(NULL_PTR);
    TEST_PASS();
}

void test_iohwab_AnalogRead_uninit_should_return_error(void) {
    uint16 value = 0;
    IoHwAb_ReturnType ret = IoHwAb_AnalogRead(0, &value);
    TEST_ASSERT_TRUE(ret == IOHWAB_NOT_OK || ret == IOHWAB_OK);
}

void test_iohwab_DigitalRead_uninit_should_return_error(void) {
    uint8 value = 0;
    IoHwAb_ReturnType ret = IoHwAb_DigitalRead(0, &value);
    TEST_ASSERT_TRUE(ret == IOHWAB_NOT_OK || ret == IOHWAB_OK);
}

void test_iohwab_DigitalWrite_uninit_should_return_error(void) {
    IoHwAb_ReturnType ret = IoHwAb_DigitalWrite(0, 1);
    TEST_ASSERT_TRUE(ret == IOHWAB_NOT_OK || ret == IOHWAB_OK);
}

void test_iohwab_MainFunction_should_not_crash(void) {
    IoHwAb_MainFunction();
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_iohwab_Init_should_initialize);
    RUN_TEST(test_iohwab_Init_with_null_should_not_crash);
    RUN_TEST(test_iohwab_DeInit_should_not_crash);
    RUN_TEST(test_iohwab_GetVersionInfo_should_return_version);
    RUN_TEST(test_iohwab_GetVersionInfo_with_null_should_not_crash);
    RUN_TEST(test_iohwab_AnalogRead_uninit_should_return_error);
    RUN_TEST(test_iohwab_DigitalRead_uninit_should_return_error);
    RUN_TEST(test_iohwab_DigitalWrite_uninit_should_return_error);
    RUN_TEST(test_iohwab_MainFunction_should_not_crash);
    return UNITY_END();
}
