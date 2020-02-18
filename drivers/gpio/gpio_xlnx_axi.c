/*
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file GPIO driver for the Xilinx AXI GPIO v2.0 LogiCORE IP Core.
 */

/*
 * IP core documentation used:
 * Xilinx AXI GPIO v2.0 LogiCORE IP Product Guide PG144 dated October 5, 2016
 * NOTICE: this driver only supports the AXI GPIO IP core in single channel
 * operation mode. While pin access in index-based and could therefore handle
 * the dual-channel mode range [0..63], the bit masks of the callback API
 * are limited to 32 bits.
 */

#include <kernel.h>
#include <device.h>

#include <drivers/gpio.h>
#include "gpio_utils.h"

#include <errno.h>

#define GPIO_XLNX_AXI_PINS_PER_CHANNEL			32

#define GPIO_XLNX_AXI_GPIO_DATA_REG_OFFSET		0x0000
#define GPIO_XLNX_AXI_GPIO_TRI_REG_OFFSET		0x0004
#define GPIO_XLNX_AXI_GPIO_GIER_REG_OFFSET		0x011C
#define GPIO_XLNX_AXI_GPIO_IP_IER_REG_OFFSET	0x0128
#define GPIO_XLNX_AXI_GPIO_IP_ISR_REG_OFFSET	0x0120

#define GPIO_XLNX_AXI_GLOBAL_INT_ENABLE			(1 << 31)
#define GPIO_XLNX_AXI_CH1_INT_ENABLE			(1 <<  0)
#define GPIO_XLNX_AXI_CH1_INT_DISABLE			(0 <<  0)
#define GPIO_XLNX_AXI_CH1_INT_PENDING			(1 <<  0)

typedef void (*gpio_xlnx_axi_config_irq_t)(struct device*);
static void gpio_xlnx_axi_config_interrupt(struct device* dev);

struct gpio_xlnx_axi_dev_cfg {
	u8_t						supp_interrupt;

	u32_t						base_addr;
	u32_t						data_reg_offset;
	u32_t						tri_reg_offset;
	u32_t						gier_reg_offset;
	u32_t						ip_ier_reg_offset;
	u32_t						ip_isr_reg_offset;

	gpio_xlnx_axi_config_irq_t	config_func;
};

struct gpio_xlnx_axi_dev_data {
	sys_slist_t 				callbacks;

	u32_t						last_data;
	u32_t						pin_dir;
	u32_t						int_mask;
	u32_t						callback_mask;
	u32_t						invert_mask;

	u8_t						use_interrupt;
};

/* Device configuration / run-time data resolver macros */

#define DEV_CFG(dev) \
	((struct gpio_xlnx_axi_dev_cfg*)(dev)->config->config_info)
#define DEV_DATA(dev) \
	((struct gpio_xlnx_axi_dev_data*)(dev)->driver_data)

static void gpio_xlnx_axi_isr (void *arg)
{
	struct device					*dev         = (struct device*)arg;
	struct gpio_xlnx_axi_dev_cfg	*dev_conf    = DEV_CFG(dev);
	struct gpio_xlnx_axi_dev_data	*dev_data    = DEV_DATA(dev);
	u32_t							isr_reg_val  = 0;
	u32_t							curr_in_data = 0;
	u32_t							last_in_data = 0;
	u32_t							pin_mask     = 0;

	isr_reg_val = sys_read32(dev_conf->base_addr + dev_conf->ip_isr_reg_offset);
	if ((isr_reg_val & GPIO_XLNX_AXI_CH1_INT_PENDING) == GPIO_XLNX_AXI_CH1_INT_PENDING) {
		curr_in_data  = sys_read32(dev_conf->base_addr + dev_conf->data_reg_offset);
		curr_in_data ^= dev_data->invert_mask;
		curr_in_data &= ~dev_data->pin_dir;

		last_in_data  = dev_data->last_data & ~dev_data->pin_dir;
		pin_mask      = curr_in_data ^ last_in_data;

		if ((pin_mask & dev_data->callback_mask) != 0) {
			gpio_fire_callbacks(&dev_data->callbacks, dev, pin_mask);
		}

		sys_write32(GPIO_XLNX_AXI_CH1_INT_PENDING, dev_conf->base_addr + dev_conf->ip_isr_reg_offset);
		dev_data->last_data = (dev_data->last_data & dev_data->pin_dir) | curr_in_data;
	}
}

