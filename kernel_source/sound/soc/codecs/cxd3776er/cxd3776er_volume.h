/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_volume.h
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

#ifndef _CXD3776ER_VOLUME_HEADER_
#define _CXD3776ER_VOLUME_HEADER_

int cxd3776er_volume_initialize(void);
int cxd3776er_volume_finalize(void);

/* msater volume */
int cxd3776er_set_master_volume(struct cxd3776er_status * now, int volume);

/* master gain */
int cxd3776er_set_master_gain(struct cxd3776er_status * now, int gain);

/* for ??? */
int cxd3776er_set_playback_mute(struct cxd3776er_status * now, int mute);

/* mic mute */
int cxd3776er_set_capture_mute(struct cxd3776er_status * now, int mute);

/* for application */
int cxd3776er_set_analog_playback_mute(struct cxd3776er_status * now, int mute);

/* for audio policy manager */
int cxd3776er_set_analog_stream_mute(struct cxd3776er_status * now, int mute);

/* for device switch */
int cxd3776er_set_mix_timed_mute(struct cxd3776er_status * now, int mute);

/* for STD sound effect */
int cxd3776er_set_std_timed_mute(struct cxd3776er_status * now, int mute);

/* for STD sound effect */
int cxd3776er_set_icx_timed_mute(struct cxd3776er_status * now, int mute);

/* for no pcm */
int cxd3776er_set_no_pcm_mute(int mute);

/* for uncg */
int cxd3776er_set_uncg_mute(struct cxd3776er_status * now, int mute);

/* for driver */
int cxd3776er_set_output_path_mute(struct cxd3776er_status * now, int mute, int hw);

/* for driver */
int cxd3776er_set_input_device_mute(struct cxd3776er_status * now, int mute);

#endif

