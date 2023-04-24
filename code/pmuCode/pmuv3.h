#ifndef PMUV3_H
#define PMUV3_H

	#include <xil_types.h>
	#include <xpseudo_asm_gcc.h>

	//
	// Performance metrics
	//
	#define L1D_CACHE_ALLOCATE 		0x1F
	#define CHAIN 					0x1E
	#define BUS_CYCLES 				0x1D
	#define TTBR_WRITE_RETIRED 		0x1C
	#define INST_SPEC 				0x1B
	#define MEMORY_ERROR 			0x1A
	#define BUS_ACCESS 				0x19
	#define L2D_CACHE_WB 			0x18
	#define L2D_CACHE_REFILL 		0x17
	#define L2D_CACHE 				0x16
	#define L1D_CACHE_WB 			0x15
	#define L1I_CACHE 				0x14
	#define MEM_ACCESS 				0x13
	#define BR_PRED 				0x12
	#define CPU_CYCLES 				0x11
	#define BR_MIS_PRED 			0x10
	#define UNALIGNED_LDST_RETIRED 	0x0F
	#define BR_RETURN_RETIRED 		0x0E
	#define BR_IMMED_RETIRED 		0x0D
	#define PC_WRITE_RETIRED 		0x0C
	#define CID_WRITE_RETIRED 		0x0B
	#define EXC_RETURN 				0x0A
	#define EXC_TAKEN 				0x09
	#define INST_RETIRED 			0x08
	#define ST_RETIRED 				0x07
	#define LD_RETIRED 				0x06
	#define L1D_TLB_REFILL 			0x05
	#define L1D_CACHE 				0x04
	#define L1D_CACHE_REFILL 		0x03
	#define L1I_TLB_REFILL 			0x02
	#define L1I_CACHE_REFILL 		0x01
	#define SW_INCR 				0x00
	//

	//
	// available counters
	//
	#define	ARMV8_IDX_CYCLE_COUNTER	0

	#define	ARMV8_IDX_COUNTER0		1
	#define	ARMV8_IDX_COUNTER1		2
	#define	ARMV8_IDX_COUNTER2		3
	#define	ARMV8_IDX_COUNTER3		4
	#define	ARMV8_IDX_COUNTER4		5
	#define	ARMV8_IDX_COUNTER5		6
	//


	#define	ARMV8_PMU_MAX_COUNTERS	32
	#define	ARMV8_PMU_COUNTER_MASK	(ARMV8_PMU_MAX_COUNTERS - 1)

	#define	ARMV8_PMU_PMCR_MASK		0x7f	 	/* Mask for writable bits */
	#define	ARMV8_PMU_EVTYPE_MASK	0xc800ffff	/* Mask for writable bits */
	#define	ARMV8_IDX_TO_COUNTER(x) (((x) - ARMV8_IDX_COUNTER0) & ARMV8_PMU_COUNTER_MASK)

	#define ARMV8_PMU_PMCR_E	(1 << 0) 		/* Enable all counters */
	#define BIT(nr) 			(1 << nr)

	static inline int armv8pmu_disable_counter(int idx)
	{
		u32 counter = ARMV8_IDX_TO_COUNTER(idx);

		 asm volatile(
			 "msr pmcntenclr_el0, %0"
				 :: "r" (BIT(counter))
		 );

		 return idx;
	}

	static inline int armv8pmu_enable_counter(int idx)
	{
		u32 counter = ARMV8_IDX_TO_COUNTER(idx);

		asm volatile(
			"msr pmcntenset_el0, %0"
				:: "r" (BIT(counter))
		);

		return idx;
	}

	static inline int armv8pmu_select_counter(int idx)
	{
		u32 counter = ARMV8_IDX_TO_COUNTER(idx);

		asm volatile(
			"msr pmselr_el0, %0"
				:: "r" (counter)
		);

		isb();

		return idx;
	}

	static inline void armv8pmu_write_evtype(int idx, u32 val)
	{
		 if (armv8pmu_select_counter(idx) == idx) {

			 val &= ARMV8_PMU_EVTYPE_MASK;

			 asm volatile(
				"msr pmxevtyper_el0, %0"
					 :: "r" (val)
			 );
		 }
	}

	static inline int armv8pmu_enable_intens(int idx)
	{
		u32 counter = ARMV8_IDX_TO_COUNTER(idx);

		asm volatile(
			"msr pmintenset_el1, %0"
				:: "r" (BIT(counter))
		);

		return idx;
	}

	static inline int armv8pmu_disable_intens(int idx)
	{
		u32 counter = ARMV8_IDX_TO_COUNTER(idx);

		asm volatile(
			"msr pmintenclr_el1, %0"
				:: "r" (BIT(counter))
		);

		isb();

		/* Clear the overflow flag in case an interrupt is pending. */
		asm volatile(
			"msr pmovsclr_el0, %0"
				:: "r" (BIT(counter))
		);

		isb();

		return idx;
	}

	static inline u32 armv8pmu_read_counter(int idx)
	{
		u32 value = 0;

		if (idx == ARMV8_IDX_CYCLE_COUNTER) {
			asm volatile(
				"mrs %0, pmccntr_el0"
					: "=r" (value)
			);
		}
		else if (armv8pmu_select_counter(idx) == idx) {
			asm volatile(
				"mrs %0, pmxevcntr_el0"
					: "=r" (value)
			);
		}

		return value;
	}

	static inline u32 armv8pmu_pmcr_read(void)
	{
		u64 __val;

		asm volatile(
			"mrs %0, pmcr_el0"
				: "=r" (__val)
		);

		return __val;
	}

	static inline void armv8pmu_pmcr_write(u32 val)
	{
		val &= ARMV8_PMU_PMCR_MASK;

		isb();

		do {
			u64 __val = (u64)(val);
			asm volatile(
				"msr pmcr_el0, %x0"
					 : : "rZ" (__val)
			);
		} while (0);
	}

	static inline void armv8pmu_pmcr_enable()
	{
		u32 val = armv8pmu_pmcr_read();

		val |= (BIT(0) | BIT(2));

		armv8pmu_pmcr_write(val);
	}

	static void armv8pmu_start()
	{
		armv8pmu_pmcr_write(armv8pmu_pmcr_read() | ARMV8_PMU_PMCR_E);
	}

	static void armv8pmu_stop()
	{
		armv8pmu_pmcr_write(armv8pmu_pmcr_read() & ~ARMV8_PMU_PMCR_E);
	}

	static void armv8pmu_reset()
	{
		armv8pmu_pmcr_write(armv8pmu_pmcr_read() | BIT(1));
	}

	static void armv8pmu_init_counter(u32 counter, u32 metric)
	{
		armv8pmu_reset();

		armv8pmu_disable_counter(counter);
		armv8pmu_disable_intens (counter);
		armv8pmu_write_evtype	(counter, metric);
		armv8pmu_enable_intens	(counter);
		armv8pmu_enable_counter	(counter);
	}
#endif


