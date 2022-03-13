/*
 * Copyright 2016 Sony Corporation
 * File Changed on 2016-10-17
 */
/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include <linux/signal.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <cust_eint.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>
#include <asm/bitops.h>
#include "mt8590-afe.h"
#include "mt8590-audrtc.h"
#include "mt8590-memif.h"
#include <asm/div64.h>

#define FIQ_TEST 1

#define SUPPORT_GET_SCHED_CLOCK

#ifdef CUST_EINT_WIFI_6625_TSF_NUM
#undef CUST_EINT_WIFI_6625_TSF_NUM
#endif
#define CUST_EINT_WIFI_6625_TSF_NUM	(3)

#ifdef CUST_EINT_WIFI_6630_TSF_NUM
#undef CUST_EINT_WIFI_6630_TSF_NUM
#endif
#define CUST_EINT_WIFI_6630_TSF_NUM	(0)

#define AUDRTC_CMD_ACC_TRIGGER			_IOW(0, 0, int)
#define AUDRTC_CMD_SET_TARGET_ACC		_IOW(0, 1, int)
#define AUDRTC_CMD_GET_PACKET			_IOR(0, 2, int)
#define AUDRTC_CMD_GET_TARGET_ACC_STATUS	_IOR(0, 3, int)
#ifdef SUPPORT_GET_SCHED_CLOCK
#define AUDRTC_CMD_GET_SCHED_CLOCK		_IOR(0, 4, int)
#endif
#define AUDRTC_CMD_SET_MODE				_IOR(0, 5, int)
#define AUDRTC_CMD_CANCEL_TARGET_ACC    _IOR(0, 6, int)


#define PACKETBUFSIZE 100

static DEFINE_MUTEX(packet_lock);

struct audrtc_packet {
    u32 acc; /* target_acc_result(8bit) << 24 | acc(24bit) */
	u32 clk_info_0;/*bt:current_bt_clk,wlan:tsf LSB32bit */
	u32 clk_info_1;/*bt:intrasolt_offset,wlan:tsf MSB32bit*/
	u32 btwlanflag;/*bt:!0, wlan:0*/
	u32 tv_sec;
	u32 tv_usec;
};

#ifdef SUPPORT_GET_SCHED_CLOCK
struct audrtc_timeval {
	u32 tv_sec;
	u32 tv_usec;
	};
#endif

#define AUDRTC_SYSTIME_RECV_TSF_INT	0

#if !FIQ_TEST
static DECLARE_WAIT_QUEUE_HEAD(audrtc_wq);
#endif

struct audrtc_private {
	enum afe_sampling_rate fs;
	u32 acc_int;
	struct timeval tv_int;
	struct audrtc_packet packet;
	struct fasync_struct *fasync_queue;
	unsigned long long systime[3];  /* system time when TSF isr is triggered */
	volatile long event;
};

static volatile struct audrtc_private *audrtc_priv;

#define BIT_AUDRTC_MSG_QUIT		(24)		// Modified 0 -> 24 at 20160419 by Sony
#define AUDRTC_MSG_QUIT			(0x1 << BIT_AUDRTC_MSG_QUIT)
#define BIT_AUDRTC_MSG_NOTIFY		(25)	// Modified 0 -> 25 at 20160419 by Sony
#define AUDRTC_MSG_NOTIFY		(0x1 << BIT_AUDRTC_MSG_NOTIFY)
#define BIT_AUDRTC_MSG_NOTIFY_6630	(26)	// Modified 0 -> 26 at 20160419 by Sony
#define AUDRTC_MSG_NOTIFY_6630		(0x1 << BIT_AUDRTC_MSG_NOTIFY_6630)

typedef int (*pf_get_latched_tsf)(unsigned int *ptr);

static pf_get_latched_tsf pf_get_TSF_media_sync;

void register_get_TSF_for_media_synchronization_handler(pf_get_latched_tsf handler)
{
	pr_debug("%s()n", __func__);
	pf_get_TSF_media_sync = handler;
}
EXPORT_SYMBOL(register_get_TSF_for_media_synchronization_handler);

static pf_get_latched_tsf pf_get_6630_TSF_media_sync;