static int gpio_xlnx_axi_config (
	struct device *dev,
	int access_op,
	u32_t pin,
	int flags)
{
	struct gpio_xlnx_axi_dev_cfg 	*dev_conf = DEV_CFG(dev);
	struct gpio_xlnx_axi_dev_data	*dev_data = DEV_DATA(dev);

	if (flags & GPIO_INT)
	{
		 if (    (dev_conf->supp_interrupt == 0)
			  || (flags & GPIO_INT_ACTIVE_LOW)
		 	  || (flags & GPIO_INT_EDGE)
			  || (flags & GPIO_INT_DOUBLE_EDGE)
			  || (flags & GPIO_INT_DEBOUNCE)) {
			 return -ENOTSUP;
		 }

		 dev_data->use_interrupt = 1;
	}

	if ((flags & GPIO_PUD_PULL_UP) || (flags & GPIO_PUD_PULL_DOWN)) {
		return -ENOTSUP;
	}

	if (access_op == GPIO_ACCESS_BY_PORT) {
		if (flags & GPIO_DIR_OUT) {
			dev_data->pin_dir = 0xFFFFFFFF;
		} else {
			dev_data->pin_dir = 0x00000000;
		}

		if (flags & GPIO_POL_INV) {
			dev_data->invert_mask = 0xFFFFFFFF;
		} else {
			dev_data->invert_mask = 0x00000000;
		}

		if (flags & GPIO_INT) {
			dev_data->int_mask = 0xFFFFFFFF;
			dev_data->use_interrupt = 1;
		} else {
			dev_data->int_mask = 0x00000000;
			dev_data->use_interrupt = 0;
		}
	} else {
		if (pin >= GPIO_XLNX_AXI_PINS_PER_CHANNEL) {
			return -EINVAL;
		}

		if (flags & GPIO_DIR_OUT) {
			dev_data->pin_dir |= (1 << pin);
		} else {
			dev_data->pin_dir &= ~(1 << pin);
		}

		if (flags & GPIO_POL_INV) {
			dev_data->invert_mask |= (1 << pin);
		} else {
			dev_data->invert_mask &= ~(1 << pin);
		}

		if (flags & GPIO_INT) {
			dev_data->int_mask |= (1 << pin);
			dev_data->use_interrupt = 1;
		} else {
			dev_data->int_mask &= ~(1 << pin);
			if (dev_data->int_mask == 0x00000000) {
				dev_data->use_interrupt = 0;
			}
		}
	}

	sys_write32(~(dev_data->pin_dir), dev_conf->base_addr + dev_conf->tri_reg_offset);
	dev_data->last_data = sys_read32(dev_conf->base_addr + dev_conf->data_reg_offset)
		^ (dev_data->invert_mask & ~dev_data->pin_dir);

	return 0;
}

static int gpio_xlnx_axi_write(
	struct device *dev,
	int access_op,
	u32_t pin,
	u32_t value)
{
	struct gpio_xlnx_axi_dev_cfg 	*dev_conf = DEV_CFG(dev);
	struct gpio_xlnx_axi_dev_data	*dev_data = DEV_DATA(dev);

	if (pin >= GPIO_XLNX_AXI_PINS_PER_CHANNEL) {
		return -EINVAL;
	}

	if (access_op == GPIO_ACCESS_BY_PORT) {
		dev_data->last_data &= ~dev_data->pin_dir;
		dev_data->last_data |= ((value ^ dev_data->invert_mask) & dev_data->pin_dir);
	} else {
		if (((dev_data->pin_dir >> pin) & 0x1) == 0) {
			return -EINVAL;
		}
		dev_data->last_data &= ~(1 << pin);
		dev_data->last_data |= (((value & 0x1) ^ ((dev_data->invert_mask >> pin) & 0x1)) << pin);
	}

	sys_write32(dev_data->last_data, dev_conf->base_addr + dev_conf->data_reg_offset);

	return 0;
}

static int gpio_xlnx_axi_read(
	struct device *dev,
	int access_op,
	u32_t pin,
	u32_t *value)
{
	struct gpio_xlnx_axi_dev_cfg 	*dev_conf = DEV_CFG(dev);
	struct gpio_xlnx_axi_dev_data	*dev_data = DEV_DATA(dev);
	u32_t							reg_val   = 0;

	if (pin >= GPIO_XLNX_AXI_PINS_PER_CHANNEL) {
		return -EINVAL;
	}

	reg_val  = sys_read32(dev_conf->base_addr + dev_conf->data_reg_offset);
	reg_val ^= dev_data->invert_mask;
	reg_val &= ~dev_data->pin_dir;

	dev_data->last_data &= dev_data->pin_dir;
	dev_data->last_data |= reg_val;

	if (access_op == GPIO_ACCESS_BY_PORT) {
		*value = dev_data->last_data;
	} else {
		*value = (dev_data->last_data >> pin) & 0x1;
	}

	return 0;
}

