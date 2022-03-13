/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd90020.h
 *
 * CXD90020 DAmp driver
 *
 * Copyright (c) 2013 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _CXD90020_HEADER_
#define _CXD90020_HEADER_

#define CXD90020_DEVICE_NAME "DIGIAMP_CXD90020"

struct cxd90020_platform_data{
	int port_tbsamp_xint;
	int port_tbsamp_xrst;
	int port_hp_xmute2;
	int port_hp_mute4;
	int port_vg_xon;
	int port_vldo_xshunt;
	int port_tbsamp_pwr_on;
};

#endif
