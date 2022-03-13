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
#ifndef _VDEC_VERIFY_VPARSER_VP6_H_
#define _VDEC_VERIFY_VPARSER_VP6_H_

#include <mach/mt_typedefs.h>

void vVerInitVP6(UINT32 u4InstID);
UINT32 u4VerVParserVP6(UINT32 u4InstID, BOOL fgInquiry);
void vVerifyVDecSetVP6Info(UINT32 u4InstID);
void vVerVP6VDecEnd(UINT32 u4InstID);
void vVerVP6DecEndProc(UINT32 u4InstID);
BOOL fgVP6CRCPatternExist(UINT32 u4InstID);
BOOL fgVP6SmallFolder(UINT32 u4InstID);


#endif // _PR_EMU_H_

