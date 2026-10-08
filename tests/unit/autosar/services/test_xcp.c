/* 
 * @file test_xcp.c
 * @brief XCP 模块单元测试
 */

// @tests src/bsw/services/xcp/src/Xcp.c  @tests src/bsw/services/xcp/include/Xcp.h

#include <unity.h>
#include <string.h>
#include "xcp.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_Xcp_00001 */
void test_xcp_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash */
    Xcp_Init(NULL_PTR);
    TEST_ASSERT_TRUE(1);
}

void test_xcp_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    Xcp_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(XCP_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(XCP_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(XCP_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(XCP_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(XCP_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_xcp_Init_should_initialize);
    RUN_TEST(test_xcp_GetVersionInfo_should_return_version);
    return UNITY_END();
}
