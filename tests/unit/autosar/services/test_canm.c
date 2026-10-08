/* 
 * @file test_canm.c
 * @brief CANM 模块单元测试
 */

// @tests src/bsw/ecual/canNm/src/CanNm.c  @tests src/bsw/ecual/canNm/include/CanNm.h

#include <unity.h>
#include "canm.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_CanNm_00001 */
void test_canm_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (DET reports error internally) */
    CanNm_Init(NULL_PTR);
    /* If we reach here without hard fault, the guard works */
    TEST_PASS();
}

void test_canm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    CanNm_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(CANNM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(CANNM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(CANNM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(CANNM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_canm_Init_should_initialize);
    RUN_TEST(test_canm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
