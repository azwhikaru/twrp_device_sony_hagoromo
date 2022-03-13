/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf_volume.c
 *
 * CXD3774GF CODEC driver
 *
 * Copyright (c) 2013 Sony Corporation
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

#include "cxd3774gf_common.h"

#define dgtvolint(_a) ( (_a)>=0x19 ? (int)(0xFFFFFF00|(_a)) : (int)(_a) )
#define anavolint(_a) ( (_a)>=0x40 ? (int)(0xFFFFFF80|(_a)) : (int)(_a) )
#define pgavolint(_a) ( (_a)>=0x20 ? (int)(0xFFFFFFC0|(_a)) : (int)(_a) )
#define adcvolint(_a) ( (_a)>=0x01 ? (int)(0xFFFFFF80|(_a)) : (int)(_a) )

static int fade_digital_volume(unsigned int address, unsigned int volume);
static int set_pwm_out_mute(int mute);
static int set_hardware_mute(int output_device, int headphone_amp, int mute);
static int get_table_index(int output_device, int headphone_amp, int noise_cancel_active);

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

static int initialized=FALSE;

int cxd3774gf_volume_initialize(void)
{
	initialized=TRUE;

	return(0);
}

int cxd3774gf_volume_finalize(void)
{
	if(!initialized)
		return(0);

	initialized=FALSE;

	return(0);
}

/* msater volume */
int cxd3774gf_set_master_volume(struct cxd3774gf_status * now, int volume)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,volume);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active);
	effect=(now->sound_effect ? 1 : 0);

	if(dgt_vol_mute){
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0H,0x00);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1H,0x00);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0L,0x00);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1L,0x00);
	}
	else{
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0H,cxd3774gf_master_volume_table[effect][table][volume].cmx1);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1H,cxd3774gf_master_volume_table[effect][table][volume].cmx1);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0L,cxd3774gf_master_volume_table[effect][table][volume].cmx2);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1L,cxd3774gf_master_volume_table[effect][table][volume].cmx2);
	}

	if(lin_vol_mute)
		cxd3774gf_register_write(CXD3774GF_LINEIN_VOL, 0x33);
	else
		cxd3774gf_register_write(CXD3774GF_LINEIN_VOL, cxd3774gf_master_volume_table[effect][table][volume].linein);

	if(sdin1_vol_mute)
		cxd3774gf_register_write(CXD3774GF_SDIN_1_VOL, 0x33);
	else
		cxd3774gf_register_write(CXD3774GF_SDIN_1_VOL, cxd3774gf_master_volume_table[effect][table][volume].sdin1);

	if(sdin2_vol_mute)
		cxd3774gf_register_write(CXD3774GF_SDIN_2_VOL, 0x33);
	else
		cxd3774gf_register_write(CXD3774GF_SDIN_2_VOL, cxd3774gf_master_volume_table[effect][table][volume].sdin2);

	if(dac_vol_mute)
		cxd3774gf_register_write(CXD3774GF_DAC_VOL, 0x33);
	else
		cxd3774gf_register_write(CXD3774GF_DAC_VOL, cxd3774gf_master_volume_table[effect][table][volume].dac);

	if(hpout_vol_mute)
		cxd3774gf_register_write(CXD3774GF_HPOUT_VOL, 0x40);
	else
		cxd3774gf_register_write(CXD3774GF_HPOUT_VOL, cxd3774gf_master_volume_table[effect][table][volume].hpout);

	if(lout_vol_mute)
		cxd3774gf_register_write(CXD3774GF_LINEOUT_VOL, 0x40);
	else
		cxd3774gf_register_write(CXD3774GF_LINEOUT_VOL, cxd3774gf_master_volume_table[effect][table][volume].lineout);

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
			digiamp_fade_volume(cxd3774gf_master_volume_table[effect][table][volume].cxd90020);
		else if(digiamp_get_type()==DAMP_TYPE_CANARY)
			digiamp_fade_volume(cxd3774gf_master_volume_table[effect][table][volume].canary);
	}

	return(0);
}

