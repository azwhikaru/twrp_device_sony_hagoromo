/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*******************************************************************************
* mt_pwm.c PWM Drvier
*
* Copyright (c) 2012, Media Teck.inc
*
* This program is free software; you can redistribute it and/or modify it
* under the terms and conditions of the GNU General Public Licence,
* version 2, as publish by the Free Software Foundation.
*
* This program is distributed and in hope it will be useful, but WITHOUT
* ANY WARRNTY; without even the implied warranty of MERCHANTABITLITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
*
********************************************************************************
* Author : Changlei Gao (changlei.gao@mediatek.com)
********************************************************************************
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <generated/autoconf.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/atomic.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_pwm.h>
#include <mach/mt_pwm_hal_pub.h>

struct pwm_device {
	const char *name;
	atomic_t ref;
	dev_t devno;
	spinlock_t lock;
	unsigned long power_flag;	/* bitwise, bit(8):map to MT_CG_PERI0_PWM */
	struct device dev;
};

static struct pwm_device pwm_dat = {
	.name = PWM_DEVICE,
	.ref = ATOMIC_INIT(0),
	.power_flag = 0,
	.lock = __SPIN_LOCK_UNLOCKED(pwm_dat.lock)
};

static struct pwm_device *pwm_dev = &pwm_dat;

static void mt_pwm_power_on(U32 pwm_no, BOOL pmic_pad)
{
	mt_pwm_power_on_hal(pwm_no, pmic_pad, &(pwm_dev->power_flag));
}

static void mt_pwm_power_off(U32 pwm_no, BOOL pmic_pad)
{
	mt_pwm_power_off_hal(pwm_no, pmic_pad, &(pwm_dev->power_flag));
}

static S32 mt_pwm_sel_pmic(U32 pwm_no)
{
	unsigned long flags;
	S32 ret;
	struct pwm_device *dev = pwm_dev;

	spin_lock_irqsave(&dev->lock, flags);
	ret = mt_pwm_sel_pmic_hal(pwm_no);
	spin_unlock_irqrestore(&dev->lock, flags);

	if (ret == (-EEXCESSPWMNO))
		PWMDBG("PWM1~PWM4 not support pmic_pad\n");

	return ret;
}

static S32 mt_pwm_sel_ap(U32 pwm_no)
{
	unsigned long flags;
	S32 ret;
	struct pwm_device *dev = pwm_dev;

	spin_lock_irqsave(&dev->lock, flags);
	ret = mt_pwm_sel_ap_hal(pwm_no);
	spin_unlock_irqrestore(&dev->lock, flags);

	if (ret == (-EEXCESSPWMNO))
		PWMDBG("PWM1~PWM4 not support pmic_pad\n");

	return ret;
}

