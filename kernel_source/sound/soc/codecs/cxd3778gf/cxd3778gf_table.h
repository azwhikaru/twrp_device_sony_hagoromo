/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3778gf_table.h
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

#ifndef _CXD3778GF_TABLE_HEADER_
#define _CXD3778GF_TABLE_HEADER_

struct cxd3778gf_master_volume {
	unsigned char linein;
	unsigned char sdin1;
	unsigned char sdin2;
	unsigned char play;
	unsigned char lineout;
	unsigned char hpout;
	unsigned char cmx1_750;
	unsigned char cmx2_750;
	unsigned char cmx1_31;
	unsigned char cmx2_31;
	unsigned char hpout2_ctrl3;
};

struct cxd3778gf_device_gain {
	unsigned char pga;
	unsigned char adc;
};

struct cxd3778gf_deq_coefficient {
	unsigned char b0[3];
	unsigned char b1[3];
	unsigned char b2[3];
	unsigned char a1[3];
	unsigned char a2[3];
};

#define TABLE_ID_MASTER_VOLUME 0
#define TABLE_ID_DEVICE_GAIN   1
#define TABLE_ID_TONE_CONTROL  2

#define MASTER_VOLUME_TABLE_OFF                    0
#define MASTER_VOLUME_TABLE_SMASTER_SE_LG          1
#define MASTER_VOLUME_TABLE_SMASTER_SE_HG          2
#define MASTER_VOLUME_TABLE_SMASTER_SE_NC          3
#define MASTER_VOLUME_TABLE_SMASTER_BTL_50         4
#define MASTER_VOLUME_TABLE_SMASTER_BTL_100        5
#define MASTER_VOLUME_TABLE_CLASSAB                6
#define MASTER_VOLUME_TABLE_CLASSAB_NC             7
#define MASTER_VOLUME_TABLE_LINE                   8
#define MASTER_VOLUME_TABLE_FIXEDLINE              9
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD64_LG    10
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD64_HG    11
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD128_LG   12
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD128_HG   13
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD256_LG   14
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD256_HG   15
#define MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD64   16
#define MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD64  17
#define MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD128  18
#define MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD128 19
#define MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD256  20
#define MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD256 21
#define MASTER_VOLUME_TABLE_LINE_DSD64             22
#define MASTER_VOLUME_TABLE_LINE_DSD128            23
#define MASTER_VOLUME_TABLE_LINE_DSD256            24
#define MASTER_VOLUME_TABLE_MAX                    24

#define TONE_CONTROL_TABLE_NO_HP            0
#define TONE_CONTROL_TABLE_NAMP_GENERAL_HP  1
#define TONE_CONTROL_TABLE_NAMP_BUNDLE_NCHP 2
#define TONE_CONTROL_TABLE_NAMP_MODEL1_NCHP 3
#define TONE_CONTROL_TABLE_SAMP_GENERAL_HP  4
#define TONE_CONTROL_TABLE_SAMP_BUNDLE_NCHP 5
#define TONE_CONTROL_TABLE_SAMP_MODEL1_NCHP 6
#define TONE_CONTROL_TABLE_MAX              6

extern struct cxd3778gf_master_volume cxd3778gf_master_volume_table[2][MASTER_VOLUME_TABLE_MAX+1][MASTER_VOLUME_MAX+1];
extern unsigned char                  cxd3778gf_master_gain_table[MASTER_GAIN_MAX+1];
extern struct cxd3778gf_device_gain   cxd3778gf_device_gain_table[INPUT_DEVICE_MAX+1];
extern unsigned char cxd3778gf_tone_control_table[(TONE_CONTROL_TABLE_MAX+1)][CODEC_RAM_SIZE];
extern unsigned int cxd3778gf_master_volume_dsd_table[MASTER_VOLUME_TABLE_MAX+1][MASTER_VOLUME_MAX+1];

int cxd3778gf_table_initialize(struct mutex * mutex);
int cxd3778gf_table_finalize(void);

#endif

