/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd90020.c
 *
 * CXD90020 DAmp driver
 *
 * Copyright (c) 2013,2014 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/***************/
/*@ parameters */
/***************/

/* debug switch */
/* #define TRACE_PRINT_ON */
/* #define DEBUG_PRINT_ON */
#define TRACE_TAG "------- "
#define DEBUG_TAG "        "

/* volume wait time per step [ms] */
#define WAIT_TIME_PER_STEP 30

/************/
/*@ headers */
/************/

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/clk.h>
#include <linux/i2c.h>

#include <asm/mach-types.h>
#if 0
#include <mach/io.h>
#include <mach/gpio.h>
#endif
#ifdef CONFIG_REGMON_DEBUG
#include <mach/regmon.h>
#endif

#include <sound/digiamp.h>
#include <sound/cxd90020.h>

#include <mach/mt_gpio.h>
#include <mach/irqs.h>
#include <mach/eint.h>

/****************/
/*@ definitions */
/****************/

/* trace print macro */
#ifdef TRACE_PRINT_ON
	#define print_trace(fmt, args...) printk(KERN_INFO TRACE_TAG "" fmt, ##args)
#else
	#define print_trace(fmt, args...)
#endif

/* debug print macro */
#ifdef DEBUG_PRINT_ON
	#define print_debug(fmt, args...) printk(KERN_INFO DEBUG_TAG "" fmt, ##args)
#else
	#define print_debug(fmt, args...)
#endif

#define print_info(fmt, args...)    printk(KERN_INFO    "CXD90020: " fmt,                 ##args)
#define print_warning(fmt, args...) printk(KERN_WARNING "%s(): "     fmt,   __FUNCTION__, ##args)
#define print_error(fmt, args...)   printk(KERN_ERR     "%s(): "     fmt,   __FUNCTION__, ##args)
#define print_fail(fmt, args...)    printk(KERN_ERR                  fmt,                 ##args)
#define back_trace()                printk(KERN_ERR     "%s(): [%d]\n", __FUNCTION__, __LINE__)

#define CHK_RV(_msg) \
	if(rv<0){ \
		print_fail(_msg,rv); \
		return(rv); \
	}

/* basic macro */
#define minimum(_a,_b) ( (_a)<(_b) ? (_a) : (_b) )
#define maximum(_a,_b) ( (_a)>(_b) ? (_a) : (_b) )
#define absolute(_a)   ( (_a)>=0 ? (_a) : (-(_a)) )

/* basic */
#define FALSE 0
#define TRUE  1
#define OFF   0
#define ON    1

#define CXD90020_DEVICE_ID_AB    0x01
#define CXD90020_DEVICE_ID_CD    0x02
#define CXD90020_DEVICE_ID_E     0x03
#define CXD90020_REV_ID          0x05
#define CXD90020_DEVICE_CTRL     0x06
#define CXD90020_CLOCK_CTRL      0x07
#define CXD90020_PROTECTION_CTRL 0x08
#define CXD90020_HP_VOL_A        0x09
#define CXD90020_HP_VOL_B        0x0A
#define CXD90020_SOFT_RAMP_CTRL  0x0B
#define CXD90020_INT_MASK        0x0C
#define CXD90020_INT_STAT        0x0D
#define CXD90020_CLASS_H_CTRL    0x0E
#define CXD90020_CHRG_PUMP_FREQ  0x0F

#define GPIO_TBSAMP_XINT   (CXD90020->platform_data.port_tbsamp_xint)
#define GPIO_TBSAMP_XRST   (CXD90020->platform_data.port_tbsamp_xrst)
#define GPIO_HP_XMUTE2     (CXD90020->platform_data.port_hp_xmute2)
#define GPIO_HP_MUTE4      (CXD90020->platform_data.port_hp_mute4)
#define GPIO_VG_XON        (CXD90020->platform_data.port_vg_xon)
#define GPIO_VLDO_XSHUNT   (CXD90020->platform_data.port_vldo_xshunt)
#define GPIO_TBSAMP_PWR_ON (CXD90020->platform_data.port_tbsamp_pwr_on)

#define IRQ_TBSAMP_XINT gpio_to_irq(GPIO_TBSAMP_XINT)

struct cxd90020_data {
	struct i2c_client *            i2c_client;
	struct cxd90020_platform_data  platform_data;
	struct mutex                   global_mutex;
	struct mutex                   mutex;
	spinlock_t                     lock;
	struct delayed_work            clear_protect_work;
	int                            clear_protect_count;
};

/***************/
/*@ prototypes */
/***************/

static int  __init cxd90020_init(void);
static void __exit cxd90020_exit(void);

static int cxd90020_probe(
	      struct i2c_client    * client,
	const struct i2c_device_id * identify
);
static int cxd90020_remove(
	struct i2c_client * client
);
static void cxd90020_poweroff(
	struct i2c_client * client
);

