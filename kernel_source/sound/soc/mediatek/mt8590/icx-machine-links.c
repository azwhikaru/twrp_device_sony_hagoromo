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

#include <mach/upmu_hw.h>
#include <mach/mt_gpio.h>
#include <linux/delay.h>
#include "mt8590-dai.h"
#include "mt8590-private.h"

#include <sound/cxd3774gf.h>
#include <sound/cxd3776er.h>
#include <sound/cxd3778gf.h>

static int dsd_icx_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
        struct snd_soc_pcm_runtime *rtd = substream->private_data;
        struct snd_soc_dai *codec_dai = rtd->codec_dai;
        struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
        pr_debug("%s()\n", __func__);

        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
                /* codec slave, mt8590 master */
                unsigned int fmt =
                    SND_SOC_DAIFMT_PDM | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_CONT;
                unsigned int mclk_rate = 256 * 44100;
                unsigned int rate = params_rate(params) == 176400 ? 128 * 44100 : 64 * 44100;   /* data rate */
                unsigned int div_mclk_to_bck;
                div_mclk_to_bck = mclk_rate / rate;

		/* codec DSD slave */
                snd_soc_dai_set_fmt(codec_dai, fmt);

		snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);

                /* mt8590 mclk */
                snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
                /* mt8590 bck */
                snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
                /* mt8590 lrck */
                snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, 0); /* no lrck */
                /* mt8590 DSD master */
                snd_soc_dai_set_fmt(cpu_dai, fmt);
        }

        return 0;
}

static int pcm_master_icx_rate_hw_params(
    struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
        struct snd_soc_pcm_runtime *rtd = substream->private_data;
        struct snd_soc_dai *codec_dai = rtd->codec_dai;
        struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
        /* codec slave, mt8590 master */
        unsigned int fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_CONT;
        unsigned int mclk_rate;
        unsigned int rate = params_rate(params);        /* data rate */
        unsigned int div_mclk_to_bck = rate > 192000 ? 2 : 4;
        unsigned int div_bck_to_lrck = 64;
        pr_debug("%s() rate = %d\n", __func__, rate);
        mclk_rate = rate * div_bck_to_lrck * div_mclk_to_bck;
        /* codec mclk */
        snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
        /* codec slave */
        snd_soc_dai_set_fmt(codec_dai, fmt);
        /* mt8590 mclk */
        snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
        /* mt8590 bck */
        snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
        /* mt8590 lrck */
        snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
        /* mt8590 master */
        snd_soc_dai_set_fmt(cpu_dai, fmt);
        return 0;

}

/*
 * For cxd3778gf special seqpuecne
 * slave mode by alsa rate
 */
static int pcm_slave_icx_rate_hw_params(
    struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
        struct snd_soc_pcm_runtime *rtd = substream->private_data;
        struct snd_soc_dai *codec_dai = rtd->codec_dai;
        struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
        struct mt_stream *s = substream->runtime->private_data;
        /* codec slave, mt8590 master */
        unsigned int fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CONT;
        unsigned int mclk_rate;
        unsigned int rate = params_rate(params);        /* data rate */
        unsigned int div_mclk_to_bck = rate > 192000 ? 2 : 4;
        unsigned int div_bck_to_lrck = 64;
        pr_debug("%s() rate = %d\n", __func__, rate);

        mclk_rate = rate * div_bck_to_lrck * div_mclk_to_bck;
        /* codec mclk */
        snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
        /* mt8590 mclk */
        snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
        /* mt8590 bck */
        snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
        /* mt8590 lrck */
        snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
        /* mt8590 master */

	fmt |= SND_SOC_DAIFMT_CBM_CFM;
	/* codec master */
	snd_soc_dai_set_fmt(codec_dai, fmt);
	fmt |= SLAVE_USE_ASRC_NO;
	snd_soc_dai_set_fmt(cpu_dai, fmt);
	s->use_i2s_slave_clock = 1;

	return 0;
}

/*
 * For cxd3778gf special seqpuecne
 * lp slave mode by alsa rate
 */
