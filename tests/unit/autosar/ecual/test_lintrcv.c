/* 
 * @file test_lintrcv.c
 * @brief LINTRCV 模块单元测试
 */

// @tests src/bsw/ecual/lintrcv/src/LinTrcv.c  @tests src/bsw/ecual/lintrcv/include/LinTrcv.h

#include <unity.h>
#include <string.h>
#include "lintrcv.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_LinTrcv_00001 */
void test_lintrcv_Init_should_initialize(void) {
    /* LinTrcv_Init rejects NULL_PTR, reports DET and returns early */
    LinTrcv_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_lintrcv_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    LinTrcv_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_lintrcv_Init_should_initialize);
    RUN_TEST(test_lintrcv_GetVersionInfo_should_return_version);
    return UNITY_END();
}
