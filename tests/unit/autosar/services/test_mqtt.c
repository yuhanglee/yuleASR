/* 
 * @file test_mqtt.c
 * @brief MQTT 模块单元测试
 */

// @tests src/bsw/services/mqtt/src/Mqtt.c  @tests src/bsw/services/mqtt/include/Mqtt.h

#include <unity.h>
#include <string.h>
#include "mqtt.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_Mqtt_00001 */
void test_mqtt_Init_should_initialize(void) {
    /* Init with NULL_PTR should return error */
    Mqtt_ReturnType ret = Mqtt_Init(NULL_PTR);
    TEST_ASSERT_NOT_EQUAL(MQTT_OK, ret);
}

void test_mqtt_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    Mqtt_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(MQTT_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(MQTT_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(MQTT_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(MQTT_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(MQTT_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mqtt_Init_should_initialize);
    RUN_TEST(test_mqtt_GetVersionInfo_should_return_version);
    return UNITY_END();
}
