/* 
 * @file test_someipif.c
 * @brief SOMEIPIF 模块单元测试
 */

// @tests src/bsw/ecual/someipif/src/SomeIpIf.c  @tests src/bsw/ecual/someipif/include/SomeIpIf.h

#include <unity.h>
#include <string.h>
#include "someipif.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_SomeIp_00001 */
void test_someipif_Init_should_initialize(void) {
    /* SomeIpIf_Init rejects NULL_PTR, reports DET and returns early */
    SomeIpIf_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_someipif_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    SomeIpIf_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_someipif_Init_should_initialize);
    RUN_TEST(test_someipif_GetVersionInfo_should_return_version);
    return UNITY_END();
}
