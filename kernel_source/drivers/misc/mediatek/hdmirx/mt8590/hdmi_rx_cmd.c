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
#include <generated/autoconf.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/vmalloc.h>
#include <linux/disp_assert_layer.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/switch.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/mach-types.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <mach/dma.h>
#include <mach/irqs.h>
#include <asm/tlbflush.h>
#include <asm/page.h>

//#include "common.h"
#include "hdmi_rx_dvi.h"
#include "hdmi_rx_hal.h"
#include "hdmi_rx_ctrl.h"
#include "hdmi_rx_hw.h"
#include "hdmi_rx_task.h"
#include "hdmictrl.h"


void vSetHDMIRXHDCP(BYTE bData);
void HalHdmiRxHBR(void);
void _HdmiRxPHYInit(void);
void vAudPllSetting(void);
void _GetTimingInfomation(void);
void HalHdmiRxDSDBypass(void);
void _GetTimingInfomationNoreset(void);
void _SetHDMIINPort(UINT8 u1HDMIINSwitch);
void _HdmiRxToCCIRToTX(void);
void _RepeaterMode(UINT32 u1Mode, UINT32 u1HdmiInPort);

#define HDMIRX_BASE            0xFC006000
#define HDMIRX_ANA_BASE   	   0xF0209000
#define HDMI_RGB_REG_BASE      0xFC008800

const CHAR* szRxResStr0[] =
{
    "MODE_NOSIGNAL",        // No signal   //  0
    "MODE_525I_OVERSAMPLE",      //SDTV 
    "MODE_625I_OVERSAMPLE",       //
    "MODE_480P_OVERSAMPLE",       //SDTV
    "MODE_576P_OVERSAMPLE",
    "MODE_720p_50",               //HDTV 
    "MODE_720p_60",               //HDTV   
    "MODE_1080i_48",              //HDTV  
    "MODE_1080i_50",              //HDTV  
    "MODE_1080i",                 //HDTV
    "MODE_1080p_24",              //HDTV 
    "MODE_1080p_25",
    "MODE_1080p_30",
    "MODE_1080p_50",              //HDTV 
    "MODE_1080p_60",
    "MODE_525I",
    "MODE_625I",
    "MODE_480P",
    "MODE_576P",    
    "MODE_720p_24",   
    "MODE_720p_25",    // 20 
    "MODE_720p_30",        
    "MODE_240P",
    "MODE_540P",
    "MODE_288P",    
    "MODE_480P_24",    
    "MODE_480P_30",        
    "MODE_576P_25",    
    "MODE_3D_720p_50",
    "MODE_3D_720p_60",        
    "MODE_3D_1080p_24",      //30
    "MODE_3D_1080I_60_FRAMEPACKING",   
    "MODE_3D_1080I_50_FRAMEPACKING",     
    "MODE_3D_1080P60HZ",
    "MODE_3D_1080P50HZ",
    "MODE_3D_1080P30HZ",
    "MODE_3D_1080P25HZ",
    "MODE_3D_720P30HZ", 
    "MODE_3D_720P25HZ", 
    "MODE_3D_720P24HZ",
    "MODE_3D_576P50HZ",      // 40
    "MODE_3D_576I50HZ", 
    "MODE_3D_480P60HZ", 
    "MODE_3D_480I60HZ", 
    "MODE_REVERSE1",
    "MODE_REVERSE2",    
    "MODE_HDMI_640_480P",  // = 46,
};

UINT32 pdRegs_RX_GENERAL[] =
{
	0x1C006038,0x00002280,//set H/V stable threshold
	0x1C006128,0x00001FE4,//audio path setting
	0x1C006124,0xf1400000,//audio path setting
	0x1C00613C,0xc0060001,//powerdown disable,
	0x1C006008,0x00001407,
	0x1C006134,0x00000600,
	0x1C006014,0x865e01ea,
	0x1C0060FC,0x00000000,
	0x1C0060F8,0x0000ff00,
	0x1C006048,0x000a0000,
};


UINT8 pdRegs_RX_HDCPKEYDVI[] =
{

};

UINT8 HDMI_EDID_TABLE[256] =
{
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x36,0x8b,0x01,0x00,0x00,0x00,0x00,0x00,
	0x01,0x0f,0x01,0x03,0x80,0x3c,0x22,0x78,0x0a,0x0d,0xc9,0xa0,0x57,0x47,0x98,0x27,
	0x12,0x48,0x4c,0xbf,0xef,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x1d,0x00,0x72,0x51,0xd0,0x1e,0x20,0x6e,0x28,
	0x55,0x00,0xc4,0x8e,0x21,0x00,0x00,0x1e,0x01,0x1d,0x80,0x18,0x71,0x1c,0x16,0x20,
	0x58,0x2c,0x25,0x00,0xc4,0x8e,0x21,0x00,0x00,0x9e,0x00,0x00,0x00,0xfc,0x00,0x4d,
	0x54,0x4b,0x20,0x4c,0x43,0x44,0x54,0x56,0x0a,0x20,0x20,0x20,0x00,0x00,0x00,0xfd,
	0x00,0x31,0x4c,0x0f,0x50,0x0e,0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0x5a,
	0x02,0x03,0x20,0x74,0x4b,0x84,0x10,0x1f,0x05,0x13,0x14,0x01,0x02,0x11,0x06,0x15,
	0x23,0x09,0x07,0x03,0x83,0x01,0x00,0x00,0x67,0x03,0x0c,0x00,0x10,0x00,0xb8,0x2d,
	0x01,0x1d,0x00,0xbc,0x52,0xd0,0x1e,0x20,0xb8,0x28,0x55,0x40,0xc4,0x8e,0x21,0x00,
	0x00,0x1e,0x01,0x1d,0x80,0xd0,0x72,0x1c,0x16,0x20,0x10,0x2c,0x25,0x80,0xc4,0x8e,
	0x21,0x00,0x00,0x9e,0x8c,0x0a,0xd0,0x8a,0x20,0xe0,0x2d,0x10,0x10,0x3e,0x96,0x00,
	0x13,0x8e,0x21,0x00,0x00,0x18,0x8c,0x0a,0xd0,0x90,0x20,0x40,0x31,0x20,0x0c,0x40,
	0x55,0x00,0x13,0x8e,0x21,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x78,
};

