/*
 * Copyright 2016,2017 Sony Corporation
 * File changed on 2017-01-25
 */
/*
* Copyright (C) 2011-2015 MediaTek Inc.
*
* Copyright 2015 Sony Corporation.
* Author: Sony Corporation.
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

/*
 * ChangeLog:
 *   2015 changed by Sony Corporation
 *     Implement ICX platform features.
 */

/* Use this module on MT8590 DMP with WM-PORT target.
   The target has MT6323 PMIC, BQ24296 charger, and WM-PORT VBUS/DCIN power path.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

/* forward declare structure name. */
struct hwrs;

#include <mach/mt_typedefs.h>
#include <mach/upmu_common.h>
#include <mach/upmu_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_pmic_wrap.h>
#include <mach/mt_gpio.h>

#include <mach/battery_common.h>

#include <linux/power/pmic_chr_type_det_icx.h>

// ============================================================ //
//extern function
// ============================================================ //
extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
extern void Charger_Detect_Init(void);
extern void Charger_Detect_Release(void);


#define	DEBUG_FLAG_LIGHT	(0x0001)
#define	DEBUG_FLAG_FUNC		(0x0002)
#define	DEBUG_FLAG_HEAVY	(0x0004)

/*! @terminology
    _bcdet_: Battery Cherger DETect.
*/

/*! Live debug control. */
uint mt6323_bcdet_debug =
	  0
	| DEBUG_FLAG_LIGHT
	/* | DEBUG_FLAG_FUNC */
	/* | DEBUG_FLAG_HEAVY */
	;
module_param_named(debug, mt6323_bcdet_debug, uint, S_IWUSR | S_IRUGO);

uint	mt6323_bcdet_dpdm_probe_wait_ms = 0;
module_param_named(dpdm_probe_wait_ms, mt6323_bcdet_dpdm_probe_wait_ms, uint, S_IWUSR | S_IRUGO);

/*! Debug: Do light weight printk.
    define: Do light weight printk.
    undef:  Do not light weight printk.
*/
#define	CONFIG_PMIC_MT6323_BCDET_DEBUG_LIGHT_WEIGHT	(1)