static int lp_slave_icx_rate_hw_params(
    struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct mt_lp_private *priv = snd_soc_platform_get_drvdata(rtd->platform);
	/* codec slave, mt8590 master */
	unsigned int fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CONT;
	unsigned int mclk_rate;
	unsigned int rate = params_rate(params);        /* data rate */
	unsigned int div_mclk_to_bck = rate > 192000 ? 2 : 4;
	unsigned int div_bck_to_lrck = 64;
	pr_debug("%s() rate = %d\n", __func__, rate);

	mclk_rate = rate * div_bck_to_lrck * div_mclk_to_bck;
	/* codec mclk */
	snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
	/* mt8590 mclk */
	snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
	/* mt8590 bck */
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
	/* mt8590 lrck */
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
	/* mt8590 master */

	fmt |= SND_SOC_DAIFMT_CBM_CFM;
	/* codec master */
	snd_soc_dai_set_fmt(codec_dai, fmt);
	fmt |= SLAVE_USE_ASRC_NO;
	snd_soc_dai_set_fmt(cpu_dai, fmt);
	priv->use_i2s_slave_clock = 1;

	return 0;
}

static struct snd_soc_ops stream_dsd_icx_ops = {
	.hw_params = dsd_icx_hw_params,
};
static struct snd_soc_ops stream_icx_ops = {
    .hw_params = pcm_master_icx_rate_hw_params
};

static struct snd_soc_ops stream_icx_slave_ops = {
    .hw_params = pcm_slave_icx_rate_hw_params
};

static struct snd_soc_ops stream_icx_lp_slave_ops = {
    .hw_params = lp_slave_icx_rate_hw_params
};

static struct snd_soc_dai_link sony_dai_links[] = {
#ifdef CONFIG_SND_SOC_ICX_AUDIO_STATIONARY
/* sony daughter board links */
    {
         .name = "CXD3776ER-I2S1",
         .stream_name = "cxd3776er-i2s1-out",
         .cpu_dai_name = "mt8590-i2s1",
         .platform_name = "mt8590-audio",
         .codec_name = CXD3776ER_CODEC_NAME,
         .codec_dai_name = CXD3776ER_ICX_DAI_NAME,
        .dai_fmt = SND_SOC_DAIFMT_I2S
                   | SND_SOC_DAIFMT_CBS_CFS
                   | SND_SOC_DAIFMT_NB_NF,
         .ops = &stream_icx_ops,
    },
    {
         .name = "CXD3776ER-I2SMulti",
         .stream_name = "cxd3776er-i2s-multi-out",
         .cpu_dai_name = "mt8590-i2sm",
         .platform_name = "mt8590-audio",
         .codec_name = CXD3776ER_CODEC_NAME,
         .codec_dai_name = CXD3776ER_ICX_DAI_NAME,
        .dai_fmt = SND_SOC_DAIFMT_I2S
                   | SND_SOC_DAIFMT_CBS_CFS
                   | SND_SOC_DAIFMT_NB_NF,
         .ops = &stream_icx_ops,
    },
    {
        .name = "pcm1808-pcm-in3",
        .stream_name = "pcm-in3",
        .platform_name = "mt8590-audio",
        .cpu_dai_name = "mt8590-i2s3",
        .codec_dai_name = "pcm1808-i2s",
        .codec_name = "pcm1808",
        .dai_fmt = SND_SOC_DAIFMT_I2S
                 | SND_SOC_DAIFMT_CBS_CFS
                 | SND_SOC_DAIFMT_GATED,
        .ops = &stream_icx_ops,
    },
#endif
#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE
    {
         .name = "CXD3774GF_ICX",
         .stream_name = "cxd3774gf-hires-out",
         .cpu_dai_name = "mt8590-i2s2",
         .platform_name = "mt8590-audio",
         .codec_name = CXD3774GF_CODEC_NAME,
         .codec_dai_name = CXD3774GF_ICX_DAI_NAME,
        .dai_fmt = SND_SOC_DAIFMT_I2S
                   | SND_SOC_DAIFMT_CBS_CFS
                   | SND_SOC_DAIFMT_NB_NF,
         .ops = &stream_icx_ops,
     },
     {
         .name = "CXD3774GF_STD",
         .stream_name = "cxd3774gf-standard",
         .cpu_dai_name = "mt8590-i2s1",
         .platform_name = "mt8590-audio",
         .codec_name = CXD3774GF_CODEC_NAME,
         .codec_dai_name = CXD3774GF_STD_DAI_NAME,
        .dai_fmt = SND_SOC_DAIFMT_I2S
                   | SND_SOC_DAIFMT_CBS_CFS
                   | SND_SOC_DAIFMT_NB_NF,
         .ops = &stream_icx_ops,
     },
#endif
#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
    {
         .name = "CXD3778GF_ICX",
         .stream_name = "cxd3778gf-hires-out",
         .cpu_dai_name = "mt8590-i2s1",
         .platform_name = "mt8590-audio",
         .codec_name = CXD3778GF_CODEC_NAME,
         .codec_dai_name = CXD3778GF_ICX_DAI_NAME,
        .dai_fmt = SND_SOC_DAIFMT_I2S
		   | SND_SOC_DAIFMT_CBM_CFM
                   | SND_SOC_DAIFMT_NB_NF,
         .ops = &stream_icx_slave_ops,
     },
     {
         .name = "CXD3778GF_STD",
         .stream_name = "cxd3778gf-standard",
         .cpu_dai_name = "mt8590-i2s2",
         .platform_name = "mt8590-audio",
         .codec_name = CXD3778GF_CODEC_NAME,
         .codec_dai_name = CXD3778GF_STD_DAI_NAME,
        .dai_fmt = SND_SOC_DAIFMT_I2S
                   | SND_SOC_DAIFMT_CBS_CFS
                   | SND_SOC_DAIFMT_NB_NF,
         .ops = &stream_icx_ops,
     },
     {
           .name = "CXD3778GF_dsdenc",
           .stream_name = "dsdenc",
           .platform_name = "mt8590-audio",
           .cpu_dai_name = "mt8590-dsdenc",
           .codec_dai_name = CXD3778GF_ICX_DAI_NAME,
           .codec_name = CXD3778GF_CODEC_NAME,
           .dai_fmt = SND_SOC_DAIFMT_PDM | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
           .ops = &stream_dsd_icx_ops,
     },

