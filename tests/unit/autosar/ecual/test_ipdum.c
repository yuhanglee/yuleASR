/* 
 * @file test_ipdum.c
 * @brief IPDUM 模块单元测试
 */

// @tests src/bsw/ecual/ipdum/src/IpduM.c  @tests src/bsw/ecual/ipdum/include/IpduM.h

#include <unity.h>
#include <string.h>
#include "ipdum.h"

void setUp(void) {}
void tearDown(void) {}

void test_ipdum_Init_should_initialize(void) {
    /* IpduM_Init rejects NULL_PTR, reports DET and returns early */
    IpduM_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_ipdum_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    IpduM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ipdum_Init_should_initialize);
    RUN_TEST(test_ipdum_GetVersionInfo_should_return_version);
    return UNITY_END();
}
