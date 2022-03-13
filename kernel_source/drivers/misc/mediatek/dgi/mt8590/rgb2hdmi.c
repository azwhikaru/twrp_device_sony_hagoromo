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
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/jiffies.h>
#include "typedef.h"
#include "dgi_hw.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"
#include "hdmi_rx_ctrl.h"
#include "hdmi_rx_hw.h"
#include "hdmitable.h"
#include "hdmictrl.h"


#define MAX_RES 63//19

static const UINT32 HDMI_SCL_HDMISYNC[MAX_RES][1] = {
 {0x037C0306},//480i //SD H_Polar, v_polar is low
 {0x037E0003},//576i	
 {0x033E060C},//480p@27M
 {0x03400005},//576p@27M
 {0x037C060C},//480p@54
 {0x03800005},//576p@54 //Vsync end should be changed from 3 to 5
 {0x03F8060C},//480p@108
 {0x13000005},//576p@108 //hsync width is 256, bit how to set?? -->0x100
 {0x00280005},//720p@60hz
 {0x00280005},//720p@50Hz
 {0x002C0005},//1080i@60hz
 {0x002C0005},//1080i@50Hz
 {0x002C0005},//1080p@60hz
 {0x002C0005},//1080p@50Hz
 {0x002C0005},//1080p@30hz 	
 {0x002C0005},//1080p@25Hz
 {0x03f80306},//480i2880 
 {0x03fc0003},//576i2880	
 {0x002C0005},//1080p@24Hz
 {0x002C0005},//1080p@23.97Hz
 {0x002C0005}//1080p@29.97Hz
 };
 
 static const UINT32 HDMI_DATA_ENABLE[MAX_RES][3] = {
  {0x011D020C, 0x00160105, 0x00EF068E},//480i
  {0x0150026F, 0x00170136, 0x010906A8},//576i	 
  {0x002B020A, 0x002B020A, 0x007B034A},//480p@27M
  {0x002D026C, 0x002D026C, 0x00850354},//576p@27M
  {0x002B020A, 0x002B020A, 0x00F50694},//480p@54
  {0x002D026C, 0x002D026C, 0x010906A8},//576p@54
  {0x002B020A, 0x002B020A, 0x01E90D28},//480p@108M
  {0x002D026C, 0x002D026C, 0x02110D50},//576p@108M
  {0x001A02E9, 0x001A02E9, 0x01050604},//720p@60Hz
  {0x001A02E9, 0x001A02E9, 0x01050604},//720p@50Hz
  {0x02480463, 0x00150230, 0x00C10840},//1080i@60Hz
  {0x02480463, 0x00150230, 0x00C10840},//1080i@50Hz
  {0x002A0461, 0x002A0461, 0x00C10840},//1080p@60Hz
  {0x002A0461, 0x002A0461, 0x00C10840},//1080p@50Hz
  {0x002A0461, 0x002A0461, 0x00C10840},//1080p@30Hz
  {0x002A0461, 0x002A0461, 0x00C10840},//1080p@25Hz
  {0x011d020C, 0x00160105, 0x01DD0D1c},//480i2880
  {0x0150026F, 0x00170136, 0x02110d50},//576i2880
  {0x002A0461, 0x002A0461, 0x00C10840},//1080p@24Hz
  {0x002A0461, 0x002A0461, 0x00C10840},//1080p@23.976Hz
  {0x002A0461, 0x002A0461, 0x00C10840}//1080p@29.97Hz
  
  };
 
 static const UINT32 HDMI_SYNC_DELAY[MAX_RES][1] = {
  {0x00010018},//480i
  {0x00010014},//{0x0000301B},//576i 
  {0x00170020},//480p@27M
  {0x00090020},//576p@27M
  {0x00010025},//480p54
  {0x00010025},//576p54
  {0x00010025},//480p108
  {0x00010025},//576p108
  {0x00090020},//720p
  {0x00090020},//720p@50Hz
  {0x0009001F},//1080i@60hz
  {0x0009001F},//1080i@50Hz//zhiqiang modify
  {0x0009001F},//1080p60
  {0x0009001F},//1080p@50Hz 
  {0x0001002B},//1080p30
  {0x0001002B},//1080p@25Hz 
  {0x00010009},//480i2880 temply
  {0x00030005},//576i2880 temply
  {0x0001002B},//1080p@24Hz 
  {0x0001002B},//1080p@23.97Hz 
  {0x0001002B},//1080p@29.97Hz 
  {0x0009001F}, //RES_3D_1080P23HZ
  {0x0009001F}, //RES_3D_1080P24HZ
  {0x0009001F}, //RES_3D_720P60HZ
  {0x0009001F}, //RES_3D_720P50HZ
  {0x0001002B}, //RES_3D_720P30HZ
  {0x0001002B}, //RES_3D_720P25HZ
  {0x0001002B}, //RES_3D_576P50HZ
  {0x0001002B}, //RES_3D_480P60HZ
  {0x0009001F}, //RES_3D_1080I60HZ
  {0x0009001F}, //RES_3D_1080I50HZ
  {0x0001002B}, //RES_3D_1080I30HZ
  {0x0001002B}, //RES_3D_1080I25HZ
  {0x0001002B}, //33, 576i100hz
  {0x0001002B}, //34, 480i120hz
  {0x0001002B}, //35, 576i100hz
  {0x0001002B}, //36, 480i120hz
  {0x0001002B}, //37, 480i60hz
  {0x0001002B}, //38, 576i50hz
  {0x0001002B},//39
  {0x0001002B}, //40 //Total: 1344x625, Act: 1024x600, Frm: 60Hz, Clk: 50.4MHz
  {0x0001002B},  //41
  {0x0001002B},  //42
  {0x0001002B},//43
  {0x0001002B},//44
  {0x0001002B}, //45
  {0x0001002B},//46
  {0x0001002B}, //47
  {0x0001002B}, //48
  {0x0001002B}, //49
  {0x0001002B}, //50
  {0x0001002B},  //51
  {0x0001002B},  //52
  {0x0001002B}, // 53
  {0x0001002B}, // 54
  {0x0001002B}, // 55
  {0x0001002B}, // 56
  {0x0001002B},// 57
  {0x0001002B},//58
  {0x0001002B},//RES_3D_1080P30HZ,//59
  {0x0001002B},//RES_3D_1080P29HZ,//60
  {0x0001002B},//RES_3D_1080P25HZ,//61
  {0x0001002B}, //62
  {0x0001002B} //63
 };

