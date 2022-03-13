/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* BQ24262 battery charger controller.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef BQ24262_H_INCLUDED
#define BQ24262_H_INCLUDED

#include <linux/i2c.h>

/*! @note I2C bus hang-up timer expires at spending 700ms or more.
    Refer to tI2CRESET specification.
    The sequence "i2c_addr - reg_addr - reg_val" may spends about Tclk x 10 x 3 seconds.
*/
#define	BQ24262_I2C_CLK_MIN_KHZ	(1)
#define	BQ24262_I2C_CLK_MAX_KHZ	(400)

/* I2C registers */
						/* addr, mask,  selection. */
#define	BQ24262_STATUS_CONTROL			(0x00)
#define	BQ24262_STATUS_CONTROL_TMR_RST			(0x80)	/* Not functional. */
#define	BQ24262_STATUS_CONTROL_EN_BOOTST		(0x40)
#define	BQ24262_STATUS_CONTROL_STAT			(0x30)	/* STAT1, STAT0 */
#define	BQ24262_STATUS_CONTROL_STAT_SHIFT		(0x04)
#define	BQ24262_STATUS_CONTROL_STAT_READY			(0x00)
#define	BQ24262_STATUS_CONTROL_STAT_CHARGE_IN_PROGRESS		(0x10)
#define	BQ24262_STATUS_CONTROL_STAT_CHARGE_DONE			(0x20)
#define	BQ24262_STATUS_CONTROL_STAT_FAULT			(0x30)
#define	BQ24262_STATUS_CONTROL_EN_SHIPMODE		(0x08)
#define	BQ24262_STATUS_CONTROL_FAULT			(0x07)	/* FAULT_2, FAULT_1, FAULT_0 */
#define	BQ24262_STATUS_CONTROL_FAULT_SHIFT		(0x00)
#define	BQ24262_STATUS_CONTROL_FAULT_NORMAL			(0x00)	/* Normal			*/
#define	BQ24262_STATUS_CONTROL_FAULT_OVP			(0x01)	/* VIN > VOVP or Boost Mode OVP */
#define	BQ24262_STATUS_CONTROL_FAULT_UVLO			(0x02)	/* Low Supply connected (VIN<VUVLO or VIN<VSLP) or Boost Mode Overcurrent */
#define	BQ24262_STATUS_CONTROL_FAULT_TERMAL			(0x03)	/* Thermal Shutdown		*/
#define	BQ24262_STATUS_CONTROL_FAULT_BATT_TEMP			(0x04)	/* Battery Temperature Fault	*/
#define	BQ24262_STATUS_CONTROL_FAULT_TIMER			(0x05)	/* Timer Fault			*/
#define	BQ24262_STATUS_CONTROL_FAULT_BATT_OVP			(0x06)	/* Battery OVP			*/
#define	BQ24262_STATUS_CONTROL_FAULT_NO_BATT			(0x07)	/* No Battery connected.	*/

						/* addr, mask,  selection. */
#define	BQ24262_CONTROL				(0x01)
#define	BQ24262_CONTROL_RESET				(0x80)
#define	BQ24262_CONTROL_IN_LIMIT			(0x70)
#define	BQ24262_CONTROL_IN_LIMIT_SHIFT			(0x04)
#define	BQ24262_CONTROL_IN_LIMIT_100MA				(0x00)
#define	BQ24262_CONTROL_IN_LIMIT_150MA				(0x10)
#define	BQ24262_CONTROL_IN_LIMIT_500MA				(0x20)
#define	BQ24262_CONTROL_IN_LIMIT_900MA				(0x30)
#define	BQ24262_CONTROL_IN_LIMIT_1500MA				(0x40)
#define	BQ24262_CONTROL_IN_LIMIT_1950MA				(0x50)
#define	BQ24262_CONTROL_IN_LIMIT_2500MA				(0x60)
#define	BQ24262_CONTROL_IN_LIMIT_2000MA				(0x70)
#define	BQ24262_CONTROL_EN_STAT				(0x08)
#define	BQ24262_CONTROL_TE				(0x04)
#define	BQ24262_CONTROL_CE_N				(0x02)
#define	BQ24262_CONTROL_HZ_MODE				(0x01)

						/* addr, mask,  selection. */
#define	BQ24262_BATTERY_VOLTAGE			(0x02)	/* Control/Battery Voltage */
#define	BQ24262_BATTERY_VOLTAGE_VBREG			(0xfc)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_SHIFT		(0x02)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_640MV			(0x80)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_320MV			(0x40)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_160MV			(0x20)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_80MV			(0x10)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_40MV			(0x08)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_20MV			(0x04)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_BASE_MV		(3500)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_OFFSET_STEP_MV	(20)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_OFFSET_MV(offset_mv)	\
	((((offset_mv)+BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_OFFSET_STEP_MV/2) \
	  /BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_OFFSET_STEP_MV \
	 ) \
	 * (0x01 << BQ24262_BATTERY_VOLTAGE_VBREG_SHIFT) \
	)
