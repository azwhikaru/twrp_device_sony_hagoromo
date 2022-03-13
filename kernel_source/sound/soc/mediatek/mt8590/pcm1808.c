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

#include <linux/module.h>
#include <linux/init.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <mach/mt_gpio.h>

static int pcm1808_probe(struct snd_soc_codec *codec)
{
	pr_debug("%s()\n", __func__);

	/* I2S2 Pin setting */
	mt_set_gpio_mode(GPIO50, GPIO_MODE_01);
	mt_set_gpio_mode(GPIO51, GPIO_MODE_01);
	mt_set_gpio_mode(GPIO52, GPIO_MODE_01);
	mt_set_gpio_mode(GPIO188, GPIO_MODE_01);

	return 0;
}

static struct snd_soc_codec_driver pcm1808_driver = {
	.probe = pcm1808_probe,
};

static struct snd_soc_dai_driver pcm1808_dai_driver = {
	.name = "pcm1808-i2s",
	.capture = {
		    .stream_name = "pcm1808-i2s-capture",
		    .channels_min = 1,
		    .channels_max = 2,
		    .rates = SNDRV_PCM_RATE_8000_192000,
		    .formats = (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S32_LE),
		    },
};

static int pcm1808_platform_probe(struct platform_device *pdev)
{
	int ret;
	ret = snd_soc_register_codec(&pdev->dev, &pcm1808_driver, &pcm1808_dai_driver, 1);
	pr_debug("%s() call snd_soc_register_codec() return %d\n", __func__, ret);
	return ret;
}

static int pcm1808_platform_remove(struct platform_device *pdev)
{
	pr_debug("%s()\n", __func__);
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static struct platform_driver pcm1808_platform_driver = {
	.driver = {
		   .name = "pcm1808",
		   .owner = THIS_MODULE,
		   },
	.probe = pcm1808_platform_probe,
	.remove = pcm1808_platform_remove
};

module_platform_driver(pcm1808_platform_driver);

/* Module information */
MODULE_DESCRIPTION("ASoC pcm1808 codec driver");
MODULE_LICENSE("GPL");