static int gpio_xlnx_axi_manage_callback(
	struct device *dev,
	struct gpio_callback *callback,
	bool set)
{
	struct gpio_xlnx_axi_dev_cfg  *dev_conf = DEV_CFG(dev);
	struct gpio_xlnx_axi_dev_data *dev_data = DEV_DATA(dev);

	if (dev_conf->supp_interrupt == 0) {
		return -ENOTSUP;
	}

	return gpio_manage_callback(&dev_data->callbacks, callback, set);
}

static int gpio_xlnx_axi_enable_callback(
	struct device *dev,
	int access_op,
	u32_t pin)
{
	struct gpio_xlnx_axi_dev_cfg  	*dev_conf = DEV_CFG(dev);
	struct gpio_xlnx_axi_dev_data 	*dev_data = DEV_DATA(dev);

	if (dev_conf->supp_interrupt == 0) {
		return -ENOTSUP;
	}

	if (pin >= GPIO_XLNX_AXI_PINS_PER_CHANNEL) {
		return -EINVAL;
	}

	if (access_op == GPIO_ACCESS_BY_PORT) {
		dev_data->callback_mask = 0xFFFFFFFF;
	} else {
		dev_data->callback_mask |= (1 << pin);
	}

	sys_write32(GPIO_XLNX_AXI_CH1_INT_ENABLE, dev_conf->base_addr + dev_conf->ip_ier_reg_offset);
	return 0;
}

static int gpio_xlnx_axi_disable_callback(
	struct device *dev,
	int access_op,
	u32_t pin)
{
	struct gpio_xlnx_axi_dev_cfg  	*dev_conf = DEV_CFG(dev);
	struct gpio_xlnx_axi_dev_data 	*dev_data = DEV_DATA(dev);

	if (dev_conf->supp_interrupt == 0) {
		return -ENOTSUP;
	}

	if (pin >= GPIO_XLNX_AXI_PINS_PER_CHANNEL) {
		return -EINVAL;
	}

	if (access_op == GPIO_ACCESS_BY_PORT) {
		dev_data->callback_mask = 0x00000000;
	} else {
		dev_data->callback_mask &= ~(1 << pin);
	}

	if (dev_data->callback_mask == 0x00000000)
	{
		sys_write32(GPIO_XLNX_AXI_CH1_INT_DISABLE, dev_conf->base_addr + dev_conf->ip_ier_reg_offset);
	}

	return 0;
}

static const struct gpio_driver_api gpio_xlnx_axi_driver_api = {
	.config              = gpio_xlnx_axi_config,
	.write               = gpio_xlnx_axi_write,
	.read                = gpio_xlnx_axi_read,
	.manage_callback     = gpio_xlnx_axi_manage_callback,
	.enable_callback     = gpio_xlnx_axi_enable_callback,
	.disable_callback    = gpio_xlnx_axi_disable_callback,
};

static int gpio_xlnx_axi_init(struct device *dev)
{
	struct gpio_xlnx_axi_dev_cfg  	*dev_conf = DEV_CFG(dev);
	struct gpio_xlnx_axi_dev_data 	*dev_data = DEV_DATA(dev);

	dev_data->callback_mask     = 0;
	dev_data->int_mask          = 0;
	dev_data->invert_mask       = 0;
	dev_data->last_data         = 0;
	dev_data->pin_dir           = 0;
	dev_data->use_interrupt     = 0;

	dev_conf->config_func(dev);

	return 0;
}

#ifdef DT_INST_0_XLNX_AXI_GPIO

static const struct gpio_xlnx_axi_dev_cfg gpio_xlnx_axi_dev_cfg_0 = {
	#ifdef DT_INST_0_XLNX_AXI_GPIO_IRQ_0
	.supp_interrupt    = 1,
	#else
	.supp_interrupt    = 0,
	#endif /* DT_INST_0_XLNX_AXI_GPIO_IRQ_0 */
	.base_addr         = DT_INST_0_XLNX_AXI_GPIO_BASE_ADDRESS,
	.data_reg_offset   = GPIO_XLNX_AXI_GPIO_DATA_REG_OFFSET,
	.tri_reg_offset    = GPIO_XLNX_AXI_GPIO_TRI_REG_OFFSET,
	.gier_reg_offset   = GPIO_XLNX_AXI_GPIO_GIER_REG_OFFSET,
	.ip_ier_reg_offset = GPIO_XLNX_AXI_GPIO_IP_IER_REG_OFFSET,
	.ip_isr_reg_offset = GPIO_XLNX_AXI_GPIO_IP_ISR_REG_OFFSET,
	.config_func       = gpio_xlnx_axi_config_interrupt
};

