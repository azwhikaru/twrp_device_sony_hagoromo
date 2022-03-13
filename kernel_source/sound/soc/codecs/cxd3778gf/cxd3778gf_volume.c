/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3778gf_volume.c
 *
 * CXD3778GF CODEC driver
 *
 * Copyright (c) 2013-2016 Sony Corporation
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

#include "cxd3778gf_common.h"

#define dgtvolint(_a) ( (_a)>=0x19 ? (int)(0xFFFFFF00|(_a)) : (int)(_a) )
#define anavolint(_a) ( (_a)>=0x40 ? (int)(0xFFFFFF80|(_a)) : (int)(_a) )
#define pgavolint(_a) ( (_a)>=0x20 ? (int)(0xFFFFFFC0|(_a)) : (int)(_a) )
#define adcvolint(_a) ( (_a)>=0x01 ? (int)(0xFFFFFF80|(_a)) : (int)(_a) )

static int fade_volume(unsigned int address, unsigned int volume);
static int fade_sms_dsd_volume(unsigned int address, int volume, int mask);
static int set_pwm_out_mute(int mute);
static int set_hardware_mute(int output_device, int headphone_amp, int mute);
static int get_table_index(int output_device, int headphone_amp, int noise_cancel_active, int format,
				int rate, int headphone_smaster_se_gain_mode, int headphone_smaster_btl_gain_mode, unsigned int board_type);

uint    cxd3778gf_monvol_wait_ms = 150;
module_param_named(monvol_wait_ms, cxd3778gf_monvol_wait_ms, uint, S_IWUSR | S_IRUGO);

int			cxd3778gf_fade_amount = 1;
static const int	FADE_AMOUNT_MAX = 20;
static const int	FADE_AMOUNT_MIN = 1;

static int set_param_fade_amount(const char *value, const struct kernel_param *kp)
{
	long			value_num;
	int			ret;

	ret = strict_strtol(value, 0, &value_num);
	if (ret != 0) {
		return ret;
	}

	/* fade_amount: 1(min) - 20(max) */
	if (FADE_AMOUNT_MIN <= value_num && value_num <= FADE_AMOUNT_MAX) {
		cxd3778gf_fade_amount = value_num;
	} else {
		return -EINVAL;
	}

	return 0;
}

static int get_param_fade_amount(char *value, const struct kernel_param *kp)
{
	return param_get_int(value, kp);
}

static const struct kernel_param_ops	param_ops_fade_amount = {
	.set = set_param_fade_amount,
	.get = get_param_fade_amount,
};

module_param_cb(fade_amount, &param_ops_fade_amount, &cxd3778gf_fade_amount, 0644);

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
#define DSD_TIMED_MUTE 0x00000800

static unsigned int sdout_vol_mute = INPUT_MUTE;
static unsigned int sdin1_vol_mute = 0x00;
static unsigned int sdin2_vol_mute = 0x00;
static unsigned int lin_vol_mute   = INPUT_MUTE;
static unsigned int codec_play_vol_mute   = 0x00;
static unsigned int dgt_vol_mute   = 0x00;
static unsigned int dsdenc_vol_mute  = OUTPUT_MUTE;
static unsigned int hpout_vol_mute = OUTPUT_MUTE;
static unsigned int lout_vol_mute  = OUTPUT_MUTE;
static unsigned int pwm_out_mute   = OUTPUT_MUTE;
static unsigned int hw_mute        = OUTPUT_MUTE;

static int initialized=FALSE;

int cxd3778gf_volume_initialize(void)
{
	initialized=TRUE;

	return(0);
}

int cxd3778gf_volume_finalize(void)
{
	if(!initialized)
		return(0);

	initialized=FALSE;

	return(0);
}

/* msater volume */
int cxd3778gf_set_master_volume(struct cxd3778gf_status * now, int volume)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,volume);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(now->format == PCM_MODE) {
		if(dgt_vol_mute){
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_H,0x00);
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL1_H,0x00);
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_L,0x00);
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL1_L,0x00);
			if (now->noise_cancel_active == TRUE) {
				cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_H,0x00);
				cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_L,0x00);
			}
		}
		else{
			if(now->headphone_type==NCHP_TYPE_MODEL1){
				cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_H,cxd3778gf_master_volume_table[effect][table][volume].cmx1_31);
//				cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL1_H,cxd3778gf_master_volume_table[effect][table][volume].cmx1_31);
				cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_L,cxd3778gf_master_volume_table[effect][table][volume].cmx2_31);
//				cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL1_L,cxd3778gf_master_volume_table[effect][table][volume].cmx2_31);
			} else {
                                cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_H,cxd3778gf_master_volume_table[effect][table][volume].cmx1_750);
