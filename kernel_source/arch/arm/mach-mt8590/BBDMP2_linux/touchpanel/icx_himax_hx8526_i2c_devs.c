/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* Himax touch panel support for ICX
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
#include <linux/input/himax_hx8526_icx.h>

#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>


#define TOUCH_XRST_GPIO			GPIO239
#define TOUCH_XINT_GPIO			GPIO27

static int ts_power(int on)
{
	if (on) {
		mt_set_gpio_out(TOUCH_XRST_GPIO, GPIO_OUT_ONE);
		msleep(20);
		mt_set_gpio_out(TOUCH_XRST_GPIO, GPIO_OUT_ZERO);
		msleep(20);
		mt_set_gpio_out(TOUCH_XRST_GPIO, GPIO_OUT_ONE);
		msleep(20);
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

	/* XINT */
	mt_set_gpio_mode(TOUCH_XINT_GPIO, GPIO_MODE_00);
	mt_set_gpio_dir(TOUCH_XINT_GPIO, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(TOUCH_XINT_GPIO, GPIO_PULL_DISABLE);

	msleep(10);

	/* power on */
	ts_power(1);
}

struct ts_icx_platform_data himax_hx8526_icx_ts_pdata = {
	.xrst_gpio = TOUCH_XRST_GPIO,
	.xint_gpio = TOUCH_XINT_GPIO,
	.irqflags  = EINTF_TRIGGER_FALLING,

	.min_x = 0,
	.min_y = 0,
	.max_x = 480 * 2 - 1,
	.max_y = 800 * 2 - 1,

	.init = ts_init,
};

