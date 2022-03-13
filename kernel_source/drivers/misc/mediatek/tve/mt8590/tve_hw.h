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
#ifndef _TVE_HW_H_
#define _TVE_HW_H_

//#include "CTP_type.h"
#include "tve_dev.h"
// usb phy acr0
#define TVE_USB_BASE													(0xF1210000)
#define USB_PHY_ACR0											(0x0800)
	#define INTR_EN													(0x01 << 5)
	#define REF_EN														(0x01 << 4)
	#define BGR_EN														(0x01 << 0)

#define vWriteUsbDac(dAddr, dVal)  (*((volatile unsigned int *)(TVE_USB_BASE + dAddr)) = (dVal))
#define dReadUsbDac(dAddr)         (*((volatile unsigned int *)(TVE_USB_BASE + dAddr)))
#define vWriteUsbDacMsk(dAddr, dVal, dMsk) (vWriteUsbDac((dAddr), (dReadUsbDac(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

// APB Module mipi_tx_config
#define MIPI_CONFIG_BASE (0xF0010000)

// mipi vdac area for tve
#define MIPI_VDAC_BASE										(0xF0010000)

#define TVE_CTL0         									(0x0420)
	#define RG_BGREF_PWD            				(0x01 << 0)
	#define RG_TRIM_REV	            				(0x03 << 1)
	#define RG_TRIM_VAL            					(0x07 << 3)
	#define RG_VDAC_REV	            				(0xff << 8)
	#define RG_PBS_TST_EN            				(0x01 << 17)
	#define RG_VDAC_TST_EN            			(0x01 << 18)
	
#define vWriteMipiDac(dAddr, dVal)  (*((volatile unsigned int *)(MIPI_VDAC_BASE + dAddr)) = (dVal))
#define dReadMipiDac(dAddr)         (*((volatile unsigned int *)(MIPI_VDAC_BASE + dAddr)))
#define vWriteMipiDacMsk(dAddr, dVal, dMsk) (vWriteMipiDac((dAddr), (dReadMipiDac(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

// dispsys area
#define DISPSYS_BASE											(0xF4000000)

#define DISP_TVE_SYS_CFG_00          			(0x0908)
	#define TVE_CKGEN_CONFIG								(0x03 << 0)
	#define TVE_CKGEN_RSTB 									(0x03 << 2)
	#define TVE_CKGEN_MV									(0x1 << 5) 
	#define TVE_CKGEN_TVE_CLK								(0x1 << 6) 
	#define TVE_CKGEN_RSTB									(0x03 << 2) 
	#define TVE_PRGS_SEL 									(0x01 << 8)
	#define TVE_VDAC_CLK_INV 								(0x01 << 16)
	#define TVE_VDAC_TEST_SEL								(0x01 << 17)
	#define TVE_PCLK_FREE_RUN								(0x01 << 31)

#define DISP_TVE_SEL         							(0x0070)
	#define TVE_SEL													(0x01 << 0)
		#define DPI1_SEL											(0x00 << 0)
		#define DPI0_SEL											(0x01 << 0)

#define vWriteDispSys(dAddr, dVal)  (*((volatile unsigned int *)(DISPSYS_BASE + dAddr)) = (dVal))
#define dReadDispSys(dAddr)         (*((volatile unsigned int *)(DISPSYS_BASE + dAddr)))
#define vWriteDispSysMsk(dAddr, dVal, dMsk) (vWriteDispSys((dAddr), (dReadDispSys(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

// TOP clk
#define DISPSYS_TOPCKGEN_BASE							(0xF0000000)

