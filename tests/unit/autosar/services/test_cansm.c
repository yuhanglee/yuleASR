/* 
 * @file test_cansm.c
 * @brief CANSM 模块单元测试
 */

// @tests src/bsw/services/cansm/src/CanSm.c  @tests src/bsw/services/cansm/include/CanSm.h

#include <unity.h>
#include "cansm.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_cansm_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (DET reports error internally) */
    CanSM_Init(NULL_PTR);
    TEST_PASS();
}

void test_cansm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    CanSM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(CANSM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(CANSM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(CANSM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(CANSM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_cansm_Init_should_initialize);
    RUN_TEST(test_cansm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
