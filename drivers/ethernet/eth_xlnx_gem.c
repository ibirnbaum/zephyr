/* 
 * Xilinx Processor System Ethernet controller (GEM)
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>

#include <net/net_pkt.h>
#include <net/net_if.h>
#include <net/ethernet.h>
#include <ethernet/eth_stats.h>

#include "eth_xlnx_emacps_priv.h"

LOG_MODULE_REGISTER(eth_xlnx_emacps, CONFIG_ETHERNET_LOG_LEVEL);

#if defined(CONFIG_ETH_XLNX_EMACPS_PORT_0) \
	&& !defined(DT_INST_0_XLNX_EMACPS)
#error Cannot compile Xilinx EMACPS driver for GEM0: device tree configuration data is unavailable!
#endif

#if defined(CONFIG_ETH_XLNX_EMACPS_PORT_1) \
	&& !defined(DT_INST_1_XLNX_EMACPS)
#error Cannot compile Xilinx EMACPS driver for GEM1: device tree configuration data is unavailable!
#endif

/* 
 * Declare the DMA areas for one or both controllers locally if no fixed address (e.g. OCM)
 * was provided for the respective GEM controller.
 * WATCH OUT: No measures of any kind are taken in order to ensure that the data structures
 * declared below are located in non-cached, non-buffered memory!
 */

#if defined(DT_INST_0_XLNX_EMACPS) \
	&& defined(CONFIG_ETH_XLNX_EMACPS_PORT_0) \
	&& !defined(CONFIG_ETH_XLNX_EMACPS_PORT_0_DMA_FIXED)
struct eth_xlnx_dma_area_gem0 dma_area_gem0;
#endif

#if defined(DT_INST_1_XLNX_EMACPS) \
	&& defined(CONFIG_ETH_XLNX_EMACPS_PORT_1) \
	&& !defined(CONFIG_ETH_XLNX_EMACPS_PORT_1_DMA_FIXED)
struct eth_xlnx_dma_area_gem1 dma_area_gem1;
#endif

/* EMACPS Driver API declaration */

static const struct ethernet_api eth_xlnx_emacps_apis = {
	.iface_api.init		= eth_xlnx_emacps_iface_init,
	.get_capabilities	= eth_xlnx_emacps_get_capabilities,
	.send				= eth_xlnx_emacps_send,
	.start				= eth_xlnx_emacps_start_device,
	.stop				= eth_xlnx_emacps_stop_device,
	#if defined(CONFIG_NET_STATISTICS_ETHERNET)
	.get_stats			= eth_xlnx_emacps_stats,
	#endif
};

#if defined(DT_INST_0_XLNX_EMACPS) && defined(CONFIG_ETH_XLNX_EMACPS_PORT_0)

/* Driver instance declaration for GEM0, if active */

static struct eth_xlnx_emacps_dev_cfg eth_xlnx_emacps_gem0_dev_cfg = {
	/* Controller base address -> from device tree data */
	.base_addr				= DT_INST_0_XLNX_EMACPS_BASE_ADDRESS,

	/* IRQ configuration function pointer */
	.config_func    		= eth_xlnx_emacps_irq_config,

	/* Link speed & PHY init related parameters -> from autoconf */

	#if   (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_LINK_10MBIT)
	.max_link_speed			= LINK_10MBIT,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_LINK_100MBIT)
	.max_link_speed			= LINK_100MBIT,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_LINK_1GBIT)
	.max_link_speed			= LINK_1GBIT,
	#else
	#error No valid link speed setting found in GEM0 configuration data
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_INIT_PHY
	.init_phy 				= 1,
	#else
	.init_phy 				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_PHY_ADVERTISE_LOWER
	.phy_advertise_lower	= 1,
	#else
	.phy_advertise_lower	= 0,
	#endif

	/* AMBA AHB data bus width setting -> from autoconf */

	#if (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_AMBAAHB_32BIT)
	.amba_dbus_width 		= AMBA_AHB_DBUS_WIDTH_32BIT,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_AMBAAHB_64BIT)
	.amba_dbus_width 		= AMBA_AHB_DBUS_WIDTH_64BIT,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_AMBAAHB_128BIT)
	.amba_dbus_width 		= AMBA_AHB_DBUS_WIDTH_128BIT,
	#else
	#error No valid AMBA AHB data bus width setting found in GEM0 configuration data
	#endif

	/* AMBA AHB burst length -> from autoconf */

	#if   (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_AHBBURST_SINGLE)
	.ahb_burst_length  		= AHB_BURST_SINGLE,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_AHBBURST_INCR4)
	.ahb_burst_length  		= AHB_BURST_INCR4,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_AHBBURST_INCR8)
	.ahb_burst_length  		= AHB_BURST_INCR8,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_AHBBURST_INCR16)
	.ahb_burst_length  		= AHB_BURST_INCR16,
	#else
	#error No valid AMBA AHB burst length setting found in GEM0 configuration data
	#endif

	/* Hardware RX buffer size -> from autoconf */

	#if   (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_HWRX_BUFFER_SIZE_FULL)
	.hw_rx_buffer_size		= HWRX_BUFFER_SIZE_8KB,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_HWRX_BUFFER_SIZE_4KB)
	.hw_rx_buffer_size		= HWRX_BUFFER_SIZE_4KB,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_HWRX_BUFFER_SIZE_2KB)
	.hw_rx_buffer_size		= HWRX_BUFFER_SIZE_2KB,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_0_HWRX_BUFFER_SIZE_1KB)
	.hw_rx_buffer_size		= HWRX_BUFFER_SIZE_1KB,
	#else
	#error No valid Hardware RX buffer size setting found in GEM0 configuration data
	#endif

	/* RX buffer offset -> from autoconf */

	.hw_rx_buffer_offset	= CONFIG_ETH_XLNX_EMACPS_PORT_0_HWRX_BUFFER_OFFSET,

	/* AHB RX buffer size, n * 64 bytes */

	.ahb_rx_buffer_size     = CONFIG_ETH_XLNX_EMACPS_PORT_0_AHB_RX_BUFFER_SIZE,

	/* AMBA Clock enable bit of the respective GEM in the SLCR */

	.amba_clk_en_bit		= ETH_XLNX_EMACPS_AMBA_CLK_ENABLE_BIT_GEM0,

	/* Clock reference PLL and the PLL's reference clock multiplier -> from autoconf */

	#if (defined CONFIG_ZYNQ_ENET0_REFCLK_IOPLL)
	.reference_pll					= IO_PLL,
	.reference_pll_ref_clk_multi	= CONFIG_ZYNQ_IOPLL_MULTIPLIER,
	#elif (defined CONFIG_ZYNQ_ENET0_REFCLK_ARMPLL)
	.reference_pll					= ARM_PLL,
	.reference_pll_ref_clk_multi	= CONFIG_ZYNQ_ARMPLL_MULTIPLIER,
	#elif (defined CONFIG_ZYNQ_ENET0_REFCLK_DDRPLL)
	.reference_pll					= DDR_PLL,
	.reference_pll_ref_clk_multi	= CONFIG_ZYNQ_DDRPLL_MULTIPLIER,
	#elif (defined CONFIG_ZYNQ_ENET0_REFCLK_EMIOCLK)
	.reference_pll					= EMIO_CLK,
	.reference_pll_ref_clk_multi	= 1, /* TODO check this */
	#else
	#error No RX clock reference PLL setting found in GEM0 configuration data
	#endif

	/* GEM clock source -> from autoconf */

	#if (defined CONFIG_ZYNQ_ENET0_SRCSEL_MIO)
	.gem_clk_source					= CLK_SRC_MIO,
	#elif (defined CONFIG_ZYNQ_ENET0_SRCSEL_EMIO)
	.gem_clk_source					= CLK_SRC_EMIO,
	#else
	#error No GEM clock source setting found in GEM0 configuration data
	#endif

	/* GEM clock divisors -> from autoconf */

	.gem_clk_divisor1				= CONFIG_ZYNQ_ENET0_DIVISOR1,
	.gem_clk_divisor0				= CONFIG_ZYNQ_ENET0_DIVISOR0,

	/* The respective GEM's SLCR clock configuration register addresses */

	.slcr_clk_register_addr			= ETH_XLNX_SLCR_GEM0_CLK_CTRL_REGISTER,
	.slcr_rclk_register_addr		= ETH_XLNX_SLCR_GEM0_RCLK_CTRL_REGISTER,

	/* DMA area receive / transmit buffer (descriptor) related data */

	.rxbd_count						= CONFIG_ETH_XLNX_EMACPS_PORT_0_RXBD_COUNT,
	.txbd_count						= CONFIG_ETH_XLNX_EMACPS_PORT_0_TXBD_COUNT,
	.rx_buffer_size					= ((CONFIG_ETH_XLNX_EMACPS_PORT_0_RX_BUFFER_SIZE
										+ (ETH_XLNX_BUFFER_ALIGNMENT - 1)) & ~(ETH_XLNX_BUFFER_ALIGNMENT - 1)),
	.tx_buffer_size					= ((CONFIG_ETH_XLNX_EMACPS_PORT_0_TX_BUFFER_SIZE
										+ (ETH_XLNX_BUFFER_ALIGNMENT - 1)) & ~(ETH_XLNX_BUFFER_ALIGNMENT - 1)),

	/* Feature flags, mostly targeting the gem.net_cfg register -> from autoconf */

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_IGNORE_IGP_RXER
	.ignore_igp_rxer				= 1,
	#else
	.ignore_igp_rxer				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_DISABLE_REJECT_NSP
	.disable_reject_nsp				= 1,
	#else
	.disable_reject_nsp				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_IGP_STRETCH
	.enable_igp_stretch				= 1,
	#else
	.enable_igp_stretch				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_SGMII_MODE
	.enable_sgmii_mode				= 1,
	#else
	.enable_sgmii_mode				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_DISABLE_REJECT_FCS_CRC_ERRORS
	.disable_reject_fcs_crc_errors	= 1,
	#else
	.disable_reject_fcs_crc_errors	= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_RX_HALFDUP_WHILE_TX
	.enable_rx_halfdup_while_tx		= 1,
	#else
	.enable_rx_halfdup_while_tx		= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_RX_CHKSUM_OFFLOAD
	.enable_rx_chksum_offload		= 1,
	#else
	.enable_rx_chksum_offload		= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_DISABLE_PAUSE_COPY
	.disable_pause_copy				= 1,
	#else
	.disable_pause_copy				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_DISCARD_RX_FCS
	.discard_rx_fcs					= 1,
	#else
	.discard_rx_fcs					= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_DISCARD_RX_LENGTH_ERRORS
	.discard_rx_length_errors		= 1,
	#else
	.discard_rx_length_errors		= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_PAUSE
	.enable_pause					= 1,
	#else
	.enable_pause					= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_TBI
	.enable_tbi						= 1,
	#else
	.enable_tbi						= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_EXT_ADDR_MATCH
	.ext_addr_match					= 1,
	#else
	.ext_addr_match					= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_1536_FRAMES
	.enable_1536_frames				= 1,
	#else
	.enable_1536_frames				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_UCAST_HASH
	.enable_ucast_hash				= 1,
	#else
	.enable_ucast_hash				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_MCAST_HASH
	.enable_mcast_hash				= 1,
	#else
	.enable_mcast_hash				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_DISABLE_BCAST
	.disable_bcast					= 1,
	#else
	.disable_bcast					= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_COPY_ALL_FRAMES
	.copy_all_frames				= 1,
	#else
	.copy_all_frames				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_DISCARD_NON_VLAN
	.discard_non_vlan				= 1,
	#else
	.discard_non_vlan				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_FDX
	.enable_fdx						= 1,
	#else
	.enable_fdx						= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_DISC_RX_AHB_UNAVAIL
	.disc_rx_ahb_unavail			= 1,
	#else
	.disc_rx_ahb_unavail			= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_TX_CHKSUM_OFFLOAD
	.enable_tx_chksum_offload		= 1,
	#else
	.enable_tx_chksum_offload		= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_HWTX_BUFFER_SIZE_FULL
	.tx_buffer_size_full			= 1,
	#else
	.tx_buffer_size_full			= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_AHB_PACKET_ENDIAN_SWAP
	.enable_ahb_packet_endian_swap	= 1,
	#else
	.enable_ahb_packet_endian_swap	= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_ENABLE_AHB_MD_ENDIAN_SWAP
	.enable_ahb_md_endian_swap		= 1
	#else
	.enable_ahb_md_endian_swap		= 0
	#endif
};

