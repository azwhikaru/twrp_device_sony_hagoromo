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

static int dsd_prepare(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	pr_debug("%s()\n", __func__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/* PCM1795 DSD playback */
		/* codec slave, mt8590 master */
		unsigned int fmt =
			SND_SOC_DAIFMT_PDM | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_CONT;
		unsigned int mclk_rate = 256 * 44100;
		/* use I2S0_LRCK as GPIO to reset PCM1795 */
		mt_set_gpio_mode(GPIO73, GPIO_MODE_GPIO);
		mt_set_gpio_dir(GPIO73, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO73, GPIO_OUT_ZERO);
		usleep_range(10000, 11000);
		mt_set_gpio_out(GPIO73, GPIO_OUT_ONE);
		msleep(20);
		/* codec DSD slave */
		snd_soc_dai_set_fmt(codec_dai, fmt);
		/* codec mclk */
		snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
	}
	return 0;
}

static int dsd_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	pr_debug("%s()\n", __func__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/* PCM1795 DSD playback */
		/* codec slave, mt8590 master */
		unsigned int fmt =
			SND_SOC_DAIFMT_PDM | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_CONT;
		unsigned int mclk_rate = 256 * 44100;
		unsigned int rate = params_rate(params) == 176400 ? 128 * 44100 : 64 * 44100;	/* data rate */
		unsigned int div_mclk_to_bck;
		div_mclk_to_bck = mclk_rate / rate;
		/* mt8590 mclk */
		snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
		/* mt8590 bck */
		snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
		/* mt8590 lrck */
		snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, 0);	/* no lrck */
		/* mt8590 DSD master */
		snd_soc_dai_set_fmt(cpu_dai, fmt);
	} else {
		/* PCM4202 DSD capture */
		/* codec master, mt8590 slave */
		unsigned int fmt =
			SND_SOC_DAIFMT_PDM | SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_CONT;
		/* codec DSD master */
		snd_soc_dai_set_fmt(codec_dai, fmt);
		/* mt8590 DSD slave */
		snd_soc_dai_set_fmt(cpu_dai, fmt);
	}
	return 0;
}

static int pcm_master_data_rate_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	/* codec slave, mt8590 master */
	unsigned int fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_CONT;
	unsigned int mclk_rate;
	unsigned int rate = params_rate(params);	/* data rate */
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

static int pcm_master_fixed_rate_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	unsigned int mclk_rate;
	unsigned int div_mclk_to_bck = 4;
	unsigned int div_bck_to_lrck = 64;
	unsigned int rate = 48000;	/* fixed rate */
	/* codec slave, mt8590 master */
	unsigned int fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_CONT;
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

static int pcm_slave_fixed_rate_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	unsigned int mclk_rate;
	unsigned int div_mclk_to_bck = 8;
	unsigned int div_bck_to_lrck = 64;
	unsigned int rate = 96000;	/* connect OSC(49.152M) to PCM1861, set PCM1861 to master 512fs */
	/* codec master, mt8590 slave */
	unsigned int fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_CONT;
	mclk_rate = rate * div_bck_to_lrck * div_mclk_to_bck;
	/* codec mclk */
	snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
	/* codec master */
	snd_soc_dai_set_fmt(codec_dai, fmt);
	/* mt8590 mclk */
	snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
	/* mt8590 bck */
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
	/* mt8590 lrck */
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
	/* mt8590 slave */
	snd_soc_dai_set_fmt(cpu_dai, fmt | SLAVE_USE_ASRC_YES);	/* slave PCM can use asrc */
	return 0;
}

static int dop_slave_fixed_rate_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct mt_stream *s = substream->runtime->private_data;
	unsigned int mclk_rate;
	unsigned int div_mclk_to_bck = 4;
	unsigned int div_bck_to_lrck = 64;
	unsigned int rate = 88200;	/* fixed rate */
	/* codec master, mt8590 slave */
	unsigned int fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_CONT;
	mclk_rate = rate * div_bck_to_lrck * div_mclk_to_bck;
	/* codec mclk */
	snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
	/* codec master */
	snd_soc_dai_set_fmt(codec_dai, fmt);
	/* mt8590 mclk */
	snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
	/* mt8590 bck */
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
	/* mt8590 lrck */
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
	/* mt8590 slave */
	snd_soc_dai_set_fmt(cpu_dai, fmt | SLAVE_USE_ASRC_NO);	/* slave DoP can't use asrc */
	s->use_i2s_slave_clock = 1;
	return 0;
}


