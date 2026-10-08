/* 
 * @file test_stbm.c
 * @brief STBM 模块单元测试
 */

// @tests src/bsw/services/stbm/src/StbM.c  @tests src/bsw/services/stbm/include/StbM.h

#include <unity.h>
#include <string.h>
#include "stbm.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_StbM_00001 */
void test_stbm_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash */
    StbM_Init(NULL_PTR);
    TEST_ASSERT_TRUE(1);
}

void test_stbm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    StbM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(STBM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(STBM_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(STBM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(STBM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(STBM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_stbm_Init_should_initialize);
    RUN_TEST(test_stbm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
