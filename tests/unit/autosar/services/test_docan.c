/* 
 * @file test_docan.c
 * @brief DOCAN 模块单元测试
 */

// @tests src/bsw/services/docan/src/DoCan.c  @tests src/bsw/services/docan/include/DoCan.h

#include <unity.h>
#include "docan.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_DoCan_00001 */
void test_docan_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (DET reports error internally) */
    DoCan_Init(NULL_PTR);
    TEST_PASS();
}

void test_docan_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    DoCan_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(DOCAN_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(DOCAN_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(DOCAN_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(DOCAN_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_docan_Init_should_initialize);
    RUN_TEST(test_docan_GetVersionInfo_should_return_version);
    return UNITY_END();
}
