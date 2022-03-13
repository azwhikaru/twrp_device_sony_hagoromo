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

#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include "mt8590-dai.h"
#include "mt8590-dai-private.h"
#include "mt8590-afe.h"
#include "mt8590-private.h"
#include "mt8590-lp-audio.h"
#include "mt8590-audrtc.h"

struct snd_card *snd_card_test;

#include "mt8590-audio-controls.c"
#include "mt8590-memif.c"
#include "mt8590-spdifout.c"
#include "mt8590-hdmi-pcm.c"
#include "mt8590-hdmi-raw.c"

static void link_stream_and_irq(struct mt_private *priv,
				enum mt_stream_id stream_id,
				enum audio_irq_id irq_id, void (*isr)(struct mt_stream *));

static int mt8590_pcm_open(struct snd_pcm_substream *substream)
{
	struct mt_stream *s;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt_private *priv = snd_soc_platform_get_drvdata(rtd->platform);
	pr_debug("%s() cpu_dai id %d, stream direction %d\n",
		 __func__, rtd->cpu_dai->id, substream->stream);
	s = priv->dais[rtd->cpu_dai->id][substream->stream].s;
	substream->runtime->private_data = s;
	if (!s) {
		pr_err("%s() error: no mt stream for this dai\n", __func__);
		return -EINVAL;
	}
	pr_debug("%s() %s\n", __func__, s->name);
	if (s->occupied) {
		pr_warn("%s() warning: can't open %s because it has been occupied\n", __func__, s->name);
		return -EINVAL;
	}
	if (s->id >= MT_STREAM_DL1 && s->id < MT_STREAM_DL1 + I2SM_NUM) {
		if (priv->streams[MT_STREAM_DLM].occupied) {
			pr_warn("%s() warning: can't open %s because MT_STREAM_DLM has been occupied\n",
				__func__, s->name);
			return -EINVAL;
		}
	}
	if (s->id == MT_STREAM_DLM) {
		enum mt_stream_id i;
		for (i = MT_STREAM_DL1; i < MT_STREAM_DL1 + I2SM_NUM; ++i) {
			if (priv->streams[i].occupied) {
				pr_warn("%s() warning: can't open MT_STREAM_DLM because %s has been occupied\n",
					__func__, priv->streams[i].name);
				return -EINVAL;
			}
		}
	}
	s->substream = substream;
	s->occupied = 1;
	if (s->irq == NULL) {
		enum audio_irq_id irq_id = asys_irq_acquire();
		if (irq_id != IRQ_NUM) {
			/* link */
			link_stream_and_irq(priv, s->id, irq_id, memif_isr);
		} else
			pr_err("%s() error: no more asys irq\n", __func__);
	}
	snd_pcm_hw_constraint_step(substream->runtime, 0, SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 16);
	a1sys_start(AUD1PLL, priv->dais[rtd->cpu_dai->id][substream->stream].itrcon, 1);
	if (s->ops && s->ops->open)
		return s->ops->open(substream);
	return 0;
}

static int mt8590_pcm_close(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt_private *priv = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt_stream *s = substream->runtime->private_data;
	pr_debug("%s() cpu_dai id %d, stream direction %d\n",
		 __func__, rtd->cpu_dai->id, substream->stream);
	if (!s) {
		pr_err("%s() error: no mt stream for this dai\n", __func__);
		return -EINVAL;
	}
	pr_debug("%s() %s\n", __func__, s->name);
	if (s->ops && s->ops->close)
		ret = s->ops->close(substream);
	if (s->irq) {
		enum audio_irq_id irq_id = s->irq->id;
		if (irq_id >= IRQ_ASYS_IRQ1 && irq_id <= IRQ_ASYS_IRQ16) {
			/* delink */
			link_stream_and_irq(priv, s->id, irq_id, NULL);
			asys_irq_release(irq_id);
		}
	}
	a1sys_start(AUD1PLL, priv->dais[rtd->cpu_dai->id][substream->stream].itrcon, 0);
	s->occupied = 0;
	s->substream = NULL;
	substream->runtime->private_data = NULL;
	return ret;
}

