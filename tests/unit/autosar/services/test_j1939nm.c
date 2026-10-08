/* 
 * @file test_j1939nm.c
 * @brief J1939NM 模块单元测试
 */

// @tests src/bsw/services/j1939nm/src/J1939Nm.c  @tests src/bsw/services/j1939nm/include/J1939Nm.h

#include <unity.h>
#include "j1939nm.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_J1939Nm_00001 */
void test_j1939nm_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (DET reports error internally) */
    J1939Nm_Init(NULL_PTR);
    TEST_PASS();
}

void test_j1939nm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    J1939Nm_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(J1939NM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(J1939NM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(J1939NM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(J1939NM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_j1939nm_Init_should_initialize);
    RUN_TEST(test_j1939nm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