static int cxd90020_power_on(void);
static int cxd90020_power_off(void);
static int cxd90020_initialize(void);
static int cxd90020_shutdown(void);
static int cxd90020_enable(int level);
static int cxd90020_disable(int level);
static int cxd90020_fade_volume(unsigned int volume);
static int cxd90020_set_volume(unsigned int volume);
static int cxd90020_switch_sys_clock(int value);
static int cxd90020_switch_shunt_mute(int value);

static irqreturn_t cxd90020_clear_protect(int irq, void * data);
static void cxd90020_do_clear_protect_work(struct work_struct * work);
static int cxd90020_setup_platform(void);
static int cxd90020_reset_platform(void);
static int cxd90020_reset(void);
static int cxd90020_unreset(void);
static int cxd90020_switch_power(int value);
static int cxd90020_switch_vg(int value);
static int cxd90020_switch_hp_xmute2(int value);
static int cxd90020_switch_hp_mute4(int value);
static int cxd90020_switch_vldo_shunt(int value);
static int cxd90020_get_tbsamp_xint_value(void);

static int cxd90020_modify(
	unsigned int address,
	unsigned int value,
	unsigned int mask
);

static int cxd90020_write(
	unsigned int address,
	unsigned int value
);

static int cxd90020_read(
	unsigned int   address,
	unsigned int * value
);

static int cxd90020_write_core(
	unsigned int address,
	unsigned int value
);

static int cxd90020_read_core(
	unsigned int   address,
	unsigned int * value
);

#ifdef CONFIG_REGMON_DEBUG
static int cxd90020_regmon_add(void);
static int cxd90020_regmon_del(void);
#endif

/**************/
/*@ variables */
/**************/

MODULE_AUTHOR("Sony Corporation");
MODULE_DESCRIPTION("CXD90020 ALSA SoC DAmp Driver");
MODULE_LICENSE("GPL");

module_init(cxd90020_init);
module_exit(cxd90020_exit);

static const struct i2c_device_id cxd90020_id[] = {
	{CXD90020_DEVICE_NAME, 0},
	{}
};

static struct i2c_driver cxd90020_i2c_driver = {
	.driver   = {
		.name  = "CXD90020",
		.owner = THIS_MODULE,
	},
	.id_table = cxd90020_id,
	.probe    = cxd90020_probe,
	.remove   = cxd90020_remove,
	.shutdown = cxd90020_poweroff,
};

static struct digital_amp_interface cxd90020_interface = {
	.type              = DAMP_TYPE_CXD90020,
	.power_on          = cxd90020_power_on,
	.power_off         = cxd90020_power_off,
	.initialize        = cxd90020_initialize,
	.shutdown          = cxd90020_shutdown,
	.enable            = cxd90020_enable,
	.disable           = cxd90020_disable,
	.fade_volume       = cxd90020_fade_volume,
	.set_volume        = cxd90020_set_volume,
	.switch_shunt_mute = cxd90020_switch_shunt_mute,
	.switch_sys_clock  = cxd90020_switch_sys_clock,
};

static struct cxd90020_data * CXD90020=NULL;

static int initialized=FALSE;
static int enabled=FALSE;

static unsigned int now_volume=0x39;

extern int icx_board_id5;

/********************/
/*@ entry_functions */
/********************/

static int __init cxd90020_init(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	rv=i2c_add_driver(&cxd90020_i2c_driver);
	if(rv!=0) {
		print_fail("i2c_add_driver(): code %d error occurred.\n",rv);
		back_trace();
		return(rv);
	}

	return(0);
}

static void __exit cxd90020_exit(void)
{
	print_trace("%s()\n",__FUNCTION__);

	i2c_del_driver(&cxd90020_i2c_driver);

	return;
}