void vSetHDMIVideoPixelRepeat(BYTE bColorSpace, BOOL fgPixRep, BOOL fgYuv422Delay)
{
	UINT32 ui4Data;
	UINT32 ui4DataDelay=0;
	ui4Data=dReadHdmiRGB(HDMI_RGB_CTRL);
	ui4Data&=~(DOUBLE_EN|REPEAT_EN);
	ui4Data|= (RGB2HDMI_ON|UV_OFFSET|ADJ_SYNC_EN|YCBCR422_CBCR_INV);
	ui4DataDelay |= HSYNC_DELAY;

	if(bColorSpace == HDMI_YCBCR_444)
	{
		ui4Data &=~(YUV_422|RGB_MOD);
		if(fgPixRep)
		{
			ui4Data |= REPEAT_EN; //enable 4:4:4	repeater
			ui4DataDelay &= ~HSYNC_DELAY;
		}
	}
	else  if(bColorSpace ==  HDMI_YCBCR_422)
	{
		ui4Data &=~(RGB_MOD);
		ui4Data |=YUV_422;

		if(fgPixRep)
		{

			if(fgYuv422Delay)
			{
				ui4Data |= (DOUBLE_EN|REPEAT_EN);
				ui4DataDelay |= (DOUBLE422_DELAY<<1);
				ui4DataDelay &= ~HSYNC_DELAY;
			}
			else
			{
				ui4Data |= (DOUBLE_EN|REPEAT_EN);
				ui4Data &= ~(YCBCR422_CBCR_INV);
				ui4DataDelay |= (DOUBLE422_DELAY);
				ui4DataDelay &= ~HSYNC_DELAY;
			}
		}

	}
	else
	{
		ui4Data &=~(YUV_422);
		ui4Data |= RGB_MOD;
		if(fgPixRep)
		{
			ui4Data |= REPEAT_EN; //enable 4:4:4	repeater
			ui4DataDelay &= ~HSYNC_DELAY;
		}
	}
	vWriteHdmiRGB(HDMI_RGB_CTRL, ui4Data);
	vWriteHdmiRGB(HDMI_RGB2HDMI_C0, ui4DataDelay);
}

