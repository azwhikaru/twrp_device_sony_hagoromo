/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* Cypress touch panel support for ICX
 *
 * Copyright 2015 Sony Corporation
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>

#include <linux/input/ts_icx.h>
#include <linux/input/synaptics_icx.h>

#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>


#define TOUCH_XRST_GPIO			GPIO239

static int ts_power(int on)
{
	if (on) {
		mt_set_gpio_out(TOUCH_XRST_GPIO, GPIO_OUT_ZERO);
		msleep(35);
		mt_set_gpio_out(TOUCH_XRST_GPIO, GPIO_OUT_ONE);
		msleep(100);
	} else {
		mt_set_gpio_out(TOUCH_XRST_GPIO, GPIO_OUT_ZERO);
	}
	return 0;
}

static int ts_init(void)
{
	/* XRST */
	mt_set_gpio_mode(TOUCH_XRST_GPIO, GPIO_MODE_00);
	mt_set_gpio_dir(TOUCH_XRST_GPIO, GPIO_DIR_OUT);

	/* power on */
	ts_power(1);
    
	return 0;
}

struct ts_icx_platform_data synaptics_icx_ts_pdata = {
	.xrst_gpio = TOUCH_XRST_GPIO,
	.xint_eint = SYNAPTICS_ICX_EINT,
	.irqflags  = EINTF_TRIGGER_FALLING,

	.min_x = 0,
	.min_y = 0,
	.max_x = 480 * 2 - 1,
	.max_y = 854 * 2 - 1,

	.init = ts_init,
};

