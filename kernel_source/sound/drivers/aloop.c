/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 *  Loopback soundcard
 *
 *  Original code:
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *
 *  More accurate positioning and full-duplex support:
 *  Copyright (c) Ahmet İnan <ainan at mathematik.uni-freiburg.de>
 *
 *  Major (almost complete) rewrite:
 *  Copyright (c) by Takashi Iwai <tiwai@suse.de>
 *
 *  A next major update in 2010 (separate timers for playback and capture):
 *  Copyright (c) Jaroslav Kysela <perex@perex.cz>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/info.h>
#include <sound/initval.h>

MODULE_AUTHOR("Jaroslav Kysela <perex@perex.cz>");
MODULE_DESCRIPTION("A loopback soundcard");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{ALSA,Loopback soundcard}}");

#define MAX_PCM_SUBSTREAMS	8

static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;	/* Index 0-MAX */
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;	/* ID for this card */
static bool enable[SNDRV_CARDS] = {1, [1 ... (SNDRV_CARDS - 1)] = 0};
static int pcm_substreams[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 8};
static int pcm_notify[SNDRV_CARDS];

module_param_array(index, int, NULL, 0444);
MODULE_PARM_DESC(index, "Index value for loopback soundcard.");
module_param_array(id, charp, NULL, 0444);
MODULE_PARM_DESC(id, "ID string for loopback soundcard.");
module_param_array(enable, bool, NULL, 0444);
MODULE_PARM_DESC(enable, "Enable this loopback soundcard.");
module_param_array(pcm_substreams, int, NULL, 0444);
MODULE_PARM_DESC(pcm_substreams, "PCM substreams # (1-8) for loopback driver.");
module_param_array(pcm_notify, int, NULL, 0444);
MODULE_PARM_DESC(pcm_notify, "Break capture when PCM format/rate/channels changes.");

#define NO_PITCH 100000

#ifdef CONFIG_ARCH_MT8590_ICX
#include <linux/vmalloc.h>
// #define ENABLE_ICX_DEBUG
#ifdef ENABLE_ICX_DEBUG
#define ICX_DEBUG_PRINTK_INTERNAL(fmt, ...) printk("ICX_DEBUG:%d: " fmt "%s", __LINE__, ##__VA_ARGS__)
#define ICX_DEBUG_PRINTK(...) \
	ICX_DEBUG_PRINTK_INTERNAL(__VA_ARGS__, "")
#else
#define ICX_DEBUG_PRINTK(...)
#endif
#endif

// #define PAS_DELAY_CHECK
#ifdef PAS_DELAY_CHECK
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_core.h>
#include <mach/mt_gpio_base.h>
#define TIMING_GPIO (GPIO115 | 0x80000000)
#endif

struct loopback_pcm;

struct loopback_cable {
	spinlock_t lock;
	struct loopback_pcm *streams[2];
	struct snd_pcm_hardware hw;
	/* flags */
	unsigned int valid;
	unsigned int running;
	unsigned int pause;
};

struct loopback_setup {
	unsigned int notify: 1;
	unsigned int rate_shift;
	unsigned int format;
	unsigned int rate;
	unsigned int channels;
	struct snd_ctl_elem_id active_id;
	struct snd_ctl_elem_id format_id;
	struct snd_ctl_elem_id rate_id;
	struct snd_ctl_elem_id channels_id;
};

#ifdef CONFIG_ARCH_MT8590_ICX
#define PLAYBACK_BUF_BYTES (64 << 10)  /* 64kbyte */
#endif

#ifdef CONFIG_SND_ENABLE_ICX_DMA_DRIVE_MODE
#define ICX_USE_HRTIMER
#define ICX_ENABLE_DMA_DRIVE_MODE
#endif

// #define ENABLE_DEBUG_DMA_DRIVE_MODE
#ifdef ENABLE_DEBUG_DMA_DRIVE_MODE
#define ICX_DMA_DRIVE_MODE_PRINTK(...) \
	ICX_DMA_DRIVE_MODE_PRINTK_PRIVATE(__VA_ARGS__, "")
#define ICX_DMA_DRIVE_MODE_PRINTK_PRIVATE(fmt, ...) printk("ICX_DEBUG:%d: " fmt "%s", __LINE__, ##__VA_ARGS__)
#else
#define ICX_DMA_DRIVE_MODE_PRINTK(...)
#endif

#ifdef ICX_ENABLE_DMA_DRIVE_MODE
static void period_elapsed_notification_set(struct loopback_pcm *dpcm);
static void period_elapsed_notification_clear(struct loopback_pcm *dpcm);
struct mt8590_period_elapsed_notification_listener {
	void (*pcm_start_triggered)(void* data);
	void (*period_elapsed)(void* data);
	void (*period_stopped)(void* data);
	void* userData;
};
void mt8590_period_elapsed_notification_set_listener(
	struct mt8590_period_elapsed_notification_listener* listener);
void mt8590_period_elapsed_notification_set_listener_raw(
	struct mt8590_period_elapsed_notification_listener* listener);
struct period_elapsed_notification {
	struct mt8590_period_elapsed_notification_listener listener;
	bool dmaDriveMode;
	bool pcmStartTriggered;
	bool firstDmaElapsed;
	ktime_t periodStartTime;
	s64 blockNs;
	s32 blockBytes;
	s32 blockNum;
	s32 timerTriggerNum;
	struct hrtimer timer;
	/* pid of the caller of mt8590_period_elapsed_notification_listener */
	/* This value is -1 except for during calling snd_pcm_period_elapsed() from listener callback */
	pid_t dmaDriveContext;
};
static struct period_elapsed_notification s_period_elapsed_notification = {};
#endif

struct loopback {
	struct snd_card *card;
	struct mutex cable_lock;
	struct loopback_cable *cables[MAX_PCM_SUBSTREAMS][2];
	struct snd_pcm *pcm[2];
	struct loopback_setup setup[MAX_PCM_SUBSTREAMS][2];
#ifdef CONFIG_ARCH_MT8590_ICX
	wait_queue_head_t capture_close_wq;              /* capture device closing wait */
	struct snd_ctl_elem_id playback_active_kctl_id; /* Playback START/STOP notification control */
	bool playback_active;                           /* Playback active flag */
	bool capture_close_waiting;                     /* Capture device closing wait flag */
	bool capture_delay_not_treated;                 /* Capture delay flag */
	void* playback_buf;                             /* Capture delay buffer */
	unsigned long playback_buf_pos;                /* Capture delay buffer pos */
#endif
#ifdef ICX_ENABLE_DMA_DRIVE_MODE
	struct snd_ctl_elem_id playback_delay_kctl_id;  /* Playback delay notification control */
	int playback_delay_usec;                        /* Playback delay */
#endif
};

struct loopback_pcm {
	struct loopback *loopback;
	struct snd_pcm_substream *substream;
	struct loopback_cable *cable;
	unsigned int pcm_buffer_size;
	unsigned int buf_pos;	/* position in buffer */
	unsigned int silent_size;
	/* PCM parameters */
	unsigned int pcm_period_size;
	unsigned int pcm_bps;		/* bytes per second */
	unsigned int pcm_salign;	/* bytes per sample * channels */
	unsigned int pcm_rate_shift;	/* rate shift value */
	/* flags */
	unsigned int period_update_pending :1;
	/* timer stuff */
#ifdef ICX_USE_HRTIMER
	s64 irq_pos;		/* fractional IRQ position */
	s64 period_size_frac;
	unsigned int last_drift;
	ktime_t last_time;
	struct hrtimer timer;
#else
	unsigned int irq_pos;		/* fractional IRQ position */
	unsigned int period_size_frac;
	unsigned int last_drift;
	unsigned long last_jiffies;
	struct timer_list timer;
#endif
};

static struct platform_device *devices[SNDRV_CARDS];

#ifdef ICX_USE_HRTIMER
#define ONE_SECOND_TIMERRES (1000000000)
#define MINIMUM_TIME_NS (100000)
static ktime_t s_playbackStartTime = {};
static inline s64 slow_mod64(s64 divided, s64 divisor) {
	while (divided > 0 && divided >= divisor) {
		divided -= divisor;
	}
	return divided;
}
static inline s64 byte_pos(struct loopback_pcm *dpcm, s64 x)
{
	u32 rem;
	if (dpcm->pcm_rate_shift == NO_PITCH) {
		x = div_u64(x, ONE_SECOND_TIMERRES);
	} else {
		x = div_u64(NO_PITCH * (unsigned long long)x,
			    ONE_SECOND_TIMERRES * (unsigned long long)dpcm->pcm_rate_shift);
	}
	div_u64_rem(x, dpcm->pcm_salign, &rem);
	return x - rem;
}

static inline s64 frac_pos(struct loopback_pcm *dpcm, s64 x)
{
	if (dpcm->pcm_rate_shift == NO_PITCH) {	/* no pitch */
		return x * ONE_SECOND_TIMERRES;
	} else {
		x = div_u64(dpcm->pcm_rate_shift * (unsigned long long)x * ONE_SECOND_TIMERRES,
			    NO_PITCH);
	}
	return x;
}
#else
#define ONE_SECOND_TIMERRES HZ
static inline unsigned int byte_pos(struct loopback_pcm *dpcm, unsigned int x)
{
	if (dpcm->pcm_rate_shift == NO_PITCH) {
		x /= HZ;
	} else {
		x = div_u64(NO_PITCH * (unsigned long long)x,
			    HZ * (unsigned long long)dpcm->pcm_rate_shift);
	}
	return x - (x % dpcm->pcm_salign);
}

static inline unsigned int frac_pos(struct loopback_pcm *dpcm, unsigned int x)
{
	if (dpcm->pcm_rate_shift == NO_PITCH) {	/* no pitch */
		return x * HZ;
	} else {
		x = div_u64(dpcm->pcm_rate_shift * (unsigned long long)x * HZ,
			    NO_PITCH);
	}
	return x;
}
#endif

