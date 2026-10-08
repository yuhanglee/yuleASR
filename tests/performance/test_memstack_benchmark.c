/**
 * @file test_memstack_benchmark.c
 * @brief P1 Phase 8 — memory-stack performance benchmark (host build).
 *
 * Compiles the production NvM.c into this translation unit (#include) so the
 * STATIC CRC helpers (NvM_CalculateCrc8/16/32) are reachable, and measures
 * with clock_gettime(CLOCK_MONOTONIC) statistics (min/max/avg/p95/p99 via
 * perf_stats.h):
 *
 *   1. CRC correctness gate: the P1 Phase 8 lookup-table implementation
 *      (NvM_Crc8Table/16/32, 1792 bytes flash) is asserted bit-exact against
 *      local per-bit reference implementations (same polynomials 0x1D /
 *      0x1021 / 0x04C11DB7, init 0xFF / 0xFFFF / 0xFFFFFFFF, MSB first) over
 *      a matrix of buffer sizes.
 *   2. CRC throughput per algorithm at block sizes 8/64/256/1024/4096 bytes.
 *   3. NvM_WriteBlock / NvM_ReadBlock full job-cycle time against a RAM-backed
 *      MemIf stub (queue -> CRC -> MemIf -> completion polling), for blocks of
 *      64 B/CRC16, 256 B/CRC32, 1024 B/CRC32 and 8 B/CRC8.
 *
 * The four block descriptors use contiguous BlockIds 1..4, which activates
 * the P1 Phase 8 O(1) direct-index fast path of NvM_GetBlockDescriptor (the
 * layout constraint is validated by NvM_Init).
 */

#include "unity.h"
#include "NvM.h"
#include "MemIf.h"
#include "perf_stats.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* Local per-bit CRC references (identical algorithms to the pre-P1     */
/* bitwise NvM.c loops; used to prove the lookup tables are exact).     */
/* ------------------------------------------------------------------ */

