/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* ICX cradle driver.
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
#include <mach/icx_cradle.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_base.h>

//gpio eint ad settings
#define GPIO_NUM_ACC_DET (GPIO45 | 0x80000000)
#define EINT_NUM_ACC_DET 27
#define GPIO_NUM_UART_TX (GPIO82 | 0x80000000)
#define GPIO_NUM_UART_RX (GPIO81 | 0x80000000)
#define AUX_IN2 2
#define AUX_IN3 3

static struct icx_cradle_driver_data icx_cradle_driver_data =
{
    .acc_det_gpio = GPIO_NUM_ACC_DET,
    .acc_det_eint = EINT_NUM_ACC_DET,
    .uart_tx_gpio = GPIO_NUM_UART_TX,
    .uart_rx_gpio = GPIO_NUM_UART_RX,
    .adc_ch_acc1 = AUX_IN2,
    .adc_ch_acc2 = AUX_IN3,
};


static struct platform_device cradle_dev = {
    .name = ICX_CRADLE_NAME,
    .dev = 
    {
        .platform_data = &(icx_cradle_driver_data),
    },
};

int __init icx_cradle_init(void)
{
    int ret;

    ret = platform_device_register(&cradle_dev);
    if (ret != 0) {
        pr_err("%s: Fail to register device. name=%s, ret=%d\n", __func__, cradle_dev.name, ret);
    }
    return 0;
}

arch_initcall(icx_cradle_init);