static inline struct loopback_setup *get_setup(struct loopback_pcm *dpcm)
{
	int device = dpcm->substream->pstr->pcm->device;
	
	if (dpcm->substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		device ^= 1;
	return &dpcm->loopback->setup[dpcm->substream->number][device];
}

static inline unsigned int get_notify(struct loopback_pcm *dpcm)
{
	return get_setup(dpcm)->notify;
}

static inline unsigned int get_rate_shift(struct loopback_pcm *dpcm)
{
	return get_setup(dpcm)->rate_shift;
}

#ifdef ICX_USE_HRTIMER
/* call in cable->lock */
static void loopback_timer_start(struct loopback_pcm *dpcm)
{
	s64 expire;
	unsigned int rate_shift = get_rate_shift(dpcm);

	if (rate_shift != dpcm->pcm_rate_shift) {
		dpcm->pcm_rate_shift = rate_shift;
		dpcm->period_size_frac = frac_pos(dpcm, dpcm->pcm_period_size);
	}
	if (dpcm->period_size_frac <= dpcm->irq_pos) {
		dpcm->irq_pos = slow_mod64(dpcm->irq_pos, dpcm->period_size_frac);
		dpcm->period_update_pending = 1;
	}
	expire = dpcm->period_size_frac - dpcm->irq_pos;
	expire = div_u64(expire + dpcm->pcm_bps - 1, dpcm->pcm_bps);
	if (expire > 1000000) {
		expire += MINIMUM_TIME_NS;
	}
	ICX_DEBUG_PRINTK("loopback_timer_start@%lld %d %lld\n", ktime_to_us(ktime_get()), dpcm->substream->stream, div_u64(expire, 1000));
	hrtimer_start(&dpcm->timer, ktime_set(0, expire), HRTIMER_MODE_REL);
}

static inline void loopback_timer_stop(struct loopback_pcm *dpcm)
{
	hrtimer_try_to_cancel(&dpcm->timer);
}
#else
/* call in cable->lock */
static void loopback_timer_start(struct loopback_pcm *dpcm)
{
	unsigned long tick;
	unsigned int rate_shift = get_rate_shift(dpcm);

	if (rate_shift != dpcm->pcm_rate_shift) {
		dpcm->pcm_rate_shift = rate_shift;
		dpcm->period_size_frac = frac_pos(dpcm, dpcm->pcm_period_size);
	}
	if (dpcm->period_size_frac <= dpcm->irq_pos) {
		dpcm->irq_pos %= dpcm->period_size_frac;
		dpcm->period_update_pending = 1;
	}
	tick = dpcm->period_size_frac - dpcm->irq_pos;
	tick = (tick + dpcm->pcm_bps - 1) / dpcm->pcm_bps;
	dpcm->timer.expires = jiffies + tick;
	add_timer(&dpcm->timer);
}

/* call in cable->lock */
static inline void loopback_timer_stop(struct loopback_pcm *dpcm)
{
	del_timer(&dpcm->timer);
	dpcm->timer.expires = 0;
}
#endif

#define CABLE_VALID_PLAYBACK	(1 << SNDRV_PCM_STREAM_PLAYBACK)
#define CABLE_VALID_CAPTURE	(1 << SNDRV_PCM_STREAM_CAPTURE)
#define CABLE_VALID_BOTH	(CABLE_VALID_PLAYBACK|CABLE_VALID_CAPTURE)

static int loopback_check_format(struct loopback_cable *cable, int stream)
{
	struct snd_pcm_runtime *runtime, *cruntime;
	struct loopback_setup *setup;
	struct snd_card *card;
	int check;

	if (cable->valid != CABLE_VALID_BOTH) {
		if (stream == SNDRV_PCM_STREAM_PLAYBACK)
			goto __notify;
		return 0;
	}
	runtime = cable->streams[SNDRV_PCM_STREAM_PLAYBACK]->
							substream->runtime;
	cruntime = cable->streams[SNDRV_PCM_STREAM_CAPTURE]->
							substream->runtime;
	check = runtime->format != cruntime->format ||
		runtime->rate != cruntime->rate ||
		runtime->channels != cruntime->channels;
	if (!check)
		return 0;
	if (stream == SNDRV_PCM_STREAM_CAPTURE) {
		return -EIO;
	} else {
		snd_pcm_stop(cable->streams[SNDRV_PCM_STREAM_CAPTURE]->
					substream, SNDRV_PCM_STATE_DRAINING);
	      __notify:
		runtime = cable->streams[SNDRV_PCM_STREAM_PLAYBACK]->
							substream->runtime;
		setup = get_setup(cable->streams[SNDRV_PCM_STREAM_PLAYBACK]);
		card = cable->streams[SNDRV_PCM_STREAM_PLAYBACK]->loopback->card;
		if (setup->format != runtime->format) {
			snd_ctl_notify(card, SNDRV_CTL_EVENT_MASK_VALUE,
							&setup->format_id);
			setup->format = runtime->format;
		}
		if (setup->rate != runtime->rate) {
			snd_ctl_notify(card, SNDRV_CTL_EVENT_MASK_VALUE,
							&setup->rate_id);
			setup->rate = runtime->rate;
		}
		if (setup->channels != runtime->channels) {
			snd_ctl_notify(card, SNDRV_CTL_EVENT_MASK_VALUE,
							&setup->channels_id);
			setup->channels = runtime->channels;
		}
	}
	return 0;
}

static void loopback_active_notify(struct loopback_pcm *dpcm)
{
#ifdef CONFIG_ARCH_MT8590_ICX
	/* Playback active flag changed! */
	struct loopback_cable* cable = dpcm->loopback->cables[dpcm->substream->number][SNDRV_PCM_STREAM_PLAYBACK];
	dpcm->loopback->playback_active = (cable->running & (1 << SNDRV_PCM_STREAM_PLAYBACK));
	snd_ctl_notify(dpcm->loopback->card,
		       SNDRV_CTL_EVENT_MASK_VALUE,
		       &dpcm->loopback->playback_active_kctl_id);
#endif
	snd_ctl_notify(dpcm->loopback->card,
		       SNDRV_CTL_EVENT_MASK_VALUE,
		       &get_setup(dpcm)->active_id);
}

#ifdef CONFIG_ARCH_MT8590_ICX
static struct snd_pcm_hardware loopback_pcm_hardware =
{
	.info =		(SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP |
			 SNDRV_PCM_INFO_MMAP_VALID | SNDRV_PCM_INFO_PAUSE |
			 SNDRV_PCM_INFO_RESUME),
	.formats =		SNDRV_PCM_FMTBIT_S32_LE,
#ifndef CONFIG_SND_ALOOP_RATE_48K_SERIES_ONLY_MODE
	.rates =		SNDRV_PCM_RATE_44100 |
			 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000,
	.rate_min =		44100,
#else
	.rates =	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000,
	.rate_min =		48000,
#endif
	.rate_max =		96000,
	.channels_min =		2,
	.channels_max =		2,
	.buffer_bytes_max =	2 * 1024 * 1024,
	.period_bytes_min =	64,
	/* note check overflow in frac_pos() using pcm_rate_shift before
	   changing period_bytes_max value */
	.period_bytes_max =	1024 * 1024,
	.periods_min =		1,
	.periods_max =		1024,
	.fifo_size =		0,
};
#endif

static int loopback_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct loopback_pcm *dpcm = runtime->private_data;
	struct loopback_cable *cable = dpcm->cable;
	int err, stream = 1 << substream->stream;
#ifdef ICX_USE_HRTIMER
	unsigned long flags;
#endif
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		err = loopback_check_format(cable, substream->stream);
		if (err < 0)
			return err;
#ifdef ICX_USE_HRTIMER
		ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ SNDRV_PCM_TRIGGER_START: stream=%d\n", substream->stream);
		dpcm->last_time = ktime_get();
#else
		dpcm->last_jiffies = jiffies;
#endif
		dpcm->pcm_rate_shift = 0;
		dpcm->last_drift = 0;
#ifdef ICX_USE_HRTIMER
		spin_lock_irqsave(&cable->lock, flags);
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			s_playbackStartTime = ktime_get();
		}
#else
		spin_lock(&cable->lock);
#endif	
		cable->running |= stream;
		cable->pause &= ~stream;
		loopback_timer_start(dpcm);
#ifdef ICX_USE_HRTIMER
		spin_unlock_irqrestore(&cable->lock, flags);
#else
		spin_unlock(&cable->lock);
#endif
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			loopback_active_notify(dpcm);
#ifdef ICX_ENABLE_DMA_DRIVE_MODE
			s_period_elapsed_notification.firstDmaElapsed = false;
		} else {
			period_elapsed_notification_set(dpcm);
		}
#else
		}
#endif

#ifdef PAS_DELAY_CHECK
		mt_set_gpio_out(TIMING_GPIO, substream->stream == SNDRV_PCM_STREAM_PLAYBACK);
#endif

#ifdef CONFIG_ARCH_MT8590_ICX
#ifndef ICX_USE_HRTIMER
		ICX_DEBUG_PRINTK("[jiffies:%lu] SNDRV_PCM_TRIGGER_START %s\n", dpcm->last_jiffies, substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "playback" : "capture");
#endif
#endif
		break;
	case SNDRV_PCM_TRIGGER_STOP:
#ifdef ICX_USE_HRTIMER
		spin_lock_irqsave(&cable->lock, flags);
#else
		spin_lock(&cable->lock);
#endif
		cable->running &= ~stream;
		cable->pause &= ~stream;
#ifndef ICX_USE_HRTIMER
		loopback_timer_stop(dpcm);
#endif
#ifdef CONFIG_ARCH_MT8590_ICX
		/* Transit to capture device close wait state */
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			dpcm->loopback->capture_close_waiting = true;
		}
#endif
#ifdef ICX_USE_HRTIMER
		ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ SNDRV_PCM_TRIGGER_STOP: stream=%d\n", substream->stream);
		spin_unlock_irqrestore(&cable->lock, flags);
#else
		spin_unlock(&cable->lock);
#endif
#ifdef ICX_USE_HRTIMER
#ifdef ICX_ENABLE_DMA_DRIVE_MODE
		period_elapsed_notification_clear(dpcm);
#endif
		loopback_timer_stop(dpcm);
#endif
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			loopback_active_notify(dpcm);
#ifdef CONFIG_ARCH_MT8590_ICX
			/* reset hw_params to default */
			runtime->hw = loopback_pcm_hardware;
			ICX_DEBUG_PRINTK("SNDRV_PCM_TRIGGER_STOP %s hw:%p\n", substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "playback" : "capture", (void*)&runtime->hw);
#endif
		}
#ifdef CONFIG_ARCH_MT8590_ICX
#ifndef ICX_USE_HRTIMER
		ICX_DEBUG_PRINTK("[jiffies:%lu] SNDRV_PCM_TRIGGER_STOP %s\n", dpcm->last_jiffies, substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "playback" : "capture");
#endif
#endif
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	case SNDRV_PCM_TRIGGER_SUSPEND:
#ifdef ICX_USE_HRTIMER
		ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ SNDRV_PCM_TRIGGER_PAUSE_PUSH/SNDRV_PCM_TRIGGER_SUSPEND: stream=%d\n", substream->stream);
		spin_lock_irqsave(&cable->lock, flags);
#else
		spin_lock(&cable->lock);
#endif
		cable->pause |= stream;
#ifdef ICX_USE_HRTIMER
		spin_unlock_irqrestore(&cable->lock, flags);
#ifdef ICX_ENABLE_DMA_DRIVE_MODE
		period_elapsed_notification_clear(dpcm);
#endif
		loopback_timer_stop(dpcm);
#else
		loopback_timer_stop(dpcm);
		spin_unlock(&cable->lock);
#endif
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
	case SNDRV_PCM_TRIGGER_RESUME:
#ifdef ICX_USE_HRTIMER
		ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ SNDRV_PCM_TRIGGER_PAUSE_RELEASE/SNDRV_PCM_TRIGGER_RESUME: stream=%d\n", substream->stream);
		spin_lock_irqsave(&cable->lock, flags);
		dpcm->last_time = ktime_get();
#else
		spin_lock(&cable->lock);
		dpcm->last_jiffies = jiffies;
#endif
		cable->pause &= ~stream;
		loopback_timer_start(dpcm);
#ifdef ICX_USE_HRTIMER
		spin_unlock_irqrestore(&cable->lock, flags);
#ifdef ICX_ENABLE_DMA_DRIVE_MODE
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
			period_elapsed_notification_set(dpcm);
		}
#endif
#else
		spin_unlock(&cable->lock);
#endif
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void params_change_substream(struct loopback_pcm *dpcm,
				    struct snd_pcm_runtime *runtime)
{
	struct snd_pcm_runtime *dst_runtime;

	if (dpcm == NULL || dpcm->substream == NULL)
		return;
	dst_runtime = dpcm->substream->runtime;
	if (dst_runtime == NULL)
		return;
	dst_runtime->hw = dpcm->cable->hw;
}

static void params_change(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct loopback_pcm *dpcm = runtime->private_data;
	struct loopback_cable *cable = dpcm->cable;

	cable->hw.formats = pcm_format_to_bits(runtime->format);
	cable->hw.rate_min = runtime->rate;
	cable->hw.rate_max = runtime->rate;
	cable->hw.channels_min = runtime->channels;
	cable->hw.channels_max = runtime->channels;
	params_change_substream(cable->streams[SNDRV_PCM_STREAM_PLAYBACK],
				runtime);
	params_change_substream(cable->streams[SNDRV_PCM_STREAM_CAPTURE],
				runtime);
}

#ifdef CONFIG_ARCH_MT8590_ICX
#define CAPTURE_CLOSE_TIMEOUT_JIFFIES (HZ/5)	// 200ms
static bool loopback_is_need_to_wait_capture_close(
		struct loopback* loopback, struct snd_pcm_substream *playback) {
	bool needToWait;
	struct loopback_cable *cable;
	needToWait = false;
	mutex_lock(&loopback->cable_lock);
	cable = loopback->cables[playback->number][!playback->pcm->device ? playback->stream : !playback->stream];
	if (cable) {
		unsigned long flags;
		spin_lock_irqsave(&cable->lock, flags);
		if (!cable->streams[SNDRV_PCM_STREAM_CAPTURE]) {
			loopback->capture_close_waiting = false;
		}
		needToWait = loopback->capture_close_waiting;
		spin_unlock_irqrestore(&cable->lock, flags);
	}
	mutex_unlock(&loopback->cable_lock);
	return needToWait;
}
#endif

