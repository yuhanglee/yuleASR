/* 
 * @file test_crc.c
 * @brief CRC 模块单元测试
 */

// @tests src/bsw/services/crc/src/Crc.c  @tests src/bsw/services/crc/include/Crc.h

#include <unity.h>
#include "crc.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_Crc_00001 */
void test_crc_Init_should_initialize(void) {
    /* Crc_Init accepts NULL config (no config required for table-based CRC) */
    Crc_Init(NULL_PTR);
    TEST_PASS();
}

void test_crc_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(Std_VersionInfoType));
    Crc_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(CRC_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(CRC_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(CRC_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(CRC_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_crc_Init_should_initialize);
    RUN_TEST(test_crc_GetVersionInfo_should_return_version);
    return UNITY_END();
}
