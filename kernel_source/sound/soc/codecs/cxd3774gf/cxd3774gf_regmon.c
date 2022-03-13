/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf_timer.c
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

/* #define TRACE_PRINT_ON */
/* #define DEBUG_PRINT_ON */
#define TRACE_TAG "------- "
#define DEBUG_TAG "        "

#include "cxd3774gf_common.h"

#ifdef CONFIG_REGMON_DEBUG

static int cxd3774gf_regmon_write_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int   value
);

static int cxd3774gf_regmon_read_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int * value
);

static regmon_reg_info_t cxd3774gf_regmon_reg_info[] =
{
	{ "DEVICE_ID",         0x01 },
	{ "REVISION_NO",       0x04 },
	{ "POWER_CONTROL_1",   0x05 },
	{ "POWER_CONTROL_2",   0x06 },
	{ "POWER_CONTROL_3",   0x07 },
	{ "MODE_CONTROL",      0x08 },
	{ "SRC",               0x09 },
	{ "SOFT_RAMP_1",       0x0A },
	{ "SOFT_RAMP_2",       0x0B },
	{ "LINEIN_1",          0x0C },
	{ "LINEIN_2",          0x0D },
	{ "LINEIN_3",          0x0E },
	{ "PGA_1L",            0x0F },
	{ "PGA_1R",            0x10 },
	{ "ADC_1L",            0x11 },
	{ "ADC_1R",            0x12 },
	{ "ALC_1",             0x13 },
	{ "ALC_2",             0x14 },
	{ "SPC",               0x15 },
	{ "MIC_BIAS",          0x16 },
	{ "MICIN_1",           0x17 },
	{ "MICIN_2",           0x18 },
	{ "MICIN_3",           0x19 },
	{ "PGA_2L",            0x1A },
	{ "PGA_2R",            0x1B },
	{ "ADC_2L",            0x1C },
	{ "ADC_2R",            0x1D },
	{ "CHARGE_PUMP",       0x1E },
	{ "DIG_IN",            0x1F },
	{ "SDIN_1_VOL",        0x20 },
	{ "SDIN_2_VOL",        0x21 },
	{ "LINEIN_VOL",        0x22 },
	{ "SDOUT_VOL",         0x23 },
	{ "CLEAR_STEREO",      0x2A },
	{ "BEEP_1",            0x2B },
	{ "BEEP_2",            0x2C },
	{ "DAC_VOL",           0x2D },
	{ "HPOUT_VOL",         0x32 },
	{ "LINEOUT_VOL",       0x33 },
	{ "INT_MASK_1",        0x36 },
	{ "INT_MASK_2",        0x37 },
	{ "INT_RD_1",          0x38 },
	{ "INT_RD_2",          0x39 },
	{ "SDO_CONTROL",       0x3A },
	{ "DNC_CONTROL_1",     0x3B },
	{ "DNC_CONTROL_2",     0x3C },
	{ "DNC_CONTROL_3",     0x3D },
	{ "DNC_VOL_0H",        0x3E },
	{ "DNC_VOL_0L",        0x3F },
	{ "DNC_VOL_1H",        0x40 },
	{ "DNC_VOL_1L",        0x41 },
	{ "DNC_MON_0H",        0x42 },
	{ "DNC_MON_0L",        0x43 },
	{ "DNC_MON_1H",        0x44 },
	{ "DNC_MON_1L",        0x45 },
	{ "DNC_ALGAIN_0H",     0x46 },
	{ "DNC_ALGAIN_0L",     0x47 },
	{ "DNC_ALGAIN_1H",     0x48 },
	{ "DNC_ALGAIN_1L",     0x49 },
	{ "DNC_ALGAIN_2H",     0x4A },
	{ "DNC_ALGAIN_2L",     0x4B },
	{ "DNC_ALGAIN_3H",     0x4C },
	{ "DNC_ALGAIN_3L",     0x4D },
	{ "DNC_LIMIT_A0",      0x4E },
	{ "DNC_LIMIT_R0",      0x4F },
	{ "DNC_LIMIT_Y0",      0x50 },
	{ "DNC_LIMIT_A1",      0x51 },
	{ "DNC_LIMIT_R1",      0x52 },
	{ "DNC_LIMIT_Y1",      0x53 },
	{ "DNC_STATUS_0H",     0x54 },
	{ "DNC_STATUS_0L",     0x55 },
	{ "DNC_STATUS_1H",     0x56 },
	{ "DNC_STATUS_1L",     0x57 },
	{ "DNC_STATUS_2H",     0x58 },
	{ "DNC_STATUS_2L",     0x59 },
	{ "ANC_SET",           0x5A },
	{ "ANC_FALVL_H",       0x5B },
	{ "ANC_FALVL_L",       0x5C },
	{ "ANC_FAWT",          0x5D },
	{ "ANC_SET_2",         0x5E },
	{ "ANC_ENV_0H",        0x5F },
	{ "ANC_ENV_0L",        0x60 },
	{ "ANC_ENV_1H",        0x61 },
	{ "ANC_ENV_1L",        0x62 },
	{ "ANC_ENV_2H",        0x63 },
	{ "ANC_ENV_2L",        0x64 },
	{ "S_MASTER_CONTROL",  0x65 },
	{ "S_MASTER_BEEP_1",   0x66 },
	{ "S_MASTER_BEEP_2",   0x67 },
	{ "S_MASTER_DITHER_1", 0x68 },
	{ "S_MASTER_DITHER_2", 0x69 },
	{ "S_MASTER_DITHER_3", 0x6A },
	{ "S_MASTER_DITHER_4", 0x6B },
	{ "S_MASTER_DITHER_5", 0x6C },
	{ "S_MASTER_DITHER_6", 0x6D },
	{ "S_MASTER_VCONT",    0x6E },
	{ "NSAD_SYS_NSAD",     0x6F },
	{ "RAM_CONTROL_1",     0x70 },
	{ "RAM_CONTROL_2",     0x71 },
	{ "DEQ_CONTROL",       0x7F },
	{ "TRIM_1",            0xD0 },
	{ "TRIM_2",            0xD1 },
	{ "DAC",               0xD2 },
	{ "CLASS_H_1",         0xD3 },
	{ "CLASS_H_2",         0xD4 },
	{ "TEST_1",            0xD5 },
	{ "TEST_2",            0xD6 },
	{ "TEST_3",            0xD7 },
	{ "TEST_4",            0xD8 },
	{ "IO_DRIVE",          0xD9 },
	{ "DITHER_MIC",        0xDA },
	{ "DITHER_LINE",       0xDB },
	{ "DSD_TEST_1",        0xDC },
	{ "DSD_TEST_2",        0xDD },
	{ "ADD_1",             0xDE },
	{ "ADD_2",             0xDF }
};

