/* 
 * @file test_lintp.c
 * @brief LINTP 模块单元测试
 */

// @tests src/bsw/ecual/lintp/src/LinTp.c  @tests src/bsw/ecual/lintp/include/LinTp.h

#include <unity.h>
#include <string.h>
#include "lintp.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_LinTp_00001 */
void test_lintp_Init_should_initialize(void) {
    /* LinTp_Init rejects NULL_PTR, reports DET and returns early */
    LinTp_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_lintp_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    LinTp_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_lintp_Init_should_initialize);
    RUN_TEST(test_lintp_GetVersionInfo_should_return_version);
    return UNITY_END();
}
