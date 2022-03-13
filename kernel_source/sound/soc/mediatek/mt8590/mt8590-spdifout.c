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
#include <linux/delay.h>
#include "mt8590-private.h"
#include "mt8590-afe-reg.h"
#include "mt8590-hdmi-control.h"

kal_uint32 iec2_nsadr = 0;

static const struct snd_pcm_hardware spdifout_hardware = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_RESUME |
	SNDRV_PCM_INFO_MMAP_VALID),
	.formats = (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_IEC958_SUBFRAME_LE),
	.rates = (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
	SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
	SNDRV_PCM_RATE_192000),
	.rate_min = 32000,
	.rate_max = 192000,
	.channels_min = 2,
	.channels_max = 8,
	.buffer_bytes_max = (1024 * 1024 * 16),
	.period_bytes_min = (2 * 1024),
	.period_bytes_max = (64 * 1024),
	.periods_min = 2,
	.periods_max = 256,
	.fifo_size = 0,
};

static void spdifout_isr(struct mt_stream *s)
{
	kal_uint32 hw_consume_bytes = 0;
	kal_uint32 hw_mem_idex = 0;
	kal_uint32 hw_cur_idex = 0;
	kal_uint32 burst_len = 0;
	if ((s == NULL) || (afe_read(AFE_IEC2_BURST_INFO) & 0x000010000)) {
		pr_err("%s() error: invalid values\n", __func__);
		return;
	}
	hw_cur_idex = afe_read(AFE_SPDIF2_CUR);
	if (hw_cur_idex == 0) {
		pr_debug("%s() hw_cur_idex = 0\n", __func__);
		hw_cur_idex = s->substream->runtime->dma_addr;
	}
	hw_mem_idex = (hw_cur_idex - s->substream->runtime->dma_addr);
	if (hw_mem_idex > s->pointer)
		hw_consume_bytes = hw_mem_idex - s->pointer;
	else
		hw_consume_bytes = s->substream->runtime->dma_bytes + hw_mem_idex - s->pointer;
	pr_debug
	("%s() hw_cur_idex = 0x%x hw_mem_idex = 0x%x PhysicalAddr = 0x%x,hw_consume_bytes = 0x%x\n",
	 __func__, hw_cur_idex, hw_mem_idex, s->substream->runtime->dma_addr, hw_consume_bytes);
	s->pointer += hw_consume_bytes;
	s->pointer %= s->substream->runtime->dma_bytes;
	burst_len = (afe_read(AFE_IEC2_BURST_LEN) & 0x0007ffff) >> 3;
	iec2_nsadr += burst_len;
	if (iec2_nsadr >= afe_read(AFE_SPDIF2_END))
		iec2_nsadr = afe_read(AFE_SPDIF2_BASE);
	afe_msk_write(AFE_IEC2_NSADR, iec2_nsadr, 0xffffffff);
	afe_msk_write(AFE_IEC2_BURST_INFO, IEC2_BURST_READY_NOT_READY, IEC2_BURST_READY_MASK);
	snd_pcm_period_elapsed(s->substream);
}

static int mt8590_spdifout_open(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	snd_soc_set_runtime_hwparams(substream, &spdifout_hardware);
	/* Ensure that buffer size is a multiple of period size */
	enable_pll(AUD2PLL, "AUDIO");
	enable_clock(MT_CG_AUDIO_APLL, "AUDIO");
	enable_clock(MT_CG_AUDIO_SPDF2_CK, "AUDIO");
	ret = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	if (ret < 0)
		pr_err("%s() error: snd_pcm_hw_constraint_integer fail %d\n", __func__, ret);
	pr_debug("%s() substream->pcm->device = %d\n", __func__, substream->pcm->device);
	return ret;
}

static int mt8590_spdifout_close(struct snd_pcm_substream *substream)
{
	pr_debug("%s() substream->pcm->device = %d\n", __func__, substream->pcm->device);
	disable_clock(MT_CG_AUDIO_SPDF2_CK, "AUDIO");
	disable_clock(MT_CG_AUDIO_APLL, "AUDIO");
	disable_pll(AUD2PLL, "AUDIO");
	return 0;
}

