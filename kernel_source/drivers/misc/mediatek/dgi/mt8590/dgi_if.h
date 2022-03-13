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
#ifndef     DGI_IN_IF_H
#define     DGI_IN_IF_H

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

#include "hdmirx.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"


#define DGI_YCBCR_SWAP      0
#define DGI_YCRCB_SWAP      1
#define DGI_CBYCR_SWAP      2
#define DGI_CBCRY_SWAP      3
#define DGI_CRYCB_SWAP      4
#define DGI_CRCBY_SWAP      5

#define DGI_CBCR_SWAP_ON      1
#define DGI_CBCR_SWAP_OFF      0

void set_rx_dgi_to_rgb2hdmi_clk(void);
void vHal_Disable_DGI_In(void) ; 
void vHal_DgiYCSwap(unsigned char u1Swap);
BOOL vHal_DgiInCbCrSwap(BOOL u1Swap);
void vHal_RstDgiFifo(void) ; 
void vHal_SetDisturbLineCnt(UCHAR ucRes) ;
void vhal_hdmirx_mux_switch(BOOL fgRxIn) ;
void vHal_DgiIn_YUV_Sel(UINT8 u1Delay);
void vhal_set_dgi_timing(VIDEO_DGI_IN_MODE_T eMode,unsigned char ucRes);   
void set_src_to_dgi_in_fmt(unsigned int eDgiBitMode, unsigned char ucRes);   
void vHal_SetDgiInModeCtrl(VIDEO_DGI_IN_MODE_T eMode); 
void vSetHDMIDataEnable(BYTE bResIndex);
void vSetHDMI640x480PDataEnable(void);
void vSetHDMI3DDataEnable(BYTE bResIndex);
void rgb2hdmi_setting_res(UINT8 ui1Res, UINT8 ui1ColorSpace);
void vSetHDMISyncDelay(BYTE bResIndex, BYTE bAdjForward);
void vset_dgi_in_mode(DGI_VIDEO_IN_INFO_T rInfo, UCHAR ucRes);   
void vHal_DgiBitMode(VIDEO_DGI_IN_FORMAT_T eFmtmode, unsigned char ucRes);
void rgb2hdmisetres(unsigned char ucRes,unsigned char u1colorspace);

#endif
