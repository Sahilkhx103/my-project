#include "heap_driver.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define HEAP_START_ADDR  ((uint8_t*)0x20001000)
#define HEAP_SIZE        (4 * 1024)
#define BLOCK_SIZE       16
#define BLOCK_COUNT      (HEAP_SIZE / BLOCK_SIZE)

// Students should be provided the above code (includes and defines) and the function declarations in this file.
// They can figure out the rest.

// Allocation bitmap: 0 = free, 1 = used
static uint8_t block_map[BLOCK_COUNT];
// Add you code below
void heap_init(void) {
    for (int i = 0; i < BLOCK_COUNT; i++) {
        block_map[i] = 0;
    }
}

void* heap_alloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    size_t blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    size_t contiguous_free = 0;
    int start_index = -1;


    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (block_map[i] == 0) {
            if (contiguous_free == 0) {
                start_index = i;
            }
            contiguous_free++;

            if (contiguous_free == blocks_needed) {
                for (int j = start_index; j < start_index + blocks_needed; j++) {
                    block_map[j] = 1;
                }
                return (void*)(HEAP_START_ADDR + (start_index * BLOCK_SIZE));
            }
        } else {
            contiguous_free = 0;
            start_index = -1;
        }
    }

    return NULL;
}

void heap_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    uint8_t* addr = (uint8_t*)ptr;

    if (addr < HEAP_START_ADDR || addr >= (HEAP_START_ADDR + HEAP_SIZE)) {
        return; 
    }

    int start_index = (addr - HEAP_START_ADDR) / BLOCK_SIZE;

    while (start_index < BLOCK_COUNT && block_map[start_index] == 1) {
        block_map[start_index] = 0;
        start_index++;
    }
}