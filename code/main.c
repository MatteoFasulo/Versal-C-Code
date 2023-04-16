#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "platform.h"
#include "xil_printf.h"

typedef u64         uint64_t;
typedef u32         uint32_t;

#define ARMV8_PMCR_MASK         0x3f
#define ARMV8_PMCR_E            (1 << 0) /*  Enable all counters */
#define ARMV8_PMCR_P            (1 << 1) /*  Reset all counters */
#define ARMV8_PMCR_C            (1 << 2) /*  Cycle counter reset */

#define ARMV8_PMUSERENR_EN_EL0  (1 << 0) /*  EL0 access enable */
#define ARMV8_PMUSERENR_CR      (1 << 2) /*  Cycle counter read enable */
#define ARMV8_PMUSERENR_ER      (1 << 3) /*  Event counter read enable */

#define ARMV8_PMCNTENSET_EL0_ENABLE (1<<31) /* *< Enable Perf count reg */

// Performance Monitors Control Register
static inline u32 armv8pmu_pmcr_read(void)
{
	u64 val=0;
	asm volatile("mrs %0, pmcr_el0" : "=r" (val));
	return (u32)val;
}

// Performance Monitors Cycle Count Register
static inline u32 armv8pmu_pmccntr_read(void)
{
	u64 val=0;
	asm volatile("mrs %0, pmccntr_el0" : "=r" (val));
	return (u32)val;
}

// Set Cache Level
static inline void armv8pmu_set_cache_level() {
    u64 val = 0x2;

    __asm__ volatile ("msr csselr_el1, %0" : : "r" (val));
}

// Read Cache Size ID Register
static inline u32 armv8pmu_read_cache_size() {
    u64 val=0;

    __asm__ volatile ("mrs %0, ccsidr_el1": "=r" (val));
    switch(val) {
    	case 0x701FE00A:
    		printf("32 KB");
    		break;
    	case 0x201FE012:
    		printf("48 KB");
    	    break;
    	case 0x703FE07A:
    		printf("512 KB");
    	    break;
    	case 0x707FE07A:
    	    printf("1024 KB");
    	    break;
    	case 0x70FFE07A:
    		printf("2048 KB");
    	    break;
    	case 0x71FFE07A:
    		printf("4096 KB");
    	    break;
    	default:
    	    printf("Unknown value: 0x%lx\n", val);
    }
    return (u32)val;
}

static inline void armv8pmu_pmcr_write(u32 val)
{
	val &= ARMV8_PMCR_MASK;
	asm volatile("msr pmcr_el0, %0" : : "r" ((u64)val));
}

static inline void reset_pmuserenr_el0()
{
	__asm__ volatile("msr pmuserenr_el0, %0" :: "r"(0x0));
}


int main()
{
    init_platform();

    /* Reset pmuserenr_el0 to default */
    reset_pmuserenr_el0();

    /*  Enable user-mode access to counters. */
    __asm__ volatile("msr pmuserenr_el0, %0" :: "r"((u64)ARMV8_PMUSERENR_EN_EL0|ARMV8_PMUSERENR_ER|ARMV8_PMUSERENR_CR));

    /*  Initialize & Reset PMNC: C and P bits. */
    armv8pmu_pmcr_write(ARMV8_PMCR_P | ARMV8_PMCR_C);

    /* Set event type to event_number in pmxevtyper_el0 register. */
    unsigned long long event_number = 0x17; //Level 2 data cache refill (miss)
    __asm__ volatile ("msr pmxevtyper_el0, %0" :: "r" (event_number));

    /*   Performance Monitors Count Enable Set register bit 30:0 disable, 31 enable */
    __asm__ volatile("msr pmcntenset_el0, %0" :: "r" (ARMV8_PMCNTENSET_EL0_ENABLE));

    /* Set cache level */
    armv8pmu_set_cache_level(2);

    /* Print Cache Size (L2) */
    armv8pmu_read_cache_size();

    /* start*/
    armv8pmu_pmcr_write(armv8pmu_pmcr_read() | ARMV8_PMCR_E);

    int i;
    int n = 512 * 512;
    int *block = malloc(n * sizeof(int)); // (n * n * 8) > 1024KB (L2 size)

    for (i = 0; i < n / 10; i++) {
    	int ri = rand() % n;
        block[ri] = 0;
    }

    u32 reg_val = armv8pmu_pmccntr_read();
    printf("\nRegister value: %u\n", reg_val);


    cleanup_platform();
    return 0;
}