/* master gain */
int cxd3774gf_set_master_gain(struct cxd3774gf_status * now, int gain)
{
	print_trace("%s(%d)\n",__FUNCTION__,gain);

	if(sdout_vol_mute)
		cxd3774gf_register_write(CXD3774GF_SDOUT_VOL, 0x33);
	else
		cxd3774gf_register_write(CXD3774GF_SDOUT_VOL, cxd3774gf_master_gain_table[now->master_gain]);

	return(0);
}

/* for ??? */
int cxd3774gf_set_playback_mute(struct cxd3774gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		dac_vol_mute = dac_vol_mute |  PLAYBACK_MUTE;
	}
	else{
		dac_vol_mute = dac_vol_mute & ~PLAYBACK_MUTE;
	}

	if(dac_vol_mute)
		fade_digital_volume(CXD3774GF_DAC_VOL, 0x33);
	else
		fade_digital_volume(CXD3774GF_DAC_VOL, cxd3774gf_master_volume_table[effect][table][now->master_volume].dac);

	return(0);
}

/* mic mute */
int cxd3774gf_set_capture_mute(struct cxd3774gf_status * now, int mute)
{
	print_trace("%s(%d)\n",__FUNCTION__,mute);

	if(mute){
		sdout_vol_mute = sdout_vol_mute |  INPUT_MUTE;
	}
	else{
		sdout_vol_mute = sdout_vol_mute & ~INPUT_MUTE;
	}

	if(sdout_vol_mute)
		cxd3774gf_register_write(CXD3774GF_SDOUT_VOL, 0x33);
	else
		cxd3774gf_register_write(CXD3774GF_SDOUT_VOL, cxd3774gf_master_gain_table[now->master_gain]);

	return(0);
}

/* for application */
int cxd3774gf_set_analog_playback_mute(struct cxd3774gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		lin_vol_mute = lin_vol_mute |  ANALOG_MUTE;
	}
	else{
		lin_vol_mute = lin_vol_mute & ~ANALOG_MUTE;
	}

	if(lin_vol_mute)
		cxd3774gf_register_write(CXD3774GF_LINEIN_VOL, 0x33);
	else
		cxd3774gf_register_write(CXD3774GF_LINEIN_VOL, cxd3774gf_master_volume_table[effect][table][now->master_volume].linein);

	return(0);
}

/* for audio policy manager */
int cxd3774gf_set_analog_stream_mute(struct cxd3774gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		lin_vol_mute = lin_vol_mute |  STREAM_MUTE;
	}
	else{
		lin_vol_mute = lin_vol_mute & ~STREAM_MUTE;
	}

	if(lin_vol_mute)
		cxd3774gf_register_write(CXD3774GF_LINEIN_VOL, 0x33);
	else
		cxd3774gf_register_write(CXD3774GF_LINEIN_VOL, cxd3774gf_master_volume_table[effect][table][now->master_volume].linein);

	return(0);
}

/* for device switch */
int cxd3774gf_set_mix_timed_mute(struct cxd3774gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		dac_vol_mute = dac_vol_mute |  TIMED_MUTE;
	}
	else{
		dac_vol_mute = dac_vol_mute & ~TIMED_MUTE;
	}

	if(dac_vol_mute)
		fade_digital_volume(CXD3774GF_DAC_VOL, 0x33);
	else
		fade_digital_volume(CXD3774GF_DAC_VOL, cxd3774gf_master_volume_table[effect][table][now->master_volume].dac);

	return(0);
}

/* for STD sound effect */
int cxd3774gf_set_std_timed_mute(struct cxd3774gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		sdin2_vol_mute = sdin2_vol_mute |  STD_TIMED_MUTE;
	}
	else{
		sdin2_vol_mute = sdin2_vol_mute & ~STD_TIMED_MUTE;
	}

	if(sdin2_vol_mute)
		fade_digital_volume(CXD3774GF_SDIN_2_VOL, 0x33);
	else
		fade_digital_volume(CXD3774GF_SDIN_2_VOL, cxd3774gf_master_volume_table[effect][table][now->master_volume].sdin2);

	return(0);
}

