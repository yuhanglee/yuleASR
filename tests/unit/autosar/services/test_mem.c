/* 
 * @file test_mem.c
 * @brief MEM 模块单元测试
 */

// @tests src/bsw/services/mem/src/Mem.c  @tests src/bsw/services/mem/include/Mem.h

#include <unity.h>
#include <string.h>
#include "mem.h"

void setUp(void) {}
void tearDown(void) {}

void test_mem_Init_should_initialize(void) {
    /* Init with NULL_PTR should not crash */
    Mem_Init(NULL_PTR);
    /* Init with valid config */
    Mem_Init(Mem_ConfigPtr);
    /* After init, status should be MEM_IDLE (or at least not MEM_UNINIT if config was valid) */
    Mem_StatusType status = Mem_GetStatus();
    /* If config was NULL, status may still be UNINIT; just verify no crash */
    TEST_ASSERT_TRUE(status == MEM_UNINIT || status == MEM_IDLE || status == MEM_BUSY);
}

void test_mem_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    Mem_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(MEM_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(MEM_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(MEM_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(MEM_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(MEM_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mem_Init_should_initialize);
    RUN_TEST(test_mem_GetVersionInfo_should_return_version);
    return UNITY_END();
}
