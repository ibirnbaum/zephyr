/*
 * Copyright (c) 2019 Weidmueller Interface GmbH & Co. KG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <sw_isr_table.h>
#include <irq_nextlevel.h>
#include <dt-bindings/interrupt-controller/arm-gic-pl390.h>

#define GIC_MAX_NUM_LINES         256

#define DT_GIC_PL390_DIST_BASE    DT_INST_0_ARM_V7A_GIC_PL390_BASE_ADDRESS_0 /* Distributor register interface base address */
#define DT_GIC_PL390_CPU_BASE     DT_INST_0_ARM_V7A_GIC_PL390_BASE_ADDRESS_1 /* CPU register interface base address         */

#define GICD_CTLR                 (DT_GIC_PL390_DIST_BASE + 0x000)   /* Distributor Control Register <WP>,                 ICDDCR   on Zynq-7000 */
#define GICD_TYPER                (DT_GIC_PL390_DIST_BASE + 0x004)   /* Interrupt Controller Type Register,                ICDICTR  on Zynq-7000 */
#define GICD_IIDR                 (DT_GIC_PL390_DIST_BASE + 0x008)   /* Distributor Implementer Identification Register,   ICDIIDR  on Zynq-7000 */
#define GICD_ISRn                 (DT_GIC_PL390_DIST_BASE + 0x080)   /* Interrupt Security Register 0/1/2 <WP>,            ICDISRn  on Zynq-7000 */
#define GICD_ISENABLERn           (DT_GIC_PL390_DIST_BASE + 0x100)   /* Interrupt Set-Enable Register 0/1/2 <WP>,          ICDISERn on Zynq-7000 */
#define GICD_ICENABLERn           (DT_GIC_PL390_DIST_BASE + 0x180)   /* Interrupt Clear-Enable Register 0/1/2 <WP>,        ICDICERn on Zynq-7000 */
#define GICD_ISPENDRn             (DT_GIC_PL390_DIST_BASE + 0x200)   /* Interrupt Set-Pending Register 0/1/2 <WP>,         ICDISPRn on Zynq-7000 */
#define GICD_ICPENDRn             (DT_GIC_PL390_DIST_BASE + 0x280)   /* Interrupt Clear-Pending Register 0/1/2 <WP>,       ICDICPRn on Zynq-7000 */
#define GICD_IABRn                (DT_GIC_PL390_DIST_BASE + 0x300)   /* Interrupt Active Bit Register 0/1/2,               ICDABRn  on Zynq-7000 */
#define GICD_IPRIORITYRn          (DT_GIC_PL390_DIST_BASE + 0x400)   /* Interrupt Priority Register 0..23 <WP>,            ICDIPRn  on Zynq-7000 */
#define GICD_ITARGETSRn           (DT_GIC_PL390_DIST_BASE + 0x800)   /* Interrupt Processor Targets Register 0..23 <WP>,   ICDIPTRn on Zynq-7000 */
#define GICD_ICFGRn               (DT_GIC_PL390_DIST_BASE + 0xC00)   /* Interrupt Configuration Register 0..5 <WP>,        ICDICFRn on Zynq-7000 */
#define GICD_SGIR                 (DT_GIC_PL390_DIST_BASE + 0xF00)   /* Software Generated Interrupt Register,             ICDSGIR  on Zynq-7000 */

/* The registers marked "<WP>" above can be write protected via the SLCR, register APU_CTLR, CFGDISABLE bit. */

#define GICC_CTLR                 (DT_GIC_PL390_CPU_BASE  + 0x000)   /* CPU Interface Control Register,                    ICCICR   on Zynq-7000 */
#define GICC_PMR                  (DT_GIC_PL390_CPU_BASE  + 0x004)   /* Interrupt Priority Mask Register,                  ICCPMR   on Zynq-7000 */
#define GICC_BPR                  (DT_GIC_PL390_CPU_BASE  + 0x008)   /* Binary Point Register,                             ICCBPR   on Zynq-7000 */
#define GICC_IAR                  (DT_GIC_PL390_CPU_BASE  + 0x00C)   /* Interrupt Acknowledge Register,                    ICCIAR   on Zynq-7000 */
#define GICC_EOIR                 (DT_GIC_PL390_CPU_BASE  + 0x010)   /* End Of Interrupt Register,                         ICCEOIR  on Zynq-7000 */
#define GICC_RPR                  (DT_GIC_PL390_CPU_BASE  + 0x014)   /* Running Priority Register,                         ICCRPR   on Zynq-7000 */
#define GICC_HPIR                 (DT_GIC_PL390_CPU_BASE  + 0x018)   /* Highest Pending Interrupt Register,                ICCHPIR  on Zynq-7000 */
#define GICC_ABPR                 (DT_GIC_PL390_CPU_BASE  + 0x01C)   /* Aliased Non-Secure Binary Point Register,          ICCABPR  on Zynq-7000 */
#define GICC_IDR                  (DT_GIC_PL390_CPU_BASE  + 0x0FC)   /* CPU Interface Implementer Identification Register, ICCIDR   on Zynq-7000 */

