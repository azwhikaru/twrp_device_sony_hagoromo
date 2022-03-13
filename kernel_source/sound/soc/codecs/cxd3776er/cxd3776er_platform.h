/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_platform.h
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

#ifndef _CXD3776ER_PLATFORM_HEADER_
#define _CXD3776ER_PLATFORM_HEADER_

int cxd3776er_setup_platform(struct cxd3776er_platform_data * data);
int cxd3776er_reset_platform(void);

int cxd3776er_switch_330_power(int value);
int cxd3776er_switch_285_power(int value);
int cxd3776er_switch_logic_ldo(int value);
int cxd3776er_switch_external_osc(int value);
int cxd3776er_get_external_osc(void);

int cxd3776er_reset(void);
int cxd3776er_unreset(void);

int cxd3776er_switch_smaster_mute(int value);
int cxd3776er_switch_class_h_mute(int value);

int cxd3776er_switch_speaker_power(int value);

int cxd3776er_get_hp_det_value(void);
int cxd3776er_get_mic_det_value(void);
int cxd3776er_get_xpcm_det_value(void);
int cxd3776er_get_xpcm_det_irq(void);

#endif
