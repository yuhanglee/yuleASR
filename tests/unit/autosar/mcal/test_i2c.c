/* 
 * @file test_i2c.c
 * @brief I2C 模块单元测试
 * @version 1.0
 * @date 2026-01-09
 */

// @tests src/bsw/mcal/i2c/src/I2c.c  @tests src/bsw/mcal/i2c/include/I2c.h

#include <unity.h>
#include <string.h>
#include "i2c.h"
#include "i2c_Cfg.h"

/* 测试前置条件 */
void setUp(void) {
    // 初始化测试环境
}

void tearDown(void) {
    // 清理测试环境
}

/* 初始化测试 */
/** @req SWS_I2c_00001 */
void test_i2c_Init_should_initialize_successfully(void) {
    /* Normal init with valid config should not crash */
    I2c_Init(&I2c_Config);

    /* After init, status should be I2C_IDLE */
    TEST_ASSERT_EQUAL(I2C_IDLE, I2c_GetStatus());

    /* Init with NULL_PTR should be handled by DET (no crash) */
    I2c_Init(NULL_PTR);
}

/** @req SWS_I2c_00002 */
void test_i2c_DeInit_should_cleanup_successfully(void) {
    Std_ReturnType result;

    /* Init first, then deinit */
    I2c_Init(&I2c_Config);
    result = I2c_DeInit();

    TEST_ASSERT_EQUAL(E_OK, result);
    TEST_ASSERT_EQUAL(I2C_UNINIT, I2c_GetStatus());
}

/* 版本信息测试 */
/** @req SWS_I2c_00007 */
void test_i2c_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));

    I2c_GetVersionInfo(&versionInfo);

    TEST_ASSERT_EQUAL(I2C_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(I2C_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(I2C_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(I2C_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(I2C_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

/* 主函数 */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_i2c_Init_should_initialize_successfully);
    RUN_TEST(test_i2c_DeInit_should_cleanup_successfully);
    RUN_TEST(test_i2c_GetVersionInfo_should_return_version);
    
    return UNITY_END();
}
