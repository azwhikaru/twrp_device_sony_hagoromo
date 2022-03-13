/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf_table.h
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

#ifndef _CXD3774GF_TABLE_HEADER_
#define _CXD3774GF_TABLE_HEADER_

struct cxd3774gf_master_volume {
	unsigned char linein;
	unsigned char sdin1;
	unsigned char sdin2;
	unsigned char dac;
	unsigned char hpout;
	unsigned char lineout;
	unsigned char canary;
	unsigned char cxd90020;
	unsigned char cmx1;
	unsigned char cmx2;
};

struct cxd3774gf_device_gain {
	unsigned char pga;
	unsigned char adc;
};

struct cxd3774gf_deq_coefficient {
	unsigned char b0[3];
	unsigned char b1[3];
	unsigned char b2[3];
	unsigned char a1[3];
	unsigned char a2[3];
};

struct cxd3774gf_tone_control {
	struct cxd3774gf_deq_coefficient coefficient[5];
};

#define TABLE_ID_MASTER_VOLUME 0
#define TABLE_ID_DEVICE_GAIN   1
#define TABLE_ID_TONE_CONTROL  2

#define MASTER_VOLUME_TABLE_OFF          0
#define MASTER_VOLUME_TABLE_SMASTER      1
#define MASTER_VOLUME_TABLE_SMASTER_NC   2
#define MASTER_VOLUME_TABLE_CLASSH       3
#define MASTER_VOLUME_TABLE_CLASSH_NC    4
#define MASTER_VOLUME_TABLE_SPEAKER      5
#define MASTER_VOLUME_TABLE_LINE         6
#define MASTER_VOLUME_TABLE_FIXEDLINE    7
#define MASTER_VOLUME_TABLE_MAX          7

#define TONE_CONTROL_TABLE_NO_HP            0
#define TONE_CONTROL_TABLE_NAMP_GENERAL_HP  1
#define TONE_CONTROL_TABLE_NAMP_BUNDLE_NCHP 2
#define TONE_CONTROL_TABLE_NAMP_MODEL1_NCHP 3
#define TONE_CONTROL_TABLE_SAMP_GENERAL_HP  4
#define TONE_CONTROL_TABLE_SAMP_BUNDLE_NCHP 5
#define TONE_CONTROL_TABLE_SAMP_MODEL1_NCHP 6
#define TONE_CONTROL_TABLE_MAX              6

extern struct cxd3774gf_master_volume cxd3774gf_master_volume_table[2][MASTER_VOLUME_TABLE_MAX+1][MASTER_VOLUME_MAX+1];
extern unsigned char                  cxd3774gf_master_gain_table[MASTER_GAIN_MAX+1];
extern struct cxd3774gf_device_gain   cxd3774gf_device_gain_table[INPUT_DEVICE_MAX+1];
extern struct cxd3774gf_tone_control  cxd3774gf_tone_control_table[TONE_CONTROL_TABLE_MAX+1];

int cxd3774gf_table_initialize(struct mutex * mutex);
int cxd3774gf_table_finalize(void);

#endif

