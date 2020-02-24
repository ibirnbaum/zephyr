/*
 * Copyright (c) 2020, Immo Birnbaum
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_DRIVERS_ETHERNET_ETH_XLNX_GEM_PRIV_H_
#define _ZEPHYR_DRIVERS_ETHERNET_ETH_XLNX_GEM_PRIV_H_

#include <kernel.h>
#include <zephyr/types.h>

#define ETH_XLNX_BUFFER_ALIGNMENT					4			/* RX/TX buffer alignment (in bytes) */

/* Buffer descriptor (BD) related defines */

/* RX BD bits & masks: comp. Zynq-7000 TRM, Table 16-2. */

#define ETH_XLNX_GEM_RXBD_WRAP_BIT					0x00000002 	/* Address word: Wrap bit, last BD */
#define ETH_XLNX_GEM_RXBD_USED_BIT					0x00000001 	/* Address word: BD used bit */
#define ETH_XLNX_GEM_RXBD_BUFFER_ADDR_MASK			0xFFFFFFFC	/* Address word: Mask for effective buffer address -> excludes [1..0] */
#define ETH_XLNX_GEM_RXBD_BCAST_BIT					0x80000000	/* Control word: Broadcast detected */
#define ETH_XLNX_GEM_RXBD_MCAST_HASH_MATCH_BIT		0x40000000	/* Control word: Multicast hash match detected */
#define ETH_XLNX_GEM_RXBD_UCAST_HASH_MATCH_BIT		0x20000000	/* Control word: Unicast hash match detected */
#define ETH_XLNX_GEM_RXBD_SPEC_ADDR_MATCH_BIT		0x08000000	/* Control word: Specific address match detected */
#define ETH_XLNX_GEM_RXBD_SPEC_ADDR_MASK			0x00000003	/* Control word: Bits indicating which specific address register was matched */
#define ETH_XLNX_GEM_RXBD_SPEC_ADDR_SHIFT			25			/* Control word: Shift for specific address register ID bits */
#define ETH_XLNX_GEM_RXBD_BIT24						0x01000000	/* Control word: Bit [24] - this bit has different semantics depending on whether RX checksum offloading is enabled or not */
#define ETH_XLNX_GEM_RXBD_BITS23_22_MASK			0x00000003	/* Control word: Bits [23..22] - these bits have different semantics depending on whether RX checksum offloading is enabled or not */
#define ETH_XLNX_GEM_RXBD_BITS23_22_SHIFT			22			/* Control word: Shift for multi-purpose bits [23..22] */
#define ETH_XLNX_GEM_RXBD_VLAN_TAG_DETECTED_BIT		0x00200000  /* Control word: VLAN tag (type ID 0x8100) detected */
#define ETH_XLNX_GEM_RXBD_PRIO_TAG_DETECTED_BIT		0x00100000  /* Control word: VLAN tag (type ID 0x8100) detected */
#define ETH_XLNX_GEM_RXBD_VLAN_PRIORITY_MASK		0x00000007  /* Control word: Bits [19..17] contain the VLAN priority */
#define ETH_XLNX_GEM_RXBD_VLAN_PRIORITY_SHIFT		17			/* Control word: Shift for VLAN priority bits [19..17] */
#define ETH_XLNX_GEM_RXBD_CFI_BIT					0x00010000  /* Control word: Canonical format indicator bit */
#define ETH_XLNX_GEM_RXBD_END_OF_FRAME_BIT			0x00008000	/* Control word: End-of-frame bit */
#define ETH_XLNX_GEM_RXBD_START_OF_FRAME_BIT		0x00004000	/* Control word: Start-of-frame bit */
#define ETH_XLNX_GEM_RXBD_FCS_STATUS_BIT			0x00002000	/* Control word: FCS status bit for FCS ignore mode */
#define ETH_XLNX_GEM_RXBD_FRAME_LENGTH_MASK			0x00001FFF	/* Control word: mask for data length of received frame */

/* RX BD bits & masks: comp. Zynq-7000 TRM, Table 16-3. */

#define ETH_XLNX_GEM_TXBD_USED_BIT					0x80000000 	/* Control word: BD used marker */
#define ETH_XLNX_GEM_TXBD_WRAP_BIT					0x40000000 	/* Control word: Wrap bit, last BD */
#define ETH_XLNX_GEM_TXBD_RETRY_BIT					0x20000000 	/* Control word: Retry limit exceeded */
#define ETH_XLNX_GEM_TXBD_URUN_BIT					0x10000000 	/* Control word: Transmit underrun occurred */
#define ETH_XLNX_GEM_TXBD_EXH_BIT					0x08000000 	/* Control word: Buffers exhausted */
#define ETH_XLNX_GEM_TXBD_LAC_BIT					0x04000000 	/* Control word: Late collision */
#define ETH_XLNX_GEM_TXBD_NOCRC_BIT					0x00010000 	/* Control word: No CRC */
#define ETH_XLNX_GEM_TXBD_LAST_BIT					0x00008000 	/* Control word: Last buffer */
#define ETH_XLNX_GEM_TXBD_LEN_MASK					0x00003FFF 	/* Control word: Mask for length field */
#define ETH_XLNX_GEM_TXBD_ERR_MASK					0x3C000000 	/* Control word: Mask for error field */

