/* 
 * @file test_memif.c
 * @brief MEMIF 模块单元测试
 */

// @tests src/bsw/services/memif/src/MemIf.c  @tests src/bsw/services/memif/include/MemIf.h

#include <unity.h>
#include <string.h>
#include "memif.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_MemIf_00001 */
void test_memif_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash */
    MemIf_Init(NULL_PTR);
    /* After NULL init, status should be UNINIT */
    MemIf_StatusType status = MemIf_GetStatus(0);
    TEST_ASSERT_EQUAL(MEMIF_UNINIT, status);
}

/** @req SWS_MemIf_00003 */
void test_memif_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    MemIf_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(MEMIF_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(MEMIF_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(MEMIF_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(MEMIF_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(MEMIF_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_memif_Init_should_initialize);
    RUN_TEST(test_memif_GetVersionInfo_should_return_version);
    return UNITY_END();
}
