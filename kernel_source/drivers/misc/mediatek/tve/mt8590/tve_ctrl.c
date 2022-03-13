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
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/byteorder/generic.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <mach/mt_clkmgr.h>

#include "tve_ctrl.h"
#include "tve_hw.h"
#include "tve_common.h"
#include "tve_drv.h"
#include "disp_drv_platform.h"
#include "mtkfb_priv.h"
//#include "clock_manager.h"
//#include "disp_common.h"

static UCHAR _TveInitiated = 0;
UCHAR _ucVVal = 0;
UCHAR _ucUVal = 0;

UINT8 _ucCVBSOutFmt = TVE_RES_480I;
BOOL _fgEnCVBSSetUp = TRUE;
BOOL _fgEnCVBSPal60 = FALSE;
static BOOL tve_is_power_on = FALSE;
BOOL cvbs_auto_detect_enable = FALSE;

unsigned int CVBS_LOG_ENABLE = CVBS_DEF_LOG_EN;

volatile HAL_8127_CVBS_TVE_FIELD_T rTveAuxReg;
TVE_CONF_T _arTveConf[TVE_MAX];
static TVE_VIDEO_ADJUST_T _stTveVideoAdjust={0,0,0,0,0};
static IBC_CpsCommonInfoParamsDef rLastCpsInfo;

static struct timer_list r_tve_timer;

#define VERIFY_TVE_ID(id)    if (id > TVE_MAX) { return TVE_SET_ERROR; }

#define T_WRITE32(_reg_, _val_)   	(*((volatile unsigned int*)(_reg_)) = (_val_))
#define T_REG32(_reg_)		(*((volatile unsigned int*)(_reg_)))
#define T_Mask(_reg_,_val_,_mask_)	T_WRITE32(_reg_, (T_REG32( _reg_)&(~ (_mask_)))|((_val_) & (_mask_)))

UINT8 u4CVBSSyncLevelTbl[2] = {
 0x08,// 0x3c,		//TVE_RES_576I=1
 0x08,//0x3c, 	//TVE_RES_480I=0,
};

UINT8 au1YCDelay[2][4] =
{
	//  Y      YG         Cb        Cr    delay
	{0x01,  0x01, 0x02,  0x00},	//PAL=0,
	{0x01,  0x02, 0x00,  0x00},	//NTSC = 1
};

const UINT32 _pdwMvVal[5][6] =
{
	{0x25111D36, 0x1B007111, 0x07F8241B, 0x0F0F0000, 0x0450B160, 0x000103FF},  // ntsc
	{0x25111D3E, 0x1B007111, 0x07F8241B, 0x0F0F0000, 0x0450B160, 0x000103FF},  // ntsc
	{0x2115173E, 0x1B025515, 0x07F8241B, 0x0F0F0000, 0x0450B160, 0x000103FF},  // ntsc
	{0x2A221A36, 0x1C002522, 0x03FE143D, 0x7EFE0154, 0x07409060, 0x00010155},  // pal
	{0x04000024, 0x1E000000, 0x00002C1E, 0x000f0000, 0x00000000, 0x00010000},	 // p-scan
};

UINT8 au1TVE_YScale[2] =
{
	TVE_CVBS_PAL_YScale,		//TVE_RES_576I=1
	TVE_CVBS_NTSC_YScale,		//TVE_RES_480I=0,
};

UINT32 const pdCVBSGammaBase[8] =
{0x0280, 0x0140, 0x0500, 0x03c0, 0x0780, 0x0640, 0x0a00, 0x08c4};	//10 step 0.7

UINT8 au1TVE_UVScale[2][2] =
{
	{TVE_CVBS_NTSC_UScale,TVE_CVBS_NTSC_VScale},
	{TVE_CVBS_PAL_UScale,TVE_CVBS_PAL_VScale},
};

static const CHAR* szResStr[] =
{
  "TVE_RES_480I",
  "TVE_RES_576I",
  "TVE_RES_480P",
  "TVE_RES_576P",
};

static UCHAR ASPECT_RATIO_2_WSS[ASPECT_RATIO_MAX]=
{
 WSS_FULL_4_3,
 WSS_LB_14_9_C,
 WSS_LB_14_9_T,
 WSS_LB_16_9_C,
 WSS_LB_16_9_T,
 WSS_LB_LG_16_9_C,
 WSS_FULL_14_9_C,
 WSS_ANA_16_9
 };
void tve_set_tvdpll_108m(void)
{
        vWriteTVEANAMsk(TVE_TVDPLL_PWR,RG_TVE_TVDPLL_PWR_ON,RG_TVE_TVDPLL_PWR_ON);
        udelay(5);

        vWriteTVEANAMsk(TVE_TVDPLL_CON0,0,RG_TVE_TVDPLL_EN);
        vWriteTVEANAMsk(TVE_TVDPLL_CON0,(0x04 << RG_TVE_TVDPLL_POSDIV),RG_TVE_TVDPLL_POSDIV_MASK);
        vWriteTVEANAMsk(TVE_TVDPLL_CON1,(0x80109d8a << RG_TVE_TVDPLL_SDM_PCW),RG_TVE_TVDPLL_SDM_PCW_MASK);
        vWriteTVEANAMsk(TVE_TVDPLL_CON0,RG_TVE_TVDPLL_EN,RG_TVE_TVDPLL_EN);
        udelay(20);
}

void  DPI_MIPI_clk_setting(unsigned int mipi_pll_clk_ref,unsigned int mipi_pll_clk_div1,unsigned int mipi_pll_clk_div2){

	UINT32 i, j;

	UINT32 txdiv0  = 0;
	UINT32 txdiv1  = 0;
	UINT32 posdiv  = 0;
	UINT32 prediv  = 0;
              UINT32 txmul = 0;

	UINT32 sdm_ssc_en     = 0;
	UINT32 sdm_ssc_prd    = 0;  // 0~65535
	UINT32 sdm_ssc_delta1 = 0;  // 0~65535
	UINT32 sdm_ssc_delta  = 0;  // 0~65535

              UINT32 loopback_en = 0;
              UINT32 lane0_en = 1;
              UINT32 lane1_en = 1;
              UINT32 lane2_en = 1;
              UINT32 lane3_en = 1;

              txmul = mipi_pll_clk_ref ;
              posdiv = mipi_pll_clk_div1;
	prediv   = mipi_pll_clk_div2;

	//initial MIPI PLL
	T_WRITE32((MIPI_CONFIG_BASE+0x050), 0x0);
	T_WRITE32((MIPI_CONFIG_BASE+0x068), 0x2);
	T_WRITE32((MIPI_CONFIG_BASE+0x044), 0x88492480);
	T_WRITE32((MIPI_CONFIG_BASE+0x000), 0x400);
	T_WRITE32((MIPI_CONFIG_BASE+0x054), 0x2);
	T_WRITE32((MIPI_CONFIG_BASE+0x058), 0x0);
	T_WRITE32((MIPI_CONFIG_BASE+0x05C), 0x0);

	T_WRITE32((MIPI_CONFIG_BASE+0x004), 0x820);
	T_WRITE32((MIPI_CONFIG_BASE+0x008), 0x400);
	T_WRITE32((MIPI_CONFIG_BASE+0x00C), 0x100);
	T_WRITE32((MIPI_CONFIG_BASE+0x010), 0x100);
	T_WRITE32((MIPI_CONFIG_BASE+0x014), 0x100);

	T_WRITE32((MIPI_CONFIG_BASE+0x040), 0x80);
	T_WRITE32((MIPI_CONFIG_BASE+0x064), 0x0);
	T_WRITE32((MIPI_CONFIG_BASE+0x074), 0x0);
	T_WRITE32((MIPI_CONFIG_BASE+0x080), 0x0);
	T_WRITE32((MIPI_CONFIG_BASE+0x084), 0x0);
	T_WRITE32((MIPI_CONFIG_BASE+0x088), 0x0);
	T_WRITE32((MIPI_CONFIG_BASE+0x090), 0x0);

	T_WRITE32((MIPI_CONFIG_BASE+0x064), 0x300);

	CVBS_LOG(" MIPIPLL Initialed. \n");

	//Setting MIPI PLL
	T_WRITE32((MIPI_CONFIG_BASE+0x068), 0x3);
	T_WRITE32((MIPI_CONFIG_BASE+0x068), 0x1);
	T_WRITE32((MIPI_CONFIG_BASE+0x044), T_REG32((MIPI_CONFIG_BASE+0x044)) | 0x00000013);
	T_WRITE32((MIPI_CONFIG_BASE+0x040), (T_REG32((MIPI_CONFIG_BASE+0x040)) | 0x00000002));
	T_WRITE32((MIPI_CONFIG_BASE+0x000), ((T_REG32((MIPI_CONFIG_BASE+0x000)) & 0xfffffbff ) | 0x00000003));
	T_WRITE32((MIPI_CONFIG_BASE+0x050), ((prediv << 1) |(txdiv0 << 3) |(txdiv1 << 5) |(posdiv << 7)) );
	T_WRITE32((MIPI_CONFIG_BASE+0x054), (0x3 | (sdm_ssc_en<<2) | (sdm_ssc_prd<<16)) );
	T_WRITE32((MIPI_CONFIG_BASE+0x058), txmul);
	T_WRITE32((MIPI_CONFIG_BASE+0x05C), ((sdm_ssc_delta<<16) | sdm_ssc_delta1));

	T_WRITE32((MIPI_CONFIG_BASE+0x004), T_REG32(MIPI_CONFIG_BASE+0x004) |0x00000001 |(loopback_en<<1));
	if(lane0_en)
		T_WRITE32((MIPI_CONFIG_BASE+0x008), T_REG32(MIPI_CONFIG_BASE+0x008) |0x00000001 |(loopback_en<<1));
	if(lane1_en)
		T_WRITE32((MIPI_CONFIG_BASE+0x00C), T_REG32(MIPI_CONFIG_BASE+0x00C) |0x00000001 |(loopback_en<<1));
	if(lane2_en)
		T_WRITE32((MIPI_CONFIG_BASE+0x010), T_REG32(MIPI_CONFIG_BASE+0x010) |0x00000001 |(loopback_en<<1));
	if(lane3_en)
		T_WRITE32((MIPI_CONFIG_BASE+0x014), T_REG32(MIPI_CONFIG_BASE+0x014) |0x00000001 |(loopback_en<<1));

	T_WRITE32((MIPI_CONFIG_BASE+0x050), (T_REG32((MIPI_CONFIG_BASE+0x050)) | 0x1));
	T_WRITE32((MIPI_CONFIG_BASE+0x060), 0);
	T_WRITE32((MIPI_CONFIG_BASE+0x060), 1);

	for(i=0; i<100; i++)   // wait for PLL stable
	{
		j = T_REG32((MIPI_CONFIG_BASE+0x050));
	}

	CVBS_LOG("MIPIPLL Exit. \n");
}

void vDPI0_480p(void)
{
        CVBS_DEF_LOG("[cvbs] Select DPI0 ouput 480P\n");

        *(volatile unsigned int *)0xF400d000 = 0x00000003;
        *(volatile unsigned int *)0xF400d004 = 0x00000000;
        *(volatile unsigned int *)0xF400d008 = 0x00000001;
        *(volatile unsigned int *)0xF400d00c = 0x00000007;

#if	MTK_CVBS_FORMAT_YUV
        *(volatile unsigned int *)0xF400d010 = 0x00410000;
        *(volatile unsigned int *)0xF400d014 = 0x00000002;
#else
        *(volatile unsigned int *)0xF400d010 = 0x00410040;
        *(volatile unsigned int *)0xF400d014 = 0x00000000;
#endif

        *(volatile unsigned int *)0xF400d0b0 = 0x02000000;
        *(volatile unsigned int *)0xF400d018 = 0x01e002d0;
        *(volatile unsigned int *)0xF400d020 = 0x0000003e;
        *(volatile unsigned int *)0xF400d024 = 0x0010003c;
        *(volatile unsigned int *)0xF400d028 = 0x00000006;
        *(volatile unsigned int *)0xF400d02c = 0x0009001e;
        *(volatile unsigned int *)0xF400d068 = 0x00000000;
        *(volatile unsigned int *)0xF400d06c = 0x00000000;
        *(volatile unsigned int *)0xF400d070 = 0x00000000;
        *(volatile unsigned int *)0xF400d074 = 0x00000000;
        *(volatile unsigned int *)0xF400d078 = 0x00000000;
        *(volatile unsigned int *)0xF400d07c = 0x00000000;

        *(volatile unsigned int *)0xF400df00 = 0x00000040;
        *(volatile unsigned int *)0xF400d0a0 = 0x00000000;

}

void vDPI0_576p(void)
{
        CVBS_DEF_LOG("[cvbs] Select DPI0 ouput 576P\n");

        *(volatile unsigned int *)0xF400d000 = 0x00000003;
        *(volatile unsigned int *)0xF400d004 = 0x00000000;
        *(volatile unsigned int *)0xF400d008 = 0x00000001;
        *(volatile unsigned int *)0xF400d00c = 0x00000007;

#if	MTK_CVBS_FORMAT_YUV
        *(volatile unsigned int *)0xF400d010 = 0x00410000; // [6] rgb2yuv
        *(volatile unsigned int *)0xF400d014 = 0x00000002; //[2:0] channel swap
#else
        *(volatile unsigned int *)0xF400d010 = 0x00410040; // [6] rgb2yuv
        *(volatile unsigned int *)0xF400d014 = 0x00000000; //[2:0] channel swap
#endif

        *(volatile unsigned int *)0xF400d0b0 = 0x02000000;
        *(volatile unsigned int *)0xF400d018 = 0x024002d0;//0x024002c0;//0x01e002d0;
        *(volatile unsigned int *)0xF400d020 = 0x00000040;//0x00000050;//0x0000003e;
        *(volatile unsigned int *)0xF400d024 = 0x000c0044;//0x0010003c;
        *(volatile unsigned int *)0xF400d028 = 0x00000005;//0x00000006;
        *(volatile unsigned int *)0xF400d02c = 0x00050027;//0x0009001e;
        *(volatile unsigned int *)0xF400d030 = 0x00010001;//add
        *(volatile unsigned int *)0xF400d068 = 0x00000000;
        *(volatile unsigned int *)0xF400d06c = 0x00000000;
        *(volatile unsigned int *)0xF400d070 = 0x00000000;
        *(volatile unsigned int *)0xF400d074 = 0x00000000;
        *(volatile unsigned int *)0xF400d078 = 0x00000000;
        *(volatile unsigned int *)0xF400d07c = 0x00000000;

        *(volatile unsigned int *)0xF400df00 = 0x00000040;
        *(volatile unsigned int *)0xF400d0a0 = 0x00000000;

}

