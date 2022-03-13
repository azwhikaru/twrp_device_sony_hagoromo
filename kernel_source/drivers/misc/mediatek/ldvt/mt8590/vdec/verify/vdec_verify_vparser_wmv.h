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
#ifndef _VDEC_VERIFY_VPARSER_WMV_H_
#define _VDEC_VERIFY_VPARSER_WMV_H_

#include <mach/mt_typedefs.h>

UINT32 u4DecodeVOLHead_WMV3(UINT32 u4InstID);
void vRCVFileHeader(UINT32 u4InstID);
UINT32 u4DecodeVOLHead_WMV12(UINT32 u4InstID);
BOOL fgVParserProcWMV(UINT32 u4InstID);
void vWMVSearchSliceStartCode(UINT32 u4InstID);
void AdjustReconRange(UINT32 u4InstID);
void UpdateVopheaderParam(UINT32 u4InstID);


#endif // _PR_EMU_H_

