/* 
 * @file test_ethsm.c
 * @brief ETHSM 模块单元测试
 */

// @tests src/bsw/services/ethsm/src/EthSM.c  @tests src/bsw/services/ethsm/include/EthSM.h

#include <unity.h>
#include "ethsm.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_EthSM_00001 */
void test_ethsm_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (DET reports error internally) */
    EthSM_Init(NULL_PTR);
    TEST_PASS();
}

/** @req SWS_EthSM_00008 */
void test_ethsm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    EthSM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(ETHSM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(ETHSM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(ETHSM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(ETHSM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ethsm_Init_should_initialize);
    RUN_TEST(test_ethsm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