void vDPI0_CB_Enable(BOOL enable)
{

	*(volatile unsigned int *)0xF400df00 = (enable == TRUE) ? 0x00000041 : 0x00000040;
}

void tve_tvdpll_dpi0_tve_path(void)
{
        CVBS_DEF_LOG("[cvbs]tvdpll_dpi0_tve\n");
        tve_set_tvdpll_108m();

        //enable TVE clock input/output
        T_Mask(0xF4000110, 0, ((1 << 6)|(1 << 7)));

        //disp path : DPI -> TVE
        T_Mask(0xF4000070, (1 << 0), TVE_SEL);

        // dpi0
        //enable DPI0 clock digital/engine
        T_Mask(0xF4000110, 0, ((1 << 2)|(1 << 3)));

        //DPI output clk form dpi0_ck
        T_Mask(0xF4000058, (1 << 0), (0x01 << 0));
        //TVE select DPI path, not test mode
        T_Mask(0xF0000024, (0 << 6), (0x03 << 6));

        //power on dpi0
        vWriteClkGenANAMsk(TOP_CLK_CFG_4, 0, PDN_DPI0);
         //DPI0 select tvdpll clk 108m/4=27m
        vWriteClkGenANAMsk(TOP_CLK_CFG_4, DPI0_MUX_CK, CLK_DPI0_SEL);
        udelay(100);
        vWriteClkGenANAMsk(TOP_CLK_CFG_4, DPI0_LVDS, CLK_DPI0_SEL);
        udelay(100);
        vWriteClkGenANAMsk(TOP_CLK_CFG_4, DPI0_LVDS_D8|CLK_DPI0_INV, CLK_DPI0_SEL|CLK_DPI0_INV);

        //power on tve
        vWriteClkGenANAMsk(TOP_CLK_CFG_5, 0, PDN_TVE);
        //TVE select tvdpll clk 108m/2=54m
        udelay(100);
        vWriteClkGenANAMsk(TOP_CLK_CFG_5, CLK26M, CLK_TVE_SEL);
        udelay(100);
        vWriteClkGenANAMsk(TOP_CLK_CFG_5, TVDPLL, CLK_TVE_SEL);
        udelay(100);
        vWriteClkGenANAMsk(TOP_CLK_CFG_5, TVDPLL_D2|CLK_TVE_INV, CLK_TVE_SEL|CLK_TVE_INV);
        udelay(100);
}

void tve_mipipll_dpi0_tve_path(void)
{
        CVBS_DEF_LOG("[cvbs]mipipll_dpi0_tve_path\n");
        DPI_MIPI_clk_setting(1115039586, 1, 0);

        //enable TVE clock input/output
        T_Mask(0xF4000110, 0, ((1 << 6)|(1 << 7)));

        //disp path : DPI -> TVE
        T_Mask(0xF4000070, (1 << 0), TVE_SEL);

        // dpi0
        //enable DPI0 clock digital/engine
        T_Mask(0xF4000110, 0, ((1 << 2)|(1 << 3)));

        //DPI output clk form dpi0_ck
        T_Mask(0xF4000058, (1 << 0), (0x01 << 0));
        //TVE select DPI path, not test mode
        T_Mask(0xF0000024, (0 << 6), (0x03 << 6));

        //DPI0 select MIPI clk 108m/4=27m
        vWriteClkGenANAMsk(TOP_CLK_CFG_4, DPI0_MUX_CK, CLK_DPI0_SEL);
        udelay(100);
        vWriteClkGenANAMsk(TOP_CLK_CFG_4, DPI0_MIPI, CLK_DPI0_SEL);
        udelay(100);
        vWriteClkGenANAMsk(TOP_CLK_CFG_4, DPI0_MIPI_D4|CLK_DPI0_INV, CLK_DPI0_SEL|CLK_DPI0_INV);

        //TVE select MIPI clk 108m/2=54m
        udelay(100);
        vWriteClkGenANAMsk(TOP_CLK_CFG_5, CLK26M, CLK_TVE_SEL);
        udelay(100);
        vWriteClkGenANAMsk(TOP_CLK_CFG_5, TVE_MIPI, CLK_TVE_SEL);
        udelay(100);
        vWriteClkGenANAMsk(TOP_CLK_CFG_5, TVE_MIPI_D2|CLK_TVE_INV, CLK_TVE_SEL|CLK_TVE_INV);
        udelay(100);
}

BOOL fgTVECVBSSetup_En(void)
{
	return _fgEnCVBSSetUp;
}

void Tve_HalSet75IRE(UINT8 u1Enable)
{
  if(u1Enable)
  {
    rTveAuxReg.uEncMode.rEncMode.fgSetup = 1;
  }
  else
  {
    rTveAuxReg.uEncMode.rEncMode.fgSetup = 0;
  }

  TVE_WRITE32(CVBS_TVE_MODE, rTveAuxReg.uEncMode.u4EncMode);
}

static UINT32 TVE_SetSetup(void)
{
	if(fgVideoIsNtsc(_ucCVBSOutFmt))
	{
		_arTveConf[TVE_2].fgSetupEnable = fgTVECVBSSetup_En();
		Tve_HalSet75IRE(_arTveConf[TVE_2].fgSetupEnable);
	}
	else
	{
		_arTveConf[TVE_2].fgSetupEnable = FALSE;
		Tve_HalSet75IRE(_arTveConf[TVE_2].fgSetupEnable);
	}

  return TVE_SET_OK;
}

void Tve_HalSetP2IBGN(UCHAR ucY2HBGN, UCHAR ucC2HBGN)
{
  rTveAuxReg.uEncSyn.rEncSyn.u4Y2HBgn = ucY2HBGN; // Y begin
  rTveAuxReg.uEncSyn.rEncSyn.u4C2HBgn = ucC2HBGN; // C begin

  TVE_WRITE32(CVBS_TVE_ENCSYN, rTveAuxReg.uEncSyn.u4EncSyn);
}

void Tve_HalSetCvbsDelay(UCHAR ucYDEL, UCHAR ucSYDEL, UCHAR ucCbDelay, UCHAR ucCrDelay)
{
	rTveAuxReg.uEncMode.rEncMode.u4YDelay = ucYDEL; //delay of input Y
	rTveAuxReg.uEncMode.rEncMode.u4SYDelay = ucSYDEL; //delay of input YG
	TVE_WRITE32(CVBS_TVE_MODE, rTveAuxReg.uEncMode.u4EncMode);

	rTveAuxReg.uYScale.rYScale.u4CbDelay = ucCbDelay;
	rTveAuxReg.uYScale.rYScale.u4CrDelay = ucCrDelay;
	TVE_WRITE32(CVBS_TVE_YSCALE, rTveAuxReg.uYScale.u4YScale);
}

static UINT32 TVE_SetDelay(void)
{
	//set CVBS channel delay
  Tve_HalSetCvbsDelay(au1YCDelay[(fgVideoIsNtsc(_ucCVBSOutFmt))?1:0][0], //Y delay   one sample
                      au1YCDelay[(fgVideoIsNtsc(_ucCVBSOutFmt))?1:0][1], //YG delay half sample
                      au1YCDelay[(fgVideoIsNtsc(_ucCVBSOutFmt))?1:0][2],
                      au1YCDelay[(fgVideoIsNtsc(_ucCVBSOutFmt))?1:0][3]);

  if(_ucCVBSOutFmt == TVE_RES_576P)
  {
    Tve_HalSetP2IBGN(0x0C, 0x0C);
  }
  else if(_ucCVBSOutFmt == TVE_RES_576I)
  {
    Tve_HalSetP2IBGN(0x0C, 0x0C);
  }
  else if(_ucCVBSOutFmt == TVE_RES_480P)
  {
    Tve_HalSetP2IBGN(0x06, 0x04);
  }
  else if(_ucCVBSOutFmt == TVE_RES_480I)
  {
    Tve_HalSetP2IBGN(0x04, 0x04);
	}

  return TVE_SET_OK;
}

//============================================
void TVE_SetYLvl(UCHAR ucYLvl)
{
	rTveAuxReg.uYScale.rYScale.u4YScale = ucYLvl;
	TVE_WRITE32(CVBS_TVE_YSCALE,  rTveAuxReg.uYScale.u4YScale);
}

void TVE_SetSyncLevel(UCHAR ucSyncLvl)
{
	rTveAuxReg.uEncSyn.rEncSyn.u4SdSynLvl = ucSyncLvl;
	TVE_WRITE32(CVBS_TVE_ENCSYN, rTveAuxReg.uEncSyn.u4EncSyn);
}

void TVE_HalSetColorBurstLvl(UCHAR ucBurstLvl)
{
	rTveAuxReg.uBurst1.rBurst1.u4BurstLvl = ucBurstLvl;
	TVE_WRITE32(CVBS_TVE_BURST,  rTveAuxReg.uBurst1.u4Burst1);
}

UINT32 TVE_SetColorBurstLvl(UCHAR ucBurstLvl)
{
  TVE_HalSetColorBurstLvl(ucBurstLvl);

  return TVE_SET_OK;
}

void TVE_HalSetCGain(UINT8 u1UVal, UINT8 u1VVal)
{
	TVE_WRITE32(CVBS_TVE_UGAIN, u1UVal);
	TVE_WRITE32(CVBS_TVE_VGAIN, u1VVal);
}

void  TVE_ResetHue(INT16 iDegree, INT16 iSaturation)
{
	INT16 iUVal, iVVal;
//	INT16 iCbVal, iCrVal;
	INT16 iUMax, iVMax;
	INT16 iUStep, iVStep;

	iUMax = 0xA0;
	iVMax = 0xA0;

	CVBS_LOG("iDegree = %d, iSaturation = %d \n", iDegree, iSaturation);

	if(_arTveConf[TVE_2].ucVDacConfig == VDAC_LOW_IMPEDANCE)
	{
		if(fgVideoIsNtsc(_ucCVBSOutFmt))
		{
			if(_fgEnCVBSSetUp)
			{
				iUVal = au1TVE_UVScale[0][0];
				iVVal = au1TVE_UVScale[0][1];
			}
			else
			{
				iUVal = 0x45;
				iVVal = 0x63;
			}
		}
		else
		{
			iUVal =  au1TVE_UVScale[1][0];
			iVVal =  au1TVE_UVScale[1][1];;
		}
	}
	else
	{
		iUVal = 0x45;
		iVVal = 0x62;
	}

  if(_arTveConf[TVE_2].fgSetupEnable)
  {
     iUVal = iUVal *925/1000;
     iVVal = iVVal*925/1000;
  }
  if(iDegree > 0)
  {
    iUStep = (iUMax - iUVal + 5) / 9; // +5 for a larger step, iUVAL~iCbVAL
    iVStep = (iVVal + 5) / 9;
  }
  else
  {
    iVStep = (iVMax - iVVal + 5) / 9;
    iUStep = (iUVal + 5) / 9;
  }

  iUVal += (iDegree * iUStep);
  iVVal -= (iDegree * iVStep);
  iUVal = iUVal * (iSaturation + 18) / 18;
  iVVal = iVVal * (iSaturation + 18) / 18;

  iUVal = (iUVal > iUMax) ? iUMax : iUVal;
  iUVal = (iUVal < 0) ? 0 : iUVal;
  iVVal = (iVVal > iVMax) ? iVMax : iVVal;
  iVVal = (iVVal < 0) ? 0 : iVVal;

  _ucUVal  = iUVal;
  _ucVVal  = iVVal;

  TVE_HalSetCGain((UINT8)iUVal, (UINT8)iVVal);
}

UCHAR ucHalTVECVBSYScale(void)
{
  UCHAR ucYscal;
  ucYscal= (UCHAR)(TVE_READ32(CVBS_TVE_YSCALE)&0xff);

  return ucYscal;
}

void vGetTveGammaCurveTable(UINT32 *prBuff)
{
  BYTE bIndex;

  for (bIndex = 0; bIndex<8; bIndex++)
  {
    *(prBuff+bIndex) =  pdCVBSGammaBase[bIndex];
  }
}

void TVE_HalSetGamma(UINT32  *pdVal)
{
  UINT32  i;
  UINT32 dVal;

  TVE_WRITE32(CVBS_TVE_GAMMA0, 0);

  for(i = 0; i < 4; i++)
  {
    dVal = (pdVal[i * 2] << 16) | (pdVal[i * 2 + 1]);
    TVE_WRITE32(CVBS_TVE_GAMMA1 + (i * 4), dVal);
  }
}

