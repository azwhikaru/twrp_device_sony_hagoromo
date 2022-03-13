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

#ifndef __MTK_IR_REGS_H__
#define __MTK_IR_REGS_H__

#include <mach/memory.h>

#define IRRX_BASE_PHY        (u32)0x10013000
#define IRRX_BASE_VIRTUAL    (u32)(IO_PHYS_TO_VIRT(IRRX_BASE_PHY))


 
#define IRRX_CLK_FREQUENCE    32*1000            // 32KHZ

/**************************************************
    IRRX register define
    ************************************************/
#define IRRX_COUNT_HIGH_REG        0x0000  
#define IRRX_CH_BITCNT_MASK         0x0000003f
#define IRRX_CH_BITCNT_BITSFT       0
#define IRRX_CH_1ST_PULSE_MASK      0x0000ff00
#define IRRX_CH_1ST_PULSE_BITSFT    8
#define IRRX_CH_2ND_PULSE_MASK      0x00ff0000
#define IRRX_CH_2ND_PULSE_BITSFT    16
#define IRRX_CH_3RD_PULSE_MASK      0xff000000
#define IRRX_CH_3RD_PULSE_BITSFT    24




  
#define IRRX_COUNT_MID_REG         0x0004
#define IRRX_COUNT_LOW_REG         0x0008


#define IRRX_CONFIG_HIGH_REG     0x000c 

#define IRRX_CH_DISPD        ((u32)(1 << 15)) 
#define IRRX_CH_IGB0         ((u32)(1 << 14))
#define IRRX_CH_CHKEN        ((u32)(1 << 13))   //enable puse width
#define IRRX_CH_DISCH        ((u32)(1 << 7))
#define IRRX_CH_DISCL        ((u32)(1 << 6))
#define IRRX_CH_IGSYN        ((u32)(1 << 5))
#define IRRX_CH_ORDINV       ((u32)(1 << 4))
#define IRRX_CH_RC5_1ST      ((u32)(1 << 3))
#define IRRX_CH_RC5          ((u32)(1 << 2))
#define IRRX_CH_IRI          ((u32)(1 << 1))
#define IRRX_CH_HWIR         ((u32)(1 << 0))


#define IRRX_CH_END_7        ((u32)(0x07 << 16))
#define IRRX_CH_END_15       ((u32)(0x0f << 16))  //[22:16]
#define IRRX_CH_END_23		 ((u32)(0x17 << 16))
#define IRRX_CH_END_31		 ((u32)(0x1f << 16))
#define IRRX_CH_END_39		 ((u32)(0x27 << 16))
#define IRRX_CH_END_47		 ((u32)(0x2f << 16))
#define IRRX_CH_END_55		 ((u32)(0x07 << 16))
#define IRRX_CH_END_63		 ((u32)(0x0f << 16))

//////////////////////////////
#define IRRX_CONFIG_LOW_REG       0x0010           //IRCFGL
#define IRRX_SAPERIOD_MASK        ((u32)0xff<<0)  //[7:0]   sampling period
#define IRRX_SAPERIOD_OFFSET      ((u32)0)  

#define IRRX_CHK_PERIOD_MASK      ((u32)0x1fff<<8)  //[20:8]   ir pulse width sample period
#define IRRX_CHK_PERIOD_OFFSET    ((u32)8)  




#define IRRX_THRESHOLD_REG        0x0014
#define IRRX_THRESHOLD_MASK      ((u32)0x7f<<0)
#define IRRX_THRESHOLD_OFFSET     ((u32)0)

#define IRRX_ICLR_MASK          ((u32)1<<7) // interrupt clear reset ir
#define IRRX_ICLR_OFFSET         ((u32)7)

#define IRRX_DGDEL_MASK          ((u32)3<<8) // de-glitch select
#define IRRX_DGDEL_OFFSET        ((u32)8)

#define IRRX_RCMM_THD_REG        0x0018

#define IRRX_RCMM_ENABLE_MASK      ((u32)0x1<<31)
#define IRRX_RCMM_ENABLE_OFFSET    ((u32)31)    // 1 enable rcmm , 0 disable rcmm

#define IRRX_RCMM_THD_00_MASK      ((u32)0x7f<<0)
#define IRRX_RCMM_THD_00_OFFSET    ((u32)0)
#define IRRX_RCMM_THD_01_MASK      ((u32)0x7f<<7)
#define IRRX_RCMM_THD_01_OFFSET    ((u32)7)

