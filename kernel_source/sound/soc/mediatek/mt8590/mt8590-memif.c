/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
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

#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <mach/mt_clkmgr.h>
#include "mt8590-dai.h"
#include "mt8590-afe.h"
#include "mt8590-memif.h"
#include "mt8590-private.h"
#include "mt8590-audrtc.h"
#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
#include "../../codecs/cxd3778gf/cxd3778gf_common.h"
#endif

#ifdef CONFIG_SND_ENABLE_ICX_DMA_DRIVE_MODE
// #define PAS_DELAY_CHECK
#ifdef PAS_DELAY_CHECK
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_core.h>
#include <mach/mt_gpio_base.h>
#define TIMING_GPIO (GPIO115 | 0x80000000)
static bool s_gpio_state = false;
#endif

/**
* period_elapsed_notification function
*/

struct period_elapsed_notification_listener {
	void (*pcm_start_triggered)(void* data);
	void (*period_elapsed)(void* data);
	void (*period_stopped)(void* data);
	void* userData;
};

struct period_elapsed_notification {
	spinlock_t lock;
	struct period_elapsed_notification_listener* listener;
};

static struct period_elapsed_notification s_period_elapsed_notification = {};

static void period_elapsed_notification_init(void) {
	spin_lock_init(&s_period_elapsed_notification.lock);
}

static void period_elapsed_notification_notify_elapsed(void) {
	unsigned long flags;
	spin_lock_irqsave(&s_period_elapsed_notification.lock, flags);
	if (s_period_elapsed_notification.listener) {
			s_period_elapsed_notification.listener->period_elapsed(s_period_elapsed_notification.listener->userData);
	}
	spin_unlock_irqrestore(&s_period_elapsed_notification.lock, flags);
}

static void period_elapsed_notification_notify_stopped(void) {
	unsigned long flags;
	spin_lock_irqsave(&s_period_elapsed_notification.lock, flags);
	if (s_period_elapsed_notification.listener) {
			s_period_elapsed_notification.listener->period_stopped(s_period_elapsed_notification.listener->userData);
	}
	spin_unlock_irqrestore(&s_period_elapsed_notification.lock, flags);
}

static void period_elapsed_notification_notify_started(void) {
	unsigned long flags;
#ifdef PAS_DELAY_CHECK
	s_gpio_state = true;
	mt_set_gpio_out(TIMING_GPIO, s_gpio_state);
#endif
	spin_lock_irqsave(&s_period_elapsed_notification.lock, flags);
	if (s_period_elapsed_notification.listener) {
			s_period_elapsed_notification.listener->pcm_start_triggered(s_period_elapsed_notification.listener->userData);
	}
	spin_unlock_irqrestore(&s_period_elapsed_notification.lock, flags);
}

void mt8590_period_elapsed_notification_set_listener_raw(
	struct period_elapsed_notification_listener* listener) {
	s_period_elapsed_notification.listener = listener;
}

void mt8590_period_elapsed_notification_set_listener(
	struct period_elapsed_notification_listener* listener) {
	unsigned long flags;
	spin_lock_irqsave(&s_period_elapsed_notification.lock, flags);
	mt8590_period_elapsed_notification_set_listener_raw(listener);
	spin_unlock_irqrestore(&s_period_elapsed_notification.lock, flags);
}

EXPORT_SYMBOL(mt8590_period_elapsed_notification_set_listener);
EXPORT_SYMBOL(mt8590_period_elapsed_notification_set_listener_raw);
#endif

static const struct snd_pcm_hardware memif_hardware = {
	.info = SNDRV_PCM_INFO_MMAP
	      | SNDRV_PCM_INFO_INTERLEAVED
	      | SNDRV_PCM_INFO_RESUME
	      | SNDRV_PCM_INFO_MMAP_VALID,
	.formats = SNDRV_PCM_FMTBIT_S16_LE
		 | SNDRV_PCM_FMTBIT_S24_LE
		 | SNDRV_PCM_FMTBIT_S32_LE,
	.period_bytes_min = 16,
	.period_bytes_max = 1024 * 256,
	.periods_min = CONFIG_SND_SOC_PERIODS_MINIMUM_MEMIF,
	.periods_max = 1024,
	.buffer_bytes_max = 1024 * 1024 * 16,
	.fifo_size = 0,
};

