/* DVD 89G Driver.
 *
 * Copyright 2016 Sony Corporation
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

static struct platform_device dvd_89g_device = {
	.name = "dvd_89g",
	.id = 0,
};

int __init dvd_89g_init(void)
{
    int ret;

    ret = platform_device_register(&dvd_89g_device);
    if (ret != 0) {
        pr_err("%s: Fail to platform_device_register. , ret=%d\n", __func__, ret);
    }

    return 0;
}

arch_initcall(dvd_89g_init);