#if (defined(CONFIG_PMIC_MT6323_BCDET_DEBUG_LIGHT_WEIGHT))
#define	PRINTK_LI(format, ...) \
	do {\
		if (mt6323_bcdet_debug&DEBUG_FLAG_LIGHT) {\
			printk(format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_LI(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Do printk at function entry/exit.
    define: Do printk at function entry/exit.
    undef: Do NOT printk at function entry/exit.
*/
#define	CONFIG_PMIC_MT6323_BCDET_DEBUG_FUNC_ENTRY

#if (defined(CONFIG_PMIC_MT6323_BCDET_DEBUG_FUNC_ENTRY))
#define	PRINTK_FUNC_ENTRY(format, ...) \
	do {\
		if (mt6323_bcdet_debug&DEBUG_FLAG_ENTRY) {\
			printk(format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_FUNC_ENTRY(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Do heavy weight printk.
    define: Do heavy weight printk.
    undef:  Do not heavy weight printk.
*/
#define	CONFIG_PMIC_MT6323_BCDET_DEBUG_HEAVY_WEIGHT	(1)

#if (defined(CONFIG_PMIC_MT6323_BCDET_DEBUG_HEAVY_WEIGHT))
#define	PRINTK_HV(format, ...) \
	do {\
		if (mt6323_bcdet_debug&DEBUG_FLAG_HEAVY) {\
			printk(format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_HV(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/* Charger Dtection Circuit

   [RG_BC11_BIAS_EN] 0:Disable, 1:Enable

   DP------*---------------------*------------------------*--------------------------*
   (DPL)   |                     |                        |                          |
         2 o                   2 o                      2 o                        2 o RG_BC11_CMP_EN
         0|o--(+E=0.6V)--GND   0|o--(<-I=7..15uA)--GND  0|o--(->I=50..150uA)--GND  0|o----(+)|
         1 o  RG_BC11_VSRC_EN  1 o  RG_BC_11_IPU_EN     1 o  RG_BC11_IPD_EN        1 o       |>- RG_BC11_CMP_OUT
           |                     |                        |                          |  +-(-)|
   DM------*---------------------+------------------------*--------------------------*  |
   (DMI)                                                                  (0.325V=E+)-/-* 0 RG_BC11_VREF_VTH
                                                                          (1.200V=E+)-/-* 1
                                                                          (2.600V=E+)-/-* 2 x NOT WORK.
                                                                          (2.600V=E+)-/-* 3 x NOT WORK.
*/

#define	PMIC_RG_BC11_VSRC_EN_OPEN	(0)
#define	PMIC_RG_BC11_VSRC_EN_DMI	(1)
#define	PMIC_RG_BC11_VSRC_EN_DPL	(2)

#define	PMIC_RG_BC11_IPU_EN_OPEN	(0)
#define	PMIC_RG_BC11_IPU_EN_DMI		(1)
#define	PMIC_RG_BC11_IPU_EN_DPL		(2)

#define	PMIC_RG_BC11_IPD_EN_OPEN	(0)
#define	PMIC_RG_BC11_IPD_EN_DMI		(1)
#define	PMIC_RG_BC11_IPD_EN_DPL		(2)

#define	PMIC_RG_BC11_CMP_EN_OPEN	(0)
#define	PMIC_RG_BC11_CMP_EN_DMI		(1)
#define	PMIC_RG_BC11_CMP_EN_DPL		(2)

#define	PMIC_RG_BC11_VREF_VTH_0R325	(0)
#define	PMIC_RG_BC11_VREF_VTH_1R200	(1)
#define	PMIC_RG_BC11_VREF_VTH_2R600	(2)	/* May not work, always CMP_OUT=1 */

/* Init BC11 charger detect circuit, all off.
*/
void mt6323_bcdet_init(void)
{
	Charger_Detect_Init();
	if (mt6323_bcdet_dpdm_probe_wait_ms != 0) {
		msleep(mt6323_bcdet_dpdm_probe_wait_ms);
	}
	//RG_BC11_BIAS_EN=1	
	upmu_set_rg_bc11_bias_en(0x1);
	//RG_BC11_VSRC_EN[1:0]=00
	upmu_set_rg_bc11_vsrc_en(PMIC_RG_BC11_VSRC_EN_OPEN);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_0R325);
	 //RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_OPEN);
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(PMIC_RG_BC11_IPU_EN_OPEN);
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(PMIC_RG_BC11_IPD_EN_OPEN);
	//BC11_RST=1
	upmu_set_rg_bc11_rst(0x1);
	//BC11_BB_CTRL=1
	upmu_set_rg_bc11_bb_ctrl(0x1);

	msleep(50);
}

/* DCD
   circuit:
   DP ----*-----------*
          |           |
          +-<I=10uA   +----(+)|
                              |>--cmp_out
          +->I=100uA  1.2V-(-)|
          |
   DM ----*-----------
*/

static U32 hw_bc11_DCD(void)
{
	 U32 wChargerAvail = 0;

	//RG_BC11_IPU_EN[1.0] = 10
	/* Source 10uA to DP */
	upmu_set_rg_bc11_ipu_en(PMIC_RG_BC11_IPU_EN_DPL);
	//RG_BC11_IPD_EN[1.0] = 01
	/* Sink 100uA from DM. */
	upmu_set_rg_bc11_ipd_en(PMIC_RG_BC11_IPD_EN_DMI);
	//RG_BC11_VREF_VTH = [1:0]=01
	/* Set 1.2V to Cmp(-) */
	upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_1R200);
	//RG_BC11_CMP_EN[1.0] = 10
	/* Connect DP to Cmp(+) */
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_DPL);

	msleep(80);
	/* 1: Not connected DP, DM. 0: connected DP, DM or DP pull downed. */
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();

	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(PMIC_RG_BC11_IPU_EN_OPEN);
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(PMIC_RG_BC11_IPD_EN_OPEN);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_OPEN);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_0R325);

	return wChargerAvail;
}

/* A1
   circuit:
   DP ----*-----------*
          |           |
          +-<I=10uA   +-----(+)|
                               |>--cmp_out
                      2.6V--(-)|

   DM ----------------
*/

static U32 hw_bc11_stepA1(void) /* Use after _DCD() */
{
	U32 wChargerAvail = 0;
	  
	//RG_BC11_IPU_EN[1.0] = 10
	/* Source 10uA to DP. */
	upmu_set_rg_bc11_ipu_en(PMIC_RG_BC11_IPU_EN_DPL);
	//RG_BC11_VREF_VTH = [1:0]=10
	/* Set Cmp(-)=2.6V */
	upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_2R600);
	//RG_BC11_CMP_EN[1.0] = 10
	/* Connect DP to Cmp(+) */
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_DPL);

	msleep(80);
	/* 1: Above 2.6V it may apple 2.1A USB-AC or Open DPDM, 0: Not above 2.6V apple 1.0A, 0.5A USB-AC.
	   @note but we think it doesn't tell truth.
	*/
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();

	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(PMIC_RG_BC11_IPU_EN_OPEN);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_OPEN);

	return  wChargerAvail;
}

/* A2
   circuit:
   DP ----*------------
          |            
          +-(+E=0.6V)  +--------(+)|
                       |           |>--cmp_out
          +->I=100uA   | 0.325V-(-)|
          |            |
   DM ----*------------*
*/

static U32 hw_bc11_stepA2(void)
{
	U32 wChargerAvail = 0;
	  
	//RG_BC11_VSRC_EN[1.0] = 10 
	/* Source 0.6V to DP. */
	upmu_set_rg_bc11_vsrc_en(PMIC_RG_BC11_VSRC_EN_DPL);
	//RG_BC11_IPD_EN[1:0] = 01
	/* Sink 100uA from DM. */
	upmu_set_rg_bc11_ipd_en(PMIC_RG_BC11_IPD_EN_DMI);
	//RG_BC11_VREF_VTH = [1:0]=00
	/* Set 0.325V to Cmp(-) */
	upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_0R325);
	//RG_BC11_CMP_EN[1.0] = 01
	/* Connect DM to Cmp(+). */
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_DMI);

	msleep(80);
	/* 1: Connect DP, DM low resistor, 0: Not connected DP, DM low register. */
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();

	//RG_BC11_VSRC_EN[1:0]=00
	upmu_set_rg_bc11_vsrc_en(PMIC_RG_BC11_VSRC_EN_OPEN);
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(PMIC_RG_BC11_IPD_EN_OPEN);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_OPEN);
 
	return  wChargerAvail;
}

/* B2
   circuit:
   DP ----*------------
          |            
          +-<I=10uA    +--------(+)|
                       |           |>--cmp_out
                       |   1.2V-(-)|
                       |
   DM -----------------*
*/

static U32 hw_bc11_stepB2(void)
{
	U32 wChargerAvail = 0;
 
	//RG_BC11_IPU_EN[1:0]=10
	/* Source 10uA to DP. */
	upmu_set_rg_bc11_ipu_en(PMIC_RG_BC11_IPU_EN_DPL);
	//RG_BC11_VREF_VTH = [1:0]=10
	/* Set 1.2V to Cmp(-) */
	upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_1R200);
	//RG_BC11_CMP_EN[1.0] = 01
	/* Connect DM to Cmp(+) */
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_DMI);

	msleep(80);
	/* 1: DP,DM are not pull downed, 0: DP,DM are Pull downed. */
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();

	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(PMIC_RG_BC11_IPU_EN_OPEN);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_OPEN);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_0R325);

	return  wChargerAvail;
}

