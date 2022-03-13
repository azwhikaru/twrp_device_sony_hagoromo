/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_volume.c
 *
 * CXD3776ER CODEC driver
 *
 * Copyright (c) 2015 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/* #define TRACE_PRINT_ON */
/* #define DEBUG_PRINT_ON */
#define TRACE_TAG "------- "
#define DEBUG_TAG "        "

#include "cxd3776er_common.h"

#define dgtvolint(_a) ( (_a)>=0x19 ? (int)(0xFFFFFF00|(_a)) : (int)(_a) )
#define anavolint(_a) ( (_a)>=0x40 ? (int)(0xFFFFFF80|(_a)) : (int)(_a) )
#define pgavolint(_a) ( (_a)>=0x20 ? (int)(0xFFFFFFC0|(_a)) : (int)(_a) )
#define adcvolint(_a) ( (_a)>=0x01 ? (int)(0xFFFFFF80|(_a)) : (int)(_a) )

static int fade_digital_volume(unsigned int address, unsigned int volume);
static int set_pwm_out_mute(int mute);
static int set_hardware_mute(int output_path, int output_select, int mute);
static int get_table_index(int output_path, int output_select, int motion_feedback_active);

#define OUTPUT_MUTE    0x00000001
#define INPUT_MUTE     0x00000002
#define PLAYBACK_MUTE  0x00000004
#define CAPTURE_MUTE   0x00000008
#define ANALOG_MUTE    0x00000010
#define STREAM_MUTE    0x00000020
#define NO_PCM_MUTE    0x00000040
#define TIMED_MUTE     0x00000080
#define STD_TIMED_MUTE 0x00000100
#define ICX_TIMED_MUTE 0x00000200
#define UNCG_MUTE      0x00000400

static unsigned int sdout_vol_mute = INPUT_MUTE;
static unsigned int sdin1_vol_mute = 0x00;
static unsigned int sdin2_vol_mute = 0x00;
static unsigned int lin_vol_mute   = INPUT_MUTE;
static unsigned int dac_vol_mute   = 0x00;
static unsigned int dgt_vol_mute   = 0x00;
static unsigned int damp_vol_mute  = OUTPUT_MUTE;
static unsigned int hpout_vol_mute = OUTPUT_MUTE;
static unsigned int lout_vol_mute  = OUTPUT_MUTE;
static unsigned int pwm_out_mute   = OUTPUT_MUTE;
static unsigned int hw_mute        = OUTPUT_MUTE;
static unsigned int master_vol_mute  = 0x00;

static int initialized=FALSE;

int cxd3776er_volume_initialize(void)
{
	initialized=TRUE;

	return(0);
}

int cxd3776er_volume_finalize(void)
{
	if(!initialized)
		return(0);

	initialized=FALSE;

	return(0);
}
#if 0
unsigned char cxd3776er_master_volume[MASTER_VOLUME_MAX+1] =
{
	/* 00 */ 0x33,
	/* 01 */ 0x72,
	/* 02 */ 0x7E,
	/* 03 */ 0x8A,
	/* 04 */ 0x96,
	/* 05 */ 0x9E,
	/* 06 */ 0xA4,
	/* 07 */ 0xAA,
	/* 08 */ 0xB0,
	/* 09 */ 0xB6,
	/* 10 */ 0xBC,
	/* 11 */ 0xC2,
	/* 12 */ 0xC6,
	/* 13 */ 0xCA,
	/* 14 */ 0xCE,
	/* 15 */ 0xD2,
	/* 16 */ 0xD6,
	/* 17 */ 0xDA,
	/* 18 */ 0xDE,
	/* 19 */ 0xE2,
	/* 20 */ 0xE4,
	/* 21 */ 0xE6,
	/* 22 */ 0xE8,
	/* 23 */ 0xEA,
	/* 24 */ 0xEC,
	/* 25 */ 0xEE,
	/* 26 */ 0xF0,
	/* 27 */ 0xF2,
	/* 28 */ 0xF4,
	/* 29 */ 0xF4,
	/* 30 */ 0xF8,
};
#endif
/* msater volume */
int cxd3776er_set_master_volume(struct cxd3776er_status * now, int volume)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,volume);

