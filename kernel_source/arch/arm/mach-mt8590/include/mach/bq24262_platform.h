/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* bq24262_platform.h: platform common definitions.
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

#if (!defined(BQ24262_PLATFORM_H_INCLUDED))
#define BQ24262_PLATFORM_H_INCLUDED

/*! BQ24262 WM-PORT platform_data. */

#define	BQ24262_WMPORT_GPIO_ENCODE(gpio)	((gpio) | 0x80000000)
#define	BQ24262_WMPORT_GPIO_DECODE(gpio)	((gpio) & 0x7fffffff)
#define	BQ24262_WMPORT_INVALID_GPIO		(~0UL)
#define	BQ24262_WMPORT_INVALID_EINT		(~0U)

#define	BQ24262_WMPORT_FORCE_DCIN_AUTO		(0)
#define	BQ24262_WMPORT_FORCE_DCIN_FORCE		(1)

#define	BQ24262_WMPORT_CHG_SUSPEND_DRAW		(0)
#define	BQ24262_WMPORT_CHG_SUSPEND_HIZ		(1)

#define	BQ24262_WMPORT_SYS_WAK_STAT_BOOTING	(0)
#define	BQ24262_WMPORT_SYS_WAK_STAT_READY	(1)

#define	BQ24262_WMPORT_BATTERY_LEVELS		(12)

struct bq24262_wmport_battery_capacity_uv {
	long		battery_uv;
	int		capacity;
};

/*! Platform depended parameters.
    @note Specify GPIO number with encoded number.
*/
struct bq24262_wmport_platform_data {
	uint32_t	chg_i2c_timing;		/*!< Charger I2C bus frequency in kHz.	*/
	unsigned long	chg_suspend_gpio;	/*!< CHG_SUSPEND GPIO number.		*/
	unsigned int	chg_xstat_eint;		/*!< CHG_XSTAT EINT number.		*/
	unsigned int	chg_xstat_gpio;		/*!< CHG_XSTAT GPIO number.		*/
	unsigned int	dc_xdet_eint;		/*!< DC_XDET EINT number.		*/
	unsigned long	dc_xdet_gpio;		/*!< DC_XDET GPIO number.		*/
	unsigned int	vbus_xdet_eint;		/*!< VBUS_XDET EINT number.		*/
	unsigned long	vbus_xdet_gpio;		/*!< VBUS_XDET GPIO number.		*/
	unsigned long	force_dcin_gpio;	/*!< FORCE_DCIN GPIO number.		*/
	unsigned long	sys_wak_stat_gpio;	/*!< SYS_WAK_STAT GPIO number.		*/
	int		pmic_vbat_channel;	/*!< PMIC VBAT channel number.		*/
	int		pmic_vbat_samples;	/*!< PMIC VBAT samples.			*/
	int		pmic_thr_sense_channel;	/*!< NTC(thermal sense) channel number.	*/
	int		pmic_thr_sense_samples;	/*!< NTC(thermal sense) samples. If update this value, also update battery_temp_*_*_raw	*/
	long		battery_scale_to_uv_mul;	/*!< Battery voltage scaler of multiplyer */
	long		battery_scale_to_uv_div;	/*!< Battery voltage scaler of divisor. */
	long		battery_scale_to_uv_offset;	/*!< Battery voltage scaler of offset. */
	unsigned	battery_fully_charge_uv;	/*!< Fully charge battery voltage in uV.			*/
	unsigned	battery_weakly_charge_uv;	/*!< Weakly(ITAWARI in japanese) charge battery voltage in uV.	*/
	unsigned	battery_resume_charge_uv;	/*!< Resume charging battery voltage in uV.			*/
	unsigned	battery_safe_uv;		/*!< safe enough to run battery voltage in uV. */
	unsigned	battery_safe_absolutely_uv;	/*!< absolutely safe enough to run battery voltage in uV. */
	unsigned	battery_temp_lo_enter_raw;	/*!< Battery temperature low state enter. */
	unsigned	battery_temp_lo_exit_raw;	/*!< Battery temperature low state exit. */
	unsigned	battery_temp_hi_exit_raw;	/*!< Battery temperature high state exit. */
	unsigned	battery_temp_hi_enter_raw;	/*!< Battery temperature high state enter. */
	unsigned	vindpm_lo_enter_battery_uv;	/*!< Battery voltage to enter VINDPM low mode. */
	unsigned	vindpm_hi_enter_battery_uv;	/*!< Battery voltage to enter VINDPM high mode. */
	uint8_t		vindpm_lo;			/*!< VINDPM low percent.  VINDPM(LO) = (4200mV * (1.00+ vindpm_lo * 0.01)) */
	uint8_t		vindpm_hi;			/*!< VINDPM high percent, VINDPM(HI) = (4200mV * (1.00+ vindpm_hi * 0.01)) */
	long		charge_time_max_sec;		/*!< Maximum continuous charging time in seconds. */
	struct bq24262_wmport_battery_capacity_uv	capacity_uv[BQ24262_WMPORT_BATTERY_LEVELS]; /*!< Will use in the future. */
};

#endif /* (!defined(BQ24262_PLATFORM_H_INCLUDED)) */
