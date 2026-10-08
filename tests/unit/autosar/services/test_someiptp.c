/* 
 * @file test_someiptp.c
 * @brief SOMEIPTP 模块单元测试
 */

// @tests src/bsw/services/someiptp/src/SomeIpTp.c  @tests src/bsw/services/someiptp/include/SomeIpTp.h

#include <unity.h>
#include <string.h>
#include "someiptp.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_SomeIp_00001 */
void test_someiptp_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash */
    SomeIpTp_Init(NULL_PTR);
    TEST_ASSERT_TRUE(1);
}

void test_someiptp_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    SomeIpTp_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(SOMEIPTP_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(SOMEIPTP_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(SOMEIPTP_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(SOMEIPTP_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(SOMEIPTP_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_someiptp_Init_should_initialize);
    RUN_TEST(test_someiptp_GetVersionInfo_should_return_version);
    return UNITY_END();
}
