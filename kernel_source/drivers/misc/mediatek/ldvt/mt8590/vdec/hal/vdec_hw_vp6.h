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
#ifndef _VDEC_HW_VP6_H_
#define _VDEC_HW_VP6_H_
//#include "typedef.h"
//#include "vdec_info_common.h"



// *********************************************************************
//  Video Decoder HW Functions
// *********************************************************************
void vVDecWriteVP6VLD(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val, UINT32 u4BSID);
UINT32 u4VDecReadVP6VLD(UINT32 u4VDecID, UINT32 u4Addr);
void vVDecWriteVP6MC(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val);
UINT32 u4VDecReadVP6MC(UINT32 u4VDecID, UINT32 u4Addr);
UINT32 u4VDecVP6VLDGetBits(UINT32 u4BSID,UINT32 u4VDecID,UINT32 dShiftBit);
UINT32 u4VDecWaitVP6GetBitsReady(UINT32 u4VDecID, UINT32 u4BSID);
UINT32 u4VDecReadVP6VldRPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 *pu4Bits, UINT32 u4VFIFOSa);
void vVDecWriteVP6PP(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val, UINT32 u4BSID);
UINT32 u4VDecReadVP6PP(UINT32 u4VDecID, UINT32 u4Addr);
UINT32 u4VDecVP6BOOLGetBits(UINT32 u4BSID,UINT32 u4VDecID,UINT32 dShiftBit);
void vVDecVP6WriteVLD2(UINT32 u4Addr, UINT32 u4Val);    //MULTI-STREAM PANDA
UINT32 u4VDecVP6ReadVLD2(UINT32 u4Addr);    //MULTI-STREAM PANDA
void vVDecVP6SetVLD2VFIFO(UINT32 u4VFifoSa, UINT32 u4VFifoEa);  //MULTI-STREAM PANDA
BOOL fgVDecVP6WaitVld2FetchOk(void);    //MULTI-STREAM PANDA
void vVDecVP6SetVLD2Wptr(UINT32 u4WPtr);    //MULTI-STREAM PANDA
UINT32 u4VDecVP6VLD2GetBits(UINT32 dShiftBit);  //MULTI-STREAM PANDA
BOOL fgVDecWaitVld2FetchOk(UINT32 u4VDecID);    //MULTI-STREAM PANDA
void vVDecVP6WriteVLD2Shift(UINT32 u4Addr, UINT32 u4Val);    //MULTI-STREAM PANDA
UINT32 u4VDecVP6ReadVLD2Shift(UINT32 u4Addr);    //MULTI-STREAM PANDA
UINT32 u4VDecReadVP6DCAC(UINT32 u4VDecID, UINT32 u4Addr);
#endif