static struct gpio_xlnx_axi_dev_data gpio_xlnx_axi_dev_data_0;

DEVICE_AND_API_INIT(
	gpio_xlnx_axi_0,
	DT_INST_0_XLNX_AXI_GPIO_LABEL,
	gpio_xlnx_axi_init,
	&gpio_xlnx_axi_dev_data_0, &gpio_xlnx_axi_dev_cfg_0,
	POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
	&gpio_xlnx_axi_driver_api);

#endif /* DT_INST_0_XLNX_AXI_GPIO */

#ifdef DT_INST_1_XLNX_AXI_GPIO

static const struct gpio_xlnx_axi_dev_cfg gpio_xlnx_axi_dev_cfg_1 = {
	#ifdef DT_INST_1_XLNX_AXI_GPIO_IRQ_0
	.supp_interrupt    = 1,
	#else
	.supp_interrupt    = 0,
	#endif /* DT_INST_1_XLNX_AXI_GPIO_IRQ_0 */
	.base_addr         = DT_INST_1_XLNX_AXI_GPIO_BASE_ADDRESS,
	.data_reg_offset   = GPIO_XLNX_AXI_GPIO_DATA_REG_OFFSET,
	.tri_reg_offset    = GPIO_XLNX_AXI_GPIO_TRI_REG_OFFSET,
	.gier_reg_offset   = GPIO_XLNX_AXI_GPIO_GIER_REG_OFFSET,
	.ip_ier_reg_offset = GPIO_XLNX_AXI_GPIO_IP_IER_REG_OFFSET,
	.ip_isr_reg_offset = GPIO_XLNX_AXI_GPIO_IP_ISR_REG_OFFSET,
	.config_func       = gpio_xlnx_axi_config_interrupt
};

static struct gpio_xlnx_axi_dev_data gpio_xlnx_axi_dev_data_1;

DEVICE_AND_API_INIT(
	gpio_xlnx_axi_1,
	DT_INST_1_XLNX_AXI_GPIO_LABEL,
	gpio_xlnx_axi_init,
	&gpio_xlnx_axi_dev_data_1, &gpio_xlnx_axi_dev_cfg_1,
	POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
	&gpio_xlnx_axi_driver_api);

#endif /* DT_INST_1_XLNX_AXI_GPIO */

static void gpio_xlnx_axi_config_interrupt(struct device* dev)
{
	struct gpio_xlnx_axi_dev_cfg *dev_conf = DEV_CFG(dev);

	#if defined(DT_INST_0_XLNX_AXI_GPIO) && defined(DT_INST_0_XLNX_AXI_GPIO_IRQ_0)
	if (dev_conf->base_addr == DT_INST_0_XLNX_AXI_GPIO_BASE_ADDRESS) {
		IRQ_CONNECT(
			DT_INST_0_XLNX_AXI_GPIO_IRQ_0,
			DT_INST_0_XLNX_AXI_GPIO_IRQ_0_PRIORITY,
			gpio_xlnx_axi_isr,
			DEVICE_GET(gpio_xlnx_axi_0),
			0);
		irq_enable(DT_INST_0_XLNX_AXI_GPIO_IRQ_0);
		sys_write32(GPIO_XLNX_AXI_GLOBAL_INT_ENABLE, dev_conf->base_addr + dev_conf->gier_reg_offset);
	}
	#endif /* CONFIG_GPIO_XLNX_AXI_CH1 && CONFIG_GPIO_XLNX_AXI_CH1_USE_INTERRUPT */

	#if defined(DT_INST_1_XLNX_AXI_GPIO) && defined(DT_INST_1_XLNX_AXI_GPIO_IRQ_0)
	if (dev_conf->base_addr == DT_INST_1_XLNX_AXI_GPIO_BASE_ADDRESS) {
		IRQ_CONNECT(
			DT_INST_1_XLNX_AXI_GPIO_IRQ_0,
			DT_INST_1_XLNX_AXI_GPIO_IRQ_0_PRIORITY,
			gpio_xlnx_axi_isr,
			DEVICE_GET(gpio_xlnx_axi_1),
			0);
		irq_enable(DT_INST_1_XLNX_AXI_GPIO_IRQ_0);
		sys_write32(GPIO_XLNX_AXI_GLOBAL_INT_ENABLE, dev_conf->base_addr + dev_conf->gier_reg_offset);
	}
	#endif /* CONFIG_GPIO_XLNX_AXI_CH2 && CONFIG_GPIO_XLNX_AXI_CH2_USE_INTERRUPT */
}
