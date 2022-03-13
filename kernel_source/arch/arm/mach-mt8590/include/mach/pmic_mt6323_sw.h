/*
* Copyright (C) 2011-2015 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *    pmic_mt6323_sw.h
 *
 * Project:
 * --------
 *   Android_Software
 *
 * Description:
 * ------------
 *   This file is intended for PMU 6323 driver.
 *
 * Author:
 * -------
 * -------
 *
 ****************************************************************************/

#ifndef _MT6323_PMIC_SW_H_
#define _MT6323_PMIC_SW_H_

#include <mach/mt_typedefs.h>

//==============================================================================
// The CHIP INFO
//==============================================================================
#define PMIC6323_E1_CID_CODE    0x1023
#define PMIC6323_E2_CID_CODE    0x2023

//==============================================================================
// The CHIP SPEC of each block
//==============================================================================
typedef enum
{
	BUCK_VPROC = 0,
	BUCK_VSRAM,
	BUCK_VCORE,
	BUCK_VM,
	BUCK_VIO18,	
	BUCK_VPA,
	BUCK_VRF18,
	BUCK_VRF18_2,
	
	BUCK_MAX
}upmu_buck_list_enum;

typedef enum
{
	//Digital LDO
	LDO_VIO28 = 0,
	LDO_VUSB,	
	LDO_VMC1,
	LDO_VMCH1,
	LDO_VEMC_3V3,
	LDO_VEMC_1V8,
	LDO_VGP1,
	LDO_VGP2,
	LDO_VGP3,
	LDO_VGP4,
	LDO_VGP5,
	LDO_VGP6,
	LDO_VSIM1,
	LDO_VSIM2,
	LDO_VIBR,
	LDO_VRTC,
	LDO_VAST,

	//Analog LDO
	LDO_VRF28,
	LDO_VRF28_2,
	LDO_VTCXO,
	LDO_VTCXO_2,
	LDO_VA,
	LDO_VA28,
	LDO_VCAMA,
	
	LDO_MAX
}upmu_ldo_list_enum;

//==============================================================================
// PMIC6323 Exported Function
//==============================================================================
extern U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT);
extern U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT);
extern U32 pmic_read_interface_nolock (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT);
extern U32 pmic_config_interface_nolock (U32 RegNum, U32 val, U32 MASK, U32 SHIFT);
extern void pmic_lock(void);
extern void pmic_unlock(void);

