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
#ifndef _VDEC_VERIFY_VDEC_H_
#define _VDEC_VERIFY_VDEC_H_

#include <mach/mt_typedefs.h>

extern void vVDecVerifyThread(void *param_array);
extern void vSecBSVerifyThread(void *param_array);
extern void vVDecClockSelect(void);
extern void vDrmaBusySet (UINT32  u4InstID);
extern void vDrmaBusyOff (UINT32  u4InstID);

#if VDEC_MVC_SUPPORT
#define fgIsDepView() (_ucNalUnitType == H264_SLICE_EXT)
#endif

#endif // _PR_EMU_H_

