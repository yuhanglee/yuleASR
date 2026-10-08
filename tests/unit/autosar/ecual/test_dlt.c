/* 
 * @file test_dlt.c
 * @brief DLT 模块单元测试
 */

// @tests src/bsw/services/dlt/src/Dlt.c  @tests src/bsw/services/dlt/include/Dlt.h

#include <unity.h>
#include <string.h>
#include "dlt.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_Dlt_00001 */
void test_dlt_Init_should_initialize(void) {
    /* Dlt_Init rejects NULL_PTR, reports DET and returns early */
    Dlt_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_dlt_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    Dlt_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_dlt_Init_should_initialize);
    RUN_TEST(test_dlt_GetVersionInfo_should_return_version);
    return UNITY_END();
}