static int mt8590_spdifout_params(struct snd_pcm_substream *substream,
				  struct snd_pcm_hw_params *hw_params)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_dma_buffer *dma_buf = &substream->dma_buffer;
	dma_buf->dev.type = SNDRV_DMA_TYPE_DEV;
	dma_buf->dev.dev = substream->pcm->card->dev;
	ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
	if (ret < 0)
		pr_err("%s() error: snd_pcm_lib_malloc_pages fail %d\n", __func__, ret);
	pr_debug("%s() dma_bytes = %zu dma_area = %p dma_addr = 0x%llx\n",
		 __func__, runtime->dma_bytes, runtime->dma_area,
		 (unsigned long long)runtime->dma_addr);
	return ret;
}

static int mt8590_spdifout_free(struct snd_pcm_substream *substream)
{
	pr_debug("%s()\n", __func__);
	return snd_pcm_lib_free_pages(substream);
}

static int mt8590_spdifout_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct mt_stream *s = runtime->private_data;
	enum afe_sampling_rate sample_rate_idx = fs_enum(runtime->rate);
	pr_debug("%s() rate = %u  channels = %u period_size = %lu\n",
		 __func__, runtime->rate, runtime->channels, runtime->period_size);
	if (frames_to_bytes(runtime, runtime->buffer_size) % 16 != 0) {
		pr_err("%s() error: buffer-size not multiple 16 bytes\n", __func__);
		return -1;
	}
	pr_debug("%s() channel = %u,samplerate = %d,bit = %u,runtime_format = %i\n",
		 __func__, runtime->channels, sample_rate_idx, runtime->sample_bits, runtime->format);
	pr_debug("%s() period_size = %lu,periods = %u,buffer_size = %lu\n",
		 __func__, runtime->period_size, runtime->periods, runtime->buffer_size);
	pr_debug("%s() dma_bytes = %zu, dma_area = %p, dma_addr = 0x%llx\n", __func__,
		 runtime->dma_bytes, runtime->dma_area, (unsigned long long)runtime->dma_addr);
	vAudioClockSetting(sample_rate_idx, 128, APLL_SPDIF2_CK, PCM_OUTPUT_32BIT, 0);
	init_hdmi_dma_buffer(MT_AFE_MEM_IEC2, runtime, s);
	set_data_output_from_iec2_enable(sample_rate_idx, runtime);
	return 0;
}

static int mt8590_spdifout_start(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct mt_stream *s = runtime->private_data;
	pr_debug("%s()\n", __func__);
	audio_irq_enable(s->irq->id, 1);
	return 0;
}

static int mt8590_spdifout_stop(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct mt_stream *s = runtime->private_data;
	enum afe_sampling_rate sample_rate_idx = fs_enum(runtime->rate);
	pr_debug("%s()\n", __func__);
	audio_irq_enable(s->irq->id, 0);
	s->pointer = 0;
	afe_msk_write(AFE_IEC2_CFG, IEC2_MUTE_DATA_MUTE, IEC2_MUTE_DATA_MASK);
	afe_msk_write(AFE_IEC2_CFG, IEC2_SW_RST, IEC2_SW_RST_MASK);
	afe_msk_write(AFE_IEC2_CFG, IEC2_DISABLE, IEC2_DISABLE_MASK);
	afe_msk_write(AFE_SPDIF2_OUT_CON0, SPDIF2_OUT_CLOCK_OFF | SPDIF2_OUT_MEMIF_OFF,
		      SPDIF2_OUT_CLOCK_ON_OFF_MASK | SPDIF2_OUT_MEMIF_ON_OFF_MASK);
	return 0;
}

static int mt8590_spdifout_trigger(struct snd_pcm_substream *substream, int cmd)
{
	pr_debug("%s() cmd = %d\n", __func__, cmd);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		return mt8590_spdifout_start(substream);
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		return mt8590_spdifout_stop(substream);
	default:
		pr_err("%s() error: cmd = %d not valid\n", __func__, cmd);
		break;
	}
	return -EINVAL;
}

static snd_pcm_uframes_t mt8590_spdifout_pointer(struct snd_pcm_substream *substream)
{
	int offset = 0;
	struct mt_stream *s = substream->runtime->private_data;
	offset = bytes_to_frames(substream->runtime, s->pointer);
	if (unlikely(offset >= substream->runtime->buffer_size))
		offset = 0;
	pr_debug("%s() offset = %d\n", __func__, offset);
	udelay(10);
	return offset;
}

static struct snd_pcm_ops spdif_iec2_ops = {
	.open = mt8590_spdifout_open,
	.close = mt8590_spdifout_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = mt8590_spdifout_params,
	.hw_free = mt8590_spdifout_free,
	.prepare = mt8590_spdifout_prepare,
	.trigger = mt8590_spdifout_trigger,
	.pointer = mt8590_spdifout_pointer,
};
