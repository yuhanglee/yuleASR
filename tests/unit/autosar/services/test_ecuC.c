/* 
 * @file test_ecuC.c
 * @brief ECUC 模块单元测试
 */

// @tests src/bsw/services/ecuc/src/EcuC.c  @tests src/bsw/services/ecuc/include/EcuC.h

#include <unity.h>
#include "ecuc.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_EcuC_00001 */
void test_ecuc_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (DET reports error internally) */
    EcuC_Init(NULL_PTR);
    TEST_PASS();
}

void test_ecuc_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    EcuC_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(ECUC_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ecuc_Init_should_initialize);
    RUN_TEST(test_ecuc_GetVersionInfo_should_return_version);
    return UNITY_END();
}
