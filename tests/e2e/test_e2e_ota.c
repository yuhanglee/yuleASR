/**
 * @file test_e2e_ota.c
 * @brief E2E Test: OTA Update Flow
 *
 * Verifies the OTA update lifecycle:
 *   Download -> Integrity Check (CRC32) -> Install -> Verify -> Rollback
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define OTA_MAX_IMAGE_SIZE  1024
#define OTA_CRC_INIT        0xFFFFFFFFU

typedef enum {
    OTA_STATE_IDLE = 0,
    OTA_STATE_DOWNLOADING,
    OTA_STATE_DOWNLOADED,
    OTA_STATE_VERIFYING,
    OTA_STATE_VERIFIED,
    OTA_STATE_INSTALLING,
    OTA_STATE_INSTALLED,
    OTA_STATE_FAILED
} Ota_StateEnum;

typedef struct {
    Ota_StateEnum state;
    unsigned char image[OTA_MAX_IMAGE_SIZE];
    unsigned int  image_len;
    unsigned int  stored_crc;
    unsigned int  version_major;
    unsigned int  version_minor;
    unsigned int  rollback_count;
} Ota_ContextType;

static Ota_ContextType ota;

static unsigned int ota_crc32(const unsigned char *data, unsigned int len) {
    unsigned int crc = OTA_CRC_INIT;
    for (unsigned int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320U;
            else crc >>= 1;
        }
    }
    return crc;
}

static void Ota_Init(void) {
    memset(&ota, 0, sizeof(ota));
    ota.state = OTA_STATE_IDLE;
}

static int Ota_StartDownload(unsigned int expected_len) {
    if (expected_len > OTA_MAX_IMAGE_SIZE) return -1;
    ota.image_len = expected_len;
    ota.state = OTA_STATE_DOWNLOADING;
    return 0;
}

static int Ota_WriteChunk(const unsigned char *data, unsigned int offset,
                          unsigned int len) {
    if (ota.state != OTA_STATE_DOWNLOADING) return -1;
    if (offset + len > ota.image_len) return -1;
    memcpy(&ota.image[offset], data, len);
    return 0;
}

static int Ota_FinishDownload(unsigned int crc) {
    if (ota.state != OTA_STATE_DOWNLOADING) return -1;
    ota.stored_crc = crc;
    ota.state = OTA_STATE_DOWNLOADED;
    return 0;
}

static int Ota_Verify(void) {
    if (ota.state != OTA_STATE_DOWNLOADED) return -1;
    ota.state = OTA_STATE_VERIFYING;
    unsigned int computed = ota_crc32(ota.image, ota.image_len);
    if (computed != ota.stored_crc) {
        ota.state = OTA_STATE_FAILED;
        return -1;
    }
    ota.state = OTA_STATE_VERIFIED;
    return 0;
}

static int Ota_Install(void) {
    if (ota.state != OTA_STATE_VERIFIED) return -1;
    ota.state = OTA_STATE_INSTALLING;
    ota.version_major = 2;
    ota.version_minor = 0;
    ota.state = OTA_STATE_INSTALLED;
    return 0;
}

static int Ota_Rollback(void) {
    if (ota.state != OTA_STATE_FAILED && ota.state != OTA_STATE_INSTALLING)
        return -1;
    ota.rollback_count++;
    ota.state = OTA_STATE_IDLE;
    memset(ota.image, 0, sizeof(ota.image));
    ota.image_len = 0;
    return 0;
}

static int test_ota_download_and_verify(void) {
    Ota_Init();
    unsigned char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    unsigned int crc = ota_crc32(data, sizeof(data));

    assert(Ota_StartDownload(sizeof(data)) == 0);
    assert(Ota_WriteChunk(data, 0, sizeof(data)) == 0);
    assert(Ota_FinishDownload(crc) == 0);
    assert(ota.state == OTA_STATE_DOWNLOADED);
    assert(Ota_Verify() == 0);
    assert(ota.state == OTA_STATE_VERIFIED);
    printf("  [PASS] test_ota_download_and_verify\n");
    return 1;
}

static int test_ota_install_after_verify(void) {
    Ota_Init();
    unsigned char data[] = {0xAA, 0xBB, 0xCC, 0xDD};
    unsigned int crc = ota_crc32(data, sizeof(data));

    Ota_StartDownload(sizeof(data));
    Ota_WriteChunk(data, 0, sizeof(data));
    Ota_FinishDownload(crc);
    assert(Ota_Verify() == 0);
    assert(Ota_Install() == 0);
    assert(ota.state == OTA_STATE_INSTALLED);
    assert(ota.version_major == 2);
    printf("  [PASS] test_ota_install_after_verify\n");
    return 1;
}

static int test_ota_corrupted_image_rejected(void) {
    Ota_Init();
    unsigned char data[] = {0x01, 0x02, 0x03, 0x04};
    unsigned int bad_crc = 0xDEADBEEF;

    Ota_StartDownload(sizeof(data));
    Ota_WriteChunk(data, 0, sizeof(data));
    Ota_FinishDownload(bad_crc);
    assert(Ota_Verify() == -1);
    assert(ota.state == OTA_STATE_FAILED);
    printf("  [PASS] test_ota_corrupted_image_rejected\n");
    return 1;
}

static int test_ota_rollback(void) {
    Ota_Init();
    unsigned char data[] = {0x01, 0x02};
    Ota_StartDownload(sizeof(data));
    Ota_WriteChunk(data, 0, sizeof(data));
    Ota_FinishDownload(0xDEADBEEF);
    Ota_Verify();
    assert(ota.state == OTA_STATE_FAILED);
    assert(Ota_Rollback() == 0);
    assert(ota.state == OTA_STATE_IDLE);
    assert(ota.rollback_count == 1);
    printf("  [PASS] test_ota_rollback\n");
    return 1;
}

static int test_ota_oversized_image_rejected(void) {
    Ota_Init();
    assert(Ota_StartDownload(OTA_MAX_IMAGE_SIZE + 1) == -1);
    assert(ota.state == OTA_STATE_IDLE);
    printf("  [PASS] test_ota_oversized_image_rejected\n");
    return 1;
}

int main(void) {
    int passed = 0, total = 0;
    printf("=== E2E Test: OTA Update Flow ===\n");
    total++; passed += test_ota_download_and_verify();
    total++; passed += test_ota_install_after_verify();
    total++; passed += test_ota_corrupted_image_rejected();
    total++; passed += test_ota_rollback();
    total++; passed += test_ota_oversized_image_rejected();
    printf("\nResult: %d/%d tests passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
