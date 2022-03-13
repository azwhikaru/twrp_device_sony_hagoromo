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
 /******************************************************************************
*[File]             edid_data.h
*[Version]          v0.1
*[Revision Date]    2009-06-116
*[Author]           Kenny Hsieh
*[Description]
*    source file for HDMI Rx EDID data
*
*
******************************************************************************/
#ifndef _EDID_DATA_H_
#define _EDID_DATA_H_

#include "typedef.h"

#if 1//(DRV_SUPPORT_HDMI_RX)

//=============================================================================
#define SINK_480P       (1<< 0)
#define SINK_720P60     (1<< 1)
#define SINK_1080I60    (1<< 2)
#define SINK_1080P60    (1<< 3)
#define SINK_480P_1440  (1<< 4)
#define SINK_480P_2880  (1<< 5)
#define SINK_480I       (1<< 6)//actuall 480Ix1440
#define SINK_480I_1440  (1<< 7)//actuall 480Ix2880
#define SINK_480I_2880  (1<< 8)//No this type for 861D
#define SINK_1080P30    (1<< 9)
#define SINK_576P       (1<< 10)
#define SINK_720P50     (1<< 11)
#define SINK_1080I50    (1<< 12)
#define SINK_1080P50    (1<< 13)
#define SINK_576P_1440  (1<< 14)
#define SINK_576P_2880  (1<< 15)
#define SINK_576I       (1<< 16)
#define SINK_576I_1440  (1<< 17)
#define SINK_576I_2880  (1<< 18)
#define SINK_1080P25    (1<< 19)
#define SINK_1080P24    (1<< 20)
#define SINK_1080P23976 (1<< 21)
#define SINK_1080P2997  (1<< 22)
#define SINK_VGA       (1<< 23)//640x480P
#define SINK_480I_4_3   (1<< 24)//720x480I 4:3
#define SINK_480P_4_3   (1<< 25)//720x480P 4:3
#define SINK_480P_2880_4_3 (1<< 26)
#define SINK_576I_4_3   (1<< 27)//720x480I 4:3
#define SINK_576P_4_3   (1<< 28)//720x480P 4:3
#define SINK_576P_2880_4_3 (1<< 29)
#define SINK_720P24       (1<<30)
#define SINK_720P23976 (1<<31)

#define		EDID_SUPPORT_HD_AUDIO
#define _PRINT_EDID_

#define EDID_DEFAULT_BL1_ADDR_PHY		0x20

#define	HDMI_INPUT_COUNT				2

#define EEPROM_ID						0xA0	
#define	EDID_BLOCK_SIZE					128
#define	EDID_ADR_CHECK_SUM				0x7F
#define EDID_BL0_ADR_EXTENSION_NMB		0x7E

#define EDID_BL0_ADR_HEADER				0x00
#define EDID_BL0_LEN_HEADER				8

#define EDID_BL0_ADR_VERSION			0x12
#define EDID_BL0_ADR_REVISION			0x13

#define EDID_MONITOR_NAME_DTD			0xFC
#define EDID_MONITOR_RANGE_DTD			0xFD

#define	EDID_BL1_ADR_DTD_OFFSET			0x02
#define EDID_BL0_ADR_DTDs				0x36
#define END_1stPAGE_DESCR_ADDR			0x7E

#define EDID_MANUFACTURER_ID	0x4C, 0x2D	//SAM
#define EDID_PRODUCT_ID			0x01, 0x00	//0001
#define EDID_SERIAL_NUMBER		0x10, 0x00, 0x00,0x00		//00000001
#define EDID_WEEK				0x00
#define	EDID_YEAR				0x12	//2008
#define VENDOR_NAME	'B','D','-','H','T','S',0x0A,' ',' ',' ',' ', ' ', ' '	// "MT85xx-AVR0 "

#define DEFAULT_MIN_V_HZ			48 // Min. Vertical rate (for interlace this refers to field rate), in Hz.
#define DEFAULT_MAX_V_HZ			62 // Max. Vertical rate (for interlace this refers to field rate), in Hz.
#define DEFAULT_MIN_H_KHZ			23 // Min. Horizontal in kHz
#define DEFAULT_MAX_H_KHZ			47 // Max. Horizontal in kHz,
#define DEFAULT_MAX_PIX_CLK_10MHZ	8 // Max. Supported Pixel Clock, in MHz/10, rounded
#define	EDID_VIDEO_BLOCK_LEN		20 //17

#define	EDID_AUDIO_BLOCK_LEN		28
#define EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN		13

#define	EDID_SPEAKER_BLOCK_LEN		4
#define	EDID_VENDOR_BLOCK_FULL_LEN	22
#define	EDID_VENDOR_BLOCK_LEN		7
#define	EDID_VENDOR_BLOCK_4K_2K_FULL_LEN	19



#define	EDID_VENDOR_BLOCK_MINI_LEN	  7  // jitao.shi@20100921 for Sony mini VSDB include support Ai information
#define EDID_COLORMETRY_BLOCK_LEN     4
#define EDID_VCDB_BLOCK_LEN           3
#define EDID_DTD_BLOCK_LEN            72 ////'4*18

#define AUDIO_TAG			0x20
#define VIDEO_TAG			0x40
#define VENDOR_TAG			0x60
#define SPEAKER_TAG			0x80

#define EEPROM0 0
#define EEPROM1 1
#define EEPROM2 2

#define EDIDDEV0 0
#define EDIDDEV1 1
#define EDIDDEV2 2

typedef struct _PHY_ADDR_PARAMETER_T
{
  UINT16	Origin;
  UINT16	Dvd;
  UINT16	Sat;
  UINT16	Tv;
  UINT16	Else;
}PHY_ADDR_PARAMETER_T;


typedef struct _EDID_PARAMETER_T
{
  UINT8	PHYLevel;
  UINT8	bBlock0Err;
  UINT8	bBlock1Err;
  UINT8	PHYPoint;
  UINT8	bCopyDone;
  UINT8	bDownDvi;
  UINT8	Number;
}EDID_PARAMETER_T;


void EdidProcessing(void);
void Default_Edid_BL0_BL1_Write(void);
void vEDIDCreateBlock1(void);
void vReadRxEDID(UINT8 u1EDID);
void vSetEdidUpdateMode(BOOL fgVideoAndOp, BOOL fgAudioAndOP);
void vWriteEDIDBlk0(UINT8 u1EDIDNo, UINT8 *poBlock);
void vWriteEDIDBlk1(UINT8 u1EDIDNo, UINT8 *poBlock);
void vShowEdidPhyAddress(void);

#endif
#endif