/*******************************************************
*   Set PWM_ENABLE register bit to enable pwm1~pwm7
*
********************************************************/
static S32 mt_set_pwm_enable(U32 pwm_no)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;

	if (!dev) {
		PWMDBG("dev is not valid!\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number is not between PWM1~PWM7\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_enable_hal(pwm_no);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}


/*******************************************************/
static S32 mt_set_pwm_disable(U32 pwm_no)
{
	unsigned long flags;
	struct pwm_device *dev = pwm_dev;

	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number is not between PWM1~PWM7\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_disable_hal(pwm_no);
	spin_unlock_irqrestore(&dev->lock, flags);

	mdelay(1);

	return RSUCCESS;
}

void mt_pwm_disable(U32 pwm_no, BOOL pmic_pad)
{
	mt_set_pwm_disable(pwm_no);
	mt_pwm_power_off(pwm_no, pmic_pad);
}

void mt_set_pwm_enable_seqmode(void)
{
	unsigned long flags;
	struct pwm_device *dev = pwm_dev;

	if (!dev) {
		PWMDBG("dev is not valid\n");
		return;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_enable_seqmode_hal();
	spin_unlock_irqrestore(&dev->lock, flags);
}

void mt_set_pwm_disable_seqmode(void)
{
	unsigned long flags;
	struct pwm_device *dev = pwm_dev;

	if (!dev) {
		PWMDBG("dev is not valid\n");
		return;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_disable_seqmode_hal();
	spin_unlock_irqrestore(&dev->lock, flags);
}

S32 mt_set_pwm_test_sel(U32 val)	/* val as 0 or 1 */
{
	unsigned long flags;
	struct pwm_device *dev = pwm_dev;

	if (!dev) {
		PWMDBG("dev is not pwm_dev\n");
		return -EINVALID;
	}

	spin_lock_irqsave(&dev->lock, flags);
	if (mt_set_pwm_test_sel_hal(val))
		goto err;
	spin_unlock_irqrestore(&dev->lock, flags);
	return RSUCCESS;

 err:
	spin_unlock_irqrestore(&dev->lock, flags);
	return -EPARMNOSUPPORT;
}

S32 mt_set_pwm_clk(U32 pwm_no, U32 clksrc, U32 div)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	if (div >= CLK_DIV_MAX) {
		PWMDBG("division excesses CLK_DIV_MAX\n");
		return -EPARMNOSUPPORT;
	}

	if (clksrc > CLK_BLOCK_BY_1625_OR_32K) {
		PWMDBG("clksrc excesses CLK_BLOCK_BY_1625_OR_32K\n");
		return -EPARMNOSUPPORT;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_clk_hal(pwm_no, clksrc, div);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

S32 mt_get_pwm_clk(U32 pwm_no)
{
	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	return mt_get_pwm_clk_hal(pwm_no);
}

/******************************************
* Set PWM_CON register data source
* pwm_no: pwm1~pwm7(0~6)
*val: 0 is fifo mode
*       1 is memory mode
*******************************************/

static S32 mt_set_pwm_con_datasrc(U32 pwm_no, U32 val)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG(" pwm deivce doesn't exist\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	if (mt_set_pwm_con_datasrc_hal(pwm_no, val))
		goto err;
	spin_unlock_irqrestore(&dev->lock, flags);
	return RSUCCESS;

 err:
	spin_unlock_irqrestore(&dev->lock, flags);
	return -EPARMNOSUPPORT;
}


/************************************************
*  set the PWM_CON register
* pwm_no : pwm1~pwm7 (0~6)
* val: 0 is period mode
*        1 is random mode
*
***************************************************/
static S32 mt_set_pwm_con_mode(U32 pwm_no, U32 val)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	if (mt_set_pwm_con_mode_hal(pwm_no, val))
		goto err;
	spin_unlock_irqrestore(&dev->lock, flags);
	return RSUCCESS;

 err:
	spin_unlock_irqrestore(&dev->lock, flags);
	return -EPARMNOSUPPORT;
}

/***********************************************
*Set PWM_CON register, idle value bit
* val: 0 means that  idle state is not put out.
*       1 means that idle state is put out
*
*      IDLE_FALSE: 0
*      IDLE_TRUE: 1
***********************************************/
static S32 mt_set_pwm_con_idleval(U32 pwm_no, U16 val)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	if (mt_set_pwm_con_idleval_hal(pwm_no, val))
		goto err;
	spin_unlock_irqrestore(&dev->lock, flags);
	return RSUCCESS;

 err:
	spin_unlock_irqrestore(&dev->lock, flags);
	return -EPARMNOSUPPORT;
}

/*********************************************
* Set PWM_CON register guardvalue bit
*  val: 0 means guard state is not put out.
*        1 mens guard state is put out.
*
*    GUARD_FALSE: 0
*    GUARD_TRUE: 1
**********************************************/
static S32 mt_set_pwm_con_guardval(U32 pwm_no, U16 val)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	if (mt_set_pwm_con_guardval_hal(pwm_no, val))
		goto err;
	spin_unlock_irqrestore(&dev->lock, flags);
	return RSUCCESS;

 err:
	spin_unlock_irqrestore(&dev->lock, flags);
	return -EPARMNOSUPPORT;
}

/*************************************************
* Set PWM_CON register stopbits
*stop bits should be less then 0x3f
*
**************************************************/
static S32 mt_set_pwm_con_stpbit(U32 pwm_no, U32 stpbit, U32 srcsel)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}


	if (srcsel == PWM_FIFO) {
		if (stpbit > 0x3f) {
			PWMDBG("stpbit execesses the most of 0x3f in fifo mode\n");
			return -EPARMNOSUPPORT;
		}
	} else if (srcsel == MEMORY) {
		if (stpbit > 0x1f) {
			PWMDBG("stpbit excesses the most of 0x1f in memory mode\n");
			return -EPARMNOSUPPORT;
		}
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_con_stpbit_hal(pwm_no, stpbit, srcsel);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/*****************************************************
*Set PWM_CON register oldmode bit
* val: 0 means disable oldmode
*        1 means enable oldmode
*
*      OLDMODE_DISABLE: 0
*      OLDMODE_ENABLE: 1
******************************************************/

static S32 mt_set_pwm_con_oldmode(U32 pwm_no, U32 val)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	if (mt_set_pwm_con_oldmode_hal(pwm_no, val))
		goto err;

	spin_unlock_irqrestore(&dev->lock, flags);
	return RSUCCESS;

 err:
	spin_unlock_irqrestore(&dev->lock, flags);
	return -EPARMNOSUPPORT;
}

/***********************************************************
* Set PWM_HIDURATION register
*
*************************************************************/

static S32 mt_set_pwm_HiDur(U32 pwm_no, U16 DurVal)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_HiDur_hal(pwm_no, DurVal);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/************************************************
* Set PWM Low Duration register
*************************************************/
static S32 mt_set_pwm_LowDur(U32 pwm_no, U16 DurVal)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_LowDur_hal(pwm_no, DurVal);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/***************************************************
* Set PWM_GUARDDURATION register
* pwm_no: PWM1~PWM7(0~6)
* DurVal:   the value of guard duration
****************************************************/
static S32 mt_set_pwm_GuardDur(U32 pwm_no, U16 DurVal)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_GuardDur_hal(pwm_no, DurVal);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/*****************************************************
* Set pwm_buf0_addr register
* pwm_no: pwm1~pwm7 (0~6)
* addr: data address
*****************************************************
S32 mt_set_pwm_buf0_addr (U32 pwm_no, U32 addr )
{
	unsigned long flags;
	U32 reg_buff0_addr;

	struct pwm_device *dev = pwm_dev;
	if ( !dev ) {
		PWMDBG ( "dev is not valid\n" );
		return -EINVALID;
	}

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	reg_buff0_addr = PWM_register[pwm_no] + 4 * PWM_BUF0_BASE_ADDR;

	spin_lock_irqsave ( &dev->lock, flags );
	OUTREG32 ( reg_buff0_addr, addr );
	spin_unlock_irqrestore ( &dev->lock, flags );

	return RSUCCESS;
}*/

/*****************************************************
* Set pwm_buf0_size register
* pwm_no: pwm1~pwm7 (0~6)
* size: size of data
*****************************************************
S32 mt_set_pwm_buf0_size ( U32 pwm_no, U16 size)
{
	unsigned long flags;
	U32 reg_buff0_size;

	struct pwm_device *dev = pwm_dev;
	if ( !dev ) {
		PWMDBG ( "dev is not valid\n" );
		return -EINVALID;
	}

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	reg_buff0_size = PWM_register[pwm_no] + 4* PWM_BUF0_SIZE;

	spin_lock_irqsave ( &dev->lock, flags );
	OUTREG32 ( reg_buff0_size, size );
	spin_unlock_irqrestore ( &dev->lock, flags );

	return RSUCCESS;

}*/

/*****************************************************
* Set pwm_buf1_addr register
* pwm_no: pwm1~pwm7 (0~6)
* addr: data address
*****************************************************
S32 mt_set_pwm_buf1_addr (U32 pwm_no, U32 addr )
{
	unsigned long flags;
	U32 reg_buff1_addr;

	struct pwm_device *dev = pwm_dev;
	if ( !dev ) {
		PWMDBG ( "dev is not valid\n" );
		return -EINVALID;
	}

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	reg_buff1_addr = PWM_register[pwm_no] + 4 * PWM_BUF1_BASE_ADDR;

	spin_lock_irqsave ( &dev->lock, flags );
	OUTREG32 ( reg_buff1_addr, addr );
	spin_unlock_irqrestore ( &dev->lock, flags );

	return RSUCCESS;
}*/

/*****************************************************
* Set pwm_buf1_size register
* pwm_no: pwm1~pwm7 (0~6)
* size: size of data
*****************************************************
S32 mt_set_pwm_buf1_size ( U32 pwm_no, U16 size)
{
	unsigned long flags;
	U32 reg_buff1_size;

	struct pwm_device *dev = pwm_dev;
	if ( !dev ) {
		PWMDBG ( "dev is not valid\n" );
		return -EINVALID;
	}

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	reg_buff1_size = PWM_register[pwm_no] + 4* PWM_BUF1_SIZE;

	spin_lock_irqsave ( &dev->lock, flags );
	OUTREG32 ( reg_buff1_size, size );
	spin_unlock_irqrestore ( &dev->lock, flags );

	return RSUCCESS;

}*/

/*****************************************************
* Set pwm_send_data0 register
* pwm_no: pwm1~pwm7 (0~6)
* data: the data in the register
******************************************************/
static S32 mt_set_pwm_send_data0(U32 pwm_no, U32 data)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_send_data0_hal(pwm_no, data);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/*****************************************************
* Set pwm_send_data1 register
* pwm_no: pwm1~pwm7 (0~6)
* data: the data in the register
******************************************************/
static S32 mt_set_pwm_send_data1(U32 pwm_no, U32 data)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_send_data1_hal(pwm_no, data);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/*****************************************************
* Set pwm_wave_num register
* pwm_no: pwm1~pwm7 (0~6)
* num:the wave number
******************************************************/
static S32 mt_set_pwm_wave_num(U32 pwm_no, U16 num)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_wave_num_hal(pwm_no, num);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/*****************************************************
* Set pwm_data_width register.
* This is only for old mode
* pwm_no: pwm1~pwm7 (0~6)
* width: set the guard value in the old mode
******************************************************/
static S32 mt_set_pwm_data_width(U32 pwm_no, U16 width)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_data_width_hal(pwm_no, width);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/*****************************************************
* Set pwm_thresh register
* pwm_no: pwm1~pwm7 (0~6)
* thresh:  the thresh of the wave
******************************************************/
static S32 mt_set_pwm_thresh(U32 pwm_no, U16 thresh)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG(" pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_thresh_hal(pwm_no, thresh);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/*****************************************************
* Set pwm_send_wavenum register
* pwm_no: pwm1~pwm7 (0~6)
*
******************************************************/
S32 mt_get_pwm_send_wavenum(U32 pwm_no)
{
	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EBADADDR;
	}

	if (pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	return mt_get_pwm_send_wavenum_hal(pwm_no);
}

/*****************************************************
* Set pwm_send_data1 register
* pwm_no: pwm1~pwm7 (0~6)
* buf_valid_bit:
* for buf0: bit0 and bit1 should be set 1.
* for buf1: bit2 and bit3 should be set 1.
*****************************************************
S32 mt_set_pwm_valid ( U32 pwm_no, U32 buf_valid_bit )   //set 0  for BUF0 bit or set 1 for BUF1 bit
{
	unsigned long flags;
	U32 reg_valid;

	struct pwm_device *dev = pwm_dev;
	if ( !dev ) {
		PWMDBG ("dev is not valid\n");
		return -EINVALID;
	}

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	if ( !buf_valid_bit>= BUF_EN_MAX) {
		PWMDBG ( "inavlid bit\n" );
		return -EPARMNOSUPPORT;
	}

	if ( (pwm_no <= PWM2)||(pwm_no == PWM6))
		reg_valid = PWM_register[pwm_no] + 4 * PWM_VALID;
	else
		reg_valid = PWM_register[pwm_no] + 4* (PWM_VALID -2);

	spin_lock_irqsave ( &dev->lock, flags );
	SETREG32 ( reg_valid, 1 << buf_valid_bit );
	spin_unlock_irqrestore ( &dev->lock, flags );

	return RSUCCESS;
}*/

/*************************************************
*  set PWM4_delay when using SEQ mode
*
*************************************************
S32 mt_set_pwm_delay_duration(U32 pwm_delay_reg, U16 val)
{
	unsigned long flags;
	struct pwm_device *pwmdev = pwm_dev;

	if (!pwmdev) {
		PWMDBG( "device doesn't exist\n" );
		return -EBADADDR;
	}

	spin_lock_irqsave ( &pwmdev->lock, flags );
	MASKREG32 ( pwm_delay_reg, PWM_DELAY_DURATION_MASK, val );
	spin_unlock_irqrestore ( &pwmdev->lock, flags );

	return RSUCCESS;
}*/

/*******************************************************
* Set pwm delay clock
*
*
*******************************************************
S32 mt_set_pwm_delay_clock (U32 pwm_delay_reg, U32 clksrc)
{
	unsigned long flags;
	struct pwm_device *pwmdev = pwm_dev;
	if ( ! pwmdev ) {
		PWMDBG ( "device doesn't exist\n" );
		return -EBADADDR;
	}

	spin_lock_irqsave ( &pwmdev->lock, flags );
	MASKREG32 (pwm_delay_reg, PWM_DELAY_CLK_MASK, clksrc );
	spin_unlock_irqrestore (&pwmdev->lock, flags);

	return RSUCCESS;
}*/

/*******************************************
* Set intr enable register
* pwm_intr_enable_bit: the intr bit,
*
*********************************************/
S32 mt_set_intr_enable(U32 pwm_intr_enable_bit)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_intr_enable_bit >= PWM_INT_ENABLE_BITS_MAX) {
		PWMDBG(" pwm inter enable bit is not right.\n");
		return -EEXCESSBITS;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_intr_enable_hal(pwm_intr_enable_bit);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/*****************************************************
* Set intr status register
* pwm_no: pwm1~pwm7 (0~6)
* pwm_intr_status_bit
******************************************************/
S32 mt_get_intr_status(U32 pwm_intr_status_bit)
{
	unsigned long flags;
	int ret;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_intr_status_bit >= PWM_INT_STATUS_BITS_MAX) {
		PWMDBG("status bit excesses PWM_INT_STATUS_BITS_MAX\n");
		return -EEXCESSBITS;
	}

	spin_lock_irqsave(&dev->lock, flags);
	ret = mt_get_intr_status_hal(pwm_intr_status_bit);
	spin_unlock_irqrestore(&dev->lock, flags);

	return ret;
}

/*****************************************************
* Set intr ack register
* pwm_no: pwm1~pwm7 (0~6)
* pwm_intr_ack_bit
******************************************************/
S32 mt_set_intr_ack(U32 pwm_intr_ack_bit)
{
	unsigned long flags;
	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return -EINVALID;
	}

	if (pwm_intr_ack_bit >= PWM_INT_ACK_BITS_MAX) {
		PWMDBG("ack bit excesses PWM_INT_ACK_BITS_MAX\n");
		return -EEXCESSBITS;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_intr_ack_hal(pwm_intr_ack_bit);
	spin_unlock_irqrestore(&dev->lock, flags);

	return RSUCCESS;
}

/*----------3dLCM support-----------*/
/*
 base pwm2, select pwm3&4&5 same as pwm2 or inversion of pwm2
 */
void mt_set_pwm_3dlcm_enable(BOOL enable)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev) {
		PWMDBG("dev is not valid\n");
		return;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_3dlcm_enable_hal(enable);
	spin_unlock_irqrestore(&dev->lock, flags);

	return;
}