static int loopback_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct loopback_pcm *dpcm = runtime->private_data;
	struct loopback_cable *cable = dpcm->cable;
	int bps, salign;

#ifdef CONFIG_ARCH_MT8590_ICX
	/* Wait capture device closing */
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		ICX_DEBUG_PRINTK("loopback_prepare wait++\n");
		wait_event_interruptible_timeout(
			((struct loopback*)substream->private_data)->capture_close_wq,
			!loopback_is_need_to_wait_capture_close(((struct loopback*)substream->private_data), substream),
			CAPTURE_CLOSE_TIMEOUT_JIFFIES);
		ICX_DEBUG_PRINTK("loopback_prepare wait--\n");
		/* Initialize capture delay pos */
		dpcm->loopback->playback_buf_pos = 0;
	} else {
	/* Set capture delay flag to 'not treated' */
		dpcm->loopback->capture_delay_not_treated = true;
	}
#endif

	salign = (snd_pcm_format_width(runtime->format) *
						runtime->channels) / 8;
	bps = salign * runtime->rate;
	if (bps <= 0 || salign <= 0)
		return -EINVAL;

	dpcm->buf_pos = 0;
	dpcm->pcm_buffer_size = frames_to_bytes(runtime, runtime->buffer_size);
	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		/* clear capture buffer */
		dpcm->silent_size = dpcm->pcm_buffer_size;
		snd_pcm_format_set_silence(runtime->format, runtime->dma_area,
					   runtime->buffer_size * runtime->channels);
	}

	dpcm->irq_pos = 0;
	dpcm->period_update_pending = 0;
	dpcm->pcm_bps = bps;
	dpcm->pcm_salign = salign;
	dpcm->pcm_period_size = frames_to_bytes(runtime, runtime->period_size);

	mutex_lock(&dpcm->loopback->cable_lock);
	if (!(cable->valid & ~(1 << substream->stream)) ||
            (get_setup(dpcm)->notify &&
	     substream->stream == SNDRV_PCM_STREAM_PLAYBACK))
		params_change(substream);
	cable->valid |= 1 << substream->stream;
	mutex_unlock(&dpcm->loopback->cable_lock);

	return 0;
}

static void clear_capture_buf(struct loopback_pcm *dpcm, unsigned int bytes)
{
	struct snd_pcm_runtime *runtime = dpcm->substream->runtime;
	char *dst = runtime->dma_area;
	unsigned int dst_off = dpcm->buf_pos;

	if (dpcm->silent_size >= dpcm->pcm_buffer_size)
		return;
	if (dpcm->silent_size + bytes > dpcm->pcm_buffer_size)
		bytes = dpcm->pcm_buffer_size - dpcm->silent_size;

	for (;;) {
		unsigned int size = bytes;
		if (dst_off + size > dpcm->pcm_buffer_size)
			size = dpcm->pcm_buffer_size - dst_off;
		snd_pcm_format_set_silence(runtime->format, dst + dst_off,
					   bytes_to_frames(runtime, size) *
					   	runtime->channels);
		dpcm->silent_size += size;
		bytes -= size;
		if (!bytes)
			break;
		dst_off = 0;
	}
}

static void copy_play_buf(struct loopback_pcm *play,
			  struct loopback_pcm *capt,
			  unsigned int bytes)
{
	struct snd_pcm_runtime *runtime = play->substream->runtime;
	char *src = runtime->dma_area;
	char *dst = capt->substream->runtime->dma_area;
	unsigned int src_off = play->buf_pos;
	unsigned int dst_off = capt->buf_pos;
	unsigned int clear_bytes = 0;

	/* check if playback is draining, trim the capture copy size
	 * when our pointer is at the end of playback ring buffer */
	if (runtime->status->state == SNDRV_PCM_STATE_DRAINING &&
	    snd_pcm_playback_hw_avail(runtime) < runtime->buffer_size) { 
	    	snd_pcm_uframes_t appl_ptr, appl_ptr1, diff;
		appl_ptr = appl_ptr1 = runtime->control->appl_ptr;
		appl_ptr1 -= appl_ptr1 % runtime->buffer_size;
		appl_ptr1 += play->buf_pos / play->pcm_salign;
		if (appl_ptr < appl_ptr1)
			appl_ptr1 -= runtime->buffer_size;
		diff = (appl_ptr - appl_ptr1) * play->pcm_salign;
		if (diff < bytes) {
			clear_bytes = bytes - diff;
			bytes = diff;
		}
	}

	for (;;) {
		unsigned int size = bytes;
		if (src_off + size > play->pcm_buffer_size)
			size = play->pcm_buffer_size - src_off;
		if (dst_off + size > capt->pcm_buffer_size)
			size = capt->pcm_buffer_size - dst_off;
		memcpy(dst + dst_off, src + src_off, size);
		capt->silent_size = 0;
		bytes -= size;
		if (!bytes)
			break;
		src_off = (src_off + size) % play->pcm_buffer_size;
		dst_off = (dst_off + size) % capt->pcm_buffer_size;
	}

	if (clear_bytes > 0) {
		clear_capture_buf(capt, clear_bytes);
		capt->silent_size = 0;
	}
}

#ifdef ICX_USE_HRTIMER
static inline unsigned int last_byte_pos(struct loopback_pcm *dpcm) {
	return dpcm->buf_pos % dpcm->pcm_period_size;
}
static inline unsigned int bytepos_delta_internal(struct loopback_pcm *dpcm,
					 s64 hrtime_delta, unsigned long last_pos)
{
	int delta;
	unsigned long curr_pos;
	dpcm->irq_pos += hrtime_delta * dpcm->pcm_bps;
	curr_pos = byte_pos(dpcm, dpcm->irq_pos);
	delta = curr_pos - last_pos;
	if (curr_pos < last_pos) {
		ICX_DEBUG_PRINTK("+++++ stream=%d curr_irq_pos=%lld curr_pos=%ld last_pos=%ld\n", dpcm->substream->stream, dpcm->irq_pos, curr_pos, last_pos);
		delta += dpcm->pcm_period_size;
	}
	if (dpcm->irq_pos >= dpcm->period_size_frac) {
		dpcm->irq_pos = slow_mod64(dpcm->irq_pos, dpcm->period_size_frac);
		dpcm->period_update_pending = 1;
	}
	return delta;
}
static inline unsigned int bytepos_delta(struct loopback_pcm *dpcm,
					 s64 hrtime_delta)
{
	return bytepos_delta_internal(dpcm, hrtime_delta, last_byte_pos(dpcm));
}
#else
static inline unsigned int bytepos_delta(struct loopback_pcm *dpcm,
					 unsigned int jiffies_delta)
{
	unsigned long last_pos;
	unsigned int delta;

	last_pos = byte_pos(dpcm, dpcm->irq_pos);
	dpcm->irq_pos += jiffies_delta * dpcm->pcm_bps;
	delta = byte_pos(dpcm, dpcm->irq_pos) - last_pos;
	if (delta >= dpcm->last_drift)
		delta -= dpcm->last_drift;
	dpcm->last_drift = 0;
	if (dpcm->irq_pos >= dpcm->period_size_frac) {
		dpcm->irq_pos %= dpcm->period_size_frac;
		dpcm->period_update_pending = 1;
	}
	return delta;
}
#endif

static inline void bytepos_finish(struct loopback_pcm *dpcm,
				  unsigned int delta)
{
	dpcm->buf_pos += delta;
	dpcm->buf_pos %= dpcm->pcm_buffer_size;
}