#define GICC_ENABLE               3

#define NO_GIC_INT_PENDING        1023

#define GIC_SPI_INT_BASE          32 /* SPIs start at ID 32, IDs 0..15 are the SGIs, IDs 16..26 are reserved, IDs 27..31 are the PPIs. */

#define GIC_INT_TYPE_MASK         0x3
#define GIC_INT_TYPE_EDGE         (1 << 1)

#define GICD_TYPER_NUM_LINES_MASK 0x1F

struct gic_pl390_ictl_config {
    u32_t isr_table_offset;
};

static void gic_pl390_dist_init (void)
{
    /* 
     * Global initialization function - this function disregards any banked registers.
     * -> In order to properly initialize the GIC in a SMP context with n cores, this
     *    function shall be executed only once by core [0], while gic_pl390_cpu_init()
     *    shall be called once by every core.
     */

    unsigned int gic_irqs = 0;
    unsigned int i        = 0;

    /*
     * Determine the number of available external interrupt lines.
     * Comp. Zynq-7000 manual, p. 1463, ICDICTR details.
     */

    gic_irqs = (sys_read32(GICD_TYPER) & GICD_TYPER_NUM_LINES_MASK);
    gic_irqs = (gic_irqs + 1) * 32;

    if (gic_irqs > GIC_MAX_NUM_LINES)
    {
        gic_irqs = GIC_MAX_NUM_LINES;
    }

    /*
     * Disable the Distributor -> CPU Interface forwarding of pending interrupts.
     * Comp. Zynq-7000 manual, p. 1462, ICDDCR details.
     * -> value 0 disables both the secure and non-secure interrupt forwarding.
     */

    sys_write32(0, GICD_CTLR);

    /*
     * Route all SPI interrupts to CPU #0 only.
     * FIXME: there should be a configurable default value for this operation.
     * -> Each ITARGETSR register configures 4 interrupts in bits [25:24], [17:16], [9:8] and [1:0].
     * -> More bits might be used in implementations with more than 2 CPUs
     * -> b01: interrupt targets CPU #0
     * -> b10: interrupt targets CPU #1
     * Comp. Zynq-7000 manual, p. 1471 ff., ICDIPTRn details.
     */

    for (i = GIC_SPI_INT_BASE; i < gic_irqs; i += 4)
    {
        sys_write32(0x01010101, GICD_ITARGETSRn + i);
    }

    /*
     * Configure all SPIs to be level triggered / active low.
     * -> Interrupts other than the SPIs (SGIs, PPIs) cannot be configured regarding their sensitivity / polarity.
     * -> Each ICFGR register configures 16 interrupts.
     * Comp. Zynq-7000 manual, p. 1492 ff., ICDICFRn details.
     */

    for (i = GIC_SPI_INT_BASE; i < gic_irqs; i += 16)
    {
        sys_write32(0, GICD_ICFGRn + (i / 4));
    }

    /* 
     * Set the priority to 0 for all global interrupts.
     * -> Each IPRIORITY register configures 4 interrupts, 1 byte per interrupt.
     * -> The 32 available priority levels are set in each byte's *upper* 5 bits, the lower 3 bits are always 0.
     * -> No need to shift anything at this point as we're setting the priority to 0 anyways.
     * -> Not doing anything about SGIs or PPIs here, this is done in gic_pl390_cpu_init().
     * Comp. Zynq-7000 manual, p. 1470 f., ICDIPRn details.
     */

    for (i = GIC_SPI_INT_BASE; i < gic_irqs; i += 4)
    {
        sys_write32(0, GICD_IPRIORITYRn + i);
    }

    /* 
     * Clear the pending and enable bits of all SPIs.
     * -> Each of the clear-active / clear-enable registers clears 32 interrupts.
     * Clear-Pending: comp. Zynq-7000 manual, p. 1468f.,  ICDICPRn defails.
     * Clear-Enable:  comp. Zynq-7000 manual, p. 1466ff., ICDICERn defails.
     */

    for (i = GIC_SPI_INT_BASE; i < gic_irqs; i += 32) 
    {
        sys_write32(0xFFFFFFFF, GICD_ICPENDRn   + (i / 8));
        sys_write32(0xFFFFFFFF, GICD_ICENABLERn + (i / 8));
    }

    /* Enable the forwarding of pending interrupts from the Distributor to the CPU interfaces */

    /* 
     * FIXME: proper configuration via KConfig?
     * Comp. Zynq-7000 manual, p. 1461 f., ICDDCR details.
     * Bits in ICDDCR:
     * [1] : EnableNS - Enable non-secure interrupt forwarding to the CPU.
     * [0] : EnableS  - Enable secure interrupt forwarding to the CPU.
     * -> 0x01 is effectively EnableNS when running in non-secure mode
     */

    sys_write32(0x03, GICD_CTLR);
}

