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
#ifndef _VDEC_VERIFY_VPARSER_VP9_H_
#define _VDEC_VERIFY_VPARSER_VP9_H_

//#include "x_typedef.h"
#include "../include/vdec_info_vp9.h"

void vVerInitVP9(UINT32 u4InstID);
void vVerVParserVP9(UINT32 u4InstID, BOOL fgInquiry);
void vVerDecodeVP9(UINT32 u4InstID);
void vVerVP9DecEnd(UINT32 u4InstID);
BOOL fgVP9IntraOnly(VP9_COMMON_T * prCommon);
void vVP9DumpMem(UCHAR* buf, UINT32 size ,UINT32 frame_num ,UINT32 u4Type);
BOOL vVerVP9WaitDecEnd(VP9_COMMON_T* prCommon, UINT32 u4InstID);

#endif // _PR_EMU_H_

