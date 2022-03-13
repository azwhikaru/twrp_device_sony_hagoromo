/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3778gf.h
 *
 * CXD3778GF CODEC driver
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

#ifndef _CXD3778GF_HEADER_
#define _CXD3778GF_HEADER_

#define CXD3778GF_DEVICE_NAME  "CODEC_CXD3778GF"
#define CXD3778GF_CODEC_NAME   CXD3778GF_DEVICE_NAME ".2-004e"
#define CXD3778GF_STD_DAI_NAME "DAI_CXD3778GF_STD"
#define CXD3778GF_ICX_DAI_NAME "DAI_CXD3778GF_ICX"

#define TYPE_A	1
#define TYPE_Z	2

struct cxd3778gf_platform_data{

	char * regulator_180;
	char * regulator_285;
	int port_i2s0_data_out;
	int port_i2s0_data_in;
	int port_i2s0_data_bck;
	int port_i2s0_data_lrck;
	int port_i2s1_data_out;
	int port_i2s1_data_in;
	int port_i2s1_data_bck;
	int port_i2s1_data_lrck;
	int port_ada_ldo_en;
	int port_ada_xint;
	int port_se_xshtd;
	int port_btl_xshtd;
	int port_ada_xdet;
	int port_ada_xrst;
	int port_hp_xmute;
	int port_au_vl_en;
	int port_au_avdd_en;
	int port_au_btl_5v_en;
	int port_au_btl_5v_en_2;
	int port_au_btl_7v_en;
        int port_au_se_pos_en;
        int port_au_se_neg_en;
	int port_au_se_3v_en;
	int port_hp_btl_det_l;
	int port_hp_btl_det_r;
	int port_nc_cmpout;
	int port_nc_det;
	int port_hp_det;
	int port_hp_se_mute_on;
	int port_hp_se_mute_off;
	int port_hp_se_mute_cp_on;
	int port_hp_btl_mute_on;
	int port_hp_btl_mute_off;
	int port_hp_fm_series_xmute;
	int port_ada_fs480_en;;
	int port_hp_xmute4;
	int port_ada_fs441_en;
	int port_ext_ck1;
	int type1;
	int type2;
};

#define CXD3778GF_MIC_BIAS_MODE_OFF     0
#define CXD3778GF_MIC_BIAS_MODE_NORMAL  1
#define CXD3778GF_MIC_BIAS_MODE_MIC_DET 2
#define CXD3778GF_MIC_BIAS_MODE_NC_ON   3

struct cxd3778gf_dnc_interface {

	int (*initialize)(void);
	int (*shutdown)(void);
	int (*prepare)(void);
	int (*cleanup)(void);
	int (*judge)(
		int noise_cancel_mode,
		int output_device,
		int headphone_amp,
		int jack_status,
		int headphone_type,
		unsigned int format,
		unsigned int osc
	);
	int (*off)(void);
	int (*set_user_gain)(int index);
	int (*get_user_gain)(int * index);
	int (*set_base_gain)(int left, int right);
	int (*get_base_gain)(int * left, int * right);
	int (*exit_base_gain_adjustment)(int save);

	int (*set_mic_bias_mode)(int mode);
	int (*switch_dnc_power)(int value);
	int (*switch_ncmic_amp_power)(int value);

	int (*modify_reg)(unsigned int address, unsigned int   value, unsigned int mask);
	int (*write_reg) (unsigned int address, unsigned int   value);
	int (*read_reg)  (unsigned int address, unsigned int * value);
	int (*write_buf) (unsigned int address, unsigned char * value, int size);
	int (*read_buf)  (unsigned int address, unsigned char * value, int size);

	struct mutex * global_mutex;
};

int cxd3778gf_dnc_register_module(struct cxd3778gf_dnc_interface * interface);

#endif
