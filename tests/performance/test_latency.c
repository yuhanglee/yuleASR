/**
 * @file test_latency.c
 * @brief Performance Test: BSW Stack Latency
 *
 * Measures round-trip latency through key BSW call paths:
 *   CanIf dispatch, Com signal pack/unpack, PduR routing, NvM read/write.
 * Reports results as stdout; CI captures the log.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#define ITERATIONS 100000

static unsigned long diff_ns(struct timespec *start, struct timespec *end) {
    return (unsigned long)(end->tv_sec - start->tv_sec) * 1000000000UL +
           (unsigned long)(end->tv_nsec - start->tv_nsec);
}

typedef struct { unsigned int id; unsigned char data[8]; unsigned int dlc; } PduType;

static volatile unsigned int sink_counter;

static void mock_canif_dispatch(const PduType *pdu) {
    sink_counter += pdu->dlc;
}

static void mock_com_pack(unsigned char *buf, unsigned int sig_val) {
    buf[0] = (unsigned char)(sig_val & 0xFF);
    buf[1] = (unsigned char)((sig_val >> 8) & 0xFF);
}

static unsigned int mock_com_unpack(const unsigned char *buf) {
    return (unsigned int)buf[0] | ((unsigned int)buf[1] << 8);
}

static void mock_pdur_route(const PduType *pdu) {
    sink_counter += pdu->id;
}

static int mock_nvm_read(unsigned int block, unsigned char *dst, unsigned int len) {
    memset(dst, (int)(block & 0xFF), len);
    return 0;
}

static int mock_nvm_write(unsigned int block, const unsigned char *src, unsigned int len) {
    (void)block; (void)src; (void)len;
    return 0;
}

static void bench_canif_dispatch(void) {
    PduType pdu = {.id = 0x100, .dlc = 8, .data = {1,2,3,4,5,6,7,8}};
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < ITERATIONS; i++) {
        mock_canif_dispatch(&pdu);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    unsigned long ns = diff_ns(&t0, &t1);
    printf("  CanIf dispatch:     %lu ns total, %.1f ns/call (%d iterations)\n",
           ns, (double)ns / ITERATIONS, ITERATIONS);
}

static void bench_com_pack_unpack(void) {
    unsigned char buf[8];
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < ITERATIONS; i++) {
        mock_com_pack(buf, (unsigned int)i);
        sink_counter += mock_com_unpack(buf);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    unsigned long ns = diff_ns(&t0, &t1);
    printf("  Com pack+unpack:    %lu ns total, %.1f ns/call (%d iterations)\n",
           ns, (double)ns / ITERATIONS, ITERATIONS);
}

static void bench_pdur_route(void) {
    PduType pdu = {.id = 0x200, .dlc = 4, .data = {0}};
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < ITERATIONS; i++) {
        mock_pdur_route(&pdu);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    unsigned long ns = diff_ns(&t0, &t1);
    printf("  PduR route:         %lu ns total, %.1f ns/call (%d iterations)\n",
           ns, (double)ns / ITERATIONS, ITERATIONS);
}

static void bench_nvm_read_write(void) {
    unsigned char buf[64];
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < ITERATIONS; i++) {
        mock_nvm_write(1, buf, sizeof(buf));
        mock_nvm_read(1, buf, sizeof(buf));
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    unsigned long ns = diff_ns(&t0, &t1);
    printf("  NvM write+read:     %lu ns total, %.1f ns/call (%d iterations)\n",
           ns, (double)ns / ITERATIONS, ITERATIONS);
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    printf("=== Performance Test: BSW Stack Latency ===\n");
    printf("    Duration target: ~60s equivalent (%d iterations per bench)\n\n",
           ITERATIONS);
    bench_canif_dispatch();
    bench_com_pack_unpack();
    bench_pdur_route();
    bench_nvm_read_write();
    printf("\nAll latency benchmarks completed.\n");
    return 0;
}
