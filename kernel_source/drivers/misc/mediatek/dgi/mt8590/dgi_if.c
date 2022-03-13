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
#include "video_in_if.h"
#include "vsw_drv_if.h"
#include "typedef.h"
#include "dgi_if.h"
#include "hdmitable.h"

UCHAR _ucDGIResolution = RES_480P;

const UINT8 u1RxYuvInDelay[]=
{   
    0x3,	//RES_480I=0,
    0x3,	//RES_576I, //1
    0x1,	//RES_480P, //2
    0x1,	//RES_576P, //3
    0x1,	//RES_480P_1440,//4
    0x1,	//RES_576P_1440,//5
    0x1,	//RES_480P_2880,//6
    0x1,	//RES_576P_2880,//7
    0x1,	//RES_720P60HZ,//8
    0x1,	//RES_720P50HZ,//9
    0x1,	//RES_1080I60HZ,//10
    0x1,	//RES_1080I50HZ,//11
    0x1,	//RES_1080P60HZ,//12
    0x1,	//RES_1080P50HZ,//13
    0x1,	//RES_1080P30HZ,//14
    0x1,	//RES_1080P25HZ, //15
    0x3,	//RES_480I_2880,//16
    0x3,	//RES_576I_2880,//17
    0x1, 	//RES_1080P24HZ, //18
    0x1,	//RES_1080P23.97HZ, //19
    0x1, 	//RES_1080P29.97HZ, //20
    0x0, 	//RES_3D_1080P23HZ, //21, 1080p47.952Hz
    0x0, 	//RES_3D_1080P24HZ, //22, 1080p48hz
    0x0, 	//RES_3D_720P60HZ, //23, 720p120hz
    0x0, 	//RES_3D_720P50HZ, //24, 720p100hz
    0x0, 	//RES_3D_720P30HZ, //25, 720p120hz
    0x0, 	//RES_3D_720P25HZ, //26, 720p100hz
    0x0, 	//RES_3D_576P50HZ, //27, 576p100hz
    0x0, 	//RES_3D_480P60HZ, //28, 480p120hz
    0x0, 	//RES_3D_1080I60HZ, //29, 1080i120hz
    0x0, 	//RES_3D_1080I50HZ, //30, 1080i100hz
    0x0, 	//RES_3D_1080I30HZ, //31, 1080i120hz
    0x0, 	//RES_3D_1080I25HZ, //32, 1080i100hz
    0x0, 	//RES_3D_576I25HZ, //33, 576i100hz
    0x0, 	//RES_3D_480I30HZ, //34, 480i120hz
    0x0, 	//RES_3D_576I50HZ, //35, 576i100hz
    0x0, 	//RES_3D_480I60HZ, //36, 480i120hz
    0x3, 	//RES_2D_480I60HZ, //37, 480i60hz
    0x3, 	//RES_2D_576I50HZ, //38, 576i50hz   
    0x1, 	//RES_2D_640x480HZ,//39
    0x0, 	//RES_PANEL_AUO_B089AW01, //40 //Total: 1344x625, Act: 1024x600, Frm: 60Hz, Clk: 50.4MHz
    0x1, 	//RES_3D_720P60HZ_TB,  //41
    0x1, 	//RES_3D_720P50HZ_TB,  //42
    0x1, 	//RES_3D_1080I60HZ_SBS_HALF,//43
    0x1, 	//RES_3D_1080I50HZ_SBS_HALF,//44
    0x1, 	//RES_3D_1080P23HZ_TB, //45
    0x1, 	//RES_3D_1080P24HZ_TB,//46
    0x0, 	//RES_4K2K23976HZ, //47
    0x0, 	//RES_4K2K24HZ, //48
    0x0, 	//RES_4K2K23976HZ, //47
    0x0, 	//RES_4K2K24HZ, //48    
    0x0, 	//RES_2160P,//49
    0x0, 	//RES_2160P_4096, //50
    0x1, 	//RES_720P30HZ, // 51
    0x1, 	//RES_720P25HZ, // 52
    0x1, 	//RES_720P24HZ, // 53
    0x1, 	//RES_720P23HZ, // 54
    0x0, 	//RES_3D_1080P60HZ,// 55
    0x0, 	//RES_3D_1080P50HZ,//56
    0x0, 	//RES_3D_1080P30HZ,//57
    0x0, 	//RES_3D_1080P29HZ,//58
    0x0, 	//RES_3D_1080P25HZ,//59
    0x0, 	//RES_3D_720P24HZ, //60
    0x0, 	//RES_3D_720P23HZ, //61
    0x1, 	//RES_3D_1080P60HZ_TB, //62
    0x1, 	//RES_3D_1080P50HZ_TB, //63
    0x1, 	//RES_3D_1080P30HZ_TB, //64
    0x1, 	//RES_3D_1080P29HZ_TB,//65
    0x1, 	//RES_3D_1080P25HZ_TB, //66
    0x1, 	//RES_3D_1080I60HZ_TB, //67
    0x1, 	//RES_3D_1080I50HZ_TB,//68
    0x1, 	//RES_3D_1080I30HZ_TB,//69
    0x1, 	//RES_3D_1080I25HZ_TB,//70
    0x1, 	//RES_3D_720P30HZ_TB,//71
    0x1, 	//RES_3D_720P25HZ_TB,//72
    0x1, 	//RES_3D_720P24HZ_TB,//73
    0x1, 	//RES_3D_720P23HZ_TB,//74
    0x1, 	//RES_3D_576P50HZ_TB,//75
    0x3, 	//RES_3D_576I25HZ_TB,//76
    0x3, 	//RES_3D_576I50HZ_TB,//77
    0x1, 	//RES_3D_480P60HZ_TB,//78
    0x3, 	//RES_3D_480I30HZ_TB,//79
    0x3, 	//RES_3D_480I60HZ_TB,//80
    0x1, 	//RES_3D_1080P60HZ_SBS_HALF,//81
    0x1, 	//RES_3D_1080P50HZ_SBS_HALF,//82
    0x1, 	//RES_3D_1080P30HZ_SBS_HALF,//83
    0x1, 	//RES_3D_1080P29HZ_SBS_HALF,//84
    0x1, 	//RES_3D_1080P25HZ_SBS_HALF,//85
    0x1, 	//RES_3D_1080P24HZ_SBS_HALF,//86
    0x1,	//RES_3D_1080P23HZ_SBS_HALF,//87
    0x1, 	//RES_3D_1080I30HZ_SBS_HALF,//88
    0x1, 	//RES_3D_1080I25HZ_SBS_HALF,//89
    0x1, 	//RES_3D_720P60HZ_SBS_HALF,//90
    0x1, 	//RES_3D_720P50HZ_SBS_HALF,//91
    0x1, 	//RES_3D_720P30HZ_SBS_HALF,//92
    0x1, 	//RES_3D_720P25HZ_SBS_HALF,//93
    0x1, 	//RES_3D_720P24HZ_SBS_HALF,//94
    0x1, 	//RES_3D_720P23HZ_SBS_HALF,//95
    0x1, 	//RES_3D_576P50HZ_SBS_HALF,//96
    0x3, 	//RES_3D_576I25HZ_SBS_HALF,//97
    0x3, 	//RES_3D_576I50HZ_SBS_HALF,//98
    0x1, 	//RES_3D_480P60HZ_SBS_HALF,//99
    0x3, 	//RES_3D_480I30HZ_SBS_HALF,//100
    0x3, 	//RES_3D_480I60HZ_SBS_HALF,//101
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,    //RES_MODE_NUM,
    0x0,    //RES_AUTO
};

