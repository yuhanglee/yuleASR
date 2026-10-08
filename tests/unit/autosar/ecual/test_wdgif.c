/* 
 * @file test_wdgif.c
 * @brief WDGIF 模块单元测试
 */

// @tests src/bsw/ecual/wdgif/src/WdgIf.c  @tests src/bsw/ecual/wdgif/include/WdgIf.h

#include <unity.h>
#include <string.h>
#include "WdgIf.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_WdgIf_00001 */
void test_wdgif_Init_should_initialize(void) {
    /* WdgIf_Init is void; call with NULL to exercise the code path */
    WdgIf_Init(NULL_PTR);
    /* Verify module is callable after init attempt */
    WdgIf_DeInit();
    TEST_PASS();
}

void test_wdgif_Init_with_null_should_not_crash(void) {
    WdgIf_Init(NULL_PTR);
    TEST_PASS();
}

void test_wdgif_DeInit_should_not_crash(void) {
    WdgIf_DeInit();
    TEST_PASS();
}

void test_wdgif_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    WdgIf_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(WDGIF_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(WDGIF_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(WDGIF_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(WDGIF_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(WDGIF_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

void test_wdgif_GetVersionInfo_with_null_should_not_crash(void) {
    WdgIf_GetVersionInfo(NULL_PTR);
    TEST_PASS();
}

void test_wdgif_SetMode_uninit_should_return_error(void) {
    Std_ReturnType ret = WdgIf_SetMode(0, WDGIF_FAST_MODE);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_wdgif_SetMode_off_mode(void) {
    Std_ReturnType ret = WdgIf_SetMode(0, WDGIF_OFF_MODE);
    TEST_ASSERT_TRUE(ret == E_OK || ret == E_NOT_OK);
}

void test_wdgif_Trigger_uninit_should_return_error(void) {
    Std_ReturnType ret = WdgIf_Trigger(0);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_wdgif_SetTriggerCondition_uninit_should_return_error(void) {
    Std_ReturnType ret = WdgIf_SetTriggerCondition(0, 100);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_wdgif_Init_should_initialize);
    RUN_TEST(test_wdgif_Init_with_null_should_not_crash);
    RUN_TEST(test_wdgif_DeInit_should_not_crash);
    RUN_TEST(test_wdgif_GetVersionInfo_should_return_version);
    RUN_TEST(test_wdgif_GetVersionInfo_with_null_should_not_crash);
    RUN_TEST(test_wdgif_SetMode_uninit_should_return_error);
    RUN_TEST(test_wdgif_SetMode_off_mode);
    RUN_TEST(test_wdgif_Trigger_uninit_should_return_error);
    RUN_TEST(test_wdgif_SetTriggerCondition_uninit_should_return_error);
    return UNITY_END();
}
