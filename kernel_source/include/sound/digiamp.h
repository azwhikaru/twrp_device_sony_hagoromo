/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * digiamp.h
 *
 * digital amp interface
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

#ifndef _DIGITAL_AMP_HEADER_
#define _DIGITAL_AMP_HEADER_

#define DAMP_TYPE_NONE     0
#define DAMP_TYPE_CANARY   1
#define DAMP_TYPE_CXD90020 2

#define DAMP_CTL_LEVEL_NORMAL 0
#define DAMP_CTL_LEVEL_RESET  1
#define DAMP_CTL_LEVEL_POWER  2

struct digital_amp_interface {

	int type;

	int (*power_on)(void);
	int (*power_off)(void);

	int (*initialize)(void);
	int (*shutdown)(void);

	int (*enable)(int level);
	int (*disable)(int level);

	int (*fade_volume)(unsigned int value);
	int (*set_volume)(unsigned int value);

	int (*switch_sys_clock)(int value);
	int (*switch_shunt_mute)(int value);
};

int digiamp_register(struct digital_amp_interface * interface);

int digiamp_get_type(void);

int digiamp_power_on(void);
int digiamp_power_off(void);

int digiamp_initialize(void);
int digiamp_shutdown(void);

int digiamp_enable(int level);
int digiamp_disable(int level);

int digiamp_fade_volume(unsigned int volume);
int digiamp_set_volume(unsigned int volume);

int digiamp_switch_sys_clock(int value);
int digiamp_switch_shunt_mute(int value);

#endif