/* SLCR register space & magic words */

#define ETH_XLNX_SLCR_BASE_ADDRESS						0xF8000000
#define ETH_XLNX_SLCR_LOCK_REGISTER						(ETH_XLNX_SLCR_BASE_ADDRESS + 0x00000004)
#define ETH_XLNX_SLCR_UNLOCK_REGISTER					(ETH_XLNX_SLCR_BASE_ADDRESS + 0x00000008)
#define ETH_XLNX_SLCR_APER_CLK_CTRL_REGISTER			(ETH_XLNX_SLCR_BASE_ADDRESS + 0x0000012C)
#define ETH_XLNX_SLCR_GEM0_RCLK_CTRL_REGISTER			(ETH_XLNX_SLCR_BASE_ADDRESS + 0x00000138)
#define ETH_XLNX_SLCR_GEM1_RCLK_CTRL_REGISTER			(ETH_XLNX_SLCR_BASE_ADDRESS + 0x0000013C)
#define ETH_XLNX_SLCR_RCLK_CTRL_REGISTER_SRC_MASK       0x00000001
#define ETH_XLNX_SLCR_RCLK_CTRL_REGISTER_SRC_SHIFT		4
#define ETH_XLNX_SLCR_GEM0_CLK_CTRL_REGISTER			(ETH_XLNX_SLCR_BASE_ADDRESS + 0x00000140)
#define ETH_XLNX_SLCR_GEM1_CLK_CTRL_REGISTER			(ETH_XLNX_SLCR_BASE_ADDRESS + 0x00000144)
#define ETH_XLNX_SLRC_CLK_CTR_REGISTER_DIV_MASK         0x0000003F
#define ETH_XLNX_SLRC_CLK_CTR_REGISTER_DIV1_SHIFT		20
#define ETH_XLNX_SLRC_CLK_CTR_REGISTER_DIV0_SHIFT		8
#define ETH_XLNX_SLRC_CLK_CTR_REGISTER_REF_PLL_MASK		0x00000007
#define ETH_XLNX_SLRC_CLK_CTR_REGISTER_REF_PLL_SHIFT	4

#define ETH_XLNX_SLCR_UNLOCK_CONSTANT					0xDF0D
#define ETH_XLNX_SLCR_LOCK_CONSTANT						0x767B
#define ETH_XLNX_SLCR_CLK_ENABLE_BIT					0x00000001
#define ETH_XLNX_SLCR_RCLK_ENABLE_BIT					0x00000001

/* Register offsets within the respective GEM's address space */

#define ETH_XLNX_GEM_NWCTRL_OFFSET					0x00000000 /* gem.net_ctrl			Network Control           register */
#define ETH_XLNX_GEM_NWCFG_OFFSET					0x00000004 /* gem.net_cfg			Network Configuration     register */
#define ETH_XLNX_GEM_NWSR_OFFSET					0x00000008 /* gem.net_status		Network Status            register */
#define ETH_XLNX_GEM_DMACR_OFFSET					0x00000010 /* gem.dma_cfg			DMA Control               register */
#define ETH_XLNX_GEM_TXSR_OFFSET					0x00000014 /* gem.tx_status			TX Status                 register */
#define ETH_XLNX_GEM_RXQBASE_OFFSET					0x00000018 /* gem.rx_qbar			RXQ base address          register */
#define ETH_XLNX_GEM_TXQBASE_OFFSET					0x0000001C /* gem.tx_qbar			TXQ base address          register */
#define ETH_XLNX_GEM_RXSR_OFFSET					0x00000020 /* gem.rx_status			RX Status                 register */
#define ETH_XLNX_GEM_ISR_OFFSET						0x00000024 /* gem.intr_status		Interrupt status		  register */
#define ETH_XLNX_GEM_IER_OFFSET						0x00000028 /* gem.intr_en			Interrupt enable          register */
#define ETH_XLNX_GEM_IDR_OFFSET						0x0000002C /* gem.intr_dis			Interrupt disable         register */
#define ETH_XLNX_GEM_IMR_OFFSET						0x00000030 /* gem.intr_mask			Interrupt mask            register */
#define ETH_XLNX_GEM_PHY_MAINTENANCE_OFFSET			0x00000034 /* gem.phy_maint			PHY maintenance           register */
#define ETH_XLNX_GEM_LADDR1L_OFFSET					0x00000088 /* gem.spec_addr1_bot	Specific address 1 bottom register */
#define ETH_XLNX_GEM_LADDR1H_OFFSET					0x0000008C /* gem.spec_addr1_top	Specific address 1 top    register */
#define ETH_XLNX_GEM_LADDR2L_OFFSET					0x00000090 /* gem.spec_addr2_bot	Specific address 2 bottom register */
#define ETH_XLNX_GEM_LADDR2H_OFFSET					0x00000094 /* gem.spec_addr2_top	Specific address 2 top    register */
#define ETH_XLNX_GEM_LADDR3L_OFFSET					0x00000098 /* gem.spec_addr3_bot	Specific address 3 bottom register */
#define ETH_XLNX_GEM_LADDR3H_OFFSET					0x0000009C /* gem.spec_addr3_top	Specific address 3 top    register */
#define ETH_XLNX_GEM_LADDR4L_OFFSET					0x000000A0 /* gem.spec_addr4_bot	Specific address 4 bottom register */
#define ETH_XLNX_GEM_LADDR4H_OFFSET					0x000000A4 /* gem.spec_addr4_top	Specific address 4 top    register */