#define IRRX_RCMM_THD_10_MASK      ((u32)0x7f<<14)
#define IRRX_RCMM_THD_10_OFFSET    ((u32)14)
#define IRRX_RCMM_THD_11_MASK      ((u32)0x7f<<21)
#define IRRX_RCMM_THD_11_OFFSET    ((u32)21)


#define IRRX_RCMM_THD_REG0        0x001c
#define IRRX_RCMM_THD_20_MASK      ((u32)0x7f<<0)
#define IRRX_RCMM_THD_20_OFFSET    ((u32)0)
#define IRRX_RCMM_THD_21_MASK      ((u32)0x7f<<7)
#define IRRX_RCMM_THD_21_OFFSET    ((u32)7)



#define IRRX_IRCLR                0x0020
#define IRRX_IRCLR_MASK           ((u32)0x1<<0)
#define IRRX_IRCLR_OFFSET         ((u32)0)

#define IRRX_EXPEN                0x0024
#define IRRX_EXPEN_MASK           ((u32)0xff<<0)
#define IRRX_EXPEN_OFFSET         ((u32)0)



#define IRRX_EXPBCNT               0x0028
#define IRRX_IRCHK_CNT           ((u32)0x7f)
#define IRRX_IRCHK_CNT_OFFSET         ((u32)6)


#define IRRX_EXP_IRM1                0x0060   //expect byte0 ,byte1,byte2,byte3 [31:0]
#define IRRX_EXP_IRL1                0x0038   //expect byte4 ,byte5,byte6          [23:0]

// 

#define IRRX_CHKDATA0               0x0088
#define IRRX_CHKDATA1               0x008C
#define IRRX_CHKDATA2               0x0090
#define IRRX_CHKDATA3               0x0094
#define IRRX_CHKDATA4               0x0098
#define IRRX_CHKDATA5               0x009C
#define IRRX_CHKDATA6               0x00a0
#define IRRX_CHKDATA7               0x00a4
#define IRRX_CHKDATA8               0x00a8
#define IRRX_CHKDATA9               0x00ac
#define IRRX_CHKDATA10              0x00b0
#define IRRX_CHKDATA11              0x00b4
#define IRRX_CHKDATA12              0x00b8
#define IRRX_CHKDATA13              0x00bc
#define IRRX_CHKDATA14              0x00c0
#define IRRX_CHKDATA15              0x00c4
#define IRRX_CHKDATA16              0x00c8

#define IRRX_IRINT_EN              0x00cc
#define IRRX_INTEN_MASK           ((u32)0x1<<0)
#define IRRX_INTEN_OFFSET         ((u32)0)


#define IRRX_IRINT_CLR              0x00d0
#define IRRX_INTCLR_MASK           ((u32)0x1<<0)
#define IRRX_INTCLR_OFFSET         ((u32)0)

#define IRRX_WDTSET             0x00d4    //WDTSET
#define IRRX_WDT                0x00d8    //WDT

#define IRRX_INTSTAT_REG        0x00d4   //here must be care, 8127 
#define IRRX_INTSTAT_OFFSET     ((u32)18)


#define IRRX_WDTLMT             0x00dC    //WDTLMT



#define IRRX_BASE_PHY_END        (IRRX_BASE_PHY + IRRX_WDTLMT) 
#define IRRX_BASE_VIRTUAL_END    (u32)(IO_PHYS_TO_VIRT(IRRX_BASE_PHY_END))



extern void mtk_ir_core_reg_write(u16 ui2Regs, u32 value);

#define REGISTER_WRITE32(u4Addr, u4Val)     (*((volatile unsigned long*)(u4Addr)) = (u4Val))
#define REGISTER_READ32(u4Addr)             (*((volatile unsigned long*)(u4Addr)))

#define IO_WRITE32(base, offset, u4Val)		(*((volatile unsigned long*)(base + offset)) = (u4Val))
#define IO_READ32(base, offset)              (*((volatile unsigned long*)(base + offset)))

#define IR_READ32(u4Addr)          IO_READ32(IRRX_BASE_VIRTUAL, (u4Addr))
#define IR_WRITE32(u4Addr, u4Val)  mtk_ir_core_reg_write(u4Addr,u4Val)

#define IR_WRITE_MASK(u4Addr, u4Mask, u4Offet, u4Val)  IR_WRITE32(u4Addr, ((IR_READ32(u4Addr) & (~(u4Mask))) | (((u4Val) << (u4Offet)) & (u4Mask))))
#define IR_READ_MASK(u4Addr, u4Mask, u4Offet)  ((IR_READ32(u4Addr) & (u4Mask)) >> (u4Offet))


#endif /* __IRRX_VRF_HW_H__ */

 

