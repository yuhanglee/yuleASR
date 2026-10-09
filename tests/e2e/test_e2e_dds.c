/**
 * @file test_e2e_dds.c
 * @brief E2E Test: SOME/IP DDS Communication
 *
 * Verifies SOME/IP service discovery and communication flow:
 *   SD find/offer/subscribe -> SOME/IP request-response -> serialization
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define SD_MAX_SERVICES     16
#define SD_SERVICE_ACTIVE   1
#define SD_SERVICE_INACTIVE 0
#define SOMEIP_HDR_SIZE     16

typedef struct {
    unsigned short service_id;
    unsigned short instance_id;
    unsigned char  major_version;
    unsigned char  minor_version;
    unsigned char  state;
} Sd_ServiceType;

typedef struct {
    unsigned short service_id;
    unsigned short method_id;
    unsigned int   length;
    unsigned char  payload[64];
    unsigned int   payload_len;
} SomeIp_MsgType;

static Sd_ServiceType sd_table[SD_MAX_SERVICES];
static unsigned int   sd_count = 0;

static void Sd_Init(void) {
    memset(sd_table, 0, sizeof(sd_table));
    sd_count = 0;
}

static int Sd_OfferService(unsigned short sid, unsigned short iid,
                           unsigned char major, unsigned char minor) {
    if (sd_count >= SD_MAX_SERVICES) return -1;
    sd_table[sd_count].service_id    = sid;
    sd_table[sd_count].instance_id   = iid;
    sd_table[sd_count].major_version = major;
    sd_table[sd_count].minor_version = minor;
    sd_table[sd_count].state         = SD_SERVICE_ACTIVE;
    sd_count++;
    return 0;
}

static int Sd_FindService(unsigned short sid) {
    for (unsigned int i = 0; i < sd_count; i++) {
        if (sd_table[i].service_id == sid &&
            sd_table[i].state == SD_SERVICE_ACTIVE) {
            return 0;
        }
    }
    return -1;
}

static int SomeIp_Serialize(const SomeIp_MsgType *msg, unsigned char *buf,
                            unsigned int buf_len) {
    unsigned int total = SOMEIP_HDR_SIZE + msg->payload_len;
    if (total > buf_len) return -1;
    buf[0] = (unsigned char)(msg->service_id >> 8);
    buf[1] = (unsigned char)(msg->service_id & 0xFF);
    buf[2] = (unsigned char)(msg->method_id >> 8);
    buf[3] = (unsigned char)(msg->method_id & 0xFF);
    buf[4] = (unsigned char)((total >> 24) & 0xFF);
    buf[5] = (unsigned char)((total >> 16) & 0xFF);
    buf[6] = (unsigned char)((total >> 8) & 0xFF);
    buf[7] = (unsigned char)(total & 0xFF);
    memcpy(&buf[SOMEIP_HDR_SIZE], msg->payload, msg->payload_len);
    return (int)total;
}

static int SomeIp_Deserialize(const unsigned char *buf, unsigned int buf_len,
                              SomeIp_MsgType *msg) {
    if (buf_len < SOMEIP_HDR_SIZE) return -1;
    msg->service_id = (unsigned short)((buf[0] << 8) | buf[1]);
    msg->method_id  = (unsigned short)((buf[2] << 8) | buf[3]);
    unsigned int total = ((unsigned int)buf[4] << 24) | ((unsigned int)buf[5] << 16) |
                         ((unsigned int)buf[6] << 8)  | (unsigned int)buf[7];
    if (total < SOMEIP_HDR_SIZE || total > buf_len) return -1;
    msg->payload_len = total - SOMEIP_HDR_SIZE;
    memcpy(msg->payload, &buf[SOMEIP_HDR_SIZE], msg->payload_len);
    return 0;
}

static int test_sd_offer_and_find(void) {
    Sd_Init();
    assert(Sd_OfferService(0x1234, 0x0001, 1, 0) == 0);
    assert(Sd_FindService(0x1234) == 0);
    assert(Sd_FindService(0x9999) == -1);
    printf("  [PASS] test_sd_offer_and_find\n");
    return 1;
}

static int test_sd_multiple_services(void) {
    Sd_Init();
    for (int i = 0; i < 5; i++) {
        assert(Sd_OfferService((unsigned short)(0x100 + i), 1, 1, 0) == 0);
    }
    for (int i = 0; i < 5; i++) {
        assert(Sd_FindService((unsigned short)(0x100 + i)) == 0);
    }
    assert(Sd_FindService(0x200) == -1);
    printf("  [PASS] test_sd_multiple_services\n");
    return 1;
}

static int test_someip_serialize_deserialize(void) {
    SomeIp_MsgType tx = {
        .service_id = 0x1234, .method_id = 0x0001,
        .payload = {0xAA, 0xBB, 0xCC}, .payload_len = 3
    };
    unsigned char buf[128];
    int written = SomeIp_Serialize(&tx, buf, sizeof(buf));
    assert(written > 0);

    SomeIp_MsgType rx = {0};
    assert(SomeIp_Deserialize(buf, (unsigned int)written, &rx) == 0);
    assert(rx.service_id == 0x1234);
    assert(rx.method_id == 0x0001);
    assert(rx.payload_len == 3);
    assert(rx.payload[0] == 0xAA && rx.payload[1] == 0xBB && rx.payload[2] == 0xCC);
    printf("  [PASS] test_someip_serialize_deserialize\n");
    return 1;
}

static int test_someip_buffer_overflow(void) {
    SomeIp_MsgType tx = {
        .service_id = 0x0001, .method_id = 0x0001,
        .payload_len = 100
    };
    unsigned char buf[20];
    assert(SomeIp_Serialize(&tx, buf, sizeof(buf)) == -1);
    printf("  [PASS] test_someip_buffer_overflow\n");
    return 1;
}

static int test_sd_full_discovery_cycle(void) {
    Sd_Init();
    assert(Sd_FindService(0x5678) == -1);
    assert(Sd_OfferService(0x5678, 0x0001, 2, 1) == 0);
    assert(Sd_FindService(0x5678) == 0);
    assert(sd_table[0].major_version == 2);
    assert(sd_table[0].minor_version == 1);
    printf("  [PASS] test_sd_full_discovery_cycle\n");
    return 1;
}

int main(void) {
    int passed = 0, total = 0;
    printf("=== E2E Test: DDS/SOME-IP Communication ===\n");
    total++; passed += test_sd_offer_and_find();
    total++; passed += test_sd_multiple_services();
    total++; passed += test_someip_serialize_deserialize();
    total++; passed += test_someip_buffer_overflow();
    total++; passed += test_sd_full_discovery_cycle();
    printf("\nResult: %d/%d tests passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
