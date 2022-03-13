/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 *  rcs730.h - rcs730 NFC Link driver
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

#ifndef _RCS730_H
#define _RCS730_H

#define CXDNFC_MAGIC 'S'
/*
 * CXDNFC_WAKE_CTL(0): PON HIGH (normal power mode)
 * CXDNFC_WAKE_CTL(1): PON LOW (low power mode)
 */

#define CXDNFC_POWER_CTL _IO(CXDNFC_MAGIC, 0x01) 
#define CXDNFC_WAKE_CTL _IO(CXDNFC_MAGIC, 0x02) 

struct rcs730_platform_data {
	unsigned int rfdet_gpio;
        unsigned int irq_gpio; 
};

#endif
