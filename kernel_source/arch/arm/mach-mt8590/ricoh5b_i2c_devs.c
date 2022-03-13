/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* ICX ricoh5b driver.
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
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <mach/mt_gpio_base.h>
#include <linux/platform_data/ricoh5b_tuner.h>

#include "BBMini2_linux/dct/cust_gpio_usage.h"

#define RICOH5B_ICX_NAME          "ricoh5b"
#define RICOH5B_I2C_SLAVEADDRESS  0x64
#define RICOH5B_I2C_NUM           1
#define RICOH5B_CE_GPIO           GPIO_TUNER_POWER_CE_PIN
#define RICOH5B_I2C_RDS_GPIO      GPIO_TUNER_I2C_RDS_PIN
#define RICOH5B_VDD_GPIO          GPIO_TUNER_VDD_POWER_PIN

static RICOH5B_PLATFORM_RESOURCE ricoh5b_platform_resource = {
    .ce      = RICOH5B_CE_GPIO,
    .i2c_rds = RICOH5B_I2C_RDS_GPIO,
    .vdd     = RICOH5B_VDD_GPIO,
};

static struct i2c_board_info __initdata i2c_ricoh5b_boardinfo[]={
    {
        I2C_BOARD_INFO(RICOH5B_ICX_NAME, RICOH5B_I2C_SLAVEADDRESS),
   		.platform_data = &ricoh5b_platform_resource,
    },
};

int __init ricoh5b_i2c_init(void)
{
    int ret;

    ret = i2c_register_board_info(RICOH5B_I2C_NUM, i2c_ricoh5b_boardinfo, ARRAY_SIZE(i2c_ricoh5b_boardinfo));
    if (ret != 0) {
        pr_err("%s: Fail to i2c_register_board. , ret=%d\n", __func__, ret);
    }
    
    return 0;
}

arch_initcall(ricoh5b_i2c_init);