/* Masks for clearing registers during initialization */

#define ETH_XLNX_GEM_STATCLR_MASK					0x00000020 /* gem.net_ctrl	[clear_stat_regs] */
#define ETH_XLNX_GEM_TXSRCLR_MASK					0x000000FF /* gem.tx_status	[7..0]            */
#define ETH_XLNX_GEM_RXSRCLR_MASK					0x0000000F /* gem.tx_status	[3..0]            */
#define ETH_XLNX_GEM_IDRCLR_MASK					0x07FFFFFF /* gem.intr_dis	[26..0]           */

/* (Shift) masks for individual registers' fields */

#define ETH_XLNX_GEM_NWCTRL_RXTSTAMP_BIT			0x00008000 /* gem.net_ctrl: RX Timestamp in CRC */
#define ETH_XLNX_GEM_NWCTRL_ZEROPAUSETX_BIT			0x00001000 /* gem.net_ctrl: Transmit zero quantum pause frame */
#define ETH_XLNX_GEM_NWCTRL_PAUSETX_BIT				0x00000800 /* gem.net_ctrl: Transmit pause frame */
#define ETH_XLNX_GEM_NWCTRL_HALTTX_BIT				0x00000400 /* gem.net_ctrl: Halt transmission after current frame */
#define ETH_XLNX_GEM_NWCTRL_STARTTX_BIT				0x00000200 /* gem.net_ctrl: Start transmission (tx_go) */
#define ETH_XLNX_GEM_NWCTRL_STATWEN_BIT				0x00000080 /* gem.net_ctrl: Enable writing to stat counters */
#define ETH_XLNX_GEM_NWCTRL_STATINC_BIT				0x00000040 /* gem.net_ctrl: Increment statistic registers */
#define ETH_XLNX_GEM_NWCTRL_STATCLR_BIT				0x00000020 /* gem.net_ctrl: Clear statistic registers */
#define ETH_XLNX_GEM_NWCTRL_MDEN_BIT				0x00000010 /* gem.net_ctrl: Enable MDIO port */
#define ETH_XLNX_GEM_NWCTRL_TXEN_BIT				0x00000008 /* gem.net_ctrl: Enable transmit */
#define ETH_XLNX_GEM_NWCTRL_RXEN_BIT				0x00000004 /* gem.net_ctrl: Enable receive */
#define ETH_XLNX_GEM_NWCTRL_LOOPEN_BIT				0x00000002 /* gem.net_ctrl: local loopback */

