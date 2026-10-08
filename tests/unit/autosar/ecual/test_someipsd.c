/* 
 * @file test_someipsd.c
 * @brief SOMEIPSD 模块单元测试
 */

// @tests src/bsw/ecual/someipsd/src/SomeIpSd.c  @tests src/bsw/ecual/someipsd/include/SomeIpSd.h

#include <unity.h>
#include <string.h>
#include "someipsd.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_SomeIp_00001 */
void test_someipsd_Init_should_initialize(void) {
    /* SomeIpSd_Init accepts NULL_PTR and stores it as configPtr */
    SomeIpSd_Init(NULL_PTR);
    /* If we reach here without crash, init succeeded */
    TEST_PASS();
}

void test_someipsd_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    SomeIpSd_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_someipsd_Init_should_initialize);
    RUN_TEST(test_someipsd_GetVersionInfo_should_return_version);
    return UNITY_END();
}
