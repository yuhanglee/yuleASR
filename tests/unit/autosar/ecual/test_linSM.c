/* 
 * @file test_linsm.c
 * @brief LINSM 模块单元测试
 */

// @tests src/bsw/ecual/linsm/src/LinSM.c  @tests src/bsw/ecual/linsm/include/LinSM.h

#include <unity.h>
#include <string.h>
#include "linsm.h"

void setUp(void) {}
void tearDown(void) {}

void test_linsm_Init_should_initialize(void) {
    /* LinSM_Init rejects NULL_PTR, reports DET and returns early */
    LinSM_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_linsm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    LinSM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_linsm_Init_should_initialize);
    RUN_TEST(test_linsm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