void vSetRGB2Hdmi54MClk(BYTE bOn)
{
	UINT32 ui4Data;
	
	ui4Data=dReadHdmiRGB(HDMI_RGB_CTRL);

	if(bOn == TRUE)
	{
		ui4Data|=PRGS_CLK54_EN;	//enable P54M
		ui4Data &=~PRGS_CLK108_EN;
	}
	else
	{
		ui4Data &=~PRGS_CLK54_EN;
		ui4Data &=~PRGS_CLK108_EN;
		ui4Data &=~REP_4PXL;
	}

	vWriteHdmiRGB(HDMI_RGB_CTRL, ui4Data);

}

void vSetRGB2Hdmi108MClk(BYTE bOn)
{
  UINT32 ui4Data;
  ui4Data=dReadHdmiRGB(HDMI_RGB_CTRL);

  if(bOn == TRUE)
  {
     ui4Data &=~PRGS_CLK54_EN;	//disable P54M
     ui4Data |=PRGS_CLK108_EN;
    ui4Data |=REP_4PXL;
  }
  else
  {
    ui4Data &=~PRGS_CLK54_EN;
    ui4Data &=~PRGS_CLK108_EN;
    ui4Data &=~REP_4PXL;
  }
   vWriteHdmiRGB(HDMI_RGB_CTRL, ui4Data);

}

void vSetRGB2HdmiYUV709(BYTE bOn)
{
	if(bOn == TRUE)
		vWriteHdmiRGBMsk(HDMI_RGB2HDMI_64,YCBCR2RGB_709_ORG,YCBCR2RGB_3X3_BY_COEFFICIENTS);
	else
		vWriteHdmiRGBMsk(HDMI_RGB2HDMI_64,0,YCBCR2RGB_3X3_BY_COEFFICIENTS);
}

void vSetHdmiFullRGB(BYTE bOn)
{
	UINT32 ui4Data;

	ui4Data=dReadHdmiRGB(HDMI_RGB_CTRL);

	if(bOn == TRUE)
	{
		ui4Data |=  RGB_FULL_RANGE;
	}
	else
	{
		ui4Data &=  ~RGB_FULL_RANGE;
	}
	vWriteHdmiRGB(HDMI_RGB_CTRL, ui4Data);
}

void vSetHdmiFullYUV(BYTE bOn)
{
	UINT32 ui4Data;

	ui4Data=dReadHdmiRGB(HDMI_RGB_CTRL);

	if(bOn == TRUE)
	{
		ui4Data |=  YVU_FULL;
	}
	else
	{
		ui4Data &=  ~YVU_FULL;
	}

	vWriteHdmiRGB(HDMI_RGB_CTRL, ui4Data);
}

void vHalSetHdmiLimitRange(BYTE bOn, UINT16 u2TopLimit, UINT16 u1BottomLimit)
{
	vWriteHdmiRGB(HDMI_RGB2HDMI_50,(u2TopLimit<<16)|u1BottomLimit);
	if(bOn)
	{
		vWriteHdmiRGBMsk(HDMI_RGB_CTRL, LMT_EN, LMT_EN);
	}
	else
	{
		vWriteHdmiRGBMsk(HDMI_RGB_CTRL, 0, LMT_EN);
	}
}