static int mt8590_pcm_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct mt_stream *s = substream->runtime->private_data;
	pr_debug("%s()\n", __func__);
	if (s && s->ops && s->ops->hw_params)
		return s->ops->hw_params(substream, params);
	return 0;
}

static int mt8590_pcm_hw_free(struct snd_pcm_substream *substream)
{
	struct mt_stream *s = substream->runtime->private_data;
	pr_debug("%s()\n", __func__);
	if (s && s->ops && s->ops->hw_free)
		return s->ops->hw_free(substream);
	return 0;
}

static int mt8590_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct mt_stream *s = substream->runtime->private_data;
	pr_debug("%s()\n", __func__);
	if (s && s->ops && s->ops->prepare)
		return s->ops->prepare(substream);
	return 0;
}

static int mt8590_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct mt_stream *s = substream->runtime->private_data;
	pr_debug("%s()\n", __func__);
	if (s && s->ops && s->ops->trigger)
		return s->ops->trigger(substream, cmd);
	return 0;
}

static snd_pcm_uframes_t mt8590_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct mt_stream *s = substream->runtime->private_data;
	if (s && s->ops && s->ops->pointer)
		return s->ops->pointer(substream);
	return 0;
}

static struct snd_pcm_ops mt8590_pcm_ops = {
	.open = mt8590_pcm_open,
	.close = mt8590_pcm_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = mt8590_pcm_hw_params,
	.hw_free = mt8590_pcm_hw_free,
	.prepare = mt8590_pcm_prepare,
	.trigger = mt8590_pcm_trigger,
	.pointer = mt8590_pcm_pointer,
};

static int mt8590_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
	snd_pcm_lib_preallocate_pages_for_all(
		rtd->pcm, SNDRV_DMA_TYPE_DEV,
		NULL, 1024 * 1024 * 16, 1024 * 1024 * 16
	);
	return 0;
}

static void mt8590_pcm_free(struct snd_pcm *pcm)
{
	snd_pcm_lib_preallocate_free_for_all(pcm);
}

static inline void call_isr(struct mt_irq *irq)
{
	if (irq->isr)
		irq->isr(irq->s);
}

static irqreturn_t afe_isr(int irq, void *dev)
{
	enum audio_irq_id id;
	struct mt_private *priv = (struct mt_private *)dev;
	u32 status = afe_irq_status();
	/*
	if (lp_substream)
		status &= ~(0x1 << (IRQ_AFE_IRQ1 - IRQ_AFE_IRQ1));
	*/
	afe_irq_clear(status);

	if (lp_substream) {
		/* lp-audio high frequency timer */
		if (status & (0x1 << (IRQ_AFE_IRQ1 - IRQ_AFE_IRQ1)))
			lp_audio_isr();
	} else {
	for (id = IRQ_AFE_IRQ1; id <= IRQ_AFE_DMA; ++id) {
		/* AFE_IRQ1_IRQ to AFE_DMA_IRQ */
		if (status & (0x1 << (id - IRQ_AFE_IRQ1)))
			call_isr(&(priv->irqs[id]));
	}
	}
	return IRQ_HANDLED;
}

