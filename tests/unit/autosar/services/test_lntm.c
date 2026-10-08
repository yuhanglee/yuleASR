/* 
 * @file test_lntm.c
 * @brief LNTM (LinTp) 模块单元测试
 */

// @tests src/bsw/ecual/linTp/src/LinTp.c  @tests src/bsw/ecual/linTp/include/LinTp.h

#include <unity.h>
#include <string.h>
#include "lntm.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_Tm_00001 */
void test_lntm_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash */
    LinTp_Init(NULL_PTR);
    /* Init with valid config */
    LinTp_Init(&LinTp_Config);
    TEST_ASSERT_TRUE(1);
}

void test_lntm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    LinTp_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(LINTP_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(LINTP_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(LINTP_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(LINTP_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(LINTP_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_lntm_Init_should_initialize);
    RUN_TEST(test_lntm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
