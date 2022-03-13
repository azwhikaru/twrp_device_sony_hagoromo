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
#include <mach/mt_clkmgr.h>
#include <mach/mt_spm_mtcmos.h>

#include "dgi_if.h"
#include "dgi_hw.h"
#include "typedef.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"

const UINT32 u4MasterTimming[]=
{   
    0x06B420D1,	//RES_480I=0,
    0x06C02711,	//RES_576I,
    0x035A20D9,	//RES_480P,
    0x03602719,	//RES_576P,
    0x035A20D9,	//RES_480P_1440,//4
    0x03602719,	//RES_576P_1440,//5
    0x035A20D9,	//RES_480P_2880,//6
    0x03602719,	//RES_576P_2880,//7
    0x06722eef,	//RES_720P60HZ,
    0x07BC2eef,	//RES_720P50HZ,
    0x08984653,	//RES_1080I60HZ,
    0x0A504653,	//RES_1080I50HZ,
    0x0898465b,	//RES_1080P60HZ,      
    0x0A50465b,	//RES_1080P50HZ,
    0x0898465b,	//RES_1080P30HZ,
    0x0A50465b,	//RES_1080P25HZ,
    0x06B420D1,	//RES_480I_2880,//16
    0x06C02711,	//RES_576I_2880,//17
    0x0ABE465B,	//RES_1080P24HZ,
    0x0ABE465B,	//RES_1080P23.976hz //19
    0x0ABE465B,	//RES_1080P29.97hz //20   
    0x0ABE8CAB,	//RES_3D_1080P23HZ, //21, 1080p47.952Hz
    0x0ABE8CAB,	//RES_3D_1080P24HZ, //22, 1080p48hz
    0x06725DCB,	//RES_3D_720P60HZ, //23, 720p120hz
    0x07BC5DCB,	//RES_3D_720P50HZ, //24, 720p100hz
    0x0CE45DCB,	//RES_3D_720P30HZ, //25, 720p120hz
    0x0F785DCB,	//RES_3D_720P25HZ, //26, 720p100hz
    0x0ABE465B,	//RES_3D_576P50HZ, //27, 576p100hz
    0x0ABE465B,	//RES_3D_480P60HZ, //28, 480p120hz
    0x08988CAB,	//RES_3D_1080I60HZ, //29, 1080i120hz
    0x0A508CAB,	//RES_3D_1080I50HZ, //30, 1080i100hz
    0x08988CAB,	//RES_3D_1080I30HZ, //31, 1080i120hz
    0x0A508CAB,	//RES_3D_1080I25HZ, //32, 1080i100hz
    0x06C04E21,	//RES_3D_576I25HZ, //33, 576i100hz
    0x06B441A1,	//RES_3D_480I30HZ, //34, 480i120hz
    0x06C04E21,	//RES_3D_576I50HZ, //35, 576i100hz
    0x06B441A1,	//RES_3D_480I60HZ, //36, 480i120hz
    0x06B420D1,	//RES_2D_480I60HZ, //37, 480i60hz
    0x06C02711,	//RES_2D_576I50HZ, //38, 576i50hz
    0x032020d9, //RES_2D_640x480HZ,//39
    0x05402719,     //RES_PANEL_AUO_B089AW01 40
    0x06722eef,     //RES_3D_720P60HZ_TB,  //41
    0x07BC2eef,     //RES_3D_720P50HZ_TB,  //42
    0x08984653,     //RES_3D_1080I60HZ_SBS_HALF,//43
    0x0A504653,     //RES_3D_1080I50HZ_SBS_HALF,//44
    0x0ABE465B,     //RES_3D_1080P23HZ_TB, //45
    0x0ABE465B,     //RES_3D_1080P24HZ_TB,//46
    0x157C8CAB,     //RES_4K2K23976HZ, //47
    0x157C8CAB,     //RES_4K2K24HZ, //48
    0x157C8CAB,     //RES_4K2K23976HZ, //47
    0x157C8CAB,     //RES_4K2K24HZ, //48    
    0x14A08CAB,     //RES_2160P,//49
    0x157C8CAB,     //RES_2160P_4096, //50
    0x0CE42eef,     //RES_720P30HZ, // 51
    0x0F782eef,     //RES_720P25HZ, // 52
    0x0CE42eef,     //RES_720P24HZ, // 53
    0x0CE42eef,     //RES_720P23HZ, // 54
    0x08988CAb,     //RES_3D_1080P60HZ,// 55
    0x0A508CAb,     //RES_3D_1080P50HZ,//56
    0x08988CAb,     //RES_3D_1080P30HZ,//57
    0x08988CAb,     //RES_3D_1080P29HZ,//58
    0x0A508CAb,     //RES_3D_1080P25HZ,//59
    0x0CE45DCb,     //RES_3D_720P24HZ, //60
    0x0CE45DCb,     //RES_3D_720P23HZ, //61
    0x0898465b,     //RES_3D_1080P60HZ_TB, //62
    0x0A50465b,     //RES_3D_1080P50HZ_TB, //63
    0x0898465b,     //RES_3D_1080P30HZ_TB, //64
    0x0898465b,     //RES_3D_1080P29HZ_TB,//65
    0x0A50465b,     //RES_3D_1080P25HZ_TB, //66
    0x08984653,     //RES_3D_1080I60HZ_TB, //67
    0x0A504653,     //RES_3D_1080I50HZ_TB,//68
    0x08984653,     //RES_3D_1080I30HZ_TB,//69
    0x0A504653,     //RES_3D_1080I25HZ_TB,//70
    0x0CE42eef,     //RES_3D_720P30HZ_TB,//71
    0x0F782eef,     //RES_3D_720P25HZ_TB,//72
    0x0CE42eef,     //RES_3D_720P24HZ_TB,//73
    0x0CE42eef,     //RES_3D_720P23HZ_TB,//74
    0x03602719,     //RES_3D_576P50HZ_TB,//75
    0x06C02711,     //RES_3D_576I25HZ_TB,//76
    0x06C02711,     //RES_3D_576I50HZ_TB,//77
    0x035A20D9,     //RES_3D_480P60HZ_TB,//78
    0x06B420D1,     //RES_3D_480I30HZ_TB,//79
    0x06B420D1,     //RES_3D_480I60HZ_TB,//80
    0x0898465b,     //RES_3D_1080P60HZ_SBS_HALF,//81
    0x0A50465b,     //RES_3D_1080P50HZ_SBS_HALF,//82
    0x0898465b,     //RES_3D_1080P30HZ_SBS_HALF,//83
    0x0898465b,     //RES_3D_1080P29HZ_SBS_HALF,//84
    0x0A50465b,     //RES_3D_1080P25HZ_SBS_HALF,//85
    0x0ABE465B,     //RES_3D_1080P24HZ_SBS_HALF,//86
    0x0ABE465B,     //RES_3D_1080P23HZ_SBS_HALF,//87
    0x08984653,     //RES_3D_1080I30HZ_SBS_HALF,//88
    0x0A504653,     //RES_3D_1080I25HZ_SBS_HALF,//89
    0x06722eef,     //RES_3D_720P60HZ_SBS_HALF,//90
    0x07BC2eef,     //RES_3D_720P50HZ_SBS_HALF,//91
    0x0CE42eef,     //RES_3D_720P30HZ_SBS_HALF,//92
    0x0F782eef,     //RES_3D_720P25HZ_SBS_HALF,//93
    0x0CE42eef,     //RES_3D_720P24HZ_SBS_HALF,//94
    0x0CE42eef,     //RES_3D_720P23HZ_SBS_HALF,//95
    0x03602719,     //RES_3D_576P50HZ_SBS_HALF,//96
    0x06C02711,     //RES_3D_576I25HZ_SBS_HALF,//97
    0x06C02711,     //RES_3D_576I50HZ_SBS_HALF,//98
    0x035A20D9,     //RES_3D_480P60HZ_SBS_HALF,//99
    0x06B420D1,     //RES_3D_480I30HZ_SBS_HALF,//100
    0x06B420D1,     //RES_3D_480I60HZ_SBS_HALF,//101
};

