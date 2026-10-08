/**
 * @file test_memory.c
 * @brief Performance Test: BSW Memory Profiling
 *
 * Exercises NvM block operations and Com signal buffers under valgrind
 * to detect leaks and invalid accesses in BSW memory paths.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NVM_BLOCK_SIZES  {8, 64, 256, 1024, 4096}
#define NVM_NUM_BLOCKS   5
#define COM_BUF_SIZE     512

typedef struct {
    unsigned char *data;
    unsigned int   size;
    unsigned int   block_id;
    int            valid;
} Nvm_BlockType;

static Nvm_BlockType nvm_blocks[NVM_NUM_BLOCKS];
static unsigned int block_sizes[NVM_NUM_BLOCKS] = NVM_BLOCK_SIZES;

static void Nvm_Init(void) {
    for (int i = 0; i < NVM_NUM_BLOCKS; i++) {
        nvm_blocks[i].data = (unsigned char *)malloc(block_sizes[i]);
        nvm_blocks[i].size = block_sizes[i];
        nvm_blocks[i].block_id = (unsigned int)i;
        nvm_blocks[i].valid = 1;
        memset(nvm_blocks[i].data, 0, block_sizes[i]);
    }
}

static void Nvm_Deinit(void) {
    for (int i = 0; i < NVM_NUM_BLOCKS; i++) {
        free(nvm_blocks[i].data);
        nvm_blocks[i].data = NULL;
        nvm_blocks[i].valid = 0;
    }
}

static int Nvm_Write(unsigned int block_id, const unsigned char *src,
                     unsigned int len) {
    if (block_id >= NVM_NUM_BLOCKS || !nvm_blocks[block_id].valid) return -1;
    if (len > nvm_blocks[block_id].size) return -1;
    memcpy(nvm_blocks[block_id].data, src, len);
    return 0;
}

static int Nvm_Read(unsigned int block_id, unsigned char *dst, unsigned int len) {
    if (block_id >= NVM_NUM_BLOCKS || !nvm_blocks[block_id].valid) return -1;
    if (len > nvm_blocks[block_id].size) return -1;
    memcpy(dst, nvm_blocks[block_id].data, len);
    return 0;
}

static int test_nvm_all_block_sizes(void) {
    unsigned char src[4096], dst[4096];
    memset(src, 0x55, sizeof(src));

    for (int i = 0; i < NVM_NUM_BLOCKS; i++) {
        assert(Nvm_Write((unsigned int)i, src, block_sizes[i]) == 0);
        memset(dst, 0, sizeof(dst));
        assert(Nvm_Read((unsigned int)i, dst, block_sizes[i]) == 0);
        assert(memcmp(src, dst, block_sizes[i]) == 0);
    }
    printf("  [PASS] test_nvm_all_block_sizes\n");
    return 1;
}

static int test_nvm_write_read_cycles(void) {
    unsigned char src[64], dst[64];
    memset(src, 0xAA, sizeof(src));

    for (int cycle = 0; cycle < 100; cycle++) {
        src[0] = (unsigned char)cycle;
        assert(Nvm_Write(1, src, sizeof(src)) == 0);
        memset(dst, 0, sizeof(dst));
        assert(Nvm_Read(1, dst, sizeof(src)) == 0);
        assert(dst[0] == (unsigned char)cycle);
    }
    printf("  [PASS] test_nvm_write_read_cycles\n");
    return 1;
}

static int test_com_buffer_alloc_free(void) {
    for (int i = 0; i < 50; i++) {
        unsigned char *buf = (unsigned char *)malloc(COM_BUF_SIZE);
        assert(buf != NULL);
        memset(buf, (unsigned char)i, COM_BUF_SIZE);
        assert(buf[0] == (unsigned char)i);
        assert(buf[COM_BUF_SIZE - 1] == (unsigned char)i);
        free(buf);
    }
    printf("  [PASS] test_com_buffer_alloc_free\n");
    return 1;
}

static int test_nvm_boundary_access(void) {
    unsigned char src[8] = {0};
    assert(Nvm_Write(0, src, 8) == 0);
    assert(Nvm_Write(NVM_NUM_BLOCKS, src, 8) == -1);
    assert(Nvm_Write(0, src, block_sizes[0] + 1) == -1);
    printf("  [PASS] test_nvm_boundary_access\n");
    return 1;
}

int main(void) {
    int passed = 0, total = 0;
    printf("=== Performance Test: BSW Memory Profiling ===\n");

    Nvm_Init();
    total++; passed += test_nvm_all_block_sizes();
    total++; passed += test_nvm_write_read_cycles();
    total++; passed += test_com_buffer_alloc_free();
    total++; passed += test_nvm_boundary_access();
    Nvm_Deinit();

    printf("\nResult: %d/%d tests passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