void register_get_6630_TSF_for_media_synchronization_handler(pf_get_latched_tsf handler)
{
	pr_debug("%s()n", __func__);
	pf_get_6630_TSF_media_sync = handler;
}
EXPORT_SYMBOL(register_get_6630_TSF_for_media_synchronization_handler);

static enum audio_irq_id irq_id = IRQ_NUM;

volatile int target_acc_result = TARGET_ACC_IDLE;
volatile enum audio_irq_id target_acc_irq_id = IRQ_NUM;

static volatile int rtcthreadrun = 0;

volatile struct audrtc_packet packetbuf[PACKETBUFSIZE];

/*bt just use 6625 new path . wlan both 6625 and 6630*/
volatile static int btwlan=0;/*0:wlan,1:bt */

unsigned int bufRdx = 0;


static int audrtc_thread(void *data)
{

	struct audrtc_private *priv = data;

	rtcthreadrun = 1;

	if(btwlan)/*bt*/
	{
		for (;;) {
			#if FIQ_TEST
				while (bufRdx == priv->event)/*buffer empty*/
					msleep(10);
			#else
				wait_event_interruptible(audrtc_wq, (priv->event != 0));
			#endif
				while(bufRdx != priv->event)/*buffer has data,	send it all*/
				{
					priv->packet.acc = packetbuf[bufRdx].acc;
					priv->packet.btwlanflag = packetbuf[bufRdx].btwlanflag;
					priv->packet.clk_info_0 = packetbuf[bufRdx].clk_info_0;
					priv->packet.clk_info_1 = packetbuf[bufRdx].clk_info_1;
					priv->packet.tv_sec = packetbuf[bufRdx].tv_sec;
					priv->packet.tv_usec = packetbuf[bufRdx].tv_usec;
					pr_debug("%s() notify acc=0x%08x, clk_info_0=0x%08x, clk_info_1=0x%08x, btwlanflag=0x%08x\n", __func__, priv->packet.acc, priv->packet.clk_info_0,priv->packet.clk_info_1,priv->packet.btwlanflag);
					if (priv->fasync_queue)
						kill_fasync(&priv->fasync_queue, SIGIO, POLL_OUT);

					bufRdx++;
					if(bufRdx == PACKETBUFSIZE)
						bufRdx = 0;

				}
			if (test_and_clear_bit(BIT_AUDRTC_MSG_QUIT, &priv->event)) { // Added at 20160419 by Sony
				pr_debug("%s() receive event: BIT_AUDRTC_MSG_QUIT\n", __func__);
				break;
			}

		}

	}
	else/*wlan*/
	{
	for (;;) {
#if FIQ_TEST
        while (priv->event == 0 &&
               !(target_acc_result & TARGET_ACC_COMES)) {
          msleep(10);
        }
        if (target_acc_result & TARGET_ACC_COMES) {
          mutex_lock(&packet_lock);
          priv->packet.acc &= 0x00ffffff;
          priv->packet.acc |= target_acc_result << TARGET_ACC_RESULT_OFFSET;
          mutex_unlock(&packet_lock);
          pr_notice("%s() TargetAcc Result store %d\n", __func__, target_acc_result);
          target_acc_result = TARGET_ACC_IDLE;
          if (priv->fasync_queue)
            kill_fasync(&priv->fasync_queue, SIGIO, POLL_OUT);
        }
#else
		wait_event_interruptible(audrtc_wq, (priv->event != 0));
#endif
		if (test_and_clear_bit(BIT_AUDRTC_MSG_QUIT, &priv->event)) {
			pr_debug("%s() receive event: BIT_AUDRTC_MSG_QUIT\n", __func__);
			break;
		}
		if (test_and_clear_bit(BIT_AUDRTC_MSG_NOTIFY, &priv->event)) {

			unsigned int tsf_info[8] = {0};
			pr_debug("%s() receive event: BIT_AUDRTC_MSG_NOTIFY\n", __func__);
			if (pf_get_TSF_media_sync)
				pf_get_TSF_media_sync(tsf_info);
			else
				pr_err("%s() TSF func handler is not registered\n", __func__);

			mutex_lock(&packet_lock);
			priv->packet.tv_sec = priv->tv_int.tv_sec;
			priv->packet.tv_usec = priv->tv_int.tv_usec;
            priv->packet.acc &= 0xff000000;
            priv->packet.acc |= (priv->acc_int & 0x00ffffff);
			priv->packet.clk_info_0 = tsf_info[2] /* (((u64)tsf_info[3]) << 32) | ((u64)tsf_info[2]) */;
			priv->packet.clk_info_1 = tsf_info[3];
			priv->packet.btwlanflag = tsf_info[5];
			mutex_unlock(&packet_lock);
			pr_debug("%s() notify acc=0x%08x, clk_info_0=0x%08x, clk_info_1=0x%08x, btwlanflag=0x%08x\n", __func__, priv->packet.acc, priv->packet.clk_info_0,priv->packet.clk_info_1,priv->packet.btwlanflag);
			if (priv->fasync_queue)
				kill_fasync(&priv->fasync_queue, SIGIO, POLL_OUT);

                        test_and_clear_bit(BIT_AUDRTC_MSG_NOTIFY, &priv->event);

		}
		if (test_and_clear_bit(BIT_AUDRTC_MSG_NOTIFY_6630, &priv->event)) {
			unsigned int tsf_info[8] = {0};
			pr_debug("%s() receive event: BIT_AUDRTC_MSG_NOTIFY_6630\n", __func__);
			if (pf_get_6630_TSF_media_sync)
				pf_get_6630_TSF_media_sync(&tsf_info);
			else
				pr_err("%s() 6630 TSF func handler is not registered\n", __func__);
			mutex_lock(&packet_lock);
			priv->packet.tv_sec = priv->tv_int.tv_sec;
			priv->packet.tv_usec = priv->tv_int.tv_usec;
            priv->packet.acc &= 0xff000000;
            priv->packet.acc |= (priv->acc_int & 0x00ffffff);
			priv->packet.clk_info_0 = tsf_info[2];
			mutex_unlock(&packet_lock);
			pr_debug("%s() notify 6630 acc=0x%08x, clk_info_0=0x%08x\n", __func__, priv->packet.acc, priv->packet.clk_info_0);
			if (priv->fasync_queue)
				kill_fasync(&priv->fasync_queue, SIGIO, POLL_OUT);

			test_and_clear_bit(BIT_AUDRTC_MSG_NOTIFY_6630, &priv->event);
		}
	}
	}
	rtcthreadrun = 0;
	return 0;
}