void vset_dgi_in_mode(DGI_VIDEO_IN_INFO_T rInfo, UCHAR ucRes)   
{
    if(ucRes > RES_MODE_NUM || rInfo.eInputMode == FMT_INVALID || rInfo.eInputMode > FMT_656_MODE || 
      (VIDEO_DGI_IN_FORMAT_T)(rInfo.ePinType) > YC444_36BIT)
    {
		printk("[CCIR] INPUT INVALID MODE or INVALID FMT!\n");
		printk("[CCIR] ucRes = %d,rInfo.eInputMode = %d,rInfo.ePinType = %d\n",ucRes,rInfo.eInputMode,rInfo.ePinType);
        return;
    }

	//vconfig_hdmirx_sys(ucRes);
    vHal_SetDgiInModeCtrl(rInfo.eInputMode);
    set_src_to_dgi_in_fmt(rInfo.ePinType,ucRes);
    vhal_hdmirx_mux_switch(TRUE);
    vhal_set_dgi_timing(rInfo.ePinType, ucRes);
    vHal_DgiIn_YUV_Sel(u1RxYuvInDelay[ucRes]); 
    vHal_DgiBitMode(rInfo.ePinType, ucRes);
	//rgb2hdmisetres(ucRes,HDMI_RGB_FULL);
	vHal_RstDgiFifo();
}

void vdisable_DGI_In(UCHAR ucRes)   
{
    printk("[DGI] Disable DGI Input, Res = %d!\n", ucRes);
    if(ucRes >= RES_MODE_NUM)
    {
        return;
    }
    vhal_hdmirx_mux_switch(FALSE);
	vHal_Disable_DGI_In();  
}