#define	BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_MV(batt_mv)	\
	(((((batt_mv)-BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_BASE_MV) \
	     +BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_OFFSET_STEP_MV/2 \
	  ) \
	     /BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_OFFSET_STEP_MV \
	 ) \
	 *(0x01 << BQ24262_BATTERY_VOLTAGE_VBREG_SHIFT) \
	)

#define	BQ24262_BATTERY_VOLTAGE_VBREG_OFFSET_UV(offset_uv)	\
	BQ24262_BATTERY_VOLTAGE_VBREG_OFFSET_MV((offset_uv / 1000))

#define	BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_UV(batt_uv)	\
	BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_MV((batt_uv / 1000))

#define	BQ24262_BATTERY_VOLTAGE_MOD_FREQ		(0x03)
#define	BQ24262_BATTERY_VOLTAGE_MOD_FREQ_SHIFT		(0x00)
#define	BQ24262_BATTERY_VOLTAGE_MOD_FREQ_NO_CHANGE		(0x00)
#define	BQ24262_BATTERY_VOLTAGE_MOD_FREQ_PLUS_10P		(0x01)
#define	BQ24262_BATTERY_VOLTAGE_MOD_FREQ_MINUS_10P		(0x02)
#define	BQ24262_BATTERY_VOLTAGE_MOD_FREQ_NA			(0x03)

						/* addr, mask,  selection. */
#define	BQ24262_VENDOR				(0x03)	/* Vendor/Part/Revision */
#define	BQ24262_VENDOR_VENDOR				(0xe0)
#define	BQ24262_VENDOR_VENDOR_SHIFT			(0x05)
#define	BQ24262_VENDOR_VENDOR_DEFAULT				(0x40)
#define	BQ24262_VENDOR_PN				(0x18)
#define	BQ24262_VENDOR_PN_SHIFT				(0x03)
/* Other bits are not available. */

						/* addr, mask,  selection. */
#define	BQ24262_BATTERY_CURRENT			(0x04)	/* Battery Termination/Fast Charge Current Register */
#define	BQ24262_BATTERY_CURRENT_ICHRG			(0xf8)
#define	BQ24262_BATTERY_CURRENT_ICHRG_SHIFT		(0x03)
#define	BQ24262_BATTERY_CURRENT_ICHRG_1600MA			(0x80)
#define	BQ24262_BATTERY_CURRENT_ICHRG_800MA			(0x40)
#define	BQ24262_BATTERY_CURRENT_ICHRG_400MA			(0x20)
#define	BQ24262_BATTERY_CURRENT_ICHRG_200MA			(0x10)
#define	BQ24262_BATTERY_CURRENT_ICHRG_100MA			(0x08)
#define	BQ24262_BATTERY_CURRENT_ICHRG_BASE_MA			(500)
#define	BQ24262_BATTERY_CURRENT_ICHRG_OFFSET_STEP_MA		(100)

#define	BQ24262_BATTERY_CURRENT_ICHRG_OFFSET_MA(offset_ma)	\
	((((offset_ma)+BQ24262_BATTERY_CURRENT_ICHRG_OFFSET_STEP_MA/2) \
	  /BQ24262_BATTERY_CURRENT_ICHRG_OFFSET_STEP_MA \
	 ) \
	 * (0x01 << BQ24262_BATTERY_CURRENT_ICHRG_SHIFT) \
	)
#define	BQ24262_BATTERY_CURRENT_ICHRG_IBATT_MA(batt_ma)	\
	(((((batt_ma)-BQ24262_BATTERY_CURRENT_ICHRG_BASE_MA) \
	     +BQ24262_BATTERY_CURRENT_ICHRG_OFFSET_STEP_MA/2 \
	  ) \
	     /BQ24262_BATTERY_CURRENT_ICHRG_OFFSET_STEP_MA \
	 ) \
	 *(0x01 << BQ24262_BATTERY_CURRENT_ICHRG_SHIFT) \
	)

#define	BQ24262_BATTERY_CURRENT_ITERM			(0x07)
#define	BQ24262_BATTERY_CURRENT_ITERM_SHIFT		(0x00)
#define	BQ24262_BATTERY_CURRENT_ITERM_200MA			(0x04)
#define	BQ24262_BATTERY_CURRENT_ITERM_100MA			(0x02)
#define	BQ24262_BATTERY_CURRENT_ITERM_50MA			(0x01)
#define	BQ24262_BATTERY_CURRENT_ITERM_BASE_MA			(50)
#define	BQ24262_BATTERY_CURRENT_ITERM_OFFSET_STEP_MA		(50)

