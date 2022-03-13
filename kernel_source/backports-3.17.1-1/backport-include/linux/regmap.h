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
#ifndef __BACKPORT_LINUX_REGMAP_H
#define __BACKPORT_LINUX_REGMAP_H
#include_next <linux/regmap.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0) && \
    LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#define dev_get_regmap LINUX_BACKPORT(dev_get_regmap)
static inline
struct regmap *dev_get_regmap(struct device *dev, const char *name)
{
	return NULL;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0) && \
    LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#if defined(CONFIG_REGMAP)
#define devm_regmap_init LINUX_BACKPORT(devm_regmap_init)
struct regmap *devm_regmap_init(struct device *dev,
				const struct regmap_bus *bus,
				const struct regmap_config *config);
#if defined(CONFIG_REGMAP_I2C)
#define devm_regmap_init_i2c LINUX_BACKPORT(devm_regmap_init_i2c)
struct regmap *devm_regmap_init_i2c(struct i2c_client *i2c,
				    const struct regmap_config *config);
#endif /* defined(CONFIG_REGMAP_I2C) */

/*
 * We can't backport these unless we try to backport
 * the full regmap into core so warn if used.
 * No drivers are using this yet anyway.
 */
#define regmap_raw_write_async LINUX_BACKPORT(regmap_raw_write_async)
static inline int regmap_raw_write_async(struct regmap *map, unsigned int reg,
					 const void *val, size_t val_len)
{
	WARN_ONCE(1, "regmap API is disabled");
	return -EINVAL;
}

#define regmap_async_complete LINUX_BACKPORT(regmap_async_complete)
static inline void regmap_async_complete(struct regmap *map)
{
	WARN_ONCE(1, "regmap API is disabled");
}

#endif /* defined(CONFIG_REGMAP) */
#endif /* 3.2 <= version < 3.4 */

#endif /* __BACKPORT_LINUX_REGMAP_H */
