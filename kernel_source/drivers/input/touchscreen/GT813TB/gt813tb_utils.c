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
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include "tpd.h"

void tpd_close_gpio(void)
{
	TPD_DEBUG("GT813TB close gpio - wisky75_tb_ics2..\n");

/* disable V33 power */
	mt_set_gpio_mode(GPIO66, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO66, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO66, GPIO_OUT_ZERO);

/* disable level shift */
	mt_set_gpio_mode(GPIO100, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO100, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO100, GPIO_OUT_ZERO);

}

void tpd_open_gpio(void)
{
	TPD_DEBUG("GT813TB open gpio - wisky75_tb_ics2..\n");

/* enable V33 power */
	mt_set_gpio_mode(GPIO66, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO66, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO66, GPIO_OUT_ONE);

/* enable level shift */
	mt_set_gpio_mode(GPIO100, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO100, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO100, GPIO_OUT_ONE);

}