BYTE pdSNYEdid[] =
{
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x4d,0xd9,0x01,0x52,0x01,0x01,0x01,0x01,
	0x01,0x11,0x01,0x03,0x80,0xa0,0x5a,0x78,0x0a,0x0d,0xc9,0xa0,0x57,0x47,0x98,0x27,
	0x12,0x48,0x4c,0x21,0x08,0x00,0x81,0x80,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x3a,0x80,0x18,0x71,0x38,0x2d,0x40,0x58,0x2c,
	0x45,0x00,0x40,0x84,0x63,0x00,0x00,0x1e,0x01,0x1d,0x00,0x72,0x51,0xd0,0x1e,0x20,
	0x6e,0x28,0x55,0x00,0x40,0x84,0x63,0x00,0x00,0x1e,0x00,0x00,0x00,0xfc,0x00,0x53,
	0x4f,0x4e,0x59,0x20,0x54,0x56,0x20,0x58,0x56,0x0a,0x20,0x20,0x00,0x00,0x00,0xfd,
	0x00,0x30,0x3e,0x0e,0x46,0x0f,0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0xef,
	0x02,0x03,0x2c,0xf0,0x50,0x1f,0x03,0x04,0x12,0x13,0x05,0x14,0x20,0x07,0x16,0x10,
	0x15,0x11,0x02,0x06,0x01,0x23,0x09,0x07,0x07,0x83,0x01,0x00,0x00,0x67,0x03,0x0c,
	0x00,0x30,0x00,0xb8,0x2d,0xe3,0x05,0x03,0x01,0xe2,0x00,0x79,0x02,0x3a,0x80,0xd0,
	0x72,0x38,0x2d,0x40,0x10,0x2c,0x45,0x80,0x40,0x84,0x63,0x00,0x00,0x1e,0x01,0x1d,
	0x00,0xbc,0x52,0xd0,0x1e,0x20,0xb8,0x28,0x55,0x40,0x40,0x84,0x63,0x00,0x00,0x1e,
	0x01,0x1d,0x80,0x18,0x71,0x1c,0x16,0x20,0x58,0x2c,0x25,0x00,0x40,0x84,0x63,0x00,
	0x00,0x9e,0x01,0x1d,0x80,0xd0,0x72,0x1c,0x16,0x20,0x10,0x2c,0x25,0x80,0x40,0x84,
	0x63,0x00,0x00,0x9e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x83,
};
BYTE pdYMH1600Edid[] =
{
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x1e,0x6d,0x98,0x9c,0xa6,0x61,0x06,0x00,
	0x03,0x11,0x01,0x03,0x80,0x46,0x27,0x78,0xea,0xd9,0xb0,0xa3,0x57,0x49,0x9c,0x25,
	0x11,0x49,0x4b,0xa1,0x08,0x00,0xa9,0x40,0x81,0x80,0x81,0x40,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x1d,0x80,0x18,0x71,0x1c,0x16,0x20,0x58,0x2c,
	0x25,0x00,0xc4,0x8e,0x21,0x00,0x00,0x9e,0x01,0x1d,0x00,0x80,0x51,0xd0,0x1c,0x20,
	0x40,0x80,0x35,0x00,0xbc,0x88,0x21,0x00,0x00,0x1e,0x00,0x00,0x00,0xfd,0x00,0x2f,
	0x3f,0x1c,0x4b,0x10,0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xfc,
	0x00,0x4c,0x47,0x20,0x54,0x56,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0xad,
	0x02,0x03,0x2b,0xf6,0x4b,0x85,0x04,0x01,0x03,0x12,0x13,0x14,0x0e,0x0f,0x1d,0x1e,
	0x2f,0x09,0x7f,0x07,0x0d,0x1f,0x07,0x15,0x07,0x38,0x35,0x07,0x30,0x3d,0x07,0xc0,
	0x83,0x0f,0x00,0x00,0x66,0x03,0x0c,0x00,0x21,0x00,0x80,0x8c,0x0a,0xd0,0x8a,0x20,
	0xe0,0x2d,0x10,0x10,0x3e,0x96,0x00,0xc4,0x8e,0x21,0x00,0x00,0x18,0x8c,0x0a,0xd0,
	0x90,0x20,0x40,0x31,0x20,0x0c,0x40,0x55,0x00,0xc4,0x8e,0x21,0x00,0x00,0x18,0x01,
	0x1d,0x00,0xbc,0x52,0xd0,0x1e,0x20,0xb8,0x28,0x55,0x40,0xc4,0x8e,0x21,0x00,0x00,
	0x1e,0x01,0x1d,0x80,0xd0,0x72,0x1c,0x16,0x20,0x10,0x2c,0x25,0x80,0xc4,0x8e,0x21,
	0x00,0x00,0x9e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x41,
};
BYTE pdYMH1800Edid[] =
{
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x4d,0xd9,0x01,0x52,0x01,0x01,0x01,0x01,
	0x01,0x11,0x01,0x03,0x80,0xa0,0x5a,0x78,0x0a,0x0d,0xc9,0xa0,0x57,0x47,0x98,0x27,
	0x12,0x48,0x4c,0x21,0x08,0x00,0x81,0x80,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x01,0x01,0x8c,0x0a,0xd0,0x8a,0x20,0xe0,0x2d,0x10,0x10,0x3e,
	0x96,0x00,0xc4,0x8e,0x21,0x00,0x00,0x18,0x01,0x1d,0x00,0x72,0x51,0xd0,0x1e,0x20,
	0x6e,0x28,0x55,0x00,0x40,0x84,0x63,0x00,0x00,0x1e,0x00,0x00,0x00,0xfc,0x00,0x53,
	0x4f,0x4e,0x59,0x20,0x54,0x56,0x20,0x58,0x56,0x0a,0x20,0x20,0x00,0x00,0x00,0xfd,
	0x00,0x30,0x3e,0x0e,0x46,0x0f,0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0x4b,
	0x02,0x03,0x4a,0xf4,0x5c,0x1f,0x03,0x04,0x12,0x13,0x05,0x14,0x20,0x07,0x16,0x10,
	0x15,0x11,0x02,0x06,0x01,0x0e,0x0f,0x1d,0x1e,0x0a,0x0b,0x19,0x1a,0x23,0x24,0x25,
	0x26,0x3b,0x09,0x7f,0x07,0x0f,0x7f,0x07,0x15,0x07,0x50,0x35,0x07,0x48,0x3e,0x1f,
	0xc0,0x4d,0x02,0x00,0x57,0x06,0x00,0x67,0x54,0x00,0x5f,0x54,0x01,0x83,0x5f,0x00,
	0x00,0x68,0x03,0x0c,0x00,0x21,0x00,0xb8,0x2d,0x00,0x8c,0x0a,0xd0,0x90,0x20,0x40,
	0x31,0x20,0x0c,0x40,0x55,0x00,0xc4,0x8e,0x21,0x00,0x00,0x18,0x01,0x1d,0x00,0xbc,
	0x52,0xd0,0x1e,0x20,0xb8,0x28,0x55,0x40,0x40,0x84,0x63,0x00,0x00,0x1e,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa2,
};

void vPhyCrcStart(void)
{
	HDMIRX_WRITE32_MASK(REG_HDM_MACRO_CRC,1<<29,1<<29);
}

void vPhyCrcStartStop(void)
{
	HDMIRX_WRITE32_MASK(REG_HDM_MACRO_CRC,0,1<<29);
}

void vPhyCrcClear(void)
{
	HDMIRX_WRITE32_MASK(REG_HDM_MACRO_CRC,1<<28,1<<28);
}

void vPhyCrcClearStop(void)
{
	HDMIRX_WRITE32_MASK(REG_HDM_MACRO_CRC,0,1<<28);
}


void vPhyCrcMode(BYTE bMode)
{
	HDMIRX_WRITE32_MASK(REG_HDM_MACRO_CRC,(bMode&0x03)<<26,3<<26);
}


BOOL fgPhyCrcReady(void)
{
	return((HDMIRX_READ32(REG_HDM_MACRO_CRC)>>25)&0x01);
}

BOOL fgPhyCrcFail(void)
{
	return((HDMIRX_READ32(REG_HDM_MACRO_CRC)>>24)&0x01);
}

UINT32 u4PhyCrcDataOutput(void)
{
	return(HDMIRX_READ32(REG_HDM_MACRO_CRC)&0xFFFFFF);
}