static enum afe_mem_interface get_memif(struct mt_stream *s)
{
	if (!s)
		return AFE_MEM_NONE;
	switch (s->id) {
	case MT_STREAM_DL1:
		return AFE_MEM_DL1;
	case MT_STREAM_DL2:
		return AFE_MEM_DL2;
	case MT_STREAM_DL3:
		return AFE_MEM_DL3;
	case MT_STREAM_DL4:
		return AFE_MEM_DL4;
	case MT_STREAM_DL5:
		return AFE_MEM_DL5;
	case MT_STREAM_DL6:
		return AFE_MEM_DL6;
	case MT_STREAM_DLM:
		return AFE_MEM_DLMCH;
	case MT_STREAM_UL1:
		return AFE_MEM_UL1;
	case MT_STREAM_UL2:
		return AFE_MEM_UL2;
	case MT_STREAM_UL3:
		return AFE_MEM_UL3;
	case MT_STREAM_UL4:
		return AFE_MEM_UL4;
	case MT_STREAM_UL5:
		return AFE_MEM_UL5;
	case MT_STREAM_UL6:
		return AFE_MEM_UL6;
	case MT_STREAM_ARB1:
		return AFE_MEM_ARB1;
	case MT_STREAM_DSDR:
		return AFE_MEM_DSDR;
	case MT_STREAM_DAI:
		return AFE_MEM_DAI;
	case MT_STREAM_MOD_PCM:
		return AFE_MEM_MOD_PCM;
	case MT_STREAM_AWB:
		return AFE_MEM_AWB;
	case MT_STREAM_AWB2:
		return AFE_MEM_AWB2;
	case MT_STREAM_DSDW:
		return AFE_MEM_DSDW;
	case MT_STREAM_MULTIIN:
		return AFE_MEM_ULMCH;
	default:
		return AFE_MEM_NONE;
	}
}
static enum cg_clk_id get_clk(enum afe_mem_interface memif)
{
	switch (memif) {
	case AFE_MEM_DL1:
		return MT_CG_AUDIO_MMIF_DL1;
	case AFE_MEM_DL2:
		return MT_CG_AUDIO_MMIF_DL2;
	case AFE_MEM_DL3:
		return MT_CG_AUDIO_MMIF_DL3;
	case AFE_MEM_DL4:
		return MT_CG_AUDIO_MMIF_DL4;
	case AFE_MEM_DL5:
		return MT_CG_AUDIO_MMIF_DL5;
	case AFE_MEM_DL6:
		return MT_CG_AUDIO_MMIF_DL6;
	case AFE_MEM_DLMCH:
		return MT_CG_AUDIO_MMIF_DLMCH;
	case AFE_MEM_ARB1:
		return MT_CG_AUDIO_MMIF_ARB1;
	case AFE_MEM_UL1:
		return MT_CG_AUDIO_MMIF_UL1;
	case AFE_MEM_UL2:
		return MT_CG_AUDIO_MMIF_UL2;
	case AFE_MEM_UL3:
		return MT_CG_AUDIO_MMIF_UL3;
	case AFE_MEM_UL4:
		return MT_CG_AUDIO_MMIF_UL4;
	case AFE_MEM_UL5:
		return MT_CG_AUDIO_MMIF_UL5;
	case AFE_MEM_UL6:
		return MT_CG_AUDIO_MMIF_UL6;
	case AFE_MEM_DAI:
		return MT_CG_AUDIO_MMIF_DAI;
	case AFE_MEM_AWB:
		return MT_CG_AUDIO_MMIF_AWB1;
	case AFE_MEM_AWB2:
		return MT_CG_AUDIO_MMIF_AWB2;
	case AFE_MEM_DSDR:
	case AFE_MEM_ULMCH:
	case AFE_MEM_DSDW:
	case AFE_MEM_MOD_PCM:
	default:
		return NR_CLKS;
	}
}
static int memif_open(struct snd_pcm_substream *substream)
{
	struct mt_stream *s = substream->runtime->private_data;
	enum afe_mem_interface memif = get_memif(s);
	enum cg_clk_id clk = get_clk(memif);
	if (clk != NR_CLKS) {
		enable_clock(clk, "AUDIO");
		if (clk == MT_CG_AUDIO_MMIF_DLMCH) {
			enable_clock(MT_CG_AUDIO_MMIF_DL1, "AUDIO");
			enable_clock(MT_CG_AUDIO_MMIF_DL2, "AUDIO");
			enable_clock(MT_CG_AUDIO_MMIF_DL3, "AUDIO");
			enable_clock(MT_CG_AUDIO_MMIF_DL4, "AUDIO");
			enable_clock(MT_CG_AUDIO_MMIF_DL5, "AUDIO");
		}
	}
	snd_soc_set_runtime_hwparams(substream, &memif_hardware);
	return 0;
}

