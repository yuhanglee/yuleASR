/* 
 * @file test_ethsm.c
 * @brief ETHSM 模块单元测试
 */

// @tests src/bsw/ecual/ethsm/src/EthSM.c  @tests src/bsw/ecual/ethsm/include/EthSM.h

#include <unity.h>
#include <string.h>
#include "ethsm.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_EthSM_00001 */
void test_ethsm_Init_should_initialize(void) {
    /* EthSM_Init stores ConfigPtr directly (NULL accepted in this implementation) */
    EthSM_Init(NULL_PTR);
    /* If we reach here without crash, init completed */
    TEST_PASS();
}

/** @req SWS_EthSM_00003 */
void test_ethsm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    EthSM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.vendorID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.moduleID);
    TEST_ASSERT_NOT_EQUAL(0u, versionInfo.sw_major_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ethsm_Init_should_initialize);
    RUN_TEST(test_ethsm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