static struct eth_xlnx_emacps_dev_data eth_xlnx_emacps_gem0_dev_data = {
	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_MAC_DEVTREE
	.mac_addr = DT_XLNX_EMACPS_ETH0_LOCAL_MAC_ADDRESS,
	#else
	.mac_addr = {
		CONFIG_ETH_XLNX_EMACPS_PORT_0_MAC_BYTE_5,
		CONFIG_ETH_XLNX_EMACPS_PORT_0_MAC_BYTE_4,
		CONFIG_ETH_XLNX_EMACPS_PORT_0_MAC_BYTE_3,
		CONFIG_ETH_XLNX_EMACPS_PORT_0_MAC_BYTE_2,
		CONFIG_ETH_XLNX_EMACPS_PORT_0_MAC_BYTE_1,
		CONFIG_ETH_XLNX_EMACPS_PORT_0_MAC_BYTE_0},
	#endif

	.aux_thread_prio	= CONFIG_ETH_XLNX_EMACPS_PORT_0_AUX_THREAD_PRIO,
	.eff_link_speed		= LINK_DOWN,
	.phy_addr       	= 0,
	.phy_id         	= 0,
	.mdc_divisor    	= MDC_DIVISOR_224,
	.first_rx_buffer	= NULL,
	.first_tx_buffer	= NULL
};

K_THREAD_STACK_DEFINE(eth_xlnx_emacps_aux_thread_stack_gem0,
		CONFIG_ETH_XLNX_EMACPS_PORT_0_AUX_THREAD_STACK_SIZE);

ETH_NET_DEVICE_INIT(eth_xlnx_emacps_gem0, DT_XLNX_EMACPS_ETH0_LABEL,
		eth_xlnx_emacps_dev_init, &eth_xlnx_emacps_gem0_dev_data, &eth_xlnx_emacps_gem0_dev_cfg,
		CONFIG_ETH_INIT_PRIORITY,
		&eth_xlnx_emacps_apis, NET_ETH_MTU);

#endif

#if defined(DT_INST_1_XLNX_EMACPS) && defined(CONFIG_ETH_XLNX_EMACPS_PORT_1)

/* Driver instance declaration for GEM0, if active */

static struct eth_xlnx_emacps_dev_cfg eth_xlnx_emacps_gem1_dev_cfg = {
	/* Controller base address -> from device tree data */
	.base_addr				= DT_INST_1_XLNX_EMACPS_BASE_ADDRESS,

	/* IRQ configuration function pointer */
	.config_func    		= eth_xlnx_emacps_irq_config,

	/* Link speed & PHY init related parameters -> from autoconf */

	#if   (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_LINK_10MBIT)
	.max_link_speed			= LINK_10MBIT,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_LINK_100MBIT)
	.max_link_speed			= LINK_100MBIT,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_LINK_1GBIT)
	.max_link_speed			= LINK_1GBIT,
	#else
	#error No valid link speed setting found in GEM1 configuration data
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_INIT_PHY
	.init_phy 				= 1,
	#else
	.init_phy 				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_PHY_ADVERTISE_LOWER
	.phy_advertise_lower	= 1,
	#else
	.phy_advertise_lower	= 0,
	#endif

	/* AMBA AHB data bus width setting -> from autoconf */

	#if (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_AMBAAHB_32BIT)
	.amba_dbus_width 		= AMBA_AHB_DBUS_WIDTH_32BIT,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_AMBAAHB_64BIT)
	.amba_dbus_width 		= AMBA_AHB_DBUS_WIDTH_64BIT,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_AMBAAHB_128BIT)
	.amba_dbus_width 		= AMBA_AHB_DBUS_WIDTH_128BIT,
	#else
	#error No valid AMBA AHB data bus width setting found in GEM1 configuration data
	#endif

	/* AMBA AHB burst length -> from autoconf */

	#if   (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_AHBBURST_SINGLE)
	.ahb_burst_length  		= AHB_BURST_SINGLE,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_AHBBURST_INCR4)
	.ahb_burst_length  		= AHB_BURST_INCR4,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_AHBBURST_INCR8)
	.ahb_burst_length  		= AHB_BURST_INCR8,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_AHBBURST_INCR16)
	.ahb_burst_length  		= AHB_BURST_INCR16,
	#else
	#error No valid AMBA AHB burst length setting found in GEM1 configuration data
	#endif

	/* Hardware RX buffer size -> from autoconf */

	#if   (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_HWRX_BUFFER_SIZE_FULL)
	.hw_rx_buffer_size		= HWRX_BUFFER_SIZE_8KB,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_HWRX_BUFFER_SIZE_4KB)
	.hw_rx_buffer_size		= HWRX_BUFFER_SIZE_4KB,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_HWRX_BUFFER_SIZE_2KB)
	.hw_rx_buffer_size		= HWRX_BUFFER_SIZE_2KB,
	#elif (defined CONFIG_ETH_XLNX_EMACPS_PORT_1_HWRX_BUFFER_SIZE_1KB)
	.hw_rx_buffer_size		= HWRX_BUFFER_SIZE_1KB,
	#else
	#error No valid Hardware RX buffer size setting found in GEM1 configuration data
	#endif

	/* RX buffer offset -> from autoconf */

	.hw_rx_buffer_offset	= CONFIG_ETH_XLNX_EMACPS_PORT_1_HWRX_BUFFER_OFFSET,

	/* AHB RX buffer size, n * 64 bytes */

	.ahb_rx_buffer_size     = CONFIG_ETH_XLNX_EMACPS_PORT_1_AHB_RX_BUFFER_SIZE,

	/* AMBA Clock enable bit of the respective GEM in the SLCR */

	.amba_clk_en_bit		= ETH_XLNX_EMACPS_AMBA_CLK_ENABLE_BIT_GEM1,

	/* Clock reference PLL and the PLL's reference clock multiplier -> from autoconf */

	#if (defined CONFIG_ZYNQ_ENET1_REFCLK_IOPLL)
	.reference_pll					= IO_PLL,
	.reference_pll_ref_clk_multi	= CONFIG_ZYNQ_IOPLL_MULTIPLIER,
	#elif (defined CONFIG_ZYNQ_ENET1_REFCLK_ARMPLL)
	.reference_pll					= ARM_PLL,
	.reference_pll_ref_clk_multi	= CONFIG_ZYNQ_ARMPLL_MULTIPLIER,
	#elif (defined CONFIG_ZYNQ_ENET1_REFCLK_DDRPLL)
	.reference_pll					= DDR_PLL,
	.reference_pll_ref_clk_multi	= CONFIG_ZYNQ_DDRPLL_MULTIPLIER,
	#elif (defined CONFIG_ZYNQ_ENET1_REFCLK_EMIOCLK)
	.reference_pll					= EMIO_CLK,
	.reference_pll_ref_clk_multi	= 1, /* TODO check this */
	#else
	#error No RX clock reference PLL setting found in GEM1 configuration data
	#endif

	/* GEM clock source -> from autoconf */

	#if (defined CONFIG_ZYNQ_ENET1_SRCSEL_MIO)
	.gem_clk_source					= CLK_SRC_MIO,
	#elif (defined CONFIG_ZYNQ_ENET1_SRCSEL_EMIO)
	.gem_clk_source					= CLK_SRC_EMIO,
	#else
	#error No GEM clock source setting found in GEM1 configuration data
	#endif

	/* GEM clock divisors -> from autoconf */

	.gem_clk_divisor1				= CONFIG_ZYNQ_ENET1_DIVISOR1,
	.gem_clk_divisor0				= CONFIG_ZYNQ_ENET1_DIVISOR0,

	/* The respective GEM's SLCR clock configuration register addresses */

	.slcr_clk_register_addr			= ETH_XLNX_SLCR_GEM1_CLK_CTRL_REGISTER,
	.slcr_rclk_register_addr		= ETH_XLNX_SLCR_GEM1_RCLK_CTRL_REGISTER,

	/* DMA area receive / transmit buffer (descriptor) related data */

	.rxbd_count						= CONFIG_ETH_XLNX_EMACPS_PORT_1_RXBD_COUNT,
	.txbd_count						= CONFIG_ETH_XLNX_EMACPS_PORT_1_TXBD_COUNT,
	.rx_buffer_size					= ((CONFIG_ETH_XLNX_EMACPS_PORT_1_RX_BUFFER_SIZE
										+ (ETH_XLNX_BUFFER_ALIGNMENT - 1)) & ~(ETH_XLNX_BUFFER_ALIGNMENT - 1)),
	.tx_buffer_size					= ((CONFIG_ETH_XLNX_EMACPS_PORT_1_TX_BUFFER_SIZE
										+ (ETH_XLNX_BUFFER_ALIGNMENT - 1)) & ~(ETH_XLNX_BUFFER_ALIGNMENT - 1)),

	/* Feature flags, mostly targeting the gem.net_cfg register -> from autoconf */

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_IGNORE_IGP_RXER
	.ignore_igp_rxer				= 1,
	#else
	.ignore_igp_rxer				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_DISABLE_REJECT_NSP
	.disable_reject_nsp				= 1,
	#else
	.disable_reject_nsp				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_IGP_STRETCH
	.enable_igp_stretch				= 1,
	#else
	.enable_igp_stretch				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_SGMII_MODE
	.enable_sgmii_mode				= 1,
	#else
	.enable_sgmii_mode				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_DISABLE_REJECT_FCS_CRC_ERRORS
	.disable_reject_fcs_crc_errors	= 1,
	#else
	.disable_reject_fcs_crc_errors	= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_RX_HALFDUP_WHILE_TX
	.enable_rx_halfdup_while_tx		= 1,
	#else
	.enable_rx_halfdup_while_tx		= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_RX_CHKSUM_OFFLOAD
	.enable_rx_chksum_offload		= 1,
	#else
	.enable_rx_chksum_offload		= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_DISABLE_PAUSE_COPY
	.disable_pause_copy				= 1,
	#else
	.disable_pause_copy				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_DISCARD_RX_FCS
	.discard_rx_fcs					= 1,
	#else
	.discard_rx_fcs					= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_DISCARD_RX_LENGTH_ERRORS
	.discard_rx_length_errors		= 1,
	#else
	.discard_rx_length_errors		= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_PAUSE
	.enable_pause					= 1,
	#else
	.enable_pause					= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_TBI
	.enable_tbi						= 1,
	#else
	.enable_tbi						= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_EXT_ADDR_MATCH
	.ext_addr_match					= 1,
	#else
	.ext_addr_match					= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_1536_FRAMES
	.enable_1536_frames				= 1,
	#else
	.enable_1536_frames				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_UCAST_HASH
	.enable_ucast_hash				= 1,
	#else
	.enable_ucast_hash				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_MCAST_HASH
	.enable_mcast_hash				= 1,
	#else
	.enable_mcast_hash				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_DISABLE_BCAST
	.disable_bcast					= 1,
	#else
	.disable_bcast					= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_COPY_ALL_FRAMES
	.copy_all_frames				= 1,
	#else
	.copy_all_frames				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_DISCARD_NON_VLAN
	.discard_non_vlan				= 1,
	#else
	.discard_non_vlan				= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_FDX
	.enable_fdx						= 1,
	#else
	.enable_fdx						= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_DISC_RX_AHB_UNAVAIL
	.disc_rx_ahb_unavail			= 1,
	#else
	.disc_rx_ahb_unavail			= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_TX_CHKSUM_OFFLOAD
	.enable_tx_chksum_offload		= 1,
	#else
	.enable_tx_chksum_offload		= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_HWTX_BUFFER_SIZE_FULL
	.tx_buffer_size_full			= 1,
	#else
	.tx_buffer_size_full			= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_AHB_PACKET_ENDIAN_SWAP
	.enable_ahb_packet_endian_swap	= 1,
	#else
	.enable_ahb_packet_endian_swap	= 0,
	#endif

	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_ENABLE_AHB_MD_ENDIAN_SWAP
	.enable_ahb_md_endian_swap		= 1
	#else
	.enable_ahb_md_endian_swap		= 0
	#endif
};

