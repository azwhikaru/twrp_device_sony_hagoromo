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
*[File]             hal_io.c
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

#include "hal_io.h"
#include "hdmirx.h"
#include "typedef.h"
//#include "x_os.h"

UINT16 u2HdmiRxIO32Read2B(UINT32 reg32)
{
    UINT32 addr=reg32&~3;
    switch(reg32&3)
    {
        default:
        case 0:
        case 2:
            return (*(volatile UINT16 *)(reg32));
        case 1:
            return  ((*(volatile UINT32 *)(addr))>>8)&0xffff;
        case 3:
            //ASSERT((reg32&3)<3);
            return  ((*(volatile UINT32 *)(addr))>>24)&0xff;
    }
}

void vHdmiRxIO32Write1BMsk(UINT32 reg32, UINT32 val8, UINT8 msk8)
{
    //CRIT_STATE_T csState;
    UINT32 u4Val, u4Msk;
    UINT8 bByte;

    bByte = reg32&3;
    reg32 &= ~3;
    val8 &= msk8;
    u4Msk = ~(UINT32)(msk8<<((UINT32)bByte<<3));

    //csState = x_crit_start();
    u4Val = (*(volatile UINT32 *)(reg32));
    u4Val = ((u4Val & u4Msk) | ((UINT32)val8 << (bByte<<3)));
    (*(volatile UINT32 *)(reg32)=(u4Val));
    //x_crit_end(csState);

}

void vHdmiRxIO32Write2BMsk(UINT32 reg32, UINT32 val16, UINT16 msk16)
{
    //CRIT_STATE_T csState;
    UINT32 u4Val, u4Msk;
    UINT8 bByte;

    bByte = reg32&3;
    //ASSERT(bByte<3);

    reg32 &= ~3;
    val16 &= msk16;
    u4Msk = ~(UINT32)(msk16<<((UINT32)bByte<<3));

    //csState = x_crit_start();
    u4Val = (*(volatile UINT32 *)(reg32));
    u4Val = ((u4Val & u4Msk) | ((UINT32)val16 << (bByte<<3)));
    (*(volatile UINT32 *)(reg32)=(u4Val));
    //x_crit_end(csState);

}

void vHdmiRxIO32Write4BMsk(UINT32 reg32, UINT32 val32, UINT32 msk32)
{
    //CRIT_STATE_T csState;

    //ASSERT((reg32&3)==0);

    val32 &=msk32;

    //csState = x_crit_start();
    (*(volatile UINT32 *)(reg32)=((*(volatile UINT32 *)(reg32))&~msk32)|val32);
    //x_crit_end(csState);
}

