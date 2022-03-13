/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* Cypress touch panel support for ICX
 *
 * Copyright 2016 Sony Corporation
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
#include <mach/mt_gpio.h>
#include <linux/platform_data/icx_si4708.h>

#define RADIO_I2C_NUM			(2)
#define I2C_SLAVE_SI470X_ADDR	(0x20 /*!< bitrev8(0x10) */) 
#define	I2C_SLAVE_SI470X_NAME   "Si4708icx"
#define RADIO_XRST_GPIO         GPIO278

static SI4708ICX_PLATFORM_RESOURCE si4708icx_platform_resource = {
    .port_reset = RADIO_XRST_GPIO,
};

static struct i2c_board_info __initdata i2c_radio_boardinfo[] = {
	{
		I2C_BOARD_INFO(I2C_SLAVE_SI470X_NAME, (I2C_SLAVE_SI470X_ADDR >> 1)),
		.platform_data = &si4708icx_platform_resource,
	},
};

int __init icx_radio_init(void)
{
    int rv = 0;

	rv = i2c_register_board_info(RADIO_I2C_NUM, i2c_radio_boardinfo, ARRAY_SIZE(i2c_radio_boardinfo));
    if (rv) {
        printk("%s() call i2c_register_board_info() fail:%d\n", __func__, rv);
    }
	return 0;
}

arch_initcall(icx_radio_init);