static struct eth_xlnx_emacps_dev_data eth_xlnx_emacps_gem1_dev_data = {
	#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_MAC_DEVTREE
	.mac_addr = DT_XLNX_EMACPS_ETH1_LOCAL_MAC_ADDRESS,
	#else
	.mac_addr = {
		CONFIG_ETH_XLNX_EMACPS_PORT_1_MAC_BYTE_5,
		CONFIG_ETH_XLNX_EMACPS_PORT_1_MAC_BYTE_4,
		CONFIG_ETH_XLNX_EMACPS_PORT_1_MAC_BYTE_3,
		CONFIG_ETH_XLNX_EMACPS_PORT_1_MAC_BYTE_2,
		CONFIG_ETH_XLNX_EMACPS_PORT_1_MAC_BYTE_1,
		CONFIG_ETH_XLNX_EMACPS_PORT_1_MAC_BYTE_0},
	#endif

	.aux_thread_prio	= CONFIG_ETH_XLNX_EMACPS_PORT_1_AUX_THREAD_PRIO,
	.eff_link_speed		= LINK_DOWN,
	.phy_addr       	= 0,
	.phy_id         	= 0,
	.mdc_divisor    	= MDC_DIVISOR_224,
	.first_rx_buffer	= NULL,
	.first_tx_buffer	= NULL
};

K_THREAD_STACK_DEFINE(eth_xlnx_emacps_aux_thread_stack_gem1,
		CONFIG_ETH_XLNX_EMACPS_PORT_1_AUX_THREAD_STACK_SIZE);

ETH_NET_DEVICE_INIT(eth_xlnx_emacps_gem1, DT_XLNX_EMACPS_ETH1_LABEL,
		eth_xlnx_emacps_dev_init, &eth_xlnx_emacps_gem1_dev_data, &eth_xlnx_emacps_gem1_dev_cfg,
		CONFIG_ETH_INIT_PRIORITY,
		&eth_xlnx_emacps_apis, NET_ETH_MTU);

#endif

static void eth_xlnx_emacps_aux_timer (struct k_timer *timer_id) {
	struct net_if                   *iface            = (struct net_if*)timer_id->user_data;
	struct device 					*dev              = net_if_get_device(iface);
	struct eth_xlnx_emacps_dev_data	*dev_data         = DEV_DATA(dev);

	u8_t                            aux_thread_notify = ETH_XLNX_EMACPS_AUX_THREAD_POLL_PHY_BIT;

	k_msgq_put(&dev_data->aux_thread_msgq, &aux_thread_notify, K_NO_WAIT);
}

static void eth_xlnx_emacps_aux_thread (void *p1, void *p2, void *p3) {
	struct device 					*dev              = (struct device*)p1;
	struct net_if 					*iface            = (struct net_if*)p2;
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf         = DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data	*dev_data         = DEV_DATA(dev);

	u8_t                            aux_thread_notify = 0x00;

	u32_t							reg_val           = 0;
	u32_t							reg_val_txsr      = 0;
	u32_t							reg_val_rxsr      = 0;

	u8_t							first_bd_idx      = 0;
	u8_t							last_bd_idx       = 0;
	u8_t							curr_bd_idx       = 0;
	u8_t							bds_processed     = 0;
	u8_t							bd_is_last        = 0;
	u16_t							rx_data_length    = 0;

	struct net_buf 					*pkt_buf          = NULL;
	struct net_pkt 					*pkt              = NULL;
	u16_t							frag_len          = 0;
	u16_t							eff_copy_len      = 0;
	u32_t							src_buffer_offs   = 0;
	u8_t							*data_dest        = NULL;

	u16_t							phy_status        = 0x0000;
	u8_t							link_status       = 0x00;

	(void)p3;

	while(1) {
		k_msgq_get(&dev_data->aux_thread_msgq, &aux_thread_notify, K_FOREVER);

		//printk("Aux thread for GEM @ 0x%08X: notify 0x%02X\n", dev_conf->base_addr, aux_thread_notify);

		if ((aux_thread_notify & ETH_XLNX_EMACPS_AUX_THREAD_POLL_PHY_BIT) != 0) {
			phy_status = eth_xlnx_emacps_phy_poll_int_status(dev);
			if ((phy_status & (
				  PHY_COPPER_SPEED_CHANGED_INTERRUPT_BIT
				| PHY_COPPER_AUTONEG_COMPLETED_INTERRUPT_BIT
				| PHY_COPPER_LINK_STATUS_CHANGED_INTERRUPT_BIT)) != 0) {

				/* Read the PHY's link status. Handling a 'link down' event is the easiest case. */

				link_status = eth_xlnx_emacps_phy_poll_link_status(dev);

				if (link_status == 0) {
					dev_data->eff_link_speed = LINK_DOWN;
					net_eth_carrier_off(iface);
					printk("EMACPS @ 0x%08X: link down\n", dev_conf->base_addr);
				} else {
					dev_data->eff_link_speed = eth_xlnx_emacps_phy_poll_link_speed(dev);
					net_eth_carrier_on(iface);
					printk(
						"EMACPS @ 0x%08X: link up, speed %s\n",
						dev_conf->base_addr,
						(dev_data->eff_link_speed == LINK_1GBIT)   ? "1 GBit/s"   :
						(dev_data->eff_link_speed == LINK_100MBIT) ? "100 MBit/s" :
						(dev_data->eff_link_speed == LINK_10MBIT)  ? "10 MBit/s"  :
						"undefined / link down");
				}
			}
		}

		if ((aux_thread_notify & ETH_XLNX_EMACPS_AUX_THREAD_TXDONE_BIT) != 0) {
			/* Read the TX status register */

			reg_val_txsr = sys_read32(dev_conf->base_addr + ETH_XLNX_EMACPS_TXSR_OFFSET);

			/* TODO Evaluate error flags in TXSR here ... */

			curr_bd_idx = dev_data->txbd_ring.next_to_process;
			reg_val     = sys_read32((u32_t)(&dev_data->txbd_ring.first_bd[curr_bd_idx].ctrl));

			do {
				++bds_processed;

				/* TODO Evaluate error flags from current BD control word here */

				/* Check if the BD we're currently looking at is the last BD of the current transmission */

				bd_is_last = ((reg_val & ETH_XLNX_EMACPS_TXBD_LAST_BIT) != 0) ? 1 : 0;

				/* Reset control word of the current BD */

				reg_val &= ETH_XLNX_EMACPS_TXBD_WRAP_BIT;
				reg_val |= ETH_XLNX_EMACPS_TXBD_USED_BIT;
				sys_write32(reg_val, (u32_t)(&dev_data->txbd_ring.first_bd[curr_bd_idx].ctrl));

				/* Move on to the next BD or break out of the loop */

				if (bd_is_last == 1) {
					break;
				} else {
					curr_bd_idx = (curr_bd_idx + 1) % dev_conf->txbd_count;
					reg_val     = sys_read32((u32_t)(&dev_data->txbd_ring.first_bd[curr_bd_idx].ctrl));
				}
			} while (bd_is_last == 0);

			dev_data->txbd_ring.next_to_process  = (dev_data->txbd_ring.next_to_process + bds_processed) % dev_conf->txbd_count;
			dev_data->txbd_ring.free_bds        += bds_processed;

			/* Clear the TX status register */

			sys_write32(0xFFFFFFFF, dev_conf->base_addr + ETH_XLNX_EMACPS_TXSR_OFFSET);

			/* Indicate completion to a blocking eth_xlnx_emacps_send() call */

			k_sem_give(&dev_data->tx_done_sem);
		}

		if ((aux_thread_notify & ETH_XLNX_EMACPS_AUX_THREAD_RXDONE_BIT) != 0) {
			/* Read & clear the RX status register */

			reg_val_rxsr = sys_read32(dev_conf->base_addr + ETH_XLNX_EMACPS_RXSR_OFFSET);
			sys_write32(0xFFFFFFFF, dev_conf->base_addr + ETH_XLNX_EMACPS_RXSR_OFFSET);

			/* TODO Evaluate error flags in RXSR here ... */

			while (1) {
				curr_bd_idx  = dev_data->rxbd_ring.next_to_process;
				first_bd_idx = last_bd_idx = curr_bd_idx;

				reg_val = sys_read32((u32_t)(&dev_data->rxbd_ring.first_bd[first_bd_idx].addr));

				if ((reg_val & ETH_XLNX_EMACPS_RXBD_USED_BIT) == 0) {
					/* No new data contained in the current BD -> break out of the RX loop */
					break;
				}

				reg_val = sys_read32((u32_t)(&dev_data->rxbd_ring.first_bd[first_bd_idx].ctrl));
				if ((reg_val & ETH_XLNX_EMACPS_RXBD_START_OF_FRAME_BIT) == 0) {
					/* Although the current BD is marked as 'used', it doesn't contain the SOF bit */
					LOG_DBG("eth_xlnx_emacps_aux_thread: unexpected missing SOF bit in RX BD [%u]\n", first_bd_idx);
					break;
				}

				/* Extract data length from the current BD's control word. If the incoming frame spans
				 * multiple RX BDs, the length info from the subsequent BDs will be added to this value. */

				rx_data_length = (u16_t)(reg_val & ETH_XLNX_EMACPS_RXBD_FRAME_LENGTH_MASK);

				/* As long as the current BD doesn't have the EOF bit set, iterate forwards until
				 * the bit is encountered. */

				while ((reg_val & ETH_XLNX_EMACPS_RXBD_END_OF_FRAME_BIT) == 0) {
					last_bd_idx     = (last_bd_idx + 1) % dev_conf->rxbd_count;
					reg_val         = sys_read32((u32_t)(&dev_data->rxbd_ring.first_bd[last_bd_idx].ctrl));
					rx_data_length += (u16_t)(reg_val & ETH_XLNX_EMACPS_RXBD_FRAME_LENGTH_MASK);
				}

				/*
				 * We're processing all BDs belonging to the current frame, but we'll only pass
				 * the frame's data on to the network stack if a physical link is available.
				 */

				if (dev_data->eff_link_speed != LINK_DOWN) {

					/* Allocate a destination packet from the network stack now that the total
					 * frame length is known. */

					pkt = net_pkt_rx_alloc_with_buffer(
						dev_data->iface,
						rx_data_length,
						AF_UNSPEC,
						0,
						K_NO_WAIT);

					pkt_buf = pkt->buffer;

					/* Copy data from all involved RX buffers to network stack's packet buffer */

					do {
						reg_val			= sys_read32((u32_t)(&dev_data->rxbd_ring.first_bd[curr_bd_idx].ctrl));
						rx_data_length	= eff_copy_len = (u16_t)(reg_val & ETH_XLNX_EMACPS_RXBD_FRAME_LENGTH_MASK);
						src_buffer_offs = 0;

						while (rx_data_length > 0) {
							data_dest	= pkt_buf->data;
							frag_len	= net_buf_tailroom(pkt_buf);

							if (rx_data_length > frag_len)
							{
								eff_copy_len = frag_len;
							}

							memcpy(
								(void*)data_dest,
								(void*)(
									(dev_data->rxbd_ring.first_bd[curr_bd_idx].addr
										& ETH_XLNX_EMACPS_RXBD_BUFFER_ADDR_MASK)
									+ src_buffer_offs),
								eff_copy_len);

							net_buf_add(pkt_buf, eff_copy_len);

							rx_data_length	-= eff_copy_len;
							src_buffer_offs	+= eff_copy_len;
							eff_copy_len	 = rx_data_length;

							if (rx_data_length > 0) {
								pkt_buf = pkt_buf->frags;
							}
						}

						/* The entire packet data of the current BD has been processed, on to the next BD...
						 * -> preserve the RX BD's 'wrap' bit & address, but clear the 'used' bit. */

						reg_val	= sys_read32((u32_t)(&dev_data->rxbd_ring.first_bd[curr_bd_idx].addr));
						reg_val &= ~ETH_XLNX_EMACPS_RXBD_USED_BIT;
						sys_write32(reg_val, (u32_t)(&dev_data->rxbd_ring.first_bd[curr_bd_idx].addr));

						curr_bd_idx = (curr_bd_idx + 1) % dev_conf->rxbd_count;

					} while (curr_bd_idx != ((last_bd_idx + 1) % dev_conf->rxbd_count));

					/* Propagate the received packet to the network stack. */

					if (net_recv_data(dev_data->iface, pkt) < 0) {
						LOG_DBG("eth_xlnx_emacps_aux_thread: packet hand-over to IP stack failed\n");
						net_pkt_unref(pkt);
					}

				} else { /* eff_link_speed == LINK_DOWN */

					/* No data will be copied from the RX buffers as the physical link is down.
					 * -> Still, all of the involved RX BDs' 'used' bits must be cleared. */

					do {
						reg_val	= sys_read32((u32_t)(&dev_data->rxbd_ring.first_bd[curr_bd_idx].addr));
						reg_val &= ~ETH_XLNX_EMACPS_RXBD_USED_BIT;
						sys_write32(reg_val, (u32_t)(&dev_data->rxbd_ring.first_bd[curr_bd_idx].addr));

						curr_bd_idx = (curr_bd_idx + 1) % dev_conf->rxbd_count;
					} while (curr_bd_idx != ((last_bd_idx + 1) % dev_conf->rxbd_count));
				}

				/* Store the position of the first BD behind the end of the frame that has just
				 * been processed as 'next to process'. */

				dev_data->rxbd_ring.next_to_process = curr_bd_idx;
			}
		}
	}
}

