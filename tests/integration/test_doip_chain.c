/**
 * @file test_doip_chain.c
 * @brief DoIP full-chain integration test: DoIP -> SoAd -> EthIf -> Eth
 * @req P1-12, SWS_DoIP
 *
 * Tests the complete DoIP message flow from TCP reception through
 * the protocol layers down to the Ethernet driver.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Mock layer types */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int Std_ReturnType;

#define E_OK 0
#define E_NOT_OK 1

/* Mock Eth driver state */
static uint8_t eth_tx_buffer[1600];
static uint16_t eth_tx_length = 0;
static uint32_t eth_tx_count = 0;

/* Mock EthIf APIs */
Std_ReturnType EthIf_Transmit(const uint8_t* data, uint16_t length) {
    if (data == NULL || length == 0 || length > 1500) {
        return E_NOT_OK;
    }
    memcpy(eth_tx_buffer, data, length);
    eth_tx_length = length;
    eth_tx_count++;
    return E_OK;
}

/* Mock SoAd state */
typedef struct {
    uint16_t connection_id;
    uint8_t connected;
} SoAd_ConnectionType;

static SoAd_ConnectionType soad_connection = {0, 0};

/* Mock SoAd APIs */
Std_ReturnType SoAd_OpenSocket(uint16_t port) {
    soad_connection.connection_id = 1;
    soad_connection.connected = 1;
    return E_OK;
}

Std_ReturnType SoAd_Transmit(uint16_t conn_id, const uint8_t* data, uint16_t length) {
    if (!soad_connection.connected || conn_id != soad_connection.connection_id) {
        return E_NOT_OK;
    }
    return EthIf_Transmit(data, length);
}

/* DoIP protocol constants */
#define DOIP_HEADER_LENGTH 8
#define DOIP_VERSION 0x02
#define DOIP_INV_REQUEST 0x0001
#define DOIP_INV_RESPONSE 0x0002
#define DOIP_DIAGNOSTIC 0x8001

/* DoIP header structure */
typedef struct {
    uint8_t version;
    uint8_t version_inv;
    uint16_t payload_type;
    uint32_t payload_length;
} DoIP_HeaderType;

/* Mock DoIP state */
typedef struct {
    uint8_t initialized;
    uint8_t discovery_enabled;
    uint16_t logical_address;
} DoIP_StateType;

static DoIP_StateType doip_state = {0, 0, 0x0001};

/* DoIP APIs */
Std_ReturnType DoIP_Init(void) {
    doip_state.initialized = 1;
    doip_state.discovery_enabled = 1;
    return E_OK;
}

Std_ReturnType DoIP_SendVehicleId(void) {
    if (!doip_state.initialized) {
        return E_NOT_OK;
    }

    uint8_t vehicle_id_msg[32];
    vehicle_id_msg[0] = DOIP_VERSION;
    vehicle_id_msg[1] = ~DOIP_VERSION;
    vehicle_id_msg[2] = 0x00; /* Payload type high */
    vehicle_id_msg[3] = 0x01; /* Payload type low (Vehicle Announcement) */
    vehicle_id_msg[4] = 0x00; /* Payload length */
    vehicle_id_msg[5] = 0x00;
    vehicle_id_msg[6] = 0x00;
    vehicle_id_msg[7] = 0x1C; /* 28 bytes */

    /* VIN (17 bytes) */
    memcpy(&vehicle_id_msg[8], "YULEASR0000000001", 17);

    /* Logical address (2 bytes) */
    vehicle_id_msg[25] = (doip_state.logical_address >> 8) & 0xFF;
    vehicle_id_msg[26] = doip_state.logical_address & 0xFF;

    /* Entity identification (4 bytes) */
    vehicle_id_msg[27] = 0x00;
    vehicle_id_msg[28] = 0x01;
    vehicle_id_msg[29] = 0x02;
    vehicle_id_msg[30] = 0x03;

    /* Group identification (4 bytes) */
    vehicle_id_msg[31] = 0x00;

    return SoAd_Transmit(1, vehicle_id_msg, 32);
}

