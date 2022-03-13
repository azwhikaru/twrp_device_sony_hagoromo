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
*[File]             vga_table.c
*[Version]          v0.1
*[Revision Date]    2010-01-04
*[Author]           Kenny Hsieh
*[Description]
*    source file for global varabile and function in av_d directory
*
*
******************************************************************************/

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


#include "vga_table.h"
#include "hdmi_rx_ctrl.h"
#include "hdmirx.h"
#include "typedef.h"
#include "mtk_vga.h"

#define _VGA_TABLE_C_
#define VGAMODE_OFFSET 0
#define VGACAPTURE_OFFSET (VGAMODE_OFFSET + (UINT16)MAX_TIMING_FORMAT * sizeof(VGAMODE))	// 79 * 14 = 1106

#define ADCPLL_WORKAROUND   1
#define OVERSAMPLE_THRESHOLD   500 // 250//350 //25MHz


VGA_USRMODE rVgaUsrEEP[USERMODE_TIMING];	    //both  EEP & RAM
VGA_USRMODE_EXT rVgaUsrExt[USERMODE_TIMING]; //only on RAM


const UINT8 bHdtvTimings = HDTV_TIMING_NUM;
const UINT8 bUserVgaTimings = USERMODE_TIMING;
const UINT8 bAllTimings= (sizeof(VGATIMING_TABLE)/sizeof(VGAMODE));
const UINT8 bVgaTimings = (ALL_TIMING_NUM - HDTV_TIMING_NUM -USERMODE_TIMING); 
const UINT8 bUserVgaTimingBegin= (ALL_TIMING_NUM - USERMODE_TIMING);

////////////////////////////////////////////////////////////////////////////////
UINT16 Get_HDMIMODE_IHF(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IHF;
}

UINT8 Get_HDMIMODE_IVF(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IVF;
}

UINT16 Get_HDMIMODE_ICLK(UINT8 mode)
{
#if ADCPLL_WORKAROUND
       if(mode == 0xFF)
	   	 return 0;
       if(fgIsVgaTiming(mode) && (VGATIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
       {
	    return (VGATIMING_TABLE[mode].ICLK*2);
       }
       else
#endif       
       {
	    return VGATIMING_TABLE[mode].ICLK;
       }
}

UINT16 Get_HDMIMODE_IHTOTAL(UINT8 mode)
{
#if ADCPLL_WORKAROUND
        if(mode == 0xFF)
	   	 return 0;
        if(fgIsVgaTiming(mode) && (VGATIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
        {
	        return (VGATIMING_TABLE[mode].IHTOTAL*2);        
        }
        else
#endif        
        {
	        return VGATIMING_TABLE[mode].IHTOTAL;
        }
}

UINT16 Get_HDMIMODE_IVTOTAL(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IVTOTAL;
}

UINT16 Get_HDMIMODE_IPH_STA(UINT8 mode)
{
#if ADCPLL_WORKAROUND
        if(mode == 0xFF)
	   	 return 0;
        if(fgIsVgaTiming(mode) && (VGATIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
        {
	        return ((VGATIMING_TABLE[mode].IPH_BP+VGATIMING_TABLE[mode].IPH_SYNCW)*2);        
        }
        else
#endif        
        {
	        return (VGATIMING_TABLE[mode].IPH_BP+VGATIMING_TABLE[mode].IPH_SYNCW) ;
        }
}

UINT16 Get_HDMIMODE_IPH_WID(UINT8 mode)
{
#if ADCPLL_WORKAROUND
        if(mode == 0xFF)
	   	 return 0;
        if(fgIsVgaTiming(mode) && (VGATIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
        {
	        return (VGATIMING_TABLE[mode].IPH_WID*2);        
        }
        else
#endif        
        {
	        return VGATIMING_TABLE[mode].IPH_WID;
        }
}

UINT16 Get_HDMIMODE_IPH_SYNCW(UINT8 mode)
{
#if ADCPLL_WORKAROUND
        if(mode == 0xFF)
	   	  return 0;
        if(fgIsVgaTiming(mode) && (VGATIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
        {
	        return (VGATIMING_TABLE[mode].IPH_SYNCW*2);        
        }
        else
#endif        
        {
	        return VGATIMING_TABLE[mode].IPH_SYNCW;
        }
}

UINT16 Get_HDMIMODE_IPH_BP(UINT8 mode)
{
#if ADCPLL_WORKAROUND
        if(mode == 0xFF)
	   	  return 0;
        if(fgIsVgaTiming(mode) && (VGATIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
        {
	        return (VGATIMING_TABLE[mode].IPH_BP*2);        
        }
        else
#endif        
        {
	        return VGATIMING_TABLE[mode].IPH_BP;
        }
}

UINT8 Get_HDMIMODE_IPV_STA(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IPV_STA; //-Displaymode_delay; //Modify for Disaplymode delay
}

UINT16 Get_HDMIMODE_IPV_LEN(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IPV_LEN;
}

UINT16 Get_HDMIMODE_COMBINE(UINT8 mode)
{
	return VGATIMING_TABLE[mode].COMBINE;
}

 UINT8 Get_HDMIMODE_OverSample(UINT8 mode)
 {
#if ADCPLL_WORKAROUND 
        if(mode == 0xFF)
	   	 return 0;
        if(fgIsVgaTiming(mode) && (VGATIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
        {
	        return 1;        //alwasy oversample
        }
        else
#endif        
        {
	        return (VGATIMING_TABLE[mode].COMBINE &0x01);
	 }  
 }

UINT16 Get_VGAMODE_IHF(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IHF;
}

UINT8 Get_VGAMODE_IVF(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IVF;
}

UINT16 Get_VGAMODE_ICLK(UINT8 mode)
{
	 return VGATIMING_TABLE[mode].ICLK;
}

UINT16 Get_VGAMODE_IHTOTAL(UINT8 mode)
{
    UINT16 u4Htotal = 0;
	u4Htotal = VGATIMING_TABLE[mode].IHTOTAL;
	printk("[HDMI RX]u4Htotal = %d\n ",u4Htotal);
	return VGATIMING_TABLE[mode].IHTOTAL;
}

UINT16 Get_VGAMODE_IVTOTAL(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IVTOTAL;
}

UINT16 Get_VGAMODE_IPH_STA(UINT8 mode)
{
	return (VGATIMING_TABLE[mode].IPH_BP+VGATIMING_TABLE[mode].IPH_SYNCW) ;
}

UINT16 Get_VGAMODE_IPH_WID(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IPH_WID;
}

UINT16 Get_VGAMODE_IPH_SYNCW(UINT8 mode)
{
    return VGATIMING_TABLE[mode].IPH_SYNCW;
}

UINT16 Get_VGAMODE_IPH_BP(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IPH_BP;
}

UINT8 Get_VGAMODE_IPV_STA(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IPV_STA; //-Displaymode_delay; //Modify for Disaplymode delay
}

UINT16 Get_VGAMODE_IPV_LEN(UINT8 mode)
{
	return VGATIMING_TABLE[mode].IPV_LEN;
}

UINT16 Get_VGAMODE_COMBINE(UINT8 mode)
{
	return VGATIMING_TABLE[mode].COMBINE;
}

 UINT8 Get_VGAMODE_OverSample(UINT8 mode)
 {
	return (VGATIMING_TABLE[mode].COMBINE &0x01);
 }