//                              cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL1_H,cxd3778gf_master_volume_table[effect][table][volume].cmx1_750);
                                cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_L,cxd3778gf_master_volume_table[effect][table][volume].cmx2_750);
//                              cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL1_L,cxd3778gf_master_volume_table[effect][table][volume].cmx2_750);
			}
			if (now->noise_cancel_active == TRUE) {
				cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_H,0x20);
				cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_L,0x00);
			}
		}

		if(lin_vol_mute)
			cxd3778gf_register_write(CXD3778GF_CODEC_AINVOL, 0x33);
		else
			cxd3778gf_register_write(CXD3778GF_CODEC_AINVOL, cxd3778gf_master_volume_table[effect][table][volume].linein);

		if(sdin1_vol_mute)
			cxd3778gf_register_write(CXD3778GF_CODEC_SDIN1VOL, 0x33);
		else
			cxd3778gf_register_write(CXD3778GF_CODEC_SDIN1VOL, cxd3778gf_master_volume_table[effect][table][volume].sdin1);

		if(sdin2_vol_mute)
			cxd3778gf_register_write(CXD3778GF_CODEC_SDIN2VOL, 0x33);
		else
			cxd3778gf_register_write(CXD3778GF_CODEC_SDIN2VOL, cxd3778gf_master_volume_table[effect][table][volume].sdin2);

		if(codec_play_vol_mute)
			cxd3778gf_register_write(CXD3778GF_CODEC_PLAYVOL, 0x33);
		else
			cxd3778gf_register_write(CXD3778GF_CODEC_PLAYVOL, cxd3778gf_master_volume_table[effect][table][volume].play);
	} else {
#ifdef CONFIG_SND_SOC_MT8590
		if(dsdenc_vol_mute){
			afe_write(DSD_ENC_CON1, 0x0000);
			afe_write(DSD_ENC_CON2, 0x0000);
		} else {
			afe_write(DSD_ENC_CON1, cxd3778gf_master_volume_dsd_table[table][volume]);
			afe_write(DSD_ENC_CON2, cxd3778gf_master_volume_dsd_table[table][volume]);
		}
#endif
	}

	if(hpout_vol_mute){
		cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
		cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
	}
	else{
		if(now->format != DSD_MODE){
			if (now->headphone_amp == HEADPHONE_AMP_NORMAL) {
				if (cxd3778gf_master_volume_table[effect][table][volume].hpout - now->balance_volume_l < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][volume].hpout - now->balance_volume_l);
				if (cxd3778gf_master_volume_table[effect][table][volume].hpout - now->balance_volume_r < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][volume].hpout - now->balance_volume_r);
			} else {
				if (cxd3778gf_master_volume_table[effect][table][volume].hpout - (now->balance_volume_l * 4) < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][volume].hpout - (now->balance_volume_l * 4));
				if (cxd3778gf_master_volume_table[effect][table][volume].hpout - (now->balance_volume_r * 4) < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][volume].hpout - (now->balance_volume_r * 4));
			}
		} else {
			cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][volume].hpout);
			cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][volume].hpout);
		}
		cxd3778gf_register_modify(CXD3778GF_HPOUT2_CTRL3, cxd3778gf_master_volume_table[effect][table][volume].hpout2_ctrl3, 0x03);
	}

	if(lout_vol_mute)
		cxd3778gf_register_write(CXD3778GF_LINEOUT_VOL, 0x00);
	else
		cxd3778gf_register_write(CXD3778GF_LINEOUT_VOL, cxd3778gf_master_volume_table[effect][table][volume].lineout);

	return(0);
}

int cxd3778gf_set_dsd_adjust_volume(struct cxd3778gf_status * now, int volume)
{
        int table;
	int effect;

        print_trace("%s(%d)\n",__FUNCTION__,volume);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

#ifdef CONFIG_SND_SOC_MT8590
        if(dsdenc_vol_mute){
                afe_write(DSD_ENC_CON1, 0x0000);
                afe_write(DSD_ENC_CON2, 0x0000);
        } else {
                afe_write(DSD_ENC_CON1, cxd3778gf_master_volume_dsd_table[table][volume]);
                afe_write(DSD_ENC_CON2, cxd3778gf_master_volume_dsd_table[table][volume]);
        }
#endif

        if(hpout_vol_mute){
                cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
                cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
        }
        else{
		if(now->format != DSD_MODE){
			if (now->headphone_amp == HEADPHONE_AMP_NORMAL) {
				if (cxd3778gf_master_volume_table[effect][table][volume].hpout - now->balance_volume_l < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][volume].hpout - now->balance_volume_l);
				if (cxd3778gf_master_volume_table[effect][table][volume].hpout - now->balance_volume_r < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][volume].hpout - now->balance_volume_r);
			} else {
				if (cxd3778gf_master_volume_table[effect][table][volume].hpout - (now->balance_volume_l * 4) < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][volume].hpout - (now->balance_volume_l * 4));
				if (cxd3778gf_master_volume_table[effect][table][volume].hpout - (now->balance_volume_r * 4) < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][volume].hpout - (now->balance_volume_r * 4));
			}
		} else {
			cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][volume].hpout);
			cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][volume].hpout);
		}
        }

        if(lout_vol_mute)
                cxd3778gf_register_write(CXD3778GF_LINEOUT_VOL, 0x00);
        else
                cxd3778gf_register_write(CXD3778GF_LINEOUT_VOL, cxd3778gf_master_volume_table[effect][table][volume].lineout);

        return(0);

}
/* master gain */
int cxd3778gf_set_master_gain(struct cxd3778gf_status * now, int gain)
{
	print_trace("%s(%d)\n",__FUNCTION__,gain);

	if(sdout_vol_mute)
		cxd3778gf_register_write(CXD3778GF_CODEC_RECVOL, 0x33);
	else
		cxd3778gf_register_write(CXD3778GF_CODEC_RECVOL, 0x00); //if table is needed, use cxd3778gf_master_gain_table[now->master_gain]

	return(0);
}

