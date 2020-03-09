/*
 * Copyright (c) 2020 Immo Birnbaum
 * SPDX-License-Identifier: Apache-2.0
 */

/* Fixups for: Xilinx UARTPS driver interrupt vector(s) */

#ifdef DT_INST_0_XLNX_XUARTPS
#undef DT_INST_0_XLNX_XUARTPS_IRQ_0
#define DT_INST_0_XLNX_XUARTPS_IRQ_0	((DT_INST_0_XLNX_XUARTPS_IRQ_IRQ_0 + 1) << 8)
#endif

#ifdef DT_INST_1_XLNX_XUARTPS
#undef DT_INST_1_XLNX_XUARTPS_IRQ_0
#define DT_INST_1_XLNX_XUARTPS_IRQ_0	((DT_INST_1_XLNX_XUARTPS_IRQ_IRQ_0 + 1) << 8)
#endif

/* Fixups for: Cadence Triple Timer/Counter interrupt vector(s)
 * Notice: at the time being the driver only supports the first instance out of the
 * three available instances. Therefore, only instance 0 is fixed up here. */
 
#ifdef DT_INST_0_CDNS_TTC
#undef DT_INST_0_CDNS_TTC_IRQ_0
#define DT_INST_0_CDNS_TTC_IRQ_0	((DT_INST_0_CDNS_TTC_IRQ_IRQ_0 + 1) << 8)
#undef DT_INST_0_CDNS_TTC_IRQ_1
#define DT_INST_0_CDNS_TTC_IRQ_1	((DT_INST_0_CDNS_TTC_IRQ_IRQ_1 + 1) << 8)
#undef DT_INST_0_CDNS_TTC_IRQ_2
#define DT_INST_0_CDNS_TTC_IRQ_2	((DT_INST_0_CDNS_TTC_IRQ_IRQ_2 + 1) << 8)
#endif

/* Fixups for: Xilinx AXI GPIO IP core interrupt vector(s).
 * Notice: at the time being the driver only supports two instances of this IP core. 
 * Defining an interrupt vector in the respective instance's device tree entry is 
 * OPTIONAL for this driver, hence the double-check. */

#if defined(DT_INST_0_XLNX_AXI_GPIO) && defined(DT_INST_0_XLNX_AXI_GPIO_IRQ_0)
#undef DT_INST_0_XLNX_AXI_GPIO_IRQ_0
#define DT_INST_0_XLNX_AXI_GPIO_IRQ_0	((DT_INST_0_XLNX_AXI_GPIO_IRQ_IRQ_0 + 1) << 8)
#endif

#if defined(DT_INST_1_XLNX_AXI_GPIO) && defined(DT_INST_1_XLNX_AXI_GPIO_IRQ_0)
#undef DT_INST_1_XLNX_AXI_GPIO_IRQ_0
#define DT_INST_1_XLNX_AXI_GPIO_IRQ_0	((DT_INST_1_XLNX_AXI_GPIO_IRQ_IRQ_0 + 1) << 8)
#endif

/* Fixups for: Xilinx GEM Ethernet controller interrupt vector(s). */

#ifdef DT_INST_0_XLNX_GEM
#undef DT_INST_0_XLNX_GEM_IRQ_0
#define DT_INST_0_XLNX_GEM_IRQ_0	((DT_INST_0_XLNX_GEM_IRQ_IRQ_0 + 1) << 8)
#undef DT_INST_0_XLNX_GEM_IRQ_1
#define DT_INST_0_XLNX_GEM_IRQ_1	((DT_INST_0_XLNX_GEM_IRQ_IRQ_1 + 1) << 8)
#endif

#ifdef DT_INST_1_XLNX_GEM
#undef DT_INST_1_XLNX_GEM_IRQ_0
#define DT_INST_1_XLNX_GEM_IRQ_0	((DT_INST_1_XLNX_GEM_IRQ_IRQ_0 + 1) << 8)
#undef DT_INST_1_XLNX_GEM_IRQ_1
#define DT_INST_1_XLNX_GEM_IRQ_1	((DT_INST_1_XLNX_GEM_IRQ_IRQ_1 + 1) << 8)
#endif

/* Fixups for: ARM architecture global timer */

#ifdef DT_INST_0_ARM_ARM_TIMER_BASE_ADDRESS
#define DT_ARM_ARM_TIMER_TIMER_BASE_ADDRESS DT_INST_0_ARM_ARM_TIMER_BASE_ADDRESS
#endif

#ifdef DT_INST_0_ARM_ARM_TIMER_IRQ_0
#define DT_ARM_ARM_TIMER_TIMER_IRQ_0 DT_INST_0_ARM_ARM_TIMER_IRQ_0
#endif
#ifdef DT_INST_0_ARM_ARM_TIMER_IRQ_1
#define DT_ARM_ARM_TIMER_TIMER_IRQ_1 DT_INST_0_ARM_ARM_TIMER_IRQ_1
#endif
#ifdef DT_INST_0_ARM_ARM_TIMER_IRQ_2
#define DT_ARM_ARM_TIMER_TIMER_IRQ_2 DT_INST_0_ARM_ARM_TIMER_IRQ_2
#endif

