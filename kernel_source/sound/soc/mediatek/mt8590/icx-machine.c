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

#ifdef CONFIG_REGMON_DEBUG
#include "mt8590-afe-regmon.h"
#endif

#include "icx-machine-links.c"

static struct snd_soc_card sony_soc_card = {
    .name = "sony-soc-card",
    .dai_link = sony_dai_links,
    .num_links = ARRAY_SIZE(sony_dai_links),
};

static int icx_machine_probe(struct platform_device *pdev)
{
	struct snd_soc_card *sony_card = &sony_soc_card;

#ifdef CONFIG_REGMON_DEBUG
	afe_regmon_initialize();
#endif
	sony_card->dev = &pdev->dev;
	return snd_soc_register_card(sony_card);
}

static int icx_machine_remove(struct platform_device *pdev)
{
	struct snd_soc_card *sony_card = platform_get_drvdata(pdev);

#ifdef CONFIG_REGMON_DEBUG
        afe_regmon_finalize();
#endif

	return snd_soc_unregister_card(sony_card);
}

static struct platform_driver icx_machine = {
	.driver = {
		.name = "icx-machine",
		.owner = THIS_MODULE,
	},
	.probe = icx_machine_probe,
	.remove = icx_machine_remove
};

#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
static int __init icx_machine_init(void)
{
	int rv;

	rv=platform_driver_register(&icx_machine);
	if(rv!=0) {
		printk(KERN_ERR "icx_machine_init(): code %d error occurred.\n",rv);
		return(rv);
	}

	return(0);
}
late_initcall(icx_machine_init);
#else
module_platform_driver(icx_machine);
#endif

/* Module information */
MODULE_DESCRIPTION("mt8590 icx machine driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("mt8590 icx soc card");