/* for ??? */
int cxd3778gf_set_playback_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		codec_play_vol_mute = codec_play_vol_mute |  PLAYBACK_MUTE;
	}
	else{
		codec_play_vol_mute = codec_play_vol_mute & ~PLAYBACK_MUTE;
	}

	if(codec_play_vol_mute)
		cxd3778gf_register_write(CXD3778GF_CODEC_PLAYVOL, 0x33);
	else
		cxd3778gf_register_write(CXD3778GF_CODEC_PLAYVOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].play);

	return(0);
}

int cxd3778gf_set_phv_mute(struct cxd3778gf_status * now, int mute)
{
        int effect;
        int table;

        print_trace("%s(%d)\n",__FUNCTION__,mute);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
                now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

        if(mute){
		hpout_vol_mute = hpout_vol_mute | PLAYBACK_MUTE;
        }
        else{
                hpout_vol_mute = hpout_vol_mute & ~PLAYBACK_MUTE;
        }

        if(hpout_vol_mute){
		cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
		cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
		msleep(20);
	}
        else{
		if(now->format != DSD_MODE){
			if (now->headphone_amp == HEADPHONE_AMP_NORMAL) {
				if (cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - now->balance_volume_l < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - now->balance_volume_l);
				if (cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - now->balance_volume_r < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - now->balance_volume_r);
			} else {
				if (cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - (now->balance_volume_l * 4) < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - (now->balance_volume_l * 4));
				if (cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - (now->balance_volume_r * 4) < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - (now->balance_volume_r * 4));
			}
		} else {
			cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout);
			cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout);
		}
		msleep(20);
	}

	return(0);
}

int cxd3778gf_set_line_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		lout_vol_mute = lout_vol_mute | PLAYBACK_MUTE;
	}
	else{
		lout_vol_mute = lout_vol_mute & ~PLAYBACK_MUTE;
	}

	if(lout_vol_mute)
		cxd3778gf_register_write(CXD3778GF_LINEOUT_VOL, 0x00);
	else
		cxd3778gf_register_write(CXD3778GF_LINEOUT_VOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].lineout);

	return(0);
}

/* mic mute */
int cxd3778gf_set_capture_mute(struct cxd3778gf_status * now, int mute)
{
	print_trace("%s(%d)\n",__FUNCTION__,mute);

	if(mute){
		sdout_vol_mute = sdout_vol_mute |  INPUT_MUTE;
	}
	else{
		sdout_vol_mute = sdout_vol_mute & ~INPUT_MUTE;
	}

	if(sdout_vol_mute)
		cxd3778gf_register_write(CXD3778GF_CODEC_RECVOL, 0x33);
	else
		cxd3778gf_register_write(CXD3778GF_CODEC_RECVOL, 0x00); //if table is needed, use cxd3778gf_master_gain_table[now->master_gain]

	return(0);
}

/* for application */
int cxd3778gf_set_analog_playback_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		lin_vol_mute = lin_vol_mute |  ANALOG_MUTE;
	}
	else{
		lin_vol_mute = lin_vol_mute & ~ANALOG_MUTE;
	}

	if(lin_vol_mute)
		fade_volume(CXD3778GF_CODEC_AINVOL, 0x33);
	else
		fade_volume(CXD3778GF_CODEC_AINVOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].linein);

	return(0);
}