static int lp_slave_no_asrc_fixed_rate_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct mt_lp_private *priv = snd_soc_platform_get_drvdata(rtd->platform);
	unsigned int mclk_rate;
	unsigned int div_bck_to_lrck = 64;
	unsigned int rate = params_rate(params);/*no fixed rate */
	unsigned int div_mclk_to_bck = rate > 192000 ? 2 : 4;
	/* codec master, mt8590 slave */
	unsigned int fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_CONT;
	mclk_rate = rate * div_bck_to_lrck * div_mclk_to_bck;
	/* codec mclk */
	snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
	/* codec master */
	snd_soc_dai_set_fmt(codec_dai, fmt);
	/* mt8590 mclk */
	snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
	/* mt8590 bck */
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
	/* mt8590 lrck */
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
	/* mt8590 slave */
	snd_soc_dai_set_fmt(cpu_dai, fmt | SLAVE_USE_ASRC_NO);	/* slave DoP can't use asrc */
	priv->use_i2s_slave_clock = 1;
	return 0;
}

static int btpcm_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	unsigned int div_bck_to_lrck = 64;
	unsigned int fmt = SND_SOC_DAIFMT_DSP_A | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_CONT;
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
	/* codec DSD master */
	snd_soc_dai_set_fmt(codec_dai, fmt);
	/* mt8590 DSD slave */
	snd_soc_dai_set_fmt(cpu_dai, fmt);
	if ((fmt & SND_SOC_DAIFMT_MASTER_MASK) == SND_SOC_DAIFMT_CBM_CFM) {
		unsigned int rate = params_rate(params);
		/* codec rate */
		snd_soc_dai_set_sysclk(codec_dai, 0, rate, SND_SOC_CLOCK_IN);
		/* mt8590 rate */
		snd_soc_dai_set_sysclk(cpu_dai, 0, rate, SND_SOC_CLOCK_OUT);
	}
	return 0;
}

static struct snd_soc_ops stream_dsd_ops = {
        .hw_params = dsd_hw_params,
        .prepare = dsd_prepare,
};

static struct snd_soc_ops stream_pcm_master_data_rate_ops = {
	.hw_params = pcm_master_data_rate_hw_params
};

static struct snd_soc_ops stream_pcm_master_fixed_rate_ops = {
	.hw_params = pcm_master_fixed_rate_hw_params
};

static struct snd_soc_ops stream_pcm_slave_fixed_rate_ops = {
	.hw_params = pcm_slave_fixed_rate_hw_params
};

static struct snd_soc_ops stream_lp_slave_no_asrc_fixed_rate_ops = {
	.hw_params = lp_slave_no_asrc_fixed_rate_hw_params
};

static struct snd_soc_ops stream_btpcm_ops = {
	.hw_params = btpcm_hw_params
};