//==============================================================================
// BUCK/LDO Voltage Define
//==============================================================================
typedef enum
{
	PMIC_ADPT_VOLT_0_0        =     0,
	PMIC_ADPT_VOLT_0_1        =   100,
	PMIC_ADPT_VOLT_0_2        =   200,
	PMIC_ADPT_VOLT_0_3        =   300,
	PMIC_ADPT_VOLT_0_4        =   400,
	PMIC_ADPT_VOLT_0_5        =   500,
	PMIC_ADPT_VOLT_0_6        =   600,
	PMIC_ADPT_VOLT_0_7        =   700,
	PMIC_ADPT_VOLT_0_7_0_0    =   700,
	PMIC_ADPT_VOLT_0_7_2_5    =   725,
	PMIC_ADPT_VOLT_0_7_5_0    =   750,
	PMIC_ADPT_VOLT_0_7_7_5    =   775,
	PMIC_ADPT_VOLT_0_8        =   800,
	PMIC_ADPT_VOLT_0_8_0_0    =   800,
	PMIC_ADPT_VOLT_0_8_2_5    =   825,
	PMIC_ADPT_VOLT_0_8_5_0    =   850,
	PMIC_ADPT_VOLT_0_8_7_5    =   875,
	PMIC_ADPT_VOLT_0_9        =   900,
	PMIC_ADPT_VOLT_0_9_0_0    =   900,
	PMIC_ADPT_VOLT_0_9_2_5    =   925,
	PMIC_ADPT_VOLT_0_9_5_0    =   950,
	PMIC_ADPT_VOLT_0_9_7_5    =   975,
	PMIC_ADPT_VOLT_1_0        =  1000,
	PMIC_ADPT_VOLT_1_0_0_0    =  1000,
	PMIC_ADPT_VOLT_1_0_2_5    =  1025,
	PMIC_ADPT_VOLT_1_0_5_0    =  1050,
	PMIC_ADPT_VOLT_1_0_7_5    =  1075,
	PMIC_ADPT_VOLT_1_1        =  1100,
	PMIC_ADPT_VOLT_1_1_0_0    =  1100,
	PMIC_ADPT_VOLT_1_1_2_5    =  1125,
	PMIC_ADPT_VOLT_1_1_5_0    =  1150,
	PMIC_ADPT_VOLT_1_1_7_5    =  1175,
	PMIC_ADPT_VOLT_1_2        =  1200,
	PMIC_ADPT_VOLT_1_2_0_0    =  1200,
	PMIC_ADPT_VOLT_1_2_2_5    =  1225,
	PMIC_ADPT_VOLT_1_2_5_0    =  1250,
	PMIC_ADPT_VOLT_1_2_7_5    =  1275,
	PMIC_ADPT_VOLT_1_3        =  1300,
	PMIC_ADPT_VOLT_1_3_0_0    =  1300,
	PMIC_ADPT_VOLT_1_3_2_5    =  1325,
	PMIC_ADPT_VOLT_1_3_5_0    =  1350,
	PMIC_ADPT_VOLT_1_3_7_5    =  1375,
	PMIC_ADPT_VOLT_1_4        =  1400,
	PMIC_ADPT_VOLT_1_4_0_0    =  1400,
	PMIC_ADPT_VOLT_1_4_2_5    =  1425,
	PMIC_ADPT_VOLT_1_4_5_0    =  1450,
	PMIC_ADPT_VOLT_1_4_7_5    =  1475,
	PMIC_ADPT_VOLT_1_5        =  1500,
	PMIC_ADPT_VOLT_1_5_0_0    =  1500,
	PMIC_ADPT_VOLT_1_5_2_5    =  1525,
	PMIC_ADPT_VOLT_1_5_5_0    =  1550,
	PMIC_ADPT_VOLT_1_5_7_5    =  1575,
	PMIC_ADPT_VOLT_1_6        =  1600,
	PMIC_ADPT_VOLT_1_6_0_0    =  1600,
	PMIC_ADPT_VOLT_1_6_2_5    =  1625,
	PMIC_ADPT_VOLT_1_6_5_0    =  1650,
	PMIC_ADPT_VOLT_1_6_7_5    =  1675,
	PMIC_ADPT_VOLT_1_7        =  1700,
	PMIC_ADPT_VOLT_1_7_0_0    =  1700,
	PMIC_ADPT_VOLT_1_7_2_5    =  1725,
	PMIC_ADPT_VOLT_1_7_5_0    =  1750,
	PMIC_ADPT_VOLT_1_7_7_5    =  1775,
	PMIC_ADPT_VOLT_1_8        =  1800,
	PMIC_ADPT_VOLT_1_8_0_0    =  1800,
	PMIC_ADPT_VOLT_1_8_2_5    =  1825,
	PMIC_ADPT_VOLT_1_8_5_0    =  1850,
	PMIC_ADPT_VOLT_1_8_7_5    =  1875,
	PMIC_ADPT_VOLT_1_9        =  1900,
	PMIC_ADPT_VOLT_1_9_0_0    =  1900,
	PMIC_ADPT_VOLT_1_9_2_5    =  1925,
	PMIC_ADPT_VOLT_1_9_5_0    =  1950,
	PMIC_ADPT_VOLT_1_9_7_5    =  1975,
	PMIC_ADPT_VOLT_2_0        =  2000,
	PMIC_ADPT_VOLT_2_0_0_0    =  2000,
	PMIC_ADPT_VOLT_2_0_2_5    =  2025,
	PMIC_ADPT_VOLT_2_0_5_0    =  2050,
	PMIC_ADPT_VOLT_2_0_7_5    =  2075,
	PMIC_ADPT_VOLT_2_1        =  2100,
	PMIC_ADPT_VOLT_2_2        =  2200,
	PMIC_ADPT_VOLT_2_3        =  2300,
	PMIC_ADPT_VOLT_2_4        =  2400,
	PMIC_ADPT_VOLT_2_5        =  2500,
	PMIC_ADPT_VOLT_2_6        =  2600,
	PMIC_ADPT_VOLT_2_7        =  2700,
	PMIC_ADPT_VOLT_2_7_0_0    =  2700,
	PMIC_ADPT_VOLT_2_7_2_5    =  2725,
	PMIC_ADPT_VOLT_2_7_5_0    =  2750,
	PMIC_ADPT_VOLT_2_7_7_5    =  2775,
	PMIC_ADPT_VOLT_2_8        =  2800,
	PMIC_ADPT_VOLT_2_8_0_0    =  2800,
	PMIC_ADPT_VOLT_2_8_2_5    =  2825,
	PMIC_ADPT_VOLT_2_8_5_0    =  2850,
	PMIC_ADPT_VOLT_2_8_7_5    =  2875,
	PMIC_ADPT_VOLT_2_9        =  2900,
	PMIC_ADPT_VOLT_3_0        =  3000,
	PMIC_ADPT_VOLT_3_1        =  3100,
	PMIC_ADPT_VOLT_3_1_0_0    =  2000,
	PMIC_ADPT_VOLT_3_2        =  3200,
	PMIC_ADPT_VOLT_3_3        =  3300,
	PMIC_ADPT_VOLT_3_4        =  3400,
	PMIC_ADPT_VOLT_3_5        =  3500,
	PMIC_ADPT_VOLT_3_6        =  3600,
	PMIC_ADPT_VOLT_3_7        =  3700,
	PMIC_ADPT_VOLT_3_8        =  3800,
	PMIC_ADPT_VOLT_3_9        =  3900,
	PMIC_ADPT_VOLT_4_0        =  4000,
	PMIC_ADPT_VOLT_4_1        =  4100,
	PMIC_ADPT_VOLT_4_2        =  4200,
	PMIC_ADPT_VOLT_4_3        =  4300,
	PMIC_ADPT_VOLT_4_4        =  4400,
	PMIC_ADPT_VOLT_4_5        =  4500,
	PMIC_ADPT_VOLT_4_6        =  4600,
	PMIC_ADPT_VOLT_4_7        =  4700,
	PMIC_ADPT_VOLT_4_8        =  4800,
	PMIC_ADPT_VOLT_4_9        =  4900,
	PMIC_ADPT_VOLT_5_0        =  5000,
	PMIC_ADPT_VOLT_5_1        =  5100,
	PMIC_ADPT_VOLT_5_2        =  5200,
	PMIC_ADPT_VOLT_5_3        =  5300,
	PMIC_ADPT_VOLT_5_4        =  5400,
	PMIC_ADPT_VOLT_5_5        =  5500,
	PMIC_ADPT_VOLT_5_6        =  5600,
	PMIC_ADPT_VOLT_5_7        =  5700,
	PMIC_ADPT_VOLT_5_8        =  5800,
	PMIC_ADPT_VOLT_5_9        =  5900,
	PMIC_ADPT_VOLT_6_0        =  6000,
	PMIC_ADPT_VOLT_6_1        =  6100,
	PMIC_ADPT_VOLT_6_2        =  6200,
	PMIC_ADPT_VOLT_6_3        =  6300,
	PMIC_ADPT_VOLT_6_4        =  6400,
	PMIC_ADPT_VOLT_6_5        =  6500,
	PMIC_ADPT_VOLT_6_6        =  6600,
	PMIC_ADPT_VOLT_6_7        =  6700,
	PMIC_ADPT_VOLT_6_8        =  6800,
	PMIC_ADPT_VOLT_6_9        =  6900,
	PMIC_ADPT_VOLT_7_0        =  7000,
	PMIC_ADPT_VOLT_7_1        =  7100,
	PMIC_ADPT_VOLT_7_2        =  7200,
	PMIC_ADPT_VOLT_7_3        =  7300,
	PMIC_ADPT_VOLT_7_4        =  7400,
	PMIC_ADPT_VOLT_7_5        =  7500,
	PMIC_ADPT_VOLT_7_6        =  7600,
	PMIC_ADPT_VOLT_7_7        =  7700,
	PMIC_ADPT_VOLT_7_8        =  7800,
	PMIC_ADPT_VOLT_7_9        =  7900,
	PMIC_ADPT_VOLT_8_0        =  8000,

	//new	
	PMIC_ADPT_VOLT_1_5_2_0_V  =  1520,
	PMIC_ADPT_VOLT_1_5_4_0_V  =  1540,
	PMIC_ADPT_VOLT_1_5_6_0_V  =  1560,
	PMIC_ADPT_VOLT_1_5_8_0_V  =  1580,
	PMIC_ADPT_VOLT_1_6_2_0_V  =  1620,
	PMIC_ADPT_VOLT_1_6_4_0_V  =  1640,
	PMIC_ADPT_VOLT_1_6_6_0_V  =  1660,
	PMIC_ADPT_VOLT_1_6_8_0_V  =  1680,
	PMIC_ADPT_VOLT_1_7_2_0_V  =  1720,
	PMIC_ADPT_VOLT_1_7_4_0_V  =  1740,
	PMIC_ADPT_VOLT_1_7_6_0_V  =  1760,
	PMIC_ADPT_VOLT_1_7_8_0_V  =  1780,
	PMIC_ADPT_VOLT_1_8_2_0_V  =  1820,
	PMIC_ADPT_VOLT_1_8_4_0_V  =  1840,
	PMIC_ADPT_VOLT_1_8_6_0_V  =  1860,
	PMIC_ADPT_VOLT_1_8_8_0_V  =  1880,
	PMIC_ADPT_VOLT_1_9_2_0_V  =  1920,
	PMIC_ADPT_VOLT_1_9_4_0_V  =  1940,
	PMIC_ADPT_VOLT_1_9_6_0_V  =  1960,
	PMIC_ADPT_VOLT_1_9_8_0_V  =  1980,
	PMIC_ADPT_VOLT_2_0_2_0_V  =  2020,
	PMIC_ADPT_VOLT_2_0_4_0_V  =  2040,
	PMIC_ADPT_VOLT_2_0_6_0_V  =  2060,
	PMIC_ADPT_VOLT_2_0_8_0_V  =  2080,
	PMIC_ADPT_VOLT_2_1_2_0_V  =  2120,
		
	PMIC_ADPT_VOLT_MAX        = 50000000	
}pmic_adpt_voltage_enum;


