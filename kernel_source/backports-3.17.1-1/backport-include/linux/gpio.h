/*
* Copyright (C) 2011-2015 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __BACKPORT_LINUX_GPIO_H
#define __BACKPORT_LINUX_GPIO_H
#include_next <linux/gpio.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0)
#define devm_gpio_request_one LINUX_BACKPORT(devm_gpio_request_one)
#define devm_gpio_request LINUX_BACKPORT(devm_gpio_request)
#ifdef CONFIG_GPIOLIB
int devm_gpio_request(struct device *dev, unsigned gpio, const char *label);
int devm_gpio_request_one(struct device *dev, unsigned gpio,
			  unsigned long flags, const char *label);
#else
static inline int devm_gpio_request(struct device *dev, unsigned gpio,
				    const char *label)
{
	WARN_ON(1);
	return -EINVAL;
}

static inline int devm_gpio_request_one(struct device *dev, unsigned gpio,
					unsigned long flags, const char *label)
{
	WARN_ON(1);
	return -EINVAL;
}
#endif /* CONFIG_GPIOLIB */
#endif

#endif /* __BACKPORT_LINUX_GPIO_H */
