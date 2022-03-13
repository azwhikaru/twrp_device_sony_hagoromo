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
#ifndef _VDEC_VERIFY_COMMON_H_
#define _VDEC_VERIFY_COMMON_H_

#include "../include/vdec_info_common.h"


void vOutputPOCData(UINT32 dwDecOrder);
UCHAR ucVDecGetMinPOCFBuf(UINT32 u4InstID,VDEC_INFO_DEC_PRM_T *tVerMpvDecPrm, BOOL fgWithEmpty);
void vVerifyClrFBufInfo(UINT32 u4InstID, UINT32 u4FBufIdx);
void vFlushDPB(UINT32 u4InstID, VDEC_INFO_DEC_PRM_T *tVerMpvDecPrm, BOOL fgWithOutput);
void vVerInitVDec(UINT32 u4InstID);
void vVParserProc(UINT32 u4InstID);
void vVDecProc(UINT32 u4InstID);
void vChkVDec(UINT32 u4InstID);

#endif


