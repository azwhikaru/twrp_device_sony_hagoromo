/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3778gf_platform.h
 *
 * CXD3778GF CODEC driver
 *
 * Copyright (c) 2013-2016 Sony Corporation
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

#ifndef _CXD3778GF_PLATFORM_HEADER_
#define _CXD3778GF_PLATFORM_HEADER_

int cxd3778gf_setup_platform(struct cxd3778gf_platform_data * data, unsigned int * type);
int cxd3778gf_reset_platform(void);

int cxd3778gf_switch_180_power(int value);
int cxd3778gf_switch_285_power(int value);
int cxd3778gf_switch_hp3x_power(int value);
int cxd3778gf_switch_logic_ldo(int value);
int cxd3778gf_switch_external_osc(int value);
int cxd3778gf_get_external_osc(void);

int cxd3778gf_reset(void);
int cxd3778gf_unreset(void);

int cxd3778gf_switch_smaster_mute(int value, int amp);
int cxd3778gf_switch_class_h_mute(int value);

int cxd3778gf_switch_speaker_power(int value);

int cxd3778gf_get_hp_det_se_value(void);
int cxd3778gf_get_hp_det_btl_value(void);
int cxd3778gf_get_mic_det_value(void);
int cxd3778gf_get_xpcm_det_value(void);
int cxd3778gf_get_xpcm_det_irq(void);
int cxd3778gf_get_hp_det_irq(void);

int cxd3778gf_set_se_cp_mute(void);
int cxd3778gf_set_se_cp_unmute(void);

int cxd3778gf_set_441clk_enable(void);
int cxd3778gf_set_480clk_enable(void);
int cxd3778gf_get_441clk_value(void);
int cxd3778gf_get_480clk_value(void);

extern int board_set_flag;
#define BOARD_ID_NO_PULLUP_MASK (0x00000001)
#define BOARD_ID_PULLUP_MASK (0x00000002)

#endif