/* for audio policy manager */
int cxd3778gf_set_analog_stream_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		lin_vol_mute = lin_vol_mute |  STREAM_MUTE;
	}
	else{
		lin_vol_mute = lin_vol_mute & ~STREAM_MUTE;
	}

	if(lin_vol_mute)
		cxd3778gf_register_write(CXD3778GF_CODEC_AINVOL, 0x33);
	else
		cxd3778gf_register_write(CXD3778GF_CODEC_AINVOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].linein);

	return(0);
}

/* for device switch */
int cxd3778gf_set_mix_timed_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		codec_play_vol_mute = codec_play_vol_mute |  TIMED_MUTE;
	}
	else{
		codec_play_vol_mute = codec_play_vol_mute & ~TIMED_MUTE;
	}

	if(codec_play_vol_mute)
		cxd3778gf_register_write(CXD3778GF_CODEC_PLAYVOL, 0x33);
	else
		cxd3778gf_register_write(CXD3778GF_CODEC_PLAYVOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].play);

	return(0);
}

/* for STD sound effect */
int cxd3778gf_set_std_timed_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		sdin2_vol_mute = sdin2_vol_mute |  STD_TIMED_MUTE;
	}
	else{
		sdin2_vol_mute = sdin2_vol_mute & ~STD_TIMED_MUTE;
	}

	if(sdin2_vol_mute)
		cxd3778gf_register_write(CXD3778GF_CODEC_SDIN2VOL, 0x33);
	else
		cxd3778gf_register_write(CXD3778GF_CODEC_SDIN2VOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].sdin2);

	return(0);
}

/* for ICX sound effect */
int cxd3778gf_set_icx_timed_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);

	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		sdin1_vol_mute = sdin1_vol_mute |  ICX_TIMED_MUTE;
	}
	else{
		sdin1_vol_mute = sdin1_vol_mute & ~ICX_TIMED_MUTE;
	}

	if(sdin1_vol_mute)
		cxd3778gf_register_write(CXD3778GF_CODEC_SDIN1VOL, 0x33);
	else
		cxd3778gf_register_write(CXD3778GF_CODEC_SDIN1VOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].sdin1);

	return(0);
}

/* for ICX fader mute */
int cxd3778gf_set_icx_fader_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;
	const int MUTE_VOL = 0x33;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		sdin1_vol_mute = sdin1_vol_mute |  PLAYBACK_MUTE;
	}
	else{
		sdin1_vol_mute = sdin1_vol_mute & ~PLAYBACK_MUTE;
	}

	if(sdin1_vol_mute)
		fade_volume(CXD3778GF_CODEC_SDIN1VOL, MUTE_VOL);
	else
		fade_volume(CXD3778GF_CODEC_SDIN1VOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].sdin1);

	return(0);
}

/* for STD fader mute */
int cxd3778gf_set_std_fader_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;
	const int MUTE_VOL = 0x33;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		sdin2_vol_mute = sdin2_vol_mute |  PLAYBACK_MUTE;
	}
	else{
		sdin2_vol_mute = sdin2_vol_mute & ~PLAYBACK_MUTE;
	}

	if(sdin2_vol_mute)
		fade_volume(CXD3778GF_CODEC_SDIN2VOL, MUTE_VOL);
	else
		fade_volume(CXD3778GF_CODEC_SDIN2VOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].sdin2);

	return(0);
}

#ifdef CONFIG_SND_SOC_MT8590
/* for dsd-remastering */
int cxd3778gf_set_dsd_timed_mute(struct cxd3778gf_status * now, int mute)
{
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	if(mute){
		dsdenc_vol_mute  = dsdenc_vol_mute  |  DSD_TIMED_MUTE;
	}
	else{
		dsdenc_vol_mute  = dsdenc_vol_mute  &  ~DSD_TIMED_MUTE;
	}

	if(now->format == DSD_MODE) {
		table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
			now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);

		if(dsdenc_vol_mute){
			afe_write(DSD_ENC_CON1, 0x0000);
			afe_write(DSD_ENC_CON2, 0x0000);
			msleep(20);
			fade_sms_dsd_volume(CXD3778GF_SMS_DSD_CTRL,0x00,0x0F);
			cxd3778gf_register_write(CXD3778GF_SMS_DSD_PMUTE, 0x80);
		}
		else{
			if(now->headphone_amp == HEADPHONE_AMP_SMASTER_BTL)
				cxd3778gf_set_phv_mute(now,TRUE);
			cxd3778gf_register_write(CXD3778GF_SMS_DSD_PMUTE, 0x00);
			afe_write(DSD_ENC_CON1, cxd3778gf_master_volume_dsd_table[table][now->master_volume]);
			afe_write(DSD_ENC_CON2, cxd3778gf_master_volume_dsd_table[table][now->master_volume]);
			if (now->headphone_amp == HEADPHONE_AMP_SMASTER_BTL){
				cxd3778gf_register_modify(CXD3778GF_SMS_DSD_CTRL,0x0F,0x0F);
				cxd3778gf_set_phv_mute(now,FALSE);
			}
			else{
				fade_sms_dsd_volume(CXD3778GF_SMS_DSD_CTRL,0x0F,0x0F);
				msleep(20);
			}
		}
	}
	return(0);
}
#endif

