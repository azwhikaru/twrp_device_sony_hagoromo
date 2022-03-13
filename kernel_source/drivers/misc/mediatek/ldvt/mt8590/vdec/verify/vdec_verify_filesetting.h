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
#ifndef _VDEC_VERIFY_FILESETTING_H_
#define _VDEC_VERIFY_FILESETTING_H_

#include "vdec_info_verify.h"

void vWrite2PC(UINT32 u4InstID, UCHAR bType, UCHAR *pbAddr);
BOOL fgVdecReadFileName(UINT32 u4InstID, VDEC_INFO_VERIFY_FILE_INFO_T *ptFileInfo, VDEC_INFO_VERIFY_FILE_INFO_T *ptFileRecInfo, UINT32 *u4StartComp, UINT32 *u4EndComp , UINT32 *DumpPic);
void vAddStartCode2Dram(UCHAR *pbDramAddr);
void vH264WrData2PC(UINT32 u4InstID, UCHAR *ptAddr, UINT32 u4Size);
void vWMVWrData2PC(UINT32 u4InstID, UCHAR *ptAddr, UINT32 u4Size);
void vMPEGWrData2PC(UINT32 u4InstID, UCHAR *ptAddr, UINT32 u4Size);
void vFilledFBuf(UINT32 u4InstID, UCHAR *ptAddr, UINT32 u4Size);

void vDvWrData2PC(UINT32 u4InstID, UCHAR *ptAddr);

void vGenerateDownScalerGolden(UINT32 u4InstID, UINT32 DecYAddr,UINT32 DecCAddr, UINT32 u4Size);
void vConcateStr(char *strTarFileName, char *strSrcFileName, char *strAddFileName, UINT32 u4Idx);

void vH265Dump_ESA_NBM_performane_log(UINT32 u4VDecID, char* pBName, BOOL bIsUFOmode);
void vH265DumpInfo(UINT32 u4VDecID);
void vH265DumpYUV( UINT32 u4InstID, UINT32  PP_OUT_Y_ADDR , UINT32  PP_OUT_C_ADDR, UINT32 PIC_SIZE_Y );
int  vH265GoldenComparison( UINT32 u4InstID, int frame_num, unsigned int PIC_SIZE_Y, UINT32 Ptr_output_Y, UINT32 Ptr_output_C , UINT32 MV_COL_PIC_SIZE, bool isDump,
                                               UINT32 Dpb_ufo_Y_len_addr, UINT32 Dpb_ufo_C_len_addr, UINT32 UFO_LEN_SIZE_Y, UINT32 UFO_LEN_SIZE_C );

void vH265DumpYCbCr( UINT32 u4InstID, UINT32  PP_OUT_Y_ADDR , UINT32  PP_OUT_C_ADDR, UINT32 PIC_SIZE_Y);
void vH265DumpVFIFO( UINT32 u4InstID, UCHAR *vfifoaddr, UINT32  vfifolen, UINT32 flag);
void vH265RingFIFO_read( UINT32 u4InstID, BOOL bLoadBitstream );
BOOL vH264_CheckCRCResult(UINT32 u4InstID);
#ifdef LETTERBOX_SUPPORT
void vLBDParaParsing(UINT32 u4InstID);
void vCheckLBDResult(UINT32 u4InstID);
#endif

BOOL vH265_CheckCRCResult(UINT32 u4InstID);

#endif