static int cxd90020_probe(
	      struct i2c_client    * client,
	const struct i2c_device_id * identify
)
{
	struct cxd90020_platform_data * platdata;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	print_info("starting driver...\n");

	if(CXD90020!=NULL){
		print_error("device busy.\n");
		return(-EBUSY);
	}

	CXD90020=kzalloc(sizeof(struct cxd90020_data),GFP_KERNEL);
	if(CXD90020==NULL){
		print_fail("kzalloc(): no memory.\n");
		back_trace();
		return(-ENOMEM);
	}

	i2c_set_clientdata(client,CXD90020);
	CXD90020->i2c_client=client;

	platdata=dev_get_platdata(&client->dev);
	if(platdata==NULL){
		print_error("no platform data.");
		kfree(CXD90020);
		return(-ENXIO);
	}

	memcpy(
		&CXD90020->platform_data,
		platdata,
		sizeof(struct cxd90020_platform_data)
	);

	mutex_init(&CXD90020->global_mutex);
	mutex_init(&CXD90020->mutex);
	spin_lock_init(&CXD90020->lock);

	rv=cxd90020_setup_platform();
	if(rv<0){
		back_trace();
		return(rv);
	}

	INIT_DELAYED_WORK(
		&CXD90020->clear_protect_work,
		cxd90020_do_clear_protect_work
	);
#if 0
	rv=request_irq(
		IRQ_TBSAMP_XINT,
		cxd90020_clear_protect,
		IRQF_TRIGGER_FALLING,
		"cxd90020_xint",
		NULL
	);
	if(rv<0){
		print_fail("request_irq(): code %d error occurred.\n",rv);
		back_trace();
		cxd90020_reset_platform();
		kfree(CXD90020);
		return(rv);
	}
#endif
	mt_eint_registration(93, EINTF_TRIGGER_FALLING, cxd90020_clear_protect, 1);

	rv=digiamp_register(&cxd90020_interface);
	if(rv<0){
		print_fail("digiamp_register(): code %d error occurred.\n",rv);
		back_trace();
		cancel_delayed_work_sync(&CXD90020->clear_protect_work);
		flush_delayed_work(&CXD90020->clear_protect_work);

//		free_irq(IRQ_TBSAMP_XINT,NULL);

		cxd90020_reset_platform();
		kfree(CXD90020);
		return(rv);
	}

	return(0);
}

static int cxd90020_remove(
	struct i2c_client * client
)
{
	print_trace("%s()\n",__FUNCTION__);

	cancel_delayed_work_sync(&CXD90020->clear_protect_work);
	flush_delayed_work(&CXD90020->clear_protect_work);

	digiamp_register(NULL);

//	free_irq(IRQ_TBSAMP_XINT,NULL);

	cxd90020_reset_platform();

	kfree(CXD90020);

	return(0);
}

static void cxd90020_poweroff(
	struct i2c_client * client
)
{
	print_trace("%s()\n",__FUNCTION__);

	cancel_delayed_work_sync(&CXD90020->clear_protect_work);
	flush_delayed_work(&CXD90020->clear_protect_work);

	digiamp_register(NULL);

//	free_irq(IRQ_TBSAMP_XINT,NULL);

	cxd90020_reset_platform();

	kfree(CXD90020);

	return;
}

/**********************/
/*@ control_fucntions */
/**********************/

static int cxd90020_power_on(void)
{
	mutex_lock(&CXD90020->global_mutex);

	print_trace("%s()\n",__FUNCTION__);

	mutex_unlock(&CXD90020->global_mutex);

	return(0);
}

static int cxd90020_power_off(void)
{
	mutex_lock(&CXD90020->global_mutex);

	print_trace("%s()\n",__FUNCTION__);

	mutex_unlock(&CXD90020->global_mutex);

	return(0);
}

static int cxd90020_initialize(void)
{
	print_trace("%s()\n",__FUNCTION__);

	mutex_lock(&CXD90020->global_mutex);

#ifdef ICX_ENABLE_AU2CLK

	/* VG=off */
	cxd90020_switch_vg(OFF);

#else

	cxd90020_unreset();

	/* all off (make sure) */
	cxd90020_modify(CXD90020_DEVICE_CTRL,0xE0,0xE0);

	/* mclk=off, osc=off, 1024fs, sysclk=off, reclk_trig=rising, reclk=on */
	cxd90020_modify(CXD90020_CLOCK_CTRL,0xF0,0xF3);

	msleep(50);

	cxd90020_switch_vldo_shunt(ON);
	cxd90020_switch_vg(OFF);

#endif

#ifdef CONFIG_REGMON_DEBUG

	cxd90020_regmon_add();

#endif

	initialized=TRUE;

	mutex_unlock(&CXD90020->global_mutex);

	return(0);
}

static int cxd90020_shutdown(void)
{
	cxd90020_disable(DAMP_CTL_LEVEL_POWER);

	mutex_lock(&CXD90020->global_mutex);

	print_trace("%s()\n",__FUNCTION__);

	initialized=FALSE;

#ifdef CONFIG_REGMON_DEBUG

	cxd90020_regmon_del();

#endif

/* V */
#ifndef ICX_ENABLE_AU2CLK

	cxd90020_reset();

#endif

	mutex_unlock(&CXD90020->global_mutex);

	return(0);
}

