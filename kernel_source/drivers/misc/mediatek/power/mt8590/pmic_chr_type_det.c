/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
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
#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <linux/earlysuspend.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h>

#include <mach/upmu_common.h>
#include <mach/upmu_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_pmic_wrap.h>
#include <mach/mt_gpio.h>
#include <mach/mtk_rtc.h>
#include <mach/mt_spm_mtcmos.h>

#include <mach/battery_common.h>
#include <linux/time.h>

#if (defined(CONFIG_ARCH_MT8590_ICX))
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

#endif /* (defined(CONFIG_ARCH_MT8590_ICX)) */

// ============================================================ //
//extern function
// ============================================================ //
extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
extern void Charger_Detect_Init(void);
extern void Charger_Detect_Release(void);

static void hw_bc11_init(void)
{
	msleep(300);
	 Charger_Detect_Init();
		 
	 //RG_BC11_BIAS_EN=1	
	 upmu_set_rg_bc11_bias_en(0x1);
	 //RG_BC11_VSRC_EN[1:0]=00
	 upmu_set_rg_bc11_vsrc_en(0x0);
	 //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
	 //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	 //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	 //BC11_RST=1
	 upmu_set_rg_bc11_rst(0x1);
	 //BC11_BB_CTRL=1
	 upmu_set_rg_bc11_bb_ctrl(0x1);
 
 	 //msleep(10);
#if (!defined(CONFIG_ARCH_MT8590_ICX))
/* MTK reference platform. */
 	 mdelay(50);
#else /* (!defined(CONFIG_ARCH_MT8590_ICX)) */
/* SONY sound platform. */
	 msleep(50);
#endif /* (!defined(CONFIG_ARCH_MT8590_ICX)) */
}

static U32 hw_bc11_DCD(void)
{
	 U32 wChargerAvail = 0;
 
	 //RG_BC11_IPU_EN[1.0] = 10
	 upmu_set_rg_bc11_ipu_en(0x2);
	 //RG_BC11_IPD_EN[1.0] = 01
	 upmu_set_rg_bc11_ipd_en(0x1);
	 //RG_BC11_VREF_VTH = [1:0]=01
	 upmu_set_rg_bc11_vref_vth(0x1);
	 //RG_BC11_CMP_EN[1.0] = 10
	 upmu_set_rg_bc11_cmp_en(0x2);
 
	 //msleep(20);
#if (!defined(CONFIG_ARCH_MT8590_ICX))
/* MTK reference platform. */
	 mdelay(80);
#else /* (!defined(CONFIG_ARCH_MT8590_ICX)) */
/* SONY sound platform. */
	 msleep(80);
#endif /* (!defined(CONFIG_ARCH_MT8590_ICX)) */

 	 wChargerAvail = upmu_get_rgs_bc11_cmp_out();
	 
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	 //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	 //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	 //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
 
	 return wChargerAvail;
}

#ifndef MTK_PMIC_MT6397 //only for mt6323 use
static U32 hw_bc11_stepA1(void)
{
	U32 wChargerAvail = 0;
	  
	//RG_BC11_IPU_EN[1.0] = 10
	upmu_set_rg_bc11_ipu_en(0x2);
	//RG_BC11_VREF_VTH = [1:0]=10
	upmu_set_rg_bc11_vref_vth(0x2);
	//RG_BC11_CMP_EN[1.0] = 10
	upmu_set_rg_bc11_cmp_en(0x2);
 
	//msleep(80);
#if (!defined(CONFIG_ARCH_MT8590_ICX))
/* MTK reference platform. */
	mdelay(80);
#else /* (!defined(CONFIG_ARCH_MT8590_ICX)) */
/* SONY sound platform. */
	msleep(80);
#endif /* (!defined(CONFIG_ARCH_MT8590_ICX)) */
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
 
	return  wChargerAvail;
}
#endif
static U32 hw_bc11_stepA2(void)
{
	U32 wChargerAvail = 0;
	  
	//RG_BC11_VSRC_EN[1.0] = 10 
	upmu_set_rg_bc11_vsrc_en(0x2);
	//RG_BC11_IPD_EN[1:0] = 01
	upmu_set_rg_bc11_ipd_en(0x1);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
	//RG_BC11_CMP_EN[1.0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	//msleep(80);
#if (!defined(CONFIG_ARCH_MT8590_ICX))
/* MTK reference platform. */
	mdelay(80);
#else /* (!defined(CONFIG_ARCH_MT8590_ICX)) */
/* SONY sound platform. */
	msleep(80);
#endif /* (!defined(CONFIG_ARCH_MT8590_ICX)) */
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	//RG_BC11_VSRC_EN[1:0]=00
	upmu_set_rg_bc11_vsrc_en(0x0);
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
 
	return  wChargerAvail;
}