BOOL vHal_DgiInCbCrSwap(BOOL u1Swap)
{
    
    if(u1Swap)
    {
    	printk("enter %s @L%d\n", __FUNCTION__, __LINE__);
		vWriteDgiMsk(DGI_CTRL, DGI_SWAP_UV, DGI_SWAP_UV);
    }
    else
    {
		printk("enter %s @L%d\n", __FUNCTION__, __LINE__);
		vWriteDgiMsk(DGI_CTRL, 0, DGI_SWAP_UV);
    }
}

void vHal_DgiYCSwap(unsigned char u1Swap)
{
	printk("[DGI] set input YC = %d\n", u1Swap);
	vWriteDgiMsk(INPUT_CTRL, 0, DGI_Y_CHANNEL_SEL|DGI_C_CHANNEL_SEL|DGI_C2_CHANNEL_SEL);
    switch (u1Swap)
    {
        case 0: //Y Cb Cr
        default:
			vWriteDgiMsk(INPUT_CTRL, INPUT_Y_Y_CHANNEL|INPUT_C_CB_CHANNEL|INPUT_C2_CR_CHANNEL, INPUT_Y_Y_CHANNEL|INPUT_C_CB_CHANNEL|INPUT_C2_CR_CHANNEL);
            break;

        case 1: // Y Cr Cb
			vWriteDgiMsk(INPUT_CTRL, INPUT_Y_Y_CHANNEL|INPUT_C_CR_CHANNEL|INPUT_C2_CB_CHANNEL, INPUT_Y_Y_CHANNEL|INPUT_C_CR_CHANNEL|INPUT_C2_CB_CHANNEL);
            break;     

        case 2: // Cb Y Cr
			vWriteDgiMsk(INPUT_CTRL, INPUT_Y_CB_CHANNEL|INPUT_C_Y_CHANNEL|INPUT_C2_CR_CHANNEL, INPUT_Y_CB_CHANNEL|INPUT_C_Y_CHANNEL|INPUT_C2_CR_CHANNEL);
            break; 

        case 3: // Cb Cr Y
			vWriteDgiMsk(INPUT_CTRL, INPUT_Y_CB_CHANNEL|INPUT_C_CR_CHANNEL|INPUT_C2_Y_CHANNEL, INPUT_Y_CB_CHANNEL|INPUT_C_CR_CHANNEL|INPUT_C2_Y_CHANNEL);
            break;

        case 4: // Cr Y Cb
			vWriteDgiMsk(INPUT_CTRL, INPUT_Y_CR_CHANNEL|INPUT_C_Y_CHANNEL|INPUT_C2_CB_CHANNEL, INPUT_Y_CR_CHANNEL|INPUT_C_Y_CHANNEL|INPUT_C2_CB_CHANNEL);
            break; 

        case 5: // Cr Cb Y
			vWriteDgiMsk(INPUT_CTRL, INPUT_Y_CR_CHANNEL|INPUT_C_CB_CHANNEL|INPUT_C2_Y_CHANNEL, INPUT_Y_CR_CHANNEL|INPUT_C_CB_CHANNEL|INPUT_C2_Y_CHANNEL);
            break;            
    }  
	
}