static int cxd90020_enable(int level)
{
	mutex_lock(&CXD90020->global_mutex);

	print_trace("%s(%d)\n",__FUNCTION__,level);

	if(enabled){
		print_debug("already enabled.\n");
		mutex_unlock(&CXD90020->global_mutex);
		return(0);
	}

#ifdef ICX_ENABLE_AU2CLK

	if(level==DAMP_CTL_LEVEL_POWER){

		/* VG=on */
		cxd90020_switch_vg(ON);

		/* VD/VCP=on */
		cxd90020_switch_power(ON);
	}

	if(level==DAMP_CTL_LEVEL_POWER || level==DAMP_CTL_LEVEL_RESET){

		/* unresset */
		cxd90020_unreset();
	}

#else

	/* VG=on */
	cxd90020_switch_vg(ON);

#endif

	/* all off (make sure) */
	cxd90020_modify(CXD90020_DEVICE_CTRL,0xE0,0xE0);

#ifdef ICX_ENABLE_AU2CLK

	/* mclk=off, osc=off, 1024fs, sysclk=off, reclk_trig=rising, reclk=on */
	cxd90020_modify(CXD90020_CLOCK_CTRL,0xF0,0xF3);

#endif

	/* mclk=on */
	cxd90020_modify(CXD90020_CLOCK_CTRL,0x00,0x80);

	/* disable short detect */
	cxd90020_modify(CXD90020_PROTECTION_CTRL,0x20,0xF0);

	/* ADPTPWR=fixed VCP/1 (for A0) */
	cxd90020_modify(CXD90020_CLASS_H_CTRL,0x01,0x03);

	/* PDN_ALL=0, gang mode */
	cxd90020_modify(CXD90020_DEVICE_CTRL,0x02,0x82);

	msleep(50);

	/* VLDO shunt=off */
	cxd90020_switch_vldo_shunt(OFF);

	msleep(50);

	/* set ramp=on, zc=off */
	cxd90020_modify(CXD90020_SOFT_RAMP_CTRL,0x22,0x3F);

	/* mute hp amp */
	cxd90020_write(CXD90020_HP_VOL_A,0x39);
	now_volume=0x39;

	/* hp amp on */
	cxd90020_modify(CXD90020_DEVICE_CTRL,0x00,0x60);

	/* ???? */
	cxd90020_write(0x51,0x3C);
	cxd90020_write(0x52,0xF8);

	/* enable interrupt */
	cxd90020_modify(CXD90020_INT_MASK,0x00,0x27);

	enabled=TRUE;

	mutex_unlock(&CXD90020->global_mutex);

	return(0);
}

static int cxd90020_disable(int level)
{
	print_trace("%s(%d)\n",__FUNCTION__,level);

	mutex_lock(&CXD90020->global_mutex);

	if(!enabled){
		print_debug("already disabled.\n");
		mutex_unlock(&CXD90020->global_mutex);
		return(0);
	}

	enabled=FALSE;

	/* disable interrupt */
	cxd90020_modify(CXD90020_INT_MASK,0x27,0x27);

	/* mute hp amp */
	cxd90020_write(CXD90020_HP_VOL_A,0x39);
	now_volume=0x39;

	/* hp amp off */
	cxd90020_modify(CXD90020_DEVICE_CTRL,0x60,0x60);

	msleep(50);

	/* VLDO shunt=on */
	cxd90020_switch_vldo_shunt(ON);

	msleep(50);

	/* PDN_ALL=1 */
	cxd90020_modify(CXD90020_DEVICE_CTRL,0x80,0x80);

	/* mclk=off */
	cxd90020_modify(CXD90020_CLOCK_CTRL,0x80,0x80);

#ifdef ICX_ENABLE_AU2CLK

	if(level==DAMP_CTL_LEVEL_POWER || level==DAMP_CTL_LEVEL_RESET){

		/* resset */
		cxd90020_reset();
	}

	if(level==DAMP_CTL_LEVEL_POWER){

		/* VD/VCP=off */
		cxd90020_switch_power(OFF);

		/* VG=off */
		cxd90020_switch_vg(OFF);
	}

#else

	/* VG=off */
	cxd90020_switch_vg(OFF);

#endif

	mutex_unlock(&CXD90020->global_mutex);

	return(0);
}

static int cxd90020_fade_volume(unsigned int volume)
{
	int wait;
	int rv;

	mutex_lock(&CXD90020->global_mutex);

	print_trace("%s(%02X)\n",__FUNCTION__,volume);

	if(!enabled){
		wait=WAIT_TIME_PER_STEP*1000;
		usleep_range(wait,wait+100);
		print_debug("wait time = %d [us]\n",wait);
		mutex_unlock(&CXD90020->global_mutex);
		return(0);
	}

	volume=volume&0x3F;
#if 0
	if(machine_is_icx1237() || machine_is_icx1240()){
		/* workaround for B0 */
		if(icx_board_id5==0){
			if(volume<0x11)
				volume=0x11;
		}
		/********************/
	}
#endif
	if(volume>now_volume){

		wait=WAIT_TIME_PER_STEP*1000/(volume-now_volume);
		if(wait<1000)
			wait=1000;

		while(now_volume!=volume){

			now_volume++;

			rv=cxd90020_write(CXD90020_HP_VOL_A,now_volume);
			if(rv<0){
				back_trace();
				mutex_unlock(&CXD90020->global_mutex);
				return(rv);
			}

			usleep_range(wait,wait+100);
			print_debug("wait time = %d [us]\n",wait);
		}
	}

	else if(volume<now_volume){

		wait=WAIT_TIME_PER_STEP*1000/(now_volume-volume);
		if(wait<1000)
			wait=1000;

		while(now_volume!=volume){

			now_volume--;

			rv=cxd90020_write(CXD90020_HP_VOL_A,now_volume);
			if(rv<0){
				back_trace();
				mutex_unlock(&CXD90020->global_mutex);
				return(rv);
			}

			usleep_range(wait,wait+100);
			print_debug("wait time = %d [us]\n",wait);
		}
	}

	else{
		wait=WAIT_TIME_PER_STEP*1000;
		usleep_range(wait,wait+100);
		print_debug("wait time = %d [us]\n",wait);
	}

	mutex_unlock(&CXD90020->global_mutex);

	return(0);
}