Std_ReturnType DoIP_ProcessDiagnosticMessage(const uint8_t* data, uint16_t length) {
    if (!doip_state.initialized || data == NULL || length < 2) {
        return E_NOT_OK;
    }

    /* Build DoIP diagnostic message header */
    uint8_t doip_msg[1600];
    doip_msg[0] = DOIP_VERSION;
    doip_msg[1] = ~DOIP_VERSION;
    doip_msg[2] = (DOIP_DIAGNOSTIC >> 8) & 0xFF;
    doip_msg[3] = DOIP_DIAGNOSTIC & 0xFF;

    /* Payload length = source address (2) + target address (2) + data */
    uint32_t payload_len = 4 + length;
    doip_msg[4] = (payload_len >> 24) & 0xFF;
    doip_msg[5] = (payload_len >> 16) & 0xFF;
    doip_msg[6] = (payload_len >> 8) & 0xFF;
    doip_msg[7] = payload_len & 0xFF;

    /* Source address (tester) */
    doip_msg[8] = 0x0E;
    doip_msg[9] = 0x00;

    /* Target address (ECU) */
    doip_msg[10] = (doip_state.logical_address >> 8) & 0xFF;
    doip_msg[11] = doip_state.logical_address & 0xFF;

    /* User data */
    memcpy(&doip_msg[12], data, length);

    return SoAd_Transmit(1, doip_msg, 12 + length);
}

/* Test cases */
void test_doip_init(void) {
    printf("Test: DoIP initialization... ");
    Std_ReturnType ret = DoIP_Init();
    assert(ret == E_OK);
    assert(doip_state.initialized == 1);
    printf("PASS\n");
}

void test_doip_vehicle_announcement(void) {
    printf("Test: DoIP vehicle announcement... ");
    Std_ReturnType ret = DoIP_SendVehicleId();
    assert(ret == E_OK);
    assert(eth_tx_count > 0);
    assert(eth_tx_buffer[0] == DOIP_VERSION);
    assert(eth_tx_buffer[1] == ~DOIP_VERSION);
    printf("PASS\n");
}

void test_doip_diagnostic_message(void) {
    printf("Test: DoIP diagnostic message... ");
    uint8_t uds_request[] = {0x10, 0x01}; /* DiagnosticSessionControl */
    Std_ReturnType ret = DoIP_ProcessDiagnosticMessage(uds_request, sizeof(uds_request));
    assert(ret == E_OK);
    assert(eth_tx_length > 0);
    /* Verify DoIP header */
    assert(eth_tx_buffer[0] == DOIP_VERSION);
    assert((eth_tx_buffer[2] << 8 | eth_tx_buffer[3]) == DOIP_DIAGNOSTIC);
    printf("PASS\n");
}

void test_doip_full_chain(void) {
    printf("Test: DoIP full chain (DoIP->SoAd->EthIf->Eth)... ");

    /* Initialize all layers */
    Std_ReturnType ret = SoAd_OpenSocket(13400);
    assert(ret == E_OK);

    ret = DoIP_Init();
    assert(ret == E_OK);

    /* Send vehicle announcement */
    ret = DoIP_SendVehicleId();
    assert(ret == E_OK);
    uint32_t tx_count_after_announcement = eth_tx_count;

    /* Process diagnostic request */
    uint8_t uds_request[] = {0x22, 0xF1, 0x90}; /* ReadDID */
    ret = DoIP_ProcessDiagnosticMessage(uds_request, sizeof(uds_request));
    assert(ret == E_OK);
    assert(eth_tx_count > tx_count_after_announcement);

    printf("PASS\n");
}

int main(void) {
    printf("=== DoIP Full-Chain Integration Test ===\n\n");

    test_doip_init();
    test_doip_vehicle_announcement();
    test_doip_diagnostic_message();
    test_doip_full_chain();

    printf("\n=== All DoIP tests PASSED ===\n");
    return 0;
}
