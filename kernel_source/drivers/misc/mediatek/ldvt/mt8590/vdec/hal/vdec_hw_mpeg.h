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
#ifndef _VDEC_HW_MPEG_H_
#define _VDEC_HW_MPEG_H_
//#include <mach/mt6575_typedefs.h>
//#include "vdec_info_common.h"



// *********************************************************************
//  Video Decoder HW Functions
// *********************************************************************
extern void vVLDDec(UINT32 u4VDecID, VDEC_INFO_DEC_PRM_T *ptDecPrm);
extern void vVLDDx3Dec(UINT32 u4VDecID, VDEC_INFO_DEC_PRM_T *ptDecPrm);
extern void vVLDM4vDec(UINT32 u4VDecID, VDEC_INFO_DEC_PRM_T *ptDecPrm, BOOL fgBVop);
extern void vVDecMpegCommSetting (UINT32 u4VDecID, VDEC_INFO_DEC_PRM_T *prDecPrm);
extern void vMCSetOutputBuf(UINT32 u4VDecID, UINT32 u4OutBufIdx, UINT32 u4FRefBufIdx);
extern void VDecDumpMP4Register(UINT32 u4VDecID);
extern void VDecDumpMP4RegisterByYiFeng(UINT32 u4VDecID);
extern void VDecDumpMpegRegister(UINT32 u4VDecID,BOOL fgTriggerAB);

#endif

