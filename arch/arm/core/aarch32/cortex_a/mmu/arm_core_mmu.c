/* 
 * Simple ARMv7 MMU setup
 * Copyright (c) 2020, Immo Birnbaum
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <init.h>
#include <kernel.h>
#include <soc.h>

#define ARM_MMU_PAGE_PERM_READ	(1 << 0)
#define ARM_MMU_PAGE_PERM_WRITE	(1 << 1)
#define ARM_MMU_PAGE_CACHEABLE	(1 << 2)
#define ARM_MMU_PAGE_BUFFERABLE	(1 << 3)
#define ARM_MMU_PAGE_SHARED		(1 << 4)

#define ARM_MMU_FIRST_LEVEL_SECTION_ID 0x2

union arm_mmu_first_level_pagetable_entry {
	struct {
		u32_t id			: 2;
		u32_t b				: 1;
		u32_t c				: 1;
		u32_t imp			: 1;
		u32_t domain		: 4;
		u32_t sbz0			: 1;
		u32_t ap			: 2;
		u32_t sbz1			: 8;
		u32_t base_address	: 12;
	} data;
	u32_t word;
};

static u32_t simple_pagetable[4096] __aligned(16384);

static u32_t arm_mmu_gen_page_entry(u32_t base, u8_t flags) {
	union arm_mmu_first_level_pagetable_entry entry;

	entry.data.id			 = ARM_MMU_FIRST_LEVEL_SECTION_ID;
	entry.data.b 			 = (flags & ARM_MMU_PAGE_BUFFERABLE) ? 1 : 0;
	entry.data.c			 = (flags & ARM_MMU_PAGE_CACHEABLE)  ? 1 : 0;
	entry.data.imp			 = 1;
	entry.data.domain		 = 0;
	entry.data.sbz0			 = 0;
	entry.data.ap 			 = (flags & ARM_MMU_PAGE_PERM_WRITE) ? 3 : 0;
	entry.data.sbz1			 = 0;
	entry.data.sbz1			|= (flags & ARM_MMU_PAGE_SHARED)     ? (1 << 4) : 0;
	entry.data.base_address	 = (base >> 20);

	return (u32_t)entry.word;
}

static int arm_mmu_init(struct device *arg) {
	u32_t pagetable_base	= (u32_t)simple_pagetable;
	u32_t reg_val			= 0;
	u32_t iter				= 0;

	u32_t ram_base			= DT_INST_0_MMIO_SRAM_BASE_ADDRESS;
	u32_t ram_top			= ram_base + DT_INST_0_MMIO_SRAM_SIZE;

    /* TTB Registers */

    __asm__ __volatile__ ("mcr p15,0,%0,c2,c0,0" : : "r"(pagetable_base));
    __asm__ __volatile__ ("mcr p15,0,%0,c2,c0,1" : : "r"(pagetable_base));
    __asm__ __volatile__ ("mcr p15,0,%0,c2,c0,2" : : "r"(reg_val));

    /* Domain Access Control -> manager mode */

    reg_val = -1;
    __asm__ __volatile__ ("mcr p15,0,%0,c3,c0,0" : : "r"(reg_val));

    /* Set up a simple 4G address space virt = phys page table */

    for (iter = 0; iter < 4096; iter++)
    {
    	if (    ((iter * 0x00100000) >= ram_base) 
			 && ((iter * 0x00100000)  < ram_top)) {
        	simple_pagetable[iter] = arm_mmu_gen_page_entry(
        		(iter * 0x00100000),
    			(   ARM_MMU_PAGE_PERM_READ 
				  | ARM_MMU_PAGE_PERM_WRITE 
				  | ARM_MMU_PAGE_CACHEABLE 
				  | ARM_MMU_PAGE_BUFFERABLE)); /* Regular memory */
    	} else {
        	simple_pagetable[iter] = arm_mmu_gen_page_entry(
        		(iter * 0x00100000),
    			(   ARM_MMU_PAGE_PERM_READ 
				  | ARM_MMU_PAGE_PERM_WRITE)); /* Strongly ordered */
    	}
    }

    /* Enable the MMU */

    __asm__ __volatile__ ("mrc p15,0,%0,c1,c0,0" : "=r"(reg_val));
    reg_val |= 0x00000001;
    __asm__ __volatile__ ("mcr p15,0,%0,c1,c0,0" : : "r"(reg_val));

	return 0;
}

SYS_INIT(arm_mmu_init, PRE_KERNEL_1,
	 CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
