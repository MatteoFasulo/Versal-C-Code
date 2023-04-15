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

#define SYSREG_NUM(reg) \
    ({ unsigned long long __val; \
        __asm__ volatile ("mrs %0, " #reg : "=r" (__val)); \
        __val; })

#define SYSREG(reg) (1ULL << SYSREG_NUM(reg))

unsigned long long read_sysreg(const char* reg_name)
{
	unsigned long long reg_val;

	#define REG_MAP(reg) else if (!strcmp(reg_name, #reg)) { \
		__asm__ volatile ("mrs %0, " #reg : "=r" (reg_val)); }
	if (0) {}
	// sys 14
	REG_MAP(pmevcntr0_el0)
	REG_MAP(pmevcntr1_el0)
	REG_MAP(pmevcntr2_el0)
	REG_MAP(pmevcntr3_el0)
	REG_MAP(pmevcntr4_el0)
	REG_MAP(pmevcntr5_el0)
	REG_MAP(pmevtyper0_el0)
	REG_MAP(pmevtyper1_el0)
	REG_MAP(pmevtyper2_el0)
	REG_MAP(pmevtyper3_el0)
	REG_MAP(pmevtyper4_el0)
	REG_MAP(pmevtyper5_el0)

	// sys 9
	REG_MAP(pmcr_el0)
	REG_MAP(pmccntr_el0)
	REG_MAP(pmxevtyper_el0)
	REG_MAP(pmuserenr_el0)
	REG_MAP(pmxevcntr_el0)

	else {
		print("Register name not mapped\n");
		exit(0);
	    return 0;
	}
	return reg_val;
}

static inline u32 armv8pmu_pmcr_read(void)
{
	u64 val=0;
	asm volatile("mrs %0, pmcr_el0" : "=r" (val));
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
    unsigned long long reg_val;
    unsigned long long event_number = 0x13; //Memory access

    /* Reset pmuserenr_el0 to default */
    reset_pmuserenr_el0();
    /*  Enable user-mode access to counters. */
    __asm__ volatile("msr pmuserenr_el0, %0" :: "r"((u64)ARMV8_PMUSERENR_EN_EL0|ARMV8_PMUSERENR_ER|ARMV8_PMUSERENR_CR));
    /*  Initialize & Reset PMNC: C and P bits. */
    armv8pmu_pmcr_write(ARMV8_PMCR_P | ARMV8_PMCR_C);

    /* Set event type to event_number in pmxevtyper_el0 register. */
    __asm__ volatile ("msr pmxevtyper_el0, %0" :: "r" (event_number));
    /*   Performance Monitors Count Enable Set register bit 30:0 disable, 31 enable */
    __asm__ volatile("msr pmcntenset_el0, %0" :: "r" (ARMV8_PMCNTENSET_EL0_ENABLE));
    /* start*/
    armv8pmu_pmcr_write(armv8pmu_pmcr_read() | ARMV8_PMCR_E);



    /* Read value from a specific register mapped to a macro definition */
    reg_val = read_sysreg("pmccntr_el0");
    printf("CPU Cycles: %llu\n", reg_val);

    cleanup_platform();
    return 0;
}