#if FIQ_TEST
static void audrtc_fiq_isr(void *arg, void *regs, void *svc_sp)
{
	unsigned long long t,sec,usec,res;
	unsigned int tsf_info[8] = {0};

	if (!audrtc_priv){
		mt_eint_unmask(CUST_EINT_WIFI_6625_TSF_NUM);
		return;
	}

	t = sched_clock(); /*ns*/
	res = do_div(t,1000000000);
	sec = t;
	do_div(res,1000);
	usec = res;

	if(btwlan)/*bt*/
	{
		if (pf_get_TSF_media_sync)   // Restored for bt mode at 20160419 by Sony
			pf_get_TSF_media_sync(tsf_info);

		packetbuf[audrtc_priv->event].acc = audio_irq_cnt_mon(irq_id);
	        packetbuf[audrtc_priv->event].tv_sec = sec;
		packetbuf[audrtc_priv->event].tv_usec = usec;
		packetbuf[audrtc_priv->event].clk_info_0 = tsf_info[2];
		packetbuf[audrtc_priv->event].clk_info_1 = tsf_info[3];
		packetbuf[audrtc_priv->event].btwlanflag = tsf_info[5];
		audrtc_priv->event++;
		if(PACKETBUFSIZE == audrtc_priv->event)
			audrtc_priv->event = 0;
	}
	else//wlan
	{
		audrtc_priv->acc_int = audio_irq_cnt_mon(irq_id);
	audrtc_priv->tv_int.tv_sec = sec;
	audrtc_priv->tv_int.tv_usec = usec;
	audrtc_priv->event |= AUDRTC_MSG_NOTIFY;
	}

	mt_eint_unmask(CUST_EINT_WIFI_6625_TSF_NUM);
}
static void audrtc_fiq_isr_6630(void *arg, void *regs, void *svc_sp)
{
	unsigned long long t,sec,usec,res;
	if (!audrtc_priv){
		mt_eint_unmask(CUST_EINT_WIFI_6630_TSF_NUM);
		return;
	}
	audrtc_priv->acc_int = audio_irq_cnt_mon(irq_id);
	t = sched_clock();
	res = do_div(t,1000000000);
	sec = t;
	do_div(res,1000);
	usec = res;
	audrtc_priv->tv_int.tv_sec = sec;
	audrtc_priv->tv_int.tv_usec = usec;
	audrtc_priv->event |= AUDRTC_MSG_NOTIFY_6630;
	mt_eint_unmask(CUST_EINT_WIFI_6630_TSF_NUM);
}
#else
static void audrtc_irq_isr(void)
{
	unsigned long long t;
	if (!audrtc_priv) {
		pr_warn("%s() audrtc is not running\n", __func__);
		return;
	}
	t = sched_clock();
	do_gettimeofday(&audrtc_priv->tv_int);
	audrtc_priv->acc_int = audio_irq_cnt_mon(irq_id);
	audrtc_priv->event |= AUDRTC_MSG_NOTIFY;
	wake_up_interruptible(&audrtc_wq);
	audrtc_priv->systime[AUDRTC_SYSTIME_RECV_TSF_INT] = t;
}
static void audrtc_irq_isr_6630(void)
{
	unsigned long long t;
	if (!audrtc_priv) {
		pr_warn("%s() audrtc is not running\n", __func__);
		return;
	}
	t = sched_clock();
	do_gettimeofday(&audrtc_priv->tv_int);
	audrtc_priv->acc_int = audio_irq_cnt_mon(irq_id);
	audrtc_priv->event |= AUDRTC_MSG_NOTIFY_6630;
	wake_up_interruptible(&audrtc_wq);
	audrtc_priv->systime[AUDRTC_SYSTIME_RECV_TSF_INT] = t;
}
#endif