/* Digital audio interface glue - connects codec <---> CPU */
static struct snd_soc_dai_link demo_dai_links[] = {
	/* demo daughter board(MT8590D1Vx) links */
	{
		.name = "demo-dsd-out",
		.stream_name = "dsd-out",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-i2s1",
		.codec_dai_name = "pcm1795-i2s",
		.codec_name = "pcm1795.0-004c",
		.dai_fmt = SND_SOC_DAIFMT_PDM | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_dsd_ops,
	},
	{
		.name = "demo-dsd-in",
		.stream_name = "dsd-in",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-i2s2",
		.codec_dai_name = "pcm4202-i2s",
		.codec_name = "pcm4202",
		.dai_fmt = SND_SOC_DAIFMT_PDM | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_dsd_ops,
	},
	{
		.name = "demo-dmic1",
		.stream_name = "dmic1",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-dmic1",
		/* dummy codec is temporary, please change to real codec */
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
	},
	{
		.name = "demo-dmic2",
		.stream_name = "dmic2",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-dmic2",
		/* dummy codec is temporary, please change to real codec */
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
	},

#if defined(SND_SOC_MTK_BT8590M1_EMMC)\
|| defined(SND_SOC_MTK_BX8590M1_EMMC)\
|| defined(SND_SOC_MTK_BX8590M1_LDVT)\
|| defined(SND_SOC_MTK_BX8590M1_LINUX)
	/* M1 board has no i2s4~6 */
    {
        .name = "demo-pcm-out1",
        .stream_name = "pcm-out1",
        .platform_name = "mt8590-audio",
        .cpu_dai_name = "mt8590-i2s1",
        .codec_dai_name = "pcm5102a-i2s",
        .codec_name = "pcm5102a",
        .dai_fmt = SND_SOC_DAIFMT_I2S
                 | SND_SOC_DAIFMT_CBS_CFS
                 | SND_SOC_DAIFMT_GATED,
        .ops = &stream_pcm_master_data_rate_ops
    },
    {
        .name = "demo-pcm-in1",
        .stream_name = "pcm-in1",
        .platform_name = "mt8590-audio",
        .cpu_dai_name = "mt8590-i2s1",
        .codec_dai_name = "pcm1861-i2s",
        .codec_name = "pcm1861",
        .dai_fmt = SND_SOC_DAIFMT_I2S
                 | SND_SOC_DAIFMT_CBS_CFS
                 | SND_SOC_DAIFMT_GATED,
        .ops = &stream_pcm_master_data_rate_ops
    },
    {
        .name = "demo-pcm-out2",
        .stream_name = "pcm-out2",
        .platform_name = "mt8590-audio",
        .cpu_dai_name = "mt8590-i2s2",
        .codec_dai_name = "pcm5102a-i2s",
        .codec_name = "pcm5102a",
        .dai_fmt = SND_SOC_DAIFMT_I2S
                 | SND_SOC_DAIFMT_CBS_CFS
                 | SND_SOC_DAIFMT_GATED,
        .ops = &stream_pcm_master_fixed_rate_ops
    },
    {
        .name = "demo-pcm-in2",
        .stream_name = "pcm-in2",
        .platform_name = "mt8590-audio",
        .cpu_dai_name = "mt8590-i2s2",
        .codec_dai_name = "pcm1861-i2s",
        .codec_name = "pcm1861",
        .dai_fmt = SND_SOC_DAIFMT_I2S
                 | SND_SOC_DAIFMT_CBS_CFS
                 | SND_SOC_DAIFMT_GATED,
        .ops = &stream_pcm_master_fixed_rate_ops
    },
    {
        .name = "demo-pcm-out3",
        .stream_name = "pcm-out3",
        .platform_name = "mt8590-audio",
        .cpu_dai_name = "mt8590-i2s3",
        .codec_dai_name = "pcm5102a-i2s",
        .codec_name = "pcm5102a",
        .dai_fmt = SND_SOC_DAIFMT_I2S
                 | SND_SOC_DAIFMT_CBS_CFS
                 | SND_SOC_DAIFMT_GATED,
        .ops = &stream_pcm_slave_fixed_rate_ops
    },
    {
        .name = "demo-pcm-in3",
        .stream_name = "pcm-in3",
        .platform_name = "mt8590-audio",
        .cpu_dai_name = "mt8590-i2s3",
        .codec_dai_name = "pcm1861-i2s",
        .codec_name = "pcm1861",
        .dai_fmt = SND_SOC_DAIFMT_I2S
                 | SND_SOC_DAIFMT_CBS_CFS
                 | SND_SOC_DAIFMT_GATED,
        .ops = &stream_pcm_slave_fixed_rate_ops
    },
#else
	{
		.name = "demo-pcm-out1",
		.stream_name = "pcm-out1",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-i2s4",
		.codec_dai_name = "pcm5102a-i2s",
		.codec_name = "pcm5102a",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_pcm_master_data_rate_ops
	},
	{
		.name = "demo-pcm-in1",
		.stream_name = "pcm-in1",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-i2s4",
		.codec_dai_name = "pcm1861-i2s",
		.codec_name = "pcm1861",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_pcm_master_data_rate_ops
	},
	{
		.name = "demo-pcm-out2",
		.stream_name = "pcm-out2",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-i2s5",
		.codec_dai_name = "pcm5102a-i2s",
		.codec_name = "pcm5102a",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_pcm_master_fixed_rate_ops
	},
	{
		.name = "demo-pcm-in2",
		.stream_name = "pcm-in2",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-i2s5",
		.codec_dai_name = "pcm1861-i2s",
		.codec_name = "pcm1861",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_pcm_master_fixed_rate_ops
	},
	{
		.name = "demo-pcm-out3",
		.stream_name = "pcm-out3",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-i2s6",
		.codec_dai_name = "pcm5102a-i2s",
		.codec_name = "pcm5102a",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_pcm_master_data_rate_ops
	},
	{
		.name = "demo-pcm-in3",
		.stream_name = "pcm-in3",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-i2s6",
		.codec_dai_name = "pcm1861-i2s",
		.codec_name = "pcm1861",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_pcm_slave_fixed_rate_ops
	},
#endif
	/* demo main board(MT8590MxVx MT8590PxVx) links */
	{
		.name = "demo-multi-in",
		.stream_name = "multi-in",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-multi-in",
		/* dummy codec is temporary, please change to real codec */
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
	},
	{
		.name = "demo-hdmi-pcm-out",
		.stream_name = "hdmi-pcm-out",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-hdmi-pcm-out",
		/* dummy codec is temporary, please change to real codec */
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
	},
	{
		.name = "demo-hdmi-rawpcm-out",
		.stream_name = "hdmi-rawpcm-out",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-hdmi-rawpcm-out",
		/* dummy codec is temporary, please change to real codec */
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
	},
	{
		.name = "demo-spdif-out",
		.stream_name = "spdif-out",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-spdif-out",
		/* dummy codec is temporary, please change to real codec */
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
	},
	{
		/* merge interface's PCM in/out */
		.name = "demo-mrgbt",
		.stream_name = "merge-bt",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-mrgbt",
		/* dummy codec is temporary, please change to real codec */
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
	},