#define	MT6323_BCDET_STEP_END	((uint32_t)0xffffffff)
#define	MT6323_BCDET_STEP_SKIP	((uint32_t)0xfffffffe)


/* SINK_OPEN(1)
   circuit:
   DP ----*------------+
          |            |
          +->I=100uA   /--------(+)|
                       |           |>--cmp_out
                       | 0.325V-(-)|
                       |
   DM -----------------

   DP=L, DM=L AC-S508U or CDP or DCP or STANDARD_HOST
   DP=H, DM=L Unknown
   DP=L, DM=H APL_0R5, APL_1R0, (APL_2R1)
   DP=H, DM=H (APL_2R1)
*/

/* SINK_OPEN(2)
   circuit:
   DP -----------------+
                       |
                       /--------(+)|
                       |           |>--cmp_out
          +->I=100uA   | 0.325V-(-)|
          |            |
   DM ----*------------+

 DP=L, DM=L AC-S508U
 DP=H, DM=L APL_0R5, (APL_1R0), APL_2R1
 DP=L, DM=H Unknown
 DP=H, DM=H (APL_1R0)
*/

/*! Pull down sink detect algorithm step.
*/
struct mt6323_bcdet_ipd_step {
	uint32_t	ipd;	/*!< pull down (current sink) selector.	*/
	uint32_t	cmp;	/*!< comparator input selector.		*/
};

