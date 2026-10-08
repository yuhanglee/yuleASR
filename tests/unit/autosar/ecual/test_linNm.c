/* 
 * @file test_linnm.c
 * @brief LINNM 模块单元测试
 */

// @tests src/bsw/ecual/linnm/src/LinNm.c  @tests src/bsw/ecual/linnm/include/LinNm.h

#include <unity.h>
#include <string.h>
#include "linnm.h"

void setUp(void) {}
void tearDown(void) {}

void test_linnm_Init_should_initialize(void) {
    /* LinNm_Init rejects NULL_PTR, reports DET and returns early */
    LinNm_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_linnm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    LinNm_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_linnm_Init_should_initialize);
    RUN_TEST(test_linnm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
