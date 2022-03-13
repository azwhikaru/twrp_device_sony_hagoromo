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
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include "partition_define.h"


/*=======================================================================*/
/* NAND PARTITION Mapping                                                  */
/*=======================================================================*/
static struct mtd_partition g_pasStatic_Partition[] = {

	{
		.name = "preloader",
		.offset = 0x0,
		.size = PART_SIZE_PRELOADER,
		.mask_flags  = MTD_WRITEABLE,
	},
	{
		.name = "pro_info",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_PRO_INFO,
	},
	{
		.name = "nvram",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_NVRAM,
	},
	{
		.name = "protect_f",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_PROTECT_F,
	},
	{
		.name = "seccnfg",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_SECCFG,
	},
	{
		.name = "uboot",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_UBOOT,
		.mask_flags  = MTD_WRITEABLE,
	},
	{
		.name = "boot",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_BOOTIMG,
	},
	{
		.name = "recovery",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_RECOVERY,
	},
	{
		.name = "secstatic",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_SEC_RO,
		.mask_flags  = MTD_WRITEABLE,
	},
	{
		.name = "misc",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_MISC,
	},
	{
		.name = "logo",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_LOGO,
	},
	{
		.name = "expdb",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_EXPDB,
	},
	{
		.name = "tee1",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_TEE1,
	},
	{
		.name = "tee2",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_TEE2,
	},
	{
		.name = "kb",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_KB,
	},
	{
		.name = "dkb",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_DKB,
	},
	{
		.name = "chrome",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_CHROME,
	},
	{
		.name = "system",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_ANDROID,
	},
	{
		.name = "cache",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_CACHE,
	},
	{
		.name = "cm4",
		.offset = MTDPART_OFS_APPEND,
		.size = PART_SIZE_CM4,
	},
	{
		.name = "userdata",
		.offset = MTDPART_OFS_APPEND,
		.size = MTDPART_SIZ_FULL,
	},
};
#define NUM_PARTITIONS ARRAY_SIZE(g_pasStatic_Partition)
extern int part_num;	// = NUM_PARTITIONS;
