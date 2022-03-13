/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf_register.h
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

#ifndef _CXD3774GF_REGISTER_HEADER_
#define _CXD3774GF_REGISTER_HEADER_

#define CXD3774GF_DEVICE_ID         0x01
#define CXD3774GF_REVISION_NO       0x04
#define CXD3774GF_POWER_CONTROL_1   0x05
#define CXD3774GF_POWER_CONTROL_2   0x06
#define CXD3774GF_POWER_CONTROL_3   0x07
#define CXD3774GF_MODE_CONTROL      0x08
#define CXD3774GF_SRC               0x09
#define CXD3774GF_SOFT_RAMP_1       0x0A
#define CXD3774GF_SOFT_RAMP_2       0x0B
#define CXD3774GF_LINEIN_1          0x0C
#define CXD3774GF_LINEIN_2          0x0D
#define CXD3774GF_LINEIN_3          0x0E
#define CXD3774GF_PGA_1L            0x0F
#define CXD3774GF_PGA_1R            0x10
#define CXD3774GF_ADC_1L            0x11
#define CXD3774GF_ADC_1R            0x12
#define CXD3774GF_ALC_1             0x13
#define CXD3774GF_ALC_2             0x14
#define CXD3774GF_SPC               0x15
#define CXD3774GF_MIC_BIAS          0x16
#define CXD3774GF_MICIN_1           0x17
#define CXD3774GF_MICIN_2           0x18
#define CXD3774GF_MICIN_3           0x19
#define CXD3774GF_PGA_2L            0x1A
#define CXD3774GF_PGA_2R            0x1B
#define CXD3774GF_ADC_2L            0x1C
#define CXD3774GF_ADC_2R            0x1D
#define CXD3774GF_CHARGE_PUMP       0x1E
#define CXD3774GF_DIG_IN            0x1F
#define CXD3774GF_SDIN_1_VOL        0x20
#define CXD3774GF_SDIN_2_VOL        0x21
#define CXD3774GF_LINEIN_VOL        0x22
#define CXD3774GF_SDOUT_VOL         0x23
#define CXD3774GF_CLEAR_STEREO      0x2A
#define CXD3774GF_BEEP_1            0x2B
#define CXD3774GF_BEEP_2            0x2C
#define CXD3774GF_DAC_VOL           0x2D
#define CXD3774GF_HPOUT_VOL         0x32
#define CXD3774GF_LINEOUT_VOL       0x33
#define CXD3774GF_INT_MASK_1        0x36
#define CXD3774GF_INT_MASK_2        0x37
#define CXD3774GF_INT_RD_1          0x38
#define CXD3774GF_INT_RD_2          0x39
#define CXD3774GF_SDO_CONTROL       0x3A
#define CXD3774GF_DGT_CONTROL_1     0x3B
#define CXD3774GF_DGT_CONTROL_2     0x3C
#define CXD3774GF_DGT_CONTROL_3     0x3D
#define CXD3774GF_DGT_VOL_0H        0x3E
#define CXD3774GF_DGT_VOL_0L        0x3F
#define CXD3774GF_DGT_VOL_1H        0x40
#define CXD3774GF_DGT_VOL_1L        0x41
#define CXD3774GF_DGT_MON_0H        0x42
#define CXD3774GF_DGT_MON_0L        0x43
#define CXD3774GF_DGT_MON_1H        0x44
#define CXD3774GF_DGT_MON_1L        0x45
#define CXD3774GF_DGT_ALGAIN_0H     0x46
#define CXD3774GF_DGT_ALGAIN_0L     0x47
#define CXD3774GF_DGT_ALGAIN_1H     0x48
#define CXD3774GF_DGT_ALGAIN_1L     0x49
#define CXD3774GF_DGT_ALGAIN_2H     0x4A
#define CXD3774GF_DGT_ALGAIN_2L     0x4B
#define CXD3774GF_DGT_ALGAIN_3H     0x4C
#define CXD3774GF_DGT_ALGAIN_3L     0x4D
#define CXD3774GF_DGT_LIMIT_A0      0x4E
#define CXD3774GF_DGT_LIMIT_R0      0x4F
#define CXD3774GF_DGT_LIMIT_Y0      0x50
#define CXD3774GF_DGT_LIMIT_A1      0x51
#define CXD3774GF_DGT_LIMIT_R1      0x52
#define CXD3774GF_DGT_LIMIT_Y1      0x53
#define CXD3774GF_DGT_STATUS_0H     0x54
#define CXD3774GF_DGT_STATUS_0L     0x55
#define CXD3774GF_DGT_STATUS_1H     0x56
#define CXD3774GF_DGT_STATUS_1L     0x57
#define CXD3774GF_DGT_STATUS_2H     0x58
#define CXD3774GF_DGT_STATUS_2L     0x59
#define CXD3774GF_ALG_SET           0x5A
#define CXD3774GF_ALG_FALVL_H       0x5B
#define CXD3774GF_ALG_FALVL_L       0x5C
#define CXD3774GF_ALG_FAWT          0x5D
#define CXD3774GF_ALG_SET_2         0x5E
#define CXD3774GF_ALG_ENV_0H        0x5F
#define CXD3774GF_ALG_ENV_0L        0x60
#define CXD3774GF_ALG_ENV_1H        0x61
#define CXD3774GF_ALG_ENV_1L        0x62
#define CXD3774GF_ALG_ENV_2H        0x63
#define CXD3774GF_ALG_ENV_2L        0x64
#define CXD3774GF_S_MASTER_CONTROL  0x65
#define CXD3774GF_S_MASTER_BEEP_1   0x66
#define CXD3774GF_S_MASTER_BEEP_2   0x67
#define CXD3774GF_S_MASTER_DITHER_1 0x68
#define CXD3774GF_S_MASTER_DITHER_2 0x69
#define CXD3774GF_S_MASTER_DITHER_3 0x6A
#define CXD3774GF_S_MASTER_DITHER_4 0x6B
#define CXD3774GF_S_MASTER_DITHER_5 0x6C
#define CXD3774GF_S_MASTER_DITHER_6 0x6D
#define CXD3774GF_S_MASTER_VCONT    0x6E
#define CXD3774GF_NSAD_SYS_NSAD     0x6F
#define CXD3774GF_RAM_CONTROL_1     0x70
#define CXD3774GF_RAM_CONTROL_2     0x71
#define CXD3774GF_RAM_WRITE_BASE    0x72
#define CXD3774GF_RAM_READ_BASE     0x77
#define CXD3774GF_DEQ_CONTROL       0x7F
#define CXD3774GF_DEQ_1_COEF_BASE   0x80
#define CXD3774GF_DEQ_2_COEF_BASE   0x8F
#define CXD3774GF_DEQ_3_COEF_BASE   0x9E
#define CXD3774GF_DEQ_4_COEF_BASE   0xAD
#define CXD3774GF_DEQ_5_COEF_BASE   0xBC
#define CXD3774GF_TRIM_1            0xD0
#define CXD3774GF_TRIM_2            0xD1
#define CXD3774GF_DAC               0xD2
#define CXD3774GF_CLASS_H_1         0xD3
#define CXD3774GF_CLASS_H_2         0xD4
#define CXD3774GF_TEST_1            0xD5
#define CXD3774GF_TEST_2            0xD6
#define CXD3774GF_TEST_3            0xD7
#define CXD3774GF_TEST_4            0xD8
#define CXD3774GF_IO_DRIVE          0xD9
#define CXD3774GF_DITHER_MIC        0xDA
#define CXD3774GF_DITHER_LINE       0xDB
#define CXD3774GF_DSD_TEST_1        0xDC
#define CXD3774GF_DSD_TEST_2        0xDD
#define CXD3774GF_ADD_1             0xDE
#define CXD3774GF_ADD_2             0xDF

int cxd3774gf_register_initialize(struct i2c_client * client);
int cxd3774gf_register_finalize(void);

int cxd3774gf_register_write_multiple(
	unsigned int    address,
	unsigned char * value,
	int             size
);

int cxd3774gf_register_read_multiple(
	unsigned int    address,
	unsigned char * value,
	int             size
);

int cxd3774gf_register_modify(
	unsigned int address,
	unsigned int value,
	unsigned int mask
);

int cxd3774gf_register_write(
	unsigned int address,
	unsigned int value
);

int cxd3774gf_register_read(
	unsigned int   address,
	unsigned int * value
);

#endif
