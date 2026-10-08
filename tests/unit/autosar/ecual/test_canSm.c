/* 
 * @file test_cansm.c
 * @brief CANSM 模块单元测试
 */

// @tests src/bsw/services/cansm/src/CanSm.c  @tests src/bsw/services/cansm/include/CanSm.h

#include <unity.h>
#include <string.h>
#include "cansm.h"

void setUp(void) {}
void tearDown(void) {}

void test_cansm_Init_should_initialize(void) {
    /* CanSM_Init accepts NULL_PTR and defaults to pre-compile config */
    CanSM_Init(NULL_PTR);
    /* If we reach here without crash, init succeeded */
    TEST_PASS();
}

void test_cansm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    CanSM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_cansm_Init_should_initialize);
    RUN_TEST(test_cansm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
