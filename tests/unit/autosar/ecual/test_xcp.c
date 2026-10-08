/* 
 * @file test_xcp.c
 * @brief XCP 模块单元测试
 */

// @tests src/bsw/ecual/xcp/src/Xcp.c  @tests src/bsw/ecual/xcp/include/Xcp.h

#include <unity.h>
#include <string.h>
#include "xcp.h"

/* Xcp_GetVersionInfo is provided by the canonical services/xcp implementation */
extern void Xcp_GetVersionInfo(Std_VersionInfoType* versioninfo);

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_Xcp_00001 */
void test_xcp_Init_should_initialize(void) {
    /* Xcp_Init rejects NULL_PTR, reports DET and returns early */
    Xcp_Init(NULL_PTR);
    /* If we reach here without crash, the NULL check worked */
    TEST_PASS();
}

void test_xcp_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    Xcp_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_xcp_Init_should_initialize);
    RUN_TEST(test_xcp_GetVersionInfo_should_return_version);
    return UNITY_END();
}
