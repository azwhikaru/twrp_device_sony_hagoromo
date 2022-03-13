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
#ifndef _DGI_FMT_HW_H_
#define _DGI_FMT_HW_H_

#define DGI_GRL_BASE            (0xFC008000)
#define DGI_SYS_BASE   			(0xFC008500)
#define HDMI_RGB_REG_BASE       (0xFC008800)
#define CKGEN_BASE              (0xF0000000)

#define vWriteRegDgi(dAddr, dVal)  (*((volatile unsigned int *)(DGI_GRL_BASE + dAddr)) = (dVal)) 
#define bReadRegDgi(dAddr)         (*((volatile unsigned int *)(DGI_GRL_BASE + dAddr))) 
#define vWriteDgiMsk(dAddr, dVal, dMsk) vWriteRegDgi((dAddr), (bReadRegDgi(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

#define vWriteDgiSYS(dAddr, dVal)  (*((volatile unsigned int *)(DGI_SYS_BASE + dAddr)) = (dVal))
#define dReadDgiSYS(dAddr)         (*((volatile unsigned int *)(DGI_SYS_BASE + dAddr)))
#define vWriteDgiSYSMsk(dAddr, dVal, dMsk) (vWriteDgiSYS((dAddr), (dReadDgiSYS(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define vWriteHdmiRGB(dAddr, dVal)  (*((volatile unsigned int *)(HDMI_RGB_REG_BASE + dAddr)) = (dVal))
#define dReadHdmiRGB(dAddr)         (*((volatile unsigned int *)(HDMI_RGB_REG_BASE + dAddr)))
#define vWriteHdmiRGBMsk(dAddr, dVal, dMsk) (vWriteHdmiRGB((dAddr), (dReadHdmiRGB(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define vWriteTOPCK(dAddr, dVal)  (*((volatile unsigned int *)(CKGEN_BASE + dAddr)) = (dVal))
#define dReadTOPCK(dAddr)         (*((volatile unsigned int *)(CKGEN_BASE + dAddr)))
#define vWriteTOPCKMsk(dAddr, dVal, dMsk) (vWriteTOPCK((dAddr), (dReadTOPCK(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define INPUT_CTRL     0x0C
    #define DGI_INBUF_TEST			   (0x01<<8)
    #define DGI_FIFO_IN_TEST		   (0x01<<9)
    #define DGI_FIFO_OUT_TEST		   (0x01<<10)
	#define DGI_Y_CHANNEL_SEL		   (0x3<<18)
		#define INPUT_Y_Y_CHANNEL        (0x0 << 18)
		#define INPUT_Y_CB_CHANNEL       (0x1 << 18)
		#define INPUT_Y_CR_CHANNEL       (0x2 << 18)
    #define DGI_C_CHANNEL_SEL		   (0x3<<20)
		#define INPUT_C_Y_CHANNEL 	   (0x0 << 20)
		#define INPUT_C_CB_CHANNEL	   (0x1 << 20)
		#define INPUT_C_CR_CHANNEL	   (0x2 << 20)
    #define DGI_C2_CHANNEL_SEL		   (0x3<<22)
		#define INPUT_C2_Y_CHANNEL 	   (0x0 << 22)
		#define INPUT_C2_CB_CHANNEL	   (0x1 << 22)
		#define INPUT_C2_CR_CHANNEL	   (0x2 << 22)
	
#define DGI_CTRL	   0x10
    #define DGI_ON			           (0x01<<0)
	#define MIX_PLN1_MSK_SEL		   (0x01<<1)
    #define MIX_PLN2_MSK_SEL		   (0x01<<2) //separate h/v sync
    #define DGI_10BIT_MODE		   	   (0x01<<3)
    #define DGI_12BIT_MODE		   	   (0x01<<4)
    #define DGI_SWAP_YC			       (0x01<<5)
	
    #define DGI_INVERSE_BIT			   (0x01<<6)
    #define DGI_FIFO_RESET_SEL		   (0x01<<7)
    #define DGI_FIFO_SW_RST			   (0x01<<8)
    #define DGI_VSYNC_POL 			   (0x01<<9)
	
    #define DGI_YUV_SEL_MODE		   (0x3<<10)
    #define DGI_HSYNC_POL			   (0x01<<12)
    #define DGI_INVERT_FIELD		   (0x01<<15)
	
    #define DGI_FIFO_OUT_CR_DEL		   (0x3<<16)
    #define DGI_FIFO_OUT_CB_DEL 	   (0x3<<18)
    #define DGI_FIFO_OUT_Y_DEL 	       (0x3<<20)
    #define DGI_MIXING_MODE 	       (0x01<<22)
    #define DGI_SWAP_UV 		       (0x01<<23)
    #define DGI_TEST_MODE_EN 		   (0x01<<26)
    #define DGI_444 			       (0x01<<27)
	#define DGI_ENCRYPT_Y			   (0x0F<<28)
#define DGI_DBG	       0x14
	#define DGI_MIX_ON			   (0x01<<14)

#define MIX_CTRL3	       0x18
	#define DGI_4FS_OPT		   (0x01<<25)
	#define DGI_2FS_OPT 	   (0x01<<26)
	
