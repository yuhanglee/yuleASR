/* 
 * @file test_schm.c
 * @brief SCHM 模块单元测试
 */

// @tests src/bsw/services/schm/src/SchM.c  @tests src/bsw/services/schm/include/SchM.h

#include <unity.h>
#include <string.h>
#include "schm.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_SchM_00001 */
void test_schm_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash */
    SchM_Init(NULL_PTR);
    TEST_ASSERT_TRUE(1);
}

void test_schm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    SchM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(SCHM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(SCHM_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(SCHM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(SCHM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(SCHM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_schm_Init_should_initialize);
    RUN_TEST(test_schm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