/*
 set "pwm_no" inversion of pwm2 or not,
 pwm_no: only support PWM3/4/5
 */
void mt_set_pwm_3dlcm_inv(U32 pwm_no, BOOL inv)
{
	unsigned long flags;

	struct pwm_device *dev = pwm_dev;
	if (!dev || pwm_no < PWM3 || pwm_no > PWM5) {
		PWMDBG("dev is not valid\n");
		return;
	}

	spin_lock_irqsave(&dev->lock, flags);
	mt_set_pwm_3dlcm_inv_hal(pwm_no, inv);
	spin_unlock_irqrestore(&dev->lock, flags);
	return;
}

S32 pwm_set_easy_config(struct pwm_easy_config *conf)
{

	U32 duty = 0;
	U16 duration = 0;
	U32 data_AllH = 0xffffffff;
	U32 data0 = 0;
	U32 data1 = 0;

	if (conf->pwm_no >= PWM_MAX || conf->pwm_no < PWM_MIN) {
		PWMDBG("pwm number excess PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	if ((conf->clk_div >= CLK_DIV_MAX) || (conf->clk_div < CLK_DIV_MIN)) {
		PWMDBG("PWM clock division invalid\n");
		return -EINVALID;
	}

	if ((conf->clk_src >= PWM_CLK_SRC_INVALID) || (conf->clk_src < PWM_CLK_SRC_MIN)) {
		PWMDBG("PWM clock source invalid\n");
		return -EINVALID;
	}

	if (conf->duty < 0) {
		PWMDBG("duty parameter is invalid\n");
		return -EINVALID;
	}

	PWMDBG("pwm_set_easy_config\n");

	if (conf->duty == 0) {
		mt_set_pwm_disable(conf->pwm_no);
		mt_pwm_power_off(conf->pwm_no, conf->pmic_pad);
		return RSUCCESS;
	}

	duty = conf->duty;
	duration = conf->duration;

	switch (conf->clk_src) {
	case PWM_CLK_OLD_MODE_BLOCK:
	case PWM_CLK_OLD_MODE_32K:
		if (duration > 8191 || duration < 0) {
			PWMDBG("duration invalid parameter\n");
			return -EPARMNOSUPPORT;
		}
		if (duration < 10)
			duration = 10;
		break;

	case PWM_CLK_NEW_MODE_BLOCK:
	case PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625:
		if (duration > 65535 || duration < 0) {
			PWMDBG("invalid paramters\n");
			return -EPARMNOSUPPORT;
		}
		break;
	default:
		PWMDBG("invalid clock source\n");
		return -EPARMNOSUPPORT;
	}

	if (duty > 100)
		duty = 100;

	if (duty > 50) {
		data0 = data_AllH;
		data1 = data_AllH >> ((PWM_NEW_MODE_DUTY_TOTAL_BITS * (100 - duty)) / 100);
	} else {
		data0 = data_AllH >> ((PWM_NEW_MODE_DUTY_TOTAL_BITS * (50 - duty)) / 100);
		PWMDBG("DATA0 :0x%x\n", data0);
		data1 = 0;
	}

	mt_pwm_power_on(conf->pwm_no, conf->pmic_pad);
	if (conf->pmic_pad)
		mt_pwm_sel_pmic(conf->pwm_no);
	else
		mt_pwm_sel_ap(conf->pwm_no);
	mt_set_pwm_con_guardval(conf->pwm_no, GUARD_TRUE);

	switch (conf->clk_src) {
	case PWM_CLK_OLD_MODE_32K:
		mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_ENABLE);
		mt_set_pwm_clk(conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div);
		break;

	case PWM_CLK_OLD_MODE_BLOCK:
		mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_ENABLE);
		mt_set_pwm_clk(conf->pwm_no, CLK_BLOCK, conf->clk_div);
		break;

	case PWM_CLK_NEW_MODE_BLOCK:
		mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
		mt_set_pwm_clk(conf->pwm_no, CLK_BLOCK, conf->clk_div);
		mt_set_pwm_con_datasrc(conf->pwm_no, PWM_FIFO);
		mt_set_pwm_con_stpbit(conf->pwm_no, 0x3f, PWM_FIFO);
		break;

	case PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625:
		mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
		mt_set_pwm_clk(conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div);
		mt_set_pwm_con_datasrc(conf->pwm_no, PWM_FIFO);
		mt_set_pwm_con_stpbit(conf->pwm_no, 0x3f, PWM_FIFO);
		break;

	default:
		break;
	}
	PWMDBG("The duration is:%x\n", duration);
	PWMDBG("The data0 is:%x\n", data0);
	PWMDBG("The data1 is:%x\n", data1);
	mt_set_pwm_HiDur(conf->pwm_no, duration);
	mt_set_pwm_LowDur(conf->pwm_no, duration);
	mt_set_pwm_GuardDur(conf->pwm_no, 0);