static int cxd90020_set_volume(unsigned int volume)
{
	int rv;

	mutex_lock(&CXD90020->global_mutex);

	print_trace("%s(%02X)\n",__FUNCTION__,volume);

	if(!enabled){
		mutex_unlock(&CXD90020->global_mutex);
		return(0);
	}

	volume=volume&0x3F;
#if 0
	if(machine_is_icx1237() || machine_is_icx1240()){
		/* workaround for B0 */
		if(icx_board_id5==0){
			if(volume<0x11)
				volume=0x11;
		}
		/********************/
	}
#endif
	rv=cxd90020_write(CXD90020_HP_VOL_A,volume);
	if(rv<0){
		back_trace();
		mutex_unlock(&CXD90020->global_mutex);
		return(rv);
	}

	now_volume=volume;

	mutex_unlock(&CXD90020->global_mutex);

	return(0);
}

static int cxd90020_switch_sys_clock(int value)
{
	mutex_lock(&CXD90020->global_mutex);

	print_trace("%s(%d)\n",__FUNCTION__,value);

	print_error("not supported.");

	mutex_unlock(&CXD90020->global_mutex);

	return(0);
}

static int cxd90020_switch_shunt_mute(int value)
{
	mutex_lock(&CXD90020->global_mutex);

	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(GPIO_HP_XMUTE2>0){
		cxd90020_switch_hp_xmute2(!value);
		msleep(50);
	}

	if(GPIO_HP_MUTE4>0){
		cxd90020_switch_hp_mute4(value);
		msleep(50);
	}

	mutex_unlock(&CXD90020->global_mutex);

	return(0);
}

/********************/
/*@ local_functions */
/********************/

static irqreturn_t cxd90020_clear_protect(int irq, void * data)
{
	unsigned long flags;

	print_trace("%s()\n",__FUNCTION__);

	spin_lock_irqsave(&CXD90020->lock,flags);
	if(CXD90020->clear_protect_count!=0){
		spin_unlock_irqrestore(&CXD90020->lock,flags);
		return(IRQ_HANDLED);
	}
	CXD90020->clear_protect_count=1;
	spin_unlock_irqrestore(&CXD90020->lock,flags);

	schedule_delayed_work(&CXD90020->clear_protect_work, 0);

	return(IRQ_HANDLED);
}

static void cxd90020_do_clear_protect_work(struct work_struct * work)
{
	unsigned long flags;
	unsigned int val;
	int rv;

	mutex_lock(&CXD90020->global_mutex);

	print_trace("%s()\n",__FUNCTION__);

#ifdef ICX_ENABLE_AU2CLK

	if(!enabled){
		CXD90020->clear_protect_count=0;
		mutex_unlock(&CXD90020->global_mutex);
		return;
	}

#endif

	cxd90020_read(CXD90020_INT_STAT,&val);

	print_info(
		"clear 0x%02X protect. (%d)\n",
		val&0xFF,
		CXD90020->clear_protect_count
	);

	cxd90020_write(CXD90020_PROTECTION_CTRL,0x20);
	cxd90020_write(CXD90020_PROTECTION_CTRL,0x70);
	cxd90020_write(CXD90020_PROTECTION_CTRL,0x20);

	rv=cxd90020_read(CXD90020_INT_STAT,&val);

	spin_lock_irqsave(&CXD90020->lock,flags);

	if(rv<0){
		CXD90020->clear_protect_count=0;
		spin_unlock_irqrestore(&CXD90020->lock,flags);
		mutex_unlock(&CXD90020->global_mutex);
		return;
	}

	if(val==0){
		CXD90020->clear_protect_count=0;
		spin_unlock_irqrestore(&CXD90020->lock,flags);
		mutex_unlock(&CXD90020->global_mutex);
		return;
	}

	CXD90020->clear_protect_count++;

	spin_unlock_irqrestore(&CXD90020->lock,flags);

	schedule_delayed_work(
		&CXD90020->clear_protect_work,
		msecs_to_jiffies(100)
	);

	mutex_unlock(&CXD90020->global_mutex);

	return;
}

