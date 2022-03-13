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
#include <sound/soc.h>

struct demo_factory_mode {
	int i2s_passthrough;
	int spdif_passthrough;
	int dmic_passthrough;
};

struct demo_private {
	struct demo_factory_mode factory_mode;
};

#include "demo-machine-links.c"
#include "demo-machine-controls.c"

static struct snd_soc_card demo_soc_card = {
	.name = "demo-soc-card",
	.dai_link = demo_dai_links,
	.num_links = ARRAY_SIZE(demo_dai_links),
	.controls = demo_controls,
	.num_controls = ARRAY_SIZE(demo_controls),
};

static int demo_machine_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &demo_soc_card;

	struct demo_private *priv = 
		devm_kzalloc(&pdev->dev, sizeof(struct demo_private), GFP_KERNEL);
	pr_debug("%s()\n", __func__);
	if (priv == NULL)
		return -ENOMEM;

	card->dev = &pdev->dev;
	snd_soc_card_set_drvdata(card, priv);
	return snd_soc_register_card(card);
}

static int demo_machine_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	devm_kfree(&pdev->dev, snd_soc_card_get_drvdata(card));
	return snd_soc_unregister_card(card);
}

static struct platform_driver demo_machine = {
	.driver = {
		.name = "demo-machine",
		.owner = THIS_MODULE,
	},
	.probe = demo_machine_probe,
	.remove = demo_machine_remove
};

#ifndef CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
module_platform_driver(demo_machine);
#endif

/* Module information */
MODULE_DESCRIPTION("mt8590 demo machine driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("mt8590 demo soc card");