/* mt_set_pwm_buf0_addr (conf->pwm_no, 0 ); */
/* mt_set_pwm_buf0_size( conf->pwm_no, 0 ); */
/* mt_set_pwm_buf1_addr (conf->pwm_no, 0 ); */
/* mt_set_pwm_buf1_size (conf->pwm_no, 0 ); */
	mt_set_pwm_send_data0(conf->pwm_no, data0);
	mt_set_pwm_send_data1(conf->pwm_no, data1);
	mt_set_pwm_wave_num(conf->pwm_no, 0);

/* if ( conf->pwm_no <= PWM2 || conf->pwm_no == PWM6) */
/* { */
	mt_set_pwm_data_width(conf->pwm_no, duration);
	mt_set_pwm_thresh(conf->pwm_no, ((duration * conf->duty) / 100));
/* mt_set_pwm_valid (conf->pwm_no, BUF0_EN_VALID ); */
/* mt_set_pwm_valid ( conf->pwm_no, BUF1_EN_VALID ); */

/* } */

	mb();
	mt_set_pwm_enable(conf->pwm_no);
	PWMDBG("mt_set_pwm_enable\n");

	return RSUCCESS;

}
EXPORT_SYMBOL(pwm_set_easy_config);

S32 pwm_set_spec_config(struct pwm_spec_config *conf)
{

	if (conf->pwm_no >= PWM_MAX) {
		PWMDBG("pwm number excess PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	if ((conf->mode >= PWM_MODE_INVALID) || (conf->mode < PWM_MODE_MIN)) {
		PWMDBG("PWM mode invalid\n");
		return -EINVALID;
	}

	if ((conf->clk_src >= PWM_CLK_SRC_INVALID) || (conf->clk_src < PWM_CLK_SRC_MIN)) {
		PWMDBG("PWM clock source invalid\n");
		return -EINVALID;
	}

	if ((conf->clk_div >= CLK_DIV_MAX) || (conf->clk_div < CLK_DIV_MIN)) {
		PWMDBG("PWM clock division invalid\n");
		return -EINVALID;
	}

	if ((conf->mode == PWM_MODE_OLD &&
	     (conf->clk_src == PWM_CLK_NEW_MODE_BLOCK
	      || conf->clk_src == PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625))
	    || (conf->mode != PWM_MODE_OLD
		&& (conf->clk_src == PWM_CLK_OLD_MODE_32K
		    || conf->clk_src == PWM_CLK_OLD_MODE_BLOCK))) {

		PWMDBG("parameters match error\n");
		return -ERROR;
	}

	mt_pwm_power_on(conf->pwm_no, conf->pmic_pad);
	if (conf->pmic_pad)
		mt_pwm_sel_pmic(conf->pwm_no);

	switch (conf->mode) {
	case PWM_MODE_OLD:
		PWMDBG("PWM_MODE_OLD\n");
		mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_ENABLE);
		mt_set_pwm_con_idleval(conf->pwm_no, conf->PWM_MODE_OLD_REGS.IDLE_VALUE);
		mt_set_pwm_con_guardval(conf->pwm_no, conf->PWM_MODE_OLD_REGS.GUARD_VALUE);
		mt_set_pwm_GuardDur(conf->pwm_no, conf->PWM_MODE_OLD_REGS.GDURATION);
		mt_set_pwm_wave_num(conf->pwm_no, conf->PWM_MODE_OLD_REGS.WAVE_NUM);
		mt_set_pwm_data_width(conf->pwm_no, conf->PWM_MODE_OLD_REGS.DATA_WIDTH);
		mt_set_pwm_thresh(conf->pwm_no, conf->PWM_MODE_OLD_REGS.THRESH);
		PWMDBG("PWM set old mode finish\n");
		break;
	case PWM_MODE_FIFO:
		PWMDBG("PWM_MODE_FIFO\n");
		mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
		mt_set_pwm_con_datasrc(conf->pwm_no, PWM_FIFO);
		mt_set_pwm_con_mode(conf->pwm_no, PERIOD);
		mt_set_pwm_con_idleval(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.IDLE_VALUE);
		mt_set_pwm_con_guardval(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.GUARD_VALUE);
		mt_set_pwm_HiDur(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.HDURATION);
		mt_set_pwm_LowDur(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.LDURATION);
		mt_set_pwm_GuardDur(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.GDURATION);
		mt_set_pwm_send_data0(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.SEND_DATA0);
		mt_set_pwm_send_data1(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.SEND_DATA1);
		mt_set_pwm_wave_num(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.WAVE_NUM);
		mt_set_pwm_con_stpbit(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE, PWM_FIFO);
		break;
/*	case PWM_MODE_MEMORY:
		PWMDBG("PWM_MODE_MEMORY\n");
		mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
		mt_set_pwm_con_datasrc(conf->pwm_no, MEMORY);
		mt_set_pwm_con_mode (conf->pwm_no, PERIOD);
		mt_set_pwm_con_idleval(conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.IDLE_VALUE);
		mt_set_pwm_con_guardval (conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.GUARD_VALUE);
		mt_set_pwm_HiDur (conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.HDURATION);
		mt_set_pwm_LowDur (conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.LDURATION);
		mt_set_pwm_GuardDur (conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.GDURATION);
		mt_set_pwm_buf0_addr(conf->pwm_no, (U32)conf->PWM_MODE_MEMORY_REGS.BUF0_BASE_ADDR);
		mt_set_pwm_buf0_size (conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.BUF0_SIZE);
		mt_set_pwm_wave_num(conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.WAVE_NUM);
		mt_set_pwm_con_stpbit(conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.STOP_BITPOS_VALUE,MEMORY);

		break;
	case PWM_MODE_RANDOM:
		PWMDBG("PWM_MODE_RANDOM\n");
		mt_set_pwm_disable(conf->pwm_no);
		mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
		mt_set_pwm_con_datasrc(conf->pwm_no, MEMORY);
		mt_set_pwm_con_mode (conf->pwm_no, RAND);
		mt_set_pwm_con_idleval(conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.IDLE_VALUE);
		mt_set_pwm_con_guardval (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.GUARD_VALUE);
		mt_set_pwm_HiDur (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.HDURATION);
		mt_set_pwm_LowDur (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.LDURATION);
		mt_set_pwm_GuardDur (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.GDURATION);
		mt_set_pwm_buf0_addr(conf->pwm_no, (U32 )conf->PWM_MODE_RANDOM_REGS.BUF0_BASE_ADDR);
		mt_set_pwm_buf0_size (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.BUF0_SIZE);
		mt_set_pwm_buf1_addr(conf->pwm_no, (U32 )conf->PWM_MODE_RANDOM_REGS.BUF1_BASE_ADDR);
		mt_set_pwm_buf1_size (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.BUF1_SIZE);
		mt_set_pwm_wave_num(conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.WAVE_NUM);
		mt_set_pwm_con_stpbit(conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.STOP_BITPOS_VALUE, MEMORY);
		mt_set_pwm_valid(conf->pwm_no, BUF0_EN_VALID);
		mt_set_pwm_valid(conf->pwm_no, BUF1_EN_VALID);
		break;

	case PWM_MODE_DELAY:
		PWMDBG("PWM_MODE_DELAY\n");
		mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
		mt_set_pwm_enable_seqmode();
		mt_set_pwm_disable(PWM2);
		mt_set_pwm_disable(PWM3);
		mt_set_pwm_disable(PWM4);
		mt_set_pwm_disable(PWM5);
		if ( conf->PWM_MODE_DELAY_REGS.PWM3_DELAY_DUR <0 ||
		conf->PWM_MODE_DELAY_REGS.PWM3_DELAY_DUR >= (1<<17)
		|| conf->PWM_MODE_DELAY_REGS.PWM4_DELAY_DUR < 0||
		conf->PWM_MODE_DELAY_REGS.PWM4_DELAY_DUR >= (1<<17)
		|| conf->PWM_MODE_DELAY_REGS.PWM5_DELAY_DUR <0 ||
		conf->PWM_MODE_DELAY_REGS.PWM5_DELAY_DUR >=(1<<17) )
		{
			PWMDBG("Delay value invalid\n");
			return -EINVALID;
		}
		mt_set_pwm_delay_duration(PWM3_DELAY, conf->PWM_MODE_DELAY_REGS.PWM3_DELAY_DUR );
		mt_set_pwm_delay_clock(PWM3_DELAY, conf->PWM_MODE_DELAY_REGS.PWM3_DELAY_CLK);
		mt_set_pwm_delay_duration(PWM4_DELAY, conf->PWM_MODE_DELAY_REGS.PWM4_DELAY_DUR);
		mt_set_pwm_delay_clock(PWM4_DELAY, conf->PWM_MODE_DELAY_REGS.PWM4_DELAY_CLK);
		mt_set_pwm_delay_duration(PWM5_DELAY, conf->PWM_MODE_DELAY_REGS.PWM5_DELAY_DUR);
		mt_set_pwm_delay_clock(PWM5_DELAY, conf->PWM_MODE_DELAY_REGS.PWM5_DELAY_CLK);

		mt_set_pwm_enable(PWM2);
		mt_set_pwm_enable(PWM3);
		mt_set_pwm_enable(PWM4);
		mt_set_pwm_enable(PWM5);
		break;
*/
	default:
		break;
	}

	switch (conf->clk_src) {
	case PWM_CLK_OLD_MODE_BLOCK:
		mt_set_pwm_clk(conf->pwm_no, CLK_BLOCK, conf->clk_div);
		PWMDBG("Enable oldmode and set clock block\n");
		break;
	case PWM_CLK_OLD_MODE_32K:
		mt_set_pwm_clk(conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div);
		PWMDBG("Enable oldmode and set clock 32K\n");
		break;
	case PWM_CLK_NEW_MODE_BLOCK:
		mt_set_pwm_clk(conf->pwm_no, CLK_BLOCK, conf->clk_div);
		PWMDBG("Enable newmode and set clock block\n");
		break;
	case PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625:
		mt_set_pwm_clk(conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div);
		PWMDBG("Enable newmode and set clock 32K\n");
		break;
	default:
		break;
	}

	mb();
	mt_set_pwm_enable(conf->pwm_no);
	PWMDBG("mt_set_pwm_enable\n");

	return RSUCCESS;

}
EXPORT_SYMBOL(pwm_set_spec_config);

