/* 
 * @file test_ramsafety.c
 * @brief RAMSAFETY 模块单元测试
 */

// @tests src/bsw/services/ramsafety/src/RamSafety.c  @tests src/bsw/services/ramsafety/include/RamSafety.h

#include <unity.h>
#include <string.h>
#include "ramsafety.h"

void setUp(void) {}
void tearDown(void) {}

void test_ramsafety_Init_should_initialize(void) {
    /* Init with NULL_PTR should return E_NOT_OK */
    Std_ReturnType ret = RamSafety_Init(NULL_PTR);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_ramsafety_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    RamSafety_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(RAMSAFETY_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(RAMSAFETY_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(RAMSAFETY_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(RAMSAFETY_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ramsafety_Init_should_initialize);
    RUN_TEST(test_ramsafety_GetVersionInfo_should_return_version);
    return UNITY_END();
}
