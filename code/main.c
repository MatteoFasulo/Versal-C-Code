#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "platform.h"
#include "xil_printf.h"

typedef u64 uint64_t;
typedef u32 uint32_t;

#define ARMV8_PMCR_MASK 0x3f
#define ARMV8_PMCR_E (1 << 0) /*  Enable all counters */
#define ARMV8_PMCR_P (1 << 1) /*  Reset all counters */
#define ARMV8_PMCR_C (1 << 2) /*  Cycle counter reset */

#define ARMV8_PMUSERENR_EN_EL0 (1 << 0) /*  EL0 access enable */
#define ARMV8_PMUSERENR_CR (1 << 2)     /*  Cycle counter read enable */
#define ARMV8_PMUSERENR_ER (1 << 3)     /*  Event counter read enable */

#define ARMV8_PMCNTENSET_EL0_ENABLE (1 << 31) /* *< Enable Perf count reg */

#define NUM_ACCESSES 1000
#define L2_CACHE_SIZE 0x100000 // 1024KB => 1MB
#define CACHE_LINE_SIZE 64     // 64B

// Performance Monitors Control Register
static inline u32 armv8pmu_pmcr_read(void)
{
    u64 val = 0;
    asm volatile("mrs %0, pmcr_el0"
                 : "=r"(val));
    return (u32)val;
}

// Performance Monitors Cycle Count Register
static inline u32 armv8pmu_pmccntr_read(void)
{
    u64 val = 0;
    asm volatile("mrs %0, pmccntr_el0"
                 : "=r"(val));
    return (u32)val;
}

// Set Cache Level
static inline void armv8pmu_set_cache_level()
{
    u64 val = 0x2;

    __asm__ volatile("msr csselr_el1, %0"
                     :
                     : "r"(val));
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

static inline void armv8pmu_pmcr_write(u32 val)
{
    val &= ARMV8_PMCR_MASK;
    asm volatile("msr pmcr_el0, %0"
                 :
                 : "r"((u64)val));
}

static inline void reset_pmuserenr_el0()
{
    __asm__ volatile("msr pmuserenr_el0, %0" ::"r"(0x0));
}

u64 read_cache_misses_sequential(volatile u32 *memory, size_t size)
{
    u64 start, end;
    u32 cache_misses = 0;
    size_t i;

    // Start measuring performance counter
    start = armv8pmu_pmccntr_read();

    // Sequential read
    for (i = 0; i < size; i += CACHE_LINE_SIZE)
    {
        u32 value = memory[i];
        (void)value;
    }
    // Stop measuring performance counter
    end = armv8pmu_pmccntr_read();

    // Count number of cache misses
    cache_misses = (end - start);

    return cache_misses;
}

u64 read_cache_misses_random(volatile u32 *memory, size_t size)
{
    u64 start, end;
    u32 cache_misses = 0;
    size_t i;

    // Start measuring performance counter
    start = armv8pmu_pmccntr_read();

    // Random read
    for (i = 0; i < NUM_ACCESSES; i++)
    {
        size_t index = rand() % size;
        u32 value = memory[index];
        (void)value;
    }
    // Stop measuring performance counter
    end = armv8pmu_pmccntr_read();

    // Count number of cache misses
    cache_misses = (end - start);

    return cache_misses;
}

u64 write_cache_misses_sequential(volatile u32 *memory)
{
    u64 start, end;
    u32 cache_misses = 0;

    start = armv8pmu_pmccntr_read();
    for (int i = 0; i < NUM_ACCESSES; i++)
    {
        volatile u32 x = memory[i % (L2_CACHE_SIZE / sizeof(u32))];
        (void)x; // suppress unused variable warning
    }
    end = armv8pmu_pmccntr_read();

    // Count number of cache misses
    cache_misses = (end - start);

    return cache_misses;
}

u64 write_cache_misses_random(volatile u32 *memory)
{
    u64 start, end;
    u32 cache_misses = 0;
    unsigned long long sum = 0;

    // Random access pattern
    start = armv8pmu_pmccntr_read();
    for (int i = 0; i < NUM_ACCESSES; i++)
    {
        int rand_index = rand() % NUM_ACCESSES;
        memory[rand_index * CACHE_LINE_SIZE] = 'a';
        sum += memory[rand_index * CACHE_LINE_SIZE];
    }
    end = armv8pmu_pmccntr_read();

    // Count number of cache misses
    cache_misses = (end - start);

    return cache_misses;
}

int main()
{
    init_platform();

    // Initialize random seed
    srand(12345);

    /* Reset pmuserenr_el0 to default */
    reset_pmuserenr_el0();

    /*  Enable user-mode access to counters. */
    __asm__ volatile("msr pmuserenr_el0, %0" ::"r"((u64)ARMV8_PMUSERENR_EN_EL0 | ARMV8_PMUSERENR_ER | ARMV8_PMUSERENR_CR));

    /*  Initialize & Reset PMNC: C and P bits. */
    armv8pmu_pmcr_write(ARMV8_PMCR_P | ARMV8_PMCR_C);

    /* Set event type to event_number in pmxevtyper_el0 register. */
    unsigned long long event_number = 0x17; // Level 2 data cache refill (miss)
    __asm__ volatile("msr pmxevtyper_el0, %0" ::"r"(event_number));

    /*   Performance Monitors Count Enable Set register bit 30:0 disable, 31 enable */
    __asm__ volatile("msr pmcntenset_el0, %0" ::"r"(ARMV8_PMCNTENSET_EL0_ENABLE));

    /* Set cache level */
    armv8pmu_set_cache_level();

    /* Print Cache Size (L2) */
    armv8pmu_read_cache_size();

    /* start*/
    armv8pmu_pmcr_write(armv8pmu_pmcr_read() | ARMV8_PMCR_E);

    volatile u32 *memory;
    memory = malloc(L2_CACHE_SIZE * 10);
    size_t size = NUM_ACCESSES * CACHE_LINE_SIZE;

    // Measure cache misses for sequential read
    u64 read_misses_seq = read_cache_misses_sequential(memory, size);
    printf("\nSequential read cache misses: %lu\n", read_misses_seq);

    // Free the memory block
    free((void *)memory);

    memory = malloc(L2_CACHE_SIZE * 10);

    // Measure cache misses for random read
    u64 read_misses_rand = read_cache_misses_random(memory, size);
    printf("\nRandom read cache misses: %lu\n", read_misses_rand);

    // Free the memory block
    free((void *)memory);

    // Allocate a memory block that is larger than the L2 cache size
    memory = malloc(L2_CACHE_SIZE * 10);

    // Measure cache misses for sequential write
    u64 write_misses_seq = write_cache_misses_sequential(memory);
    printf("\nSequential write cache misses: %lu\n", write_misses_seq);

    // Free the memory block
    free((void *)memory);

    memory = malloc(L2_CACHE_SIZE * 10);

    // Measure cache misses for random write
    u64 write_misses_rand = write_cache_misses_random(memory);
    printf("-------------");
    printf("\nRandom write cache misses: %lu\n", write_misses_rand);

    // Free the memory block
    free((void *)memory);

    cleanup_platform();
    return 0;
}
