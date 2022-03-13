/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 *  cxd224x-i2c.c - cxd224x NFC driver
 *
 * Copyright (C) 2012 Sony Corporation.
 * Copyright (C) 2012 Broadcom Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _CXD224X_H
#define _CXD224X_H

#define CXDNFC_MAGIC 'S'

struct cxd224x_platform_data {
	unsigned int xint_gpio;
	unsigned int pon_gpio; 
	unsigned int rst_gpio; 
	unsigned int en_gpio;
};

#define CXDNFC_RESET_CTL	_IO(CXDNFC_MAGIC, 0x01)
#define CXDNFC_POWER_CTL	_IO(CXDNFC_MAGIC, 0x02) 
#define CXDNFC_WAKE_CTL		_IO(CXDNFC_MAGIC, 0x03) 
#define CXDNFC_LDOEN_CTL	_IO(CXDNFC_MAGIC, 0x04)

/* IOCTL parameter */
#define ARG_PON_ON 0
#define ARG_PON_OFF 1

#endif