void mt_pwm_dump_regs(void)
{
	mt_pwm_dump_regs_hal();
	pr_info("<0>" "pwm power_flag: 0x%x\n", (unsigned int)pwm_dev->power_flag);
}
EXPORT_SYMBOL(mt_pwm_dump_regs);

struct platform_device pwm_plat_dev = {
	.name = "mt-pwm",
};

/****/
static ssize_t pwm_debug_store(struct device *dev, struct device_attribute *attr, const char *buf,
			       size_t count)
{
	pr_info("<0>" "pwm power_flag: 0x%x\n", (unsigned int)pwm_dev->power_flag);
	pwm_debug_store_hal();
	return count;
}

static ssize_t pwm_debug_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	pwm_debug_show_hal();
	return sprintf(buf, "%s\n", buf);
}

static DEVICE_ATTR(pwm_debug, 0644, pwm_debug_show, pwm_debug_store);

static int mt_pwm_probe(struct platform_device *pdev)
{
	int ret;

	platform_set_drvdata(pdev, pwm_dev);

	ret = device_create_file(&pdev->dev, &dev_attr_pwm_debug);
	if (ret)
		PWMDBG("error creating sysfs files: pwm_debug\n");

/* request_irq(69, mt_pwm_irq, IRQF_TRIGGER_LOW, "mt6589_pwm", NULL); */

	return RSUCCESS;
}