BOOL vHdmiRxPhyCrc(INT16 ntry)
{
    UINT8 idx;
	UINT8 result[3][3];
	UINT8 tmp[3];

	idx = 0;
	result[0][0] = 0;
	result[0][1] = 0;
	result[0][2] = 0;
	result[1][0] = 0;
	result[1][1] = 0;
	result[1][2] = 0;
	result[2][0] = 0;
	result[2][1] = 0;
	result[2][2] = 0;	

	printk("vHdmiRxPhyCrc: %d\n", ntry);

	while (ntry > 0)
	{
		ntry--;
		mdelay(10);
		if (idx > 2)
		{
			printk("CRC fail\n");
			return 0;
		}
        vPhyCrcStartStop();
		vPhyCrcClear();
		mdelay(1);
		vPhyCrcClearStop();
		printk("CRC clear status\n");

		while (fgPhyCrcReady())
		{
			mdelay(1);

		}
		while (u4PhyCrcDataOutput() != 0x00)
		{
			mdelay(1);
		}


		if (1)
		{
			vPhyCrcStart();
			printk("PHY CRC start\n");

			while (fgPhyCrcReady()!=1)
			{
				mdelay(1);
			}

			if ( fgPhyCrcReady() == 0x1 )
			{
				tmp[0] = (u4PhyCrcDataOutput()&0xFF);
				tmp[1] = ((u4PhyCrcDataOutput()>>8)&0xFF);
				tmp[2] = ((u4PhyCrcDataOutput()>>16)&0xFF);
				// compare and update result if necessary
				if ((tmp[0] == result[0][0]) && (tmp[1] == result[0][1]) && (tmp[2] == result[0][2]))
				{
					continue;
				}
				if ((tmp[0] == result[1][0]) && (tmp[1] == result[1][1]) && (tmp[2] == result[1][2]))
				{
					continue;
				}
				/*lint -e661*/ /*lint -e662*/
				result[idx][0] = tmp[0];
				result[idx][1] = tmp[1];
				result[idx][2] = tmp[2];
				/*lint +e661*/ /*lint +e662*/
				idx++;
				continue;
			}
			else
			{
				printk("CRC is not ready\n");
				return 0;
			}
		}
		else
		{
			printk("reset CRC fail");
			return 0;
		}
	}

	if (HDMIRX_READ32(REG_VID_CH_MAP) & 0x0400)
	{
		printk("interlace signal\n");
	}
	else
	{
		printk("progressive signal\n");
	}

	if (idx == 1)
	{
		printk("assume progressive signal\n");
		printk("CRC result:\n");
		printk("%x %x %x\n", result[0][0], result[0][1], result[0][2]);
	}
	else if (idx == 2)
	{
		printk("assume interlaced signal\n");
		printk("CRC result:\n");
		printk("%x %x %x\n", result[0][0], result[0][1], result[0][2]);
		printk("%x %x %x\n", result[1][0], result[1][1], result[1][2]);
	}

	return 1;

}
// HDMI RX MACRO CRC CHECK
INT32 _HdmiRxPhyCrcCheck(INT16 ntry) 
{
	vHdmiRxPhyCrc(ntry);
}

// AFTER DECODER , HDMI RX VIDEO DATA CRC CHECK
INT32 _HdmiRxCrcCheck(INT16 ntry)
{
  HDMIRX_WRITE32_MASK(0x68,0,(1<<23));
  fgHDMICRC(ntry);
}

void vLoadEdidFromCODE(void)
{
	UINT8 i;
    printk("[HDMI RX]-----------\n");
	UINT32 u4Data;
	DDCCI_WRITE32_MASK(0x1c,(1<<26)|(1<<23),(1<<26)|(1<<23));//enable download mode
	for(i=0; i<64; i++)
	{
		DDCCI_WRITE32(0x50, pdYMH1800Edid[i*4]|(pdYMH1800Edid[i*4+1]<<8)|(pdYMH1800Edid[i*4+2]<<16)|(pdYMH1800Edid[i*4+3]<<24));//address 255 is invalid
	}
	DDCCI_WRITE32_MASK(0x1c,(1<<26),(1<<26)|(1<<23));	//disable download mode
    printk("[HDMI RX]-(1<<26)|(1<<23) = %x----------\n",(1<<26)|(1<<23));
    printk("[HDMI RX]-(1<<26)|(1<<23) = %x----------\n",~((1<<26)|(1<<23)));
	printk("[HDMI RX]read 1c = %x\n",DDCCI_READ32(0x1c));
    printk("[HDMI RX]read 1c &~ = %x\n",DDCCI_READ32(0x1c) & (~((1<<26)|(1<<23))));
    printk("[HDMI RX]value & mask = %x\n", (1<<26) & ((1<<26)|(1<<23)) );
    printk("[HDMI RX]result = %x\n", (DDCCI_READ32(0x1c) & (~((1<<26)|(1<<23)))) | ((1<<26) & ((1<<26)|(1<<23))) );
	u4Data = DDCCI_READ32(0x1c);
	
	printk("[HDMI RX]u4Data = %x\n",u4Data);
	
	//DDCCI_WRITE32(0x1c,0x04000000);
	DDCCI_WRITE32_MASK(0x04,(pdYMH1800Edid[255]<<16),(0xff<<16));//checksum
}

// for the whole system power on
void MMSYS_BDP_POWER_ON(void)
{
	// MMSYS BDP POWER ON
	vRxWriteReg(0x6000,0x0b160001);
	vRxWriteReg(0x5170,0x0000f81f);
	vRxWriteReg(0x623c,0x0000ff16);
	vRxWriteReg(0x623c,0x0000ff1e);
	vRxWriteReg(0x623c,0x0000ff0e);
	vRxWriteReg(0x623c,0x0000ff0c);
	vRxWriteReg(0x623c,0x0000fe0c);
	vRxWriteReg(0x623c,0x0000fc0c);
	vRxWriteReg(0x623c,0x0000f80c);
	vRxWriteReg(0x623c,0x0000f00c);
	vRxWriteReg(0x623c,0x0000f00d);
	vRxWriteReg(0x629c,0x0000ff16);
	vRxWriteReg(0x629c,0x0000ff1e);
	vRxWriteReg(0x629c,0x0000ff0e);
	vRxWriteReg(0x629c,0x0000ff0c);
	vRxWriteReg(0x629c,0x0000fe0c);
	vRxWriteReg(0x629c,0x0000fc0c);
	vRxWriteReg(0x629c,0x0000f80c);
	vRxWriteReg(0x629c,0x0000f00c);
	vRxWriteReg(0x629c,0x0000f00d);

}

