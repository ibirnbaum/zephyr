/*
 * Copyright (c) 2019 Weidmüller Interface GmbH & Co. KG.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
#ifndef __DT_BINDING_ARM_GIC_PL390_H
#define __DT_BINDING_ARM_GIC_PL390_H

/* CPU Private Peripherial Interrupt (PPI) numbers */

#define GIC_PL390_PPI0_INT_GLOBAL_TIMER        27 /* rising edge */
#define GIC_PL390_PPI1_INT_FIQ                 28 /* low level   */
#define GIC_PL390_PPI2_INT_PRIVATE_TIMER       29 /* rising edge */
#define GIC_PL390_PPI3_INT_PRIVATE_WDOG        30 /* rising edge */
#define GIC_PL390_PPI4_INT_IRQ                 31 /* low level   */

/* Shared Peripherial Interrupt (SPI) configuration */

#define SGI_IRQ_RISING_EDGE                    0x2 /* used for SGI interrupts in ICDICFR0      */
#define PPI_IRQ_LOW_LEVEL                      0x1 /* used for PPI interrupts in ICDICFR1      */
#define PPI_IRQ_RISING_EDGE                    0x3 /* used for PPI interrupts in ICDICFR1      */
#define SPI_IRQ_HIGH_LEVEL                     0x1 /* used for SPI interrupts in ICDICFR[2..5] */
#define SPI_IRQ_RISING_EDGE                    0x3 /* used for SPI interrupts in ICDICFR[2..5] */

/* Interrupt prioritization */

#define IRQ_DEFAULT_PRIORITY                   (1 << 3) /* set in ICDIPR[0..23] -> 1 byte per interrupt source, value is written to the bits [7..3] only */

/* Interrupt sense definitions for top-level interface (will be substituted by SPI_IRQ_HIGH_LEVEL / SPI_IRQ_RISING_EDGE in ICDICFRn access) */

#define IRQ_TYPE_LEVEL                         0x0
#define IRQ_TYPE_EDGE                          0x1

#endif