static irqreturn_t asys_isr(int irq, void *dev)
{
	enum audio_irq_id id;
	struct mt_private *priv;
	unsigned int idx;
	u32 status;
	status = asys_irq_status();
	if (lp_substream)
		status &= ~(0x1 << (lp_irq_id - IRQ_ASYS_IRQ1));
	asys_irq_clear(status);
	priv = dev;
	if ((target_acc_irq_id != IRQ_NUM) && (status & (0x1 << (target_acc_irq_id - IRQ_ASYS_IRQ1)))) {
		for(idx=0;idx<total_playback_num;idx++){
                	afe_memif_enable(rtc_multi[idx].memiftargetacc,1);
			audio_irq_enable(rtc_multi[idx].targetaccirqid,1);
		}
		if (total_playback_num > 0) {
			target_acc_result = TARGET_ACC_COMES | TARGET_ACC_OK;
            pr_notice("%s() TargetAccHasCome:Success!!!!\n", __func__);
		} else {
			target_acc_result = TARGET_ACC_COMES | TARGET_ACC_FAILED;
            pr_notice("%s() TargetAccHasCome:Fail!!!!\n", __func__);
		}
		total_playback_num = 0;
		audio_irq_enable(target_acc_irq_id, 0);
		asys_irq_release(target_acc_irq_id);
		target_acc_irq_id = IRQ_NUM; /* let trigger go */
	}
	#if 0
	if (lp_substream) {
		/* lp-audio high frequency timer */
		u32 pos = (0x1 << (lp_irq_id - IRQ_ASYS_IRQ1));
		if (status & pos) {
			status &= ~pos;
			lp_audio_isr();
		}
	}
	#endif
	for (id = IRQ_ASYS_IRQ1; id <= IRQ_ASYS_IRQ16; ++id) {
		/* ASYS_IRQ1_IRQ to ASYS_IRQ16_IRQ */
		if (status & (0x1 << (id - IRQ_ASYS_IRQ1)))
			call_isr(&(priv->irqs[id]));
	}
	return IRQ_HANDLED;
}

static void link_dai_and_stream(struct mt_private *priv,
				int dai_id, int dir, enum mt_stream_id stream_id, itrcon_action itrcon)
{
	priv->dais[dai_id][dir].s = &(priv->streams[stream_id]);
	priv->dais[dai_id][dir].itrcon = itrcon;
}

static void link_stream_and_irq(struct mt_private *priv,
				enum mt_stream_id stream_id,
				enum audio_irq_id irq_id, void (*isr)(struct mt_stream *))
{
	if (stream_id < MT_STREAM_NUM) {
		if ((irq_id < IRQ_NUM)) {
			if ((isr != NULL)) {
			priv->streams[stream_id].irq = &(priv->irqs[irq_id]);
			priv->irqs[irq_id].s = &(priv->streams[stream_id]);
			} else {
				priv->streams[stream_id].irq = NULL;
				priv->irqs[irq_id].s = NULL;
			}
		} else
			priv->streams[stream_id].irq = NULL;
	} else {
		if (irq_id < IRQ_NUM)
			priv->irqs[irq_id].s = NULL;
	}
	if (irq_id < IRQ_NUM)
		priv->irqs[irq_id].isr = isr;
}

static void link_stream_and_ops(struct mt_private *priv,
				enum mt_stream_id stream_id, struct snd_pcm_ops *ops)
{
	priv->streams[stream_id].ops = ops;
}

static void spdifrx_isr(struct mt_stream *s)
{
	afe_spdifrx_isr();
}

static int itrcon_i2s1out(int on)
{
	itrcon_connect(I12, O15, on);
	itrcon_connect(I13, O16, on);
	return 0;
}

static int itrcon_arb1_i2s1out(int on)
{
	itrcon_connect(I35, O15, on);
	itrcon_connect(I36, O16, on);
	return 0;
}

static int itrcon_i2s2out(int on)
{
	itrcon_connect(I14, O17, on);
	itrcon_connect(I15, O18, on);
	return 0;
}

static int itrcon_arb1_i2s2out(int on)
{
	itrcon_connect(I35, O17, on);
	itrcon_connect(I36, O18, on);
	return 0;
}

static int itrcon_i2s3out(int on)
{
	itrcon_connect(I16, O19, on);
	itrcon_connect(I17, O20, on);
	return 0;
}

static int itrcon_arb1_i2s3out(int on)
{
	itrcon_connect(I35, O19, on);
	itrcon_connect(I36, O20, on);
	return 0;
}

static int itrcon_i2s4out(int on)
{
	itrcon_connect(I18, O21, on);
	itrcon_connect(I19, O22, on);
	return 0;
}

static int itrcon_arb1_i2s4out(int on)
{
	itrcon_connect(I35, O21, on);
	itrcon_connect(I36, O22, on);
	return 0;
}

static int itrcon_i2s5out(int on)
{
	itrcon_connect(I20, O23, on);
	itrcon_connect(I21, O24, on);
	return 0;
}