static void HalHdmiRxPHYInit(void)
{

	UINT32 u4CKPRD;
	
	printk("[HDMI RX] HdmiRxPHYInit\n");
	
	// ICG setting part
	vRxWriteReg(0xC000100,0x00000000);
	vRxWriteReg(0xC000110,0x00000000);
	vRxWriteReg(0xC000104,0x00000000);
	
	//HDMI Setting
	vRxWriteReg(0xC00001C,0xfb5aab4f);
	vRxWriteReg(0xC000020,0x00000000);
	vRxWriteReg(0xC000024,0x00000001);
	vRxWriteReg(0xC0062EC,0x00132000);
	vRxWriteReg(0xC0062EC,0x00102000);
	vRxWriteReg(0x0209164,0x0000000E);

	vRxWriteRegMsk(0xC0062f4,(1<<1),(1<<1));
	vRxWriteRegMsk(0xC0062f4,(1<<0),(1<<0));
	mdelay(1);
	u4CKPRD = ((((vRxReadReg(0xC0062f8))& 0xFF)<< 8)|((vRxReadReg(0xC0062f8)>>8)& 0xFF));
	mdelay(1);
	//u4CKPRD = (vRxReadReg(0xC00622C)>>12)&0xFF;
	
	if((26*64) <= (u4CKPRD*(27+3)))  // 27MHz
	{
		printk("[HDMI_RX] <=27MHz \n");
		vRxWriteReg(0x209150,0x80000000);
		mdelay(1);
		vRxWriteReg(0x209154,0x55A00E4A);
		mdelay(1);
		vRxWriteReg(0x209158,0x2B217612);
		mdelay(1);
		vRxWriteReg(0x20915C,0x00B13F30);
		mdelay(1);
		vRxWriteReg(0x209160,0x00000002);
		mdelay(1);
		vRxWriteReg(0x209168,0x00000004);
		mdelay(1);
		vRxWriteReg(0x20916C,0x060A0000);
		mdelay(1);
		vRxWriteReg(0x209160,0x00000802);
		vRxWriteReg(0x209158,0x3B217612);
		mdelay(1);
		vRxWriteReg(0x209150,0x00000000);
		mdelay(1);
		vRxWriteReg(0x209154,0x55A00A4A);
		vRxWriteReg(0x209158,0x3B217613);
		mdelay(1);
		vRxWriteReg(0x209154,0x55200A4A);
		mdelay(1);
	}
	else if((26*64) <= (u4CKPRD*(40+5)))
	{
		printk("[HDMI_RX] <=40MHz \n");
		vRxWriteReg(0x209150,0x80000000);
		mdelay(1);
		vRxWriteReg(0x209154,0x55A00E4A);
		mdelay(1);
		vRxWriteReg(0x209158,0x2B217612);
		mdelay(1);
		vRxWriteReg(0x20915C,0x00914F30);
		mdelay(1);
		vRxWriteReg(0x209160,0x0000000A);
		mdelay(1);
		vRxWriteReg(0x209168,0x00000004);
		mdelay(1);
		vRxWriteReg(0x20916C,0x060A0000);
		mdelay(1);
		vRxWriteReg(0x209160,0x0000080A);
		vRxWriteReg(0x209158,0x3B217612);
		mdelay(1);
		vRxWriteReg(0x209150,0x00000000);
		mdelay(1);
		vRxWriteReg(0x209154,0x55A00A4A);
		vRxWriteReg(0x209158,0x3B217613);
		mdelay(1);
		vRxWriteReg(0x209154,0x55200A4A);
		mdelay(1);
	}
	else if((26*64) <= (u4CKPRD*60))
	{
		printk("[HDMI_RX] <=60MHz \n");
		vRxWriteReg(0x209150,0x80000000);
		mdelay(1);
		vRxWriteReg(0x209154,0x55A00E4A);
		mdelay(1);
		vRxWriteReg(0x209158,0x2B217412);
		mdelay(1);
		vRxWriteReg(0x20915C,0x00914F30);
		mdelay(1);
		vRxWriteReg(0x209160,0x0000000A);
		mdelay(1);
		vRxWriteReg(0x209168,0x00000004);
		mdelay(1);
		vRxWriteReg(0x20916C,0x060A0000);
		mdelay(1);
		vRxWriteReg(0x209160,0x0000080A);
		vRxWriteReg(0x209158,0x3B217412);
		mdelay(1);
		vRxWriteReg(0x209150,0x00000000);
		mdelay(1);
		vRxWriteReg(0x209154,0x55A00A4A);
		vRxWriteReg(0x209158,0x3B217413);
		mdelay(1);
		vRxWriteReg(0x209154,0x55200A4A);
		mdelay(1);
	}
	else if((26*64) <= (u4CKPRD*(75+2)))
	{
		printk("[HDMI_RX] <=75MHz \n");
		vRxWriteReg(0x209150,0x80000000);
		mdelay(1);
		vRxWriteReg(0x209154,0x55A00E4A);
		mdelay(1);
		vRxWriteReg(0x209158,0x2B217412);
		mdelay(1);
		vRxWriteReg(0x20915C,0x00814C30);
		mdelay(1);
		vRxWriteReg(0x209160,0x00000008);
		mdelay(1);
		vRxWriteReg(0x209168,0x00000004);
		mdelay(1);
		vRxWriteReg(0x20916C,0x06020000);
		mdelay(1);
		vRxWriteReg(0x209160,0x00000808);
		vRxWriteReg(0x209158,0x3B217412);
		mdelay(1);
		vRxWriteReg(0x209150,0x00000000);
		mdelay(1);
		vRxWriteReg(0x209154,0x55A00A4A);
		vRxWriteReg(0x209158,0x3B217413);
		mdelay(1);
		vRxWriteReg(0x209154,0x55200A4A);
		mdelay(1);
	}
	else if((26*64) <= (u4CKPRD*(110+4)))
	{
		printk("[HDMI_RX] <=110MHz \n");
		vRxWriteReg(0x209150,0x80000000);
		mdelay(1);
		vRxWriteReg(0x209154,0x55A00E4A);
		mdelay(1);
		vRxWriteReg(0x209158,0x2B217412);
		mdelay(1);
		vRxWriteReg(0x20915C,0x00868630);
		mdelay(1);
		vRxWriteReg(0x209160,0x00010008);
		mdelay(1);
		vRxWriteReg(0x209168,0x00000004);
		mdelay(1);
		vRxWriteReg(0x20916C,0x06020000);
		mdelay(1);
		vRxWriteReg(0x209160,0x00001808);
		vRxWriteReg(0x209158,0x3B217412);
		mdelay(1);
		vRxWriteReg(0x209150,0x00000000);
		mdelay(1);
		vRxWriteReg(0x209154,0x55A00A4A);
		vRxWriteReg(0x209158,0x3B217413);
		mdelay(1);
		vRxWriteReg(0x209154,0x55200A4A);
		mdelay(1);
	}
	else if((26*64) <= (u4CKPRD*160))
	{
		printk("[HDMI_RX] <=160MHz \n");
		vRxWriteReg(0x209150,0x80000000);
		mdelay(1);
		vRxWriteReg(0x209154,0xD5A00E4A);
		mdelay(1);
		vRxWriteReg(0x209158,0x2B217412);
		mdelay(1);
		vRxWriteReg(0x20915C,0x00845630);
		mdelay(1);
		vRxWriteReg(0x209160,0x00010008);
		mdelay(1);
		vRxWriteReg(0x209168,0x00000004);
		mdelay(1);
		vRxWriteReg(0x20916C,0x0612c000);
		mdelay(1);
		vRxWriteReg(0x209160,0x00001808);
		vRxWriteReg(0x209158,0x3B217412);
		mdelay(1);
		vRxWriteReg(0x209150,0x00000000);
		mdelay(1);
		vRxWriteReg(0x209154,0xD5A00A4A);
		vRxWriteReg(0x209158,0x3B217413);
		mdelay(1);
		vRxWriteReg(0x209154,0xD5200A4A);
		mdelay(1);
	}
	else if((26*64) <= (u4CKPRD*(250)))
	{
		printk("[HDMI_RX] <=250MHz \n");
		vRxWriteReg(0x209150,0x80000000);
		mdelay(1);
		vRxWriteReg(0x209154,0xD5A00E4A);
		mdelay(1);
		vRxWriteReg(0x209158,0x2B215012);
		mdelay(1);
		vRxWriteReg(0x20915C,0x00822630);
		mdelay(1);
		vRxWriteReg(0x209160,0x00002009);
		mdelay(1);
		vRxWriteReg(0x209168,0x00000004);
		mdelay(1);
		vRxWriteReg(0x20916C,0x0632c000);
		mdelay(1);
		vRxWriteReg(0x209160,0x00002809);
		vRxWriteReg(0x209158,0x3B215012);
		mdelay(1);
		vRxWriteReg(0x209150,0x00000000);
		mdelay(1);
		vRxWriteReg(0x209154,0xD5A00A4A);
		vRxWriteReg(0x209158,0x3B215013);
		mdelay(1);
		vRxWriteReg(0x209154,0xD5200A4A);
		mdelay(1);
	}
	else
	{
		printk("[HDMI_RX] >250MHz \n");
		vRxWriteReg(0x209150,0x80000000);
		mdelay(1);
		vRxWriteReg(0x209154,0xD5A00E4A);
		mdelay(1);
		vRxWriteReg(0x209158,0x29214012);
		mdelay(1);
		vRxWriteReg(0x20915C,0x00822630);
		mdelay(1);
		vRxWriteReg(0x209160,0x00002009);
		mdelay(1);
		vRxWriteReg(0x209168,0x00000004);
		mdelay(1);
		vRxWriteReg(0x20916C,0x06320000);
		mdelay(1);
		vRxWriteReg(0x209160,0x00002809);
		vRxWriteReg(0x209158,0x39214012);
		mdelay(1);
		vRxWriteReg(0x209150,0x00000000);
		mdelay(1);
		vRxWriteReg(0x209154,0xD5A00A4A);
		vRxWriteReg(0x209158,0x39214013);
		mdelay(1);
		vRxWriteReg(0x209154,0xD5200A4A);
		mdelay(1);
	}
     
	//digital part
	vRxWriteReg(0xC006038,0x00002280);
	vRxWriteReg(0xC006128,0x00001FE4);
	vRxWriteReg(0xC006124,0xF1400000);
	vRxWriteReg(0xC00613C,0xC0060001);
	vRxWriteReg(0xC006134,0x00020600);//zhiqiang modify for 720P 10BIT H ACTIVE ERROR
	vRxWriteReg(0xC006008,0x00001407);
	vRxWriteReg(0xC006014,0x865E01EA);
	vRxWriteReg(0xC006280,0x000003A0);
	vRxWriteReg(0xC006284,0x00000000);
	vRxWriteRegMsk(0xC00606C,(1<<22),(1<<22));
	vRxWriteRegMsk(0xC00606C,(1<<23),(1<<23));
	vRxWriteReg(0xC0060FC,0x00000000);
	vRxWriteReg(0xC0060F8,0x0000ff00);

	vRxWriteRegMsk(0xC006088,(1<<12)|(1<<13),(1<<12)|(1<<13)); //for audio unflow,overflow
	vRxWriteRegMsk(0xC006088,0,(1<<14));
	vRxWriteReg(0xC006100,0x00000000);	//audio clk auto mode
	vRxWriteRegMsk(0xC006260,(1<<8),(1<<8));
	
	printk("[HDMI RX] vHDMIRXColorSpaceConveter \n");
	mdelay(100);
    vHDMIRXColorSpaceConveter();
	vLoadEdidFromCODE();//
	vSetHDMIRXHDCP(1);//write hdcp key
    _GetTimingInfomationNoreset();
}

