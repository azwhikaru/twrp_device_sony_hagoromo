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
static const struct excel_info PartInfo_Private[PART_NUM]={			{"preloader",1048576,0x0, NAND},
			{"pro_info",1048576,0x100000, NAND},
			{"nvram",1048576,0x200000, NAND},
			{"protect_f",1048576,0x300000, NAND},
			{"seccfg",3145728,0x400000, NAND},
			{"uboot",2097152,0x700000, NAND},
			{"bootimg",8388608,0x900000, NAND},
			{"recovery",8388608,0x1100000, NAND},
			{"sec_ro",1048576,0x1900000, NAND},
			{"misc",1048576,0x1a00000, NAND},
			{"logo",2097152,0x1b00000, NAND},
			{"expdb",2097152,0x1d00000, NAND},
			{"tee1",1048576,0x1f00000, NAND},
			{"tee2",1048576,0x2000000, NAND},
			{"kb",2097152,0x2100000, NAND},
			{"dkb",2097152,0x2300000, NAND},
			{"chrome",134217728,0x2500000, NAND},
			{"android",146800640,0xa500000, NAND},
			{"cache",8388608,0x13100000, NAND},
			{"cm4",1048576,0x13900000, NAND},
			{"usrdata",0,0x13a00000, NAND},
			{"bmtpool",20971520,0xFFFF0000, NAND},
 };

#ifdef  CONFIG_MTK_EMMC_SUPPORT
struct MBR_EBR_struct MBR_EBR_px[MBR_COUNT]={
	{"mbr", {}},
};

EXPORT_SYMBOL(MBR_EBR_px);
#endif