	{
		/* merge interface's PCM in/out */
		.name = "demo-mrgi2s",
		.stream_name = "merge-i2s",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-mrgi2s",
		/* dummy codec is temporary, please change to real codec */
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
	},
	{
		/* PCM interface's in/out */
		.name = "demo-btpcm",
		.stream_name = "demo-bluetooth",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-btpcm",
		/* dummy codec is temporary, please change to real codec */
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
		.dai_fmt = SND_SOC_DAIFMT_DSP_A | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_btpcm_ops
	},
	{
		.name = "demo-dsdenc",
		.stream_name = "dsdenc",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-dsdenc",
		.codec_dai_name = "pcm1795-i2s",
		.codec_name = "pcm1795.0-004c",
		.dai_fmt = SND_SOC_DAIFMT_PDM | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_dsd_ops,
	},
	{
		.name = "demo-dsdenc-record",
		.stream_name = "dsdenc-record",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-dsdenc-record",
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
	},
	/* demo board(MT8590D1Vx) doesn't support pcm multi-channel out
	 * this link is for test */
	{
		.name = "demo-pcm-out-multich",
		.stream_name = "pcm-out-multich",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-i2sm",
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
		.ops = &stream_pcm_master_data_rate_ops
	},
	{
		.name = "demo-lowpower-audio1",
		.stream_name = "lowpower-audio1",
		.platform_name = "mt8590-lp-audio",
		.cpu_dai_name = "mt8590-i2s1",
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
		.ops = &stream_pcm_master_data_rate_ops
	},
	{
		.name = "demo-lowpower-audio1-slave",
		.stream_name = "lowpower-audio1-slave",
		.platform_name = "mt8590-lp-audio",
		.cpu_dai_name = "mt8590-i2s1",
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
		.ops = &stream_lp_slave_no_asrc_fixed_rate_ops
	},
	{
		.name = "demo-lowpower-audio2",
		.stream_name = "lowpower-audio2",
		.platform_name = "mt8590-lp-audio",
		.cpu_dai_name = "mt8590-i2s2",
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
		.ops = &stream_pcm_master_data_rate_ops
	},
	{
		.name = "demo-lowpower-audio2-slave",
		.stream_name = "lowpower-audio2-slave",
		.platform_name = "mt8590-lp-audio",
		.cpu_dai_name = "mt8590-i2s2",
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
		.ops = &stream_lp_slave_no_asrc_fixed_rate_ops
	},
	{
		.name = "demo-immsnd",
		.stream_name = "immsnd",
		.platform_name = "mt8590-audio",
		.cpu_dai_name = "mt8590-immsnd",
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_pcm_master_data_rate_ops
	}
	/* add other link here */
};