static int itrcon_arb1_i2s5out(int on)
{
	itrcon_connect(I35, O23, on);
	itrcon_connect(I36, O24, on);
	return 0;
}

static int itrcon_i2s6out(int on)
{
	itrcon_connect(I22, O25, on);
	itrcon_connect(I23, O26, on);
	return 0;
}

static int itrcon_arb1_i2s6out(int on)
{
	itrcon_connect(I35, O25, on);
	itrcon_connect(I36, O26, on);
	return 0;
}

static int itrcon_immsnd(int on)
{
	itrcon_connect(I22, O15, on);
	itrcon_connect(I23, O16, on);
	return 0;
}

static int itrcon_i2smout(int on)
{
	switch (I2SM_NUM) {
	case 5:
		itrcon_connect(I20, O23, on);
		itrcon_connect(I21, O24, on);
		/* no break */
	case 4:
		itrcon_connect(I18, O21, on);
		itrcon_connect(I19, O22, on);
		/* no break */
	case 3:
		itrcon_connect(I16, O19, on);
		itrcon_connect(I17, O20, on);
		/* no break */
	case 2:
		itrcon_connect(I14, O17, on);
		itrcon_connect(I15, O18, on);
		/* no break */
	case 1: default:
		itrcon_connect(I12, O15, on);
		itrcon_connect(I13, O16, on);
		break;
	}
	return 0;
}

static int itrcon_i2s1in(int on)
{
	itrcon_connect(I00, O00, on);
	itrcon_connect(I01, O01, on);
	return 0;
}

static int itrcon_i2s2in(int on)
{
	itrcon_connect(I02, O02, on);
	itrcon_connect(I03, O03, on);
	return 0;
}

static int itrcon_i2s3in(int on)
{
	itrcon_connect(I04, O04, on);
	itrcon_connect(I05, O05, on);
	return 0;
}

static int itrcon_i2s4in(int on)
{
	itrcon_connect(I06, O06, on);
	itrcon_connect(I07, O07, on);
	return 0;
}

static int itrcon_i2s5in(int on)
{
	itrcon_connect(I08, O08, on);
	itrcon_connect(I09, O09, on);
	return 0;
}

static int itrcon_i2s6in(int on)
{
	itrcon_connect(I10, O10, on);
	itrcon_connect(I11, O11, on);
	return 0;
}

static int itrcon_btpcmout(int on)
{
	itrcon_connect(I35, O31, on);
	return 0;
}

static int itrcon_btpcmin(int on)
{
	itrcon_connect(I26, O14, on);
	return 0;
}

static int itrcon_mrgif_btout(int on)
{
	itrcon_connect(I35, O31, on);
	return 0;
}

static int itrcon_mrgif_btin(int on)
{
	itrcon_connect(I26, O14, on);
	return 0;
}

static int itrcon_mrgif_i2sout(int on)
{
	itrcon_connect(I22, O25, on);
	itrcon_connect(I23, O26, on);
	return 0;
}

static int itrcon_mrgif_i2sin(int on)
{
	itrcon_connect(I24, O00, on);
	itrcon_connect(I25, O01, on);
	return 0;
}

static int itrcon_dmic1(int on)
{
	itrcon_connect(I31, O12, on);
	itrcon_connect(I32, O13, on);
	return 0;
}

static int itrcon_dmic2(int on)
{
	itrcon_connect(I33, O32, on);
	itrcon_connect(I34, O33, on);
	return 0;
}