void vSetHDMI640x480PDataEnable(void)
{
	//Set Vsync start, hsync width, v_polar, h_polar

	vWriteHdmiRGB(HDMI_RGB_TIME0, 0x03600002);
	//Write hdmi data enable
	vWriteHdmiRGBMsk(HDMI_RGB_TIME1, 0x00910310, 0xFFFFFFF); //0xABC~0xABF
	vWriteHdmiRGBMsk(HDMI_RGB_TIME2, 0x00240203, 0x7FFFFFF); //0xAC0~0xAC3
	vWriteHdmiRGBMsk(HDMI_RGB_TIME3, 0x00240203, 0x7FFFFFF); //0xAC4~0xAC7
}

void v85553DInterlaceDataEnable(UINT32 u4Data)
{
	vWriteHdmiRGB(HDMI_RGB_TIME2, u4Data);
}

void v85553DProgressDataEnable(UINT32 u4Data)
{
	vWriteHdmiRGB(HDMI_RGB_TIME2, u4Data);
}

void vSetHDMIDataEnable(BYTE bResIndex)
{
	printk("[RGB2HDMI] set input res = %d\n", bResIndex);
	//Set Vsync start, hsync width, v_polar, h_polar
	if(bResIndex < MAX_RES)
	{
		vWriteHdmiRGB(HDMI_RGB_TIME0, HDMI_SCL_HDMISYNC[bResIndex][0]);
		//Write hdmi data enable
		vWriteHdmiRGBMsk(HDMI_RGB_TIME1, HDMI_DATA_ENABLE[bResIndex][2], 0x9FFF1FFF); //0xABC~0xABF
		vWriteHdmiRGBMsk(HDMI_RGB_TIME2, HDMI_DATA_ENABLE[bResIndex][1], 0x7FFFFFF); //0xAC0~0xAC3
		vWriteHdmiRGBMsk(HDMI_RGB_TIME3, HDMI_DATA_ENABLE[bResIndex][0], 0x7FFFFFF); //0xAC4~0xAC7
	}
}
void vSetHDMI3DDataEnable(BYTE bResIndex)
{
	BYTE bResTableIndx=0;

    if(bResIndex==RES_2D_480I60HZ)
    {
        bResTableIndx =RES_480I-RES_480I;
  	 	vSetHDMIDataEnable(bResTableIndx);
    }

    if(bResIndex==RES_2D_576I50HZ)
    {
        bResTableIndx =RES_576I-RES_480I;
  	 	vSetHDMIDataEnable(bResTableIndx);
    }

    if(bResIndex==RES_720P23HZ||bResIndex==RES_720P24HZ||bResIndex==RES_720P30HZ)
    {
        bResTableIndx = RES_720P60HZ-RES_480I;
        vSetHDMIDataEnable(bResTableIndx);
    }
    else if(bResIndex==RES_720P25HZ)
    {
        bResTableIndx = RES_720P50HZ-RES_480I;
        vSetHDMIDataEnable(bResTableIndx);
    }

    if(fgIs3DOutput(bResIndex))
    {
		if((bResIndex==RES_3D_480I60HZ)||(bResIndex==RES_3D_480I30HZ))
		{
			bResTableIndx =RES_480I-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_576I50HZ)||(bResIndex==RES_3D_576I25HZ))
		{
			bResTableIndx =RES_576I-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if(bResIndex==RES_3D_480P60HZ)
		{
			bResTableIndx =RES_480P-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DProgressDataEnable(0x002B0417);
		}
		else if(bResIndex==RES_3D_576P50HZ)
		{
			bResTableIndx =RES_576P-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DProgressDataEnable(0x002D0479);
		}
		else if((bResIndex==RES_3D_720P60HZ)||(bResIndex==RES_3D_720P30HZ))
		{
			bResTableIndx =RES_720P60HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DProgressDataEnable(0x001A05D7);
		}
		else if((bResIndex==RES_3D_720P50HZ)||(bResIndex==RES_3D_720P25HZ))
		{
			bResTableIndx =RES_720P50HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DProgressDataEnable(0x001A05D7);
		}
		else if((bResIndex==RES_3D_1080I60HZ)||(bResIndex==RES_3D_1080I30HZ))
		{
			bResTableIndx =RES_1080I60HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DInterlaceDataEnable(0x001508C8);
		}
		else if((bResIndex==RES_3D_1080I50HZ)||(bResIndex==RES_3D_1080I25HZ))
		{
			bResTableIndx =RES_1080I50HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DInterlaceDataEnable(0x001508C8);
		}
		else if((bResIndex==RES_3D_1080P24HZ)||(bResIndex==RES_3D_1080P23HZ))
		{
			bResTableIndx =RES_1080P24HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DProgressDataEnable(0x002A08C6);
		}
		else if((bResIndex==RES_3D_720P23HZ)||(bResIndex==RES_3D_720P24HZ))
		{
			bResTableIndx =RES_720P60HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DProgressDataEnable(0x001A05D7);
		}
		else if(bResIndex==RES_3D_1080P30HZ)
		{
			bResTableIndx = RES_1080P30HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DProgressDataEnable(0x002A08C6);
		}
		else if(bResIndex==RES_3D_1080P29HZ)
		{
			bResTableIndx = RES_1080P29_97HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DProgressDataEnable(0x002A08C6);
		}
		else if(bResIndex==RES_3D_1080P25HZ)
		{
			bResTableIndx = RES_1080P25HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
			v85553DProgressDataEnable(0x002A08C6);
		}
		else if((bResIndex==RES_3D_480I60HZ_TB)||(bResIndex==RES_3D_480I30HZ_TB)||
				(bResIndex==RES_3D_480I60HZ_SBS_HALF)||(bResIndex==RES_3D_480I30HZ_SBS_HALF))
		{
			bResTableIndx = RES_480I-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_576I50HZ_TB)||(bResIndex==RES_3D_576I25HZ_TB)||
				(bResIndex==RES_3D_576I50HZ_SBS_HALF)||(bResIndex==RES_3D_576I25HZ_SBS_HALF))
		{
			bResTableIndx = RES_576I-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_480P60HZ_TB)||(bResIndex==RES_3D_480P60HZ_SBS_HALF))
		{
			bResTableIndx = RES_480P-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_576P50HZ_TB)||(bResIndex==RES_3D_576P50HZ_SBS_HALF))
		{
			bResTableIndx = RES_576P-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_720P60HZ_TB)||(bResIndex==RES_3D_720P30HZ_TB)||
				(bResIndex==RES_3D_720P60HZ_SBS_HALF)||(bResIndex==RES_3D_720P30HZ_SBS_HALF))
		{
			bResTableIndx = RES_720P60HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_720P50HZ_TB)||(bResIndex==RES_3D_720P25HZ_TB)||
				(bResIndex==RES_3D_720P50HZ_SBS_HALF)||(bResIndex==RES_3D_720P25HZ_SBS_HALF))
		{
			bResTableIndx = RES_720P50HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_720P23HZ_TB)||(bResIndex==RES_3D_720P24HZ_TB)||
				(bResIndex==RES_3D_720P23HZ_SBS_HALF)||(bResIndex==RES_3D_720P24HZ_SBS_HALF))
		{
			bResTableIndx =RES_720P60HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_1080I60HZ_TB)||(bResIndex==RES_3D_1080I30HZ_TB)||
				(bResIndex==RES_3D_1080I60HZ_SBS_HALF)||(bResIndex==RES_3D_1080I30HZ_SBS_HALF))
		{
			bResTableIndx = RES_1080I60HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_1080I50HZ_TB)||(bResIndex==RES_3D_1080I25HZ_TB)||
				(bResIndex==RES_3D_1080I50HZ_SBS_HALF)||(bResIndex==RES_3D_1080I25HZ_SBS_HALF))
		{
			bResTableIndx = RES_1080I50HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_1080P23HZ_TB)||(bResIndex==RES_3D_1080P23HZ_SBS_HALF))
		{
			bResTableIndx = RES_1080P23_976HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_1080P24HZ_TB)||(bResIndex==RES_3D_1080P24HZ_SBS_HALF))
		{
			bResTableIndx = RES_1080P24HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_1080P25HZ_TB)||(bResIndex==RES_3D_1080P25HZ_SBS_HALF))
		{
			bResTableIndx = RES_1080P25HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_1080P29HZ_TB)||(bResIndex==RES_3D_1080P29HZ_SBS_HALF))
		{
			bResTableIndx = RES_1080P29_97HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_1080P30HZ_TB)||(bResIndex==RES_3D_1080P30HZ_SBS_HALF))
		{
			bResTableIndx = RES_1080P30HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_1080P50HZ_TB)||(bResIndex==RES_3D_1080P50HZ_SBS_HALF))
		{
			bResTableIndx = RES_1080P50HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}
		else if((bResIndex==RES_3D_1080P60HZ_TB)||(bResIndex==RES_3D_1080P60HZ_SBS_HALF))
		{
			bResTableIndx = RES_1080P60HZ-RES_480I;
			vSetHDMIDataEnable(bResTableIndx);
		}

	}
}
void vSetRGBYCbCrDelay(BYTE bRDelay,BYTE bGDelay,BYTE bBDelay,BYTE bYDelay,BYTE bCbDelay,BYTE bCrDelay)
{
	vWriteHdmiRGB(HDMI_RGB2HDMI_4O,((bRDelay&0x07))|((bGDelay&0x07)<<4)|((bBDelay&0x07)<<8)|((bYDelay&0x07)<<12)|((bCbDelay&0x07)<<16)|((bCrDelay&0x07)<<20));
}