static const struct mt6323_bcdet_ipd_step mt6323_bcdet_sink_open_ac_s508u[] = {	/* CMP_OUT */
	/* pull down selector */   /* comparator selector */		/* AC-S508U, apple */
	{PMIC_RG_BC11_IPD_EN_DPL,  PMIC_RG_BC11_CMP_EN_DMI},		/* 0,        1 */
	{PMIC_RG_BC11_IPD_EN_DMI,  PMIC_RG_BC11_CMP_EN_DPL},		/* 0,        1 */
	{PMIC_RG_BC11_IPD_EN_OPEN, PMIC_RG_BC11_CMP_EN_DPL},		/* 1,        1 */
	{PMIC_RG_BC11_IPD_EN_OPEN, PMIC_RG_BC11_CMP_EN_DMI},		/* 1,        1 */
	{PMIC_RG_BC11_IPD_EN_DMI,  PMIC_RG_BC11_CMP_EN_DPL},		/* 0,        1 */
	{PMIC_RG_BC11_IPD_EN_DPL,  PMIC_RG_BC11_CMP_EN_DMI},		/* 0,        1 */
	{PMIC_RG_BC11_IPD_EN_OPEN, PMIC_RG_BC11_CMP_EN_DPL},		/* 1,        1 */
	{PMIC_RG_BC11_IPD_EN_OPEN, PMIC_RG_BC11_CMP_EN_DMI},		/* 1,        1 */
	{MT6323_BCDET_STEP_END, MT6323_BCDET_STEP_END},
};

/*! @note according to observed wave form, wait time should be greater than 2ms. */
#define	MT6323_BCDET_IPD_STEP_WAIT_MS	(8)
#define	SINK_OPEN_MATCH_AC_S508U	 (0x33)
#define	SINK_OPEN_MATCH_OPEN_DPDM	(0x000)
#define	SINK_OPEN_MATCH_APL_ANY		(0x1ef)