static void gic_pl390_cpu_init (void)
{
    /* 
     * Local initialization function - this function handles any banked registers.
     * -> This function shall be called by every core in a SMP context.
     */

    int   i   = 0;
    u32_t val = 0;

    /* 
     * Clear the pending and enable bits of all PPIs and SGIs, that is, all interrupts
     * in the ID range from 0 to 31:
     * [00..15] SGIs
     * [16..26] reserved
     * [27..31] PPIs
     * -> Each of the clear-active / clear-enable registers clears 32 interrupts, therefore,
     * only the first register of each category needs to be accessed.
     * Clear-Pending: comp. Zynq-7000 manual, p. 1468f.,  ICDICPRn defails.
     * Clear-Enable:  comp. Zynq-7000 manual, p. 1466ff., ICDICERn defails.
     * TODO: Should the SGIs be initially enabled?
     */

    sys_write32(0xFFFFFFFF, GICD_ICPENDRn);
    sys_write32(0xFFFFFFFF, GICD_ICENABLERn);

    /*
     * Set priority on PPI and SGI interrupts
     */

    /* 
     * Set the priority to 20 for all banked interrupts.
     * -> Each IPRIORITY register configures 4 interrupts, 1 byte per interrupt.
     * -> The 32 available priority levels are set in each byte's *upper* 5 bits, the lower 3 bits are always 0.
     * -> Using prio level 20 -> SHL 3 = 0xA0
     * Comp. Zynq-7000 manual, p. 1470 f., ICDIPRn details.
     */

    for (i = 0; i < GIC_SPI_INT_BASE; i += 4)
    {
        sys_write32(0xA0A0A0A0, GICD_IPRIORITYRn + i);
    }

    /* 
     * Set the priority mask -> only interrupts with a priority higher than the
     * priority specified here will be serviced.
     * -> Minimum interrupt priority must be written to the register's lowest byte, upper 5 bits.
     * -> Notice: lower value = higher priority!
     * -> Accept the lower half of the supported priority range.
     * Comp. Zynq-7000 manual, p. 1444, ICCPMR details.
     */

    sys_write32(0x000000F0, GICC_PMR);

    /* Enable interrupts and signal them using the IRQ signal. */

    /* 
     * FIXME: proper configuration via KConfig?
     * Comp. Zynq-7000 manual, p. 1443 f., ICCICR details.
     * Bits in ICCICR:
     * [4] : SPBR     - Secure/non-secure binary point register switch
     * [3] : FIQen    - 0: Secure interrupts are delivered using IRQ, 1: delivery using FIQ.
     * [2] : AckCtl   - controls acknowledge behaviour if a secure read of ICCIAR returns a non-secure pending interrupt.
     * [1] : EnableNS - Enable non-secure interrupt forwarding to the CPU.
     * [0] : EnableS  - Enable secure interrupt forwarding to the CPU.
     */

    val = sys_read32(GICC_CTLR);
    val |= GICC_ENABLE;
    sys_write32(val, GICC_CTLR);
}

static void gic_pl390_irq_enable(struct device *dev, unsigned int irq)
{
    int int_grp = 0;
    int int_off = 0;

    //irq += GIC_SPI_INT_BASE;
    int_grp = irq / 32;
    int_off = irq % 32;

    sys_write32((1 << int_off), (GICD_ISENABLERn + int_grp * 4));
}

