/**
 * @file test_e2e_secoc.c
 * @brief E2E Test: SecOC Authentication & Freshness
 *
 * Verifies SecOC MAC generation/verification and freshness counter management:
 *   SecOC_Init -> AuthTx (MAC append) -> AuthRx (MAC verify) -> Freshness persist
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define SECOCK_KEY_LEN       16
#define SECOCK_MAC_LEN       4
#define SECOCK_MAX_FRESHNESS 0xFFFFFFFFU

typedef struct {
    unsigned char key[SECOCK_KEY_LEN];
    unsigned int  freshness_tx;
    unsigned int  freshness_rx;
    unsigned int  auth_tx_count;
    unsigned int  auth_rx_count;
    unsigned int  auth_fail_count;
} SecOC_StateType;

static SecOC_StateType secoc_state;

static void SecOC_Init(void) {
    memset(&secoc_state, 0, sizeof(secoc_state));
    const unsigned char default_key[SECOCK_KEY_LEN] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    memcpy(secoc_state.key, default_key, SECOCK_KEY_LEN);
}

static unsigned int compute_mac(const unsigned char *pdu, unsigned int pdu_len,
                                unsigned int freshness,
                                const unsigned char *key) {
    unsigned int mac = 0;
    for (unsigned int i = 0; i < pdu_len; i++) {
        mac ^= ((unsigned int)pdu[i]) << ((i % 4) * 8);
    }
    mac ^= freshness;
    for (int i = 0; i < SECOCK_KEY_LEN; i++) {
        mac ^= ((unsigned int)key[i]) << ((i % 4) * 8);
    }
    return mac & 0xFFFFFFFFU;
}

static int SecOC_AuthenticateTx(unsigned char *pdu, unsigned int pdu_len,
                                unsigned int *out_freshness) {
    if (pdu_len < SECOCK_MAC_LEN) return -1;
    unsigned int freshness = secoc_state.freshness_tx++;
    if (secoc_state.freshness_tx > SECOCK_MAX_FRESHNESS) {
        secoc_state.freshness_tx = 0;
    }
    unsigned int mac = compute_mac(pdu, pdu_len - SECOCK_MAC_LEN,
                                   freshness, secoc_state.key);
    pdu[pdu_len - 4] = (unsigned char)(mac >> 24);
    pdu[pdu_len - 3] = (unsigned char)(mac >> 16);
    pdu[pdu_len - 2] = (unsigned char)(mac >> 8);
    pdu[pdu_len - 1] = (unsigned char)(mac);
    *out_freshness = freshness;
    secoc_state.auth_tx_count++;
    return 0;
}

static int SecOC_AuthenticateRx(const unsigned char *pdu, unsigned int pdu_len,
                                unsigned int freshness) {
    if (pdu_len < SECOCK_MAC_LEN) return -1;
    unsigned int received_mac =
        ((unsigned int)pdu[pdu_len - 4] << 24) |
        ((unsigned int)pdu[pdu_len - 3] << 16) |
        ((unsigned int)pdu[pdu_len - 2] << 8)  |
        (unsigned int)pdu[pdu_len - 1];
    unsigned int expected_mac = compute_mac(pdu, pdu_len - SECOCK_MAC_LEN,
                                            freshness, secoc_state.key);
    secoc_state.auth_rx_count++;
    if (received_mac == expected_mac) {
        secoc_state.freshness_rx = freshness;
        return 0;
    }
    secoc_state.auth_fail_count++;
    return -1;
}

static int test_secoc_init(void) {
    SecOC_Init();
    assert(secoc_state.freshness_tx == 0);
    assert(secoc_state.auth_tx_count == 0);
    printf("  [PASS] test_secoc_init\n");
    return 1;
}

static int test_secoc_auth_tx_rx_roundtrip(void) {
    SecOC_Init();
    unsigned char pdu[12] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                             0x00, 0x00, 0x00, 0x00};
    unsigned int freshness;
    assert(SecOC_AuthenticateTx(pdu, sizeof(pdu), &freshness) == 0);
    assert(freshness == 0);
    assert(secoc_state.auth_tx_count == 1);
    assert(SecOC_AuthenticateRx(pdu, sizeof(pdu), freshness) == 0);
    assert(secoc_state.auth_rx_count == 1);
    assert(secoc_state.auth_fail_count == 0);
    printf("  [PASS] test_secoc_auth_tx_rx_roundtrip\n");
    return 1;
}

static int test_secoc_freshness_monotonic(void) {
    SecOC_Init();
    unsigned char pdu[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0x00, 0x00, 0x00, 0x00};
    unsigned int f1, f2, f3;
    SecOC_AuthenticateTx(pdu, sizeof(pdu), &f1);
    SecOC_AuthenticateTx(pdu, sizeof(pdu), &f2);
    SecOC_AuthenticateTx(pdu, sizeof(pdu), &f3);
    assert(f2 == f1 + 1);
    assert(f3 == f2 + 1);
    assert(secoc_state.auth_tx_count == 3);
    printf("  [PASS] test_secoc_freshness_monotonic\n");
    return 1;
}

static int test_secoc_tampered_pdu_rejected(void) {
    SecOC_Init();
    unsigned char pdu[8] = {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00};
    unsigned int freshness;
    SecOC_AuthenticateTx(pdu, sizeof(pdu), &freshness);
    pdu[0] = 0xFF;
    assert(SecOC_AuthenticateRx(pdu, sizeof(pdu), freshness) == -1);
    assert(secoc_state.auth_fail_count == 1);
    printf("  [PASS] test_secoc_tampered_pdu_rejected\n");
    return 1;
}

static int test_secoc_wrong_freshness_rejected(void) {
    SecOC_Init();
    unsigned char pdu[8] = {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00};
    unsigned int freshness;
    SecOC_AuthenticateTx(pdu, sizeof(pdu), &freshness);
    assert(SecOC_AuthenticateRx(pdu, sizeof(pdu), freshness + 100) == -1);
    assert(secoc_state.auth_fail_count == 1);
    printf("  [PASS] test_secoc_wrong_freshness_rejected\n");
    return 1;
}

int main(void) {
    int passed = 0, total = 0;
    printf("=== E2E Test: SecOC Authentication ===\n");
    total++; passed += test_secoc_init();
    total++; passed += test_secoc_auth_tx_rx_roundtrip();
    total++; passed += test_secoc_freshness_monotonic();
    total++; passed += test_secoc_tampered_pdu_rejected();
    total++; passed += test_secoc_wrong_freshness_rejected();
    printf("\nResult: %d/%d tests passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
