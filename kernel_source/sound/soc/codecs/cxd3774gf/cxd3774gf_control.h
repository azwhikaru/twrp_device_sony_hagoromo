/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf_control.h
 *
 * CXD3774GF CODEC driver
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

#ifndef _CXD3774GF_CONTROL_HEADER_
#define _CXD3774GF_CONTROL_HEADER_

int cxd3774gf_core_initialize(struct mutex * mutex);
int cxd3774gf_core_finalize(void);

int cxd3774gf_suspend(void);
int cxd3774gf_resume(void);

int cxd3774gf_startup(int icx_playback, int std_playback, int capture);
int cxd3774gf_shutdown(int icx_playback, int std_playback, int capture);

int cxd3774gf_set_icx_playback_dai_rate(unsigned int rate);
int cxd3774gf_set_std_playback_dai_rate(unsigned int rate);
int cxd3774gf_set_capture_dai_rate(unsigned int rate);



int cxd3774gf_put_noise_cancel_mode(int value);
int cxd3774gf_get_noise_cancel_mode(int * value);

int cxd3774gf_get_noise_cancel_status(int * value);

int cxd3774gf_put_user_noise_cancel_gain(int index);
int cxd3774gf_get_user_noise_cancel_gain(int * index);

int cxd3774gf_put_base_noise_cancel_gain(int left, int right);
int cxd3774gf_get_base_noise_cancel_gain(int * left, int * right);

int cxd3774gf_exit_base_noise_cancel_gain_adjustment(int save);



int cxd3774gf_put_sound_effect(int value);
int cxd3774gf_get_sound_effect(int * value);



int cxd3774gf_put_master_volume(int value);
int cxd3774gf_get_master_volume(int * value);

int cxd3774gf_put_master_gain(int value);
int cxd3774gf_get_master_gain(int * value);

int cxd3774gf_put_playback_mute(int value);
int cxd3774gf_get_playback_mute(int * value);

int cxd3774gf_put_capture_mute(int value);
int cxd3774gf_get_capture_mute(int * value);

int cxd3774gf_put_analog_playback_mute(int value);
int cxd3774gf_get_analog_playback_mute(int * value);

int cxd3774gf_put_analog_stream_mute(int value);
int cxd3774gf_get_analog_stream_mute(int * value);

int cxd3774gf_put_timed_mute(int value);
int cxd3774gf_get_timed_mute(int * value);

int cxd3774gf_put_std_timed_mute(int value);
int cxd3774gf_get_std_timed_mute(int * value);

int cxd3774gf_put_icx_timed_mute(int value);
int cxd3774gf_get_icx_timed_mute(int * value);

int cxd3774gf_put_input_device(int value);
int cxd3774gf_get_input_device(int * value);

int cxd3774gf_put_output_device(int value);
int cxd3774gf_get_output_device(int * value);

int cxd3774gf_put_headphone_amp(int value);
int cxd3774gf_get_headphone_amp(int * value);

int cxd3774gf_put_headphone_type(int value);
int cxd3774gf_get_headphone_type(int * value);

int cxd3774gf_put_jack_mode(int value);
int cxd3774gf_get_jack_mode(int * value);

int cxd3774gf_get_jack_status(int * value);



int cxd3774gf_register_dnc_module(struct cxd3774gf_dnc_interface * interface);
int cxd3774gf_check_jack_status(int force);
int cxd3774gf_handle_pcm_event(void);
int cxd3774gf_apply_table_change(int id);



int cxd3774gf_put_debug_test(int value);
int cxd3774gf_get_debug_test(int * value);

#endif