#define ETH_XLNX_GEM_NWCFG_IGNIPGRXERR_BIT			0x40000000 /* gem.net_cfg: Ignore IPG RX Error */
#define ETH_XLNX_GEM_NWCFG_BADPREAMBEN_BIT			0x20000000 /* gem.net_cfg: Disable rejection of non-standard preamble */
#define ETH_XLNX_GEM_NWCFG_IPDSTRETCH_BIT			0x10000000 /* gem.net_cfg: Enable transmit IPG */
#define ETH_XLNX_GEM_NWCFG_SGMIIEN_BIT				0x08000000 /* gem.net_cfg: Enable SGMII mode */
#define ETH_XLNX_GEM_NWCFG_FCSIGNORE_BIT			0x04000000 /* gem.net_cfg: Disable rejection of FCS error */
#define ETH_XLNX_GEM_NWCFG_HDRXEN_BIT				0x02000000 /* gem.net_cfg: RX half duplex */
#define ETH_XLNX_GEM_NWCFG_RXCHKSUMEN_BIT			0x01000000 /* gem.net_cfg: Enable RX checksum offload */
#define ETH_XLNX_GEM_NWCFG_PAUSECOPYDI_BIT			0x00800000 /* gem.net_cfg: Do not copy pause Frames to memory */
#define ETH_XLNX_GEM_NWCFG_DBUSW_MASK				0x3        /* gem.net_cfg: Mask for data bus width */
#define ETH_XLNX_GEM_NWCFG_DBUSW_SHIFT				21         /* gem.net_cfg: Shift for data bus width */
#define ETH_XLNX_GEM_NWCFG_MDC_MASK					0x7        /* gem.net_cfg: Mask for MDC clock divisor */
#define ETH_XLNX_GEM_NWCFG_MDC_SHIFT				18         /* gem.net_cfg: Shift for MDC clock divisor */
#define ETH_XLNX_GEM_NWCFG_MDCCLKDIV_MASK			0x001C0000 /* gem.net_cfg: MDC Mask PCLK divisor */
#define ETH_XLNX_GEM_NWCFG_FCSREM_BIT				0x00020000 /* gem.net_cfg: Discard FCS from received frames */
#define ETH_XLNX_GEM_NWCFG_LENGTHERRDSCRD_BIT		0x00010000 /* gem.net_cfg: RX length error discard */
#define ETH_XLNX_GEM_NWCFG_RXOFFS_MASK				0x00000003 /* gem.net_cfg: Mask for RX buffer offset */
#define ETH_XLNX_GEM_NWCFG_RXOFFS_SHIFT				14         /* gem.net_cfg: Shift for RX buffer offset*/
#define ETH_XLNX_GEM_NWCFG_PAUSEEN_BIT				0x00002000 /* gem.net_cfg: Enable pause TX */
#define ETH_XLNX_GEM_NWCFG_RETRYTESTEN_BIT			0x00001000 /* gem.net_cfg: Retry test */
#define ETH_XLNX_GEM_NWCFG_TBIINSTEAD_BIT			0x00000800 /* gem.net_cfg: Use TBI instead of the GMII/MII interface */
#define ETH_XLNX_GEM_NWCFG_1000_BIT					0x00000400 /* gem.net_cfg: Gigabit mode */
#define ETH_XLNX_GEM_NWCFG_EXTADDRMATCHEN_BIT		0x00000200 /* gem.net_cfg: External address match enable */
#define ETH_XLNX_GEM_NWCFG_1536RXEN_BIT				0x00000100 /* gem.net_cfg: Enable 1536 byte frames reception */
#define ETH_XLNX_GEM_NWCFG_UCASTHASHEN_BIT			0x00000080 /* gem.net_cfg: Receive unicast hash frames */
#define ETH_XLNX_GEM_NWCFG_MCASTHASHEN_BIT			0x00000040 /* gem.net_cfg: Receive multicast hash frames */
#define ETH_XLNX_GEM_NWCFG_BCASTDIS_BIT				0x00000020 /* gem.net_cfg: Do not receive broadcast frames */
#define ETH_XLNX_GEM_NWCFG_COPYALLEN_BIT			0x00000010 /* gem.net_cfg: Copy all frames = promiscuous mode */
#define ETH_XLNX_GEM_NWCFG_NVLANDISC_BIT			0x00000004 /* gem.net_cfg: Receive only VLAN frames */
#define ETH_XLNX_GEM_NWCFG_FDEN_BIT					0x00000002 /* gem.net_cfg: Full duplex */
#define ETH_XLNX_GEM_NWCFG_100_BIT					0x00000001 /* gem.net_cfg: 10 or 100 Mbs */

#define ETH_XLNX_GEM_DMACR_DISCNOAHB_BIT			0x01000000 /* gem.dma_cfg: Discard packets when AHB resource is unavailable */
#define ETH_XLNX_GEM_DMACR_RX_BUF_MASK				0x000000FF /* gem.dma_cfg: Mask for RX buffer size */
#define ETH_XLNX_GEM_DMACR_RX_BUF_SHIFT				16         /* gem.dma_cfg: Shift count for RX buffer size */
#define ETH_XLNX_GEM_DMACR_TCP_CHKSUM_BIT			0x00000800 /* gem.dma_cfg: Enable/disable TCP|UDP/IP TX checksum offload */
#define ETH_XLNX_GEM_DMACR_TX_SIZE_BIT				0x00000400 /* gem.dma_cfg: TX buffer half/full memory size */
#define ETH_XLNX_GEM_DMACR_RX_SIZE_MASK				0x00000300 /* gem.dma_cfg: Mask for RX buffer memory size */
#define ETH_XLNX_GEM_DMACR_RX_SIZE_SHIFT			8          /* gem.dma_cfg: Shift for for RX buffer memory size */
#define ETH_XLNX_GEM_DMACR_ENDIAN_BIT				0x00000080 /* gem.dma_cfg: Endianess configuration */
#define ETH_XLNX_GEM_DMACR_DESCR_ENDIAN_BIT			0x00000040 /* gem.dma_cfg: Descriptor access endianess configuration */
#define ETH_XLNX_GEM_DMACR_AHB_BURST_LENGTH_MASK	0x0000001F /* gem.dma_cfg: AHB burst length */