/* for no pcm */
int cxd3778gf_set_no_pcm_mute(int mute)
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

int cxd3778gf_set_uncg_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		dgt_vol_mute = dgt_vol_mute |  UNCG_MUTE;
	}
	else{
		dgt_vol_mute = dgt_vol_mute & ~UNCG_MUTE;
	}

	if(dgt_vol_mute){
		cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_H,0x00);
		cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL1_H,0x00);
		cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_L,0x00);
		cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL1_L,0x00);
		if (now->noise_cancel_active == TRUE) {
			cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_H,0x00);
			cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_L,0x00);
			msleep(cxd3778gf_monvol_wait_ms);
		}
	}
	else{
		if(now->headphone_type==NCHP_TYPE_MODEL1){
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_H,cxd3778gf_master_volume_table[effect][table][now->master_volume].cmx1_31);
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_L,cxd3778gf_master_volume_table[effect][table][now->master_volume].cmx2_31);
		} else {
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_H,cxd3778gf_master_volume_table[effect][table][now->master_volume].cmx1_750);
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_L,cxd3778gf_master_volume_table[effect][table][now->master_volume].cmx2_750);
		}
		if (now->noise_cancel_active == TRUE) {
			cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_H,0x20);
			cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_L,0x00);
			msleep(cxd3778gf_monvol_wait_ms);
		}
	}

	return(0);
}

/* for driver */
int cxd3778gf_set_output_device_mute(struct cxd3778gf_status * now, int mute, int pwm_wait)
{
	int effect;
	int table;

	print_trace("%s(%d,%d)\n",__FUNCTION__,mute,pwm_wait);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		dgt_vol_mute   = dgt_vol_mute   |  OUTPUT_MUTE;
		codec_play_vol_mute   = codec_play_vol_mute   |  OUTPUT_MUTE;
		hpout_vol_mute = hpout_vol_mute |  OUTPUT_MUTE;
		lout_vol_mute  = lout_vol_mute  |  OUTPUT_MUTE;
		dsdenc_vol_mute  = dsdenc_vol_mute  |  OUTPUT_MUTE;
		pwm_out_mute   = pwm_out_mute   |  OUTPUT_MUTE;
		hw_mute    = hw_mute        |  OUTPUT_MUTE;
	}
	else{
		dgt_vol_mute   = dgt_vol_mute   & ~OUTPUT_MUTE;
		codec_play_vol_mute   = codec_play_vol_mute   & ~OUTPUT_MUTE;
		dsdenc_vol_mute  = dsdenc_vol_mute  & ~OUTPUT_MUTE;
		hpout_vol_mute = hpout_vol_mute & ~OUTPUT_MUTE;
		lout_vol_mute  = lout_vol_mute  & ~OUTPUT_MUTE;
		pwm_out_mute   = pwm_out_mute   & ~OUTPUT_MUTE;
		hw_mute    = hw_mute        & ~OUTPUT_MUTE;
	}

	if(now->format == PCM_MODE){
		if(dgt_vol_mute){
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_H,0x00);
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL1_H,0x00);
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_L,0x00);
			cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL1_L,0x00);
			if (now->noise_cancel_active == TRUE) {
				cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_H,0x00);
				cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_L,0x00);
			//	msleep(cxd3778gf_monvol_wait_ms);
			}
		}

		if(codec_play_vol_mute){
			cxd3778gf_register_write(CXD3778GF_CODEC_PLAYVOL, 0x33);
		}
	}

	if (now->board_type == TYPE_A || (now->board_type == TYPE_Z && now->headphone_amp!=HEADPHONE_AMP_SMASTER_BTL)){
		if(hw_mute){
			set_hardware_mute(now->output_device,now->headphone_amp,ON);
			msleep(130);
		}
	}

        if(lout_vol_mute)
                cxd3778gf_register_write(CXD3778GF_LINEOUT_VOL, 0x00);

#ifdef CONFIG_SND_SOC_MT8590
	if(dsdenc_vol_mute){
		afe_write(DSD_ENC_CON1, 0x0000);
		afe_write(DSD_ENC_CON2, 0x0000);
	}
#endif
	if(hpout_vol_mute){
		cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
		cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
		msleep(20);
//		if(pwm_wait){
		/* wait till mute by fader codec vol and phv vol */
//		msleep(150);
//		}
	}

	if(pwm_out_mute)
		set_pwm_out_mute(ON);

	if (now->board_type == TYPE_Z && now->headphone_amp==HEADPHONE_AMP_SMASTER_BTL){
		if(hw_mute){
			msleep(50);
			set_hardware_mute(now->output_device,now->headphone_amp,ON);
			msleep(150);
		}
	}