BOOL fgVideoisHD(BYTE bRes)
{
	if((bRes == RES_480I)||(bRes == RES_576I)||(bRes == RES_480P)||(bRes == RES_576P)||
	  (bRes == RES_480P_1440)||(bRes == RES_576P_1440)||(bRes == RES_480P_2880)||(bRes == RES_576P_2880)||
	  (bRes == RES_480I_2880)||(bRes == RES_576I_2880)||
	  (bRes == RES_2D_480I60HZ)||(bRes == RES_2D_576I50HZ)||(bRes == RES_2D_640x480HZ)||
	  (bRes == RES_3D_576P50HZ)||(bRes == RES_3D_576I50HZ)||(bRes == RES_3D_576I25HZ)||
	  (bRes == RES_3D_480P60HZ)||(bRes == RES_3D_480I60HZ)||(bRes == RES_3D_480I30HZ)||
	  (bRes == RES_3D_576P50HZ_TB)||(bRes == RES_3D_576I50HZ_TB)||(bRes == RES_3D_576I25HZ_TB)||
	  (bRes == RES_3D_480P60HZ_TB)||(bRes == RES_3D_480I60HZ_TB)||(bRes == RES_3D_480I30HZ_TB)||
	  (bRes == RES_3D_576P50HZ_SBS_HALF)||(bRes == RES_3D_576I50HZ_SBS_HALF)||(bRes == RES_3D_576I25HZ_SBS_HALF)||
	  (bRes == RES_3D_480P60HZ_SBS_HALF)||(bRes == RES_3D_480I60HZ_SBS_HALF)||(bRes == RES_3D_480I30HZ_SBS_HALF))
		return FALSE;
	else
		return TRUE;

}

