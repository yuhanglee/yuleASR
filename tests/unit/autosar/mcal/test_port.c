/* 
 * @file test_port.c
 * @brief PORT 模块单元测试
 * @version 1.0
 * @date 2026-01-09
 */

// @tests src/bsw/mcal/port/src/Port.c  @tests src/bsw/mcal/port/include/Port.h

#include <unity.h>
#include <string.h>
#include "port.h"
#include "port_Cfg.h"

/* 测试前置条件 */
void setUp(void) {
    // 初始化测试环境
}

void tearDown(void) {
    // 清理测试环境
}

/* 初始化测试 */
/** @req SWS_Port_00001 */
void test_port_Init_should_initialize_successfully(void) {
    /* Normal init with valid config should not crash */
    Port_Init(&Port_Config);

    /* Init with NULL_PTR should be handled by DET (no crash) */
    Port_Init(NULL_PTR);
}

/** @req SWS_Port_00002 */
void test_port_DeInit_should_cleanup_successfully(void) {
    /* Init first, then deinit */
    Port_Init(&Port_Config);
    Port_DeInit();

    /* After DeInit, re-init should work */
    Port_Init(&Port_Config);
    TEST_ASSERT_TRUE(TRUE);
}

/* 版本信息测试 */
/** @req SWS_Port_00005 */
void test_port_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));

    Port_GetVersionInfo(&versionInfo);

    TEST_ASSERT_EQUAL(PORT_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(PORT_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(PORT_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(PORT_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(PORT_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

/* 主函数 */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_port_Init_should_initialize_successfully);
    RUN_TEST(test_port_DeInit_should_cleanup_successfully);
    RUN_TEST(test_port_GetVersionInfo_should_return_version);
    
    return UNITY_END();
}