static int mt_pwm_remove(struct platform_device *pdev)
{
	if (!pdev) {
		PWMDBG("The plaform device is not exist\n");
		return -EBADADDR;
	}
	device_remove_file(&pdev->dev, &dev_attr_pwm_debug);

	PWMDBG("mt_pwm_remove\n");
	return RSUCCESS;
}

static void mt_pwm_shutdown(struct platform_device *pdev)
{
	pr_info("mt_pwm_shutdown\n");
	return;
}

struct platform_driver pwm_plat_driver = {
	.probe = mt_pwm_probe,
	.remove = mt_pwm_remove,
	.shutdown = mt_pwm_shutdown,
	.driver = {
		   .name = "mt-pwm"},
};

static int __init mt_pwm_init(void)
{
	int ret;
	ret = platform_device_register(&pwm_plat_dev);
	if (ret < 0) {
		PWMDBG("platform_device_register error\n");
		goto out;
	}
	ret = platform_driver_register(&pwm_plat_driver);
	if (ret < 0) {
		PWMDBG("platform_driver_register error\n");
		goto out;
	}

 out:
	mt_pwm_init_power_flag(&(pwm_dev->power_flag));
	return ret;
}

static void __exit mt_pwm_exit(void)
{
	platform_device_unregister(&pwm_plat_dev);
	platform_driver_unregister(&pwm_plat_driver);
}