static int cxd90020_setup_platform(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	mt_set_gpio_mode(GPIO_TBSAMP_XRST, GPIO_MODE_GPIO);
	mt_set_gpio_dir(GPIO_TBSAMP_XRST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_TBSAMP_XRST, GPIO_OUT_ONE);
	msleep(1);
	mt_set_gpio_out(GPIO_TBSAMP_XRST, GPIO_OUT_ZERO);
	msleep(1);
	mt_set_gpio_out(GPIO_TBSAMP_XRST, GPIO_OUT_ONE);

	mt_set_gpio_mode(GPIO_TBSAMP_XINT, GPIO_MODE_GPIO);
	mt_set_gpio_dir(GPIO_TBSAMP_XINT, GPIO_DIR_IN);

	mt_set_gpio_mode(GPIO_HP_XMUTE2, GPIO_MODE_GPIO);
	mt_set_gpio_dir(GPIO_HP_XMUTE2, GPIO_DIR_OUT);

#if 0
	rv=gpio_request(GPIO_TBSAMP_XINT, "TBSAMP_XINT"); CHK_RV("gpio_request(TBSAMP_XINT): code %d error occured.\n");
	rv=gpio_request(GPIO_TBSAMP_XRST, "TBSAMP_XRST"); CHK_RV("gpio_request(TBSAMP_XRST): code %d error occured.\n");

	rv=gpio_direction_input (GPIO_TBSAMP_XINT);    CHK_RV("gpio_direction_input(TBSAMP_XINT): code %d error occured.\n");
	rv=gpio_direction_output(GPIO_TBSAMP_XRST, 0); CHK_RV("gpio_direction_outout(TBSAMP_XRST): code %d error occured.\n");

	if(GPIO_HP_XMUTE2>=0){
		rv=gpio_request(GPIO_HP_XMUTE2,   "HP_XMUTE2"); CHK_RV("gpio_request(HP_XMUTE2): code %d error occured.\n");
		rv=gpio_direction_output(GPIO_HP_XMUTE2,    0); CHK_RV("gpio_direction_outout(HP_XMUTE2): code %d error occured.\n");
	}

	if(GPIO_HP_MUTE4>=0){
		rv=gpio_request(GPIO_HP_MUTE4,    "HP_MUTE4"); CHK_RV("gpio_request(HP_MUTE4): code %d error occured.\n");
		rv=gpio_direction_output(GPIO_HP_MUTE4,    1); CHK_RV("gpio_direction_outout(HP_MUTE4): code %d error occured.\n");
	}

	if(GPIO_VG_XON>=0){
		rv=gpio_request(GPIO_VG_XON, "VG_XON");   CHK_RV("gpio_request(VG_XON): code %d error occured.\n");
		rv=gpio_direction_output(GPIO_VG_XON, 0); CHK_RV("gpio_direction_outout(VG_XON): code %d error occured.\n");
	}

	if(GPIO_VLDO_XSHUNT>=0){
		rv=gpio_request(GPIO_VLDO_XSHUNT, "VLDO_XSHUNT"); CHK_RV("gpio_request(VLDO_XSHUNT): code %d error occured.\n");
		rv=gpio_direction_output(GPIO_VLDO_XSHUNT, 0);    CHK_RV("gpio_direction_outout(VLDO_XSHUNT): code %d error occured.\n");
	}

	if(GPIO_TBSAMP_PWR_ON>=0){
		rv=gpio_request(GPIO_TBSAMP_PWR_ON, "TBSAMP_PWR_ON"); CHK_RV("gpio_request(TBSAMP_PWR_ON): code %d error occured.\n");
		rv=gpio_direction_output(GPIO_TBSAMP_PWR_ON, 0);    CHK_RV("gpio_direction_outout(TBSAMP_PWR_ON): code %d error occured.\n");
	}
#endif
	return(0);
}

static int cxd90020_reset_platform(void)
{
	print_trace("%s()\n",__FUNCTION__);
#if 0
	if(GPIO_TBSAMP_PWR_ON>=0){
		gpio_direction_input(GPIO_TBSAMP_PWR_ON);
		gpio_free(GPIO_TBSAMP_PWR_ON);
	}

	if(GPIO_VLDO_XSHUNT>=0){
		gpio_direction_input(GPIO_VLDO_XSHUNT);
		gpio_free(GPIO_VLDO_XSHUNT);
	}

	if(GPIO_VG_XON>=0){
		gpio_direction_input(GPIO_VG_XON);
		gpio_free(GPIO_VG_XON);
	}

	if(GPIO_HP_MUTE4>=0){
		gpio_direction_input(GPIO_HP_MUTE4);
		gpio_free(GPIO_HP_MUTE4);
	}
	if(GPIO_HP_XMUTE2>=0){
		gpio_direction_input(GPIO_HP_XMUTE2);
		gpio_free(GPIO_HP_XMUTE2);
	}
	gpio_direction_input(GPIO_TBSAMP_XRST);
	gpio_direction_input(GPIO_TBSAMP_XINT);

	gpio_free(GPIO_TBSAMP_XRST);
	gpio_free(GPIO_TBSAMP_XINT);
#endif
	return(0);
}