#define TOP_CLK_CFG_4													0x80
	#define CLK_DPI1_SEL_MUX 								(0x3<<24)
		#define DPI1_SEL_CK26M 								(0x0<<24)
		#define DPI1_SEL_TVDPLL 							(0x1<<24)
		#define DPI1_SEL_TVDPLL_D2  					(0x2<<24)
		#define DPI1_SEL_TVDPLL_D4  					(0x3<<24)		
	#define CLK_DPI1_INV										(0x1<<28)
	#define PDN_DPI1												(0x1<<31)
	#define CLK_DPI0_SEL										(0x7<<16)
		#define DPI0_MUX_CK 									(0x0<<16)
		#define DPI0_MIPI											(0x1<<16)
		#define DPI0_MIPI_D2 									(0x2<<16)
		#define DPI0_MIPI_D4 									(0x3<<16)
		#define DPI0_LVDS 										(0x4<<16)
		#define DPI0_LVDS_D2									(0x5<<16)
		#define DPI0_LVDS_D4									(0x6<<16) 
		#define DPI0_LVDS_D8									(0x7<<16) 
	#define CLK_DPI0_INV										(0x1<<20)
	#define PDN_DPI0												(0x1<<23)

#define TOP_CLK_CFG_5													0x90   
  #define CLK_TVE_SEL 										(0x7<<0)  
        #define CLK26M											(0x0<<0)
  		#define TVE_MIPI 										(0x1<<0)
		#define TVE_MIPI_D2										(0x2<<0)
		#define TVE_MIPI_D4										(0x3<<0)
	  #define CLK_26M 											(0x4<<0)
  	#define TVDPLL 												(0x5<<0)
	  #define TVDPLL_D2  										(0x6<<0)
	  #define TVDPLL_D4  										(0x7<<0)
	#define CLK_TVE_INV 										(0x1<<4)
	#define PDN_TVE			 										(0x1<<7)

#define vWriteClkGenANA(dAddr, dVal)  (*((volatile unsigned int *)(DISPSYS_TOPCKGEN_BASE + dAddr)) = (dVal))
#define dReadClkGenANA(dAddr)         (*((volatile unsigned int *)(DISPSYS_TOPCKGEN_BASE + dAddr)))
#define vWriteClkGenANAMsk(dAddr, dVal, dMsk) (vWriteClkGenANA((dAddr), (dReadClkGenANA(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

//TVE area	
#define DISPSYS_TVE_BASE									(0xF4017000)

#define CVBS_TVE_ENCSYN         					(0x600)	
#define CVBS_TVE_MODE           					(0x604)
#define CVBS_TVE_CC 											(0x608)
#define CVBS_TVE_YSCALE       						(0x60C)
#define CVBS_TVE_UGAIN       							(0x618)
#define CVBS_TVE_VGAIN       							(0x61C)
#define CVBS_TVE_GAMMA0       						(0x620)
#define CVBS_TVE_GAMMA1       						(0x624)
#define CVBS_TVE_GAMMA2       						(0x628)
#define CVBS_TVE_GAMMA3       						(0x62C)
#define CVBS_TVE_GAMMA4       						(0x630)
#define CVBS_TVE_DACTRL       						(0x634)
        #define PLUG_REF_MASK           (0x03 << 22)
#define CVBS_TVE_BURST       							(0x638)
	#define PAL60_EN                 				(1<<15)
  #define BURST_LEVEL_SCH_MASK    				0x7FFF
	
#define CVBS_TVE_CHROMA								(0x63C)

#define CVBS_TVE_WSSI           					(0x644)
#define VBI_ON					   					(0x1 << 20)
#define VBI_LVL_MASK		 						((unsigned)0xff << 24)

#define CVBS_TVE_YGLIMIT      						(0x66c)
#define CVBS_TVE_DSIP_AREA    						(0x670)
#define CVBS_TVE_BURST2         					(0x674)
#define CVBS_TVE_MUX_CTRL       					(0x678)
#define CVBS_TVE_SYNC_CTRL      					(0x67C)
#define CVBS_TVE_SYNC_TIME0     					(0x680)
#define CVBS_TVE_SYNC_TIME1     					(0x684)
#define CVBS_TVE_SYNC_TIME2     					(0x688)
#define CVBS_TVE_SYNC_TIME3     					(0x68c)
#define CVBS_TVE_ABIST_CONTROL  					(0x694)
#define CVBS_TVE_VPS_CTRL1 								(0x69C)
#define CVBS_TVE_VPS_DATA1 								(0x6A0)
#define CVBS_TVE_VPS_DATA2  							(0x6A4)
#define CVBS_TVE_VPS_DATA3 								(0x6A8)
#define CVBS_TVE_VPS_DATA4_VPS_CTRL2 			(0x6AC)
#define CVBS_TVE_ABIST_SELFGEN_CONTROL		(0x6BC)
#define CVBS_TVE_MV1				  						(0x780)
#define CVBS_TVE_MV2				  						(0x784)
#define CVBS_TVE_MV3				  						(0x788)
#define CVBS_TVE_MV4				  						(0x78C)
#define CVBS_TVE_MV5				  						(0x790)
#define CVBS_TVE_MV6				  						(0x794)
	#define MVOFF				(0x1 << 28)
  #define CPNTMVOFF		(0x1 << 29)
  #define MUVSW       (0x1 << 30)