static int memif_close(struct snd_pcm_substream *substream)
{
	struct mt_stream *s = substream->runtime->private_data;
	enum afe_mem_interface memif = get_memif(s);
	enum cg_clk_id clk = get_clk(memif);
	if (clk != NR_CLKS) {
		disable_clock(clk, "AUDIO");
		if (clk == MT_CG_AUDIO_MMIF_DLMCH) {
			disable_clock(MT_CG_AUDIO_MMIF_DL1, "AUDIO");
			disable_clock(MT_CG_AUDIO_MMIF_DL2, "AUDIO");
			disable_clock(MT_CG_AUDIO_MMIF_DL3, "AUDIO");
			disable_clock(MT_CG_AUDIO_MMIF_DL4, "AUDIO");
			disable_clock(MT_CG_AUDIO_MMIF_DL5, "AUDIO");
		}
	}
	return 0;
}

static int memif_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	int ret;
#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt_stream *s = substream->runtime->private_data;
#endif
	pr_debug("%s()\n", __func__);
	ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(params));
	if (ret < 0) {
		pr_err("%s() error: allocation of memory failed\n", __func__);
		return ret;
	}
#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
	if (rtd->cpu_dai->id == MT8590_DAI_DSDENC_ID)
                set_bit(0x01,&(s->drain_flag));
#endif

	return 0;
}

static int memif_hw_free(struct snd_pcm_substream *substream)
{
	pr_debug("%s()\n", __func__);

	return snd_pcm_lib_free_pages(substream);
}

