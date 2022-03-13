/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_switch.h
 *
 * CXD3776ER CODEC driver
 *
 * Copyright (c) 2015 Sony Corporation
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

#ifndef _CXD3776ER_SWITCH_HEADER_
#define _CXD3776ER_SWITCH_HEADER_

int cxd3776er_switch_initialize(void);
int cxd3776er_switch_finalize(void);
int cxd3776er_switch_set_headphone_value(int value);
int cxd3776er_switch_set_antenna_value(int value);

#endif