static int eth_xlnx_emacps_dev_init (struct device *dev) {
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);

	u32_t	reg_val = 0;

	/* The PS7Init code generated by the Xilinx toolchain already configures the
	   relevant clocks, just in case that this initialization has not been performed,
	   set the clock configuration explicitly. All registers affected by this (re-)
	   configuration are located within the SLCR. */

	eth_xlnx_emacps_amba_clk_enable(dev);

	/* Initialization procedure as described in the Zynq-7000 TRM, chapter 16.3.x */

	eth_xlnx_emacps_reset_hw(dev);				/* Chapter 16.3.1 */
	eth_xlnx_emacps_set_initial_nwcfg(dev);		/* Chapter 16.3.2 */
	eth_xlnx_emacps_set_mac_address(dev);		/* Chapter 16.3.2 */
	eth_xlnx_emacps_set_initial_dmacr(dev);		/* Chapter 16.3.2 */

	/* Enable MDIO -> set gem.net_ctrl[mgmt_port_en] */

	reg_val = sys_read32(dev_conf->base_addr + ETH_XLNX_EMACPS_NWCTRL_OFFSET);
	reg_val |= ETH_XLNX_EMACPS_NWCTRL_MDEN_BIT;
	sys_write32(reg_val, dev_conf->base_addr + ETH_XLNX_EMACPS_NWCTRL_OFFSET);

	eth_xlnx_emacps_configure_clocks(dev);		/* Chapter 16.3.3 */
	if (dev_conf->init_phy == 1) {
		eth_xlnx_emacps_init_phy(dev);			/* Chapter 16.3.4 */
	}
	eth_xlnx_emacps_configure_buffers(dev);		/* Chapter 16.3.5 */

	return 0;
}

static void eth_xlnx_emacps_iface_init (struct net_if *iface) {
	struct device 					*dev      = net_if_get_device(iface);
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data = DEV_DATA(dev);

	dev_data->iface = iface;

	net_if_set_link_addr(iface, dev_data->mac_addr, 6, NET_LINK_ETHERNET);
	ethernet_init(iface);
	net_eth_carrier_off(iface);

	/* Initialize TX completion semaphore */

	k_sem_init(&dev_data->tx_done_sem, 0, 1);

	/* Initialize data in the RX/TX BD ring values which have not yet been initialized */

	dev_data->rxbd_ring.next_to_process = 0;
	dev_data->rxbd_ring.next_to_use     = 0;
	dev_data->rxbd_ring.free_bds        = dev_conf->rxbd_count;
	k_sem_init(&dev_data->rxbd_ring.ring_sem, 1, 1);

	dev_data->txbd_ring.next_to_process = 0;
	dev_data->txbd_ring.next_to_use     = 0;
	dev_data->txbd_ring.free_bds        = dev_conf->txbd_count;
	k_sem_init(&dev_data->txbd_ring.ring_sem, 1, 1);

	/* Initialize the mailbox for the auxiliary thread */

	k_msgq_init(
		&dev_data->aux_thread_msgq,
		dev_data->aux_thread_msgq_data,
		sizeof(u8_t),
		10);

	/* Initialize the timer for the auxiliary thread */

	k_timer_init(
		&dev_data->phy_poll_timer,
		eth_xlnx_emacps_aux_timer,
		NULL);
	dev_data->phy_poll_timer.user_data = (void*)iface;

	/* Initialize & start the auxiliary thread */

	#if defined(DT_INST_0_XLNX_EMACPS) && defined(CONFIG_ETH_XLNX_EMACPS_PORT_0)
	if (dev_conf->base_addr == DT_INST_0_XLNX_EMACPS_BASE_ADDRESS) {
		dev_data->aux_thread_tid =  k_thread_create(
			&dev_data->aux_thread_data,
			eth_xlnx_emacps_aux_thread_stack_gem0,
			K_THREAD_STACK_SIZEOF(eth_xlnx_emacps_aux_thread_stack_gem0),
			eth_xlnx_emacps_aux_thread,
			(void*)dev, (void*)iface, NULL,
			dev_data->aux_thread_prio,
			0,
			K_NO_WAIT);
	}
	#endif

	#if defined(DT_INST_1_XLNX_EMACPS) && defined(CONFIG_ETH_XLNX_EMACPS_PORT_1)
	if (dev_conf->base_addr == DT_INST_1_XLNX_EMACPS_BASE_ADDRESS) {
		dev_data->aux_thread_tid =  k_thread_create(
			&dev_data->aux_thread_data,
			eth_xlnx_emacps_aux_thread_stack_gem1,
			K_THREAD_STACK_SIZEOF(eth_xlnx_emacps_aux_thread_stack_gem1),
			eth_xlnx_emacps_aux_thread,
			(void*)dev, (void*)iface, NULL,
			dev_data->aux_thread_prio,
			0,
			K_NO_WAIT);
	}
	#endif

	/* Start the PHY polling timer */

	k_timer_start(&dev_data->phy_poll_timer, K_SECONDS(1), K_SECONDS(1));

	/* Initialize interrupts */

	dev_conf->config_func(dev); /* effectively: eth_xlnx_emacps_irq_config(dev) */
}

static void eth_xlnx_emacps_irq_config (struct device *dev) {
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);

	#if defined(DT_INST_0_XLNX_EMACPS) && defined(CONFIG_ETH_XLNX_EMACPS_PORT_0)
	if (dev_conf->base_addr == DT_INST_0_XLNX_EMACPS_BASE_ADDRESS) {
		IRQ_CONNECT(DT_INST_0_XLNX_EMACPS_IRQ_0,
			    DT_INST_0_XLNX_EMACPS_IRQ_0_PRIORITY,
			    eth_xlnx_emacps_isr, DEVICE_GET(eth_xlnx_emacps_gem0), 0);
		irq_enable(DT_INST_0_XLNX_EMACPS_IRQ_0);
	}
	#endif

	#if defined(DT_INST_1_XLNX_EMACPS) && defined(CONFIG_ETH_XLNX_EMACPS_PORT_1)
	if (dev_conf->base_addr == DT_INST_1_XLNX_EMACPS_BASE_ADDRESS) {
		IRQ_CONNECT(DT_INST_1_XLNX_EMACPS_IRQ_0,
			    DT_INST_1_XLNX_EMACPS_IRQ_0_PRIORITY,
			    eth_xlnx_emacps_isr, DEVICE_GET(eth_xlnx_emacps_gem1), 0);
		irq_enable(DT_INST_1_XLNX_EMACPS_IRQ_0);
	}
	#endif
}

static void eth_xlnx_emacps_isr (void *arg) {
	struct device 					*const dev        = (struct device *const)arg;
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf         = DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data         = DEV_DATA(dev);

	u32_t							reg_val_isr       = 0;

	u8_t							aux_thread_notify = 0x00;

    /* Read & clear interrupt status register */
    	
	reg_val_isr = sys_read32(dev_conf->base_addr + ETH_XLNX_EMACPS_ISR_OFFSET);

    /* TODO: handling if one or more error flag(s) are set in the interrupt status reg.
     * -> For now, just log them. */

	if (reg_val_isr & 0x00000C60) {
		LOG_DBG("eth_xlnx_emacps_isr: error(s) set in ISR reg: 0x%08X\n", reg_val_isr);
	}

	/* Dispatch auxiliary thread only if there is at least one of the following to handle:
	   reg_val & 0x00000080 -> gem.intr_status bit [7] = Frame TX complete
	   reg_val & 0x00000002 -> gem.intr_status bit [1] = Frame received
	   comp. Zynq-7000 TRM, Chapter B.18, p. 1289/1290 */

	if ((reg_val_isr & ETH_XLNX_EMACPS_IXR_TXCOMPL_BIT) != 0) {
		aux_thread_notify |= ETH_XLNX_EMACPS_AUX_THREAD_TXDONE_BIT;
	}

	if ((reg_val_isr & ETH_XLNX_EMACPS_IXR_FRAMERX_BIT) != 0) {
		aux_thread_notify |= ETH_XLNX_EMACPS_AUX_THREAD_RXDONE_BIT;
	}

	if (aux_thread_notify != 0x00) {
		k_msgq_put(&dev_data->aux_thread_msgq, &aux_thread_notify, K_NO_WAIT);
	}

	sys_write32(0xFFFFFFFF, dev_conf->base_addr + ETH_XLNX_EMACPS_ISR_OFFSET);
}

static int eth_xlnx_emacps_start_device (struct device *dev) {
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	u32_t							reg_val   = 0;

	/* TODO start may not be performed if link is down? -> compare to other drivers */

	/* Disable all the MAC interrupts */

	sys_write32(ETH_XLNX_EMACPS_IXR_ALL_MASK, dev_conf->base_addr + ETH_XLNX_EMACPS_IDR_OFFSET);

	/* RX and TX enable */

	reg_val  = sys_read32(dev_conf->base_addr + ETH_XLNX_EMACPS_NWCTRL_OFFSET);
	reg_val |= (ETH_XLNX_EMACPS_NWCTRL_RXEN_BIT | ETH_XLNX_EMACPS_NWCTRL_TXEN_BIT);
	sys_write32(reg_val, dev_conf->base_addr + ETH_XLNX_EMACPS_NWCTRL_OFFSET);

	/* Enable all the MAC interrupts */

	sys_write32(ETH_XLNX_EMACPS_IXR_ALL_MASK, dev_conf->base_addr + ETH_XLNX_EMACPS_IER_OFFSET);

	return 0;
}

static int eth_xlnx_emacps_stop_device (struct device *dev) {
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	u32_t							reg_val   = 0;

	/* RX and TX disable */

	reg_val  = sys_read32(dev_conf->base_addr + ETH_XLNX_EMACPS_NWCTRL_OFFSET);
	reg_val &= (~(ETH_XLNX_EMACPS_NWCTRL_RXEN_BIT | ETH_XLNX_EMACPS_NWCTRL_TXEN_BIT));
	sys_write32(reg_val, dev_conf->base_addr + ETH_XLNX_EMACPS_NWCTRL_OFFSET);

	/* Disable & clear all the MAC interrupts */

	sys_write32(ETH_XLNX_EMACPS_IXR_ALL_MASK, dev_conf->base_addr + ETH_XLNX_EMACPS_IDR_OFFSET);
	sys_write32(ETH_XLNX_EMACPS_IXR_ALL_MASK, dev_conf->base_addr + ETH_XLNX_EMACPS_ISR_OFFSET);

	return 0;
}

