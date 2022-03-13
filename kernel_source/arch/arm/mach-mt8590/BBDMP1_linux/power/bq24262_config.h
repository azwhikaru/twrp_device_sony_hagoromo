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
#define	BQ24262_WMPORT_PMIC_VBAT_CHANNEL	(7) /* MT6323.GPS */
#define	BQ24262_WMPORT_PMIC_VBAT_SAMPLES	(4) /* Samples 4 times. */
#define	BQ24262_WMPORT_PMIC_THR_SENSE_CHANNEL	(5) /* MT6323.ACCDET (THR_SENSE2) */
#define	BQ24262_WMPORT_PMIC_THR_SENSE_SAMPLES	(4) /* Samples 4 times. */

#define	BQ24262_BATTERY_FULLY_VOLT_MV		(4200)
#define	BQ24262_BATTERY_SOFTY_VOLT_MV		(4100)
#define	BQ24262_BATTERY_SAFE_MV			(3500)

/*! @note these are temporal value.
          Pull downed by NTC thermistor, higher temperature makes voltage low.
*/
							/*  Celsius, voltage  */
#define	BQ24262_BATTERY_TEMP_HI_ENTER_MV	(312)	/*!<  0 deg, 0.3112V  */
#define	BQ24262_BATTERY_TEMP_HI_EXIT_MV		(343)   /*!<  3 deg, 0.3425V  */
#define	BQ24262_BATTERY_TEMP_LO_EXIT_MV		(910)   /*!< 42 deg, 0.9107V  */
#define	BQ24262_BATTERY_TEMP_LO_ENTER_MV	(953)	/*!< 45 deg, 0.9538V  */

#endif /* (!defined(BQ24262_PLATFORM_H_INCLUDED)) */
