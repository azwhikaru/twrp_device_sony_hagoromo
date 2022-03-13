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
#ifndef _VDOIN_FMT_HW_H_
#define _VDOIN_FMT_HW_H_

#define VIDEO_IN_BASE      0xFC007000

#define bReadRegVIN(dAddr)             (*((volatile unsigned int *)(VIDEO_IN_BASE + dAddr)))
#define vWriteRegVIN(dAddr, dVal)      (*((volatile unsigned int *)(VIDEO_IN_BASE + dAddr)) = (dVal))
#define vWriteVINMsk(dAddr, dVal, dMsk)  vWriteRegVIN((dAddr), (bReadRegVIN(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))


#define RW_VDI_ENA         0x00
	#define VI_EN		       (0x01<<0)
	#define VIDEO_FMT		   (0x01<<2)
	#define VIN_PROGRESSIVE    (0x01<<3)   
	#define SRAM_EN_SEL 	   (0x01<<4)
	#define SD_2FS_INPUT	   (0x01<<11)
	#define FLD_INV_EN	       (0x01<<12)
	#define LINEEAR_ENA	       (0x01<<14)
    #define VIN_SWAP 	       (0x03<<23)
	#define VIN_MODE_422	   (0x01<<25) 
	#define FIELD0_DIS	       (0x01<<27) 
	#define FIELD1_DIS	       (0x01<<28) 

#define RW_VDI_MODE         0x04
	#define VIN_FIFO_THRES	   (0x07<<4)
	#define VIN_FIFO_THRES_0	   (0x01<<4)
	#define VIN_FIFO_THRES_1	   (0x01<<5)
	#define VIN_FIFO_THRES_2	   (0x01<<6)

#define RW_VDI_YADDR0       0x08
#define RW_VDI_ACTLINE      0x0C
#define RW_VDI_CADDR0       0x10
#define RW_VDI_DW_NEED      0x14
#define RW_VDI_HPCNT        0x18
#define RW_VDI_HBCNT        0x1C
#define RW_VDI_INPUT_CTRL   0x20
	#define VIN_Y_CHN_SEL	    (0x03<<0)
	#define VIN_CB_CHN_SEL	    (0x03<<2)
	#define VIN_CR_CHN_SEL      (0x03<<4)   
	#define VIN_INSERVE_BIT     (0x01<<6)
	#define VIN_12BIT_BIT	    (0x01<<7) 
	#define VIN_10BIT_BIT 	    (0x01<<8) 
	#define VIN_444_BIT	        (0x01<<9)
	#define VIN_C_DEL_SEL 	    (0x01<<10)
	#define VIN_C2_DEL_SEL	    (0x01<<11)
	#define VIN_SD_4FS_OPT		(0x01<<23)
	#define VIN_3D_WRAP         (0x01<<27)
    #define VIN_SD_480I_MIX_ECO (0x01<<31)
	
#define RW_VDI_VPCNT        0x24
#define RW_VDI_3D_TOTAL     0x28
#define RW_VDI_3D_VPOS_H    0x2C
#define RW_VDI_DATA         0x30
	#define VIN_3D_VSYC_WIDTH	       (0xFF<<0)
	#define VIN_3D_HSYC_WIDTH		   (0xFF<<16)
	#define VIN_3D_HSYNC_IN_POLARITY 	   (0x01<<24)
	#define VIN_3D_VSYNC_IN_POLARITY	(0x01<<25)
	#define VIN_3D_FREE_RUN	   (0x01<<26) 
	#define VIN_3D_RST		   (0x01<<27) 
	#define SW_RST		       (0x03<<30) 

#define RW_VDI_HCREG1       0x34
#define RW_VDI_HCREG2       0x38
#define RW_VDI_VSCALE       0x3C
#define RW_VDI_HSCALE       0x54
#define RO_VDI_DBG1         0x44
	#define BLV_FIELD			   (0x01<<26) 
#define RO_VDI_DBG2         0x48
#define RO_VDI_DBG3         0x4C
#define RO_VDI_DBG4         0x58
#define RO_VDI_DBG5         0x5C

#endif