static int eth_xlnx_emacps_send (struct device *dev, struct net_pkt *pkt) {
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf 		= DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data 		= DEV_DATA(dev);
	struct net_buf 					*frag			= NULL;

	u16_t							pkt_len 		= 0;
	u16_t                           used_in_buf		= 0;
	u16_t							rem_frag_len	= 0;

	u8_t							bds_reqd		= 0;
	u8_t							curr_bd_idx		= 0;
	u8_t							*tx_buffer_offs = NULL;

	u32_t							reg_val			= 0;

	pkt_len = net_pkt_get_len(pkt);
	if (pkt_len == 0)
	{
		LOG_DBG("eth_xlnx_emacps_send: cannot TX, zero packet length\n");
		return -EINVAL;
	}

	if (dev_data->eff_link_speed == LINK_DOWN)
	{
		/* Won't write any packets to the TX buffers if the physical link is down */

		LOG_DBG("eth_xlnx_emacps_send: cannot TX, link down\n");
		return -EIO;
	}

	bds_reqd = (u8_t)((pkt_len + (dev_conf->tx_buffer_size - 1)) / dev_conf->tx_buffer_size);

	k_sem_take(&(dev_data->txbd_ring.ring_sem), K_FOREVER);

	/* Check if enough buffer descriptors are available for the amount of data to be TXed */

	if (bds_reqd > dev_data->txbd_ring.free_bds)
	{
		LOG_DBG(
			"eth_xlnx_emacps_send: cannot TX, packet length %hu requires %hhu BDs, only have %hhu free\n", 
			pkt_len, 
			bds_reqd, 
			dev_data->txbd_ring.free_bds);

		k_sem_give(&(dev_data->txbd_ring.ring_sem));
		return -EIO;
	}

	curr_bd_idx                      = dev_data->txbd_ring.next_to_use;
	dev_data->txbd_ring.next_to_use  = (curr_bd_idx + bds_reqd) % dev_conf->txbd_count;
	dev_data->txbd_ring.free_bds    -= bds_reqd;

	k_sem_give(&(dev_data->txbd_ring.ring_sem));

	/* Calculate the base pointer of the target TX buffer */

	tx_buffer_offs = (u8_t*)(dev_data->first_tx_buffer + (dev_conf->tx_buffer_size * curr_bd_idx));

	/* Copy packet data to the target TX data buffers, prepare BDs for TX */

	for (frag = pkt->frags; frag; frag = frag->frags) {
		rem_frag_len = frag->len;

		while (rem_frag_len > 0) {
			if ((used_in_buf + rem_frag_len) <= dev_conf->tx_buffer_size) {
				/* The current packet fragment fits into the current BD's buffer */

				memcpy(
					(void*)(&tx_buffer_offs[used_in_buf]),
					(void*)frag->data,
					rem_frag_len);

				used_in_buf  += rem_frag_len;
				rem_frag_len  = 0;
			} else {
				/* Only a part of the current packet fragment still fits into the current BD's buffer
				 * -> copy the first part, set up the BD control word, move on to the next BD */

				memcpy(
					(void*)(&tx_buffer_offs[used_in_buf]),
					(void*)frag->data,
					(dev_conf->tx_buffer_size - used_in_buf));

				rem_frag_len -= (dev_conf->tx_buffer_size - used_in_buf);

				/* Read the current BD's control word, set the length information, update BD */

				reg_val  = sys_read32((u32_t)(&dev_data->txbd_ring.first_bd[curr_bd_idx].ctrl));
				reg_val &= ETH_XLNX_EMACPS_TXBD_WRAP_BIT; /* Preserve the 'wrap' bit */
				reg_val |= ((reg_val & ~ETH_XLNX_EMACPS_TXBD_LEN_MASK) | dev_conf->tx_buffer_size);
				sys_write32(reg_val, (u32_t)(&dev_data->txbd_ring.first_bd[curr_bd_idx].ctrl));

				/* Move on to the next BD */

				curr_bd_idx    = (curr_bd_idx + 1) % dev_conf->txbd_count;
				tx_buffer_offs = (u8_t*)(dev_data->first_tx_buffer + (dev_conf->tx_buffer_size * curr_bd_idx));
				used_in_buf    = 0;
			}
		}
	}

	/* All fragments transferred to the buffers, configure the current (=last) BD */

	reg_val  = sys_read32((u32_t)(&dev_data->txbd_ring.first_bd[curr_bd_idx].ctrl));
	reg_val &= ETH_XLNX_EMACPS_TXBD_WRAP_BIT; /* Preserve the 'wrap' bit */
	reg_val |= (((reg_val & ~ETH_XLNX_EMACPS_TXBD_LEN_MASK) | used_in_buf) | ETH_XLNX_EMACPS_TXBD_LAST_BIT); /* Set the length + 'last' bit */
	sys_write32(reg_val, (u32_t)(&dev_data->txbd_ring.first_bd[curr_bd_idx].ctrl));

	/* Set the start TX bit in the gem.net_ctrl register */

	reg_val  = sys_read32(dev_conf->base_addr + ETH_XLNX_EMACPS_NWCTRL_OFFSET);
	reg_val |= ETH_XLNX_EMACPS_NWCTRL_STARTTX_BIT;
	sys_write32(reg_val, dev_conf->base_addr + ETH_XLNX_EMACPS_NWCTRL_OFFSET);

	/* Block until TX has completed */

	k_sem_take(&dev_data->tx_done_sem, K_FOREVER);

	return 0;
}

#if defined(CONFIG_NET_STATISTICS_ETHERNET)

static struct net_stats_eth *eth_xlnx_emacps_stats (struct device *dev) {
	return &(DEV_DATA(dev)->stats);
}

#endif

static enum ethernet_hw_caps eth_xlnx_emacps_get_capabilities (struct device *dev) {
	struct eth_xlnx_emacps_dev_cfg	*dev_conf	= DEV_CFG(dev);
	enum ethernet_hw_caps			caps 		= (enum ethernet_hw_caps)0;

	if (dev_conf->max_link_speed == LINK_1GBIT)	{
		if (dev_conf->phy_advertise_lower == 1) {
			caps |= (ETHERNET_LINK_1000BASE_T | ETHERNET_LINK_100BASE_T | ETHERNET_LINK_10BASE_T);
		} else {
			caps |= ETHERNET_LINK_1000BASE_T;
		}
	} else if (dev_conf->max_link_speed == LINK_100MBIT) {
		if (dev_conf->phy_advertise_lower == 1) {
			caps |= (ETHERNET_LINK_100BASE_T | ETHERNET_LINK_10BASE_T);
		} else {
			caps |= ETHERNET_LINK_100BASE_T;
		}
	} else {
		caps |= ETHERNET_LINK_10BASE_T;
	}

	if (dev_conf->enable_rx_chksum_offload == 1) {
		caps |= ETHERNET_HW_RX_CHKSUM_OFFLOAD;
	}

	if (dev_conf->enable_tx_chksum_offload == 1) {
		caps |= ETHERNET_HW_TX_CHKSUM_OFFLOAD;
	}

	if (dev_conf->enable_fdx == 1) {
		caps |= ETHERNET_DUPLEX_SET;
	}

	if (dev_conf->copy_all_frames == 1) {
		caps |= ETHERNET_PROMISC_MODE;
	}

	caps |= ETHERNET_HW_VLAN;
	return caps;
}

static void eth_xlnx_emacps_amba_clk_enable (struct device *dev)
{
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	u32_t				reg_val	  = 0;

	/* Enable the AMBA Peripheral Clock for the respective GEM */

	sys_write32(ETH_XLNX_SLCR_UNLOCK_CONSTANT, ETH_XLNX_SLCR_UNLOCK_REGISTER); /* SLCR unlock */

	reg_val  = sys_read32(ETH_XLNX_SLCR_APER_CLK_CTRL_REGISTER);
	reg_val |= (u32_t)dev_conf->amba_clk_en_bit;
	sys_write32(reg_val, ETH_XLNX_SLCR_APER_CLK_CTRL_REGISTER); /* Write the updated AMBA clk config */

	sys_write32(ETH_XLNX_SLCR_LOCK_CONSTANT, ETH_XLNX_SLCR_LOCK_REGISTER); /* SLCR lock */
}

static void eth_xlnx_emacps_reset_hw (struct device *dev)
{
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);

	/* Controller reset sequence as described in the Zynq-7000 TRM, chapter 16.3.1. */

	/* Clear the NWCTRL register */
	sys_write32(0x00000000, dev_conf->base_addr + ETH_XLNX_EMACPS_NWCTRL_OFFSET);

	/* Clear the statistics counters */
	sys_write32(ETH_XLNX_EMACPS_STATCLR_MASK, dev_conf->base_addr + ETH_XLNX_EMACPS_NWCTRL_OFFSET);

	/* Clear the RX/TX status registers */
	sys_write32(ETH_XLNX_EMACPS_TXSRCLR_MASK, dev_conf->base_addr + ETH_XLNX_EMACPS_TXSR_OFFSET);
	sys_write32(ETH_XLNX_EMACPS_RXSRCLR_MASK, dev_conf->base_addr + ETH_XLNX_EMACPS_RXSR_OFFSET);

	/* Disable all interrupts */
	sys_write32(ETH_XLNX_EMACPS_IDRCLR_MASK, dev_conf->base_addr + ETH_XLNX_EMACPS_IDR_OFFSET);

	/* Clear the buffer queues */
	sys_write32(0x00000000, dev_conf->base_addr + ETH_XLNX_EMACPS_RXQBASE_OFFSET);
	sys_write32(0x00000000, dev_conf->base_addr + ETH_XLNX_EMACPS_TXQBASE_OFFSET);
}

