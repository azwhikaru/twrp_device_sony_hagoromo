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
#ifndef _MT_GPIO_CORE_H_
#define _MT_GPIO_CORE_H_
/******************************************************************************
 MACRO Definition
******************************************************************************/
#define GPIO_DEVICE "mt-gpio"
#define VERSION     GPIO_DEVICE

#define GPIOTAG                "[GPIO] "
#ifdef CONFIG_ARCH_MT8590_ICX
#define GPIOLOG(fmt, arg...)   pr_warn(GPIOTAG fmt, ##arg)
#define GPIOMSG(fmt, arg...)   pr_warn(fmt, ##arg)
#else /* CONFIG_ARCH_MT8590_ICX */
#define GPIOLOG(fmt, arg...)   pr_debug(GPIOTAG fmt, ##arg)
#define GPIOMSG(fmt, arg...)   pr_debug(fmt, ##arg)
#endif /* CONFIG_ARCH_MT8590_ICX */
#define GPIOERR(fmt, arg...)   pr_err(GPIOTAG "%5d: "fmt, __LINE__, ##arg)
#define GPIOFUC(fmt, arg...)	/* pr_info(GPIOTAG "%s\n", __FUNCTION__) */
/*----------------------------------------------------------------------------*/
/* Error Code No. */
#define RSUCCESS        0
#define ERACCESS        1
#define ERINVAL         2
#define ERWRAPPER		3

#define GPIO_RETERR(res, fmt, args...)                                               \
    do {                                                                             \
	pr_err(GPIOTAG "%s:%04d: " fmt"\n", __func__, __LINE__, ##args);\
	return res;                                                                  \
    } while (0)
#define GIO_INVALID_OBJ(ptr)   ((ptr) != mt_gpio)

enum {
	MT_BASE = 0,
	MT_EXT,
	MT_NOT_SUPPORT,
};
#define MT_GPIO_PLACE(pin) ({\
	int ret = -1;\
	if ((pin >= MT_GPIO_BASE_START) && (pin < MT_GPIO_BASE_MAX)) {\
		ret = MT_BASE;\
		GPIOFUC("pin in base is %d\n", (int)pin);\
	} else if ((pin >= MT_GPIO_EXT_START) && (pin < MT_GPIO_EXT_MAX)) {\
		ret = MT_EXT;\
		GPIOFUC("pin in ext is %d\n", (int)pin);\
	} else{\
		GPIOMSG("Pin number error %d\n", (int)pin);	\
		ret = -1;\
	} \
	ret; })
/* int where_is(unsigned long pin) */
/* { */
/* int ret = -1; */
/* //    GPIOLOG("pin is %d\n",pin); */
/* if((pin >= MT_GPIO_BASE_START) && (pin < MT_GPIO_BASE_MAX)){ */
/* ret = MT_BASE; */
/* //GPIOLOG("pin in base is %d\n",pin); */
/* }else if((pin >= MT_GPIO_EXT_START) && (pin < MT_GPIO_EXT_MAX)){ */
/* ret = MT_EXT; */
/* //GPIOLOG("pin in ext is %d\n",pin); */
/* }else{ */
/* GPIOERR("Pin number error %d\n",pin); */
/* ret = -1; */
/* } */
/* return ret; */
/* } */
/*decrypt pin*/

/*---------------------------------------------------------------------------*/
int mt_set_gpio_dir_base(unsigned long pin, unsigned long dir);
int mt_get_gpio_dir_base(unsigned long pin);
int mt_set_gpio_pull_enable_base(unsigned long pin, unsigned long enable);
int mt_get_gpio_pull_enable_base(unsigned long pin);
int mt_set_gpio_ies_base(unsigned long pin, unsigned long enable);
int mt_get_gpio_ies_base(unsigned long pin);
int mt_set_gpio_pull_select_base(unsigned long pin, unsigned long select);
int mt_get_gpio_pull_select_base(unsigned long pin);
int mt_set_gpio_inversion_base(unsigned long pin, unsigned long enable);
int mt_get_gpio_inversion_base(unsigned long pin);
int mt_set_gpio_out_base(unsigned long pin, unsigned long output);
int mt_get_gpio_out_base(unsigned long pin);
int mt_get_gpio_in_base(unsigned long pin);
int mt_set_gpio_mode_base(unsigned long pin, unsigned long mode);
int mt_get_gpio_mode_base(unsigned long pin);
#ifdef CONFIG_PM
void mt_gpio_suspend(void);
void mt_gpio_resume(void);
#endif				/*CONFIG_PM */

int mt_set_gpio_dir_ext(unsigned long pin, unsigned long dir);
int mt_get_gpio_dir_ext(unsigned long pin);
int mt_set_gpio_pull_enable_ext(unsigned long pin, unsigned long enable);
int mt_get_gpio_pull_enable_ext(unsigned long pin);
int mt_set_gpio_ies_ext(unsigned long pin, unsigned long enable);
int mt_get_gpio_ies_ext(unsigned long pin);
int mt_set_gpio_pull_select_ext(unsigned long pin, unsigned long select);
int mt_get_gpio_pull_select_ext(unsigned long pin);
int mt_set_gpio_inversion_ext(unsigned long pin, unsigned long enable);
int mt_get_gpio_inversion_ext(unsigned long pin);
int mt_set_gpio_out_ext(unsigned long pin, unsigned long output);
int mt_get_gpio_out_ext(unsigned long pin);
int mt_get_gpio_in_ext(unsigned long pin);
int mt_set_gpio_mode_ext(unsigned long pin, unsigned long mode);
int mt_get_gpio_mode_ext(unsigned long pin);

void mt_gpio_pin_decrypt(unsigned long *cipher);
int mt_gpio_create_attr(struct device *dev);
int mt_gpio_delete_attr(struct device *dev);

#endif
