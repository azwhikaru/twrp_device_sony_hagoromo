/*
* Copyright (C) 2011-2015 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __MT6627_FM_REG_H__
#define __MT6627_FM_REG_H__

enum MT6627_REG {
	FM_MAIN_CG1_CTRL = 0x60,
	FM_MAIN_CG2_CTRL = 0x61,
	FM_MAIN_HWVER = 0x62,
	FM_MAIN_CTRL = 0x63,
	FM_CHANNEL_SET = 0x65,
	FM_MAIN_CFG1 = 0x66,
	FM_MAIN_CFG2 = 0x67,
	FM_MAIN_MCLKDESENSE = 0x38,
	FM_MAIN_INTR = 0x69,
	FM_MAIN_INTRMASK = 0x6A,
	FM_MAIN_EXTINTRMASK = 0x6B,
	FM_RSSI_IND = 0x6C,
	FM_RSSI_TH = 0x6D,
	FM_MAIN_RESET = 0x6E,
	FM_MAIN_CHANDETSTAT = 0x6F,
	FM_RDS_CFG0 = 0x80,
	FM_RDS_INFO = 0x81,
	FM_RDS_DATA_REG = 0x82,
	FM_RDS_GOODBK_CNT = 0x83,
	FM_RDS_BADBK_CNT = 0x84,
	FM_RDS_PWDI = 0x85,
	FM_RDS_PWDQ = 0x86,
	FM_RDS_FIFO_STATUS0 = 0x87,
	FM_FT_CON9 = 0x8F,
	FM_DSP_PATCH_CTRL = 0x90,
	FM_DSP_PATCH_OFFSET = 0x91,
	FM_DSP_PATCH_DATA = 0x92,
	FM_DSP_MEM_CTRL4 = 0x93,
	FM_ADDR_PAMD = 0xB4,
	FM_RDS_BDGRP_ABD_CTRL_REG = 0xB6,
	FM_RDS_POINTER = 0xF0,
};

/* RDS_BDGRP_ABD_CTRL_REG */
enum {
	BDGRP_ABD_EN = 0x0001,
	BER_RUN = 0x2000
};
#define FM_DAC_CON1 0x83
#define FM_DAC_CON2 0x84
#define FM_FT_CON0 0x86
enum {
	FT_EN = 0x0001
};

#define FM_I2S_CON0 0x90
enum {
	I2S_EN = 0x0001,
	FORMAT = 0x0002,
	WLEN = 0x0004,
	I2S_SRC = 0x0008
};

/* FM_MAIN_CTRL */
enum {
	TUNE = 0x0001,
	SEEK = 0x0002,
	SCAN = 0x0004,
	CQI_READ = 0x0008,
	RDS_MASK = 0x0010,
	MUTE = 0x0020,
	RDS_BRST = 0x0040,
	RAMP_DOWN = 0x0100,
};

/* FM_MAIN_INTR */
enum {
	FM_INTR_STC_DONE = 0x0001,
	FM_INTR_IQCAL_DONE = 0x0002,
	FM_INTR_DESENSE_HIT = 0x0004,
	FM_INTR_CHNL_CHG = 0x0008,
	FM_INTR_SW_INTR = 0x0010,
	FM_INTR_RDS = 0x0020
};

enum {
	ANTENNA_TYPE = 0x0010,	/* 0x61 D4, 0:long,  1:short */
	ANALOG_I2S = 0x0080,	/* 0x61 D7, 0:lineout,  1:I2S */
	DE_EMPHASIS = 0x1000,	/* 0x61 D12,0:50us,  1:75 us */
};

#define OSC_FREQ_BITS 0x0070	/* 0x60 bit4~6 */
#define OSC_FREQ_MASK (~OSC_FREQ_BITS)

#endif				/* __MT6627_FM_REG_H__ */