/* call in cable->lock */
static unsigned int loopback_pos_update(struct loopback_cable *cable)
{
	struct loopback_pcm *dpcm_play =
			cable->streams[SNDRV_PCM_STREAM_PLAYBACK];
	struct loopback_pcm *dpcm_capt =
			cable->streams[SNDRV_PCM_STREAM_CAPTURE];
#ifdef ICX_USE_HRTIMER
	ktime_t now = ktime_get();
	s64 delta_play = 0, delta_capt = 0;
#else
	unsigned long delta_play = 0, delta_capt = 0;
#endif
	unsigned int running, count1, count2;
#ifdef CONFIG_ARCH_MT8590_ICX
	unsigned int playback_buf_end_pos;
	unsigned int this_timing_delay_bytes;
	unsigned int capture_delay_max_bytes;
#endif

	running = cable->running ^ cable->pause;
	if (running & (1 << SNDRV_PCM_STREAM_PLAYBACK)) {
#ifdef ICX_USE_HRTIMER
		delta_play = ktime_to_ns(ktime_sub(now, dpcm_play->last_time));
		dpcm_play->last_time = now;
#else
		delta_play = jiffies - dpcm_play->last_jiffies;
		dpcm_play->last_jiffies += delta_play;
#endif
	}

	if (running & (1 << SNDRV_PCM_STREAM_CAPTURE)) {
#ifdef ICX_USE_HRTIMER
		delta_capt = ktime_to_ns(ktime_sub(now, dpcm_capt->last_time));
		dpcm_capt->last_time = now;
#else
		delta_capt = jiffies - dpcm_capt->last_jiffies;
		dpcm_capt->last_jiffies += delta_capt;
#endif
#ifdef CONFIG_ARCH_MT8590_ICX
		/* Capture delay treatment */
		if (dpcm_capt->loopback->capture_delay_not_treated) {
			dpcm_capt->loopback->capture_delay_not_treated = false;
			if (running & (1 << SNDRV_PCM_STREAM_PLAYBACK)) {
				ICX_DEBUG_PRINTK("[jiffies:%lu] Capture delay: playback_buf_pos=%lu(%u)[byte](%lld[jiffies]) delta_play,delta_capt=%lu,%lu\n",
					jiffies,
					dpcm_play->loopback->playback_buf_pos,
					PLAYBACK_BUF_BYTES,
					div_u64(dpcm_play->loopback->playback_buf_pos * ONE_SECOND_TIMERRES, dpcm_play->pcm_bps),
					delta_play,
					delta_capt);
				if (dpcm_capt->buf_pos == 0
						&& delta_play >= delta_capt) {
#ifndef ICX_USE_HRTIMER
					this_timing_delay_bytes = bytepos_delta(dpcm_capt, delta_play - delta_capt);
#else
					this_timing_delay_bytes = bytepos_delta_internal(dpcm_capt, delta_play - delta_capt, byte_pos(dpcm_capt, dpcm_capt->irq_pos));
#endif
					capture_delay_max_bytes = CONFIG_SND_ALOOP_AUDIO_OUT_BUFFER_PERIOD_NUM * dpcm_capt->pcm_period_size;
					if (capture_delay_max_bytes > dpcm_capt->pcm_buffer_size - dpcm_capt->pcm_period_size) {
						capture_delay_max_bytes = dpcm_capt->pcm_buffer_size - dpcm_capt->pcm_period_size;
					}
					if (this_timing_delay_bytes < capture_delay_max_bytes
							&& this_timing_delay_bytes < dpcm_play->pcm_buffer_size - dpcm_play->buf_pos) {
						if (dpcm_play->loopback->playback_buf_pos > 0) {
							dpcm_capt->buf_pos = dpcm_play->loopback->playback_buf_pos > PLAYBACK_BUF_BYTES ?
									PLAYBACK_BUF_BYTES : dpcm_play->loopback->playback_buf_pos;
							if (dpcm_capt->buf_pos > capture_delay_max_bytes - this_timing_delay_bytes) {
								dpcm_capt->buf_pos = capture_delay_max_bytes - this_timing_delay_bytes;
							}
							playback_buf_end_pos = dpcm_play->loopback->playback_buf_pos % PLAYBACK_BUF_BYTES;
							if (dpcm_capt->buf_pos <= playback_buf_end_pos) {
								memcpy(
									dpcm_capt->substream->runtime->dma_area,
									dpcm_play->loopback->playback_buf + playback_buf_end_pos - dpcm_capt->buf_pos,
									dpcm_capt->buf_pos);
								ICX_DEBUG_PRINTK("[jiffies:%lu] capt->dma_area <- %u bytes %u@playback_buf.\n",
										jiffies, dpcm_capt->buf_pos, playback_buf_end_pos - dpcm_capt->buf_pos);
							} else {
								memcpy(
									dpcm_capt->substream->runtime->dma_area,
									dpcm_play->loopback->playback_buf + PLAYBACK_BUF_BYTES - dpcm_capt->buf_pos + playback_buf_end_pos,
									dpcm_capt->buf_pos - playback_buf_end_pos);
								memcpy(
									dpcm_capt->substream->runtime->dma_area + dpcm_capt->buf_pos - playback_buf_end_pos,
									dpcm_play->loopback->playback_buf,
									playback_buf_end_pos);
								ICX_DEBUG_PRINTK("[jiffies:%lu] capt->dma_area <- %u bytes %u@playback_buf, %u bytes 0@playback_buf.\n",
										jiffies, dpcm_capt->buf_pos - playback_buf_end_pos, PLAYBACK_BUF_BYTES - dpcm_capt->buf_pos + playback_buf_end_pos,
										playback_buf_end_pos);
							}
#ifndef ICX_USE_HRTIMER
							bytepos_delta(dpcm_capt, div_u64((s64)dpcm_capt->buf_pos * ONE_SECOND_TIMERRES, dpcm_play->pcm_bps));
#else
							bytepos_delta_internal(dpcm_capt, div_u64((s64)dpcm_capt->buf_pos * ONE_SECOND_TIMERRES, dpcm_play->pcm_bps), byte_pos(dpcm_capt, dpcm_capt->irq_pos));
#endif
						}
						if (this_timing_delay_bytes > 0) {
							memcpy(
								dpcm_capt->substream->runtime->dma_area + dpcm_capt->buf_pos,
								dpcm_play->substream->runtime->dma_area + dpcm_play->buf_pos,
								this_timing_delay_bytes);
							ICX_DEBUG_PRINTK("[jiffies:%lu] capt->dma_area@%u <- %u bytes %u@play->dma_area.\n",
									jiffies, dpcm_capt->buf_pos, this_timing_delay_bytes, dpcm_play->buf_pos);
							dpcm_capt->buf_pos += this_timing_delay_bytes;
						}
						ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ loopback_pos_update: Capture delay %d frames\n", dpcm_capt->buf_pos / dpcm_capt->pcm_salign);
					} else {
						dpcm_capt->irq_pos = 0;
						dpcm_capt->period_update_pending = 0;
						ICX_DEBUG_PRINTK("[jiffies:%lu] Delay is too big(%u bytes).\n",
								jiffies, this_timing_delay_bytes);
					}
				}
			}
			dpcm_play->loopback->playback_buf_pos = 0;
		}
#endif
	}

	if (delta_play == 0 && delta_capt == 0)
		goto unlock;
		
	if (delta_play > delta_capt) {
		count1 = bytepos_delta(dpcm_play, delta_play - delta_capt);
#ifdef CONFIG_ARCH_MT8590_ICX
		if (count1 > 0
				&& delta_capt == 0
				&& !(running & (1 << SNDRV_PCM_STREAM_CAPTURE))
				&& dpcm_play->buf_pos + count1 < dpcm_play->pcm_buffer_size) {
			playback_buf_end_pos = dpcm_play->loopback->playback_buf_pos % PLAYBACK_BUF_BYTES;
			if (playback_buf_end_pos + count1 > PLAYBACK_BUF_BYTES) {
				if (count1 > PLAYBACK_BUF_BYTES) {
					playback_buf_end_pos = (dpcm_play->loopback->playback_buf_pos + count1 - PLAYBACK_BUF_BYTES) % PLAYBACK_BUF_BYTES;
					memcpy(
						dpcm_play->loopback->playback_buf + playback_buf_end_pos,
						dpcm_play->substream->runtime->dma_area + dpcm_play->buf_pos + count1 - PLAYBACK_BUF_BYTES,
						PLAYBACK_BUF_BYTES - playback_buf_end_pos);
					memcpy(
						dpcm_play->loopback->playback_buf,
						dpcm_play->substream->runtime->dma_area + dpcm_play->buf_pos + count1 - playback_buf_end_pos,
						playback_buf_end_pos);
					ICX_DEBUG_PRINTK("[jiffies:%lu] cpos=%lu %u(%u) bytes: %u bytes %u@ -> %u@playback_buf, %u bytes %u@ -> 0@playback_buf(%u).\n",
							jiffies,
							dpcm_play->loopback->playback_buf_pos,
							count1,	PLAYBACK_BUF_BYTES,
							PLAYBACK_BUF_BYTES - playback_buf_end_pos, dpcm_play->buf_pos + count1 - PLAYBACK_BUF_BYTES, playback_buf_end_pos,
							playback_buf_end_pos, dpcm_play->buf_pos + count1 - playback_buf_end_pos, PLAYBACK_BUF_BYTES);
				} else {
					memcpy(
						dpcm_play->loopback->playback_buf + playback_buf_end_pos,
						dpcm_play->substream->runtime->dma_area + dpcm_play->buf_pos,
						PLAYBACK_BUF_BYTES - playback_buf_end_pos);
					memcpy(
						dpcm_play->loopback->playback_buf,
						dpcm_play->substream->runtime->dma_area + dpcm_play->buf_pos + PLAYBACK_BUF_BYTES - playback_buf_end_pos,
						playback_buf_end_pos + count1 - PLAYBACK_BUF_BYTES);
					ICX_DEBUG_PRINTK("[jiffies:%lu] cpos=%lu %u bytes: %u bytes %u@ -> %u@playback_buf, %u bytes %u@ -> 0@playback_buf(%u).\n",
							jiffies,
							dpcm_play->loopback->playback_buf_pos,
							count1,
							PLAYBACK_BUF_BYTES - playback_buf_end_pos, dpcm_play->buf_pos, playback_buf_end_pos,
							playback_buf_end_pos + count1 - PLAYBACK_BUF_BYTES, dpcm_play->buf_pos + PLAYBACK_BUF_BYTES - playback_buf_end_pos, PLAYBACK_BUF_BYTES);
				}
			} else {
				memcpy(
					dpcm_play->loopback->playback_buf + playback_buf_end_pos,
					dpcm_play->substream->runtime->dma_area + dpcm_play->buf_pos,
					count1);
				ICX_DEBUG_PRINTK("[jiffies:%lu] %u bytes %u@ -> %u@playback_buf(%u).\n",
						jiffies, count1, dpcm_play->buf_pos,
						playback_buf_end_pos, PLAYBACK_BUF_BYTES);
			}
			dpcm_play->loopback->playback_buf_pos += count1;
		}
#endif
		bytepos_finish(dpcm_play, count1);
		delta_play = delta_capt;
	} else if (delta_play < delta_capt) {
		count1 = bytepos_delta(dpcm_capt, delta_capt - delta_play);
		clear_capture_buf(dpcm_capt, count1);
		bytepos_finish(dpcm_capt, count1);
		delta_capt = delta_play;
	}

	if (delta_play == 0 && delta_capt == 0)
		goto unlock;

	/* note delta_capt == delta_play at this moment */
	count1 = bytepos_delta(dpcm_play, delta_play);
	count2 = bytepos_delta(dpcm_capt, delta_capt);
#ifndef ICX_USE_HRTIMER
	if (count1 < count2) {
		dpcm_capt->last_drift = count2 - count1;
		count1 = count2;
	} else if (count1 > count2) {
		dpcm_play->last_drift = count1 - count2;
	}
#else
	if (count1 < count2) {
		ICX_DEBUG_PRINTK("@@@@@ c1,c2=%d,%d capt_irq_pos=%lld capt_curr_pos=%lld capt_last_pos=%d\n", count1, count2, delta_capt, byte_pos(dpcm_capt, dpcm_capt->irq_pos), last_byte_pos(dpcm_capt));
		count2 = count1;
	} else if (count1 > count2) {
		ICX_DEBUG_PRINTK("@@@@@ c1,c2=%d,%d capt_irq_pos=%lld capt_curr_pos=%lld capt_last_pos=%d\n", count1, count2, delta_capt, byte_pos(dpcm_capt, dpcm_capt->irq_pos), last_byte_pos(dpcm_capt));
		count1 = count2;
	}
#endif
	copy_play_buf(dpcm_play, dpcm_capt, count1);
	bytepos_finish(dpcm_play, count1);
	bytepos_finish(dpcm_capt, count1);
 unlock:
	return running;
}