static void init_mt_private(struct mt_private *priv)
{
	enum mt_stream_id stream_id;
	enum audio_irq_id irq_id;
	static const char *names[MT_STREAM_NUM] = {
		/* playback streams */
		"MT_STREAM_DL1",
		"MT_STREAM_DL2",
		"MT_STREAM_DL3",
		"MT_STREAM_DL4",
		"MT_STREAM_DL5",
		"MT_STREAM_DL6",
		"MT_STREAM_DLM",
		"MT_STREAM_ARB1",
		"MT_STREAM_DSDR",
		"MT_STREAM_8CH_I2S_OUT",
		"MT_STREAM_IEC1",
		"MT_STREAM_IEC2",
		/* capture streams */
		"MT_STREAM_UL1",
		"MT_STREAM_UL2",
		"MT_STREAM_UL3",
		"MT_STREAM_UL4",
		"MT_STREAM_UL5",
		"MT_STREAM_UL6",
		"MT_STREAM_DAI",
		"MT_STREAM_MOD_PCM",
		"MT_STREAM_AWB",
		"MT_STREAM_AWB2",
		"MT_STREAM_DSDW",
		"MT_STREAM_MULTIIN",
	};
	static const itrcon_action i2sm_auxiliary_stereo_itrcon[6] = {
		itrcon_arb1_i2s1out,
		itrcon_arb1_i2s2out,
		itrcon_arb1_i2s3out,
		itrcon_arb1_i2s4out,
		itrcon_arb1_i2s5out,
		itrcon_arb1_i2s6out
	};
	if (!priv) {
		pr_err("%s() error: priv is NULL\n", __func__);
		return;
	}
	for (stream_id = MT_STREAM_DL1; stream_id < MT_STREAM_NUM; ++stream_id) {
		priv->streams[stream_id].id = stream_id;
		priv->streams[stream_id].name = names[stream_id];
	}
	for (irq_id = IRQ_AFE_IRQ1; irq_id < IRQ_NUM; ++irq_id)
		priv->irqs[irq_id].id = irq_id;
	/* 1. stream <-> ops */
	link_stream_and_ops(priv, MT_STREAM_DL1,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DL2,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DL3,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DL4,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DL5,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DL6,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DLM,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_ARB1,    &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DSDR,    &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_8CH_I2S_OUT, &hdmi_pcm_ops);
	link_stream_and_ops(priv, MT_STREAM_IEC1,    &hdmi_raw_ops);
	link_stream_and_ops(priv, MT_STREAM_IEC2,    &spdif_iec2_ops);
	link_stream_and_ops(priv, MT_STREAM_UL1,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_UL2,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_UL3,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_UL4,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_UL5,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_UL6,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DAI,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_MOD_PCM, &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_AWB,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_AWB2,    &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DSDW,    &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_MULTIIN, &memif_ops);
	/* 2. dai <-> stream */
	link_dai_and_stream(priv, MT8590_DAI_I2S1_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL1, itrcon_i2s1out);
	link_dai_and_stream(priv, MT8590_DAI_I2S2_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL2, itrcon_i2s2out);
	link_dai_and_stream(priv, MT8590_DAI_I2S3_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL3, itrcon_i2s3out);
	/*itrcon_connect(I18, O27, 1); hwgain interconnect*/
	/*itrcon_connect(I19, O28, 1);*/
	/*itrcon_connect(I27, O21, 1);*/
	/*itrcon_connect(I28, O22, 1);*/
	link_dai_and_stream(priv, MT8590_DAI_I2S4_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL4, itrcon_i2s4out);
	link_dai_and_stream(priv, MT8590_DAI_I2S5_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL5, itrcon_i2s5out);
	link_dai_and_stream(priv, MT8590_DAI_I2S6_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL6, itrcon_i2s6out);
	link_dai_and_stream(priv, MT8590_DAI_IMMSND_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL6, itrcon_immsnd);
	link_dai_and_stream(priv, MT8590_DAI_I2SM_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DLM, itrcon_i2smout);
	link_dai_and_stream(priv, I2SM_AUXILIARY_STEREO_ID, SNDRV_PCM_STREAM_PLAYBACK,
		MT_STREAM_ARB1, i2sm_auxiliary_stereo_itrcon[I2SM_AUXILIARY_STEREO_ID-MT8590_DAI_I2S1_ID]);
	link_dai_and_stream(priv, MT8590_DAI_I2S1_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL1, itrcon_i2s1in);
	link_dai_and_stream(priv, MT8590_DAI_I2S2_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL2, itrcon_i2s2in);
	link_dai_and_stream(priv, MT8590_DAI_I2S3_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL3, itrcon_i2s3in);
	link_dai_and_stream(priv, MT8590_DAI_I2S4_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL4, itrcon_i2s4in);
	link_dai_and_stream(priv, MT8590_DAI_I2S5_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL5, itrcon_i2s5in);
	link_dai_and_stream(priv, MT8590_DAI_I2S6_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL6, itrcon_i2s6in);
	link_dai_and_stream(priv, MT8590_DAI_SPDIF_OUT_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_IEC2, NULL);
	link_dai_and_stream(priv, MT8590_DAI_MULTI_IN_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_MULTIIN, NULL);
	link_dai_and_stream(priv, MT8590_DAI_HDMI_OUT_I2S_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_8CH_I2S_OUT, NULL);
	link_dai_and_stream(priv, MT8590_DAI_HDMI_OUT_IEC_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_IEC1, NULL);
	link_dai_and_stream(priv, MT8590_DAI_BTPCM_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_ARB1, itrcon_btpcmout);
	link_dai_and_stream(priv, MT8590_DAI_MRGIF_BT_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_ARB1, itrcon_mrgif_btout);
	link_dai_and_stream(priv, MT8590_DAI_MRGIF_I2S_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL6, itrcon_mrgif_i2sout);
	link_dai_and_stream(priv, MT8590_DAI_BTPCM_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_DAI, itrcon_btpcmin);
	link_dai_and_stream(priv, MT8590_DAI_MRGIF_BT_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_DAI, itrcon_mrgif_btin);
	link_dai_and_stream(priv, MT8590_DAI_MRGIF_I2S_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL1, itrcon_mrgif_i2sin);
	link_dai_and_stream(priv, MT8590_DAI_DSDENC_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DSDR, NULL);
	link_dai_and_stream(priv, MT8590_DAI_DSDENC_RECORD_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_DSDW, NULL);
	link_dai_and_stream(priv, MT8590_DAI_DMIC1_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_AWB, itrcon_dmic1);
	link_dai_and_stream(priv, MT8590_DAI_DMIC2_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_AWB2, itrcon_dmic2);
	/* 3. stream <-> irq */
	/*
	 * The streams below statically link to the specified irq.
	 */
	link_stream_and_irq(priv, MT_STREAM_AWB, IRQ_AFE_IRQ1, memif_isr);
	link_stream_and_irq(priv, MT_STREAM_AWB2, IRQ_AFE_IRQ2, memif_isr);
	link_stream_and_irq(priv, MT_STREAM_8CH_I2S_OUT, IRQ_AFE_HDMI, hdmi_pcm_isr);
	link_stream_and_irq(priv, MT_STREAM_IEC1, IRQ_AFE_SPDIF, hdmi_raw_isr);
	link_stream_and_irq(priv, MT_STREAM_IEC2, IRQ_AFE_SPDIF2, spdifout_isr);
	link_stream_and_irq(priv, MT_STREAM_MULTIIN, IRQ_AFE_MULTIIN, memif_isr);
	link_stream_and_irq(priv, MT_STREAM_NUM, IRQ_AFE_SPDIFIN, spdifrx_isr);
	/*
	 * The streams that don't link to any irq here
	 * will link to the asys irq at OPEN stage,
	 * and delink at CLOSE stage.
	 * The dynamic link/delink can save the irq resource.
	 */
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mt8590_audio_early_suspend(struct early_suspend *h)
{
	pr_debug("%s()\n", __func__);
	afe_enable(0);
}

static void mt8590_audio_late_resume(struct early_suspend *h)
{
	pr_debug("%s()\n", __func__);
	afe_enable(1);
}
#endif

#define AFE_MCU_IRQ_ID  (136)
#define ASYS_MCU_IRQ_ID (164)

static int mt8590_pcm_probe(struct snd_soc_platform *platform)
{
	struct mt_private *priv;
	int ret;
	dev_dbg(platform->dev, "%s()\n", __func__);
	priv = devm_kzalloc(platform->dev, sizeof(struct mt_private), GFP_KERNEL);
	if (!priv) {
		dev_err(platform->dev, "%s() can't allocate memory\n", __func__);
		return -ENOMEM;
	}
	afe_enable(1);
	init_mt_private(priv);
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(SND_SOC_MTK_BX8590P2_LINUX) && !defined(CONFIG_SND_SOC_ICX_AUDIO_STATIONARY)
	priv->es.suspend = mt8590_audio_early_suspend;
	priv->es.resume = mt8590_audio_late_resume;
	priv->es.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	register_early_suspend(&priv->es);
	priv->has_es = 1;
#endif
	snd_card_test = platform->card->snd_card;
	snd_soc_platform_set_drvdata(platform, priv);
	ret = request_irq(AFE_MCU_IRQ_ID, afe_isr, IRQF_TRIGGER_LOW, "afe-isr", priv);
	if (ret) {
		dev_err(platform->dev, "%s() can't register ISR for IRQ %u (ret=%i)\n",
			__func__, AFE_MCU_IRQ_ID, ret);
	}
	ret = request_irq(ASYS_MCU_IRQ_ID, asys_isr, IRQF_TRIGGER_LOW, "asys-isr", priv);
	if (ret) {
		dev_err(platform->dev, "%s() can't register ISR for IRQ %u (ret=%i)\n",
			__func__, ASYS_MCU_IRQ_ID, ret);
	}
#ifdef CONFIG_SND_ENABLE_ICX_DMA_DRIVE_MODE
	period_elapsed_notification_init();
#endif
	return 0;
}

static int mt8590_pcm_remove(struct snd_soc_platform *platform)
{
	struct mt_private *priv;
	priv = snd_soc_platform_get_drvdata(platform);
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(SND_SOC_MTK_BX8590P2_LINUX) && !defined(CONFIG_SND_SOC_ICX_AUDIO_STATIONARY)
	unregister_early_suspend(&priv->es);
#endif
	free_irq(AFE_MCU_IRQ_ID, priv);
	free_irq(ASYS_MCU_IRQ_ID, priv);
	devm_kfree(platform->dev, priv);
	itrcon_disconnectall();
	afe_enable(0);
	return 0;
}

static struct snd_soc_platform_driver mt8590_soc_platform_driver = {
	.probe = mt8590_pcm_probe,
	.remove = mt8590_pcm_remove,
	.pcm_new = mt8590_pcm_new,
	.pcm_free = mt8590_pcm_free,
	.ops = &mt8590_pcm_ops,
	.controls = mt8590_soc_controls,
	.num_controls = ARRAY_SIZE(mt8590_soc_controls),
};

static int mt8590_audio_probe(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "%s()\n", __func__);
	return snd_soc_register_platform(&pdev->dev, &mt8590_soc_platform_driver);
}