#define	BQ24262_BATTERY_CURRENT_ITERM_OFFSET_MA(offset_ma)	\
	((((offset_ma)+BQ24262_BATTERY_CURRENT_ITERM_OFFSET_STEP_MA/2) \
	  /BQ24262_BATTERY_CURRENT_ITERM_OFFSET_STEP_MA \
	 ) \
	 * (0x01 << BQ24262_BATTERY_CURRENT_ITERM_SHIFT) \
	)
#define	BQ24262_BATTERY_CURRENT_ITERM_IBATT_MA(batt_ma)	\
	(((((batt_ma)-BQ24262_BATTERY_CURRENT_ITERM_BASE_MA) \
	     +BQ24262_BATTERY_CURRENT_ITERM_OFFSET_STEP_MA/2 \
	  ) \
	     /BQ24262_BATTERY_CURRENT_ITERM_OFFSET_STEP_MA \
	 ) \
	 *(0x01 << BQ24262_BATTERY_CURRENT_ITERM_SHIFT) \
	)

						/* addr, mask,  selection. */
#define	BQ24262_VIN_MINSYS			(0x05)	/* VIN-DPM Voltage/ MINSYS Status Register */
#define	BQ24262_VIN_MINSYS_MINSYS_STATUS		(0x80)
#define	BQ24262_VIN_MINSYS_VINDPM_STATUS		(0x40)
#define	BQ24262_VIN_MINSYS_LOW_CHG			(0x20)
#define	BQ24262_VIN_MINSYS_DPDM_EN			(0x10)	/* Not functional. */
#define	BQ24262_VIN_MINSYS_CD_STATUS			(0x08)
#define	BQ24262_VIN_MINSYS_VINDPM			(0x07)
#define	BQ24262_VIN_MINSYS_VINDPM_SHIFT			(0x00)
#define	BQ24262_VIN_MINSYS_VINDPM_PLUS_8P			(0x04)
#define	BQ24262_VIN_MINSYS_VINDPM_PLUS_4P			(0x02)
#define	BQ24262_VIN_MINSYS_VINDPM_PLUS_2P			(0x01)
#define	BQ24262_VIN_MINSYS_VINDPM_BASE_PCENT			(0)
#define	BQ24262_VIN_MINSYS_VINDPM_OFFSET_STEP_PCENT		(2)

#define	BQ24262_VIN_MINSYS_VINDPM_PLUS(pcent)	\
	(((((pcent)-BQ24262_VIN_MINSYS_VINDPM_BASE_PCENT) \
	     +BQ24262_VIN_MINSYS_VINDPM_OFFSET_STEP_PCENT/2 \
	  ) \
	     /BQ24262_VIN_MINSYS_VINDPM_OFFSET_STEP_PCENT \
	 ) \
	 *(0x01 << BQ24262_VIN_MINSYS_VINDPM_SHIFT) \
	)

					/* addr, mask,  selection. */
#define	BQ24262_SAFETY			(0x06)	/* Safety Timer/ NTC Monitor Register */
#define	BQ24262_SAFETY_2XTMR_EN			(0x80)
#define	BQ24262_SAFETY_TMR			(0x60)	/* Not functional. */
#define	BQ24262_SAFETY_TMR_SHIFT		(0x05)
#define	BQ24262_SAFETY_BOOST_ILIM		(0x10)
#define	BQ24262_SAFETY_BOOST_ILIM_1000MA		(0x10)
#define	BQ24262_SAFETY_BOOST_ILIM_500MA			(0x00)
#define	BQ24262_SAFETY_TS_EN			(0x08)
#define	BQ24262_SAFETY_TS_FAULT			(0x06)
#define	BQ24262_SAFETY_TS_FAULT_SHIFT		(0x01)
#define	BQ24262_SAFETY_TS_FAULT_NORMAL			(0x00)
#define	BQ24262_SAFETY_TS_FAULT_TCOLD_HOT		(0x02)
#define	BQ24262_SAFETY_TS_FAULT_TCOOL			(0x04)
#define	BQ24262_SAFETY_TS_FAULT_TWARM			(0x06)
#define	BQ24262_SAFETY_VINDPM_OFF		(0x01)
#define	BQ24262_SAFETY_VINDPM_OFF_4200MV		(0x00)
#define	BQ24262_SAFETY_VINDPM_OFF_10100MV		(0x01)

#endif /* BQ24296_H_INCLUDED */