#ifdef ICX_USE_HRTIMER
static enum hrtimer_restart loopback_timer_function(struct hrtimer *timer)
{
	struct loopback_pcm *dpcm = container_of(timer, struct loopback_pcm, timer);
	unsigned long flags;
	s64 expire;
	enum hrtimer_restart res = HRTIMER_NORESTART;
	spin_lock_irqsave(&dpcm->cable->lock, flags);
#ifndef ICX_ENABLE_DMA_DRIVE_MODE
	if (loopback_pos_update(dpcm->cable) & (1 << dpcm->substream->stream)) {
#else
	if (s_period_elapsed_notification.dmaDriveMode == false &&
		(loopback_pos_update(dpcm->cable) & (1 << dpcm->substream->stream))) {
#endif
		res = HRTIMER_RESTART;
		expire = dpcm->period_size_frac - dpcm->irq_pos;
		expire = div_u64(expire + dpcm->pcm_bps - 1, dpcm->pcm_bps);
		if (expire > 1000000) {
			expire += MINIMUM_TIME_NS;  // timer fires a little early
		}
		ICX_DEBUG_PRINTK("loopback_timer_function@%lld %d %lld update=%d\n", ktime_to_us(ktime_get()), dpcm->substream->stream, div_u64(expire, 1000), dpcm->period_update_pending);
		hrtimer_forward_now(&dpcm->timer, ktime_set(0, expire));
		if (dpcm->period_update_pending) {
			dpcm->period_update_pending = 0;
			spin_unlock_irqrestore(&dpcm->cable->lock, flags);
			/* need to unlock before calling below */
			snd_pcm_period_elapsed(dpcm->substream);
			return res;
		}
	}
	spin_unlock_irqrestore(&dpcm->cable->lock, flags);
	return res;
}
#else
static void loopback_timer_function(unsigned long data)
{
	struct loopback_pcm *dpcm = (struct loopback_pcm *)data;
	unsigned long flags;

	spin_lock_irqsave(&dpcm->cable->lock, flags);
	if (loopback_pos_update(dpcm->cable) & (1 << dpcm->substream->stream)) {
		loopback_timer_start(dpcm);
		if (dpcm->period_update_pending) {
			dpcm->period_update_pending = 0;
			spin_unlock_irqrestore(&dpcm->cable->lock, flags);
			/* need to unlock before calling below */
			snd_pcm_period_elapsed(dpcm->substream);
			return;
		}
	}
	spin_unlock_irqrestore(&dpcm->cable->lock, flags);
}
#endif

#ifdef ICX_ENABLE_DMA_DRIVE_MODE
#ifdef PAS_DELAY_CHECK
static bool s_gpio_state = false;
#endif
#define DMA_SUPPLEMENT_TIMER_DELAY_NS (300000)
static void period_elapsed_notification_irq_pos_update(struct loopback_pcm *dpcm, int blockNum) {
	dpcm->irq_pos += frac_pos(dpcm, s_period_elapsed_notification.blockBytes * blockNum);
	if (dpcm->irq_pos >= dpcm->period_size_frac) {
		dpcm->irq_pos = slow_mod64(dpcm->irq_pos, dpcm->period_size_frac);
		dpcm->period_update_pending = 1;
	}
}
static void period_elapsed_notification_pos_update(const ktime_t now, struct loopback_pcm *dpcm_capt, int blockNum) {
	struct loopback_pcm *dpcm_play;
	dpcm_play = dpcm_capt->cable->streams[SNDRV_PCM_STREAM_PLAYBACK];
	/* last_time */
	dpcm_capt->last_time = now;
	dpcm_play->last_time = now;
	/* irq_pos */
	period_elapsed_notification_irq_pos_update(dpcm_capt, blockNum);
	period_elapsed_notification_irq_pos_update(dpcm_play, blockNum);
	/* copy */
	copy_play_buf(dpcm_play, dpcm_capt, s_period_elapsed_notification.blockBytes);
	/* byte_pos */
	bytepos_finish(dpcm_capt, s_period_elapsed_notification.blockBytes);
	bytepos_finish(dpcm_play, s_period_elapsed_notification.blockBytes);
}
static enum hrtimer_restart period_elapsed_notification_timer_function(struct hrtimer *timer) {
	struct loopback_pcm *dpcm = (struct loopback_pcm*)s_period_elapsed_notification.listener.userData;
	struct loopback_pcm *dpcm_capt = dpcm;
	struct loopback_pcm *dpcm_play;
	unsigned long flags;
	unsigned int running;
	int currBlock;
	int consumedBlockNum = 1;
	s64 calCurrNs;
	s64 currNs;
	s64 delayNs;
	s64 expire;
	ktime_t now = ktime_get();
	enum hrtimer_restart res = HRTIMER_NORESTART;
	spin_lock_irqsave(&dpcm->cable->lock, flags);
	dpcm_play = dpcm_capt->cable->streams[SNDRV_PCM_STREAM_PLAYBACK];
	if (s_period_elapsed_notification.dmaDriveMode) {
		running = dpcm_capt->cable->running ^ dpcm_capt->cable->pause;
		if ((running & (1 << SNDRV_PCM_STREAM_PLAYBACK)) && (running & (1 << SNDRV_PCM_STREAM_CAPTURE))) {
#ifdef PAS_DELAY_CHECK
			s_gpio_state = !s_gpio_state;
			mt_set_gpio_out(TIMING_GPIO, s_gpio_state);
#endif
			expire = s_period_elapsed_notification.blockNs;
			if (s_period_elapsed_notification.timerTriggerNum > 1) {
				currBlock = s_period_elapsed_notification.blockNum - s_period_elapsed_notification.timerTriggerNum;
				calCurrNs = currBlock * s_period_elapsed_notification.blockNs;
				currNs = ktime_to_ns(ktime_sub(now, s_period_elapsed_notification.periodStartTime));
				delayNs = currNs - calCurrNs;
				if (delayNs > DMA_SUPPLEMENT_TIMER_DELAY_NS) {
					consumedBlockNum = s_period_elapsed_notification.timerTriggerNum;
					ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ period_elapsed_notification_timer_function: timerTrigger dropped %d blocks %d@%d t%lld(r%lld). Wait for dma handler...\n",
						consumedBlockNum, currBlock, s_period_elapsed_notification.blockNum, calCurrNs, currNs);
				} else {
					expire -= delayNs;
				}
			}
			s_period_elapsed_notification.timerTriggerNum -= consumedBlockNum;
			if (s_period_elapsed_notification.timerTriggerNum > 0) {
				res = HRTIMER_RESTART;
				hrtimer_forward_now(&s_period_elapsed_notification.timer, ktime_set(0, expire));
			}
			period_elapsed_notification_pos_update(now, dpcm_capt, consumedBlockNum);
			if (dpcm_capt->period_update_pending) {
				dpcm_capt->period_update_pending = 0;
			} else {
				dpcm_capt = 0;
			}
			if (dpcm_play->period_update_pending) {
				dpcm_play->period_update_pending = 0;
			} else {
				dpcm_play = 0;
			}
			spin_unlock_irqrestore(&dpcm->cable->lock, flags);
			if (dpcm_capt) snd_pcm_period_elapsed(dpcm_capt->substream);
			if (dpcm_play) snd_pcm_period_elapsed(dpcm_play->substream);
			return res;
		}
	}
	spin_unlock_irqrestore(&dpcm->cable->lock, flags);
	return res;
}
static void period_elapsed_notification_dma_started(struct loopback_pcm *dpcm_capt) {
	s64 playback_delay_usec;
	if (s_period_elapsed_notification.firstDmaElapsed) return;
	s_period_elapsed_notification.firstDmaElapsed = true;
	if (s_period_elapsed_notification.pcmStartTriggered) {
		playback_delay_usec = div_u64(frac_pos(dpcm_capt, dpcm_capt->pcm_period_size), dpcm_capt->pcm_bps * 1000);
		playback_delay_usec = ktime_to_us(ktime_sub(ktime_get(), s_playbackStartTime)) - playback_delay_usec;
	} else {
		playback_delay_usec = div_u64(frac_pos(dpcm_capt, dpcm_capt->pcm_period_size), dpcm_capt->pcm_bps * 1000)
			* CONFIG_SND_ALOOP_AUDIO_OUT_BUFFER_PERIOD_NUM;
	}
	playback_delay_usec += CONFIG_SND_ALOOP_AUDIO_OUT_DELAY_US;
	dpcm_capt->loopback->playback_delay_usec = (int)playback_delay_usec;
	snd_ctl_notify(dpcm_capt->loopback->card,
		       SNDRV_CTL_EVENT_MASK_VALUE,
		       &dpcm_capt->loopback->playback_delay_kctl_id);
}
static void period_elapsed_notification_triggered(void* data) {
	unsigned long flags;
	unsigned int running;
	s64 minimum_playback_delay_usec;
	struct loopback_pcm *dpcm_capt = (struct loopback_pcm *)data;
	ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ period_elapsed_notification_triggered:\n");
	spin_lock_irqsave(&dpcm_capt->cable->lock, flags);
	running = dpcm_capt->cable->running ^ dpcm_capt->cable->pause;
	if ((running & (1 << SNDRV_PCM_STREAM_PLAYBACK)) && ktime_to_us(s_playbackStartTime) != 0) {
		minimum_playback_delay_usec = div_u64(frac_pos(dpcm_capt, dpcm_capt->pcm_period_size), dpcm_capt->pcm_bps * 1000) * (CONFIG_SND_ALOOP_AUDIO_OUT_BUFFER_PERIOD_NUM - 1);
		if (ktime_to_us(ktime_sub(ktime_get(), s_playbackStartTime)) > minimum_playback_delay_usec) {
#ifdef PAS_DELAY_CHECK
			s_gpio_state = true;
			mt_set_gpio_out(TIMING_GPIO, s_gpio_state);
#endif
			s_period_elapsed_notification.pcmStartTriggered = true;
		}
	}
	spin_unlock_irqrestore(&dpcm_capt->cable->lock, flags);
}
static void period_elapsed_notification_elapsed(void* data) {
	struct loopback_pcm *dpcm = (struct loopback_pcm *)data;
	struct loopback_pcm *dpcm_capt = dpcm;
	struct loopback_pcm *dpcm_play;
	unsigned int running;
	unsigned long flags;
	bool dmaDriveModeEnabled;
	bool firstDmaElapsed;
	spin_lock_irqsave(&dpcm_capt->cable->lock, flags);
	running = dpcm_capt->cable->running ^ dpcm_capt->cable->pause;
	dpcm_play = dpcm_capt->cable->streams[SNDRV_PCM_STREAM_PLAYBACK];
	dmaDriveModeEnabled = (running & (1 << SNDRV_PCM_STREAM_PLAYBACK)) && (running & (1 << SNDRV_PCM_STREAM_CAPTURE));
	if (dmaDriveModeEnabled) {
		if (!s_period_elapsed_notification.dmaDriveMode && !s_period_elapsed_notification.pcmStartTriggered && ktime_to_us(s_playbackStartTime) != 0) {
			dmaDriveModeEnabled = ktime_to_us(ktime_sub(ktime_get(), s_playbackStartTime)) >
				div_u64(frac_pos(dpcm_capt, dpcm_capt->pcm_period_size), dpcm_capt->pcm_bps * 1000) * (CONFIG_SND_ALOOP_AUDIO_OUT_BUFFER_PERIOD_NUM + 4);
		}
	}
	if (dmaDriveModeEnabled) {
#ifdef PAS_DELAY_CHECK
		s_gpio_state = !s_gpio_state;
		mt_set_gpio_out(TIMING_GPIO, s_gpio_state);
#endif
		s_period_elapsed_notification.periodStartTime = ktime_get();
		firstDmaElapsed = !s_period_elapsed_notification.dmaDriveMode;
		if (!s_period_elapsed_notification.dmaDriveMode) {
			s_period_elapsed_notification.dmaDriveMode = true;
			loopback_timer_stop(dpcm_play);
			loopback_timer_stop(dpcm_capt);
			loopback_pos_update(dpcm_capt->cable);
			s_period_elapsed_notification.blockBytes = dpcm_play->pcm_period_size;
			s_period_elapsed_notification.blockNum = dpcm_capt->pcm_period_size / dpcm_play->pcm_period_size;
			s_period_elapsed_notification.blockNs = div_u64(frac_pos(dpcm_play, s_period_elapsed_notification.blockBytes), dpcm_play->pcm_bps);
			ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ period_elapsed_notification_elapsed: transit into dmaDriveMode! period=%d block=%d blockNum=%d blockNs=%lld\n",
				dpcm_capt->pcm_period_size, s_period_elapsed_notification.blockBytes, s_period_elapsed_notification.blockNum, s_period_elapsed_notification.blockNs);
		} else {
			period_elapsed_notification_pos_update(ktime_get(), dpcm_capt, 1);
		}
		s_period_elapsed_notification.timerTriggerNum = s_period_elapsed_notification.blockNum - 1;
		if (s_period_elapsed_notification.timerTriggerNum > 0) {
			if (hrtimer_try_to_cancel(&s_period_elapsed_notification.timer) == 0) {
				hrtimer_start(&s_period_elapsed_notification.timer, ktime_set(0, s_period_elapsed_notification.blockNs), HRTIMER_MODE_REL);
			}
		}
		if (dpcm_capt->period_update_pending) {
			dpcm_capt->period_update_pending = 0;
		} else {
			dpcm_capt = 0;
		}
		if (dpcm_play->period_update_pending) {
			dpcm_play->period_update_pending = 0;
		} else {
			dpcm_play = 0;
		}
		s_period_elapsed_notification.dmaDriveContext = current->pid;
		spin_unlock_irqrestore(&dpcm->cable->lock, flags);
		if (dpcm_capt) snd_pcm_period_elapsed(dpcm_capt->substream);
		if (dpcm_play) snd_pcm_period_elapsed(dpcm_play->substream);
		if (firstDmaElapsed) period_elapsed_notification_dma_started(dpcm);
		spin_lock_irqsave(&dpcm->cable->lock, flags);
		s_period_elapsed_notification.dmaDriveContext = -1;
		spin_unlock_irqrestore(&dpcm->cable->lock, flags);
		return;
	}
	spin_unlock_irqrestore(&dpcm_capt->cable->lock, flags);
}
static void period_elapsed_notification_stopped(void* data) {
	struct loopback_pcm *dpcm = (struct loopback_pcm *)data;
	struct loopback_pcm *dpcm_capt = dpcm;
	struct loopback_pcm *dpcm_play;
	unsigned int running;
	unsigned long flags;
	spin_lock_irqsave(&dpcm->cable->lock, flags);
	running = dpcm_capt->cable->running ^ dpcm_capt->cable->pause;
	dpcm_play = dpcm_capt->cable->streams[SNDRV_PCM_STREAM_PLAYBACK];
	if (s_period_elapsed_notification.dmaDriveMode) {
		ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ period_elapsed_notification_stopped:\n");
		s_period_elapsed_notification.dmaDriveMode = false;
		hrtimer_try_to_cancel(&s_period_elapsed_notification.timer);
		loopback_pos_update(dpcm_capt->cable);
		if ((running & (1 << SNDRV_PCM_STREAM_PLAYBACK))) loopback_timer_start(dpcm_play);
		if ((running & (1 << SNDRV_PCM_STREAM_CAPTURE))) loopback_timer_start(dpcm_capt);
		if ((running & (1 << SNDRV_PCM_STREAM_PLAYBACK)) && (running & (1 << SNDRV_PCM_STREAM_CAPTURE))) {
			if (dpcm_capt->period_update_pending) {
				dpcm_capt->period_update_pending = 0;
			} else {
				dpcm_capt = 0;
			}
			if (dpcm_play->period_update_pending) {
				dpcm_play->period_update_pending = 0;
			} else {
				dpcm_play = 0;
			}
			s_period_elapsed_notification.dmaDriveContext = current->pid;
			spin_unlock_irqrestore(&dpcm->cable->lock, flags);
			if (dpcm_capt) snd_pcm_period_elapsed(dpcm_capt->substream);
			if (dpcm_play) snd_pcm_period_elapsed(dpcm_play->substream);
			spin_lock_irqsave(&dpcm->cable->lock, flags);
			s_period_elapsed_notification.dmaDriveContext = -1;
			spin_unlock_irqrestore(&dpcm->cable->lock, flags);
			return;
		}
	}
	spin_unlock_irqrestore(&dpcm->cable->lock, flags);
}
static void period_elapsed_notification_set(struct loopback_pcm *dpcm_capt) {
	unsigned int running;
	unsigned long flags;
	s64 max_capture_start_delay_usec;
	ktime_t now = ktime_get();
	ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ period_elapsed_notification_set:\n");
	spin_lock_irqsave(&dpcm_capt->cable->lock, flags);
	running = dpcm_capt->cable->running ^ dpcm_capt->cable->pause;
	if ((running & (1 << SNDRV_PCM_STREAM_PLAYBACK)) == 0
		|| dpcm_capt->pcm_period_size < dpcm_capt->cable->streams[SNDRV_PCM_STREAM_PLAYBACK]->pcm_period_size
		|| (dpcm_capt->pcm_period_size % dpcm_capt->cable->streams[SNDRV_PCM_STREAM_PLAYBACK]->pcm_period_size) != 0) {
		spin_unlock_irqrestore(&dpcm_capt->cable->lock, flags);
		return;
	}
	spin_unlock_irqrestore(&dpcm_capt->cable->lock, flags);
	s_period_elapsed_notification.listener.userData = dpcm_capt;
	s_period_elapsed_notification.pcmStartTriggered = false;
	max_capture_start_delay_usec = div_u64(frac_pos(dpcm_capt, dpcm_capt->pcm_period_size), dpcm_capt->pcm_bps * 1000) * (CONFIG_SND_ALOOP_AUDIO_OUT_BUFFER_PERIOD_NUM - 1);
	if (!s_period_elapsed_notification.firstDmaElapsed
	  && ktime_to_us(ktime_sub(now, s_playbackStartTime)) > max_capture_start_delay_usec) {
		/* capture start is too late, reset the measurement start time to avoid capture start delay. */
		ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ period_elapsed_notification_set: capture start is too late, reset the measurement start time to avoid capture start delay.\n");
		s_playbackStartTime = now;
	}
	mt8590_period_elapsed_notification_set_listener(&s_period_elapsed_notification.listener);
}
static void period_elapsed_notification_clear(struct loopback_pcm *dpcm) {
	unsigned int running;
	unsigned long flags;
	bool isDmaDriveContext = false;
	ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ period_elapsed_notification_clear:\n");
	if (dpcm->substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		spin_lock_irqsave(&dpcm->cable->lock, flags);
		isDmaDriveContext = s_period_elapsed_notification.dmaDriveContext == current->pid;
		spin_unlock_irqrestore(&dpcm->cable->lock, flags);
		if (!isDmaDriveContext) {
			mt8590_period_elapsed_notification_set_listener(0);
		} else {
			ICX_DMA_DRIVE_MODE_PRINTK("@@@@@@@@@@@@@@@@@@@@@@@@@ period_elapsed_notification_clear: From dma listener callback!\n");
			mt8590_period_elapsed_notification_set_listener_raw(0);
		}
	}
	spin_lock_irqsave(&dpcm->cable->lock, flags);
	if (s_period_elapsed_notification.dmaDriveMode) {
		running = dpcm->cable->running ^ dpcm->cable->pause;
		if ((running & (1 << (!dpcm->substream->stream)))) {
			loopback_pos_update(dpcm->cable);
			loopback_timer_start(dpcm->cable->streams[!dpcm->substream->stream]);
		}
	}
	s_period_elapsed_notification.dmaDriveMode = false;
	hrtimer_try_to_cancel(&s_period_elapsed_notification.timer);
	spin_unlock_irqrestore(&dpcm->cable->lock, flags);
}
static void period_elapsed_notification_init(void) {
	s_period_elapsed_notification.listener.pcm_start_triggered = period_elapsed_notification_triggered;
	s_period_elapsed_notification.listener.period_elapsed = period_elapsed_notification_elapsed;
	s_period_elapsed_notification.listener.period_stopped = period_elapsed_notification_stopped;
	hrtimer_init(&s_period_elapsed_notification.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	s_period_elapsed_notification.timer.function = period_elapsed_notification_timer_function;
	s_period_elapsed_notification.dmaDriveContext = -1;
}
#endif

static snd_pcm_uframes_t loopback_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct loopback_pcm *dpcm = runtime->private_data;
	snd_pcm_uframes_t pos;

#ifdef ICX_USE_HRTIMER
	unsigned long flags;
	spin_lock_irqsave(&dpcm->cable->lock, flags);
#ifdef ICX_ENABLE_DMA_DRIVE_MODE
	if (s_period_elapsed_notification.dmaDriveMode) {
		pos = dpcm->buf_pos;
		spin_unlock_irqrestore(&dpcm->cable->lock, flags);
		return bytes_to_frames(runtime, pos);
	}
#endif
#else
	spin_lock(&dpcm->cable->lock);
#endif
	loopback_pos_update(dpcm->cable);
	pos = dpcm->buf_pos;
#ifdef ICX_USE_HRTIMER
	spin_unlock_irqrestore(&dpcm->cable->lock, flags);
#else
	spin_unlock(&dpcm->cable->lock);
#endif
	return bytes_to_frames(runtime, pos);
}