//	table=get_table_index(now->output_path,now->output_select,now->motion_feedback_active);
	effect=(now->sound_effect ? 1 : 0);

	if(master_vol_mute){
		cxd3776er_register_write(CXD3776ER_VOL1L07, 0x00);
		cxd3776er_register_write(CXD3776ER_VOL1R07, 0x00);
		cxd3776er_register_write(CXD3776ER_VOL2L07, 0x00);
		cxd3776er_register_write(CXD3776ER_VOL2R07, 0x00);
	} else {
		cxd3776er_register_write(CXD3776ER_VOL1L07, cxd3776er_master_volume_table[now->output_path][volume].out);
		cxd3776er_register_write(CXD3776ER_VOL1R07, cxd3776er_master_volume_table[now->output_path][volume].out);
		cxd3776er_register_write(CXD3776ER_VOL2L07, cxd3776er_master_volume_table[now->output_path][volume].out);
		cxd3776er_register_write(CXD3776ER_VOL2R07, cxd3776er_master_volume_table[now->output_path][volume].out);
	}

	if(now->sdin_mix==SDIN_SEPARATE_1 || now->sdin_mix==SDIN_INVERT){

//		if(sdin1_vol_mute){
//			cxd3776er_register_write(CXD3776ER_VOL1L00, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL1R01, 0x33);
//		}
//		else{
			cxd3776er_register_write(CXD3776ER_VOL1L00, cxd3776er_master_volume_table[now->output_path][volume].sdin1);
			cxd3776er_register_write(CXD3776ER_VOL1R01, cxd3776er_master_volume_table[now->output_path][volume].sdin1);
//		}
//		if(sdin2_vol_mute){
//			cxd3776er_register_write(CXD3776ER_VOL2L02, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL2R03, 0x33);
//		}
//		else{
			cxd3776er_register_write(CXD3776ER_VOL2L02, cxd3776er_master_volume_table[now->output_path][volume].sdin2);
			cxd3776er_register_write(CXD3776ER_VOL2R03, cxd3776er_master_volume_table[now->output_path][volume].sdin2);
//		}
	}

	if(now->sdin_mix==SDIN_SEPARATE_2){

//		if(sdin1_vol_mute){
//			cxd3776er_register_write(CXD3776ER_VOL1L00, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL1R01, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL2L00, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL2R01, 0x33);
//		}
//		else{
			cxd3776er_register_write(CXD3776ER_VOL1L00, cxd3776er_master_volume_table[now->output_path][volume].sdin1);
			cxd3776er_register_write(CXD3776ER_VOL1R01, cxd3776er_master_volume_table[now->output_path][volume].sdin1);
			cxd3776er_register_write(CXD3776ER_VOL2L00, cxd3776er_master_volume_table[now->output_path][volume].sdin1);
			cxd3776er_register_write(CXD3776ER_VOL2R01, cxd3776er_master_volume_table[now->output_path][volume].sdin1);
//		}
	}

	if(now->sdin_mix==SDIN_SEPARATE_3){

//		if(sdin2_vol_mute){
//			cxd3776er_register_write(CXD3776ER_VOL1L02, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL1R03, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL2L02, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL2R03, 0x33);
//		}
//		else{
			cxd3776er_register_write(CXD3776ER_VOL1L02, cxd3776er_master_volume_table[now->output_path][volume].sdin2);
			cxd3776er_register_write(CXD3776ER_VOL1R03, cxd3776er_master_volume_table[now->output_path][volume].sdin2);
			cxd3776er_register_write(CXD3776ER_VOL2L02, cxd3776er_master_volume_table[now->output_path][volume].sdin2);
			cxd3776er_register_write(CXD3776ER_VOL2R03, cxd3776er_master_volume_table[now->output_path][volume].sdin2);
//		}
	}

	if(now->sdin_mix==SDIN_MIX){

//		if(sdin2_vol_mute){
//			cxd3776er_register_write(CXD3776ER_VOL1L00, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL1R00, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL2L00, 0x33);
//			cxd3776er_register_write(CXD3776ER_VOL2R00, 0x33);
//		}
//		else{
			cxd3776er_register_write(CXD3776ER_VOL1L00, cxd3776er_master_volume_table[now->output_path][volume].sdin1);
			cxd3776er_register_write(CXD3776ER_VOL1R00, cxd3776er_master_volume_table[now->output_path][volume].sdin1);
			cxd3776er_register_write(CXD3776ER_VOL2L00, cxd3776er_master_volume_table[now->output_path][volume].sdin2);
			cxd3776er_register_write(CXD3776ER_VOL2R00, cxd3776er_master_volume_table[now->output_path][volume].sdin2);
//		}
	}