#define MT_PWM_GPIO_MODE	(GPIO_MODE_01)
#define	MT_PWM_PIN_NUM		(5)

static unsigned long mt_pwm_ch_to_gpio[PWM_NUM]=
	{GPIO203 | 0x80000000,		/* Mode1 PWM0 TP56 */
	 GPIO204 | 0x80000000,		/* Mode1 PWM1 TP58 */
	 GPIO205 | 0x80000000,		/* Mode1 PWM2 TP60 */
	 GPIO206 | 0x80000000,		/* Mode1 PWM3 TP62 */
	 GPIO207 | 0x80000000,		/* Mode1 PWM4 TP64 */
	};

static int mt_pwm_gpio_get(char *s, struct kernel_param *kp)
{	return snprintf(s, PAGE_SIZE, "0x%x", (unsigned int)*(const uint*)(kp->arg));
}

static int mt_pwm_gpio_set(const char *s, struct kernel_param *kp)
{	int	result = 0;
	unsigned long	val = 0;

	if (!s) {
		pr_err("%s: Pointer to parameter is NULL.\n", __func__);
		return -EINVAL;
	}
	result = strict_strtoul(s, 0, &val);
	if (result == 0) {
		/* Numeric format. */
		uint		i;
		unsigned long	port;
		uint		m;

		*((uint*)(kp->arg)) = (uint)(val);
		i = 0;
		while (i < MT_PWM_PIN_NUM) {
			m=1 << i;
			port = mt_pwm_ch_to_gpio[i];
			if ((val & m) != 0) {
				printk("%s: PWM enable. i=%u, port=0x%lx\n", __func__, i, port);
				mt_set_gpio_mode(port, MT_PWM_GPIO_MODE);
				mt_set_gpio_dir(port, GPIO_DIR_OUT);
				mt_set_gpio_pull_enable(port, GPIO_PULL_DISABLE);
				mt_set_gpio_pull_select(port, GPIO_PULL_DOWN);
			} else {
				printk("%s: PWM disable. i=%u, port=0x%lx\n", __func__, i, port);
				mt_set_gpio_mode(port, GPIO_MODE_GPIO);
				mt_set_gpio_dir(port, GPIO_DIR_IN);
				mt_set_gpio_pull_enable(port, GPIO_PULL_ENABLE);
				mt_set_gpio_pull_select(port, GPIO_PULL_DOWN);
			}
			i++;
		}
		return 0;
	}
	return -EINVAL;
}


static uint mt_pwm_gpio = 0x00;

module_param_call(gpio, mt_pwm_gpio_set, mt_pwm_gpio_get, &mt_pwm_gpio, 0644);

static int mt_pwm_duration_get(char *s, struct kernel_param *kp)
{	size_t	len;
	size_t	len_ch;
	int	i;

	len = 0;
	i = 0;
	while (i < MT_PWM_PIN_NUM) {
		len_ch = snprintf(s + len, PAGE_SIZE, "%d:0x%.4x:0x%.4x",
			i,
			mt_get_pwm_LowDur_hal(i),
			mt_get_pwm_HiDur_hal(i)
		);
		len += len_ch;
		i++;
		if (i < MT_PWM_PIN_NUM) {
			*(s + len + 0) = ',';
			*(s + len + 1) = ' ';
			len+=2;
		}
	}
	return len;
}

static const char *str_skip_space(const char *s)
{	while (*s) {
		if (*s > ' ') {
			break;
		}
		if (*s == '\n') {
			break;
		}
		s++;
	}
	return s;
}

static const char *str_skip_coln(const char *s)
{	s = str_skip_space(s);
	if (*s == ':') {
		s++;
	}
	s = str_skip_space(s);
	return s;
}

static const char *str_skip_comma(const char *s)
{	s = str_skip_space(s);
	if (*s == ',') {
		s++;
	}
	s = str_skip_space(s);
	return s;
}

static int mt_pwm_duration_set(const char *s, struct kernel_param *kp)
{	char	*s2;
	unsigned long	pwm_no;
	unsigned long	lo;
	unsigned long	hi;

	if (!s) {
		return -EINVAL;
	}
	while (*s) {
		if (*s == '\n') {
			break;
		}
		s = str_skip_space(s);
		pwm_no = simple_strtoul(s, &s2, 0);
		if (s == s2) {
			printk("%s: No PWM number.\n", __func__);
			return -EINVAL;
		}
		s = str_skip_coln(s2);
		lo = simple_strtoul(s, &s2, 0);
		if (s == s2) {
			printk("%s: No Low duration.\n", __func__);
			return -EINVAL;
		}
		s = str_skip_coln(s2);
		hi = simple_strtoul(s, &s2, 0);
		if (s == s2) {
			printk("%s: No High duration.\n", __func__);
			return -EINVAL;
		}
		printk("%s: PWM duration. pwm_no=%lu, lo=0x%lx, hi=0x%lx\n", __func__, pwm_no, lo, hi);
		mt_set_pwm_wave_num_hal(pwm_no, 0);
		mt_set_pwm_LowDur_hal(pwm_no, lo);
		mt_set_pwm_HiDur_hal(pwm_no, hi);
		mt_set_pwm_GuardDur_hal(pwm_no, 0);
		mt_set_pwm_con_oldmode_hal(pwm_no, OLDMODE_ENABLE);
		mt_set_pwm_clk_hal(pwm_no, PWM_CLK_OLD_MODE_32K, 0);
		s = str_skip_comma(s2);
	}
	return 0;
}

