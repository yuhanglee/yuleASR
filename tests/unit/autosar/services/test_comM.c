/* 
 * @file test_comM.c
 * @brief COMM 模块单元测试
 */

// @tests src/bsw/services/comm/src/ComM.c  @tests src/bsw/services/comm/include/ComM.h

#include <unity.h>
#include "comm.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_ComM_00001 */
void test_comm_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (DET reports error internally) */
    ComM_Init(NULL_PTR);
    TEST_PASS();
}

void test_comm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    ComM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(COMM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(COMM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(COMM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_comm_Init_should_initialize);
    RUN_TEST(test_comm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