TVE_STATUS_T TVE_SetGamma(INT16 iGamma, INT16 iGContrast, INT16 iGBright )
{
	UINT32 pdwGammaVal[8];
	UINT32 dwNewVal;
	UINT32 bIdx;
	INT32  iTmpGVal;
	UINT32	iGBrightStep;
	UINT32	iMax;

	CVBS_LOG("Gamma para: %d , %d , %d \n",iGamma,iGContrast,iGBright);

	vGetTveGammaCurveTable(&pdwGammaVal[0]);

	if(ucHalTVECVBSYScale()==0)
		iMax = 0xfff;
	else
		iMax = 0xfff * 0x80 / ucHalTVECVBSYScale();

	if(iMax > 0xfff)
	{
		iMax = 0xfff;
	}

	iGContrast += 53;

	iGBrightStep = (0x400 - iGContrast*10)/20;
	iGBright = iGBright*iGBrightStep;

	for(bIdx = 0; bIdx < 8; bIdx++)
	{
		dwNewVal = pdwGammaVal[bIdx];
		iTmpGVal = (INT16)dwNewVal * iGContrast / 53 + iGBright;

		if(_arTveConf[TVE_2].fgNtsc)
		{
			iTmpGVal = iTmpGVal*714/700;
		}

		if(_arTveConf[TVE_2].fgSetupEnable)
		{
			iTmpGVal = iTmpGVal*925/1000;
		}

		if(iTmpGVal < 0)
		{
			iTmpGVal = 0;
		}

		if((UINT32)iTmpGVal > iMax)
		{
			iTmpGVal = iMax;
		}

		dwNewVal = (UINT32)iTmpGVal;
		pdwGammaVal[bIdx] = dwNewVal;
	}

	TVE_HalSetGamma(pdwGammaVal);

	return TVE_SET_OK;
}

TVE_STATUS_T TVE_SetBrightness(UCHAR ucLevel)
{
	CVBS_LOG("brightness level = 0x%x \n", ucLevel);

  _stTveVideoAdjust.i2BrightnessLevel = (((int)ucLevel+1) -128)*40/256;//( x-128)*32/256 where x=1...256
  TVE_SetGamma(0, _stTveVideoAdjust.i2ContrastLevel, _stTveVideoAdjust.i2BrightnessLevel);

  return TVE_SET_OK;
}

TVE_STATUS_T TVE_SetContrast(UCHAR ucLevel)
{
	CVBS_LOG("SetContrast level = 0x%x \n", ucLevel);

  _stTveVideoAdjust.i2ContrastLevel = (((int)ucLevel+1) -128)>>3;//( x-128)*40/256 where x=1...256	to -16~16
  TVE_SetGamma(0, _stTveVideoAdjust.i2ContrastLevel, _stTveVideoAdjust.i2BrightnessLevel);

  return TVE_SET_OK;
}

TVE_STATUS_T TVE_SetHue(UCHAR ucLevel)
{
	CVBS_LOG("SetHue level = 0x%x \n", ucLevel);

  _stTveVideoAdjust.i2HueLevel = (((int)ucLevel+1) -128)*18/256;//( x-128)*40/256 where x=1...256	to -9~9
  TVE_ResetHue(_stTveVideoAdjust.i2HueLevel, _stTveVideoAdjust.i2SaturationLevel);

  return TVE_SET_OK;
}

TVE_STATUS_T TVE_SetSaturation(UCHAR ucLevel)
{
	CVBS_LOG("SetSaturation level = 0x%x \n", ucLevel);

  _stTveVideoAdjust.i2SaturationLevel = (((int)ucLevel+1) -128)*18/256;//( x-128)*40/256 where x=1...256	to -9~9

  TVE_ResetHue(_stTveVideoAdjust.i2HueLevel, _stTveVideoAdjust.i2SaturationLevel);

  return TVE_SET_OK;
}

UINT32 TVE_CVBSSetup_En(BOOL fgEnable)
{
	if(fgEnable)
		_fgEnCVBSSetUp = TRUE;
	else
		_fgEnCVBSSetUp = FALSE;

	TVE_SetSetup();
	TVE_SetGamma(0, _stTveVideoAdjust.i2ContrastLevel, _stTveVideoAdjust.i2BrightnessLevel);
	TVE_ResetHue(_stTveVideoAdjust.i2HueLevel, _stTveVideoAdjust.i2SaturationLevel);

  return TVE_SET_OK;
}

void TVE_HalSetCVBSPal60(BOOL fgEnable)
{
	if(fgEnable)
	{
		rTveAuxReg.uBurst1.rBurst1.fgPal60 = TRUE;
		TVE_WRITE32(CVBS_TVE_BURST, PAL60_EN | TVE_READ32(CVBS_TVE_BURST));
	}
	else
	{
		rTveAuxReg.uBurst1.rBurst1.fgPal60 = FALSE;
		TVE_WRITE32(CVBS_TVE_BURST, (~PAL60_EN)& TVE_READ32(CVBS_TVE_BURST));
	}
}

void TVE_CVBSPal60_En(BOOL fgEnable)
{
	if(fgEnable)
	{
		_fgEnCVBSPal60 = TRUE ;
	}
	else
	{
		_fgEnCVBSPal60 = FALSE;
	}

	TVE_HalSetCVBSPal60(fgEnable);
}

void TVE_PalLineMissing(UCHAR ucFmt)
{
	if(!fgVideoIsNtsc(ucFmt))
	{
    rTveAuxReg.uMuxCtrl.rMuxCtrl.fgPalLineMissing = 1;
	}
	else
	{
		rTveAuxReg.uMuxCtrl.rMuxCtrl.fgPalLineMissing = 0;
	}

  TVE_WRITE32(CVBS_TVE_MUX_CTRL, rTveAuxReg.uMuxCtrl.u4MuxCtrl);
}

void TVE_PalBDBurstError(UCHAR ucFmt)
{
	if((!fgVideoIsNtsc(ucFmt))&&(rTveAuxReg.uMuxCtrl.rMuxCtrl.u4TvType == 3))
	{
		rTveAuxReg.uMuxCtrl.rMuxCtrl.fgPalBurstError = 1;
	}
	else
	{
		rTveAuxReg.uMuxCtrl.rMuxCtrl.fgPalBurstError = 0;
	}

	TVE_WRITE32(CVBS_TVE_MUX_CTRL, rTveAuxReg.uMuxCtrl.u4MuxCtrl);
}

TVE_STATUS_T TVE_DACPower(UCHAR ucEnable)
{
	CVBS_DEF_LOG("[cvbs] DACPower is %s \n", ucEnable == TRUE ? "ON" :"OFF");

  if(ucEnable)
  {
		//if(_arTveConf[TVE_2].ucVDacConfig == VDAC_LOW_IMPEDANCE)
		{
			vWriteMipiDacMsk(TVE_CTL0, (0 << 0)|(1 << 3)|(0x04 << 8), RG_BGREF_PWD|RG_TRIM_REV|RG_TRIM_VAL|RG_VDAC_REV|RG_PBS_TST_EN|RG_VDAC_TST_EN);
			vWriteDispSysMsk(DISP_TVE_SYS_CFG_00,((1<<6)|(1<<8)|(1<<16)), TVE_PRGS_SEL|TVE_VDAC_CLK_INV|TVE_VDAC_TEST_SEL|TVE_CKGEN_TVE_CLK);
		}
		/*
		else
		{
			T_Mask(TVE_CTL0, (0 << 0)|(VDAC_HIGH_IMPEDANCE << 3)|(0 << 17)|(0 << 18), RG_BGREF_PWD|RG_TRIM_VAL|RG_PBS_TST_EN|RG_VDAC_TST_EN);
			T_Mask(DISP_TVE_SYS_CFG_00, (1 << 8)|(0 << 16)|(0 << 17), TVE_PRGS_SEL|TVE_VDAC_CLK_INV|TVE_VDAC_TEST_SEL);
		}
		*/
		#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT//auto detect
                if(cvbs_auto_detect_enable) //check auto detect enable?
                {
		rTveAuxReg.uDispArea.rDispArea.u4ForAutoDetect = 0x1;
		TVE_WRITE32(CVBS_TVE_DSIP_AREA, rTveAuxReg.uDispArea.u4DispArea);
		rTveAuxReg.uDacControl.rDacControl.u4DAC1 = 0x1;
		rTveAuxReg.uDacControl.rDacControl.fgPlugOnEn = 0x1;
                rTveAuxReg.uDacControl.rDacControl.u4plugref = 0x3;
                }
                else
                    rTveAuxReg.uDacControl.rDacControl.u4DAC1 = 0x2;
		#else
		rTveAuxReg.uDacControl.rDacControl.u4DAC1 = 0x2;
		#endif

		//rTveAuxReg.uDacControl.rDacControl.fgPlugOnEn = 0x1;
		//rTveAuxReg.uDacControl.rDacControl.u4IbandPlugDet = 0x3;
		rTveAuxReg.uDacControl.rDacControl.u4IbandPlugDet = 0x2;
		rTveAuxReg.uDacControl.rDacControl.u4u4IbandX = 0x3;

		TVE_WRITE32(CVBS_TVE_DACTRL, rTveAuxReg.uDacControl.u4DacCtrl);
  }
  else
  {
		rTveAuxReg.uDacControl.rDacControl.u4DAC1 = 0x0;

		TVE_WRITE32(CVBS_TVE_DACTRL, rTveAuxReg.uDacControl.u4DacCtrl);
  }

	return TVE_SET_OK;
}

TVE_STATUS_T TVE_SetEnable(UCHAR ucEnable)
{
  if(ucEnable)
  {
	  rTveAuxReg.uEncMode.rEncMode.fgEncOff = 0;

		TVE_WRITE32(CVBS_TVE_MODE, rTveAuxReg.uEncMode.u4EncMode);
  }
  else
  {
	  rTveAuxReg.uEncMode.rEncMode.fgEncOff = 1;

		TVE_WRITE32(CVBS_TVE_MODE, rTveAuxReg.uEncMode.u4EncMode);
  }

	return TVE_SET_OK;
}

TVE_STATUS_T TVE_Reset(void)
{
	// TVE_2 (Aux_CVBS_SVIDEO)
	memset(((void *)&rTveAuxReg), 0, sizeof(HAL_8127_CVBS_TVE_FIELD_T));

	rTveAuxReg.uEncSyn.rEncSyn.u4Rst = 3;
	TVE_WRITE32(CVBS_TVE_ENCSYN, rTveAuxReg.uEncSyn.u4EncSyn);

	rTveAuxReg.uEncSyn.rEncSyn.u4Rst = 0;
	TVE_WRITE32(CVBS_TVE_ENCSYN, rTveAuxReg.uEncSyn.u4EncSyn);

	return TVE_SET_OK;
}

/************************************************************************
     Function : void TVE_HalSetMv(UCHAR bType,UCHAR ucComponentEnable)
  Description : Set MacroVision parameters
    Parameter : bType: MacroVision type (4 = PAL)
    Return    : NONE
************************************************************************/
void TVE_HalSetMv(UCHAR bType, UCHAR ucComponentMvEnable)
{
  UINT8   i;
  UINT32 dwMv6;
  UINT32 dwCVBSMv6 = 0;

	CVBS_DEF_LOG("[cvbs] APS = %d[%s] \n", bType, (ucComponentMvEnable ? "TRUE" : "FALSE"));

  //enable macrovision clock
  vWriteDispSysMsk(DISP_TVE_SYS_CFG_00,TVE_CKGEN_MV, TVE_CKGEN_MV);

  dwMv6 = TVE_READ32(CVBS_TVE_MV6) & (MVOFF | CPNTMVOFF | MUVSW);

  if(ucComponentMvEnable)
  {
    dwMv6 &= (~CPNTMVOFF);
  }
  else
  {
    dwMv6 |= CPNTMVOFF;
  }

  if(bType == 0)
  {
    for(i = 0; i < 6; i++)
    {
      TVE_WRITE32(CVBS_TVE_MV1+ (i * 4), 0);
    }

    TVE_WRITE32(CVBS_TVE_MV6, dwMv6);
    TVE_WRITE32(CVBS_TVE_MV7, (TVE_READ32(CVBS_TVE_MV7) & ~(_pdwMvVal[4][0])));
  }
  else if(bType < 6)
  {
    for(i = 0; i < 5; i++)
    {
      TVE_WRITE32(CVBS_TVE_MV1 + (i * 4), _pdwMvVal[bType - 1][i]);
    }

		if(1/*(_arTveConf[TVE_1].ucMediaType == MEDIA_TYPE_DME) || (_arTveConf[TVE_1].ucMediaType == MEDIA_TYPE_VOD)*/)
		{
			switch(_arTveConf[TVE_2].ucAps)
			{
			  case 1:
				TVE_WRITE32(CVBS_TVE_MV5 , (_pdwMvVal[bType - 1][4]&0xFFFFFF00)|0x40); // 4line Back porch pulses
				break;
			  case 2:
				TVE_WRITE32(CVBS_TVE_MV5 , _pdwMvVal[bType - 1][4]); // 6line 	Back porch pulses
				break;
			  case 3:
				TVE_WRITE32(CVBS_TVE_MV5 , (_pdwMvVal[bType - 1][4]&0xFFFFFF00)|0x50); // 5line Back porch pulses
				break;
			}
		}

    dwMv6 |= _pdwMvVal[bType - 1][5];
    TVE_WRITE32(CVBS_TVE_MV6, dwMv6);

    dwMv6 = (TVE_READ32(CVBS_TVE_MV7) & ~MV_AGC_BP_MASK);

    switch (bType)
    {
	  	case 4: //PAL
		  	dwMv6 |= (MV_AGCLVL_PAL|MV_BPLVL_PAL);
				dwCVBSMv6=dwMv6;
	 	  	break;

      case 1:
      case 2:
      case 3:
      case 5: //p-scan
      default:
		  	dwCVBSMv6=dwMv6;
		  	dwCVBSMv6 |= (MV_AGCLVL_NTSC_CVBS|MV_BPLVL_NTSC_CVBS);
        break;
    }

    TVE_WRITE32(CVBS_TVE_MV7, dwCVBSMv6 | (_pdwMvVal[4][0]));
  }
}
extern u32 get_devinfo_with_index(u32 index);
UINT32 TVE_SetMacroVision(UCHAR ucAps)
{
  UCHAR ucType;
  u32 temp;

//0x10206040[5]
  temp = get_devinfo_with_index(3);

    if((temp & (1 << 13)) == 0) //here reture for donot support Macrovison should check latter
  	{
        CVBS_DEF_LOG("[cvbs]no MV func, FUC: %s line %d \n",__FUNCTION__,__LINE__);
  	 return TVE_SET_ERROR;
  	}

	CVBS_DEF_LOG("[cvbs] Set MacroVision type is = 0x%x(%s : %s) \n", _arTveConf[TVE_2].ucAps, (_arTveConf[TVE_2].fgNtsc ? "NTSC" :"PAL"), (TVE_MOD_COMPOSITE == _arTveConf[TVE_2].ucMode ? "CVBS Output" :"UNKNOW"));

  ucType = _arTveConf[TVE_2].ucAps;

  if(ucType != 0)
  {
    if(!_arTveConf[TVE_2].fgNtsc)
    {
      ucType = 4;
    }
  }

  if(_arTveConf[TVE_2].ucMode != TVE_MOD_COMPOSITE)
  {
    TVE_HalSetMv(ucType,FALSE);
  }
  else
  {
    TVE_HalSetMv(ucType,TRUE);
  }

  return TVE_SET_OK;
}
void TVE_SetAsBlack(BOOL fgBlkScrEn)
{
	//UINT32  u4RegUVal;
	//UINT32  u4RegVVal;

	if(fgBlkScrEn == TRUE)	// Enable Black screen
	{
		TVE_WRITE32(CVBS_TVE_YSCALE, (TVE_READ32(CVBS_TVE_YSCALE)&0xFFFFFF00) | 0x00);
		TVE_WRITE32(CVBS_TVE_UGAIN, 0x00000000);
		TVE_WRITE32(CVBS_TVE_VGAIN, 0x00000000);
	}
	else	// Disable Black screen  (Default)
	{
		TVE_WRITE32(CVBS_TVE_YSCALE, (TVE_READ32(CVBS_TVE_YSCALE)&0xFFFFFF00) | rTveAuxReg.uYScale.rYScale.u4YScale);
		TVE_WRITE32(CVBS_TVE_UGAIN, _ucUVal);
		TVE_WRITE32(CVBS_TVE_VGAIN, _ucVVal);
	}
}