#if 0
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
#endif
	if(!dgt_vol_mute){
		if (now->noise_cancel_active == TRUE) {
			cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_H,0x20);
			cxd3778gf_register_write(CXD3778GF_DNC1_MONVOL0_L,0x00);
		}
	}

	if (now->board_type == TYPE_Z && now->headphone_amp==HEADPHONE_AMP_SMASTER_BTL){
		if(!hw_mute){
			msleep(50);
			set_hardware_mute(now->output_device,now->headphone_amp,OFF);
			msleep(50);
		}
	}

	if(!pwm_out_mute){
		set_pwm_out_mute(OFF);
	}

	if(!lout_vol_mute)
		cxd3778gf_register_write(CXD3778GF_LINEOUT_VOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].lineout);

	if(!hpout_vol_mute){
		if(now->format != DSD_MODE){
			if (now->headphone_amp == HEADPHONE_AMP_NORMAL) {
				if (cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - now->balance_volume_l < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - now->balance_volume_l);
				if (cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - now->balance_volume_r < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - now->balance_volume_r);
			} else {
				if (cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - (now->balance_volume_l * 4) < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_L, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - (now->balance_volume_l * 4));
				if (cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - (now->balance_volume_r * 4) < 0x00)
					cxd3778gf_register_write(CXD3778GF_PHV_R, 0x00);
				else
					cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout - (now->balance_volume_r * 4));
			}
		} else {
			cxd3778gf_register_write(CXD3778GF_PHV_L, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout);
			cxd3778gf_register_write(CXD3778GF_PHV_R, cxd3778gf_master_volume_table[effect][table][now->master_volume].hpout);
		}
	}

	if (now->board_type == TYPE_A || (now->board_type == TYPE_Z && now->headphone_amp!=HEADPHONE_AMP_SMASTER_BTL)){
		if(!hw_mute){
			msleep(50);
			set_hardware_mute(now->output_device,now->headphone_amp,OFF);
			msleep(50);
		}
	}

#ifdef CONFIG_SND_SOC_MT8590
        if(!dsdenc_vol_mute){
                afe_write(DSD_ENC_CON1, cxd3778gf_master_volume_dsd_table[table][now->master_volume]);
                afe_write(DSD_ENC_CON2, cxd3778gf_master_volume_dsd_table[table][now->master_volume]);
        }
#endif

	if(now->format == PCM_MODE){
		if(!codec_play_vol_mute){
			cxd3778gf_register_write(CXD3778GF_CODEC_PLAYVOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].play);
		}

		if(!dgt_vol_mute){
			if(now->headphone_type==NCHP_TYPE_MODEL1){
				cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_H,cxd3778gf_master_volume_table[effect][table][now->master_volume].cmx1_31);
				cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_L,cxd3778gf_master_volume_table[effect][table][now->master_volume].cmx2_31);
			} else {
				cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_H,cxd3778gf_master_volume_table[effect][table][now->master_volume].cmx1_750);
				cxd3778gf_register_write(CXD3778GF_DNC1_CANVOL0_L,cxd3778gf_master_volume_table[effect][table][now->master_volume].cmx2_750);
			}
		}
	}

	return(0);
}

/* for driver */
int cxd3778gf_set_input_device_mute(struct cxd3778gf_status * now, int mute)
{
	int effect;
	int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

        table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
		now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);
	if (now->sound_effect && now->sample_rate <= 192000)
		effect=1;
	else
		effect=0;

	if(mute){
		lin_vol_mute   = lin_vol_mute   |  INPUT_MUTE;
		sdout_vol_mute = sdout_vol_mute |  INPUT_MUTE;
	}
	else{
		lin_vol_mute   = lin_vol_mute   & ~INPUT_MUTE;
		sdout_vol_mute = sdout_vol_mute & ~INPUT_MUTE;
	}

	if(lin_vol_mute)
		cxd3778gf_register_write(CXD3778GF_CODEC_AINVOL, 0x33);
	else
		cxd3778gf_register_write(CXD3778GF_CODEC_AINVOL, cxd3778gf_master_volume_table[effect][table][now->master_volume].linein);

	if(sdout_vol_mute)
		cxd3778gf_register_write(CXD3778GF_CODEC_RECVOL, 0x33);
	else
		cxd3778gf_register_write(CXD3778GF_CODEC_RECVOL, cxd3778gf_master_gain_table[now->master_gain]);

	return(0);
}

