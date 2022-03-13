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
*[File]             hal_io.h
*[Version]          v0.1
*[Revision Date]    2010-01-04
*[Author]           Kenny Hsieh
*[Description]
*    source file for global varabile and function in av_d directory
*
*
******************************************************************************/

#ifndef HAL_IO_H
#define HAL_IO_H

#include "hdmi_rx_hw.h"
#include "typedef.h"

UINT16 u2HdmiRxIO32Read2B(UINT32 reg32);
void vHdmiRxIO32Write1BMsk(UINT32 reg32, UINT32 val8, UINT8 msk8);
void vHdmiRxIO32Write2BMsk(UINT32 reg32, UINT32 val16, UINT16 msk16);
void vHdmiRxIO32Write4BMsk(UINT32 reg32, UINT32 val32, UINT32 msk32);


extern void Assert(const char* szExpress, const char* szFile, int i4Line);
#undef ASSERT				
#define ASSERT(x)        ((x) ? (void)0 : Assert(#x, __FILE__, __LINE__))

// field access macro-----------------------------------------------------------

/* field macros */
#define Fld(wid,shft,ac)    (((UINT32)wid<<16)|(shft<<8)|ac)
#define Fld_wid(fld)    (UINT8)((fld)>>16)
#define Fld_shft(fld)   (UINT8)((fld)>>8)
#define Fld_ac(fld)     (UINT8)(fld)

/* access method*/
#define AC_FULLB0       1
#define AC_FULLB1       2
#define AC_FULLB2       3
#define AC_FULLB3       4
#define AC_FULLW10      5
#define AC_FULLW21      6
#define AC_FULLW32      7
#define AC_FULLDW       8
#define AC_MSKB0        11
#define AC_MSKB1        12
#define AC_MSKB2        13
#define AC_MSKB3        14
#define AC_MSKW10       15
#define AC_MSKW21       16
#define AC_MSKW32       17
#define AC_MSKDW        18

/* Reg32 to Reg8*/
#define REGB0(reg)  (reg)
#define REGB1(reg)  ((reg)+1)
#define REGB2(reg)  ((reg)+2)
#define REGB3(reg)  ((reg)+3)
/* Reg32 to Reg16*/
#define REGW0(reg)  (reg)
#define REGW1(reg)  ((reg)+1)
#define REGW2(reg)  ((reg)+2)

/* --------FLD help macros, mask32 to mask8,mask16,maskalign ----------*/
/* mask32 -> mask8 */
#define MSKB0(msk)  (UINT8)(msk)
#define MSKB1(msk)  (UINT8)((msk)>>8)
#define MSKB2(msk)  (UINT8)((msk)>>16)
#define MSKB3(msk)  (UINT8)((msk)>>24)

/* mask32 -> mask16 */
#define MSKW0(msk)  (UINT16)(msk)
#define MSKW1(msk)  (UINT16)((msk)>>8)
#define MSKW2(msk)  (UINT16)((msk)>>16)
                        /* mask32 -> maskalign */
#define MSKAlignB(msk)  (((msk)&0xff)?(msk):(\
            ((msk)&0xff00)?((msk)>>8):(\
            ((msk)&0xff0000)?((msk)>>16):((msk)>>24)\
        )\
    ))

/* --------FLD help macros, mask32 to mask8,mask16,maskalign ----------*/
#define Fld2Msk32(fld)  /*lint -save -e504 */ (((UINT32)0xffffffff>>(32-Fld_wid(fld)))<<Fld_shft(fld)) /*lint -restore */
#define Fld2MskB0(fld)  MSKB0(Fld2Msk32(fld))
#define Fld2MskB1(fld)  MSKB1(Fld2Msk32(fld))
#define Fld2MskB2(fld)  MSKB2(Fld2Msk32(fld))
#define Fld2MskB3(fld)  MSKB3(Fld2Msk32(fld))
#define Fld2MskBX(fld,byte) ((UINT8)(Fld2Msk32(fld)>>((byte&3)*8)))

#define Fld2MskW0(fld)  MSKW0(Fld2Msk32(fld))
#define Fld2MskW1(fld)  MSKW1(Fld2Msk32(fld))
#define Fld2MskW2(fld)  MSKW2(Fld2Msk32(fld))
#define Fld2MskWX(fld,byte) ((UINT16)(Fld2Msk32(fld)>>((byte&3)*8)))

#define Fld2MskAlignB(fld)  MSKAlignB(Fld2Msk32(fld))
#define FldshftAlign(fld)   ((Fld_shft(fld)<8)?Fld_shft(fld):(\
            (Fld_shft(fld)<16)?(Fld_shft(fld)-8):(\
            (Fld_shft(fld)<24)?(Fld_shft(fld)-16):(Fld_shft(fld)-24)\
        )\
    ))
#define ValAlign2Fld(val,fld)   ((val)<<FldshftAlign(fld))

#endif  // X_HAL_IO_H