static regmon_customer_info_t cxd3774gf_customer_info =
{
	.name           = "cxd3774gf",
	.reg_info       = cxd3774gf_regmon_reg_info,
	.reg_info_count = sizeof(cxd3774gf_regmon_reg_info)/sizeof(regmon_reg_info_t),
	.write_reg      = cxd3774gf_regmon_write_reg,
	.read_reg       = cxd3774gf_regmon_read_reg,
	.private_data   = NULL,
};

#endif

static int initialized = FALSE;

int cxd3774gf_regmon_initialize(void)
{
	print_trace("%s()\n",__FUNCTION__);

#ifdef CONFIG_REGMON_DEBUG
	regmon_add(&cxd3774gf_customer_info);
#endif

	initialized=TRUE;

	return(0);
}

int cxd3774gf_regmon_finalize(void)
{
	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

#ifdef CONFIG_REGMON_DEBUG
	regmon_del(&cxd3774gf_customer_info);
#endif

	initialized=FALSE;

	return(0);
}

#ifdef CONFIG_REGMON_DEBUG

static int cxd3774gf_regmon_write_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int   value
)
{
	int rv;

	rv=cxd3774gf_register_write(address,value);

	return(rv);
}

static int cxd3774gf_regmon_read_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int * value
)
{
	int rv;

	rv=cxd3774gf_register_read(address,value);

	return(rv);
}

#endif