static int audrtc_fasync(int fd, struct file *filp, int on)
{
	struct audrtc_private *priv = filp->private_data;
	if (!priv)
		return -EFAULT;
	pr_debug("%s()\n", __func__);
	return fasync_helper(fd, filp, on, &priv->fasync_queue);
}


static int audrtc_open(struct inode *node, struct file *filp)
{
	struct task_struct *t;
	struct audrtc_private *priv;
	int timeout = 5000;/*5s quit*/
	pr_debug("%s()\n", __func__);
	if (audrtc_priv) {
		pr_err("%s() error: audrtc is running\n", __func__);
		goto error;
	}
	priv = kzalloc(sizeof(struct audrtc_private), GFP_KERNEL);
	if (!priv) {
		pr_err("%s() error: kzalloc audrtc_private failed\n", __func__);
		goto alloc_priv_error;
	}
	t = kthread_run(audrtc_thread, priv, "audrtc_thread");
	if (IS_ERR(t)) {
		pr_err("%s() error: kthread_run return 0x%p\n", __func__, t);
		goto create_thread_error;
	}
	while (!rtcthreadrun && (--timeout != 0))
		msleep(1);

        if (timeout == 0) {
		pr_err("%s() error: rtcthreadrun can not run\n", __func__);
		goto create_thread_error;
        }
	bufRdx = 0; // Added at 20160419 by Sony
	filp->private_data = priv;
	audrtc_priv = priv;
	target_acc_result = TARGET_ACC_IDLE;
	a1sys_start(AUD1PLL, NULL, 1);
	enable_pll(AUD2PLL, "AUDIO");
	enable_clock(MT_CG_AUDIO_A2SYS, "AUDIO");
	return 0;
create_thread_error:
	kzfree(priv);
alloc_priv_error:
error:
	return -EPERM;
}

static int audrtc_release(struct inode *node, struct file *filp)
{
	struct audrtc_private *priv = filp->private_data;
	pr_debug("%s()\n", __func__);
	if (!priv)
		return -ENOMEM;
	audio_irq_enable(irq_id, 0);
	asys_irq_release(irq_id);
	irq_id = IRQ_NUM;
	target_acc_irq_id = IRQ_NUM;
	/* Changed to use AUDRTC_MSG_QUIT for BT and WLAN at 20160419 by Sony
	if(btwlan)
		priv->event = 0;
	else */
	priv->event |= AUDRTC_MSG_QUIT;
#if !FIQ_TEST
	wake_up_interruptible(&audrtc_wq);
#endif
	audrtc_fasync(-1, filp, 0);
        while (rtcthreadrun)
		msleep(2);
	kzfree(priv);
	filp->private_data = NULL;
	audrtc_priv = NULL;
	disable_clock(MT_CG_AUDIO_A2SYS, "AUDIO");
	disable_pll(AUD2PLL, "AUDIO");
	a1sys_start(AUD1PLL, NULL, 0);
	return 0;
}