#if 0
	if(dac_vol_mute)
		cxd3776er_register_write(CXD3776ER_DAC_VOL, 0x33);
	else
		cxd3776er_register_write(CXD3776ER_DAC_VOL, cxd3776er_master_volume_table[effect][table][volume].dac);

	if(hpout_vol_mute)
		cxd3776er_register_write(CXD3776ER_HPOUT_VOL, 0x40);
	else
		cxd3776er_register_write(CXD3776ER_HPOUT_VOL, cxd3776er_master_volume_table[effect][table][volume].hpout);

	if(lout_vol_mute)
		cxd3776er_register_write(CXD3776ER_LINEOUT_VOL, 0x40);
	else
		cxd3776er_register_write(CXD3776ER_LINEOUT_VOL, cxd3776er_master_volume_table[effect][table][volume].lineout);
#endif

	if(damp_vol_mute){
		if(digiamp_get_type()==DAMP_TYPE_CXD90020){
			if(hw_mute)
				digiamp_fade_volume(0x39);
			else
				digiamp_fade_volume(0x38);
		}
		else if(digiamp_get_type()==DAMP_TYPE_CANARY)
			digiamp_fade_volume(0x00);
	}
	else{
		if(digiamp_get_type()==DAMP_TYPE_CXD90020)
			digiamp_fade_volume(cxd3776er_master_volume_table[now->output_path][volume].cxd90020);
		else if(digiamp_get_type()==DAMP_TYPE_CANARY)
			digiamp_fade_volume(cxd3776er_master_volume_table[now->output_path][volume].canary);
	}


	return(0);
}

/* master gain */
int cxd3776er_set_master_gain(struct cxd3776er_status * now, int gain)
{
	print_trace("%s(%d)\n",__FUNCTION__,gain);
#if 0
	if(sdout_vol_mute)
		cxd3776er_register_write(CXD3776ER_SDOUT_VOL, 0x33);
	else
		cxd3776er_register_write(CXD3776ER_SDOUT_VOL, cxd3776er_master_gain_table[now->master_gain]);
#endif
	return(0);
}

/* for ??? */
int cxd3776er_set_playback_mute(struct cxd3776er_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_path,now->output_select,now->motion_feedback_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		master_vol_mute = master_vol_mute |  PLAYBACK_MUTE;
	}
	else{
		master_vol_mute = master_vol_mute & ~PLAYBACK_MUTE;
	}

	if(dac_vol_mute) {
		fade_digital_volume(CXD3776ER_VOL1L07, 0x33);
		fade_digital_volume(CXD3776ER_VOL1R07, 0x33);
		fade_digital_volume(CXD3776ER_VOL2L07, 0x33);
		fade_digital_volume(CXD3776ER_VOL2R07, 0x33);
	}
	else {
		fade_digital_volume(CXD3776ER_VOL1L07, (CXD3776ER_VOL1L07, cxd3776er_master_volume_table[now->output_path][now->master_volume].out));
		fade_digital_volume(CXD3776ER_VOL1R07, (CXD3776ER_VOL1L07, cxd3776er_master_volume_table[now->output_path][now->master_volume].out));
		fade_digital_volume(CXD3776ER_VOL2L07, (CXD3776ER_VOL1L07, cxd3776er_master_volume_table[now->output_path][now->master_volume].out));
		fade_digital_volume(CXD3776ER_VOL2R07, (CXD3776ER_VOL1L07, cxd3776er_master_volume_table[now->output_path][now->master_volume].out));
	}

	return(0);
}