static void eth_xlnx_emacps_set_initial_nwcfg (struct device *dev)
{
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf	= DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data	= DEV_DATA(dev);
	enum eth_xlnx_mdc_clock_divisor	mdc_divisor	= MDC_DIVISOR_224;
	u32_t							cpu_1x_clk	= 0;
	u32_t							reg_val		= 0;

	/* Set the gem.net_cfg register contents for the respective GEM as defined 
	   using the menuconfig tool. The resulting defines are taken from autoconf.h.*/

	/* gem.net_cfg register bit (field) definitions: comp. Zynq-7000 TRM, p. 1274 ff. */

	/* MDC divisor depends on the CPU_1X clock frequency - calculation: comp. Zynq-7000 TRM chapter 25.3.
	 * Calculate the divisor regardless of which GEM is being initialized */

	cpu_1x_clk = ((CONFIG_ZYNQ_PS_REF_FREQUENCY * CONFIG_ZYNQ_ARMPLL_MULTIPLIER) / CONFIG_ZYNQ_ARMPLL_DIVISOR0);

	#if   (defined CONFIG_ZYNQ_CLOCK_RATIO_6321)
	cpu_1x_clk /= 6;
	#elif (defined CONFIG_ZYNQ_CLOCK_RATIO_4221)
	cpu_1x_clk /= 4;
	#else
	#error No clock divisor ratio setting found in ZYNQ configuration, cannot calculate MDC divider
	#endif

	if (cpu_1x_clk < 20000000)	{
		mdc_divisor = MDC_DIVISOR_8;
	} else if (cpu_1x_clk < 40000000) {
		mdc_divisor = MDC_DIVISOR_16;
	} else if (cpu_1x_clk < 80000000) {
		mdc_divisor = MDC_DIVISOR_32;
	} else if (cpu_1x_clk < 120000000) {
		mdc_divisor = MDC_DIVISOR_48;
	} else if (cpu_1x_clk < 160000000) {
		mdc_divisor = MDC_DIVISOR_64;
	} else if (cpu_1x_clk < 240000000) {
		mdc_divisor = MDC_DIVISOR_96;
	} else if (cpu_1x_clk < 320000000) {
		mdc_divisor = MDC_DIVISOR_128;
	} else {
		mdc_divisor = MDC_DIVISOR_224;
	}

	dev_data->mdc_divisor = mdc_divisor;

	if (dev_conf->ignore_igp_rxer == 1)	{
		reg_val |= ETH_XLNX_EMACPS_NWCFG_IGNIPGRXERR_BIT;                    /* [30]     ignore IPG rx_er */
	}

	if (dev_conf->disable_reject_nsp == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_BADPREAMBEN_BIT;                    /* [29]     disable rejection of non-standard preamble */
	}

	if (dev_conf->enable_igp_stretch == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_IPDSTRETCH_BIT;                     /* [28]     enable IPG stretch */
	}

	if (dev_conf->enable_sgmii_mode == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_SGMIIEN_BIT;                        /* [27]     SGMII mode enable */
	}

	if (dev_conf->disable_reject_fcs_crc_errors == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_FCSIGNORE_BIT;                      /* [26]     disable rejection of FCS/CRC errors */
	}

	if (dev_conf->enable_rx_halfdup_while_tx == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_HDRXEN_BIT;                         /* [25]     RX half duplex while TX enable */
	}

	if (dev_conf->enable_rx_chksum_offload == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_RXCHKSUMEN_BIT;                     /* [24]     enable RX IP/TCP/UDP checksum offload */
	}

	if (dev_conf->disable_pause_copy == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_PAUSECOPYDI_BIT;                    /* [23]     Do not copy pause Frames to memory */
	}

	reg_val |= (((u32_t)(dev_conf->amba_dbus_width) & ETH_XLNX_EMACPS_NWCFG_DBUSW_MASK)
		 << ETH_XLNX_EMACPS_NWCFG_DBUSW_SHIFT);                              /* [22..21] Data bus width */

	reg_val |= (((u32_t)mdc_divisor & ETH_XLNX_EMACPS_NWCFG_MDC_MASK)
		<< ETH_XLNX_EMACPS_NWCFG_MDC_SHIFT);                                 /* [20..18] MDC clock divisor */

	if (dev_conf->discard_rx_fcs == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_FCSREM_BIT;                         /* [17]     Discard FCS from received frames */
	}

	if (dev_conf->discard_rx_length_errors == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_LENGTHERRDSCRD_BIT;                 /* [16]     RX length error discard */
	}

	reg_val |= (((u32_t)dev_conf->hw_rx_buffer_offset & ETH_XLNX_EMACPS_NWCFG_RXOFFS_MASK)
		<< ETH_XLNX_EMACPS_NWCFG_RXOFFS_SHIFT);			                     /* [15..14] RX buffer offset */

	if (dev_conf->enable_pause == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_PAUSEEN_BIT;                        /* [13]     Enable pause TX */
	}

	if (dev_conf->enable_tbi == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_TBIINSTEAD_BIT;                     /* [11]     enable TBI instead of GMII/MII */
	}

	if (dev_conf->ext_addr_match == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_EXTADDRMATCHEN_BIT;                 /* [09]     External address match enable */
	}

	if (dev_conf->enable_1536_frames == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_1536RXEN_BIT;                       /* [08]     Enable 1536 byte frames reception */
	}

	if (dev_conf->enable_ucast_hash == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_UCASTHASHEN_BIT;                    /* [07]     Receive unicast hash frames */
	}

	if (dev_conf->enable_mcast_hash == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_MCASTHASHEN_BIT;                    /* [06]     Receive multicast hash frames */
	}

	if (dev_conf->disable_bcast == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_BCASTDIS_BIT;                       /* [05]     Do not receive broadcast frames */
	}

	if (dev_conf->copy_all_frames == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_COPYALLEN_BIT;                      /* [04]     Copy all frames */
	}

	if (dev_conf->discard_non_vlan == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_NVLANDISC_BIT;                      /* [02]     Receive only VLAN frames */
	}

	if (dev_conf->enable_fdx == 1) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_FDEN_BIT;                           /* [01]     enable Full duplex */
	}

	if (dev_conf->max_link_speed == LINK_100MBIT) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_100_BIT;                            /* [00]     10 or 100 Mbs */
	} else if (dev_conf->max_link_speed == LINK_1GBIT) {
		reg_val |= ETH_XLNX_EMACPS_NWCFG_1000_BIT;                           /* [10]     Gigabit mode enable */
	} else {
		/* In 10 MBit/s mode, both bits [00] and [10] remain 0. */
		reg_val &= ~(ETH_XLNX_EMACPS_NWCFG_1000_BIT | ETH_XLNX_EMACPS_NWCFG_100_BIT);
	}

	/* Write the assembled register contents to gem.net_cfg */

	sys_write32(reg_val, dev_conf->base_addr + ETH_XLNX_EMACPS_NWCFG_OFFSET);
}

static void eth_xlnx_emacps_set_initial_dmacr (struct device *dev)
{
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	u32_t 				reg_val   = 0;

	/* gem.dma_cfg register bit (field) definitions: comp. Zynq-7000 TRM, p. 1278 ff. */

	if (dev_conf->disc_rx_ahb_unavail == 1)
	{
		reg_val |= ETH_XLNX_EMACPS_DMACR_DISCNOAHB_BIT;		/* [24]     Discard RX packet when AHB unavailable */
	}

	/* [23..16] DMA RX buffer size in AHB system memory
	 *    e.g.: 0x02 = 128, 0x18 = 1536, 0xA0 = 10240  */

	reg_val |= (((u32_t)dev_conf->ahb_rx_buffer_size & ETH_XLNX_EMACPS_DMACR_RX_BUF_MASK)
			<< ETH_XLNX_EMACPS_DMACR_RX_BUF_SHIFT);

	if (dev_conf->enable_tx_chksum_offload == 1)
	{
		reg_val |= ETH_XLNX_EMACPS_DMACR_TCP_CHKSUM_BIT;	/* [11]     TX TCP/UDP/IP checksum offload to GEM  */
	}

	if (dev_conf->tx_buffer_size_full == 1)
	{
		reg_val |= ETH_XLNX_EMACPS_DMACR_TX_SIZE_BIT;		/* [10]     TX buffer memory size select           */
	}

	/* [09..08] RX packet buffer memory size select
	 *          0 = 1kB, 1 = 2kB, 2 = 4kB, 3 = 8kB */

	reg_val |= (((u32_t)dev_conf->hw_rx_buffer_size << ETH_XLNX_EMACPS_DMACR_RX_SIZE_SHIFT)
			& ETH_XLNX_EMACPS_DMACR_RX_SIZE_MASK);

	if (dev_conf->enable_ahb_packet_endian_swap == 1)
	{
		reg_val |= ETH_XLNX_EMACPS_DMACR_ENDIAN_BIT;		/* [07]     AHB packet data endian swap enable     */
	}

	if (dev_conf->enable_ahb_md_endian_swap == 1)
	{
		reg_val |= ETH_XLNX_EMACPS_DMACR_DESCR_ENDIAN_BIT;	/* [06]     AHB mgmt descriptor endian swap enable */
	}

	/* [04..00] AHB fixed burst length for DMA ops.
	 *          00001 = single AHB bursts,
	 *          001xx = attempt to use INCR4  bursts,
	 *          01xxx = attempt to use INCR8  bursts,
	 *          1xxxx = attempt to use INCR15 bursts */

	reg_val |= ((u32_t)dev_conf->ahb_burst_length & ETH_XLNX_EMACPS_DMACR_AHB_BURST_LENGTH_MASK);

	/* Write the assembled register contents */

	sys_write32(reg_val, dev_conf->base_addr + ETH_XLNX_EMACPS_DMACR_OFFSET);
}

static void eth_xlnx_emacps_set_mac_address (struct device *dev)
{
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data = DEV_DATA(dev);

	u32_t 	regval_top = 0;
	u32_t 	regval_bot = 0;

	/* Reverse the order of the MAC bytes: if the high byte of the address is
	 * specified first in the array, the bytes will end up in the config registers
	 * (gem.spec_addr1_bot, gem.spec_addr1_top) in reverse order, prompting the
	 * controller to discard any non-bcast packets since the packets specifically
	 * addressed to us don't make it past the MAC address filter which uses the
	 * values from the config registers. */

	regval_bot  = (dev_data->mac_addr[0] & 0xFF);
	regval_bot |= (dev_data->mac_addr[1] & 0xFF) << 8;
	regval_bot |= (dev_data->mac_addr[2] & 0xFF) << 16;
	regval_bot |= (dev_data->mac_addr[3] & 0xFF) << 24;
	    
	regval_top  = (dev_data->mac_addr[4] & 0xFF);
	regval_top |= (dev_data->mac_addr[5] & 0xFF) << 8;

	sys_write32(regval_bot, dev_conf->base_addr + ETH_XLNX_EMACPS_LADDR1L_OFFSET);
	sys_write32(regval_top, dev_conf->base_addr + ETH_XLNX_EMACPS_LADDR1H_OFFSET);
}

static void eth_xlnx_emacps_configure_clocks (struct device *dev)
{
	/* MIO/EMIO setup for the respective GEM as described in the TRM, Chapter 16.3.3,
	 * is not tackled here. This *should* be tackled by the PS7Init code. */

	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);

	u32_t	reg_val	= 0;	
	u32_t	div0	= 0;
	u32_t	div1	= 0;
	u32_t	in		= (CONFIG_ZYNQ_PS_REF_FREQUENCY * dev_conf->reference_pll_ref_clk_multi);
	u32_t	out		= 0;
	u32_t	tmp		= 0;

	/* TODO: use dev_data->eff_link_speed instead. Break out if link is down.
	 *
	 */
	if (dev_conf->max_link_speed == LINK_10MBIT)
	{
		out = 2500000;   /* Target frequency: 2.5 MHz */
	}
	else if (dev_conf->max_link_speed == LINK_100MBIT)
	{
		out = 25000000;  /* Target frequency: 25 MHz */
	}
	else if (dev_conf->max_link_speed == LINK_1GBIT)
	{
		out = 125000000; /* Target frequency: 125 MHz */
	}

	div0 = dev_conf->gem_clk_divisor0;
	div1 = dev_conf->gem_clk_divisor1;

	if (div0 == 0 && div1 == 0)
	{
		/* Both divisors == 0 -> auto-calculate the divisors */

		for (div0 = 1; div0 < 64; div0++)
		{
			for (div1 = 1; div1 < 64; div1++)
			{
				tmp = ((in / div0) / div1);
				if (tmp >= (out - 2) && tmp <= (out + 2))
				{
					break;
				}
			}
			if (tmp >= (out - 2) && tmp <= (out + 2))
			{
				break;
			}
		}
	}

	sys_write32(ETH_XLNX_SLCR_UNLOCK_CONSTANT, ETH_XLNX_SLCR_UNLOCK_REGISTER); /* SLCR unlock */

	/* Write the respective GEM's (R)CLK configuration registers in the SLCR.
	 * In both cases, bit [0] is the clock enable bit. */

	reg_val = (
			(((u32_t)dev_conf->gem_clk_source & ETH_XLNX_SLCR_RCLK_CTRL_REGISTER_SRC_MASK)
			<< ETH_XLNX_SLCR_RCLK_CTRL_REGISTER_SRC_SHIFT)
			| ETH_XLNX_SLCR_RCLK_ENABLE_BIT);
	sys_write32(reg_val, dev_conf->slcr_rclk_register_addr);

	reg_val = (
		    ((div1 & ETH_XLNX_SLRC_CLK_CTR_REGISTER_DIV_MASK)
		    << ETH_XLNX_SLRC_CLK_CTR_REGISTER_DIV1_SHIFT)
			| ((div0 & ETH_XLNX_SLRC_CLK_CTR_REGISTER_DIV_MASK)
			<< ETH_XLNX_SLRC_CLK_CTR_REGISTER_DIV0_SHIFT)
			| (((u32_t)dev_conf->reference_pll & ETH_XLNX_SLRC_CLK_CTR_REGISTER_REF_PLL_MASK)
			<< ETH_XLNX_SLRC_CLK_CTR_REGISTER_REF_PLL_SHIFT)
			| ETH_XLNX_SLCR_CLK_ENABLE_BIT);
	sys_write32(reg_val, dev_conf->slcr_clk_register_addr);

	sys_write32(ETH_XLNX_SLCR_LOCK_CONSTANT, ETH_XLNX_SLCR_LOCK_REGISTER); /* SLCR lock */
}