#define CVBS_TVE_MV7				  						(0x798)
	#define MV_AGC_BP_MASK   (0xffff << 8)

#define CVBS_SD_PPF_VSTORE	(0x7C0)
  #define V_STORE_END_MASK   (0x3FF << 16)
  #define V_STORE_BGN_MASK   (0x3FFF << 0)
        
#define CVBS_SD_PPF_Y_HSTORE  (0x7C4)
  #define Y_HSTORE_END_MASK   (0x7FF << 16)
  #define Y_HSTORE_BGN_MASK   (0x7FF << 0)
        
#define CVBS_SD_PPF_C_HSTORE  (0x7C8)
  #define C_HSTORE_END_MASK   (0x7FF << 16)
  #define C_HSTORE_BGN_MASK   (0x7FF << 16)
        
#define CVBS_SD_PPF_C_COEF  (0x7CC)
  #define COEF_3_C_MASK   (0xFF << 16)
  #define COEF_2_C_MASK   (0xFF << 8)
  #define COEF_1_C_MASK   (0xFF << 0)
        
#define CVBS_SD_PPF_Y_COEF  (0x7D0)
  #define COEF_3_Y_MASK   (0xFF << 16)
  #define COEF_2_Y_MASK   (0xFF << 8)
  #define COEF_1_Y_MASK   (0xFF << 0)
        
#define CVBS_FIRST_SECOND_COEF  (0x7D4)
  #define SECOND_COEF_MASK   (0xFF << 8)
  #define FIRST_COEF_MASK   (0xFF << 0)
        
#define CVBS_CONTROL  (0x7D8)
  #define RG_SD_PPF_ON_MASK   (0x01 << 3)
  #define LINE_IN_SHIFT_MASK   (0x03 << 0)
        
#define CVBS_TT_CTL  (0x648)
  #define TELETEXT_CTRL_MASK   (0x3F << 0)
        
#define CVBS_TELTXT1  (0x64C)
  #define TELETEXT_ADDR_MASK   (0xFFFFFFF)

#define CVBS_TELTXT2  (0x650)
  #define TT_BGN_HCNT_MASK   (0x7FF << 16)
  #define TT_END_HCNT_MASK   (0x7FF << 0)
        
#define CVBS_TELTXT3  (0x654)
  #define TT_ADDR_NUM_MASK   (0xF << 28)
  #define TT_BUFR_WIDTH_MASK   (0x1FF << 16)
  #define TT_BGN_LINE_MASK   (0x1F << 8)
  #define TT_END_LINE_MASK   (0x1F << 0)
        
#define CVBS_TELTXT4  (0x658)
  #define TT_P2_MASK   (0x7FF << 16)
  #define TT_P1_MASK   (0x1FF << 0)
        
#define CVBS_TELTXT5  (0x65C)
  #define TT_CODE_END_MASK   (0x1FF << 16)
  #define TT_SYN_MASK   (0xFF)
        
#define CVBS_Y_FILTER_COEFF_GROUP_1  (0x700)
#define CVBS_Y_FILTER_COEFF_GROUP_2  (0x704)
#define CVBS_Y_FILTER_COEFF_GROUP_3  (0x708)
#define CVBS_Y_FILTER_COEFF_GROUP_4  (0x70C)
#define CVBS_Y_FILTER_COEFF_GROUP_5  (0x710)
#define CVBS_C_FILTER_COEFF_GROUP_1  (0x714)
#define CVBS_C_FILTER_COEFF_GROUP_2  (0x718)
	