/* mic mute */
int cxd3776er_set_capture_mute(struct cxd3776er_status * now, int mute)
{
	print_trace("%s(%d)\n",__FUNCTION__,mute);
#if 0
	if(mute){
		sdout_vol_mute = sdout_vol_mute |  INPUT_MUTE;
	}
	else{
		sdout_vol_mute = sdout_vol_mute & ~INPUT_MUTE;
	}

	if(sdout_vol_mute)
		cxd3776er_register_write(CXD3776ER_SDOUT_VOL, 0x33);
	else
		cxd3776er_register_write(CXD3776ER_SDOUT_VOL, cxd3776er_master_gain_table[now->master_gain]);
#endif
	return(0);
}

/* for application */
int cxd3776er_set_analog_playback_mute(struct cxd3776er_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);
#if 0
	table=get_table_index(now->output_path,now->output_select,now->motion_feedback_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		lin_vol_mute = lin_vol_mute |  ANALOG_MUTE;
	}
	else{
		lin_vol_mute = lin_vol_mute & ~ANALOG_MUTE;
	}

	if(lin_vol_mute)
		cxd3776er_register_write(CXD3776ER_LINEIN_VOL, 0x33);
	else
		cxd3776er_register_write(CXD3776ER_LINEIN_VOL, cxd3776er_master_volume_table[effect][table][now->master_volume].linein);
#endif
	return(0);
}

/* for audio policy manager */
int cxd3776er_set_analog_stream_mute(struct cxd3776er_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);
#if 0
	table=get_table_index(now->output_path,now->output_select,now->motion_feedback_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		lin_vol_mute = lin_vol_mute |  STREAM_MUTE;
	}
	else{
		lin_vol_mute = lin_vol_mute & ~STREAM_MUTE;
	}

	if(lin_vol_mute)
		cxd3776er_register_write(CXD3776ER_LINEIN_VOL, 0x33);
	else
		cxd3776er_register_write(CXD3776ER_LINEIN_VOL, cxd3776er_master_volume_table[effect][table][now->master_volume].linein);
#endif
	return(0);
}

/* for device switch */
int cxd3776er_set_mix_timed_mute(struct cxd3776er_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);
#if 0
	table=get_table_index(now->output_path,now->output_select,now->motion_feedback_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		dac_vol_mute = dac_vol_mute |  TIMED_MUTE;
	}
	else{
		dac_vol_mute = dac_vol_mute & ~TIMED_MUTE;
	}

	if(dac_vol_mute)
		fade_digital_volume(CXD3776ER_DAC_VOL, 0x33);
	else
		fade_digital_volume(CXD3776ER_DAC_VOL, cxd3776er_master_volume_table[effect][table][now->master_volume].dac);
#endif
	return(0);
}

/* for STD sound effect */
int cxd3776er_set_std_timed_mute(struct cxd3776er_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);
#if 0
	table=get_table_index(now->output_path,now->output_select,now->motion_feedback_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		sdin2_vol_mute = sdin2_vol_mute |  STD_TIMED_MUTE;
	}
	else{
		sdin2_vol_mute = sdin2_vol_mute & ~STD_TIMED_MUTE;
	}

	if(sdin2_vol_mute)
		fade_digital_volume(CXD3776ER_SDIN_2_VOL, 0x33);
	else
		fade_digital_volume(CXD3776ER_SDIN_2_VOL, cxd3776er_master_volume_table[effect][table][now->master_volume].sdin2);
#endif
	return(0);
}

