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
#ifndef _VDEC_TYPE_H_
#define _VDEC_TYPE_H_

//#include "drv_config.h"
//#include "chip_ver.h"
// *********************************************************************
// MPV Constant define
// *********************************************************************
#define DRAMA_NONCACH_BASE_ADDRESS          0xC0000000L

#define VDEC_REMOVE_UNUSED_FUNC 0

#if (!CONFIG_DRV_LINUX_DATA_CONSISTENCY)
#define u4AbsDramANc(u4Ptr)    (UINT32)PHYSICAL(u4Ptr) //(((UINT32)(u4Ptr)) & (~DRAMA_NONCACH_BASE_ADDRESS))
#else
#define u4AbsDramANc(u4Ptr)    ((UINT32)(u4Ptr))
#endif

//MPEG Version
#define MPEG1       1
#define MPEG2       2

//Header Type
//#define SEQ_HDR           1
//#define GOP_HDR           2
//#define SEQ_END           3

// for _ptPicIdx->dwVtype in MPEG1/2 and _u4PicCdTp in MPEG1/2/4
#define I_TYPE            1
#define P_TYPE            2
#define B_TYPE            3
#define D_TYPE            4
// MPEG4 new picture Coding Type
// for _ptPicIdx->dwVtype in MPEG4
#define VIS_OBJ           0x8b  // visual_object_start_code,      000001B5
#define VID_OBJ_LAY       0x85  // video_object_layer_start_code, 000001[20-2f]
//#define VID_OBJ           0x84  // video_object_start_code,       000001[00~1f]
#define GOVOP             0x89  // group_of_vop_start_code,       000001B3
#define I_VOP             0x80  // vop_start_code                 000001B6
#define P_VOP             0x81  // vop_start_code                 000001B6
#define B_VOP             0x82  // vop_start_code                 000001B6
#define S_VOP             0x83  // vop_start_code                 000001B6
#define SH_I_VOP          0x98  // short_video_start_marker
#define SH_P_VOP          0x99  // short_video_start_marker
#define DX3_I_FRM         0xf0  // generated by firmware
#define DX3_P_FRM         0xf1  // generated by firmware
// WMV picture type define
#define    IVOP                0xa0
#define    PVOP                0xa1
#define    BVOP                0xa2
#define    BIVOP               0xa3
#define    SKIPFRAME        0xa4


// Picture Structure
#define TOP_FLD_PIC       1
#define BTM_FLD_PIC       2
#define FRM_PIC           3
// The follow 2 define use in Reference Field Picture
#define TWO_FLDPIC_TOPFIRST  4
#define TWO_FLDPIC_BTMFIRST  5
#define ERR_PIC_STRUCT       0xFF
#define PIC_STRUCT_MASK       0xF

//Flags for pic
#define PIC_FLAG_FORCE_DEC        (0x1 << 0)
#define PIC_FLAG_REAL_DEC        (0x1 << 1)
#define PIC_FLAG_REPEAT_1st_FLD        (0x1 << 2)
#define PIC_FLAG_BROKEN_LINK        (0x1 << 3)
#define PIC_FLAG_WITHOUT_NEXT_STARTCODE        (0x1 << 4)

//Flags for decoding
#define DEC_FLG_2ND_FLD_PIC        (0x1 << 0)
#define DEC_DROP_2ND_FLD_PIC      (0x1 << 1)
#define DEC_NEED_2ND_FLD_PIC      (0x1 << 2)