#ifndef CONFIG_ARCH_MT8590_ICX
static struct snd_pcm_hardware loopback_pcm_hardware =
{
	.info =		(SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP |
			 SNDRV_PCM_INFO_MMAP_VALID | SNDRV_PCM_INFO_PAUSE |
			 SNDRV_PCM_INFO_RESUME),
	.formats =	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S16_BE |
			 SNDRV_PCM_FMTBIT_FLOAT_LE | SNDRV_PCM_FMTBIT_FLOAT_BE),
	.rates =	SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000_48000,
	.rate_min =		8000,
	.rate_max =		48000,
	.channels_min =		1,
	.channels_max =		32,
	.buffer_bytes_max =	2 * 1024 * 1024,
	.period_bytes_min =	64,
	/* note check overflow in frac_pos() using pcm_rate_shift before
	   changing period_bytes_max value */
	.period_bytes_max =	1024 * 1024,
	.periods_min =		1,
	.periods_max =		1024,
	.fifo_size =		0,
};
#endif

static void loopback_runtime_free(struct snd_pcm_runtime *runtime)
{
	struct loopback_pcm *dpcm = runtime->private_data;
	kfree(dpcm);
}

static int loopback_hw_params(struct snd_pcm_substream *substream,
			      struct snd_pcm_hw_params *params)
{
	return snd_pcm_lib_alloc_vmalloc_buffer(substream,
						params_buffer_bytes(params));
}

static int loopback_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct loopback_pcm *dpcm = runtime->private_data;
	struct loopback_cable *cable = dpcm->cable;

	mutex_lock(&dpcm->loopback->cable_lock);
	cable->valid &= ~(1 << substream->stream);
	mutex_unlock(&dpcm->loopback->cable_lock);
	return snd_pcm_lib_free_vmalloc_buffer(substream);
}

static unsigned int get_cable_index(struct snd_pcm_substream *substream)
{
	if (!substream->pcm->device)
		return substream->stream;
	else
		return !substream->stream;
}

static int rule_format(struct snd_pcm_hw_params *params,
		       struct snd_pcm_hw_rule *rule)
{

	struct snd_pcm_hardware *hw = rule->private;
	struct snd_mask *maskp = hw_param_mask(params, rule->var);

	maskp->bits[0] &= (u_int32_t)hw->formats;
	maskp->bits[1] &= (u_int32_t)(hw->formats >> 32);
	memset(maskp->bits + 2, 0, (SNDRV_MASK_MAX-64) / 8); /* clear rest */
	if (! maskp->bits[0] && ! maskp->bits[1])
		return -EINVAL;
	return 0;
}

static int rule_rate(struct snd_pcm_hw_params *params,
		     struct snd_pcm_hw_rule *rule)
{
	struct snd_pcm_hardware *hw = rule->private;
	struct snd_interval t;

        t.min = hw->rate_min;
        t.max = hw->rate_max;
        t.openmin = t.openmax = 0;
        t.integer = 0;
	return snd_interval_refine(hw_param_interval(params, rule->var), &t);
}

static int rule_channels(struct snd_pcm_hw_params *params,
			 struct snd_pcm_hw_rule *rule)
{
	struct snd_pcm_hardware *hw = rule->private;
	struct snd_interval t;

        t.min = hw->channels_min;
        t.max = hw->channels_max;
        t.openmin = t.openmax = 0;
        t.integer = 0;
	return snd_interval_refine(hw_param_interval(params, rule->var), &t);
}

static int loopback_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct loopback *loopback = substream->private_data;
	struct loopback_pcm *dpcm;
	struct loopback_cable *cable;
	int err = 0;
	int dev = get_cable_index(substream);

#ifdef CONFIG_ARCH_MT8590_ICX
	/* Wait capture device closing */
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
#ifdef PAS_DELAY_CHECK
		mt_set_gpio_pull_select(TIMING_GPIO, GPIO_PULL_DOWN);
		mt_set_gpio_pull_enable(TIMING_GPIO, GPIO_PULL_ENABLE);
		mt_set_gpio_out(TIMING_GPIO, 0);
		mt_set_gpio_dir(TIMING_GPIO, GPIO_DIR_OUT);
		mt_set_gpio_mode(TIMING_GPIO, GPIO_MODE_GPIO);
#endif
		ICX_DEBUG_PRINTK("loopback_open wait++\n");
		wait_event_interruptible_timeout(
				loopback->capture_close_wq,
				!loopback_is_need_to_wait_capture_close(loopback, substream),
				CAPTURE_CLOSE_TIMEOUT_JIFFIES);
		ICX_DEBUG_PRINTK("loopback_open wait--\n");
	}
#endif
	mutex_lock(&loopback->cable_lock);
	dpcm = kzalloc(sizeof(*dpcm), GFP_KERNEL);
	if (!dpcm) {
		err = -ENOMEM;
		goto unlock;
	}
	dpcm->loopback = loopback;
	dpcm->substream = substream;
#ifdef ICX_USE_HRTIMER
	hrtimer_init(&dpcm->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dpcm->timer.function = loopback_timer_function;
#else
	setup_timer(&dpcm->timer, loopback_timer_function,
		    (unsigned long)dpcm);
#endif

	cable = loopback->cables[substream->number][dev];
	if (!cable) {
		cable = kzalloc(sizeof(*cable), GFP_KERNEL);
		if (!cable) {
			kfree(dpcm);
			err = -ENOMEM;
			goto unlock;
		}
		spin_lock_init(&cable->lock);
		cable->hw = loopback_pcm_hardware;
		loopback->cables[substream->number][dev] = cable;
	}
	dpcm->cable = cable;
	cable->streams[substream->stream] = dpcm;
#ifdef CONFIG_ARCH_MT8590_ICX
	/* Initialize hw_params */
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK
			&& cable->streams[!substream->stream] == NULL) {
		cable->hw = loopback_pcm_hardware;
	}