/* for ICX sound effect */
int cxd3776er_set_icx_timed_mute(struct cxd3776er_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);
#if 0
	table=get_table_index(now->output_path,now->output_select,now->motion_feedback_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		sdin1_vol_mute = sdin1_vol_mute |  ICX_TIMED_MUTE;
	}
	else{
		sdin1_vol_mute = sdin1_vol_mute & ~ICX_TIMED_MUTE;
	}

	if(sdin1_vol_mute)
		fade_digital_volume(CXD3776ER_SDIN_1_VOL, 0x33);
	else
		fade_digital_volume(CXD3776ER_SDIN_1_VOL, cxd3776er_master_volume_table[effect][table][now->master_volume].sdin1);
#endif
	return(0);
}

/* for no pcm */
int cxd3776er_set_no_pcm_mute(int mute)
{
	print_trace("%s(%d)\n",__FUNCTION__,mute);
#if 0
	if(mute){
		pwm_out_mute = pwm_out_mute |  NO_PCM_MUTE;
	}
	else{
		pwm_out_mute = pwm_out_mute & ~NO_PCM_MUTE;
	}

	if(pwm_out_mute)
		set_pwm_out_mute(ON);
	else
		set_pwm_out_mute(OFF);
#endif
	return(0);
}

int cxd3776er_set_uncg_mute(struct cxd3776er_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);
#if 0
	table=get_table_index(now->output_path,now->output_select,now->motion_feedback_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		dgt_vol_mute = dgt_vol_mute |  UNCG_MUTE;
	}
	else{
		dgt_vol_mute = dgt_vol_mute & ~UNCG_MUTE;
	}

	if(dgt_vol_mute){
		cxd3776er_register_write(CXD3776ER_DGT_VOL_0H,0x00);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_1H,0x00);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_0L,0x00);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_1L,0x00);
	}
	else{
		cxd3776er_register_write(CXD3776ER_DGT_VOL_0H,cxd3776er_master_volume_table[effect][table][now->master_volume].cmx1);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_1H,cxd3776er_master_volume_table[effect][table][now->master_volume].cmx1);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_0L,cxd3776er_master_volume_table[effect][table][now->master_volume].cmx2);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_1L,cxd3776er_master_volume_table[effect][table][now->master_volume].cmx2);
	}
#endif
	return(0);
}

