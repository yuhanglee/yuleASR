/* 
 * @file test_cryif.c
 * @brief CRYIF 模块单元测试
 */

// @tests src/bsw/services/cryif/src/CryIf.c  @tests src/bsw/services/cryif/include/CryIf.h

#include <unity.h>
#include "cryif.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_CryIf_00001 */
void test_cryif_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (DET reports error internally) */
    CryIf_Init(NULL_PTR);
    TEST_PASS();
}

void test_cryif_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    CryIf_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(CRYIF_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(CRYIF_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(CRYIF_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(CRYIF_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_cryif_Init_should_initialize);
    RUN_TEST(test_cryif_GetVersionInfo_should_return_version);
    return UNITY_END();
}