void vhal_set_dgi_timing(VIDEO_DGI_IN_MODE_T eMode,unsigned char ucRes)   
{
    UINT32 u4RegTmp;
     
	printk("[DGI] set input res = %d\n", ucRes);
	switch (ucRes)
	{
		case RES_480I:
		case RES_480I_2880:  		
		case RES_576I:
		case RES_576I_2880:          
		case RES_480P:
		case RES_480P_1440:
		case RES_480P_2880:    	
		case RES_576P:
		case RES_576P_1440:
		case RES_576P_2880:
		case RES_2D_480I60HZ:
		case RES_2D_576I50HZ:
		case RES_2D_640x480HZ:
			vWriteDgiMsk(VIDEO_FMT_CTRL, FMT_CTRL_ON, FMT_CTRL_ON|FMT_CTRL_HPOR|FMT_CTRL_VPOR);
		break;

		default:
			vWriteDgiMsk(VIDEO_FMT_CTRL, FMT_CTRL_ON|FMT_CTRL_HPOR|FMT_CTRL_VPOR, FMT_CTRL_ON|FMT_CTRL_HPOR|FMT_CTRL_VPOR);
		break;
	}
	
    u4RegTmp = bReadRegDgi(VIDEO_FMT_CTRL);
	printk("[DGI] VIDEO FMT CTRL 0XAC = 0x%x\n", u4RegTmp);
	
	vWriteDgiMsk(MULTI_R_CTRL, (0x01<<2), (0x01<<2));
    if(ucRes < (sizeof(u4MasterTimming)/4))
    {
      u4RegTmp = u4MasterTimming[ucRes];
    }
	
    vWriteRegDgi(MASTER_T_CTRL, u4RegTmp);
	if(fgIsProgress(ucRes))
	{
		vWriteDgiMsk(MODE_CTRL, Progressive_Mode_EN,Progressive_Mode_EN);
	}
	
}