//Status for decoding
#define VDEC_CHK_BEF_VPARSE        ((unsigned int)0x1 << 0)
#define VDEC_WAIT_NEXT_I        ((unsigned int)0x1 << 1)
#define VDEC_PB_COND_PAUSE_MODE        ((unsigned int)0x1 << 2)
#define VDEC_DECODING        ((unsigned int)0x1 << 3)
#define VDEC_INQ_MODE_COMPLETE     ((unsigned int)0x1 << 4)
#define VDEC_CHK_WAIT_2ND_VPARSE        ((unsigned int)0x1 << 5)
#define VDEC_FAC_MODE_COMPLETE      ((unsigned int)0x1<<6)
#define VDEC_WAIT_PIC_SIZE        ((unsigned int)0x1 << 7)
#define VDEC_DRIP_FRM_PRS_MODE        ((unsigned int)0x1 << 8)
#define VDEC_DRIP_FRM_DEC_MODE        ((unsigned int)0x1 << 9)
#define VDEC_NO_DISP_PIC_BEF_I        ((unsigned int)0x1 << 10)
#define VDEC_WAIT_FB        ((unsigned int)0x1 << 11)
#define VDEC_NIPB_2_IPB        ((unsigned int)0x1 << 12)
#define VDEC_FORCE_FLUSH_DPB        ((unsigned int)0x1 << 13)
#define VDEC_DEC_DISP_ONE_FRAME     ((unsigned int)0x1 << 14)
#define VDEC_DEC_START_BY_INV_PTS     ((unsigned int)0x1 << 15)
#define VDEC_DEC_WITH_AVCHD_XVYCC     ((unsigned int)0x1 << 16)
#define VDEC_DEC_DRIP_PIC                            ((unsigned int)0x1 << 17)
#define VDEC_INTERLACED_FRM                       ((unsigned int)0x1 << 18)
#define VDEC_NO_DEC_NON_REF                      ((unsigned int)0x1 << 19)
#define VDEC_DISP_LAST_PIC                          ((unsigned int)0x1 << 20)
#define VDEC_UPD_VFIFO_ONLY                       ((unsigned int)0x1 << 21)
#define VDEC_WITH_SEQ_END                          ((unsigned int)0x1 << 22)
#define VDEC_PROGRESSIVE_SEQ                     ((unsigned int)0x1 << 23)
#define VDEC_HANDLING_DEC_RES                  ((unsigned int)0x1 << 24)
#define VDEC_DVDAUDIO_PIC                           ((unsigned int)0x1 << 25)
#define VDEC_DEC_ERR_DROPPED                    ((unsigned int)0x1 << 26)
#define VDEC_DEC_EOS                                    ((unsigned int)0x1 << 27)
#define VDEC_MVC_L_ALIGN_BASE                   ((unsigned int)0x1 << 28)  
#define VDEC_USE_PTS_REPLACE_FRAMERATE   ((unsigned int)0x1 << 29)  ///< check if USE PTS to replace frame rate for BD
#define VDEC_DEC_WITH_HDV_XVCOLOR         ((unsigned int)0x1 << 30)
#define VDEC_DEC_INQUIRY_24P                    ((unsigned int)0x1 << 31)

//MVC Status for decoding
#define VDEC_MVC_DROP_PIC        ((unsigned int)0x1 << 0)
#define VDEC_MVC_VIRTUAL_DEC        ((unsigned int)0x1 << 1)

// VC1 PP SCALE TYPE
#define PP_NO_SCALE             (0x0)
#define PP_SCALE_DOWN           (0x1)
#define PP_SCALE_UP             (0x1 << 1)


//Block Mode
#define MC_BLK_TB        0
#define MC_BLK_SWAP   1 
#define MC_BLK_NORM   2


#define OFF 0
#define ON  1


// Referenc Buf / B Buf / Digest Buf / Post Processing Buf Index
#define MC_REF_BUF_1       0
#define MC_REF_BUF_2       1
#define MC_DIG_BUF          2
#define MC_B_BUF            3
#define MC_Prg_B_BUF_1        4
#define MC_Prg_B_BUF_2        5


// Picture Field control for MC
#define MC_TOP_FLD          0
#define MC_BOTTOM_FLD       1

#define MC_2ND_FLD_OFF      0
#define MC_2ND_FLD_ON       1


#define MPV_ES_ID_UNKNOWN		0xFF
#define MPV_CHANNEL_ID_UNKNOWN    0xFF

#define MPV_ESMQ_0  0
#define MPV_ESMQ_1  1

#define MPV_ESMQ_0_USED   1
#define MPV_ESMQ_1_USED   2
#define MPV_ESMQ_CLEAR   0xFF


#define MPV_TYPE_SD   0
#define MPV_TYPE_HD   1
#define MPV_TYPE_720HD   720

#ifdef DRV_VDEC_VDP_RACING
   #define MAX_MAIN_DI_FB_NUM 2
   #define MAX_SUB_DI_FB_NUM 2
#else
   #define MAX_MAIN_DI_FB_NUM 4
   #define MAX_SUB_DI_FB_NUM 3
#endif
#define MAX_MVC_FB_NUM 3

// Read address must 128 bit alignment
#define MPV_READ_ADDR_ALIGN		(128 / 8)

// Quantization Table Size, 64 = 16 * 4 (Byte)
#define MPV_MATRIX_SIZE			16
#define MPV_MATRIX_RAW_SIZE		((MPV_MATRIX_SIZE * 8) + MPV_READ_ADDR_ALIGN)


//Block Mode Alignment
#define MPV_BLK_ALIGMENT               0x3F
#define MPV_SWAP_BLK_ALIGMENT    0x7F

#define VDEC_INV_8             0xFF
#define VDEC_INV_16             0xFFFF
#define VDEC_INV_32             0xFFFFFFFF
#define VDEC_INV_64             0xFFFFFFFFFFFFFFFFLL

#define STC_DELTA_MSB				(0x80000000)

#define MPV_FORCE_SD  1

#define VDEC_FBG_ID_UNKNOWN		0xFF
#define VDEC_FB_ID_UNKNOWN		0xFF
#define FBM_MS_VDEC                  0

//For Closed Caption
#define CC_LINE21_INDICATOR		0x4343
#define CC_ATSC_IDENTIFIER  0x47413934
#define CC_AFD_IDENTIFIER    0x44544731
#define VC1_USER_DATA_IDENTIFIER 0x48444D56
#define EIA_CC  0x03
#define MAX_CC_DATA  0x03
//#define FAILED(x)  (x < 0)

