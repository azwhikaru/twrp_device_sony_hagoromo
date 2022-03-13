/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_control.h
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

#ifndef _CXD3776ER_CONTROL_HEADER_
#define _CXD3776ER_CONTROL_HEADER_

int cxd3776er_core_initialize(struct mutex * mutex);
int cxd3776er_core_finalize(void);

int cxd3776er_suspend(void);
int cxd3776er_resume(void);

int cxd3776er_startup(int icx_playback, int std_playback, int capture);
int cxd3776er_shutdown(int icx_playback, int std_playback, int capture);

int cxd3776er_set_icx_playback_dai_rate(unsigned int rate);
int cxd3776er_set_std_playback_dai_rate(unsigned int rate);
int cxd3776er_set_capture_dai_rate(unsigned int rate);



int cxd3776er_put_motion_feedback_mode(int value);
int cxd3776er_get_motion_feedback_mode(int * value);

int cxd3776er_get_motion_feedback_status(int * value);

int cxd3776er_put_user_motion_feedback_gain(int index);
int cxd3776er_get_user_motion_feedback_gain(int * index);

int cxd3776er_put_base_motion_feedback_gain(int left, int right);
int cxd3776er_get_base_motion_feedback_gain(int * left, int * right);

int cxd3776er_exit_base_motion_feedback_gain_adjustment(int save);



int cxd3776er_put_sound_effect(int value);
int cxd3776er_get_sound_effect(int * value);



int cxd3776er_put_master_volume(int value);
int cxd3776er_get_master_volume(int * value);

int cxd3776er_put_master_gain(int value);
int cxd3776er_get_master_gain(int * value);

int cxd3776er_put_playback_mute(int value);
int cxd3776er_get_playback_mute(int * value);

int cxd3776er_put_capture_mute(int value);
int cxd3776er_get_capture_mute(int * value);

int cxd3776er_put_analog_playback_mute(int value);
int cxd3776er_get_analog_playback_mute(int * value);

int cxd3776er_put_analog_stream_mute(int value);
int cxd3776er_get_analog_stream_mute(int * value);

int cxd3776er_put_timed_mute(int value);
int cxd3776er_get_timed_mute(int * value);

int cxd3776er_put_std_timed_mute(int value);
int cxd3776er_get_std_timed_mute(int * value);

int cxd3776er_put_icx_timed_mute(int value);
int cxd3776er_get_icx_timed_mute(int * value);

int cxd3776er_put_input_device(int value);
int cxd3776er_get_input_device(int * value);

int cxd3776er_put_output_path(int value);
int cxd3776er_get_output_path(int * value);

int cxd3776er_put_output_select(int value);
int cxd3776er_get_output_select(int * value);

int cxd3776er_put_sdin_mix(int value);
int cxd3776er_get_sdin_mix(int * value);

int cxd3776er_put_tpm_mode(int value);
int cxd3776er_get_tpm_mode(int * value);

int cxd3776er_get_tpm_status(int * value);



int cxd3776er_register_dmfb_module(struct cxd3776er_dmfb_interface * interface);
int cxd3776er_register_tpm_module(struct cxd3776er_tpm_interface * interface);
int cxd3776er_check_tpm_status(int force);
int cxd3776er_handle_pcm_event(void);
int cxd3776er_apply_table_change(int id);



int cxd3776er_put_debug_test(int value);
int cxd3776er_get_debug_test(int * value);

#endif
