/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
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
#include "partition_define.h"
#if 1
static const struct excel_info PartInfo_Private[PART_NUM]={
			{"preloader",   262144,     0, EMMC, 0,EMMC_PART_BOOT1},
			{"mbr",         524288,     0x0, EMMC, 0,EMMC_PART_USER},
			{"ebr1",        524288,     0x80000, EMMC, 1,EMMC_PART_USER},
			{"pro_info",    3145728,    0x100000, EMMC, 0,EMMC_PART_USER},
			{"nvram",       5242880,    0x400000, EMMC, 0,EMMC_PART_USER},
			{"protect_f",   10485760,   0x900000, EMMC, 2,EMMC_PART_USER},
			{"protect_s",   10485760,   0x1300000, EMMC, 3,EMMC_PART_USER},
			{"seccfg",      131072,     0x1d00000, EMMC, 0,EMMC_PART_USER},
			{"uboot",       393216,     0x1d20000, EMMC, 0,EMMC_PART_USER},
			{"bootimg",     16777216,   0x1d80000, EMMC, 0,EMMC_PART_USER},
			{"recovery",    16777216,   0x2d80000, EMMC, 0,EMMC_PART_USER},
			{"sec_ro",      6291456,    0x3d80000, EMMC, 4,EMMC_PART_USER},
			{"misc",        524288,     0x4380000, EMMC, 0,EMMC_PART_USER},
			{"logo",        3145728,    0x4400000, EMMC, 0,EMMC_PART_USER},
			{"ebr2",        524288,     0x4700000, EMMC, 0,EMMC_PART_USER},
			{"custom",      57671680,   0x4780000, EMMC, 9,EMMC_PART_USER},
			{"expdb",       10485760,   0x7e80000, EMMC, 0,EMMC_PART_USER},
			{"tee1",        5242880,    0x8880000, EMMC, 0,EMMC_PART_USER},
			{"tee2",        5242880,    0x8d80000, EMMC, 0,EMMC_PART_USER},
			{"kb",          1048576,    0x9280000, EMMC, 0,EMMC_PART_USER},
			{"dkb",         1048576,    0x9380000, EMMC, 0,EMMC_PART_USER},
			{"android",     629145600,  0x9480000, EMMC, 5,EMMC_PART_USER},
			{"cache",       7340032,    0x2ec80000, EMMC, 6,EMMC_PART_USER},
			{"usrdata",     41943040,   0x2f380000, EMMC, 7,EMMC_PART_USER},
			{"chrome",      134217728,  0x31b80000, EMMC, 8,EMMC_PART_USER},
			{"otp",         45088768,   0x39b80000, EMMC, 0,EMMC_PART_USER},
			{"ebr3",        524288,     0x3c680000, EMMC, 0,EMMC_PART_USER},
			{"nvp",         15728640,   0x3c700000, EMMC, 11,EMMC_PART_USER},
			{"var",         115343360,  0x3d600000, EMMC, 12,EMMC_PART_USER},
			{"db",          545259520,  0x44400000, EMMC, 13,EMMC_PART_USER},
/*			{"contents",    6081740800, 0x64c00000, EMMC, 10,EMMC_PART_USER},*/
			{"fat",         6081740800, 0x64c00000, EMMC, 10,EMMC_PART_USER},
			{"bmtpool",     22020096,   0x1cf400000, EMMC, 0,EMMC_PART_USER},
 };
#else
static const struct excel_info PartInfo_Private[PART_NUM]={
			{"preloader",262144,0, EMMC, 0,EMMC_PART_BOOT1},
			{"mbr",524288,0x0, EMMC, 0,EMMC_PART_USER},
			{"ebr1",524288,0x80000, EMMC, 1,EMMC_PART_USER},
			{"pro_info",3145728,0x100000, EMMC, 0,EMMC_PART_USER},
			{"nvram",5242880,0x400000, EMMC, 0,EMMC_PART_USER},
			{"protect_f",10485760,0x900000, EMMC, 2,EMMC_PART_USER},
			{"protect_s",10485760,0x1300000, EMMC, 3,EMMC_PART_USER},
			{"seccfg",131072,0x1d00000, EMMC, 0,EMMC_PART_USER},
			{"uboot",393216,0x1d20000, EMMC, 0,EMMC_PART_USER},
			{"bootimg",16777216,0x1d80000, EMMC, 0,EMMC_PART_USER},
			{"recovery",16777216,0x2d80000, EMMC, 0,EMMC_PART_USER},
			{"sec_ro",6291456,0x3d80000, EMMC, 4,EMMC_PART_USER},
			{"misc",524288,0x4380000, EMMC, 0,EMMC_PART_USER},
			{"logo",3145728,0x4400000, EMMC, 0,EMMC_PART_USER},
			{"ebr2",524288,0x4700000, EMMC, 0,EMMC_PART_USER},
			{"expdb",10485760,0x4780000, EMMC, 0,EMMC_PART_USER},
			{"tee1",5242880,0x5180000, EMMC, 0,EMMC_PART_USER},
			{"tee2",5242880,0x5680000, EMMC, 0,EMMC_PART_USER},
			{"kb",1048576,0x5b80000, EMMC, 0,EMMC_PART_USER},
			{"dkb",1048576,0x5c80000, EMMC, 0,EMMC_PART_USER},
			{"android",1073741824,0x5d80000, EMMC, 5,EMMC_PART_USER},
			{"cache",132120576,0x45d80000, EMMC, 6,EMMC_PART_USER},
			{"usrdata",2147483648,0x4db80000, EMMC, 7,EMMC_PART_USER},
			{"fat",0,0xcdb80000, EMMC, 8,EMMC_PART_USER},
			{"bmtpool",22020096,0xFFFF00a8, EMMC, 0,EMMC_PART_USER},
 };
#endif

#ifdef  CONFIG_MTK_EMMC_SUPPORT
struct MBR_EBR_struct MBR_EBR_px[MBR_COUNT]={
	{"mbr", {1, 2, 3, 4, }},
	{"ebr1", {5, 6, 7, }},
	{"ebr2", {8, 9, 10, }},
	{"ebr3", {11, 12, 13, }},
};

EXPORT_SYMBOL(MBR_EBR_px);
#endif

