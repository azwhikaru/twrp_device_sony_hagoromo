/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* bq24262_config.h: target specific definitions.
 *
 * Copyright 2015 Sony Corporation.
 * Author: Sony Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if (!defined(BQ24262_CONFIG_H_INCLUDED))
#define BQ24262_CONFIG_H_INCLUDED

#include "mach/eint.h"
#include "mach/mt_gpio.h"
#include "mach/mt_gpio_base.h"
#include "mach/bq24262_platform.h"

#define	BQ24262_WMPORT_DRIVER_NAME		"bq24262_wmport"

#define	BQ24262_WMPORT_I2C_BUS			(2)
#define	BQ24262_WMPORT_I2C_ADDRESS		(0xd6 /*!< bitrev8(0x6b) */)
#define	BQ24262_WMPORT_I2C_TIMING		(100 /* Bus clock frequency in kHz */)

#define	BQ24262_WMPORT_CHG_SUSPEND_GPIO		(BQ24262_WMPORT_GPIO_ENCODE(GPIO21))
#define	BQ24262_WMPORT_CHG_SUSPEND_OUT		BQ24262_WMPORT_CHG_SUSPEND_OUT_NATIVE
#define	BQ24262_WMPORT_CHG_XSTAT_EINT		(154) /*!< According to xlsx sheet, not data sheet. data sheet says 156. */
#define	BQ24262_WMPORT_CHG_XSTAT_GPIO		(BQ24262_WMPORT_GPIO_ENCODE(GPIO6))
#define	BQ24262_WMPORT_DC_XDET_EINT		(4)
#define	BQ24262_WMPORT_DC_XDET_GPIO		(BQ24262_WMPORT_GPIO_ENCODE(GPIO26))
#define	BQ24262_WMPORT_VBUS_XDET_EINT		(89)
#define	BQ24262_WMPORT_VBUS_XDET_GPIO		(BQ24262_WMPORT_GPIO_ENCODE(GPIO116))
#define	BQ24262_WMPORT_FORCE_DCIN_GPIO		(BQ24262_WMPORT_GPIO_ENCODE(GPIO13))
#define	BQ24262_WMPORT_SYS_WAK_STAT_GPIO	(BQ24262_WMPORT_GPIO_ENCODE(GPIO5))

/*! @note MTK defines in arch/arm/mach-mt8590/${target}/power/cust_battery_meter.h */
#define	BQ24262_WMPORT_PMIC_VBAT_CHANNEL	(7) /* MT6323.BATSNS */
#define	BQ24262_WMPORT_PMIC_VBAT_SAMPLES	(4) /* Samples 4 times. */
#define	BQ24262_WMPORT_PMIC_THR_SENSE_CHANNEL	(5) /* MT6323.BATON1 */
#define	BQ24262_WMPORT_PMIC_THR_SENSE_SAMPLES	(4) /* Samples 4 times. */

/* Scale factors to convert from MT6323 AUXADC.BATSNS read value to battery voltage in uV. */
#define	BQ24262_WMPORT_BATTERY_SCALE_TO_UV_MUL		(4L * 1800L * 1000L)
#define	BQ24262_WMPORT_BATTERY_SCALE_TO_UV_DIV		(32768L)
#define	BQ24262_WMPORT_BATTERY_SCALE_TO_UV_OFFSET	(0L)

						/*   mV uV */
#define	BQ24262_BATTERY_FULLY_CHARGE_UV		(4200000)
#define	BQ24262_BATTERY_WEAKLY_CHARGE_UV	(4100000)
#define	BQ24262_BATTERY_RESUME_CHARGE_UV	(3900000)
#define	BQ24262_BATTERY_SAFE_UV				(3800000)
#define	BQ24262_BATTERY_SAFE_ABSOLUTELY_UV	(3900000)

/*! @note these are temporal value.
          Pull downed by NTC thermistor, higher temperature makes voltage low.
*/
													/*  Celsius, voltage  */
#define	BQ24262_BATTERY_TEMP_HI_ENTER_RAW	((0x1622)*BQ24262_WMPORT_PMIC_THR_SENSE_SAMPLES)	/*!< 45 deg, 0.3112V  */
#define	BQ24262_BATTERY_TEMP_HI_EXIT_RAW	((0x185B)*BQ24262_WMPORT_PMIC_THR_SENSE_SAMPLES)	/*!< 42 deg, 0.3425V  */
//#define	BQ24262_BATTERY_TEMP_LO_EXIT_RAW	((0x40C2)*BQ24262_WMPORT_PMIC_THR_SENSE_SAMPLES)	/*!<  3 deg, 0.9107V  */
//#define	BQ24262_BATTERY_TEMP_LO_ENTER_RAW	((0x43D3)*BQ24262_WMPORT_PMIC_THR_SENSE_SAMPLES)	/*!<  0 deg, 0.9538V  */
#define	BQ24262_BATTERY_TEMP_LO_EXIT_RAW	((0x45C6)*BQ24262_WMPORT_PMIC_THR_SENSE_SAMPLES)	/*!< -2 deg, 0.9812V  */
#define	BQ24262_BATTERY_TEMP_LO_ENTER_RAW	((0x4888)*BQ24262_WMPORT_PMIC_THR_SENSE_SAMPLES)	/*!< -5 deg, 1.0200V  */

/*! control VINDPM (VBUS ANTICOLLAPSE) condition constant.
    Condition, Action, State
    (VINDPM_LO && (battery_mv <  HI_ENTER)), VINDPM=LO_PERCENT, VINDPM_LO
    (VINDPM_LO && (battery_mv >= HI_ENTER)), VINDPM=HI_PERCENT, VINDPM_HI
    (VINDPM_HI && (battery_mv >= LO_ENTER)), VINDPM=HI_PERCENT, VINDPM_HI
    (VINDPM_HI && (battery_mv <  LO_ENTER)), VINDPM=LO_PERCENT, VINDPM_LO
*/
						/*   mV uV */
#define	BQ24262_VINDPM_LO_ENTER_BATTERY_UV	(3750000)	/*!< Battery voltage to enter VINDPM low mode.  */
#define	BQ24262_VINDPM_HI_ENTER_BATTERY_UV	(3900000)	/*!< Battery voltage to enter VINDPM high mode.  */

/*! VINDPM LO/HI percent constants, represent VBUS ANTICOLLAPSE voltage.
*/
#define	BQ24262_VINDPM_LO_PERCENT	(0)	/*!< VINDPM low mode percent=1.00  (= 1.00 +  BQ24262_VINDPM_LO_PERCENT * 0.01), 4200mV */
#define	BQ24262_VINDPM_HI_PERCENT	(8)	/*!< VINDPM high mode percent=1.08 (= 1.00 +  BQ24262_VINDPM_HI_PERCENT * 0.01), 4536mV */

/*! max charge time(seconds). */
#define	BQ24262_WMPORT_CHARGE_TIME_MAX_SEC	(9*3600L)	/*!< 9 hour. */

#endif /* (!defined(BQ24262_PLATFORM_H_INCLUDED)) */
