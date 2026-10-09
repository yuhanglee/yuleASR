/**
 * @file test_e2e_e2e.c
 * @brief E2E Test: E2E Profile Protection (Profile 1 / Profile 2)
 *
 * Verifies E2E data protection: counter monotonicity, CRC integrity,
 * data-ID verification, and corrupted-data rejection.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define E2E_HEADER_SIZE  8
#define E2E_CRC_INIT     0xFFFFFFFFU
#define E2E_MAX_COUNTER  0x0FU

typedef struct {
    unsigned int data_id;
    unsigned int counter;
    unsigned int crc;
} E2E_HeaderType;

typedef struct {
    unsigned int data_id;
    unsigned int counter;
    int          initialized;
} E2E_StateType;

static E2E_StateType e2e_state;

static unsigned int e2e_crc32(const unsigned char *data, unsigned int len) {
    unsigned int crc = E2E_CRC_INIT;
    for (unsigned int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320U;
            else
                crc >>= 1;
        }
    }
    return crc;
}

static void E2E_Init(unsigned int data_id) {
    e2e_state.data_id    = data_id;
    e2e_state.counter    = 0;
    e2e_state.initialized = 1;
}

static int E2E_Protect(unsigned char *pdu, unsigned int pdu_len,
                       unsigned int data_id) {
    if (!e2e_state.initialized || pdu_len < E2E_HEADER_SIZE) return -1;
    if (data_id != e2e_state.data_id) return -1;

    pdu[0] = (unsigned char)(data_id >> 24);
    pdu[1] = (unsigned char)(data_id >> 16);
    pdu[2] = (unsigned char)(data_id >> 8);
    pdu[3] = (unsigned char)(data_id);
    pdu[4] = (unsigned char)(e2e_state.counter & E2E_MAX_COUNTER);

    unsigned int crc = e2e_crc32(pdu, pdu_len - 4);
    pdu[pdu_len - 4] = (unsigned char)(crc >> 24);
    pdu[pdu_len - 3] = (unsigned char)(crc >> 16);
    pdu[pdu_len - 2] = (unsigned char)(crc >> 8);
    pdu[pdu_len - 1] = (unsigned char)(crc);

    e2e_state.counter = (e2e_state.counter + 1) & E2E_MAX_COUNTER;
    return 0;
}

static int E2E_Check(const unsigned char *pdu, unsigned int pdu_len,
                     unsigned int data_id) {
    if (!e2e_state.initialized || pdu_len < E2E_HEADER_SIZE) return -1;

    unsigned int rx_id = ((unsigned int)pdu[0] << 24) | ((unsigned int)pdu[1] << 16) |
                         ((unsigned int)pdu[2] << 8)  | (unsigned int)pdu[3];
    if (rx_id != data_id) return -1;

    unsigned int stored_crc =
        ((unsigned int)pdu[pdu_len - 4] << 24) |
        ((unsigned int)pdu[pdu_len - 3] << 16) |
        ((unsigned int)pdu[pdu_len - 2] << 8)  |
        (unsigned int)pdu[pdu_len - 1];

    unsigned int computed_crc = e2e_crc32(pdu, pdu_len - 4);
    if (stored_crc != computed_crc) return -1;

    return 0;
}

static int test_e2e_init_and_protect(void) {
    E2E_Init(0x12345678);
    unsigned char pdu[16];
    memset(pdu, 0, sizeof(pdu));
    assert(E2E_Protect(pdu, sizeof(pdu), 0x12345678) == 0);
    assert(pdu[4] == 0);
    printf("  [PASS] test_e2e_init_and_protect\n");
    return 1;
}

static int test_e2e_protect_check_roundtrip(void) {
    E2E_Init(0xAABBCCDD);
    unsigned char pdu[12] = {0};
    pdu[5] = 0x42;
    assert(E2E_Protect(pdu, sizeof(pdu), 0xAABBCCDD) == 0);
    assert(E2E_Check(pdu, sizeof(pdu), 0xAABBCCDD) == 0);
    printf("  [PASS] test_e2e_protect_check_roundtrip\n");
    return 1;
}

static int test_e2e_counter_increment(void) {
    E2E_Init(0x00000001);
    unsigned char pdu[12];
    memset(pdu, 0, sizeof(pdu));
    for (int i = 0; i <= (int)E2E_MAX_COUNTER; i++) {
        assert(E2E_Protect(pdu, sizeof(pdu), 0x00000001) == 0);
        assert(pdu[4] == (unsigned char)i);
    }
    assert(E2E_Protect(pdu, sizeof(pdu), 0x00000001) == 0);
    assert(pdu[4] == 0);
    printf("  [PASS] test_e2e_counter_increment\n");
    return 1;
}

static int test_e2e_corrupted_data_rejected(void) {
    E2E_Init(0x11111111);
    unsigned char pdu[10] = {0};
    assert(E2E_Protect(pdu, sizeof(pdu), 0x11111111) == 0);
    assert(E2E_Check(pdu, sizeof(pdu), 0x11111111) == 0);
    pdu[5] ^= 0xFF;
    assert(E2E_Check(pdu, sizeof(pdu), 0x11111111) == -1);
    printf("  [PASS] test_e2e_corrupted_data_rejected\n");
    return 1;
}

static int test_e2e_wrong_data_id_rejected(void) {
    E2E_Init(0x22222222);
    unsigned char pdu[10] = {0};
    assert(E2E_Protect(pdu, sizeof(pdu), 0x22222222) == 0);
    assert(E2E_Check(pdu, sizeof(pdu), 0x33333333) == -1);
    printf("  [PASS] test_e2e_wrong_data_id_rejected\n");
    return 1;
}

int main(void) {
    int passed = 0, total = 0;
    printf("=== E2E Test: E2E Profile Protection ===\n");
    total++; passed += test_e2e_init_and_protect();
    total++; passed += test_e2e_protect_check_roundtrip();
    total++; passed += test_e2e_counter_increment();
    total++; passed += test_e2e_corrupted_data_rejected();
    total++; passed += test_e2e_wrong_data_id_rejected();
    printf("\nResult: %d/%d tests passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
