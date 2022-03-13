/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_register.h
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

#ifndef _CXD3776ER_REGISTER_HEADER_
#define _CXD3776ER_REGISTER_HEADER_

#define CXD3776ER_DEVICE_ID         0x00
#define CXD3776ER_REVISION_NO       0x01
#define CXD3776ER_MAIN_CLOCK        0x02
#define CXD3776ER_POWER_CONTROL_1   0x03
#define CXD3776ER_POWER_CONTROL_2   0x04
#define CXD3776ER_SOFT_RAMP_1       0x05
#define CXD3776ER_VOL1L00           0x10
#define CXD3776ER_VOL1L01           0x11
#define CXD3776ER_VOL1L02           0x12
#define CXD3776ER_VOL1L03           0x13
#define CXD3776ER_VOL1L06           0x14
#define CXD3776ER_VOL1L07           0x15
#define CXD3776ER_VOL1L07EM         0x16
#define CXD3776ER_VOL1R00           0x17
#define CXD3776ER_VOL1R01           0x18
#define CXD3776ER_VOL1R02           0x19
#define CXD3776ER_VOL1R03           0x1A
#define CXD3776ER_VOL1R06           0x1B
#define CXD3776ER_VOL1R07           0x1C
#define CXD3776ER_VOL1R07EM         0x1D
#define CXD3776ER_VOL2L00           0x1E
#define CXD3776ER_VOL2L01           0x1F
#define CXD3776ER_VOL2L02           0x20
#define CXD3776ER_VOL2L03           0x21
#define CXD3776ER_VOL2L04           0x22
#define CXD3776ER_VOL2L05           0x23
#define CXD3776ER_VOL2L06           0x24
#define CXD3776ER_VOL2L07           0x25
#define CXD3776ER_VOL2L07EM         0x26
#define CXD3776ER_VOL2R00           0x27
#define CXD3776ER_VOL2R01           0x28
#define CXD3776ER_VOL2R02           0x29
#define CXD3776ER_VOL2R03           0x2A
#define CXD3776ER_VOL2R04           0x2B
#define CXD3776ER_VOL2R05           0x2C
#define CXD3776ER_VOL2R06           0x2D
#define CXD3776ER_VOL2R07           0x2E
#define CXD3776ER_VOL2R07EM         0x2F
#define CXD3776ER_CODEC             0x30
#define CXD3776ER_SD1               0x35
#define CXD3776ER_SD2               0x36
#define CXD3776ER_DACOUT            0x37
#define CXD3776ER_TRIM_1            0x40
#define CXD3776ER_TRIM_2            0x41
#define CXD3776ER_PWMBUF            0x44
#define CXD3776ER_TEST              0x45
#define CXD3776ER_MISC              0x60
#define CXD3776ER_RAM_CONTROL_1     0x70
#define CXD3776ER_RAM_CONTROL_2     0x71
#define CXD3776ER_RAM_WRITE_BASE    0x72
#define CXD3776ER_RAM_READ_BASE     0x77
#define CXD3776ER_MFB_CONTROL_1     0x80
#define CXD3776ER_MFB_CONTROL_2     0x81
#define CXD3776ER_MFB_CONTROL_3     0x82
#define CXD3776ER_MFB_CANVOL1       0x83
#define CXD3776ER_MFB_CANVOL2       0x84
#define CXD3776ER_MFB_CANVOL3       0x85
#define CXD3776ER_MFB_CANVOL4       0x86
#define CXD3776ER_TPM_CONTROL       0x92
#define CXD3776ER_MFTP_MEMIF0_1     0x93
#define CXD3776ER_MFTP_MEMIF0_2     0x94
#define CXD3776ER_MFTP_MEMIF0_3     0x95
#define CXD3776ER_SMASTER_NS1_A     0xA0
#define CXD3776ER_SMASTER_NS1_B     0xA1
#define CXD3776ER_SMASTER_PWM_A     0xAC
#define CXD3776ER_SMASTER_PWM_B     0xAD

int cxd3776er_register_initialize(struct i2c_client * client);
int cxd3776er_register_finalize(void);

int cxd3776er_register_write_multiple(
	unsigned int    address,
	unsigned char * value,
	int             size
);

int cxd3776er_register_read_multiple(
	unsigned int    address,
	unsigned char * value,
	int             size
);

int cxd3776er_register_modify(
	unsigned int address,
	unsigned int value,
	unsigned int mask
);

int cxd3776er_register_write(
	unsigned int address,
	unsigned int value
);

int cxd3776er_register_read(
	unsigned int   address,
	unsigned int * value
);

#endif
