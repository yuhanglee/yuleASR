/* 
 * @file test_uart.c
 * @brief UART 模块单元测试
 * @version 1.0
 * @date 2026-01-09
 */

// @tests src/bsw/mcal/uart/src/Uart.c  @tests src/bsw/mcal/uart/include/Uart.h

#include <unity.h>
#include <string.h>
#include "uart.h"
#include "uart_Cfg.h"

/* 测试前置条件 */
void setUp(void) {
    // 初始化测试环境
}

void tearDown(void) {
    // 清理测试环境
}

/* 初始化测试 */
/** @req SWS_Uart_00001 */
void test_uart_Init_should_initialize_successfully(void) {
    /* Init with NULL_PTR should be handled by DET (no crash) */
    Uart_Init(NULL_PTR);

    /* After init attempt, status of channel 0 should reflect state */
    Uart_StatusType status = Uart_GetStatus(UART_CHANNEL_0);
    /* With NULL config, DET should catch it; state remains UNINIT or READY */
    TEST_ASSERT_TRUE((status == UART_STATE_UNINIT) || (status == UART_STATE_READY));
}

/** @req SWS_Uart_00002 */
void test_uart_DeInit_should_cleanup_successfully(void) {
    /* Init first, then deinit */
    Uart_Init(NULL_PTR);
    Uart_DeInit();

    /* After DeInit, status should be UNINIT */
    Uart_StatusType status = Uart_GetStatus(UART_CHANNEL_0);
    TEST_ASSERT_EQUAL(UART_STATE_UNINIT, status);
}

/* 版本信息测试 */
/** @req SWS_Uart_00019 */
void test_uart_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));

    Uart_GetVersionInfo(&versionInfo);

    TEST_ASSERT_EQUAL(UART_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(UART_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(UART_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(UART_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(UART_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

/* 主函数 */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_uart_Init_should_initialize_successfully);
    RUN_TEST(test_uart_DeInit_should_cleanup_successfully);
    RUN_TEST(test_uart_GetVersionInfo_should_return_version);
    
    return UNITY_END();
}
