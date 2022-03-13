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
// 2013/03/28 Chia-Mao Hung For hevc early emulation

#ifndef _VDEC_HW_VP9_H_
#define _VDEC_HW_VP9_H_
//extern UINT32 _u4VP9LogOption;

/*
#define SIM_PRINT(x, ...) \
    do {                           \
        if(_u4VP9LogOption == 2 || _u4VP9LogOption == 3)    \
           printk(x, ##__VA_ARGS__);             \
    }while(0)
*/
//#define DRV_WriteReg(IO_BASE, dAddr, dVal)  *(volatile UINT32 *)(IO_BASE + 0xF0000000 + dAddr) = dVal
//#define DRV_ReadReg(IO_BASE, dAddr)        *(volatile UINT32 *)(IO_BASE + 0xF0000000 + dAddr)
void vVP9RISCWrite_MC( UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId);
void vVP9RISCRead_MC( UINT32 u4Addr, UINT32* pu4Value, UINT32 u4CoreId);
void vVP9RISCWrite_MV( UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId);
void vVP9RISCRead_MV(UINT32 u4Addr, UINT32* pu4Value, UINT32 u4CoreId);
void vVP9RISCWrite_PP( UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId);
void vVP9RISCRead_PP( UINT32 u4Addr, UINT32* pu4Value, UINT32 u4CoreId);
void vVP9RISCWrite_VLD_TOP( UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId);
void vVP9RISCRead_VLD_TOP( UINT32 u4Addr, UINT32* pu4Value, UINT32 u4CoreId);
void vVP9RISCRead_MISC ( UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId);
void vVP9RISCWrite_MISC( UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId);
void vVP9RISCWrite_VLD( UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId);
void vVP9RISCRead_VLD ( UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId);
void vVP9RISCRead_VDEC_TOP ( UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId);
void vVP9RISCWrite_VDEC_TOP( UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId);
void vVP9RISCRead_VP9_VLD(UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId);
void vVP9RISCWrite_VP9_VLD(UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId);
void vVP9RISCRead_BS2(UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId);
void vVP9RISCWrite_BS2(UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId);
void vVP9RISCWrite_MCore_TOP( UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId);
void vVP9RISCRead_MCore_TOP(UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId);

void vVP9RISCRead_GCON ( UINT32 u4Addr , UINT32* pu4Value);
void vVP9RISCWrite_GCON( UINT32 u4Addr, UINT32 u4Value );
void vVP9RISC_instructions();


#endif