/* for ICX sound effect */
int cxd3774gf_set_icx_timed_mute(struct cxd3774gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		sdin1_vol_mute = sdin1_vol_mute |  ICX_TIMED_MUTE;
	}
	else{
		sdin1_vol_mute = sdin1_vol_mute & ~ICX_TIMED_MUTE;
	}

	if(sdin1_vol_mute)
		fade_digital_volume(CXD3774GF_SDIN_1_VOL, 0x33);
	else
		fade_digital_volume(CXD3774GF_SDIN_1_VOL, cxd3774gf_master_volume_table[effect][table][now->master_volume].sdin1);

	return(0);
}

/* for no pcm */
int cxd3774gf_set_no_pcm_mute(int mute)
{
	print_trace("%s(%d)\n",__FUNCTION__,mute);

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

	return(0);
}

int cxd3774gf_set_uncg_mute(struct cxd3774gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active);
	effect=(now->sound_effect ? 1 : 0);

	if(mute){
		dgt_vol_mute = dgt_vol_mute |  UNCG_MUTE;
	}
	else{
		dgt_vol_mute = dgt_vol_mute & ~UNCG_MUTE;
	}

	if(dgt_vol_mute){
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0H,0x00);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1H,0x00);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0L,0x00);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1L,0x00);
	}
	else{
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0H,cxd3774gf_master_volume_table[effect][table][now->master_volume].cmx1);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1H,cxd3774gf_master_volume_table[effect][table][now->master_volume].cmx1);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0L,cxd3774gf_master_volume_table[effect][table][now->master_volume].cmx2);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1L,cxd3774gf_master_volume_table[effect][table][now->master_volume].cmx2);
	}

	return(0);
}

/* for driver */
int cxd3774gf_set_output_device_mute(struct cxd3774gf_status * now, int mute, int hw)
{
	int effect;
	int table;

	print_trace("%s(%d,%d)\n",__FUNCTION__,mute,hw);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active);
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
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0H,0x00);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1H,0x00);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0L,0x00);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1L,0x00);
	}

	if(dac_vol_mute)
		fade_digital_volume(CXD3774GF_DAC_VOL, 0x33);

	if(pwm_out_mute)
		set_pwm_out_mute(ON);

	if(hw && hw_mute){
		set_hardware_mute(now->output_device,now->headphone_amp,ON);
		msleep(100);
	}

	if(hpout_vol_mute)
		cxd3774gf_register_write(CXD3774GF_HPOUT_VOL, 0x40);

	if(lout_vol_mute)
		cxd3774gf_register_write(CXD3774GF_LINEOUT_VOL, 0x40);

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
			digiamp_fade_volume(cxd3774gf_master_volume_table[effect][table][now->master_volume].cxd90020);
		}
		else if(digiamp_get_type()==DAMP_TYPE_CANARY){
			digiamp_fade_volume(cxd3774gf_master_volume_table[effect][table][now->master_volume].canary);
		}
	}

	if(!lout_vol_mute)
		cxd3774gf_register_write(CXD3774GF_LINEOUT_VOL, cxd3774gf_master_volume_table[effect][table][now->master_volume].lineout);

	if(!hpout_vol_mute)
		cxd3774gf_register_write(CXD3774GF_HPOUT_VOL, cxd3774gf_master_volume_table[effect][table][now->master_volume].hpout);

	if(hw && !hw_mute){
		msleep(100);
		set_hardware_mute(now->output_device,now->headphone_amp,OFF);
	}

	if(!pwm_out_mute)
		set_pwm_out_mute(OFF);

	if(!dac_vol_mute)
		fade_digital_volume(CXD3774GF_DAC_VOL, cxd3774gf_master_volume_table[effect][table][now->master_volume].dac);

	if(!dgt_vol_mute){
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0H,cxd3774gf_master_volume_table[effect][table][now->master_volume].cmx1);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1H,cxd3774gf_master_volume_table[effect][table][now->master_volume].cmx1);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_0L,cxd3774gf_master_volume_table[effect][table][now->master_volume].cmx2);
		cxd3774gf_register_write(CXD3774GF_DGT_VOL_1L,cxd3774gf_master_volume_table[effect][table][now->master_volume].cmx2);
	}

	return(0);
}

