/* 
 * @file test_linm.c
 * @brief LINM 模块单元测试
 */

// @tests src/bsw/services/linm/src/LinM.c  @tests src/bsw/services/linm/include/LinM.h

#include <unity.h>
#include "linm.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_LinM_00001 */
void test_linm_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (DET reports error internally) */
    LinM_Init(NULL_PTR);
    TEST_PASS();
}

void test_linm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    LinM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(LINM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(LINM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(LINM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(LINM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_linm_Init_should_initialize);
    RUN_TEST(test_linm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
