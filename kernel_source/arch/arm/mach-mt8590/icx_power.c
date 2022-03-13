/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* ICX platform power management initializer and cross device controls.
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
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <mach/icx_pm_helper.h>
#include <linux/i2c.h>
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
#include <mach/bq24262_platform.h>
#include <bq24262_config.h>
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */


/*!< SPM and EINT status holder. */
struct icx_pm_helper_spm_stat	icx_pm_helper_spm_stat;

/*! ICX PM helper global semaphore. */
DEFINE_SEMAPHORE(icx_pm_helper_global_sem);

struct platform_device	icx_pm_helper_device = {
	.name = "icx_pm_helper",
	.id = -1,	/* Do not use. */
	/* Any other members are not used.
	   This is not hardware device.
	*/
};

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
/* @note: place bq24262_wmport_platform into .data. */
static struct bq24262_wmport_platform_data bq24262_wmport_platform_def = {
	.chg_i2c_timing =      BQ24262_WMPORT_I2C_TIMING,
	.chg_suspend_gpio =    BQ24262_WMPORT_CHG_SUSPEND_GPIO,
	.chg_xstat_eint =      BQ24262_WMPORT_CHG_XSTAT_EINT,
	.chg_xstat_gpio =      BQ24262_WMPORT_CHG_XSTAT_GPIO,
	.dc_xdet_eint =        BQ24262_WMPORT_DC_XDET_EINT,
	.dc_xdet_gpio =        BQ24262_WMPORT_DC_XDET_GPIO,
	.vbus_xdet_eint =      BQ24262_WMPORT_VBUS_XDET_EINT,
	.vbus_xdet_gpio =      BQ24262_WMPORT_VBUS_XDET_GPIO,
	.force_dcin_gpio =     BQ24262_WMPORT_FORCE_DCIN_GPIO,
	.sys_wak_stat_gpio =   BQ24262_WMPORT_SYS_WAK_STAT_GPIO,

	.pmic_vbat_channel =      BQ24262_WMPORT_PMIC_VBAT_CHANNEL,
	.pmic_vbat_samples =      BQ24262_WMPORT_PMIC_VBAT_SAMPLES,
	.pmic_thr_sense_channel = BQ24262_WMPORT_PMIC_THR_SENSE_CHANNEL,
	.pmic_thr_sense_samples = BQ24262_WMPORT_PMIC_THR_SENSE_SAMPLES,

	.battery_scale_to_uv_mul =    BQ24262_WMPORT_BATTERY_SCALE_TO_UV_MUL,
	.battery_scale_to_uv_div =    BQ24262_WMPORT_BATTERY_SCALE_TO_UV_DIV,
	.battery_scale_to_uv_offset = BQ24262_WMPORT_BATTERY_SCALE_TO_UV_OFFSET,

	.battery_fully_charge_uv =    BQ24262_BATTERY_FULLY_CHARGE_UV,
	.battery_weakly_charge_uv =   BQ24262_BATTERY_WEAKLY_CHARGE_UV,
	.battery_resume_charge_uv =   BQ24262_BATTERY_RESUME_CHARGE_UV,
	.battery_safe_uv =	      BQ24262_BATTERY_SAFE_UV,
	.battery_safe_absolutely_uv = BQ24262_BATTERY_SAFE_ABSOLUTELY_UV,
	.battery_temp_lo_enter_raw = BQ24262_BATTERY_TEMP_LO_ENTER_RAW,
	.battery_temp_lo_exit_raw =  BQ24262_BATTERY_TEMP_LO_EXIT_RAW,
	.battery_temp_hi_exit_raw =  BQ24262_BATTERY_TEMP_HI_EXIT_RAW,
	.battery_temp_hi_enter_raw = BQ24262_BATTERY_TEMP_HI_ENTER_RAW,
	.vindpm_lo_enter_battery_uv = BQ24262_VINDPM_LO_ENTER_BATTERY_UV,
	.vindpm_hi_enter_battery_uv = BQ24262_VINDPM_HI_ENTER_BATTERY_UV,
	.vindpm_lo = BQ24262_VINDPM_LO_PERCENT,
	.vindpm_hi = BQ24262_VINDPM_HI_PERCENT,
	.charge_time_max_sec =      BQ24262_WMPORT_CHARGE_TIME_MAX_SEC,
};

static struct i2c_board_info __initdata bq24262_wmport_i2c_devices[] = {
		/* Charger IC BQ24262 works with WM-PORT.
		   See section 8.5 Programming for I2C address.
		*/
	{	I2C_BOARD_INFO(BQ24262_WMPORT_DRIVER_NAME, (BQ24262_WMPORT_I2C_ADDRESS >> 1)),
		.platform_data = &bq24262_wmport_platform_def,
		.irq = BQ24262_WMPORT_CHG_XSTAT_EINT
	},
};
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */

int __init icx_power_init(void)
{	int	ret;

	spin_lock_init(&(icx_pm_helper_spm_stat.lock));
	
	ret = platform_device_register(&icx_pm_helper_device);
	if (ret != 0) {
		pr_err("%s: Fail to register device. name=%s, ret=%d\n", __func__, icx_pm_helper_device.name, ret);
	}
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
	/* always do */ {
		int i2c_bus = BQ24262_WMPORT_I2C_BUS;
		printk("%s: Register I2C device. i2c_bus=%d, addr(rev.shr)=0x%x, type=\"%s\"\n",
			__func__,
			i2c_bus,
			bq24262_wmport_i2c_devices[0].addr,
			bq24262_wmport_i2c_devices[0].type
		);
		ret = i2c_register_board_info(i2c_bus, bq24262_wmport_i2c_devices,
			ARRAY_SIZE(bq24262_wmport_i2c_devices)
		);
		if (ret != 0) {
			pr_err("%s: Fail to register device. name=%s, ret=%d\n",
				__func__, bq24262_wmport_i2c_devices[0].type, ret
			);
		}
	}
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */
	return 0;
}

arch_initcall(icx_power_init);

#if defined(CONFIG_ENABLE_ICX_KEY)
EXPORT_SYMBOL(icx_pm_helper_spm_stat);
#endif /* CONFIG_ENABLE_ICX_KEY */
