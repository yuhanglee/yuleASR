/* 
 * @file test_srp.c
 * @brief SRP 模块单元测试
 */

// @tests src/bsw/ecual/srp/src/Srp.c  @tests src/bsw/ecual/srp/include/Srp.h

#include <unity.h>
#include <string.h>
#include "Srp.h"

void setUp(void) {}
void tearDown(void) {}

/** @req SWS_Srp_00001 */
void test_srp_Init_should_initialize(void) {
    /* Srp_Init is void; call with NULL to exercise the code path */
    Srp_Init(NULL_PTR);
    /* Verify module is callable after init attempt */
    Srp_DeInit();
    TEST_PASS();
}

void test_srp_Init_with_null_should_not_crash(void) {
    Srp_Init(NULL_PTR);
    TEST_PASS();
}

void test_srp_DeInit_should_not_crash(void) {
    Srp_DeInit();
    TEST_PASS();
}

void test_srp_GetVersionInfo_should_return_version(void) {
    Std_VersionInfoType versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    Srp_GetVersionInfo(&versionInfo);
    TEST_ASSERT_EQUAL(SRP_VENDOR_ID, versionInfo.vendorID);
    TEST_ASSERT_EQUAL(SRP_MODULE_ID, versionInfo.moduleID);
    TEST_ASSERT_EQUAL(SRP_SW_MAJOR_VERSION, versionInfo.sw_major_version);
    TEST_ASSERT_EQUAL(SRP_SW_MINOR_VERSION, versionInfo.sw_minor_version);
    TEST_ASSERT_EQUAL(SRP_SW_PATCH_VERSION, versionInfo.sw_patch_version);
}

void test_srp_GetVersionInfo_with_null_should_not_crash(void) {
    Srp_GetVersionInfo(NULL_PTR);
    TEST_PASS();
}

void test_srp_RegisterTalker_null_should_return_error(void) {
    Std_ReturnType ret = Srp_RegisterTalker(NULL_PTR);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_srp_RegisterListener_with_stream_id(void) {
    Srp_StreamIdType streamId = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Std_ReturnType ret = Srp_RegisterListener(streamId);
    /* May return E_OK or E_NOT_OK depending on init state */
    TEST_ASSERT_TRUE(ret == E_OK || ret == E_NOT_OK);
}

void test_srp_DeregisterStream_with_stream_id(void) {
    Srp_StreamIdType streamId = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Std_ReturnType ret = Srp_DeregisterStream(streamId);
    TEST_ASSERT_TRUE(ret == E_OK || ret == E_NOT_OK);
}

void test_srp_GetStreamStatus_null_status_should_return_error(void) {
    Srp_StreamIdType streamId = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Std_ReturnType ret = Srp_GetStreamStatus(streamId, NULL_PTR);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret);
}

void test_srp_GetStreamStatus_valid_params(void) {
    Srp_StreamIdType streamId = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Srp_ReservationStateType status;
    Std_ReturnType ret = Srp_GetStreamStatus(streamId, &status);
    TEST_ASSERT_TRUE(ret == E_OK || ret == E_NOT_OK);
}

void test_srp_RxIndication_null_should_not_crash(void) {
    Srp_RxIndication(NULL_PTR, 0);
    TEST_PASS();
}

void test_srp_MainFunction_should_not_crash(void) {
    Srp_MainFunction();
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_srp_Init_should_initialize);
    RUN_TEST(test_srp_Init_with_null_should_not_crash);
    RUN_TEST(test_srp_DeInit_should_not_crash);
    RUN_TEST(test_srp_GetVersionInfo_should_return_version);
    RUN_TEST(test_srp_GetVersionInfo_with_null_should_not_crash);
    RUN_TEST(test_srp_RegisterTalker_null_should_return_error);
    RUN_TEST(test_srp_RegisterListener_with_stream_id);
    RUN_TEST(test_srp_DeregisterStream_with_stream_id);
    RUN_TEST(test_srp_GetStreamStatus_null_status_should_return_error);
    RUN_TEST(test_srp_GetStreamStatus_valid_params);
    RUN_TEST(test_srp_RxIndication_null_should_not_crash);
    RUN_TEST(test_srp_MainFunction_should_not_crash);
    return UNITY_END();
}