void rgb2hdmi_setting_res(UINT8 ui1Res, UINT8 ui1ColorSpace)
{
	BYTE bColorSpace;
	BOOL fgPixRep, fgYuv422Delay;
	fgPixRep = FALSE;
	fgYuv422Delay = FALSE;

	if(fgHdmiRepeaterIsBypassMode())
	{
		if(bHDMIInputType()!=HDMIRX_RGB)
		{//YUV
			if(bHDMI422Input() == HDMIRX_YCBCR_444) //YUV444 input
			{
				ui1ColorSpace = HDMI_YCBCR_444;
				printk("RGB2HDMI HDMI_YCBCR_444\n");
			}
			else//YUV4222 input
			{
				ui1ColorSpace = HDMI_YCBCR_422;
				printk("RGB2HDMI HDMI_YCBCR_422\n");
			}
			}
			else
			{ //RGB
				ui1ColorSpace = HDMI_RGB_FULL;
				printk("RGB2HDMI HDMI_RGB\n");
			}
	}
	if((ui1ColorSpace == HDMI_YCBCR_444)||(ui1ColorSpace == HDMI_XV_YCC)) //YCbCr 4:4:4
	{
		bColorSpace = HDMI_YCBCR_444;

		if((ui1Res == RES_480I)||(ui1Res == RES_576I)||(ui1Res == RES_480P_1440)||(ui1Res == RES_576P_1440)||(ui1Res == RES_480P_2880)||(ui1Res == RES_576P_2880))
		{
			fgPixRep = TRUE; //enable 4:4:4	repeater
		}
	}
	else if(ui1ColorSpace == HDMI_YCBCR_422)//YCbCr 4:2:2
	{
		bColorSpace = HDMI_YCBCR_422;
		if	((ui1Res == RES_480I) ||(ui1Res == RES_576I))
		{
			fgPixRep = TRUE; //enable 4:2:2	repeater
		}
		if((ui1Res == RES_480P_1440) ||(ui1Res == RES_576P_1440)||(ui1Res == RES_480P_2880)||(ui1Res == RES_576P_2880))
		{
			fgPixRep = TRUE; //enable 4:2:2	repeater
			fgYuv422Delay = TRUE;
		}
	}
	else
	{
		bColorSpace = HDMI_RGB_FULL;
		printk("RGB2HDMI HDMI_RGB_FULL\n");
		if((ui1Res == RES_480I)||(ui1Res == RES_576I)||(ui1Res == RES_480P_1440)||(ui1Res == RES_576P_1440)||(ui1Res == RES_480P_2880)||(ui1Res == RES_576P_2880))
		{
			fgPixRep = TRUE; //enable 4:4:4	repeater
		}
	}
	vSetHDMIVideoPixelRepeat(bColorSpace, fgPixRep, fgYuv422Delay);
	if((ui1Res == RES_480P_1440)||(ui1Res == RES_576P_1440))
	{
		vSetRGB2Hdmi54MClk(TRUE);	//enable P54M, for 1080P HD-JPEG it should be enabled
	}
	else if((ui1Res == RES_480P_2880)||(ui1Res == RES_576P_2880))
	{
		vSetRGB2Hdmi108MClk(TRUE);
	}
	else
	{
		vSetRGB2Hdmi54MClk(FALSE);	//disable P54M
		vSetRGB2Hdmi108MClk(FALSE);
	}

	if(fgVideoisHD(ui1Res))
	{
		vSetRGB2HdmiYUV709(TRUE);
	}
	else
	{
		vSetRGB2HdmiYUV709(FALSE);
	}
	vSetHdmiFullRGB(TRUE);
	vSetHdmiFullYUV(TRUE);
	if(fgHdmiRepeaterIsBypassMode())
	{
		if(ui1ColorSpace == HDMI_RGB_FULL)
		{
			vWriteHdmiRGBMsk(HDMI_RGB2HDMI_60,YCBCR2RGB_3X3_ON,YCBCR2RGB_3X3_ON); //Y offset 16
			if(fgVideoisHD(ui1Res)) //709
				vWriteHdmiRGBMsk(HDMI_RGB2HDMI_64,YCBCR2RGB_709_NEW2,YCBCR2RGB_3X3_BY_COEFFICIENTS); //limit to 16 ~ 235
			else
				vWriteHdmiRGBMsk(HDMI_RGB2HDMI_64,YCBCR2RGB_601_NEW2,YCBCR2RGB_3X3_BY_COEFFICIENTS); //limit to 16 ~ 235
			//YCbCr to YCbCr enhance
			vHalSetHdmiLimitRange(TRUE, 0xfeff, 0x100);//16bit

		}
		else if(ui1ColorSpace == HDMI_RGB)
		{
			vWriteHdmiRGBMsk(HDMI_RGB2HDMI_60,YCBCR2RGB_3X3_ON,YCBCR2RGB_3X3_ON); //Y offset 16
			if(fgVideoisHD(ui1Res)) //709
				vWriteHdmiRGBMsk(HDMI_RGB2HDMI_64,YCBCR2RGB_709_ORG,YCBCR2RGB_3X3_BY_COEFFICIENTS); //limit to 16 ~ 235
			else
				vWriteHdmiRGBMsk(HDMI_RGB2HDMI_64,YCBCR2RGB_601_NEW2,YCBCR2RGB_3X3_BY_COEFFICIENTS); //limit to 16 ~ 235
			vHalSetHdmiLimitRange(FALSE, 0xebff, 0x0000);//16bit
		}
		else
		{
			vWriteHdmiRGBMsk(HDMI_RGB2HDMI_60,0,YCBCR2RGB_3X3_ON); //Y offset 16
			vWriteHdmiRGBMsk(HDMI_RGB2HDMI_64,YCBCR2RGB_601_ORG,YCBCR2RGB_3X3_BY_COEFFICIENTS); //limit to 16 ~ 235
			vHalSetHdmiLimitRange(FALSE, 0xebff, 0x0000);//16bit
		}
	}
	if((ui1Res==RES_1080P60HZ)||(ui1Res==RES_1080P50HZ)||(ui1Res==RES_1080P24HZ)||(ui1Res==RES_1080P25HZ)||(ui1Res==RES_1080P30HZ)||(ui1Res==RES_1080P23_976HZ))
	{
		if(ui1ColorSpace == HDMI_YCBCR_422)
		{
			vSetRGBYCbCrDelay(0,0,0,1,0,1);
		}
		else
		{
			vSetRGBYCbCrDelay(0,0,0,1,1,1);
		}
	}
	else if((ui1Res==RES_1080I60HZ)||(ui1Res==RES_1080I50HZ))
	{
		if(ui1ColorSpace == HDMI_YCBCR_422)
		{
			vSetRGBYCbCrDelay(0,0,0,1,0,1);
		}
		else
		{
			vSetRGBYCbCrDelay(0,0,0,1,1,1);
		}
	}
	else
	{
		if(ui1ColorSpace == HDMI_YCBCR_422)
		{
			if((ui1Res==RES_480I)||(ui1Res==RES_576I))
				vSetRGBYCbCrDelay(0,0,0,1,1,1);
			else if(ui1Res == RES_3D_1080I60HZ || ui1Res == RES_3D_1080I50HZ)
				vSetRGBYCbCrDelay(0,0,0,3,0,3);
			else
				vSetRGBYCbCrDelay(0,0,0,1,0,1);
		}
		else
		{
			vSetRGBYCbCrDelay(0,0,0,1,1,1);
		}
	}

}
void vSetHDMISyncDelay(BYTE bResIndex, BYTE bAdjForward)
{
	if(bResIndex < MAX_RES)
		vWriteHdmiRGB(HDMI_RGB2HDMI_B4, HDMI_SYNC_DELAY[bResIndex][0]);
	
	printk("[DGI] bAdjForward = %d\n", bAdjForward);
	if(bAdjForward == TRUE)
	  vWriteHdmiRGBMsk(HDMI_RGB2HDMI_B4,ADJ_FORWARD,ADJ_FORWARD);
	else
	  vWriteHdmiRGBMsk(HDMI_RGB2HDMI_B4, 0, ADJ_FORWARD);
	
}
void rgb2hdmisetres(unsigned char ucRes,unsigned char u1colorspace)
{
	//RGB2HDMI Setting 
	vselecthdmisrcpath();
	vSetHDMIDataEnable(ucRes);
	vSetHDMI3DDataEnable(ucRes);
	rgb2hdmi_setting_res(ucRes,u1colorspace);
	if((ucRes == RES_480P) ||(ucRes == RES_576P))
		vSetHDMISyncDelay(ucRes,TRUE);
	else
		vSetHDMISyncDelay(ucRes,TRUE);

}

