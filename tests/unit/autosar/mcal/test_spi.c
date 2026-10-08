/* 
 * @file test_spi.c
 * @brief SPI 模块单元测试
 * @version 1.0
 * @date 2026-01-09
 */

// @tests src/bsw/mcal/spi/src/Spi.c  @tests src/bsw/mcal/spi/include/Spi.h

#include <unity.h>
#include <string.h>
#include "spi.h"
#include "spi_Cfg.h"

/* 测试前置条件 */
void setUp(void) {
    // 初始化测试环境
}

void tearDown(void) {
    // 清理测试环境
}

/* 初始化测试 */
/** @req SWS_Spi_00001 */
void test_spi_Init_should_initialize_successfully(void) {
    /* Normal init with valid config should not crash */
    Spi_Init(NULL_PTR);

    /* After init, status should be SPI_IDLE */
    TEST_ASSERT_EQUAL(SPI_IDLE, Spi_GetStatus());
}

/** @req SWS_Spi_00002 */
void test_spi_DeInit_should_cleanup_successfully(void) {
    Std_ReturnType result;

    /* Init first, then deinit */
    Spi_Init(NULL_PTR);
    result = Spi_DeInit();

    TEST_ASSERT_EQUAL(E_OK, result);
    TEST_ASSERT_EQUAL(SPI_UNINIT, Spi_GetStatus());
}

/* 版本信息测试 */
/** @req SWS_Spi_00009 */
void test_spi_GetVersionInfo_should_return_version(void) {
    /* Verify version macros are non-zero and consistent */
    TEST_ASSERT_NOT_EQUAL(0U, SPI_VENDOR_ID);
    TEST_ASSERT_NOT_EQUAL(0U, SPI_MODULE_ID);
    TEST_ASSERT_EQUAL(1U, SPI_SW_MAJOR_VERSION);
    TEST_ASSERT_EQUAL(0U, SPI_SW_MINOR_VERSION);
    TEST_ASSERT_EQUAL(0U, SPI_SW_PATCH_VERSION);
    TEST_ASSERT_EQUAL(4U, SPI_AR_RELEASE_MAJOR_VERSION);
    TEST_ASSERT_EQUAL(4U, SPI_AR_RELEASE_MINOR_VERSION);
}

/* 主函数 */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_spi_Init_should_initialize_successfully);
    RUN_TEST(test_spi_DeInit_should_cleanup_successfully);
    RUN_TEST(test_spi_GetVersionInfo_should_return_version);
    
    return UNITY_END();
}