#define ETH_XLNX_GEM_IXR_PTPPSTX_BIT				0x02000000 /* gem.intr_*: PTP Psync transmitted */
#define ETH_XLNX_GEM_IXR_PTPPDRTX_BIT				0x01000000 /* gem.intr_*: PTP Pdelay_req transmitted */
#define ETH_XLNX_GEM_IXR_PTPSTX_BIT					0x00800000 /* gem.intr_*: PTP Sync transmitted */
#define ETH_XLNX_GEM_IXR_PTPDRTX_BIT				0x00400000 /* gem.intr_*: PTP Delay_req transmitted */
#define ETH_XLNX_GEM_IXR_PTPPSRX_BIT				0x00200000 /* gem.intr_*: PTP Psync received */
#define ETH_XLNX_GEM_IXR_PTPPDRRX_BIT				0x00100000 /* gem.intr_*: PTP Pdelay_req received */
#define ETH_XLNX_GEM_IXR_PTPSRX_BIT					0x00080000 /* gem.intr_*: PTP Sync received */
#define ETH_XLNX_GEM_IXR_PTPDRRX_BIT				0x00040000 /* gem.intr_*: PTP Delay_req received */
#define ETH_XLNX_GEM_IXR_PARTNER_PGRX_BIT			0x00020000 /* gem.intr_*: partner_pg_rx */
#define ETH_XLNX_GEM_IXR_AUTONEG_COMPLETE_BIT		0x00010000 /* gem.intr_*: Auto-negotiation completed */
#define ETH_XLNX_GEM_IXR_EXTERNAL_INT_BIT			0x00008000 /* gem.intr_*: External interrupt signal */
#define ETH_XLNX_GEM_IXR_PAUSETX_BIT				0x00004000 /* gem.intr_*: Pause frame transmitted */
#define ETH_XLNX_GEM_IXR_PAUSEZERO_BIT				0x00002000 /* gem.intr_*: Pause time has reached zero */
#define ETH_XLNX_GEM_IXR_PAUSENZERO_BIT				0x00001000 /* gem.intr_*: Pause frame received */
#define ETH_XLNX_GEM_IXR_HRESPNOK_BIT				0x00000800 /* gem.intr_*: hresp not ok */
#define ETH_XLNX_GEM_IXR_RXOVR_BIT					0x00000400 /* gem.intr_*: Receive overrun occurred */
#define ETH_XLNX_GEM_IXR_TXCOMPL_BIT				0x00000080 /* gem.intr_*: Frame transmitted ok */
#define ETH_XLNX_GEM_IXR_TXEXH_BIT					0x00000040 /* gem.intr_*: Transmit err occurred or no buffers*/
#define ETH_XLNX_GEM_IXR_RETRY_BIT					0x00000020 /* gem.intr_*: Retry limit exceeded */
#define ETH_XLNX_GEM_IXR_URUN_BIT					0x00000010 /* gem.intr_*: Transmit underrun */
#define ETH_XLNX_GEM_IXR_TXUSED_BIT					0x00000008 /* gem.intr_*: Tx buffer used bit read */
#define ETH_XLNX_GEM_IXR_RXUSED_BIT					0x00000004 /* gem.intr_*: Rx buffer used bit read */
#define ETH_XLNX_GEM_IXR_FRAMERX_BIT				0x00000002 /* gem.intr_*: Frame received ok */
#define ETH_XLNX_GEM_IXR_MGMNT_BIT					0x00000001 /* gem.intr_*: PHY management complete */
#define ETH_XLNX_GEM_IXR_ALL_MASK					0x03FC7FFE /* gem.intr_*: Bit mask for all handled interrupt sources */

#define ETH_XLNX_GEM_MDIO_IDLE_BIT					0x00000004 /* gem.net_status: PHY management idle bit */
#define ETH_XLNX_GEM_MDIO_IN_STATUS_BIT				0x00000002 /* gem.net_status: MDIO input status */

#define ETH_XLNX_GEM_PHY_MAINT_CONST_BITS    		0x40020000 /* gem.phy_maint: Bits constant for every operation: [31:30], [17:16] */
#define ETH_XLNX_GEM_PHY_MAINT_READ_OP_BIT	  		0x20000000 /* gem.phy_maint: Read operation control bit */
#define ETH_XLNX_GEM_PHY_MAINT_WRITE_OP_BIT	 		0x10000000 /* gem.phy_maint: Write operation control bit */
#define ETH_XLNX_GEM_PHY_MAINT_PHY_ADDRESS_MASK  	0x0000001F /* gem.phy_maint: PHY address bits mask */
#define ETH_XLNX_GEM_PHY_MAINT_PHY_ADDRESS_SHIFT	23         /* gem.phy_maint: Shift for PHY address bits */
#define ETH_XLNX_GEM_PHY_MAINT_REGISTER_ID_MASK		0x0000001F /* gem.phy_maint: PHY register bits mask */
#define ETH_XLNX_GEM_PHY_MAINT_REGISTER_ID_SHIFT	18         /* gem.phy_maint: Shift for PHY register bits */
#define ETH_XLNX_GEM_PHY_MAINT_DATA_MASK			0x0000FFFF /* gem.phy_maint: 16-bit data word */

