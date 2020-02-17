/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Per-arch thread definition
 *
 * This file contains definitions for
 *
 *  struct _thread_arch
 *  struct _callee_saved
  *
 * necessary to instantiate instances of struct k_thread.
 */

#ifndef ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_THREAD_H_
#define ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_THREAD_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

struct _callee_saved {
	u32_t v1;  /* r4 */
	u32_t v2;  /* r5 */
	u32_t v3;  /* r6 */
	u32_t v4;  /* r7 */
	u32_t v5;  /* r8 */
	u32_t v6;  /* r9 */
	u32_t v7;  /* r10 */
	u32_t v8;  /* r11 */
#if defined(CONFIG_CPU_CORTEX_R) \
	|| defined(CONFIG_CPU_CORTEX_A)
	u32_t spsr;/* r12 */
	u32_t psp; /* r13 */
	u32_t lr;  /* r14 */
#else
	u32_t psp; /* r13 */
#endif
};

typedef struct _callee_saved _callee_saved_t;

#ifndef CONFIG_ARMV7_A
#if defined(CONFIG_FLOAT) && defined(CONFIG_FP_SHARING)
struct _preempt_float {
	float  s16;
	float  s17;
	float  s18;
	float  s19;
	float  s20;
	float  s21;
	float  s22;
	float  s23;
	float  s24;
	float  s25;
	float  s26;
	float  s27;
	float  s28;
	float  s29;
	float  s30;
	float  s31;
};
#endif
#else
struct _preempt_float {
	double  d00;
	double  d01;
	double  d02;
	double  d03;
	double  d04;
	double  d05;
	double  d06;
	double  d07;
	double  d08;
	double  d09;
	double  d10;
	double  d11;
	double  d12;
	double  d13;
	double  d14;
	double  d15;
	double  d16;
	double  d17;
	double  d18;
	double  d19;
	double  d20;
	double  d21;
	double  d22;
	double  d23;
	double  d24;
	double  d25;
	double  d26;
	double  d27;
	double  d28;
	double  d29;
	double  d30;
	double  d31;
	u32_t   fpscr;
	u32_t   fpexc;
};
#endif

typedef struct _preempt_float _preempt_float_t;

struct _thread_arch {

	/* interrupt locking key */
	u32_t basepri;

	/* r0 in stack frame cannot be written to reliably */
	u32_t swap_return_value;

#if (defined(CONFIG_FLOAT) && defined(CONFIG_FP_SHARING)) \
	|| defined(CONFIG_ARMV7_A)
	/*
	 * No cooperative floating point register set structure exists for
	 * the Cortex-M as it automatically saves the necessary registers
	 * in its exception stack frame.
	 */
	struct _preempt_float  preempt_float;
#endif

#if defined(CONFIG_USERSPACE) || defined(CONFIG_FP_SHARING)
	u32_t mode;
#if defined(CONFIG_USERSPACE)
	u32_t priv_stack_start;
#endif
#endif
};

typedef struct _thread_arch _thread_arch_t;

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_THREAD_H_ */