static inline u32 acc_distance(u32 cur, u32 target, u32 range)
{
	if (target < cur)
		return cur - target;
	else
		return range + cur - target;
}

static long audrtc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct audrtc_private *priv = filp->private_data;
	pr_debug("%s() cmd=0x%08x, arg=0x%lx\n", __func__, cmd, arg);
	if (!priv)
		return -ENOMEM;
	switch (cmd) {
	case AUDRTC_CMD_ACC_TRIGGER:
		if (arg) {
			struct audio_irq_config config = {
				.mode = fs_enum(arg),
				.init_val = 0xffffff
			};
			irq_id = asys_irq_acquire();
			audio_irq_configurate(irq_id, &config);
			if (audio_irq_enable(irq_id, 1) != 0) {
				asys_irq_release(irq_id);
				irq_id = IRQ_NUM;
				return -EINVAL;
			}
			priv->fs = config.mode;
		} else {
			audio_irq_enable(irq_id, 0);
			asys_irq_release(irq_id);
			irq_id = IRQ_NUM;
			target_acc_irq_id = IRQ_NUM;
		}
		break;
	case AUDRTC_CMD_GET_PACKET: {
		unsigned long ret;
		if (!arg)
			return -EFAULT;
		mutex_lock(&packet_lock);
		ret = copy_to_user((void __user *)(arg), (void *)(&priv->packet), sizeof(struct audrtc_packet));
        priv->packet.acc &= 0x00ffffff;
		mutex_unlock(&packet_lock);
		if (ret != 0)
			return -EFAULT;
		break;
	}
	case AUDRTC_CMD_SET_TARGET_ACC: {
		u32 cur_acc;
		if (irq_id == IRQ_NUM) {
			pr_err("%s() error: AUDRTC_CMD_SET_TARGET_ACC, acc is not running\n", __func__);
			return -EPERM;
		}
		if (target_acc_irq_id != IRQ_NUM) {
			pr_err("%s() error: AUDRTC_CMD_SET_TARGET_ACC, target acc is running, can't set new target acc\n", __func__);
			return -EBUSY;
		}
		if (arg > 0xffffff || arg < 0x1) {
			pr_err("%s() error: AUDRTC_CMD_SET_TARGET_ACC, bad acc value: 0x%08lx\n", __func__, arg);
			return -EINVAL;
		}
        if (target_acc_result != TARGET_ACC_IDLE) {
          pr_err("%s() error: AUDRTC_CMD_SET_TARGET_ACC, target acc result is not notified\n", __func__);
          return -EBUSY;
        }
        if (priv->packet.acc & 0xff000000) {
          pr_err("%s() error: AUDRTC_CMD_SET_TARGET_ACC, target acc result is not got by app\n", __func__);
          return -EBUSY;
        }
		target_acc_irq_id = asys_irq_acquire();
		if (target_acc_irq_id == IRQ_NUM) {
			pr_err("%s() error: AUDRTC_CMD_SET_TARGET_ACC, no more asys irq\n", __func__);
			return -EINVAL;
		}
		/******** must be fast enough (start) ********/
		cur_acc = audio_irq_cnt_mon(irq_id);
		{
			struct audio_irq_config config = {
				.mode = priv->fs,
				.init_val = acc_distance(cur_acc, arg, 0xffffff)
			};
			pr_notice("%s() AUDRTC_CMD_SET_TARGET_ACC, fs=%d, acc_distance=0x%08x\n", __func__, config.mode, config.init_val);
			audio_irq_configurate(target_acc_irq_id, &config);
			if (audio_irq_enable(target_acc_irq_id, 1) != 0) {
				asys_irq_release(target_acc_irq_id);
				target_acc_irq_id = IRQ_NUM;
				return -EINVAL;
			}
		}
		/******** must be fast enough (end) ********/
		break;
	}
	case AUDRTC_CMD_GET_TARGET_ACC_STATUS: {
		int status;
		if (!arg)
			return -EFAULT;
		status = (target_acc_irq_id == IRQ_NUM) ? 0 : 1;
		if (copy_to_user((void __user *)(arg), (void *)(&status), sizeof(status)) != 0)
			return -EFAULT;
		break;
	}

	#ifdef SUPPORT_GET_SCHED_CLOCK
	case AUDRTC_CMD_GET_SCHED_CLOCK: {
		unsigned long ret;
		unsigned long long t, sec, usec, res;
		struct audrtc_timeval tv;
		if (!arg)
			return -EFAULT;
		t = sched_clock();
		res = do_div(t,1000000000);
		sec = t;
		do_div(res,1000);
		usec = res;
		tv.tv_sec = (u32)sec;
		tv.tv_usec = (u32)usec;
		ret = copy_to_user((void __user *)(arg), (void *)(&tv), sizeof(tv));
		if (ret != 0)
			return -EFAULT;
		break;
	}
	#endif

	#ifdef AUDRTC_CMD_SET_MODE
	case AUDRTC_CMD_SET_MODE:
		btwlan = arg;
		pr_notice("%s() AUDRTC_CMD_SET_MODE, btwlan=%d\n", __func__, btwlan);
		break;
	#endif

	case AUDRTC_CMD_CANCEL_TARGET_ACC: {
		int idx;
		for (idx=0; idx < total_playback_num; idx++){
			afe_memif_enable(rtc_multi[idx].memiftargetacc, 1);
			audio_irq_enable(rtc_multi[idx].targetaccirqid, 1);
		}
		total_playback_num = 0;
		audio_irq_enable(target_acc_irq_id, 0);
		asys_irq_release(target_acc_irq_id);
		target_acc_irq_id = IRQ_NUM; /* let trigger go */
		pr_notice("%s() AUDRTC_CMD_CANCEL_TARGET_ACC\n", __func__);
		break;
	}
	
	default:
		pr_err("%s() error: unknown audrtc cmd 0x%08x\n", __func__, cmd);
		return -EINVAL;
	}
	return 0;
}

