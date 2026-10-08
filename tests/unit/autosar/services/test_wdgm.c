/* 
 * @file test_wdgm.c
 * @brief WDGM 模块单元测试
 */

// @tests src/bsw/services/wdgm/src/WdgM.c  @tests src/bsw/services/wdgm/include/WdgM.h

#include <unity.h>
#include <string.h>
#include "wdgm.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_WdgM_00001 */
void test_wdgm_Init_should_initialize(void) {
    /* Init with NULL_PTR should return E_NOT_OK */
    Std_ReturnType ret = WdgM_Init(NULL_PTR);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

/** @req SWS_WdgM_00020 */
void test_wdgm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    WdgM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(WDGM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(WDGM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(WDGM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(WDGM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_wdgm_Init_should_initialize);
    RUN_TEST(test_wdgm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