// Common S/W structure
typedef enum
{
	UPMU_VOLT_0_0_0_0_V = PMIC_ADPT_VOLT_0_0,
	UPMU_VOLT_0_1_0_0_V = PMIC_ADPT_VOLT_0_1,
	UPMU_VOLT_0_2_0_0_V = PMIC_ADPT_VOLT_0_2,
	UPMU_VOLT_0_3_0_0_V = PMIC_ADPT_VOLT_0_3,
	UPMU_VOLT_0_4_0_0_V = PMIC_ADPT_VOLT_0_4,
	UPMU_VOLT_0_5_0_0_V = PMIC_ADPT_VOLT_0_5,
	UPMU_VOLT_0_6_0_0_V = PMIC_ADPT_VOLT_0_6,
	UPMU_VOLT_0_7_0_0_V = PMIC_ADPT_VOLT_0_7_0_0,
	UPMU_VOLT_0_7_2_5_V = PMIC_ADPT_VOLT_0_7_2_5,
	UPMU_VOLT_0_7_5_0_V = PMIC_ADPT_VOLT_0_7_5_0,
	UPMU_VOLT_0_7_7_5_V = PMIC_ADPT_VOLT_0_7_7_5,
	UPMU_VOLT_0_8_0_0_V = PMIC_ADPT_VOLT_0_8_0_0,
	UPMU_VOLT_0_8_2_5_V = PMIC_ADPT_VOLT_0_8_2_5,
	UPMU_VOLT_0_8_5_0_V = PMIC_ADPT_VOLT_0_8_5_0,
	UPMU_VOLT_0_8_7_5_V = PMIC_ADPT_VOLT_0_8_7_5,
	UPMU_VOLT_0_9_0_0_V = PMIC_ADPT_VOLT_0_9_0_0,
	UPMU_VOLT_0_9_2_5_V = PMIC_ADPT_VOLT_0_9_2_5,
	UPMU_VOLT_0_9_5_0_V = PMIC_ADPT_VOLT_0_9_5_0,
	UPMU_VOLT_0_9_7_5_V = PMIC_ADPT_VOLT_0_9_7_5,
	UPMU_VOLT_1_0_0_0_V = PMIC_ADPT_VOLT_1_0_0_0,
	UPMU_VOLT_1_0_2_5_V = PMIC_ADPT_VOLT_1_0_2_5,
	UPMU_VOLT_1_0_5_0_V = PMIC_ADPT_VOLT_1_0_5_0,
	UPMU_VOLT_1_0_7_5_V = PMIC_ADPT_VOLT_1_0_7_5,
	UPMU_VOLT_1_1_0_0_V = PMIC_ADPT_VOLT_1_1_0_0,
	UPMU_VOLT_1_1_2_5_V = PMIC_ADPT_VOLT_1_1_2_5,
	UPMU_VOLT_1_1_5_0_V = PMIC_ADPT_VOLT_1_1_5_0,
	UPMU_VOLT_1_1_7_5_V = PMIC_ADPT_VOLT_1_1_7_5,
	UPMU_VOLT_1_2_0_0_V = PMIC_ADPT_VOLT_1_2_0_0,
	UPMU_VOLT_1_2_2_5_V = PMIC_ADPT_VOLT_1_2_2_5,
	UPMU_VOLT_1_2_5_0_V = PMIC_ADPT_VOLT_1_2_5_0,
	UPMU_VOLT_1_2_7_5_V = PMIC_ADPT_VOLT_1_2_7_5,
	UPMU_VOLT_1_3_0_0_V = PMIC_ADPT_VOLT_1_3_0_0,
	UPMU_VOLT_1_3_2_5_V = PMIC_ADPT_VOLT_1_3_2_5,
	UPMU_VOLT_1_3_5_0_V = PMIC_ADPT_VOLT_1_3_5_0,
	UPMU_VOLT_1_3_7_5_V = PMIC_ADPT_VOLT_1_3_7_5,
	UPMU_VOLT_1_4_0_0_V = PMIC_ADPT_VOLT_1_4_0_0,
	UPMU_VOLT_1_4_2_5_V = PMIC_ADPT_VOLT_1_4_2_5,
	UPMU_VOLT_1_4_5_0_V = PMIC_ADPT_VOLT_1_4_5_0,
	UPMU_VOLT_1_4_7_5_V = PMIC_ADPT_VOLT_1_4_7_5,
	UPMU_VOLT_1_5_0_0_V = PMIC_ADPT_VOLT_1_5_0_0,
	UPMU_VOLT_1_5_2_5_V = PMIC_ADPT_VOLT_1_5_2_5,
	UPMU_VOLT_1_5_5_0_V = PMIC_ADPT_VOLT_1_5_5_0,
	UPMU_VOLT_1_5_7_5_V = PMIC_ADPT_VOLT_1_5_7_5,
	UPMU_VOLT_1_6_0_0_V = PMIC_ADPT_VOLT_1_6_0_0,
	UPMU_VOLT_1_6_2_5_V = PMIC_ADPT_VOLT_1_6_2_5,
	UPMU_VOLT_1_6_5_0_V = PMIC_ADPT_VOLT_1_6_5_0,
	UPMU_VOLT_1_6_7_5_V = PMIC_ADPT_VOLT_1_6_7_5,
	UPMU_VOLT_1_7_0_0_V = PMIC_ADPT_VOLT_1_7_0_0,
	UPMU_VOLT_1_7_2_5_V = PMIC_ADPT_VOLT_1_7_2_5,
	UPMU_VOLT_1_7_5_0_V = PMIC_ADPT_VOLT_1_7_5_0,
	UPMU_VOLT_1_7_7_5_V = PMIC_ADPT_VOLT_1_7_7_5,
	UPMU_VOLT_1_8_0_0_V = PMIC_ADPT_VOLT_1_8_0_0,
	UPMU_VOLT_1_8_2_5_V = PMIC_ADPT_VOLT_1_8_2_5,
	UPMU_VOLT_1_8_5_0_V = PMIC_ADPT_VOLT_1_8_5_0,
	UPMU_VOLT_1_8_7_5_V = PMIC_ADPT_VOLT_1_8_7_5,
	UPMU_VOLT_1_9_0_0_V = PMIC_ADPT_VOLT_1_9_0_0,
	UPMU_VOLT_1_9_2_5_V = PMIC_ADPT_VOLT_1_9_2_5,
	UPMU_VOLT_1_9_5_0_V = PMIC_ADPT_VOLT_1_9_5_0,
	UPMU_VOLT_1_9_7_5_V = PMIC_ADPT_VOLT_1_9_7_5,
	UPMU_VOLT_2_0_0_0_V = PMIC_ADPT_VOLT_2_0_0_0,
	UPMU_VOLT_2_0_2_5_V = PMIC_ADPT_VOLT_2_0_2_5,
	UPMU_VOLT_2_0_5_0_V = PMIC_ADPT_VOLT_2_0_5_0,
	UPMU_VOLT_2_0_7_5_V = PMIC_ADPT_VOLT_2_0_7_5,
	UPMU_VOLT_2_1_0_0_V = PMIC_ADPT_VOLT_2_1,
	UPMU_VOLT_2_2_0_0_V = PMIC_ADPT_VOLT_2_2,
	UPMU_VOLT_2_3_0_0_V = PMIC_ADPT_VOLT_2_3,
	UPMU_VOLT_2_4_0_0_V = PMIC_ADPT_VOLT_2_4,
	UPMU_VOLT_2_5_0_0_V = PMIC_ADPT_VOLT_2_5,
	UPMU_VOLT_2_6_0_0_V = PMIC_ADPT_VOLT_2_6,
	UPMU_VOLT_2_7_0_0_V = PMIC_ADPT_VOLT_2_7_0_0,
	UPMU_VOLT_2_7_2_5_V = PMIC_ADPT_VOLT_2_7_2_5,
	UPMU_VOLT_2_7_5_0_V = PMIC_ADPT_VOLT_2_7_5_0,
	UPMU_VOLT_2_7_7_5_V = PMIC_ADPT_VOLT_2_7_7_5,
	UPMU_VOLT_2_8_0_0_V = PMIC_ADPT_VOLT_2_8_0_0,
	UPMU_VOLT_2_8_2_5_V = PMIC_ADPT_VOLT_2_8_2_5,
	UPMU_VOLT_2_8_5_0_V = PMIC_ADPT_VOLT_2_8_5_0,
	UPMU_VOLT_2_8_7_5_V = PMIC_ADPT_VOLT_2_8_7_5,
	UPMU_VOLT_2_9_0_0_V = PMIC_ADPT_VOLT_2_9,
	UPMU_VOLT_3_0_0_0_V = PMIC_ADPT_VOLT_3_0,
	UPMU_VOLT_3_1_0_0_V = PMIC_ADPT_VOLT_3_1,
	UPMU_VOLT_3_2_0_0_V = PMIC_ADPT_VOLT_3_2,
	UPMU_VOLT_3_3_0_0_V = PMIC_ADPT_VOLT_3_3,
	UPMU_VOLT_3_4_0_0_V = PMIC_ADPT_VOLT_3_4,
	UPMU_VOLT_3_5_0_0_V = PMIC_ADPT_VOLT_3_5,
	UPMU_VOLT_3_6_0_0_V = PMIC_ADPT_VOLT_3_6,
	UPMU_VOLT_3_7_0_0_V = PMIC_ADPT_VOLT_3_7,
	UPMU_VOLT_3_8_0_0_V = PMIC_ADPT_VOLT_3_8,
	UPMU_VOLT_3_9_0_0_V = PMIC_ADPT_VOLT_3_9,
	UPMU_VOLT_4_0_0_0_V = PMIC_ADPT_VOLT_4_0,
	UPMU_VOLT_4_1_0_0_V = PMIC_ADPT_VOLT_4_1,
	UPMU_VOLT_4_2_0_0_V = PMIC_ADPT_VOLT_4_2,
	UPMU_VOLT_4_3_0_0_V = PMIC_ADPT_VOLT_4_3,
	UPMU_VOLT_4_4_0_0_V = PMIC_ADPT_VOLT_4_4,
	UPMU_VOLT_4_5_0_0_V = PMIC_ADPT_VOLT_4_5,
	UPMU_VOLT_4_6_0_0_V = PMIC_ADPT_VOLT_4_6,
	UPMU_VOLT_4_7_0_0_V = PMIC_ADPT_VOLT_4_7,
	UPMU_VOLT_4_8_0_0_V = PMIC_ADPT_VOLT_4_8,
	UPMU_VOLT_4_9_0_0_V = PMIC_ADPT_VOLT_4_9,
	UPMU_VOLT_5_0_0_0_V = PMIC_ADPT_VOLT_5_0,
	UPMU_VOLT_5_1_0_0_V = PMIC_ADPT_VOLT_5_1,
	UPMU_VOLT_5_2_0_0_V = PMIC_ADPT_VOLT_5_2,
	UPMU_VOLT_5_3_0_0_V = PMIC_ADPT_VOLT_5_3,
	UPMU_VOLT_5_4_0_0_V = PMIC_ADPT_VOLT_5_4,
	UPMU_VOLT_5_5_0_0_V = PMIC_ADPT_VOLT_5_5,
	UPMU_VOLT_5_6_0_0_V = PMIC_ADPT_VOLT_5_6,
	UPMU_VOLT_5_7_0_0_V = PMIC_ADPT_VOLT_5_7,
	UPMU_VOLT_5_8_0_0_V = PMIC_ADPT_VOLT_5_8,
	UPMU_VOLT_5_9_0_0_V = PMIC_ADPT_VOLT_5_9,
	UPMU_VOLT_6_0_0_0_V = PMIC_ADPT_VOLT_6_0,
	UPMU_VOLT_6_1_0_0_V = PMIC_ADPT_VOLT_6_1,
	UPMU_VOLT_6_2_0_0_V = PMIC_ADPT_VOLT_6_2,
	UPMU_VOLT_6_3_0_0_V = PMIC_ADPT_VOLT_6_3,
	UPMU_VOLT_6_4_0_0_V = PMIC_ADPT_VOLT_6_4,
	UPMU_VOLT_6_5_0_0_V = PMIC_ADPT_VOLT_6_5,
	UPMU_VOLT_6_6_0_0_V = PMIC_ADPT_VOLT_6_6,
	UPMU_VOLT_6_7_0_0_V = PMIC_ADPT_VOLT_6_7,
	UPMU_VOLT_6_8_0_0_V = PMIC_ADPT_VOLT_6_8,
	UPMU_VOLT_6_9_0_0_V = PMIC_ADPT_VOLT_6_9,
	UPMU_VOLT_7_0_0_0_V = PMIC_ADPT_VOLT_7_0,
	UPMU_VOLT_7_1_0_0_V = PMIC_ADPT_VOLT_7_1,
	UPMU_VOLT_7_2_0_0_V = PMIC_ADPT_VOLT_7_2,
	UPMU_VOLT_7_3_0_0_V = PMIC_ADPT_VOLT_7_3,
	UPMU_VOLT_7_4_0_0_V = PMIC_ADPT_VOLT_7_4,
	UPMU_VOLT_7_5_0_0_V = PMIC_ADPT_VOLT_7_5,
	UPMU_VOLT_7_6_0_0_V = PMIC_ADPT_VOLT_7_6,
	UPMU_VOLT_7_7_0_0_V = PMIC_ADPT_VOLT_7_7,
	UPMU_VOLT_7_8_0_0_V = PMIC_ADPT_VOLT_7_8,
	UPMU_VOLT_7_9_0_0_V = PMIC_ADPT_VOLT_7_9,
	UPMU_VOLT_8_0_0_0_V = PMIC_ADPT_VOLT_8_0,

	//new
	UPMU_VOLT_1_5_2_0_V = PMIC_ADPT_VOLT_1_5_2_0_V, 
	UPMU_VOLT_1_5_4_0_V = PMIC_ADPT_VOLT_1_5_4_0_V, 
	UPMU_VOLT_1_5_6_0_V = PMIC_ADPT_VOLT_1_5_6_0_V,	
	UPMU_VOLT_1_5_8_0_V = PMIC_ADPT_VOLT_1_5_8_0_V, 
	UPMU_VOLT_1_6_2_0_V = PMIC_ADPT_VOLT_1_6_2_0_V, 
	UPMU_VOLT_1_6_4_0_V = PMIC_ADPT_VOLT_1_6_4_0_V, 
	UPMU_VOLT_1_6_6_0_V = PMIC_ADPT_VOLT_1_6_6_0_V, 
	UPMU_VOLT_1_6_8_0_V = PMIC_ADPT_VOLT_1_6_8_0_V, 
	UPMU_VOLT_1_7_2_0_V = PMIC_ADPT_VOLT_1_7_2_0_V, 
	UPMU_VOLT_1_7_4_0_V = PMIC_ADPT_VOLT_1_7_4_0_V, 
	UPMU_VOLT_1_7_6_0_V = PMIC_ADPT_VOLT_1_7_6_0_V, 
	UPMU_VOLT_1_7_8_0_V = PMIC_ADPT_VOLT_1_7_8_0_V,
	UPMU_VOLT_1_8_2_0_V = PMIC_ADPT_VOLT_1_8_2_0_V, 
	UPMU_VOLT_1_8_4_0_V = PMIC_ADPT_VOLT_1_8_4_0_V, 
	UPMU_VOLT_1_8_6_0_V = PMIC_ADPT_VOLT_1_8_6_0_V, 
	UPMU_VOLT_1_8_8_0_V = PMIC_ADPT_VOLT_1_8_8_0_V,
	UPMU_VOLT_1_9_2_0_V = PMIC_ADPT_VOLT_1_9_2_0_V, 
	UPMU_VOLT_1_9_4_0_V = PMIC_ADPT_VOLT_1_9_4_0_V, 
	UPMU_VOLT_1_9_6_0_V = PMIC_ADPT_VOLT_1_9_6_0_V, 
	UPMU_VOLT_1_9_8_0_V = PMIC_ADPT_VOLT_1_9_8_0_V,
	UPMU_VOLT_2_0_2_0_V = PMIC_ADPT_VOLT_2_0_2_0_V, 
	UPMU_VOLT_2_0_4_0_V = PMIC_ADPT_VOLT_2_0_4_0_V, 
	UPMU_VOLT_2_0_6_0_V = PMIC_ADPT_VOLT_2_0_6_0_V, 
	UPMU_VOLT_2_0_8_0_V = PMIC_ADPT_VOLT_2_0_8_0_V,
	UPMU_VOLT_2_1_2_0_V = PMIC_ADPT_VOLT_2_1_2_0_V,
	
	UPMU_VOLT_MAX = PMIC_ADPT_VOLT_MAX
}upmu_buck_vol_enum, upmu_ldo_vol_enum;

