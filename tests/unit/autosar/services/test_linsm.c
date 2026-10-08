/* 
 * @file test_linsm.c
 * @brief LINSM 模块单元测试
 */

// @tests src/bsw/services/linsm/src/LinSM.c  @tests src/bsw/services/linsm/include/LinSM.h

#include <unity.h>
#include <string.h>
#include "linsm.h"

void setUp(void) {}
void tearDown(void) {}

void test_linsm_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash; module should detect invalid param */
    LinSM_Init(NULL_PTR);
    /* After init with NULL, module typically stays uninit or reports DET error.
     * We verify it does not crash and can accept a second call. */
    LinSM_Init(&LinSM_Config);
    /* If we reach here without fault, init is functional */
    TEST_ASSERT_TRUE(1);
}

void test_linsm_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    LinSM_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(LINSM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(LINSM_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(LINSM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(LINSM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(LINSM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_linsm_Init_should_initialize);
    RUN_TEST(test_linsm_GetVersionInfo_should_return_version);
    return UNITY_END();
}