void TVE_SetDispArea(UCHAR ucOn, UINT16 bgn, UINT16 end)
{
  rTveAuxReg.uDispArea.rDispArea.fgNewDinEn = ucOn;
  rTveAuxReg.uDispArea.rDispArea.u4DispBgnPxl = bgn;
  rTveAuxReg.uDispArea.rDispArea.u4DispEndPxl = end;

  TVE_WRITE32(CVBS_TVE_DSIP_AREA, rTveAuxReg.uDispArea.u4DispArea);
}

void TVE_SetSyncTime0(UCHAR ucOn, UINT16 vtotal, UINT16 htotal)
{
 // rTveAuxReg.uSyncCtrl.rSyncCtrl.fgAdjSelfEn = 0;
  rTveAuxReg.uSyncCtrl.rSyncCtrl.fgAdjTotalEn = ucOn;
  rTveAuxReg.uSyncTime0.rSyncTime0.u4AdjHTotal = htotal;
  rTveAuxReg.uSyncTime0.rSyncTime0.u4AdjVTotal = vtotal;

  TVE_WRITE32(CVBS_TVE_SYNC_CTRL, rTveAuxReg.uSyncCtrl.u4SyncCtrl);
  TVE_WRITE32(CVBS_TVE_SYNC_TIME0, rTveAuxReg.uSyncTime0.u4SyncTime0);
}

void TVE_SetSyncTime1(UCHAR ucOn,UCHAR slf_run,UINT16 vsync, UINT16 hsync)
{
    rTveAuxReg.uSyncCtrl.rSyncCtrl.fgAdjSelfEn = slf_run;
	rTveAuxReg.uSyncCtrl.rSyncCtrl.fgAdjSyncEn = ucOn;
	rTveAuxReg.uSyncTime1.rSyncTime1.u4AdjVsync = vsync;
	rTveAuxReg.uSyncTime1.rSyncTime1.u4AdjHsync = hsync;

	TVE_WRITE32(CVBS_TVE_SYNC_CTRL, rTveAuxReg.uSyncCtrl.u4SyncCtrl);
	TVE_WRITE32(CVBS_TVE_SYNC_TIME1, rTveAuxReg.uSyncTime1.u4SyncTime1);
}

void TVE_SetSyncTime2(UCHAR ucOn, UINT16 oend, UINT16 ostart)
{
  UINT32 synctime2;

  synctime2 = ((UINT32)oend<<16) | (UINT32)ostart;
  rTveAuxReg.uSyncCtrl.rSyncCtrl.fgSelfYActiveEn = ucOn;

  TVE_WRITE32(CVBS_TVE_SYNC_CTRL, rTveAuxReg.uSyncCtrl.u4SyncCtrl);
  TVE_WRITE32(CVBS_TVE_SYNC_TIME2, synctime2);
}

void TVE_HalColorBar(UCHAR ucOn)
{
  rTveAuxReg.uEncMode.rEncMode.fgCbOn = ucOn;
	rTveAuxReg.uMuxCtrl.rMuxCtrl.fgPrgsSelfEn = ucOn;

  TVE_WRITE32(CVBS_TVE_MUX_CTRL, rTveAuxReg.uMuxCtrl.u4MuxCtrl);
  TVE_WRITE32(CVBS_TVE_SYNC_CTRL, rTveAuxReg.uSyncCtrl.u4SyncCtrl);
  TVE_WRITE32(CVBS_TVE_MODE, rTveAuxReg.uEncMode.u4EncMode);
}

