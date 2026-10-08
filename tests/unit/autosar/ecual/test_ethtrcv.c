/* 
 * @file test_ethtrcv.c
 * @brief ETHTRCV 模块单元测试
 */

// @tests src/bsw/ecual/ethtrcv/src/EthTrcv.c  @tests src/bsw/ecual/ethtrcv/include/EthTrcv.h

#include <unity.h>
#include <string.h>
#include "ethtrcv.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_EthTrcv_00001 */
void test_ethtrcv_Init_should_initialize(void) {
    /* EthTrcv_Init accepts NULL_PTR and defaults to &EthTrcv_Config */
    EthTrcv_Init(NULL_PTR);
    /* If we reach here without crash, init succeeded */
    TEST_PASS();
}

void test_ethtrcv_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    EthTrcv_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ethtrcv_Init_should_initialize);
    RUN_TEST(test_ethtrcv_GetVersionInfo_should_return_version);
    return UNITY_END();
}
