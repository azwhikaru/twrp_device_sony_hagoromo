/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *    mt_dac_hal.c
 *
 * Project:
 * --------
 *   MT8590
 *
 * Description:
 * ------------
 *   This Module defines functions of mt8590 DAC
 *
 * Author:
 * -------
 * Liguo Zhang
 *
 ****************************************************************************/

#include <linux/init.h>		/* For init/exit macros */
#include <linux/module.h>	/* For MODULE_ marcros  */
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "mt_dac_sw.h"
#include "mt_dac_hw.h"

int mt_dac_set(u8 channel, u8 voltage)
{
#ifdef CONFIG_ENABLE_MTK_8BDAC
	if (channel > 1) {
		pr_err("channel number is invalid!\n");
		return -1;
	}

	*(volatile unsigned int *)CLK_8BDAC_CFG = 0x00000033;
	*(volatile unsigned int *)CLK_CFG_15 = 0x00000100;
	
	if (channel == 0) {
		*(volatile unsigned int *)MIPI_8BDAC_40 = 0x00010001;
		*(volatile unsigned int *)MIPI_8BDAC_38 = voltage;
	} else {
		*(volatile unsigned int *)MIPI_8BDAC_44 = 0x00010001;
		*(volatile unsigned int *)MIPI_8BDAC_3C = voltage;
	}

	*(volatile unsigned int *)MIPI_8BDAC_ANA30 = 0x0000009D;
	*(volatile unsigned int *)MIPI_8BDAC_ANA34 = 0x00000300;
#else
	pr_err("WARNING! MTK 8bit DAC is disabled. But Call %s function.\n",__func__);
	return -1;
#endif
	return 0;
}

#ifdef CONFIG_ENABLE_MTK_8BDAC
ssize_t mt_dac_a_voltage_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 val;

	val = *(volatile unsigned int *)MIPI_8BDAC_38;
	return sprintf(buf, "%u\n", val);
}
ssize_t mt_dac_a_voltage_set(struct device *dev,
	struct device_attribute *attr, char *buf, size_t count)
{
	int status = count;
	u8 val;

	if (kstrtou8(buf, 10, &val) < 0) {
		return -EINVAL;
	}
	mt_dac_set(0,val);
	return status;
}
static DEVICE_ATTR(dac_a_voltage, S_IWUSR | S_IRUGO, mt_dac_a_voltage_show, mt_dac_a_voltage_set);

ssize_t mt_dac_b_voltage_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 val;

	val = *(volatile unsigned int *)MIPI_8BDAC_3C;
	return sprintf(buf, "%u\n", val);
}
ssize_t mt_dac_b_voltage_set(struct device *dev,
	struct device_attribute *attr, char *buf, size_t count)
{
	int status = count;
	u8 val;

	if (kstrtou8(buf, 10, &val) < 0) {
		return -EINVAL;
	}
	mt_dac_set(1,val);
	return status;
}
static DEVICE_ATTR(dac_b_voltage, S_IWUSR | S_IRUGO, mt_dac_b_voltage_show, mt_dac_b_voltage_set);

static struct attribute *mt_dac_attributes[] = {
	&dev_attr_dac_a_voltage.attr,
	&dev_attr_dac_b_voltage.attr,
	NULL,
};

static const struct attribute_group mt_dac_attr_group = {
	.attrs = mt_dac_attributes,
};
#endif

static int mt_dac_probe(struct platform_device *dev)
{
	int ret;
	pr_debug("******** MT DAC driver probe!! ********\n");
#ifdef CONFIG_ENABLE_MTK_8BDAC
	ret = sysfs_create_group(&(dev->dev.kobj), &mt_dac_attr_group);
	if (ret) {
		pr_err("%s: Could not create sysfs files. ret=%d\n", __func__, ret);
	}
#else
	/* Power off 8bit DAC*/
	*(volatile unsigned int *)MIPI_8BDAC_ANA34 = 0x00000003;
#endif
	return 0;
}

static int mt_dac_remove(struct platform_device *dev)
{
	pr_debug("******** MT DAC driver remove!! ********\n" );
#ifdef CONFIG_ENABLE_MTK_8BDAC
	sysfs_remove_group(&(dev->dev.kobj), &mt_dac_attr_group);
#endif
	return 0;
}

static void mt_dac_shutdown(struct platform_device *dev)
{
	pr_debug("******** MT DAC driver shutdown!! ********\n" );
}

static struct platform_driver mt_dac_driver = {
	.probe      = mt_dac_probe,
	.remove     = mt_dac_remove,
	.shutdown   = mt_dac_shutdown,

	.driver     = {
		.name       = "mt-dac",
	},
};

static int __init mt_dac_init(void)
{
	int ret;

	ret = platform_driver_register(&mt_dac_driver);
	if (ret) {
		pr_err("****[mt_dac_driver] Unable to register driver (%d)\n", ret);
		return ret;
	}
	
	pr_debug("****[mt_dac_driver] Initialization : DONE \n");
	return 0;
}

static void __exit mt_dac_exit (void)
{
}

module_init(mt_dac_init);
module_exit(mt_dac_exit);

MODULE_AUTHOR("MTK");
MODULE_DESCRIPTION("MTK DAC Device Driver");
MODULE_LICENSE("GPL");