void _HdmiRxPHYInit(void)
{
	HalHdmiRxPHYInit();
}

void _SetHDMIINPort(UINT8 u1HDMIINSwitch)
{

	printk("HDMI RX PORT =%d\n",u1HDMIINSwitch);
	
	if(u1HDMIINSwitch == 1)//PORT1 select
	{
		vRxWriteRegMsk(0x209158,0,1<<31);	//HDMI Port1 En
		vRxWriteRegMsk(0xC006270, 0 ,DDC_SEL);// DDC select to Port1
		vRxWriteRegMsk(0xC006270,0,PWR5V_SEL);	// DDC select to Port2
	}

}

void vResetVpll(void)
{
	vRxWriteRegMsk(0x4000334,0,(1<<21));
	udelay(20);
	
	vRxWriteRegMsk(0x4000334,(1<<21),(1<<21));
	
	vRxWriteRegMsk(0x400033c, 0, ((1<<12)|(1<<23)));
	udelay(20);
	
	vRxWriteRegMsk(0x400033c, (1<<23), (1<<23));
	udelay(20);
	
	vRxWriteRegMsk(0x400033c, 1<<12, 1<<12);
}

void _HdmiRxToCCIRToTX(void)
{
	printk("RxToCCIRToTX\n");

	vRxWriteRegMsk(0xC008574,(1<<18),(1<<18));
	vRxWriteRegMsk(0xC008010,0,(1<<26));
	vRxWriteRegMsk(0xC008010,(1<<1),(1<<1));
	vRxWriteRegMsk(0xC008578,(1<<4),(1<<4));
	vRxWriteRegMsk(0xC00850c,(1<<30)|(1<<31),(1<<30)|(1<<31));
	vRxWriteRegMsk(0xC008504,(1<<4)|(1<<5),(1<<4)|(1<<5));
	vRxWriteRegMsk(0xC008504,(1<<10),(1<<10));

	vRxWriteRegMsk(0xC00850c,0,(1<<19));
	vRxWriteRegMsk(0xC00852c,(1<<18),(1<<18));
	vRxWriteRegMsk(0xC00852c,(1<<20),(1<<20));

	vRxWriteRegMsk(0xC00850c,(1<<10),(1<<10));
	vRxWriteRegMsk(0xC008588,(1<<8),(1<<8));

	vRxWriteRegMsk(0xC0080ac,(1<<3),(1<<3));
	vRxWriteRegMsk(0xC00800c,(1<<16)|(1<<17),(1<<16)|(1<<17));

	vRxWriteRegMsk(0xC006004,(1<<26),(1<<26));
	mdelay(100);

	vResetVpll(); //reset hdmi tx pll
	mdelay(100);

	return;
}

void HalHdmiRxAudioReset(void)
{
	printk("[HDMI RX]HalHdmiRxAudioReset\n");
	UINT32 u4Data = 0;
	vRxWriteRegMsk(0xc006004,(1<<10),(1<<10));
	udelay(20);
	vRxWriteRegMsk(0xc006004,0,(1<<10));
	vRxWriteRegMsk(0xc006034,(1<<27),(1<<27));
	mdelay(1);
	vRxWriteRegMsk(0xc006034,0,(1<<27));
	
	mdelay(1);
	vAudPllSetting();
	mdelay(1);
	mdelay(100);
	vRxWriteRegMsk(0xc006004,(1<<9),(1<<9));
	vRxWriteRegMsk(0xc006004,0,(1<<9));
	
	mdelay(1);
	printk("[AUDIO] 60781 = %x\n",vRxReadReg(0xc006078));
	vRxWriteRegMsk(0xc006078,(1<<1)|(1<<0),(1<<1)|(1<<0));
	printk("[AUDIO] 6078 2= %x\n",vRxReadReg(0xc006078));
	vRxWriteRegMsk(0xc006088,(1<<12)|(1<<13),(1<<12)|(1<<13)|(1<<14));	//for audio unflow,overflow
	vRxWriteRegMsk(0xc006078,(1<<1)|(1<<0),(1<<1)|(1<<0));
	//vRxWriteRegMsk(0x1c006084,0,1<<11);	
	u4Data = vRxReadReg(0xc006078)& 0x3;
	printk("[AUDIO] u4Data = %x\n",u4Data);
}

void _HdmiRxAudioReset(void)
{
	HalHdmiRxAudioReset();
}

