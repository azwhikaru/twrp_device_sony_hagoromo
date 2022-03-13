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
#ifndef _VDEC_HAL_IF_VP9_H_
#define _VDEC_HAL_IF_VP9_H_

#include "../include/vdec_info_vp9.h"
#include "../include/vdec_info_common.h"
#include "vdec_hw_vp9.h"
//	#include "vdec_hw_common.h"

/*! \name Video Decoder HAL VP9 Interface
* @{
*/
void vVDEC_HAL_VP9_SW_Reset(UINT32 u4CoreId);
void vVDEC_HAL_VP9_Mcore_Init(UINT32 u4LaeBufAddr, UINT32 u4ErrBufAddr);
void vVDEC_HAL_VP9_Mcore_Enable(BOOL fgEnable);
//INT32 i4VDEC_HAL_VP9_InitBarrelShifter(UINT32 u4CoreId, UINT32 u4VldRp, UINT32 u4VldRpEnd, UINT32 u4FifoStart, UINT32 u4FifoEnd);
INT32 i4VDEC_HAL_VP9_InitBarrelShifter(UINT32 u4CoreId, ULONG u4VldRp, ULONG u4VldRpEnd, ULONG u4FifoStart, ULONG u4FifoEnd);
void vVDEC_HAL_VP9_InitBool(UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_Read_Literal(UINT32 u4CoreId, UINT32 u4ShiftBit);
UINT32 u4VDEC_HAL_VP9_Read_Bit(UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_Read_Literal_Raw(UINT32 u4CoreId, UINT32 u4ShiftBit);
INT32 i4VDEC_HAL_VP9_Read_Signed_Literal_Raw(UINT32 u4CoreId, UINT32 u4ShiftBit);
UINT32 u4VDEC_HAL_VP9_Get_Vld_Read_Pointer(UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_Get_Bits_Count(UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_Reset_Bits_Count(UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_Read_Bit_Raw(UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_Get_Input_Window(UINT32 u4CoreId);
UINT32 VP9_Wait_LAE_Decode_Done(unsigned long  start_time, UINT32 u4CoreID);
void vVDEC_HAL_VP9_Set_UnCompressed(UINT32 u4CoreId, VP9_UNCOMPRESSED_HEADER_T* prUnCompressed);
void vVDEC_HAL_VP9_Set_Segmentation(UINT32 u4CoreId, SEGMENTATION* pSeg, UINT32 u4SegAddr);
void vVDEC_HAL_VP9_Clear_Counts_Table(UINT32 u4CoreId);
void vVDEC_HAL_VP9_Set_TxMode(UINT32 u4CoreId, UINT32 u4TxMode);
void vVDEC_HAL_VP9_Set_Tile_Info(UINT32 u4CoreId, VP9_COMMON_T* prCommon);
void vVDEC_HAL_VP9_Get_Counts_Table(UINT32 u4CoreId, ULONG u4WorkingBuf, FRAME_COUNTS* pCounts);
void vVDEC_HAL_VP9_Get_Probs_Table(UINT32 u4CoreId, ULONG u4WorkingBuf, FRAME_CONTEXT* prFrmCtx);
void vVDEC_HAL_VP9_Set_Probs_Table(UINT32 u4CoreId, FRAME_CONTEXT *rFrmCtx);
void vVDEC_HAL_VP9_UPDATE_TX_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_COEF_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_MBSKIP_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_INTER_MODE_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_SWITCHABLE_INTERP_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_INTRA_INTER_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_SINGLE_REF_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_Y_MODE_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_PARTITION_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_MVD_INT_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_MVD_FP_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_MVD_HP_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_COMP_REF_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_UPDATE_COMP_INTER_PROBS(UINT32 u4CoreId);
void vVDEC_HAL_VP9_VDec_DumpReg(UINT32 u4FrameNum, UINT32 u4DualCore,BOOL bDecodeDone);
UINT32 u4VDEC_HAL_VP9_VDec_ReadFinishFlag(UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_MCORE_VDec_ReadFinishFlag(UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_VDec_ReadCRC_Y0(UINT32 u4Offset, UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_VDec_ReadCRC_Y1(UINT32 u4Offset, UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_VDec_ReadCRC_C0(UINT32 u4Offset, UINT32 u4CoreId);
UINT32 u4VDEC_HAL_VP9_VDec_ReadCRC_C1(UINT32 u4Offset, UINT32 u4CoreId);
void vVDEC_HAL_VP9_Set_Compound_Ref(UINT32 u4CoreId, UINT32 u4CompoundParam);
void vVDEC_HAL_VP9_Set_MVP_Enable(UINT32 u4CoreId, BOOL fgMVP_Enable);
void vVDEC_HAL_VP9_Set_MV_Buffer_Addr(UINT32 u4CoreId, UINT32 u4MVBufVAddr);
void vVDEC_HAL_VP9_Count_TBL_WDMA(UINT32 u4CoreID, ULONG u4VP9_COUNT_TBL_Addr);
void vVDEC_HAL_VP9_Prob_TBL_WDMA(UINT32 u4CoreID, ULONG u4VP9_PROB_TBL_Addr);
void vVDEC_HAL_VP9_Prob_TBL_Read_SRAM(UINT32 u4CoreID, ULONG u4VP9_PROB_TBL_Addr);
void vVDEC_HAL_VP9_Count_TBL_Read_SRAM(UINT32 u4CoreID, ULONG u4VP9_PROB_TBL_Addr);

BOOL vVerVP9WaitDecEnd(VP9_COMMON_T* prCommon, UINT32 u4InstID);

void vVDEC_HAL_VP9_Set_MC_DecodeBuf_Addr(UINT32 u4CoreId, UINT32 u4YAddr, UINT32 u4CAddr);
void vVDEC_HAL_VP9_Set_MC_Set_UMV(UINT32 u4CoreId, UINT32 u4UMV_Width, UINT32 u4UMV_Height, UINT32 u4FrameIdx);
void vVDEC_HAL_VP9_Set_MC_Ref_Scaling_Step(UINT32 u4CoreId, INT32 i4X_Step, INT32 i4Y_Step, UINT32 u4FrameIdx);
void vVDEC_HAL_VP9_Set_MC_Set_Scaling_Factor(UINT32 u4CoreId, INT32 i4ScalingFactor_X, INT32 i4ScalingFactor_Y, INT32 u4FrameIdx);
void vVDEC_HAL_VP9_Set_MC_MAX_RRF_BLK_Size(UINT32 u4CoreId, INT32* pMaxRefBlk_X, INT32* pMaxRefBlk_Y );
void vVDEC_HAL_VP9_Set_MC_Ref_Pitch(UINT32 u4CoreId, UINT32 u4UMV_Width, UINT32 u4FrameIdx);
void vVDEC_HAL_VP9_Set_MC_MI_COLS_ROWS(UINT32 u4CoreId, UINT32 u4MI_rows, UINT32 u4MI_cols);
void vVDEC_HAL_VP9_Set_MC_RefBuf_Addr(UINT32 u4CoreId, VP9_REF_BUF_T rLastFrame, VP9_REF_BUF_T rGoldenFrame, 
                                      VP9_REF_BUF_T rARF_Frame);
void vVDEC_HAL_VP9_Set_MC_Ref_Scaling_Enable(UINT32 u4CoreId, UINT32 u4Frame0_Scaling_EN, UINT32 u4Frame1_Scaling_EN, 
                                             UINT32 u4Frame2_Scaling_EN);
void vVDEC_HAL_VP9_Set_SQT_IQ_SRAM_EN(UINT32 u4CoreId, UINT32 u4Enable);
void vVDEC_HAL_VP9_Set_SQT_Q_Table(UINT32 u4CoreId, UINT32 au4Dequant[MAX_SEGMENTS][4]);

void vVDEC_HAL_VP9_Set_PP_DBK_EN(UINT32 u4CoreId, UINT32 u4Enable);
void vVDEC_HAL_VP9_Set_PP_MB_Width(UINT32 u4CoreId, UINT32 u4Width);
void vVDEC_HAL_VP9_Set_PP_WriteByPost(UINT32 u4CoreId, UINT32 u4Enable);
void vVDEC_HAL_VP9_Set_PP_MB_LeftRightMostIdx(UINT32 u4CoreId, UINT32 u4Width);
void vVDEC_HAL_VP9_Set_PP_MB_UpDownMostIdx(UINT32 u4CoreId, UINT32 u4Height);
void vVDEC_HAL_VP9_Set_PP_Display_Range(UINT32 u4CoreId, UINT32 u4Width, UINT32 u4Height);
void vVDEC_HAL_VP9_Set_PP_EN(UINT32 u4CoreId, UINT32 u4Enable);

//void vVDEC_HAL_VP9_UFO_Config(UINT32 u4CoreId, UINT32 u4PIC_SIZE_Y, UINT32 u4PIC_SIZE_C, UINT32 u4Y_LEN_Addr, UINT32 u4C_LEN_Addr);
void vVDEC_HAL_VP9_UFO_Config(UINT32 u4CoreId,UINT32 u4Width,  UINT32 u4Height, UINT32 u4PIC_SIZE_Y, UINT32 u4PIC_SIZE_C,
                              UINT32 u4Y_LEN_Addr, UINT32 u4C_LEN_Addr);


//
/*! @} */


#endif //#ifndef _HAL_VDEC_VP9_IF_H_

