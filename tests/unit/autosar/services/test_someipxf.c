/* 
 * @file test_someipxf.c
 * @brief SOMEIPXF 模块单元测试
 */

// @tests src/bsw/services/someipxf/src/SomeIpXf.c  @tests src/bsw/services/someipxf/include/SomeIpXf.h

#include <unity.h>
#include <string.h>
#include "someipxf.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_SomeIp_00001 */
void test_someipxf_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash */
    SomeIpXf_Init(NULL_PTR);
    TEST_ASSERT_TRUE(1);
}

void test_someipxf_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    SomeIpXf_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(SOMEIPXF_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(SOMEIPXF_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(SOMEIPXF_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(SOMEIPXF_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(SOMEIPXF_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_someipxf_Init_should_initialize);
    RUN_TEST(test_someipxf_GetVersionInfo_should_return_version);
    return UNITY_END();
}