#endif

	snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);

	/* use dynamic rules based on actual runtime->hw values */
	/* note that the default rules created in the PCM midlevel code */
	/* are cached -> they do not reflect the actual state */
	err = snd_pcm_hw_rule_add(runtime, 0,
				  SNDRV_PCM_HW_PARAM_FORMAT,
				  rule_format, &runtime->hw,
				  SNDRV_PCM_HW_PARAM_FORMAT, -1);
	if (err < 0)
		goto unlock;
	err = snd_pcm_hw_rule_add(runtime, 0,
				  SNDRV_PCM_HW_PARAM_RATE,
				  rule_rate, &runtime->hw,
				  SNDRV_PCM_HW_PARAM_RATE, -1);
	if (err < 0)
		goto unlock;
	err = snd_pcm_hw_rule_add(runtime, 0,
				  SNDRV_PCM_HW_PARAM_CHANNELS,
				  rule_channels, &runtime->hw,
				  SNDRV_PCM_HW_PARAM_CHANNELS, -1);
	if (err < 0)
		goto unlock;

	runtime->private_data = dpcm;
	runtime->private_free = loopback_runtime_free;
	if (get_notify(dpcm))
		runtime->hw = loopback_pcm_hardware;
	else
		runtime->hw = cable->hw;

#ifdef CONFIG_ARCH_MT8590_ICX
	ICX_DEBUG_PRINTK(
	  "loopback_open %s other_stream=%s rate=%d-%d(hw:%p)\n",
	  substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "playback" : "capture",
	  cable->streams[!substream->stream] != NULL ? "active" : "inactive",
	  runtime->hw.rate_min,
	  runtime->hw.rate_max,
	  (void*)&runtime->hw);
#endif

 unlock:
	mutex_unlock(&loopback->cable_lock);
	return err;
}

static int loopback_close(struct snd_pcm_substream *substream)
{
	struct loopback *loopback = substream->private_data;
	struct loopback_pcm *dpcm = substream->runtime->private_data;
	struct loopback_cable *cable;
#ifdef CONFIG_ARCH_MT8590_ICX
	unsigned long flags;
#endif
	int dev = get_cable_index(substream);

	loopback_timer_stop(dpcm);
	mutex_lock(&loopback->cable_lock);
	cable = loopback->cables[substream->number][dev];
#ifdef CONFIG_ARCH_MT8590_ICX
	ICX_DEBUG_PRINTK("loopback_close %s other_stream=%s\n", substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "playback" : "capture", cable->streams[!substream->stream] != NULL ? "active" : "inactive");
	/* Wake up capture device closing waiter */
	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		spin_lock_irqsave(&cable->lock, flags);
		loopback->capture_close_waiting = false;
		spin_unlock_irqrestore(&cable->lock, flags);
		wake_up_interruptible(&loopback->capture_close_wq);
	} else if (cable->streams[!substream->stream] == NULL) {
	/* Free capture device closing flag */
		spin_lock_irqsave(&cable->lock, flags);
		loopback->capture_close_waiting = false;
		spin_unlock_irqrestore(&cable->lock, flags);
	}
#endif
	if (cable->streams[!substream->stream]) {
		/* other stream is still alive */
		cable->streams[substream->stream] = NULL;
	} else {
		/* free the cable */
		loopback->cables[substream->number][dev] = NULL;
		kfree(cable);
	}
	mutex_unlock(&loopback->cable_lock);
	return 0;
}

static struct snd_pcm_ops loopback_playback_ops = {
	.open =		loopback_open,
	.close =	loopback_close,
	.ioctl =	snd_pcm_lib_ioctl,
	.hw_params =	loopback_hw_params,
	.hw_free =	loopback_hw_free,
	.prepare =	loopback_prepare,
	.trigger =	loopback_trigger,
	.pointer =	loopback_pointer,
	.page =		snd_pcm_lib_get_vmalloc_page,
	.mmap =		snd_pcm_lib_mmap_vmalloc,
};

static struct snd_pcm_ops loopback_capture_ops = {
	.open =		loopback_open,
	.close =	loopback_close,
	.ioctl =	snd_pcm_lib_ioctl,
	.hw_params =	loopback_hw_params,
	.hw_free =	loopback_hw_free,
	.prepare =	loopback_prepare,
	.trigger =	loopback_trigger,
	.pointer =	loopback_pointer,
	.page =		snd_pcm_lib_get_vmalloc_page,
	.mmap =		snd_pcm_lib_mmap_vmalloc,
};

static int loopback_pcm_new(struct loopback *loopback,
			    int device, int substreams)
{
	struct snd_pcm *pcm;
	int err;

	err = snd_pcm_new(loopback->card, "Loopback PCM", device,
			  substreams, substreams, &pcm);
	if (err < 0)
		return err;
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &loopback_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &loopback_capture_ops);

	pcm->private_data = loopback;
	pcm->info_flags = 0;
	strcpy(pcm->name, "Loopback PCM");

	loopback->pcm[device] = pcm;
	return 0;
}

static int loopback_rate_shift_info(struct snd_kcontrol *kcontrol,   
				    struct snd_ctl_elem_info *uinfo) 
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 80000;
	uinfo->value.integer.max = 120000;
	uinfo->value.integer.step = 1;
	return 0;
}                                  

static int loopback_rate_shift_get(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct loopback *loopback = snd_kcontrol_chip(kcontrol);
	
	ucontrol->value.integer.value[0] =
		loopback->setup[kcontrol->id.subdevice]
			       [kcontrol->id.device].rate_shift;
	return 0;
}

static int loopback_rate_shift_put(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct loopback *loopback = snd_kcontrol_chip(kcontrol);
	unsigned int val;
	int change = 0;

	val = ucontrol->value.integer.value[0];
	if (val < 80000)
		val = 80000;
	if (val > 120000)
		val = 120000;	
	mutex_lock(&loopback->cable_lock);
	if (val != loopback->setup[kcontrol->id.subdevice]
				  [kcontrol->id.device].rate_shift) {
		loopback->setup[kcontrol->id.subdevice]
			       [kcontrol->id.device].rate_shift = val;
		change = 1;
	}
	mutex_unlock(&loopback->cable_lock);
	return change;
}

static int loopback_notify_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct loopback *loopback = snd_kcontrol_chip(kcontrol);
	
	ucontrol->value.integer.value[0] =
		loopback->setup[kcontrol->id.subdevice]
			       [kcontrol->id.device].notify;
	return 0;
}

static int loopback_notify_put(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct loopback *loopback = snd_kcontrol_chip(kcontrol);
	unsigned int val;
	int change = 0;

	val = ucontrol->value.integer.value[0] ? 1 : 0;
	if (val != loopback->setup[kcontrol->id.subdevice]
				[kcontrol->id.device].notify) {
		loopback->setup[kcontrol->id.subdevice]
			[kcontrol->id.device].notify = val;
		change = 1;
	}
	return change;
}

static int loopback_active_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct loopback *loopback = snd_kcontrol_chip(kcontrol);
	struct loopback_cable *cable = loopback->cables
			[kcontrol->id.subdevice][kcontrol->id.device ^ 1];
	unsigned int val = 0;

	if (cable != NULL)
		val = (cable->running & (1 << SNDRV_PCM_STREAM_PLAYBACK)) ?
									1 : 0;
	ucontrol->value.integer.value[0] = val;
	return 0;
}

static int loopback_format_info(struct snd_kcontrol *kcontrol,   
				struct snd_ctl_elem_info *uinfo) 
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = SNDRV_PCM_FORMAT_LAST;
	uinfo->value.integer.step = 1;
	return 0;
}                                  

static int loopback_format_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct loopback *loopback = snd_kcontrol_chip(kcontrol);
	
	ucontrol->value.integer.value[0] =
		loopback->setup[kcontrol->id.subdevice]
			       [kcontrol->id.device].format;
	return 0;
}

static int loopback_rate_info(struct snd_kcontrol *kcontrol,   
			      struct snd_ctl_elem_info *uinfo) 
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 192000;
	uinfo->value.integer.step = 1;
	return 0;
}                                  

static int loopback_rate_get(struct snd_kcontrol *kcontrol,
			     struct snd_ctl_elem_value *ucontrol)
{
	struct loopback *loopback = snd_kcontrol_chip(kcontrol);
	
	ucontrol->value.integer.value[0] =
		loopback->setup[kcontrol->id.subdevice]
			       [kcontrol->id.device].rate;
	return 0;
}

static int loopback_channels_info(struct snd_kcontrol *kcontrol,   
				  struct snd_ctl_elem_info *uinfo) 
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 1;
	uinfo->value.integer.max = 1024;
	uinfo->value.integer.step = 1;
	return 0;
}                                  

static int loopback_channels_get(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct loopback *loopback = snd_kcontrol_chip(kcontrol);
	
	ucontrol->value.integer.value[0] =
		loopback->setup[kcontrol->id.subdevice]
			       [kcontrol->id.device].channels;
	return 0;
}

static struct snd_kcontrol_new loopback_controls[]  = {
{
	.iface =        SNDRV_CTL_ELEM_IFACE_PCM,
	.name =         "PCM Rate Shift 100000",
	.info =         loopback_rate_shift_info,
	.get =          loopback_rate_shift_get,
	.put =          loopback_rate_shift_put,
},
{
	.iface =        SNDRV_CTL_ELEM_IFACE_PCM,
	.name =         "PCM Notify",
	.info =         snd_ctl_boolean_mono_info,
	.get =          loopback_notify_get,
	.put =          loopback_notify_put,
},
#define ACTIVE_IDX 2
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READ,
	.iface =        SNDRV_CTL_ELEM_IFACE_PCM,
	.name =         "PCM Slave Active",
	.info =         snd_ctl_boolean_mono_info,
	.get =          loopback_active_get,
},
#define FORMAT_IDX 3
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READ,
	.iface =        SNDRV_CTL_ELEM_IFACE_PCM,
	.name =         "PCM Slave Format",
	.info =         loopback_format_info,
	.get =          loopback_format_get
},
#define RATE_IDX 4
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READ,
	.iface =        SNDRV_CTL_ELEM_IFACE_PCM,
	.name =         "PCM Slave Rate",
	.info =         loopback_rate_info,
	.get =          loopback_rate_get
},
#define CHANNELS_IDX 5
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READ,
	.iface =        SNDRV_CTL_ELEM_IFACE_PCM,
	.name =         "PCM Slave Channels",
	.info =         loopback_channels_info,
	.get =          loopback_channels_get
}
};

#ifdef CONFIG_ARCH_MT8590_ICX
/* Playback active flag api */
static int loopback_playback_active_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct loopback *loopback = snd_kcontrol_chip(kcontrol);
	ucontrol->value.integer.value[0] = loopback->playback_active;
	return 0;
}

static struct snd_kcontrol_new playback_active_ctl =
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READ,
	.iface =        SNDRV_CTL_ELEM_IFACE_PCM,
	.name =         "PCM Slave Active2",
	.info =         snd_ctl_boolean_mono_info,
	.get =          loopback_playback_active_get,
};
#endif

#ifdef ICX_ENABLE_DMA_DRIVE_MODE
/* Playback delay api */
static int loopback_playback_delay_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct loopback *loopback = snd_kcontrol_chip(kcontrol);
	ucontrol->value.integer.value[0] = loopback->playback_delay_usec;
	return 0;
}

static int loopback_playback_delay_info(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = INT_MAX;
	uinfo->value.integer.step = 1;
	return 0;
}

static struct snd_kcontrol_new playback_delay_ctl =
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READ,
	.iface =        SNDRV_CTL_ELEM_IFACE_PCM,
	.name =         "Playback delay usec",
	.info =         loopback_playback_delay_info,
	.get =          loopback_playback_delay_get,
};
#endif

