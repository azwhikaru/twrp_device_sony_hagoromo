/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_timer.c
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

/* #define TRACE_PRINT_ON */
/* #define DEBUG_PRINT_ON */
#define TRACE_TAG "------- "
#define DEBUG_TAG "        "

#include "cxd3776er_common.h"

#ifdef CONFIG_REGMON_DEBUG

static int cxd3776er_regmon_write_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int   value
);

static int cxd3776er_regmon_read_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int * value
);

static regmon_reg_info_t cxd3776er_regmon_reg_info[] =
{
	{ "DEVICE_ID",         0x00 },
	{ "REVISION_NO",       0x01 },
	{ "MAIN_CLOCK",        0x02 },
	{ "POWER_CONTROL_1",   0x03 },
	{ "POWER_CONTROL_2",   0x04 },
	{ "SOFT_RAMP_1",       0x05 },
	{ "VOL1L00",           0x10 },
	{ "VOL1L01",           0x11 },
	{ "VOL1L02",           0x12 },
	{ "VOL1L03",           0x13 },
	{ "VOL1L06",           0x14 },
	{ "VOL1L07",           0x15 },
	{ "VOL1L07EM",         0x16 },
	{ "VOL1R00",           0x17 },
	{ "VOL1R01",           0x18 },
	{ "VOL1R02",           0x19 },
	{ "VOL1R03",           0x1A },
	{ "VOL1R06",           0x1B },
	{ "VOL1R07",           0x1C },
	{ "VOL1R07EM",         0x1D },
	{ "VOL2L00",           0x1E },
	{ "VOL2L01",           0x1F },
	{ "VOL2L02",           0x20 },
	{ "VOL2L03",           0x21 },
	{ "VOL2L04",           0x22 },
	{ "VOL2L05",           0x23 },
	{ "VOL2L06",           0x24 },
	{ "VOL2L07",           0x25 },
	{ "VOL2L07EM",         0x26 },
	{ "VOL2R00",           0x27 },
	{ "VOL2R01",           0x28 },
	{ "VOL2R02",           0x29 },
	{ "VOL2R03",           0x2A },
	{ "VOL2R04",           0x2B },
	{ "VOL2R05",           0x2C },
	{ "VOL2R06",           0x2D },
	{ "VOL2R07",           0x2E },
	{ "VOL2R07EM",         0x2F },
	{ "CODEC",             0x30 },
	{ "SD1",               0x35 },
	{ "SD2",               0x36 },
	{ "DACOUT",            0x37 },
	{ "TRIM_1",            0x40 },
	{ "TRIM_2",            0x41 },
	{ "PWMBUF",            0x44 },
	{ "TEST",              0x45 },
	{ "MISC",              0x60 },
	{ "RAM_CONTROL_1",     0x70 },
	{ "RAM_CONTROL_2",     0x71 },
	{ "RAM_WRITE_BASE",    0x72 },
	{ "RAM_READ_BASE",     0x77 },
	{ "MFB_CONTROL_1",     0x80 },
	{ "MFB_CONTROL_2",     0x81 },
	{ "MFB_CONTROL_3",     0x82 },
	{ "MFB_CANVOL1",       0x83 },
	{ "MFB_CANVOL2",       0x84 },
	{ "MFB_CANVOL3",       0x85 },
	{ "MFB_CANVOL4",       0x86 },
	{ "TPM_CONTROL",       0x92 },
	{ "MFTP_MEMIF0_1",     0x93 },
	{ "MFTP_MEMIF0_2",     0x94 },
	{ "MFTP_MEMIF0_3",     0x95 },
	{ "SMASTER_NS1_A",     0xA0 },
	{ "SMASTER_NS1_B",     0xA1 },
	{ "SMASTER_PWM_A",     0xAC },
	{ "SMASTER_PWM_B",     0xAD }
};

static regmon_customer_info_t cxd3776er_customer_info =
{
	.name           = "cxd3776er",
	.reg_info       = cxd3776er_regmon_reg_info,
	.reg_info_count = sizeof(cxd3776er_regmon_reg_info)/sizeof(regmon_reg_info_t),
	.write_reg      = cxd3776er_regmon_write_reg,
	.read_reg       = cxd3776er_regmon_read_reg,
	.private_data   = NULL,
};

#endif

static int initialized = FALSE;

int cxd3776er_regmon_initialize(void)
{
	print_trace("%s()\n",__FUNCTION__);

#ifdef CONFIG_REGMON_DEBUG
	regmon_add(&cxd3776er_customer_info);
#endif

	initialized=TRUE;

	return(0);
}

int cxd3776er_regmon_finalize(void)
{
	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

#ifdef CONFIG_REGMON_DEBUG
	regmon_del(&cxd3776er_customer_info);
#endif

	initialized=FALSE;

	return(0);
}

#ifdef CONFIG_REGMON_DEBUG

static int cxd3776er_regmon_write_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int   value
)
{
	int rv;

	rv=cxd3776er_register_write(address,value);

	return(rv);
}

static int cxd3776er_regmon_read_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int * value
)
{
	int rv;

	rv=cxd3776er_register_read(address,value);

	return(rv);
}

#endif
