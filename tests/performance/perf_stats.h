/**
 * @file perf_stats.h
 * @brief Host-side performance measurement helpers for the P1 Phase 8
 *        benchmark suite (tests/performance).
 *
 * All timing uses clock_gettime(CLOCK_MONOTONIC) in nanoseconds — the same
 * timing source already used by tests/crypto_benchmark — so every benchmark
 * in this suite shares one consistent clock. Reported statistics per
 * measurement campaign: min / max / avg / p95 / p99.
 *
 * Header-only: every benchmark includes this file directly and compiles with
 * -O2; all helpers are static.
 */
#ifndef PERF_STATS_H
#define PERF_STATS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/** Upper bound of samples per measurement campaign. */
#define PERF_MAX_SAMPLES    (20001u)

/** Sample accumulator (~160 KB BSS per instance). */
typedef struct {
    uint64_t samples[PERF_MAX_SAMPLES];
    uint32_t count;
} perf_stats_t;

/** Monotonic wall clock, nanoseconds since an arbitrary epoch. */
static uint64_t perf_now_ns(void)
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    return (((uint64_t)ts.tv_sec) * 1000000000ULL) + (uint64_t)ts.tv_nsec;
}

/** Record one latency sample (nanoseconds); silently drops past the cap. */
static void perf_record(perf_stats_t* stats, uint64_t ns)
{
    if ((stats != NULL) && (stats->count < PERF_MAX_SAMPLES)) {
        stats->samples[stats->count] = ns;
        stats->count++;
    }
}

static int perf_u64_cmp(const void* a, const void* b)
{
    uint64_t x = *(const uint64_t*)a;
    uint64_t y = *(const uint64_t*)b;
    if (x < y) { return -1; }
    if (x > y) { return 1; }
    return 0;
}

/**
 * @brief   Percentile of a campaign (pct in 0..100).
 * @details Sorts an internal copy; the campaign's insertion order is kept,
 *          so a campaign can be reported and then still be asserted on.
 */
static uint64_t perf_pctl_ns(const perf_stats_t* stats, uint8 pct)
{
    uint64_t* copy;
    uint32_t n;
    uint32_t idx;
    uint64_t v = 0u;

    if ((stats == NULL) || (stats->count == 0u)) { return 0u; }
    n = stats->count;
    copy = (uint64_t*)malloc((size_t)n * sizeof(uint64_t));
    if (copy == NULL) { return 0u; }
    (void)memcpy(copy, stats->samples, (size_t)n * sizeof(uint64_t));
    (void)qsort(copy, (size_t)n, sizeof(uint64_t), perf_u64_cmp);
    idx = (((uint32_t)pct) * n) / 100u;
    if (idx >= n) { idx = n - 1u; }
    v = copy[idx];
    (void)free(copy);
    return v;
}

/** Arithmetic mean in nanoseconds (0 for an empty campaign). */
static uint64_t perf_avg_ns(const perf_stats_t* stats)
{
    uint64_t sum = 0u;
    uint32_t i;

    if ((stats == NULL) || (stats->count == 0u)) { return 0u; }
    for (i = 0u; i < stats->count; i++) { sum += stats->samples[i]; }
    return sum / (uint64_t)stats->count;
}

/**
 * @brief   Print one campaign as a min/max/avg/p95/p99 report line (in us).
 */
static void perf_report(const char* title, const perf_stats_t* stats)
{
    if ((stats == NULL) || (stats->count == 0u)) {
        (void)printf("[perf] %-64s : (no samples)\n", title);
        return;
    }
    (void)printf("[perf] %s\n", title);
    (void)printf("[perf]   n=%-6u min=%9.4f  avg=%9.4f  p95=%9.4f  p99=%9.4f  max=%9.4f  (us)\n",
                 stats->count,
                 (double)perf_pctl_ns(stats, 0u) / 1000.0,
                 (double)perf_avg_ns(stats) / 1000.0,
                 (double)perf_pctl_ns(stats, 95u) / 1000.0,
                 (double)perf_pctl_ns(stats, 99u) / 1000.0,
                 (double)perf_pctl_ns(stats, 100u) / 1000.0);
}

/** Print a plain annotation line inside the perf output block. */
static void perf_note(const char* text)
{
    (void)printf("[perf] %s\n", text);
}

#endif /* PERF_STATS_H */
