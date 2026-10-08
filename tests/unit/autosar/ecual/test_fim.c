/* 
 * @file test_fim.c
 * @brief FIM 模块单元测试
 */

// @tests src/bsw/ecual/fim/src/FiM.c  @tests src/bsw/ecual/fim/include/FiM.h

#include <unity.h>
#include <string.h>
#include "FiM.h"

void setUp(void) {}
void tearDown(void) {}

void test_fim_Init_should_initialize(void) {
    /* FiM_Init is void; call with NULL to exercise the code path */
    FiM_Init(NULL_PTR);
    /* Verify module is callable after init attempt - try DeInit */
    FiM_DeInit();
    TEST_PASS();
}

void test_fim_Init_with_null_should_not_crash(void) {
    FiM_Init(NULL_PTR);
    TEST_PASS();
}

void test_fim_DeInit_should_not_crash(void) {
    FiM_DeInit();
    TEST_PASS();
}

void test_fim_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    FiM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(FIM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(FIM_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(FIM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(FIM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(FIM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

void test_fim_GetVersionInfo_with_null_should_not_crash(void) {
    FiM_GetVersionInfo(NULL_PTR);
    TEST_PASS();
}

void test_fim_GetFunctionPermission_uninit_should_return_error(void) {
    boolean permission = FALSE;
    Std_ReturnType ret = FiM_GetFunctionPermission(0, &permission);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_fim_SetFunctionAvailable_uninit_should_return_error(void) {
    Std_ReturnType ret = FiM_SetFunctionAvailable(0, TRUE);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_fim_DemTriggerOnMonitorStatus_should_not_crash(void) {
    FiM_DemTriggerOnMonitorStatus(0);
    TEST_PASS();
}

void test_fim_DemTriggerOnEventStatus_should_not_crash(void) {
    FiM_DemTriggerOnEventStatus(0);
    TEST_PASS();
}

void test_fim_MainFunction_should_not_crash(void) {
    FiM_MainFunction();
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fim_Init_should_initialize);
    RUN_TEST(test_fim_Init_with_null_should_not_crash);
    RUN_TEST(test_fim_DeInit_should_not_crash);
    RUN_TEST(test_fim_GetVersionInfo_should_return_version);
    RUN_TEST(test_fim_GetVersionInfo_with_null_should_not_crash);
    RUN_TEST(test_fim_GetFunctionPermission_uninit_should_return_error);
    RUN_TEST(test_fim_SetFunctionAvailable_uninit_should_return_error);
    RUN_TEST(test_fim_DemTriggerOnMonitorStatus_should_not_crash);
    RUN_TEST(test_fim_DemTriggerOnEventStatus_should_not_crash);
    RUN_TEST(test_fim_MainFunction_should_not_crash);
    return UNITY_END();
}