#define MIXER_CTRL4	   0x24
	#define DGI_MIX_1_OSD			   (0x01<<14)
	#define DGI_YUV_SEL 			   (0x07<<8)
	
#define DGO_MODE	   0x50
#define DGO_CTRL       0x54
#define SMPTE_SET      0x80
#define MASTER_T_CTRL  0x84
#define DGO_SYNC_ADJ   0x90
#define MODE_CTRL      0x94 
	#define Progressive_Mode_EN			   (0x01<<15)

#define VSYNC_OFS      0x98
#define FMT_HACT       0xA0
#define FMT_VOACT      0xA4
#define FMT_VEACT      0xA8
#define VIDEO_FMT_CTRL 0xAC
    #define FMT_CTRL_ON			       (0x01<<0)
    #define FMT_CTRL_HPOR			   (0x01<<3)
    #define FMT_CTRL_VPOR 			   (0x01<<4)
	
#define DGO_SWAP       0xBC
#define DGI_H_ACT     0xD0
#define HV_TOT_PIXEL   0xD4
#define DGI_VO_ACT    0xE0
#define MULTI_R_CTRL   0xE4

#define DGI_SYNC_ADJ  0xE8
#define DGI_VE_ACT    0xF0
#define MIXER_MODE     0xF4
	
#define VDTSYS2        0x04
	#define HDMI_SEL			       (0x3<<4)
	#define HDMI_MASTER_EN 			   (0x01<<10)

#define VDTCLK1        0x08
#define VDTCLK2        0x0C
#define RG_13X         0x20
#define VDTCLK3        0x2C
#define FMTCTRL        0x74
	#define HDMI_COO_SEL			   (0x01<<23)
#define RG_33X         0x88
#define VDTCLK1        0x08

#define CLK_CONFIG4        0x0000080
	#define CLK_DPI1_SEL 			   (0x03<<24)
		#define CLK_26M	                   (0x0<<24)
		#define CLK_TVDPLL	               (0x01<<24)
		#define CLK_TVDPLL_D2				   (0x02<<24)
		#define CLK_TVDPLL_D4				   (0x03<<24)
		
#define BDP_DISPSYS_DISP_CLK_CONFIG1        0xC000018
	#define NR_CLK_SEL 			   (0x01<<0)
	#define DGI_SELF_TEST_MODE	   (0x01<<1)
	#define WR_CH_DI_SEL_TEST_MODE	   (0x01<<2)
	
#define BDP_DISPSYS_VIN_CONFIG        0xC00002c
	#define VIN_SEL 			   (0x01<<0)
	
	
#define BDP_DISPSYS_CG_CON0        0xC000100
	#define HF_WR_CHANNEL_VDI_PIX_CK			   (0x01<<3)
	#define HF_WR_CHANNEL_VDI_DRAM_CK			   (0x01<<4)
	#define HF_WR_CHANNEL_BCLK_FREE_CLK 		   (0x01<<5)
	#define DGI_IN_CLK 			   (0x01<<6)
	#define DGI_OUT_CLK	           (0x01<<7)
	#define FMT_MAST_27_CK			   (0x01<<8)
	#define FMT_BCLK_FREE_CL 		   (0x01<<9)

#define AP_PLL_CON0                   0x0209000
	#define AB_REF_CLK_SEL 			   (0x01<<24)

#define AP_PLL_CON1                   0x0209004
	#define ABT_REF_CLK_DIV_SEL 			   (0x07<<10)
		#define ABT_REF_CLK_DIV 			   (0x00<<10)
		#define ABT_REF_CLK_DIV2_SEL 			   (0x01<<10)
		#define ABT_REF_CLK_DIV4_SEL 			   (0x02<<10)
		#define ABT_REF_CLK_DIV8_SEL 			   (0x03<<10)


#define HDMI_RGB_CTRL       0x00
  #define RGB2HDMI_ON          (1<<0)
  #define Y_OFF                (1<<1)
  #define RGB_MOD              (1<<2)
  #define YCBCR422_CBCR_INV         (1<<3) 
  #define LPF_RGB_DOMAIN       (1<<4)  
  #define IN_DELAY_2T          (1<<5)  
  #define ADJ_SYNC_EN          (1<<6)  
  #define YCBCR422_REPEAT_4PXL      (1<<7)  
  #define RGB_FULL_RANGE       (1<<8)  
  #define HDMI_UV_SWAP         (1<<9)  
  #define YVU_FULL             (1<<10)  
  #define UV_OFFSET            (1<<11)
		
  #define YUV_422              (1<<16)  
  #define Y_LPF_EN             (1<<17)  
  #define C_LPF_EN             (1<<18)  
  #define HALF_LPF_EN          (1<<19)  
  #define REPEAT_EN            (1<<20)  
  #define DOUBLE_EN            (1<<21)  
  #define PRGS_CLK54_EN        (1<<23)  
  #define PRGS_CLK108_EN  (1<<24) 
  #define LMT_EN  (1<<25)
  #define REP_4PXL  (1<<26)
  #define OUT_MODE_12BIT   (0<<28)
  #define OUT_MODE_8BIT    (1<<28)
  #define OUT_MODE_10BIT   (2<<28)
  #define OUT_MODE_16BIT   (3<<28)
		  