static void gic_pl390_irq_disable(struct device *dev, unsigned int irq)
{
    int int_grp = 0;
    int int_off = 0;

    //irq += GIC_SPI_INT_BASE;
    int_grp = irq / 32;
    int_off = irq % 32;

    sys_write32((1 << int_off), (GICD_ICENABLERn + int_grp * 4));
}

static unsigned int gic_pl390_irq_get_state(struct device *dev)
{
    // FIXME: get the GIC's global enable bit
    return 1;
}

static void gic_pl390_irq_set_priority(
    struct device *dev,
    unsigned int  irq, 
    unsigned int  prio, 
    u32_t         flags)
{
    int           int_grp = 0;
    int           int_off = 0;
    u8_t          val     = 0;

    if (irq == 0xFFFFFFFF)
    {
        return;
    }

    /*irq += GIC_SPI_INT_BASE;*/

    /* Set priority */
    sys_write8(prio & 0xff, GICD_IPRIORITYRn + irq);

    /* Set interrupt type */
    int_grp = irq / 4;
    int_off = (irq % 4) * 2;

    val  = sys_read8(GICD_ICFGRn + int_grp);
    val &= ~(GIC_INT_TYPE_MASK << int_off);

    if (    (irq >= GIC_SPI_INT_BASE)
         && (flags & IRQ_TYPE_EDGE))
    {
        val |= (SPI_IRQ_RISING_EDGE << int_off);
    }

    sys_write8(val, GICD_ICFGRn + int_grp);
}

static void gic_pl390_isr(void *arg)
{
    struct device *dev = arg;
    const struct gic_pl390_ictl_config *cfg = dev->config->config_info;
    void (*gic_pl390_isr_handle)(void *);
    int irq = 0;
    int isr_offset = 0;

    irq = sys_read32(GICC_IAR);
    irq &= 0x3ff;

    if (irq == NO_GIC_INT_PENDING) {
        printk("gic: Invalid interrupt\n");
        return;
    }

    isr_offset = cfg->isr_table_offset + irq;

    gic_pl390_isr_handle = _sw_isr_table[isr_offset].isr;
    if (gic_pl390_isr_handle)
    {
        gic_pl390_isr_handle(_sw_isr_table[isr_offset].arg);
    }
    else
    {
        printk("gic: no handler found for int %d\n", irq);
    }

    /* set to inactive */
    sys_write32(irq, GICC_EOIR);
}

static int gic_pl390_init(struct device *unused);

static const struct irq_next_level_api gic_pl390_apis = {
    .intr_enable       = gic_pl390_irq_enable,
    .intr_disable      = gic_pl390_irq_disable,
    .intr_get_state    = gic_pl390_irq_get_state,
    .intr_set_priority = gic_pl390_irq_set_priority,
};

static const struct gic_pl390_ictl_config gic_pl390_config = {
    .isr_table_offset  = CONFIG_2ND_LVL_ISR_TBL_OFFSET,
};

#ifdef DT_INST_0_ARM_V7A_GIC_PL390
DEVICE_AND_API_INIT(
    arm_gic_pl390,
    DT_INST_0_ARM_V7A_GIC_PL390_LABEL,
    gic_pl390_init,
    NULL,
    &gic_pl390_config,
    PRE_KERNEL_1,
    CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
    &gic_pl390_apis);
#endif /* DT_INST_0_ARM_V7A_GIC_PL390 */

/**
 *
 * @brief Initialize the GIC PL390 device driver
 *
 *
 * @return N/A
 */
 
#define GIC_PL390_PARENT_IRQ       0
#define GIC_PL390_PARENT_IRQ_PRI   0
#define GIC_PL390_PARENT_IRQ_FLAGS 0

static int gic_pl390_init(struct device *unused)
{
    IRQ_CONNECT(
        GIC_PL390_PARENT_IRQ,
        GIC_PL390_PARENT_IRQ_PRI,
        gic_pl390_isr,
        DEVICE_GET(arm_gic_pl390),
        GIC_PL390_PARENT_IRQ_FLAGS);

    /* Init of Distributor interface registers */
    gic_pl390_dist_init();

    /* Init CPU interface registers */
    gic_pl390_cpu_init();

    return 0;
}
