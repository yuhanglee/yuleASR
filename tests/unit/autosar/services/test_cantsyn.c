/* 
 * @file test_cantsyn.c
 * @brief CANTSYN 模块单元测试
 */

// @tests src/bsw/services/cantsyn/src/CanTSyn.c  @tests src/bsw/services/cantsyn/include/CanTSyn.h

#include <unity.h>
#include "cantsyn.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_CanTSyn_00001 */
void test_cantsyn_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash (DET reports error internally) */
    CanTSyn_Init(NULL_PTR);
    TEST_PASS();
}

void test_cantsyn_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    CanTSyn_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(CANTSYN_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(CANTSYN_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(CANTSYN_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(CANTSYN_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_cantsyn_Init_should_initialize);
    RUN_TEST(test_cantsyn_GetVersionInfo_should_return_version);
    return UNITY_END();
}
