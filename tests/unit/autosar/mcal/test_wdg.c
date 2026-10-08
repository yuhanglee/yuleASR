/* 
 * @file test_wdg.c
 * @brief WDG 模块单元测试
 * @version 1.0
 * @date 2026-01-09
 */

// @tests src/bsw/mcal/wdg/src/Wdg.c  @tests src/bsw/mcal/wdg/include/Wdg.h

#include <unity.h>
#include <string.h>
#include "wdg.h"
#include "wdg_Cfg.h"

/* 测试前置条件 */
void setUp(void) {
    // 初始化测试环境
}

void tearDown(void) {
    // 清理测试环境
}

/* 初始化测试 */
/** @req SWS_Wdg_00001 */
void test_wdg_Init_should_initialize_successfully(void) {
    /* Normal init with valid config should not crash */
    Wdg_Init(&Wdg_Config);

    /* After init, state should not be UNINIT */
    TEST_ASSERT_NOT_EQUAL(WDG_STATE_UNINIT, Wdg_GetStatus());

    /* Init with NULL_PTR should be handled by DET (no crash) */
    Wdg_Init(NULL_PTR);
}

/* @req SWS_Wdg_00201 */
void test_wdg_DeInit_should_cleanup_successfully(void) {
    Std_ReturnType result;

    /* Init first, then set mode to OFF to disable watchdog */
    Wdg_Init(&Wdg_Config);
    result = Wdg_SetMode(WDGIF_OFF_MODE);

    TEST_ASSERT_EQUAL(E_OK, result);
}

/* 版本信息测试 */
/** @req SWS_Wdg_00004 */
void test_wdg_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));

    Wdg_GetVersionInfo(&versionInfo);

    TEST_ASSERT_EQUAL(WDG_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(WDG_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(WDG_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(WDG_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(WDG_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

/* 主函数 */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_wdg_Init_should_initialize_successfully);
    RUN_TEST(test_wdg_DeInit_should_cleanup_successfully);
    RUN_TEST(test_wdg_GetVersionInfo_should_return_version);
    
    return UNITY_END();
}
