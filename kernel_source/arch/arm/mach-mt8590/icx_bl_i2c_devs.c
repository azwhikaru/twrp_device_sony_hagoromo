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
#include <linux/io.h>
#include <linux/delay.h>

#include <linux/platform_data/icx_lm3630_bl.h>

#include <mach/mt_gpio.h>

#define BL_I2C_NUM           1
#define BL_I2C_SLAVEADDRESS  0x36

#define BL_EN_GPIO         GPIO124
#define BL_PWM_NUM         0


static struct lm3630_bl_platform_data lm3630_pdata = {
    .blen_gpio = BL_EN_GPIO,
    .pwm_id = BL_PWM_NUM,
};

static struct i2c_board_info __initdata i2c_bl_boardinfo[]={
    {I2C_BOARD_INFO(LM3630_NAME, BL_I2C_SLAVEADDRESS),
      .platform_data = &lm3630_pdata,},
};

static int __init icx_bl_init(void)
{
    int rv = 0;
    rv = i2c_register_board_info(BL_I2C_NUM, i2c_bl_boardinfo, ARRAY_SIZE(i2c_bl_boardinfo));
    if (rv) {
        printk("%s() call i2c_register_board_info() fail:%d\n", __func__, rv);
    }     
    return 0;
}

arch_initcall(icx_bl_init);