void vAudPllSetting(void)
{
	printk("MT8590 APLL\n");
	vRxWriteReg(0xc006118,0x00004100);   
	mdelay(1);
	vRxWriteReg(0xc006154,0x2d5efee6);
	//vRxWriteReg(0x1c006154,0x16af753e);
	mdelay(1);
	vRxWriteReg(0xc006118,0x00004300);
	mdelay(1);
	vRxWriteReg(0x2092bc,0x00000003);  
	mdelay(1);
	vRxWriteReg(0x2092bc,0x00000001);  
	mdelay(1);
	vRxWriteReg(0x2092b0,0x00000121); 
	mdelay(1);
	vRxWriteReg(0x209038,0x00C012B3);  
	mdelay(1);
	vRxWriteReg(0xc00001c,0xfb5aab6f);  
	mdelay(1);
	vRxWriteReg(0xc006118,0x00004200);   
	mdelay(1);
}

void vSetHDMIRXHDCP(BYTE bData)
{
	UINT32 u4Ind;
	BYTE KEY[284];
	UINT16 I;
	
	if(bData == 1)
	{
		KEY[0]=0;
		KEY[281]=0;
		KEY[282]=0;
		KEY[283]=0;
		for(I=0;I<280;I++)
		{
			KEY[I+1]=pdRegs_RX_HDCPKEYDVI[I];
		};

		printk("MT8590 KEY	%x %x\n",KEY[0],KEY[1]);
		printk("MT8590 KEY	%x %x\n",KEY[2],KEY[3]);
		printk("MT8590 KEY	%x %x\n",KEY[280],KEY[281]);
		printk("MT8590 KEY	%x %x\n",KEY[282],KEY[283]);
		vRxWriteReg(0xC006280,0x03a0);
		vRxWriteReg(0xC006284,0x00);
		vRxWriteReg(0xC006288,0xE67C1400);
		mdelay(1);
		vRxWriteReg(0xC006288,0xffff372C);
		mdelay(1);
		for(I=0;I<71;I++)
		{
			vRxWriteReg(0xC006288,(KEY[I*4+3]<<24)|(KEY[I*4+2]<<16)|(KEY[I*4+1]<<8)|KEY[I*4+0]);
			mdelay(1);
			printk("MT8590 KEY LIST  %x\n",(KEY[I*4+3]<<24)|(KEY[I*4+2]<<16)|(KEY[I*4+1]<<8)|KEY[I*4+0]);
		}
		vRxWriteReg(0xC0060f8,0x00201000);

	}
	else if(bData == 2)//HDCP repeater setting
	{
		vRxWriteReg(0xC00602c,0x00c00000);
		vRxWriteReg(0xC00602c,0x01c00000);
		vRxWriteReg(0xC006030,0x00000003);	    //  vRxWriteReg(0x1f030,0x00000002);
		vRxWriteReg(0xC006034,0x00004b00);
		vRxWriteReg(0xC006034,0x00404b00);
		mdelay(1);
		vRxWriteReg(0xC006034,0x00004b00);
	}
	else if(bData == 3)//HDCP KSV LIST setting
	{

		u4Ind = vRxReadReg(0xC00602c);
		vRxWriteReg(0xC00602c,u4Ind &(~(1<<19)));


		vRxWriteReg(0xC00611c,0x02000a38);

//KSV LIST
		vRxWriteReg(0xC006120,0x1d);mdelay(1);
		vRxWriteReg(0xC006120,0x04);mdelay(1);
		vRxWriteReg(0xC006120,0xff);mdelay(1);
		vRxWriteReg(0xC006120,0x92);mdelay(1);
		vRxWriteReg(0xC006120,0x6c);mdelay(1);
/*
		vRxWriteReg(0xC006120,0xd5);mdelay(1);
		vRxWriteReg(0xC006120,0xc0);mdelay(1);
		vRxWriteReg(0xC006120,0x4f);mdelay(1);
		vRxWriteReg(0xC006120,0x67);mdelay(1);
		vRxWriteReg(0xC006120,0x0b);mdelay(1);

		vRxWriteReg(0xC006120,0x8b);mdelay(1);
		vRxWriteReg(0xC006120,0xc2);mdelay(1);
		vRxWriteReg(0xC006120,0x2e);mdelay(1);
		vRxWriteReg(0xC006120,0xf4);mdelay(1);
		vRxWriteReg(0xC006120,0x95);mdelay(1);

		vRxWriteReg(0xC006120,0x18);mdelay(1);
		vRxWriteReg(0xC006120,0x83);mdelay(1);
		vRxWriteReg(0xC006120,0xed);mdelay(1);
		vRxWriteReg(0xC006120,0x67);mdelay(1);
		vRxWriteReg(0xC006120,0x39);mdelay(1);

		vRxWriteReg(0xC006120,0xa2);mdelay(1);
		vRxWriteReg(0xC006120,0xa4);mdelay(1);
		vRxWriteReg(0xC006120,0xaf);mdelay(1);
		vRxWriteReg(0xC006120,0x47);mdelay(1);
		vRxWriteReg(0xC006120,0xe1);mdelay(1);

		vRxWriteReg(0xC006120,0x88);mdelay(1);
		vRxWriteReg(0xC006120,0x9f);mdelay(1);
		vRxWriteReg(0xC006120,0x13);mdelay(1);
		vRxWriteReg(0xC006120,0xd4);mdelay(1);
		vRxWriteReg(0xC006120,0xe3);mdelay(1);

		vRxWriteReg(0xC006120,0x3c);mdelay(1);
		vRxWriteReg(0xC006120,0x10);mdelay(1);
		vRxWriteReg(0xC006120,0x67);mdelay(1);
		vRxWriteReg(0xC006120,0x5d);mdelay(1);
		vRxWriteReg(0xC006120,0xe3);mdelay(1);

		vRxWriteReg(0xC006120,0x61);mdelay(1);
		vRxWriteReg(0xC006120,0x1e);mdelay(1);
		vRxWriteReg(0xC006120,0xb5);mdelay(1);
		vRxWriteReg(0xC006120,0x4e);mdelay(1);
		vRxWriteReg(0xC006120,0x8b);mdelay(1);


		vRxWriteReg(0xC006120,0xc5);mdelay(1);
		vRxWriteReg(0xC006120,0x63);mdelay(1);
		vRxWriteReg(0xC006120,0x11);mdelay(1);
		vRxWriteReg(0xC006120,0x7e);mdelay(1);
		vRxWriteReg(0xC006120,0x2e);mdelay(1);

		vRxWriteReg(0xC006120,0xc4);mdelay(1);
		vRxWriteReg(0xC006120,0x8b);mdelay(1);
		vRxWriteReg(0xC006120,0xba);mdelay(1);
		vRxWriteReg(0xC006120,0xa7);mdelay(1);
		vRxWriteReg(0xC006120,0x45);mdelay(1);

		vRxWriteReg(0xC006120,0x86);mdelay(1);
		vRxWriteReg(0xC006120,0x13);mdelay(1);
		vRxWriteReg(0xC006120,0xf1);mdelay(1);
		vRxWriteReg(0xC006120,0x99);mdelay(1);
		vRxWriteReg(0xC006120,0x9b);mdelay(1);

		vRxWriteReg(0xC006120,0xa7);mdelay(1);
		vRxWriteReg(0xC006120,0x2e);mdelay(1);
		vRxWriteReg(0xC006120,0xb3);mdelay(1);
		vRxWriteReg(0xC006120,0x22);mdelay(1);
		vRxWriteReg(0xC006120,0x17);mdelay(1);

		vRxWriteReg(0xC006120,0xdb);mdelay(1);
		vRxWriteReg(0xC006120,0x8c);mdelay(1);
		vRxWriteReg(0xC006120,0xe4);mdelay(1);
		vRxWriteReg(0xC006120,0x1f);mdelay(1);
		vRxWriteReg(0xC006120,0x60);mdelay(1);

		vRxWriteReg(0xC006120,0xdb);mdelay(1);
		vRxWriteReg(0xC006120,0xaa);mdelay(1);
		vRxWriteReg(0xC006120,0x0a);mdelay(1);
		vRxWriteReg(0xC006120,0x95);mdelay(1);
		vRxWriteReg(0xC006120,0x4b);mdelay(1);

		vRxWriteReg(0xC006120,0x51);mdelay(1);
		vRxWriteReg(0xC006120,0xa2);mdelay(1);
		vRxWriteReg(0xC006120,0xcd);mdelay(1);
		vRxWriteReg(0xC006120,0x39);mdelay(1);
		vRxWriteReg(0xC006120,0xdc);mdelay(1);
*/
		vRxWriteReg(0xC00611c,0x02000e38);
		mdelay(1);
		vRxWriteReg(0xC00611c,0x02000a38);
		vRxWriteReg(0xC006034,0x00404b00);
		mdelay(1);
		vRxWriteReg(0xC006034,0x00004b00);

		vRxWriteReg(0xC006034,0x00104b00);
		mdelay(1);
		vRxWriteReg(0xC006034,0x00004b00);

		mdelay(500);

		u4Ind = vRxReadReg(0xC00602c);
		vRxWriteReg(0xC00602c,u4Ind|(1<<19)|(1<<21));

	}
	else if(bData == 4)
	{
		vLoadEdidFromCODE();
	}
}