static unsigned mt6323_bcdet_ipd_step_run(const struct mt6323_bcdet_ipd_step *step)
{	unsigned	cmps;

	cmps = 0;
	while (   (step->ipd != MT6323_BCDET_STEP_END) 
	       && (step->cmp != MT6323_BCDET_STEP_END)
	       && (step->cmp != PMIC_RG_BC11_CMP_EN_OPEN)
	      ) {
		upmu_set_rg_bc11_ipd_en(step->ipd);
		upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_1R200);
		upmu_set_rg_bc11_cmp_en(step->cmp);
		msleep(MT6323_BCDET_IPD_STEP_WAIT_MS);
		cmps <<= 1;
		cmps |= upmu_get_rgs_bc11_cmp_out();
		step++;
	}
	upmu_set_rg_bc11_ipd_en(PMIC_RG_BC11_IPD_EN_OPEN);
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_OPEN);
	upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_0R325);
	return cmps;
}

struct mt6323_bcdet_ipu_ipd_step {
	uint32_t	ipu;	/*!< pull up(current source) selector.	*/
	uint32_t	ipd;	/*!< pull down(current sink) selector.	*/
	uint32_t	vth;	/*!< comparator threshold level.	*/
	uint32_t	cmp;	/*!< comparator input selector.		*/
	int32_t		wait_ms;
};

static const struct mt6323_bcdet_ipu_ipd_step mt6323_bcdet_contact_std_cdp_dcp[] = {				       /* CMP_OUT @ STD/CDP/DCP */
	{PMIC_RG_BC11_IPU_EN_DPL,  PMIC_RG_BC11_IPD_EN_DMI,  PMIC_RG_BC11_VREF_VTH_0R325, PMIC_RG_BC11_CMP_EN_DMI, 5}, /* 0 */
	{PMIC_RG_BC11_IPU_EN_DPL,  PMIC_RG_BC11_IPD_EN_DMI,  PMIC_RG_BC11_VREF_VTH_0R325, PMIC_RG_BC11_CMP_EN_DPL, 1}, /* 0 */
	{MT6323_BCDET_STEP_END,    MT6323_BCDET_STEP_END,    MT6323_BCDET_STEP_END,       MT6323_BCDET_STEP_END,  -1},
};

#define	CONTACT_STD_DCP_DCP_MATCH	(0x0)

static const struct mt6323_bcdet_ipu_ipd_step mt6323_bcdet_contact_charger[] = {				         /* CMP_OUT @ apple/S508U */
	{PMIC_RG_BC11_IPU_EN_OPEN,  PMIC_RG_BC11_IPD_EN_OPEN,  PMIC_RG_BC11_VREF_VTH_1R200, PMIC_RG_BC11_CMP_EN_DMI, 5}, /* 1 */
	{PMIC_RG_BC11_IPU_EN_OPEN,  PMIC_RG_BC11_IPD_EN_OPEN,  PMIC_RG_BC11_VREF_VTH_1R200, PMIC_RG_BC11_CMP_EN_DPL, 1}, /* 1 */
	{MT6323_BCDET_STEP_END,     MT6323_BCDET_STEP_END,     MT6323_BCDET_STEP_END,       MT6323_BCDET_STEP_END,  -1},
};

#define	CONTACT_CHARGER_MATCH	(0x3)

static unsigned mt6323_bcdet_ipu_ipd_step_run(const struct mt6323_bcdet_ipu_ipd_step *step)
{	unsigned	cmps;

	cmps = 0;
	while (   (step->ipu != MT6323_BCDET_STEP_END)
	       && (step->ipd != MT6323_BCDET_STEP_END)
	       && (step->vth != MT6323_BCDET_STEP_END)
	       && (step->cmp != MT6323_BCDET_STEP_END)
	       && (step->cmp != PMIC_RG_BC11_CMP_EN_OPEN)
	       && (step->wait_ms > 0)
	      ) {
		upmu_set_rg_bc11_ipu_en(step->ipu);
		upmu_set_rg_bc11_ipd_en(step->ipd);
		upmu_set_rg_bc11_vref_vth(step->vth);
		upmu_set_rg_bc11_cmp_en(step->cmp);
		if (step->wait_ms > 0) {
			msleep((unsigned long)(step->wait_ms));
		}
		cmps <<= 1;
		cmps |= upmu_get_rgs_bc11_cmp_out();
		step++;
	}
	upmu_set_rg_bc11_ipu_en(PMIC_RG_BC11_IPD_EN_OPEN);
	upmu_set_rg_bc11_ipd_en(PMIC_RG_BC11_IPD_EN_OPEN);
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_OPEN);
	upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_0R325);
	return cmps;
}


