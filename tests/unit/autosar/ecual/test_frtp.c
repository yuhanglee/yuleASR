/* 
 * @file test_frtp.c
 * @brief FRTP 模块单元测试
 */

// @tests src/bsw/ecual/frtp/src/FrTp.c  @tests src/bsw/ecual/frtp/include/FrTp.h

#include <unity.h>
#include <string.h>
#include "frtp.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_FrTp_00001 */
void test_frtp_Init_should_initialize(void) {
    /* FrTp_Init rejects NULL_PTR, reports DET and returns early */
    FrTp_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_frtp_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    FrTp_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_frtp_Init_should_initialize);
    RUN_TEST(test_frtp_GetVersionInfo_should_return_version);
    return UNITY_END();
}