extern HDMI_RESOLUTION_MODE_T _bVDITiming;
void _GetTimingInfomationNoreset(void)
{
	UINT16 u2ActiveWidth, u2ActiveHigh, u2V2ACTIVELINES;
	UINT16 u2VFRONTPORCH, u2HFRONTPORCH, u2HsyncActWidth;
	UINT16 u2HTotal, u2VTotal;
	UINT32 u4Data,bTimingSearch1;
	UINT8 u1HResStable, u1VResStable;
	UINT16 wDviHclk, wDviHtotal,wDviWidth = 0;
	UINT8 bDviVclk;
	UINT8 u1InputPClkX, u1VSYNCPOL, u1HSYNCPOL;
	BOOL fgInterlace = 0;
	UINT32 u4DeepColor=0;
	//UINT32 i;
	printk("Get TimingInfomation No reset\n");


	u4Data = vRxReadReg(0xC00604c);
	u2ActiveWidth= (UINT16)((u4Data >> 16)&0xfff);

	u4Data = vRxReadReg(0xC006050);
	u2ActiveHigh = 	(UINT16)(u4Data & 0xfff);
	u2V2ACTIVELINES =  (UINT16)((u4Data >>16) & 0x7f);
	u2VFRONTPORCH =  (UINT16)((u4Data >>24) & 0x3f);

	u4Data = vRxReadReg(0xC006038);
	u2HTotal = (UINT16)((u4Data >>16) & 0x1fff);
	u1HResStable = (u4Data & 0x80000000)? 1:0;

	u4Data = vRxReadReg(0xC00603c);
	u2VTotal = (UINT16)(u4Data & 0x0fff);
	u1VResStable = (u4Data & 0x00008000)? 1:0;

	u4Data = vRxReadReg(0xC006008);
	if((u4Data & 0x30)== 0x00)
	{
		u1InputPClkX = 1; //no repeat
	}
	else if((u4Data & 0x30)== 0x10)
	{
		u1InputPClkX = 2; //repeat 1 times, 2x Pclk
		u2HTotal <<=  1;
		u2ActiveWidth <<= 1;
	}
	else if((u4Data & 0x30)== 0x30)
	{
		u1InputPClkX = 4; //repeat 2 times, 4x Pclk
		u2HTotal <<=  2;
		u2ActiveWidth <<= 2;
	}

	u4Data = vRxReadReg(0xC006054);
	u1VSYNCPOL = (u4Data >> 9)&0x01;
	u1HSYNCPOL = (u4Data >> 8)&0x01;


	u4Data = vRxReadReg(0xC006058);
	u2HFRONTPORCH = (u4Data >> 8)&0xff;
	u2HsyncActWidth = ((u4Data >> 24)&0xff)|((vRxReadReg(0xC00605c)&0x3)<<8);

	fgInterlace = fgHDMIinterlaced();
	wDviHclk = dwHDMILineFreq();
	bDviVclk = bHDMIRefreshRate();
	wDviHtotal =  wHDMIHTotal();
	u4DeepColor = (vRxReadReg(0xC006004)>>28)&0x03;

	printk("**************Timing Information Start**************************\n");
	if(u1HResStable)
		printk("H resolution Stable\n");
	else
		printk("H resolution unStable\n");

	if(u1VResStable)
		printk("V resolution Stable\n");
	else
		printk("V resolution unStable\n");

	printk("H Active = %d\n", u2ActiveWidth);
	printk("V Active = %d\n", u2ActiveHigh);
	if(fgInterlace)
		printk("I/P = Interlace\n");
	else
		printk("I/P = Progressive\n");

	printk("H Total = %d\n", u2HTotal);
	printk("V Total = %d\n", u2VTotal);
	printk("H Freq = %d hz\n", wDviHclk*100);
	printk("V Freq = %d hz\n", bDviVclk);

	printk("Vsync Front Porch = %d \n", u2VFRONTPORCH);
	printk("Vsync width + back porch = %d\n", u2V2ACTIVELINES);
	printk("Hsync Front Porch = %d \n", u2HFRONTPORCH);
	printk("Hsync Active width = %d \n", u2HsyncActWidth);

	if(u1VSYNCPOL)
		printk("VSYNC Polarity is Positive\n");
	else
		printk("VSYNC Polarity is Negative\n");

	if(u1HSYNCPOL)
		printk("HSYNC Polarity is Positive\n");
	else
		printk("HSYNC Polarity is Negative\n");


	bTimingSearch1 = bDviStdTimingSearch(1, wDviHclk, bDviVclk, wDviHtotal,wDviWidth);
	printk("bTimingSearch1 = %d\n",bTimingSearch1);
	switch(bTimingSearch1)
	{
	case MODE_HDMI_640_480P:
		printk("Rx detect 640x480P \n");
		_bVDITiming = RES_2D_640x480HZ;
	break;

	case MODE_480P:
		printk("Rx detect 720x480P \n");
		_bVDITiming = RES_480P;
	break;

	case MODE_720p_60:
		printk("Rx detect 1280x720P60HZ \n");
		_bVDITiming = RES_720P60HZ;
	break;

	case MODE_1080i:
		printk("Rx detect 1920x1080(I)@60Hz \n");
		_bVDITiming = RES_1080I60HZ;
	break;

	case MODE_525I:
		printk("Rx detect 1440 x 480(I)@60Hz \n"); //Actually 1440x480i
		_bVDITiming = RES_480I;

	break;

	case MODE_525I_OVERSAMPLE:
		printk("Rx detect 2880x480(I)@60Hz \n"); //Actually 2880x480i
		_bVDITiming = RES_480I_2880;
	break;

	case MODE_480P_OVERSAMPLE:
		printk("Rx detect 1440x480(P)@60Hz \n");
		_bVDITiming = RES_480P_1440;
	break;

	case MODE_1080p_60:
		printk("Rx detect 1920x1080@60Hz \n");
		_bVDITiming = RES_1080P60HZ;
	break;

	case MODE_576P:
		printk("Rx detect 720x576@50Hz \n");
		_bVDITiming = RES_576P;
	break;

	case MODE_720p_50:
		printk("Rx detect 1280x720@50Hz \n");
		_bVDITiming = RES_720P50HZ;
	break;

	case MODE_1080i_50:
		printk("Rx detect 1920x1080(I)@50Hz \n");
		_bVDITiming = RES_1080I50HZ;
	break;

	case MODE_625I:
		printk("Rx detect 1440x576(I)@50Hz \n");
		_bVDITiming = RES_576I;
	break;

	case MODE_625I_OVERSAMPLE:
		printk("Rx detect 2880x576(I)@50Hz \n");
		_bVDITiming = RES_576I_2880;
	break;

	case MODE_576P_OVERSAMPLE:
		printk("Rx detect 1440x576(P)@50Hz \n");
		_bVDITiming = RES_576P_1440;
	break;

	case MODE_1080p_50:
		printk("Rx detect 1920x1080P@50Hz \n");
		_bVDITiming = RES_1080P50HZ;
	break;

	case MODE_1080p_24:
		printk("Rx detect 1920x1080@24Hz \n");
		_bVDITiming = RES_1080P23_976HZ;
	break;

	case MODE_1080p_25:
		printk("Rx detect 1920x1080@25Hz \n");
		_bVDITiming = RES_1080P25HZ;
	break;

	case MODE_1080p_30:
		printk("Rx detect 1920x1080@30Hz \n");
		_bVDITiming = RES_1080P30HZ;
	break;

	case MODE_3D_480P60HZ:
		printk("MODE_3D_480P60HZ \n");
		_bVDITiming = RES_3D_480P60HZ;
	break;

	case MODE_3D_1080I_60_FRAMEPACKING:
		printk("MODE_3D_1080I_60_FRAMEPACKING \n");
		_bVDITiming = RES_3D_1080I60HZ;
	break;

	default:
		_bVDITiming=vConvertHdmiRXResToVDORes(bTimingSearch1);
		//printk("Rx detect bTimingSearch1 %s = %d \n", szRxResStr0[bTimingSearch1],_bVDITiming);
	break;
	}

	if(u4DeepColor == 0)
	  printk("Rx detect 8 BIT \n");
	else if(u4DeepColor == 1)
	  printk("Rx detect 10 BIT \n");
	else if(u4DeepColor == 2)
	  printk("Rx detect 12 BIT \n");
	else if(u4DeepColor == 3)
	  printk("Rx detect 16 BIT \n");

	if(vRxReadReg(0xC006134)&0x01)
	printk("HDMI Mode \n");
	else
	printk("DVI Mode \n");


	printk("AUDIO PACKET layer =0x%x \n",(vRxReadReg(0xC006134)>>3)&0x01);

	printk("AUDIO PACKET valid =0x%x \n",(vRxReadReg(0xC006294)&0x0F));
		{
	printk("SPDIF channel status BYTE1~BYTE5 =0x%x,0x%x,0x%x,0x%x,0x%x \n",((vRxReadReg(0xC006128)>>16)&0xFF),
		((vRxReadReg(0xC006128)>>24)&0xFF),((vRxReadReg(0xC00612C))&0xFF),((vRxReadReg(0xC006130))&0xFF),((vRxReadReg(0xC006130)>>8)&0xFF));
		}
	printk("**************Timing Information End****************************\n");
    //while(1){
		//_HdmiRxCrcCheck(100);
		//mdelay(50);
	//}
	return;
}

