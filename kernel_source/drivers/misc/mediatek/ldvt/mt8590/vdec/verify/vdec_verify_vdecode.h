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
#ifndef _VDEC_VERIFY_VDECODE_H_
#define _VDEC_VERIFY_VDECODE_H_

#include "vdec_verify_mpv_prov.h"
#include "../include/vdec_info_common.h"


void vMpvPlay(UINT32 u4InstID);
void vVerifyVDecIsrInit(UINT32 u4InstID);
void vVerifyVDecIsrStop(UINT32 u4InstID);
extern void vDrmaBusySet(UINT32  u4InstID);


#endif