module_param_call(duration, mt_pwm_duration_set, mt_pwm_duration_get, NULL, 0644);


static int mt_pwm_enable_get(char *s, struct kernel_param *kp)
{	return snprintf(s, PAGE_SIZE, "0x%x", (unsigned int)mt_get_pwm_all_enable_hal());
}

static int mt_pwm_enable_set(const char *s, struct kernel_param *kp)
{	int	result = 0;
	unsigned long	val = 0;

	if (!s) {
		pr_err("%s: Pointer to parameter is NULL.\n", __func__);
		return -EINVAL;
	}
	result = strict_strtoul(s, 0, &val);
	if (result == 0) {
		/* Numeric format. */
		*((uint*)(kp->arg)) = (uint)(val);
		mt_set_pwm_all_enable_hal(val);
		return 0;
	}
	return -EINVAL;
}

static uint mt_pwm_enable = 0x0;

module_param_call(enable, mt_pwm_enable_set, mt_pwm_enable_get, &mt_pwm_enable, 0644);


static int mt_pwm_clock_get(char *s, struct kernel_param *kp)
{	return snprintf(s, PAGE_SIZE, "0x%x", (unsigned int)*(const uint*)(kp->arg));
}

static int mt_pwm_clock_set(const char *s, struct kernel_param *kp)
{	int	result = 0;
	unsigned long	val = 0;

	if (!s) {
		pr_err("%s: Pointer to parameter is NULL.\n", __func__);
		return -EINVAL;
	}
	result = strict_strtoul(s, 0, &val);
	if (result == 0) {
		/* Numeric format. */
		uint		i;
		uint		m;

		*((uint*)(kp->arg)) = (uint)(val);
		i = 0;
		while (i < MT_PWM_PIN_NUM) {
			m=1 << i;
			if ((val & m) != 0) {
				printk("%s: PWM clock on. i=%u\n", __func__, i);
				mt_pwm_power_on(i, false);
				mt_set_pwm_enable(i);
			} else {
				printk("%s: PWM clock off. i=%u\n", __func__, i);
				mt_set_pwm_disable(i);
				mt_pwm_power_off(i, false);
			}
			i++;
		}
		return 0;
	}
	return -EINVAL;
}


static uint mt_pwm_clock = 0x00;

module_param_call(clock, mt_pwm_clock_set, mt_pwm_clock_get, &mt_pwm_clock, 0644);

static int mt_pwm_easy_get(char *s, struct kernel_param *kp)
{	struct pwm_easy_config	*conf;
	size_t	len;
	size_t	len_ch;
	int	i;

	conf = (struct pwm_easy_config*)(kp->arg);

	len = 0;
	i = 0;
	while (i < MT_PWM_PIN_NUM) {
		len_ch = snprintf(s + len, PAGE_SIZE, "%d:%d:%d",
			i,
			conf->duty,
			conf->duration
		);
		len += len_ch;
		conf++;
		i++;
		if (i < MT_PWM_PIN_NUM) {
			*(s + len + 0) = ',';
			*(s + len + 1) = ' ';
			len+=2;
		}
	}
	return len;
}


static int mt_pwm_easy_set(const char *s, struct kernel_param *kp)
{	char	*s2;

	struct pwm_easy_config	*conf;
	unsigned long	pwm_no;
	unsigned long	duty;
	unsigned long	duration;
	int		r;

	if (!s) {
		return -EINVAL;
	}
	while (*s) {
		if (*s == '\n') {
			break;
		}
		s = str_skip_space(s);
		pwm_no = simple_strtoul(s, &s2, 0);
		if (s == s2) {
			printk("%s: No PWM number.\n", __func__);
			return -EINVAL;
		}
		if (pwm_no >= MT_PWM_PIN_NUM) {
			printk("%s: Out of range PWM number, should be [0...%d]. pwm_no=%lu\n",
			__func__, MT_PWM_PIN_NUM, pwm_no
			);
			return -EINVAL;
		}
		s = str_skip_coln(s2);
		duty = simple_strtoul(s, &s2, 0);
		if (s == s2) {
			printk("%s: No duty value.\n", __func__);
			return -EINVAL;
		}
		if (duty > 100) {
			printk("%s: Out of range duty, should be [0..100]. duty=%lu\n",
				__func__, duty
			);
			return -EINVAL;
		}
		s = str_skip_coln(s2);
		duration = simple_strtoul(s, &s2, 0);
		if (s == s2) {
			printk("%s: No duration value.\n", __func__);
			return -EINVAL;
		}
		if (duration > 8191) {
			printk("%s: Out of range duration, should be [0..8191]. duration=%lu\n",
				__func__, duration
			);
			return -EINVAL;
		}
		conf = ((struct pwm_easy_config*)(kp->arg)) + pwm_no;
		conf->pwm_no = pwm_no;
		conf->duty = duty;
		conf->clk_src = PWM_CLK_OLD_MODE_32K;
		conf->clk_div = CLK_DIV1;
		conf->duration = duration;
		conf->pmic_pad = false;
		r = pwm_set_easy_config(conf);
		printk("%s: Set PWM easy mode. pwm_no=%lu, duty=%lu, duration=%lu, r=%d\n", __func__, pwm_no, duty, duration, r);
		s = str_skip_comma(s2);
	}
	return 0;
}

static struct pwm_easy_config	easy_conf[MT_PWM_PIN_NUM];

module_param_call(easy, mt_pwm_easy_set, mt_pwm_easy_get, easy_conf, 0644);


module_init(mt_pwm_init);
module_exit(mt_pwm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("How wang <How.wang@mediatek.com>");
MODULE_DESCRIPTION(" This module is for mtk chip of mediatek");