void mt6323_bcdet_done(void)
{
	//RG_BC11_VSRC_EN[1:0]=00
	upmu_set_rg_bc11_vsrc_en(PMIC_RG_BC11_VSRC_EN_OPEN);
	//RG_BC11_VREF_VTH = [1:0]=0
	upmu_set_rg_bc11_vref_vth(PMIC_RG_BC11_VREF_VTH_0R325);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(PMIC_RG_BC11_CMP_EN_OPEN);
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(PMIC_RG_BC11_IPU_EN_OPEN);
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(PMIC_RG_BC11_IPD_EN_OPEN);
	//RG_BC11_BIAS_EN=0
	upmu_set_rg_bc11_bias_en(0x0);

	Charger_Detect_Release();
}

/* Other platform. */
unsigned hw_charger_type_detection(void)
{	pr_err("%s: ERROR: Unexpected function call, INTENTIONALLY DUMP STACK, return STANDARD_HOST.\n", __func__);
	dump_stack();
	return STANDARD_HOST;
}

/* ICX with WM-PORT platform. */
unsigned mt6323_bcdet_type_detection(void)
{
	unsigned ret = ICX_CHARGER_UNKNOWN;
	mt6323_bcdet_init();

	if (hw_bc11_DCD() == 1) {
		/* Not connected DP, DM. */
		PRINTK_HV("%s.DCD_A1_DPDM: apple any.\n", __func__);
		/* We treat all apple adapters as 0.5A output adapters. */
		ret = ICX_CHARGER_APL_0R5;
	} else {
		/* "Connected DP, DM" or "DP pull downed". */
		if (hw_bc11_stepA2() == 1){
			/* Connected DP, DM low resistor. */
			if (hw_bc11_stepB2() == 1) {
				/* DP and DM are not pull downed. */
				PRINTK_HV("%s.DCD_A2_B2: DCP.\n", __func__);
				ret = ICX_CHARGER_DCP;
			} else {
				/* pull downed. */
				PRINTK_HV("%s.DCD_A2_B2: CDP port.\n", __func__);
				ret = ICX_CHARGER_CDP;
			}
		} else {
			/* Not connected DP, DM low resistor, DM keeps low level. */
			unsigned	cmps;
			cmps = mt6323_bcdet_ipd_step_run(mt6323_bcdet_sink_open_ac_s508u);
			if (cmps == SINK_OPEN_MATCH_AC_S508U) {
				PRINTK_HV("%s.DCD_A_SO: AC-S508U. cmps=0x%.2x\n", __func__, cmps);
				ret = ICX_CHARGER_AC_S508U;
			} else {
				PRINTK_HV("%s.DCD_A_SO: Standard port. cmps=0x%.2x\n", __func__, cmps);
				ret = ICX_CHARGER_STD;
			}
		}
	}
	mt6323_bcdet_done();
	msleep(100);
	return ret;
}

/*! Check contact DP and DM.
*/
bool mt6323_bcdet_is_contact_dpdm(void)
{	unsigned	cmps;
	int		result;

	result = false;
	cmps = mt6323_bcdet_ipu_ipd_step_run(mt6323_bcdet_contact_std_cdp_dcp);
	if (cmps == CONTACT_STD_DCP_DCP_MATCH) {
		result = true;
		goto out;
	}
	cmps = mt6323_bcdet_ipu_ipd_step_run(mt6323_bcdet_contact_charger);
	if (cmps == CONTACT_CHARGER_MATCH) {
		result = true;
		goto out;
	}
out:
	return result;
}