static U32 hw_bc11_stepB2(void)
{
	U32 wChargerAvail = 0;
 
	//RG_BC11_IPU_EN[1:0]=10
	upmu_set_rg_bc11_ipu_en(0x2);
	//RG_BC11_VREF_VTH = [1:0]=10
	upmu_set_rg_bc11_vref_vth(0x1);
	//RG_BC11_CMP_EN[1.0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	//msleep(80);
#if (!defined(CONFIG_ARCH_MT8590_ICX))
/* MTK reference platform. */
	mdelay(80);
#else /* (!defined(CONFIG_ARCH_MT8590_ICX)) */
/* SONY sound platform. */
	msleep(80);
#endif /* (!defined(CONFIG_ARCH_MT8590_ICX)) */
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
 
	return  wChargerAvail;
 }

#if (defined(CONFIG_ARCH_MT8590_ICX))
#define	MT6323_BCDET_STEP_END	((uint32_t)0xffffffff)
#define	MT6323_BCDET_STEP_SKIP	((uint32_t)0xfffffffe)

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

/*! Check contact DP and DM.
*/
static bool mt6323_bcdet_is_contact_dpdm(void)
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

#endif /* (defined(CONFIG_ARCH_MT8590_ICX)) */

static void hw_bc11_done(void)
{
	//RG_BC11_VSRC_EN[1:0]=00
	upmu_set_rg_bc11_vsrc_en(0x0);
	//RG_BC11_VREF_VTH = [1:0]=0
	upmu_set_rg_bc11_vref_vth(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(0x0);
	//RG_BC11_BIAS_EN=0
	upmu_set_rg_bc11_bias_en(0x0); 
 
	Charger_Detect_Release();
}
#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)

CHARGER_TYPE hw_charger_type_detection(void)
{
    return STANDARD_HOST;
}

#else

#ifdef MTK_PMIC_MT6397 //for mt6397 detect flow

CHARGER_TYPE hw_charger_type_detection(void)
{
    CHARGER_TYPE ret = CHARGER_UNKNOWN;
	hw_bc11_init();
 
	if(1 == hw_bc11_DCD())
	{
		 ret = NONSTANDARD_CHARGER;
	} else {
		 if(1 == hw_bc11_stepA2())
		 {
			 if(1 == hw_bc11_stepB2())
			 {
				 ret = STANDARD_CHARGER;
			 } else {
				 ret = CHARGING_HOST;
			 }
		 } else {
             ret = STANDARD_HOST;
		 }
	}
	hw_bc11_done();

	return ret;
}
#else //for mt6323 detect flow

CHARGER_TYPE hw_charger_type_detection(void)
{
    CHARGER_TYPE ret = CHARGER_UNKNOWN;
	hw_bc11_init();
#if (defined(CONFIG_ARCH_MT8590_ICX))
#define	CONTACT_STABLE_COUNT		(4)
#define	CONTACT_POLL_MS			(1)
#define	CONTACT_CHECK_TIMEOUT_MS	(600)
/* #define	CONTACT_CHECK_DEBUG */
	/* always do */ {
		int		contact_count;
		unsigned long	jiffies_end;

		jiffies_end = jiffies + msecs_to_jiffies(CONTACT_CHECK_TIMEOUT_MS);

		contact_count = 0;
		while (time_before(jiffies, jiffies_end)) {
			if (mt6323_bcdet_is_contact_dpdm()) {
				contact_count++;
				if (contact_count >= CONTACT_STABLE_COUNT) {
#if (defined(CONTACT_CHECK_DEBUG))
					printk("%s: Contacted.\n", __func__);
#endif /* (defined(CONTACT_CHECK_DEBUG)) */
					break;
				}
			} else {
				contact_count = 0;
			}
			msleep(CONTACT_POLL_MS);
		}
#if (defined(CONTACT_CHECK_DEBUG))
		printk("%s: Contact check done. contact_count=%d, remain=%ld\n"
			, __func__, contact_count,
			((long)(jiffies_end) - (long)(jiffies))
		);
#endif /* (defined(CONTACT_CHECK_DEBUG)) */
		/* @note Detect "open DP and DM line",
		   check if "contact_count < CONTACT_STABLE_COUNT",
		   it becomes true, then DP and DM are open.
		*/
		if (contact_count < CONTACT_STABLE_COUNT) {
			/* open DP and DM. */
			/* Off charger detector. */
			hw_bc11_done();
			ret = CHARGER_UNKNOWN;
			return ret;
		}
	}
#endif /* (defined(CONFIG_ARCH_MT8590_ICX)) */
	if(1 == hw_bc11_DCD())
	{
		ret = NONSTANDARD_CHARGER;
	} else {
		 if(1 == hw_bc11_stepA2())
		 {
			 if(1 == hw_bc11_stepB2())
			 {
				 ret = STANDARD_CHARGER;
			 } else {
				 ret = CHARGING_HOST;
			 }
		 } else {
			 ret = STANDARD_HOST;
		 }
	}
	hw_bc11_done();

	return ret;
}
#endif
#endif
