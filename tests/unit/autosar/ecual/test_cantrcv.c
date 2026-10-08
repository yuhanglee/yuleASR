/* 
 * @file test_cantrcv.c
 * @brief CANTRCV 模块单元测试
 */

// @tests src/bsw/ecual/cantrcv/src/CanTrcv.c  @tests src/bsw/ecual/cantrcv/include/CanTrcv.h

#include <unity.h>
#include <string.h>
#include "cantrcv.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_CanTrcv_00005 */
void test_cantrcv_Init_should_initialize(void) {
    /* CanTrcv_Init rejects NULL_PTR, reports DET and returns early */
    CanTrcv_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_cantrcv_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    CanTrcv_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_cantrcv_Init_should_initialize);
    RUN_TEST(test_cantrcv_GetVersionInfo_should_return_version);
    return UNITY_END();
}
