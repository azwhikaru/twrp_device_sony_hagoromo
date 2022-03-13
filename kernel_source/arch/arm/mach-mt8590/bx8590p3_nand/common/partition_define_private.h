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
#ifdef  CONFIG_MTK_EMMC_SUPPORT

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

struct MBR_EBR_struct MBR_EBR_px[MBR_COUNT]={
	{"mbr", {1, 2, 3, 4, }},
	{"ebr1", {5, 6, 7, }},
	{"ebr2", {8, }},
};
EXPORT_SYMBOL(MBR_EBR_px);


#else
static const struct excel_info PartInfo_Private[PART_NUM]={			
			{"preloader",262144,0, NAND, 0,NAND_PART_USER},
			{"mbr",524288,0x0, NAND, 0,NAND_PART_USER},
			{"ebr1",524288,0x80000, NAND, 1,NAND_PART_USER},
			{"pro_info",3145728,0x100000, NAND, 0,NAND_PART_USER},
			{"nvram",5242880,0x400000, NAND, 0,NAND_PART_USER},
			{"protect_f",10485760,0x900000, NAND, 2,NAND_PART_USER},
			{"protect_s",10485760,0x1300000, NAND, 3,NAND_PART_USER},
			{"seccfg",131072,0x1d00000, NAND, 0,NAND_PART_USER},
			{"uboot",393216,0x1d20000, NAND, 0,NAND_PART_USER},
			{"bootimg",16777216,0x1d80000, NAND, 0,NAND_PART_USER},
			{"recovery",16777216,0x2d80000, NAND, 0,NAND_PART_USER},
			{"sec_ro",6291456,0x3d80000, NAND, 4,NAND_PART_USER},
			{"misc",524288,0x4380000, NAND, 0,NAND_PART_USER},
			{"logo",3145728,0x4400000, NAND, 0,NAND_PART_USER},
			{"ebr2",524288,0x4700000, NAND, 0,NAND_PART_USER},
			{"expdb",10485760,0x4780000, NAND, 0,NAND_PART_USER},
			{"tee1",5242880,0x5180000, NAND, 0,NAND_PART_USER},
			{"tee2",5242880,0x5680000, NAND, 0,NAND_PART_USER},
			{"kb",1048576,0x5b80000, NAND, 0,NAND_PART_USER},
			{"dkb",1048576,0x5c80000, NAND, 0,NAND_PART_USER},
			{"android",1073741824,0x5d80000, NAND, 5,NAND_PART_USER},
			{"cache",132120576,0x45d80000, NAND, 6,NAND_PART_USER},
			{"usrdata",2147483648,0x4db80000, NAND, 7,NAND_PART_USER},
			{"fat",0,0xcdb80000, NAND, 8,NAND_PART_USER},
			{"bmtpool",22020096,0xFFFF00a8, NAND, 0,NAND_PART_USER},
 };



#endif