// For Line23
#define LINE23_INDICATOR		0x53455346

// For Offset Metadata
#define OFFSET_METADATA_INDICATOR		0x4F464D44

// For XVYcc
#define AVCHD_XVYCC_IDENTIFIER    0x434C4944

#define UUID_ISO_IEC_11578_0    0xA74602BB
#define UUID_ISO_IEC_11578_1    0xF8A14CC0
#define UUID_ISO_IEC_11578_2    0xA93648E3
#define UUID_ISO_IEC_11578_3    0x91DCE761

#define UUID_ISO_IEC_11578_0_BD    0x17EE8C60
#define UUID_ISO_IEC_11578_1_BD    0xF84D11D9
#define UUID_ISO_IEC_11578_2_BD    0x8CD60800
#define UUID_ISO_IEC_11578_3_BD    0x200C9A66


#define MAX_EDPB_NUM 30

#define vSetPicFlag(arg1, arg2)    arg1 |= (arg2)
#define vClrPicFlagAll(arg1)     arg1 = 0
#define vClrPicFlag(arg1, arg2)     arg1 &= (~(arg2))
#define fgIsPicFlag(arg1, arg2)      ((arg1 & (arg2)) > 0)

#define vSetDecStatus(arg1, arg2)    arg1 |= (arg2)
#define vClrDecStatus(arg1, arg2)     arg1 &= (~(arg2))
#define fgIsDecStatus(arg1, arg2)      ((arg1 & (arg2)) > 0)

#define vSetDecMVCStatus(arg1, arg2)    arg1 |= (arg2)
#define vClrDecMVCStatus(arg1, arg2)     arg1 &= (~(arg2))
#define fgIsDecMVCStatus(arg1, arg2)      ((arg1 & (arg2)) > 0)

#define VDEC_ALIGN_MASK(value, mask)			((value + mask) & (~mask))

#define VDEC_GET_FBG_ID(prVDecEsInfo)  (prVDecEsInfo->rVDecFbmInfo.ucFbgId)

#define fgIsMpeg1(prVDecEsInfo)    (prVDecEsInfo->rVDecNormInfo.eVdecCodecType == VDEC_MPEG1)
#define fgIsMpeg2(prVDecEsInfo)    (prVDecEsInfo->rVDecNormInfo.eVdecCodecType == VDEC_MPEG2)

#define fgIsMPEG2IPic(u4PicCdTp)         ((u4PicCdTp & 0xff) == I_TYPE) 

#define fgIsMPEG2RefPic(u4PicCdTp)         (((u4PicCdTp & 0xff) == I_TYPE) || \
                                                                    ((u4PicCdTp & 0xff) == P_TYPE))

#define fgIsWMVRefPic(u4PicCdTp)         (((u4PicCdTp & 0xff) == IVOP) || \
                                                                    ((u4PicCdTp & 0xff) == PVOP) || \
                                                                    ((u4PicCdTp & 0xff) == SKIPFRAME))                                                                    

#define fgIsVDecFrmPic(ucPicStruct)      (ucPicStruct == FRM_PIC)

#define fgIsVDecFldPic(ucPicStruct)       (ucPicStruct != FRM_PIC)                        

#define vSetVDecFlag(u4DecFlag,arg)      (u4DecFlag |= (arg))

#define vClrVDecFlag(u4DecFlag,arg)      (u4DecFlag &= (~(arg)))

#define vToggleVDecFlag(u4DecFlag,arg)   (u4DecFlag ^= (arg))
#define fgIsVDecFlagSet(u4DecFlag,arg)   ((u4DecFlag & (arg)) > 0)

#define fgIsCCOutput(prVDecEsInfo)    ((prVDecEsInfo->prVDecPBInfo->e_speed_type >= VID_DEC_SPEED_TYPE_PAUSE) \
                                                              && (prVDecEsInfo->prVDecPBInfo->e_speed_type <= VID_DEC_SPEED_TYPE_NORMAL))

#define VDEC_ASSERT(arg) ASSERT(arg)

#define VDEC_FORCEWHILE(arg) while(arg);

#define fgIsRSFwdDec(arg) FALSE

#define fgIsRSRvsDec(arg) FALSE

#define fgIsVDecNorm(arg) ((arg >= VID_DEC_SPEED_TYPE_PAUSE) && (arg <= VID_DEC_SPEED_TYPE_NORMAL))

#if 0//def NONCACHE
#define MpvAddrWithNonCacheOffset(arg) CACHE(arg)
#define MpvAddrWithoutNonCacheOffset(arg) NONCACHE(arg)
#else
#define MpvAddrWithNonCacheOffset(arg) (arg)//((arg & 0x0FFFFFFF) + 0xC0000000)
#define MpvAddrWithoutNonCacheOffset(arg) (arg)//(arg & 0x0FFFFFFF)
#endif


#endif /* _MPV_TYPE_H_ */