static int mt8590_audio_remove(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "%s()\n", __func__);
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

#ifdef CONFIG_PM

static int mt8590_audio_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct mt_private *priv;
	priv = dev_get_drvdata(&pdev->dev);
	dev_dbg(&pdev->dev, "%s() has_es=%d\n", __func__, priv->has_es);
	if (!priv->has_es)
		afe_enable(0);
	return 0;
}

static int mt8590_audio_resume(struct platform_device *pdev)
{
	struct mt_private *priv;
	priv = dev_get_drvdata(&pdev->dev);
	dev_dbg(&pdev->dev, "%s() has_es=%d\n", __func__, priv->has_es);
	if (!priv->has_es)
		afe_enable(1);
	return 0;
}

#else

#define mt8590_audio_suspend (NULL)
#define mt8590_audio_resume (NULL)

#endif

static struct platform_driver mt8590_audio = {
	.driver = {
		.name = "mt8590-audio",
		.owner = THIS_MODULE,
	},
	.probe = mt8590_audio_probe,
	.remove = mt8590_audio_remove,
	.suspend = mt8590_audio_suspend,
	.resume = mt8590_audio_resume,
};

module_platform_driver(mt8590_audio);

MODULE_DESCRIPTION("mt8590 audio driver");
MODULE_LICENSE("GPL");