static int loopback_mixer_new(struct loopback *loopback, int notify)
{
	struct snd_card *card = loopback->card;
	struct snd_pcm *pcm;
	struct snd_kcontrol *kctl;
	struct loopback_setup *setup;
	int err, dev, substr, substr_count, idx;

	strcpy(card->mixername, "Loopback Mixer");
#ifdef CONFIG_ARCH_MT8590_ICX
	/* Create Playback active flag mixer control */
	kctl = snd_ctl_new1(&playback_active_ctl, loopback);
	if (!kctl) return -ENOMEM;
	err = snd_ctl_add(card, kctl);
	if (err < 0) return err;
	loopback->playback_active_kctl_id = kctl->id;
#endif
#ifdef ICX_ENABLE_DMA_DRIVE_MODE
	/* Create Playback delay mixer control */
	kctl = snd_ctl_new1(&playback_delay_ctl, loopback);
	if (!kctl) return -ENOMEM;
	err = snd_ctl_add(card, kctl);
	if (err < 0) return err;
	loopback->playback_delay_kctl_id = kctl->id;
#endif
	for (dev = 0; dev < 2; dev++) {
		pcm = loopback->pcm[dev];
		substr_count =
		    pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream_count;
		for (substr = 0; substr < substr_count; substr++) {
			setup = &loopback->setup[substr][dev];
			setup->notify = notify;
			setup->rate_shift = NO_PITCH;
			setup->format = SNDRV_PCM_FORMAT_S16_LE;
			setup->rate = 48000;
			setup->channels = 2;
			for (idx = 0; idx < ARRAY_SIZE(loopback_controls);
									idx++) {
				kctl = snd_ctl_new1(&loopback_controls[idx],
						    loopback);
				if (!kctl)
					return -ENOMEM;
				kctl->id.device = dev;
				kctl->id.subdevice = substr;
				switch (idx) {
				case ACTIVE_IDX:
					setup->active_id = kctl->id;
					break;
				case FORMAT_IDX:
					setup->format_id = kctl->id;
					break;
				case RATE_IDX:
					setup->rate_id = kctl->id;
					break;
				case CHANNELS_IDX:
					setup->channels_id = kctl->id;
					break;
				default:
					break;
				}
				err = snd_ctl_add(card, kctl);
				if (err < 0)
					return err;
			}
		}
	}
	return 0;
}

#ifdef CONFIG_PROC_FS

static void print_dpcm_info(struct snd_info_buffer *buffer,
			    struct loopback_pcm *dpcm,
			    const char *id)
{
	snd_iprintf(buffer, "  %s\n", id);
	if (dpcm == NULL) {
		snd_iprintf(buffer, "    inactive\n");
		return;
	}
	snd_iprintf(buffer, "    buffer_size:\t%u\n", dpcm->pcm_buffer_size);
	snd_iprintf(buffer, "    buffer_pos:\t\t%u\n", dpcm->buf_pos);
	snd_iprintf(buffer, "    silent_size:\t%u\n", dpcm->silent_size);
	snd_iprintf(buffer, "    period_size:\t%u\n", dpcm->pcm_period_size);
	snd_iprintf(buffer, "    bytes_per_sec:\t%u\n", dpcm->pcm_bps);
	snd_iprintf(buffer, "    sample_align:\t%u\n", dpcm->pcm_salign);
	snd_iprintf(buffer, "    rate_shift:\t\t%u\n", dpcm->pcm_rate_shift);
	snd_iprintf(buffer, "    update_pending:\t%u\n",
						dpcm->period_update_pending);
#ifdef ICX_USE_HRTIMER
	snd_iprintf(buffer, "    irq_pos:\t\t%lld\n", dpcm->irq_pos);
	snd_iprintf(buffer, "    period_frac:\t%lld\n", dpcm->period_size_frac);
	snd_iprintf(buffer, "    last_jiffies:\t%lld (%lld)\n",
					ktime_to_ns(dpcm->last_time), ktime_to_ns(ktime_get()));
	snd_iprintf(buffer, "    timer_expires:\t%lld\n", hrtimer_get_expires_ns(&dpcm->timer));
#else
	snd_iprintf(buffer, "    irq_pos:\t\t%u\n", dpcm->irq_pos);
	snd_iprintf(buffer, "    period_frac:\t%u\n", dpcm->period_size_frac);
	snd_iprintf(buffer, "    last_jiffies:\t%lu (%lu)\n",
					dpcm->last_jiffies, jiffies);
	snd_iprintf(buffer, "    timer_expires:\t%lu\n", dpcm->timer.expires);
#endif
}

static void print_substream_info(struct snd_info_buffer *buffer,
				 struct loopback *loopback,
				 int sub,
				 int num)
{
	struct loopback_cable *cable = loopback->cables[sub][num];

	snd_iprintf(buffer, "Cable %i substream %i:\n", num, sub);
	if (cable == NULL) {
		snd_iprintf(buffer, "  inactive\n");
		return;
	}
	snd_iprintf(buffer, "  valid: %u\n", cable->valid);
	snd_iprintf(buffer, "  running: %u\n", cable->running);
	snd_iprintf(buffer, "  pause: %u\n", cable->pause);
#ifdef CONFIG_ARCH_MT8590_ICX
	snd_iprintf(buffer, "ICX Config:\n");
	snd_iprintf(buffer, "  rates: 0x%0x(%d->%d)\n", loopback_pcm_hardware.rates, loopback_pcm_hardware.rate_min, loopback_pcm_hardware.rate_max);
	snd_iprintf(buffer, "  delays: buffer_delay=%d, out_delay_usec=%d\n", CONFIG_SND_ALOOP_AUDIO_OUT_BUFFER_PERIOD_NUM, CONFIG_SND_ALOOP_AUDIO_OUT_DELAY_US);
#ifdef ICX_ENABLE_DMA_DRIVE_MODE
	snd_iprintf(buffer, "  playbackDelay: %d usec\n", loopback->playback_delay_usec);
	if (cable->streams[1]) {
		snd_iprintf(buffer,
			"  idealDmaStartDelay: %lld usec\n",
			div_u64(frac_pos(cable->streams[1], cable->streams[1]->pcm_period_size), cable->streams[1]->pcm_bps * 1000) * CONFIG_SND_ALOOP_AUDIO_OUT_BUFFER_PERIOD_NUM);
	}
#endif
#endif
	print_dpcm_info(buffer, cable->streams[0], "Playback");
	print_dpcm_info(buffer, cable->streams[1], "Capture");
}

static void print_cable_info(struct snd_info_entry *entry,
			     struct snd_info_buffer *buffer)
{
	struct loopback *loopback = entry->private_data;
	int sub, num;

	mutex_lock(&loopback->cable_lock);
	num = entry->name[strlen(entry->name)-1];
	num = num == '0' ? 0 : 1;
	for (sub = 0; sub < MAX_PCM_SUBSTREAMS; sub++)
		print_substream_info(buffer, loopback, sub, num);
	mutex_unlock(&loopback->cable_lock);
}

static int loopback_proc_new(struct loopback *loopback, int cidx)
{
	char name[32];
	struct snd_info_entry *entry;
	int err;

	snprintf(name, sizeof(name), "cable#%d", cidx);
	err = snd_card_proc_new(loopback->card, name, &entry);
	if (err < 0)
		return err;

	snd_info_set_text_ops(entry, loopback, print_cable_info);
	return 0;
}

#else /* !CONFIG_PROC_FS */

#define loopback_proc_new(loopback, cidx) do { } while (0)

#endif

static int loopback_probe(struct platform_device *devptr)
{
	struct snd_card *card;
	struct loopback *loopback;
	int dev = devptr->id;
	int err;

	err = snd_card_create(index[dev], id[dev], THIS_MODULE,
			      sizeof(struct loopback), &card);
	if (err < 0)
		return err;
	loopback = card->private_data;

	if (pcm_substreams[dev] < 1)
		pcm_substreams[dev] = 1;
	if (pcm_substreams[dev] > MAX_PCM_SUBSTREAMS)
		pcm_substreams[dev] = MAX_PCM_SUBSTREAMS;
	
	loopback->card = card;
	mutex_init(&loopback->cable_lock);
#ifdef CONFIG_ARCH_MT8590_ICX
	init_waitqueue_head(&loopback->capture_close_wq);
	loopback->playback_active = false;
	loopback->capture_close_waiting = false;
	loopback->capture_delay_not_treated = false;
	loopback->playback_buf = vmalloc(PLAYBACK_BUF_BYTES);
	if (!loopback->playback_buf) {
		err = -ENOMEM;
		goto __nodev;
	}
	loopback->playback_buf_pos = 0;
#endif
#ifdef ICX_ENABLE_DMA_DRIVE_MODE
	loopback->playback_delay_usec = 0;
	period_elapsed_notification_init();
#endif
	err = loopback_pcm_new(loopback, 0, pcm_substreams[dev]);
	if (err < 0)
		goto __nodev;
	err = loopback_pcm_new(loopback, 1, pcm_substreams[dev]);
	if (err < 0)
		goto __nodev;
	err = loopback_mixer_new(loopback, pcm_notify[dev] ? 1 : 0);
	if (err < 0)
		goto __nodev;
	loopback_proc_new(loopback, 0);
	loopback_proc_new(loopback, 1);
	strcpy(card->driver, "Loopback");
	strcpy(card->shortname, "Loopback");
	sprintf(card->longname, "Loopback %i", dev + 1);
	err = snd_card_register(card);
	if (!err) {
		platform_set_drvdata(devptr, card);
		return 0;
	}
      __nodev:
	snd_card_free(card);
	return err;
}

static int loopback_remove(struct platform_device *devptr)
{
#ifdef CONFIG_ARCH_MT8590_ICX
	struct snd_card *card;
	struct loopback *loopback;
	card = platform_get_drvdata(devptr);
	loopback = card->private_data;
	vfree(loopback->playback_buf);
#endif
	snd_card_free(platform_get_drvdata(devptr));
	platform_set_drvdata(devptr, NULL);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int loopback_suspend(struct device *pdev)
{
	struct snd_card *card = dev_get_drvdata(pdev);
	struct loopback *loopback = card->private_data;

	snd_power_change_state(card, SNDRV_CTL_POWER_D3hot);

	snd_pcm_suspend_all(loopback->pcm[0]);
	snd_pcm_suspend_all(loopback->pcm[1]);
	return 0;
}
	
static int loopback_resume(struct device *pdev)
{
	struct snd_card *card = dev_get_drvdata(pdev);

	snd_power_change_state(card, SNDRV_CTL_POWER_D0);
	return 0;
}

static SIMPLE_DEV_PM_OPS(loopback_pm, loopback_suspend, loopback_resume);
#define LOOPBACK_PM_OPS	&loopback_pm
#else
#define LOOPBACK_PM_OPS	NULL
#endif

#define SND_LOOPBACK_DRIVER	"snd_aloop"

static struct platform_driver loopback_driver = {
	.probe		= loopback_probe,
	.remove		= loopback_remove,
	.driver		= {
		.name	= SND_LOOPBACK_DRIVER,
		.owner	= THIS_MODULE,
		.pm	= LOOPBACK_PM_OPS,
	},
};

static void loopback_unregister_all(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(devices); ++i)
		platform_device_unregister(devices[i]);
	platform_driver_unregister(&loopback_driver);
}

static int __init alsa_card_loopback_init(void)
{
	int i, err, cards;

	err = platform_driver_register(&loopback_driver);
	if (err < 0)
		return err;


	cards = 0;
	for (i = 0; i < SNDRV_CARDS; i++) {
		struct platform_device *device;
		if (!enable[i])
			continue;
		device = platform_device_register_simple(SND_LOOPBACK_DRIVER,
							 i, NULL, 0);
		if (IS_ERR(device))
			continue;
		if (!platform_get_drvdata(device)) {
			platform_device_unregister(device);
			continue;
		}
		devices[i] = device;
		cards++;
	}
	if (!cards) {
#ifdef MODULE
		printk(KERN_ERR "aloop: No loopback enabled\n");
#endif
		loopback_unregister_all();
		return -ENODEV;
	}
	return 0;
}

static void __exit alsa_card_loopback_exit(void)
{
	loopback_unregister_all();
}

module_init(alsa_card_loopback_init)
module_exit(alsa_card_loopback_exit)