/* AMBA clock configuration related defines */

#define ETH_XLNX_GEM_AMBA_CLK_ENABLE_BIT_GEM0		(1 << 6)
#define ETH_XLNX_GEM_AMBA_CLK_ENABLE_BIT_GEM1		(1 << 7)

/* Auxiliary thread trigger bits */

#define ETH_XLNX_GEM_AUX_THREAD_RXDONE_BIT			(1 << 0)
#define ETH_XLNX_GEM_AUX_THREAD_TXDONE_BIT			(1 << 1)
#define ETH_XLNX_GEM_AUX_THREAD_POLL_PHY_BIT		(1 << 7)

/* PHY registers & constants -> Marvell Alaska specific! */

#define PHY_BASE_REGISTERS_PAGE							0
#define PHY_COPPER_CONTROL_REGISTER						0
#define PHY_COPPER_STATUS_REGISTER                      1
#define PHY_IDENTIFIER_1_REGISTER						2
#define PHY_IDENTIFIER_2_REGISTER						3
#define PHY_COPPER_AUTONEG_ADV_REGISTER					4
#define PHY_COPPER_LINK_PARTNER_ABILITY_REGISTER		5
#define PHY_1000BASET_CONTROL_REGISTER					9
#define PHY_COPPER_CONTROL_1_REGISTER					16
#define PHY_COPPER_STATUS_1_REGISTER					17
#define PHY_COPPER_INTERRUPT_ENABLE_REGISTER			18
#define PHY_COPPER_INTERRUPT_STATUS_REGISTER			19
#define PHY_COPPER_PAGE_SWITCH_REGISTER					22
#define PHY_GENERAL_CONTROL_1_REGISTER					20
#define PHY_GENERAL_CONTROL_1_PAGE						18

#define PHY_ADV_BIT_100BASET_FDX						(1 << 8)
#define PHY_ADV_BIT_100BASET_HDX						(1 << 7)
#define PHY_ADV_BIT_10BASET_FDX							(1 << 6)
#define PHY_ADV_BIT_10BASET_HDX							(1 << 5)

#define PHY_MDIX_CONFIG_MASK							0x0003
#define PHY_MDIX_CONFIG_SHIFT							5
#define PHY_MODE_CONFIG_MASK							0x0003
#define PHY_MODE_CONFIG_SHIFT							0

#define PHY_COPPER_SPEED_CHANGED_INTERRUPT_BIT			(1 << 14)
#define PHY_COPPER_DUPLEX_CHANGED_INTERRUPT_BIT			(1 << 13)
#define PHY_COPPER_AUTONEG_COMPLETED_INTERRUPT_BIT		(1 << 11)
#define PHY_COPPER_LINK_STATUS_CHANGED_INTERRUPT_BIT	(1 << 10)
#define PHY_COPPER_LINK_STATUS_BIT_SHIFT                5

#define PHY_LINK_SPEED_SHIFT							14
#define PHY_LINK_SPEED_MASK								0x3

/* Device configuration / run-time data resolver macros */

#define DEV_CFG(dev) \
	((struct eth_xlnx_gem_dev_cfg *)(dev)->config->config_info)
#define DEV_DATA(dev) \
	((struct eth_xlnx_gem_dev_data *)(dev)->driver_data)

/* IRQ handler function type */

typedef void (*eth_xlnx_gem_config_irq_t)(struct device *dev);

/* Enums for bitfields representing configuration settings */

enum eth_xlnx_link_speed
{
	/* The values of this enum are consecutively numbered */
	LINK_DOWN = 0,
	LINK_10MBIT,
	LINK_100MBIT,
	LINK_1GBIT
};

enum eth_xlnx_amba_dbus_width
{
	/* The values of this enum are consecutively numbered */
	AMBA_AHB_DBUS_WIDTH_32BIT = 0,
	AMBA_AHB_DBUS_WIDTH_64BIT,
	AMBA_AHB_DBUS_WIDTH_128BIT
};

enum eth_xlnx_mdc_clock_divisor
{
	/* The values of this enum are consecutively numbered */
	MDC_DIVISOR_8 = 0, 
	MDC_DIVISOR_16, 
	MDC_DIVISOR_32, 
	MDC_DIVISOR_48,
	MDC_DIVISOR_64, 
	MDC_DIVISOR_96, 
	MDC_DIVISOR_128, 
	MDC_DIVISOR_224
};

enum eth_xlnx_hwrx_buffer_size
{
	/* The values of this enum are consecutively numbered */
	HWRX_BUFFER_SIZE_1KB = 0,
	HWRX_BUFFER_SIZE_2KB,
	HWRX_BUFFER_SIZE_4KB,
	HWRX_BUFFER_SIZE_8KB
};

