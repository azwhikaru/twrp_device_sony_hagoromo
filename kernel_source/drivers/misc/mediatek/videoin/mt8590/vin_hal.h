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
#ifndef _VIN_HAL_H_
#define _VIN_HAL_H_
#include "typedef.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"

void vHal_VinEnableDramIf(BOOL fgEnable);
void vHal_VinSetEnable(BOOL fgEnable);
void vhal_vin_clk_enable(BOOL fgEnable);
void vHal_VinSetBufPtr(VIDEO_IN_ADDR* prBuf);
BOOL fgHal_VinIs3DLSight(void);
BOOL fgHal_VinIsTopField(void);
UINT8 u1Hal_Vin3DFieldCnt(void);
void vin_hal_set_src_type(VIDEO_IN_SRC_ID_E esrctype);
void vclear_vin_irq(void);
void vHal_VinSetDramMode(unsigned char ucFmtMode,unsigned char ucAddrMode, unsigned char ucSwap);
void vHal_VinSetPara(unsigned char ucRes, unsigned char ucOutputcolorFmt, unsigned char ucMode,unsigned char ucInputBitMode,bool fgVgaCea);


#endif