#define HDMI_RGB_GAMMA0     0x04
#define HDMI_RGB_GAMMA1     0x08
#define HDMI_RGB_GAMMA2     0x0c
#define HDMI_RGB_GAMMA3     0x10
#define HDMI_RGB_GAMMA4	 0x38
#define HDMI_RGB_CBGAIN     0x14
#define HDMI_RGB_CRGAIN     0x18
#define HDMI_RGB_TIME0      0x1c
  #define V_POLAR           (0x1 << 24)
  #define H_POLAR	    (0x1 << 25)
  #define DE_POLAR	    (0x1 << 26)
  #define DE_SEL	    (0x1 << 27)
#define HDMI_RGB_TIME1      0x20
#define HDMI_RGB_TIME2      0x24
#define HDMI_RGB_TIME3      0x28
#define HDMI_RGB_TIME4      0x2c
#define HDMI_RGB_YLPF1		0x34
#define HDMI_RGB2HDMI_4O    0x40
  #define R_DELAY           7
  #define G_DELAY 		    (7<<4)
  #define B_DELAY 		    (7<<8)
  #define Y_DELAY 		    (7<<12)
  #define CB_DELAY 		    (7<<16)
  #define CR_DELAY 		    (7<<20)
		  
#define HDMI_RGB2HDMI_44    0x44
   #define DEBUG_MONITOR_SELECT (0xf<<0)
		
#define HDMI_RGB2HDMI_48    0x48
  #define ODD_V_START_OPT   (0xF)
  #define ODD_V_END_OPT	    (0xF << 4)
  #define EVEN_V_START_OPT  (0xF << 8)
  #define EVEN_V_END_OPT	(0xF << 12)
#define HDMI_RGB2HDMI_4C    0x4C
  #define USE_UV_LMT        (1 << 26)  
  #define Y_GAMMA_OFF	    (1 << 27)  
  #define CBCR_IN_SIGN 	    (1 << 30)
#define HDMI_RGB2HDMI_50    0x50
  #define BOT_LIMIT_MASK (0xffff)
  #define TOP_LIMIT_MASK (0xffff << 16)//(0xfff << 20)  
#define HDMI_RGB2HDMI_54    0x54
  #define BOT_UV_LIMIT_MASK (0xffff)
  #define TOP_UV_LIMIT_MASK (0xffff << 16)//(0xfff << 20)  
#define HDMI_RGB2HDMI_60    0x60
  #define YCBCR2RGB_3X3_ON   (1<<31)
#define HDMI_RGB2HDMI_64    0x64
  #define YCBCR2RGB_601_ORG        (0<<28)//16-235  16-235
  #define YCBCR2RGB_601_NEW1       (1<<28)//0-255    0-255  
  #define YCBCR2RGB_601_NEW2       (2<<28)//16-235  0-255
  #define YCBCR2RGB_709_ORG        (4<<28)//16-235  16-235
  #define YCBCR2RGB_709_NEW1       (5<<28)//0-255    0-255 
  #define YCBCR2RGB_709_NEW2       (6<<28)//16-235  0-255  
  #define YCBCR2RGB_3X3_BY_COEFFICIENTS  (7<<28)
#define HDMI_RGB2HDMI_90      0x90
		
#define HDMI_RGB2HDMI_B0      0xB0
  #define RGB2HDMI_ADJUST_TOTAL    (1<<31)
  #define RGB2HDMI_HSYN_TOTAL_BIT12	(1 << 30)
  #define RGB2HDMI_PRGS                    (1<<29)
  #define RGB2HDMI_ADJUST_PRGS      (1<<28)
  #define  RGB2HDMI_HSYN_TOTAL       (0xfff<<16)
  #define  RGB2HDMI_VSYN_TOTAL       (0xfff)
		  
#define HDMI_RGB2HDMI_B4      0xB4
  #define ADJ_HSYNC_DELAY_MASK	0xFFF
  #define ADJ_VSYNC_DELAY_MASK  (0xFFF << 16)
  #define ADJ_FORWARD           (1 << 31)
#define HDMI_RGB2HDMI_C0      0xC0
  #define HSYNC_DELAY 		 (1)
  #define DOUBLE422_DELAY 	 (1<<4)
		
#define HDMI_RGB2HDMI_F0	0xF0
  #define RGB2HDMI_ALTERNATIVE_OSTART2  (0xFFF << 16)
  #define RGB2HDMI_ALTERNATIVE_OEND2  (0xFFF << 0)
#define HDMI_RGB2HDMI_F4	0xF4
  #define RGB2HDMI_ALTERNATIVE_ESTART2  (0xFFF << 16)
  #define RGB2HDMI_ALTERNATIVE_ENABLE	(1 << 15)
  #define RGB2HDMI_ALTERNATIVE_EEND2  (0xFFF << 0)
		
#define HDMI_RGB2HDMI_F8      0xF8
		
#define HDMI_RGB2HDMI_FC      0xFC
   #define FIELD_STATUS            (3<<2)  


#endif
