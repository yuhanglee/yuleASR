/**
 * @file test_throughput.c
 * @brief Performance Test: BSW Stack Throughput
 *
 * Measures message/signal throughput through BSW paths:
 *   CanIf multi-PDU dispatch, Com multi-signal pack, NvM block transfer.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DURATION_SEC  2
#define MAX_PDUS      256

typedef struct { unsigned int id; unsigned char data[8]; unsigned int dlc; } PduType;

static volatile unsigned long g_ops;

static unsigned long elapsed_sec(struct timespec *t0, struct timespec *t1) {
    return (unsigned long)(t1->tv_sec - t0->tv_sec);
}

static void mock_canif_dispatch(const PduType *pdu) {
    (void)pdu;
    g_ops++;
}

static void mock_com_pack(unsigned char *buf, unsigned int val) {
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
}

static void bench_canif_throughput(void) {
    PduType pdus[MAX_PDUS];
    for (int i = 0; i < MAX_PDUS; i++) {
        pdus[i].id = (unsigned int)(0x100 + i);
        pdus[i].dlc = 8;
        memset(pdus[i].data, (unsigned char)i, 8);
    }

    struct timespec t0, t1;
    g_ops = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    do {
        for (int i = 0; i < MAX_PDUS; i++) {
            mock_canif_dispatch(&pdus[i]);
        }
        clock_gettime(CLOCK_MONOTONIC, &t1);
    } while (elapsed_sec(&t0, &t1) < DURATION_SEC);

    unsigned long secs = elapsed_sec(&t0, &t1);
    if (secs == 0) secs = 1;
    printf("  CanIf dispatch:     %lu ops in %lu s (%.0f ops/s)\n",
           g_ops, secs, (double)g_ops / (double)secs);
}

static void bench_com_throughput(void) {
    unsigned char buf[8];
    struct timespec t0, t1;
    g_ops = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    do {
        for (int i = 0; i < 1024; i++) {
            mock_com_pack(buf, (unsigned int)i);
            g_ops++;
        }
        clock_gettime(CLOCK_MONOTONIC, &t1);
    } while (elapsed_sec(&t0, &t1) < DURATION_SEC);

    unsigned long secs = elapsed_sec(&t0, &t1);
    if (secs == 0) secs = 1;
    printf("  Com signal pack:    %lu ops in %lu s (%.0f ops/s)\n",
           g_ops, secs, (double)g_ops / (double)secs);
}

static void bench_nvm_block_throughput(void) {
    unsigned char src[256], dst[256];
    memset(src, 0xAA, sizeof(src));
    struct timespec t0, t1;
    g_ops = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    do {
        memcpy(dst, src, sizeof(src));
        g_ops++;
        clock_gettime(CLOCK_MONOTONIC, &t1);
    } while (elapsed_sec(&t0, &t1) < DURATION_SEC);

    unsigned long secs = elapsed_sec(&t0, &t1);
    if (secs == 0) secs = 1;
    printf("  NvM 256B block xfer: %lu ops in %lu s (%.0f ops/s)\n",
           g_ops, secs, (double)g_ops / (double)secs);
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    printf("=== Performance Test: BSW Stack Throughput ===\n");
    printf("    Duration per benchmark: %d seconds\n\n", DURATION_SEC);
    bench_canif_throughput();
    bench_com_throughput();
    bench_nvm_block_throughput();
    printf("\nAll throughput benchmarks completed.\n");
    return 0;
}