/* for driver */
int cxd3774gf_set_input_device_mute(struct cxd3774gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active);
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
		fade_digital_volume(CXD3774GF_LINEIN_VOL, 0x33);
	else
		fade_digital_volume(CXD3774GF_LINEIN_VOL, cxd3774gf_master_volume_table[effect][table][now->master_volume].linein);

	if(sdout_vol_mute)
		fade_digital_volume(CXD3774GF_SDOUT_VOL, 0x33);
	else
		fade_digital_volume(CXD3774GF_SDOUT_VOL, cxd3774gf_master_gain_table[now->master_gain]);

	return(0);
}

static int fade_digital_volume(unsigned int address, unsigned int volume)
{
	unsigned int tmp;
	int req;
	int now;
	int rv;

	/* cxd3774gf_register_write(address,volume); */

	req=dgtvolint(volume);

	rv=cxd3774gf_register_read(address,&tmp);
	if(rv<0)
		tmp=0x33; /* for safe */

	/* for safe */
	/* if(tmp>0x00 && tmp<=0x18) */
		/* tmp=0x33; */

	now=dgtvolint(tmp);

	if(now<req){
		while(now!=req){
			/* now++; */
			now=minimum(now+8,req);
			cxd3774gf_register_write(address,now);
			usleep_range(1000,1200);
		}
	}
	else if(now>req){
		while(now!=req){
			/* now--; */
			now=maximum(now-8,req);
			cxd3774gf_register_write(address,now);
			usleep_range(1000,1200);
		}
	}

	return(0);
}

static int set_pwm_out_mute(int mute)
{
	print_trace("%s(%d)\n",__FUNCTION__,mute);

	if(mute){
		print_debug("pwm out mute = ON\n");
		cxd3774gf_register_modify(CXD3774GF_S_MASTER_CONTROL,0x80,0x80);
	}
	else{
		print_debug("pwm out mute = OFF\n");
		cxd3774gf_register_modify(CXD3774GF_S_MASTER_CONTROL,0x00,0x80);
	}

	return(0);
}

static int set_hardware_mute(int output_device, int headphone_amp, int mute)
{
	print_trace("%s(%d,%d,%d)\n",__FUNCTION__,output_device,headphone_amp,mute);

	if(mute){

		digiamp_switch_shunt_mute(ON);

		cxd3774gf_switch_smaster_mute(ON);
		cxd3774gf_switch_class_h_mute(ON);

		msleep(50);
	}

	if(!mute){

		if(output_device==OUTPUT_DEVICE_HEADPHONE){
			if(headphone_amp==HEADPHONE_AMP_SMASTER){
				cxd3774gf_switch_smaster_mute(OFF);
				cxd3774gf_switch_class_h_mute(ON);
			}
			else{
				cxd3774gf_switch_class_h_mute(OFF);
				cxd3774gf_switch_smaster_mute(ON);
			}
		}
		else{
			cxd3774gf_switch_class_h_mute(ON);
			cxd3774gf_switch_smaster_mute(ON);
		}

		msleep(50);

		digiamp_switch_shunt_mute(OFF);
	}

	return(0);
}

static int get_table_index(int output_device, int headphone_amp, int noise_cancel_active)
{
	int table;

	switch(output_device){
		case OUTPUT_DEVICE_HEADPHONE:
			if(headphone_amp==HEADPHONE_AMP_SMASTER){
				if(noise_cancel_active)
					table=MASTER_VOLUME_TABLE_SMASTER_NC;
				else
					table=MASTER_VOLUME_TABLE_SMASTER;
			}
			else{
				if(noise_cancel_active)
					table=MASTER_VOLUME_TABLE_CLASSH_NC;
				else
					table=MASTER_VOLUME_TABLE_CLASSH;
			}
			break;
		case OUTPUT_DEVICE_LINE:
			table=MASTER_VOLUME_TABLE_LINE;
			break;
		case OUTPUT_DEVICE_SPEAKER:
			table=MASTER_VOLUME_TABLE_SPEAKER;
			break;
		case OUTPUT_DEVICE_FIXEDLINE:
			table=MASTER_VOLUME_TABLE_FIXEDLINE;
			break;
		case OUTPUT_DEVICE_NONE:
		default:
			table=MASTER_VOLUME_TABLE_OFF;
			break;
	}

	return(table);
}