static uint8_t ref_crc8(const uint8_t* data, uint32_t len)
{
    uint8_t crc = 0xFFu;
    uint32_t i;
    uint8_t b;

    for (i = 0u; i < len; i++) {
        crc ^= data[i];
        for (b = 0u; b < 8u; b++) {
            if ((crc & 0x80u) != 0u) {
                crc = (uint8_t)((uint8_t)(crc << 1) ^ 0x1Du);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

static uint16_t ref_crc16(const uint8_t* data, uint32_t len)
{
    uint16_t crc = 0xFFFFu;
    uint32_t i;
    uint8_t b;

    for (i = 0u; i < len; i++) {
        crc = (uint16_t)(crc ^ (uint16_t)((uint16_t)data[i] << 8));
        for (b = 0u; b < 8u; b++) {
            if ((crc & 0x8000u) != 0u) {
                crc = (uint16_t)((uint16_t)(crc << 1) ^ 0x1021u);
            } else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }
    return crc;
}

static uint32_t ref_crc32(const uint8_t* data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    uint32_t i;
    uint8_t b;

    for (i = 0u; i < len; i++) {
        crc ^= ((uint32_t)data[i]) << 24;
        for (b = 0u; b < 8u; b++) {
            if ((crc & 0x80000000u) != 0u) {
                crc = (crc << 1) ^ 0x04C11DB7u;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

/* ------------------------------------------------------------------ */
/* Det mock                                                            */
/* ------------------------------------------------------------------ */

static uint32_t mock_DetCalls = 0u;

Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId)
{
    (void)ModuleId;
    (void)InstanceId;
    (void)ApiId;
    (void)ErrorId;
    mock_DetCalls++;
    return E_OK;
}

/* ------------------------------------------------------------------ */
/* SUT: production NvM.c compiled into this TU (STATIC reachability)   */
/* ------------------------------------------------------------------ */

#include "NvM.c"

/* ------------------------------------------------------------------ */
/* RAM-backed MemIf driver stub                                        */
/* ------------------------------------------------------------------ */

#define MEMB_NUM_BLOCKS (4u)
#define MEMB_MAX_RAW    (1024u + 8u)  /* largest NvBlockLength + CRC32 */

static uint8_t memb_store[MEMB_NUM_BLOCKS][MEMB_MAX_RAW];

/* forward: benchmark NvM configuration (defined below, filled by
 * init_config()); the MemIf stubs below need the descriptor table to derive
 * the raw frame length MemIf_Write has to copy (it carries no length). */
static NvM_BlockDescriptorType testBlockDescriptors[MEMB_NUM_BLOCKS];
static NvM_ConfigType testConfig;

static const NvM_BlockDescriptorType* memb_find_block(uint16_t blockNumber)
{
    uint32_t i;
    for (i = 0u; i < MEMB_NUM_BLOCKS; i++) {
        if (testBlockDescriptors[i].BlockBaseNumber == blockNumber) {
            return &testBlockDescriptors[i];
        }
    }
    return NULL;
}

static uint16_t memb_crc_len(NvM_BlockCrcType crcType)
{
    switch (crcType) {
        case NVM_CRC_8:  return 1u;
        case NVM_CRC_16: return 2u;
        case NVM_CRC_32: return 4u;
        default:         return 0u;
    }
}

Std_ReturnType MemIf_Read(uint8 DeviceIndex,
                          uint16 BlockNumber,
                          uint16 BlockOffset,
                          uint8* DataPtr,
                          uint16 Length)
{
    const NvM_BlockDescriptorType* blk = memb_find_block(BlockNumber);

    (void)DeviceIndex;
    if ((blk != NULL) && (DataPtr != NULL) &&
        ((uint32_t)BlockOffset + (uint32_t)Length <= MEMB_MAX_RAW)) {
        (void)memcpy(DataPtr, &memb_store[blk->BlockId - 1u][BlockOffset], (size_t)Length);
        return E_OK;
    }
    return E_NOT_OK;
}

Std_ReturnType MemIf_Write(uint8 DeviceIndex,
                           uint16 BlockNumber,
                           const uint8_t* DataPtr)
{
    const NvM_BlockDescriptorType* blk = memb_find_block(BlockNumber);

    (void)DeviceIndex;
    if ((blk != NULL) && (DataPtr != NULL)) {
        /* MemIf_Write carries no length; the raw frame size is the block
         * length plus the CRC of this block's configuration. */
        uint16_t len = (uint16_t)(blk->NvBlockLength + memb_crc_len(blk->CrcType));
        (void)memcpy(memb_store[blk->BlockId - 1u], DataPtr, (size_t)len);
        return E_OK;
    }
    return E_NOT_OK;
}

Std_ReturnType MemIf_EraseImmediateBlock(uint8 DeviceIndex, uint16 BlockNumber)
{
    (void)DeviceIndex;
    (void)BlockNumber;
    return E_OK;
}

Std_ReturnType MemIf_InvalidateBlock(uint8 DeviceIndex, uint16 BlockNumber)
{
    (void)DeviceIndex;
    (void)BlockNumber;
    return E_OK;
}

MemIf_StatusType MemIf_GetStatus(uint8 DeviceIndex)
{
    (void)DeviceIndex;
    return MEMIF_IDLE;
}

MemIf_JobResultType MemIf_GetJobResult(uint8 DeviceIndex)
{
    (void)DeviceIndex;
    return MEMIF_JOB_OK;
}

/* ------------------------------------------------------------------ */
/* Benchmark configuration                                             */
/* ------------------------------------------------------------------ */

static void init_config(void)
{
    (void)memset(testBlockDescriptors, 0, sizeof(testBlockDescriptors));
    (void)memset(&testConfig, 0, sizeof(testConfig));

    /* Block 1: 64 B, CRC16 */
    testBlockDescriptors[0].BlockId = 1u;
    testBlockDescriptors[0].DeviceId = 0u;
    testBlockDescriptors[0].BlockBaseNumber = 0x0011u;
    testBlockDescriptors[0].ManagementType = NVM_BLOCK_NATIVE;
    testBlockDescriptors[0].NumberOfNvBlocks = 1u;
    testBlockDescriptors[0].NvBlockLength = 64u;
    testBlockDescriptors[0].CrcType = NVM_CRC_16;
    testBlockDescriptors[0].BlockUseCrc = TRUE;

    /* Block 2: 256 B, CRC32 */
    testBlockDescriptors[1].BlockId = 2u;
    testBlockDescriptors[1].DeviceId = 0u;
    testBlockDescriptors[1].BlockBaseNumber = 0x0012u;
    testBlockDescriptors[1].ManagementType = NVM_BLOCK_NATIVE;
    testBlockDescriptors[1].NumberOfNvBlocks = 1u;
    testBlockDescriptors[1].NvBlockLength = 256u;
    testBlockDescriptors[1].CrcType = NVM_CRC_32;
    testBlockDescriptors[1].BlockUseCrc = TRUE;

    /* Block 3: 1024 B, CRC32 (NVM_MAX_BLOCK_SIZE) */
    testBlockDescriptors[2].BlockId = 3u;
    testBlockDescriptors[2].DeviceId = 0u;
    testBlockDescriptors[2].BlockBaseNumber = 0x0013u;
    testBlockDescriptors[2].ManagementType = NVM_BLOCK_NATIVE;
    testBlockDescriptors[2].NumberOfNvBlocks = 1u;
    testBlockDescriptors[2].NvBlockLength = 1024u;
    testBlockDescriptors[2].CrcType = NVM_CRC_32;
    testBlockDescriptors[2].BlockUseCrc = TRUE;

    /* Block 4: 8 B, CRC8 */
    testBlockDescriptors[3].BlockId = 4u;
    testBlockDescriptors[3].DeviceId = 0u;
    testBlockDescriptors[3].BlockBaseNumber = 0x0014u;
    testBlockDescriptors[3].ManagementType = NVM_BLOCK_NATIVE;
    testBlockDescriptors[3].NumberOfNvBlocks = 1u;
    testBlockDescriptors[3].NvBlockLength = 8u;
    testBlockDescriptors[3].CrcType = NVM_CRC_8;
    testBlockDescriptors[3].BlockUseCrc = TRUE;

    testConfig.BlockDescriptors = testBlockDescriptors;
    testConfig.NumBlockDescriptors = MEMB_NUM_BLOCKS;
    testConfig.NumOfNvBlocks = 4u;
    testConfig.MaxNumberOfWriteRetries = 3u;
    testConfig.MaxNumberOfReadRetries = 3u;
    testConfig.MainFunctionPeriod = 10u;
}

/* ------------------------------------------------------------------ */
/* Campaigns                                                           */
/* ------------------------------------------------------------------ */

static const uint16_t crcSizes[5] = { 8u, 64u, 256u, 1024u, 4096u };
static const char* crcSizeNames[5] = { "8", "64", "256", "1024", "4096" };

static perf_stats_t g_crc8_by_size[5];
static perf_stats_t g_crc16_by_size[5];
static perf_stats_t g_crc32_by_size[5];
static perf_stats_t g_write_cycle[MEMB_NUM_BLOCKS];
static perf_stats_t g_read_cycle[MEMB_NUM_BLOCKS];

static uint8_t crc_buf[4096];
static uint8_t memb_wbuf[1024];
static uint8_t memb_rbuf[1024];
static volatile uint32_t g_crcSink = 0u;

void setUp(void)
{
    mock_DetCalls = 0u;
}

void tearDown(void)
{
}

/* --- 0: lookup-table CRCs must be bit-exact vs per-bit references --- */
void test_00_CrcTables_ReferenceEquivalence(void)
{
    static const uint32_t sizes[8] = { 1u, 7u, 8u, 63u, 64u, 100u, 255u, 1024u };
    uint32_t s;
    uint32_t i;

    for (i = 0u; i < 1024u; i++) {
        crc_buf[i] = (uint8_t)((i * 13u) + 7u);
    }

    for (s = 0u; s < 8u; s++) {
        uint32_t n = sizes[s];
        TEST_ASSERT_EQUAL_HEX8(ref_crc8(crc_buf, n), NvM_CalculateCrc8(crc_buf, (uint16_t)n));
        TEST_ASSERT_EQUAL_HEX16(ref_crc16(crc_buf, n), NvM_CalculateCrc16(crc_buf, (uint16_t)n));
        TEST_ASSERT_EQUAL_HEX32(ref_crc32(crc_buf, n), NvM_CalculateCrc32(crc_buf, (uint16_t)n));
    }
}

/* --- 1: CRC throughput per algorithm and size ----------------------- */
void test_10_CrcThroughput(void)
{
    const uint32_t iterations = 2000u;
    uint32_t a;
    uint32_t s;
    uint32_t i;
    uint32_t prefill;

    for (prefill = 0u; prefill < 4096u; prefill++) {
        crc_buf[prefill] = (uint8_t)(prefill * 31u);
    }

    for (a = 0u; a < 3u; a++) {
        for (s = 0u; s < 5u; s++) {
            uint16_t n = crcSizes[s];
            for (i = 0u; i < iterations; i++) {
                uint64_t t0;
                uint64_t t1;
                crc_buf[(i * 37u) & 4095u] = (uint8_t)i; /* defeat CSE */
                t0 = perf_now_ns();
                switch (a) {
                    case 0u: g_crcSink += NvM_CalculateCrc8(crc_buf, n);  break;
                    case 1u: g_crcSink += NvM_CalculateCrc16(crc_buf, n); break;
                    default: g_crcSink += NvM_CalculateCrc32(crc_buf, n); break;
                }
                t1 = perf_now_ns();
                switch (a) {
                    case 0u: perf_record(&g_crc8_by_size[s], t1 - t0);  break;
                    case 1u: perf_record(&g_crc16_by_size[s], t1 - t0); break;
                    default: perf_record(&g_crc32_by_size[s], t1 - t0); break;
                }
            }
        }
    }

    for (s = 0u; s < 5u; s++) {
        char title[96];
        (void)snprintf(title, sizeof(title), "NvM CRC8  throughput | %s-byte block (256-entry lookup table)", crcSizeNames[s]);
        perf_report(title, &g_crc8_by_size[s]);
        (void)snprintf(title, sizeof(title), "NvM CRC16 throughput | %s-byte block (512-entry lookup table)", crcSizeNames[s]);
        perf_report(title, &g_crc16_by_size[s]);
        (void)snprintf(title, sizeof(title), "NvM CRC32 throughput | %s-byte block (1024-entry lookup table)", crcSizeNames[s]);
        perf_report(title, &g_crc32_by_size[s]);
    }
    TEST_ASSERT_TRUE(g_crcSink != 0u);
}

/* --- 2: NvM block job cycles ---------------------------------------- */

/**
 * Drive NvM_MainFunction until the block job leaves the queue.
 *
 * Polls the internal JobPending flag (this TU includes NvM.c): the public
 * NvM_GetErrorStatus only reports LastResult, which still holds the PREVIOUS
 * job's value right after NvM_WriteBlock/NvM_ReadBlock queued a new job, so
 * a LastResult-based wait would spuriously succeed before the job starts.
 */
static boolean memb_drive_job(NvM_BlockIdType blockId)
{
    uint32_t guard = 0u;
    boolean done = FALSE;

    while ((NvM_InternalState.BlockStates[blockId].JobPending != 0U) &&
           (guard < 100u)) {
        NvM_MainFunction();
        guard++;
    }

    done = ((NvM_InternalState.BlockStates[blockId].JobPending == 0U) &&
            (NvM_InternalState.BlockStates[blockId].LastResult == NVM_REQ_OK))
               ? TRUE : FALSE;

    return done;
}

void test_20_NvM_BlockWriteReadCycles(void)
{
    const uint32_t cycles = 300u;
    uint32_t b;
    uint32_t i;

    init_config();
    NvM_Init(&testConfig);

    for (b = 0u; b < MEMB_NUM_BLOCKS; b++) {
        NvM_BlockIdType blockId = (NvM_BlockIdType)(b + 1u);
        NvM_BlockDescriptorType* blk = &testBlockDescriptors[b];
        const char* crcName;

        switch (blk->CrcType) {
            case NVM_CRC_8:  crcName = "CRC8";  break;
            case NVM_CRC_16: crcName = "CRC16"; break;
            default:         crcName = "CRC32"; break;
        }

        /* warmup + data integrity check through the full write/read path */
        for (i = 0u; i < blk->NvBlockLength; i++) {
            memb_wbuf[i] = (uint8_t)((i * 7u) + 0x40u);
        }
        TEST_ASSERT_EQUAL(E_OK, NvM_WriteBlock(blockId, memb_wbuf));
        TEST_ASSERT_TRUE(memb_drive_job(blockId));
        /* write completed; now read back and compare */
        TEST_ASSERT_EQUAL(E_OK, NvM_ReadBlock(blockId, memb_rbuf));
        TEST_ASSERT_TRUE(memb_drive_job(blockId));
        TEST_ASSERT_EQUAL_MEMORY(memb_wbuf, memb_rbuf, blk->NvBlockLength);

        /* timed write cycles */
        for (i = 0u; i < cycles; i++) {
            uint64_t t0;
            uint64_t t1;
            memb_wbuf[i % blk->NvBlockLength] = (uint8_t)i; /* vary CRC input */
            t0 = perf_now_ns();
            TEST_ASSERT_EQUAL(E_OK, NvM_WriteBlock(blockId, memb_wbuf));
            TEST_ASSERT_TRUE(memb_drive_job(blockId));
            t1 = perf_now_ns();
            perf_record(&g_write_cycle[b], t1 - t0);
        }

        /* timed read cycles */
        for (i = 0u; i < cycles; i++) {
            uint64_t t0;
            uint64_t t1;
            t0 = perf_now_ns();
            TEST_ASSERT_EQUAL(E_OK, NvM_ReadBlock(blockId, memb_rbuf));
            TEST_ASSERT_TRUE(memb_drive_job(blockId));
            t1 = perf_now_ns();
            perf_record(&g_read_cycle[b], t1 - t0);
        }

        {
            char title[96];
            (void)snprintf(title, sizeof(title), "NvM_WriteBlock job cycle | %u B, %s (RAM MemIf stub)",
                           (unsigned)blk->NvBlockLength, crcName);
            perf_report(title, &g_write_cycle[b]);
            (void)snprintf(title, sizeof(title), "NvM_ReadBlock  job cycle | %u B, %s (RAM MemIf stub)",
                           (unsigned)blk->NvBlockLength, crcName);
            perf_report(title, &g_read_cycle[b]);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Runner                                                              */
/* ------------------------------------------------------------------ */

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_00_CrcTables_ReferenceEquivalence);
    RUN_TEST(test_10_CrcThroughput);
    RUN_TEST(test_20_NvM_BlockWriteReadCycles);

    return UNITY_END();
}
