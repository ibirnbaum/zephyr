/*
 * Copyright (c) 2019 Carlo Caione <ccaione@baylibre.com>
 * Copyright (c) 2020 Immo Birnbaum
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_TIMER_H_
#define ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_TIMER_H_

#ifndef _ASMLANGUAGE

#include <drivers/timer/arm_arch_timer.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARM_ARCH_TIMER_IRQ			((ARM_TIMER_VIRTUAL_IRQ + 1) << 8)
#define CNTV_CTL_ENABLE				(1 << 0)
#define COMP_CTL_ENABLE				(1 << 1)
#define IRQ_CTL_ENABLE				(1 << 2)
#define AUTO_INCR_ENABLE            (1 << 3)

#define COUNTVAL_LOW_REG_OFFSET		0x00
#define COUNTVAL_HIGH_REG_OFFSET	0x04
#define CONTROL_REG_OFFSET			0x08
#define INT_STATUS_REG_OFFSET		0x0C
#define COMPVAL_LOW_REG_OFFSET		0x10
#define COMPVAL_HIGH_REG_OFFSET		0x14
#define AUTO_INCR_REG_OFFSET		0x18

static ALWAYS_INLINE void arm_arch_timer_set_compare(u64_t val)
{
	u32_t high     = (u32_t)(val >> 32);
	u32_t low      = (u32_t)val;
	u32_t cntv_ctl = 0;

	/* Compare register update procedure as described in the Zynq-7000
	 * TRM, Appendix B, p. 1452 :
	 * 1. Clear the Comp Enable bit in the Timer Control Register.
	 * 2. Write the lower 32-bit Comparator Value Register.
	 * 3. Write the upper 32-bit Comparator Value Register.
	 * 4. Set the Comparator Enable bit and the IRQ enable bit. */

	cntv_ctl = sys_read32(DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + CONTROL_REG_OFFSET);
	sys_write32((cntv_ctl & ~COMP_CTL_ENABLE), DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + CONTROL_REG_OFFSET);

	sys_write32(low,  DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + COMPVAL_LOW_REG_OFFSET);
	sys_write32(high, DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + COMPVAL_HIGH_REG_OFFSET);

	sys_write32((cntv_ctl | COMP_CTL_ENABLE), DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + CONTROL_REG_OFFSET);
}

static ALWAYS_INLINE u8_t arm_arch_timer_get_int_status(void)
{
	return (u8_t)(sys_read32(DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + INT_STATUS_REG_OFFSET) & 0x1);
}

static ALWAYS_INLINE void arm_arch_timer_clear_int_status(void)
{
	sys_write32(0x1, DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + INT_STATUS_REG_OFFSET);
}

static ALWAYS_INLINE void arm_arch_timer_set_auto_increment(u32_t val)
{
	u32_t cntv_ctl = 0;

	sys_write32(val, DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + AUTO_INCR_REG_OFFSET);

	cntv_ctl = sys_read32(DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + CONTROL_REG_OFFSET);
	sys_write32((cntv_ctl | AUTO_INCR_ENABLE), DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + CONTROL_REG_OFFSET);	
}

static ALWAYS_INLINE void arm_arch_timer_enable(unsigned char enable)
{
	u32_t cntv_ctl = sys_read32(DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + CONTROL_REG_OFFSET);

	if (enable) {
		cntv_ctl &= ~(CNTV_CTL_ENABLE | IRQ_CTL_ENABLE);
		sys_write32(cntv_ctl, DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + CONTROL_REG_OFFSET);

		sys_write32(0, DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + COUNTVAL_LOW_REG_OFFSET);
		sys_write32(0, DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + COUNTVAL_HIGH_REG_OFFSET);

		cntv_ctl |=  (CNTV_CTL_ENABLE | IRQ_CTL_ENABLE);
	} else {
		cntv_ctl &= (~CNTV_CTL_ENABLE | IRQ_CTL_ENABLE);
	}

	sys_write32(cntv_ctl, DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + CONTROL_REG_OFFSET);
}

static ALWAYS_INLINE u64_t arm_arch_timer_count(void)
{
	u32_t high_first  = 0;
	u32_t high_second = 1;
	u32_t low;

	/* Counter register read procedure as described in the Zynq-7000
	 * TRM, Appendix B, p. 1449 :
	 * 1. Read the upper 32-bit timer counter register
	 * 2. Read the lower 32-bit timer counter register
	 * 3. Read the upper 32-bit timer counter register again. 
	 * If the value is different to the 32-bit upper value read previously, 
	 * go back to step 2. Otherwise the 64-bit timer counter value is correct. */

	while (high_first != high_second) {
		high_first  = sys_read32(DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + COUNTVAL_HIGH_REG_OFFSET);
		low         = sys_read32(DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + COUNTVAL_LOW_REG_OFFSET);
		high_second = sys_read32(DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS + COUNTVAL_HIGH_REG_OFFSET);
	}

	return (((u64_t)high_first << 32) | (u64_t)low);
}

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_TIMER_H_ */