#define UPMU_MAX_BUCK_VOL_SEL_NUM   32
typedef struct
{
	kal_uint32 addr;
	kal_uint32 vol_list_num;  // 1: Means the voltage is fixed, not allow to configure
	upmu_buck_vol_enum vol_list[UPMU_MAX_BUCK_VOL_SEL_NUM];
}upmu_buck_profile_entry;

#define UPMU_MAX_LDO_VOL_SEL_NUM   16
typedef struct
{
	kal_uint32 addr;
	kal_uint32 vol_list_num;  // 1: Means the voltage is fixed, not allow to configure
	upmu_ldo_vol_enum vol_list[UPMU_MAX_LDO_VOL_SEL_NUM];
}upmu_ldo_profile_entry;


/* a enum type to identify which of
 * pmic's hardware resource
 */
enum hwrs_id {
	VOLT_VTCXO_PMU,
	CTRL_EXT_PMIC_EN,
	NR_HWRS,
};

/* operation set includes the action
 * at suspend/resume state
 */
struct hwrs_ops {
	int (*suspend)(struct hwrs *hwrs);
	int (*resume)(struct hwrs *hwrs);
};

/* this struct definition is used to
 * record the hardware resource usage,
 * which includes voltage/control pins
 * from pmic.
 */
struct hwrs {
	unsigned int id;
	const char *name;
	unsigned int cnt;
	struct hwrs_ops *ops;
};

/* extern declarion for API
 */
extern void pmic_hwrs_claim(enum hwrs_id hwrs_id);
extern void pmic_hwrs_release(enum hwrs_id hwrs_id);

#endif // _MT6323_PMIC_SW_H_

