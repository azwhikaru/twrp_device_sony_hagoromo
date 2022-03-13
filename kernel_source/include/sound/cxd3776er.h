/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er.h
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

#ifndef _CXD3776ER_HEADER_
#define _CXD3776ER_HEADER_

#define CXD3776ER_DEVICE_NAME  "CODEC_CXD3776ER"
#define CXD3776ER_CODEC_NAME   CXD3776ER_DEVICE_NAME ".0-001e"
#define CXD3776ER_STD_DAI_NAME "DAI_CXD3776ER_STD"
#define CXD3776ER_ICX_DAI_NAME "DAI_CXD3776ER_ICX"

struct cxd3776er_platform_data{
	char * regulator_330;
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
	int port_sp_xshtd;
	int port_xpcm_det;
	int port_ada_xrst;
	int port_hp_xmute;
	int port_nc_cmpout;
	int port_nc_det;
	int port_hp_det;
	int port_hp_xmute3;
	int port_ada_osc_en;
	int port_hp_xmute4;
	int port_mclk_441_en;
};

#define CXD3776ER_MIC_BIAS_MODE_OFF     0
#define CXD3776ER_MIC_BIAS_MODE_NORMAL  1
#define CXD3776ER_MIC_BIAS_MODE_MIC_DET 2
#define CXD3776ER_MIC_BIAS_MODE_NC_ON   3

struct cxd3776er_dmfb_interface {

	int (*initialize)(void);
	int (*shutdown)(void);
	int (*prepare)(void);
	int (*cleanup)(void);
	int (*judge)(
		int motion_feedback_mode,
		int output_path,
		int output_select,
		int tpm_status,
		int sdin_mix
	);
	int (*off)(void);
	int (*set_user_gain)(int index);
	int (*get_user_gain)(int * index);
	int (*set_base_gain)(int left, int right);
	int (*get_base_gain)(int * left, int * right);
	int (*exit_base_gain_adjustment)(int save);

	int (*set_mic_bias_mode)(int mode);
	int (*switch_dmfb_power)(int value);
	int (*switch_ncmic_amp_power)(int value);

	int (*modify_reg)(unsigned int address, unsigned int   value, unsigned int mask);
	int (*write_reg) (unsigned int address, unsigned int   value);
	int (*read_reg)  (unsigned int address, unsigned int * value);
	int (*write_buf) (unsigned int address, unsigned char * value, int size);
	int (*read_buf)  (unsigned int address, unsigned char * value, int size);

	struct mutex * global_mutex;
};

struct cxd3776er_tpm_interface {

        int (*initialize)(void);
        int (*shutdown)(void);
        int (*prepare)(void);
        int (*cleanup)(void);
        int (*judge)(
                int motion_feedback_mode,
                int output_path,
                int output_select,
                int tpm_status,
                int sdin_mix
        );
        int (*off)(void);
        int (*set_temperature)(int index);
        int (*get_temperature)(int * index);
        int (*set_base_gain)(int left, int right);
        int (*get_base_gain)(int * left, int * right);
        int (*exit_base_gain_adjustment)(int save);

        int (*set_mic_bias_mode)(int mode);
        int (*switch_tpm_power)(int value);
        int (*switch_ncmic_amp_power)(int value);

        int (*modify_reg)(unsigned int address, unsigned int   value, unsigned int mask);
        int (*write_reg) (unsigned int address, unsigned int   value);
        int (*read_reg)  (unsigned int address, unsigned int * value);
        int (*write_buf) (unsigned int address, unsigned char * value, int size);
        int (*read_buf)  (unsigned int address, unsigned char * value, int size);

        struct mutex * global_mutex;
};

#endif