/* for driver */
int cxd3776er_set_output_path_mute(struct cxd3776er_status * now, int mute, int hw)
{
	int effect;
	int table;

	print_trace("%s(%d,%d)\n",__FUNCTION__,mute,hw);
#if 0
	table=get_table_index(now->output_path,now->output_select,now->motion_feedback_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		dgt_vol_mute   = dgt_vol_mute   |  OUTPUT_MUTE;
		dac_vol_mute   = dac_vol_mute   |  OUTPUT_MUTE;
		hpout_vol_mute = hpout_vol_mute |  OUTPUT_MUTE;
		lout_vol_mute  = lout_vol_mute  |  OUTPUT_MUTE;
		damp_vol_mute  = damp_vol_mute  |  OUTPUT_MUTE;
		pwm_out_mute   = pwm_out_mute   |  OUTPUT_MUTE;
		if(hw)
			hw_mute    = hw_mute        |  OUTPUT_MUTE;
	}
	else{
		dgt_vol_mute   = dgt_vol_mute   & ~OUTPUT_MUTE;
		dac_vol_mute   = dac_vol_mute   & ~OUTPUT_MUTE;
		damp_vol_mute  = damp_vol_mute  & ~OUTPUT_MUTE;
		hpout_vol_mute = hpout_vol_mute & ~OUTPUT_MUTE;
		lout_vol_mute  = lout_vol_mute  & ~OUTPUT_MUTE;
		pwm_out_mute   = pwm_out_mute   & ~OUTPUT_MUTE;
		if(hw)
			hw_mute    = hw_mute        & ~OUTPUT_MUTE;
	}

	if(dgt_vol_mute){
		cxd3776er_register_write(CXD3776ER_DGT_VOL_0H,0x00);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_1H,0x00);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_0L,0x00);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_1L,0x00);
	}

	if(dac_vol_mute)
		fade_digital_volume(CXD3776ER_DAC_VOL, 0x33);

	if(pwm_out_mute)
		set_pwm_out_mute(ON);

	if(hw && hw_mute){
		set_hardware_mute(now->output_path,now->output_select,ON);
		msleep(100);
	}

	if(hpout_vol_mute)
		cxd3776er_register_write(CXD3776ER_HPOUT_VOL, 0x40);

	if(lout_vol_mute)
		cxd3776er_register_write(CXD3776ER_LINEOUT_VOL, 0x40);

	if(damp_vol_mute){
		if(digiamp_get_type()==DAMP_TYPE_CXD90020){
			digiamp_fade_volume(0x38);
		}
		else if(digiamp_get_type()==DAMP_TYPE_CANARY){
			digiamp_fade_volume(0x00);
		}
	}

	if(hw && hw_mute){
		if(digiamp_get_type()==DAMP_TYPE_CXD90020){
			digiamp_set_volume(0x39);
		}
	}

	/***************************************/

	if(hw && !hw_mute){
		if(digiamp_get_type()==DAMP_TYPE_CXD90020){
			digiamp_set_volume(0x38);
		}
	}

	if(!damp_vol_mute){
		if(digiamp_get_type()==DAMP_TYPE_CXD90020){
			digiamp_fade_volume(cxd3776er_master_volume_table[effect][table][now->master_volume].cxd90020);
		}
		else if(digiamp_get_type()==DAMP_TYPE_CANARY){
			digiamp_fade_volume(cxd3776er_master_volume_table[effect][table][now->master_volume].canary);
		}
	}

	if(!lout_vol_mute)
		cxd3776er_register_write(CXD3776ER_LINEOUT_VOL, cxd3776er_master_volume_table[effect][table][now->master_volume].lineout);

	if(!hpout_vol_mute)
		cxd3776er_register_write(CXD3776ER_HPOUT_VOL, cxd3776er_master_volume_table[effect][table][now->master_volume].hpout);

	if(hw && !hw_mute){
		msleep(100);
		set_hardware_mute(now->output_path,now->output_select,OFF);
	}

	if(!pwm_out_mute)
		set_pwm_out_mute(OFF);

	if(!dac_vol_mute)
		fade_digital_volume(CXD3776ER_DAC_VOL, cxd3776er_master_volume_table[effect][table][now->master_volume].dac);

	if(!dgt_vol_mute){
		cxd3776er_register_write(CXD3776ER_DGT_VOL_0H,cxd3776er_master_volume_table[effect][table][now->master_volume].cmx1);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_1H,cxd3776er_master_volume_table[effect][table][now->master_volume].cmx1);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_0L,cxd3776er_master_volume_table[effect][table][now->master_volume].cmx2);
		cxd3776er_register_write(CXD3776ER_DGT_VOL_1L,cxd3776er_master_volume_table[effect][table][now->master_volume].cmx2);
	}
#endif
	return(0);
}

/* for driver */
int cxd3776er_set_input_device_mute(struct cxd3776er_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);
#if 0
	table=get_table_index(now->output_path,now->output_select,now->motion_feedback_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		lin_vol_mute   = lin_vol_mute   |  INPUT_MUTE;
		sdout_vol_mute = sdout_vol_mute |  INPUT_MUTE;
	}
	else{
		lin_vol_mute   = lin_vol_mute   & ~INPUT_MUTE;
		sdout_vol_mute = sdout_vol_mute & ~INPUT_MUTE;
	}

	if(lin_vol_mute)
		fade_digital_volume(CXD3776ER_LINEIN_VOL, 0x33);
	else
		fade_digital_volume(CXD3776ER_LINEIN_VOL, cxd3776er_master_volume_table[effect][table][now->master_volume].linein);

	if(sdout_vol_mute)
		fade_digital_volume(CXD3776ER_SDOUT_VOL, 0x33);
	else
		fade_digital_volume(CXD3776ER_SDOUT_VOL, cxd3776er_master_gain_table[now->master_gain]);
