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
#ifndef __PARTITION_DEFINE_H__
#define __PARTITION_DEFINE_H__




#define KB  (1024)
#define MB  (1024 * KB)
#define GB  (1024 * MB)

#define PART_PRELOADER "PRELOADER" 
#define PART_PRO_INFO "PRO_INFO" 
#define PART_NVRAM "NVRAM" 
#define PART_PROTECT_F "PROTECT_F" 
#define PART_SECCFG "SECCFG" 
#define PART_UBOOT "UBOOT" 
#define PART_BOOTIMG "BOOTIMG" 
#define PART_RECOVERY "RECOVERY" 
#define PART_SEC_RO "SEC_RO" 
#define PART_MISC "MISC" 
#define PART_LOGO "LOGO" 
#define PART_EXPDB "EXPDB" 
#define PART_TEE1 "TEE1" 
#define PART_TEE2 "TEE2" 
#define PART_KB "KB" 
#define PART_DKB "DKB" 
#define PART_CHROME "CHROME" 
#define PART_ANDROID "ANDROID" 
#define PART_CACHE "CACHE" 
#define PART_CM4 "CM4" 
#define PART_USRDATA "USRDATA" 
#define PART_BMTPOOL "BMTPOOL" 
/*preloader re-name*/
#define PART_SECURE "SECURE" 
#define PART_SECSTATIC "SECSTATIC" 
#define PART_ANDSYSIMG "ANDSYSIMG" 
#define PART_USER "USER" 
/*Uboot re-name*/
#define PART_APANIC "APANIC" 

#define PART_FLAG_NONE              0 
#define PART_FLAG_LEFT             0x1 
#define PART_FLAG_END              0x2 
#define PART_MAGIC              0x58881688 

#define PART_SIZE_PRELOADER			(1024*KB)
#define PART_OFFSET_PRELOADER			(0x0)
#define PART_SIZE_PRO_INFO			(1024*KB)
#define PART_OFFSET_PRO_INFO			(0x100000)
#define PART_SIZE_NVRAM			(1024*KB)
#define PART_OFFSET_NVRAM			(0x200000)
#define PART_SIZE_PROTECT_F			(1024*KB)
#define PART_OFFSET_PROTECT_F			(0x300000)
#define PART_SIZE_SECCFG			(3072*KB)
#define PART_OFFSET_SECCFG			(0x400000)
#define PART_SIZE_UBOOT			(2048*KB)
#define PART_OFFSET_UBOOT			(0x700000)
#define PART_SIZE_BOOTIMG			(8192*KB)
#define PART_OFFSET_BOOTIMG			(0x900000)
#define PART_SIZE_RECOVERY			(8192*KB)
#define PART_OFFSET_RECOVERY			(0x1100000)
#define PART_SIZE_SEC_RO			(1024*KB)
#define PART_OFFSET_SEC_RO			(0x1900000)
#define PART_SIZE_MISC			(1024*KB)
#define PART_OFFSET_MISC			(0x1a00000)
#define PART_SIZE_LOGO			(2048*KB)
#define PART_OFFSET_LOGO			(0x1b00000)
#define PART_SIZE_EXPDB			(2048*KB)
#define PART_OFFSET_EXPDB			(0x1d00000)
#define PART_SIZE_TEE1			(1024*KB)
#define PART_OFFSET_TEE1			(0x1f00000)
#define PART_SIZE_TEE2			(1024*KB)
#define PART_OFFSET_TEE2			(0x2000000)
#define PART_SIZE_KB			(2048*KB)
#define PART_OFFSET_KB			(0x2100000)
#define PART_SIZE_DKB			(2048*KB)
#define PART_OFFSET_DKB			(0x2300000)
#define PART_SIZE_CHROME			(131072*KB)
#define PART_OFFSET_CHROME			(0x2500000)
#define PART_SIZE_ANDROID			(143360*KB)
#define PART_OFFSET_ANDROID			(0xa500000)
#define PART_SIZE_CACHE			(8192*KB)
#define PART_OFFSET_CACHE			(0x13100000)
#define PART_SIZE_CM4			(1024*KB)
#define PART_OFFSET_CM4			(0x13900000)
#define PART_SIZE_USRDATA			(0*KB)
#define PART_OFFSET_USRDATA			(0x13a00000)
#define PART_SIZE_BMTPOOL			(0*KB)
#define PART_OFFSET_BMTPOOL			(0xFFFF0000)
#ifndef RAND_START_ADDR
#define RAND_START_ADDR   256
#endif


#define PART_NUM			22



#define PART_MAX_COUNT			 40

#define MBR_START_ADDRESS_BYTE			(*KB)

typedef enum  {
	EMMC = 1,
	NAND = 2,
} dev_type;

#if defined(MTK_EMMC_SUPPORT) || defined(CONFIG_MTK_EMMC_SUPPORT)
typedef enum {
	EMMC_PART_UNKNOWN=0
	,EMMC_PART_BOOT1
	,EMMC_PART_BOOT2
	,EMMC_PART_RPMB
	,EMMC_PART_GP1
	,EMMC_PART_GP2
	,EMMC_PART_GP3
	,EMMC_PART_GP4
	,EMMC_PART_USER
	,EMMC_PART_END
} Region;
#else
typedef enum {
NAND_PART_UNKNOWN=0
,NAND_PART_USER
} Region;
#endif

struct excel_info{
	char * name;
	unsigned long long size;
	unsigned long long start_address;
	dev_type type ;
	unsigned int partition_idx;
	Region region;
};

#if defined(MTK_EMMC_SUPPORT) || defined(CONFIG_MTK_EMMC_SUPPORT)
/*MBR or EBR struct*/
#define SLOT_PER_MBR 4
#define MBR_COUNT 8

struct MBR_EBR_struct{
	char part_name[8];
	int part_index[SLOT_PER_MBR];
};

extern struct MBR_EBR_struct MBR_EBR_px[MBR_COUNT];
#endif
extern struct excel_info *PartInfo;


#endif