enum eth_xlnx_ahb_burst_length
{
	AHB_BURST_SINGLE = 1,
	AHB_BURST_INCR4  = 4,
	AHB_BURST_INCR8  = 8,
	AHB_BURST_INCR16 = 16
};

enum eth_xlnx_ref_pll
{
	IO_PLL		= 0,
	ARM_PLL		= 2,
	DDR_PLL		= 3,
	EMIO_CLK	= 4
};

enum eth_xlnx_clk_src
{
	/* The values of this enum are consecutively numbered */
	CLK_SRC_MIO	= 0,
	CLK_SRC_EMIO
};

/* DMA buffer descriptor */

struct eth_xlnx_gem_bd
{
	u32_t 					addr;			 /* Buffer physical address */
	u32_t					ctrl;			 /* Control word */
};

/* DMA buffer descriptor management structure, used for both RX and TX buffer rings */

struct eth_xlnx_gem_bdring
{
	struct k_sem			ring_sem;		 /* Concurrent modification protection */
	struct eth_xlnx_gem_bd	*first_bd;		 /* Points to the first BD in list */
	u8_t					next_to_use;	 /* The next BD to be used for TX */
	u8_t					next_to_process; /* The next BD whose status shall be processed (both RX/TX) */
	u8_t					free_bds;		 /* Number of currently available BDs */
};

/*
 * Separate BD / Buffer structs for GEM0/1, as buffer counts and sizes can be configured
 * per interface.
 */

#if defined(DT_INST_0_XLNX_GEM) && defined(CONFIG_ETH_XLNX_GEM_PORT_0)

/* DMA memory area - GEM0 */

struct eth_xlnx_dma_area_gem0
{
	struct eth_xlnx_gem_bd	rx_bd[CONFIG_ETH_XLNX_GEM_PORT_0_RXBD_COUNT];
	struct eth_xlnx_gem_bd	tx_bd[CONFIG_ETH_XLNX_GEM_PORT_0_TXBD_COUNT];

	u8_t 					rx_buffer
								[CONFIG_ETH_XLNX_GEM_PORT_0_RXBD_COUNT]
								 [((CONFIG_ETH_XLNX_GEM_PORT_0_RX_BUFFER_SIZE
								  + (ETH_XLNX_BUFFER_ALIGNMENT - 1)) & ~(ETH_XLNX_BUFFER_ALIGNMENT -1))];
	u8_t 					tx_buffer
								[CONFIG_ETH_XLNX_GEM_PORT_0_TXBD_COUNT]
								 [((CONFIG_ETH_XLNX_GEM_PORT_0_TX_BUFFER_SIZE
								  + (ETH_XLNX_BUFFER_ALIGNMENT - 1)) & ~(ETH_XLNX_BUFFER_ALIGNMENT -1))];
};

#endif

#if defined(DT_INST_1_XLNX_GEM) && defined (CONFIG_ETH_XLNX_GEM_PORT_1)

/* DMA memory area - GEM1 */

struct eth_xlnx_dma_area_gem1
{
	struct eth_xlnx_gem_bd	rx_bd[CONFIG_ETH_XLNX_GEM_PORT_1_RXBD_COUNT];
	struct eth_xlnx_gem_bd	tx_bd[CONFIG_ETH_XLNX_GEM_PORT_1_TXBD_COUNT];

	u8_t 					rx_buffer
								[CONFIG_ETH_XLNX_GEM_PORT_1_RXBD_COUNT]
								 [((CONFIG_ETH_XLNX_GEM_PORT_1_RX_BUFFER_SIZE
								  + (ETH_XLNX_BUFFER_ALIGNMENT - 1)) & ~(ETH_XLNX_BUFFER_ALIGNMENT -1))];
	u8_t 					tx_buffer
								[CONFIG_ETH_XLNX_GEM_PORT_1_TXBD_COUNT]
								 [((CONFIG_ETH_XLNX_GEM_PORT_1_TX_BUFFER_SIZE
								  + (ETH_XLNX_BUFFER_ALIGNMENT - 1)) & ~(ETH_XLNX_BUFFER_ALIGNMENT -1))];
};

#endif

/* Device constant configuration parameters */

struct eth_xlnx_gem_dev_cfg {
	u32_t							base_addr;
	eth_xlnx_gem_config_irq_t		config_func;

	enum eth_xlnx_link_speed		max_link_speed;
	u8_t							init_phy;
	u8_t							phy_advertise_lower;

	enum eth_xlnx_amba_dbus_width	amba_dbus_width;
	enum eth_xlnx_ahb_burst_length	ahb_burst_length;
	enum eth_xlnx_hwrx_buffer_size	hw_rx_buffer_size;
	u8_t							hw_rx_buffer_offset;
	u8_t							ahb_rx_buffer_size;
	u8_t							amba_clk_en_bit;

