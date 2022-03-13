/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf_common.h
 *
 * CXD3774GF CODEC driver
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

#ifndef _CXD3774GF_COMMON_HEADER_
#define _CXD3774GF_COMMON_HEADER_

#define TRACE_PRINT_ON
#define DEBUG_PRINT_ON

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/switch.h>
#include <linux/notifier.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/proc_fs.h>
#include "../../fs/proc/internal.h"

#include <linux/regulator/consumer.h>

#include <mach/io.h>
#include <mach/mt_gpio.h>

#ifdef CONFIG_REGMON_DEBUG
#include <mach/regmon.h>
#endif

#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>
#include <sound/initval.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>
#include <sound/jack.h>

#include <sound/digiamp.h>
#include <sound/cxd3774gf.h>

/**************/
/* parameters */
/**************/

#define USE_PCM_MONITORING_MUTE

#define REGISTER_ACCESS_RETRY_COUNT 1

#define PWM_OUT_MUTE_DEALY_2 2000

#define CXD3774GF_WAKE_LOCK_TIME 10 /* sec */

/***************/
/* definitions */
/***************/

/* basic */
#define FALSE 0
#define TRUE  1
#define OFF   0
#define ON    1
#define LOW   0
#define HIGH  1

/* nc headphone type */
#define NCHP_TYPE_BUNDLE 0
#define NCHP_TYPE_MODEL1 1
#define NCHP_TYPE_MAX    1

/* jack mode */
#define JACK_MODE_HEADPHONE 0
#define JACK_MODE_ANTENNA   1
#define JACK_MODE_MAX       1

/* tone control */
#define TONE_CONTROL_NON_HP     0
#define TONE_CONTROL_GENERAL_HP 1
#define TONE_CONTROL_NC_HP      2 /* add NCHP_TYPE_XX to use */
#define TONE_CONTROL_MAX       (2+NCHP_TYPE_MAX)

/* noise cancel mode */
#define NOISE_CANCEL_MODE_OFF      0
#define NOISE_CANCEL_MODE_AIRPLANE 1
#define NOISE_CANCEL_MODE_TRAIN    2
#define NOISE_CANCEL_MODE_OFFICE   3
#define NOISE_CANCEL_MODE_MAX      3

/* input device */
#define INPUT_DEVICE_NONE      0
#define INPUT_DEVICE_TUNER     1
#define INPUT_DEVICE_MIC       2
#define INPUT_DEVICE_LINE      3
#define INPUT_DEVICE_DIRECTMIC 4
#define INPUT_DEVICE_MAX       4

/* output device */
#define OUTPUT_DEVICE_NONE      0
#define OUTPUT_DEVICE_HEADPHONE 1
#define OUTPUT_DEVICE_LINE      2
#define OUTPUT_DEVICE_SPEAKER   3
#define OUTPUT_DEVICE_FIXEDLINE 4
#define OUTPUT_DEVICE_MAX       4

/* headphone amp */
#define HEADPHONE_AMP_NORMAL   0
#define HEADPHONE_AMP_SMASTER  1
#define HEADPHONE_AMP_MAX      1

/* jack status */
#define JACK_STATUS_NONE    0
#define JACK_STATUS_3PIN    1
#define JACK_STATUS_5PIN    2
#define JACK_STATUS_ANTENNA 3
#define JACK_STATUS_MAX     3

/* master volume */
#define MASTER_VOLUME_MIN  0
#ifdef ICX_ENABLE_VOL60STP
#define MASTER_VOLUME_MAX 60
#else
#define MASTER_VOLUME_MAX 30
#endif

/* master gain */
#define MASTER_GAIN_MIN  0
#define MASTER_GAIN_MAX 30

/* base noise cancel gain index */
#define BASE_NOISE_CANCEL_GAIN_INDEX_MAX 50

/* user noise cancel gain index */
#define USER_NOISE_CANCEL_GAIN_INDEX_MAX 30

/* external OSC */
#define EXTERNAL_OSC_OFF  0
#define EXTERNAL_OSC_KEEP 1
#define EXTERNAL_OSC_441  2
#define EXTERNAL_OSC_480  3

/* private structure */
struct cxd3774gf_driver_data {
	struct i2c_client * i2c;
	struct snd_soc_codec * codec;
	struct mutex mutex;
	struct wake_lock wake_lock;
};

/* status struction */
struct cxd3774gf_status{
	int noise_cancel_mode;
	int noise_cancel_active;

	int sound_effect;

	int playback_mute;
	int capture_mute;
	int master_volume;
	int master_gain;

	int analog_playback_mute;
	int analog_stream_mute;
	int icx_playback_active;
	int std_playback_active;
	int capture_active;

	int mix_timed_mute;
	int std_timed_mute;
	int icx_timed_mute;

	int uncg_mute;

	int output_device;
	int input_device;
	int headphone_amp;
	int headphone_type;
	int jack_mode;
	int jack_status;

	int pcm_monitoring;

	unsigned int sample_rate;
};

/**********/
/* macros */
/**********/

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

#define print_info(fmt, args...)    printk(KERN_INFO    "CXD3774GF: " fmt,                 ##args)
#define print_warning(fmt, args...) printk(KERN_WARNING "%s(): "      fmt,   __FUNCTION__, ##args)
#define print_error(fmt, args...)   printk(KERN_ERR     "%s(): "      fmt,   __FUNCTION__, ##args)
#define print_fail(fmt, args...)    printk(KERN_ERR                   fmt,                 ##args)
#define back_trace()                printk(KERN_ERR     "%s(): [%d]\n", __FUNCTION__, __LINE__)

/* basic macro */
#define minimum(_a,_b) ( (_a)<(_b) ? (_a) : (_b) )
#define maximum(_a,_b) ( (_a)>(_b) ? (_a) : (_b) )
#define absolute(_a)   ( (_a)>=0 ? (_a) : (-(_a)) )

/***********/
/* headers */
/***********/

#include "cxd3774gf_platform.h"
#include "cxd3774gf_register.h"
#include "cxd3774gf_volume.h"
#include "cxd3774gf_table.h"
#include "cxd3774gf_dnc.h"
#include "cxd3774gf_switch.h"
#include "cxd3774gf_control.h"
#include "cxd3774gf_timer.h"
#include "cxd3774gf_interrupt.h"
#include "cxd3774gf_regmon.h"

#endif