static int cxd90020_reset(void)
{
	print_trace("%s()\n",__FUNCTION__);
	mt_set_gpio_out(GPIO_TBSAMP_XRST, GPIO_OUT_ZERO);
#if 0
	gpio_set_value(GPIO_TBSAMP_XRST,0);
#endif
	return(0);
}

static int cxd90020_unreset(void)
{
	print_trace("%s()\n",__FUNCTION__);
	mt_set_gpio_out(GPIO_TBSAMP_XRST, GPIO_OUT_ONE);
#if 0
	gpio_set_value(GPIO_TBSAMP_XRST,1);

	msleep(10);
#endif
	return(0);
}

static int cxd90020_switch_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(GPIO_TBSAMP_PWR_ON>=0){
		if(value){
			gpio_set_value(GPIO_TBSAMP_PWR_ON,1);
			msleep(50);
		}
		else{
			gpio_set_value(GPIO_TBSAMP_PWR_ON,0);
		}
	}
#endif
	return(0);
}

static int cxd90020_switch_vg(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(GPIO_VG_XON>=0){
		if(value)
			gpio_set_value(GPIO_VG_XON,0);
		else
			gpio_set_value(GPIO_VG_XON,1);
	}
#endif
	return(0);
}

static int cxd90020_switch_hp_xmute2(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value) {
//		gpio_set_value(GPIO_HP_XMUTE2,1);
		mt_set_gpio_out(GPIO_HP_XMUTE2, GPIO_OUT_ONE);
	}else {
//		gpio_set_value(GPIO_HP_XMUTE2,0);
		mt_set_gpio_out(GPIO_HP_XMUTE2, GPIO_OUT_ZERO);
	}
	return(0);
}

static int cxd90020_switch_hp_mute4(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value)
		gpio_set_value(GPIO_HP_MUTE4,1);
	else
		gpio_set_value(GPIO_HP_MUTE4,0);
#endif
	return(0);
}

static int cxd90020_switch_vldo_shunt(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(GPIO_VLDO_XSHUNT>=0){
		if(value)
			gpio_set_value(GPIO_VLDO_XSHUNT,0);
		else
			gpio_set_value(GPIO_VLDO_XSHUNT,1);
	}
#endif
	return(0);
}

static int cxd90020_get_tbsamp_xint_value(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);
#if 0
	rv=gpio_get_value(GPIO_TBSAMP_XINT);
#endif
	return(rv);
}

/*********************/
/*@ access_functions */
/*********************/

static int cxd90020_modify(
	unsigned int address,
	unsigned int value,
	unsigned int mask
)
{
	unsigned int tmp;
	int rv;

	/* print_trace("%s(%X,%X,%X)\n",__FUNCTION__,address,value,mask); */

	mutex_lock(&CXD90020->mutex);

	rv=cxd90020_read_core(address,&tmp);
	if(rv<0) {
		back_trace();
		mutex_unlock(&CXD90020->mutex);
		return(rv);
	}

	tmp=(tmp&~mask)|(value&mask);

	rv=cxd90020_write_core(address,tmp);
	if(rv<0) {
		back_trace();
		mutex_unlock(&CXD90020->mutex);
		return(rv);
	}

	mutex_unlock(&CXD90020->mutex);

	return(0);
}

static int cxd90020_write(
	unsigned int address,
	unsigned int value
)
{
	int rv;

	/* print_trace("%s(%X,%X)\n",__FUNCTION__,address,value); */

	mutex_lock(&CXD90020->mutex);

	rv=cxd90020_write_core(address,value);
	if(rv<0) {
		back_trace();
		mutex_unlock(&CXD90020->mutex);
		return(rv);
	}

	mutex_unlock(&CXD90020->mutex);

	return(0);
}

static int cxd90020_read(
	unsigned int   address,
	unsigned int * value
)
{
	int rv;

	/* print_trace("%s(%X)\n",__FUNCTION__,address); */

	mutex_lock(&CXD90020->mutex);

	rv=cxd90020_read_core(address,value);
	if(rv<0) {
		back_trace();
		mutex_unlock(&CXD90020->mutex);
		return(rv);
	}

	mutex_unlock(&CXD90020->mutex);

	return(0);
}

