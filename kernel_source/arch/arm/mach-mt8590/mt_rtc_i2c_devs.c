/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* RTC BU9873 support for ICX
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
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/rtc/bu9873.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_base.h>

#define RTC_INTRB_GPIO GPIO24
#define RTC_INTRB_EINT 2

#define RTC_I2C_SLAVEADDRESS 0x64
#define RTC_I2C_NUM 2
#define RTC_I2C_TIMING 100

/***********************/
/*@ platform_data      */
/***********************/
static struct bu9873_platform_data bu9873_platform_data = {
    .rtc_intrb_gpio =  RTC_INTRB_GPIO,
    .rtc_intrb_eint = RTC_INTRB_EINT,
    .rtc_i2c_timing = RTC_I2C_TIMING,
};

static struct i2c_board_info __initdata i2c_rtc_board_info[]={
    { I2C_BOARD_INFO(BU9873_NAME, (RTC_I2C_SLAVEADDRESS>>1)),
    .platform_data = &bu9873_platform_data,
    },
};

static int __init mt_rtc_i2c_devs_init(void)
{
    int err;
    
    err = i2c_register_board_info(RTC_I2C_NUM, i2c_rtc_board_info, ARRAY_SIZE(i2c_rtc_board_info));
    if (err) {
        printk("%s() call i2c_register_board_info() fail:%d\n", __func__, err);
    }     
    return 0;
}
arch_initcall(mt_rtc_i2c_devs_init);