void TVE_SetColorBar(UCHAR ucOn)
{
	CVBS_LOG("SetColorBar is %s \n", (ucOn == TRUE) ? "On" : "Off");

	//TVE_Init();
	TVE_SetFmt(TVE_RES_576P, TVE_RES_576P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
       TVE_SetEnable(TRUE);
	TVE_SetSyncTime1(TRUE,TRUE,0x008, 0x32b);

  TVE_HalColorBar(ucOn);
}

void TVE_SetVpsEnable(UCHAR ucEnable)
{
	if(ucEnable == TRUE)
	{
		rTveAuxReg.uVpsCtrl1.rVpsCtrl1.fgVpsOn = 0x01;
		rTveAuxReg.uVpsCtrl1.rVpsCtrl1.fgWaveformType = 0x0;
		rTveAuxReg.uVpsCtrl1.rVpsCtrl1.u4VpsP1 = 0xbd ;
		rTveAuxReg.uVpsCtrl1.rVpsCtrl1.u4VpsP2 = 0x5ce ;
		rTveAuxReg.uVpsData1.u4VpsData1 = 0x0;
		rTveAuxReg.uVpsData2.u4VpsData2 = 0x0;
		rTveAuxReg.uVpsData3.u4VpsData3 = 0x0;
		rTveAuxReg.uVpsCtrl2.rVpsCtrl2.u4VpsData4 = 0x0;
		rTveAuxReg.uVpsCtrl2.rVpsCtrl2.u4VpsBgnPxl = 0x0 ;
		rTveAuxReg.uVpsCtrl2.rVpsCtrl2.u4VpsEndPxl = 0x13 ;
		rTveAuxReg.uVpsCtrl2.rVpsCtrl2.u4VpsBgnLine = 0x10 ;

		TVE_WRITE32(CVBS_TVE_VPS_CTRL1, rTveAuxReg.uVpsCtrl1.u4VpsCtrl1);
		TVE_WRITE32(CVBS_TVE_VPS_DATA1, rTveAuxReg.uVpsData1.u4VpsData1);
		TVE_WRITE32(CVBS_TVE_VPS_DATA2, rTveAuxReg.uVpsData2.u4VpsData2);
		TVE_WRITE32(CVBS_TVE_VPS_DATA3, rTveAuxReg.uVpsData3.u4VpsData3);
		TVE_WRITE32(CVBS_TVE_VPS_DATA4_VPS_CTRL2, rTveAuxReg.uVpsCtrl2.u4VpsCtrl2);

		CVBS_LOG("VPS ON \n");
	}
	else
	{
		rTveAuxReg.uVpsCtrl1.rVpsCtrl1.fgVpsOn = 0x00;
		CVBS_LOG("VPS OFF \n");
	}
}

void TVE_BitInverse(UINT32 u4InputData, UINT32 *pu4OutputData)
{
	UINT8 i = 0;
	UINT32 u4TempValue = 0, u4TempResult = 0;

	for(; i<32; i++)
	{
		u4TempValue = u4InputData & 1;
		u4InputData = u4InputData >> 1;
		u4TempResult =  u4TempResult | (u4TempValue << (31 - i));
	}

	*pu4OutputData = u4TempResult;
}

void TVE_HalSendVPSValue(const UCHAR *pucData)
{
	UINT32 u4TempValue = 0, u4InverseRealData = 0;
	UINT8 i = 0, j = 0;

	for(; i<4; i++)
	{
		if(j == 12)
		{
			u4TempValue = (UINT32)pucData[j];
			TVE_BitInverse(u4TempValue, &u4InverseRealData);
			u4InverseRealData = u4InverseRealData | (TVE_READ32(CVBS_TVE_VPS_DATA4_VPS_CTRL2) & 0x00ffffff);
			TVE_WRITE32(CVBS_TVE_VPS_DATA4_VPS_CTRL2,u4InverseRealData);

			break;
		}

		u4TempValue = (UINT32)(pucData[j]) | (UINT32)(pucData[j+1] << 8) | (UINT32)(pucData[j+2] << 16) | (UINT32)(pucData[j+3] << 24);
		TVE_BitInverse(u4TempValue, &u4InverseRealData);
		TVE_WRITE32(CVBS_TVE_VPS_DATA1 + i*4, u4InverseRealData);

		j = j + 4;
	}
}

UINT32 TVE_SendVPS(const UCHAR *pucData)
{
  UINT32 u4RetVal = TVE_SET_OK;

	if(pucData != NULL)
	{
		TVE_HalSendVPSValue(pucData);
	}
	else
	{
		u4RetVal = TVE_SET_ERROR;
	}

	return u4RetVal;
}

const UINT32 _pdwCVBSYLPFi[6][5] =
{
	 {0x01020201, 0xFBF8FAFE, 0x0E120C02, 0xE6DFECFF, 0x84653404}, // SD STRONG
	 {0x01020201, 0xFCF9FBFE, 0x0C100B03, 0xE8E1ECFE, 0x84663607}, // SD MIDDLE
	 #if 1
	 {0x01020201, 0x10FAFCFA, 0x0600FE03, 0xEA03ECFE, 0x846739EC}, // SD FLAT
	 #else
	 {0x01020201, 0xFCFAFBFE, 0x040C0C04, 0xECE4EAFB, 0x8478350E}, // SD FLAT
	 #endif
	 {0xF9FE0205, 0x0F05FCF8, 0xF50A1617, 0xE8D1CFDE, 0x8C714411}, // SD CVBS scale Down Use
	 {0xff000001, 0xfafbfdfe, 0xfffbf9f9, 0x21170d05, 0x3e3a342b}, // SD	4M, -15dB
};

const UINT32 SD_PPF_480I_time[]=
{
 0x020B002A,
 0x0356006C,
 0x0356006C,
 0x00408040,
 0x00555655,
 0x000040C0,
};

const UINT32 SD_PPF_576I_time[]=
{
 0x026D002C,
 0x0356006C,
 0x0356006C,
 0x00408040,
 0x00555655,
 0x000040C0,
};

void TVE_SetFilter(unsigned char select)
{
        TVE_WRITE32(CVBS_Y_FILTER_COEFF_GROUP_1,_pdwCVBSYLPFi[select][0]);
        TVE_WRITE32(CVBS_Y_FILTER_COEFF_GROUP_2,_pdwCVBSYLPFi[select][1]);
        TVE_WRITE32(CVBS_Y_FILTER_COEFF_GROUP_3,_pdwCVBSYLPFi[select][2]);
        TVE_WRITE32(CVBS_Y_FILTER_COEFF_GROUP_4,_pdwCVBSYLPFi[select][3]);
        TVE_WRITE32(CVBS_Y_FILTER_COEFF_GROUP_5,_pdwCVBSYLPFi[select][4]);
}
void TVE_SD_PPF_enbale(unsigned char en,unsigned char ucFmt)
{
        if(en == 0)
        {
                TVE_WRITE32(CVBS_CONTROL,TVE_READ32(CVBS_CONTROL) & (~RG_SD_PPF_ON_MASK));
        }
        else
        {
                TVE_WRITE32(CVBS_CONTROL,TVE_READ32(CVBS_CONTROL) |RG_SD_PPF_ON_MASK);

                if(fgVideoIsNtsc(ucFmt))
                {
                        TVE_WRITE32(CVBS_SD_PPF_VSTORE,SD_PPF_480I_time[0]);
                        TVE_WRITE32(CVBS_SD_PPF_Y_HSTORE,SD_PPF_480I_time[1]);
                        TVE_WRITE32(CVBS_SD_PPF_C_HSTORE,SD_PPF_480I_time[2]);
                        TVE_WRITE32(CVBS_SD_PPF_C_COEF,SD_PPF_480I_time[3]);
                        TVE_WRITE32(CVBS_SD_PPF_Y_COEF,SD_PPF_480I_time[4]);
                        TVE_WRITE32(CVBS_FIRST_SECOND_COEF,SD_PPF_480I_time[5]);
                }
                else
                {
                        TVE_WRITE32(CVBS_SD_PPF_VSTORE,SD_PPF_576I_time[0]);
                        TVE_WRITE32(CVBS_SD_PPF_Y_HSTORE,SD_PPF_576I_time[1]);
                        TVE_WRITE32(CVBS_SD_PPF_C_HSTORE,SD_PPF_576I_time[2]);
                        TVE_WRITE32(CVBS_SD_PPF_C_COEF,SD_PPF_576I_time[3]);
                        TVE_WRITE32(CVBS_SD_PPF_Y_COEF,SD_PPF_576I_time[4]);
                        TVE_WRITE32(CVBS_FIRST_SECOND_COEF,SD_PPF_576I_time[5]);
                }
        }
}
UINT32 dwMirror(UINT32 dwVal, UINT32 dwBitNum)
{
  UINT32 i;
  UINT32 dwOut = 0;
  UINT32 dwMask = 0;
  UINT32 dwShift;


  dwMask = 1;

  dwShift = 0;

  for(i = 0; i < dwBitNum; i++)
  {
    dwOut = (dwOut << 1);

    dwOut |= ((dwVal & dwMask) >> dwShift);

    dwShift++;
    dwMask = (dwMask << 1);
  }
  return(dwOut);

}


UINT32 dwCRCC(UINT32 ui4Input)
{
  UINT32 ui4Output;
  UINT32 ui4Feedback;
  UINT8   i;

  ui4Output = 0x3f;  // preset to all 1

  for(i = 0; i < 14; i++)
  {
    ui4Feedback = (ui4Output & 0x1) ^ ((ui4Input >> i) & 0x1); // only bit 0 has value
    ui4Output >>= 1; // bits 0-4 have values
    ui4Output ^= (ui4Feedback << 4);  // bit 4 XOR with feedback
    ui4Output |= (ui4Feedback << 5);  // bit 5 set to feedback
  }

  return(ui4Output << 14);
}

void TVE_HalSetInterlaceVbi(UINT32 ui4RegVal, UCHAR ucComponentVbiEnable)
{
  ui4RegVal |= 0xC8000000;
  TVE_WRITE32(CVBS_TVE_WSSI, ui4RegVal |VBI_ON);
}

static void vSetNtscVbiSignal(void)
{
   UINT32 dwInterlaceVbi = 0;
   UINT32 ui4InterlaceCgms = 0;

   switch(_arTveConf[TVE_2].ucAspectRatio )
   {
     case ASPECT_RATIO_16_9_FULL_FORMAT:
       dwInterlaceVbi = CGMS_NOR_16_9;
       break;
     case ASPECT_RATIO_4_3:
     case ASPECT_RATIO_14_9_LB_CENTRE:
     case ASPECT_RATIO_14_9_LB_TOP:
     case ASPECT_RATIO_16_9_LB_TOP:
     case ASPECT_RATIO_14_9_FULL_FORMAT:
       dwInterlaceVbi = CGMS_NOR_4_3;
       break;
     case ASPECT_RATIO_16_9_LB_CENTRE:
     case ASPECT_RATIO_LARGE_16_9_LB_CENTRE:
       dwInterlaceVbi = CGMS_LB_4_3;
       break;
     default:
       dwInterlaceVbi = CGMS_NOR_4_3;
       break;
   }

   if(_arTveConf[TVE_2].ucCgmsaInfo == IBC_CPS_INFO_CGMS_COPY_PERMITTED)
   {
     ui4InterlaceCgms = TVE_480I_CGMS_COPY_PERMITTED;
   }
   else if(_arTveConf[TVE_2].ucCgmsaInfo == IBC_CPS_INFO_CGMS_NO_MORE_COPY)
   {
     ui4InterlaceCgms = TVE_480I_CGMS_RESERVED;	//
   }
   else if(_arTveConf[TVE_2].ucCgmsaInfo == IBC_CPS_INFO_CGMS_ONE_COPY_ALLOWED)
   {
     ui4InterlaceCgms = TVE_480I_CGMS_ONE_COPY_ALLOWED;
   }
   else if(_arTveConf[TVE_2].ucCgmsaInfo == IBC_CPS_INFO_CGMS_NO_COPY_PERMITTED)
   {
     ui4InterlaceCgms = TVE_480I_CGMS_NO_COPY_PERMITTED;
   }
   else if(_arTveConf[TVE_2].ucCgmsaInfo == IBC_CPS_INFO_CGMS_RESERVED)
   {
     ui4InterlaceCgms = TVE_480I_CGMS_RESERVED;
   }

   dwInterlaceVbi = dwInterlaceVbi | ((_arTveConf[TVE_2].ucAps & 0x1) << 9);
   dwInterlaceVbi = dwInterlaceVbi | ((_arTveConf[TVE_2].ucAps & 0x2) << 7);

   dwInterlaceVbi = dwInterlaceVbi | ((ui4InterlaceCgms & 0x1) << 7);
   dwInterlaceVbi = dwInterlaceVbi | ((ui4InterlaceCgms & 0x2) << 5);

   dwInterlaceVbi = dwInterlaceVbi | ((_arTveConf[TVE_2].ucAnalogSrc & 0x1) << 10);

   dwInterlaceVbi |= dwCRCC(dwInterlaceVbi & 0x3fff);   // calculate the CRCC
   dwInterlaceVbi = ((dwInterlaceVbi & 0xfff00000) | ((dwMirror((dwInterlaceVbi & 0xfffff), 20)) & 0xfffff));

   if(_arTveConf[TVE_2].ucMode == TVE_MOD_OFF)
   {
     TVE_HalSetInterlaceVbi(dwInterlaceVbi, FALSE);
   }
   else
   {
     TVE_HalSetInterlaceVbi(dwInterlaceVbi, TRUE); // For CVBS & CAV 525i output at first
   }

}
//===========================================

static void vSetPalVbiSignal(void)
{
  UINT32 dwVbi = 0;
  UINT32 ui4Cgms = 0;

   dwVbi |= ASPECT_RATIO_2_WSS[_arTveConf[TVE_2].ucAspectRatio];

   if(_arTveConf[TVE_2].ucCgmsaInfo == IBC_CPS_INFO_CGMS_COPY_PERMITTED)
   {
     ui4Cgms = TVE_576_CGMS_COPY_PERMITTED;
   }
   else if(_arTveConf[TVE_2].ucCgmsaInfo == IBC_CPS_INFO_CGMS_NO_MORE_COPY)
   {
     ui4Cgms = TVE_576_CGMS_RESERVED;	//TVE_576_CGMS_NO_COPY_PERMITTED;
   }
   else if(_arTveConf[TVE_2].ucCgmsaInfo == IBC_CPS_INFO_CGMS_ONE_COPY_ALLOWED)
   {
     ui4Cgms = TVE_576_CGMS_ONE_COPY_ALLOWED;
   }
   else if(_arTveConf[TVE_2].ucCgmsaInfo == IBC_CPS_INFO_CGMS_NO_COPY_PERMITTED)
   {
     ui4Cgms = TVE_576_CGMS_NO_COPY_PERMITTED;
   }
   else if(_arTveConf[TVE_2].ucCgmsaInfo == IBC_CPS_INFO_CGMS_RESERVED)
   {
     ui4Cgms = TVE_576_CGMS_RESERVED;
   }

  dwVbi = dwVbi | ((ui4Cgms & 0x1) << 13);
  dwVbi = dwVbi | ((ui4Cgms & 0x2) << 11);

  dwVbi = ((dwVbi & 0xfff00000) | ((dwMirror((dwVbi & 0x3fff), 14)) & 0x3fff));
  if(_arTveConf[TVE_2].ucMode == TVE_MOD_OFF)
  {
     TVE_HalSetInterlaceVbi(dwVbi, FALSE);
  }
  else
  {
     TVE_HalSetInterlaceVbi(dwVbi, TRUE);
  }
}

UINT32 TVE_SetVbiSignal(void)
{

  if(_arTveConf[TVE_2].fgNtsc)
  {
    vSetNtscVbiSignal();
  }
  else        // PAL TV
  {
    vSetPalVbiSignal();
  }
  return TVE_SET_OK;
}
void printf_cps(IBC_CpsCommonInfoParamsDef* prCpsInfo)
{
	CVBS_LOG("u4InfoValid =%x \n", prCpsInfo->u4InfoValid);
	CVBS_LOG("u1OriginalCgms =%x \n", prCpsInfo->u1OriginalCgms);
	CVBS_LOG("u1Cgms =%x \n", prCpsInfo->u1Cgms);
	CVBS_LOG("u1Aps =%x \n", prCpsInfo->u1Aps);
	CVBS_LOG("u1AnalogSrc =%x \n", prCpsInfo->u1AnalogSrc);
	CVBS_LOG("u1ICT =%x \n", prCpsInfo->u1ICT);
	CVBS_LOG("u1DOT =%x \n", prCpsInfo->u1DOT);
	CVBS_LOG("u1CSS =%x \n", prCpsInfo->u1CSS);
	CVBS_LOG("u1AACS =%x \n", prCpsInfo->u1AACS);
	CVBS_LOG("u1EPN =%x \n", prCpsInfo->u1EPN);
	CVBS_LOG("u1NotPassCnt =%x \n", prCpsInfo->u1NotPassCnt);
	CVBS_LOG("u1DCICCI =%x \n", prCpsInfo->u1DCICCI);
}
UINT32 TVE_SetCps(IBC_CpsCommonInfoParamsDef *prCpsInfo )
{
  UINT32 u4InfoValid;
  UINT8 u1Temp;
  u4InfoValid = prCpsInfo-> u4InfoValid;
  CVBS_DEF_LOG("[cvbs]VOD Copy control CPS: MediaType=%x,u4InfoValid = %x \n",_arTveConf[TVE_2].ucMediaType,u4InfoValid);
  CVBS_DEF_LOG("[cvbs]last:CGMS=%d, APS=%d, SRC= %d, ICT=%d, DOT=%d, CSS=%d, AACS=%d,Epn=%d \n", rLastCpsInfo.u1Cgms, rLastCpsInfo.u1Aps,  rLastCpsInfo.u1AnalogSrc,	 rLastCpsInfo.u1ICT, rLastCpsInfo.u1DOT, rLastCpsInfo.u1CSS, rLastCpsInfo.u1AACS,rLastCpsInfo.u1EPN);
  CVBS_DEF_LOG("[cvbs]now:CGMS=%d, APS=%d, SRC= %d, ICT=%d, DOT=%d, CSS=%d, AACS=%d,Epn=%d \n", prCpsInfo->u1Cgms, prCpsInfo->u1Aps,  prCpsInfo->u1AnalogSrc,	 prCpsInfo->u1ICT, prCpsInfo->u1DOT, prCpsInfo->u1CSS, prCpsInfo->u1AACS,prCpsInfo->u1EPN);

  if(u4InfoValid & IBC_CPS_INFO_CGMS_VALID)
  {
	//_ucBackupCGMS = prCpsInfo->u1Cgms;

	_arTveConf[TVE_2].ucCgmsaInfo= prCpsInfo->u1Cgms;
  }
  if(u4InfoValid & IBC_CPS_INFO_APS_VALID)
  {
	_arTveConf[TVE_2].ucAps= prCpsInfo->u1Aps;
  }

  if(u4InfoValid & IBC_CPS_INFO_EPN_VALID)
  {
	_arTveConf[TVE_2].fgEpn= prCpsInfo->u1EPN;
  }

  if(u4InfoValid & IBC_CPS_INFO_ANALOG_SRC_VALID)
  {
	_arTveConf[TVE_2].ucAnalogSrc= prCpsInfo->u1AnalogSrc;
  }
  if(u4InfoValid & IBC_CPS_INFO_ICT_VALID)
  {
  /// David recorrects , u1ICT=1 means HD analog output is permittable, but for TVE view, fgICT=1 means no output HD for analog vidoe outputs  !
	_arTveConf[TVE_2].fgICT= !(prCpsInfo->u1ICT);
  }

  if(u4InfoValid & IBC_CPS_INFO_DOT_VALID)
  {
	_arTveConf[TVE_2].fgDOT= prCpsInfo->u1DOT;
  }
  if(u4InfoValid & IBC_CPS_INFO_CSS_VALID)
  {
	_arTveConf[TVE_2].fgCSSTitle= prCpsInfo->u1CSS;

  }
  if(u4InfoValid & IBC_CPS_INFO_AACS_VALID)
  {
	_arTveConf[TVE_2].fgAACS= prCpsInfo->u1AACS;
  }

	if(u4InfoValid & IBC_CPS_INFO_NPCNT_VALID)
  {
	_arTveConf[TVE_2].fgNotPassCnt= prCpsInfo->u1NotPassCnt;

#if 1  //CONFIG_DRV_TVE_SONY
	if((_arTveConf[TVE_2].ucMediaType==MEDIA_TYPE_VOD)&&(_arTveConf[TVE_2].fgNotPassCnt))
	{
	  _arTveConf[TVE_2].fgBlackScn = 1;
	}
#endif

  }

	if(u4InfoValid & IBC_CPS_INFO_DCICCI_VALID)
  {
	_arTveConf[TVE_2].fgDCICCI= prCpsInfo->u1DCICCI;
  }



        u1Temp = (_arTveConf[TVE_2].fgEpn != (!(_arTveConf[TVE_2].ucCgmsaInfo))) ? 1 :0 ;

if(u1Temp!=_arTveConf[TVE_2].fgRCIRCD)
{
;
}

  _arTveConf[TVE_2].fgRCIRCD = u1Temp;

	{
		 rLastCpsInfo = *prCpsInfo;
	}

#if 1  //CONFIG_DRV_TVE_SONY
if(_arTveConf[TVE_2].ucMediaType!=MEDIA_TYPE_VOD)
{
  _arTveConf[TVE_2].fgBlackScn = 0;
}
#endif

  TVE_SetMacroVision(_arTveConf[TVE_2].ucAps);
  TVE_SetVbiSignal();

 return TVE_SET_OK;
}

UINT32 TVE_SetAspectRatio(TVE_ASPECT_RATIO_T ucAspectRatio)
{
  if( ucAspectRatio>=ASPECT_RATIO_MAX )
  {
    return TVE_SET_ERROR;
  }

  _arTveConf[TVE_2].ucAspectRatio = ucAspectRatio;

  TVE_SetVbiSignal();
  return TVE_SET_OK;
}

void TVE_SetFmt(UCHAR ucFmt, UCHAR ucTargetFmt, UCHAR ucMode, UCHAR ucVDacConfig, UCHAR ucEnable)
{
	UINT32 u4RegTmp;

	CVBS_DEF_LOG("[cvbs] ucFmt=%s, ucMode= %d, ucEnable=%d \n", szResStr[ucFmt], ucMode, ucEnable);

	_arTveConf[TVE_2].ucMainFmt = ucFmt;

  if((ucTargetFmt == TVE_RES_576P) || (ucFmt == TVE_RES_576P))
  {
    ucFmt = TVE_RES_576I;

		if(ucTargetFmt == TVE_RES_576P)
			_ucCVBSOutFmt = TVE_RES_576I;
		else
			_ucCVBSOutFmt = ucTargetFmt;
  }
  else if((ucTargetFmt == TVE_RES_480P) || (ucFmt == TVE_RES_480P))
  {
    ucFmt = TVE_RES_480I;

		if(ucTargetFmt == TVE_RES_480P)
			_ucCVBSOutFmt = TVE_RES_480I;
		else
			_ucCVBSOutFmt = ucTargetFmt;
  }
  else
  {
    CVBS_DEF_LOG("[cvbs] Illegal fmt!");

		return;
  }

  _arTveConf[TVE_2].ucFmt = ucFmt;
	_arTveConf[TVE_2].ucTargetFmt = ucTargetFmt;
  _arTveConf[TVE_2].ucTveEnable = ucEnable;
  _arTveConf[TVE_2].ucVFreq = ((ucFmt == TVE_RES_480I) ? 60:50);
	_arTveConf[TVE_2].ucMode = ucMode;
	_arTveConf[TVE_2].fgNtsc= fgVideoIsNtsc(ucFmt);
	_arTveConf[TVE_2].ucVDacConfig = ucVDacConfig;

  if (ucMode == TVE_MOD_COMPOSITE)
	{
    rTveAuxReg.uEncMode.rEncMode.u4OutMode0 = TVE_MOD_COMPOSITE;
  }
	else if(ucMode == TVE_MOD_S_VIDEO)
	{
    rTveAuxReg.uEncMode.rEncMode.u4OutMode0 = TVE_MOD_S_VIDEO;
  }
  else
	{
    rTveAuxReg.uEncMode.rEncMode.u4OutMode0 = TVE_MOD_OFF;
  }

	rTveAuxReg.uMuxCtrl.rMuxCtrl.fgPrgsOut = 0x1;
	rTveAuxReg.uMuxCtrl.rMuxCtrl.fgCVBSP2IEn = 0x1;
	rTveAuxReg.uSyncCtrl.rSyncCtrl.fgAdjSelfEn = 0x1;
	rTveAuxReg.uSyncCtrl.rSyncCtrl.fgRev = 0x1;
	rTveAuxReg.uSyncCtrl.rSyncCtrl.fgAdjForward = 0x1;

  if(fgVideoIsNtsc(ucFmt))
  {
		rTveAuxReg.uMuxCtrl.rMuxCtrl.u4TvType = 0x0;

		rTveAuxReg.uBurst2.rBurst2.u4BurstStart = 0x90;	// 144;
		rTveAuxReg.uBurst2.rBurst2.u4BurstEnd = 0xd8;		// 216;

                            //macrovision color stripe burst position
		rTveAuxReg.uBurst2.rBurst2.mv_burst_start = 0x42;	// 144;
		rTveAuxReg.uBurst2.rBurst2.mv_burst_end = 0x49;		// 216;
				
    rTveAuxReg.uChromaGain.rChromaGain.u4Y2ActiveEnd = 0x20;
    rTveAuxReg.uChromaGain.rChromaGain.u4C2ActiveEnd = 0x1a;

		TVE_SetSyncTime0(TRUE, 0x20d, 0x35a);
  }
  else
  {
		rTveAuxReg.uMuxCtrl.rMuxCtrl.u4TvType = 0x3;

		rTveAuxReg.uBurst2.rBurst2.u4BurstStart = 0x94;	// 148;
		rTveAuxReg.uBurst2.rBurst2.u4BurstEnd = 0xd0;		// 208;

    rTveAuxReg.uChromaGain.rChromaGain.u4Y2ActiveEnd = 0x29;
    rTveAuxReg.uChromaGain.rChromaGain.u4C2ActiveEnd = 0x19;

		TVE_SetSyncTime0(TRUE, 0x271, 0x360);
  }

	TVE_WRITE32(CVBS_TVE_CHROMA, rTveAuxReg.uChromaGain.u4ChromaGain);
	TVE_WRITE32(CVBS_TVE_BURST2, rTveAuxReg.uBurst2.u4Burst2);

	rTveAuxReg.uEncSyn.rEncSyn.u4YLPFCoefSet = 2;
	rTveAuxReg.uEncSyn.rEncSyn.u4CLPFCoefSet = 1;

  rTveAuxReg.uYScale.rYScale.u4ColorWidth = 0x16;//0x0a;
  rTveAuxReg.uYScale.rYScale.u4BlankLevel = 0x4;
  rTveAuxReg.uYScale.rYScale.u4Bright2 = 0;
  rTveAuxReg.uYScale.rYScale.fgYScale8 = 0;
  rTveAuxReg.uYScale.rYScale.u4CbDelay = 0;
  rTveAuxReg.uYScale.rYScale.u4CrDelay = 0;
  rTveAuxReg.uYScale.rYScale.fgMixUVOffset = 0;
	rTveAuxReg.uYScale.rYScale.u4YScale = au1TVE_YScale[fgVideoIsNtsc(ucFmt)];

  rTveAuxReg.uEncMode.rEncMode.fgYLPOn = 1;
  rTveAuxReg.uEncMode.rEncMode.fgCLPOn = 1;

  rTveAuxReg.uEncMode.rEncMode.u4SYDelay = 0;
  rTveAuxReg.uEncMode.rEncMode.u4YDelay = 0;
  rTveAuxReg.uEncMode.rEncMode.u4SYDelay = 0x1;

  if(fgVideoIsNtsc(ucFmt))
  {
    rTveAuxReg.uEncSyn.rEncSyn.u4SdSynrfadj = 0x01;
  }
  else
  {
    rTveAuxReg.uEncSyn.rEncSyn.u4SdSynrfadj = 0x00;
  }

  rTveAuxReg.uEncSyn.rEncSyn.u4SdSynLvl = u4CVBSSyncLevelTbl[fgVideoIsNtsc(ucFmt)];

  rTveAuxReg.uEncMode.rEncMode.fgSlewOff = 1;
  rTveAuxReg.uEncMode.rEncMode.fgFulW = 1;
	rTveAuxReg.uEncMode.rEncMode.fgBlker = 0;

	rTveAuxReg.uDispArea.rDispArea.fgAgcSkewSel = 0x01;
	rTveAuxReg.uDispArea.rDispArea.fgPorchSkewSel = 0x01;
	rTveAuxReg.uDispArea.rDispArea.fgPseudoSkewSel = 0x01;

  TVE_WRITE32(CVBS_TVE_ENCSYN, rTveAuxReg.uEncSyn.u4EncSyn);
	TVE_WRITE32(CVBS_TVE_MODE, rTveAuxReg.uEncMode.u4EncMode);
  TVE_WRITE32(CVBS_TVE_YSCALE, rTveAuxReg.uYScale.u4YScale);

  u4RegTmp = TVE_READ32(CVBS_TVE_BURST);

  if(fgVideoIsNtsc(ucFmt))
  {
  	extern BOOL _fgEnCVBSPal60;
    u4RegTmp =(u4RegTmp & ~BURST_LEVEL_SCH_MASK) | NTSC_BURST |(NTSC_SCH << 8);
  	if(_fgEnCVBSPal60) u4RegTmp|=PAL60_EN;
  }
  else
  {
    u4RegTmp =(u4RegTmp & ~BURST_LEVEL_SCH_MASK) |PAL_BURST |(PAL_SCH << 8);
  }

	TVE_WRITE32(CVBS_TVE_BURST, u4RegTmp);

	TVE_PalBDBurstError(ucFmt);
	TVE_PalLineMissing(ucFmt);

	TVE_SetSetup();
	TVE_SetDelay();
	TVE_SetGamma(0, _stTveVideoAdjust.i2ContrastLevel, _stTveVideoAdjust.i2BrightnessLevel);
	TVE_ResetHue(_stTveVideoAdjust.i2HueLevel, _stTveVideoAdjust.i2SaturationLevel);
              TVE_SetFilter(2);
              TVE_SD_PPF_enbale(1,ucFmt);
	TVE_SetSyncTime1(TRUE,TRUE, 0, 0);
	TVE_SetMacroVision((UCHAR)TVE_RES_576P);
	//TVE_SetVbiSignal();
	TVE_DACPower(ucEnable);
	//TVE_SetEnable(ucEnable);

  return;
}

void TVE_SetInitFmt(UCHAR ucFmt, UCHAR ucTargetFmt, UCHAR ucMode, UCHAR ucVDacConfig, UCHAR ucEnable)
{
	CVBS_DEF_LOG("[cvbs] init,ucFmt=%s, ucMode= %d, ucEnable=%d \n", szResStr[ucFmt], ucMode, ucEnable);

	_arTveConf[TVE_2].ucMainFmt = ucFmt;

  if((ucTargetFmt == TVE_RES_576P) || (ucFmt == TVE_RES_576P))
  {
    ucFmt = TVE_RES_576I;

		if(ucTargetFmt == TVE_RES_576P)
			_ucCVBSOutFmt = TVE_RES_576I;
		else
			_ucCVBSOutFmt = ucTargetFmt;
  }
  else if((ucTargetFmt == TVE_RES_480P) || (ucFmt == TVE_RES_480P))
  {
    ucFmt = TVE_RES_480I;

		if(ucTargetFmt == TVE_RES_480P)
			_ucCVBSOutFmt = TVE_RES_480I;
		else
			_ucCVBSOutFmt = ucTargetFmt;
  }
  else
  {
    CVBS_LOG(" Illegal fmt!");

		return;
  }

  _arTveConf[TVE_2].ucFmt = ucFmt;
	_arTveConf[TVE_2].ucTargetFmt = ucTargetFmt;
  _arTveConf[TVE_2].ucTveEnable = ucEnable;
  _arTveConf[TVE_2].ucVFreq = ((ucFmt == TVE_RES_480I) ? 60:50);
	_arTveConf[TVE_2].ucMode = ucMode;
	_arTveConf[TVE_2].fgNtsc= fgVideoIsNtsc(ucFmt);
	_arTveConf[TVE_2].ucVDacConfig = ucVDacConfig;

  if (ucMode == TVE_MOD_COMPOSITE)
	{
    rTveAuxReg.uEncMode.rEncMode.u4OutMode0 = TVE_MOD_COMPOSITE;
  }
	else if(ucMode == TVE_MOD_S_VIDEO)
	{
    rTveAuxReg.uEncMode.rEncMode.u4OutMode0 = TVE_MOD_S_VIDEO;
  }
  else
	{
    rTveAuxReg.uEncMode.rEncMode.u4OutMode0 = TVE_MOD_OFF;
  }

	rTveAuxReg.uMuxCtrl.rMuxCtrl.fgPrgsOut = 0x1;
	rTveAuxReg.uMuxCtrl.rMuxCtrl.fgCVBSP2IEn = 0x1;
	rTveAuxReg.uSyncCtrl.rSyncCtrl.fgAdjSelfEn = 0x1;
	rTveAuxReg.uSyncCtrl.rSyncCtrl.fgRev = 0x1;
	rTveAuxReg.uSyncCtrl.rSyncCtrl.fgAdjForward = 0x1;

  if(fgVideoIsNtsc(ucFmt))
  {
		rTveAuxReg.uMuxCtrl.rMuxCtrl.u4TvType = 0x0;

		rTveAuxReg.uBurst2.rBurst2.u4BurstStart = 0x90;	// 144;
		rTveAuxReg.uBurst2.rBurst2.u4BurstEnd = 0xd8;		// 216;

    rTveAuxReg.uChromaGain.rChromaGain.u4Y2ActiveEnd = 0x20;
    rTveAuxReg.uChromaGain.rChromaGain.u4C2ActiveEnd = 0x1a;

		//TVE_SetSyncTime0(TRUE, 0x20d, 0x35a);
  }
  else
  {
		rTveAuxReg.uMuxCtrl.rMuxCtrl.u4TvType = 0x3;

		rTveAuxReg.uBurst2.rBurst2.u4BurstStart = 0x94;	// 148;
		rTveAuxReg.uBurst2.rBurst2.u4BurstEnd = 0xd0;		// 208;

    rTveAuxReg.uChromaGain.rChromaGain.u4Y2ActiveEnd = 0x29;
    rTveAuxReg.uChromaGain.rChromaGain.u4C2ActiveEnd = 0x19;

		//TVE_SetSyncTime0(TRUE, 0x271, 0x360);
  }

	//TVE_WRITE32(CVBS_TVE_CHROMA, rTveAuxReg.uChromaGain.u4ChromaGain);
	//TVE_WRITE32(CVBS_TVE_BURST2, rTveAuxReg.uBurst2.u4Burst2);

	rTveAuxReg.uEncSyn.rEncSyn.u4YLPFCoefSet = 2;
	rTveAuxReg.uEncSyn.rEncSyn.u4CLPFCoefSet = 1;

  rTveAuxReg.uYScale.rYScale.u4ColorWidth = 0x16;//0x0a;
  rTveAuxReg.uYScale.rYScale.u4BlankLevel = 0x4;
  rTveAuxReg.uYScale.rYScale.u4Bright2 = 0;
  rTveAuxReg.uYScale.rYScale.fgYScale8 = 0;
  rTveAuxReg.uYScale.rYScale.u4CbDelay = 0;
  rTveAuxReg.uYScale.rYScale.u4CrDelay = 0;
  rTveAuxReg.uYScale.rYScale.fgMixUVOffset = 0;
	rTveAuxReg.uYScale.rYScale.u4YScale = au1TVE_YScale[fgVideoIsNtsc(ucFmt)];

  rTveAuxReg.uEncMode.rEncMode.fgYLPOn = 1;
  rTveAuxReg.uEncMode.rEncMode.fgCLPOn = 1;

  rTveAuxReg.uEncMode.rEncMode.u4SYDelay = 0;
  rTveAuxReg.uEncMode.rEncMode.u4YDelay = 0;
  rTveAuxReg.uEncMode.rEncMode.u4SYDelay = 0x1;

  if(fgVideoIsNtsc(ucFmt))
  {
    rTveAuxReg.uEncSyn.rEncSyn.u4SdSynrfadj = 0x01;
  }
  else
  {
    rTveAuxReg.uEncSyn.rEncSyn.u4SdSynrfadj = 0x00;
  }

  rTveAuxReg.uEncSyn.rEncSyn.u4SdSynLvl = u4CVBSSyncLevelTbl[fgVideoIsNtsc(ucFmt)];

  rTveAuxReg.uEncMode.rEncMode.fgSlewOff = 1;
  rTveAuxReg.uEncMode.rEncMode.fgFulW = 1;
	rTveAuxReg.uEncMode.rEncMode.fgBlker = 0;

	rTveAuxReg.uDispArea.rDispArea.fgAgcSkewSel = 0x01;
	rTveAuxReg.uDispArea.rDispArea.fgPorchSkewSel = 0x01;
	rTveAuxReg.uDispArea.rDispArea.fgPseudoSkewSel = 0x01;

 // TVE_WRITE32(CVBS_TVE_ENCSYN, rTveAuxReg.uEncSyn.u4EncSyn);
//	TVE_WRITE32(CVBS_TVE_MODE, rTveAuxReg.uEncMode.u4EncMode);
 // TVE_WRITE32(CVBS_TVE_YSCALE, rTveAuxReg.uYScale.u4YScale);
  /*

  u4RegTmp = TVE_READ32(CVBS_TVE_BURST);

  if(fgVideoIsNtsc(ucFmt))
  {
  	extern BOOL _fgEnCVBSPal60;
    u4RegTmp =(u4RegTmp & ~BURST_LEVEL_SCH_MASK) | NTSC_BURST |(NTSC_SCH << 8);
  	if(_fgEnCVBSPal60) u4RegTmp|=PAL60_EN;
  }
  else
  {
    u4RegTmp =(u4RegTmp & ~BURST_LEVEL_SCH_MASK) |PAL_BURST |(PAL_SCH << 8);
  }

	TVE_WRITE32(CVBS_TVE_BURST, u4RegTmp);

	TVE_PalBDBurstError(ucFmt);
	TVE_PalLineMissing(ucFmt);

	TVE_SetSetup();
	TVE_SetDelay();
	TVE_SetGamma(0, _stTveVideoAdjust.i2ContrastLevel, _stTveVideoAdjust.i2BrightnessLevel);
	TVE_ResetHue(_stTveVideoAdjust.i2HueLevel, _stTveVideoAdjust.i2SaturationLevel);
	TVE_SetSyncTime1(TRUE,TRUE, 0, 0);
	TVE_SetMacroVision((UCHAR)TVE_RES_576P);
	//TVE_SetVbiSignal();
	TVE_DACPower(ucEnable);
	TVE_SetEnable(ucEnable);
*/
  return;
}

extern void cvbs_recchg_nofity(int index);

BOOL is_tve_pal_ntsc_chg = FALSE;

void TveSetFmt(int fmt)
{

    if(r_tve_timer.function)
    {
        is_tve_pal_ntsc_chg = TRUE;
        TVE_SetEnable(FALSE);
        msleep(100);
    }

   if(fmt == TVE_RES_480P)
   	{
	   TVE_SetFmt(TVE_RES_480P, TVE_RES_480P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
	  // TVE_SetColorBar(FALSE);
	   TVE_SetSyncTime1(TRUE,FALSE,0x01a, 0x322);
	   vDPI0_480p();
    }
   else//TVE_RES_576P
   	{
	   TVE_SetFmt(TVE_RES_576P, TVE_RES_576P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
	  // TVE_SetColorBar(FALSE);
	   TVE_SetSyncTime1(TRUE,FALSE,0x008, 0x32b);
	   vDPI0_576p();
    }
   //cvbs_recchg_nofity((fmt==TVE_RES_480P) ? 0 : 1);

   if(r_tve_timer.function)
   {
        mod_timer(&r_tve_timer, jiffies + 500/(1000/HZ));
    }
}

void tve_setfmt_delay(unsigned long n)
{
    if(is_tve_pal_ntsc_chg == TRUE)
    {
        TVE_SetEnable(TRUE);
        is_tve_pal_ntsc_chg = FALSE;
    }
}

void TveSetInitFmt(int fmt)
{
   if(fmt == TVE_RES_480P)
   	{
	   TVE_SetInitFmt(TVE_RES_480P, TVE_RES_480P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
	   //TVE_SetSyncTime1(TRUE,FALSE,0x01a, 0x322);

    }
   else//TVE_RES_576P
   	{
	   TVE_SetInitFmt(TVE_RES_576P, TVE_RES_576P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
	   //TVE_SetSyncTime1(TRUE,FALSE,0x008, 0x32b);
    }
}

void TveGetFmt(char* str)
{
  strcpy(str,szResStr[_ucCVBSOutFmt]);
  CVBS_LOG("_ucCVBSOutFmt %d str= %s\n",_ucCVBSOutFmt,str);
}
void TveSetDPI0Colorbar(unsigned short flag)
{
	//TVE_Init();
	TVE_SetFmt(TVE_RES_576P, TVE_RES_576P, TVE_MOD_COMPOSITE, VDAC_LOW_IMPEDANCE, TRUE);
              TVE_SetEnable(TRUE);
	//TVE_SetColorBar(TRUE);
	TVE_SetSyncTime1(TRUE,FALSE,0x008, 0x32b);
		//vDPI0_480p();
	vDPI0_576p();
	vDPI0_CB_Enable((BOOL)flag);
}
void TveEnable(unsigned short flag)
{
   if(flag == TRUE)
   	{
	   TVE_DACPower(TRUE);
	   TVE_SetEnable(TRUE);
    }
   else
   	{
	   TVE_DACPower(FALSE);
	   TVE_SetEnable(FALSE);
    }
}

void TVE_PowerOn(void)
{
        CVBS_DEF_LOG("[cvbs]tve_is_power_on %d\n",tve_is_power_on);

        if(!tve_is_power_on)
        {
                enable_clock(MT_CG_DISP1_TVE_OUTPUT_CLOCK,"TVE");
        	enable_clock(MT_CG_DISP1_TVE_INPUT_CLOCK,"TVE");
        	enable_clock(MT_CG_DISP1_DPI_ENGINE,"TVE");
        	enable_clock(MT_CG_DISP1_DPI_DIGITAL_LANE,"TVE");
                tve_is_power_on = TRUE;
        }

        return;
}

void TVE_PowerOff(void)
{
        CVBS_DEF_LOG("[cvbs]tve_is_power_on %d\n",tve_is_power_on);
        if(tve_is_power_on)
        {
                disable_clock(MT_CG_DISP1_TVE_OUTPUT_CLOCK,"TVE");
        	disable_clock(MT_CG_DISP1_TVE_INPUT_CLOCK,"TVE");
        	disable_clock(MT_CG_DISP1_DPI_ENGINE,"TVE");
        	disable_clock(MT_CG_DISP1_DPI_DIGITAL_LANE,"TVE");
                tve_is_power_on = FALSE;
        }

        return;
}

void TVE_Suspend(void)
{
	CVBS_DEF_LOG("[cvbs] TVE_Suspend \n");
	TveEnable(FALSE);
	TVE_PowerOff();
}
void TVE_Resume(void)
{
	CVBS_DEF_LOG("[cvbs] TVE_Resume ntsc %d\n",_arTveConf[TVE_2].fgNtsc);
	TVE_PowerOn();
	TVE_PreInit();//add by xuguo for power on need to set clk
	TveEnable(TRUE);
	TveSetFmt(_arTveConf[TVE_2].fgNtsc ? TVE_RES_480P : TVE_RES_576P);
}

void Tve_LogEnable(unsigned short enable)
{
    CVBS_LOG_ENABLE = enable;
}
void USB_Dac_Init(void)
{
	CVBS_LOG("Enter USB Dac initial... \n");

	vWriteUsbDacMsk(USB_PHY_ACR0, (0 << 5)|(1 << 4)|(1 << 0), INTR_EN|REF_EN|BGR_EN);
}

BOOL tve_get_auto_detect_status(void)
{
    if(TVE_READ32(0x634) & 0x100)
        return TRUE;
    return FALSE;
}
UINT32 TVE_HalGetPowerDacStatus(void)// here need to confirm register
{
#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
    CVBS_AUTO_DETECT_LOG("TVE_HalGetPowerDacStatus %x, %d \n",TVE_READ32(0x634),cvbs_auto_detect_enable);
    if(cvbs_auto_detect_enable)
	return (UINT32)((TVE_READ32(0x634) & 0x40000000) >> 30); //31:Y 30:x
    else
        return 0;
#else
    return 0;
#endif
}

BOOL tve_detect_is_pending(void)
{
    if(((TVE_READ32(CVBS_TVE_MODE) & 0x01) != 0) && (is_tve_pal_ntsc_chg == TRUE))
    {
        CVBS_AUTO_DETECT_LOG("tve is changing\n");
        return TRUE;
    }

    return FALSE;
}

void tve_check_teltext(void)
{
        TVE_WRITE32(CVBS_TT_CTL,0x01);
        TVE_WRITE32(CVBS_TELTXT1,(0x87d00000>>2)); //teltext data dram address
        TVE_WRITE32(CVBS_TELTXT2,0x010e8690);
        TVE_WRITE32(CVBS_TELTXT3,0xc0100a14);
        TVE_WRITE32(CVBS_TELTXT4,0x02d800d9);
        TVE_WRITE32(CVBS_TELTXT5,0x01270018);
}
#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT

void TVE_GetTveStatus(TVE_STATUS_CONF_T *TveStatus)
{
  TveStatus->ucAspectRatio=_arTveConf[TVE_2].ucAspectRatio;
  TveStatus->ucCgmsaInfo=_arTveConf[TVE_2].ucCgmsaInfo;
  TveStatus->ucAps=_arTveConf[TVE_2].ucAps;
  TveStatus->ucAnalogSrc=_arTveConf[TVE_2].ucAnalogSrc;
  TveStatus->ucMediaType=_arTveConf[TVE_2].ucMediaType;
  TveStatus->fgEpn=_arTveConf[TVE_2].fgEpn;
  TveStatus->fgNotPassCnt=_arTveConf[TVE_2].fgNotPassCnt;
  TveStatus->fgDCICCI=_arTveConf[TVE_2].fgDCICCI;
  TveStatus->fgICT=_arTveConf[TVE_2].fgICT;
  TveStatus->fgDOT=_arTveConf[TVE_2].fgDOT;
  TveStatus->fgCSSTitle=_arTveConf[TVE_2].fgCSSTitle;
  TveStatus->fgAACS=_arTveConf[TVE_2].fgAACS;

  TveStatus->ucTveEnable=_arTveConf[TVE_2].ucTveEnable;
  TveStatus->ucMainFmt=_arTveConf[TVE_2].ucMainFmt;
  TveStatus->ucTargetFmt=_arTveConf[TVE_2].ucTargetFmt;
  TveStatus->ucVFreq=_arTveConf[TVE_2].ucVFreq;
  TveStatus->ucTveDacStatus = TVE_HalGetPowerDacStatus() & 0x1;
  CVBS_LOG("ucAspectRatio =%x ucCgmsaInfo %x ucAps= %x ucAnalogSrc = %x ucMediaType = %x fgEpn=%x fgNotPassCnt= %x\n",TveStatus->ucAspectRatio,\
  	TveStatus->ucCgmsaInfo,TveStatus->ucAps,TveStatus->ucAnalogSrc,TveStatus->ucMediaType,TveStatus->fgEpn,TveStatus->fgNotPassCnt);
  CVBS_LOG("fgDCICCI=%x fgICT =%x fgDOT=%x fgCSSTitle=%x fgAACS=%x \n",TveStatus->fgDCICCI,TveStatus->fgICT,TveStatus->fgDOT,\
  	TveStatus->fgCSSTitle,TveStatus->fgAACS);
  CVBS_LOG("ucTveEnable=%x ucMainFmt =%x ucTargetFmt =%x ucVFreq = %x ucTveDacStatus =%x \n",TveStatus->ucTveEnable,\
  	TveStatus->ucMainFmt,TveStatus->ucTargetFmt,TveStatus->ucVFreq,TveStatus->ucTveDacStatus);
 }

#endif

void TVE_Init(void)
{
	CVBS_DEF_LOG("[cvbs] Enter TVE initial... \n");

	USB_Dac_Init();

	vWriteClkGenANAMsk(TOP_CLK_CFG_5, 0x0 << 7, PDN_TVE);

                if(_TveInitiated == 0)
                {
                TVE_Reset();

                _TveInitiated = 1;
                }

	return;
}
void TVE_PreInit(void)
{
#if DISPLAY_V4L2
	tve_tvdpll_dpi0_tve_path();
#endif
	tve_mipipll_dpi0_tve_path();
	TVE_Init();
}
void cvbs_drv_init(void)
{
        CVBS_DEF_LOG("[cvbs]cvbs_drv_init\n");

    USB_Dac_Init();

    if(_TveInitiated == 0)
    {
        _TveInitiated = 1;
    }

    memset((void*)&r_tve_timer, 0, sizeof(r_tve_timer));
    r_tve_timer.expires  = jiffies + 1000/(1000/HZ);   // wait 1s to stable
    r_tve_timer.function = tve_setfmt_delay;
    r_tve_timer.data     = 0;
    init_timer(&r_tve_timer);
    add_timer(&r_tve_timer);
}
/********************************************************************

        TVE debug system

*******************************************************************/
extern CVBS_STATE cvbs_connect_state ;

#define TVE_ATTR_SPRINTF(fmt, arg...)  \
                do { \
                         temp_len = sprintf(buf,fmt,##arg);      \
                         buf += temp_len; \
                         len += temp_len; \
                         buf[0] = 0;\
                }while (0)

				
void tve_driver_debug(char *pbuf)
{
	int var;
	u32 reg;
	unsigned int val;
	unsigned int vadr_regstart;
	unsigned int vadr_regend;
	char *buf;
	int temp_len = 0;
	int len = 0;
	buf = pbuf;
	IBC_CpsCommonInfoParamsDef s_cps;

	if (strncmp(buf, "dbgtype:",8) == 0)
	{
		sscanf(buf+8, "%x", &var);
		CVBS_LOG_ENABLE = var;
		printk("CVBS_LOG_ENABLE = 0x%08x\n",CVBS_LOG_ENABLE);
	}
	else if (strncmp(buf, "w:",2) == 0)
	{
		sscanf(buf+2, "%x=%x", &reg , &val);
		printk("w:0x%08x=0x%08x\n", reg, val);

		*(volatile unsigned int*)(reg) = val;
	}
	else if (strncmp(buf, "r:",2) == 0)
	{
		sscanf(buf+2, "%x/%x", &vadr_regstart , &vadr_regend);
		vadr_regend  &= 0x3ff;
		printk("r:0x%08x/0x%08x\n", vadr_regstart, vadr_regend);
		vadr_regend = vadr_regstart + vadr_regend;
		while (vadr_regstart <= vadr_regend)
		{
			TVE_ATTR_SPRINTF("0x%08x = 0x%08x\n",vadr_regstart,*(volatile unsigned int*)(vadr_regstart));
			printk("0x%08x = 0x%08x\n",vadr_regstart,*(volatile unsigned int*)(vadr_regstart));
			vadr_regstart += 4;
		}
	}
	else if (strncmp(buf,"status",6) == 0)
	{
		//status
		TVE_ATTR_SPRINTF("CVBS_LOG_ENABLE=%x\n",CVBS_LOG_ENABLE);
		TVE_ATTR_SPRINTF("tve_is_power_on=%d\n",tve_is_power_on);
		TVE_ATTR_SPRINTF("cvbs_connect_state=%d\n",cvbs_connect_state);
		TVE_ATTR_SPRINTF("cvbs_auto_detect_enable=%d,status=%x\n",cvbs_auto_detect_enable, TVE_HalGetPowerDacStatus());
		TVE_ATTR_SPRINTF("format=%d,fgntsc=%d,mode=%d\n",_arTveConf[TVE_2].ucFmt,_arTveConf[TVE_2].fgNtsc,_arTveConf[TVE_2].ucMode);
		TVE_ATTR_SPRINTF("ucAspectRatio =%x ucCgmsaInfo %x ucAps= %x ucAnalogSrc = %x ucMediaType = %x fgEpn=%x fgNotPassCnt= %x\n",_arTveConf[TVE_2].ucAspectRatio,_arTveConf[TVE_2].ucCgmsaInfo,_arTveConf[TVE_2].ucAps,_arTveConf[TVE_2].ucAnalogSrc,_arTveConf[TVE_2].ucMediaType,_arTveConf[TVE_2].fgEpn,_arTveConf[TVE_2].fgNotPassCnt);
		TVE_ATTR_SPRINTF("fgDCICCI=%x fgICT =%x fgDOT=%x fgCSSTitle=%x fgAACS=%x \n",_arTveConf[TVE_2].fgDCICCI,_arTveConf[TVE_2].fgICT,_arTveConf[TVE_2].fgDOT,_arTveConf[TVE_2].fgCSSTitle,_arTveConf[TVE_2].fgAACS);
		TVE_ATTR_SPRINTF("ucTveEnable=%x ucMainFmt =%x ucTargetFmt =%x ucVFreq = %x \n",_arTveConf[TVE_2].ucTveEnable,_arTveConf[TVE_2].ucMainFmt,_arTveConf[TVE_2].ucTargetFmt,_arTveConf[TVE_2].ucVFreq);

		//dump reg
		TVE_ATTR_SPRINTF("0xF4000110=%08x\n", T_REG32(0xF4000110));
		TVE_ATTR_SPRINTF("0xF4000070=%08x\n", T_REG32(0xF4000070));
		TVE_ATTR_SPRINTF("0xF4000058=%08x\n", T_REG32(0xF4000058));
		TVE_ATTR_SPRINTF("0xF0000024=%08x\n", T_REG32(0xF0000024));
		TVE_ATTR_SPRINTF("0xF0000080=%08x\n", T_REG32(0xF0000080));
		TVE_ATTR_SPRINTF("0xF0000090=%08x\n", T_REG32(0xF0000090));

		TVE_ATTR_SPRINTF("0xF000D010=%08x\n", T_REG32(0xF000D010));
		TVE_ATTR_SPRINTF("0xF000D014=%08x\n", T_REG32(0xF000D014));
		TVE_ATTR_SPRINTF("0xF000DF00=%08x\n", T_REG32(0xF000DF00));

		TVE_ATTR_SPRINTF("0xF4017600=%08x\n", T_REG32(0xF4017600));
		TVE_ATTR_SPRINTF("0xF4017604=%08x\n", T_REG32(0xF4017604));
		TVE_ATTR_SPRINTF("0xF4017634=%08x\n", T_REG32(0xF4017634));

		//status
		printk("CVBS_LOG_ENABLE=%x\n",CVBS_LOG_ENABLE);
		printk("tve_is_power_on=%d\n",tve_is_power_on);
		printk("cvbs_connect_state=%d\n",cvbs_connect_state);
		printk("cvbs_auto_detect_enable=%d,status=%x\n",cvbs_auto_detect_enable, TVE_HalGetPowerDacStatus());
		printk("format=%d,fgntsc=%d,mode=%d\n",_arTveConf[TVE_2].ucFmt,_arTveConf[TVE_2].fgNtsc,_arTveConf[TVE_2].ucMode);
		printk("ucAspectRatio =%x ucCgmsaInfo %x ucAps= %x ucAnalogSrc = %x ucMediaType = %x fgEpn=%x fgNotPassCnt= %x\n",_arTveConf[TVE_2].ucAspectRatio,_arTveConf[TVE_2].ucCgmsaInfo,_arTveConf[TVE_2].ucAps,_arTveConf[TVE_2].ucAnalogSrc,_arTveConf[TVE_2].ucMediaType,_arTveConf[TVE_2].fgEpn,_arTveConf[TVE_2].fgNotPassCnt);
		printk("fgDCICCI=%x fgICT =%x fgDOT=%x fgCSSTitle=%x fgAACS=%x \n",_arTveConf[TVE_2].fgDCICCI,_arTveConf[TVE_2].fgICT,_arTveConf[TVE_2].fgDOT,_arTveConf[TVE_2].fgCSSTitle,_arTveConf[TVE_2].fgAACS);
		printk("ucTveEnable=%x ucMainFmt =%x ucTargetFmt =%x ucVFreq = %x \n",_arTveConf[TVE_2].ucTveEnable,_arTveConf[TVE_2].ucMainFmt,_arTveConf[TVE_2].ucTargetFmt,_arTveConf[TVE_2].ucVFreq);

		//dump reg
		printk("0xF4000110=%08x\n", T_REG32(0xF4000110));
		printk("0xF4000070=%08x\n", T_REG32(0xF4000070));
		printk("0xF4000058=%08x\n", T_REG32(0xF4000058));
		printk("0xF0000024=%08x\n", T_REG32(0xF0000024));
		printk("0xF0000080=%08x\n", T_REG32(0xF0000080));
		printk("0xF0000090=%08x\n", T_REG32(0xF0000090));

		printk("0xF000D010=%08x\n", T_REG32(0xF000D010));
		printk("0xF000D014=%08x\n", T_REG32(0xF000D014));
		printk("0xF000DF00=%08x\n", T_REG32(0xF000DF00));

		printk("0xF4017600=%08x\n", T_REG32(0xF4017600));
		printk("0xF4017604=%08x\n", T_REG32(0xF4017604));
		printk("0xF4017634=%08x\n", T_REG32(0xF4017634));
	}
	else if (strncmp(buf,"dpicb:",6) == 0)
	{
		sscanf(buf+6, "%x", &var);
		*((unsigned int *) 0xf400df00) = var;
	}
	else if (strncmp(buf,"tvecb:",6) == 0)
	{
		sscanf(buf+6, "%x", &var);
		if(var != 0)
		TVE_WRITE32(CVBS_TVE_MODE,TVE_READ32(CVBS_TVE_MODE) |(1<<1));
		else
		TVE_WRITE32(CVBS_TVE_MODE,TVE_READ32(CVBS_TVE_MODE) & (~(1<<1)));
	}
	else if (strncmp(buf,"mipipll",7) == 0)
	{
		tve_mipipll_dpi0_tve_path();
	}
	else if (strncmp(buf,"tvdpll",6) == 0)
	{
		tve_tvdpll_dpi0_tve_path();
	}
	else if (strncmp(buf,"pal",3) == 0)
	{
		TveSetFmt(TVE_RES_576P);
	}
	else if (strncmp(buf,"ntsc",4) == 0)
	{
		TveSetFmt(TVE_RES_480P);
	}
	else if (strncmp(buf,"cps:",4) == 0)
	{
		sscanf(buf+4, "%x/%x/%x", &(s_cps.u4InfoValid),&(s_cps.u1Cgms),&(s_cps.u1Aps));
		if (s_cps.u4InfoValid |IBC_CPS_INFO_CGMS_VALID)
			printk("IBC_CPS_INFO_CGMS_VALID\n");
		if (s_cps.u4InfoValid |IBC_CPS_INFO_APS_VALID)
			printk("IBC_CPS_INFO_APS_VALID\n");
		TVE_SetCps(&s_cps);
	}	
	else if (strncmp(buf,"asp:",4) == 0)
	{
		sscanf(buf+4, "%x", &val);
		TVE_SetAspectRatio((TVE_ASPECT_RATIO_T)val);
	}	
	else if (strncmp(buf, "help",4) == 0)
	{
		TVE_ATTR_SPRINTF("---tve debug help---\n");
		TVE_ATTR_SPRINTF("please go in to sys/kernel/debug\n");
		TVE_ATTR_SPRINTF("[dbgtype]echo dbgtype:VALUE>tve\n");
		TVE_ATTR_SPRINTF("[dbgtype]bit0:default log; bit1:auto detect log,bit2:func log\n");
		TVE_ATTR_SPRINTF("[w]echo w:ADDR=VALUE>tve\n");
		TVE_ATTR_SPRINTF("[r]echo r:ADDR/LENGTH>tve;cat tve\n");
		TVE_ATTR_SPRINTF("[status]echo status>tve;cat tve\n");
		TVE_ATTR_SPRINTF("[dpicb]echo dpi0cb:VALUE>tve\n");
		TVE_ATTR_SPRINTF("[dpicb]0:disable; 0x41:dpi0 colorbar;0x21:dpi0 256 h gray;0x51:dpi0 user color\n");
		TVE_ATTR_SPRINTF("[tvecb]echo tvecb:VALUE>tve\n");
		TVE_ATTR_SPRINTF("[tvecb]0:disbale;1:enable tve colorbar\n");
		TVE_ATTR_SPRINTF("[mipipll]echo mipipll>tve\n");
		TVE_ATTR_SPRINTF("[tvdpll]echo tvdpll>tve\n");
		TVE_ATTR_SPRINTF("[ntsc]echo ntsc>tve\n");
		TVE_ATTR_SPRINTF("[pal]echo pal>tve\n");
		TVE_ATTR_SPRINTF("[mipipll]+[pal]+[dpicb]:mipipll+dpi0+tve output dpi0 colorbar\n");
		TVE_ATTR_SPRINTF("[tvdpll]+[pal]+[dpicb]:tvdpll+dpi0+tve output dpi0 colorbar\n");

		printk("---tve debug help---\n");
		printk("please go in to sys/kernel/debug\n");
		printk("[dbgtype]echo dbgtype:VALUE>tve\n");
		printk("[dbgtype]bit0:default log; bit1:auto detect log,bit2:func log\n");
		printk("[w]echo w:ADDR=VALUE>tve\n");
		printk("[r]echo r:ADDR/LENGTH>tve;cat tve\n");
		printk("[status]echo status>tve;cat tve\n");
		printk("[dpicb]echo dpi0cb:VALUE>tve\n");
		printk("[dpicb]0:disable; 0x41:dpi0 colorbar;0x21:dpi0 256 h gray;0x51:dpi0 user color\n");
		printk("[tvecb]echo tvecb:VALUE>tve\n");
		printk("[tvecb]0:disbale;1:enable tve colorbar\n");
		printk("[mipipll]echo mipipll>tve\n");
		printk("[tvdpll]echo tvdpll>tve\n");
		printk("[ntsc]echo ntsc>tve\n");
		printk("[pal]echo pal>tve\n");
		printk("[mipipll]+[pal]+[dpicb]:mipipll+dpi0+tve output dpi0 colorbar\n");
		printk("[tvdpll]+[pal]+[dpicb]:tvdpll+dpi0+tve output dpi0 colorbar\n");
	}
}

void tve_path_init(void)
{
		tve_tvdpll_dpi0_tve_path();
		TveSetFmt(TVE_RES_576P);
}

