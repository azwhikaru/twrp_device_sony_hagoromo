/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/nfc/rcs730.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_gpio_base.h>
#include <rcs730_board_devs.h>

/***********************/
/*@ platform_data      */
/***********************/
static struct rcs730_platform_data rcs730_pdata = {
      .rfdet_gpio = CXDNFC_RFDET,
      .irq_gpio = CXDNFC_IRQ,
};

static struct i2c_board_info __initdata i2c_devs[]={
    { I2C_BOARD_INFO("rcs730", CXDNFC_I2C_ADR),
      .platform_data = &rcs730_pdata,
    },
};

static int __init mt_rcs730_i2c_devs_init(void)
{
    printk("rcs730 %s()\n", __func__);    
    int err;

    err = i2c_register_board_info(CXDNFC_I2C_CH, i2c_devs, ARRAY_SIZE(i2c_devs));
    if (err) {
        printk("%s() call i2c_register_board_info() fail:%d\n", __func__, err);
    }     
    return 0;
}
arch_initcall(mt_rcs730_i2c_devs_init);
