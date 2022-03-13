/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* Touch panel support for ICX
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
#include <linux/input/himax_hx8526_icx.h>

#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>


#define TOUCH_I2C_NUM 1

#define	ICX_TS_NVP_MASK			0x00000007
#define	ICX_TS_NVP_SYNAPTICS	0x00000000
#define	ICX_TS_NVP_HIMAX_HX8526	0x00000001

extern unsigned long icx_pm_helper_sysinfo;

extern struct ts_icx_platform_data synaptics_icx_ts_pdata;
extern struct ts_icx_platform_data himax_hx8526_icx_ts_pdata;

static struct i2c_board_info __initdata i2c_touch_boardinfo[] = {
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_ICX
	{
		I2C_BOARD_INFO(SYNAPTICS_ICX_NAME, SYNAPTICS_ICX_I2C_SLAVEADDRESS),
		.platform_data = &synaptics_icx_ts_pdata,
		.irq = SYNAPTICS_ICX_EINT,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_HIMAX_HX8526_ICX
	{
		I2C_BOARD_INFO(HIMAX_HX8526_ICX_NAME, HIMAX_HX8526_ICX_I2C_SLAVEADDRESS),
		.platform_data = &himax_hx8526_icx_ts_pdata,
		.irq = HIMAX_HX8526_ICX_EINT,
	},
#endif
};

int __init icx_ts_init(void)
{
	unsigned long sysinfo_ts;
	int id;
	struct ts_icx_platform_data *pdata;
	int ret;

	sysinfo_ts = icx_pm_helper_sysinfo & ICX_TS_NVP_MASK;
	if (sysinfo_ts == ICX_TS_NVP_SYNAPTICS)
		id = 0;
	else if (sysinfo_ts == ICX_TS_NVP_HIMAX_HX8526)
		id = 1;
	else
		return -ENODEV;

	pdata = i2c_touch_boardinfo[id].platform_data;
	if (pdata->init)
		pdata->init();

	ret = i2c_register_board_info(TOUCH_I2C_NUM, &i2c_touch_boardinfo[id], 1);

	return ret;
}

arch_initcall(icx_ts_init);