static const struct file_operations audrtc_fops = {
	.owner = THIS_MODULE,
	.open = audrtc_open,
	.release = audrtc_release,
	.unlocked_ioctl = audrtc_ioctl,
	.write = NULL,
	.read = NULL,
	.flush = NULL,
	.fasync = audrtc_fasync,
	.mmap = NULL
};

static struct miscdevice audrtc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "audrtc",
	.fops = &audrtc_fops,
};

static int audrtc_mod_init(void)
{
	int ret;
	pr_debug("%s()\n", __func__);
	ret = misc_register(&audrtc_device);
	if (ret) {
		pr_err("%s() error: misc_register audrtc failed %d\n", __func__, ret);
		return ret;
	}
#if FIQ_TEST
	pr_debug("%s() reg FIQ\n", __func__);
	mt_deint_registration(CUST_EINT_WIFI_6625_TSF_NUM, EINTF_TRIGGER_RISING, audrtc_fiq_isr);
#else
	init_waitqueue_head(&audrtc_wq);
	pr_debug("%s() reg IRQ\n", __func__);
	mt_eint_registration(CUST_EINT_WIFI_6625_TSF_NUM, EINTF_TRIGGER_RISING, audrtc_irq_isr, 1);
#endif

#if FIQ_TEST
    pr_debug("%s() reg 6630 FIQ\n", __func__);
	mt_deint_registration(CUST_EINT_WIFI_6630_TSF_NUM, EINTF_TRIGGER_RISING, audrtc_fiq_isr_6630);
#else
	init_waitqueue_head(&audrtc_wq);
	pr_debug("%s() reg 6630 IRQ\n", __func__);
	mt_eint_registration(CUST_EINT_WIFI_6630_TSF_NUM, EINTF_TRIGGER_RISING, audrtc_irq_isr_6630, 1);
#endif
	return 0;
}

static void audrtc_mod_exit(void)
{
	int ret;
	pr_debug("%s()\n", __func__);
	ret = misc_deregister(&audrtc_device);
	if (ret)
		pr_err("%s() error: misc_deregister audrtc failed %d\n", __func__, ret);
}
module_init(audrtc_mod_init);
module_exit(audrtc_mod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("audrtc driver");
