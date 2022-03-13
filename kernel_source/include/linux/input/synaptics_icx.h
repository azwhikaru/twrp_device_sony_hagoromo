/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * include/linux/synaptics_i2c_rmi.h - platform data structure
 * for f75375s sensor
 *
 * Copyright (C) 2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*
 * ChangeLog:
 *   2011,2012,2013 Changed by Sony Corporation
 */

#ifndef _LINUX_SYNAPTICS_ICX_H
#define _LINUX_SYNAPTICS_ICX_H

#define SYNAPTICS_ICX_NAME				"synaptics-icx"
#define SYNAPTICS_ICX_I2C_SLAVEADDRESS  0x20
#define SYNAPTICS_ICX_EINT				5

enum {
	SYNAPTICS_FLIP_X = 1UL << 0,
	SYNAPTICS_FLIP_Y = 1UL << 1,
	SYNAPTICS_SWAP_XY = 1UL << 2,
	SYNAPTICS_SNAP_TO_INACTIVE_EDGE = 1UL << 3,
};

#endif /* _LINUX_SYNAPTICS_ICX_H */