#define HAL_READ32(_reg_)           			(*((volatile UINT32*)(_reg_)))
#define HAL_WRITE32(_reg_, _val_)   			(*((volatile UINT32*)(_reg_)) = (_val_))
	
#define IO_READ32(base, offset)						HAL_READ32((base) + (offset))
#define IO_WRITE32(base, offset, value)		HAL_WRITE32((base) + (offset), (value))
	
#define TVE_READ32(offset) 								IO_READ32(DISPSYS_TVE_BASE, (offset))	                         
#define TVE_WRITE32(offset, value) 				IO_WRITE32(DISPSYS_TVE_BASE, (offset), (value))

typedef struct _HAL_8127_CVBS_TVE_FIELD_T
{
	/* 0x00 => TVE sync for SD and reset */
	union
	{
		UINT32 u4EncSyn;
		
		struct 
		{
			UINT32 u4Y2HBgn				:	6;	/* Y2HBGN */
			UINT32 u4C2HBgn				:	6;	/* C2HBGN */
			UINT32 fgUvSwap				:	1;	/* UV swap */
			UINT32								:	1;
			UINT32 u4LineShift		:	2;	/* Line shift */
			UINT32 u4SdSynLvl			:	8;	/* SD sync level */
			UINT32								:	1;
			UINT32 u4YLPFCoefSet	:	2;
			UINT32 u4CLPFCoefSet	:	1;
			UINT32 u4SdSynrfadj		:	1;
			UINT32 								:	1;
			UINT32 u4Rst					:	2;	/* Reset TVE */
		} rEncSyn;
		
	} uEncSyn;
	
	/* 0x04 => TVE mode */
	union
	{
		UINT32 u4EncMode;
		
		struct 
		{
			UINT32 fgEncOff				:	1;	/* disable tv encoder */
			UINT32 fgCbOn					:	1;	/* color bar on */
			UINT32 fgSetup				:	1;	/* setup: 7.5 IRE for Composite/S-video */
			UINT32 fgOldLine			:	1;	/* setup2: 7.5 IRE for Component */
			UINT32 								:	1;
			UINT32 fgCLPOn				:	1;	/* UV low pass filter on */
			UINT32 								:	1;	/* use programmable burst freq setting */
			UINT32 fgYLPOn				:	1;	/* Y low pass filter on */

			UINT32 u4YDelay				:	2;	/* delay of input Y */
			UINT32 u4SYDelay			:	2;	/* delay of Y in Composite/S-video */
			UINT32 								:	2;	/* delay of Y in Component */
			UINT32 fgUvUpOn				:	1;	/* UV up-sampling on */
			UINT32 								:	1;	/* CBCR up-sampling on */

			UINT32 u4OutMode0			:	2;	/* output mode (Composite/S-Video) */
			UINT32 fgCrcClrCvbs		:	1;
			UINT32 fgCrcClrData		:	1;
			UINT32 fgGSync				:	1;	/* sync on Y/G channel on */
			UINT32 								:	1;	/* MT5351 BA: composite burst advanced */
			UINT32 fgTvFld				:	1;	
			UINT32 u4EncTst				:	2;	
			UINT32 fgSlewOff			:	1;	/* SLEW off */
			UINT32 								:	1;	
			UINT32 fgFulW					:	1;	/* full width output on */
			UINT32 fgCrcCvbsInt		:	1;	/* 0: normal color bar (8 colors) */
			UINT32 fgCrcDataInt		:	1;	
			UINT32 fgBlker				:	1;	/* blacker-than-black mode on */
			UINT32 fgTTRst				:	1;	/* rest teltext function */
		} rEncMode;
		
	} uEncMode;

	/* 0x08 => Closed Caption */
	union
	{
		UINT32 u4Cc;
			
		struct 
		{
			UINT32 u4CcData0		:	8;
			UINT32 u4CcData1		:	8;
			UINT32 u4CcMode			:	2;
			UINT32							:	14;
		} rCc;
		
	} uCc;

	/* 0x0C => Y Scale */
	union
	{
		UINT32 u4YScale;
		
		struct
		{
			UINT32 u4YScale				:	8;	/* Gain of Y [7:0] */
			UINT32 u4ColorWidth		:	8;	/* Color bar width */
			UINT32 u4BlankLevel		:	4;	/* Y data at this level will be blank */
			UINT32 u4Bright2			:	4;	/* brightness adjustment */
			UINT32 fgYScale8			:	1;	/* Gain of Y [8] */
			UINT32								:	1;
			UINT32 u4CbDelay			:	2;	/* cb delay */
			UINT32 u4CrDelay			:	2;	/* cr delay */
			UINT32 fgMixUVOffset	:	1;
			UINT32								:	1;
		} rYScale;
		
	} uYScale;

	/* 0x10 => Cb Scale -->TTX test data */
	union
	{
		UINT32 u4TTXTestData;

		struct 
		{
			UINT32 u4TTXTestData0		:	8;
			UINT32 u4TTXTestData1		:	8;
			UINT32 u4TTXTestData2		:	8;
			UINT32 u4TTXTestData3		:	8;
		} rTTXTestData;
		
	} uTTXTestData;
			
	/* 0x18 => U Scale */
	union
	{
		UINT32 u4UGain;

		struct 
		{
			UINT32 u4UGain0	:	8;
			UINT32					:	24;		
		} rUGain;
		
	} uUGain;
	
	/* 0x1C => V Scale */
	union
	{
		UINT32 u4VGain;

		struct 
		{
			UINT32 u4VGain0	:	8;
			UINT32					:	24;		
		} rVGain;
		
	} uVGain;
	
	/* 0x20 => Gamma Coeff 1 */
	union
	{
		UINT32 u4GammaCoeff1;

		struct 
		{
			UINT32 u4Gamma0					:	12;
			UINT32 u4ActiveVbiLine	:	5;
			UINT32 u4ActiveVbiEn		:	1;
			UINT32 									:	2;
			UINT32 u4Bright1				:	8;
		} rGammaCoeff1;
		
	} uGammaCoeff1;

	/* 0x24 => Gamma Coeff 2 */
	union
	{
		UINT32 u4GammaCoeff2;

		struct 
		{
			UINT32 u4Gamma1			:	12;
			UINT32 							:	4;
			UINT32 u4Gamma2			:	12;
			UINT32 							:	4;			
		} rGammaCoeff2;
		
	} uGammaCoeff2;

	/* 0x28 => Gamma Coeff 3 */
	union
	{
		UINT32 u4GammaCoeff3;

		struct 
		{
			UINT32 u4Gamma3			:	12;
			UINT32 							:	4;
			UINT32 u4Gamma4			:	12;
			UINT32 							:	4;			
		} rGammaCoeff3;
		
	} uGammaCoeff3;

	/* 0x2C => Gamma Coeff 4 */
	union
	{
		UINT32 u4GammaCoeff4;

		struct 
		{
			UINT32 u4Gamma5			:	12;
			UINT32 							:	4;
			UINT32 u4Gamma6			:	12;
			UINT32 							:	4;			
		} rGammaCoeff4;
		
	} uGammaCoeff4;

	/* 0x30 => Gamma Coeff 5 */
	union
	{
		UINT32 u4GammaCoeff5;

		struct 
		{
			UINT32 u4Gamma7			:	12;
			UINT32 							:	4;
			UINT32 u4Gamma8			:	12;
			UINT32 							:	4;			
		} rGammaCoeff5;
		
	} uGammaCoeff5;

	/* 0x34 => DAC Configuration */
	union
	{
		UINT32 u4DacCtrl;
		
		struct 
		{
			UINT32 u4DAC1					:	2;	
			UINT32 								:	2;
			
			UINT32 								:	2;	
			UINT32 u4DetLen				:	2;	
			UINT32 fgPlugOnEn			:	1;	
			UINT32 								:	3;	
			UINT32 u4IbandPlugDet	:	2;
			UINT32 u4u4IbandX			:	2;
                                          UINT32 : 6;
			UINT32 u4plugref        :       2;
			UINT32 								:	8;
		} rDacControl;
		
	} uDacControl;

	/* 0x38 => Color Burst Configuration 1 */
	union
	{
		UINT32 u4Burst1;

		struct 
		{
			UINT32 u4BurstLvl			:	8;
			UINT32 u4Sch					:	7;
			UINT32 fgPal60				:	1;
			UINT32 								:	8;
			UINT32								:	1;
			UINT32 fgExtGainEn		:	1;
			UINT32 u4DemodGain		:	6;
		} rBurst1;
		
	} uBurst1;

	/* 0x3C => Chroma Gain for SC */
	union
	{
		UINT32 u4ChromaGain;

		struct 
		{
			UINT32 u4ChromaGain		:	8;
			UINT32 fgChromaGainEn	:	1;
			UINT32				 				:	1;
			UINT32 u4VdoinGain		:	6;
			UINT32 u4Y2ActiveEnd	:	6;
			UINT32				 				:	4;
			UINT32 u4C2ActiveEnd	:	6;
		} rChromaGain;

	} uChromaGain;
		
	
	/* 0x44 => WSS in Interlace Mode (480i) */
	union
	{
		UINT32 u4Wss480i;

		/* for 480i */
		struct 
		{
		    UINT32 u4Word2_Crc	    :	6; 
		    UINT32 	                :	3;  /* b11 ~ b13 */
		    UINT32 u4Word2_b10			:	1;
		    UINT32 u4Word2_b8b9			:	2;
		    UINT32 u4Word2_b6b7			:	2;
		    UINT32 u4Word1		    	:	4;
			UINT32 u4Word0		    		:	2;  /* MSB go first */
			UINT32 fgWssOn						:	1;	/* Wss enable */
			UINT32 fgFixWssShoot			:	1;	
			UINT32										:	2;
			UINT32 u4WssLvl						:	8;	/* Wss Level */
		} rWss480i;
		
	} uWss480i;

	/* 0x44 => WSS in Interlace Mode (576i) */
	union
	{
		UINT32 u4Wss576i;

		/* for 576i */
		struct 
		{
		    UINT32 u4Cgmsa		    :	2;
		    UINT32 		            :	8;
		    UINT32 u4Wss		    	:	4;
			UINT32 		            	:	6;  /* MSB go first */
			UINT32 fgWssOn					:	1;	/* Wss enable */
			UINT32 fgFixWssShoot		:	1;
			UINT32									:	2;
			UINT32 u4WssLvl					:	8;	/* Wss Level */
		} rWss576i;
		
	} uWss576i;
		
	/* 0x6C => Y/G Limit */
	union
	{
		UINT32 u4YGLimit;

		struct 
		{
			UINT32 u4TopThrd			:	8;
			UINT32 u4B0tThrd			:	9;
			UINT32 fgTvIFldXor		:	1;
			UINT32 fgTvIFldSel		:	1;
			UINT32 								:	13;
		} rYGLimit;
		
	} uYGLimit;

	/* 0x70 => DSIP_AREA */
	union
	{
		UINT32 u4DispArea;

		struct 
		{
			UINT32 u4DispEndPxl			:	12;
			UINT32 u4ForAutoDetect      :    1;
			UINT32 									:	2;
			UINT32 fgNewDinEn				:	1;
			UINT32 u4DispBgnPxl			:	12;			
			UINT32 fgAgcSkewSel			:	1;	/* cvbs agc skew sel */
			UINT32 fgPorchSkewSel		:	1;	/* cvbs porch skew sel */
			UINT32 fgPseudoSkewSel	:	1;	/* cvbs pseudo skew sel */
			UINT32 									:	1;

		} rDispArea;
		
	} uDispArea;

	/* 0x74 => Burst2 */
	union
	{
		UINT32 u4Burst2;

		struct 
		{
			UINT32 u4BurstEnd			:	8;
			UINT32 u4BurstStart		:	8;
			UINT32 mv_burst_end : 8;	
                                          UINT32 mv_burst_start : 8;	
		} rBurst2;
		
	} uBurst2;	

	/* 0x78 => Mux control */
	union
	{
		UINT32 u4MuxCtrl;

		struct 
		{
			UINT32 										:	1;
			UINT32 fgCVBSP2IEn				:	1;
			UINT32 										:	1;
			UINT32 fgPipRndEn					:	1;
			UINT32 fgVdacTestEN				:	1;
			UINT32 fgCvbsOsdSel				:	1;
			UINT32 fgCvbsOsdSyncSel		:	1;
			UINT32 										:	1;
			UINT32 fgPrgsSelfEn				:	1;
			UINT32 fgPrgsOut					:	1;
			UINT32 u4TvType						:	2;	
			UINT32 fgPipDataSyncOff		:	1;
			UINT32 fgPipMixerOn				:	1;
			UINT32 fgPipMixerBlacker	:	1;
			UINT32 										:	1;
			UINT32 fgMixer1Sel				:	1;
			UINT32 fgMixer2Sel				:	1;
			UINT32 fgMixerDac1Sel			:	1;	
			UINT32 fgMixerDac2Sel			:	1;
			UINT32 fgAdDaTest1				:	1;
			UINT32 fgAdDaTest2				:	1;
			UINT32 										:	1;
			UINT32 										:	1;
			UINT32 fgPalLineMissing		:	1;    //eco00037953:Pal 336 line missing
			UINT32 fgPalBurstError		:	1;    //eco00038106:burst squence error at pal b/d
			UINT32 										:	6;
		} rMuxCtrl;

	} uMuxCtrl;

	/* 0x7C => Sync Control  */
	union
	{
		UINT32 u4SyncCtrl;
		
		struct 
		{
			UINT32 fgRev						:	1;
			UINT32									:	1;
			UINT32 fgAdjSyncEn			:	1;
			UINT32 fgAdjForward			:	1;
			UINT32 fgOldLineToggle	:	1;
			UINT32 fgFieldPol				:	1;
			UINT32 fgVsyncPol				:	1;
			UINT32 fgHsyncPol				:	1;			
			UINT32 fgHsyncEdgeSel		:	1;
			UINT32 fgVsyncEdgeSel		:	1;
			UINT32 fgAdjSelfEn			:	1;	
			UINT32 fgAdjTotalEn			:	1;
			UINT32 fgSelfYActiveEn	:	1;
			UINT32 									:	3;	
			UINT32 u4YUVSelInit			:	2;
			UINT32 u4YDelay					:	2;
			UINT32 u4CbDelay				:	2;	
			UINT32 u4CrDelay				:	2;
			UINT32 									:	8;
		} rSyncCtrl;

	} uSyncCtrl;

	/* 0x80 => Sync timing Control 0 */
	union
	{
		UINT32 u4SyncTime0;
		
		struct 
		{
			UINT32 u4AdjHTotal		:	12;
			UINT32								:	4;
			UINT32 u4AdjVTotal		:	11;
			UINT32								:	5;
		} rSyncTime0;

	} uSyncTime0;

	/* 0x84 => Sync timing Control 1 */
	union
	{
		UINT32 u4SyncTime1;
		
		struct 
		{
			UINT32 u4AdjHsync		:	12;
			UINT32							:	4;
			UINT32 u4AdjVsync		:	12;
			UINT32							:	4;
		} rSyncTime1;

	} uSyncTime1;

	/* 0x88 => Sync timing Control 2 */
	union
	{
		UINT32 u4SyncTime2;
		
		struct 
		{
			UINT32 u4ActLineOs		:	12;
			UINT32								:	4;
			UINT32 u4ActLineOe		:	11;
			UINT32								:	5;
		} rSyncTime2;

	} uSyncTime2;

	/* 0x8C => Sync timing Control 3 */
	union
	{
		UINT32 u4SyncTime3;
		
		struct 
		{
			UINT32 u4ActLineEs		:	12;
			UINT32								:	4;
			UINT32 u4ActLineEe		:	12;
			UINT32								:	4;
		} rSyncTime3;

	} uSyncTime3;
	
	/* 0x94 => abist control */
	union
	{
		UINT32 u4AbistControl;
			
		struct 
		{
			UINT32 u4SigGenGain 						:	8;
			UINT32 fgDac1En     						:	1;
			UINT32 fgDac2En     						:	1;
			UINT32 fgSwRst  								:	1;
			UINT32 fgSreSel									:	1;
			UINT32 fgDac2SigGenInvertPhase	:	1;
			UINT32 fgDspHiBitSel						:	1;
			UINT32 fgDspDataSelOrder				:	1;
			UINT32                  				:   1;
			UINT32 u4SigGenP1   						:	10;
			UINT32                  				:   6;
		} rAbistControl;
	
	} uAbistControl;

	/* 0x9C => VPS ctrl1 */
	union
	{
		UINT32 u4VpsCtrl1;
		
		struct 
		{
			UINT32 fgVpsOn					:	1;
			UINT32									:	3;
			UINT32 u4LvlAdj					:	3;
			UINT32 fgWaveformType		:	1;
			UINT32 u4VpsP1					:	9;
			UINT32 									:	3;
			UINT32 u4VpsP2					:	11;
			UINT32									:	1;
		} rVpsCtrl1;

	} uVpsCtrl1;

	/* 0xA0 => VPS data1 */
	union
	{
		UINT32 u4VpsData1;
	} uVpsData1;

	/* 0xA4 => VPS data2 */
	union
	{
		UINT32 u4VpsData2;
	} uVpsData2;

	/* 0xA8 => VPS data3 */
	union
	{
		UINT32 u4VpsData3;
	} uVpsData3;

	/* 0xAC => VPS data4 and VPS ctrl2 */
	union
	{
		UINT32 u4VpsCtrl2;

		struct 
		{
			UINT32 u4VpsEndPxl		:	8;
			UINT32 u4VpsBgnPxl		:	8;
			UINT32 u4VpsBgnLine		:	8;
			UINT32 u4VpsData4			:	8;
		} rVpsCtrl2;
	} uVpsCtrl2;	

	/* 0xBC => u4AbistSelfGenControl*/
	union
	{
		UINT32 u4AbistSelfGenControl;
			
		struct 
		{
			UINT32 u4AbistDcLevel	:	12;
			UINT32 u4AbistRampDiv	:	3;
			UINT32          			:	1;
			UINT32 u4SqrHfPeriod	:	10;
			UINT32      					:	2;
			UINT32 u4AbistPatSel2	:	2;
			UINT32 u4AbistPatSel1	:	2;
		} rAbistSelfGenControl;
	
	} uAbistSelfGenControl;

	/* 0xC0 => u4AbistSelfGenControl*/
	union
	{
		UINT32 u4AbistPattenGainOffset;
		
		struct 
		{
			UINT32 u4AbistSinOffset	:	12;
			UINT32 u4AbistRampOffset:	12;
			UINT32 u4AbistRampGain	:	8;
		} rAbistPattenGainOffset;

	} uAbistPattenGainOffset;
} HAL_8127_CVBS_TVE_FIELD_T;


#define TVE_TVDPLL_CON0	0x250
	#define RG_TVE_TVDPLL_EN			(1)
	#define RG_TVE_TVDPLL_POSDIV				(4)
	#define RG_TVE_TVDPLL_POSDIV_MASK			(0x07 << 4)
#define TVE_TVDPLL_CON1	0x254
	#define RG_TVE_TVDPLL_SDM_PCW				(0)
	#define RG_TVE_TVDPLL_SDM_PCW_MASK			(0x1FFFFF)
#define TVE_TVDPLL_PWR	0x25C
	#define RG_TVE_TVDPLL_PWR_ON		(1)

#define TVE_ANALOG_BASE		0xF0209000

#define vWriteTVEANA(dAddr, dVal)  (*((volatile unsigned int *)(TVE_ANALOG_BASE + dAddr)) = (dVal))
#define dReadTVEANA(dAddr)         (*((volatile unsigned int *)(TVE_ANALOG_BASE + dAddr)))
#define vWriteTVEANAMsk(dAddr, dVal, dMsk) (vWriteTVEANA((dAddr), (dReadTVEANA(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))


#endif /* _TVE_HW_8127_H_ */