static int memif_prepare(struct snd_pcm_substream *substream)
{
	int ret;
	struct mt_stream *s = substream->runtime->private_data;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	enum afe_mem_interface memif = get_memif(s);
	enum afe_sampling_rate fs;
	enum afe_channel_mode channel = STEREO;
	enum afe_dlmch_ch_num dlmch_ch_num = DLMCH_0CH;
	enum afe_dsd_width dsd_width = DSD_WIDTH_32BIT;
	enum afe_multilinein_chnum ch_num = AFE_MULTILINEIN_2CH;
	enum afe_i2s_format fmt = FMT_64CYCLE_16BIT_I2S;
	int hd_audio = 1;
	int dsd_mode = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	int dai_id = rtd->cpu_dai->id;
	if (s->use_i2s_slave_clock) {
		if (dai_id >= MT8590_DAI_I2S1_ID && dai_id <= MT8590_DAI_I2S6_ID)
			fs = FS_I2S1 + (dai_id - MT8590_DAI_I2S1_ID);
		else
			fs = FS_I2S1;
	} else {
		fs = fs_enum(runtime->rate);
		/*afe_hwgain_gainmode_set(AFE_HWGAIN_1, fs);*/
	}
	channel = (runtime->channels == 1) ? MONO : STEREO;
	if (runtime->channels > 0 && runtime->channels <= 10)
		dlmch_ch_num = (enum afe_dlmch_ch_num)runtime->channels;
	else {
		pr_err("%s() error: unsupported channel %u\n", __func__, runtime->channels);
		return -EINVAL;
	}
	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S16_LE:
		hd_audio = 0;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		hd_audio = 1;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		hd_audio = 1;
		break;
	case SNDRV_PCM_FORMAT_DSD_U8:
		hd_audio = 1;
		dsd_width = DSD_WIDTH_8BIT;
		break;
	case SNDRV_PCM_FORMAT_DSD_U16_LE:
		hd_audio = 1;
		dsd_width = DSD_WIDTH_16BIT;
		break;
	default:
		pr_err("%s() error: unsupported format %d\n", __func__, runtime->format);
		return -EINVAL;
	}
	/* configurate memory interface */
	{
		struct afe_memif_config config = {
			.fs = fs,
			.hd_audio = (memif == AFE_MEM_ULMCH) ? 1 : hd_audio,
			.dsd_width = dsd_width,
			.first_bit = MSB_FIRST,
			.daimod_fs = DAIMOD_8000HZ,
			.channel = channel,
			.dlmch_ch_num = dlmch_ch_num,
			.mono_sel = MONO_USE_L,
			.dup_write = DUP_WR_DISABLE,
			.buffer = {
				.base = runtime->dma_addr,
				.size = frames_to_bytes(runtime, runtime->buffer_size)
			}
		};
		pr_debug("%s() fs=%d, hd_audio=%d, dsd_width=%d, channel=%d\n", __func__,
			 config.fs, config.hd_audio, config.dsd_width, config.channel);
		ret = afe_memif_configurate(memif, &config);
		if (ret < 0) {
			pr_err("%s() error: afe_memif_configurate return %d\n", __func__, ret);
			return ret;
		}
		s->pointer = 0;
	}
	if (AFE_MEM_ULMCH == memif) {
		if (SNDRV_PCM_FORMAT_DSD_U8 == runtime->format){
			dsd_mode = 1;
			fmt = FMT_64CYCLE_32BIT_I2S;
		}
		if (SNDRV_PCM_FORMAT_S16_LE == runtime->format)
			fmt = FMT_64CYCLE_16BIT_I2S;
		else if (SNDRV_PCM_FORMAT_S32_LE == runtime->format)
			fmt = FMT_64CYCLE_32BIT_I2S;
		if ((runtime->channels == 2) || (runtime->channels == 6) || (runtime->channels == 8)) {
			if (runtime->channels == 2)
				ch_num = AFE_MULTILINEIN_2CH;
			else if (runtime->channels == 6)
				ch_num = AFE_MULTILINEIN_6CH;
			else if (runtime->channels == 8)
				ch_num = AFE_MULTILINEIN_8CH;
		} else {
			pr_err("%s() error: multiline need set 2ch ,6ch or 8ch mode!\n", __func__);
			return -EINVAL;
		}
		pr_debug("%s() : dsd_mode = 0x%x,fmt = 0x%x,ch_num = 0x%x,dsd_width = 0x%x\n", __func__,dsd_mode,fmt,ch_num,dsd_width);
		{
			struct afe_multilinein_config config = {
				.dsd_mode = dsd_mode,
				.endian = AFE_MULTILINEIN_LITTILE_ENDIAN,
				.fmt = fmt,
				.ch_num = ch_num,
				.intr_period = AFE_MULTILINEIN_INTR_PERIOD_256,
				.mss = AFE_MULTILINE_FROM_RX,
				.dsdWidth = dsd_width
			};
			afe_multilinein_configurate(&config);
		}
	}
	/* configurate irq */
	if (s->irq) {
		struct audio_irq_config config = {
			.mode = fs,
			.init_val = runtime->period_size
		};
		audio_irq_configurate(s->irq->id , &config);
	}

	return 0;
}

struct rtc_multi_i2s_playback rtc_multi[10];

int  total_playback_num = 0;


static int memif_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct mt_stream *s = runtime->private_data;
	enum afe_mem_interface memif = get_memif(s);
	struct mt_irq *irq = s->irq;
	int pll = rate_to_pll(fs_enum(runtime->rate));
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		/* silence the buffer that has been processed by hw */
		runtime->silence_threshold = 0;
		runtime->silence_size = runtime->boundary;