	enum eth_xlnx_ref_pll			reference_pll;
	u32_t							reference_pll_ref_clk_multi;
	enum eth_xlnx_clk_src			gem_clk_source;
	u32_t							gem_clk_divisor1;
	u32_t							gem_clk_divisor0;
	u32_t							slcr_clk_register_addr;
	u32_t							slcr_rclk_register_addr;

	u8_t							rxbd_count;
	u8_t							txbd_count;
	u16_t							rx_buffer_size;
	u16_t							tx_buffer_size;

	u8_t							ignore_igp_rxer;
	u8_t							disable_reject_nsp;
	u8_t							enable_igp_stretch;
	u8_t							enable_sgmii_mode;
	u8_t							disable_reject_fcs_crc_errors;
	u8_t							enable_rx_halfdup_while_tx;
	u8_t							enable_rx_chksum_offload;
	u8_t							disable_pause_copy;
	u8_t							discard_rx_fcs;
	u8_t							discard_rx_length_errors;
	u8_t							enable_pause;
	u8_t							enable_tbi;
	u8_t							ext_addr_match;
	u8_t							enable_1536_frames;
	u8_t							enable_ucast_hash;
	u8_t							enable_mcast_hash;
	u8_t							disable_bcast;
	u8_t							copy_all_frames;
	u8_t							discard_non_vlan;
	u8_t							enable_fdx;
	u8_t							disc_rx_ahb_unavail;
	u8_t							enable_tx_chksum_offload;
	u8_t							tx_buffer_size_full;
	u8_t							enable_ahb_packet_endian_swap;
	u8_t							enable_ahb_md_endian_swap;
};

/* Device run time data */

struct eth_xlnx_gem_dev_data {
	struct net_if					*iface;
	u8_t 							mac_addr[6];

	struct k_sem					tx_done_sem;

	struct k_thread					aux_thread_data;
	k_tid_t 						aux_thread_tid;
	int								aux_thread_prio;
	struct k_msgq                   aux_thread_msgq;
	u8_t __aligned(4)               aux_thread_msgq_data[10];

	enum eth_xlnx_link_speed		eff_link_speed;

	u8_t							phy_addr;
	u32_t							phy_id;
	struct k_timer                  phy_poll_timer;

	enum eth_xlnx_mdc_clock_divisor	mdc_divisor;

	u8_t							*first_rx_buffer;
	u8_t							*first_tx_buffer;

	struct eth_xlnx_gem_bdring		rxbd_ring;
	struct eth_xlnx_gem_bdring		txbd_ring;
};

/* Forward declarations */

static int  eth_xlnx_gem_dev_init(struct device *dev);
static void eth_xlnx_gem_iface_init(struct net_if *iface);
static enum ethernet_hw_caps eth_xlnx_gem_get_capabilities(struct device *dev);

static void eth_xlnx_gem_irq_config(struct device *dev);
static void eth_xlnx_gem_isr(void *arg);

static int  eth_xlnx_gem_start_device(struct device *dev);
static int  eth_xlnx_gem_stop_device(struct device *dev);

static int  eth_xlnx_gem_send(struct device *dev, struct net_pkt *pkt);

static void eth_xlnx_gem_amba_clk_enable(struct device *dev);
static void eth_xlnx_gem_reset_hw(struct device *dev);
static void eth_xlnx_gem_configure_clocks(struct device *dev);
static void eth_xlnx_gem_set_initial_nwcfg(struct device *dev);
static void eth_xlnx_gem_set_mac_address(struct device *dev);
static void eth_xlnx_gem_set_initial_dmacr(struct device *dev);
static void eth_xlnx_gem_init_phy(struct device *dev);
static void eth_xlnx_gem_configure_buffers(struct device *dev);

#if defined(CONFIG_NET_STATISTICS_ETHERNET)
static struct net_stats_eth *eth_xlnx_gem_stats(struct device *dev);
#endif

/* PHY access functions */

static void eth_xlnx_gem_phy_detect(struct device *dev);
static void eth_xlnx_gem_phy_reset(struct device *dev);
static void eth_xlnx_gem_phy_configure(struct device *dev);

static u16_t eth_xlnx_gem_phy_poll_int_status(struct device *dev);
static u8_t  eth_xlnx_gem_phy_poll_link_status(struct device *dev);
static enum eth_xlnx_link_speed eth_xlnx_gem_phy_poll_link_speed(struct device *dev);

static int  eth_xlnx_gem_mdio_read(u32_t base_addr, u8_t phy_addr, u8_t reg_addr);
static void eth_xlnx_gem_mdio_write(u32_t base_addr, u8_t phy_addr, u8_t reg_addr, u16_t value);

#endif /* _ZEPHYR_DRIVERS_ETHERNET_ETH_XLNX_GEM_PRIV_H_ */