static int cxd90020_write_core(
	unsigned int address,
	unsigned int value
)
{
	struct i2c_msg msg;
	unsigned char buf[2];
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	buf[0]=(unsigned char)address;
	buf[1]=(unsigned char)value;

	msg.addr  = CXD90020->i2c_client->addr;
	msg.flags = 0;
	msg.len   = 2;
	msg.buf   = buf;
#ifdef CONFIG_MTK_I2C
	msg.ext_flag = 0;
	msg.timing = 400;
#endif

	rv=i2c_transfer(CXD90020->i2c_client->adapter,&msg,1);
	if(rv<0) {
		print_fail("i2c_transfer(): code %d error occurred.\n",rv);
		back_trace();
		return(rv);
	}

	if(rv!=1){
		print_error("count mismacth.\n");
		return(-1);
	}

	/* print_debug("W %02X <- %02X\n",address,value); */

	return(0);
}

static int cxd90020_read_core(
	unsigned int   address,
	unsigned int * value
)
{
	struct i2c_msg msg[2];
	unsigned char addr;
	unsigned char val;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	addr=(unsigned char)address;

	msg[0].addr  = CXD90020->i2c_client->addr;
	msg[0].flags = 0;
	msg[0].len   = 1;
	msg[0].buf   = &addr;
#ifdef CONFIG_MTK_I2C
	msg[0].ext_flag = 0;
	msg[0].timing = 400;
#endif

	msg[1].addr  = CXD90020->i2c_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len   = 1;
	msg[1].buf   = &val;
#ifdef CONFIG_MTK_I2C
	msg[1].ext_flag = 0;
	msg[1].timing = 400;
#endif
	rv=i2c_transfer(CXD90020->i2c_client->adapter,msg,2);
	if(rv<0) {
		print_fail("i2c_transfer(): code %d error occurred.\n",rv);
		back_trace();
		return(rv);
	}

	if(rv!=2){
		print_error("count mismacth.\n");
		return(-1);
	}

	*value=val;

	/* print_debug("R %02X -> %02X\n",address,*value); */

	return(0);
}

/*********************/
/*@ regmon_functions */
/*********************/

#ifdef CONFIG_REGMON_DEBUG

static int cxd90020_regmon_write_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int   value
);

static int cxd90020_regmon_read_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int * value
);

static regmon_reg_info_t cxd90020_regmon_reg_info[] =
{
	{ "DEVICE_ID_AB",    0x01 },
	{ "DEVICE_ID_CD",    0x02 },
	{ "DEVICE_ID_E",     0x03 },
	{ "REV_ID",          0x05 },
	{ "DEVICE_CTRL",     0x06 },
	{ "CLOCK_CTRL",      0x07 },
	{ "PROTECTION_CTRL", 0x08 },
	{ "HP_VOL_A",        0x09 },
	{ "HP_VOL_B",        0x0A },
	{ "SOFT_RAMP_CTRL",  0x0B },
	{ "INT_MASK",        0x0C },
	{ "INT_STAT",        0x0D },
	{ "CLASS_H_CTRL",    0x0E },
	{ "CHRG_PUMP_FREQ",  0x0F },
};

static regmon_customer_info_t cxd90020_customer_info =
{
	.name           = "cxd90020",
	.reg_info       = cxd90020_regmon_reg_info,
	.reg_info_count = sizeof(cxd90020_regmon_reg_info)/sizeof(regmon_reg_info_t),
	.write_reg      = cxd90020_regmon_write_reg,
	.read_reg       = cxd90020_regmon_read_reg,
	.private_data   = NULL,
};

static int cxd90020_regmon_add(void)
{
	regmon_add(&cxd90020_customer_info);
	return(0);
}

static int cxd90020_regmon_del(void)
{
	regmon_del(&cxd90020_customer_info);
	return(0);
}

static int cxd90020_regmon_write_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int   value
)
{
	int rv;

	mutex_lock(&CXD90020->global_mutex);

#ifdef ICX_ENABLE_AU2CLK
	if(!enabled){
		mutex_unlock(&CXD90020->global_mutex);
		return(0);
	}
#else
	if(!initialized){
		mutex_unlock(&CXD90020->global_mutex);
		return(0);
	}
#endif

	rv=cxd90020_write(address,value);

	mutex_unlock(&CXD90020->global_mutex);

	return(rv);
}

static int cxd90020_regmon_read_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int * value
)
{
	int rv;

	mutex_lock(&CXD90020->global_mutex);

#ifdef ICX_ENABLE_AU2CLK
	if(!enabled){
		*value=0xFFFFFFFF;
		mutex_unlock(&CXD90020->global_mutex);
		return(0);
	}
#else
	if(!initialized){
		*value=0xFFFFFFFF;
		mutex_unlock(&CXD90020->global_mutex);
		return(0);
	}
#endif

	rv=cxd90020_read(address,value);

	mutex_unlock(&CXD90020->global_mutex);

	return(rv);
}

#endif