static void eth_xlnx_emacps_init_phy (struct device *dev)
{
	struct eth_xlnx_emacps_dev_data *dev_data = DEV_DATA(dev);

	eth_xlnx_emacps_phy_detect(dev);
		
	if (dev_data->phy_id != 0x00000000 && dev_data->phy_id != 0xFFFFFFFF)
	{
		eth_xlnx_emacps_phy_reset(dev);
		eth_xlnx_emacps_phy_configure(dev);
	}
}

static void eth_xlnx_emacps_configure_buffers (struct device *dev)
{
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf	= DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data	= DEV_DATA(dev);

	u32_t							i 			= 0;
	struct eth_xlnx_emacps_bd		*bdptr		= NULL; /* Current buffer descriptor pointer */

	/* Initial configuration of the RX/TX BD rings */

	#if defined(DT_INST_0_XLNX_EMACPS) && defined(CONFIG_ETH_XLNX_EMACPS_PORT_0)

	if (dev_conf->base_addr == DT_INST_0_XLNX_EMACPS_BASE_ADDRESS)
	{
		#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_0_DMA_FIXED
			struct eth_xlnx_dma_area_gem0 *dma_area_gem0 = (struct eth_xlnx_dma_area_gem0*)CONFIG_ETH_XLNX_EMACPS_PORT_0_DMA_BASE_ADDRESS;

			dev_data->rxbd_ring.first_bd = &(dma_area_gem0->rx_bd[0]);
			dev_data->txbd_ring.first_bd = &(dma_area_gem0->tx_bd[0]);
			dev_data->first_rx_buffer    = (u8_t*)dma_area_gem0->rx_buffer;
			dev_data->first_tx_buffer    = (u8_t*)dma_area_gem0->tx_buffer;
		#else
			dev_data->rxbd_ring.first_bd = &(dma_area_gem0.rx_bd[0]);
			dev_data->txbd_ring.first_bd = &(dma_area_gem0.tx_bd[0]);
			dev_data->first_rx_buffer    = (u8_t*)dma_area_gem0.rx_buffer;
			dev_data->first_tx_buffer    = (u8_t*)dma_area_gem0.tx_buffer;
		#endif
	}

	#endif

	#if defined(DT_INST_1_XLNX_EMACPS) && defined(CONFIG_ETH_XLNX_EMACPS_PORT_1)

	if (dev_conf->base_addr == DT_INST_1_XLNX_EMACPS_BASE_ADDRESS)
	{
		#ifdef CONFIG_ETH_XLNX_EMACPS_PORT_1_DMA_FIXED
			struct eth_xlnx_dma_area_gem1 *dma_area_gem1 = (struct eth_xlnx_dma_area_gem1*)CONFIG_ETH_XLNX_EMACPS_PORT_1_DMA_BASE_ADDRESS;

			dev_data->rxbd_ring.first_bd = &(dma_area_gem1->rx_bd[0]);
			dev_data->txbd_ring.first_bd = &(dma_area_gem1->tx_bd[0]);
			dev_data->first_rx_buffer    = (u8_t*)dma_area_gem1->rx_buffer;
			dev_data->first_tx_buffer    = (u8_t*)dma_area_gem1->tx_buffer;
		#else
			dev_data->rxbd_ring.first_bd = &(dma_area_gem1.rx_bd[0]);
			dev_data->txbd_ring.first_bd = &(dma_area_gem1.tx_bd[0]);
			dev_data->first_rx_buffer    = (u8_t*)dma_area_gem0.rx_buffer;
			dev_data->first_tx_buffer    = (u8_t*)dma_area_gem0.tx_buffer;
		#endif
	}

	#endif

	/* Set initial RX BD data -> comp. Zynq-7000 TRM, Chapter 16.3.5, "Receive Buffer Descriptor List" */
	/* The BD ring data other than the base RX/TX buffer pointers will be set in eth_xlnx_emacps_iface_init(). */

	bdptr = dev_data->rxbd_ring.first_bd;

	for (i = 0; i < (dev_conf->rxbd_count - 1); i++)
	{
		bdptr->ctrl = 0; /* clear 'used' bit -> BD is owned by the controller -> as per TRM */
		bdptr->addr = (u32_t)((u32_t)dev_data->first_rx_buffer + (i * (u32_t)dev_conf->rx_buffer_size));
		++bdptr;
	}

	/* For the last BD, bit [1] must be OR'ed in the buffer memory address
	   -> this is the 'wrap' bit indicating that this is the last BD in the ring.
	   This location is used as bits [1..0] can't be part of the buffer address
	   due to alignment requirements anyways. Watch out: TX BDs handle this
	   differently, their wrap bit is located in the BD's control word! */

	bdptr->ctrl = 0; /* BD is owned by the controller */
	bdptr->addr = (u32_t)((u32_t)dev_data->first_rx_buffer + (i * (u32_t)dev_conf->rx_buffer_size)) | ETH_XLNX_EMACPS_RXBD_WRAP_BIT;

	/* Set initial TX BD data -> comp. Zynq-7000 TRM, Chapter 16.3.5, "Transmit Buffer Descriptor List" */
	/* TX BD ring data has already been set up in eth_xlnx_emacps_iface_init(). */

	bdptr = dev_data->txbd_ring.first_bd;

	for (i = 0; i < (dev_conf->txbd_count - 1); i++)
	{
		/* FIXME clear 'used' bit -> BD is owned by the controller -> as per TRM - CONTRADICTION */
		bdptr->ctrl = ETH_XLNX_EMACPS_TXBD_USED_BIT;
		bdptr->addr = (u32_t)((u32_t)dev_data->first_tx_buffer + (i * (u32_t)dev_conf->tx_buffer_size));
		++bdptr;
	}

	/* For the last BD, set the 'wrap' bit indicating to the controller that this
	   BD is the last one in the ring. -> For TX BDs, the 'wrap' bit isn't located
	   in the address word, but in the control word instead. */

	bdptr->ctrl = (ETH_XLNX_EMACPS_TXBD_USED_BIT | ETH_XLNX_EMACPS_TXBD_WRAP_BIT);
	bdptr->addr = (u32_t)((u32_t)dev_data->first_tx_buffer + (i * (u32_t)dev_conf->tx_buffer_size));

	sys_write32((u32_t)dev_data->rxbd_ring.first_bd, dev_conf->base_addr + ETH_XLNX_EMACPS_RXQBASE_OFFSET);
	sys_write32((u32_t)dev_data->txbd_ring.first_bd, dev_conf->base_addr + ETH_XLNX_EMACPS_TXQBASE_OFFSET);
}

static int eth_xlnx_emacps_mdio_read (u32_t base_addr, u8_t phy_addr, u8_t reg_addr)
{
	u32_t	reg_val = 0;

	/* MDIO read operation as described in Zynq-7000 TRM, Chapter 16.3.4, p. 517 */

	/* Wait until gem.net_status[phy_mgmt_idle] == 1 before issuing the current command */
	/* TODO: This should have a time-out! */

	do {
		reg_val = sys_read32(base_addr + ETH_XLNX_EMACPS_NWSR_OFFSET);
	} while ((reg_val & ETH_XLNX_EMACPS_MDIO_IDLE_BIT) == 0);

	/* Assemble & write the current read command to the gem.phy_maint register */

	reg_val  = ETH_XLNX_EMACPS_PHY_MAINT_CONST_BITS; /* Set the bits constant for any operation */
	reg_val |= ETH_XLNX_EMACPS_PHY_MAINT_READ_OP_BIT; /* Indicate a read operation */
	reg_val |= (((u32_t)phy_addr & ETH_XLNX_EMACPS_PHY_MAINT_PHY_ADDRESS_MASK) << ETH_XLNX_EMACPS_PHY_MAINT_PHY_ADDRESS_SHIFT); /* PHY address */
	reg_val |= (((u32_t)reg_addr & ETH_XLNX_EMACPS_PHY_MAINT_REGISTER_ID_MASK) << ETH_XLNX_EMACPS_PHY_MAINT_REGISTER_ID_SHIFT); /* Register address */

	sys_write32(reg_val, base_addr + ETH_XLNX_EMACPS_PHY_MAINTENANCE_OFFSET);

	/* Wait until gem.net_status[phy_mgmt_idle] == 1 -> current command completed */
	/* TODO: This should have a time-out! */

	do {
		reg_val = sys_read32(base_addr + ETH_XLNX_EMACPS_NWSR_OFFSET);
	} while ((reg_val & ETH_XLNX_EMACPS_MDIO_IDLE_BIT) == 0);

	/* Read the data returned by the PHY -> lower 16 bits of the PHY maintenance register */

	reg_val = sys_read32(base_addr + ETH_XLNX_EMACPS_PHY_MAINTENANCE_OFFSET);

	return (u16_t)reg_val;
}

static void eth_xlnx_emacps_mdio_write (u32_t base_addr, u8_t phy_addr, u8_t reg_addr, u16_t value)
{
	u32_t	reg_val = 0;

	/* MDIO write operation as described in Zynq-7000 TRM, chapter 16.3.4, p. 517 */

	/* Wait until gem.net_status[phy_mgmt_idle] == 1 before issuing the current command */
	/* TODO: This should have a time-out! */

	do {
		reg_val = sys_read32(base_addr + ETH_XLNX_EMACPS_NWSR_OFFSET);
	} while ((reg_val & ETH_XLNX_EMACPS_MDIO_IDLE_BIT) == 0);

	/* Assemble & write the current read command to the gem.phy_maint register */

	reg_val  = ETH_XLNX_EMACPS_PHY_MAINT_CONST_BITS; /* Set the bits constant for any operation */
	reg_val |= ETH_XLNX_EMACPS_PHY_MAINT_WRITE_OP_BIT; /* Indicate a read operation */
	reg_val |= (((u32_t)phy_addr & ETH_XLNX_EMACPS_PHY_MAINT_PHY_ADDRESS_MASK) << ETH_XLNX_EMACPS_PHY_MAINT_PHY_ADDRESS_SHIFT); /* PHY address */
	reg_val |= (((u32_t)reg_addr & ETH_XLNX_EMACPS_PHY_MAINT_REGISTER_ID_MASK) << ETH_XLNX_EMACPS_PHY_MAINT_REGISTER_ID_SHIFT);  /* Register address */
	reg_val |= ((u32_t)value & ETH_XLNX_EMACPS_PHY_MAINT_DATA_MASK); /* 16 bits of data for the destination register */

	sys_write32(reg_val, base_addr + ETH_XLNX_EMACPS_PHY_MAINTENANCE_OFFSET);

	/* Wait until gem.net_status[phy_mgmt_idle] == 1 -> current command completed */
	/* TODO: This should have a time-out! */

	do {
		reg_val = sys_read32(base_addr + ETH_XLNX_EMACPS_NWSR_OFFSET);
	} while ((reg_val & ETH_XLNX_EMACPS_MDIO_IDLE_BIT) == 0);
}

