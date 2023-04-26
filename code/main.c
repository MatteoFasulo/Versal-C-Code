#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "platform.h"
#include "xil_printf.h"
#include "pmuv3.h"
 
typedef u64 uint64_t;
typedef u32 uint32_t;
 
#define NUM_ACCESSES 1000
#define L2_CACHE_SIZE 0x100000 // 1024KB => 1MB
#define CACHE_LINE_SIZE 64     // 64B
 
u32 read_cache_misses_sequential(volatile u32 *memory, size_t size)
{
	u32 start, end;
    size_t i;
 
    if (NUM_ACCESSES==1)
    {
    	// Sequential read
    	start = armv8pmu_read_counter(ARMV8_IDX_COUNTER1);
    	for (i = 0; i < NUM_ACCESSES; i += CACHE_LINE_SIZE)
    	{
    		u32 value = memory[i];
    	    (void)value;
    	}
    	end = armv8pmu_read_counter(ARMV8_IDX_COUNTER1);
    } else {
    	// Sequential read
    	start = armv8pmu_read_counter(ARMV8_IDX_COUNTER1);
    	    	for (i = 0; i < size; i += CACHE_LINE_SIZE)
    	    	{
    	    		u32 value = memory[i];
    	    	    (void)value;
    	    	}
    	end = armv8pmu_read_counter(ARMV8_IDX_COUNTER1);
    }
    return (end - start);
}
 
u64 read_cache_misses_random(volatile u32 *memory, size_t size)
{
    u64 start, end;
    size_t i;
 
    // Start measuring performance counter
    start = armv8pmu_read_counter(ARMV8_IDX_COUNTER1);
 
    // Random read
    for (i = 0; i < NUM_ACCESSES; i++)
    {
        size_t index = rand() % size;
        u32 value = memory[index];
        (void)value;
    }
    // Stop measuring performance counter
    end = armv8pmu_read_counter(ARMV8_IDX_COUNTER1);
 
    return (end - start);
}
 
u64 write_cache_misses_sequential(volatile u32 *memory, size_t size)
{
    u64 start, end;
    int i;
    unsigned long long sum = 0;
 
    start = armv8pmu_read_counter(ARMV8_IDX_COUNTER1);
    for (i = 0; i < NUM_ACCESSES; i++)
    {
    	memory[i] = 1;
    	sum += memory[i];
    }
    end = armv8pmu_read_counter(ARMV8_IDX_COUNTER1);
 
    return (end - start);
}
 
u64 write_cache_misses_random(volatile u32 *memory, size_t size)
{
    u64 start, end;
    unsigned long long sum = 0;
 
    // Random access pattern
    start = armv8pmu_read_counter(ARMV8_IDX_COUNTER1);
    for (int i = 0; i < NUM_ACCESSES; i++)
    {
        int rand_index = rand() % size;
        memory[rand_index * CACHE_LINE_SIZE] = 1;
        sum += memory[rand_index * CACHE_LINE_SIZE];
    }
    end = armv8pmu_read_counter(ARMV8_IDX_COUNTER1);
 
    return (end - start);
}
 
// Read Cache Size ID Register
static inline u32 armv8pmu_read_cache_size()
{
    u64 val = 0;
 
    __asm__ volatile("mrs %0, ccsidr_el1"
                     : "=r"(val));
    switch (val)
    {
    case 0x701FE00A:
        printf("Cache size: 32 KB\n\n");
        break;
    case 0x201FE012:
        printf("Cache size: 48 KB\n\n");
        break;
    case 0x703FE07A:
        printf("Cache size: 512 KB\n\n");
        break;
    case 0x707FE07A:
        printf("Cache size: 1024 KB\n\n");
        break;
    case 0x70FFE07A:
        printf("Cache size: 2048 KB\n\n");
        break;
    case 0x71FFE07A:
        printf("Cache size: 4096 KB\n\n");
        break;
    default:
        printf("Unknown value: 0x%lx\n", val);
    }
    return (u32)val;
}
 
int main()
{
	init_platform();
 
	u32 start, end;
 
    armv8pmu_reset();
 
    // Initialize random seed
    srand(12345);
 
    armv8pmu_init_counter(ARMV8_IDX_COUNTER1, L2D_CACHE_REFILL);
    armv8pmu_init_counter(ARMV8_IDX_COUNTER2, MEM_ACCESS);
    armv8pmu_start();
 
    volatile u32 *memory;
    size_t size = NUM_ACCESSES * CACHE_LINE_SIZE;
    int m_size = (L2_CACHE_SIZE * 10);
    memory = malloc(m_size);
 
    // Measure cache misses for sequential read
    start = armv8pmu_read_counter(ARMV8_IDX_COUNTER2);
    u32 read_misses_seq = read_cache_misses_sequential(memory, size);
    end = armv8pmu_read_counter(ARMV8_IDX_COUNTER2);
    printf("\nSequential read cache misses: %d\n", read_misses_seq);
    printf("\nMemory Access: %d\n", (end - start));
 
    // Free the memory block
    free((void *)memory);
 
    memory = malloc(L2_CACHE_SIZE * 10);
 
    // Measure cache misses for random read
    u32 read_misses_rand = read_cache_misses_random(memory, size);
    printf("\nRandom read cache misses: %d\n", read_misses_rand);
 
    // Free the memory block
    free((void *)memory);
 
    // Allocate a memory block that is larger than the L2 cache size
    memory = malloc(L2_CACHE_SIZE * 10);
 
    // Measure cache misses for sequential write
    u32 write_misses_seq = write_cache_misses_sequential(memory, size);
    printf("\nSequential write cache misses: %d\n", write_misses_seq);
 
    // Free the memory block
    free((void *)memory);
 
    memory = malloc(L2_CACHE_SIZE * 10);
 
    // Measure cache misses for random write
    u32 write_misses_rand = write_cache_misses_random(memory, size);
    printf("\nRandom write cache misses: %d\n", write_misses_rand);
 
    // Free the memory block
    free((void *)memory);
 
    armv8pmu_stop();
 
    cleanup_platform();
    return 0;
}
