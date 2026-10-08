/* 
 * @file test_det.c
 * @brief DET 模块单元测试
 */

// @tests src/bsw/services/det/src/Det.c  @tests src/bsw/services/det/include/Det.h

#include <unity.h>
#include "det.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_Det_00001 */
void test_det_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (guards against NULL) */
    Det_Init(NULL_PTR);
    TEST_PASS();
}

/** @req SWS_Det_00006 */
void test_det_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    Det_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(DET_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(DET_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(DET_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(DET_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_det_Init_should_initialize);
    RUN_TEST(test_det_GetVersionInfo_should_return_version);
    return UNITY_END();
}