static void eth_xlnx_emacps_phy_detect (struct device *dev)
{
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data = DEV_DATA(dev);
	u8_t	phy_addr = 0;
	u32_t	phy_id   = 0;
	u16_t	phy_data = 0;

	/* PHY detection as described in Zynq-7000 TRM, chapter 16.3.4, p. 517 */

	for (phy_addr = 1; phy_addr <= 32; phy_addr++)
	{
		phy_data  = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, phy_addr, PHY_IDENTIFIER_1_REGISTER);
		phy_id    = (((u32_t)phy_data << 16) & 0xFFFF0000);
		phy_data  = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, phy_addr, PHY_IDENTIFIER_2_REGISTER);
		phy_id   |= ((u32_t)phy_data & 0x0000FFFF);

		if (phy_id != 0x00000000 && phy_id != 0xFFFFFFFF)
		{
			dev_data->phy_addr = phy_addr;
			dev_data->phy_id   = phy_id;
			LOG_DBG("eth_xlnx_emacps_phy_detect: PHY detected at address %hhu: ID 0x%08X", phy_addr, phy_id);

			return;
		}
	}

	LOG_DBG("eth_xlnx_emacps_phy_detect: PHY auto-detection failed!");
}

static void eth_xlnx_emacps_phy_reset (struct device *dev)
{
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data = DEV_DATA(dev);
	u16_t	phy_data = 0;

	/* 
	 * PHY state machine reset as implemented in the PHY found on the ZedBoard:
	 * https://www.marvell.com/documents/eoxwrbluvwybgxvagkkf/
	 * Marvell Alaska 88E15xx
	 * Page 0, register address 0 = Copper control register, bit [15] = PHY reset.
	 * Accessing the register 0/0 in a RMW fashion.
	 */

	phy_data  = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_REGISTER);
	phy_data |= 0x8000;  /* Reset bit */
	phy_data &= ~0x1000; /* Auto-neg disable (for now) */
	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_REGISTER, phy_data);

	/* 
	 * Bit [15] reverts to 0 once the reset is complete.
	 * TODO: reset polling completion should have a time-out 
	 */

	while ((phy_data & 0x8000) != 0)
	{
		phy_data = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_REGISTER);
	}
}

static void eth_xlnx_emacps_phy_configure (struct device *dev)
{
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data = DEV_DATA(dev);
	u16_t	phy_data		= 0;
	u16_t	phy_data_gbit	= 0;

	/*
	 * All register / mask data based on:
	 * https://www.marvell.com/documents/eoxwrbluvwybgxvagkkf/
	 * Marvell Alaska 88E1510/88E1518/88E1512/88E1514 datasheet
	 */

	/* 
	 * Configure the system interface and media type (e.g. "RGMII to Copper").
	 * TODO Make this value configurable via KConfig
	 * THIS IS VENDOR-SPECIFIC -> only works on Zedboard PHY type Marvell Alaska 88E15xx
	 * and related models!
	 * Page 18, register address 20 = General Control Register 1,
	 * bits [2..0] = mode configuration
	 * NOTICE: a change of this value requires a subsequent software reset command
	 * via the same register's bit [15].
	 */

	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_PAGE_SWITCH_REGISTER, PHY_GENERAL_CONTROL_1_PAGE);
	phy_data  = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_PAGE_SWITCH_REGISTER);

	phy_data  = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_GENERAL_CONTROL_1_REGISTER);

	/* [2..0] 000 = RGMII (System Mode) to Copper */

	phy_data &= ~(PHY_MODE_CONFIG_MASK << PHY_MODE_CONFIG_SHIFT);	
	phy_data |= 0;
	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_GENERAL_CONTROL_1_REGISTER, phy_data);

	/* [15] Mode Software Reset bit, affecting pages 6 and 18
	 * Reset is performed immediately, bit [15] is self-clearing */

	phy_data |= 0x8000; 
	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_GENERAL_CONTROL_1_REGISTER, phy_data);

	/* Bit [15] reverts to 0 once the reset is complete.
	 * TODO: reset polling completion should have a time-out */

	while ((phy_data & 0x8000) != 0)
	{
		phy_data = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_GENERAL_CONTROL_1_REGISTER);
	}

	/* Revert to register page 0 */

	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_PAGE_SWITCH_REGISTER, PHY_BASE_REGISTERS_PAGE);

	/* Configure MDIX
	 * TODO Make this value configurable via KConfig
	 * THIS IS VENDOR-SPECIFIC -> only works on Zedboard PHY type Marvell Alaska 88E15xx
	 * and related models!
	 * Page 0, register address 16 = Copper specific control register 1,
	 * bits [6..5] = MDIO crossover mode.
	 * NOTICE: a change of this value requires a subsequent software reset command
	 * via Copper Control Register's bit [15]. */

	/* [6..5] 11 = Enable auto cross over detection */

	phy_data  = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_1_REGISTER);
	phy_data &= ~(PHY_MDIX_CONFIG_MASK << PHY_MDIX_CONFIG_SHIFT);
	phy_data |= (0x03 << PHY_MDIX_CONFIG_SHIFT); 
	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_1_REGISTER, phy_data);

	/* Configure the Copper Interrupt Enable Register.
	 * -> all bits contained herein will be retained during the upcoming software reset operation.
	 * Page 0, register address 18 = Copper Specific Interrupt Enable Register,
	 * bit [14] = Speed changed interrupt enable,
	 * bit [13] = Duplex changed interrupt enable,
	 * bit [11] = Auto-negotiation completed interrupt enable,
	 * bit [10] = Link status changed interrupt enable. */

	phy_data =   PHY_COPPER_SPEED_CHANGED_INTERRUPT_BIT
			   | PHY_COPPER_DUPLEX_CHANGED_INTERRUPT_BIT
			   | PHY_COPPER_AUTONEG_COMPLETED_INTERRUPT_BIT
			   | PHY_COPPER_LINK_STATUS_CHANGED_INTERRUPT_BIT;
	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_INTERRUPT_ENABLE_REGISTER, phy_data);

	/* Page 0 / Reg 0 [15] Copper Software Reset bit, affecting pages 0, 2, 3, 5, 7
	 * Reset is performed immediately, bit [15] is self-clearing */

	phy_data  = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_REGISTER);
	phy_data |= 0x8000; 
	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_REGISTER, phy_data);

	/* Bit [15] reverts to 0 once the reset is complete.
	 * TODO: reset polling completion should have a time-out */

	while ((phy_data & 0x8000) != 0)
	{
		phy_data = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_REGISTER);
	}

	/* Clear the interrupt status register before advertising the supported link speed(s) */

	phy_data = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_INTERRUPT_STATUS_REGISTER);

	/* Set what link speeds shall be advertised during auto-negotiation, re-enable auto-negotiation
	 * PHY link speed advertisement configuration as described in Zynq-7000 TRM, chapter 16.3.4, p. 517 */

	/* Advertise the link speed from the device configuration & perform auto-negotiation
	 * This process involves:
	 * Page 0, register address 4 = Copper Auto-Negotiation Advertisement Register,
	 * Page 0, register address 0 = Copper Control Register, bit [15] = Reset -> apply all changes made regarding advertisement,
	 * Page 0, register address 9 = 1000BASE-T Control Register (if link speed = 1GBit/s),
	 * Page 0, register address 1 = Copper Status Register, bit [5] = Copper Auto-Negotiation Complete. */

	/* Advertise the speed & duplex specified in the device configuration data -> targets: registers 4 & 9 */

	phy_data       = 0x01;   /* [4..0] = Selector field, 00001 = 802.3 */
	phy_data_gbit  = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_1000BASET_CONTROL_REGISTER);

	if (dev_conf->enable_fdx == 1) {
		if (dev_conf->max_link_speed == LINK_1GBIT) {
			phy_data_gbit  = (1 << 9);  /* Advertise 1 GBit/s, full duplex */ 
			if (dev_conf->phy_advertise_lower == 1) {
				phy_data  |= (1 << 8);  /* 100BASE-TX, full duplex */
				phy_data  |= (1 << 6);  /* 10BASE-TX, full duplex */
			}
		} else if (dev_conf->max_link_speed == LINK_100MBIT) {
			phy_data      |= (1 << 8);  /* 100BASE-TX, full duplex */
			if (dev_conf->phy_advertise_lower == 1) {
				phy_data  |= (1 << 6);  /* 10BASE-TX, full duplex */
			}
			phy_data_gbit &= ~(0x0300); /* clear 1000BASE-TX advertisement bits */
		} else if (dev_conf->max_link_speed == LINK_10MBIT) {
			phy_data      |= (1 << 6);  /* 10BASE-TX, full duplex */
			phy_data_gbit &= ~(0x0300); /* clear 1000BASE-TX advertisement bits */
		}
	}
	else
	{
		if (dev_conf->max_link_speed == LINK_1GBIT) {
			phy_data_gbit  = (1 << 8);  /* Advertise 1 GBit/s, half duplex */ 
			if (dev_conf->phy_advertise_lower == 1) {
				phy_data  |= (1 << 7);  /* 100BASE-TX, half duplex */
				phy_data  |= (1 << 5);  /* 10BASE-TX, half duplex */
			}
		} else if (dev_conf->max_link_speed == LINK_100MBIT) {
			phy_data      |= (1 << 7);  /* 100BASE-TX, half duplex */
			if (dev_conf->phy_advertise_lower == 1) {
				phy_data  |= (1 << 5);  /* 10BASE-TX, half duplex */
			}
			phy_data_gbit &= ~(0x0300); /* clear 1000BASE-TX advertisement bits */
		} else if (dev_conf->max_link_speed == LINK_10MBIT) {
			phy_data      |= (1 << 5);  /* 10BASE-TX, half duplex */
			phy_data_gbit &= ~(0x0300); /* clear 1000BASE-TX advertisement bits */
		}
	}

	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_1000BASET_CONTROL_REGISTER, phy_data_gbit);
	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_AUTONEG_ADV_REGISTER, phy_data);

	/* Page 0 / Reg 0 [15] Copper Software Reset bit, affecting pages 0, 2, 3, 5, 7
	 * Reset is performed immediately, bit [15] is self-clearing */

	phy_data  = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_REGISTER);
	phy_data |= 0x8000; /* Reset bit */
	phy_data |= 0x1000; /* Enable auto-negotiation */ 
	eth_xlnx_emacps_mdio_write(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_REGISTER, phy_data);

	/* Bit [15] reverts to 0 once the reset is complete.
	 * TODO: reset polling completion should have a time-out */

	while ((phy_data & 0x8000) != 0)
	{
		phy_data = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_CONTROL_REGISTER);
	}

	/* Set the link speed to 'link down' for now, once auto-negotiation is complete,
	 * the result will be handled by the auxiliary thread. */

	dev_data->eff_link_speed = LINK_DOWN;
}

static u16_t eth_xlnx_emacps_phy_poll_int_status (struct device *dev) {
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data = DEV_DATA(dev);
	u16_t phy_data = 0;

	phy_data = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_INTERRUPT_STATUS_REGISTER);
	return phy_data;
}

static u8_t eth_xlnx_emacps_phy_poll_link_status (struct device *dev) {
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf = DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data = DEV_DATA(dev);
	u16_t phy_data = 0;

	phy_data = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_STATUS_REGISTER);
	return ((phy_data >> PHY_COPPER_LINK_STATUS_BIT_SHIFT) & 0x01);
}

static enum eth_xlnx_link_speed eth_xlnx_emacps_phy_poll_link_speed (struct device *dev) {
	struct eth_xlnx_emacps_dev_cfg 	*dev_conf  = DEV_CFG(dev);
	struct eth_xlnx_emacps_dev_data *dev_data  = DEV_DATA(dev);

	enum eth_xlnx_link_speed 		link_speed = LINK_DOWN;
	u16_t 							phy_data   = 0;

	phy_data   = eth_xlnx_emacps_mdio_read(dev_conf->base_addr, dev_data->phy_addr, PHY_COPPER_STATUS_1_REGISTER);
	phy_data >>= PHY_LINK_SPEED_SHIFT;
	phy_data  &= PHY_LINK_SPEED_MASK;

	/* Link speed bit masks: comp. Marvell Alaska PHY 88E1510/88E1518/88E1512/88E1514 datasheet, Table 77 */

	switch (phy_data) {
		case 0:
			link_speed = LINK_10MBIT;
			break;
		case 1:
			link_speed = LINK_100MBIT;
			break;
		case 2:
			link_speed = LINK_1GBIT;
			break;
		default:
			link_speed = LINK_DOWN;
			break;
	};

	return link_speed;
}
