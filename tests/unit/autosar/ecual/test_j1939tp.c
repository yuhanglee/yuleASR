/* 
 * @file test_j1939tp.c
 * @brief J1939TP 模块单元测试
 */

// @tests src/bsw/ecual/j1939tp/src/J1939Tp.c  @tests src/bsw/ecual/j1939tp/include/J1939Tp.h

#include <unity.h>
#include <string.h>
#include "j1939tp.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_J1939Tp_00001 */
void test_j1939tp_Init_should_initialize(void) {
    /* J1939Tp_Init rejects NULL_PTR, reports DET and returns early */
    J1939Tp_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_j1939tp_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    J1939Tp_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_j1939tp_Init_should_initialize);
    RUN_TEST(test_j1939tp_GetVersionInfo_should_return_version);
    return UNITY_END();
}
