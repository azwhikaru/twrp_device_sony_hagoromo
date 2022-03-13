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
#include <linux/nfc/cxd224x.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_gpio_base.h>
#include <nfc_board_devs.h>
#include <cust_eint.h>

/***********************/
/*@ platform_data      */
/***********************/
static struct cxd224x_platform_data cxd224x_pdata = {
      .xint_gpio = CXDNFC_IRQ,
      .pon_gpio = CXDNFC_PON,
      .rst_gpio = CXDNFC_RST,
      .en_gpio = CXDNFC_LEN,
};

static struct i2c_board_info __initdata i2c_devs[]={
    { I2C_BOARD_INFO("cxd224x-i2c", CXDNFC_I2C_ADR),
      .platform_data = &cxd224x_pdata,
//      .irq = CXDNFC_IRQ
      .irq = CUST_EINT_IRQ_SONY_DMP_NFC_NUM 
    },
};

static int __init mt_nfc_i2c_devs_init(void)
{
    printk("cxd224x %s()\n", __func__);    
    int err;

    err = i2c_register_board_info(CXDNFC_I2C_CH, i2c_devs, ARRAY_SIZE(i2c_devs));
    if (err) {
        printk("%s() call i2c_register_board_info() fail:%d\n", __func__, err);
    }     
    return 0;
}
arch_initcall(mt_nfc_i2c_devs_init);