void vHal_DgiBitMode(VIDEO_DGI_IN_FORMAT_T eFmtmode, unsigned char ucRes)
{
	vWriteDgiMsk(DGI_CTRL, 0, DGI_10BIT_MODE|DGI_12BIT_MODE|DGI_MIXING_MODE|DGI_444);
    switch (eFmtmode)
    {
       case YCMIX_8BIT:   
       case YC422_16BIT:
			vWriteDgiMsk(DGI_CTRL, 0, DGI_10BIT_MODE|DGI_12BIT_MODE);
           break;
        
       case YCMIX_10BIT:
       case YC422_20BIT:
			vWriteDgiMsk(DGI_CTRL, DGI_10BIT_MODE, DGI_10BIT_MODE);
           break;

       case YCMIX_12BIT:
       case YC422_24BIT:
			vWriteDgiMsk(DGI_CTRL, DGI_12BIT_MODE, DGI_12BIT_MODE);
           break;

       case YC444_24BIT:
       case YC444_30BIT:
       case YC444_36BIT:
			vWriteDgiMsk(DGI_CTRL, DGI_444,DGI_444);
           break;

       default:
           break;
    }

    switch (ucRes)
    {
       case RES_480I:    //27M
       case RES_480I_2880:  		
       case RES_576I:
       case RES_576I_2880:  
		   vWriteDgiMsk(DGI_CTRL, 0, DGI_MIXING_MODE);
           break;    

       case RES_480P:
       case RES_480P_1440:
       case RES_480P_2880:    	
       case RES_576P:
       case RES_576P_1440:
       case RES_576P_2880:
  	
       case RES_720P60HZ:   //74M
       case RES_720P50HZ:    	
	   case RES_720P30HZ:               
       case RES_720P25HZ:               
       case RES_720P24HZ:               
       case RES_720P23HZ: 
   
       case RES_1080I60HZ:   //74M
       case RES_1080I50HZ:  
       case RES_1080P25HZ:	
       case RES_1080P30HZ:	 
       case RES_1080P24HZ:	 
       case RES_1080P60HZ:	
       case RES_1080P50HZ:	 
       case RES_1080P23_976HZ:	
       case RES_1080P29_97HZ:
			vWriteDgiMsk(DGI_CTRL, 0, DGI_MIXING_MODE);
           break; 
           
       default:
           break;  
    }
	
	vWriteDgiMsk(MIX_CTRL3, 0, DGI_4FS_OPT|DGI_2FS_OPT);
    if(!fgIsYCbCr444((VIDEO_DGI_IN_FORMAT_T)eFmtmode))
    {
		switch (ucRes)
		{
			case RES_480I:   
			case RES_576I:
			case RES_480P_1440:
			case RES_576P_1440:    
			case RES_3D_576I25HZ_SBS_HALF:   
			case RES_3D_576I50HZ_SBS_HALF:   
			case RES_3D_480I30HZ_SBS_HALF:   
			case RES_3D_480I60HZ_SBS_HALF: 
			case RES_3D_576I25HZ_TB:         
			case RES_3D_576I50HZ_TB:         
			case RES_3D_480I30HZ_TB:         
			case RES_3D_480I60HZ_TB:  
				vWriteDgiMsk(MIX_CTRL3, DGI_2FS_OPT, DGI_4FS_OPT|DGI_2FS_OPT);// 2fs
			break; 

			case RES_480I_2880:  		
			case RES_576I_2880: 
			case RES_480P_2880:    	
			case RES_576P_2880:
				vWriteDgiMsk(MIX_CTRL3, DGI_4FS_OPT, DGI_4FS_OPT|DGI_2FS_OPT);// 4fs		  
			break;        

			default:
			break;  
		} 
    }
}

void vHal_DgiIn_YUV_Sel(UINT8 u1Delay)
{
	UINT32 u4RegTmp,u4Tmp;
	vWriteDgiMsk(MIXER_CTRL4,0,DGI_YUV_SEL);
	
	u4Tmp = (u1Delay > 7) ? 7 : u1Delay;
	u4RegTmp |= (u4Tmp << 8);
	
	vWriteRegDgi(MIXER_CTRL4, u4RegTmp);

}