/*@fade func */
static int fade_volume(unsigned int address, unsigned int volume)
{
	unsigned int tmp;
	int req;
	int now;
	int rv;

	/* cxd3778gf_register_write(address,volume); */

	req=dgtvolint(volume);

	rv=cxd3778gf_register_read(address,&tmp);
	if(rv<0)
		tmp=0x33; /* for safe */

	/* for safe */
	/* if(tmp>0x00 && tmp<=0x18) */
		/* tmp=0x33; */

	now=dgtvolint(tmp);

	if(now<req){ // mute off
		while(now!=req){
			now=minimum(now+cxd3778gf_fade_amount,req);
			cxd3778gf_register_write(address,now);
			usleep_range(100,120);
		}
	}
	else if(now>req){ // mute on
		while(now!=req){
			now=maximum(now-cxd3778gf_fade_amount,req);
			cxd3778gf_register_write(address,now);
			usleep_range(100,120);
		}
	}

	return(0);
}

static int fade_sms_dsd_volume(unsigned int address, int volume, int mask)
{
	int now;
	int req;
	int rv;
	unsigned int tmp;

	rv=cxd3778gf_register_read(address,&tmp);
	if(rv<0)
		tmp=0x00; /* for safe */

	now=(tmp & mask);

	if(now<volume){
		while(now!=volume){
			/* now++; */
			now=minimum(now+3,volume);
			cxd3778gf_register_modify(address,now,mask);
			usleep_range(1000,1200);
                }
        }
	else if(now>volume){
		while(now!=volume){
			/* now--; */
			now=maximum(now-3,volume);
			cxd3778gf_register_modify(address,now,mask);
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
		cxd3778gf_register_modify(CXD3778GF_SMS_NS_PMUTE,0x80,0x80);
	}
	else{
		print_debug("pwm out mute = OFF\n");
		cxd3778gf_register_modify(CXD3778GF_SMS_NS_PMUTE,0x00,0x80);
	}

	return(0);
}

static int set_hardware_mute(int output_device, int headphone_amp, int mute)
{
	print_trace("%s(%d,%d,%d)\n",__FUNCTION__,output_device,headphone_amp,mute);

	if(mute){

//		digiamp_switch_shunt_mute(ON);

		cxd3778gf_switch_smaster_mute(ON,headphone_amp);
//		cxd3778gf_switch_class_h_mute(ON);

		msleep(50);
	}

	if(!mute){

		if(output_device==OUTPUT_DEVICE_HEADPHONE){
				cxd3778gf_switch_smaster_mute(OFF,headphone_amp);
//				cxd3778gf_switch_class_h_mute(OFF);
		}
		else{
//			cxd3778gf_switch_class_h_mute(ON);
			cxd3778gf_switch_smaster_mute(ON,headphone_amp);
		}

		msleep(50);

//		digiamp_switch_shunt_mute(OFF);
	}

	return(0);
}
#ifdef CONFIG_SND_SOC_MT8590
/* for dsd-remastering */
int cxd3778gf_set_dsd_remastering_mute(struct cxd3778gf_status * now, int mute)
{
        int table;

	print_trace("%s(%d)\n",__FUNCTION__,mute);

	if(mute){
		dsdenc_vol_mute  = dsdenc_vol_mute  |  PLAYBACK_MUTE;
	}
	else{
		dsdenc_vol_mute  = dsdenc_vol_mute  &  ~PLAYBACK_MUTE;
	}

	if(now->format == DSD_MODE) {
		table=get_table_index(now->output_device,now->headphone_amp,now->noise_cancel_active,
			now->format, now->sample_rate, now->headphone_smaster_se_gain_mode, now->headphone_smaster_btl_gain_mode, now->board_type);

		if(dsdenc_vol_mute){
			afe_write(DSD_ENC_CON1, 0x0000);
			afe_write(DSD_ENC_CON2, 0x0000);
			if(now->headphone_amp == HEADPHONE_AMP_SMASTER_BTL){
				cxd3778gf_set_phv_mute(now,TRUE);
				cxd3778gf_register_modify(CXD3778GF_SMS_DSD_CTRL,0x00,0x0F);
				cxd3778gf_set_phv_mute(now,FALSE);
			} else {
				fade_sms_dsd_volume(CXD3778GF_SMS_DSD_CTRL,0x00,0x0F);
			}
			cxd3778gf_register_write(CXD3778GF_SMS_DSD_PMUTE,0x80);
		}
		else{
			if(now->headphone_amp == HEADPHONE_AMP_SMASTER_BTL)
				cxd3778gf_set_phv_mute(now,TRUE);
			cxd3778gf_register_write(CXD3778GF_SMS_DSD_PMUTE,0x00);
			afe_write(DSD_ENC_CON1, cxd3778gf_master_volume_dsd_table[table][now->master_volume]);
			afe_write(DSD_ENC_CON2, cxd3778gf_master_volume_dsd_table[table][now->master_volume]);
			if (now->headphone_amp == HEADPHONE_AMP_SMASTER_BTL){
				cxd3778gf_register_modify(CXD3778GF_SMS_DSD_CTRL,0x0F,0x0F);
				cxd3778gf_set_phv_mute(now,FALSE);
			}
			else{
				fade_sms_dsd_volume(CXD3778GF_SMS_DSD_CTRL,0x0F,0x0F);
				//msleep(20); /* no need */
			}
		}
	}
        return(0);
}
#endif

static int get_table_index(int output_device, int headphone_amp, int noise_cancel_active, int format, int rate,
			int headphone_smaster_se_gain_mode, int headphone_smaster_btl_gain_mode, unsigned int board_type)
{
	int table;

	if (format==PCM_MODE) {
		switch(output_device){
			case OUTPUT_DEVICE_HEADPHONE:
				if(headphone_amp==HEADPHONE_AMP_SMASTER_SE){
					if(noise_cancel_active)
						table=MASTER_VOLUME_TABLE_SMASTER_SE_NC;
					else {
						if(headphone_smaster_se_gain_mode==HEADPHONE_SMASTER_SE_GAIN_MODE_NORMAL && board_type==TYPE_Z)
							table=MASTER_VOLUME_TABLE_SMASTER_SE_LG;
						else
							table=MASTER_VOLUME_TABLE_SMASTER_SE_HG; /* TYPE_A is fixed HG */
					}
				}
				else if(headphone_amp==HEADPHONE_AMP_SMASTER_BTL){
					if(headphone_smaster_btl_gain_mode==HEADPHONE_SMASTER_BTL_GAIN_MODE_HIGH)
						table=MASTER_VOLUME_TABLE_SMASTER_BTL_100;
					else
						table=MASTER_VOLUME_TABLE_SMASTER_BTL_50;
				}
				else {
					if(noise_cancel_active)
						table=MASTER_VOLUME_TABLE_CLASSAB_NC;
					else
						table=MASTER_VOLUME_TABLE_CLASSAB;
				}
				break;
			case OUTPUT_DEVICE_LINE:
				table=MASTER_VOLUME_TABLE_LINE;
				break;
			case OUTPUT_DEVICE_FIXEDLINE:
				table=MASTER_VOLUME_TABLE_FIXEDLINE;
				break;
			case OUTPUT_DEVICE_NONE:
			default:
				table=MASTER_VOLUME_TABLE_OFF;
				break;
		}
	} else {
		switch(output_device){
                        case OUTPUT_DEVICE_HEADPHONE:
                                if(headphone_amp==HEADPHONE_AMP_SMASTER_SE){
					if(headphone_smaster_se_gain_mode==HEADPHONE_SMASTER_SE_GAIN_MODE_NORMAL && board_type==TYPE_Z){
						if(rate==88200)
							table=MASTER_VOLUME_TABLE_SMASTER_SE_DSD64_LG;
						else if(rate==176400)
							table=MASTER_VOLUME_TABLE_SMASTER_SE_DSD128_LG;
						else
							table=MASTER_VOLUME_TABLE_SMASTER_SE_DSD256_LG;
					}else{ /* TYPE_A is fixed HG */
						if(rate==88200)
							table=MASTER_VOLUME_TABLE_SMASTER_SE_DSD64_HG;
						else if(rate==176400)
							table=MASTER_VOLUME_TABLE_SMASTER_SE_DSD128_HG;
						else
							table=MASTER_VOLUME_TABLE_SMASTER_SE_DSD256_HG;
					}
                                }
                                else if(headphone_amp==HEADPHONE_AMP_SMASTER_BTL){
                                        if(headphone_smaster_btl_gain_mode==HEADPHONE_SMASTER_BTL_GAIN_MODE_HIGH){
						if(rate==88200)
                                                	table=MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD64;
						else if(rate==176400)
							table=MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD128;
						else
							table=MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD256;
                                        }else{
                                                if(rate==88200)
                                                        table=MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD64;
                                                else if(rate==176400)
                                                        table=MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD128;
                                                else
                                                        table=MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD256;
					}
                                }
                                else {
					table=MASTER_VOLUME_TABLE_OFF;
                                }
                                break;
                        case OUTPUT_DEVICE_LINE:
				if(rate==88200)
					table=MASTER_VOLUME_TABLE_LINE_DSD64;
				else if(rate==176400)
					table=MASTER_VOLUME_TABLE_LINE_DSD128;
                                else
					table=MASTER_VOLUME_TABLE_LINE_DSD256;
                                break;
                        case OUTPUT_DEVICE_NONE:
                        default:
                                table=MASTER_VOLUME_TABLE_OFF;
                                break;
		}
	}
	return(table);
}