#ifdef CONFIG_SND_ENABLE_ICX_DMA_DRIVE_MODE
		if (substream->pcm->device == 0) {
			period_elapsed_notification_notify_started();
		}
#endif

		if (pll == AUD2PLL) {
			enable_pll(AUD2PLL, "AUDIO");
			enable_clock(MT_CG_AUDIO_A2SYS, "AUDIO");
		}
		if (target_acc_irq_id != IRQ_NUM) {
			if (total_playback_num > (sizeof(rtc_multi)/sizeof(rtc_multi[0])))
				   pr_notice("%s() above i2s max num! not support!\n", __func__);

			rtc_multi[total_playback_num].memiftargetacc = memif;

			if (irq)
				rtc_multi[total_playback_num].targetaccirqid = irq->id;
			else
				rtc_multi[total_playback_num].targetaccirqid = IRQ_NUM;

			total_playback_num++;

		} else {
			afe_memif_enable(memif, 1);
			/*afe_hwgain_enable(AFE_HWGAIN_1, 1);*/
			if (irq)
				audio_irq_enable(irq->id, 1);
		}
		return 0;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
#ifdef CONFIG_SND_ENABLE_ICX_DMA_DRIVE_MODE
		if (substream->pcm->device == 0) {
			period_elapsed_notification_notify_stopped();
		}
#endif
		if (irq) {
			audio_irq_enable(irq->id, 0);
			/*
			 * To make sure the interrupt signal is disabled,
			 * we must clear the status again after disabling irq.
			 */
			if (irq->id >= IRQ_ASYS_IRQ1)
				asys_irq_clear(0x1 << (irq->id - IRQ_ASYS_IRQ1));
			else
				afe_irq_clear(0x1 << (irq->id - IRQ_AFE_IRQ1));
		}
		afe_memif_enable(memif, 0);
		/*afe_hwgain_enable(AFE_HWGAIN_1, 0);*/
		if (pll == AUD2PLL) {
			disable_clock(MT_CG_AUDIO_A2SYS, "AUDIO");
			disable_pll(AUD2PLL, "AUDIO");
		}
		return 0;
	default:
		return -EINVAL;
	}
}

static snd_pcm_uframes_t memif_pointer(struct snd_pcm_substream *substream)
{
	snd_pcm_uframes_t offset;
	struct mt_stream *s = substream->runtime->private_data;
	struct snd_pcm_runtime *runtime = substream->runtime;
#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
#endif
	offset = bytes_to_frames(runtime, s->pointer);
	if (unlikely(offset >= runtime->buffer_size))
		offset = 0;

#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
	if (rtd->cpu_dai->id == MT8590_DAI_DSDENC_ID)
		if (substream->runtime->status->state == SNDRV_PCM_STATE_DRAINING && test_and_clear_bit(0x01, &(s->drain_flag))){
			set_timed_mute_dsd_termination();
	}
#endif
	return offset;
}

static struct snd_pcm_ops memif_ops = {
	.open = memif_open,
	.close = memif_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = memif_hw_params,
	.hw_free = memif_hw_free,
	.prepare = memif_prepare,
	.trigger = memif_trigger,
	.pointer = memif_pointer,
};

static void memif_isr(struct mt_stream *s)
{
	if (s) {
		u32 base, cur;
		enum afe_mem_interface memif = get_memif(s);
		afe_memif_base(memif, &base);
		afe_memif_pointer(memif, &cur);
		s->pointer = cur - base;
#ifdef CONFIG_SND_ENABLE_ICX_DMA_DRIVE_MODE
		if (s->substream->pcm->device == 0) {
#ifdef PAS_DELAY_CHECK_ISR
			s_gpio_state = !s_gpio_state;
			mt_set_gpio_out(TIMING_GPIO, s_gpio_state);
#endif
			period_elapsed_notification_notify_elapsed();
		}
#endif
		snd_pcm_period_elapsed(s->substream);
	}
}