void set_src_to_dgi_in_fmt(unsigned int eDgiBitMode, unsigned char ucRes)   
{
    printk("[DGI] set input Fmt = %d, res = %d\n", eDgiBitMode, ucRes);
    switch(eDgiBitMode)
    {
        case YC444_24BIT:
        case YC444_30BIT:
        case YC444_36BIT:
            vHal_DgiYCSwap(DGI_CBCRY_SWAP);
            vHal_DgiInCbCrSwap(DGI_CBCR_SWAP_OFF);
            break;

        case YC422_16BIT:
        case YC422_20BIT:
        case YC422_24BIT:
            vHal_DgiYCSwap(DGI_CBYCR_SWAP);
			vHal_DgiInCbCrSwap(DGI_CBCR_SWAP_OFF);
            break;
            
        default:
            vHal_DgiYCSwap(DGI_YCRCB_SWAP);
            vHal_DgiInCbCrSwap(DGI_CBCR_SWAP_OFF);
            break;
    }

}

void vhal_hdmirx_mux_switch(BOOL fgRxIn)   
{
    if(fgRxIn)
    {
    	vWriteTOPCKMsk(BDP_DISPSYS_DISP_CLK_CONFIG1,0, DGI_SELF_TEST_MODE);
		vWriteTOPCKMsk(BDP_DISPSYS_DISP_CLK_CONFIG1,0,(1<<3)|(1<<4));
		vWriteTOPCKMsk(0x10438,(1<<30),(1<<29)|(1<<30)|(1<<31));
    }
    else
    {
		//vWriteTOPCKMsk(BDP_DISPSYS_DISP_CLK_CONFIG1,0, DGI_SELF_TEST_MODE|WR_CH_DI_SEL_TEST_MODE)
		vWriteTOPCKMsk(BDP_DISPSYS_DISP_CLK_CONFIG1,(1<<4), (1<<3)|(1<<4)|(1<<5));
		vWriteTOPCKMsk(0x10438,(1<<29)|(1<<31),(1<<29)|(1<<30)|(1<<31));
    }
}

void vHal_SetDisturbLineCnt(UCHAR ucRes) 
{
    UINT32 u4RegTmp;

    u4RegTmp = (u4MasterTimming[ucRes] & 0xF) | 0x00100108;
    vWriteRegDgi(MASTER_T_CTRL, u4RegTmp);
    
    u4RegTmp = bReadRegDgi(MULTI_R_CTRL);
    vWriteRegDgi(MULTI_R_CTRL, u4RegTmp & ~(0x01<<2));
}

void vHal_SetDgiInModeCtrl(VIDEO_DGI_IN_MODE_T eMode) 
{
    //****************  DGI control   //*****************************/ 
    //Bit 0: dgi on  Bit1-2:  dgi format    -> 8281 seperated H/V sync
    set_rx_dgi_to_rgb2hdmi_clk();  
	vWriteDgiMsk(DGI_CTRL, DGI_ON, DGI_ON|DGI_TEST_MODE_EN|DGI_ENCRYPT_Y);
    
    if(eMode == FMT_601_MODE)
    {
		vWriteDgiMsk(DGI_CTRL,MIX_PLN1_MSK_SEL|MIX_PLN2_MSK_SEL, DGI_INVERT_FIELD|MIX_PLN1_MSK_SEL|MIX_PLN2_MSK_SEL);
    }
    else
    {
		vWriteDgiMsk(DGI_CTRL, DGI_INVERT_FIELD, DGI_INVERT_FIELD|MIX_PLN1_MSK_SEL|MIX_PLN2_MSK_SEL);
    }
    
}

void set_rx_dgi_to_rgb2hdmi_clk(void)
{
    int ret = 0;
	//ret += enable_clock(MT_CG_DGI_IN, "dgi");
	//ret += enable_clock(MT_CG_DGI_OUT, "dgi");
	//ret += enable_clock(MT_CG_FMT_MAST_27_CK, "dgi");
	//ret += enable_clock(MT_CG_FMT_BCLK, "dgi");
	vWriteTOPCKMsk(BDP_DISPSYS_DISP_CLK_CONFIG1,0,DGI_SELF_TEST_MODE|(1<<3)|(1<<4)); 
}

void vHal_Disable_DGI_In(void)  
{
    int ret = 0;
	vWriteDgiMsk(DGI_CTRL,0, DGI_ON); //turn off DGI 
	ret += disable_clock(MT_CG_DGI_IN, "dgi");
	ret += disable_clock(MT_CG_DGI_OUT, "dgi");
}

void vHal_RstDgiFifo(void)   
{
    vWriteRegDgi(DGI_CTRL, bReadRegDgi(DGI_CTRL) | (0x01<<8));
    vWriteRegDgi(DGI_CTRL, bReadRegDgi(DGI_CTRL) & ~(0x01<<8));   
}

