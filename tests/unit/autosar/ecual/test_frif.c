/* 
 * @file test_frif.c
 * @brief FRIF 模块单元测试
 */

// @tests src/bsw/ecual/frif/src/FrIf.c  @tests src/bsw/ecual/frif/include/FrIf.h

#include <unity.h>
#include <string.h>
#include "frif.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_FrIf_00001 */
void test_frif_Init_should_initialize(void) {
    /* FrIf_Init rejects NULL_PTR, reports DET and returns early */
    FrIf_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_frif_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    FrIf_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_frif_Init_should_initialize);
    RUN_TEST(test_frif_GetVersionInfo_should_return_version);
    return UNITY_END();
}