     {
         .name = "CXD3778GF_ICX_DSD",
         .stream_name = "cxd3778gf-dsd-out",
         .cpu_dai_name = "mt8590-i2s1",
         .platform_name = "mt8590-audio",
         .codec_name = CXD3778GF_CODEC_NAME,
         .codec_dai_name = CXD3778GF_ICX_DAI_NAME,
        .dai_fmt = SND_SOC_DAIFMT_PDM
                   | SND_SOC_DAIFMT_CBS_CFS
                   | SND_SOC_DAIFMT_NB_NF,
         .ops = &stream_dsd_icx_ops,
     },
     {
         .name = "CXD3778GF_ICX_lowpower",
         .stream_name = "cxd3778gf-icx-lowpower",
         .cpu_dai_name = "mt8590-i2s1",
         .platform_name = "mt8590-lp-audio",
         .codec_name = CXD3778GF_CODEC_NAME,
         .codec_dai_name = CXD3778GF_ICX_DAI_NAME,
         .dai_fmt = SND_SOC_DAIFMT_I2S
		   | SND_SOC_DAIFMT_CBM_CFM
                   | SND_SOC_DAIFMT_NB_NF,
         .ops = &stream_icx_lp_slave_ops,
     },
     {
         .name = "CXD3778GF_ICX_lowpower_test",
         .stream_name = "cxd3778gf-icx-lowpower_test",
         .cpu_dai_name = "mt8590-i2s1",
         .platform_name = "mt8590-lp-audio",
         .codec_name = CXD3778GF_CODEC_NAME,
         .codec_dai_name = CXD3778GF_ICX_DAI_NAME,
         .dai_fmt = SND_SOC_DAIFMT_I2S
		   | SND_SOC_DAIFMT_CBS_CFS
                   | SND_SOC_DAIFMT_NB_NF,
         .ops = &stream_icx_ops,
     },
#endif
};