static void HalGetTimingInfomation(void)
{
    // PHY INIT HDMI RX ANALOG AND DIGITAL SETTING 
	HalHdmiRxPHYInit();	
	// GET HDMI RX TIMING INFOMATION 
	_GetTimingInfomationNoreset();
}

void _GetTimingInfomation(void)
{
	HalGetTimingInfomation();
}

void _HdmiRxI2SBypass(void)
{
	HalHdmiRxI2SBypass();
}

void HalHdmiRxDSD(void)
{
	  printk("HdmiRxDSD\n");
	  vRxWriteReg(0xC006100,0x00f20002);
	  vRxWriteRegMsk(0xC006270,(0x01<<6)|(0x01<<19),(0x01<<6)|(0x01<<19));
	  vRxWriteRegMsk(0xC006084,(0x01<<8)|(0x01<<22),(0x01<<8)|(0x01<<22));
}

void _HdmiRxDSD(void)
{
	HalHdmiRxDSD();
}

static INT32 _HdmiRxDSDI2S(INT32 i4Argc, const CHAR **szArgv)
{
	HalHdmiRxPHYInit();
	HalHdmiRxHBR();

	HalGetTimingInfomation();
	HalHdmiRxAudioReset();
	HalHdmiRxDSD();
	HalHdmiRxDSDBypass();
}

void HalHdmiRxHBR(void)
{
	printk("HdmiRxHBR\n");
	vRxWriteReg(0xC006100,0x000e0002);
	vRxWriteRegMsk(0xC006128,1<<10,1<<10); // I2S MODE
	vRxWriteRegMsk(0xC006270,0,0x07<<17);//HBR DSD HBR_I2S_CLK_SEL,HBR_SEL,DSD_SEL. USE I2S ,0
	vRxWriteRegMsk(0xC006084,(0x01<<9)|(0x01<<11),(0x01<<9)|(0x01<<11)|(0x01<<16)|(0x01<<17));//bit9 TT0_HBR_EN
	vRxWriteRegMsk(0xC006124,(0x01<<21),(0x01<<16)|(0x01<<17)|(0x01<<19)|(0x01<<21));
	vRxWriteRegMsk(0xC006264,0,(0x01<<16));
	vRxWriteRegMsk(0xC00626c,0xB2<<24,(0xff<<24));
    HalHdmiRxAudioReset();
}

void _HdmiRxHBR(void)
{
	HalHdmiRxHBR();
}
void _HdmiRxDSDBypass(void)
{
	HalHdmiRxDSDBypass();
}

void _HdmiRxHBRSPDIFBYPASS(void)
{
	//HalHdmiRxPHYInit();
	//HalHdmiRxHBR();
	//HalGetTimingInfomation();
	//HalHdmiRxAudioReset();
	printk("_HdmiRxHBRSPDIFBYPASS\n");
	vRxWriteRegMsk(0xc006088,(1<<12)|(1<<13),(1<<12)|(1<<13)|(1<<14)); //for audio unflow,overflow
	vRxWriteReg(0xc006100,0x00000000); //audio clk auto mode
	HalHdmiRxHBR(); //HBR Ralated SETTING
	vRxWriteReg(0xc006084,0x00d70a00);//enable spdif 8 ch relaterd 
	vRxWriteReg(0xc006270,0x5c);
	vRxWriteReg(0xc006100,0x00090002);
	//HBR BYPASS SPDIF
	HalHdmiRxI2SBypass();
	HalHdmirxSpdifBypass();
	vRxWriteRegMsk(0xc006034,1<<30,1<<30); //ECO for spdif channel status shift one bit
}

void rxchangedepth(unsigned char depthstatus,unsigned char depthvalue)
{

    if(depthstatus == 0) {
      printk("moddep w 0x%x\n", depthvalue);
	  if(depthvalue>7) {
	  printk("error: please input the value 0-7  \n");
	  return 0;
	  }
      vModifyBstatusDepth(depthvalue);
    }

    if(depthstatus == 1)
    {
      vShowRxHDCPBstatus();
    }

    if(depthstatus == 2)
    {
      vRecoverBstatusDepth();
    }

    return 0;

}
