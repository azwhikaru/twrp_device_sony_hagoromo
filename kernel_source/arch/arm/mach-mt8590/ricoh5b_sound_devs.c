/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* ricoh5b driver.
 *
 * Copyright 2015 Sony Corporation
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/device.h>
#include <linux/platform_device.h>

static struct platform_device ricoh5b_snd_device = {
	.name = "ricoh5b_bbb",
	.id = 0,
};

int __init ricoh5b_sound_init(void)
{
    int ret;

    ret = platform_device_register(&ricoh5b_snd_device);
    if (ret != 0) {
        pr_err("%s: Fail to platform_device_register. , ret=%d\n", __func__, ret);
    }

    return 0;
}

arch_initcall(ricoh5b_sound_init);

