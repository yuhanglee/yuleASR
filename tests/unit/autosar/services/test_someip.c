/* 
 * @file test_someip.c
 * @brief SOMEIP 模块单元测试
 */

// @tests src/bsw/services/someip/src/SomeIp.c  @tests src/bsw/services/someip/include/SomeIp.h

#include <unity.h>
#include <string.h>
#include "someip.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_SomeIp_00001 */
void test_someip_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash */
    SomeIp_Init(NULL_PTR);
    TEST_ASSERT_TRUE(1);
}

void test_someip_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    SomeIp_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(SOMEIP_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(SOMEIP_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(SOMEIP_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_someip_Init_should_initialize);
    RUN_TEST(test_someip_GetVersionInfo_should_return_version);
    return UNITY_END();
}
