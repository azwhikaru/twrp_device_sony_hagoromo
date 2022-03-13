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
#include <linux/kernel.h>
#include <linux/delay.h>

#include "tve_test.h"
#include "tve_ctrl.h"
#include "tve_drv.h"

#define HDMI_ANALOG_BASE (0xf0209000)
#define MHL_TVDPLL_CON0	0x260
#define RG_TVDPLL_EN			(1)
#define RG_TVDPLL_POSDIV				(4)
#define RG_TVDPLL_POSDIV_MASK			(0x07 << 4)
#define MHL_TVDPLL_CON1	0x264
#define RG_TVDPLL_SDM_PCW				(0)
#define RG_TVDPLL_SDM_PCW_MASK			(0x1FFFFF)
#define TVDPLL_SDM_PCW_CHG        (1 << 31)
#define TVDPLL_SDM_PCW_F        (1<<23)

#define MHL_TVDPLL_PWR	0x26C
#define RG_TVDPLL_PWR_ON		(1)

#define vWriteHdmiANA(dAddr, dVal)  (*((volatile unsigned int *)(HDMI_ANALOG_BASE + dAddr)) = (dVal))
#define dReadHdmiANA(dAddr)         (*((volatile unsigned int *)(HDMI_ANALOG_BASE + dAddr)))
#define vWriteHdmiANAMsk(dAddr, dVal, dMsk) (vWriteHdmiANA((dAddr), (dReadHdmiANA(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

 void dpi_setting_res(unsigned char arg)
{
	switch(arg)
	 {
	  case 0://HDMI_VIDEO_720x480p_60Hz:
	  {
	
	*(volatile unsigned int *)0xF4014000 = 0x00000001;
	*(volatile unsigned int *)0xF4014004 = 0x00000000;
	*(volatile unsigned int *)0xF4014008 = 0x00000000;
	*(volatile unsigned int *)0xF401400c = 0x00000007;
	*(volatile unsigned int *)0xF4014010 = 0x00410010;
	
    *(volatile unsigned int *)0xF40140b0 = 0x02000000;
	*(volatile unsigned int *)0xF4014014 = 0x00000000;
	*(volatile unsigned int *)0xF4014018 = 0x01e002d0;
	*(volatile unsigned int *)0xF4014020 = 0x0000003e;
	*(volatile unsigned int *)0xF4014024 = 0x0010003c;
	*(volatile unsigned int *)0xF4014028 = 0x00000006;
	*(volatile unsigned int *)0xF401402c = 0x0009001e;
	*(volatile unsigned int *)0xF4014068 = 0x00000000;
	*(volatile unsigned int *)0xF401406c = 0x00000000;
	*(volatile unsigned int *)0xF4014070 = 0x00000000;
	*(volatile unsigned int *)0xF4014074 = 0x00000000;
	*(volatile unsigned int *)0xF4014078 = 0x00000000;
	*(volatile unsigned int *)0xF401407c = 0x00000000;

	*(volatile unsigned int *)0xF4014f00 = 0x00000041;


	*(volatile unsigned int *)0xF40140a0 = 0x00000000;


		 break;
	  }
	  case 1://HDMI_VIDEO_720x576p_50Hz:
	  {
	
	*(volatile unsigned int *)0xF4014000 = 0x00000001;
	*(volatile unsigned int *)0xF4014004 = 0x00000000;
	*(volatile unsigned int *)0xF4014008 = 0x00000000;
	*(volatile unsigned int *)0xF401400c = 0x00000007;
	*(volatile unsigned int *)0xF4014010 = 0x00410010;
	
    *(volatile unsigned int *)0xF40140b0 = 0x02000000;
	*(volatile unsigned int *)0xF4014014 = 0x00000000;
	*(volatile unsigned int *)0xF4014018 = 0x024002d0;
	*(volatile unsigned int *)0xF4014020 = 0x00000040;
	*(volatile unsigned int *)0xF4014024 = 0x000c0044;
	*(volatile unsigned int *)0xF4014028 = 0x00000005;
	*(volatile unsigned int *)0xF401402c = 0x00050027;
	*(volatile unsigned int *)0xF4014030 = 0x00010001;
	*(volatile unsigned int *)0xF4014068 = 0x00000000;
	*(volatile unsigned int *)0xF401406c = 0x00000000;
	*(volatile unsigned int *)0xF4014070 = 0x00000000;
	*(volatile unsigned int *)0xF4014074 = 0x00000000;
	*(volatile unsigned int *)0xF4014078 = 0x00000000;
	*(volatile unsigned int *)0xF401407c = 0x00000000;

	*(volatile unsigned int *)0xF4014f00 = 0x00000041;


	*(volatile unsigned int *)0xF40140a0 = 0x00000000;

		 break;
	  }

	  default:
	  	break;
	 }

     mdelay(10);
     *((volatile unsigned int *)(0xF4014004)) = (1);
     mdelay(40);
     *((volatile unsigned int *)(0xF4014004)) = (0);

}
int tve_test_cmd(int psInput)
 {
	 printk("[cvbs] Test item is %s and the Id is 0x%x \n",__FUNCTION__,psInput);
 
	 switch(psInput)
	 {
		 case CHECK_TVE_NTSC_VERIFY:
		 	 TveSetFmt(TVE_RES_480P);
			 //TVE_SetFmt(TVE_RES_480P, TVE_RES_480P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
			 break;
			 
		 case CHECK_TVE_PAL_VERIFY:
		 	 TveSetFmt(TVE_RES_576P);
			// TVE_SetFmt(TVE_RES_576P, TVE_RES_576P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
			 break;
 
		 case CHECK_TVE_TEST_PATTERN:
			 printk("[cvbs] Set resoluton to 576P. \n");
			 tve_mipipll_dpi0_tve_path();

			 TVE_Init();
			// TVE_SetFmt(TVE_RES_480P, TVE_RES_480P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
			 TVE_SetFmt(TVE_RES_576P, TVE_RES_576P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
                      TVE_SetEnable(TRUE);
			// TVE_SetColorBar(TRUE);
			 TVE_HalColorBar(TRUE);
			 TVE_SetSyncTime1(TRUE,FALSE,0x008, 0x32b);

			 break;
			 
		 case CHECK_TVE_MACROVERSION_TEST:
			 TVE_SetMacroVision((UCHAR)TVE_RES_576P);
			 
			 break;
			 
		 case CHECK_DPI0_TEST_PATTERN:
			 tve_mipipll_dpi0_tve_path();
			 
			 TVE_Init();
			 TVE_SetFmt(TVE_RES_576P, TVE_RES_576P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
                                           TVE_SetEnable(TRUE);
			// TVE_SetFmt(TVE_RES_480P, TVE_RES_480P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
			 //TVE_SetColorBar(TRUE);
			 TVE_HalColorBar(FALSE);
			 TVE_SetSyncTime1(TRUE,FALSE,0x008, 0x32b);
			 //vDPI0_480p();
			 vDPI0_576p();
			 break;

		 case CHECK_TVE_DEBUG_LOG:
			{
			static bool flag = FALSE;
			flag=!flag;
			Tve_LogEnable(flag);
			break;
			}
		 case 9:
	 		{
		 	extern void cvbs_state_callback(CVBS_STATE state);
		 	cvbs_state_callback(CVBS_STATE_ACTIVE);
			break;
		 	}
		 case 10:
			{
			TVE_Resume();
			break;
			}
		 case 11:
			{
			TVE_Suspend();
			break;
			}
		 default:
			 break;
	 }	 
   return 0;
 }