#endif
	return(0);
}

static int fade_digital_volume(unsigned int address, unsigned int volume)
{
	unsigned int tmp;
	int req;
	int now;
	int rv;

	/* cxd3776er_register_write(address,volume); */

//	req=dgtvolint(volume);

	rv=cxd3776er_register_read(address,&now);
	if(rv<0)
		now=0x33; /* for safe */

	/* for safe */
	/* if(tmp>0x00 && tmp<=0x18) */
		/* tmp=0x33; */

//	now=dgtvolint(tmp);

	if(now<volume){
		while(now!=volume){
			/* now++; */
			now=minimum(now+8,volume);
			cxd3776er_register_write(address,now);
			usleep_range(1000,1200);
		}
	}
	else if(now>volume){
		while(now!=volume){
			/* now--; */
			now=maximum(now-8,volume);
			cxd3776er_register_write(address,now);
			usleep_range(1000,1200);
		}
	}

	return(0);
}

static int set_pwm_out_mute(int mute)
{
	print_trace("%s(%d)\n",__FUNCTION__,mute);
#if 0
	if(mute){
		print_debug("pwm out mute = ON\n");
		cxd3776er_register_modify(CXD3776ER_S_MASTER_CONTROL,0x80,0x80);
	}
	else{
		print_debug("pwm out mute = OFF\n");
		cxd3776er_register_modify(CXD3776ER_S_MASTER_CONTROL,0x00,0x80);
	}
#endif
	return(0);
}

static int set_hardware_mute(int output_path, int output_select, int mute)
{
	print_trace("%s(%d,%d,%d)\n",__FUNCTION__,output_path,output_select,mute);
#if 0
	if(mute){

		digiamp_switch_shunt_mute(ON);

		cxd3776er_switch_smaster_mute(ON);
		cxd3776er_switch_class_h_mute(ON);

		msleep(50);
	}

	if(!mute){

		if(output_path==OUTPUT_SELECT_NONE_HEADPHONE){
			if(output_select==HEADPHONE_AMP_SMASTER){
				cxd3776er_switch_smaster_mute(OFF);
				cxd3776er_switch_class_h_mute(ON);
			}
			else{
				cxd3776er_switch_class_h_mute(OFF);
				cxd3776er_switch_smaster_mute(ON);
			}
		}
		else{
			cxd3776er_switch_class_h_mute(ON);
			cxd3776er_switch_smaster_mute(ON);
		}

		msleep(50);

		digiamp_switch_shunt_mute(OFF);
	}
#endif
	return(0);
}

static int get_table_index(int output_path, int output_select, int motion_feedback_active)
{
	int table;
#if 0
	switch(output_path){
		case OUTPUT_SELECT_NONE_HEADPHONE:
			if(output_select==HEADPHONE_AMP_SMASTER){
				if(motion_feedback_active)
					table=MASTER_VOLUME_TABLE_SMASTER_NC;
				else
					table=MASTER_VOLUME_TABLE_SMASTER;
			}
			else{
				if(motion_feedback_active)
					table=MASTER_VOLUME_TABLE_CLASSH_NC;
				else
					table=MASTER_VOLUME_TABLE_CLASSH;
			}
			break;
		case OUTPUT_SELECT_NONE_LINE:
			table=MASTER_VOLUME_TABLE_LINE;
			break;
		case OUTPUT_SELECT_NONE_SPEAKER:
			table=MASTER_VOLUME_TABLE_SPEAKER;
			break;
		case OUTPUT_SELECT_NONE_FIXEDLINE:
			table=MASTER_VOLUME_TABLE_FIXEDLINE;
			break;
		case OUTPUT_PATH_NONE:
		default:
			table=MASTER_VOLUME_TABLE_OFF;
			break;
	}
#endif
	return(table);
}









