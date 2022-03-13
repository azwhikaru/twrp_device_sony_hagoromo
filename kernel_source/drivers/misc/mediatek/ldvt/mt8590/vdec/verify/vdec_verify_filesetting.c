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
#define _FILE_SETTING_C_

#include <mach/mt_typedefs.h>
#include <linux/file.h> 
#include <linux/fs.h>
#include <linux/mm.h> 
#include <linux/slab.h> 
#include <linux/vmalloc.h>
#include <asm/uaccess.h> 
#include <linux/kernel.h> 
//#include "common.h"
//#define DBG_PRINT
//#include "debug.h"
#include "vdec_verify_mpv_prov.h"

//#include "vdec_verify_vdecode.h"

#include "../hal/vdec_hal_if_h264.h"
#include "../hal/vdec_hal_if_h265.h"
#include "../hal/vdec_hal_if_wmv.h"
#include "../hal/vdec_hal_if_mpeg.h"
#include "../hal/vdec_hw_common.h"
#include "../hal/vdec_hal_if_common.h"
//#include "vdec_info_dv.h"
//#include "RvdUtil.h"
//#include "vdec_ide.h"
#include "../include/vdec_type.h"
//#include "x_printf.h"
//#include "x_debug.h"

#include "vdec_verify_file_common.h"
#include "vdec_verify_filesetting.h"
#include "vdec_verify_common.h"

#if (!CONFIG_DRV_LINUX)
#include <stdio.h>
#include <string.h>
#endif
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>


/// Compare decode checksum with golden, only for verification
/// \return True for match, false for mismatch
//extern BOOL fgVDEC_HAL_DV_VDec_Verify_CompCheckSum(
//  UINT32 *pu4DecCheckSum,
//  UINT32 *pu4GoldenCheckSum
//);

extern char gpfH264LogFileBuffer[4096];
extern int gfpH264log;
extern unsigned int gH264logbufferOffset;
int vdecwriteFile(int fp,char *buf,int writelen);
int vH265DumpMem( UINT32 u4InstID, unsigned char* buf, unsigned int size , int  frame_num , unsigned int type , bool isTimeout);

#define DBG_H264_PRINTF
/*
#define DBG_H264_PRINTF(format,...)  \
    do { \
        if (-1 != gfpH264log) {\
            { gH264logbufferOffset += sprintf((char *)(gpfH264LogFileBuffer+gH264logbufferOffset),format, ##__VA_ARGS__);} \
            if (gH264logbufferOffset >= 3840 ) { \
                vdecwriteFile(gfpH264log, gpfH264LogFileBuffer, gH264logbufferOffset); \
                gH264logbufferOffset = 0; \
            } \
        } \
    } while (0)
*/



#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8520) 
void vdscl (UINT32 u4InstID, int srcWidth, int srcHeight, int targetWidth, int targetHeight, 
        UCHAR *srcData, UCHAR *targetData, int component, int pic_stru,
        int tgbufLen, int h_ofst, int v_ofst,
        int src_agent, int spec_type, int mbaff, int fg_src);
#else
void vdscl (UINT32 u4InstID, int srcWidth, int srcHeight, int targetWidth, int targetHeight, 
        UCHAR *srcData, UCHAR *targetData, int component, int pic_stru,
        int tgbufLen, int h_ofst, int v_ofst,
        int src_agent, int spec_type, int mbaff, int fg_src,
		int src_h_ofst, int src_v_ofst, int scl_h_ofst, int scl_v_ofst, int lk_en, int luma_key);
#endif

extern UINT32			_dwDvFormat;
extern UINT32			_dwDvFrmSize;
extern UINT32			_dwDvCbCrFormat;

#if 1//ndef SATA_HDD_FS_SUPPORT
#if (VDEC_PP_ENABLE)
#if 1//#ifdef MPEG4_FORCE_DUMP_OUTPUT
CHAR _bFileAddStrY[1][10] = {"_Y.bok\0"};
CHAR _bFileAddStrC[1][10] = {"_CbCr.bok\0"};
#else
CHAR _bFileAddStrY[1][10] = {"_Y.out\0"};
CHAR _bFileAddStrC[1][10] = {"_CbCr.out\0"};
#endif
#else
#ifdef MPEG4_FORCE_DUMP_OUTPUT
CHAR _bFileAddStrY[1][10] = {"_Y.bok\0"};
CHAR _bFileAddStrC[1][10] = {"_CbCr.bok\0"};
#else
CHAR _bFileAddStrY[1][10] = {"_Y.out\0"};
CHAR _bFileAddStrC[1][10] = {"_CbCr.out\0"};
#endif
//CHAR _bFileAddStrY[1][20] = {"_4dw.ramY.raw\0"};
//CHAR _bFileAddStrC[1][20] = {"_4dw.ramCbCr.raw\0"};
#endif
UINT32 u4CompareCnt = 0;
extern mm_segment_t oldfs;
CHAR _bFileAddStrY2[1][10] = {"_Y.out\0"};
CHAR _bFileAddStrC2[1][10] = {"_CbCr.out\0"};

extern void VDecDumpMpegRegister(UINT32 u4VDecID,BOOL fgTriggerAB);
extern void VDecDumpMP4RegisterByYiFeng(UINT32 u4VDecID);
extern void vPrintDumpReg(UINT32 u4InstID,UINT32 fgTAB);
#else
CHAR _bFileAddStrY[1][10] = {"Y.out\0"};
CHAR _bFileAddStrC[1][10] = {"C.out\0"};
#endif

void vConcateStr(char *strTarFileName, char *strSrcFileName, char *strAddFileName, UINT32 u4Idx);
BOOL fgCompMPEGChkSumGolden(UINT32 u4InstID);

// CRC/golden path generation
void vMPEG4GoldenGetFilePath(CHAR *ptBitstreamFilePath, CHAR *ptGoldenFilePath);
CHAR* str_tok(CHAR *ptStr, CHAR *ptDelim);
void vMPEG4CrcGetFilePath(CHAR *ptBitstreamFilePath, CHAR *ptCrcFilePath);
CHAR *ptSrc = 0;
CHAR cTokBuf[300];

void reset_dec_counter(UINT32 u4InstID);

#if 0
// *********************************************************************
// Function    : void vAddStr(char *strFileName)
// Description : Add string
// Parameter   : None
// Return      : None
// *********************************************************************
void vAddStr(char *strTarFileName, char *strSrcFileName, char *strAddFileName, UINT32 u4Idx)
{
    UINT32 u4Cnt;
    UINT32 u4CntAdd;
    UINT32 u4RecNum;
    UCHAR bNum[16];
    INT32 i;

    // Set file name string
    u4Cnt = 0;
    while(strSrcFileName[u4Cnt] != '\0')
    {
        strTarFileName[u4Cnt] = strSrcFileName[u4Cnt];
        u4Cnt ++;
    }

    // analysis file idx, support to 10^16-1 so far , 
    u4RecNum= u4Idx;  
    u4CntAdd= 0; // at least num "0"
    do
    {
        bNum[u4CntAdd] = u4RecNum % 10;
        u4CntAdd ++;    
        u4RecNum /= 10;    
    }while((u4RecNum > 0) && (u4CntAdd < 16));
    for(i=1; i<=u4CntAdd; i++)
    {
        strTarFileName[u4Cnt] = bNum[u4CntAdd - i] + 48;
        u4Cnt ++;    
    }    

    u4CntAdd = 0;    
    while(strAddFileName[u4CntAdd] != '\0')
    {
        strTarFileName[u4Cnt + u4CntAdd] = strAddFileName[u4CntAdd];
        u4CntAdd ++;
    }
    strTarFileName[u4Cnt + u4CntAdd] = '\0';
}
#endif


UINT32 u4CalculatePixelAddress_Y(
  UINT32 u4YBase,                           ///< [IN] frame buffer Y component address
  UINT32 u4XPos,                             ///< [IN] x position of pixel in frame buffer
  UINT32 u4YPos,                             ///< [IN] y position of pixel in frame buffer
  UINT32 u4FrameWidth,                 ///< [IN] Frame buffer width
  BOOL fgBlock,                               ///< [IN] MTK block / raster scan
  UCHAR bBlockW
)
{
    UINT32 u4YAdr;
    UINT32 u4X = u4XPos;
    UINT32 u4Y = u4YPos;
    UINT32 u4YBlockNum = 0;
    UINT32 u4YOffsetInBlock = 0;
    UINT32 u4XBlockNum = 0;
    UINT32 u4XOffsetInBlock = 0;

    if(bBlockW == 4)  // in MT8520, one block width is (16x32)
    {
        u4XOffsetInBlock = u4X & 0xF;
    }
    else if(bBlockW == 3)  // in MT8108 and MT1389S, one block width is (8x32)
    {
        u4XOffsetInBlock = u4X & 0x7;
    }
    else if(bBlockW == 2)  // in MT8105, one block width is (4x32)
    {
        u4XOffsetInBlock = u4X & 0x3;
    }

    // Y arrangement is the same in 420 and 422 YCbCr Mode.
    u4YBlockNum = u4Y >> 5;
    u4YOffsetInBlock = u4Y & 0x1F;
    u4XBlockNum = u4X >> bBlockW;
    if (fgBlock)
        u4YAdr = u4YBase + ((u4FrameWidth * u4YBlockNum) << 5) + ((u4YOffsetInBlock + (u4XBlockNum << 5) ) << bBlockW) + u4XOffsetInBlock;
    else
        u4YAdr =  u4YBase + (u4Y * u4FrameWidth) + u4X;

    return u4YAdr;
}

UINT32 u4CalculatePixelAddress_C(
  UINT32 u4CBase,                           ///< [IN] frame buffer CbCr component address
  UINT32 u4XPos,                             ///< [IN] x position of pixel in frame buffer
  UINT32 u4YPos,                             ///< [IN] y position of pixel in frame buffer
  UINT32 u4FrameWidth,                 ///< [IN] Frame buffer width
  BOOL fgBlock,                               ///< [IN] MTK block / raster scan
  BOOL fg420,                                   ///< [IN] 420 / 422
  UCHAR bBlockW
)
{
    UINT32 u4CAdr;
    UINT32 u4X = u4XPos;
    UINT32 u4Y = u4YPos;
    UINT32 u4YBlockNum = 0;
    UINT32 u4YOffsetInBlock = 0;
    UINT32 u4XBlockNum = 0;
    UINT32 u4XOffsetInBlock = 0;


    u4YBlockNum = u4Y >> 5;
    u4YOffsetInBlock = u4Y & 0x1F;
    u4XBlockNum = u4X >> bBlockW;

    // Map C (chrominance)
    u4X &= 0xFFFE;
    if(bBlockW == 4)
    {
        u4XOffsetInBlock = u4X & 0xF;
    }
    else if(bBlockW == 3)
    {
        u4XOffsetInBlock = u4X & 0x7;
    }
    else if(bBlockW == 2)
    {
        u4XOffsetInBlock = u4X & 0x3;
    }

    u4FrameWidth = ((u4FrameWidth + 1) & 0xFFFE);
    if (fg420)
    {
        u4Y = u4Y >> 1;
        u4YOffsetInBlock = u4Y & 0xF;
        if (fgBlock)
            u4CAdr = u4CBase + ((u4FrameWidth * u4YBlockNum) << 4) + ((u4YOffsetInBlock + (u4XBlockNum << 4) ) << bBlockW) + u4XOffsetInBlock;
        else
            u4CAdr = u4CBase + (u4Y * u4FrameWidth) + (u4X);
    }
    else
    {
        // This code should be emended.
        if (fgBlock)
            u4CAdr = u4CBase + ((u4FrameWidth * u4YBlockNum) << 5) + ((u4YOffsetInBlock + (u4XBlockNum << 4) ) << bBlockW) + u4XOffsetInBlock;
        else
            u4CAdr = u4CBase + (u4Y * u4FrameWidth) + (u4X);
    }
    return u4CAdr;
}

UINT32 u4InverseAddrSwap(UINT32 u4AddrSwapMode, UINT32 u4SwappedAddr, BOOL fgIsYComponent)
{
  unsigned int u4NonSwappedAddr, u4TempAddr;
  printk("u4AddrSwapMode is %d\n",u4AddrSwapMode);
  switch(u4AddrSwapMode)
  {
  case 0x1: //MT8520_SWAP_MODE_1
    u4NonSwappedAddr = ((u4SwappedAddr&0xFFFFFFC0) | ((u4SwappedAddr&0x20)>>5) | ((u4SwappedAddr&0x10)>>2) | ((u4SwappedAddr&0x8)>>2) | ((u4SwappedAddr&0x7)<<3));
    break;
  case 0x2: //MT8520_SWAP_MODE_2
    u4NonSwappedAddr = ((u4SwappedAddr&0xFFFFFFE0) | ((u4SwappedAddr&0x10)>>4) | ((u4SwappedAddr&0xF)<<1));
    break;
  case 0x4: // MT5351_SWAP_MODE_0
    if(fgIsYComponent)
    {
      u4TempAddr = ((u4SwappedAddr&0xFFFFFF80) | ((u4SwappedAddr&0x40)>>4) | ((u4SwappedAddr&0x3C)<<1) | (u4SwappedAddr&0x3));
      u4NonSwappedAddr = ((u4TempAddr&0xFFFFFF80) | ((u4TempAddr&0x7C)>>2) | ((u4TempAddr&0x3)<<5));
    }
    else
    {
      u4TempAddr = ((u4SwappedAddr&0xFFFFFFC0) | ((u4SwappedAddr&0x20)>>3) | ((u4SwappedAddr&0x1C)<<1) | (u4SwappedAddr&0x3));
      u4NonSwappedAddr = ((u4TempAddr&0xFFFFFFC0) | ((u4TempAddr&0x3C)>>2) | ((u4TempAddr&0x3)<<4));
    }
    break;
  case 0x5: // MT5351_SWAP_MODE_1
    if(fgIsYComponent)
    {
      u4TempAddr = ((u4SwappedAddr&0xFFFFFF00) | ((~u4SwappedAddr)&0x80) | (u4SwappedAddr&0x7F));
      u4NonSwappedAddr = ((u4TempAddr&0xFFFFFF80) | ((u4TempAddr&0x7C)>>2) | ((u4TempAddr&0x3)<<5));
    }
    else
    {
      u4TempAddr = ((u4SwappedAddr&0xFFFFFF80) | ((~u4SwappedAddr)&0x40) | (u4SwappedAddr&0x3F));
      u4NonSwappedAddr = ((u4TempAddr&0xFFFFFFC0) | ((u4TempAddr&0x3C)>>2) | ((u4TempAddr&0x3)<<4));
    }
    break;
  case 0x6: // MT5351_SWAP_MODE_2
    if(fgIsYComponent)
    {
      u4NonSwappedAddr = ((u4SwappedAddr&0xFFFFFF80) | ((u4SwappedAddr&0x7C)>>2) | ((u4SwappedAddr&0x3)<<5));
    }
    else
    {
      u4NonSwappedAddr = ((u4SwappedAddr&0xFFFFFFC0) | ((u4SwappedAddr&0x3C)>>2) | ((u4SwappedAddr&0x3)<<4));
    }
    break;
  #if VDEC_FIELD_COMPACT
  case 0xF: // MT8320_FIELD_COMPACT
    if(fgIsYComponent)
    {
      //u4NonSwappedAddr = ((u4SwappedAddr&0xFFFFFFE0) | ((u4SwappedAddr&0x1E)>>1) | ((u4SwappedAddr&0x1)<<4));
      u4NonSwappedAddr = ((u4SwappedAddr&0xFFFFFFE0) | ((u4SwappedAddr&0x0F)<<1) | ((u4SwappedAddr&0x10)>>4));
    }
    else
    {
      //u4NonSwappedAddr = ((u4SwappedAddr&0xFFFFFFF0) | ((u4SwappedAddr&0x0E)>>1) | ((u4SwappedAddr&0x1)<<3));
      u4NonSwappedAddr = ((u4SwappedAddr&0xFFFFFFF0) | ((u4SwappedAddr&0x07)<<1) | ((u4SwappedAddr&0x8)>>3));
    }
    break;
  #endif
  default:
    u4NonSwappedAddr = u4SwappedAddr;
    break;
  }
  return u4NonSwappedAddr;
}

 UINT8 auAddrSwapMapTable[9] =
 {
    4, 5, 6, 7, 0, 1, 2, 3, 15
  };

void vMPEG_InvAddressSwap(UINT32 u4InstID, 
                                                         BYTE* pbSrcBufY, BYTE* pbSrcBufC, 
                                                         BYTE* pbOutBufY, BYTE* pbOutBufC,
                                                         UINT32 u4AlignWidth, UINT32 u4AlignHeight, UINT32 u4AlignSize,
                                                         UINT32 u4HwSwapMode)
{
  UINT32 i;
  UINT32 u4DataLength;
  UINT32 u4AlignW_Luma;
  UINT32 u4AlignH_Luma;
  UINT32 u4AlignW_Chroma;
  UINT32 u4AlignH_Chroma;
  //UINT32 u4AlignSize = 0x32000;
  UINT32 u4NonSwappedAddr;  
  UINT32 u4SwappedAddr;
  BYTE * pbTempBufAddr;
  UINT32 u4AddrressSwapSize = 16;
  UINT32 u4AddrSwapMode;

  #ifdef RM_DDR3MODE_ENABLE
  u4AddrressSwapSize = 16;
  #else //RM_DDR3MODE_ENABLE
  u4AddrressSwapSize = 16;
  #endif //RM_DDR3MODE_ENABLE

  //prParsingPic = (VDEC_INFO_RM_PICINFO_T*) &_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecRMDecPrm.rRMParsPicInfo;

  u4AlignW_Luma = u4AlignWidth;//prParsingPic->u4Width;
  u4AlignH_Luma = u4AlignHeight;//prParsingPic->u4Height;
  
  //pbSrcBufY = (BYTE*) prParsingPic->u4OutBufY;
  //pbSrcBufC = (BYTE*) prParsingPic->u4OutBufC;
  //pbOutBufY = (BYTE*) _pucDumpYBuf[u4InstID];
  //pbOutBufC = (BYTE*) _pucDumpCBuf[u4InstID];

  u4AddrSwapMode = auAddrSwapMapTable[u4HwSwapMode];
  
  //Luma 
  u4DataLength = u4AlignW_Luma * u4AlignH_Luma;
  //u4DataLength = (u4DataLength + u4AlignSize-1)/u4AlignSize;
  //u4DataLength = u4DataLength * u4AlignSize;
  u4SwappedAddr = 0;
  
  for (i=0; i<u4DataLength; i+=u4AddrressSwapSize)
  {
    u4NonSwappedAddr = u4InverseAddrSwap(u4AddrSwapMode, u4SwappedAddr, TRUE);
    pbTempBufAddr = (BYTE*) (pbSrcBufY+i);
    memcpy(&pbOutBufY[u4NonSwappedAddr<<4], &pbTempBufAddr[0],u4AddrressSwapSize);
    u4SwappedAddr++;
  }
  

  //Chroma
  u4AlignW_Chroma = u4AlignW_Luma;
  u4AlignH_Chroma = u4AlignH_Luma / 2;
  
  u4DataLength = u4AlignW_Chroma * u4AlignH_Chroma;
  //u4DataLength = (u4DataLength + u4AlignSize-1)/u4AlignSize;
  //u4DataLength = u4DataLength * u4AlignSize;
  u4SwappedAddr = 0;

  for (i=0; i<u4DataLength; i+=u4AddrressSwapSize)
  {
    u4NonSwappedAddr = u4InverseAddrSwap(u4AddrSwapMode, u4SwappedAddr, FALSE);
    pbTempBufAddr = (BYTE*) (pbSrcBufC+i);
    memcpy(&pbOutBufC[u4NonSwappedAddr<<4], &pbTempBufAddr[0],u4AddrressSwapSize);
    u4SwappedAddr++;
  }
}


// *********************************************************************
// Function    : void vMPEG4GoldenGetFilePath(CHAR *ptBitstreamFilePath, CHAR *ptGoldenFilePath)
// Description : Generate golden path from original bitstream path
// Parameter   : None
// Return      : None
// *********************************************************************
void vMPEG4GoldenGetFilePath(CHAR *ptBitstreamFilePath, CHAR *ptGoldenFilePath)
{
    CHAR *ptTok;
    CHAR cLastTok[300];
    UINT32 u4GoldenFilePathLen = 0;
    UINT32 u4TokenLen = 0;
    UINT32 i = 0;
    INT32 dotCnt = 0;
    memset(cLastTok, 0, 300);
    memset(ptGoldenFilePath, 0, 300);

    ptTok = str_tok(ptBitstreamFilePath, "\\/");
    while (ptTok != NULL)
    {
        u4TokenLen = strlen(ptTok);
        if (strncmp("bitstream", ptTok, 9) == 0 && u4TokenLen == 9)
        {
            memcpy(ptGoldenFilePath + u4GoldenFilePathLen, "pattern", 7);            
            u4GoldenFilePathLen += 7;
            ptGoldenFilePath[u4GoldenFilePathLen] = '/'; //'\\';
            u4GoldenFilePathLen ++;
        }
        else
        {
            memcpy(ptGoldenFilePath + u4GoldenFilePathLen, ptTok, u4TokenLen);
            u4GoldenFilePathLen += u4TokenLen;
            ptGoldenFilePath[u4GoldenFilePathLen] = '/'; //'\\';
            u4GoldenFilePathLen ++;
        }
        memcpy(cLastTok, ptTok, u4TokenLen);
        cLastTok[u4TokenLen] = '\0';        
        ptTok = str_tok(NULL, "\\/");
    }

    u4GoldenFilePathLen -= 1;
    ptGoldenFilePath[u4GoldenFilePathLen] = '\0';

    for (i = u4TokenLen; i > 0; i--)
    {
        if (cLastTok[i] == '.') 
        {
            dotCnt ++;
        }
        #if H263_USE_SORENSON        
        if (dotCnt > 1) //Sorenson H.263 has 2 part file extension
        #else
        if (dotCnt > 0) // MPEG4/DIVX/H.263/DIVX311
        #endif
        {
            break;
        }
    }

    if (i > 0)
    {
        u4GoldenFilePathLen = u4GoldenFilePathLen - u4TokenLen + i;
    }
    ptGoldenFilePath[u4GoldenFilePathLen] = '/'; //'\\';
    u4GoldenFilePathLen ++;
    
    #if (VDEC_PP_ENABLE)
    sprintf(ptGoldenFilePath + u4GoldenFilePathLen, "%s", "dbk_\0");
    #elif VDEC_UFO_ENABLE
    sprintf(ptGoldenFilePath + u4GoldenFilePathLen, "%s", "ufo_\0");
	#else
    sprintf(ptGoldenFilePath + u4GoldenFilePathLen, "%s", "mc_out_divx2006_\0");
    #endif
    
    //memset(ptGoldenFilePath + u4GoldenFilePathLen, 0, 300 - u4GoldenFilePathLen);
}


// *********************************************************************
// Function    : void vMPEGWrData2PC(UINT32 u4InstID, BYTE *ptAddr, UINT32 u4Size)
// Description : Write the decoded data to PC for compare
// Parameter   : None
// Return      : None
// *********************************************************************
void vMPEGWrData2PC(UINT32 u4InstID, UCHAR *ptAddr, UINT32 u4Size)
{
    #if ((!defined(COMP_HW_CHKSUM)) || defined(DOWN_SCALE_SUPPORT))
    UINT32 u4Cnt;
    #ifdef GOLDEN_128BIT_COMP  
    UINT32 u4XPix,u4YPix;
    #endif
    #if defined(DOWN_SCALE_SUPPORT)
    UINT32 u4DramPicSize = 0x1FE000;
    #endif //DOWN_SCALE_SUPPORT

    UINT32 u4Width,u4Height;
    ULONG u4YBase,u4CBase;
    //UINT32 u4BufferWidth;
    UCHAR *pbDecBuf,*pbGoldenBuf;

    #ifndef GOLDEN_128BIT_COMP  
    UINT32 u4Pix;
    #else
    UINT32 u4Ty0, u4Tx0, u4Ty1, u4Tx1;
    UINT32 u4X, u4Y;
    UINT32 mbw, mbh, i, j;
    UINT32 u4Start;  
    #endif  

 
    #if VDEC_DDR3_SUPPORT
    UINT32 u4TxDDR3, u4DDR3Start;
    #endif  

    #endif  
    BOOL fgDecErr;
    BOOL fgOpen;//check sum qiguo
    char strMessage[256];
    UINT32 u4Temp;

//#ifndef DOWN_SCALE_SUPPORT               
#ifndef COMP_HW_CHKSUM
    ULONG u4NonSwapYBase = 0;
    ULONG u4NonSwapCBase = 0;
#endif   

#if VDEC_UFO_ENABLE
    UINT32 u4YCompare, u4CCompare;
    UINT32 u4Idx;
#endif
    //printk("<vdec> %s, %d\n", __FUNCTION__, __LINE__);
    strncpy(_tFileListRecInfo[u4InstID].bFileName, _FileList_Rec[u4InstID], sizeof(_FileList_Rec[u4InstID]));

    #ifdef REDEC  
    if(_u4FileCnt[u4InstID] == _u4ReDecPicNum[u4InstID] )
    {
        if(_u4ReDecNum[u4InstID] != 0)
        {
            _u4ReDecPicNum[u4InstID] = 0xFFFFFFFF;
            _u4ReDecCnt[u4InstID] = _u4ReDecNum[u4InstID];
            vVDecOutputDebugString("/n!!!!!!!!!!!!!! Re-Decode and Wait for debug !!!!!!!!!!!!!!!!\n");
        }
    }
    if(_u4ReDecCnt[u4InstID] > 0)
    {
        _u4ReDecCnt[u4InstID]--;
    }
    #endif

    fgDecErr = FALSE;

    #ifdef GEN_HW_CHKSUM
    #ifndef INTERGRATION_WITH_DEMUX
    vWrite2PC(u4InstID, 9, NULL);
    #endif
    #endif

    #ifndef INTERGRATION_WITH_DEMUX
    #ifndef MPEG4_FORCE_DUMP_OUTPUT // golden compare
    if(!(
		#ifndef VPMODE
		(_fgVerVopCoded0[u4InstID]) || 
		#endif
		((_u4BrokenLink[u4InstID] != 0) && (_u4PicCdTp[u4InstID] == B_TYPE))))
    {
        //#if (defined(COMP_HW_CHKSUM) && (!defined(DOWN_SCALE_SUPPORT)))
        #if VDEC_MP2_COMPARE_CRC // MPEG2 compare CRC
        if(!fgCompMPEGChkSumGolden(u4InstID))
        {
            fgDecErr = TRUE;
            printk("<vdec> Check sum comparison mismatch\n");
        }
        #else // compare pixel by pixel
        // Y compare
        _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
        _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
        _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
        _tFBufFileInfo[u4InstID].u4FileLength = 0;  
        // Y decoded data Compare
        if(_u4CodecVer[u4InstID] != VDEC_MPEG2)
        {
            // Cheng-Jung new file path[
           // vMPEG4GoldenGetFilePath(_bFileStr1[u4InstID][1], _bFileStr1[u4InstID][0]);
            // ]
            #if VDEC_UFO_ENABLE
			  vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_bits_Y.out\0", _u4FileCnt[u4InstID]); 		
			  
            #else				 
			  vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], _u4FileCnt[u4InstID]);
            #endif
            printk("[MP4_Dump] Y Path: %s\n", _bFileStr1[u4InstID][3]);
        }
        else
        {
            #if 1 // 8320
                #if VDEC_PP_ENABLE
                vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_y_dbk.out\0", _u4FileCnt[u4InstID]);
                #else
                  #if VDEC_UFO_ENABLE
                    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_bits_Y.out\0", _u4FileCnt[u4InstID]);                
                  #else                
                    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_y_gold.raw\0", _u4FileCnt[u4InstID]);
                  #endif
                #endif
			#else
            vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], ".ramY\0", _u4FileCnt[u4InstID]);
			#endif
            printk("[MP2_Dump] Y Path: %s\n", _bFileStr1[u4InstID][3]);            
        }

        #ifdef EXT_COMPARE 
        _tFBufFileInfo[u4InstID].u4FileLength = (((_u4RealHSize[u4InstID] + 15)>>4)<<4) *(((_u4RealVSize[u4InstID] + 31)>>5)<<5);
        #else
        #ifdef DIRECT_DEC
        if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
        #endif    
        {
            #ifdef DOWN_SCALE_SUPPORT
            if(_u4CodecVer[u4InstID] == VDEC_MPEG2)
            {
                if(!(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecMPEGDecPrm.fgDec2ndFld))
                {
                    x_memset(_pucDumpYBuf[u4InstID], 0x0, GOLD_Y_SZ);
                    x_memset(_pucDumpCBuf[u4InstID], 0x0, GOLD_C_SZ);
                }
            }
//add flush cache here qiguo 29/4
		///TODO: HalFlushInvalidateDCache();
            u4NonSwapYBase = (UINT32)_pucDecWorkBuf[u4InstID];
            u4NonSwapCBase = (UINT32)_pucDecCWorkBuf[u4InstID];
            if (_tVerMpvDecPrm[u4InstID].ucAddrSwapMode != 4)
            {
                UINT32 u4AlignWidth, u4AlignHeight;
                UINT32 u4AlignSize = 0;

                // swap off down scaler data
                u4AlignWidth = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW;
                u4AlignWidth = (((u4AlignWidth +63) >>6) <<6); //Align to 4MB width

                if((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD) ||(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD))
                {
                    u4AlignHeight = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV + (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight*2);
                }
                else
                {
                    u4AlignHeight = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV + _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight;
                }

                u4AlignHeight =  (((u4AlignHeight +31) >>5) <<5);

                vMPEG_InvAddressSwap(u4InstID, 
                                   (BYTE*)_pucVDSCLBuf[u4InstID],  (BYTE*)(_pucVDSCLBuf[u4InstID] + 0x1FE000), 
                                   (BYTE*)_pucDumpYBuf[u4InstID], (BYTE*) _pucDumpCBuf[u4InstID],
                                  u4AlignWidth,  u4AlignHeight, u4AlignSize,
                                  _tVerMpvDecPrm[u4InstID].ucAddrSwapMode);

                u4NonSwapYBase = (UINT32)_pucVDSCLBuf[u4InstID];
                u4NonSwapCBase = (UINT32)(_pucVDSCLBuf[u4InstID] + 0x1FE000);
                memcpy((UCHAR*)u4NonSwapYBase, _pucDumpYBuf[u4InstID],(u4AlignWidth*u4AlignHeight) +u4AlignSize);
                memcpy((UCHAR*)u4NonSwapCBase, _pucDumpCBuf[u4InstID],(u4AlignWidth*u4AlignHeight/2) + u4AlignSize);                 

                // swap off mc data
                u4AlignWidth = _tVerMpvDecPrm[u4InstID].u4PicW;
                u4AlignWidth = (((u4AlignWidth +63) >>6) <<6); //Align to 4MB width                    
                u4AlignHeight = _tVerMpvDecPrm[u4InstID].u4PicH;
                u4AlignHeight =  (((u4AlignHeight +31) >>5) <<5);              

                vMPEG_InvAddressSwap(u4InstID, 
                                   (BYTE*)_pucDecWorkBuf[u4InstID],  (BYTE*)_pucDecCWorkBuf[u4InstID], 
                                   (BYTE*)_pucDumpYBuf[u4InstID], (BYTE*) _pucDumpCBuf[u4InstID],
                                  u4AlignWidth,  u4AlignHeight, u4AlignSize,
                                  _tVerMpvDecPrm[u4InstID].ucAddrSwapMode);

                u4NonSwapYBase = (UINT32)_pucPpYSa[u4InstID];
                u4NonSwapCBase = (UINT32)_pucPpCSa[u4InstID];
                memcpy((UCHAR*)u4NonSwapYBase, _pucDumpYBuf[u4InstID],(u4AlignWidth*u4AlignHeight) +u4AlignSize);
                memcpy((UCHAR*)u4NonSwapCBase, _pucDumpCBuf[u4InstID],(u4AlignWidth*u4AlignHeight/2) + u4AlignSize);                 

                printk("MPV emu mpeg inverse swap mode %d\n", _tVerMpvDecPrm[u4InstID].ucAddrSwapMode);
            }

            if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecMPEGDecPrm.rMpegPpInfo.fgPpEnable)
            {
                vGenerateDownScalerGolden(u4InstID, (UINT32)_pucPpYSa[u4InstID],(UINT32)(_pucPpCSa[u4InstID]),u4Size);
            }
            else
            {
                vGenerateDownScalerGolden(u4InstID, u4NonSwapYBase, u4NonSwapCBase, u4Size);
            }
            #else
      	    #if (VDEC_PP_ENABLE)
      	    u4NonSwapYBase = (ULONG)_pucPpYSa[u4InstID];
            u4NonSwapCBase = (ULONG)_pucPpCSa[u4InstID];
	     #else
		 	#if VMMU_SUPPORT
            u4NonSwapYBase = (ULONG)_pucDecWorkBuf[u4InstID] + 0x1000;
            u4NonSwapCBase = (ULONG)_pucDecCWorkBuf[u4InstID] + 0x1000;
			#else
            u4NonSwapYBase = (ULONG)_pucDecWorkBuf[u4InstID];
            u4NonSwapCBase = (ULONG)_pucDecCWorkBuf[u4InstID];
	     #endif
	     #endif
	     ///TODO: HalFlushInvalidateDCache();
            if (_tVerMpvDecPrm[u4InstID].ucAddrSwapMode != 4)
            {
                UINT32 u4AlignWidth, u4AlignHeight;
                UINT32 u4AlignSize = 0;

                u4AlignWidth = _tVerMpvDecPrm[u4InstID].u4PicW;
                u4AlignWidth = (((u4AlignWidth +63) >>6) <<6); //Align to 4MB width                    
                u4AlignHeight = _tVerMpvDecPrm[u4InstID].u4PicH;
                u4AlignHeight =  (((u4AlignHeight +31) >>5) <<5);              

                vMPEG_InvAddressSwap(u4InstID, 
                                   #if (VDEC_PP_ENABLE)
                                   (BYTE*)_pucPpYSa[u4InstID],  (BYTE*)_pucPpCSa[u4InstID], 
                                   #else
                                   (BYTE*)_pucDecWorkBuf[u4InstID],  (BYTE*)_pucDecCWorkBuf[u4InstID], 
                                   #endif
                                   (BYTE*)_pucDumpYBuf[u4InstID], (BYTE*) _pucDumpCBuf[u4InstID],
                                  u4AlignWidth,  u4AlignHeight, u4AlignSize,
                                  _tVerMpvDecPrm[u4InstID].ucAddrSwapMode);

                u4NonSwapYBase = (UINT32)_pucVDSCLBuf[u4InstID];
                u4NonSwapCBase = (UINT32)_pucVDSCLBuf[u4InstID] + 0x1FE000;
                memcpy((UCHAR*)u4NonSwapYBase, _pucDumpYBuf[u4InstID],(u4AlignWidth*u4AlignHeight) +u4AlignSize);
                memcpy((UCHAR*)u4NonSwapCBase, _pucDumpCBuf[u4InstID],(u4AlignWidth*u4AlignHeight/2) + u4AlignSize);                 

                printk("MPV emu mpeg inverse swap mode %d\n", _tVerMpvDecPrm[u4InstID].ucAddrSwapMode);
            }

                   fgOpenFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);               
            _u4GoldenYSize[u4InstID] = _tFBufFileInfo[u4InstID].u4RealGetBytes;
            #endif
        }
        #endif


        u4Cnt = 0; 
        #ifdef EXT_COMPARE 
        if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
        {
            vWrite2PC(u4InstID, 5, _pucDecWorkBuf[u4InstID]);
        }
        #else
        #if defined(DOWN_SCALE_SUPPORT)
        u4YBase = (UINT32)_pucVDSCLBuf[u4InstID];
        //u4BufferWidth = ((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW + 15) >> 4) << 4;
        u4Width = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW;
        if((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD) ||(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD))
        {
            u4Height = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV + (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight*2);
        }
        else
        {
            u4Height = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV + _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight;
        }

        printk("MPV emu down scaler target width = %d\n", u4Width);
        printk("MPV emu down scaler target height = %d\n", u4Height);
        
        #else
        u4YBase = u4NonSwapYBase;//(UINT32)_pucDecWorkBuf[u4InstID];
        //u4BufferWidth = ((_tVerPic[u4InstID].u4W+ 15) >> 4) << 4;
        u4Width = _u4RealHSize[u4InstID];
        u4Height = _u4RealVSize[u4InstID];
        #endif  

//add flush cache here qiguo 29/4
	///TODO: HalFlushInvalidateDCache();
        //fred add for 32byte align in height
        //u4Height = ((u4Height>>5)<<5);
        printk("[Cheng-Jung] Comparing... YBase 0x%px\n", u4YBase);
        #ifdef DIRECT_DEC
        if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
        #endif    
        {
            #ifdef GOLDEN_128BIT_COMP
            u4Tx0 = (u4Width >> 4);   // w/ 16
            u4Ty0 = (u4Height >> 5);  // h /32
            u4X = (u4Width & 0xF);    // w % 16
            u4Y = (u4Height & 0x1F);  // h%32
            u4Tx1 = (u4X==0)? u4Tx0 : (u4Tx0+1);
            u4Ty1 = (u4Y==0)? u4Ty0 : (u4Ty0+1);
//	     printk("real decode data length = 0x%x\n",u4Tx1*u4Ty1*16*32);

            #if VDEC_DDR3_SUPPORT
            u4TxDDR3 = (((u4Width + 15) >> 4) + 3) / 4 * 4;
            printk("u4Tx1,u4TxDDR3:%x,%x\n",u4Tx1,u4TxDDR3);
            #endif

	        printk("Compare Y data start .......\n");
            for (mbh=0; mbh < u4Ty1; mbh++)
            {
                for (mbw=0; mbw < u4Tx1; mbw++)
                {
                    u4Start = (mbh*u4Tx1 + mbw) * (16*32);

                    #if VDEC_DDR3_SUPPORT
                    u4DDR3Start = (mbh*u4TxDDR3 + mbw) * (16*32);
                    #endif

                    pbGoldenBuf = (UCHAR*) (((ULONG) (_pucDumpYBuf[u4InstID])) + u4Start);
                    pbDecBuf = (UCHAR*) (u4YBase + u4Start);

                    #if VDEC_DDR3_SUPPORT
                    pbDecBuf = (UCHAR*) (u4YBase + u4DDR3Start);
//			if(u4Start != u4DDR3Start)
//			{
//				printk("mbh = 0x%x,mbw = 0x%x\n",mbh,mbw);
//			}
                    #endif


                    for(j=0; j < 32; j++)
                    {             
                        for(i=0; i < 16; i++)
                        {                   
                            if((_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecMPEGDecPrm.fgDec2ndFld) || (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRM_PIC))
                            {
                                if(  (mbw == u4Tx0 && i >= u4X) || (mbh == u4Ty0 && j >= u4Y))
                                {
                                    //Do not compare
                                }
                                else
                                {
                                    if((j==0)&&(mbh==0)&&(mbw==0)) //Louis
                                    {
                                        printk("%x ",*(pbDecBuf));
                                    }
                                    if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                                    {
                                        u4Cnt ++;
                                        u4XPix = mbw * 16 + i;
                                        u4YPix = mbh * 32 + j;
								        printk("Pic count to [%d]\n", _u4FileCnt[u4InstID]);  
                                        printk("Y Data Mismatch at [x=%d, y=%d] = 0x%x, Golden = 0x%x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                        //sprintf(strMessage,"Y Data Mismatch at [x=%d, y=%d] = 0x%x, Golden = 0x%x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
										//printk("%s",strMessage);                                        
                                        fgDecErr = TRUE;                                        
                                    #if (!VDEC_UFO_ENABLE) //Louis-Kuo                                       
                                        //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                        //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                        fgDecErr = TRUE;
                                        //vDumpReg();  // mark by ginny
                                        vVDEC_HAL_MPEG_VDec_DumpReg(u4InstID, FALSE);  // mark by ginny
                                        //vWrite2PC(u4InstID, 12, _pucDecWorkBuf[u4InstID]); // dump check sum
                                        //vWrite2PC(u4InstID, 13,_pucVFifo[u4InstID] + _u4CurrPicStartAddr[u4InstID]);
                                        //vWrite2PC(u4InstID, 1, _pucVFifo[u4InstID]);
                                        _tFBufFileInfo[u4InstID].u4FileLength = (((_tVerPic[u4InstID].u4W+ 15) >> 4) << 4) * (((_tVerPic[u4InstID].u4H+ 32) >> 5) << 5);
                                        //vWrite2PC(u4InstID, 2, _pucDecWorkBuf[u4InstID]);
                                        #if defined(DOWN_SCALE_SUPPORT)
                                        _tFBufFileInfo[u4InstID].u4FileLength = (((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5);
                                        vWrite2PC(u4InstID, 5, _pucVDSCLBuf[u4InstID]);
                                        vWrite2PC(u4InstID, 14, _pucDumpYBuf[u4InstID]); //dump VDSCL golden
                                        vWrite2PC(u4InstID, 16, (UCHAR*)&_tVerMpvDecPrm[u4InstID].rDownScalerPrm); //dump VDSCL parameter
                                        #endif 
                                        #endif        
                                        break;    
                                    }
                                }
                            }//end of if


                            pbGoldenBuf++;
                            pbDecBuf++;
                            
                            //pbDecBuf = (UCHAR*)u4CalculatePixelAddress_Y((UINT32) (u4YBase + u4Start), i, j, 16, 1, 4);
                        }//End of i

                        if(fgDecErr == TRUE)
                        {
                        break;
                        }
                    }//End of j

                    if(fgDecErr == TRUE)
                    {
                        break;
                    }
                }

                if(fgDecErr == TRUE)
                {
                    break;
                }
            }
            //msleep(100);
            if(fgDecErr == TRUE)
                printk("[MPEG4]Y Data Compare fail\n");
            else
                printk("[MPEG4]Y Data Compare pass\n");
        #if VDEC_UFO_ENABLE //compare Y len
            u4YCompare = (((_tVerMpvDecPrm[u4InstID].u4PicW+15)>>4)<<4)*(((_tVerMpvDecPrm[u4InstID].u4PicH+31)>>5)<<5)/256;
            u4CCompare = u4YCompare >>1; 
            _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
            _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
            _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
            _tFBufFileInfo[u4InstID].u4FileLength = 0; 
			// Y decoded data Compare
           	vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_len_Y.out\0", _u4FileCnt[u4InstID]);                
		
            printk("[MP4_Dump] Y Path: %s\n", _bFileStr1[u4InstID][3]);
            fgOpenFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);               
            _u4GoldenYSize[u4InstID] = _tFBufFileInfo[u4InstID].u4RealGetBytes;
            pbGoldenBuf = (UCHAR*) ((ULONG) (_pucDumpYBuf[u4InstID]));
            pbDecBuf = (UCHAR*) (ULONG)(u4YBase + _tVerMpvDecPrm[u4InstID].u4PIC_SIZE_BS);
            
            for(u4Idx=0; u4Idx<_u4GoldenYSize[u4InstID]/sizeof(ULONG); u4Idx++){
                if(u4Idx*sizeof(ULONG)>=u4YCompare) break;
                if(pbDecBuf[u4Idx] != pbGoldenBuf[u4Idx]){
                sprintf(strMessage,"Y len Mismatch at [%d]= 0x%x, Golden = 0x%x !!! \n", u4Idx, pbDecBuf[u4Idx], pbGoldenBuf[u4Idx]);
                printk("%s",strMessage);
                fgDecErr = TRUE;
                break;
                }
            } 
            msleep(100);
            if(fgDecErr == TRUE)
                printk("[MPEG4 UFO]Y len Compare fail\n");
            else
                printk("[MPEG4 UFO]Y len Compare pass\n");
        #endif
            #else              
            for (u4Pix = 0; u4Pix < u4Width*u4Height; u4Pix++)
            {
                if((_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecMPEGDecPrm.fgDec2ndFld) || (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRM_PIC))
                {
                    //pbDecBuf = (UCHAR*)u4CalculatePixelAddress_Y(u4YBase, u4XPix, u4YPix, u4BufferWidth, 1, 4);
                    pbDecBuf = (UCHAR*)(u4YBase+u4Pix);
                    //pbGoldenBuf = (UCHAR*)u4CalculatePixelAddress_Y((UINT32)_pucDumpYBuf[u4InstID], u4XPix, u4YPix, u4BufferWidth, 1, 4);
                    pbGoldenBuf = (UCHAR*)((UINT32)_pucDumpYBuf[u4InstID]+u4Pix);
                    if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                    {
                        u4Cnt ++;
                        //vVDecOutputDebugString("Y Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                        vVDecOutputDebugString("Y Data Mismatch at [%d] = 0x%.2x, Golden = 0x%.2x !!! \n", u4Pix, (*pbDecBuf), (*pbGoldenBuf));
                        sprintf(strMessage,"Y Data Mismatch at [%d] = 0x%.2x, Golden = 0x%.2x !!! \n", u4Pix, (*pbDecBuf), (*pbGoldenBuf));
                        fgDecErr = TRUE;                        
                    #if (!VDEC_UFO_ENABLE)
                        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                        //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpYBuf[_u4VDecID][i]);
                        fgDecErr = TRUE;
                        //vDumpReg();  // mark by ginny
                        //vWrite2PC(_u4VDecID, 2, _pucDecWorkBuf);
                        vVDEC_HAL_MPEG_VDec_DumpReg(u4InstID, FALSE);  // mark by ginny
                        vWrite2PC(u4InstID, 12, _pucDecWorkBuf[u4InstID]); // dump check sum
                        vWrite2PC(u4InstID, 1, _pucVFifo[u4InstID]);
                        _tFBufFileInfo[u4InstID].u4FileLength = (((_u4RealHSize[u4InstID]+ 15) >> 4) << 4) * (((_u4RealVSize[u4InstID]+ 32) >> 5) << 5);
                        vWrite2PC(u4InstID, 2, _pucDecWorkBuf[u4InstID]);
                        #if defined(DOWN_SCALE_SUPPORT)
                        _tFBufFileInfo[u4InstID].u4FileLength = (((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5);
                        vWrite2PC(u4InstID, 5, _pucVDSCLBuf[u4InstID]);
                        vWrite2PC(u4InstID, 14, _pucDumpYBuf[u4InstID]); //dump VDSCL golden
                        #endif
                        #endif        
                        break;    
                    }            
                }
                if(fgDecErr == TRUE)
                {
                    break;
                }
            }

            /*
            for (u4XPix = 0; u4XPix < u4Width; u4XPix++)
            {
                for (u4YPix = 0; u4YPix < u4Height; u4YPix++)
                {
                    if((_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecMPEGDecPrm.fgDec2ndFld) || (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRM_PIC))
                    {
                        // compare Y
                        #ifdef DOWN_SCALE_SUPPORT
                        pbDecBuf = (UCHAR*)u4CalculatePixelAddress_Y(u4YBase, u4XPix, u4YPix, u4BufferWidth, (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType == 0), 4);
                        pbGoldenBuf = (UCHAR*)u4CalculatePixelAddress_Y((UINT32)_pucDumpYBuf[u4InstID], u4XPix, u4YPix, u4BufferWidth, (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType == 0), 4);
                        #else
                        pbDecBuf = (UCHAR*)u4CalculatePixelAddress_Y(u4YBase, u4XPix, u4YPix, u4BufferWidth, 1, 4);
                        pbGoldenBuf = (UCHAR*)u4CalculatePixelAddress_Y((UINT32)_pucDumpYBuf[u4InstID], u4XPix, u4YPix, u4BufferWidth, 1, 4);
                        #endif


                        if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                        {
                            u4Cnt ++;
                            vVDecOutputDebugString("Y Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                            sprintf(strMessage,"Y Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                            //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpYBuf[_u4VDecID][i]);
                            fgDecErr = TRUE;
                            //vDumpReg();  // mark by ginny
                            //vWrite2PC(_u4VDecID, 2, _pucDecWorkBuf);
                            vWrite2PC(u4InstID, 12, _pucDecWorkBuf[u4InstID]); // dump check sum
                            vWrite2PC(u4InstID, 1, _pucVFifo[u4InstID]);
                            _tFBufFileInfo[u4InstID].u4FileLength = (((_u4RealHSize[u4InstID]+ 15) >> 4) << 4) * (((_u4RealVSize[u4InstID]+ 32) >> 5) << 5);
                            vWrite2PC(u4InstID, 2, _pucDecWorkBuf[u4InstID]);
                            #if defined(DOWN_SCALE_SUPPORT)
                            _tFBufFileInfo[u4InstID].u4FileLength = (((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5);
                            vWrite2PC(u4InstID, 5, _pucVDSCLBuf[u4InstID]);
                            vWrite2PC(u4InstID, 14, _pucDumpYBuf[u4InstID]); //dump VDSCL golden
                            #endif        
                            break;    
                        }
                    }

                }
                if(fgDecErr == TRUE)
                {
                break;
                }
            }
            */
            #endif		
            //vVDecOutputDebugString("\nY Data Compare Over!!! Total bytes [0x%.8x] & error [%d]\n", _u4GoldenYSize[_u4VDecID], u4Cnt);
        }
        #endif

        // CbCr compare
        //if(!fgIsMonoPic(_u4VDecID))
        {
            // CbCr decoded data Compare
            _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
            _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpCBuf[u4InstID];
            _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_C_SZ;  
            _tFBufFileInfo[u4InstID].u4FileLength = 0;
            if(_u4CodecVer[u4InstID] != VDEC_MPEG2)
            {
                // Cheng-Jung new file path[
                //vMPEG4GoldenGetFilePath(_bFileStr1[u4InstID][1], _bFileStr1[u4InstID][0]);
                // ]     
                #if VDEC_UFO_ENABLE
				vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], "_bits_CbCr.out\0", _u4FileCnt[u4InstID]);
				#else
				vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], _bFileAddStrC[0], _u4FileCnt[u4InstID]);
				#endif
				printk("[MP4_Dump] C Path: %s\n", _bFileStr1[u4InstID][4]);
            }
            else
            {
                #if 1 // 8320
                    #if VDEC_PP_ENABLE
                    vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], "_c_dbk.out\0", _u4FileCnt[u4InstID]);
                    #else
                 #if VDEC_UFO_ENABLE
                    vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], "_bits_CbCr.out\0", _u4FileCnt[u4InstID]);                
                 #else                
                    vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], "_c_gold.raw\0", _u4FileCnt[u4InstID]);
                 #endif
                #endif
				#else
                vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], ".ramCbCr\0", _u4FileCnt[u4InstID]);
				#endif
                printk("[MP2_Dump] C Path: %s\n", _bFileStr1[u4InstID][4]);                
            }
            #ifdef EXT_COMPARE 
            _tFBufFileInfo[u4InstID].u4FileLength = (((_u4RealHSize[u4InstID] + 15)>>4)<<4) *(((_u4RealVSize[u4InstID] + 31)>>5)<<5) >>1;
            #else
            #ifdef DIRECT_DEC
            if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
            #endif    
            {  
                #ifndef DOWN_SCALE_SUPPORT
                   fgOpenFile(u4InstID, _bFileStr1[u4InstID][4],"r+b", &_tFBufFileInfo[u4InstID]);               
                _u4GoldenCSize[u4InstID] = _tFBufFileInfo[u4InstID].u4RealGetBytes;
                #endif
            }
            #endif       
            u4Cnt = 0; 
            #ifdef EXT_COMPARE  
            if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
            {
                vWrite2PC(u4InstID, 6, _pucDecCWorkBuf[u4InstID]);
            }
            #else    
            #if defined(DOWN_SCALE_SUPPORT)
            u4CBase = (UINT32)_pucVDSCLBuf[u4InstID] + u4DramPicSize;
            #else
            u4CBase = u4NonSwapCBase;//(UINT32)_pucDecCWorkBuf[u4InstID];
            #endif  

//add flush cache here qiguo 29/4
		///TODO: HalFlushInvalidateDCache();
			
            #ifdef DIRECT_DEC
            if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
            #endif      
            {
                #ifdef GOLDEN_128BIT_COMP
                UINT32 u4WidthC = u4Width / 2;
                UINT32 u4HeightC = u4Height / 2;
                u4Tx0 = (u4WidthC >> 3);   // w/ 8
                u4Ty0 = (u4HeightC >> 4);  // h /16
                u4X = (u4WidthC & 0x7);    // w % 8
                u4Y = (u4HeightC & 0xF);  // h % 16
                u4Tx1 = (u4X==0)? u4Tx0 : (u4Tx0+1);
                u4Ty1 = (u4Y==0)? u4Ty0 : (u4Ty0+1);

                #if VDEC_DDR3_SUPPORT
                u4TxDDR3 = ((((u4Width + 15) >> 4) + 3) / 4 * 4);
                printk("u4Tx1,u4TxDDR3 in C:%x,%x\n",u4Tx1,u4TxDDR3);
                #endif

		  printk("Compare CbCr data start .......\n");
                for (mbh=0; mbh < u4Ty1; mbh++)
                {
                    for (mbw=0; mbw < u4Tx1; mbw++)
                    {
                        u4Start = (mbh*u4Tx1 + mbw) * (16*16);

                        #if VDEC_DDR3_SUPPORT
                        u4DDR3Start = (mbh*u4TxDDR3 + mbw) * (16*16);
                        #endif

                        pbGoldenBuf = (UCHAR*) (((ULONG) (_pucDumpCBuf[u4InstID])) + u4Start);
                        pbDecBuf = (UCHAR*) (u4CBase + u4Start);

                        #if VDEC_DDR3_SUPPORT
                        pbDecBuf = (UCHAR*) (u4CBase + u4DDR3Start);
                        #endif


                        for(j=0; j < 16; j++)
                        {
                            for(i=0; i < 8; i++)
                            {                       
                                if((_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecMPEGDecPrm.fgDec2ndFld) || (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRM_PIC))
                                {
                                    if(  (mbw == u4Tx0 && i >= u4X) || (mbh == u4Ty0 && j >= u4Y))
                                    {
                                        //Do not compare
                                        pbGoldenBuf+=2;
                                        pbDecBuf+=2;
                                    }
                                    else
                                    {
                                        //Compare Cb
                                        if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                                        {
                                            u4XPix = mbw * 8 + i;
                                            u4YPix = mbh * 16 + j;
                                            u4Cnt ++;
                                            vVDecOutputDebugString("Cb Data Mismatch at [x=%d, y=%d] = 0x%x, Golden = 0x%x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                            sprintf(strMessage,"<vdec> Cb Data Mismatch at [x=%d, y=%d] = 0x%x, Golden = 0x%x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                            printk("%s",strMessage);                                            
                                            fgDecErr = TRUE;                                            
                                            #if (!VDEC_UFO_ENABLE)                                            
                                            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                            //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                            fgDecErr = TRUE;
                                            //vDumpReg();  // mark by ginny
                                            vVDEC_HAL_MPEG_VDec_DumpReg(u4InstID, FALSE);  // mark by ginny
                                            vWrite2PC(u4InstID, 12, _pucDecCWorkBuf[u4InstID]); // dump check sum
                                            _tFBufFileInfo[u4InstID].u4FileLength = ((((_tVerPic[u4InstID].u4W+ 15) >> 4) << 4) * (((_tVerPic[u4InstID].u4H+ 32) >> 5) << 5)) / 2;
                                            vWrite2PC(u4InstID, 3, _pucDecCWorkBuf[u4InstID]);
                                            vWrite2PC(u4InstID, 1, _pucVFifo[u4InstID]);
                                            #if defined(DOWN_SCALE_SUPPORT)
                                            _tFBufFileInfo[u4InstID].u4FileLength = ((((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5))/(2 - _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt);
                                            vWrite2PC(u4InstID, 6, _pucVDSCLBuf[u4InstID] + u4Size);
                                            vWrite2PC(u4InstID, 15, _pucDumpCBuf[u4InstID]); //dump VDSCL golden
                                            vWrite2PC(u4InstID, 16, (UCHAR*)&_tVerMpvDecPrm[u4InstID].rDownScalerPrm); //dump VDSCL parameter
                                            #endif        
                                            #endif                                            
                                            break;    
                                        }

                                        pbGoldenBuf++;
                                        pbDecBuf++;
                                        //Compare Cr
                                        if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                                        {
                                            u4XPix = mbw * 8 + i;
                                            u4YPix = mbh * 16 + j;
                                            u4Cnt ++;
                                            vVDecOutputDebugString("Cr Data Mismatch at [x=%d, y=%d] = 0x%x, Golden = 0x%x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                            sprintf(strMessage,"Cr Data Mismatch at [x=%d, y=%d] = 0x%x, Golden = 0x%x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                            fgDecErr = TRUE;                                            
                                            #if (!VDEC_UFO_ENABLE)                                            
                                            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                            //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                            fgDecErr = TRUE;
                                            //vDumpReg();  // mark by ginny
                                            vVDEC_HAL_MPEG_VDec_DumpReg(u4InstID, FALSE);  // mark by ginny
                                            vWrite2PC(u4InstID, 12, _pucDecCWorkBuf[u4InstID]); // dump check sum
                                            vWrite2PC(u4InstID, 3, _pucDecCWorkBuf[u4InstID]);
                                            #if defined(DOWN_SCALE_SUPPORT)
                                            _tFBufFileInfo[u4InstID].u4FileLength = ((((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5))/(2 - _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt);
                                            vWrite2PC(u4InstID, 6, _pucVDSCLBuf[u4InstID] + u4Size);
                                            vWrite2PC(u4InstID, 15, _pucDumpCBuf[u4InstID]); //dump VDSCL golden
                                            vWrite2PC(u4InstID, 16, (UCHAR*)&_tVerMpvDecPrm[u4InstID].rDownScalerPrm); //dump VDSCL parameter
                                            #endif 
                                            #endif        
                                            break;    
                                        }
                                        pbGoldenBuf++;
                                        pbDecBuf++;
                                    }
                                }
                                else
                                {
                                    pbGoldenBuf+=2;
                                    pbDecBuf+=2;
                                }

                            }
                            if(fgDecErr == TRUE)
                            {
                                break;
                            }
                        }

                        if(fgDecErr == TRUE)
                        {
                            break;
                        }
                    }

                    if(fgDecErr == TRUE)
                    {
                        break;
                    }
                }
                //msleep(100);
                if(fgDecErr == TRUE)
                    printk("[MPEG4]CbCr Data Compare fail\n");
                else
                    printk("[Louis-Kuo]CbCr Data Compare pass\n");
        #if VDEC_UFO_ENABLE //compare CbCr len
            u4YCompare = (((_tVerMpvDecPrm[u4InstID].u4PicW+15)>>4)<<4)*(((_tVerMpvDecPrm[u4InstID].u4PicH+31)>>5)<<5)/256;
            u4CCompare = u4YCompare >>1; 
            _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
            _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
            _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_C_SZ;  
            _tFBufFileInfo[u4InstID].u4FileLength = 0; 
            vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], "_len_CbCr.out\0", _u4FileCnt[u4InstID]);                
            printk("[MP4_Dump] CbCr len Path: %s\n", _bFileStr1[u4InstID][3]);
            fgOpenFile(u4InstID, _bFileStr1[u4InstID][4],"r+b", &_tFBufFileInfo[u4InstID]);               
            _u4GoldenYSize[u4InstID] = _tFBufFileInfo[u4InstID].u4RealGetBytes;
            pbGoldenBuf = (UCHAR*) ((ULONG) (_pucDumpYBuf[u4InstID]));
            pbDecBuf = (UCHAR*) (u4YBase + _tVerMpvDecPrm[u4InstID].u4PIC_SIZE_BS+_tVerMpvDecPrm[u4InstID].u4UFO_LEN_SIZE_Y);
            
            for(u4Idx=0; u4Idx<_u4GoldenYSize[u4InstID]/sizeof(ULONG); u4Idx++){
                if(u4Idx*sizeof(ULONG)>=u4CCompare) break;
                if(pbDecBuf[u4Idx] != pbGoldenBuf[u4Idx]){
                sprintf(strMessage,"CbCr len Mismatch at [%d]= 0x%x, Golden = 0x%x !!! \n", u4Idx, pbDecBuf[u4Idx], pbGoldenBuf[u4Idx]);
                printk("%s",strMessage);
                fgDecErr = TRUE;
                break;
                }
            } 
            msleep(100);
            if(fgDecErr == TRUE)
                printk("[MPEG4 UFO]CbCr len Compare fail\n");
            else
                printk("[MPEG4 UFO]CbCr len Compare pass\n");
        #endif
                #else                          
                for (u4Pix = 0; u4Pix < u4Width*(u4Height>>1); u4Pix++)
                {
                    if((_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecMPEGDecPrm.fgDec2ndFld) || (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRM_PIC))
                    {
                        pbDecBuf = (UCHAR*)(u4CBase+u4Pix);
                        pbGoldenBuf = (UCHAR*)((ULONG)_pucDumpCBuf[u4InstID]+u4Pix);
                        if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                        {
                        #if defined(DOWN_SCALE_SUPPORT)
                            if(VDEC_PP_ENABLE)
                            {
                                if ((((*(pbDecBuf)) > (*(pbGoldenBuf))) && ((*(pbDecBuf))<= (*(pbGoldenBuf) + 1)))  || 
                                    (((*(pbDecBuf)) < (*(pbGoldenBuf))) && ((*(pbDecBuf))>= (*(pbGoldenBuf) - 1)) ))
                                    {
                                        //Pass
                                        //How the C Code round off floating number method is not the same as HW in full agreement
                                        //Therefor, difference between +-1 is tolerated
                                    }
                            }
                            else
                        #endif
                            {
                            u4Cnt ++;
                            vVDecOutputDebugString("C Data Mismatch at [%d] = 0x%.2x, Golden = 0x%.2x !!! \n", u4Pix, (*pbDecBuf), (*pbGoldenBuf));
                            sprintf(strMessage,"C Data Mismatch at [%d] = 0x%.2x, Golden = 0x%.2x !!! \n", u4Pix, (*pbDecBuf), (*pbGoldenBuf));
                            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                            //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                            fgDecErr = TRUE;
                            vVDEC_HAL_MPEG_VDec_DumpReg(u4InstID, FALSE);  // mark by ginny
                            //vDumpReg();  // mark by ginny
                            //vWrite2PC(_u4VDecID, 2, _pucDecWorkBuf);
                            vWrite2PC(u4InstID, 12, _pucDecWorkBuf[u4InstID]); // dump check sum
                            vWrite2PC(u4InstID, 1, _pucVFifo[u4InstID]);
                            _tFBufFileInfo[u4InstID].u4FileLength = ((((_u4RealHSize[u4InstID]+ 15) >> 4) << 4) * (((_u4RealVSize[u4InstID]+ 32) >> 5) << 5))/2;
                            vWrite2PC(u4InstID, 3, _pucDecCWorkBuf[u4InstID]);          		  
                            break;    
                        }
                    }
                    }
                    if(fgDecErr == TRUE)
                    {
                        break;
                    }
                }
                /*
                for (u4XPix = 0; u4XPix < u4Width; u4XPix++)
                {
                    for (u4YPix = 0; u4YPix < u4Height; u4YPix++)
                    {
                        if((_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecMPEGDecPrm.fgDec2ndFld) || (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRM_PIC))
                        {
                            // compare Cb
                            #ifdef DOWN_SCALE_SUPPORT
                            pbDecBuf = (UCHAR*)u4CalculatePixelAddress_C(u4CBase, u4XPix, u4YPix, u4BufferWidth, (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType == 0), 1, 4);
                            pbGoldenBuf = (UCHAR*)u4CalculatePixelAddress_C((UINT32)_pucDumpCBuf[u4InstID], u4XPix, u4YPix, u4BufferWidth, (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType == 0), 1, 4);
                            #else
                            pbDecBuf = (UCHAR*)u4CalculatePixelAddress_C(u4CBase, u4XPix, u4YPix, u4BufferWidth, 1, 1, 4);
                            pbGoldenBuf = (UCHAR*)u4CalculatePixelAddress_C((UINT32)_pucDumpCBuf[u4InstID], u4XPix, u4YPix, u4BufferWidth, 1, 1, 4);
                            #endif


                            if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                            {
                                u4Cnt ++;
                                vVDecOutputDebugString("Cb Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                sprintf(strMessage,"Cb Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                fgDecErr = TRUE;
                                //vDumpReg();  // mark by ginny
                                //vWrite2PC(_u4VDecID, 2, _pucDecWorkBuf);
                                vWrite2PC(u4InstID, 12, _pucDecWorkBuf[u4InstID]); // dump check sum
                                vWrite2PC(u4InstID, 1, _pucVFifo[u4InstID]);
                                _tFBufFileInfo[u4InstID].u4FileLength = ((((_u4RealHSize[u4InstID]+ 15) >> 4) << 4) * (((_u4RealVSize[u4InstID]+ 32) >> 5) << 5))/2;
                                vWrite2PC(u4InstID, 3, _pucDecCWorkBuf[u4InstID]);
                                #if defined(DOWN_SCALE_SUPPORT)
                                _tFBufFileInfo[u4InstID].u4FileLength = ((((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5))/(2 - _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt);
                                vWrite2PC(u4InstID, 6, _pucVDSCLBuf[u4InstID] + u4Size);
                                vWrite2PC(u4InstID, 15, _pucDumpCBuf[u4InstID]); //dump VDSCL golden
                                #endif        
                                break;    
                            }
                            // compare Cr
                            pbDecBuf++;
                            pbGoldenBuf++;
                            if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                            {
                                u4Cnt ++;
                                vVDecOutputDebugString("Cr Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                sprintf(strMessage,"Cr Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                fgDecErr = TRUE;
                                //vDumpReg();  // mark by ginny
                                //vWrite2PC(_u4VDecID, 2, _pucDecWorkBuf);
                                vWrite2PC(u4InstID, 3, _pucDecCWorkBuf[u4InstID]);
                                #if defined(DOWN_SCALE_SUPPORT)
                                vWrite2PC(u4InstID, 6, _pucVDSCLBuf[u4InstID] + u4Size);
                                #endif        
                                break;    
                            }
                        }

                    }
                    if(fgDecErr == TRUE)
                    {
                    break;
                    }
                }
                */
                #endif          
                //vVDecOutputDebugString("CbCr Data Compare Over!!! Total bytes [0x%.8x] & error [%d]\n", _u4GoldenCSize[_u4VDecID], u4Cnt);
            }
            #endif    
        }
        #endif
    }
    #endif // if 0

    #ifndef IDE_WRITE_SUPPORT
    if((_u4FileCnt[u4InstID]%10) == 0)
    #endif
    {
        //#ifndef IDE_WRITE_SUPPORT
        //vVDecOutputDebugString("Pic count to [%d]\n", _u4FileCnt[u4InstID]);  
        //#endif
        //sprintf(strMessage,"[%d], ", _u4FileCnt[u4InstID]);  
        //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
    }
    #endif
#ifdef MPEG4_FORCE_DUMP_OUTPUT // force dump
    fgDecErr = TRUE;
    #if VDEC_PP_ENABLE
    u4YBase = (ULONG)_pucPpYSa[u4InstID];
    u4CBase = (ULONG)_pucPpCSa[u4InstID];
    #else
    u4YBase = (ULONG)_pucDecWorkBuf[u4InstID];
    u4CBase = (ULONG)_pucDecCWorkBuf[u4InstID];
    #endif
#endif

    #ifndef  VPMODE
    if(!_fgVerVopCoded0[u4InstID])
    #endif
    {
        if((_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecMPEGDecPrm.fgDec2ndFld) || (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRM_PIC))
        {
            if (fgDecErr)
            {            
                u4Width = ((_u4RealHSize[u4InstID] + 15) >> 4) << 4;
                u4Height = ((_u4RealVSize[u4InstID] + 31) >> 5) << 5;
                // dump Y data to PC
                if(_u4CodecVer[u4InstID] != VDEC_MPEG2)
                {
                    // Cheng-Jung new file path[
                    //vMPEG4GoldenGetFilePath(_bFileStr1[u4InstID][1], _bFileStr1[u4InstID][0]);
                    // ]
                    #ifdef VPMODE
                    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], u4CompareCnt);
                    #else
                    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], _u4FileCnt[u4InstID]);
                    #endif

                    printk("[MP4_Dump][2] Y Path: %s\n", _bFileStr1[u4InstID][3]);
                }
                else
                {
                    #if 1 // 8320
                    #if VDEC_PP_ENABLE
                    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_y_dbk.out.8320.out\0", _u4FileCnt[u4InstID]);
                    #else
                    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_y_gold.raw.8320.raw\0", _u4FileCnt[u4InstID]);
                    #endif
                    #else
                    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], ".ramY\0", _u4FileCnt[u4InstID]);
                    #endif
                }

                printk("<vdec> dump pattern: %s\n", _bFileStr1[u4InstID][3]);
                //fgWrData2PC(_pucDumpYBuf[u4InstID], u4Width*u4Height, 7, _bFileStr1[u4InstID][3]);                
                #ifdef MPEG4_FORCE_DUMP_OUTPUT
	                //fgWrData2PC(u4YBase, u4Width*u4Height, 7, _bFileStr1[u4InstID][3]);
                #endif

                // dump C data to PC
                if(_u4CodecVer[u4InstID] != VDEC_MPEG2)
                {
                    // Cheng-Jung new file path[
                    //vMPEG4GoldenGetFilePath(_bFileStr1[u4InstID][1], _bFileStr1[u4InstID][0]);
                    // ]
                    #ifdef VPMODE
                    vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], _bFileAddStrC[0], u4CompareCnt);//_u4FileCnt[u4InstID]);
                    #else
                    vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], _bFileAddStrC[0], _u4FileCnt[u4InstID]);
                    #endif
                    printk("[MP4_Dump][2] C Path: %s\n", _bFileStr1[u4InstID][4]);
                }
                else
                {
                    #if 1 // 8320
                    #if VDEC_PP_ENABLE
                    vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], "_c_dbk.out.8320.out\0", _u4FileCnt[u4InstID]);
                    #else
                    vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], "_c_gold.raw.8320.raw\0", _u4FileCnt[u4InstID]);
                    #endif
                    #else
                    vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], ".ramCbCr\0", _u4FileCnt[u4InstID]);
                    #endif
                }
                printk("<vdec> dump pattern: %s\n", _bFileStr1[u4InstID][4]);
                //fgWrData2PC(_pucDumpCBuf[u4InstID], u4Width*u4Height, 7, _bFileStr1[u4InstID][4]);
                #ifdef MPEG4_FORCE_DUMP_OUTPUT
	                //fgWrData2PC(u4CBase, u4Width*u4Height/2, 7, _bFileStr1[u4InstID][4]);
                #endif
            }
            //printk("_u4FileCnt = %d, u4CompareCnt = %d, Y base = 0x%x\n", _u4FileCnt[u4InstID], u4CompareCnt, u4YBase);

            #ifdef REDEC    
            if(_u4ReDecCnt[u4InstID] == 0)
            #endif    
            _u4FileCnt[u4InstID] ++;
	     #ifdef  VPMODE
	     u4CompareCnt++;
         //printk("_u4FileCnt = %d, u4CompareCnt = %d, Y base = 0x%x\n", _u4FileCnt[u4InstID], u4CompareCnt, u4YBase);
	     if(_fgVerVopCoded0[u4InstID])
	     {
	     	printk("VopCoded Index = %d\n",u4CompareCnt);
	     }
	     #endif
        }
    }

    #ifndef INTERGRATION_WITH_DEMUX
    // Check if still pic needed compare
    #if (defined(COMP_HW_CHKSUM) && (!defined(DOWN_SCALE_SUPPORT)))
    _tTempFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tTempFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tTempFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tTempFileInfo[u4InstID].u4FileLength = 0; 
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_chksum.bin\0", _u4FileCnt[u4InstID]);
    #ifdef IDE_READ_SUPPORT
    fgOpen = fgPureOpenIdeFile( _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    #else
    fgOpen = fgOpenFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    #endif
    #else
    _tFBufFileInfo[u4InstID].fgGetFileInfo = FALSE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tFBufFileInfo[u4InstID].u4FileLength = 4;      
    if(_u4CodecVer[u4InstID] != VDEC_MPEG2)
    {
        // Cheng-Jung new file path[
        //vMPEG4GoldenGetFilePath(_bFileStr1[u4InstID][1], _bFileStr1[u4InstID][0]);
        // ]    
        #if VDEC_UFO_ENABLE
		vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0],"_bits_Y.out\0" , _u4FileCnt[u4InstID]);        
		#else
		//vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY2[0], _u4FileCnt[u4InstID]);
        vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], _u4FileCnt[u4InstID]);        
		#endif
    }
    else
    {
        #if 1 // 8320
            #if VDEC_PP_ENABLE
                vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_y_dbk.out\0", _u4FileCnt[u4InstID]);
            #else
                #if VDEC_UFO_ENABLE
                vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_bits_Y.out\0", _u4FileCnt[u4InstID]);                
                #else   
				#if VDEC_MP2_COMPARE_CRC // MPEG2 compare CRC
				//_tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE; 
                vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_0.crc\0", _u4FileCnt[u4InstID]);
				#else
                vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_y_gold.raw\0", _u4FileCnt[u4InstID]);
				#endif
                #endif
            #endif
		#else
        vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], ".ramY\0", _u4FileCnt[u4InstID]);
		#endif
    }
    #ifdef IDE_READ_SUPPORT
    fgOpen = fgPureOpenIdeFile( _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);
    #else
	//printk("fgOpenFile target address 0x%p\n",_tFBufFileInfo[u4InstID].pucTargetAddr);
    fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);
    #endif
    #endif
	if (fgDecErr == FALSE)
    {
        //if ((_u4FileCnt[u4InstID] - 1) % 10 == 0)
        {
            printk("[MPEG] Compare OK Pic Cnt %d\n", _u4FileCnt[u4InstID] - 1);
        }
    }
#ifdef VDEC_SARNOFF_ON
    if(fgOpen == FALSE)
#elif VDEC_PP_ENABLE
    if(fgOpen == FALSE)
#else
    if((fgOpen == FALSE)||(fgDecErr == TRUE) )//|| (_fgVDecErr[u4InstID] == TRUE))
#endif
    {
        sprintf(strMessage, "%s", "\n");
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        //fprintf(_tFileListRecInfo.fpFile, "\n");  
        // break decode
        if(fgOpen == FALSE)
        {
            sprintf(strMessage," Compare Finish==> Pic count to [%d] \n", _u4FileCnt[u4InstID] - 1);   
            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
            //fprintf(_tFileListRecInfo.fpFile, " Compare Finish==> Pic count to [%d] \n", _u4FileCnt[_u4VDecID] - 1);   
            if(_u4FileCnt[u4InstID] == 1)
            {
                if(fgOpen == FALSE)
                {
                    vVDecOutputDebugString("real NULL\n");
                }
            }
        }
        else
        {
            sprintf(strMessage," Error ==> Pic count to [%d] \n", _u4FileCnt[u4InstID] - 1);     
            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
            //fprintf(_tFileListRecInfo.fpFile, " Error ==> Pic count to [%d] \n", _u4FileCnt[_u4VDecID] - 1);         
        }
        #ifndef MPEG4_6589_ERROR_CONCEAL        
        _u4VerBitCount[u4InstID] = 0xffffffff;
        #endif
    }
    #endif
    if(_u4FileCnt[u4InstID] >= _u4EndCompPicNum[u4InstID])
    {
        _u4VerBitCount[u4InstID] = 0xffffffff;
    }
}

#define CRC_SZ 819200
BOOL fgYCrcOpen = FALSE;
BOOL fgCbCrCrcOpen = FALSE;
UINT32 u4FilePicNum = 0;
UINT32 u4FileTotalPicNum = 0;
UINT32 u4FilePicCont_noVOP = 0;
UINT32 u4FirstErrFrm = 0xFFFFFFFF;



CHAR* str_tok(CHAR *ptStr, CHAR *ptDelim)
{
    UINT32 u4DelimLen = strlen(ptDelim);
    UINT32 i = 0;
    UINT32 u4Len = 0;
    BOOL bIsDelim = FALSE;
    if (ptStr) ptSrc = ptStr;

    while(*ptSrc != '\0')
    {
        for (i = 0; i < u4DelimLen; i++)
        {
            if (*ptSrc == ptDelim[i])
            {
                bIsDelim = TRUE;
                break;
            }
        }
        if (bIsDelim)
        {
            ptSrc ++;
            break;
        }
        cTokBuf[u4Len] = *ptSrc;
        u4Len ++;
        ptSrc ++;
    }
    cTokBuf[u4Len] = '\0';

    if (u4Len == 0 && !bIsDelim)
    {
        return 0;
    }
    
    return cTokBuf;
}

void vMPEG4CrcGetFilePath(CHAR *ptBitstreamFilePath, CHAR *ptCrcFilePath)
{
    CHAR *ptTok;
    CHAR cLastTok[300];
    UINT32 u4CrcFilePathLen = 0;
    UINT32 u4TokenLen = 0;
    UINT32 i = 0;
    INT32 dotCnt = 0;
    memset(cLastTok, 0, 300);
    memset(ptCrcFilePath, 0, 300);

    ptTok = str_tok(ptBitstreamFilePath, "\\/");
    while (ptTok != NULL)
    {
        u4TokenLen = strlen(ptTok);
        if (strncmp("bitstream", ptTok, 9) == 0 && u4TokenLen == 9)
        {
            memcpy(ptCrcFilePath + u4CrcFilePathLen, "pattern", 7);            
            u4CrcFilePathLen += 7;
            ptCrcFilePath[u4CrcFilePathLen] = '/'; //'\\';
            u4CrcFilePathLen ++;
			memcpy(ptCrcFilePath + u4CrcFilePathLen, "crc", 3);            
            u4CrcFilePathLen += 3;
            ptCrcFilePath[u4CrcFilePathLen] = '/'; //'\\';
            u4CrcFilePathLen ++;
        }
        else
        {
            memcpy(ptCrcFilePath + u4CrcFilePathLen, ptTok, u4TokenLen);
            u4CrcFilePathLen += u4TokenLen;
            ptCrcFilePath[u4CrcFilePathLen] = '/'; //'\\';
            u4CrcFilePathLen ++;
        }
        memcpy(cLastTok, ptTok, u4TokenLen);
        cLastTok[u4TokenLen] = '\0';        
        ptTok = str_tok(NULL, "\\/");
    }

    u4CrcFilePathLen -= 1;
    ptCrcFilePath[u4CrcFilePathLen] = '\0';

    for (i = u4TokenLen; i > 0; i--)
    {
        if (cLastTok[i] == '.') 
        {
            dotCnt ++;
        }
        #if H263_USE_SORENSON        
        if (dotCnt > 1) //Sorenson H.263 has 2 part file extension
        #else
        if (dotCnt > 0) // MPEG4/DIVX/H.263/DIVX311
        #endif
        {
            break;
        }
    }

    if (i > 0)
    {
        u4CrcFilePathLen = u4CrcFilePathLen - u4TokenLen + i;
    }
    ptCrcFilePath[u4CrcFilePathLen] = '/'; //'\\';
    u4CrcFilePathLen ++;
    memcpy(ptCrcFilePath + u4CrcFilePathLen, cLastTok, i);
    u4CrcFilePathLen += i;
    memset(ptCrcFilePath + u4CrcFilePathLen, 0, 300 - u4CrcFilePathLen);
}

#ifdef MPEG4_CRC_CMP
void vMPEG4CrcCmp(UINT32 u4InstID,UCHAR *ptAddr,UINT32 u4Size)
{
	UINT32 u4HW_Y_Result[4],u4HW_CbCr_Result[4],u4Golden;
	UINT32 u4FileNameLen;
	UINT32 i;
	UCHAR ucCbCrCrc[20] = "Ccrc.bin";
	UCHAR ucYCrc[20] = "Ycrc.bin";
	BOOL fgOpen = FALSE;
	BOOL fgCmpRet = TRUE;

	if(fgYCrcOpen == FALSE)
	{
		//u4FileNameLen = strlen(_bFileStr1[u4InstID][14]);
		//sprintf(_bFileStr1[u4InstID][14] + u4FileNameLen ,"%s","Ycrc.bin");
		//sprintf(_bFileStr1[u4InstID][15] + u4FileNameLen ,"%s","Ccrc.bin");

		u4FileNameLen = strlen(_bFileStr1[u4InstID][1]);
	    #if !H263_USE_SORENSON
		vMPEG4CrcGetFilePath(_bFileStr1[u4InstID][1], _bFileStr1[u4InstID][14]);
		#endif
		memcpy(_bFileStr1[u4InstID][15], _bFileStr1[u4InstID][14], 300);
		u4FileNameLen = strlen(_bFileStr1[u4InstID][14]);
	#if VDEC_FIELD_COMPACT
	    #if !H263_USE_SORENSON
		sprintf(_bFileStr1[u4InstID][14] + u4FileNameLen, "%s", "_fldcmp_Ycrc.bin");
		sprintf(_bFileStr1[u4InstID][15] + u4FileNameLen, "%s", "_fldcmp_Ccrc.bin");
		#else
        sprintf(_bFileStr1[u4InstID][14] + u4FileNameLen, "%s", "fldcmp_Ycrc.bin");
		sprintf(_bFileStr1[u4InstID][15] + u4FileNameLen, "%s", "fldcmp_Ccrc.bin");
		#endif
	#else
	    #if !H263_USE_SORENSON
        sprintf(_bFileStr1[u4InstID][14] + u4FileNameLen, "%s", "_prog_Ycrc.bin");
		sprintf(_bFileStr1[u4InstID][15] + u4FileNameLen, "%s", "_prog_Ccrc.bin");
		#else
		sprintf(_bFileStr1[u4InstID][14] + u4FileNameLen, "%s", "prog_Ycrc.bin");
		sprintf(_bFileStr1[u4InstID][15] + u4FileNameLen, "%s", "prog_Ccrc.bin");
		#endif
	#endif
		printk("<vdec>Y CRC file name = %s\n",_bFileStr1[u4InstID][14]);
		printk("<vdec>CbCr CRC file name = %s\n",_bFileStr1[u4InstID][15]);

		//Open Y crc file
        //_tInFileInfo[u4InstID].fgGetFileInfo = TRUE;
		_tInFileInfo[u4InstID].pucTargetAddr = _pucCRCYBuf[u4InstID];
		_tInFileInfo[u4InstID].u4TargetSz = CRC_SZ;
		_tInFileInfo[u4InstID].u4FileLength = 0;
		_tInFileInfo[u4InstID].u4FileOffset = 0;

		fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][14],"r+b", &_tInFileInfo[u4InstID]);//Load bitstream
		if(fgOpen == FALSE)
		{
			msleep(100);
			printk("<vdec> Open Y crc file error!\n");
		}
		else
		{
			fgYCrcOpen = TRUE;
		   printk("[VDEC-MPEG4] Opne Y crc file OK!\n");
		}

		_tInFileInfo[u4InstID].pucTargetAddr = _pucCRCCBCRBuf[u4InstID];
		_tInFileInfo[u4InstID].u4TargetSz = CRC_SZ;
		_tInFileInfo[u4InstID].u4FileLength = 0;
		_tInFileInfo[u4InstID].u4FileOffset = 0;

		fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][15],"r+b", &_tInFileInfo[u4InstID]);//Load bitstream
		if(fgOpen == FALSE)
		{
			printk("<vdec> Open cbcr crc file error!\n");
		}
		else
		{
			fgCbCrCrcOpen = TRUE;
		//	printk("[VDEC-MPEG4] Opne C crc file OK!\n");
		}

		u4FileTotalPicNum =_tInFileInfo[u4InstID].u4FileLength >> 4;

		//Add some code here
		if(fgYCrcOpen && fgCbCrCrcOpen)
		{
		}
		else
		{
		}
	}
		for(i = 0; i < 4; i++)
		{
			u4HW_Y_Result[i] = u4VDecReadCRC(u4InstID,(i + 2) <<2);
			u4Golden = (((UINT32 *)(_pucCRCYBuf[u4InstID]))[(u4FilePicNum << 2) + i]);
			if(u4HW_Y_Result[i] != u4Golden)
			{
				fgCmpRet = FALSE;
				printk("<vdec>Y CRC Compare failed!\n");
				printk("<vdec>HW = 0x%x,golden = 0x%x\n",u4HW_Y_Result[i],u4Golden);
			}
		}

		for(i = 0; i < 4; i++)
		{
			u4HW_CbCr_Result[i] = u4VDecReadCRC(u4InstID,(i + 6) <<2);
			u4Golden = (((UINT32 *)_pucCRCCBCRBuf[u4InstID])[(u4FilePicNum << 2) + i]);
			if(u4HW_CbCr_Result[i] != u4Golden)
			{
				fgCmpRet = FALSE;
				printk("<vdec>CBCR CRC Compare failed!\n");
				printk("<vdec>HW = 0x%x,golden = 0x%x\n",u4HW_CbCr_Result[i],u4Golden);
			}
		}

        if (fgCmpRet == FALSE)
    	{
            //VDecDumpMP4RegisterByYiFeng(0);
    	}

		
		if((u4FilePicNum < u4FileTotalPicNum) && fgCmpRet)
		{
			if(!(u4FilePicNum%10))
				printk("<vdec> Decode ok @ pic %d\n",u4FilePicNum);
			#ifndef VPMODE
			if(!_fgVerVopCoded0[u4InstID])
			#endif
			{
				u4FilePicNum++;
				_u4FileCnt[u4InstID]++;
			}

			if((u4FilePicNum == u4FileTotalPicNum) || (u4FilePicNum > _u4EndCompPicNum[u4InstID]))
			{
				printk("<vdec>OK ==>Compare complete total pic = %d, 1st error @ %u\n",u4FilePicNum, u4FirstErrFrm);
				printk("<vdec>OK bitstream = %s ok @ %d\n",_bFileStr1[u4InstID][1],u4FilePicNum);
				memset(&_tVerMpvDecPrm[u4InstID],0,sizeof(VDEC_INFO_DEC_PRM_T));
			#if defined(CAPTURE_ESA_LOG) && defined(CAPTURE_ALL_IN_ONE) && (!WRITE_ESA_PER_FRAME)
				fgWrData2PC(_pucESATotalBuf[u4InstID],_u4ESATotalLen[u4InstID],7,_ESAFileName[u4InstID]); 
			#endif
				u4FilePicNum = 0;
				u4FilePicCont_noVOP = 0;
				_u4FileCnt[u4InstID] = 0;
				fgYCrcOpen = FALSE;
				fgCbCrCrcOpen = FALSE;
				_u4VerBitCount[u4InstID] = 0xffffffff;
				u4FirstErrFrm = 0xFFFFFFFF;
			}
		}
		else
		{
		
			if(!fgCmpRet)
			{
				#ifndef VPMODE
				if(!_fgVerVopCoded0[u4InstID])
				#endif
				{
					u4FilePicNum++;
					_u4FileCnt[u4InstID]++;
				}
				if (u4FirstErrFrm == 0xFFFFFFFF)
				{
					u4FirstErrFrm = u4FilePicNum;
				}
				//vPrintDumpReg(u4InstID,0);
				//vPrintDumpReg(u4InstID,1);
				printk("<vdec>NG ==>Compare mismatch @ pic %d decode order %d\n",u4FilePicNum,u4FilePicCont_noVOP - 1);
				printk("<vdec>NG ==> %s,mismatch @ %d\n",_bFileStr1[u4InstID][1],u4FilePicNum);
				//if((u4FilePicNum == u4FileTotalPicNum) || (u4FilePicNum > _u4EndCompPicNum[u4InstID]))
				{
                #if defined(CAPTURE_ESA_LOG) && defined(CAPTURE_ALL_IN_ONE) && (!WRITE_ESA_PER_FRAME)
                    fgWrData2PC(_pucESATotalBuf[u4InstID],_u4ESATotalLen[u4InstID],7,_ESAFileName[u4InstID]); 
                #endif
					memset(&_tVerMpvDecPrm[u4InstID],0,sizeof(VDEC_INFO_DEC_PRM_T));            
					u4FilePicNum = 0;
					u4FilePicCont_noVOP = 0;
					_u4FileCnt[u4InstID] = 0;
					fgYCrcOpen = FALSE;
					fgCbCrCrcOpen = FALSE;
					_u4VerBitCount[u4InstID] = 0xffffffff;
					u4FirstErrFrm = 0xFFFFFFFF;
				}
			} 
			else
			{
            #if defined(CAPTURE_ESA_LOG) && defined(CAPTURE_ALL_IN_ONE) && (!WRITE_ESA_PER_FRAME)
                fgWrData2PC(_pucESATotalBuf[u4InstID],_u4ESATotalLen[u4InstID],7,_ESAFileName[u4InstID]); 
            #endif
				memset(&_tVerMpvDecPrm[u4InstID],0,sizeof(VDEC_INFO_DEC_PRM_T));
				u4FilePicNum = 0;
				u4FilePicCont_noVOP = 0;
				_u4FileCnt[u4InstID] = 0;
				fgYCrcOpen = FALSE;
				fgCbCrCrcOpen = FALSE;
				_u4VerBitCount[u4InstID] = 0xffffffff;
				u4FirstErrFrm = 0xFFFFFFFF;
			}
            
		}
		
	
}
#endif

BOOL fgCompWMVChkSumGolden(UINT32 u4InstID);
// *********************************************************************
// Function    : void vWMVWrData2PC(UINT32 u4InstID, BYTE *ptAddr, UINT32 u4Size)
// Description : Write the decoded data to PC for compare
// Parameter   : None
// Return      : None
// *********************************************************************
void vWMVWrData2PC(UINT32 u4InstID, UCHAR *ptAddr, UINT32 u4Size)
{
    #if 1 //#ifndef  WMV_PIX_COMP_IF_CRC_COMP_ERR
    #if VDEC_DDR3_SUPPORT
    UINT32 u4TxDDR3, u4DDR3Start;
    #endif
    #if ((!defined(COMP_HW_CHKSUM)) || defined(DOWN_SCALE_SUPPORT))
    UINT32 u4Cnt;
    UINT32 u4Width,u4Height;
    #if defined(DOWN_SCALE_SUPPORT)
    UINT32 u4DramPicSize = 0x1FE000;
    #endif //DOWN_SCALE_SUPPORT
    
    #ifndef DOWN_SCALE_SUPPORT               
    UINT32 u4NonSwapYBase, u4NonSwapCBase;   
    #endif               
    
    UINT32 u4YBase,u4CBase;
    #ifndef GOLDEN_128BIT_COMP 
    UINT32 u4BufferWidth,u4GoldenBufferWidth;
    UCHAR *pbDecBuf,*pbGoldenBuf;
    #endif
    #endif  

    UINT32 u4XPix,u4YPix;
    #ifdef GOLDEN_128BIT_COMP    
    UINT32 *u4DecBuf,*u4GoldenBuf;
    UINT32 u4DecData,u4GoldenData;
    UINT32 u4Ty0, u4Tx0, u4Ty1, u4Tx1, u4GoldenTx0, u4GoldenTx1;
    UINT32 u4X, u4Y, u4GoldenX;
    UINT32 mbw, mbh, i, j;
    UINT32 u4Start, u4GoldenStart;
    #endif
    #endif
    BOOL fgDecErr,fgOpen = TRUE;
    VDEC_INFO_WMV_PIC_PRM_T *prWMVPPS = &_rWMVPPS[u4InstID];
    char strMessage[256];

#ifdef WMV_CRCCHECK_ENABLE
    BOOL fgNeedCmpPixel = FALSE;
#endif

    strncpy(_tFileListRecInfo[u4InstID].bFileName, _FileList_Rec[u4InstID],sizeof(_FileList_Rec[u4InstID]));

    #ifdef REDEC  
    if(_u4FileCnt[u4InstID] == _u4ReDecPicNum[u4InstID] )
    {
        if(_u4ReDecNum[u4InstID] != 0)
        {
            _u4ReDecPicNum[u4InstID] = 0xFFFFFFFF;
            _u4ReDecCnt[u4InstID] = _u4ReDecNum[u4InstID];
            vVDecOutputDebugString("/n!!!!!!!!!!!!!! Re-Decode and Wait for debug !!!!!!!!!!!!!!!!\n");
        }
    }
    if(_u4ReDecCnt[u4InstID] > 0)
    {
        _u4ReDecCnt[u4InstID]--;
    }
    #endif

    fgDecErr = FALSE;

    #ifdef GEN_HW_CHKSUM
    #ifndef INTERGRATION_WITH_DEMUX
    vWrite2PC(u4InstID, 9, NULL);
    #endif
    #endif

    #ifndef INTERGRATION_WITH_DEMUX
    if(prWMVPPS->ucPicType != SKIPFRAME) 
    {
        
        #if ((defined(COMP_HW_CHKSUM) && (!defined(DOWN_SCALE_SUPPORT)) ) || defined(WMV_CRCCHECK_ENABLE))
        #ifdef DIRECT_DEC
        if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
        #endif    
        {
#ifndef WMV_PIX_COMP_IF_CRC_COMP_ERR
            if(!fgCompWMVChkSumGolden(u4InstID))
            {
                fgDecErr = TRUE;
                printk("Check sum comparison error\n");
            }
            else
            {
                fgDecErr = FALSE;
                // printk("Check sum comparison OK\n");
            }  
#endif            
        }
        #endif
        //#else         
#if defined(WMV_PIX_COMP_IF_CRC_COMP_ERR)
        if (TRUE == fgDecErr)
        {
          printk("CRC error, fgNeedCmpPixel:%d\n", fgNeedCmpPixel);
          fgDecErr = FALSE;
          fgNeedCmpPixel = TRUE;
        }
#endif  
        
#ifdef WMV_CRCCHECK_ENABLE
        if (TRUE == fgNeedCmpPixel)
#endif
        {
            printk("Compare pixel!!\n");
          
          // compare pixel by pixel
           #ifndef DOWN_SCALE_SUPPORT
            #if VMMU_SUPPORT
            u4NonSwapYBase = (UINT32)_pucDecWorkBuf[u4InstID] + 0x1000;
            u4NonSwapCBase = (UINT32)_pucDecCWorkBuf[u4InstID] + 0x1000;

            printk("[WMV] vWMVWrData2PC, 1, Y Base:0x%x, C Base:0x%x\n", u4NonSwapYBase, u4NonSwapCBase); 
            #else
            u4NonSwapYBase = (UINT32)_pucDecWorkBuf[u4InstID];
            u4NonSwapCBase = (UINT32)_pucDecCWorkBuf[u4InstID];
            #endif
           #endif
           
           if (_tVerMpvDecPrm[u4InstID].ucAddrSwapMode != 4)
           {               
               #ifdef DOWN_SCALE_SUPPORT
                    //vGenerateDownScalerGolden(u4InstID, (UINT32)_pucDecWorkBuf[u4InstID],(UINT32)(_pucDecCWorkBuf[u4InstID]),u4Size);
               #else            
               UINT32 u4AlignWidth, u4AlignHeight;               
               UINT32 u4AlignSize = 0;
                #if VMMU_SUPPORT
                u4NonSwapYBase = (UINT32)_pucDecWorkBuf[u4InstID] + 0x1000;
                u4NonSwapCBase = (UINT32)_pucDecCWorkBuf[u4InstID] + 0x1000;
                //u4NonSwapCBase = (UINT32)_pucDecCWorkBuf[u4InstID]+0x1000;

                printk("[WMV] vWMVWrData2PC, 2, Y Base:0x%x, C Base:0x%x\n", u4NonSwapYBase, u4NonSwapCBase); 
                #else               
                u4NonSwapYBase = (UINT32)_pucDecWorkBuf[u4InstID];
                u4NonSwapCBase = (UINT32)_pucDecCWorkBuf[u4InstID];
                #endif                
               
               if(_rWMVSPS[u4InstID].u4PicWidthDec > _tVerPic[u4InstID].u4W)
                    u4AlignWidth = _tVerPic[u4InstID].u4W;
                else
                    u4AlignWidth = _rWMVSPS[u4InstID].u4PicWidthDec;
                
                u4AlignWidth = (((u4AlignWidth +63) >>6) <<6); //Align to 4MB width
                
                if(_rWMVSPS[u4InstID].u4PicHeightDec > _tVerPic[u4InstID].u4H)
                    u4AlignHeight = _tVerPic[u4InstID].u4H;
                else
                   u4AlignHeight = _rWMVSPS[u4InstID].u4PicHeightDec;

                u4AlignHeight =  (((u4AlignHeight +31) >>5) <<5); 
                
                vMPEG_InvAddressSwap(u4InstID, 
                                     (BYTE*)_pucDecWorkBuf[u4InstID],  (BYTE*)_pucDecCWorkBuf[u4InstID], 
                                     (BYTE*)_pucDumpYBuf[u4InstID], (BYTE*) _pucDumpCBuf[u4InstID],
                                     u4AlignWidth,  u4AlignHeight, u4AlignSize,
                                     _tVerMpvDecPrm[u4InstID].ucAddrSwapMode);

                u4NonSwapYBase = (UINT32)_pucVDSCLBuf[u4InstID];
                u4NonSwapCBase = (UINT32)_pucVDSCLBuf[u4InstID] + 0x1FE000;
                memcpy((UCHAR*)u4NonSwapYBase, _pucDumpYBuf[u4InstID],(u4AlignWidth*u4AlignHeight) +u4AlignSize);
                memcpy((UCHAR*)u4NonSwapCBase, _pucDumpCBuf[u4InstID],(u4AlignWidth*u4AlignHeight/2) + u4AlignSize);
               #endif
           }

        ///TODO: HalFlushInvalidateDCache();
        // Y compare
        _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
        _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
        //_tFBufFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
        _tFBufFileInfo[u4InstID].u4TargetSz = PIC_Y_SZ;  
        _tFBufFileInfo[u4InstID].u4FileLength = 0;  
        // Y decoded data Compare
        vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], _u4FileCnt[u4InstID]);
        #ifdef EXT_COMPARE 
        _tFBufFileInfo[u4InstID].u4FileLength = (((_tVerPic[u4InstID].u4W + 15)>>4)<<4) *(((_tVerPic[u4InstID].u4H + 31)>>5)<<5);
        #else
        #ifdef DIRECT_DEC
        if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
        #endif    
        {
            #ifdef DOWN_SCALE_SUPPORT
            vGenerateDownScalerGolden(u4InstID, (UINT32)_pucDecWorkBuf[u4InstID],(UINT32)(_pucDecCWorkBuf[u4InstID]),u4Size);
            #else            
            fgOpenFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);               
            _u4GoldenYSize[u4InstID] = _tFBufFileInfo[u4InstID].u4RealGetBytes;
            #endif
        }
        #endif


        u4Cnt = 0; 
        #ifdef EXT_COMPARE 
        if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
        {
            vWrite2PC(u4InstID, 5, _pucDecWorkBuf[u4InstID]);
        }
        #else
        #if defined(DOWN_SCALE_SUPPORT)
        u4YBase = (UINT32)_pucVDSCLBuf[u4InstID];
        #ifndef GOLDEN_128BIT_COMP
        u4BufferWidth = ((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW + 15) >> 4) << 4;
        u4GoldenBufferWidth = ((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW  + 15) >> 4) << 4;
        #endif
        u4Width = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW;
        if((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD) ||(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD))
        {
            u4Height = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV + (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight*2);
        }
        else
        {
            u4Height = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV + _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight;
        }
        #else
            //Normal Decode
            //u4YBase = (UINT32)_pucDecWorkBuf[u4InstID];
            u4YBase = u4NonSwapYBase;
        #ifndef GOLDEN_128BIT_COMP
        u4BufferWidth = ((_rWMVSPS[u4InstID].u4PicWidthDec+ 15) >> 4) << 4;
        if(_i4CodecVersion[u4InstID] != VDEC_VC1)
        {
            u4GoldenBufferWidth = ((_tVerPic[u4InstID].u4W  + 15) >> 4) << 4;
        }
        else
        {
            u4GoldenBufferWidth = ((_rWMVSPS[u4InstID].u4MaxPicWidthSrc + 15) >> 4) << 4;
        }
        #endif
        if(_rWMVSPS[u4InstID].u4PicWidthDec > _tVerPic[u4InstID].u4W)
        {
            u4Width = _tVerPic[u4InstID].u4W;
        }
        else
        {
            u4Width = _rWMVSPS[u4InstID].u4PicWidthDec;
        }
        if(_rWMVSPS[u4InstID].u4PicHeightDec > _tVerPic[u4InstID].u4H)
        {
            u4Height = _tVerPic[u4InstID].u4H;
        }
        else
        {
            u4Height = _rWMVSPS[u4InstID].u4PicHeightDec;
        }
        #endif  
        ///TODO: HalFlushInvalidateDCache();
        #ifdef DIRECT_DEC
        if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
        #endif    
        {
            #ifdef GOLDEN_128BIT_COMP
            u4Tx0 = (u4Width >> 4);   // w/ 16
            u4Ty0 = (u4Height >> 5);  // h /32
            u4X = (u4Width & 0xF);    // w % 16
            u4Y = (u4Height & 0x1F);  // h%32
            u4Tx1 = (u4X==0)? u4Tx0 : (u4Tx0+1);
            u4Ty1 = (u4Y==0)? u4Ty0 : (u4Ty0+1);
            #if VDEC_DDR3_SUPPORT
            u4TxDDR3 = (((u4Width +15) >>4) + 3) /4 *4;
            #endif

            u4GoldenTx0 = (_tVerPic[u4InstID].u4W >> 4);   // w/ 16
            u4GoldenX = (_tVerPic[u4InstID].u4W & 0xF); 
            u4GoldenTx1 = (u4GoldenX==0)? u4GoldenTx0 : (u4GoldenTx0+1);

            for (mbh=0; mbh < u4Ty1; mbh++)
            {
                for (mbw=0; mbw < u4Tx1; mbw++)
                {
                    u4Start = (mbh*u4Tx1 + mbw) * (16*32);
                    #if VDEC_DDR3_SUPPORT
                    //u4DDR3Start = (mbh*u4GoldenTx1 + mbw) *(16*32);
                    u4DDR3Start = (mbh*u4TxDDR3 + mbw) * (16*32);   
                    #endif
                    u4GoldenStart = (mbh*u4GoldenTx1 + mbw) * (16*32);
                    u4GoldenBuf = (UINT32*) (((UINT32) (_pucDumpYBuf[u4InstID])) + u4GoldenStart);
                    u4DecBuf = (UINT32*) (u4YBase + u4Start);
                    #if VDEC_DDR3_SUPPORT
                    u4DecBuf = (UINT32*) (u4YBase + u4DDR3Start);
                    #endif

                    for(j=0; j < 32; j++)
                    {             
                        for(i=0; i < 16; i+=4)
                        {                   
                            if(((_rWMVPPS[u4InstID].ucFrameCodingMode == INTERLACEFIELD)&& ((j%2) == _rWMVPPS[u4InstID].i4CurrentField)) || (_rWMVPPS[u4InstID].ucFrameCodingMode != INTERLACEFIELD))
                            {
                                if(  (mbw == u4Tx0 && i >= u4X) || (mbh == u4Ty0 && j >= u4Y))
                                {
                                    //Do not compare
                                }
                                else
                                {
                                    if((mbw == u4Tx0)&& ((i+4) > u4X))
                                    {
                                        u4DecData = (((*(u4DecBuf)) << (8*(4-u4X))) >> (8*(4-u4X)));
                                        u4GoldenData = (((*(u4GoldenBuf)) << (8*(4-u4X))) >> (8*(4-u4X)));
                                    }
                                    else
                                    {
                                        u4DecData = (*(u4DecBuf));
                                        u4GoldenData = (*(u4GoldenBuf));
                                    }

                                    //if ((*(u4DecBuf)) != (*(u4GoldenBuf)))
                                    if (u4DecData != u4GoldenData)
                                    {
                                        u4Cnt ++;
                                        u4XPix = mbw * 16 + i;
                                        u4YPix = mbh * 32 + j;
                                        vVDecOutputDebugString("Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                        sprintf(strMessage,"Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                        vVDecOutputDebugString("Y Data Mismatch at [x= 0x%d, y=0x%d] = 0x%x, Golden = 0x%x !!! \n", u4XPix, u4YPix, (*u4DecBuf), (*u4GoldenBuf));
                                        sprintf(strMessage,"Y Data Mismatch at [x= 0x%d, y=0x%d] = 0x%x, Golden = 0x%x !!! \n", u4XPix, u4YPix, (*u4DecBuf), (*u4GoldenBuf));
                                        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                        //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                        fgDecErr = TRUE;
                                        printk("Golden address = %x.size = %x\n",_tFBufFileInfo[u4InstID].pucTargetAddr ,_u4GoldenYSize[u4InstID]);
                                        printk("HW address = %x.size = %x\n",u4NonSwapYBase,_u4GoldenYSize[u4InstID]);
                                        vVDEC_HAL_WMV_VDec_DumpReg(u4InstID, FALSE);  // mark by ginny
                                       // ASSERT(0);   
#if 0                                        
                                        vWrite2PC(u4InstID, 12, _pucDecWorkBuf[u4InstID]); // dump check sum
                                        vWrite2PC(u4InstID, 13,_pucVFifo[u4InstID] + _u4CurrPicStartAddr[u4InstID]);
                                        vWrite2PC(u4InstID, 1, _pucVFifo[u4InstID]);
                                        _tFBufFileInfo[u4InstID].u4FileLength = (((_tVerPic[u4InstID].u4W+ 15) >> 4) << 4) * (((_tVerPic[u4InstID].u4H+ 32) >> 5) << 5);
                                        vWrite2PC(u4InstID, 2, _pucDecWorkBuf[u4InstID]);
                                        #if defined(DOWN_SCALE_SUPPORT)
                                        _tFBufFileInfo[u4InstID].u4FileLength = (((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5);
                                        vWrite2PC(u4InstID, 5, _pucVDSCLBuf[u4InstID]);
                                        vWrite2PC(u4InstID, 14, _pucDumpYBuf[u4InstID]); //dump VDSCL golden
                                        vWrite2PC(u4InstID, 16, (UCHAR*)&_tVerMpvDecPrm[u4InstID].rDownScalerPrm); //dump VDSCL parameter
                                        #endif        
#endif                                        
                                        break;    
                                    }
                                }
                            }//end of if

                            u4GoldenBuf++;
                            u4DecBuf++;
                        }//End of i

                        if(fgDecErr == TRUE)
                        {
                            break;
                        }
                    }//End of j

                    if(fgDecErr == TRUE)
                    {
                        break;
                    }
                }

                if(fgDecErr == TRUE)
                {
                    break;
                }
            }
            #else
            for (u4XPix = 0; u4XPix < u4Width; u4XPix++)
            {
                for (u4YPix = 0; u4YPix < u4Height; u4YPix++)
                {
                    if(((_rWMVPPS[u4InstID].ucFrameCodingMode == INTERLACEFIELD)&& ((u4YPix%2) == _rWMVPPS[u4InstID].i4CurrentField)) || (_rWMVPPS[u4InstID].ucFrameCodingMode != INTERLACEFIELD))
                    {
                        // compare Y
                        #ifdef DOWN_SCALE_SUPPORT
                        pbDecBuf = (UCHAR*)u4CalculatePixelAddress_Y(u4YBase, u4XPix, u4YPix, u4BufferWidth, (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType == 0), 4);
                        pbGoldenBuf = (UCHAR*)u4CalculatePixelAddress_Y((UINT32)_pucDumpYBuf[u4InstID], u4XPix, u4YPix, u4GoldenBufferWidth, (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType == 0), 4);
                        #else
                        pbDecBuf = (UCHAR*)u4CalculatePixelAddress_Y(u4YBase, u4XPix, u4YPix, u4BufferWidth, 1, 4);
                        pbGoldenBuf = (UCHAR*)u4CalculatePixelAddress_Y((UINT32)_pucDumpYBuf[u4InstID], u4XPix, u4YPix, u4GoldenBufferWidth, 1, 3);
                        #endif
                        if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                        {
                            u4Cnt ++;
                            vVDecOutputDebugString("Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                            sprintf(strMessage,"Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                            vVDecOutputDebugString("Y Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                            sprintf(strMessage,"Y Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                            //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                            fgDecErr = TRUE;
                            vVDEC_HAL_WMV_VDec_DumpReg(u4InstID, FALSE);  // mark by ginny
                            //vDumpReg();  // mark by ginny
                            vWrite2PC(u4InstID, 12, _pucDecWorkBuf[u4InstID]); // dump check sum
                            vWrite2PC(u4InstID, 13,_pucVFifo[u4InstID] + _u4CurrPicStartAddr[u4InstID]);
                            vWrite2PC(u4InstID, 1, _pucVFifo[u4InstID]);
                            _tFBufFileInfo[u4InstID].u4FileLength = (((_tVerPic[u4InstID].u4W+ 15) >> 4) << 4) * (((_tVerPic[u4InstID].u4H+ 32) >> 5) << 5);
                            vWrite2PC(u4InstID, 2, _pucDecWorkBuf[u4InstID]);
                            #if defined(DOWN_SCALE_SUPPORT)
                            _tFBufFileInfo[u4InstID].u4FileLength = (((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5);
                            vWrite2PC(u4InstID, 5, _pucVDSCLBuf[u4InstID]);
                            vWrite2PC(u4InstID, 14, _pucDumpYBuf[u4InstID]); //dump VDSCL golden
                            vWrite2PC(u4InstID, 16, (UCHAR*)&_tVerMpvDecPrm[u4InstID].rDownScalerPrm); //dump VDSCL parameter
                            #endif        
                            break;    
                        }
                    }

                }
                if(fgDecErr == TRUE)
                {
                    break;
                }
            }
            #endif        
            //vVDecOutputDebugString("\nY Data Compare Over!!! Total bytes [0x%.8x] & error [%d]\n", _u4GoldenYSize[_u4VDecID], u4Cnt);
            }
        #endif

        // CbCr compare
         ///TODO: HalFlushInvalidateDCache();
        //if(!fgIsMonoPic(_u4VDecID))
        {
            // CbCr decoded data Compare
            _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
            _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpCBuf[u4InstID];
            //_tFBufFileInfo[u4InstID].u4TargetSz = GOLD_C_SZ;  
            _tFBufFileInfo[u4InstID].u4TargetSz = PIC_C_SZ;  
            _tFBufFileInfo[u4InstID].u4FileLength = 0;
            vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], _bFileAddStrC[0], _u4FileCnt[u4InstID]);
            #ifdef EXT_COMPARE 
            _tFBufFileInfo[u4InstID].u4FileLength = (((_tVerPic[u4InstID].u4W + 15)>>4)<<4) *(((_tVerPic[u4InstID].u4H + 31)>>5)<<5) >>1;
            #else
            #ifdef DIRECT_DEC
            if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
            #endif    
            {  
                #ifndef DOWN_SCALE_SUPPORT                
                   fgOpenFile(u4InstID, _bFileStr1[u4InstID][4],"r+b", &_tFBufFileInfo[u4InstID]);               
                _u4GoldenCSize[u4InstID] = _tFBufFileInfo[u4InstID].u4RealGetBytes;
                #endif
            }
            #endif    

            u4Cnt = 0; 
            #ifdef EXT_COMPARE  
            if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
            {
                vWrite2PC(u4InstID, 6, _pucDecCWorkBuf[u4InstID]);
            }
            #else    
            #if defined(DOWN_SCALE_SUPPORT)
            u4CBase = (UINT32)_pucVDSCLBuf[u4InstID] + u4DramPicSize;
            #else
                    //u4CBase = (UINT32)_pucDecCWorkBuf[u4InstID];
                    u4CBase = u4NonSwapCBase;
            #endif  
            ///TODO: HalFlushInvalidateDCache();
            #ifdef DIRECT_DEC
            if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
            #endif      
            {
                #ifdef GOLDEN_128BIT_COMP
                UINT32 u4WidthC = u4Width / 2;
                UINT32 u4HeightC = u4Height / 2;
                u4Tx0 = (u4WidthC >> 3);   // w/ 8
                u4Ty0 = (u4HeightC >> 4);  // h /16
                u4X = (u4WidthC & 0x7);    // w % 8
                u4Y = (u4HeightC & 0xF);  // h % 16
                u4Tx1 = (u4X==0)? u4Tx0 : (u4Tx0+1);
                u4Ty1 = (u4Y==0)? u4Ty0 : (u4Ty0+1);
                #if VDEC_DDR3_SUPPORT
                u4TxDDR3 = (((u4Width +15) >>4) + 3) /4 *4;
                #endif
                

                u4GoldenTx0 = ((_tVerPic[u4InstID].u4W >> 1) >> 3);   // w/ 16
                u4GoldenX = ((_tVerPic[u4InstID].u4W>>1) & 0x7); 
                u4GoldenTx1 = (u4GoldenX==0)? u4GoldenTx0 : (u4GoldenTx0+1);

                for (mbh=0; mbh < u4Ty1; mbh++)
                {
                    for (mbw=0; mbw < u4Tx1; mbw++)
                    {
                        u4Start = (mbh*u4Tx1 + mbw) * (16*16);
                        #if VDEC_DDR3_SUPPORT
                        u4DDR3Start = (mbh*u4TxDDR3 + mbw) * (16*16);
                        //u4DDR3Start = (mbh*u4GoldenTx1 + mbw) *(16*16);
                        #endif
                        u4GoldenStart = (mbh*u4GoldenTx1 + mbw) * (16*16);
                        u4GoldenBuf = (UINT32*) (((UINT32) (_pucDumpCBuf[u4InstID])) + u4GoldenStart);
                        u4DecBuf = (UINT32*) (u4CBase + u4Start);
                        #if VDEC_DDR3_SUPPORT
                        u4DecBuf = (UINT32*) (u4CBase + u4DDR3Start);
                        #endif
                        for(j=0; j < 16; j++)
                        {
                            for(i=0; i < 16; i+=4)
                            {                       
                                #if defined(DOWN_SCALE_SUPPORT)
                                if(((_rWMVPPS[u4InstID].ucFrameCodingMode == INTERLACEFIELD)&& (((j >> (1 - _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt))%2) == _rWMVPPS[u4InstID].i4CurrentField)) || (_rWMVPPS[u4InstID].ucFrameCodingMode != INTERLACEFIELD))
                                #else
                                if(((_rWMVPPS[u4InstID].ucFrameCodingMode == INTERLACEFIELD)&& ((j%2) == _rWMVPPS[u4InstID].i4CurrentField)) || (_rWMVPPS[u4InstID].ucFrameCodingMode != INTERLACEFIELD))
                                #endif
                                {
                                    if(  (mbw == u4Tx0 && (i/2) >= u4X) || (mbh == u4Ty0 && j >= u4Y))
                                    {
                                        //Do not compare
                                        u4GoldenBuf++;
                                        u4DecBuf++;
                                    }
                                    else
                                    {
                                        if((mbw == u4Tx0)&& (((i+4)/2) > u4X))
                                        {
                                            u4DecData = (((*(u4DecBuf)) << (8*(4-(2*u4X)))) >> (8*(4-(2*u4X))));
                                            u4GoldenData = (((*(u4GoldenBuf)) << (8*(4-(2*u4X)))) >> (8*(4-(2*u4X))));
                                        }
                                        else
                                        {
                                            u4DecData = (*(u4DecBuf));
                                            u4GoldenData = (*(u4GoldenBuf));
                                        }
                                        //Compare CbCr
                                        //if ((*(u4DecBuf)) != (*(u4GoldenBuf)))
                                        if (u4DecData != u4GoldenData)
                                        {
                                            u4XPix = mbw * 8 + (i/2);
                                            u4YPix = mbh * 16 + j;
                                            u4Cnt ++;
                                            vVDecOutputDebugString("Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                            sprintf(strMessage,"Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                            vVDecOutputDebugString("CbCr Data Mismatch at [x= 0x%d, y=0x%d] = 0x%x, Golden = 0x%x !!! \n", u4XPix, u4YPix, (*u4DecBuf), (*u4GoldenBuf));
                                            sprintf(strMessage,"CbCr Data Mismatch at [x= 0x%d, y=0x%d] = 0x%x, Golden = 0x%x !!! \n", u4XPix, u4YPix, (*u4DecBuf), (*u4GoldenBuf));
                                            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                            //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                            fgDecErr = TRUE;
                                            //vDumpReg();  // mark by ginny
                                            vVDEC_HAL_WMV_VDec_DumpReg(u4InstID, FALSE);  // mark by ginny
#if 0                                            
                                            vWrite2PC(u4InstID, 12, _pucDecCWorkBuf[u4InstID]); // dump check sum
                                            _tFBufFileInfo[u4InstID].u4FileLength = ((((_tVerPic[u4InstID].u4W+ 15) >> 4) << 4) * (((_tVerPic[u4InstID].u4H+ 32) >> 5) << 5)) / 2;
                                            vWrite2PC(u4InstID, 3, _pucDecCWorkBuf[u4InstID]);
                                            vWrite2PC(u4InstID, 1, _pucVFifo[u4InstID]);
                                            #if defined(DOWN_SCALE_SUPPORT)
                                            _tFBufFileInfo[u4InstID].u4FileLength = ((((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5))/(2 - _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt);
                                            vWrite2PC(u4InstID, 6, _pucVDSCLBuf[u4InstID] + u4Size);
                                            vWrite2PC(u4InstID, 15, _pucDumpCBuf[u4InstID]); //dump VDSCL golden
                                            vWrite2PC(u4InstID, 16, (UCHAR*)&_tVerMpvDecPrm[u4InstID].rDownScalerPrm); //dump VDSCL parameter
                                            #endif        
#endif                                            
                                            break;    
                                        }

                                        u4GoldenBuf++;
                                        u4DecBuf++;
                                    }
                                }
                                else
                                {
                                    u4GoldenBuf++;
                                    u4DecBuf++;
                                }

                            }
                            if(fgDecErr == TRUE)
                            {
                                break;
                            }
                        }

                        if(fgDecErr == TRUE)
                        {
                            break;
                        }
                    }

                    if(fgDecErr == TRUE)
                    {
                        break;
                    }
                }
                #else

                for (u4XPix = 0; u4XPix < u4Width; u4XPix++)
                {
                    for (u4YPix = 0; u4YPix < u4Height; u4YPix++)
                    {
                        #if defined(DOWN_SCALE_SUPPORT)
                        if(((_rWMVPPS[u4InstID].ucFrameCodingMode == INTERLACEFIELD)&& (((u4YPix >> (1 - _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt))%2) == _rWMVPPS[u4InstID].i4CurrentField)) || (_rWMVPPS[u4InstID].ucFrameCodingMode != INTERLACEFIELD))
                        #else
                        if(((_rWMVPPS[u4InstID].ucFrameCodingMode == INTERLACEFIELD)&& (((u4YPix >> 1)%2) == _rWMVPPS[u4InstID].i4CurrentField)) || (_rWMVPPS[u4InstID].ucFrameCodingMode != INTERLACEFIELD))
                        #endif
                        {
                            // compare Cb
                            #ifdef DOWN_SCALE_SUPPORT
                            pbDecBuf = (UCHAR*)u4CalculatePixelAddress_C(u4CBase, u4XPix, u4YPix, u4BufferWidth, (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType == 0), (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt == 0), 4);
                            pbGoldenBuf = (UCHAR*)u4CalculatePixelAddress_C(((UINT32)_pucDumpCBuf[u4InstID]), u4XPix, u4YPix, u4GoldenBufferWidth, (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType == 0), (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt == 0), 4);
                            #else
                            pbDecBuf = (UCHAR*)u4CalculatePixelAddress_C(u4CBase, u4XPix, u4YPix, u4BufferWidth, 1, 1, 4);
                            pbGoldenBuf = (UCHAR*)u4CalculatePixelAddress_C((UINT32)_pucDumpCBuf[u4InstID], u4XPix, u4YPix, u4GoldenBufferWidth, 1, 1, 3);
                            #endif
                            if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                            {
                                u4Cnt ++;
                                vVDecOutputDebugString("Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                sprintf(strMessage,"Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                vVDecOutputDebugString("Cb Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                sprintf(strMessage,"Cb Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                fgDecErr = TRUE;
                                //vDumpReg();  // mark by ginny
                                vVDEC_HAL_WMV_VDec_DumpReg(u4InstID, FALSE);  // mark by ginny
                                vWrite2PC(u4InstID, 12, _pucDecCWorkBuf[u4InstID]); // dump check sum
                                _tFBufFileInfo[u4InstID].u4FileLength = ((((_tVerPic[u4InstID].u4W+ 15) >> 4) << 4) * (((_tVerPic[u4InstID].u4H+ 32) >> 5) << 5)) / 2;
                                vWrite2PC(u4InstID, 3, _pucDecCWorkBuf[u4InstID]);
                                vWrite2PC(u4InstID, 1, _pucVFifo[u4InstID]);
                                #if defined(DOWN_SCALE_SUPPORT)
                                _tFBufFileInfo[u4InstID].u4FileLength = ((((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5))/(2 - _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt);
                                vWrite2PC(u4InstID, 6, _pucVDSCLBuf[u4InstID] + u4Size);
                                vWrite2PC(u4InstID, 15, _pucDumpCBuf[u4InstID]); //dump VDSCL golden
                                vWrite2PC(u4InstID, 16, (UCHAR*)&_tVerMpvDecPrm[u4InstID].rDownScalerPrm); //dump VDSCL parameter
                                #endif        
                                break;    
                            }
                            // compare Cr
                            pbDecBuf++;
                            pbGoldenBuf++;
                            if ((*(pbDecBuf)) != (*(pbGoldenBuf)))
                            {
                                u4Cnt ++;
                                vVDecOutputDebugString("Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                sprintf(strMessage,"Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                vVDecOutputDebugString("Cr Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                sprintf(strMessage,"Cr Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pbDecBuf), (*pbGoldenBuf));
                                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                fgDecErr = TRUE;
                                //vDumpReg();  // mark by ginny
                                vVDEC_HAL_WMV_VDec_DumpReg(u4InstID, FALSE);  // mark by ginny
                                vWrite2PC(u4InstID, 12, _pucDecCWorkBuf[u4InstID]); // dump check sum
                                _tFBufFileInfo[u4InstID].u4FileLength = ((((_tVerPic[u4InstID].u4W+ 15) >> 4) << 4) * (((_tVerPic[u4InstID].u4H+ 32) >> 5) << 5))/2;
                                vWrite2PC(u4InstID, 3, _pucDecCWorkBuf[u4InstID]);
                                #if defined(DOWN_SCALE_SUPPORT)
                                _tFBufFileInfo[u4InstID].u4FileLength = ((((u4Width+ 15) >> 4) << 4) * (((u4Height+ 32) >> 5) << 5))/(2 - _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt);
                                vWrite2PC(u4InstID, 6, _pucVDSCLBuf[u4InstID] + u4Size);
                                vWrite2PC(u4InstID, 15, _pucDumpCBuf[u4InstID]); //dump VDSCL golden
                                vWrite2PC(u4InstID, 16, (UCHAR*)&_tVerMpvDecPrm[u4InstID].rDownScalerPrm); //dump VDSCL parameter
                                #endif        
                                break;    
                            }
                        }

                    }
                    if(fgDecErr == TRUE)
                    {
                        break;
                    }
                }
                #endif          
                //vVDecOutputDebugString("CbCr Data Compare Over!!! Total bytes [0x%.8x] & error [%d]\n", _u4GoldenCSize[_u4VDecID], u4Cnt);
            }
            #endif    
        }
        printk("Compare pixel done!!\n");
        }
        //#endif

        #ifdef WMV_CRC_COMPOSITE_CHECK_ENABLE
        _u4CRCCnt[u4InstID]++;
        #endif
    }

    #ifndef IDE_WRITE_SUPPORT
    if((_u4FileCnt[u4InstID]%10) == 0)
    #endif
    {
        #ifndef IDE_WRITE_SUPPORT
        vVDecOutputDebugString("Pic count to [%d]\n", _u4FileCnt[u4InstID]);  
        #endif
        sprintf(strMessage,"[%d], ", _u4FileCnt[u4InstID]);  
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
    }

    #if 0 // temp for debug
    if ((124 < _u4FileCnt[u4InstID]) && (128 > _u4FileCnt[u4InstID]))
    {
      printk("Pic count to [%d]\n", _u4FileCnt[u4InstID]);  
      
      vVDEC_HAL_WMV_VDec_DumpReg(u4InstID, FALSE);
    }
    #endif
    #endif

    if(((_rWMVPPS[u4InstID].ucFrameCodingMode == INTERLACEFIELD) && (_rWMVPPS[u4InstID].i4CurrentTemporalField == 1)) || (_rWMVPPS[u4InstID].ucFrameCodingMode != INTERLACEFIELD))
    {
        #ifdef REDEC    
        if(_u4ReDecCnt[u4InstID] == 0)
        #endif    
        _u4FileCnt[u4InstID] ++;
    }

#ifndef INTERGRATION_WITH_DEMUX
    // Check if still pic needed compare
#if (defined(COMP_HW_CHKSUM) && (!defined(DOWN_SCALE_SUPPORT)))
    _tTempFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tTempFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tTempFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tTempFileInfo[u4InstID].u4FileLength = 0; 
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_chksum.bin\0", _u4FileCnt[u4InstID]);
#ifdef IDE_READ_SUPPORT
    fgOpen = fgPureOpenIdeFile( _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
#else
    fgOpen = fgOpenFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
#endif
#else
    _tFBufFileInfo[u4InstID].fgGetFileInfo = FALSE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tFBufFileInfo[u4InstID].u4FileLength = 4;      
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], _u4FileCnt[u4InstID]);
    if (fgNeedCmpPixel) {
#ifdef IDE_READ_SUPPORT
        fgOpen = fgPureOpenIdeFile( _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);
#else
        fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);
#endif
    }
    printk("fgOpen %d, open file %s\n", fgOpen, _bFileStr1[u4InstID][3]);
#endif
    printk("_u4WMVByteCount[u4InstID] %d, _tInFileInfo[u4InstID].u4FileLength %d, _tInFileInfo[u4InstID].bFileName %s\n", _u4WMVByteCount[u4InstID], _tInFileInfo[u4InstID].u4FileLength, _tInFileInfo[u4InstID].bFileName);
#ifndef RING_VFIFO_SUPPORT
    if((_u4WMVByteCount[u4InstID] + 4) > _tInFileInfo[u4InstID].u4FileLength)
    {
        fgOpen = FALSE;
    }
#endif
    printk("fgOpen %d, fgDecErr %d, _fgVDecErr[u4InstID] %d\n", fgOpen, fgDecErr, _fgVDecErr[u4InstID]);
#ifndef WMV_CRC_COMPOSITE_CHECK_ENABLE // BD case to skip first thress
    if (fgDecErr == TRUE && (_u4FileCnt[u4InstID] - 1) < 3) {
        printk("ingore first three frame error for BD case (%d)", _u4FileCnt[u4InstID] - 1);
        fgDecErr = FALSE;
    }
#endif
    if((fgOpen == FALSE) ||(fgDecErr == TRUE) || (_fgVDecErr[u4InstID] == TRUE))
    {
        sprintf(strMessage, "%s", "\n");
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        //fprintf(_tFileListRecInfo.fpFile, "\n");  
        // break decode
        if(fgOpen == FALSE)
        {
#if defined(CAPTURE_ESA_LOG) && defined(CAPTURE_ALL_IN_ONE) && (!WRITE_ESA_PER_FRAME)
            //fgWrData2PC(_pucESATotalBuf[u4InstID],_u4ESATotalLen[u4InstID],7,_ESAFileName[u4InstID]); 
#endif
            msleep(3000);
			printk(" Compare Finish  ==> %s Pic count to [%d] \n", _bFileStr1[u4InstID][1], _u4FileCnt[u4InstID] - 1);   
            msleep(3000);
            sprintf(strMessage," Compare Finish  ==>%s Pic count to [%d] \n", _bFileStr1[u4InstID][1], _u4FileCnt[u4InstID] - 1);   
            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
            //fprintf(_tFileListRecInfo.fpFile, " Compare Finish==> Pic count to [%d] \n", _u4FileCnt[_u4VDecID] - 1);   
            if(_u4FileCnt[u4InstID] == 1)
            {
                if(fgOpen == FALSE)
                {
                    vVDecOutputDebugString("real NULL\n");
                }
            }
        }
        else
        {
            msleep(3000);
			printk(" Error ==>%s Pic count to [%d] \n", _bFileStr1[u4InstID][0], _u4FileCnt[u4InstID] - 1);     
            msleep(3000);
            sprintf(strMessage," Error ==>%s Pic count to [%d] \n", _bFileStr1[u4InstID][0], _u4FileCnt[u4InstID] - 1);     
            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
           // ASSERT(0);
            //fprintf(_tFileListRecInfo.fpFile, " Error ==> Pic count to [%d] \n", _u4FileCnt[_u4VDecID] - 1);         
        }
        _u4VerBitCount[u4InstID] = 0xffffffff;
    }
    #endif
    if(_u4FileCnt[u4InstID] >= _u4EndCompPicNum[u4InstID])
    {
        _u4VerBitCount[u4InstID] = 0xffffffff;
    }
}


INT32 CharToInt(UCHAR c)
{
    INT32 v = 0;
    v = c - '0';
    return v;
}


#ifdef LETTERBOX_SUPPORT
void vLBDParaParsing(UINT32 u4InstID)
{
     UINT32 u4Temp;

    u4Temp = 0;
    do
    {
        u4Temp = u4Temp*10;
        u4Temp += CharToInt((*_pcLBDSettingFile[u4InstID]));
        _pcLBDSettingFile[u4InstID]++;
    }while(( (*_pcLBDSettingFile[u4InstID]) != ' ') && ( (*_pcLBDSettingFile[u4InstID]) != '\n'));
    _pcLBDSettingFile[u4InstID]++;
    _rLBDPrm[u4InstID].u4UpBoundStart = u4Temp;

    u4Temp = 0;
    do
    {
        u4Temp = u4Temp*10;
        u4Temp += CharToInt((*_pcLBDSettingFile[u4InstID]));
        _pcLBDSettingFile[u4InstID]++;
    }while(( (*_pcLBDSettingFile[u4InstID]) != ' ') && ( (*_pcLBDSettingFile[u4InstID]) != '\n'));
    _pcLBDSettingFile[u4InstID]++;
    _rLBDPrm[u4InstID].u4UpBoundEnd = u4Temp;

    u4Temp = 0;
    do
    {
        u4Temp = u4Temp*10;
        u4Temp += CharToInt((*_pcLBDSettingFile[u4InstID]));
        _pcLBDSettingFile[u4InstID]++;
    }while(( (*_pcLBDSettingFile[u4InstID]) != ' ') && ( (*_pcLBDSettingFile[u4InstID]) != '\n'));
    _pcLBDSettingFile[u4InstID]++;
    _rLBDPrm[u4InstID].u4LowBoundEnd = u4Temp;

    u4Temp = 0;
    do
    {
        u4Temp = u4Temp*10;
        u4Temp += CharToInt((*_pcLBDSettingFile[u4InstID]));
        _pcLBDSettingFile[u4InstID]++;
    }while(( (*_pcLBDSettingFile[u4InstID]) != ' ') && ( (*_pcLBDSettingFile[u4InstID]) != '\n'));
    _pcLBDSettingFile[u4InstID]++;
    _rLBDPrm[u4InstID].u4LowBoundStart = u4Temp;

    u4Temp = 0;
    do
    {
        u4Temp = u4Temp*10;
        u4Temp += CharToInt((*_pcLBDSettingFile[u4InstID]));
        _pcLBDSettingFile[u4InstID]++;
    }while(( (*_pcLBDSettingFile[u4InstID]) != ' ') && ( (*_pcLBDSettingFile[u4InstID]) != '\n'));
    _pcLBDSettingFile[u4InstID]++;
    _rLBDPrm[u4InstID].u4LeftBound = u4Temp;

    u4Temp = 0;
    do
    {
        u4Temp = u4Temp*10;
        u4Temp += CharToInt((*_pcLBDSettingFile[u4InstID]));
        _pcLBDSettingFile[u4InstID]++;
    }while(( (*_pcLBDSettingFile[u4InstID]) != ' ') && ( (*_pcLBDSettingFile[u4InstID]) != '\n'));
    _pcLBDSettingFile[u4InstID]++;
    _rLBDPrm[u4InstID].u4RightBound = u4Temp;
    
    u4Temp = 0;
    do
    {
        u4Temp = u4Temp*10;
        u4Temp += CharToInt((*_pcLBDSettingFile[u4InstID]));
        _pcLBDSettingFile[u4InstID]++;
    }while(( (*_pcLBDSettingFile[u4InstID]) != ' ') && ( (*_pcLBDSettingFile[u4InstID]) != '\n'));
    _pcLBDSettingFile[u4InstID]++;
    _rLBDPrm[u4InstID].u4YValueThd = u4Temp;

    u4Temp = 0;
    do
    {
        u4Temp = u4Temp*10;
        u4Temp += CharToInt((*_pcLBDSettingFile[u4InstID]));
        _pcLBDSettingFile[u4InstID]++;
    }while(( (*_pcLBDSettingFile[u4InstID]) != ' ') && ( (*_pcLBDSettingFile[u4InstID]) != '\n'));
    _pcLBDSettingFile[u4InstID]++;
    _rLBDPrm[u4InstID].u4CValueThd = u4Temp;

    u4Temp = 0;
    do
    {
        u4Temp = u4Temp*10;
        u4Temp += CharToInt((*_pcLBDSettingFile[u4InstID]));
        _pcLBDSettingFile[u4InstID]++;
    }while(( (*_pcLBDSettingFile[u4InstID]) != ' ') && ( (*_pcLBDSettingFile[u4InstID]) != '\n'));
    _pcLBDSettingFile[u4InstID]++;
    _rLBDPrm[u4InstID].u4YCntThd = u4Temp;

    u4Temp = 0;
    do
    {
        u4Temp = u4Temp*10;
        u4Temp += CharToInt((*_pcLBDSettingFile[u4InstID]));
        _pcLBDSettingFile[u4InstID]++;
    }while(( (*_pcLBDSettingFile[u4InstID]) != '\r') && ( (*_pcLBDSettingFile[u4InstID]) != '\n'));
    _pcLBDSettingFile[u4InstID]++;
    _pcLBDSettingFile[u4InstID]++;
    _rLBDPrm[u4InstID].u4CCntThd = u4Temp;

    _rLBDPrm[u4InstID].u4YOffset = 0;
    _rLBDPrm[u4InstID].u4COffset = 0x80;
}


void vCheckLBDResult(UINT32 u4InstID)
{
    UINT32 u4YResult, u4CResult;
    UINT32 u4YUpBound, u4YLowBound, u4CUpBound, u4CLowBound;
    UINT32 u4YUpBoundGod, u4YLowBoundGod, u4CUpBoundGod, u4CLowBoundGod;
    char strMessage[256];
    
    vVDECReadLetetrBoxDetResult(u4InstID, &u4YResult, &u4CResult);
    u4YUpBound = (u4YResult&0xFFF);
    u4YLowBound = ((u4YResult >> 16)&0xFFF);
    u4CUpBound = (u4CResult&0xFFF);
    u4CLowBound = ((u4CResult >> 16)&0xFFF);
    printk("%d %d %d %d\n", u4YUpBound, u4YLowBound, u4CUpBound, u4CLowBound);

    if(_u4CodecVer[u4InstID] == VDEC_H264)
    {
        u4YUpBoundGod = 0;
        do
        {
            u4YUpBoundGod = u4YUpBoundGod*10;
            u4YUpBoundGod += CharToInt((*_pcLBDGoldenFile[u4InstID]));
            _pcLBDGoldenFile[u4InstID]++;
        }while(( (*_pcLBDGoldenFile[u4InstID]) != ' ') && ( (*_pcLBDGoldenFile[u4InstID]) != '\n'));
        _pcLBDGoldenFile[u4InstID]++;
         
        u4YLowBoundGod = 0;
        do
        {
            u4YLowBoundGod = u4YLowBoundGod*10;
            u4YLowBoundGod += CharToInt((*_pcLBDGoldenFile[u4InstID]));
            _pcLBDGoldenFile[u4InstID]++;
        }while(( (*_pcLBDGoldenFile[u4InstID]) != ' ') && ( (*_pcLBDGoldenFile[u4InstID]) != '\n'));
         _pcLBDGoldenFile[u4InstID]++;
    
        u4CUpBoundGod = 0;
        do
        {
            u4CUpBoundGod = u4CUpBoundGod*10;
            u4CUpBoundGod += CharToInt((*_pcLBDGoldenFile[u4InstID]));
            _pcLBDGoldenFile[u4InstID]++;
        }while(( (*_pcLBDGoldenFile[u4InstID]) != ' ') && ( (*_pcLBDGoldenFile[u4InstID]) != '\n'));
         _pcLBDGoldenFile[u4InstID]++;
    
        u4CLowBoundGod = 0;
        do
        {
            u4CLowBoundGod = u4CLowBoundGod*10;
            u4CLowBoundGod += CharToInt((*_pcLBDGoldenFile[u4InstID]));
            _pcLBDGoldenFile[u4InstID]++;
        }while(( (*_pcLBDGoldenFile[u4InstID]) != '\r') && ( (*_pcLBDGoldenFile[u4InstID]) != '\n'));
         _pcLBDGoldenFile[u4InstID]++;
         _pcLBDGoldenFile[u4InstID]++;
    
         if ((u4YUpBound != u4YUpBoundGod) ||(u4YLowBound != u4YLowBoundGod) 
              ||(u4CUpBound != u4CUpBoundGod) ||(u4CLowBound != u4CLowBoundGod))
         {
             printk("LetterBox Compara Mismatch @ PicCnt=%d\n",_u4FileCnt[u4InstID]);
         }
    
         if ( (*_pcLBDGoldenFile[u4InstID]) == '@')
         {
            sprintf(strMessage," Compare Finish==> Pic count to [%d] \n", _u4FileCnt[u4InstID] - 1);   
            printk("%s\n",strMessage);
            if(_u4FileCnt[u4InstID] != 1) //flush DPB buffer
            {
                if(fgIsDecFlagSet(u4InstID, DEC_FLAG_CHG_FBUF))
                {
                    _ptCurrFBufInfo[u4InstID]->eH264DpbStatus = H264_DPB_STATUS_DECODED;
                    _ptCurrFBufInfo[u4InstID]->u4DecOrder = _u4TotalDecFrms[u4InstID];
                }
                vFlushDPB(u4InstID, &_tVerMpvDecPrm[u4InstID], FALSE);
            }
            _u4VerBitCount[u4InstID] = 0xffffffff;
        }
    
        _ptCurrFBufInfo[u4InstID]->u4FrameCnt = _u4FileCnt[u4InstID];
        if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
        {
            #ifdef REDEC    
            if(_u4ReDecCnt[u4InstID] == 0)
            #endif    
            {
                _u4FileCnt[u4InstID] ++;
                vSetDecFlag(u4InstID, DEC_FLAG_CHG_FBUF);
            }
        }
        //else
        {
            _ucPrevDecFld[u4InstID] = _ptCurrFBufInfo[u4InstID]->ucFBufStatus;
        }
    }
    else if(_u4CodecVer[u4InstID] == VDEC_WMV)
    {
        if(((_rWMVPPS[u4InstID].ucFrameCodingMode == INTERLACEFIELD) && (_rWMVPPS[u4InstID].i4CurrentTemporalField == 1)) || (_rWMVPPS[u4InstID].ucFrameCodingMode != INTERLACEFIELD))
        {
            #ifdef REDEC    
            if(_u4ReDecCnt[u4InstID] == 0)
            #endif    
            _u4FileCnt[u4InstID] ++;
        }

        if((_u4WMVByteCount[u4InstID] + 4) > _tInFileInfo[u4InstID].u4FileLength)
        {
            sprintf(strMessage," Compare Finish==> Pic count to [%d] \n", _u4FileCnt[u4InstID] - 1);   
            printk("%s\n",strMessage);
            _u4VerBitCount[u4InstID] = 0xffffffff;
        }
        if(_u4FileCnt[u4InstID] >= 13)
        {
            _u4VerBitCount[u4InstID] = 0xffffffff;
        }
        if(_u4FileCnt[u4InstID] >= _u4EndCompPicNum[u4InstID])
        {
            _u4VerBitCount[u4InstID] = 0xffffffff;
        }
    }
    else if(_u4CodecVer[u4InstID] == VDEC_MPEG4)
    {
        if(!_fgVerVopCoded0[u4InstID])
        {
            if((_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecMPEGDecPrm.fgDec2ndFld) || (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRM_PIC))
            {
                #ifdef REDEC    
                if(_u4ReDecCnt[u4InstID] == 0)
                #endif    
                _u4FileCnt[u4InstID] ++;
            }
        }
        if(_u4FileCnt[u4InstID] >= 100)
        {
            _u4VerBitCount[u4InstID] = 0xffffffff;
        }
        if(_u4FileCnt[u4InstID] >= _u4EndCompPicNum[u4InstID])
        {
            _u4VerBitCount[u4InstID] = 0xffffffff;
        }
    }
}
#endif

#if HEVC_CRC_COMPARE

#define H265CRCSIZE 102400 //100k
UINT32 _u4H265Cnt = 0;
UINT32 _u4H265FileTotalPicNum = 0;
BOOL _fgH265YCrcOpen[2] = {0};

BOOL vH265_CheckCRCResult(UINT32 u4InstID)
{
    UINT32 u4FileNameLen = 0;
    UINT32 i = 0;
    UINT32 u4HW_Y_Result[4] = {0};
    UINT32 u4HW_CbCr_Result[4] = {0};
    UINT32 u4Golden = 0;
    UINT32 u4GoldenOffset = 4 ;
    char file_name[200] = {0};
    char bitstream_name[200] = {0};
    BOOL fgOpen = FALSE;
    BOOL fgCmpRet = TRUE;
    BOOL fgDumpY = FALSE;
    BOOL fgDumpC = FALSE;

    if (_fgH265YCrcOpen[u4InstID] == FALSE)
    {
        strncpy (bitstream_name , _bFileStr1[u4InstID][1], (strlen(_bFileStr1[u4InstID][1]) -21));

        // Load Y-CRC to DRAM
        if (0) //(_u4DualCoreEnable[u4InstID] == 1)
        {
            if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            //if(0)
            {
                sprintf(file_name, "%scrc_ufo_mcore%d_Y.dat", bitstream_name, 0);           
            }
            else // w/o UFO
            {
                sprintf(file_name, "%scrc_ufo_bypass_mcore%d_Y.dat", bitstream_name, 0);   
            }
            
            printk("//  [ %s ]Y0 CRC file = %s\n", __FUNCTION__, file_name);
            _tInCRCFileInfo[u4InstID].fgGetFileInfo = TRUE;
            _tInCRCFileInfo[u4InstID].pucTargetAddr = _pucH265CRCYBuf0[u4InstID];
            _tInCRCFileInfo[u4InstID].u4TargetSz = H265CRCSIZE;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;

            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);

            if (fgOpen == FALSE)
            {
                printk("// [H265] open Y0 CRC file fail!!\n");
                return TRUE;
            }
        
            memset(file_name, 0, 200);
            if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            //if(0)
            {
                sprintf(file_name, "%scrc_ufo_mcore%d_Y.dat", bitstream_name, 1);
            }
            else
            {                
                sprintf(file_name, "%scrc_ufo_bypass_mcore%d_Y.dat", bitstream_name, 1);
            }
            printk("// Y1 CRC file = %s\n", file_name);
            
            _tInCRCFileInfo[u4InstID].pucTargetAddr = _pucH265CRCYBuf1[u4InstID];
            _tInCRCFileInfo[u4InstID].u4TargetSz = H265CRCSIZE;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;

            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);

            if (fgOpen == FALSE)
            {
                printk("// [H265] open Y1 CRC file fail!!\n");
                return TRUE;
            }
        }
        else // single-core
        {
            if (_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            {
                sprintf(file_name, "%scrc_ufo_single_Y.dat", bitstream_name);           
            }
            else // w/o UFO
            {
                sprintf(file_name, "%scrc_ufo_bypass_single_Y.dat", bitstream_name);   
            }
            
            printk("// Y CRC file = %s\n", file_name);
            _tInCRCFileInfo[u4InstID].fgGetFileInfo = TRUE;
            _tInCRCFileInfo[u4InstID].pucTargetAddr = _pucH265CRCYBuf0[u4InstID];
            _tInCRCFileInfo[u4InstID].u4TargetSz = H265CRCSIZE;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;

            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);

            if (fgOpen == FALSE)
            {
                printk("// [H265] open Y CRC file fail!!\n");
                return TRUE;
            }
        }

        // Load C-CRC to DRAM
        memset(file_name, 0, 200);

        if (0) //(_u4DualCoreEnable[u4InstID] == 1)
        {
            if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            //if(0)
            {
                sprintf(file_name, "%scrc_ufo_mcore%d_C.dat", bitstream_name, 0);
            }
            else
            {
                sprintf(file_name, "%scrc_ufo_bypass_mcore%d_C.dat", bitstream_name, 0);                
            }
            printk("// CRC0 file = %s\n", file_name);
            
            _tInCRCFileInfo[u4InstID].pucTargetAddr = _pucH265CRCCBuf0[u4InstID];
            _tInCRCFileInfo[u4InstID].u4TargetSz = H265CRCSIZE;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;
            
            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);
            
            if (fgOpen == FALSE)
            {
                printk("// [H265] open C0 CRC file fail!!\n");
                return TRUE;
            }

            memset(file_name, 0, 200);
            
            if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            //if(0)
            {
                sprintf(file_name, "%scrc_ufo_mcore%d_C.dat", bitstream_name, 1);
            }
            else
            {
                sprintf(file_name, "%scrc_ufo_bypass_mcore%d_C.dat", bitstream_name, 1);
            }
            printk("// CRC1 file = %s\n", file_name);
            
            _tInCRCFileInfo[u4InstID].pucTargetAddr = _pucH265CRCCBuf1[u4InstID];
            _tInCRCFileInfo[u4InstID].u4TargetSz = H265CRCSIZE;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;

            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);
        
            if (fgOpen == FALSE)
            {
                printk("// [H265] open C1 CRC file fail!!\n");
                return TRUE;
            }            
        }
        else // single-core
        {
            if (_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            {
                sprintf(file_name, "%scrc_ufo_single_C.dat", bitstream_name);
            }
            else
            {
                sprintf(file_name, "%scrc_ufo_bypass_single_C.dat", bitstream_name);                
            }
            printk("// CRC file = %s\n", file_name);
            
            _tInCRCFileInfo[u4InstID].pucTargetAddr = _pucH265CRCCBuf0[u4InstID];
            _tInCRCFileInfo[u4InstID].u4TargetSz = H265CRCSIZE;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;
            
            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);
            
            if (fgOpen == FALSE)
            {
                printk("// [H265] open C CRC file fail!!\n");
                return TRUE;
            }       
        }

        // one picture need 4 set crc, per crc contain 4 bytes, (total 16-byte per picture needed to be compared) 
        // pictureNum = filesize >> 4
        _u4H265FileTotalPicNum = _tInCRCFileInfo[u4InstID].u4FileLength >> 4;
        _fgH265YCrcOpen[u4InstID] = TRUE;        
    }

    /*
    Core0 (single) Y: mcore_top_193 ~ 196
    Core0 (single) C: mcore_top_197 ~ 200
    Core1 Y:          mcore_top_201 ~ 204
    Core1 C:          mcore_top_205 ~ 208
    10-bit the same
    */

    // check Y CRC
    for (i = 0; i < 4; i++)
    {
        #if 0
        if (_u4DualCoreEnable[u4InstID] == 1)
        {
            //if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            {                
                u4HW_Y_Result[i] = u4VDecReadMCORETOP(u4InstID, (193+i) << 2 );
            }
            #if 0
            else
            {
                u4HW_Y_Result[i] = u4VDecReadCRC(u4InstID, (i+2)<<2);
            }
            #endif
            
            // core 0 compare
            u4Golden = ((UINT32*)_pucH265CRCYBuf0[u4InstID])[_u4H265Cnt*u4GoldenOffset + i];            
            if (u4HW_Y_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpY = TRUE;
                printk("// [H265] Y CRC compare fail!!\n");
                printk("// [H265][Core-%d] i:%d, HW: 0x%x, Golden: 0x%x\n", 0, i, u4HW_Y_Result[i], u4Golden);
            }

            //if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            {
                u4HW_Y_Result[i] = u4VDecReadMCORETOP(u4InstID, (201+i) << 2 );
            }
            #if 0
            else
            {
                u4HW_Y_Result[i] = u4VDecReadCRC(1, (i+2)<<2);         
            }
            #endif
            
            u4Golden = ((UINT32*)_pucH265CRCYBuf1[u4InstID])[_u4H265Cnt*u4GoldenOffset + i];
            // core 1 compare
            if (u4HW_Y_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpY = TRUE;
                printk("// [H265] Y CRC compare fail!!\n");
                printk("// [H265][Core-%d] i:%d, HW: 0x%x, Golden: 0x%x\n", 1, i, u4HW_Y_Result[i], u4Golden);
            }                 
        }
        else // single core
        #endif
        {
            //if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            { 
                #if 1 //HUB MT8137 HEVC
                u4HW_Y_Result[i] = u4VDecReadHEVCMISC(u4InstID, (95 + i) << 2);
                //printk("CRC Y:0x%x ", u4HW_Y_Result[i]);
                #else
                u4HW_Y_Result[i] = u4VDecReadMCORETOP(u4InstID, (193+i) << 2 );
                #endif
            }
            #if 0
            else
            {
                u4HW_Y_Result[i] = u4VDecReadCRC(u4InstID, (i+2)<<2);
            }
            #endif

            u4Golden = ((UINT32*)_pucH265CRCYBuf0[u4InstID])[_u4H265Cnt*u4GoldenOffset + i];            
            if (u4HW_Y_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpY = TRUE;
                printk("// [H265] Y CRC compare fail!!\n");
                printk("// [H265] i:%d, HW: 0x%x, Golden: 0x%x\n", i, u4HW_Y_Result[i], u4Golden);
            }
            else
            {
                printk("// [H265] Y CRC compare ok!!\n");
            }
        }       
    }
    //printk("\n");
    
#ifndef SATA_HDD_READ_SUPPORT
#if 1
    // dump dram
    if (fgDumpY)
    {
        vH265DumpMem( u4InstID, _ptH265CurrFBufInfo[u4InstID]->u4YStartAddr, _ptH265CurrFBufInfo[u4InstID]->u4DramYSize,
                     _u4H265Cnt, 1, 0 );
    }
#endif

#if 0
    // dump dram
    if( fgDumpY )
    {
        vH265DumpMem( u4InstID, _ptH265FBufInfo[u4InstID][0].u4YStartAddr, 0x7D00000,
                     _u4H265Cnt, 1, 0 );
    }
#endif
#endif       

    // check C CRC
    for (i = 0; i < 4; i++)
    {
        #if 0
        if (_u4DualCoreEnable[u4InstID] == 1)
        {
            //if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            {
                u4HW_CbCr_Result[i] = u4VDecReadMCORETOP(u4InstID, (197+i) << 2 );
            }
            #if 0
            else
            {
                u4HW_CbCr_Result[i] = u4VDecReadCRC(u4InstID, (i+6)<<2);   
            }
            #endif
            
            u4Golden = ((UINT32*)_pucH265CRCCBuf0[u4InstID])[_u4H265Cnt*u4GoldenOffset + i];
            if (u4HW_CbCr_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpC = TRUE;
                printk("// [H265] CbCr CRC compare fail!!\n");
                printk("// [H265][Core-%d] i:%d, HW: 0x%x, Golden: 0x%x\n", 0, i, u4HW_CbCr_Result[i], u4Golden);
            }
            
            //if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            {
                u4HW_CbCr_Result[i] = u4VDecReadMCORETOP(u4InstID, (205+i) << 2 );
            }
            #if 0
            else
            {
                u4HW_CbCr_Result[i] = u4VDecReadCRC(1, (i+6)<<2);  
            }
            #endif
            
            u4Golden = ((UINT32*)_pucH265CRCCBuf1[u4InstID])[_u4H265Cnt*u4GoldenOffset + i];                      
            if (u4HW_CbCr_Result[i] != u4Golden)
            {
                  fgCmpRet = FALSE;
                  fgDumpC = TRUE;
                  printk("// [H265] CbCr CRC compare fail!!\n");
                  printk("// [H265][Core-%d] i:%d, HW: 0x%x, Golden: 0x%x\n", 1, i, u4HW_CbCr_Result[i], u4Golden);
            }
        }
        else // single core
        #endif
        {   
            //if(_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
            {
                #if 1 //HUB MT8137 HEVC
                u4HW_CbCr_Result[i] = u4VDecReadHEVCMISC(u4InstID, (99 + i) << 2);
                //printk("CRC C:0x%x ", u4HW_CbCr_Result[i]);
                #else
                u4HW_CbCr_Result[i] = u4VDecReadMCORETOP(u4InstID, (197+i) << 2 );
                #endif
            }
            #if 0
            else
            {
                u4HW_CbCr_Result[i] = u4VDecReadCRC(u4InstID, (i+6)<<2);   
            }
            #endif

            u4Golden = ((UINT32*)_pucH265CRCCBuf0[u4InstID])[_u4H265Cnt*u4GoldenOffset + i];
            if (u4HW_CbCr_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpC = TRUE;
                printk("// [H265] CbCr CRC compare fail!!\n");
                printk("// [H265] i:%d, HW: 0x%x, Golden: 0x%x\n", i, u4HW_CbCr_Result[i], u4Golden);
            }
            else
            {
                printk("// [H265] CbCr CRC compare ok!!\n");
            }
        }
    } 

#ifndef SATA_HDD_READ_SUPPORT       
#if 1
    // dump dram
    if (fgDumpC)
    {
        vH265DumpMem( u4InstID, _ptH265CurrFBufInfo[u4InstID]->u4CStartAddr, _ptH265CurrFBufInfo[u4InstID]->u4DramYSize >> 1,
                     _u4H265Cnt, 2, 0 );
    }
#endif

#if 0
    if( fgDumpC )
    {

        vH265DumpMem( u4InstID, _ptH265FBufInfo[u4InstID][0].u4YStartAddr, 0x7D00000,
                     _u4H265Cnt,2, 0 );
    }
#endif
#endif

    if ((_u4H265Cnt < _u4H265FileTotalPicNum) && fgCmpRet)
    {
        //if (!(_u4H265Cnt % 10))
        {
            //printk("[H265] Decode ok @ frame %d\n", _u4H265Cnt);
        }
        printk("// [H265] Decode ok @ frame %d\n", _u4H265Cnt);
        _u4H265Cnt++;        

        if (_u4H265Cnt == _u4H265FileTotalPicNum)        
        {
            printk("// [H265] OK [%d]===> compare complete! total frame: %d\n", u4InstID, _u4H265Cnt);
            printk("// [H265] OK [%d] bitstream: %s ok @ %d\n", u4InstID, _bFileStr1[u4InstID][1], _u4H265Cnt);
            _u4H265Cnt = 0;
            _fgH265YCrcOpen[u4InstID] = FALSE;
            _u4VerBitCount[u4InstID] = 0xFFFFFFFF;
        }
		#if 0
        else if(_u4H265Cnt == 800) 
        {
            _u4H265Cnt = 0;
            _fgH265YCrcOpen[u4InstID] = FALSE;
            _u4VerBitCount[u4InstID] = 0xFFFFFFFF; 
        }
		#endif
        return FALSE;
    }
    else
    {
        if (!fgCmpRet)
        {
            printk("// [H265] [%d] mismatch ===> compare mismatch @ frame: %d\n", u4InstID, _u4H265Cnt);
            printk("// [H265] [%d] NG bitstream: %s \n", u4InstID,_bFileStr1[u4InstID][1]);
        }        
        _u4H265Cnt = 0;
        _fgH265YCrcOpen[u4InstID] = FALSE;
        _u4VerBitCount[u4InstID] = 0xFFFFFFFF;
        return TRUE;
    } 
}
#endif


// *********************************************************************
// Function    : BOOL vH264_CheckCRCResult(UINT32 u4InstID, UINT32 u4Size)
// Description : Write the decoded data to PC for compare
// Parameter   : None
// Return      : None
// *********************************************************************
#if AVC_NEW_CRC_COMPARE
#define H264CRCSIZE 409600 //400k
UINT32 u4H264Cnt = 0;
UINT32 u4H264FileTotalPicNum = 0;
BOOL fgH264YCrcOpen = FALSE;

#define LEN_OF_FILE_NAME 1024
char _output_file_name[LEN_OF_FILE_NAME];
#define _FANTASIA_WRITE_OUT_DATA_ 0
extern UINT32 u4BitStreamLengthH264;

void vReadH264ChkSumGolden(UINT32 u4InstID);  //


BOOL vH264_CheckCRCResult(UINT32 u4InstID)
{
  UINT32 u4FileNameLen = 0;
  UINT32 i = 0;
  UINT32 u4HW_Y_Result[4] = {0};
  UINT32 u4HW_CbCr_Result[4] = {0};
  UINT32 u4Golden = 0;
  UINT32 u4GoldenOffset = 8;
  UINT32 u4BackupSize;
  BOOL fgOpen = FALSE;
  BOOL fgCmpRet = TRUE;
  msleep(10);
#if VDEC_MVC_SUPPORT
  printk("MVC compare %s\n",(u4InstID?"DEP":"BASE"));
#endif

  if (fgH264YCrcOpen == FALSE)
  {
    u4FileNameLen = strlen(_bFileStr1[u4InstID][1]);
	sprintf(_bFileStr1[u4InstID][16],"%s",_bFileStr1[u4InstID][1]);
    sprintf(_bFileStr1[u4InstID][16] + (u4FileNameLen - 4), "%s", ".crc");
    printk("CRC file = %s\n", _bFileStr1[u4InstID][16]);

    u4BackupSize = _tInFileInfo[u4InstID].u4FileLength;
    // open Y CRC file
    _tInFileInfo[u4InstID].pucTargetAddr = _pucH264CRCYBuf[u4InstID];
    _tInFileInfo[u4InstID].u4TargetSz = H264CRCSIZE;
    _tInFileInfo[u4InstID].u4FileLength = 0;
    _tInFileInfo[u4InstID].u4FileOffset = 0;

    fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][16], "r+b", &_tInFileInfo[u4InstID]);

    if (fgOpen == FALSE)
    {
      printk("[H264] open Y CRC file fail!!\n");
      _tInFileInfo[u4InstID].u4FileLength = u4BackupSize;
      //return FALSE;
    }
    else
    {
      #if VDEC_MVC_SUPPORT
	  _pucH264CRCYBuf[1] = _pucH264CRCYBuf[0];
      #endif
      u4H264FileTotalPicNum = _tInFileInfo[u4InstID].u4FileLength >> 5;
      fgH264YCrcOpen = TRUE;
    }
  }
  else {
    fgOpen = TRUE;
  }

  printk("CRC_Y: \n");
  for (i = 0; i < 4; i++)
  {
    u4HW_Y_Result[i] = u4VDecReadCRC(u4InstID, (i+2)<<2);
    u4Golden = ((UINT32*)_pucH264CRCYBuf[u4InstID])[u4H264Cnt*u4GoldenOffset + i];

    if (u4HW_Y_Result[i] != u4Golden && fgOpen == TRUE)
    {
      fgCmpRet = FALSE;
      printk("[H264] Y CRC compare fail!!\n");
      DBG_H264_PRINTF("[H264] Y CRC compare fail!!\n");
      printk("[H264] i:%d, HW: 0x%x, Golden: 0x%x\n", i, u4HW_Y_Result[i], u4Golden);
      DBG_H264_PRINTF("[H264] i:%d, HW: 0x%x, Golden: 0x%x\n", i, u4HW_Y_Result[i], u4Golden);
    }
    printk("%08X ", u4HW_Y_Result[i]);
  }
  printk("\n");
  printk("CRC_C: \n");
  for (i = 0; i < 4; i++)
  {
    u4HW_CbCr_Result[i] = u4VDecReadCRC(u4InstID, (i+6)<<2);
    u4Golden = ((UINT32*)_pucH264CRCYBuf[u4InstID])[u4H264Cnt*u4GoldenOffset + 4 + i];

    if (u4HW_CbCr_Result[i] != u4Golden && fgOpen == TRUE)
    {
      fgCmpRet = FALSE;
      printk("[H264] CbCr CRC compare fail!!\n");
      DBG_H264_PRINTF("[H264] CbCr CRC compare fail!!\n");
      printk("[H264] i:%d, HW: 0x%x, Golden: 0x%x\n", i, u4HW_CbCr_Result[i], u4Golden);
      DBG_H264_PRINTF("[H264] i:%d, HW: 0x%x, Golden: 0x%x\n", i, u4HW_CbCr_Result[i], u4Golden);
    }
    printk("%08X ", u4HW_CbCr_Result[i]);
  }
  printk("\n");
  if (fgOpen == FALSE)
  {
    printk("[H264] open Y CRC file fail!!\n");
    _tInFileInfo[u4InstID].u4FileLength = u4BackupSize;
    return FALSE;
  }
  
   _ptCurrFBufInfo[u4InstID]->u4FrameCnt = _u4FileCnt[u4InstID];
   if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
   {
       #ifdef REDEC    
       if(_u4ReDecCnt[u4InstID] == 0)
       #endif    
       {
           _u4FileCnt[u4InstID] ++;
           vSetDecFlag(u4InstID, DEC_FLAG_CHG_FBUF);
       }
   }
   //else
   {
       _ucPrevDecFld[u4InstID] = _ptCurrFBufInfo[u4InstID]->ucFBufStatus;
   }

  if ((u4H264Cnt < u4H264FileTotalPicNum) && fgCmpRet)
  {
    //if (!(u4H264Cnt % 10))//fantasia >> let every frame print out a message
      printk("[H264] Decode ok @ frame %d\n", u4H264Cnt);
      DBG_H264_PRINTF("[H264] Decode ok @ frame %d\n", u4H264Cnt);

    u4H264Cnt++;

    if (u4H264Cnt == u4H264FileTotalPicNum)
    {
      msleep(1000);
      printk("\n[H264] OK ===> compare complete! total frame: %d\n", u4H264Cnt);
      DBG_H264_PRINTF("[H264] OK ===> compare complete! total frame: %d\n", u4H264Cnt);
      printk("[H264] OK bitstream: %s ok @ %d\n", _bFileStr1[u4InstID][1], u4H264Cnt);
      DBG_H264_PRINTF("[H264] OK bitstream: %s ok @ %d\n", _bFileStr1[u4InstID][1], u4H264Cnt);
      msleep(1000);
      u4H264Cnt = 0;
      fgH264YCrcOpen = FALSE;
	  #if VDEC_MVC_SUPPORT
	  for(i = 0; i < 2; i++)
	  {
	    _u4VerBitCount[i] = 0xFFFFFFFF;
	  	if(fgIsDecFlagSet(i, DEC_FLAG_CHG_FBUF))
        {
           _ptCurrFBufInfo[i]->eH264DpbStatus = H264_DPB_STATUS_DECODED;
           _ptCurrFBufInfo[i]->u4DecOrder = _u4TotalDecFrms[i];
        }
        vFlushDPB(i, &_tVerMpvDecPrm[i], FALSE);
	  }
	  #else
      _u4VerBitCount[u4InstID] = 0xFFFFFFFF;
      if(fgIsDecFlagSet(u4InstID, DEC_FLAG_CHG_FBUF))
      {
        _ptCurrFBufInfo[u4InstID]->eH264DpbStatus = H264_DPB_STATUS_DECODED;
        _ptCurrFBufInfo[u4InstID]->u4DecOrder = _u4TotalDecFrms[u4InstID];
      }
        //ESA H264 BEGIN
        #if defined(CAPTURE_ESA_LOG) && defined(CAPTURE_ALL_IN_ONE) && (!WRITE_ESA_PER_FRAME)
            fgWrData2PC(_pucESATotalBuf[u4InstID],_u4ESATotalLen[u4InstID],7,_ESAFileName[u4InstID]); 
        #endif
        _u4VerBitCount[u4InstID] = 0xffffffff;
        // ESA H264 END
      vFlushDPB(u4InstID, &_tVerMpvDecPrm[u4InstID], FALSE);
	  #endif
    }

    return TRUE;
  }
  else
  {
    if (!fgCmpRet)
    {
      msleep(1000);
      #if VDEC_MVC_SUPPORT
	  printk("CRC Error @ picture %d\n",u4H264Cnt);
	  #else
      printk("[H264] NG ===> compare mismatch @ frame: %d\n", u4H264Cnt);
      DBG_H264_PRINTF("[H264] NG ===> compare mismatch @ frame: %d\n", u4H264Cnt);
      printk("[H264] NG bitstream: %s mismatch @ %d\n", _bFileStr1[u4InstID][1], u4H264Cnt);
      DBG_H264_PRINTF("[H264] NG bitstream: %s mismatch @ %d\n", _bFileStr1[u4InstID][1], u4H264Cnt);
      msleep(1000);

      // dump check sum -- fantasia
      printk("[H264] dump check sum @ pic %d\n", u4H264Cnt);
      DBG_H264_PRINTF("[H264] dump check sum @ pic %d\n", u4H264Cnt);
      vReadH264ChkSumGolden(u4InstID);
      
      #ifdef VDEC_SIM_DUMP  //dump raw data
      printk("[H264] DPB buffers = 0x%x (0x%x), size %d\n", _pucDPB[u4InstID], PHYSICAL(_pucDPB[u4InstID]), DPB_SZ);
      DBG_H264_PRINTF("[H264] DPB buffers = 0x%x (0x%x), size %d\n", _pucDPB[u4InstID], PHYSICAL(_pucDPB[u4InstID]), DPB_SZ);
      printk("[H264] MV buffers = 0x%x (0x%x), size %d\n", _pucAVCMVBuff_Main[0], PHYSICAL(_pucAVCMVBuff_Main[0]), MVBUF_SZ);
      DBG_H264_PRINTF("[H264] MV buffers = 0x%x (0x%x), size %d\n", _pucAVCMVBuff_Main[0], PHYSICAL(_pucAVCMVBuff_Main[0]), MVBUF_SZ);
      printk("[H264] Bit-stream = 0x%x (0x%x), size %d\n", _pucVFifo[u4InstID], PHYSICAL(_pucVFifo[u4InstID]),u4BitStreamLengthH264);
      DBG_H264_PRINTF("[H264] Bit-stream = 0x%x (0x%x), size %d\n", _pucVFifo[u4InstID], PHYSICAL(_pucVFifo[u4InstID]),u4BitStreamLengthH264);
      sprintf(_output_file_name, "D:\\sandbox\\DPB_0x%x.raw", PHYSICAL(_pucDPB[u4InstID]));
      printk("[H264] DPB buffers => %s\n", _output_file_name);
      DBG_H264_PRINTF("[H264] DPB buffers => %s\n", _output_file_name);
      #if _FANTASIA_WRITE_OUT_DATA_
      fgWrData2PC((void*)_pucDPB[u4InstID],DPB_SZ,7,_output_file_name);
      #endif
      sprintf(_output_file_name, "D:\\sandbox\\AVCMV_0x%x.raw", PHYSICAL(_pucAVCMVBuff_Main[0]));      
      printk("[H264] DPB buffers => %s\n", _output_file_name);
      DBG_H264_PRINTF("[H264] DPB buffers => %s\n", _output_file_name);
      #if _FANTASIA_WRITE_OUT_DATA_
      fgWrData2PC((void*)_pucAVCMVBuff_Main[0],MVBUF_SZ,7,_output_file_name);
      #endif
      sprintf(_output_file_name, "D:\\sandbox\\BitStream_0x%x.raw", PHYSICAL(_pucVFifo[u4InstID]));
      printk("[H264] DPB buffers => %s\n", _output_file_name);
      DBG_H264_PRINTF("[H264] DPB buffers => %s\n", _output_file_name);
      #if _FANTASIA_WRITE_OUT_DATA_
      fgWrData2PC((void*)_pucVFifo[u4InstID],u4BitStreamLengthH264,7,_output_file_name);
      #endif
      #endif
	  #endif
    }
    _u4FileCnt[u4InstID]--;
    #if VDEC_MVC_SUPPORT
    #else
    u4H264Cnt = 0;
    fgH264YCrcOpen = FALSE;
    _u4VerBitCount[u4InstID] = 0xFFFFFFFF;
     #endif

    return FALSE;
  }  
}
#else
BOOL vH264_CheckCRCResult(UINT32 u4InstID)
{
//read
//REG_2: y_crc_checksum[31:0]
//REG_3: y_crc_checksum[63:32]
//REG_4: y_crc_checksum[95:64]
//REG_5: y_crc_checksum[127:96]

//REG_6: c_crc_checksum[31:0]
//REG_7: c_crc_checksum[63:32]
//REG_8: c_crc_checksum[95:64]
//REG_9: c_crc_checksum[127:96]


  //UINT32* pu4CRCResultCurrAddr;
  //UINT32 u4CRCResult = 0;
  //UINT32 i=0;
  //UINT32 u4SWResult;
    UINT32 u4HWCRC_Y0, u4HWCRC_Y1, u4HWCRC_Y2, u4HWCRC_Y3;
    UINT32 u4HWCRC_C0, u4HWCRC_C1, u4HWCRC_C2, u4HWCRC_C3;

    CHAR _bFileNameCRC[20] = {"_CRC.out\0"};
    CHAR _bFileNameCRCF[20] = {"_CRC_F.out\0"};
    CHAR _bFileNameCRCT[20] = {"_CRC_T.out\0"};
    CHAR _bFileNameCRCB[20] = {"_CRC_B.out\0"};
    //UINT32 u4Temp;
    UINT32 u4CRCValueY0, u4CRCValueY1, u4CRCValueY2, u4CRCValueY3;
    UINT32 u4CRCValueC0, u4CRCValueC1, u4CRCValueC2, u4CRCValueC3;
    UINT32 u4CRCTmp3, u4CRCTmp2, u4CRCTmp1, u4CRCTmp0;
    BOOL fgDecErr = FALSE;
    BOOL fgCRCPass = TRUE;
    BOOL fgOpen;
    //INT32 i, j;
    UCHAR strMessage [ 256];

    if (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRAME)
    {
       strncpy(_bFileNameCRC, _bFileNameCRCF, sizeof(_bFileNameCRCF));
    }
    else 
    if (_tVerMpvDecPrm[u4InstID].ucPicStruct == TOP_FIELD)   
    {
       strncpy(_bFileNameCRC, _bFileNameCRCT, sizeof(_bFileNameCRCT));
    }
    else
    if (_tVerMpvDecPrm[u4InstID].ucPicStruct == BOTTOM_FIELD)   
    {
       strncpy(_bFileNameCRC, _bFileNameCRCB, sizeof(_bFileNameCRCB));
    }
    else
    {
    	VDEC_ASSERT(0);
    }

    // Y compare
    _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = _pucCRCBuf[u4InstID];
    _tFBufFileInfo[u4InstID].u4TargetSz = 32;  
    _tFBufFileInfo[u4InstID].u4FileLength = 0;  
    // Y decoded data Compare
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileNameCRC, _u4FileCnt[u4InstID]);    
    fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);         
    if (fgOpen == FALSE)
    {
        sprintf(strMessage,"CRC File Open Fail!!!!\n");   
        printk("%s",strMessage);
        return FALSE;
    }
    

    u4CRCTmp3 = (_pucCRCBuf[u4InstID][3] );
    u4CRCTmp2 = (_pucCRCBuf[u4InstID][2] );
    u4CRCTmp1 = (_pucCRCBuf[u4InstID][1] );
    u4CRCTmp0 = (_pucCRCBuf[u4InstID][0] );
    u4CRCValueY0 = (u4CRCTmp3 << 24) | (u4CRCTmp2 << 16) |  (u4CRCTmp1 << 8) |  (u4CRCTmp0 << 0) ;

    u4CRCTmp3 = (_pucCRCBuf[u4InstID][7] );
    u4CRCTmp2 = (_pucCRCBuf[u4InstID][6] );
    u4CRCTmp1 = (_pucCRCBuf[u4InstID][5] );
    u4CRCTmp0 = (_pucCRCBuf[u4InstID][4] );
    u4CRCValueY1 = (u4CRCTmp3 << 24) | (u4CRCTmp2 << 16) |  (u4CRCTmp1 << 8) |  (u4CRCTmp0 << 0) ;

    u4CRCTmp3 = (_pucCRCBuf[u4InstID][11] );
    u4CRCTmp2 = (_pucCRCBuf[u4InstID][10] );
    u4CRCTmp1 = (_pucCRCBuf[u4InstID][9] );
    u4CRCTmp0 = (_pucCRCBuf[u4InstID][8] );
    u4CRCValueY2 = (u4CRCTmp3 << 24) | (u4CRCTmp2 << 16) |  (u4CRCTmp1 << 8) |  (u4CRCTmp0 << 0) ;

    u4CRCTmp3 = (_pucCRCBuf[u4InstID][15] );
    u4CRCTmp2 = (_pucCRCBuf[u4InstID][14] );
    u4CRCTmp1 = (_pucCRCBuf[u4InstID][13] );
    u4CRCTmp0 = (_pucCRCBuf[u4InstID][12] );
    u4CRCValueY3 = (u4CRCTmp3 << 24) | (u4CRCTmp2 << 16) |  (u4CRCTmp1 << 8) |  (u4CRCTmp0 << 0) ;

    u4CRCTmp3 = (_pucCRCBuf[u4InstID][19] );
    u4CRCTmp2 = (_pucCRCBuf[u4InstID][18] );
    u4CRCTmp1 = (_pucCRCBuf[u4InstID][17] );
    u4CRCTmp0 = (_pucCRCBuf[u4InstID][16] );
    u4CRCValueC0 = (u4CRCTmp3 << 24) | (u4CRCTmp2 << 16) |  (u4CRCTmp1 << 8) |  (u4CRCTmp0 << 0) ;

    u4CRCTmp3 = (_pucCRCBuf[u4InstID][23] );
    u4CRCTmp2 = (_pucCRCBuf[u4InstID][22] );
    u4CRCTmp1 = (_pucCRCBuf[u4InstID][21] );
    u4CRCTmp0 = (_pucCRCBuf[u4InstID][20] );
    u4CRCValueC1 = (u4CRCTmp3 << 24) | (u4CRCTmp2 << 16) |  (u4CRCTmp1 << 8) |  (u4CRCTmp0 << 0) ;

    u4CRCTmp3 = (_pucCRCBuf[u4InstID][27] );
    u4CRCTmp2 = (_pucCRCBuf[u4InstID][26] );
    u4CRCTmp1 = (_pucCRCBuf[u4InstID][25] );
    u4CRCTmp0 = (_pucCRCBuf[u4InstID][24] );
    u4CRCValueC2 = (u4CRCTmp3 << 24) | (u4CRCTmp2 << 16) |  (u4CRCTmp1 << 8) |  (u4CRCTmp0 << 0) ;

    u4CRCTmp3 = (_pucCRCBuf[u4InstID][31] );
    u4CRCTmp2 = (_pucCRCBuf[u4InstID][30] );
    u4CRCTmp1 = (_pucCRCBuf[u4InstID][29] );
    u4CRCTmp0 = (_pucCRCBuf[u4InstID][28] );
    u4CRCValueC3 = (u4CRCTmp3 << 24) | (u4CRCTmp2 << 16) |  (u4CRCTmp1 << 8) |  (u4CRCTmp0 << 0) ;

    u4HWCRC_Y0 = u4VDecReadCRC(u4InstID, VDEC_CRC_Y_CHKSUM0);
    u4HWCRC_Y1 = u4VDecReadCRC(u4InstID, VDEC_CRC_Y_CHKSUM1);
    u4HWCRC_Y2 = u4VDecReadCRC(u4InstID, VDEC_CRC_Y_CHKSUM2);
    u4HWCRC_Y3 = u4VDecReadCRC(u4InstID, VDEC_CRC_Y_CHKSUM3);

    u4HWCRC_C0 = u4VDecReadCRC(u4InstID, VDEC_CRC_C_CHKSUM0);
    u4HWCRC_C1 = u4VDecReadCRC(u4InstID, VDEC_CRC_C_CHKSUM1);
    u4HWCRC_C2 = u4VDecReadCRC(u4InstID, VDEC_CRC_C_CHKSUM2);
    u4HWCRC_C3 = u4VDecReadCRC(u4InstID, VDEC_CRC_C_CHKSUM3);


    if( (u4HWCRC_Y0 != u4CRCValueY0) 
    	|| (u4HWCRC_Y1 != u4CRCValueY1)
    	|| (u4HWCRC_Y2 != u4CRCValueY2)
    	|| (u4HWCRC_Y3 != u4CRCValueY3)
    	|| (u4HWCRC_C0 != u4CRCValueC0)
    	|| (u4HWCRC_C1 != u4CRCValueC1)
    	|| (u4HWCRC_C2 != u4CRCValueC2)
    	|| (u4HWCRC_C3 != u4CRCValueC3)
      )
    {
       fgDecErr = TRUE;
       fgCRCPass = FALSE;
       sprintf(strMessage,"CRC Error ==> Pic count to [%d] \n", _u4FileCnt[u4InstID]);     
       fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
       return FALSE;
    }

    _ptCurrFBufInfo[u4InstID]->u4FrameCnt = _u4FileCnt[u4InstID];
    if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
    {
        #ifdef REDEC    
        if(_u4ReDecCnt[u4InstID] == 0)
        #endif    
        {
            _u4FileCnt[u4InstID] ++;
            vSetDecFlag(u4InstID, DEC_FLAG_CHG_FBUF);
        }
    }
    //else
    {
        _ucPrevDecFld[u4InstID] = _ptCurrFBufInfo[u4InstID]->ucFBufStatus;
    }
    #ifndef INTERGRATION_WITH_DEMUX
    // Check if still pic needed compare
    #if (defined(COMP_HW_CHKSUM) && (!defined(DOWN_SCALE_SUPPORT)) && (!defined(FGT_SUPPORT)))
    _tTempFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tTempFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tTempFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tTempFileInfo[u4InstID].u4FileLength = 0; 
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_chksum.bin\0", _u4FileCnt[u4InstID]);
    #ifdef IDE_READ_SUPPORT
    fgOpen = fgPureOpenIdeFile( _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    #else
    fgOpen = fgOpenFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    #endif
    #else
    _tFBufFileInfo[u4InstID].fgGetFileInfo = FALSE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tFBufFileInfo[u4InstID].u4FileLength = 4;      
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], _u4FileCnt[u4InstID]);
    #ifdef VDEC_DEBUG_USAGE  
    fgOpen = TRUE;
    #else
    #ifdef IDE_READ_SUPPORT
    fgOpen = fgPureOpenIdeFile( _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);
    #else
    fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);
    #endif
    #endif
    #endif
    if((fgOpen == FALSE) ||(fgDecErr == TRUE) || (_fgVDecErr[u4InstID] == TRUE))
    {
        sprintf(strMessage, "%s", "\n");
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        //fprintf(_tFileListRecInfo.fpFile, "\n");  
        // break decode
        if(fgOpen == FALSE)
        {
            sprintf(strMessage," Compare Finish==> Pic count to [%d] \n", _u4FileCnt[u4InstID] - 1);   
            printk("%s\n",strMessage);
            if(_u4FileCnt[u4InstID] == 1)
            {
                if(fgOpen == FALSE)
                {
                    vVDecOutputDebugString("real NULL\n");
                }
            }
            else //flush DPB buffer
            {
                if(fgIsDecFlagSet(u4InstID, DEC_FLAG_CHG_FBUF))
                {
                    _ptCurrFBufInfo[u4InstID]->eH264DpbStatus = H264_DPB_STATUS_DECODED;
                    _ptCurrFBufInfo[u4InstID]->u4DecOrder = _u4TotalDecFrms[u4InstID];
                }
                vFlushDPB(u4InstID, &_tVerMpvDecPrm[u4InstID], FALSE);
            }
        }
        else
        {
            sprintf(strMessage," Error ==> Pic count to [%d] \n", _u4FileCnt[u4InstID] - 1);     
            printk("%s\n",strMessage);
        }
        _u4VerBitCount[u4InstID] = 0xffffffff;
    }
    #endif

    
    if(_u4FileCnt[u4InstID] >= _u4EndCompPicNum[u4InstID])
    {
        _u4VerBitCount[u4InstID] = 0xffffffff;
    }    

    return fgCRCPass;
}
#endif


extern void initKernelEnv(void);
extern int closeFile(struct file *fp); 
extern struct file *openFile(char *path,int flag,int mode);
extern int readFile(struct file *fp,char *buf,int readlen );
extern int readFileSize(struct file *fp,char *buf,int readlen );
extern void RISCRead_VLD_TOP( UINT32 u4Addr, UINT32* pu4Value );
extern void Margin_padding(UINT32 Ptr_output_Y, UINT32 Ptr_output_C, UINT32 PIC_SIZE_Y );
void vH265Dump_ESA_NBM_performane_log(UINT32 u4VDecID, char* pBName, BOOL bIsUFOmode)
{    
    char file_name[200];
    char info_buff[500];
    struct file *fd; 
    struct file *filp_info; 
    int ret = 0;
    UINT32 NBM_DLE_NUM,ESA_REQ_DATA_NUM,MC_REQ_DATA_NUM,MC_MBX,MC_MBY,CYC_SYS,INTRA_CNT,LAT_BUF_BYPASS,Y_BLK_CNT,C_BLK_CNT,WAIT_CNT,REQ_CNT,MC_DLE_NBM,CYCLE_DRAM;

    initKernelEnv(); 

    if (bIsUFOmode)
        sprintf(file_name, "/mnt/sdcard/ESA_NBM_log/HEVC_UFO_%s.csv", pBName );
    else
        sprintf(file_name, "/mnt/sdcard/ESA_NBM_log/HEVC_%s.csv", pBName );
    
    fd = openFile(file_name,O_RDONLY,0); 
    if (IS_ERR(fd) ){   // if info file exitst-> append; not exitst -> create
        filp_info = filp_open( file_name, O_CREAT|O_RDWR,0777 );
        sprintf( info_buff, "NBM_DLE_NUM,ESA_REQ_DATA_NUM,MC_REQ_DATA_NUM,MC_MBX,MC_MBY,CYC_SYS,INTRA_CNT,LAT_BUF_BYPASS,Y_BLK_CNT,C_BLK_CNT,WAIT_CNT,REQ_CNT,MC_DLE_NBM,CYCLE_DRAM\n" );
        ret = filp_info->f_op->write( filp_info, info_buff, strlen(info_buff), &(filp_info->f_pos) );
    } else {
        closeFile(fd); 
        filp_info = filp_open( file_name , O_APPEND|O_RDWR,0777 );
    }

    NBM_DLE_NUM = u4VDecReadMC(u4VDecID, 476*4);
    ESA_REQ_DATA_NUM = u4VDecReadMC(u4VDecID, 558*4);
    MC_REQ_DATA_NUM = u4VDecReadMC(u4VDecID, 650*4);
    MC_MBX = u4VDecReadMC(u4VDecID, 10*4);
    MC_MBY = u4VDecReadMC(u4VDecID, 11*4);
    CYC_SYS = u4VDecReadMC(u4VDecID, 632*4);
    INTRA_CNT = u4VDecReadMC(u4VDecID, 633*4);
    LAT_BUF_BYPASS = u4VDecReadVLDTOP(u4VDecID, 60*4);  
    LAT_BUF_BYPASS &= 0x1;
    Y_BLK_CNT = u4VDecReadMC(u4VDecID, 634*4);
    C_BLK_CNT = u4VDecReadMC(u4VDecID, 635*4);
    WAIT_CNT = u4VDecReadMC(u4VDecID, 636*4);
    REQ_CNT = u4VDecReadMC(u4VDecID, 493*4);
    MC_DLE_NBM = u4VDecReadMC(u4VDecID, 477*4);
    CYCLE_DRAM = u4VDecReadMC(u4VDecID, 478*4);
           
    sprintf( info_buff, "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", 
        NBM_DLE_NUM,ESA_REQ_DATA_NUM,MC_REQ_DATA_NUM,MC_MBX,MC_MBY,CYC_SYS,INTRA_CNT,LAT_BUF_BYPASS,Y_BLK_CNT,C_BLK_CNT,WAIT_CNT,REQ_CNT,MC_DLE_NBM,CYCLE_DRAM);
    ret = filp_info->f_op->write( filp_info, info_buff, strlen(info_buff), &(filp_info->f_pos) );

    if (!IS_ERR(filp_info))
        closeFile(filp_info); 
    set_fs( oldfs );
        
}
void vH265DumpInfo(UINT32 u4VDecID)
{
    UINT32 risc_val1,risc_val2;
    struct file* fd;
    struct file* filp_info;
    int ret = 0;   
    char bitstream_name[200];
    char risc_temp[200];

    //Dump info MC476 VLD_TOP40
    memcpy (bitstream_name , _bFileStr1[u4VDecID][1]+12 , (strlen(_bFileStr1[u4VDecID][1]) -38) );
    bitstream_name[(strlen(_bFileStr1[u4VDecID][1]) -38)] = '\0';

    initKernelEnv(); 

    sprintf(risc_temp, "/mnt/sdcard/%s_info", bitstream_name  );
    fd = openFile(risc_temp,O_RDONLY,0); 
    if (IS_ERR(fd) ) {   // if info file exitst-> append; not exitst -> create
        filp_info = filp_open( risc_temp, O_CREAT|O_RDWR,0777 );
        sprintf( risc_temp, "Frame# MC476 VLD_TOP40\n" );
        ret = filp_info->f_op->write( filp_info, risc_temp, strlen(risc_temp), &(filp_info->f_pos) );
    } else {
        closeFile(fd); 
        filp_info = filp_open( risc_temp , O_APPEND|O_RDWR,0777 );
    }

    if (IS_ERR(filp_info) )
         printk("\n[ERROR] File Open Error:%s %0x08X\n",risc_temp, filp_info);
    
    risc_val1 = u4VDecReadMC(u4VDecID, 476*4);
    risc_val2 = u4VDecReadVLDTOP(u4VDecID, 40*4);  
    sprintf( risc_temp, "%d %u %u\n", _u4PicCnt[u4VDecID] ,risc_val1 ,risc_val2  );

    ret = filp_info->f_op->write( filp_info, risc_temp, strlen(risc_temp), &(filp_info->f_pos) );

    if (!IS_ERR(filp_info))
        closeFile(filp_info); 
    set_fs( oldfs );
    
}

#if 0
void vH265DumpYUV( UINT32 u4InstID, UINT32  PP_OUT_Y_ADDR , UINT32  PP_OUT_C_ADDR, UINT32 PIC_SIZE_Y )
{
    //transform 16*32 block mode to yuv video

    UINT32 u4YLength, trans_addr;
    UINT32 mask = 0x00FFFFFF;
    UINT32 width, height;
    struct file *EC_rec_filp;
    struct file *rdFd;
    char file_name[200];
    char bitstream_name[200] = {0};
    char* Y_out;
    char* U_out;
    char* V_out;
    char* buffY;
    char* buffC;
    int i,j;

    memcpy (bitstream_name , _bFileStr1[u4InstID][1]+12 , (strlen(_bFileStr1[u4InstID][1]) -38) );
    bitstream_name[(strlen(_bFileStr1[u4InstID][1]) -38)] = '\0';

    width = _tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.prSPS->u4PicWidthInLumaSamples;
    height = _tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.prSPS->u4PicHeightInLumaSamples;

    buffY  = PP_OUT_Y_ADDR;
    buffC = PP_OUT_C_ADDR;

    initKernelEnv(); 

    sprintf( file_name, "/mnt/sdcard/%s_er%d_rec.yuv", bitstream_name, 0 );

    rdFd = openFile(file_name,O_RDONLY,0); 
    if (IS_ERR(rdFd) ){
        EC_rec_filp = filp_open( file_name , O_CREAT|O_RDWR,0777 );
    } else {
        closeFile(rdFd); 
        EC_rec_filp = filp_open( file_name , O_APPEND|O_RDWR,0777 );
    }
    
    if (IS_ERR(EC_rec_filp))
         printk("\n[ERROR] File Open Error:%s 0x%08X\n",file_name, EC_rec_filp);

    u4YLength = width*( (height+31)>>5 ) <<5;

    Y_out = (char*) vmalloc(sizeof(char)*u4YLength);
    U_out = (char*) vmalloc(sizeof(char)*u4YLength/4);
    V_out = (char*) vmalloc(sizeof(char)*u4YLength/4);

    for (i =0; i<u4YLength; i+=16 ){
        trans_addr = (( (i>>4 & mask) % (width>>4& mask)) <<9 ) + ((((i>>4 & mask)/(width>>4& mask)) %32) <<4)
            + (((i>>4 & mask)/(width>>4& mask))/32)*  (((width+63) >>6 ) <<11);
        //printk( "PIC_SIZE_Y: 0x%08X,  u4YLength: 0x%08X, i(0x%08X) <- trans_addr(0x%08X)\n", PIC_SIZE_Y, u4YLength, i, trans_addr );
        memcpy( Y_out+i, buffY+trans_addr, 16);
    }

    j = 0;
    for (i =0; i<u4YLength/2; i+=16 ){
        trans_addr = (( (i>>4 & mask) % (width>>4& mask)) <<8 ) + ((((i>>4 & mask)/(width>>4& mask)) %16) <<4)
            + (((i>>4 & mask)/(width>>4& mask))/16)*  (((width+63) >>6 ) <<10);
        //printk( "PIC_SIZE_C: 0x%08X,  u4YLength/2: 0x%08X, j(0x%08X) <- trans_addr(0x%08X)\n", PIC_SIZE_Y/2, u4YLength/2, j, trans_addr );
        U_out[j] = buffC[trans_addr];
        U_out[j+1] = buffC[trans_addr+2];
        U_out[j+2] = buffC[trans_addr+4];
        U_out[j+3] = buffC[trans_addr+6];
        U_out[j+4] = buffC[trans_addr+8];
        U_out[j+5] = buffC[trans_addr+10];
        U_out[j+6] = buffC[trans_addr+12];
        U_out[j+7] = buffC[trans_addr+14];
        j += 8;
    }
    
    j = 0;
    for (i =0; i<u4YLength/2; i+=16 ){
        trans_addr = (( (i>>4 & mask) % (width>>4& mask)) <<8 ) + ((((i>>4 & mask)/(width>>4& mask)) %16) <<4)
            + (((i>>4 & mask)/(width>>4& mask))/16)*  (((width+63) >>6 ) <<10);
        //printk( "PIC_SIZE_C: 0x%08X,  u4YLength/2: 0x%08X, j(0x%08X) <- trans_addr(0x%08X)\n", PIC_SIZE_Y/2, u4YLength/2, j, trans_addr );
        V_out[j] = buffC[trans_addr+1];
        V_out[j+1] = buffC[trans_addr+3];
        V_out[j+2] = buffC[trans_addr+5];
        V_out[j+3] = buffC[trans_addr+7];
        V_out[j+4] = buffC[trans_addr+9];
        V_out[j+5] = buffC[trans_addr+11];
        V_out[j+6] = buffC[trans_addr+13];
        V_out[j+7] = buffC[trans_addr+15];
        j += 8;
    }

    printk("[INFO] YUV dump bytes: %d \n", u4YLength*3/2);
    EC_rec_filp->f_op->write( EC_rec_filp, Y_out, u4YLength, &EC_rec_filp->f_pos );        
    EC_rec_filp->f_op->write( EC_rec_filp, U_out, u4YLength/4, &EC_rec_filp->f_pos );
    EC_rec_filp->f_op->write( EC_rec_filp, V_out, u4YLength/4, &EC_rec_filp->f_pos );

    vfree(Y_out);
    vfree(U_out);
    vfree(V_out);

    if (!IS_ERR(EC_rec_filp))
        closeFile(EC_rec_filp); 
    set_fs( oldfs );
    
}

#else
void vH265DumpYUV( UINT32 u4InstID, UINT32  PP_OUT_Y_ADDR , UINT32  PP_OUT_C_ADDR, UINT32 PIC_SIZE_Y)
{
    //transform 16*32 block mode to yuv video

    UINT32 u4YLength, trans_addr;
    UINT32 mask = 0x00FFFFFF;
    UINT32 width, height;
    struct file *EC_rec_filp;
    struct file *rdFd;
    char file_name[200];
    char bitstream_name[200] = {0};
    char* Y_out;
    char* U_out;
    char* V_out;
    char* buffY;
    char* buffC;
    int i,j;

    width  = _tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.prSPS->u4PicWidthInLumaSamples;
    height = _tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.prSPS->u4PicHeightInLumaSamples;

    buffY  = PP_OUT_Y_ADDR;
    buffC = PP_OUT_C_ADDR;

    printk("yaochang _bFileStr1 = %s \n", _bFileStr1[u4InstID][1]);
    memcpy (bitstream_name , _bFileStr1[u4InstID][1] + 10 , (strlen(_bFileStr1[u4InstID][1]) - 14) );
    
    bitstream_name[(strlen(_bFileStr1[u4InstID][1]) - 14)] = '\0';
    printk("yaochang bitstream_name = %s \n", bitstream_name);
    sprintf( file_name, "D:\\yuv\\%s_frame_%d_rec.yuv", bitstream_name, _u4PicCnt[u4InstID]);
    
    #ifdef VDEC_SUPPORT_FATS
        printk("\nTry to Open File : %s.\n",file_name);
    #else
        initKernelEnv(); 
        rdFd = openFile(file_name,O_RDONLY,0); 
        if (IS_ERR(rdFd) ){
            EC_rec_filp = filp_open( file_name , O_CREAT|O_RDWR,0777 );
        } else {
            closeFile(rdFd); 
            EC_rec_filp = filp_open( file_name , O_APPEND|O_RDWR,0777 );
        }
        
        if (IS_ERR(EC_rec_filp))
             printk("\n[ERROR] File Open Error:%s 0x%08X\n",file_name, EC_rec_filp);
    #endif

    width  =  ((width + 63) >> 6) << 6;
    height = ( (height + 31) >> 5 ) << 5;
    u4YLength = width * height;

    Y_out = (char*) vmalloc(sizeof(char)*u4YLength);
    U_out = (char*) vmalloc(sizeof(char)*u4YLength/4);
    V_out = (char*) vmalloc(sizeof(char)*u4YLength/4);

    for (i =0; i< u4YLength; i+= 16 )
    {
        trans_addr = (( (i>>4 & mask) % (width>>4& mask)) <<9 ) + ((((i>>4 & mask)/(width>>4& mask)) %32) <<4)
            + (((i>>4 & mask)/(width>>4& mask))/32)*  (((width+63) >>6 ) <<11);
        //printk( "PIC_SIZE_Y: 0x%08X,  u4YLength: 0x%08X, i(0x%08X) <- trans_addr(0x%08X)\n", PIC_SIZE_Y, u4YLength, i, trans_addr );
        memcpy( Y_out+i, buffY + trans_addr, 16);
    }

    j = 0;
    for (i =0; i<u4YLength/2; i+=16 )
    {
        trans_addr = (( (i>>4 & mask) % (width>>4& mask)) <<8 ) + ((((i>>4 & mask)/(width>>4& mask)) %16) <<4)
                      + (((i>>4 & mask)/(width>>4& mask))/16)*  (((width+63) >>6 ) <<10);
       // printk( "PIC_SIZE_C: 0x%08X,  u4YLength/2: 0x%08X, j(0x%08X) <- trans_addr(0x%08X)\n", PIC_SIZE_Y/2, u4YLength/2, j, trans_addr );
        U_out[j] = buffC[trans_addr];
        U_out[j+1] = buffC[trans_addr+2];
        U_out[j+2] = buffC[trans_addr+4];
        U_out[j+3] = buffC[trans_addr+6];
        U_out[j+4] = buffC[trans_addr+8];
        U_out[j+5] = buffC[trans_addr+10];
        U_out[j+6] = buffC[trans_addr+12];
        U_out[j+7] = buffC[trans_addr+14];
        j += 8;
    }
    
    j = 0;
    for (i =0; i<u4YLength/2; i+=16 )
    {
        trans_addr = (( (i>>4 & mask) % (width>>4& mask)) <<8 ) + ((((i>>4 & mask)/(width>>4& mask)) %16) <<4)
                     + (((i>>4 & mask)/(width>>4& mask))/16)*  (((width+63) >>6 ) <<10);
       // printk( "PIC_SIZE_C: 0x%08X,  u4YLength/2: 0x%08X, j(0x%08X) <- trans_addr(0x%08X)\n", PIC_SIZE_Y/2, u4YLength/2, j, trans_addr );
        V_out[j] = buffC[trans_addr+1];
        V_out[j+1] = buffC[trans_addr+3];
        V_out[j+2] = buffC[trans_addr+5];
        V_out[j+3] = buffC[trans_addr+7];
        V_out[j+4] = buffC[trans_addr+9];
        V_out[j+5] = buffC[trans_addr+11];
        V_out[j+6] = buffC[trans_addr+13];
        V_out[j+7] = buffC[trans_addr+15];
        j += 8;
    }
    
    #ifdef VDEC_SUPPORT_FATS
        printk("[INFO] YUV dump bytes: %d \n", u4YLength*3/2);
        fgWrData2PC(Y_out, u4YLength, 7, file_name);    
        fgWrData2PC(U_out, u4YLength/4, 7, file_name);
        fgWrData2PC(V_out, u4YLength/4, 7, file_name);

    #else
        printk("[INFO] YUV dump bytes: %d \n", u4YLength*3/2);
        EC_rec_filp->f_op->write( EC_rec_filp, Y_out, u4YLength, &EC_rec_filp->f_pos );        
        EC_rec_filp->f_op->write( EC_rec_filp, U_out, u4YLength/4, &EC_rec_filp->f_pos );
        EC_rec_filp->f_op->write( EC_rec_filp, V_out, u4YLength/4, &EC_rec_filp->f_pos );
    #endif

    vfree(Y_out);
    vfree(U_out);
    vfree(V_out);
    
    #ifdef VDEC_SUPPORT_FATS
    #else
        if (!IS_ERR(EC_rec_filp))
            closeFile(EC_rec_filp); 
        set_fs( oldfs );
    #endif    
}
#endif

static void vH265DumpYC( UINT32 u4InstID, UINT32  PP_OUT_Y_ADDR , UINT32  PP_OUT_C_ADDR, UINT32 PIC_SIZE_Y)
{
    UINT32 u4YSize;
    UINT32 width, height;
    char file_name[200]      = {0};
    char bitstream_name[200] = {0};
    char* buffY;
    char* buffC;

    width  = _tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.prSPS->u4PicWidthInLumaSamples;
    height = _tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.prSPS->u4PicHeightInLumaSamples;
    width  =  ((width + 63) >> 6) << 6;
    height =  ((height + 31) >> 5 ) << 5;
    u4YSize = width * height;
    
    buffY  = PP_OUT_Y_ADDR;
    buffC  = PP_OUT_C_ADDR;
    
    printk("yaochang _bFileStr1 = %s \n", _bFileStr1[u4InstID][1]);
    memcpy (bitstream_name , _bFileStr1[u4InstID][1] + 3 , (strlen(_bFileStr1[u4InstID][1]) - 25) );
    bitstream_name[(strlen(_bFileStr1[u4InstID][1]) - 25)] = '\0';
    printk("yaochang bitstream_name = %s \n", bitstream_name);

    /* dump Y to PC */
    sprintf( file_name, "D:\\yuv\\%s_frame_%d_Y.dat", bitstream_name, _u4PicCnt[u4InstID]);
    printk(" Try to Open File : %s.\n",file_name);
    printk(" Y dump bytes: %d \n", u4YSize);
    fgWrData2PC(buffY, u4YSize, 7, file_name);    

    /* dump CbCr to PC */
    sprintf( file_name, "D:\\yuv\\%s_frame_%d_C.dat", bitstream_name, _u4PicCnt[u4InstID]);
    printk(" Try to Open File : %s.\n",file_name);
    printk(" CbCr dump bytes: %d \n", u4YSize >> 1);
    fgWrData2PC(buffC, u4YSize >> 1, 7, file_name);  
}

void vH265DumpVFIFO( UINT32 u4InstID, UCHAR *vfifoaddr, UINT32  vfifolen, UINT32 flag)
{
    char file_name[200]      = {0};
    char bitstream_name[200] = {0};
    
    printk("Yaochuang _bFileStr1 = %s \n", _bFileStr1[u4InstID][1]);
    memcpy (bitstream_name , _bFileStr1[u4InstID][1] + 3 , (strlen(_bFileStr1[u4InstID][1]) - 25) );
    bitstream_name[(strlen(_bFileStr1[u4InstID][1]) - 25)] = '\0';
    printk("Yaochuang bitstream_name = %s \n", bitstream_name);

	if (flag) //decode finished
	{
		sprintf( file_name, "D:\\yuv\\%s_after_bitstream.bin", bitstream_name);
	}
	else
	{
		sprintf( file_name, "D:\\yuv\\%s_before_bitstream.bin", bitstream_name);
	}
	printk(" Try to Open File : %s.\n",file_name);
    printk(" vfifo size = %d \n", vfifolen);
    fgWrData2PC(vfifoaddr, vfifolen, 7, file_name);    
}

void vH265DumpYCbCr( UINT32 u4InstID, UINT32  PP_OUT_Y_ADDR , UINT32  PP_OUT_C_ADDR, UINT32 PIC_SIZE_Y)
{
    UINT32 u4YSize;
    UINT32 width, height;
    char file_name[200]      = {0};
    char bitstream_name[200] = {0};
    char* buffY;
    char* buffC;

    width  = _tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.prSPS->u4PicWidthInLumaSamples;
    height = _tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.prSPS->u4PicHeightInLumaSamples;
    width  =  ((width + 63) >> 6) << 6;
    height = ( (height + 31) >> 5 ) << 5;
    u4YSize = width * height;
    
    buffY  = PP_OUT_Y_ADDR;
    buffC  = PP_OUT_C_ADDR;
    
    printk("yaochang _bFileStr1 = %s \n", _bFileStr1[u4InstID][1]);
    memcpy (bitstream_name , _bFileStr1[u4InstID][1] + 10 , (strlen(_bFileStr1[u4InstID][1]) - 14) );
    bitstream_name[(strlen(_bFileStr1[u4InstID][1]) - 14)] = '\0';
    printk("yaochang bitstream_name = %s \n", bitstream_name);

    /* dump Y to PC */
    sprintf( file_name, "D:\\yuv\\%s_frame_%d_Y.dat", bitstream_name, _u4PicCnt[u4InstID]);
    printk(" Try to Open File : %s.\n",file_name);
    printk(" Y dump bytes: %d \n", u4YSize);
    fgWrData2PC(buffY, u4YSize, 7, file_name);    

    /* dump CbCr to PC */
    sprintf( file_name, "D:\\yuv\\%s_frame_%d_C.dat", bitstream_name, _u4PicCnt[u4InstID]);
    printk(" Try to Open File : %s.\n",file_name);
    printk(" CbCr dump bytes: %d \n", u4YSize >> 1);
    fgWrData2PC(buffC, u4YSize >> 1, 7, file_name);  
}
int vH265DumpMem( UINT32 u4InstID, unsigned char* buf, unsigned int size , int  frame_num , unsigned int type , bool isTimeout){
    struct file *filp = NULL;
    static struct file *filp_error = NULL;
    char filename_error[256] = {0};
    char bitstream_name[200] = {0};
    char YCM;
    int ret,i;

    memcpy (bitstream_name , _bFileStr1[u4InstID][1]+12 , (strlen(_bFileStr1[u4InstID][1]) -38) );
    bitstream_name[(strlen(_bFileStr1[u4InstID][1]) -38)] = '\0';

    if ( type == 1 ){
        YCM = 'Y';
    }else if ( type == 2 ){
        YCM = 'C';
    }else if ( type == 3 ){
        YCM = 'M';
    }else if ( type == 4 ){
        YCM = 'L';
    }else if ( type == 5 ){
        YCM = 'K';
    }else {
        YCM = 'T';
    }

    initKernelEnv(); 
    if (_ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded){
        sprintf( filename_error, "/mnt/sdcard/%s_decoded_%d_UFO_%c.dat", bitstream_name, frame_num, YCM );
    } else {
        sprintf( filename_error, "/mnt/sdcard/%s_decoded_%d_%c.dat", bitstream_name, frame_num, YCM );
    }
    
    filp = filp_open( filename_error, O_CREAT|O_RDWR,0777 );
    if ( filp_error==NULL ){
        filp_error = filp_open( "/mnt/sdcard/HEVC_test_error_frames" , O_CREAT|O_RDWR,0777 );
    }else{
        filp_error = filp_open( "/mnt/sdcard/HEVC_test_error_frames" , O_APPEND|O_RDWR,0777 );
    }
    
    if(IS_ERR(filp) ||IS_ERR(filp_error) ){
        printk("\nFile Open Error:%s\n",filename_error);
        set_fs( oldfs );
        return -1;
    }
    
    // dump frame data
    ret = filp->f_op->write( filp, buf, size, &filp->f_pos );        
    
    if ( !isTimeout ){
        sprintf(filename_error,"%s %c %d\n", bitstream_name, YCM, frame_num );
        ret = filp_error->f_op->write( filp_error, filename_error, strlen(filename_error), &filp_error->f_pos );
    } else if ( isTimeout ){
        sprintf(filename_error,"%s %c %d timeout\n", bitstream_name, YCM, frame_num );
        ret = filp_error->f_op->write( filp_error, filename_error, strlen(filename_error), &filp_error->f_pos );
    }

    closeFile( filp );
    closeFile( filp_error );

    set_fs( oldfs );
    return 0;
}

BOOL fgGoldenCmp(UINT32 u4DecBuf,UINT32 u4GoldenBuf,UINT32 u4Size)
{
    UINT32 i;
    UINT32 *pSrc,*pDes;
    pSrc = (UINT32 *)u4DecBuf;
    pDes = (UINT32 *)u4GoldenBuf;
    //width 64 align,height 32 align
    for(i = 0; i < u4Size/4; i++)
    {
        if(*pSrc != *pDes)
        {
            printk("Decode_buf 0x%x [0x%x] != Golden_Buf 0x%x [0x%x] offset i =  %d\n",u4DecBuf,*pSrc,u4GoldenBuf,*pDes,i);
            return 1;
        }
        pSrc ++;
        pDes ++;
    }

    return 0;
}

int  vH265GoldenComparison( UINT32 u4InstID, int frame_num, unsigned int PIC_SIZE_Y, UINT32 Ptr_output_Y, UINT32 Ptr_output_C , UINT32 MV_COL_PIC_SIZE, bool isDump,
                                               UINT32 Dpb_ufo_Y_len_addr, UINT32 Dpb_ufo_C_len_addr, UINT32 UFO_LEN_SIZE_Y, UINT32 UFO_LEN_SIZE_C )
#if 1 //HUB MT8137 HEVC
{
    //golden result comparison
    unsigned char *buf_Golden;
    unsigned char *buf_Golden_C;
    char *ptr_base = NULL;
    char file_name[200] = {0};
    char bitstream_name[200] = {0};
    char dump_file_name[200] = {0};
    struct file *fd; 
    int ret, file_num, file_len,retY,retC; 
    UINT32 u4CompareRange;
    UINT32 u4RetRegValue;
    UINT32 u4LCUsize;
    UINT32 u4W_lcu;
    UINT32 u4RealWidth, u4RealHeight;
    BOOL fgOpen;

    retY = 0;
    retC = 0;
    file_len = 0;
    file_num = 0;
    ret = 0;
    
    strncpy (bitstream_name , _bFileStr1[u4InstID][1], (strlen(_bFileStr1[u4InstID][1]) -21));
//#ifdef VDEC_SIM_DUMP
    printk("// [INFO] GoldenComparison PIC_SIZE_Y: 0x%08X, PP_OUT_Y_ADDR: 0x%08X,  PP_OUT_C_ADDR: 0x%08X\n", PIC_SIZE_Y, Ptr_output_Y, Ptr_output_C);
//#endif
    printk("// Begin the Golden Compare frame #%d\n", frame_num);
    printk("// Instance id = %d, _ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded  = %d\n",u4InstID, _ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded);

    RISCRead_VLD_TOP(28, &u4RetRegValue);
    u4RealWidth = u4RetRegValue & 0xFFFF;
    u4RealHeight = (u4RetRegValue>>16) & 0xFFFF;
    u4W_lcu = ((u4RealWidth + 64-1)/64)*64;
    
    if (frame_num < _u4StartCompPicNum[u4InstID])
    {
        printk("skip current frame @ %d\n",frame_num);
        goto Check_End;
    }
    // Load golden file
    buf_Golden = _pucDumpYBuf[u4InstID];

    // UFO
    if (_ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded)
    {
        if (_u4Main10Mode == 0) // 8-bit
        {
            sprintf(file_name, "%sufo_pat/ufo_%d_bits_Y.out", bitstream_name, frame_num);
        }
        else // 10-bit compact mode, other mode did not support
        {            
            sprintf(file_name, "%sufo_pat/ufo_%d_compact_Y.out", bitstream_name, frame_num);
        }
    } // not UFO
    else if (_u4Main10Mode == 0)
    {
        sprintf(file_name, "%ssao_pat/frame_%d_Y.dat", bitstream_name, frame_num);
    }
    else if(_u4Main10Mode == HEVC_16BIT_MODE) // 16-bit mode
    {                  
        sprintf(file_name, "%ssao_pat/frame_%d_Y.dat", bitstream_name, frame_num);
    }
    else if(_u4Main10Mode == HEVC_REORDER_MODE) // reorder mode
    {          
        sprintf(file_name, "%sufo_pat/ufo_%d_reorder_bypass_Y.out", bitstream_name, frame_num);
    }
    else if(_u4Main10Mode == HEVC_COMPACT_MODE) // compact mode
    {          
        // without UFO 
        sprintf(file_name, "%sufo_pat/ufo_%d_compact_bypass_Y.out", bitstream_name, frame_num);
    }
           
    //read Y golden
    _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = buf_Golden;
    _tFBufFileInfo[u4InstID].u4TargetSz = PIC_SIZE_Y;  
    _tFBufFileInfo[u4InstID].u4FileLength = 0;  
    memset ( buf_Golden , 0 ,PIC_SIZE_Y );
    fgOpen = fgOpenFile(u4InstID, file_name,"r+b", &_tFBufFileInfo[u4InstID]);
    if (fgOpen == FALSE)
    {
        printk("// Open golden file error : %s\n",file_name);
    }
    else
    {
    }
    
    buf_Golden_C = _pucDumpCBuf[u4InstID];

    // UFO
    if (_ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded)
    {
        if(_u4Main10Mode == 0) // 8-bit
        {
            sprintf(file_name, "%sufo_pat/ufo_%d_bits_CbCr.out", bitstream_name, frame_num);
        }
        else // 10-bit compact mode 
        {            
            sprintf(file_name, "%sufo_pat/ufo_%d_compact_CbCr.out", bitstream_name, frame_num);
        }        
    } // not UFO
    else if(_u4Main10Mode == 0)
    {
        sprintf(file_name, "%ssao_pat/frame_%d_C.dat", bitstream_name, frame_num);
    }
    else if(_u4Main10Mode == HEVC_16BIT_MODE) // 16-bit mode
    {   
        sprintf(file_name, "%ssao_pat/frame_%d_C.dat", bitstream_name, frame_num);
    }
    else if(_u4Main10Mode == HEVC_REORDER_MODE) // reorder mode
    {   
        sprintf(file_name, "%sufo_pat/ufo_%d_reorder_bypass_CbCr.out", bitstream_name, frame_num);
    }
    else if(_u4Main10Mode == HEVC_COMPACT_MODE) // compact mode
    {
        sprintf(file_name, "%sufo_pat/ufo_%d_compact_bypass_CbCr.out", bitstream_name, frame_num);
    }
       
    //read CbCr golden
    _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = buf_Golden_C;
    _tFBufFileInfo[u4InstID].u4TargetSz = (PIC_SIZE_Y >> 1);  
    _tFBufFileInfo[u4InstID].u4FileLength = 0;  
    memset ( buf_Golden_C , 0 ,(PIC_SIZE_Y >> 1) );
    fgOpen = fgOpenFile(u4InstID, file_name,"r+b", &_tFBufFileInfo[u4InstID]);
    if (fgOpen == FALSE)
    {
        printk("// Open golden file error : %s\n",file_name);
    }
    else
    {
    }

    //////////////Y golden comparison////////////////////
    printk("Y address 0x%x\n",Ptr_output_Y);
    retY = fgGoldenCmp(Ptr_output_Y,buf_Golden,PIC_SIZE_Y);
    printk("\n// ======== Frame %d Golden Y test: %d ========\n", frame_num, retY );
    
    //////////////C golden comparison////////////////////
    printk("CbCr address 0x%x\n",Ptr_output_C);
    retC = fgGoldenCmp(Ptr_output_C,buf_Golden_C,(PIC_SIZE_Y >> 1));
    printk("\n// ======== Frame %d Golden C test: %d ========\n", frame_num, retC );

    if((retY != 0) || (retC != 0) || (ret != 0))
    {
        printk("Golden compare mismatch: frame #%4d bit stream: %s\n", frame_num, _bFileStr1[u4InstID][1]);
        vH265DumpYC(u4InstID, Ptr_output_Y ,  Ptr_output_C, 0);
        return 1;
    }

	//UFO MODE Compare Y LEN, C LEN
    if (_ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded)
    {

        UINT32 cmp_size;

         //////////////Y LEN comparison////////////////////
        sprintf(file_name, "%sufo_pat/ufo_%d_len_Y.out", bitstream_name, frame_num);
        #if 1
        //dump Y golden
        _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
        _tFBufFileInfo[u4InstID].pucTargetAddr = buf_Golden;
        _tFBufFileInfo[u4InstID].u4TargetSz = UFO_LEN_SIZE_Y;  
        _tFBufFileInfo[u4InstID].u4FileLength = 0;  
        memset ( buf_Golden , 0 ,UFO_LEN_SIZE_Y );
        fgOpen = fgOpenFile(u4InstID, file_name,"r+b", &_tFBufFileInfo[u4InstID]);
        if (fgOpen == FALSE)
        {
            printk("// Open golden file error : %s\n",file_name);
        }
        else
        {
            
        }
        #else
		
        #endif
        ret = memcmp(buf_Golden, Dpb_ufo_Y_len_addr, (PIC_SIZE_Y+255)>>8 );
        if ( isDump || ret==0 )
            printk("\n// ======== Frame %d UFO Y LEN test: %d ========\n", frame_num, ret );
        
#ifndef SATA_HDD_READ_SUPPORT
        if (ret !=0 ){
            if (isDump)
                vH265DumpMem( u4InstID, Dpb_ufo_Y_len_addr, cmp_size, frame_num , 4, 0); 
            //set_fs( oldfs );
            return 1;
        }
#else
        if(ret != 0)
        {
            return ret;
        }
#endif
        //////////////C LEN comparison////////////////////
        sprintf(file_name, "%sufo_pat/ufo_%d_len_CbCr.out", bitstream_name, frame_num);
        #if 1
        //dump Y golden
        _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
        _tFBufFileInfo[u4InstID].pucTargetAddr = buf_Golden;
        _tFBufFileInfo[u4InstID].u4TargetSz = UFO_LEN_SIZE_C;  
        _tFBufFileInfo[u4InstID].u4FileLength = 0;  
        memset ( buf_Golden , 0 ,UFO_LEN_SIZE_C);
        fgOpen = fgOpenFile(u4InstID, file_name,"r+b", &_tFBufFileInfo[u4InstID]);
        if (fgOpen == FALSE)
        {
            printk("Open golden file error : %s\n",file_name);
        }
        else
        {
            
        }
        #else
        fd = openFile(file_name,O_RDONLY,0); 
        memset ( buf_Golden , 0 ,UFO_LEN_SIZE_C);
        if (IS_ERR(fd) ){
            printk("[Error] Miss file:%s!!!!!!!!!!!!!\n", file_name);
        } else {  
            cmp_size = readFileSize(fd ,buf_Golden, UFO_LEN_SIZE_C );
            closeFile(fd); 
        }
        #endif
        
        ret =  memcmp(buf_Golden, Dpb_ufo_C_len_addr, UFO_LEN_SIZE_Y>>1 );
        if ( isDump || ret==0 )
            printk("\n// ======== Frame %d UFO C LEN test: %d ========\n", frame_num, ret );
        
        #ifndef SATA_HDD_READ_SUPPORT
        if (ret !=0 )
        {
            if (isDump)
                vH265DumpMem( u4InstID, Dpb_ufo_C_len_addr, cmp_size, frame_num, 5, 0 );
            return 1;
        }
        #else
        if(ret != 0)
        {
            return ret;
        }
        #endif

    }
    printk("\n");
    
Check_End:
    //check end picture then break
    buf_Golden = _pucDumpYBuf[u4InstID];
    
    if (_ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded)
    {
        if(_u4Main10Mode == 0) //8 bit
        {
            sprintf(file_name, "%sufo_pat/ufo_%d_bits_Y.out", bitstream_name, frame_num+1);
        }
        else // 10-bit compact mode, other mode did not support
        {            
            sprintf(file_name, "%sufo_pat/ufo_%d_compact_Y.out", bitstream_name, frame_num+1);
        }
    }
    else // UFO OFF
    {   
        if(_u4Main10Mode == 0) //8 bit
        {
            sprintf(file_name, "%ssao_pat/frame_%d_Y.dat", bitstream_name, frame_num+1);
        }
        else if(_u4Main10Mode == HEVC_16BIT_MODE)
        {   
            sprintf(file_name, "%ssao_pat/frame_%d_Y.dat", bitstream_name, frame_num+1);
        }
        else if(_u4Main10Mode == HEVC_REORDER_MODE) // reorder mode
        {   
            sprintf(file_name, "%sufo_pat/ufo_%d_reorder_bypass_Y.out", bitstream_name, frame_num+1);
        }
        else if(_u4Main10Mode == HEVC_COMPACT_MODE) // compact mode
        {
            sprintf(file_name, "%sufo_pat/ufo_%d_compact_bypass_Y.out", bitstream_name, frame_num+1);
        }              
    }
    
    #if 1
    //dump Y golden
    _tFBufFileInfo[u4InstID].fgGetFileInfo = FALSE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = buf_Golden;
    _tFBufFileInfo[u4InstID].u4TargetSz = PIC_SIZE_Y;  
    _tFBufFileInfo[u4InstID].u4FileLength = 4;  
    memset ( buf_Golden , 0 ,PIC_SIZE_Y);
    fgOpen = fgOpenFile(u4InstID, file_name,"r+b", &_tFBufFileInfo[u4InstID]);
    if (fgOpen == FALSE)
    {
        printk("Golden miss,Golden Compare complete : %s\n",file_name);
        _u4VerBitCount[u4InstID] = 0xFFFFFFFF;
    }
    else
    {
        
    }
    #else

    #endif

#if 0 //#ifdef HEVC_VP_MODE //MT8137 HEVC VP Mode
    i4VDEC_HAL_H265_InitVDecHW(u4InstID);

    vVDecWriteHEVCMISC(u4InstID, 41*4, u4VDecReadHEVCMISC(u4InstID, 41*4) & (0x0 << 12));
    
    _tVerMpvDecPrm[u4InstID].u4PicW = ((_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.prSPS->u4PicWidthInLumaSamples + 15) >> 4) << 4;
    _tVerMpvDecPrm[u4InstID].u4PicH = ((_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.prSPS->u4PicHeightInLumaSamples + 31) >> 5) << 5;

    vVDecWriteVLDTOP(u4InstID, 26*4, (((_tVerMpvDecPrm[u4InstID].u4PicH / 16) - 1) << 16) | ((_tVerMpvDecPrm[u4InstID].u4PicW / 16) - 1));
    
    if (HEVC_ADDRESS_MODE == _u4VPMode)
    {
        vVDecWriteVLDTOP(u4InstID, 56*4, _ptH265FBufInfo[u4InstID][_tVerMpvDecPrm[u4InstID].ucDecFBufIdx].u4YStartAddr);
        vVDecWriteVLDTOP(u4InstID, 57*4, _ptH265FBufInfo[u4InstID][_tVerMpvDecPrm[u4InstID].ucDecFBufIdx + 1].u4YStartAddr);
        
        //16*16 MB count
        vVDecWriteVLDTOP(u4InstID, 67*4, ((_tVerMpvDecPrm[u4InstID].u4PicH / 16) - 1) * ((_tVerMpvDecPrm[u4InstID].u4PicW / 16) - 1));
        
        //Address Mode
        vVDecWriteVLDTOP(u4InstID, 67*4, u4VDecReadVLDTOP(u4InstID,67*4) | (0x1 << 24));
        
        //vp_dma trigger start
        vVDecWriteVLDTOP(u4InstID, 65*4, (0x1 << 8) | 0x1);
    }
    else if (HEVC_BLOCK_MODE == _u4VPMode)
    {
        vVDecWriteVLDTOP(u4InstID, 56*4, _ptH265FBufInfo[u4InstID][_tVerMpvDecPrm[u4InstID].ucDecFBufIdx].u4YStartAddr);
        vVDecWriteVLDTOP(u4InstID, 57*4, _ptH265FBufInfo[u4InstID][_tVerMpvDecPrm[u4InstID].ucDecFBufIdx].u4CStartAddr);
        vVDecWriteVLDTOP(u4InstID, 58*4, _ptH265FBufInfo[u4InstID][_tVerMpvDecPrm[u4InstID].ucDecFBufIdx + 1].u4YStartAddr);
        vVDecWriteVLDTOP(u4InstID, 59*4, _ptH265FBufInfo[u4InstID][_tVerMpvDecPrm[u4InstID].ucDecFBufIdx + 1].u4CStartAddr);
        
        if (_tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
        {
            vVDecWriteVLDTOP(u4InstID, 60*4, _ptH265FBufInfo[u4InstID][_tVerMpvDecPrm[u4InstID].ucDecFBufIdx].u4YLenStartAddr);
            vVDecWriteVLDTOP(u4InstID, 61*4, _ptH265FBufInfo[u4InstID][_tVerMpvDecPrm[u4InstID].ucDecFBufIdx].u4CLenStartAddr);
            vVDecWriteVLDTOP(u4InstID, 62*4, _ptH265FBufInfo[u4InstID][_tVerMpvDecPrm[u4InstID].ucDecFBufIdx + 1].u4YLenStartAddr);
            vVDecWriteVLDTOP(u4InstID, 63*4, _ptH265FBufInfo[u4InstID][_tVerMpvDecPrm[u4InstID].ucDecFBufIdx + 1].u4CLenStartAddr);

            if (_u4Main10Mode == 0)
            {
                vVDecWriteVLDTOP(u4InstID, 64*4, 0x0);
            }
            else
            {
                vVDecWriteVLDTOP(u4InstID, 64*4, 0x2);
            }
            
            //start MBx / MBy
            vVDecWriteVLDTOP(u4InstID, 54*4, (0x0 << 16) | 0x0);
            
            //16*16 MB count
            vVDecWriteVLDTOP(u4InstID, 67*4, ((_tVerMpvDecPrm[u4InstID].u4PicH / 16) - 1) * ((_tVerMpvDecPrm[u4InstID].u4PicW / 16) - 1));

            //UFO
            vVDecWriteVLDTOP(u4InstID, 67*4, u4VDecReadVLDTOP(u4InstID,67*4) | (0x1 << 28));
            
            //vp_dma trigger start
            vVDecWriteVLDTOP(u4InstID, 65*4, (0x1 << 8) | 0x1);
        }
        else
        {                        
            if (_u4Main10Mode == 0)
            {
                vVDecWriteVLDTOP(u4InstID, 64*4, 0x0);
            }
            else
            {
                vVDecWriteVLDTOP(u4InstID, 64*4, 0x2);
            }
            
            //start MBx / MBy
            vVDecWriteVLDTOP(u4InstID, 54*4, (0x0 << 16) | 0x0);

            //16*16 MB count
            vVDecWriteVLDTOP(u4InstID, 67*4, ((_tVerMpvDecPrm[u4InstID].u4PicH / 16) - 1) * ((_tVerMpvDecPrm[u4InstID].u4PicW / 16) - 1));

            //vp_dma trigger start
            vVDecWriteVLDTOP(u4InstID, 65*4, (0x1 << 8) | 0x1);
        }
    }
    
    while (u4VDEC_HAL_H265_VDec_ReadFinishFlag(u4InstID) != 1)
    {
        //
    }
    
    u4VDEC_HAL_H265_VDec_ClearInt(u4InstID);
#endif
    
    return 0;    
}
#else
{
    //golden result comparison
    unsigned char *buf_Golden;
    unsigned char *buf_Golden_C;
    char *ptr_base = NULL;
    char file_name[200] = {0};
    char bitstream_name[200] = {0};
    struct file *fd; 
    int ret, file_num, file_len;
    BOOL bIsFail;
    UINT32 u4CompareRange;
    UINT32 u4RetRegValue;
    UINT32 u4LCUsize;
    UINT32 u4W_lcu;
    UINT32 u4RealWidth, u4RealHeight;

    bIsFail = 0;

    strncpy (bitstream_name , _bFileStr1[u4InstID][1], (strlen(_bFileStr1[u4InstID][1]) -21));
#ifdef VDEC_SIM_DUMP
        printk("[INFO] GoldenComparison PIC_SIZE_Y: 0x%08X, PP_OUT_Y_ADDR: 0x%08X,  PP_OUT_C_ADDR: 0x%08X\n", PIC_SIZE_Y, Ptr_output_Y, Ptr_output_C);
#endif

    RISCRead_VLD_TOP(28, &u4RetRegValue);
    u4RealWidth = u4RetRegValue & 0xFFFF;
    u4RealHeight = (u4RetRegValue>>16) & 0xFFFF;
    u4W_lcu = ((u4RealWidth + 64-1)/64)*64;
    
    initKernelEnv();
    
    // Load golden file
    buf_Golden = _pucDumpYBuf[u4InstID];
    if (_ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded){
        sprintf(file_name, "%sufo_pat/ufo_%d_bits_Y.out", bitstream_name, frame_num);
    }else{
        sprintf(file_name, "%ssao_pat/frame_%d_Y.dat", bitstream_name, frame_num);
    }

    fd = openFile(file_name,O_RDONLY,0); 
    memset ( buf_Golden , 0 ,PIC_SIZE_Y );
    if (IS_ERR(fd) ){
        printk("[Error] Miss file: %s!!!!!!!!!!!!!\n", file_name);
    } else {  
        readFile(fd ,buf_Golden, PIC_SIZE_Y);
        closeFile(fd); 
    }

    buf_Golden_C = _pucDumpCBuf[u4InstID];
    if (_ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded){
        sprintf(file_name, "%sufo_pat/ufo_%d_bits_CbCr.out", bitstream_name, frame_num);
    }else{
        sprintf(file_name, "%ssao_pat/frame_%d_C.dat", bitstream_name, frame_num);
    }

    fd = openFile(file_name,O_RDONLY,0); 
    memset ( buf_Golden_C , 0 ,PIC_SIZE_Y>>1 );
    if (IS_ERR(fd) ){
        printk("[Error] Miss file: %s!!!!!!!!!!!!!\n", file_name);
    } else {  
        readFile(fd ,buf_Golden_C, PIC_SIZE_Y>>1 );
        closeFile(fd); 
    }

    if (_ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded && isDump){
        Margin_padding(buf_Golden, buf_Golden_C, PIC_SIZE_Y);
    }

    //////////////Y golden comparison////////////////////
    u4CompareRange = (((u4RealHeight+31)>>5)<<5) * u4W_lcu;
    ret = memcmp(buf_Golden, Ptr_output_Y, u4CompareRange );
    if ( isDump || ret==0 )
        printk("\n======== Frame %d Golden Y test: %d ========\n", frame_num, ret );

    if (ret !=0 ){
        if (isDump)
            vH265DumpMem( u4InstID, Ptr_output_Y, PIC_SIZE_Y, frame_num , 1, 0); 
        bIsFail = 1;
    }

    //////////////C golden comparison////////////////////
    u4CompareRange = (((u4RealHeight/2+15)>>4)<<4) * u4W_lcu;
    ret =  memcmp(buf_Golden_C, Ptr_output_C, u4CompareRange );
    if ( isDump || ret==0 )
        printk("\n======== Frame %d Golden C test: %d ========\n", frame_num, ret );

    if (ret !=0 ){
        if (isDump)
            vH265DumpMem( u4InstID, Ptr_output_C, PIC_SIZE_Y>>1, frame_num, 2, 0 );
        bIsFail = 1;
    }

    if (_ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded){

        UINT32 cmp_size;

         //////////////Y LEN comparison////////////////////
        sprintf(file_name, "%sufo_pat/ufo_%d_len_Y.out", bitstream_name, frame_num);

        fd = openFile(file_name,O_RDONLY,0); 
        memset ( buf_Golden , 0 ,UFO_LEN_SIZE_Y );
        if (IS_ERR(fd) ){
            printk("[Error] Miss file: %s!!!!!!!!!!!!!\n", file_name);
        } else {  
            cmp_size = readFileSize(fd ,buf_Golden, UFO_LEN_SIZE_Y);
            closeFile(fd); 
        }

        ret = memcmp(buf_Golden, Dpb_ufo_Y_len_addr, (PIC_SIZE_Y+255)>>8 );
        if ( isDump || ret==0 )
            printk("\n======== Frame %d UFO Y LEN test: %d ========\n", frame_num, ret );

        if (ret !=0 ){
            if (isDump)
                vH265DumpMem( u4InstID, Dpb_ufo_Y_len_addr, cmp_size, frame_num , 4, 0); 
            bIsFail = 1;
        }

        //////////////C LEN comparison////////////////////
        sprintf(file_name, "%sufo_pat/ufo_%d_len_CbCr.out", bitstream_name, frame_num);

        fd = openFile(file_name,O_RDONLY,0); 
        memset ( buf_Golden , 0 ,UFO_LEN_SIZE_C);
        if (IS_ERR(fd) ){
            printk("[Error] Miss file:%s!!!!!!!!!!!!!\n", file_name);
        } else {  
            cmp_size = readFileSize(fd ,buf_Golden, UFO_LEN_SIZE_C );
            closeFile(fd); 
        }

        ret =  memcmp(buf_Golden, Dpb_ufo_C_len_addr, UFO_LEN_SIZE_Y>>1 );
        if ( isDump || ret==0 )
            printk("\n======== Frame %d UFO C LEN test: %d ========\n", frame_num, ret );

        if (ret !=0 ){
            if (isDump)
                vH265DumpMem( u4InstID, Dpb_ufo_C_len_addr, cmp_size, frame_num, 5, 0 );
            bIsFail = 1;
        }

    }
    printk("\n");
    
    //check end picture then break
    if (_ptH265CurrFBufInfo[u4InstID]->bIsUFOEncoded){
        sprintf(file_name, "%sufo_pat/ufo_%d_bits_Y.out", bitstream_name, frame_num+1);
    }else{
        sprintf(file_name, "%ssao_pat/frame_%d_Y.dat", bitstream_name, frame_num+1);
    }
    fd = openFile(file_name,O_RDONLY,0); 
    
    if (IS_ERR(fd) ){
        _u4VerBitCount[u4InstID] = 0xFFFFFFFF;
    }

    set_fs( oldfs );
    return bIsFail; 
}
#endif
void vH265RingFIFO_read( UINT32 u4InstID ,BOOL bLoadBitstream)
{
    VDEC_INFO_H265_BS_INIT_PRM_T rH265BSInitPrm;    
    UINT32* u4Bit;
    UINT32 u4CurrentPtr;
    BOOL bReadFail = 0;

    u4Bit = (UINT32*)vmalloc(sizeof(UINT32));
    _u4CurrPicStartAddr[u4InstID] = u4VDEC_HAL_H265_ReadRdPtr(_u4BSID[u4InstID], u4InstID, (UINT32)_pucVFifo[u4InstID], u4Bit);
    rH265BSInitPrm.u4VFifoSa = (UINT32)_pucVFifo[u4InstID];
    rH265BSInitPrm.u4VFifoEa = (UINT32)_pucVFifo[u4InstID] + V_FIFO_SZ;
    rH265BSInitPrm.u4VLDRdPtr = (UINT32)_pucVFifo[u4InstID] + _u4CurrPicStartAddr[u4InstID];
#ifndef  RING_VFIFO_SUPPORT
    rH265BSInitPrm.u4VLDWrPtr = (UINT32)_pucVFifo[u4InstID] + V_FIFO_SZ;
#else
    rH265BSInitPrm.u4VLDWrPtr = (UINT32)_pucVFifo[u4InstID] + ((_u4LoadBitstreamCnt[u4InstID]%2)?(V_FIFO_SZ):(V_FIFO_SZ/2));
#endif
    rH265BSInitPrm.u4PredSa = /*PHYSICAL*/((UINT32)_pucPredSa[u4InstID]);

    if (bLoadBitstream)
    {        
#ifndef INTERGRATION_WITH_DEMUX
#ifdef  RING_VFIFO_SUPPORT
          if((_u4LoadBitstreamCnt[u4InstID]&0x1) && (rH265BSInitPrm.u4VLDRdPtr > 
          ((UINT32)_pucVFifo[u4InstID] + (V_FIFO_SZ/2) + 16 )))     // HEVC read pointer precise to 16 bytes
          {
            _tInFileInfo[u4InstID].fgGetFileInfo = TRUE;
            _tInFileInfo[u4InstID].pucTargetAddr = _pucVFifo[u4InstID];
            _tInFileInfo[u4InstID].u4FileOffset = ((V_FIFO_SZ * (_u4LoadBitstreamCnt[u4InstID]+ 1))/2);
            _tInFileInfo[u4InstID].u4TargetSz = (V_FIFO_SZ/2);    

          #ifdef  SATA_HDD_READ_SUPPORT
                if(!fgOpenHDDFile(u4InstID, _bFileStr1[u4InstID][1],"r+b", &_tInFileInfo[u4InstID]))
                {
                  bReadFail = 1;
                  fgOpenPCFile(u4InstID, _bFileStr1[u4InstID][1],"r+b", &_tInFileInfo[u4InstID]);
                }
          #elif defined(IDE_READ_SUPPORT)
                   fgOpenIdeFile(_bFileStr1[u4InstID][1],"r+b", &_tInFileInfo[u4InstID]);
          #else
                fgOpenPCFile(u4InstID, _bFileStr1[u4InstID][1],"r+b", &_tInFileInfo[u4InstID]);
          #endif
                if (!bReadFail){
                    _tInFileInfo[u4InstID].u4FileLength = _tInFileInfo[u4InstID].u4FileOffset + _tInFileInfo[u4InstID].u4RealGetBytes;
                    _u4LoadBitstreamCnt[u4InstID]++;
                    printk("[INFO] u4FileOffset =  0x%08X; Read size = 0x%08X bytes\n",_tInFileInfo[u4InstID].u4FileOffset  ,_tInFileInfo[u4InstID].u4RealGetBytes);
                } else {
                    _u4LoadBitstreamCnt[u4InstID]++;
                }
            
          }
          else if((!(_u4LoadBitstreamCnt[u4InstID]&0x1)) && (rH265BSInitPrm.u4VLDRdPtr < 
          ((UINT32)_pucVFifo[u4InstID] + (V_FIFO_SZ/2) + 16 )))     // HEVC read pointer precise to 16 bytes
          {
            _tInFileInfo[u4InstID].fgGetFileInfo = TRUE;
            _tInFileInfo[u4InstID].pucTargetAddr = _pucVFifo[u4InstID] + (V_FIFO_SZ/2);
            _tInFileInfo[u4InstID].u4FileOffset = ((V_FIFO_SZ * (_u4LoadBitstreamCnt[u4InstID]+ 1))/2);
            _tInFileInfo[u4InstID].u4TargetSz = (V_FIFO_SZ/2);

          #ifdef  SATA_HDD_READ_SUPPORT
                if(!fgOpenHDDFile(u4InstID, _bFileStr1[u4InstID][1],"r+b", &_tInFileInfo[u4InstID]))
                {
                  bReadFail = 1;
                  fgOpenPCFile(u4InstID, _bFileStr1[u4InstID][1],"r+b", &_tInFileInfo[u4InstID]);
                }
          #elif defined(IDE_READ_SUPPORT)
                   fgOpenIdeFile(_bFileStr1[u4InstID][1],"r+b", &_tInFileInfo[u4InstID]);
          #else
                fgOpenPCFile(u4InstID, _bFileStr1[u4InstID][1],"r+b", &_tInFileInfo[u4InstID]);
          #endif
                if (!bReadFail){
                    _tInFileInfo[u4InstID].u4FileLength = _tInFileInfo[u4InstID].u4FileOffset + _tInFileInfo[u4InstID].u4RealGetBytes;
                    _u4LoadBitstreamCnt[u4InstID]++;
                    printk("[INFO] u4FileOffset =  0x%08X; Read size = 0x%08X bytes\n",_tInFileInfo[u4InstID].u4FileOffset  ,_tInFileInfo[u4InstID].u4RealGetBytes);
                } else {
                    _u4LoadBitstreamCnt[u4InstID]++;
                }
            
          }
          if((0 ==_tInFileInfo[u4InstID].u4RealGetBytes) ||
            (V_FIFO_SZ/2 != _tInFileInfo[u4InstID].u4RealGetBytes) )
            {
                //vAddStartCode2Dram(_pucVFifo+_tInFileInfo.u4FileLength);
                UCHAR *pbDramAddr = _tInFileInfo[u4InstID].pucTargetAddr+_tInFileInfo[u4InstID].u4RealGetBytes;
        
                pbDramAddr[0] = 0x00; pbDramAddr++;
                if((UINT32)(_pucVFifo[u4InstID] + V_FIFO_SZ) <= (UINT32)pbDramAddr) 
                {
                    pbDramAddr = _pucVFifo[u4InstID];
                }
                pbDramAddr[0] = 0x00; pbDramAddr++;
                if((UINT32)(_pucVFifo[u4InstID] + V_FIFO_SZ) <= (UINT32)pbDramAddr) 
                {
                    pbDramAddr = _pucVFifo[u4InstID];
                }
                pbDramAddr[0] = 0x01;  
            }
#endif
#endif
    }

    if ( _u4VerBitCount[u4InstID] != 0xFFFFFFFF ){
        u4CurrentPtr = u4VDEC_HAL_H265_ReadRdPtr(_u4BSID[u4InstID], u4InstID, (UINT32)_pucVFifo[u4InstID], u4Bit);
        u4CurrentPtr = (u4CurrentPtr<<3) + (*u4Bit);
        if(u4CurrentPtr < _u4PrevPtr[u4InstID])//HW is ring,so read fifo overflow
        {
          printk("[INFO] HW decode overflow!! u4CurrentPtr = 0x%x(bits), u4PrevPtr = 0x%x(bits), VFIFO = 0x%x(bytes)\n",u4CurrentPtr,_u4PrevPtr[u4InstID], V_FIFO_SZ);
             _u4VerBitCount[u4InstID] += (u4CurrentPtr + (V_FIFO_SZ<<3) -_u4PrevPtr[u4InstID] );

        } else {
             _u4VerBitCount[u4InstID] += (u4CurrentPtr -_u4PrevPtr[u4InstID] );
        }
        printk("[INFO] Decoded BitCount = 0x%08X, FileLength = 0x%08X, LoadBitstreamCnt = %d\n", _u4VerBitCount[u4InstID], _tInFileInfo[u4InstID].u4FileLength<< 3,  _u4LoadBitstreamCnt[u4InstID]);
        _u4PrevPtr[u4InstID] = u4CurrentPtr;
    }
    vfree(u4Bit);

}
// *********************************************************************
// Function    : void vH264WrData2PC(UINT32 u4InstID, UINT32 u4Size)
// Description : Write the decoded data to PC for compare
// Parameter   : None
// Return      : None
// *********************************************************************
void vH264WrData2PC(UINT32 u4InstID, UCHAR *ptAddr, UINT32 u4Size)
{
    #if ((!defined(COMP_HW_CHKSUM)) || defined(DOWN_SCALE_SUPPORT) || defined(FGT_SUPPORT))
    UINT32 u4Cnt;
    UINT32 u4Upd;
    UINT32 u4Pic32XSize;
    UCHAR *pbDecBuf;
    INT32 i;
    #endif
    BOOL fgDecErr,fgOpen;
    char strMessage[256];
    UINT32 *pu4WorkBuf;
    UINT32 *pu4GoldenBuf;
    UINT32 u4Idx;

    #ifndef DOWN_SCALE_SUPPORT               
       UINT32 u4NonSwapYBase, u4NonSwapCBase;   
     UINT32 u4SwapSrcY, u4SwapSrcC;
     UINT32 u4NonSwapTargY, u4NonSwapTargC;
    #endif   

     UINT32 u4AlignWidth, u4AlignHeight;               
     UINT32 u4AlignSize = 0;                   
     UINT32 u4XPix, u4YPix;
     CHAR *pucWorkBuf, *pucGoldenBuf;
     #ifdef VDEC_DDR3_SUPPORT
	 UINT32 u4DDR3AlignWidth;
     #endif

     if(_ucMVCType[u4InstID] > 0) 
     {
          for (i=0; ; i++)
         {
          if (_bFileStr1[u4InstID][1][i] == '\0')
          {
          	_bFileStr1[u4InstID][0][i] = '_';
              _bFileStr1[u4InstID][0][i+1] = '\0';
          	break;
          }
          else
          if (_bFileStr1[u4InstID][1][i] == '.')
          {
              if( (_bFileStr1[u4InstID][1][i+1] == '2')
              && (_bFileStr1[u4InstID][1][i+2] == '6')
              && (_bFileStr1[u4InstID][1][i+3] == '4'))
              {
                  _bFileStr1[u4InstID][0][i] = '_';
                  _bFileStr1[u4InstID][0][i+1] = '\0';
                  break;
              }
              else
              {
                  _bFileStr1[u4InstID][0][i] =  _bFileStr1[u4InstID][1][i]; 
              }
           }
           else
           {
                _bFileStr1[u4InstID][0][i] =  _bFileStr1[u4InstID][1][i]; 
           }
        }
     }
   

    strncpy(_tFileListRecInfo[u4InstID].bFileName, _FileList_Rec[u4InstID], sizeof(_FileList_Rec[u4InstID]));

    if(_ucMVCType[u4InstID] == 1 && _u4FileCnt[u4InstID] %10 == 0)       
    	  printk("[Base] FrmCnt = %d\n", (_u4FileCnt[u4InstID]<<_fgMVCReady[u4InstID]) + (_ucMVCType[u4InstID] - _fgMVCReady[u4InstID]));    
    else if(_ucMVCType[u4InstID] == 2 && _u4FileCnt[u4InstID] %10 == 0)       
         printk("[Dep] FrmCnt = %d\n", (_u4FileCnt[u4InstID]<<_fgMVCReady[u4InstID]) + (_ucMVCType[u4InstID] - _fgMVCReady[u4InstID]));    
    else if (_u4FileCnt[u4InstID] %10 == 0)
       printk("[AVC] FrmCnt = %d\n", _u4FileCnt[u4InstID]);

    #ifdef REDEC    
    if(_u4ReDecCnt[u4InstID] == 0)
    #endif  
    {
        vClrDecFlag(u4InstID, DEC_FLAG_CHG_FBUF);
    }
    if((_ptCurrFBufInfo[u4InstID]->ucFBufStatus != FRAME)
    && (_ptCurrFBufInfo[u4InstID]->ucFBufStatus == _ucPrevDecFld[u4InstID]))
    {
        #ifdef REDEC    
        if(_u4ReDecCnt[u4InstID] == 0)
        #endif    
        {
            _u4FileCnt[u4InstID] ++;
            vSetDecFlag(u4InstID, DEC_FLAG_CHG_FBUF);
        }
    }

    #ifdef REDEC  
    if(_u4FileCnt[u4InstID] == _u4ReDecPicNum[u4InstID] )
    {
        if(_u4ReDecNum[u4InstID] != 0)
        {
            _u4ReDecPicNum[u4InstID] = 0xFFFFFFFF;
            _u4ReDecCnt[u4InstID] = _u4ReDecNum[u4InstID];
            vVDecOutputDebugString("/n!!!!!!!!!!!!!! Re-Decode and Wait for debug !!!!!!!!!!!!!!!!\n");
        }
    }
    if(_u4ReDecCnt[u4InstID] > 0)
    {
        _u4ReDecCnt[u4InstID]--;
    }
    #endif

    fgDecErr = FALSE;
///TODO:    BSP_InvDCacheRange((UINT32)_pucDPB[u4InstID],DPB_SZ);
    #ifdef GEN_HW_CHKSUM
    #ifndef INTERGRATION_WITH_DEMUX
    vWrite2PC(u4InstID, 9, NULL);
    #endif
    #endif


    #ifndef INTERGRATION_WITH_DEMUX
    #if (defined(COMP_HW_CHKSUM) && (!defined(DOWN_SCALE_SUPPORT)) && (!defined(FGT_SUPPORT)))
    if(!fgCompH264ChkSumGolden(u4InstID))
    {
        fgDecErr = TRUE;
        vVDecOutputDebugString("Check sum comparison mismatch\n");
    }
    #else // compare pixel by pixel
    
           #ifdef DOWN_SCALE_SUPPORT
                    //vGenerateDownScalerGolden(u4InstID, (UINT32)_pucDecWorkBuf[u4InstID],(UINT32)(_pucDecCWorkBuf[u4InstID]),u4Size);
           #else
            u4NonSwapYBase = (UINT32)_pucDecWorkBuf[u4InstID];
            u4NonSwapCBase = (UINT32)(_pucDecWorkBuf[u4InstID]+_ptCurrFBufInfo[u4InstID]->u4DramPicSize);

           if (_tVerMpvDecPrm[u4InstID].ucAddrSwapMode != ADDRSWAP_OFF)
           {                         
                   u4NonSwapYBase = (UINT32)_pucDecWorkBuf[u4InstID];
                   u4NonSwapCBase = (UINT32)_pucDecCWorkBuf[u4InstID];

                   u4AlignWidth = _tVerMpvDecPrm[u4InstID].u4PicW;
                   u4AlignWidth = (((u4AlignWidth +63) >>6) <<6); //Align to 4MB width                    
                   u4AlignHeight = _tVerMpvDecPrm[u4InstID].u4PicH;
                   u4AlignHeight =  (((u4AlignHeight +31) >>5) <<5);              
                    
                   u4SwapSrcY = (UINT32)_pucDecWorkBuf[u4InstID];
                   u4SwapSrcC = (UINT32)(_pucDecWorkBuf[u4InstID]+_ptCurrFBufInfo[u4InstID]->u4DramPicSize);

                   u4NonSwapTargY = (UINT32)_pucAddressSwapBuf[u4InstID];
                   u4NonSwapTargC = (UINT32)_pucAddressSwapBuf[u4InstID] + 0x1FE000;
                    vMPEG_InvAddressSwap(u4InstID, 
                                                           (BYTE*) u4SwapSrcY, (BYTE*) u4SwapSrcC, 
                                                           (BYTE*) u4NonSwapTargY, (BYTE*) u4NonSwapTargC,
                                                          u4AlignWidth,  u4AlignHeight, u4AlignSize,
                                                          _tVerMpvDecPrm[u4InstID].ucAddrSwapMode);
                    
                    u4NonSwapYBase = u4NonSwapTargY;
                    u4NonSwapCBase = u4NonSwapTargC;
           }

          #endif
    
    // Y compare
    _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tFBufFileInfo[u4InstID].u4FileLength = 0;  
    // Y decoded data Compare
#if (!VDEC_MVC_SUPPORT)
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], _u4FileCnt[u4InstID]);
#else
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], (_u4FileCnt[u4InstID]<<_fgMVCReady[u4InstID]) + (_ucMVCType[u4InstID] - _fgMVCReady[u4InstID]));
#endif
    #ifdef EXT_COMPARE 
    _tFBufFileInfo[u4InstID].u4FileLength = (((_ptCurrFBufInfo[u4InstID]->u4W + 15)>>4)<<4) *(((_ptCurrFBufInfo[u4InstID]->u4H + 31)>>5)<<5);
    #else
    #ifdef DIRECT_DEC
    if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
    #endif    
    {
        #ifdef DOWN_SCALE_SUPPORT
        #ifdef FGT_SUPPORT
        vGenerateDownScalerGolden(u4InstID, (UINT32)_pucFGTBuf[u4InstID],(UINT32)(_pucFGTBuf[u4InstID]+_ptCurrFBufInfo[u4InstID]->u4DramPicSize),u4Size);
        #else
        vGenerateDownScalerGolden(u4InstID, (UINT32)_pucDecWorkBuf[u4InstID],(UINT32)(_pucDecWorkBuf[u4InstID]+_ptCurrFBufInfo[u4InstID]->u4DramPicSize),u4Size);
        #endif
        #else

        fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);         
        if (fgOpen == FALSE)
        {
             if(_u4FileCnt[u4InstID] == 0)
            {
                sprintf(strMessage,"Pattern List File Wrong!!!!\n");   
                printk("%s",strMessage);

                if(fgIsDecFlagSet(u4InstID, DEC_FLAG_CHG_FBUF))
                {
                    _ptCurrFBufInfo[u4InstID]->eH264DpbStatus = H264_DPB_STATUS_DECODED;
                    _ptCurrFBufInfo[u4InstID]->u4DecOrder = _u4TotalDecFrms[u4InstID];
                }
                vFlushDPB(u4InstID, &_tVerMpvDecPrm[u4InstID], FALSE);
                _u4VerBitCount[u4InstID] = 0xffffffff;
                return ;
            }
            else
            {
                sprintf(strMessage," Open Golden Fail ==> Pic count to [%d] \n", _u4FileCnt[u4InstID]);     
                printk("%s",strMessage);
                return;
            }
        }
        
        _u4GoldenYSize[u4InstID] = _tFBufFileInfo[u4InstID].u4RealGetBytes;

        if (_ucMVCType[u4InstID] > 0 && _tVerMpvDecPrm[u4InstID].u4PicW == 1280 && _tVerMpvDecPrm[u4InstID].u4PicH == 736)
        {
            if (_u4GoldenYSize[u4InstID] > 1280*704)
            _u4GoldenYSize[u4InstID] = 1280*704;
        }
        #endif
    }
    #endif

    #ifdef DOWN_SCALE_SUPPORT
    if((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD) ||(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD))
    {
        u4Upd = (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight*2) % 32;
        u4Pic32XSize = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgWidth * ((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight*2) - u4Upd);
    }
    else
    {
        u4Upd = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight % 32;
        u4Pic32XSize = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgWidth * (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight - u4Upd);
    }
    #else
    u4Upd = _ptCurrFBufInfo[u4InstID]->u4H % 32; // 16 or 0
    u4Pic32XSize = _ptCurrFBufInfo[u4InstID]->u4W * (_ptCurrFBufInfo[u4InstID]->u4H - u4Upd);
    #endif

    u4Cnt = 0; 
    #if defined(NO_COMPARE)
    #elif defined(EXT_COMPARE)
    if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
    {
        #ifdef DIRECT_DEC
        if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
        #endif    
        {
            vWrite2PC(u4InstID, 5, _pucDecWorkBuf[u4InstID]);
        }
    }
    #else
    #if defined(DOWN_SCALE_SUPPORT)
    pbDecBuf = _pucVDSCLBuf[u4InstID];
    if((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD) ||(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD))
    {
        _u4GoldenYSize[u4InstID] = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW*((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight*2)+_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV);
    }
    else
    {
        _u4GoldenYSize[u4InstID] = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW*(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight+_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV);
    }
    #elif defined(FGT_SUPPORT)
    pbDecBuf = _pucFGTBuf[u4InstID];
    #else
    //pbDecBuf = _pucDecWorkBuf[u4InstID];
    pbDecBuf = (UCHAR*)u4NonSwapYBase;
    #endif  
	#if VMMU_SUPPORT
	pu4WorkBuf = (UINT32 *)((UINT32)pbDecBuf) + (0x1000 >> 2);
	#else
    pu4WorkBuf = (UINT32 *)((UINT32)pbDecBuf);
	#endif
    pu4GoldenBuf = (UINT32 *)((UINT32)_pucDumpYBuf[u4InstID]);

    #ifdef DIRECT_DEC
    if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
    #endif    
    {

     if (_tVerMpvDecPrm[u4InstID].ucAddrSwapMode == ADDRSWAP_OFF)
     //if (1) // Cheng-Jung 20120904 Uncomment to dump error frame
     {    
        printk("@@__ADDRSWAP_OFF GoldenYSize = %d  : u4Size = %d \n",_u4GoldenYSize[u4InstID], u4Size);
        for(u4Idx=0; u4Idx< _u4GoldenYSize[u4InstID]>>2; u4Idx++)
        {
            i = (u4Idx << 2);
            #ifdef DOWN_SCALE_SUPPORT
            //raster scan mode
            if(1 == _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType)
            {
                if((_tVerMpvDecPrm[u4InstID].ucPicStruct == BOTTOM_FIELD) && ((i%_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen) == 0) )
                {      
                    // Skip Top lines
                    i += _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen;
                }  
            }
            else
            {
                if((_tVerMpvDecPrm[u4InstID].ucPicStruct == BOTTOM_FIELD) && ((i%16) == 0) && (!((i>>4)%2)))
                {      
                    // Skip Top lines
                    i += 16;
                }  
            }
            #else
            if((_tVerMpvDecPrm[u4InstID].ucPicStruct == BOTTOM_FIELD) && ((i%16) == 0) && (!((i>>4)%2)))
            {      
                // Skip Top lines
                i += 16;
            }
            #endif  
            u4Idx = i >> 2;
            //if((_pucDumpYBuf[u4InstID][i] != pbDecBuf[i]))
            if((pu4GoldenBuf[u4Idx] != pu4WorkBuf[u4Idx]))
            //if (1) // Cheng-Jung 20120904 Uncomment to dump error frame
            {
                u4Cnt ++;
                sprintf(strMessage,"Y Data Mismatch at [%d] = 0x%X, Golden = 0x%X !!\n", i, pu4WorkBuf[u4Idx],pu4GoldenBuf[u4Idx]);
                printk("%s",strMessage);
                fgDecErr = TRUE;
                vVDEC_HAL_H264_VDec_DumpReg(u4InstID);  // mark by ginny
                //vWrite2PC(u4InstID, 11, _pucDecWorkBuf[u4InstID]); // dump check sum
                //vWrite2PC(u4InstID, 2, pbDecBuf);
                //#if defined(FGT_SUPPORT) || defined(DOWN_SCALE_SUPPORT)
                //vWrite2PC(u4InstID, 5, _pucDecWorkBuf[u4InstID]);
                //#endif        
                    #if 1  //dump raw data
                      //Y
                      printk("[H264] Y data  = 0x%x (0x%x), size %d\n", pu4WorkBuf, PHYSICAL(pu4WorkBuf), _u4GoldenYSize[u4InstID]);
                      DBG_H264_PRINTF("[H264] Y data  = 0x%x (0x%x), size %d\n", pu4WorkBuf, PHYSICAL(pu4WorkBuf), _u4GoldenYSize[u4InstID]);
                      sprintf(_output_file_name, "D:\\sandbox\\DPB_Y_%d_0x%x_Size0x%x.raw", _u4FileCnt[u4InstID] - 1,PHYSICAL(pu4WorkBuf),_u4GoldenYSize[u4InstID]);
                      printk("[H264] [H264] Y data => %s\n", _output_file_name);
                      DBG_H264_PRINTF("[H264] [H264] Y data => %s\n", _output_file_name);
                      #if _FANTASIA_WRITE_OUT_DATA_
                      fgWrData2PC((void*)pu4WorkBuf,_u4GoldenYSize[u4InstID],7,_output_file_name);
                      #endif
                      //CbCr
                      /*
                      _u4GoldenCSize[u4InstID] = _u4GoldenYSize[u4InstID] / 2;
                      printk("[H264] CbCr data  = 0x%x (0x%x), size %d\n", pu4WorkBuf+(u4Size>>2), PHYSICAL(pu4WorkBuf+(u4Size>>2)), _u4GoldenCSize[u4InstID]);
                      DBG_H264_PRINTF("[H264] CbCr data  = 0x%x (0x%x), size %d\n", pu4WorkBuf+(u4Size>>2), PHYSICAL(pu4WorkBuf+(u4Size>>2)), _u4GoldenCSize[u4InstID]);
                      sprintf(_output_file_name, "D:\\sandbox\\DPB_CbCr_%d_0x%x_Size0x%x.raw", _u4FileCnt[u4InstID] - 1, PHYSICAL(pu4WorkBuf+(u4Size>>2)),_u4GoldenCSize[u4InstID]);
                      printk("[H264] [H264] CbCr data => %s\n", _output_file_name);
                      DBG_H264_PRINTF("[H264] [H264] CbCr data => %s\n", _output_file_name);
                      */
                      _u4GoldenCSize[u4InstID] = _u4GoldenYSize[u4InstID] / 2;
                      printk("[H264] CbCr data  = 0x%x (0x%x), size %d\n", pu4WorkBuf+u4Size, PHYSICAL(pu4WorkBuf+u4Size), _u4GoldenCSize[u4InstID]);
                      DBG_H264_PRINTF("[H264] CbCr data  = 0x%x (0x%x), size %d\n", pu4WorkBuf+u4Size, PHYSICAL(pu4WorkBuf+u4Size), _u4GoldenCSize[u4InstID]);
                      sprintf(_output_file_name, "D:\\sandbox\\DPB_CbCr_%d_0x%x_Size0x%x.raw", _u4FileCnt[u4InstID] - 1, PHYSICAL(pu4WorkBuf+u4Size),_u4GoldenCSize[u4InstID]);
                      printk("[H264] [H264] CbCr data => %s\n", _output_file_name);
                      DBG_H264_PRINTF("[H264] [H264] CbCr data => %s\n", _output_file_name);
                        
                      #if _FANTASIA_WRITE_OUT_DATA_
                      fgWrData2PC((void*)pu4WorkBuf+u4Size,_u4GoldenCSize[u4InstID],7,_output_file_name);
                      #endif
                    #endif
                break;        
            }
            i+=3;
            #ifdef DOWN_SCALE_SUPPORT
            //raster scan mode
            if(1 == _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType)
            {
                if((_tVerMpvDecPrm[u4InstID].ucPicStruct == TOP_FIELD) && ((i%_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen) == (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen-1)) )
                {      
                    // Skip bottom lines
                    i += _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen;
                }  
            }
            else
            {
                if(u4Upd && (i>u4Pic32XSize) && ((i-u4Pic32XSize)%512 == ((u4Upd << 4)-1)) && (fgIsFrmPic(u4InstID)))
                {
                    i += ((32 - u4Upd) << 4);
                }    
                else if(u4Upd && (i>u4Pic32XSize) && (i%256 == 255) && (!fgIsFrmPic(u4InstID)))
                {
                    i += 256;
                }
                if((_tVerMpvDecPrm[u4InstID].ucPicStruct == TOP_FIELD) && ((i%16) == 15) && (!((i>>4)%2)))
                {      
                    // Skip bottom lines
                    i += 16;
                }
            }
            #else
            if(u4Upd && (i>u4Pic32XSize) && ((i-u4Pic32XSize)%512 == ((u4Upd << 4)-1)) && (fgIsFrmPic(u4InstID)))
            {
                i += ((32 - u4Upd) << 4);
            }    
            else if(u4Upd && (i>u4Pic32XSize) && (i%256 == 255) && (!fgIsFrmPic(u4InstID)))
            {
                i += 256;
            }
            // odd lines for top, even lines for bottom
            if((_tVerMpvDecPrm[u4InstID].ucPicStruct == TOP_FIELD) && ((i%16) == 15) && (!((i>>4)%2)))
            {      
                // Skip bottom lines
                i += 16;
            }
            #endif
            u4Idx = i >> 2;
        }
     }
     else
     {
         if (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRAME )
         {
            #ifndef VDEC_DDR3_SUPPORT
            u4AlignWidth = ((_tVerMpvDecPrm[u4InstID].u4PicW+ 15) >> 4) << 4;
	     #else
	     u4AlignWidth = ((_tVerMpvDecPrm[u4InstID].u4PicW+ 15) >> 4) << 4;
            u4DDR3AlignWidth = _tVerMpvDecPrm[u4InstID].u4PicW;
            u4DDR3AlignWidth = (((u4DDR3AlignWidth +63) >>6) <<6); //Align to 4MB width
            #endif

            printk("@@ Y FRAME W : H  = %d : %d \n",_tVerMpvDecPrm[u4InstID].u4PicW, _tVerMpvDecPrm[u4InstID].u4PicH);
            
            for (u4XPix = 0; u4XPix < _tVerMpvDecPrm[u4InstID].u4PicW; u4XPix++)
            {
                for (u4YPix = 0; u4YPix < _tVerMpvDecPrm[u4InstID].u4PicH; u4YPix++)
                {                                        
                	   #ifndef VDEC_DDR3_SUPPORT
                        pucWorkBuf = (UCHAR*)u4CalculatePixelAddress_Y((UINT32) pbDecBuf, u4XPix, u4YPix, u4AlignWidth, 1, 4);
 			   #else
			   pucWorkBuf = (UCHAR*)u4CalculatePixelAddress_Y((UINT32) pbDecBuf, u4XPix, u4YPix, u4DDR3AlignWidth, 1, 4);
			   #endif
                        pucGoldenBuf = (UCHAR*)u4CalculatePixelAddress_Y((UINT32) _pucDumpYBuf[u4InstID], u4XPix, u4YPix, u4AlignWidth, 1, 4);
                        if ((*(pucWorkBuf)) != (*(pucGoldenBuf)))
                        {
                            u4Cnt ++;
				printk("FRAME, u4InstID = %x \n",u4InstID);
                            vVDecOutputDebugString("Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                            sprintf(strMessage,"Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                            vVDecOutputDebugString("Y Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pucWorkBuf), (*pucGoldenBuf));                            
				sprintf(strMessage,"Y Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pucWorkBuf), (*pucGoldenBuf));
				printk("%s \n",strMessage);
                            fgDecErr = TRUE;
                            break;    
                        }                    
                }
                if(fgDecErr == TRUE)
                {
                    break;
                }
            }
         }        
     }
    }
    #endif
    if (fgDecErr == FALSE) {
        printk("Y compare pass\n");
    }

    // CbCr compare
    if(!fgIsMonoPic(u4InstID))
    {
        // CbCr decoded data Compare
        _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
        _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpCBuf[u4InstID];
        _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_C_SZ;  
        _tFBufFileInfo[u4InstID].u4FileLength = 0;
#if (!VDEC_MVC_SUPPORT)
        vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], _bFileAddStrC[0], _u4FileCnt[u4InstID]);
#else
        vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], _bFileAddStrC[0], (_u4FileCnt[u4InstID]<<_fgMVCReady[u4InstID]) + (_ucMVCType[u4InstID] - _fgMVCReady[u4InstID]));
#endif
        #if defined(NO_COMPARE)
        #elif defined(EXT_COMPARE)
        _tFBufFileInfo[u4InstID].u4FileLength = (((_ptCurrFBufInfo[u4InstID]->u4W + 15)>>4)<<4) *(((_ptCurrFBufInfo[u4InstID]->u4H + 31)>>5)<<5) >>1;
        #else
        #ifdef DIRECT_DEC
        if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
        #endif    
        {  
            #ifndef DOWN_SCALE_SUPPORT
            fgOpenFile(u4InstID, _bFileStr1[u4InstID][4],"r+b", &_tFBufFileInfo[u4InstID]);
            _u4GoldenCSize[u4InstID] = _tFBufFileInfo[u4InstID].u4RealGetBytes;

            if (_ucMVCType[u4InstID] > 0 && _tVerMpvDecPrm[u4InstID].u4PicW == 1280 && _tVerMpvDecPrm[u4InstID].u4PicH == 736)
            {
                if (_u4GoldenCSize[u4InstID] > 1280*704 / 2)
                   _u4GoldenCSize[u4InstID] = 1280*704 / 2;
            }            
            #endif
        }
        #endif    
        #ifdef DOWN_SCALE_SUPPORT
        //u4Upd = (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight >> 1) % 16;
        #else
        //u4Upd = (_ptCurrFBufInfo->u4H >> 1) % 16;
        #endif  
        u4Pic32XSize >>= 1;    
        u4Cnt = 0; 
        #ifdef EXT_COMPARE  
        if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
        {
            #ifdef DIRECT_DEC
            if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
            #endif 
            {
                vWrite2PC(u4InstID, 6, _pucDecWorkBuf[u4InstID]+u4Size);
            }
        }
        #else    
        #if defined(DOWN_SCALE_SUPPORT)
        u4Size = 0x1FE000;
        pbDecBuf = _pucVDSCLBuf[u4InstID];
        if(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt==0)
        {
            if((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD) ||(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD))
            {
                _u4GoldenCSize[u4InstID] = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW*((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight*2)+_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV)/2;
            }
            else
            {
                _u4GoldenCSize[u4InstID] = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW*(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight+_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV)/2;
            }
        }
        else
        {
            if((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD) ||(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD))
            {
                _u4GoldenCSize[u4InstID] = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW*((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight*2)+_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV);
            }
            else
            {
                _u4GoldenCSize[u4InstID] = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4DispW*(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight+_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV);
            }
        }
        #elif defined(FGT_SUPPORT)
        pbDecBuf = _pucFGTBuf[u4InstID];
        #else
        pbDecBuf = (UCHAR*)u4NonSwapYBase;

        if (_tVerMpvDecPrm[u4InstID].ucAddrSwapMode != ADDRSWAP_OFF)
        {
           u4Size = 0x1FE000;
        }
        #endif  
        pu4WorkBuf = (UINT32 *)((UINT32)pbDecBuf);
        pu4GoldenBuf = (UINT32 *)((UINT32)_pucDumpCBuf[u4InstID]);

        #ifdef DIRECT_DEC
        if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
        #endif      
        {

           if (_tVerMpvDecPrm[u4InstID].ucAddrSwapMode == ADDRSWAP_OFF)
           {
           printk("@@__ADDRSWAP_OFF GoldenCBCrSize = %d : u4Size = %d\n",_u4GoldenCSize[u4InstID], u4Size);
           	
            for(u4Idx=0; u4Idx< _u4GoldenCSize[u4InstID]>>2; u4Idx++)
            {
                i = (u4Idx << 2);
                #ifdef DOWN_SCALE_SUPPORT
                //raster scan mode
                if(1 == _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType)
                {
                    if((_tVerMpvDecPrm[u4InstID].ucPicStruct == BOTTOM_FIELD) && ((i%_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen) == 0) )
                    {      
                        // Skip Top lines
                        i += _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen;
                    }  
                }
                else
                {
                    if((_tVerMpvDecPrm[u4InstID].ucPicStruct == BOTTOM_FIELD) && ((i%16) == 0) && (!((i>>4)%2)))
                    {      
                        // Skip Top lines
                        i += 16;
                    }  
                }
                #else
                if((_tVerMpvDecPrm[u4InstID].ucPicStruct == BOTTOM_FIELD) && ((i%16) == 0) && (!((i>>4)%2)))
                {      
                    // Skip Top lines
                    i += 16;
                }     
                #endif

                u4Idx = i >> 2;
                #if defined(DOWN_SCALE_SUPPORT)  
                if(pu4GoldenBuf[u4Idx] != pu4WorkBuf[u4Idx + (u4Size>>2)])
                #else
                if(pu4GoldenBuf[u4Idx] != pu4WorkBuf[u4Idx + (u4Size>>2)])
                #endif
                {
                    printk("CbCr buffer address = 0x%x,offset = 0x%x\n",(UINT32)pu4WorkBuf,(u4Idx + (u4Size>>2)<<2));
                    u4Cnt ++ ;
                    sprintf(strMessage,"CbCr Data Mismatch at [%d] = 0x%X, Golden = 0x%X !!! \n", i, pu4WorkBuf[u4Idx + (u4Size>>2)], pu4GoldenBuf[u4Idx]);
                    printk("%s",strMessage);
                    fgDecErr = TRUE;        
                    vVDEC_HAL_H264_VDec_DumpReg(u4InstID);     // mark by ginny   
                    //vWrite2PC(u4InstID, 11, _pucDecWorkBuf[u4InstID]+u4Size); // dump check sum
                    //vWrite2PC(u4InstID, 3, pbDecBuf+u4Size);
                    //#if defined(FGT_SUPPORT) || defined(DOWN_SCALE_SUPPORT)
                    //vWrite2PC(u4InstID, 6, _pucDecWorkBuf[u4InstID]+u4Size);
                    //#endif
                    break;        
                }
                i+=3;
                #ifdef DOWN_SCALE_SUPPORT
                //raster scan mode
                if(1 == _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType)
                {
                    if((_tVerMpvDecPrm[u4InstID].ucPicStruct == TOP_FIELD) && ((i%_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen) == (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen-1)) )
                    {      
                        // Skip Top lines
                        i += _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen;
                    }  
                }
                else
                {
                    if(u4Upd && (i>u4Pic32XSize) && ((i-u4Pic32XSize)%256 == ((u4Upd << 3)-1)) && fgIsFrmPic(u4InstID))
                    {
                        i += ((16 - (u4Upd>>1)) << 4); // remainder x 16
                    }    
                    else if(u4Upd && (i>u4Pic32XSize) && (i%128 == 127) && (!fgIsFrmPic(u4InstID)))
                    {
                        i += 128;
                    }     
                    if((_tVerMpvDecPrm[u4InstID].ucPicStruct == TOP_FIELD) && ((i%16) == 15) && (!((i>>4)%2)))
                    {      
                        // Skip bottom lines
                        i += 16;
                    }
                }
                #else
                if(u4Upd && (i>u4Pic32XSize) && ((i-u4Pic32XSize)%256 == ((u4Upd << 3)-1)) && fgIsFrmPic(u4InstID))
                {
                    i += ((16 - (u4Upd>>1)) << 4); // remainder x 16
                }    
                else if(u4Upd && (i>u4Pic32XSize) && (i%128 == 127) && (!fgIsFrmPic(u4InstID)))
                {
                    i += 128;
                }          

                // odd lines for top, even lines for bottom
                if((_tVerMpvDecPrm[u4InstID].ucPicStruct == TOP_FIELD) && ((i%16) == 15) && (!((i>>4)%2)))
                {      
                    // Skip bottom lines
                    i += 16;
                }
                #endif
                u4Idx = i >> 2;
            }
           }
           else
           {
               if (_tVerMpvDecPrm[u4InstID].ucPicStruct == FRAME )
               {
                   #ifndef VDEC_DDR3_SUPPORT
                   u4AlignWidth = ((_tVerMpvDecPrm[u4InstID].u4PicW+ 15) >> 4) << 4;
		     #else
                   u4AlignWidth = ((_tVerMpvDecPrm[u4InstID].u4PicW+ 15) >> 4) << 4;
                   u4DDR3AlignWidth = _tVerMpvDecPrm[u4InstID].u4PicW;
                   u4DDR3AlignWidth = (((u4DDR3AlignWidth +63) >>6) <<6); //Align to 4MB width
                   #endif

                    printk("@@  CBcr FRAME W : H  = %d : %d \n",_tVerMpvDecPrm[u4InstID].u4PicW, _tVerMpvDecPrm[u4InstID].u4PicH);
                   
                  for (u4XPix = 0; u4XPix < _tVerMpvDecPrm[u4InstID].u4PicW; u4XPix++)
                  {
                     for (u4YPix = 0; u4YPix < _tVerMpvDecPrm[u4InstID].u4PicH; u4YPix++)
                     {                        
                            // compare Cb
                            #ifndef VDEC_DDR3_SUPPORT
                            pucWorkBuf = (UCHAR*)u4CalculatePixelAddress_C((INT32)(pbDecBuf+u4Size), u4XPix, u4YPix, u4AlignWidth, 1, 1, 4);
				#else
				pucWorkBuf = (UCHAR*)u4CalculatePixelAddress_C((INT32)(pbDecBuf+u4Size), u4XPix, u4YPix, u4DDR3AlignWidth, 1, 1, 4);
				#endif
                            pucGoldenBuf = (UCHAR*)u4CalculatePixelAddress_C((UINT32)_pucDumpCBuf[u4InstID], u4XPix, u4YPix, u4AlignWidth, 1, 1, 4);
                            if ((*(pucWorkBuf)) != (*(pucGoldenBuf)))
                            {
                                u4Cnt ++;
                                vVDecOutputDebugString("Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                sprintf(strMessage,"Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                vVDecOutputDebugString("Cb Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pucWorkBuf), (*pucGoldenBuf));
                                sprintf(strMessage,"Cb Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pucWorkBuf), (*pucGoldenBuf));
                                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                fgDecErr = TRUE; 
                                break;    
                            }
                            // compare Cr
                            pucWorkBuf++;
                            pucGoldenBuf++;
                            if ((*(pucWorkBuf)) != (*(pucGoldenBuf)))
                            {
                                u4Cnt ++;
                                vVDecOutputDebugString("Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                sprintf(strMessage,"Current Pic Cnt = %d !!!\n", _u4FileCnt[u4InstID]);
                                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                vVDecOutputDebugString("Cr Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pucWorkBuf), (*pucGoldenBuf));
                                sprintf(strMessage,"Cr Data Mismatch at [x= 0x%.8x, y=0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", u4XPix, u4YPix, (*pucWorkBuf), (*pucGoldenBuf));
                                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                                //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);
                                fgDecErr = TRUE;    
                                break;    
                            }                        
                    }
                    if(fgDecErr == TRUE)
                    {
                        break;
                    }
                 }
               }
           }
            //vVDecOutputDebugString("CbCr Data Compare Over!!! Total bytes [0x%.8x] & error [%d]\n", _u4GoldenCSize[u4InstID], u4Cnt);
        }
        #endif    
    }
    #endif
    if (fgDecErr == FALSE) {
        printk("CbCr compare pass\n");
        printk("%d frame compare pass\n", _u4FileCnt[u4InstID]);
    }

    #ifndef IDE_WRITE_SUPPORT
    if((_u4FileCnt[u4InstID]%10) == 0)
    #endif
    {
        #ifndef IDE_WRITE_SUPPORT
        vVDecOutputDebugString("Pic count to [%d]\n", _u4FileCnt[u4InstID]);  
        #endif
        sprintf(strMessage,"[%d], ", _u4FileCnt[u4InstID]);  
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
    }
    //fprintf(_tFileListRecInfo.fpFile, "[%d], ", _u4FileCnt[_u4VDecID]);  
    #endif

    _ptCurrFBufInfo[u4InstID]->u4FrameCnt = _u4FileCnt[u4InstID];
    if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
    {
        #ifdef REDEC    
        if(_u4ReDecCnt[u4InstID] == 0)
        #endif    
        {
            _u4FileCnt[u4InstID] ++;
            vSetDecFlag(u4InstID, DEC_FLAG_CHG_FBUF);
        }
    }
    //else
    {
        _ucPrevDecFld[u4InstID] = _ptCurrFBufInfo[u4InstID]->ucFBufStatus;
    }
    #ifndef INTERGRATION_WITH_DEMUX
    // Check if still pic needed compare
    #if (defined(COMP_HW_CHKSUM) && (!defined(DOWN_SCALE_SUPPORT)) && (!defined(FGT_SUPPORT)))
    _tTempFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tTempFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tTempFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tTempFileInfo[u4InstID].u4FileLength = 0; 
    //vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_chksum.bin\0", _u4FileCnt[u4InstID]);
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_chksum.bin\0", (_u4FileCnt[u4InstID]<<_fgMVCReady[u4InstID]) + (_ucMVCType[u4InstID] - _fgMVCReady[u4InstID]));
    #ifdef IDE_READ_SUPPORT
    fgOpen = fgPureOpenIdeFile( _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    #else
    fgOpen = fgOpenFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    #endif
    #else
    _tFBufFileInfo[u4InstID].fgGetFileInfo = FALSE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tFBufFileInfo[u4InstID].u4FileLength = 4;      
    //vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], _u4FileCnt[u4InstID]);
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], (_u4FileCnt[u4InstID]<<_fgMVCReady[u4InstID]) + (_ucMVCType[u4InstID] - _fgMVCReady[u4InstID]));
    #ifdef VDEC_DEBUG_USAGE  
    fgOpen = TRUE;
    #else
    #ifdef IDE_READ_SUPPORT
    fgOpen = fgPureOpenIdeFile( _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);
    #else
    fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);
    #endif
    #endif
    #endif

	printk("open file = %s  fgOpen = %d fgDecErr = %d _fgVDecErr[u4InstID] = %d ==>%d\n",_bFileStr1[u4InstID][3],fgOpen, fgDecErr, _fgVDecErr[u4InstID], u4InstID);
    if (_ucMVCType[u4InstID] == 0)
    {
    if((fgOpen == FALSE) ||(fgDecErr == TRUE) || (_fgVDecErr[u4InstID] == TRUE))
    {
        sprintf(strMessage, "%s", "\n");
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        //fprintf(_tFileListRecInfo.fpFile, "\n");  
        // break decode
        if(fgOpen == FALSE)
        {
            msleep(3000);
            sprintf(strMessage," Compare Finish==> %s Pic count to [%d] \n", _bFileStr1[u4InstID][1], _u4FileCnt[u4InstID] - 1);   
            printk("%s\n",strMessage);
            DBG_H264_PRINTF("%s\n",strMessage);
            sprintf(strMessage,"[H264] OK bitstream: %s golden ok,  @%d \n", _bFileStr1[u4InstID][1], _u4FileCnt[u4InstID] - 1);   
            printk("%s\n",strMessage);
            DBG_H264_PRINTF("%s\n",strMessage);
            msleep(3000);
            if(_u4FileCnt[u4InstID] == 1)
            {
                if(fgOpen == FALSE)
                {
                    vVDecOutputDebugString("real NULL\n");
                }
            }
            else //flush DPB buffer
            {
                if(fgIsDecFlagSet(u4InstID, DEC_FLAG_CHG_FBUF))
                {
                    _ptCurrFBufInfo[u4InstID]->eH264DpbStatus = H264_DPB_STATUS_DECODED;
                    _ptCurrFBufInfo[u4InstID]->u4DecOrder = _u4TotalDecFrms[u4InstID];
                }
                
                //ESA H264 BEGIN
                #if defined(CAPTURE_ESA_LOG) && defined(CAPTURE_ALL_IN_ONE) && (!WRITE_ESA_PER_FRAME)
                    fgWrData2PC(_pucESATotalBuf[u4InstID],_u4ESATotalLen[u4InstID],7,_ESAFileName[u4InstID]); 
                #endif
                _u4VerBitCount[u4InstID] = 0xffffffff;
                // ESA H264 END

                vFlushDPB(u4InstID, &_tVerMpvDecPrm[u4InstID], FALSE);
            }
        }
        else
        {
            msleep(3000);
            sprintf(strMessage," Compare Finish Error ==> %s Pic count to [%d] \n", _bFileStr1[u4InstID][1], _u4FileCnt[u4InstID] - 1);     
            printk("%s\n",strMessage);
            msleep(3000);
        }
        _u4VerBitCount[u4InstID] = 0xffffffff;
    }
    }
    else
    {
         if (_ucMVCType[u4InstID] == 1 && fgDecErr == TRUE)
         {
         	_fgMVCError[0] = TRUE;
         }
         else
         if (_ucMVCType[u4InstID] == 2 && fgDecErr == TRUE)
         {
             _fgMVCError[0] = TRUE;
             fgDecErr = FALSE;
         }         	      

         if((fgOpen == FALSE) ||(fgDecErr == TRUE) || (_fgVDecErr[u4InstID] == TRUE) ||(_fgMVCError[u4InstID] == TRUE) )
        {      
        sprintf(strMessage, "%s", "\n");
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        //fprintf(_tFileListRecInfo.fpFile, "\n");  
        // break decode
        if(fgOpen == FALSE)
        {
            sprintf(strMessage," Compare Finish [%d]==> Pic count to [%d] \n", u4InstID,_u4FileCnt[u4InstID] - 1);   
            printk("%s\n",strMessage);
            if(_u4FileCnt[u4InstID] == 1)
            {
                if(fgOpen == FALSE)
                {
                    vVDecOutputDebugString("real NULL\n");
                }
            }
            else //flush DPB buffer
            {
                if(fgIsDecFlagSet(u4InstID, DEC_FLAG_CHG_FBUF))
                {
                    _ptCurrFBufInfo[u4InstID]->eH264DpbStatus = H264_DPB_STATUS_DECODED;
                    _ptCurrFBufInfo[u4InstID]->u4DecOrder = _u4TotalDecFrms[u4InstID];
                }
                vFlushDPB(u4InstID, &_tVerMpvDecPrm[u4InstID], FALSE);
            }
        }
        else
        {
            sprintf(strMessage," Error[%d] ==> Pic count to [%d] \n", u4InstID,_u4FileCnt[u4InstID] - 1);     
            printk("%s\n",strMessage);
        }
        _u4VerBitCount[u4InstID] = 0xffffffff;
        }

         if (_fgMVCError[0] == TRUE)
         {             
             _fgMVCError[1] = TRUE;
         }         	      
    }
    #endif

    if(_u4FileCnt[u4InstID] >= _u4EndCompPicNum[u4InstID])
    {
        _u4VerBitCount[u4InstID] = 0xffffffff;
    }
    
}

// *********************************************************************
// Function    : void vFilledFBuf(UINT32 u4InstID, UCHAR *ptAddr, UINT32 u4Size)
// Description : Write the decoded data to PC for compare
// Parameter   : None
// Return      : None
// *********************************************************************
void vFilledFBuf(UINT32 u4InstID, UCHAR *ptAddr, UINT32 u4Size)
{
    UINT32 *pu4VDSCLBufSa;
    int i;

    pu4VDSCLBufSa = (UINT32*)_pucVDSCLBuf[u4InstID];
    for (i = 0; i < (VDSCL_BUF_SZ >> 2) ; i++)
    {
        *(pu4VDSCLBufSa++) = 0;
    }
}

// *********************************************************************
// Function    : void vConcateStr(char *strFileName)
// Description : Add string
// Parameter   : None
// Return      : None
// *********************************************************************
void vConcateStr(char *strTarFileName, char *strSrcFileName, char *strAddFileName, UINT32 u4Idx)
{
    UINT32 u4Temp;

    u4Temp = sprintf(strTarFileName, "%s", strSrcFileName);
    u4Temp += sprintf(strTarFileName + u4Temp, "%d", u4Idx);  
    u4Temp += sprintf(strTarFileName + u4Temp, "%s", strAddFileName);  
    strTarFileName[u4Temp] = '\0';
}

#if 1
void vVDecVerify_PrepareFileName_RM(UINT32 u4InstID, 
                                                                           UINT32 u4FileNameCnt1, 
                                                                           UINT32 u4PathNumCount,
                                                                           UINT32 u4SubNameCnt)
{
  UINT32 u4Loop = 0;
  UINT32 u4BWShiftCount = 0;
  
  u4BWShiftCount = u4FileNameCnt1 - u4PathNumCount;
        
  //Frame Info File Path
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(12)] = 'f';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(11)] = 'r';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(10)] = 'm';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(9)] = 'i';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(8)] = 'n';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(7)] = 'f';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(6)] = 'o';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(5)] = '.';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(4)] = 'b';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(3)] = 'i';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(2)] = 'n';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(1)] = '\0';

  //Golden File Path
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(12)] = 'g';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount)] = 'g';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount-1)] = 'o';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount-2)] = 'l';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount -3)] = 'd';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount - 4)] = 'e';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount - 5)] = 'n';
           
  _bFileStr1[u4InstID][5][u4FileNameCnt1 -(u4SubNameCnt)] = '.';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4SubNameCnt -1)] = 'y';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4SubNameCnt-2)] = 'u';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4SubNameCnt-3)] = 'v';
  _bFileStr1[u4InstID][5][u4FileNameCnt1] = '\0';

  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount)] = 'g';
  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount - 1)] = 'o';
  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount - 2)] = 'l';
  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount - 3)] = 'd';
  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount - 4)] = 'e';
  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount - 5)] = 'n';
        
  for (u4Loop=0; u4Loop<13; u4Loop++)
  {
    _bFileStr1[u4InstID][9][u4FileNameCnt1-(u4Loop)] = 0x0;
    _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4Loop)] = 0x0;
  }

  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount)] = 'g';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 1)] = 'o';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 2)] = 'l';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 3)] = 'd';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 4)] = 'e';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 5)] = 'n';
        
  for (u4Loop=0; u4Loop<13; u4Loop++)
  {
    _bFileStr1[u4InstID][RM_SOURCEPATH_INDEX][u4FileNameCnt1-(u4Loop)] = 0x0;
    _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4Loop)] = 0x0;
  }
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(12)] = 'a';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(11)] = 'u';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(10)] = 'l';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(9)] = 'i';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(8)] = 'n';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(7)] = 'f';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(6)] = 'o';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(5)] = '.';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(4)] = 'b';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(3)] = 'i';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(2)] = 'n';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(1)] = '\0';          

  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(12)] = 'f';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(11)] = 'r';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(10)] = 'm';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(9)] = 'i';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(8)] = 'n';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(7)] = 'f';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(6)] = 'o';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(5)] = '.';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(4)] = 'b';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(3)] = 'i';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(2)] = 'n';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(1)] = '\0';          

  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(12)] = 's';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(11)] = 'u';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(10)] = 'm';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(9)] = 'i';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(8)] = 'n';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(7)] = 'f';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(6)] = 'o';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(5)] = '.';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(4)] = 'b';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(3)] = 'i';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(2)] = 'n';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(1)] = '\0';    

  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount)] = 'g';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 1)] = 'o';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 2)] = 'l';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 3)] = 'd';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 4)] = 'e';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 5)] = 'n';
  
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(12)] = 'C';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(11)] = 'r';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(10)] = 'c';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(9)] = 'R';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(8)] = 'e';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(7)] = 's';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(6)] = 'u';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(5)] = 'l';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(4)] = 't';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(3)] = '.';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(2)] = 'c';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(1)] = 'r';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(0)] = 'c';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1+(1)] = '\0';
}



// *********************************************************************
// Function    : void fgVdecReadFileName(UINT32 u4InstID, VDEC_INFO_VERIFY_FILE_INFO_T *ptFileInfo, VDEC_INFO_VERIFY_FILE_INFO_T *ptFileRecInfo)
// Description : Write the decoded data to PC for compare
// Parameter   : None
// Return      : None
// *********************************************************************
BOOL fgVdecReadFileName(UINT32 u4InstID, VDEC_INFO_VERIFY_FILE_INFO_T *ptFileListInfo, VDEC_INFO_VERIFY_FILE_INFO_T *ptFileRecInfo, UINT32 *u4StartComp, UINT32 *u4EndComp, UINT32 *DumpPic)
{
    UINT32 u4FileNameCnt,u4FileNameCnt1,u4FileNameCnt2;
    UCHAR bFileName;
    BOOL fgGetName,fgMpegGetName;
    UINT32 u4SubNameCnt;
    CHAR strMessage[256];
    BOOL fgGetSubName, fgGetStartFrmCnt, fgGetEndFrmCnt, fgGetDumpFrmCnt;  
    INT32 u4StarFrm = 0;
    INT32 u4EndFrm = 0;
    INT32 u4DumpFrm = 0;
    INT32 u4Value;
    #ifdef VDEC_VIDEOCODEC_RM
    UINT32 u4PathNumCount = 17;// Important, For RM verification, Recoder filelist charnumber before frminfo;
    #endif //VDEC_VIDEOCODEC_RM

    // 1st read FileList.txt
    if(_u4FileListIdx[u4InstID] == 0)
    {
        ptFileListInfo->fgGetFileInfo = TRUE;  
#if (!CONFIG_DRV_LINUX)
        ptFileListInfo->pucTargetAddr = _pucFileListSa[u4InstID];
#else
        #ifdef IDE_READ_SUPPORT
        //ptFileListInfo->pucTargetAddr = (char *) PHYSICAL(_pucFileListSa[u4InstID]);
        ptFileListInfo->pucTargetAddr = (UCHAR *) (_pucFileListSa[u4InstID]);
        #else //IDE_READ_SUPPORT
        ptFileListInfo->pucTargetAddr = (char *) _pucFileListSa[u4InstID];
        #endif //IDE_READ_SUPPORT
#endif
        ptFileListInfo->u4TargetSz = FILE_LIST_SZ;  
        ptFileListInfo->u4FileLength = 0; 
        _pcBitstreamFileName[u4InstID] = (char *)_pucFileListSa[u4InstID];
       
        #ifdef SATA_HDD_FS_SUPPORT
            if((fgOpenHDDFile(u4InstID,  _FileList[u4InstID],"r+b",ptFileListInfo)) == FALSE)
        #elif defined(IDE_READ_SUPPORT)
        if((fgOpenIdeFile( _FileList[u4InstID],"r+b",ptFileListInfo)) == FALSE)
        #else
            if((fgOpenFile(u4InstID, _FileList[u4InstID],"r+b",ptFileListInfo)) == FALSE)
        #endif
        {
            //vVDecOutputDebugString("\n NULL for file list\n");
            sprintf(strMessage, "%s", "\n NULL for file list\n");
            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        }
        else
        {
            //vVDecOutputDebugString("\n Opened file list\n");
            //sprintf(strMessage, "%s", "\n Opened file list\n");
            //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
            strncpy(_tFileListRecInfo[u4InstID].bFileName,_FileList_Rec[u4InstID], sizeof(_FileList_Rec[u4InstID]));
        }
    }
    else
    {
        strncpy(_tFileListRecInfo[u4InstID].bFileName,_FileList_Rec[u4InstID], sizeof(_FileList_Rec[u4InstID]));
        //ptFileRecInfo->fpFile = fopen(_FileList_Rec,"a+t");
    }
    //fclose(ptFileRecInfo->fpFile);   

    #ifdef ReDecBitstream  
    if(_u4RedecBistreamCnt[u4InstID] < _u4MaxReDecBistreamCnt[u4InstID])
    {
        _u4RedecBistreamCnt[u4InstID] ++;
        _u4FileCnt[u4InstID] = 0;
        _u4VerBitCount[u4InstID] = 0;
        _ucPrevDecFld[u4InstID] = NO_PIC;
        #if defined(SW_RESET) || defined(REDEC)
        _u4FileOffset[u4InstID] = 0;   
        #endif
        _tVerDec[u4InstID].ucState = DEC_NORM_INIT_PRM; 
        return TRUE;
    }
    #endif

    //ptFileRecInfo->fpFile = fopen(_FileList_Rec,"a+t");
    #ifdef VDEC_SIM_DUMP //for debuging specific pic, with input file, fantasia
    #if 0
    *u4StartComp = 0;
    *u4EndComp = 0; // only uncomment when want to limit range
    *DumpPic = 0; // only uncomment when want to dump single pic
    #endif
    #endif
    sprintf(strMessage, "%s", "\n\n\n===============================================\n");
    sprintf(strMessage,"%s", "\nStart to rec ==>");
    fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
    //fprintf(ptFileRecInfo->fpFile, "\nStart to rec ==>"); 

    _u4RedecBistreamCnt[u4InstID] = 0;

    fgGetName = FALSE;
    fgMpegGetName = FALSE;
    fgGetSubName = FALSE;
    fgGetStartFrmCnt = FALSE;
    fgGetEndFrmCnt = FALSE;
    fgGetDumpFrmCnt = FALSE;
    u4FileNameCnt = 0;   
    u4FileNameCnt1 = 0;
    u4FileNameCnt2 = 1;
    sprintf(_bFileStr1[u4InstID][8], "%s", "\\");
    u4SubNameCnt = 1; // count from '.'
    _u4CodecVer[u4InstID] = 0xFF;
    _fgShortHeader[u4InstID] = FALSE;
    _fgMVCReady[u4InstID] = FALSE;    
    _ucMVCType[u4InstID] = 0;

    if(((*_pcBitstreamFileName[u4InstID]) == 'H')&&((*(_pcBitstreamFileName[u4InstID]+1)) == '2')&&((*(_pcBitstreamFileName[u4InstID]+2)) == '6')&&((*(_pcBitstreamFileName[u4InstID]+3)) == '5'))
    {
            _u4CodecVer[u4InstID] = VDEC_H265;
            _pcBitstreamFileName[u4InstID] += 4;
            _ucMVCType[u4InstID] = 0;
            _fgMVCError[u4InstID] = 0;            
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'H')&&((*(_pcBitstreamFileName[u4InstID]+1)) == '2')&&((*(_pcBitstreamFileName[u4InstID]+2)) == '6')&&((*(_pcBitstreamFileName[u4InstID]+3)) == '4'))
    
    {
            _u4CodecVer[u4InstID] = VDEC_H264;
            _pcBitstreamFileName[u4InstID] += 4;
            _ucMVCType[u4InstID] = 0;
            _fgMVCError[u4InstID] = 0;            
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P')&&((*(_pcBitstreamFileName[u4InstID]+2)) == '9'))
    {
            _u4CodecVer[u4InstID] = VDEC_VP9;
            _pcBitstreamFileName[u4InstID] += 3;
            _ucMVCType[u4InstID] = 0;
            _fgMVCError[u4InstID] = 0;    
	   printk("<vdec> _u4CodecVer=VP9 -------------\n");
    }

    else if(((*_pcBitstreamFileName[u4InstID]) == 'M')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'C'))
    {      
            _u4CodecVer[u4InstID] = VDEC_H264;        
            _pcBitstreamFileName[u4InstID] += 3;	
            _fgMVCReady[u4InstID] = (u4InstID==0)? TRUE: FALSE;	
            _ucMVCType[u4InstID] = (u4InstID==0)? 1: 2;    
            _fgMVCError[u4InstID] = 0;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'W')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'M')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'V'))
    {
        _u4CodecVer[u4InstID] = VDEC_WMV;
        _pcBitstreamFileName[u4InstID] += 3;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'M')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'E')&&((*(_pcBitstreamFileName[u4InstID]+3)) == 'G')&&((*(_pcBitstreamFileName[u4InstID]+4)) == '2'))
    {
        _u4CodecVer[u4InstID] = VDEC_MPEG2;
        _pcBitstreamFileName[u4InstID] += 5;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'M')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'E')&&((*(_pcBitstreamFileName[u4InstID]+3)) == 'G')&&((*(_pcBitstreamFileName[u4InstID]+4)) == '4'))
    {
        _u4CodecVer[u4InstID] = VDEC_MPEG4;
        _pcBitstreamFileName[u4InstID] += 5;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'S')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'H')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'O')&&((*(_pcBitstreamFileName[u4InstID]+3)) == 'R')&&((*(_pcBitstreamFileName[u4InstID]+4)) == 'T'))
    {
        _u4CodecVer[u4InstID] = VDEC_MPEG4;
        _fgShortHeader[0] = TRUE;
        _fgShortHeader[1] = TRUE;
        _pcBitstreamFileName[u4InstID] += 5;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'D')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'I')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+3)) == 'X')&&((*(_pcBitstreamFileName[u4InstID]+4)) == '3'))
    {
        _u4CodecVer[u4InstID] = VDEC_DIVX3;
        _pcBitstreamFileName[u4InstID] += 5;
        //sscanf(_pcBitstreamFileName[_u4VDecID], "%d %d", &_u4DIVX3Width, &_u4DIVX3Height);
        //_pcBitstreamFileName[_u4VDecID] += 8;
    }
    //PANDA H263
    else if(((*_pcBitstreamFileName[u4InstID]) == 'H')&&((*(_pcBitstreamFileName[u4InstID]+1)) == '2')&&((*(_pcBitstreamFileName[u4InstID]+2)) == '6')&&((*(_pcBitstreamFileName[u4InstID]+3)) == '3'))
    {
        _u4CodecVer[u4InstID] = VDEC_H263;
        _fgShortHeader[0] = TRUE;
        _fgShortHeader[1] = TRUE;
        _pcBitstreamFileName[u4InstID] += 4;
        //sscanf(_pcBitstreamFileName[_u4VDecID], "%d %d", &_u4DIVX3Width, &_u4DIVX3Height);
        //_pcBitstreamFileName[_u4VDecID] += 8;
    }
    #ifdef VDEC_VIDEOCODEC_RM
    else if (((*_pcBitstreamFileName[u4InstID]) == 'R')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'M'))
    {
      _u4CodecVer[u4InstID] = VDEC_RM;
      _pcBitstreamFileName[u4InstID] += 2;
    }
    #endif //VDEC_VIDEOCODEC_RM
    else if (((*_pcBitstreamFileName[u4InstID]) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P') && ((*(_pcBitstreamFileName[u4InstID]+2)) == '6') && ((*(_pcBitstreamFileName[u4InstID]+3)) == ' '))
    {
      _u4CodecVer[u4InstID] = VDEC_VP6;
        _pcBitstreamFileName[u4InstID] += 3;
        _u1AlphaBitstream[u4InstID] = 0;
        _u1AlphaFlag[u4InstID] = 0;
    }
    else if (((*_pcBitstreamFileName[u4InstID]) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P') && ((*(_pcBitstreamFileName[u4InstID]+2)) == '6') && ((*(_pcBitstreamFileName[u4InstID]+3)) == 'A'))
    {
        _u4CodecVer[u4InstID] = VDEC_VP6;
        _pcBitstreamFileName[u4InstID] += 4;
        _u1AlphaBitstream[u4InstID] = 1;
        _u1AlphaFlag[u4InstID] = VP6_ALPHA_ENABLE;
        _u4AdobeMode[u4InstID] = 0;
    }
    else if (((*_pcBitstreamFileName[u4InstID]) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P') && ((*(_pcBitstreamFileName[u4InstID]+2)) == '6') && ((*(_pcBitstreamFileName[u4InstID]+3)) == 'O'))
    {
        _u4CodecVer[u4InstID] = VDEC_VP6;
        _pcBitstreamFileName[u4InstID] += 4;
        _u1AlphaBitstream[u4InstID] = 0;
        _u1AlphaFlag[u4InstID] = 0;
        _u4AdobeMode[u4InstID] = 0;
    }   
    else if (((*_pcBitstreamFileName[u4InstID]) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P') && ((*(_pcBitstreamFileName[u4InstID]+2)) == '8'))
    {
      _u4CodecVer[u4InstID] = VDEC_VP8;
        _pcBitstreamFileName[u4InstID] += 3;
    }   
    else if(((*_pcBitstreamFileName[u4InstID]) == 'A')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'S'))
    {      
            _u4CodecVer[u4InstID] = VDEC_AVS;        
            _pcBitstreamFileName[u4InstID] += 3;	
    }
    else
    {
        vVDecOutputDebugString("\n Video Codec is not specified\n");
        sprintf(strMessage, "%s", "\nVideo Codec is not specified\n");
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
    }

    printk("<vdec> _u4CodecVer=0x%x\n", _u4CodecVer[u4InstID]);
    _u4StringCnt[u4InstID] =0;

    while(((*_pcBitstreamFileName[u4InstID]) != '\n')&&((*_pcBitstreamFileName[u4InstID]) != '\r')
    &&(_u4FileListIdx[u4InstID] < ptFileListInfo->u4FileLength))
    {
        if(((*_pcBitstreamFileName[u4InstID]) != ' ') || ((_u4CodecVer[u4InstID] == VDEC_VP8) && (_u4StringCnt[u4InstID] >0)))
        {

            if ( (!fgGetStartFrmCnt) && (!fgGetEndFrmCnt) &  (!fgGetDumpFrmCnt) )
            {
                bFileName = (UCHAR)(*_pcBitstreamFileName[u4InstID]);
                _bFileStr1[u4InstID][1][u4FileNameCnt] = bFileName;
                _bFileStr1[u4InstID][1][u4FileNameCnt+1] = '\0';
                sprintf(strMessage,"%c", _bFileStr1[u4InstID][1][u4FileNameCnt]);
                fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                //fprintf(ptFileRecInfo->fpFile, "%c", _bFileStr1[_u4VDecID][1][u4FileNameCnt]); 
                if(bFileName == '.')
                {
                    //for MVC file name parsing
                   if (_ucMVCType[u4InstID] == 0)                   
                    fgGetName = TRUE;
                   else
                   {
                       if( (*(_pcBitstreamFileName[u4InstID]+1)) == '2' && (*(_pcBitstreamFileName[u4InstID]+2)) == '6')
                    fgGetName = TRUE;
                       else
                           fgGetName = FALSE;
                   }
                }
                else
                {
                    if((!fgGetName)&&(fgMpegGetName))
                    {
                        _bFileStr1[u4InstID][8][u4FileNameCnt2] = bFileName;
                        _bFileStr1[u4InstID][8][u4FileNameCnt2+1] = '\0';
                        u4FileNameCnt2 ++;
                    }
                    if(!fgGetName)
                    {
                        _bFileStr1[u4InstID][0][u4FileNameCnt1] = bFileName; 
                        _bFileStr1[u4InstID][0][u4FileNameCnt1+1] = '\0';
                        _bFileStr1[u4InstID][2][u4FileNameCnt1] = bFileName;
                        _bFileStr1[u4InstID][2][u4FileNameCnt1+1] = '\0';
                        _bFileStr1[u4InstID][6][u4FileNameCnt1] = bFileName;
                        _bFileStr1[u4InstID][6][u4FileNameCnt1+1] = '\0';
                        _bFileStr1[u4InstID][7][u4FileNameCnt1] = bFileName;
                        _bFileStr1[u4InstID][7][u4FileNameCnt1+1] = '\0';
                    #ifdef LETTERBOX_SUPPORT
                        _bFileStr1[u4InstID][12][u4FileNameCnt1] = bFileName;
                        _bFileStr1[u4InstID][13][u4FileNameCnt1] = bFileName;
                    #endif

                        #ifdef VDEC_VIDEOCODEC_RM
                        if (_u4CodecVer[u4InstID] == VDEC_RM)
                        {
                          //Frame Info File Path
                          _bFileStr1[u4InstID][8][u4FileNameCnt1] = bFileName;

                          //Golden file Path
                          _bFileStr1[u4InstID][5][u4FileNameCnt1] = bFileName;

                          //Path 
                          _bFileStr1[u4InstID][9][u4FileNameCnt1] = bFileName;
                          _bFileStr1[u4InstID][10][u4FileNameCnt1] = bFileName;

                          _bFileStr1[u4InstID][2][u4FileNameCnt1] = bFileName;
                          _bFileStr1[u4InstID][3][u4FileNameCnt1] = bFileName;
                          _bFileStr1[u4InstID][4][u4FileNameCnt1] = bFileName;

                          _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1] = bFileName;
                        }
                        #endif //VDEC_VIDEOCODEC_RM
                    }
                    else
                    {
                        u4SubNameCnt ++;
                        fgGetSubName = TRUE;
                    }
                }   
                u4FileNameCnt ++;
                u4FileNameCnt1 ++;
            }
            else if(fgGetDumpFrmCnt)
            {
                bFileName = (UCHAR)(*_pcBitstreamFileName[u4InstID]);
                u4Value = CharToInt(bFileName);
                u4DumpFrm *= 10;
                u4DumpFrm += u4Value; 
                *DumpPic =  u4DumpFrm;
            }
            else if (fgGetEndFrmCnt)
            {
                bFileName = (UCHAR)(*_pcBitstreamFileName[u4InstID]);
                u4Value = CharToInt(bFileName);
                u4EndFrm *= 10;
                u4EndFrm += u4Value; 
                *u4EndComp = u4EndFrm;
            }
            else if(fgGetStartFrmCnt)
            {
                bFileName = (UCHAR)(*_pcBitstreamFileName[u4InstID]);
                u4Value = CharToInt(bFileName);
                u4StarFrm *= 10;
                u4StarFrm += u4Value;      
                *u4StartComp = u4StarFrm;
            }
        }
        else if(((*_pcBitstreamFileName[u4InstID]) == ' ') && (u4FileNameCnt != 0) && (!fgGetName))
        {
            fgMpegGetName = TRUE;
#if 1 // MT8320
            sprintf(_bFileStr1[u4InstID][1] + u4FileNameCnt, "%s", "bitstream/"); 
			//sprintf(strMessage, "%s", "bitstream/");
			//fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
			#if (defined(MPEG4_CRC_CMP) || VDEC_MP2_COMPARE_CRC)
            sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1, "%s", "pattern/crc/"); 
            sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1, "%s", "pattern/crc/"); 
            sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1, "%s", "pattern/crc/"); 
            sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1, "%s", "pattern/crc/"); 
			u4FileNameCnt += 10;
            u4FileNameCnt1 += 12;
			#else
            sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1, "%s", "pattern/golden/"); 
            sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1, "%s", "pattern/golden/"); 
            sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1, "%s", "pattern/golden/"); 
            sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1, "%s", "pattern/golden/");

            u4FileNameCnt += 10;
            u4FileNameCnt1 += 15;
			#endif
#else
            sprintf(_bFileStr1[u4InstID][1] + u4FileNameCnt, "%s", "bitstream\\"); 
            sprintf(strMessage, "%s", "bitstream\\");
            fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
            sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1, "%s", "pattern\\"); 
            sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1, "%s", "pattern\\"); 
            sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1, "%s", "pattern\\"); 
            sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1, "%s", "pattern\\"); 
#endif
            sprintf(_bFileStr1[u4InstID][1] + u4FileNameCnt, "%c", '\0'); 
            sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1, "%c", '\0'); 
            sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1, "%c", '\0'); 
            sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1, "%c", '\0'); 
            sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1, "%c", '\0'); 
        }
        else if( ( (*_pcBitstreamFileName[u4InstID]) == ' ' ) && (u4FileNameCnt != 0) 
        && (fgGetName) && (fgGetSubName) && (!fgGetStartFrmCnt) & (!fgGetDumpFrmCnt))
        {
            fgGetStartFrmCnt = TRUE;      
        }
        else  if( ( (*_pcBitstreamFileName[u4InstID]) == ' ' ) && (u4FileNameCnt != 0) 
        && (fgGetName) && (fgGetSubName) && (fgGetStartFrmCnt) && (!fgGetEndFrmCnt) & (!fgGetDumpFrmCnt))
        {
            fgGetEndFrmCnt = TRUE;
        }
        else  if( ( (*_pcBitstreamFileName[u4InstID]) == ' ' ) && (u4FileNameCnt != 0) 
        && (fgGetName) && (fgGetSubName) && (fgGetStartFrmCnt) && (fgGetEndFrmCnt) & (!fgGetDumpFrmCnt))
        {
            fgGetDumpFrmCnt = TRUE;
        }

        _pcBitstreamFileName[u4InstID]++;
        _u4FileListIdx[u4InstID] ++;
        _u4StringCnt[u4InstID]++;
    }
    printk("Start %d, End %d, Dump %d\n", u4StarFrm, u4EndFrm, u4DumpFrm);
    
    if((*_pcBitstreamFileName[u4InstID]) == '\r')
    {
        _pcBitstreamFileName[u4InstID]++;
    }
    if((*_pcBitstreamFileName[u4InstID]) == '\n')
    {
        _pcBitstreamFileName[u4InstID]++;
    }
    //sprintf(strMessage, "%s", "\n");
    //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
    //fprintf(ptFileRecInfo->fpFile, "\n"); 
    //fclose(ptFileRecInfo->fpFile);   
    if((_u4CodecVer[u4InstID] == VDEC_DIVX3) || (_u4CodecVer[u4InstID] == VDEC_MPEG4) || (_u4CodecVer[u4InstID] == VDEC_H263))
    {
    #if VDEC_UFO_ENABLE
        sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1 - u4SubNameCnt, "%s", "\\ufo_"); 
	#else
        sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1 - u4SubNameCnt, "%s", _bFileStr1[u4InstID][8]); 
	#endif
        sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1 - u4SubNameCnt, "%s", _bFileStr1[u4InstID][8]); 
        sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1 - u4SubNameCnt, "%s", _bFileStr1[u4InstID][8]); 
        sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1 - u4SubNameCnt, "%s", _bFileStr1[u4InstID][8]); 
        u4FileNameCnt1 += u4FileNameCnt2;
    }
    if(u4FileNameCnt != 0)
    {
        if(_u4CodecVer[u4InstID] == VDEC_MPEG2)
        {
            const char *p = _bFileStr1[u4InstID][8];
        #if 1 // for MT8320
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt)] = '/';
		    //printk("<vdec> _bFileStr1[%d][%d] = (%s), (%s, %d)\n", u4InstID, 0, _bFileStr1[u4InstID][0], __FUNCTION__, __LINE__);
			sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1 - (u4SubNameCnt - 1), "%s_", p+1);
		    //printk("<vdec> _bFileStr1[%d][%d] = (%s), (%s, %d)\n", u4InstID, 0, _bFileStr1[u4InstID][0], __FUNCTION__, __LINE__);		
		#else
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt)] = '\\';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-1)] = 'P';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-2)] = 't';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-3)] = 'n';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-4)] = '_';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-5)] = '\0';
		#endif
        }
        else
        {
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt)] = '_';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-1)] = '\0';
        }

        _bFileStr1[u4InstID][1][u4FileNameCnt] = '\0';

        _bFileStr1[u4InstID][2][u4FileNameCnt1-(u4SubNameCnt)] = '.';
        _bFileStr1[u4InstID][2][u4FileNameCnt1-(u4SubNameCnt-1)] = 'r';      
        _bFileStr1[u4InstID][2][u4FileNameCnt1-(u4SubNameCnt-2)] = 'e';            
        _bFileStr1[u4InstID][2][u4FileNameCnt1-(u4SubNameCnt-3)] = 'c';            
        _bFileStr1[u4InstID][2][u4FileNameCnt1] = '\0';

        _bFileStr1[u4InstID][6][u4FileNameCnt1-(u4SubNameCnt)] = '.';
        _bFileStr1[u4InstID][6][u4FileNameCnt1-(u4SubNameCnt-1)] = 'd';      
        _bFileStr1[u4InstID][6][u4FileNameCnt1-(u4SubNameCnt-2)] = 'e';            
        _bFileStr1[u4InstID][6][u4FileNameCnt1-(u4SubNameCnt-3)] = 'f';            
        _bFileStr1[u4InstID][6][u4FileNameCnt1] = '\0';

        _bFileStr1[u4InstID][7][u4FileNameCnt1-(u4SubNameCnt)] = '.';
        _bFileStr1[u4InstID][7][u4FileNameCnt1-(u4SubNameCnt-1)] = 'p';      
        _bFileStr1[u4InstID][7][u4FileNameCnt1-(u4SubNameCnt-2)] = 'a';            
        _bFileStr1[u4InstID][7][u4FileNameCnt1-(u4SubNameCnt-3)] = 't';            
        _bFileStr1[u4InstID][7][u4FileNameCnt1] = '\0';

    #ifdef LETTERBOX_SUPPORT
        _bFileStr1[u4InstID][12][u4FileNameCnt1-(u4SubNameCnt)] = '.';
        _bFileStr1[u4InstID][12][u4FileNameCnt1-(u4SubNameCnt-1)] = 's';      
        _bFileStr1[u4InstID][12][u4FileNameCnt1-(u4SubNameCnt-2)] = 'e';            
        _bFileStr1[u4InstID][12][u4FileNameCnt1-(u4SubNameCnt-3)] = 't';            
        _bFileStr1[u4InstID][12][u4FileNameCnt1] = '\0';

        _bFileStr1[u4InstID][13][u4FileNameCnt1-(u4SubNameCnt)] = '.';
        _bFileStr1[u4InstID][13][u4FileNameCnt1-(u4SubNameCnt-1)] = 'g';      
        _bFileStr1[u4InstID][13][u4FileNameCnt1-(u4SubNameCnt-2)] = 'o';            
        _bFileStr1[u4InstID][13][u4FileNameCnt1-(u4SubNameCnt-3)] = 'd';            
        _bFileStr1[u4InstID][13][u4FileNameCnt1] = '\0';
    #endif

        
        #ifdef VDEC_VIDEOCODEC_RM
        if (_u4CodecVer[u4InstID] == VDEC_RM)
        {
          vVDecVerify_PrepareFileName_RM(u4InstID, u4FileNameCnt1, u4PathNumCount, u4SubNameCnt);          
        }
        else
        {
          _bFileStr1[u4InstID][8][u4FileNameCnt1] = '\0';
        }
        #else //VDEC_VIDEOCODEC_RM
        _bFileStr1[u4InstID][8][u4FileNameCnt1] = '\0';
        #endif //VDEC_VIDEOCODEC_RM

        _u4FileCnt[u4InstID] = 0;
		  u4H264Cnt = 0;
		  fgH264YCrcOpen = FALSE;
		
        #ifdef WMV_CRC_COMPOSITE_CHECK_ENABLE
        _u4CRCCnt[u4InstID] = 0;
        #endif
        _u4VerBitCount[u4InstID] = 0;
        _ucPrevDecFld[u4InstID] = NO_PIC;   
        #if defined(SW_RESET) || defined(REDEC)
        _u4FileOffset[u4InstID] = 0;
        #endif
        _tVerDec[u4InstID].ucState = DEC_NORM_INIT_PRM;
        
        #if AVC_NEW_CRC_COMPARE
        sprintf(_bFileStr1[u4InstID][16], "%s", _bFileStr1[u4InstID][0]);
        #endif

		#ifdef MPEG4_CRC_CMP
		sprintf(_bFileStr1[u4InstID][14], "%s", _bFileStr1[u4InstID][0]);
		sprintf(_bFileStr1[u4InstID][15], "%s", _bFileStr1[u4InstID][0]);
		#endif

        // flush record file
        //ptFileRecInfo->fpFile = fopen(_bFileStr1[_u4VDecID][2], "a");
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);

        printk("_bFileStr1[1] = %s\n", _bFileStr1[u4InstID][1]);
        printk("_bFileStr1[0] = %s\n", _bFileStr1[u4InstID][0]);
        printk("_bFileStr1[2] = %s\n", _bFileStr1[u4InstID][2]);
        printk("_bFileStr1[6] = %s\n", _bFileStr1[u4InstID][6]);
        printk("_bFileStr1[7] = %s\n", _bFileStr1[u4InstID][7]);
        printk("_bFileStr1[8] = %s\n", _bFileStr1[u4InstID][8]);
        printk("_bFileStr1[12] = %s\n", _bFileStr1[u4InstID][12]);
        printk("_bFileStr1[13] = %s\n", _bFileStr1[u4InstID][13]);
        printk("_bFileStr1[16] = %s\n", _bFileStr1[u4InstID][16]);
        printk("_bFileStr1[14] = %s\n", _bFileStr1[u4InstID][14]);
        printk("_bFileStr1[15] = %s\n", _bFileStr1[u4InstID][15]);
        
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#else

void vVDecVerify_PrepareFileName_RM(UINT32 u4InstID,
                                                                           UINT32 u4FileNameCnt1,
                                                                           UINT32 u4PathNumCount,
                                                                           UINT32 u4SubNameCnt)
{
  UINT32 u4Loop = 0;
  UINT32 u4BWShiftCount = 0;

  u4BWShiftCount = u4FileNameCnt1 - u4PathNumCount;

  //Frame Info File Path
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(12)] = 'f';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(11)] = 'r';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(10)] = 'm';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(9)] = 'i';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(8)] = 'n';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(7)] = 'f';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(6)] = 'o';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(5)] = '.';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(4)] = 'b';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(3)] = 'i';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(2)] = 'n';
  _bFileStr1[u4InstID][8][u4FileNameCnt1-(1)] = '\0';

  //Golden File Path
  #if 0
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(12)] = 'g';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount)] = 'g';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount-1)] = 'o';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount-2)] = 'l';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount -3)] = 'd';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount - 4)] = 'e';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount - 5)] = 'n'
  #else	
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(12)] = 'g';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount+6)] = 'g';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount-1+6)] = 'o';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount-2+6)] = 'l';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount -3+6)] = 'd';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount - 4+6)] = 'e';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4BWShiftCount - 5+6)] = 'n';
  #endif

  _bFileStr1[u4InstID][5][u4FileNameCnt1 -(u4SubNameCnt)] = '.';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4SubNameCnt -1)] = 'y';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4SubNameCnt-2)] = 'u';
  _bFileStr1[u4InstID][5][u4FileNameCnt1-(u4SubNameCnt-3)] = 'v';
  _bFileStr1[u4InstID][5][u4FileNameCnt1] = '\0';

  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount)] = 'g';
  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount - 1)] = 'o';
  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount - 2)] = 'l';
  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount - 3)] = 'd';
  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount - 4)] = 'e';
  _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4BWShiftCount - 5)] = 'n';

  for (u4Loop=0; u4Loop<13; u4Loop++)
  {
    _bFileStr1[u4InstID][9][u4FileNameCnt1-(u4Loop)] = 0x0;
    _bFileStr1[u4InstID][10][u4FileNameCnt1-(u4Loop)] = 0x0;
  }

  #if 0
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount)] = 'g';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 1)] = 'o';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 2)] = 'l';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 3)] = 'd';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 4)] = 'e';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 5)] = 'n';
  #else
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount+6)] = 'g';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 1+6)] = 'o';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 2+6)] = 'l';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 3+6)] = 'd';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 4+6)] = 'e';
  _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4BWShiftCount - 5+6)] = 'n';

  #endif

  for (u4Loop=0; u4Loop<13; u4Loop++)
  {
    _bFileStr1[u4InstID][RM_SOURCEPATH_INDEX][u4FileNameCnt1-(u4Loop)] = 0x0;
    _bFileStr1[u4InstID][RM_GOLDENPATH_INDEX][u4FileNameCnt1-(u4Loop)] = 0x0;
  }
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(12)] = 'a';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(11)] = 'u';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(10)] = 'l';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(9)] = 'i';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(8)] = 'n';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(7)] = 'f';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(6)] = 'o';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(5)] = '.';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(4)] = 'b';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(3)] = 'i';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(2)] = 'n';
  _bFileStr1[u4InstID][RM_AUFIFO_INDEX][u4FileNameCnt1-(1)] = '\0';

  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(12)] = 'f';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(11)] = 'r';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(10)] = 'm';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(9)] = 'i';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(8)] = 'n';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(7)] = 'f';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(6)] = 'o';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(5)] = '.';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(4)] = 'b';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(3)] = 'i';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(2)] = 'n';
  _bFileStr1[u4InstID][RM_FRMINFO_INDEX][u4FileNameCnt1-(1)] = '\0';

  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(12)] = 's';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(11)] = 'u';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(10)] = 'm';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(9)] = 'i';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(8)] = 'n';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(7)] = 'f';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(6)] = 'o';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(5)] = '.';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(4)] = 'b';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(3)] = 'i';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(2)] = 'n';
  _bFileStr1[u4InstID][RM_SUMINFO_INDEX][u4FileNameCnt1-(1)] = '\0';

  #if 0
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount)] = 'g';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 1)] = 'o';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 2)] = 'l';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 3)] = 'd';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 4)] = 'e';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 5)] = 'n';
  #else
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount+6)] = 'g';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 1+6)] = 'o';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 2+6)] = 'l';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 3+6)] = 'd';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 4+6)] = 'e';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(u4BWShiftCount - 5+6)] = 'n';

  #endif

  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(12)] = 'C';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(11)] = 'r';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(10)] = 'c';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(9)] = 'R';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(8)] = 'e';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(7)] = 's';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(6)] = 'u';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(5)] = 'l';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(4)] = 't';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(3)] = '.';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(2)] = 'c';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(1)] = 'r';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1-(0)] = 'c';
  _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1+(1)] = '\0';
}






// *********************************************************************
// Function    : void fgVdecReadFileName(UINT32 u4InstID, VDEC_INFO_VERIFY_FILE_INFO_T *ptFileInfo, VDEC_INFO_VERIFY_FILE_INFO_T *ptFileRecInfo)
// Description : Write the decoded data to PC for compare
// Parameter   : None
// Return      : None
// *********************************************************************
BOOL fgVdecReadFileName(UINT32 u4InstID, VDEC_INFO_VERIFY_FILE_INFO_T *ptFileListInfo, VDEC_INFO_VERIFY_FILE_INFO_T *ptFileRecInfo, UINT32 *u4StartComp, UINT32 *u4EndComp, UINT32 *DumpPic)
{
    UINT32 u4FileNameCnt,u4FileNameCnt1,u4FileNameCnt2;
    UCHAR bFileName;
    BOOL fgGetName,fgMpegGetName;
    UINT32 u4SubNameCnt;
    CHAR strMessage[256];
    BOOL fgGetSubName, fgGetStartFrmCnt, fgGetEndFrmCnt, fgGetDumpFrmCnt;
    INT32 u4StarFrm = 0;
    INT32 u4EndFrm = 0;
    INT32 u4DumpFrm = 0;
    INT32 u4Value;
    #ifdef VDEC_VIDEOCODEC_RM
    UINT32 u4PathNumCount = 17;// Important, For RM verification, Recoder filelist charnumber before frminfo;
    #endif //VDEC_VIDEOCODEC_RM
	//UTIL_Printf("fgVdecReadFileName!\n");
    // 1st read FileList.txt
    if(_u4FileListIdx[u4InstID] == 0)
    {
        ptFileListInfo->fgGetFileInfo = TRUE;
#if (!CONFIG_DRV_LINUX)
        ptFileListInfo->pucTargetAddr = _pucFileListSa[u4InstID];
#else
        #ifdef IDE_READ_SUPPORT
        //ptFileListInfo->pucTargetAddr = (char *) PHYSICAL(_pucFileListSa[u4InstID]);
        ptFileListInfo->pucTargetAddr = (UCHAR *) (_pucFileListSa[u4InstID]);
        #else //IDE_READ_SUPPORT
        ptFileListInfo->pucTargetAddr = (char *) _pucFileListSa[u4InstID];
        #endif //IDE_READ_SUPPORT
#endif

		
        ptFileListInfo->u4TargetSz = FILE_LIST_SZ;
        ptFileListInfo->u4FileLength = 0;
		#ifdef SEMI_RING_FIFO
		ptFileListInfo->u4BSLenConsumed = 0; //xiaolei.li's
		ptFileListInfo->u4BSLenOffset = 0;
		ptFileListInfo->ucVFIFOEnough = 1;
		#endif
        _pcBitstreamFileName[u4InstID] = (char *)_pucFileListSa[u4InstID];

        #ifdef SATA_HDD_FS_SUPPORT
            //if((fgOpenHDDFile(u4InstID,  _FileList[u4InstID],"r+b",ptFileListInfo)) == FALSE)
            if((fgOpenFile(u4InstID, _FileList[u4InstID],"r+b",ptFileListInfo)) == FALSE)
        #elif defined(IDE_READ_SUPPORT)
        if((fgOpenIdeFile( _FileList[u4InstID],"r+b",ptFileListInfo)) == FALSE)
        #else
            if((fgOpenFile(u4InstID, _FileList[u4InstID],"r+b",ptFileListInfo)) == FALSE)
        #endif
        {
            //vVDecOutputDebugString("\n NULL for file list\n");
            sprintf(strMessage, "%s", "\n NULL for file list\n");
            //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        }
        else
        {
        	//UTIL_Printf("copy file list\n");
            //vVDecOutputDebugString("\n Opened file list\n");
            sprintf(strMessage, "%s", "\n Opened file list\n");
            //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
            strcpy(_tFileListRecInfo[u4InstID].bFileName,_FileList_Rec[u4InstID]);
			//UTIL_Printf("FileList_Rec:%s\n", _FileList_Rec[u4InstID]);
			//UTIL_Printf("ptFileListInfo->pucTargetAddr:%s\n", ptFileListInfo->pucTargetAddr);
        }
    }
    else
    {
    	//UTIL_Printf("copy file to list\n");
		//UTIL_Printf("FileList_Rec:%s\n", _FileList_Rec[u4InstID]);
        strcpy(_tFileListRecInfo[u4InstID].bFileName,_FileList_Rec[u4InstID]);
        //ptFileRecInfo->fpFile = fopen(_FileList_Rec,"a+t");
    }
	//UTIL_Printf("ptFileListInfo->pucTargetAddr:%s\n", ptFileListInfo->pucTargetAddr);
    //fclose(ptFileRecInfo->fpFile);
	//UTIL_Printf("\n open file list done\n");
    #ifdef ReDecBitstream
    if(_u4RedecBistreamCnt[u4InstID] < _u4MaxReDecBistreamCnt[u4InstID])
    {
        _u4RedecBistreamCnt[u4InstID] ++;
        _u4FileCnt[u4InstID] = 0;
        _u4VerBitCount[u4InstID] = 0;
        _ucPrevDecFld[u4InstID] = NO_PIC;
        #if defined(SW_RESET) || defined(REDEC)
        _u4FileOffset[u4InstID] = 0;
        #endif
        _tVerDec[u4InstID].ucState = DEC_NORM_INIT_PRM;
        return TRUE;
    }
    #endif
	//UTIL_Printf("\n Test 0\n");
    vVDecOutputDebugString("\n Test 0\n");
    //ptFileRecInfo->fpFile = fopen(_FileList_Rec,"a+t");
    sprintf(strMessage, "%s", "\n\n\n===============================================\n");
    sprintf(strMessage,"%s", "\nStart to rec ==>");
    //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);

    vVDecOutputDebugString("\n Test 1\n");
	//UTIL_Printf("\n Test 1\n");

    _u4RedecBistreamCnt[u4InstID] = 0;

    fgGetName = FALSE;
    fgMpegGetName = FALSE;
    fgGetSubName = FALSE;
    fgGetStartFrmCnt = FALSE;
    fgGetEndFrmCnt = FALSE;
    fgGetDumpFrmCnt = FALSE;
    u4FileNameCnt = 0;
    u4FileNameCnt1 = 0;
    u4FileNameCnt2 = 1;
	#if USB_LOAD_BS
	sprintf(_bFileStr1[u4InstID][8], "%s", "/");
	#else
    sprintf(_bFileStr1[u4InstID][8], "%s", "\\");
	#endif
    u4SubNameCnt = 1; // count from '.'
    _u4CodecVer[u4InstID] = 0xFF;
    _fgShortHeader[u4InstID] = FALSE;
    _fgMVCReady[u4InstID] = FALSE;
    _ucMVCType[u4InstID] = 0;
	//UTIL_Printf("firstBit:%c, BitstreamFileName:%s\n", *_pcBitstreamFileName[u4InstID], _pcBitstreamFileName[u4InstID]);
#if 0
    if(((*_pcBitstreamFileName[u4InstID]) == 'H')&&((*(_pcBitstreamFileName[u4InstID]+1)) == '2')&&((*(_pcBitstreamFileName[u4InstID]+2)) == '6')&&((*(_pcBitstreamFileName[u4InstID]+3)) == '5'))
    {
            _u4CodecVer[u4InstID] = VDEC_H265;
            _pcBitstreamFileName[u4InstID] += 4;
            _ucMVCType[u4InstID] = 0;
            _fgMVCError[u4InstID] = 0;            
    }
    else 
#endif
    if(((*_pcBitstreamFileName[u4InstID]) == 'H')&&((*(_pcBitstreamFileName[u4InstID]+1)) == '2')&&((*(_pcBitstreamFileName[u4InstID]+2)) == '6')&&((*(_pcBitstreamFileName[u4InstID]+3)) == '4'))
    {
            _u4CodecVer[u4InstID] = VDEC_H264;
            _pcBitstreamFileName[u4InstID] += 4;
            _ucMVCType[u4InstID] = 0;
            _fgMVCError[u4InstID] = 0;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'M')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'C'))
    {
            _u4CodecVer[u4InstID] = VDEC_H264;
            _pcBitstreamFileName[u4InstID] += 3;
            _fgMVCReady[u4InstID] = (u4InstID==0)? TRUE: FALSE;
            _ucMVCType[u4InstID] = (u4InstID==0)? 1: 2;
            _fgMVCError[u4InstID] = 0;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'W')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'M')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'V'))
    {
        _u4CodecVer[u4InstID] = VDEC_WMV;
        _pcBitstreamFileName[u4InstID] += 3;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'M')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'E')&&((*(_pcBitstreamFileName[u4InstID]+3)) == 'G')&&((*(_pcBitstreamFileName[u4InstID]+4)) == '2'))
    {
        _u4CodecVer[u4InstID] = VDEC_MPEG2;
        _pcBitstreamFileName[u4InstID] += 5;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'M')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'E')&&((*(_pcBitstreamFileName[u4InstID]+3)) == 'G')&&((*(_pcBitstreamFileName[u4InstID]+4)) == '4'))
    {
        _u4CodecVer[u4InstID] = VDEC_MPEG4;
        _pcBitstreamFileName[u4InstID] += 5;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'S')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'H')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'O')&&((*(_pcBitstreamFileName[u4InstID]+3)) == 'R')&&((*(_pcBitstreamFileName[u4InstID]+4)) == 'T'))
    {
        _u4CodecVer[u4InstID] = VDEC_MPEG4;
        _fgShortHeader[0] = TRUE;
        _fgShortHeader[1] = TRUE;
        _pcBitstreamFileName[u4InstID] += 5;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'D')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'I')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+3)) == 'X')&&((*(_pcBitstreamFileName[u4InstID]+4)) == '3'))
    {
        _u4CodecVer[u4InstID] = VDEC_DIVX3;
        _pcBitstreamFileName[u4InstID] += 5;
        //sscanf(_pcBitstreamFileName[_u4VDecID], "%d %d", &_u4DIVX3Width, &_u4DIVX3Height);
        //_pcBitstreamFileName[_u4VDecID] += 8;
    }
    //PANDA H263
    else if(((*_pcBitstreamFileName[u4InstID]) == 'H')&&((*(_pcBitstreamFileName[u4InstID]+1)) == '2')&&((*(_pcBitstreamFileName[u4InstID]+2)) == '6')&&((*(_pcBitstreamFileName[u4InstID]+3)) == '3'))
    {
        _u4CodecVer[u4InstID] = VDEC_H263;
        _fgShortHeader[0] = TRUE;
        _fgShortHeader[1] = TRUE;
        _pcBitstreamFileName[u4InstID] += 4;
        //sscanf(_pcBitstreamFileName[_u4VDecID], "%d %d", &_u4DIVX3Width, &_u4DIVX3Height);
        //_pcBitstreamFileName[_u4VDecID] += 8;
    }
    #ifdef VDEC_VIDEOCODEC_RM
    else if (((*_pcBitstreamFileName[u4InstID]) == 'R')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'M'))
    {
      _u4CodecVer[u4InstID] = VDEC_RM;
      _pcBitstreamFileName[u4InstID] += 2;
    }
    #endif //VDEC_VIDEOCODEC_RM
    else if (((*_pcBitstreamFileName[u4InstID]) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P') && ((*(_pcBitstreamFileName[u4InstID]+2)) == '6') && ((*(_pcBitstreamFileName[u4InstID]+3)) == ' '))
    {
      _u4CodecVer[u4InstID] = VDEC_VP6;
        _pcBitstreamFileName[u4InstID] += 3;
        _u1AlphaBitstream[u4InstID] = 0;
        _u1AlphaFlag[u4InstID] = 0;
		_u4AdobeMode[u4InstID] = 0;

    }
    else if (((*_pcBitstreamFileName[u4InstID]) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P') && ((*(_pcBitstreamFileName[u4InstID]+2)) == '6') && ((*(_pcBitstreamFileName[u4InstID]+3)) == 'A'))
    {
        _u4CodecVer[u4InstID] = VDEC_VP6;
        _pcBitstreamFileName[u4InstID] += 4;
        _u1AlphaBitstream[u4InstID] = 1;
        _u1AlphaFlag[u4InstID] = VP6_ALPHA_ENABLE;
        _u4AdobeMode[u4InstID] = 0;
    }
    else if (((*_pcBitstreamFileName[u4InstID]) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P') && ((*(_pcBitstreamFileName[u4InstID]+2)) == '6') && ((*(_pcBitstreamFileName[u4InstID]+3)) == 'B'))
    {
        _u4CodecVer[u4InstID] = VDEC_VP6;
        _pcBitstreamFileName[u4InstID] += 4;
        _u1AlphaBitstream[u4InstID] = 0;
        _u1AlphaFlag[u4InstID] = 0;
        _u4AdobeMode[u4InstID] = 1;
    }
    else if (((*_pcBitstreamFileName[u4InstID]) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'P') && ((*(_pcBitstreamFileName[u4InstID]+2)) == '8'))
    {
      _u4CodecVer[u4InstID] = VDEC_VP8;
        _pcBitstreamFileName[u4InstID] += 3;
    }
    else if(((*_pcBitstreamFileName[u4InstID]) == 'A')&&((*(_pcBitstreamFileName[u4InstID]+1)) == 'V')&&((*(_pcBitstreamFileName[u4InstID]+2)) == 'S'))
    {
            _u4CodecVer[u4InstID] = VDEC_AVS;
            _pcBitstreamFileName[u4InstID] += 3;
    }
    else
    {
        vVDecOutputDebugString("\n Video Codec is not specified\n");
        sprintf(strMessage, "%s", "\nVideo Codec is not specified\n");
        //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
    }

    //UTIL_Printf("[MPV%d] fgVdecReadFileName, cur codec:0x%x.\n", u4InstID, _u4CodecVer[u4InstID]);
    
    _u4StringCnt[u4InstID] =0;

    while(((*_pcBitstreamFileName[u4InstID]) != '\n')&&((*_pcBitstreamFileName[u4InstID]) != '\r')
    &&(_u4FileListIdx[u4InstID] < ptFileListInfo->u4FileLength))
    {
        if(((*_pcBitstreamFileName[u4InstID]) != ' ') || ((_u4CodecVer[u4InstID] == VDEC_VP8) && (_u4StringCnt[u4InstID] >0)))
        {

            if ( (!fgGetStartFrmCnt) && (!fgGetEndFrmCnt) &  (!fgGetDumpFrmCnt) )
            {
                bFileName = (UCHAR)(*_pcBitstreamFileName[u4InstID]);
                _bFileStr1[u4InstID][1][u4FileNameCnt] = bFileName;
                _bFileStr1[u4InstID][1][u4FileNameCnt+1] = '\0';
                sprintf(strMessage,"%c", _bFileStr1[u4InstID][1][u4FileNameCnt]);
                //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
                //fprintf(ptFileRecInfo->fpFile, "%c", _bFileStr1[_u4VDecID][1][u4FileNameCnt]);
               // UTIL_Printf("bFileName:%c\n", bFileName);
                if(bFileName == '.')
                {
                    //for MVC file name parsing
                   if (_ucMVCType[u4InstID] == 0)
                    fgGetName = TRUE;
                   else
                   {
                       if( (*(_pcBitstreamFileName[u4InstID]+1)) == '2' && (*(_pcBitstreamFileName[u4InstID]+2)) == '6')
                    fgGetName = TRUE;
                       else
                           fgGetName = FALSE;
                   }
                }
                else
                {
                    if((!fgGetName)&&(fgMpegGetName))
                    {
                        _bFileStr1[u4InstID][8][u4FileNameCnt2] = bFileName;
                        _bFileStr1[u4InstID][8][u4FileNameCnt2+1] = '\0';
                        u4FileNameCnt2 ++;
                    }
                    if(!fgGetName)
                    {
                        _bFileStr1[u4InstID][0][u4FileNameCnt1] = bFileName;
                        _bFileStr1[u4InstID][2][u4FileNameCnt1] = bFileName;
                        _bFileStr1[u4InstID][6][u4FileNameCnt1] = bFileName;
                        _bFileStr1[u4InstID][7][u4FileNameCnt1] = bFileName;
                    #ifdef LETTERBOX_SUPPORT
                        _bFileStr1[u4InstID][12][u4FileNameCnt1] = bFileName;
                        _bFileStr1[u4InstID][13][u4FileNameCnt1] = bFileName;
                    #endif

                        #ifdef VDEC_VIDEOCODEC_RM
                        if (_u4CodecVer[u4InstID] == VDEC_RM)
                        {
                          //Frame Info File Path
                          _bFileStr1[u4InstID][8][u4FileNameCnt1] = bFileName;

                          //Golden file Path
                          _bFileStr1[u4InstID][5][u4FileNameCnt1] = bFileName;

                          //Path
                          _bFileStr1[u4InstID][9][u4FileNameCnt1] = bFileName;
                          _bFileStr1[u4InstID][10][u4FileNameCnt1] = bFileName;

                          _bFileStr1[u4InstID][2][u4FileNameCnt1] = bFileName;
                          _bFileStr1[u4InstID][3][u4FileNameCnt1] = bFileName;
                          _bFileStr1[u4InstID][4][u4FileNameCnt1] = bFileName;

                          _bFileStr1[u4InstID][RM_CRCINFO_INDEX][u4FileNameCnt1] = bFileName;
                        }
                        #endif //VDEC_VIDEOCODEC_RM
                    }
                    else
                    {
                        u4SubNameCnt ++;
                        fgGetSubName = TRUE;
                    }
                }
                u4FileNameCnt ++;
                u4FileNameCnt1 ++;
            }
            else if(fgGetDumpFrmCnt)
            {
                bFileName = (UCHAR)(*_pcBitstreamFileName[u4InstID]);
                u4Value = CharToInt(bFileName);
                u4DumpFrm *= 10;
                u4DumpFrm += u4Value;
                *DumpPic =  u4DumpFrm;
            }
            else if (fgGetEndFrmCnt)
            {
                bFileName = (UCHAR)(*_pcBitstreamFileName[u4InstID]);
                u4Value = CharToInt(bFileName);
                u4EndFrm *= 10;
                u4EndFrm += u4Value;
                *u4EndComp = u4EndFrm;
            }
            else if(fgGetStartFrmCnt)
            {
                bFileName = (UCHAR)(*_pcBitstreamFileName[u4InstID]);
                u4Value = CharToInt(bFileName);
                u4StarFrm *= 10;
                u4StarFrm += u4Value;
                *u4StartComp = u4StarFrm;
            }
        }
        else if(((*_pcBitstreamFileName[u4InstID]) == ' ') && (u4FileNameCnt != 0) && (!fgGetName))
        {
            fgMpegGetName = TRUE;
			#if USB_LOAD_BS
            sprintf(_bFileStr1[u4InstID][1] + u4FileNameCnt, "%s", "bitstream/");
            sprintf(strMessage, "%s", "bitstream/");
            //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
            sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1, "%s", "pattern/");
            sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1, "%s", "pattern/");
            sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1, "%s", "pattern/");
            sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1, "%s", "pattern/");
			#else
            #if 0
            sprintf(_bFileStr1[u4InstID][1] + u4FileNameCnt, "%s", "bitstream\\");
            sprintf(strMessage, "%s", "bitstream\\");
            //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
            sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1, "%s", "pattern\\");
            sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1, "%s", "pattern\\");
            sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1, "%s", "pattern\\");
            sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1, "%s", "pattern\\");
            #else
            sprintf(_bFileStr1[u4InstID][1] + u4FileNameCnt, "%s", "bitstream\\");
            sprintf(strMessage, "%s", "bitstream\\");
            //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
			if((_u4CodecVer[u4InstID] == VDEC_MPEG2) || (_u4CodecVer[u4InstID] == VDEC_MPEG4))//tutuyc0605
            {
            #ifdef MPEG4_CRCCHECK_ENABLE 
				sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1, "%s", "pattern\\crc\\");
				sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1, "%s", "pattern\\crc\\");
				sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1, "%s", "pattern\\crc\\");
				sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1, "%s", "pattern\\crc\\");

				u4FileNameCnt += 10;
				u4FileNameCnt1 += 12;
			#else
				sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1, "%s", "pattern\\golden\\");
				sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1, "%s", "pattern\\golden\\");
				sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1, "%s", "pattern\\golden\\");
				sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1, "%s", "pattern\\golden\\");
				
				u4FileNameCnt += 10;
				u4FileNameCnt1 += 15;

			#endif
			}
			else
			{
            sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1, "%s", "pattern\\");
            sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1, "%s", "pattern\\");
            sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1, "%s", "pattern\\");
            sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1, "%s", "pattern\\");

			u4FileNameCnt += 10;
            u4FileNameCnt1 += 8;
			}
            #endif
			#endif
        }
        else if( ( (*_pcBitstreamFileName[u4InstID]) == ' ' ) && (u4FileNameCnt != 0)
        && (fgGetName) && (fgGetSubName) && (!fgGetStartFrmCnt) & (!fgGetDumpFrmCnt))
        {
            fgGetStartFrmCnt = TRUE;
        }
        else  if( ( (*_pcBitstreamFileName[u4InstID]) == ' ' ) && (u4FileNameCnt != 0)
        && (fgGetName) && (fgGetSubName) && (fgGetStartFrmCnt) && (!fgGetEndFrmCnt) & (!fgGetDumpFrmCnt))
        {
            fgGetEndFrmCnt = TRUE;
        }
        else  if( ( (*_pcBitstreamFileName[u4InstID]) == ' ' ) && (u4FileNameCnt != 0)
        && (fgGetName) && (fgGetSubName) && (fgGetStartFrmCnt) && (fgGetEndFrmCnt) & (!fgGetDumpFrmCnt))
        {
            fgGetDumpFrmCnt = TRUE;
        }

        _pcBitstreamFileName[u4InstID]++;
        _u4FileListIdx[u4InstID] ++;
        _u4StringCnt[u4InstID]++;
		//UTIL_Printf("_pcBitstreamFileName[u4InstID]:%s, _u4FileListIdx[u4InstID] :%d, _u4StringCnt[u4InstID]:%d\n", _pcBitstreamFileName[u4InstID]++, _u4FileListIdx[u4InstID] ++, _u4StringCnt[u4InstID]++);
    }
    if((*_pcBitstreamFileName[u4InstID]) == '\r')
    {
        _pcBitstreamFileName[u4InstID]++;
    }
    if((*_pcBitstreamFileName[u4InstID]) == '\n')
    {
        _pcBitstreamFileName[u4InstID]++;
    }
    sprintf(strMessage, "%s", "\n");
  //  fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
    //fprintf(ptFileRecInfo->fpFile, "\n");
    //fclose(ptFileRecInfo->fpFile);
    #if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8561 || CONFIG_CHIP_VER_CURR ==CONFIG_CHIP_VER_MT5890)
      if((_u4CodecVer[u4InstID] == VDEC_DIVX3) || (_u4CodecVer[u4InstID] == VDEC_MPEG4) ||(_u4CodecVer[u4InstID] == VDEC_MPEG2) 
	  	|| (_u4CodecVer[u4InstID] == VDEC_H263))
    #else
    if((_u4CodecVer[u4InstID] == VDEC_DIVX3) || (_u4CodecVer[u4InstID] == VDEC_MPEG4))
    #endif
    {
        #if VDEC_UFO_ENABLE
        if(_u4CodecVer[u4InstID] != VDEC_MPEG2)
        {
            sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1 - u4SubNameCnt, "%s", "\\ufo_"); 
        }
        else
        {
            sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1 - u4SubNameCnt, "%s", _bFileStr1[u4InstID][8]);
        }
	#else
        sprintf(_bFileStr1[u4InstID][0] + u4FileNameCnt1 - u4SubNameCnt, "%s", _bFileStr1[u4InstID][8]);
        #endif
        sprintf(_bFileStr1[u4InstID][2] + u4FileNameCnt1 - u4SubNameCnt, "%s", _bFileStr1[u4InstID][8]);
        sprintf(_bFileStr1[u4InstID][6] + u4FileNameCnt1 - u4SubNameCnt, "%s", _bFileStr1[u4InstID][8]);
        sprintf(_bFileStr1[u4InstID][7] + u4FileNameCnt1 - u4SubNameCnt, "%s", _bFileStr1[u4InstID][8]);
        u4FileNameCnt1 += u4FileNameCnt2;
    }
    if(u4FileNameCnt != 0)
    {
     #if (/*CONFIG_CHIP_VER_CURR != CONFIG_CHIP_VER_MT8561 && */CONFIG_CHIP_VER_CURR != CONFIG_CHIP_VER_MT5890)
        if(_u4CodecVer[u4InstID] == VDEC_MPEG2)
        {
        	#if USB_LOAD_BS
			_bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt)] = '/';
			#else
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt)] = '\\';
			#endif
			_bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-1)] = 'P';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-2)] = 't';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-3)] = 'n';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-4)] = '_';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-5)] = '\0';
        }
        else
	#endif
        {
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt)] = '_';
            _bFileStr1[u4InstID][0][u4FileNameCnt1-(u4SubNameCnt-1)] = '\0';
        }

        _bFileStr1[u4InstID][1][u4FileNameCnt] = '\0';

        _bFileStr1[u4InstID][2][u4FileNameCnt1-(u4SubNameCnt)] = '.';
        _bFileStr1[u4InstID][2][u4FileNameCnt1-(u4SubNameCnt-1)] = 'r';
        _bFileStr1[u4InstID][2][u4FileNameCnt1-(u4SubNameCnt-2)] = 'e';
        _bFileStr1[u4InstID][2][u4FileNameCnt1-(u4SubNameCnt-3)] = 'c';
        _bFileStr1[u4InstID][2][u4FileNameCnt1] = '\0';

        _bFileStr1[u4InstID][6][u4FileNameCnt1-(u4SubNameCnt)] = '.';
        _bFileStr1[u4InstID][6][u4FileNameCnt1-(u4SubNameCnt-1)] = 'd';
        _bFileStr1[u4InstID][6][u4FileNameCnt1-(u4SubNameCnt-2)] = 'e';
        _bFileStr1[u4InstID][6][u4FileNameCnt1-(u4SubNameCnt-3)] = 'f';
        _bFileStr1[u4InstID][6][u4FileNameCnt1] = '\0';

        _bFileStr1[u4InstID][7][u4FileNameCnt1-(u4SubNameCnt)] = '.';
        _bFileStr1[u4InstID][7][u4FileNameCnt1-(u4SubNameCnt-1)] = 'p';
        _bFileStr1[u4InstID][7][u4FileNameCnt1-(u4SubNameCnt-2)] = 'a';
        _bFileStr1[u4InstID][7][u4FileNameCnt1-(u4SubNameCnt-3)] = 't';
        _bFileStr1[u4InstID][7][u4FileNameCnt1] = '\0';

    #ifdef LETTERBOX_SUPPORT
        _bFileStr1[u4InstID][12][u4FileNameCnt1-(u4SubNameCnt)] = '.';
        _bFileStr1[u4InstID][12][u4FileNameCnt1-(u4SubNameCnt-1)] = 's';
        _bFileStr1[u4InstID][12][u4FileNameCnt1-(u4SubNameCnt-2)] = 'e';
        _bFileStr1[u4InstID][12][u4FileNameCnt1-(u4SubNameCnt-3)] = 't';
        _bFileStr1[u4InstID][12][u4FileNameCnt1] = '\0';

        _bFileStr1[u4InstID][13][u4FileNameCnt1-(u4SubNameCnt)] = '.';
        _bFileStr1[u4InstID][13][u4FileNameCnt1-(u4SubNameCnt-1)] = 'g';
        _bFileStr1[u4InstID][13][u4FileNameCnt1-(u4SubNameCnt-2)] = 'o';
        _bFileStr1[u4InstID][13][u4FileNameCnt1-(u4SubNameCnt-3)] = 'd';
        _bFileStr1[u4InstID][13][u4FileNameCnt1] = '\0';
    #endif


        #ifdef VDEC_VIDEOCODEC_RM
        if (_u4CodecVer[u4InstID] == VDEC_RM)
        {
          vVDecVerify_PrepareFileName_RM(u4InstID, u4FileNameCnt1, u4PathNumCount, u4SubNameCnt);
        }
        else
        {
          _bFileStr1[u4InstID][8][u4FileNameCnt1] = '\0';
        }
        #else //VDEC_VIDEOCODEC_RM
        _bFileStr1[u4InstID][8][u4FileNameCnt1] = '\0';
        #endif //VDEC_VIDEOCODEC_RM

        _u4FileCnt[u4InstID] = 0;
        u4H264Cnt = 0;
        fgH264YCrcOpen = FALSE;

	#ifdef WMV_CRC_COMPOSITE_CHECK_ENABLE
	_u4CRCCnt[u4InstID] = 0;
	#endif
        _u4VerBitCount[u4InstID] = 0;
        _ucPrevDecFld[u4InstID] = NO_PIC;
        #if defined(SW_RESET) || defined(REDEC)
        _u4FileOffset[u4InstID] = 0;
        #endif
        _tVerDec[u4InstID].ucState = DEC_NORM_INIT_PRM;
        #if AVC_NEW_CRC_COMPARE
		if(_u4CodecVer[u4InstID] == VDEC_H264)
        sprintf(_bFileStr1[u4InstID][16], "%s", _bFileStr1[u4InstID][0]);
        #endif

        // flush record file
        //ptFileRecInfo->fpFile = fopen(_bFileStr1[_u4VDecID][2], "a");
        //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        //fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif
// *********************************************************************
// Function    : void vWrite2PC(UINT32 u4InstID, UCHAR bType, UCHAR *pbAddr)
// Description : Write the decoded data to PC for compare
// Parameter   : None
// Return      : None
// *********************************************************************
void vWrite2PC(UINT32 u4InstID, UCHAR bType, UCHAR *pbAddr)
{
    UINT32 u4Temp;
    char strMessage[256];
    UINT32 u4Mode;

    u4Temp = sprintf(_bTempStr1[u4InstID], "%s", _bFileStr1[u4InstID][0]);
    _tTempFileInfo[u4InstID].fgGetFileInfo = FALSE;

    switch(bType)
    {
        case 1:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%s%d%s", "dump_bitstream_VDec",u4InstID,".err");
            fgWrData2PC((void*)pbAddr,_tInFileInfo[u4InstID].u4FileLength,7,_bTempStr1[u4InstID]);
            break;
        case 2:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "dump_VDec%d_%d_Y.err", u4InstID, _u4FileCnt[u4InstID]);
            fgWrData2PC((void*)pbAddr,_tFBufFileInfo[u4InstID].u4FileLength,7,_bTempStr1[u4InstID]);
            break;
        case 3:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "dump_VDec%d_%d_C.err", u4InstID, _u4FileCnt[u4InstID]);
            fgWrData2PC((void*)pbAddr,_tFBufFileInfo[u4InstID].u4FileLength,7,_bTempStr1[u4InstID]);
            break;
        case 4:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "dump_VDec%d_%d_MV.err", u4InstID, _u4FileCnt[u4InstID]);
            fgWrData2PC((void*)pbAddr,_tFBufFileInfo[u4InstID].u4FileLength,7,_bTempStr1[u4InstID]);
            break;
        case 5:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "dump_VDec%d_%d_Y_PP.bin", u4InstID, _u4FileCnt[u4InstID]);
            fgWrData2PC((void*)pbAddr,_tFBufFileInfo[u4InstID].u4FileLength,7,_bTempStr1[u4InstID]);
            break;      
        case 6:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "dump_VDec%d_%d_C_PP.bin", u4InstID, _u4FileCnt[u4InstID]);
            fgWrData2PC((void*)pbAddr,_tFBufFileInfo[u4InstID].u4FileLength,7,_bTempStr1[u4InstID]);
            break; 
        case 7:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%s", "dec_cycle.txt");
            fgWrData2PC(strMessage,strlen(strMessage),8,_bTempStr1[u4InstID]);
            if(fgIsDecFlagSet(u4InstID, DEC_FLAG_CHG_FBUF))
            {
                sprintf(strMessage, "%s", "\n");
                fgWrData2PC(strMessage,strlen(strMessage),8,_bTempStr1[u4InstID]);
            }
            break;      
        case 8:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%s", "disp_order.txt");
            sprintf(strMessage, "%s", "\n");
            fgWrData2PC(strMessage,strlen(strMessage),8,_bTempStr1[u4InstID]); 
            break; 
        case 9:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%d_chksum_8530.bin", _u4FileCnt[u4InstID]);
            if((!fgIsFrmPic(u4InstID)) && (_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)) //Only 2nd field pic needs offset 256
            {
                u4Mode = 8;
            }
            else
            {
                u4Mode = 7;
            }      

            fgWrData2PC(&_u4DumpChksum[u4InstID][0],MAX_CHKSUM_NUM*4, u4Mode,_bTempStr1[u4InstID]);
            break; 
        case 10:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%d_chksum.bin", _u4FileCnt[u4InstID]);
            if((!fgWMVIsFrmPic(u4InstID)) && (_rWMVPPS[u4InstID].i4CurrentTemporalField == 1)) //Only 2nd field pic needs offset 256
            {
                u4Mode = 8;
            }
            else
            {
                u4Mode = 7;
            }      
            
            fgWrData2PC(&_u4DumpChksum[u4InstID][0],MAX_CHKSUM_NUM*4, u4Mode, _bTempStr1[u4InstID]);
            break;
        case 11:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "VDec%d_%d_error_chksum.bin", u4InstID, _u4FileCnt[u4InstID]);
            if((!fgIsFrmPic(u4InstID)) && (_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)) //Only 2nd field pic needs offset 256
            {
                u4Mode = 8;
            }
            else
            {
                u4Mode = 7;
            }      

            fgWrData2PC(&_u4DumpChksum[u4InstID][0],MAX_CHKSUM_NUM*4, u4Mode,_bTempStr1[u4InstID]);
            break; 
        case 12:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "VDec%d_%d_error_chksum.bin", u4InstID, _u4FileCnt[u4InstID]);
            if((!fgWMVIsFrmPic(u4InstID)) && (_rWMVPPS[u4InstID].i4CurrentTemporalField == 1)) //Only 2nd field pic needs offset 256
            {
                u4Mode = 8;
            }
            else
            {
                u4Mode = 7;
            }      

            fgWrData2PC(&_u4DumpChksum[u4InstID][0],MAX_CHKSUM_NUM*4, u4Mode, _bTempStr1[u4InstID]);
            break;
        case 13:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "VDec%d_dump_%d_bitstream.err", u4InstID, _u4FileCnt[u4InstID]);
            fgWrData2PC((void*)pbAddr,(_u4WMVByteCount[u4InstID] -_u4CurrPicStartAddr[u4InstID]),7,_bTempStr1[u4InstID]);
            break;
        case 14:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "dump_%d_Y_Golden_PP.bin", _u4FileCnt[u4InstID]);
            fgWrData2PC((void*)pbAddr,_tFBufFileInfo[u4InstID].u4FileLength,7,_bTempStr1[u4InstID]);

            break;      
        case 15:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "dump_%d_C_Golden_PP.bin", _u4FileCnt[u4InstID]);
            fgWrData2PC((void*)pbAddr,_tFBufFileInfo[u4InstID].u4FileLength,7,_bTempStr1[u4InstID]);
        case 16:
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%d_vdscl_seeting.bin", _u4FileCnt[u4InstID]);
            fgWrData2PC(&_tVerMpvDecPrm[u4InstID].rDownScalerPrm,16*4,7,_bTempStr1[u4InstID]); 
            break;

       #ifdef CAPTURE_ESA_LOG
       case 17:
       	#ifdef CAPTURE_ALL_IN_ONE
          #if WRITE_ESA_PER_FRAME
            _tFBufFileInfo[u4InstID].fgGetFileInfo = FALSE;  
            _tFBufFileInfo[u4InstID].pucTargetAddr = _pucESALog[u4InstID];
            _tFBufFileInfo[u4InstID].u4TargetSz = ESALOG_SZ;  
            _tFBufFileInfo[u4InstID].u4FileLength = 0;  
            if (fgOpenFile(u4InstID, _ESAFileName[u4InstID], "r+b", &_tFBufFileInfo[u4InstID]) == 0)
            {
            	u4Temp = sprintf(_pucESALog[u4InstID], "%s\n", _bESAStr1[u4InstID]);
            }
            else
            	{
            	u4Temp = 0;
            	}
            u4Temp += sprintf(_pucESALog[u4InstID] + u4Temp,  "%s,",_bTempStr1[u4InstID]);
            fgWrData2PC((void*)pbAddr,u4VDEC_HAL_Read_ESA(u4InstID, u4Temp),7,_ESAFileName[u4InstID]); 
          #else
            u4Temp = 0;
            u4Temp += sprintf(_pucESALog[u4InstID] + u4Temp,  "%s,",_bTempStr1[u4InstID]);
            u4Temp = u4VDEC_HAL_Read_ESA(u4InstID, u4Temp);
            _u4ESATotalLen[u4InstID] += sprintf(_pucESATotalBuf[u4InstID] + _u4ESATotalLen[u4InstID], "%s", _pucESALog[u4InstID]);
            if (_u4ESATotalLen[u4InstID] >= (ESALOGTOTAL_SZ - 2048))
            {
                fgWrData2PC(_pucESATotalBuf[u4InstID],_u4ESATotalLen[u4InstID],7,_ESAFileName[u4InstID]);
                _u4ESATotalLen[u4InstID] = 0;
            }
          #endif // WRITE_ESA_PER_FRAME
       	#else // !CAPTURE_ALL_IN_ONE
            u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%s","ESA.csv");
            _tFBufFileInfo[u4InstID].fgGetFileInfo = FALSE;  
            _tFBufFileInfo[u4InstID].pucTargetAddr = _pucESALog[u4InstID];
            _tFBufFileInfo[u4InstID].u4TargetSz = ESALOG_SZ;  
            _tFBufFileInfo[u4InstID].u4FileLength = 0;  
            if (fgOpenFile(u4InstID, _bTempStr1[u4InstID], "r+b", &_tFBufFileInfo[u4InstID]) == 0)
            {
            	u4Temp = sprintf(_pucESALog[u4InstID], "%s\n", _bESAStr1[u4InstID]);
            }
            else
            	{
            	u4Temp = 0;
            	}
            fgWrData2PC((void*)pbAddr,u4VDEC_HAL_Read_ESA(u4InstID, u4Temp),7,_bTempStr1[u4InstID]); 
        #endif // CAPTURE_ALL_IN_ONE
            break;
        #endif
        default:
            break;
    }

    //fclose(_tTempFileInfo.fpFile);
}


// *********************************************************************
// Function    : void vAddStartCode2Dram(UCHAR bType, UCHAR *pbAddr)
// Description : Write the decoded data to PC for compare
// Parameter   : None
// Return      : None
// *********************************************************************
void vAddStartCode2Dram(UCHAR *pbDramAddr)
{
  if((_u4CodecVer[0] == VDEC_H264))
  {
  pbDramAddr[0] = 0x00;
  pbDramAddr[1] = 0x00;
  pbDramAddr[2] = 0x01;  
  pbDramAddr[3] = 0x0B;
  }
  else if((_u4CodecVer[0] == VDEC_MPEG4))
  {
  	pbDramAddr[0] = 0x00;
  	pbDramAddr[1] = 0x00;
  	pbDramAddr[2] = 0x01;  
  	pbDramAddr[3] = 0xB1;
  }
}





// *********************************************************************
// Function    : BOOL fgCompH264ChkSumGolden(UINT32 u4InstID)
// Description : write check sum in rec file
// Parameter   : None
// Return      : TRUE pass, FALSE err
// *********************************************************************
BOOL fgCompH264ChkSumGolden(UINT32 u4InstID)
{
    BOOL fgIsCompNoErr,fgOpen;
    UINT32 *pu4GoldenCheckSum;
    UINT32 u4Temp;

    u4Temp = sprintf(_bTempStr1[u4InstID], "%s", _bFileStr1[u4InstID][0]);
    _tTempFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tTempFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tTempFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tTempFileInfo[u4InstID].u4FileLength = 0; 

    u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%d_chksum.bin", _u4FileCnt[u4InstID]);
    fgOpen = fgOpenPCFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    pu4GoldenCheckSum = (UINT32 *)_pucDumpYBuf[u4InstID];
    if(fgOpen)
    {
        if((!fgIsFrmPic(u4InstID)) && (_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)) //Only 2nd field pic needs offset 256
        {
            pu4GoldenCheckSum += MAX_CHKSUM_NUM;
        }
        else
        {
            pu4GoldenCheckSum += 0;
        }   
        //????????
        fgIsCompNoErr = fgVDEC_HAL_H264_VDec_CompCheckSum(&_u4DumpChksum[u4InstID][0], pu4GoldenCheckSum);      
    }
    else
    {
        fgIsCompNoErr = FALSE;
    }

    return fgIsCompNoErr;
}


// *********************************************************************
// Function    : BOOL fgWMV_CheckCRCResult(UINT32 u4InstID,UINT32 u4CRCResBuf)
// Description : compare wmv crc
// Parameter   : None
// Return      : TRUE pass, FALSE err
// *********************************************************************
BOOL  fgWMV_CheckCRCResult(UINT32 u4InstID,
                                   #ifdef WMV_CRC_COMPOSITE_CHECK_ENABLE
                                   UINT32 u4DecFrameCnt,
                                   #endif
                                   UINT32 u4CRCResBuf)
{
  UINT32* pu4CRCResultCurrAddr;
  UINT32 u4CRCResult = 0;
  UINT32 i=0;
  UINT32 u4SWResult;

#ifdef WMV_CRC_COMPOSITE_CHECK_ENABLE
  printk("u4DecFrameCnt %d\n",u4DecFrameCnt);
#endif
  #ifdef WMV_CRC_COMPOSITE_CHECK_ENABLE
  pu4CRCResultCurrAddr = (UINT32*) (u4CRCResBuf + u4DecFrameCnt * 32);
  #else
  pu4CRCResultCurrAddr = (UINT32*) u4CRCResBuf ;
  #endif

  u4SWResult = *pu4CRCResultCurrAddr;

  //Y
  printk("CRC_Y: ");
  for (i=0; i<4; i++)
  {
    u4CRCResult = u4ReadReg(VDEC_CRC_REG_OFFSET0 + ((2+i)*4));
    u4SWResult = *(pu4CRCResultCurrAddr+i);
    if (u4SWResult != u4CRCResult)
    {
      printk("u4SWResult: 0x%x, u4CRCResult: 0x%x\n", u4SWResult, u4CRCResult);
      printk("Y CRC Compare Golden Error\n");
      
      //vVDEC_HAL_WMV_VDec_DumpReg(u4InstID, FALSE);
        
      return (FALSE);
    }

    printk("%08x", u4CRCResult);
  }  
  printk("\n");

  //C
  printk("CRC_C: ");
  for (i=0; i<4; i++)
  {
    u4CRCResult = u4ReadReg(VDEC_CRC_REG_OFFSET0 + ((6+i)*4));
    u4SWResult = *(pu4CRCResultCurrAddr+4+i);
    if (u4SWResult != u4CRCResult)
    {
      printk("u4SWResult: 0x%x, u4CRCResult: 0x%x\n", u4SWResult, u4CRCResult);
      printk("UV CRC Compare Golden Error\n");
      
      //vVDEC_HAL_WMV_VDec_DumpReg(u4InstID, FALSE);
      
      return (FALSE);
    }

    printk("%08x", u4CRCResult);
  }
  printk("\n");

  return (TRUE);
}

// *********************************************************************
// Function    : BOOL fgCompWMVChkSumGolden(UINT32 u4InstID)
// Description : write check sum in rec file
// Parameter   : None
// Return      : TRUE pass, FALSE err
// *********************************************************************
CHAR _bPrevfileStrl[300] = {'\0'};
BOOL fgCompWMVChkSumGolden(UINT32 u4InstID)
{
    printk("fgCompWMVChkSumGolden\n");
    BOOL fgIsCompNoErr,fgOpen;
 #ifndef WMV_CRCCHECK_ENABLE   
    UINT32 *pu4GoldenCheckSum;
 #endif
    UINT32 u4Temp;
 #ifdef WMV_CRC_COMPOSITE_CHECK_ENABLE
    char _CRCFolder[100] = "/mnt/sdcard/VC1_CRC/";
 #else
    char _CRCFolder[100] = "/mnt/sdcard/VC1_CRC/BD";
 #endif
    char ShortFileName[100];
    char *FileName = strrchr(_bFileStr1[u4InstID][0],'/') + 1;

    //u4Temp = sprintf(_bTempStr1[u4InstID], "%s", _bFileStr1[u4InstID][0]);
#ifdef WMV_CRC_COMPOSITE_CHECK_ENABLE
    u4Temp = sprintf(_bTempStr1[u4InstID], "%s%s", _CRCFolder, FileName);
#else
    strcpy(ShortFileName, FileName);
    ShortFileName[strlen(ShortFileName)-1] = 0;
    u4Temp = sprintf(_bTempStr1[u4InstID], "%s%s/%s", _CRCFolder, ShortFileName, FileName);
#endif
    _tTempFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tTempFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tTempFileInfo[u4InstID].u4TargetSz = 0x100000;//GOLD_Y_SZ;  
    _tTempFileInfo[u4InstID].u4FileLength = 0; 

#ifdef WMV_CRCCHECK_ENABLE    
#ifdef WMV_CRC_COMPOSITE_CHECK_ENABLE
    u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp - 1, "_crc.bin");
    //u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp - 1, ".crc");
#else
     if (_rWMVPPS[u4InstID].ucFrameCodingMode == INTERLACEFIELD) 
     {
         u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp - 1, "crc_g_%d_%d.dat", _u4FileCnt[u4InstID],_rWMVPPS[u4InstID].i4CurrentTemporalField);
        // if (_rWMVPPS[u4InstID].i4CurrentTemporalField == 1)
      }
    else
     {
         u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp - 1, "crc_g_%d.dat", _u4FileCnt[u4InstID]);
     }
#endif
    printk("_bTempStr1[u4InstID] %s, _bPrevfileStrl %s\n",_bTempStr1[u4InstID], _bPrevfileStrl);
    if (strcmp(_bTempStr1[u4InstID], _bPrevfileStrl))
    {
        fgOpen = fgOpenFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    }
    else
    {
        fgOpen = TRUE;
    }

    strcpy(_bPrevfileStrl, _bTempStr1[u4InstID]);
    
    if(fgOpen)
    {
        #ifdef WMV_CRC_COMPOSITE_CHECK_ENABLE
        fgIsCompNoErr = fgWMV_CheckCRCResult(u4InstID, _u4CRCCnt[u4InstID], (UINT32)_pucDumpYBuf[u4InstID]);      
        #else
        fgIsCompNoErr = fgWMV_CheckCRCResult(u4InstID, (UINT32)_pucDumpYBuf[u4InstID]);      
        #endif
    }
    else
    {
        fgIsCompNoErr = FALSE;
        msleep(1000);
        printk("[WMV] Open CRC golden fail\n");
        msleep(1000);
    }
#else
    u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%d_chksum.bin", _u4FileCnt[u4InstID]);
    fgOpen = fgOpenPCFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    pu4GoldenCheckSum = (UINT32 *)_pucDumpYBuf[u4InstID];
    if(fgOpen)
    {
        if((!fgWMVIsFrmPic(u4InstID)) && (_rWMVPPS[u4InstID].i4CurrentTemporalField== 1)) //Only 2nd field pic needs offset 256
        {
            pu4GoldenCheckSum += MAX_CHKSUM_NUM;
        }
        else
        {
            pu4GoldenCheckSum += 0;
        }   
        fgIsCompNoErr = fgVDEC_HAL_WMV_VDec_CompCheckSum(&_u4DumpChksum[u4InstID][0], pu4GoldenCheckSum);      
    }
    else
    {
        fgIsCompNoErr = FALSE;
    }
#endif
    return fgIsCompNoErr;
}

// *********************************************************************
// Function    : BOOL fgCompMPEGChkSumGolden(UINT32 u4InstID)
// Description : write check sum in rec file
// Parameter   : None
// Return      : TRUE pass, FALSE err
// *********************************************************************
BOOL fgCompMPEGChkSumGolden(UINT32 u4InstID)
{
#if 0
    UINT32 u4HW_Y_Result[4],u4HW_CbCr_Result[4],u4Golden;
	UINT32 u4FileNameLen;
	UINT32 i;
	BOOL fgOpen = FALSE;
	BOOL fgCmpRet = TRUE;

	CHAR _bFileNameYCRC[20] = {"Ycrc.bin\0"};
	CHAR _bFileNameCCRC[20] = {"Ccrc.bin\0"};

	UINT32 u4Temp;

    
	   

	if(fgYCrcOpen == FALSE)
	{
	    u4Temp = sprintf(_bFileStr1[u4InstID][14], "%s", _bFileStr1[u4InstID][0]);
        u4Temp += sprintf(_bFileStr1[u4InstID][14] + u4Temp, "%s", _bFileNameYCRC); 

	    u4Temp = sprintf(_bFileStr1[u4InstID][15], "%s", _bFileStr1[u4InstID][0]);
        u4Temp += sprintf(_bFileStr1[u4InstID][15] + u4Temp, "%s", _bFileNameCCRC); 

		
		printk("<vdec>Y CRC file name = %s\n",_bFileStr1[u4InstID][14]);
		printk("<vdec>CbCr CRC file name = %s\n",_bFileStr1[u4InstID][15]);

		//Open Y crc file
        //_tInFileInfo[u4InstID].fgGetFileInfo = TRUE;
		_tInFileInfo[u4InstID].pucTargetAddr = _pucCRCYBuf[u4InstID];
		_tInFileInfo[u4InstID].u4TargetSz = CRC_SZ;
		_tInFileInfo[u4InstID].u4FileLength = 0;
		_tInFileInfo[u4InstID].u4FileOffset = 0;

		fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][14],"r+b", &_tInFileInfo[u4InstID]);//Load bitstream
		if(fgOpen == FALSE)
		{
			msleep(100);
			printk("<vdec> Open Y crc file error!\n");
		}
		else
		{
			fgYCrcOpen = TRUE;
		   printk("[VDEC-MPEG2] Opne Y crc file OK!\n");
		}

		_tInFileInfo[u4InstID].pucTargetAddr = _pucCRCCBCRBuf[u4InstID];
		_tInFileInfo[u4InstID].u4TargetSz = CRC_SZ;
		_tInFileInfo[u4InstID].u4FileLength = 0;
		_tInFileInfo[u4InstID].u4FileOffset = 0;

		fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][15],"r+b", &_tInFileInfo[u4InstID]);//Load bitstream
		if(fgOpen == FALSE)
		{
			printk("<vdec> Open cbcr crc file error!\n");
		}
		else
		{
			fgCbCrCrcOpen = TRUE;
		//	printk("[VDEC-MPEG4] Opne C crc file OK!\n");
		}

		u4FileTotalPicNum =_tInFileInfo[u4InstID].u4FileLength >> 4;
	}
		for(i = 0; i < 4; i++)
		{
			u4HW_Y_Result[i] = u4VDecReadCRC(u4InstID,(i + 2) <<2);
			u4Golden = (((UINT32 *)(_pucCRCYBuf[u4InstID]))[(u4FilePicNum << 2) + i]);
			if(u4HW_Y_Result[i] != u4Golden)
			{
				fgCmpRet = FALSE;
				printk("<vdec>Y CRC Compare failed!\n");
				printk("<vdec>HW = 0x%x,golden = 0x%x\n",u4HW_Y_Result[i],u4Golden);
			}
		}

		for(i = 0; i < 4; i++)
		{
			u4HW_CbCr_Result[i] = u4VDecReadCRC(u4InstID,(i + 6) <<2);
			u4Golden = (((UINT32 *)_pucCRCCBCRBuf[u4InstID])[(u4FilePicNum << 2) + i]);
			if(u4HW_CbCr_Result[i] != u4Golden)
			{
				fgCmpRet = FALSE;
				printk("<vdec>CBCR CRC Compare failed!\n");
				printk("<vdec>HW = 0x%x,golden = 0x%x\n",u4HW_CbCr_Result[i],u4Golden);
			}
		}
		
		if((u4FilePicNum < u4FileTotalPicNum) && fgCmpRet)
		{
			if(!(u4FilePicNum%1))
				printk("<vdec> Decode ok @ pic %d\n",u4FilePicNum);
			
			u4FilePicNum++;
			_u4FileCnt[u4InstID]++;


			if((u4FilePicNum == u4FileTotalPicNum) || (u4FilePicNum > _u4EndCompPicNum[u4InstID]))
			{
				printk("<vdec>OK ==>Compare complete total pic = %d, 1st error @ %u\n",u4FilePicNum, u4FirstErrFrm);
				printk("<vdec>OK bitstream = %s ok @ %d\n",_bFileStr1[u4InstID][1],u4FilePicNum);
				memset(&_tVerMpvDecPrm[u4InstID],0,sizeof(VDEC_INFO_DEC_PRM_T));

				u4FilePicNum = 0;
				u4FilePicCont_noVOP = 0;
				_u4FileCnt[u4InstID] = 0;
				fgYCrcOpen = FALSE;
				fgCbCrCrcOpen = FALSE;
				_u4VerBitCount[u4InstID] = 0xffffffff;
				u4FirstErrFrm = 0xFFFFFFFF;
			}
		}

		#if 0
		else
		{
		
			if(!fgCmpRet)
			{
				{
					u4FilePicNum++;
					_u4FileCnt[u4InstID]++;
				}
				if (u4FirstErrFrm == 0xFFFFFFFF)
				{
					u4FirstErrFrm = u4FilePicNum;
				}
				//vPrintDumpReg(u4InstID,0);
				//vPrintDumpReg(u4InstID,1);
				printk("<vdec>NG ==>Compare mismatch @ pic %d decode order %d\n",u4FilePicNum,u4FilePicCont_noVOP - 1);
				printk("<vdec>NG ==> %s,mismatch @ %d\n",_bFileStr1[u4InstID][1],u4FilePicNum);
				//if((u4FilePicNum == u4FileTotalPicNum) || (u4FilePicNum > _u4EndCompPicNum[u4InstID]))
				{         
					u4FilePicNum = 0;
					u4FilePicCont_noVOP = 0;
					_u4FileCnt[u4InstID] = 0;
					fgYCrcOpen = FALSE;
					fgCbCrCrcOpen = FALSE;
					_u4VerBitCount[u4InstID] = 0xffffffff;
					u4FirstErrFrm = 0xFFFFFFFF;
				}
			} 
			else
			{
				u4FilePicNum = 0;
				u4FilePicCont_noVOP = 0;
				_u4FileCnt[u4InstID] = 0;
				fgYCrcOpen = FALSE;
				fgCbCrCrcOpen = FALSE;
				_u4VerBitCount[u4InstID] = 0xffffffff;
				u4FirstErrFrm = 0xFFFFFFFF;
			}
            
		}
		#endif

		return fgCmpRet;
		
	



#else
    BOOL fgIsCompNoErr,fgOpen;
    UINT32 *pu4GoldenCheckSum;
    UINT32 u4Temp;
   // printk("<vdec> fgCompMPEGChkSumGolden\n");  

    //u4Temp = sprintf(_bTempStr1[u4InstID], "%s", _bFileStr1[u4InstID][0]);
    vConcateStr(_bTempStr1[u4InstID], _bFileStr1[u4InstID][0], "_0.crc\0", _u4FileCnt[u4InstID]);
    _tTempFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tTempFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tTempFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tTempFileInfo[u4InstID].u4FileLength = 0; 

    //u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%d_chksum.bin", _u4FileCnt[u4InstID]);
    //fgOpen = fgOpenPCFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    fgOpen = fgOpenFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    pu4GoldenCheckSum = (UINT32 *)_pucDumpYBuf[u4InstID];
    if(fgOpen)
    {
    #if 1
        fgIsCompNoErr = fgVDEC_HAL_MPEG_VDec_CompCheckSum(&_u4DumpChksum[u4InstID][0], pu4GoldenCheckSum);	    
	#else
        if((!fgMPEGIsFrmPic(u4InstID)) && (_fgDec2ndFldPic[u4InstID]== 1)) //Only 2nd field pic needs offset 256
        {
            pu4GoldenCheckSum += MAX_CHKSUM_NUM;
        }
        else
        {
            pu4GoldenCheckSum += 0;
        }   
        fgIsCompNoErr = fgVDEC_HAL_MPEG_VDec_CompCheckSum(&_u4DumpChksum[u4InstID][0], pu4GoldenCheckSum);      
	#endif
    }
    else
    {
        fgIsCompNoErr = FALSE;
    }

    return fgIsCompNoErr;
#endif

}

/*--------------------------------------------------------------------------------
Golden pattern generation of video down scaler
Command Usage: vdscl srcWidth srcHeight targetWidth targetHeight pic_stru tgbufLen
                        h_ofst v_ofst src_agent spec_type mbaff v_fmt convert_en fg_src
######pic_stru:   0: frame mode, 1: top field, 2: bot field, 3: two field, 4: both fields
######tgbufLen:   the length of one row in the target buffer (unit: pxl)
######h_ofst:     the horizontal writing offset (unit:pxl)
######v_ofst:     the vertical writing offset
######src_agent:  0: MC, 2: deblocking, 4: fgt
######spec_type:  0: Mpeg2, Mpeg4, 1: WMV, 2: h264
######mbaff:      1: h264 mbaff
######v_fmt:      420(0) / 422(1)
######convert_en: color space convert enable (709 -> 601), only support in mc case
######fg_src:     the source of flim grain, mc(0) / pp(1)
Input: block based (only support 128bit arrangement) / raster scan data
Input file name (auto): golden_ori_y, golden_ori_c
Output: block based (only support 128bit arrangement) & raster scan data
Output file name (auto): golden_<targetWidth>x<targetHeight>_y_rs,
                                  golden_<targetWidth>x<targetHeight>_c_rs,
                                  golden_<targetWidth>x<targetHeight>_y_blk,
                                  golden_<targetWidth>x<targetHeight>_c_blk
--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------
Golden pattern generation of video down scaler
Command Usage: vdscl srcWidth srcHeight targetWidth targetHeight pic_stru tgbufLen
h_ofst v_ofst src_agent spec_type mbaff v_fmt convert_en fg_src
			   y_src_h_ofst c_src_h_ofst y_src_v_ofst c_src_v_ofst			   
			   y_scl_h_ofst c_scl_h_ofst y_scl_v_ofst c_scl_v_ofst
######pic_stru:     0: frame mode, 1: top field, 2: bot field, 3: two field, 4: both fields
######tgbufLen:     the length of one row in the target buffer (unit: pxl)
######h_ofst:       the horizontal writing offset (unit:pxl)
######v_ofst:       the vertical writing offset
######src_agent:    0: MC, 2: deblocking, 4: fgt
######spec_type:    0: Mpeg2, Mpeg4, 1: WMV, 2: h264
######mbaff:        1: h264 mbaff
######v_fmt:        420(0) / 422(1)
######convert_en:   color space convert enable (709 -> 601), only support in mc case
######fg_src:       the source of flim grain, mc(0) / pp(1)
######y_src_h_ofst: horizontal source offset of y component, max = 8 
######c_src_h_ofst: horizontal source offset of c component, max = 4
######y_src_v_ofst: vertical source offset of y component, max = 8
######c_src_v_ofst: vertical source offset of c component, max = 8
######y_scl_h_ofst: horizontal scale offset of y component, max = y_h_scl_factor 
######c_scl_h_ofst: horizontal scale offset of c component, max = c_h_scl_factor
######y_scl_v_ofst: vertical scale offset of y component, max = y_v_scl_factor
######c_scl_v_ofst: vertical scale offset of c component, max = c_v_scl_factor
######Input: block based (only support 128bit arrangement) / raster scan data
######Input file name (auto): golden_ori_y, golden_ori_c
######Output: block based (only support 128bit arrangement) & raster scan data
######Output file name (auto):  golden_<targetWidth>x<targetHeight>_y_rs,
                                  golden_<targetWidth>x<targetHeight>_c_rs,
                                  golden_<targetWidth>x<targetHeight>_y_blk,
                                  golden_<targetWidth>x<targetHeight>_c_blk
--------------------------------------------------------------------------------*/
void vGenerateDownScalerGolden(UINT32 u4InstID, UINT32 DecYAddr,UINT32 DecCAddr, UINT32 u4Size)
{
    UCHAR *srcDataY, *srcDataC, *targetData, *VDSCLData;
    UCHAR *DecDataY, *DecDataC;
    int srcWidth, srcHeight, targetWidth, targetHeight;
    int pic_stru, tgbufLen, h_ofst, v_ofst;
    int src_agent, spec_type, mbaff;
    int v_fmt, convert_en, fg_src;
    UCHAR tmpbuf;
    int srcbufLen, tgbufHgh;
    int srcblkCnt, tgblkCnt, tgblkCnt16;
    int i, j, k, l,h;
#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8530)    
    int y_src_h_ofst, c_src_h_ofst, y_src_v_ofst, c_src_v_ofst;
    int y_scl_h_ofst, c_scl_h_ofst, y_scl_v_ofst, c_scl_v_ofst;
#endif

    // @04072009. support luma key    
    int lk_en, luma_key;    
    int y_coord_0, y_coord_1, y_coord_2, y_coord_3;    
    int lk_region0, lk_region1, lk_region2, lk_region3;

    // for color space conversion
    int Y601, Cb601, Cr601;
    int Y709, Cb709, Cr709;

    DecDataY = (UCHAR*)DecYAddr;
    DecDataC = (UCHAR*)DecCAddr;
    srcWidth     = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4SrcWidth;
    srcHeight    = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4SrcHeight;
    targetWidth  = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgWidth;
    targetHeight = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight;

#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8530)  
    y_src_h_ofst = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4SrcYOffH;
    c_src_h_ofst = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4SrcCOffH;
    y_src_v_ofst = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4SrcYOffV ;
    c_src_v_ofst = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4SrcCOffV ;
    y_scl_h_ofst = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4SclYOffH ;
    c_scl_h_ofst = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4SclCOffH ;
    y_scl_v_ofst = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4SclYOffV ;
    c_scl_v_ofst = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4SclCOffV ;
#endif

    switch (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct)
    {
      case TOP_FIELD:
      case BOTTOM_FIELD:
        pic_stru     = 4;
        srcHeight = srcHeight*2;
        targetHeight = targetHeight*2;
        break;
      case FRAME:
        pic_stru = PIC_STRUCT_FRAME;
        break;
      default:
        pic_stru = PIC_STRUCT_TOP_BOTTOM;
        break;
    }

    
    tgbufLen     = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgBufLen;
    h_ofst       = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffH;
    v_ofst       = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgOffV;
    src_agent    = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScrAgent;
    spec_type    = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucSpectType;
    v_fmt        = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucVdoFmt;
    convert_en   = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.fgEnColorCvt;
    if(_u4CodecVer[u4InstID] == VDEC_H264)
    {
      if((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct != TOP_FIELD) && (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct != BOTTOM_FIELD))
      {
        mbaff      = _tVerMpvDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prSPS->fgMbAdaptiveFrameFieldFlag;
      }
      else
      {
        mbaff      = 0;
      }
      fg_src       = 1;
    }
    else
    {
      mbaff        = 0;
      fg_src       = 0;
    }

    // y
    srcbufLen  = ((srcWidth+15)/16)*16;
    srcblkCnt  = ((srcHeight+31)/32);
    //tgbufLen = ((targetWidth+15)/16)*16;
    tgbufHgh   = ((targetHeight + v_ofst + 31)/32)*32; // multiple of 32
    tgblkCnt   = tgbufHgh/32;
    tgblkCnt16 = tgbufHgh/16;
    
    srcDataY = (UCHAR *)_pucVDSCLWork2Sa[u4InstID];
    for(i=0;i<srcbufLen;i++)
      for(j=0;j<srcHeight;j++)
         *(srcDataY++) = 0;
    srcDataC = (UCHAR *) _pucVDSCLWork3Sa[u4InstID];
    for(i=0;i<srcbufLen;i++)
      for(j=0;j<(srcHeight>>1);j++)
         *(srcDataC++) = 0;
    targetData = (UCHAR *) _pucVDSCLWork4Sa[u4InstID];
    for(i=0;i<tgbufLen;i++)
      for(j=0;j<tgbufHgh;j++)
         *(targetData++) = 0;

    srcDataY = (UCHAR *)_pucVDSCLWork2Sa[u4InstID];
    srcDataC = (UCHAR *) _pucVDSCLWork3Sa[u4InstID];
    targetData = (UCHAR *) _pucVDSCLWork4Sa[u4InstID];
    // Y component in---------------------------------------------------------------

    for (i = 0; i < srcblkCnt; i++) // block count
        for (k=0; k < (srcbufLen/16); k++)
            for (j=0; j < 32; j++) // line count
                for (l=0; l<16; l++)
                {
                    if ((i*32+j) >= srcHeight)
                    {
                        tmpbuf = *(DecDataY++);  // unnecessary data
                    }
                    else
                    {
                        tmpbuf = *(DecDataY++);
                        srcDataY[(i*32+j)*srcbufLen+k*16+l] = (UCHAR)tmpbuf;
                    }
                }


    // C component in---------------------------------------------------------------
    for (i = 0; i < srcblkCnt; i++) // block count
        for (k=0; k < (srcbufLen/16); k++)
            for (j=0; j < 16; j++) // line count
                for (l=0; l<16; l++)
                {
                    if ((i*16+j) >= (srcHeight/2))
                    {
                        tmpbuf = *(DecDataC++);  // unnecessary data
                    }
                    else
                    {
                        tmpbuf = *(DecDataC++);
                        srcDataC[(i*16+j)*srcbufLen+k*16+l] = (UCHAR)tmpbuf;
                    }
                }

   // @04072009. support luma key    
   lk_en = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.fgLumaKeyEn;
   luma_key = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u2LumaKeyValue;
   if (lk_en == 1)    
   {
       for (j=0; j < srcHeight/2; j++)            
           for (i=0; i < srcWidth/2; i++)            
           {                
                     y_coord_0 = srcbufLen*2*j+i*2;                
                     y_coord_1 = srcbufLen*(2*j+1)+i*2;                
                     y_coord_2 = srcbufLen*2*j+i*2+1;                
                     y_coord_3 = srcbufLen*(2*j+1)+i*2+1;                
                     lk_region0 = (srcDataY[y_coord_0] <= luma_key) ? 1 : 0;                
                     lk_region1 = (srcDataY[y_coord_1] <= luma_key) ? 1 : 0;               
                     lk_region2 = (srcDataY[y_coord_2] <= luma_key) ? 1 : 0;                
                     lk_region3 = (srcDataY[y_coord_3] <= luma_key) ? 1 : 0;                
                     if ((lk_region0>0) && (lk_region1>0) && (lk_region2>0) && (lk_region3>0))                
                     {                    
                        srcDataC[j*srcbufLen+i*2] = 0x80; // Cb                    
                        srcDataC[j*srcbufLen+i*2+1] = 0x80; // Cr                
                     }            
           }    
    }

    // color space conversion
    if (convert_en == 1)
    {
        // Y component conversion
        if ((pic_stru != 3) && (pic_stru != 4))
        {
            for (j=0; j < srcHeight; j++)
                for (i=0; i < srcWidth; i++)
                {
                    Y709  = (int) srcDataY[srcbufLen*j+i];
                    if ((i%2) == 0)
                    {
                        Cb709 = ((int) srcDataC[srcbufLen*(j/2)+i]) - 128;
                        Cr709 = ((int) srcDataC[srcbufLen*(j/2)+i+1]) - 128;
                    }
                    else
                    {
                        Cb709 = ((int) srcDataC[srcbufLen*(j/2)+i-1]) - 128;
                        Cr709 = ((int) srcDataC[srcbufLen*(j/2)+i]) - 128;
                    }
                    //Original
                    //Y601 = (int)(Y709 + Cb709 * 0.099609375 + Cr709 * 0.19140625 + 0.5);
                    // modified @07012008
				Y601 = (Y709*1024 + Cb709*102 + Cr709*196 + 512) >> 10;
                    if (Y601 > 255)
                        srcDataY[srcbufLen*j+i] = 255;
                    else if (Y601 <0)
                        srcDataY[srcbufLen*j+i] = 0;
                    else
                        srcDataY[srcbufLen*j+i] = (UCHAR) Y601;
                }
        }
        else // interlaced frame
        {
            for (j=0; j < srcHeight; j=j+2)
                for (i=0; i < srcWidth; i++)
                {
                    Y709  = (int) srcDataY[srcbufLen*j+i];
                    if ((i%2) == 0)
                    {
                        Cb709 = ((int) srcDataC[srcbufLen*2*(j/4)+i]) - 128;
                        Cr709 = ((int) srcDataC[srcbufLen*2*(j/4)+i+1]) - 128;
                    }
                    else
                    {
                        Cb709 = ((int) srcDataC[srcbufLen*2*(j/4)+i-1]) - 128;
                        Cr709 = ((int) srcDataC[srcbufLen*2*(j/4)+i]) - 128;
                    }
                    //Original
                    //Y601 = (int)(Y709 + Cb709 * 0.099609375 + Cr709 * 0.19140625 + 0.5);
                    // modified @07012008
				Y601 = (Y709*1024 + Cb709*102 + Cr709*196 + 512) >> 10;
                    if (Y601 > 255)
                        srcDataY[srcbufLen*j+i] = 255;
                    else if (Y601 <0)
                        srcDataY[srcbufLen*j+i] = 0;
                    else
                        srcDataY[srcbufLen*j+i] = (UCHAR) Y601;
                }

            for (j=1; j < srcHeight; j=j+2)
                for (i=0; i < srcWidth; i++)
                {
                    Y709  = (int) srcDataY[srcbufLen*j+i];
                    if ((i%2) == 0)
                    {
                        Cb709 = ((int) srcDataC[srcbufLen*(2*(j/4)+1)+i]) - 128;
                        Cr709 = ((int) srcDataC[srcbufLen*(2*(j/4)+1)+i+1]) - 128;
                    }
                    else
                    {
                        Cb709 = ((int) srcDataC[srcbufLen*(2*(j/4)+1)+i-1]) - 128;
                        Cr709 = ((int) srcDataC[srcbufLen*(2*(j/4)+1)+i]) - 128;
                    }
                    //Original
                    //Y601 = (int)(Y709 + Cb709 * 0.099609375 + Cr709 * 0.19140625 + 0.5);
                    // modified @07012008
				Y601 = (Y709*1024 + Cb709*102 + Cr709*196 + 512) >> 10;
                    if (Y601 > 255)
                        srcDataY[srcbufLen*j+i] = 255;
                    else if (Y601 <0)
                        srcDataY[srcbufLen*j+i] = 0;
                    else
                        srcDataY[srcbufLen*j+i] = (UCHAR) Y601;
                }
        }

        // C component conversion
        for (j=0; j < srcHeight/2; j++)
            for (i=0; i < srcWidth/2; i++)
            {
                Cb709 = ((int) srcDataC[srcbufLen*j+2*i]) - 128;
                Cr709 = ((int) srcDataC[srcbufLen*j+2*i+1]) - 128;
                
                 // original
			  // Cb601 = (int)(0.9902343750 * Cb709 - 0.1103515625 * Cr709 + 128.5);				    
			  // Cr601 = (int)(0.9833984375 * Cr709 - 0.0722656250 * Cb709 + 128.5);			      
			  // modified @07012008
			  Cb601 = (1014 * Cb709 - 113 * Cr709 + 131584) >> 10;
                Cr601 = (1007 * Cr709 - 74 * Cb709 + 131584) >> 10;
                if (Cb601 > 255)
                    srcDataC[srcbufLen*j+2*i] = 255;
                else if (Cb601 < 0)
                    srcDataC[srcbufLen*j+2*i] = 0;
                else
                    srcDataC[srcbufLen*j+2*i] = (UCHAR) Cb601;

                if (Cr601 > 255)
                    srcDataC[srcbufLen*j+2*i+1] = 255;
                else if (Cr601 < 0)
                    srcDataC[srcbufLen*j+2*i+1] = 0;
                else
                    srcDataC[srcbufLen*j+2*i+1] = (UCHAR) Cr601;
            }
    }

    // Evaluate Y component ---------------------------------------------------------------
    /**** do down scaling ****/
#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8520) 
    vdscl (u4InstID, srcWidth, srcHeight, targetWidth, targetHeight, srcDataY, targetData, 0, pic_stru, 
            tgbufLen, h_ofst, v_ofst, src_agent, spec_type, mbaff, fg_src);
#else
    vdscl (u4InstID, srcWidth, srcHeight, targetWidth, targetHeight, srcDataY, targetData, 0, pic_stru,
            tgbufLen, h_ofst, v_ofst, src_agent, spec_type, mbaff, fg_src,
            y_src_h_ofst, y_src_v_ofst, y_scl_h_ofst, y_scl_v_ofst, lk_en, luma_key);
#endif

    //x_memset(_pucDumpYBuf[u4InstID], 0x0, GOLD_Y_SZ);
    //x_memset(_pucDumpCBuf[u4InstID], 0x0, GOLD_C_SZ);
    VDSCLData = (UCHAR *)_pucDumpYBuf[u4InstID];
    /**** output result to file (raster scan) ****/
    if(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType==1)
    {
       for (j=0; j < tgbufHgh; j++)
       {
          //if (pic_stru == 2) // bot field
          if (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD)
          {
              for (i=0; i < tgbufLen/16; i++)
              {
                 for(h=0;h<16;h++)
                 {
                        VDSCLData++;
                        //*(VDSCLData++);
                        //*(VDSCLData++) = 0;
                   //fwrite(&Zero[0],sizeof(char),16,file_outY_rs);
                 }
               }
                j++;
         }
          
         for (i=0; i < tgbufLen/16; i++)
         {
             for(h=0;h<16;h++)
             {
                 *(VDSCLData++) = targetData[j*tgbufLen+16*i+h];
                 //fwrite(&targetData[j*tgbufLen+16*i],sizeof(char),16,file_outY_rs);
             }
          }
          
          //if (pic_stru == 1) // top field
          if (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD)
          {
              for (i=0; i < tgbufLen/16; i++)
              {
                   for(h=0;h<16;h++)
                   {
                       VDSCLData++;
                      //*(VDSCLData++);
                      //*(VDSCLData++) = 0;
                      //fwrite(&Zero[0],sizeof(char),16,file_outY_rs);
                   }
              }
               j++;
           }
        }
    }
    else
    {
      /**** output result to file (block based) ****/
      if ((pic_stru == 0) || (pic_stru == 3) || (pic_stru == 4))
      // progressive or interlaced frame
      {
          for (i = 0; i < tgblkCnt; i++) // block count
            {
              for (k=0; k < (tgbufLen/16); k++)
                {
                  for (j=0; j < 32; j++) // line count
                  {
                      if ((i*32+j) >= tgbufHgh)
                      {
                        for(h=0;h<16;h++)
                        {
                           *(VDSCLData++) = 0;
                           //fwrite(&Zero[0],sizeof(char),16,file_outY_blk);
                         }
                      }
                      else if(((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD) && (j % 2 == 1)) || ((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD) && (j % 2 == 0)))
                      {
                          for(h=0;h<16;h++)
                         {
                             VDSCLData++;
                             //*(VDSCLData++);
                         }
                      }
                      else
                      {
                          for(h=0;h<16;h++)
                          {
                             *(VDSCLData++) = targetData[(i*32+j)*tgbufLen+k*16+h];
                            //fwrite(&targetData[(i*32+j)*tgbufLen+k*16],sizeof(char),16,file_outY_blk);
                          }
                      }
                   }
                }
            }
      }
      else // field picture: top or bottom 
      {
          for (i = 0; i < tgblkCnt16; i++) // block count
            {
              for (k=0; k < (tgbufLen/16); k++)
                {
                  for (j=0; j < 16; j++) // line count
                  {
                      //if (pic_stru == 2) // bot field
                      if (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD)
                      {
                          for(h=0;h<16;h++)
                          {
                              VDSCLData++;
                              //*(VDSCLData++);
                             //*(VDSCLData++) = 0;
                             //fwrite(&Zero[0],sizeof(char),16,file_outY_blk);
                           }
                     }
      
                      if ((i*16+j) >= tgbufHgh)
                      {
                          for(h=0;h<16;h++)
                          {
                              VDSCLData++;
                              //*(VDSCLData++);
                              //*(VDSCLData++) = 0;
                              //fwrite(&Zero[0],sizeof(char),16,file_outY_blk);
                           }
                     }
                     else
                     { 
                         for(h=0;h<16;h++)
                         {
                            *(VDSCLData++) = targetData[(i*16+j)*tgbufLen+k*16+h];
                            //fwrite(&targetData[(i*16+j)*tgbufLen+k*16],sizeof(char),16,file_outY_blk);
                         }
                     }
                        //if (pic_stru == 1) // top field
                      if (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD)
                      {
                          for(h=0;h<16;h++)
                          {
                              VDSCLData++;
                              //*(VDSCLData++);
                             //*(VDSCLData++) = 0;
                             //fwrite(&Zero[0],sizeof(char),16,file_outY_blk);
                         }
                     }
                  }
               }
            }
        }
    }


    //srcData = (UCHAR *) malloc(srcbufLen*srcHeight/2*sizeof(UCHAR));
    //memset(srcData, 0, srcbufLen*srcHeight/2);
    if (v_fmt == 1) // 422
    {
        targetData = (UCHAR *) _pucVDSCLWork4Sa[u4InstID];
        for(i=0;i<tgbufLen;i++)
          for(j=0;j<tgbufHgh;j++)
             *(targetData++) = 0;
    }
    else
    {
        targetData = (UCHAR *) _pucVDSCLWork4Sa[u4InstID];
        for(i=0;i<tgbufLen;i++)
          for(j=0;j<(tgbufHgh>>1);j++)
             *(targetData++) = 0;
    }
    targetData = (UCHAR *) _pucVDSCLWork4Sa[u4InstID];
    VDSCLData = (UCHAR *)_pucDumpCBuf[u4InstID];
    // Evaluate C component ---------------------------------------------------------------
    /**** do down scaling ****/
    if (v_fmt == 0) // 420
    {
        targetHeight = targetHeight/2;
        v_ofst = v_ofst/2;
        tgbufHgh = tgbufHgh/2;
        
    }
    else
    {
        tgblkCnt = tgblkCnt * 2;
        tgblkCnt16 = tgblkCnt16 *2;
    }

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8520) 
    vdscl (u4InstID, srcWidth, srcHeight/2, targetWidth, targetHeight, srcDataC, targetData, 1, pic_stru,
            tgbufLen, h_ofst, v_ofst, src_agent, spec_type, mbaff, fg_src);
#else
    vdscl (u4InstID, srcWidth, srcHeight/2, targetWidth, targetHeight, srcDataC, targetData, 1, pic_stru,
            tgbufLen, h_ofst, v_ofst, src_agent, spec_type, mbaff, fg_src,
            c_src_h_ofst, c_src_v_ofst, c_scl_h_ofst, c_scl_v_ofst, 0, 0);
#endif

    if(_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucScanType==1)
    {
      /**** output result to file (raster scan) ****/
      for (j=0; j < tgbufHgh; j++)
      {
            //if (pic_stru == 2) // bot field
            if (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD)
            {
                for (i=0; i < tgbufLen/16; i++)
                {
                   for(h=0;h<16;h++)
                   {
                       VDSCLData++;
                       //*(VDSCLData++);
                       //*(VDSCLData++) = 0;
                      //fwrite(&Zero[0],sizeof(char),16,file_outC_rs);
                   }
                }
                j++;
            }
      
            for (i=0; i < tgbufLen/16; i++)
            {
                for(h=0;h<16;h++)
                {
                    *(VDSCLData++) = targetData[j*tgbufLen+16*i+h];
                   //fwrite(&targetData[j*tgbufLen+16*i],sizeof(char),16,file_outC_rs);
                }
            }
      
            //if (pic_stru == 1) // top field
            if (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD)
            {
                for (i=0; i < tgbufLen/16; i++)
                {
                    for(h=0;h<16;h++)
                    {
                        VDSCLData++;
                        //*(VDSCLData++);
                        //*(VDSCLData++) = 0;
                        //fwrite(&Zero[0],sizeof(char),16,file_outC_rs);
                    }
                }
                j++;
            }
        }
    }
    else
    {
      /**** output result to file (block based) ****/
      if ((pic_stru == 0) || (pic_stru == 3) || (pic_stru == 4))
      // progressive or interlaced frame
      {
          for (i = 0; i < tgblkCnt; i++) // block count
            {
              for (k=0; k < (tgbufLen/16); k++)
                {
                  for (j=0; j < 16; j++) // line count
                  {
                      if ((i*16+j) >= (tgbufHgh))
                      {
                         for(h=0;h<16;h++)
                         {
                             //*(VDSCLData++);
                              *(VDSCLData++) = 0;
                             //fwrite(&Zero[0],sizeof(char),16,file_outC_blk);
                         }
                      }
                      else if(((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD) && (j % 2 == 1)) || ((_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD) && (j % 2 == 0)))
                      {                           
                         for(h=0;h<16;h++)        
                         {         
                            VDSCLData++;
                            //*(VDSCLData++);             
                         }                   
                      }
                      else
                      {
                         for(h=0;h<16;h++)
                         {
                            *(VDSCLData++) = targetData[(i*16+j)*tgbufLen+k*16+h];
                           //fwrite(&targetData[(i*16+j)*tgbufLen+k*16],sizeof(char),16,file_outC_blk);
                         }
                      }
                    }
                }
            }
        }
      else // field picture: top or bottom 
      {
          for (i = 0; i < tgblkCnt16; i++) // block count
          {
              for (k=0; k < (tgbufLen/16); k++)
              {
                  for (j=0; j < 8; j++) // line count
                  {
                      //if (pic_stru == 2) // bot field
                      if (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == BOTTOM_FIELD)
                      {
                          for(h=0;h<16;h++)
                          {
                              VDSCLData++;
                              //*(VDSCLData++);
                              //*(VDSCLData++) = 0;
                              //fwrite(&Zero[0],sizeof(char),16,file_outC_blk);
                           }
                      }
      
                      if ((i*8+j) >= (tgbufHgh))
                      {
                          for(h=0;h<16;h++)
                          {
                              VDSCLData++;
                              //*(VDSCLData++);
                              //*(VDSCLData++) = 0;
                              //fwrite(&Zero[0],sizeof(char),16,file_outC_blk);
                          }
                      }
                      else
                      {
                          for(h=0;h<16;h++)
                          {
                              *(VDSCLData++) = targetData[(i*8+j)*tgbufLen+k*16+h];
                              //fwrite(&targetData[(i*8+j)*tgbufLen+k*16],sizeof(char),16,file_outC_blk);
                          }
                      }
      
                      //if (pic_stru == 1) // top field
                      if (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.ucPicStruct == TOP_FIELD)
                      {
                          for(h=0;h<16;h++)
                          {
                              VDSCLData++;
                              //*(VDSCLData++);
                              //*(VDSCLData++) = 0;
                              //fwrite(&Zero[0],sizeof(char),16,file_outC_blk);
                          }
                     }
                  }
               }
            }
        }
    }

    return;
}

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8520) 
void vdscl (UINT32 u4InstID, int srcWidth, int srcHeight, int targetWidth, int targetHeight, 
        UCHAR *srcData, UCHAR *targetData, int component, int pic_stru,
        int tgbufLen, int h_ofst, int v_ofst,
        int src_agent, int spec_type, int mbaff, int fg_src)
{
    int srcbufLen;
    UCHAR *tempbuffer;
    int factor;
    int M, c, next_c, OF, next_OF;
    int weight;
    //int weight_n;
    int tg_pxl_cnt, tg_line_cnt;
    int i, j, round;
    int temp, temp_round, temp2, temp_round2;
    int hbuff, hbuff2;
    int h_result, h_result2;
    int v_result;
    int vbuff[4096];
    int v_ofst_real;
        
    srcbufLen = ((srcWidth+15)/16)*16;
    tempbuffer = (UCHAR *) _pucVDSCLWork1Sa[u4InstID];

    if ((pic_stru == 1) || (pic_stru == 2))
        v_ofst_real = v_ofst/2;
    else
        v_ofst_real = v_ofst;
        
    // Horizontal down scaling ---------------------------------------------------
    factor = (2048 * targetWidth + srcWidth/2) / srcWidth;
    M = 2048 - factor ;
    c = 0;
    next_c = 0;
    for (j=0; j<srcHeight; j++)
    {
        tg_pxl_cnt = 0;
        hbuff = 0;
        c = 0;
        next_c = 0;
        if (component == 0) // Y
        {
            for (i=1; i<srcWidth; i++)
            {
                c= (c+M)%2048;
                next_c = (c+M)%2048;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = c;
                //weight_n = next_c;
    
                if (OF == 0 && next_OF == 0)
                    temp = weight * srcData[j*srcbufLen+i] + 
                        (2048-weight) * srcData[j*srcbufLen+i-1];
                else if (OF == 0 && next_OF == 1)
                    temp = factor * srcData[j*srcbufLen+i] +
                        (2048-weight) * srcData[j*srcbufLen+i-1];
                else if (OF == 1 && next_OF == 0)
                    temp = weight * srcData[j*srcbufLen+i] + 128 * hbuff;
                else
                    temp = factor * srcData[j*srcbufLen+i] + 128 * hbuff;
    
                // 12-bit horizontal result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                // 8-bit horizontal result (after rounding)
                h_result = (temp_round + 8) / 0x10;
    
                if (tg_pxl_cnt < targetWidth)
                {
                    if (next_OF == 0)
                    {
                        tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = (UCHAR) h_result; // 8 bit
                        tg_pxl_cnt = tg_pxl_cnt + 1;
                    }
                    else
                        hbuff = temp_round; // 12 bit
                }
                else
                    break;
            }
            while (tg_pxl_cnt < targetWidth)
            {
                c= (c+M)%2048;
                next_c = (c+M)%2048;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = c;
                //weight_n = next_c;

                if (OF == 0 && next_OF == 0)
                    temp = 2048 * srcData[j*srcbufLen+srcWidth-1];
                else if (OF == 0 && next_OF == 1)
                    temp = (2048-weight+factor) * srcData[j*srcbufLen+srcWidth-1];
                else if (OF == 1 && next_OF == 0)
                    temp = weight * srcData[j*srcbufLen+srcWidth-1] + 128 * hbuff;
                else
                    temp = factor * srcData[j*srcbufLen+srcWidth-1] + 128 * hbuff;
    
                // 12-bit horizontal result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                // 8-bit horizontal result (after rounding)
                h_result = (temp_round + 8) / 0x10;
    
                if (next_OF == 0)
                {
                    tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = (UCHAR) h_result; // 8 bit
                    tg_pxl_cnt = tg_pxl_cnt + 1;
                }
                else
                    hbuff = temp_round; // 12 bit
            }
        }
        else // C
        {
            for (i=2; i<srcWidth; i=i+2)
            {
                c= (c+M)%2048;
                next_c = (c+M)%2048;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = c;
                //weight_n = next_c;
    
                if (OF == 0 && next_OF == 0)
                {
                    temp = weight * srcData[j*srcbufLen+i] + 
                        (2048-weight) * srcData[j*srcbufLen+i-2];
                    temp2 = weight * srcData[j*srcbufLen+i+1] + 
                        (2048-weight) * srcData[j*srcbufLen+i-1];
                }
                else if (OF == 0 && next_OF == 1)
                {
                    temp = factor * srcData[j*srcbufLen+i] +
                        (2048-weight) * srcData[j*srcbufLen+i-2];
                    temp2 = factor * srcData[j*srcbufLen+i+1] +
                        (2048-weight) * srcData[j*srcbufLen+i-1];
                }
                else if (OF == 1 && next_OF == 0)
                {
                    temp = weight * srcData[j*srcbufLen+i] + 128 * hbuff;
                    temp2 = weight * srcData[j*srcbufLen+i+1] + 128 * hbuff2;
                }
                else
                {
                    temp = factor * srcData[j*srcbufLen+i] + 128 * hbuff;
                    temp2 = factor * srcData[j*srcbufLen+i+1] + 128 * hbuff2;
                }
    
                // 12-bit horizontal result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                temp_round2 = (temp2 >= 0x7f800) ? 0xff0 : (temp2 / 0x80);
                // 8-bit horizontal result (after rounding)
                h_result = (temp_round + 8) / 0x10;
                h_result2 = (temp_round2 + 8) / 0x10;
    
                if (tg_pxl_cnt < targetWidth)
                {
                    if (next_OF == 0)
                    {
                        tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = (UCHAR)h_result; // 8 bit
                        tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst + 1] = (UCHAR)h_result2; // 8 bit
                        tg_pxl_cnt = tg_pxl_cnt + 2;
                    }
                    else
                    {
                        hbuff = temp_round; // 12 bit
                        hbuff2 = temp_round2; // 12 bit
                    }
                }
                else
                    break;
            }

            while (tg_pxl_cnt < targetWidth)
            {
                c= (c+M)%2048;
                next_c = (c+M)%2048;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = c;
                //weight_n = next_c;

                if (OF == 0 && next_OF == 0)
                {
                    temp = 2048 * srcData[j*srcbufLen+srcWidth-2];
                    temp2 = 2048 * srcData[j*srcbufLen+srcWidth-1];
                }
                else if (OF == 0 && next_OF == 1)
                {
                    temp = (2048-weight+factor) * srcData[j*srcbufLen+srcWidth-2];
                    temp2 = (2048-weight+factor) * srcData[j*srcbufLen+srcWidth-1];
                }
                else if (OF == 1 && next_OF == 0)
                {
                    temp = weight * srcData[j*srcbufLen+srcWidth-2] + 128 * hbuff;
                    temp2 = weight * srcData[j*srcbufLen+srcWidth-1] + 128 * hbuff2;
                }
                else
                {
                    temp = factor * srcData[j*srcbufLen+srcWidth-2] + 128 * hbuff;
                    temp2 = factor * srcData[j*srcbufLen+srcWidth-1] + 128 * hbuff2;
                }
    
                // 12-bit horizontal result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                temp_round2 = (temp2 >= 0x7f800) ? 0xff0 : (temp2 / 0x80);
                // 8-bit horizontal result (after rounding)
                h_result = (temp_round + 8) / 0x10;
                h_result2 = (temp_round2 + 8) / 0x10;

                if (next_OF == 0)
                {
                    tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = (UCHAR)h_result; // 8 bit
                    tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst + 1] = (UCHAR)h_result2; // 8 bit
                    tg_pxl_cnt = tg_pxl_cnt + 2;
                }
                else
                {
                    hbuff = temp_round; // 12 bit
                    hbuff2 = temp_round2; // 12 bit
                }
            }
        }
    }
    
    // Vertical down scaling ---------------------------------------------------
    factor = (2048 * targetHeight + srcHeight/2) / srcHeight;
    M = 2048 - factor ;
    c = 0;
    next_c = 0;
    tg_line_cnt = 0;

    if ((pic_stru != 3) && (pic_stru != 4)) // single picture
    {
        for (i=1; i<srcHeight; i++)
        {
            c= (c+M)%2048;
            next_c = (c+M)%2048;
            OF = (c<M) ? 1 : 0;
            next_OF = (next_c<M) ? 1 : 0;
            weight = c;
            //weight_n = next_c;
        
            for (j=0; j<targetWidth; j++)
            {
                if (OF == 0 && next_OF == 0)
                    temp = weight * tempbuffer[i*tgbufLen+h_ofst+j] + 
                        (2048-weight) * tempbuffer[(i-1)*tgbufLen+h_ofst+j];
                else if (OF == 0 && next_OF == 1)
                    temp = factor * tempbuffer[i*tgbufLen+h_ofst+j] +
                        (2048-weight) * tempbuffer[(i-1)*tgbufLen+h_ofst+j];
                else if (OF == 1 && next_OF == 0)
                    temp = weight * tempbuffer[i*tgbufLen+h_ofst+j] + 128 * vbuff[j];
                else
                    temp = factor * tempbuffer[i*tgbufLen+h_ofst+j] + 128 * vbuff[j];
    
                // 12-bit vertical result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                // 8-bit vertical result
                v_result = (temp_round + 8) / 0x10;
        
                if (next_OF == 0)
                    targetData[(tg_line_cnt + v_ofst_real)*tgbufLen + h_ofst + j] = (UCHAR)v_result; // 8 bit
                else
                {   
                    if (src_agent == 4) // fgt
                    {
                        if (fg_src == 1) // pp
                        {
                            if (((i%16) == 7) && (i < (srcHeight-9)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 7) && (i < (srcHeight-9)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                        else // mc
                        {
                            if (((i%16) == 15) && (i < (srcHeight-1)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 7) && (i < (srcHeight-1)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                    }
                    else if (src_agent == 2) // pp
                    {
                        if (spec_type == 1) // WMV
                        {
                            if (((i%16) == 7) && (i < (srcHeight-9)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 7) && (i < (srcHeight-9)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                        else if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                        {
                            if (((i%32) == 25) && (i < (srcHeight-7)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%16) == 13) && (i < (srcHeight-3)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                        else // 264normal
                        {
                            if (((i%16) == 11) && (i < (srcHeight-5)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 5) && (i < (srcHeight-3)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                    }
                    else // mc
                    {
                        if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                        {
                            if (((i%32) == 31) && (i < (srcHeight-1)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%16) == 15) && (i < (srcHeight-1)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                        else
                        {
                            if (((i%16) == 15) && (i < (srcHeight-1)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 7) && (i < (srcHeight-1)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                    }
                }
            }
            tg_line_cnt = (next_OF == 0) ? (tg_line_cnt+1) : tg_line_cnt;
            if (tg_line_cnt >= targetHeight)
                break;
        }
        
        while (tg_line_cnt < targetHeight)
        {
            c= (c+M)%2048;
            next_c = (c+M)%2048;
            OF = (c<M) ? 1 : 0;
            next_OF = (next_c<M) ? 1 : 0;
            weight = c;
            //weight_n = next_c;
        
            for (j=0; j<targetWidth; j++)
            {
                if (OF == 0 && next_OF == 0)
                    temp = weight * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j] + 
                        (2048-weight) * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j];
                else if (OF == 0 && next_OF == 1)
                    temp = factor * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j] +
                        (2048-weight) * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j];
                else if (OF == 1 && next_OF == 0)
                    temp = weight * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j] + 128 * vbuff[j];
                else
                    temp = factor * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j] + 128 * vbuff[j];
    
                // 12-bit vertical result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                // 8-bit vertical result
                v_result = (temp_round + 8) / 0x10;
        
                if (next_OF == 0)
                    targetData[(tg_line_cnt+v_ofst_real)*tgbufLen + h_ofst + j] = (UCHAR)v_result; // 8 bit
                else
                    vbuff[j] = temp_round; // 12 bit
            }
            tg_line_cnt = (next_OF == 0) ? (tg_line_cnt+1) : tg_line_cnt;
        }
    }
    else // interlaced frame
    {
        for (round=2; round<=3; round++)
        {
            c = 0;
            next_c = 0;
            tg_line_cnt = round-2;

            for (i=round; i<srcHeight; i=i+2)
            {
                c= (c+M)%2048;
                next_c = (c+M)%2048;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = c;
                //weight_n = next_c;
            
                for (j=0; j<targetWidth; j++)
                {
                    if (OF == 0 && next_OF == 0)
                        temp = weight * tempbuffer[i*tgbufLen+h_ofst+j] + 
                            (2048-weight) * tempbuffer[(i-2)*tgbufLen+h_ofst+j];
                    else if (OF == 0 && next_OF == 1)
                        temp = factor * tempbuffer[i*tgbufLen+h_ofst+j] +
                            (2048-weight) * tempbuffer[(i-2)*tgbufLen+h_ofst+j];
                    else if (OF == 1 && next_OF == 0)
                        temp = weight * tempbuffer[i*tgbufLen+h_ofst+j] + 128 * vbuff[j];
                    else
                        temp = factor * tempbuffer[i*tgbufLen+h_ofst+j] + 128 * vbuff[j];
        
                    // 12-bit vertical result
                    temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                    // 8-bit vertical result
                    v_result = (temp_round + 8) / 0x10;
            
                    if (next_OF == 0)
                        targetData[(tg_line_cnt+v_ofst_real)*tgbufLen + h_ofst + j] = (UCHAR)v_result; // 8 bit
                    else if (pic_stru == 3)
                    {
                        if (src_agent == 4) // fgt
                        {
                            if (fg_src == 1) // pp
                            {
                                if ((((i%16) == 7) || ((i%16) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 7) || ((i%8) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else // mc
                            {
                                if ((((i%16) == 15) || ((i%16) == 14)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 7) || ((i%8) == 6)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            
                        }
                        else if (src_agent == 2) // pp
                        {
                            if (spec_type == 1) // WMV
                            {
                                if ((((i%16) == 7) || ((i%16) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 7) || ((i%8) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                            {
                                if ((((i%32) == 25) || ((i%32) == 24)) &&
                                        (i < (srcHeight-8)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 13) || ((i%16) == 12)) &&
                                        (i < (srcHeight-4)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else // 264normal
                            {
                                if ((((i%16) == 11) || ((i%16) == 10)) &&
                                        (i < (srcHeight-6)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 5) || ((i%8) == 4)) &&
                                        (i < (srcHeight-4)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                        }
                        else // mc
                        {
                            if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                            {
                                if ((((i%32) == 31) || ((i%32) == 30)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 15) || ((i%16) == 14)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else
                            {
                                if ((((i%16) == 15) || ((i%16) == 14)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 7) || ((i%8) == 6)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                        }
                    }
                    else // both top & bottom fields, modified by cyc118 at 01202006
                    {
                        if (src_agent == 4) // fgt
                        {
                            if (fg_src == 1) // pp
                            {
                                if ((((i%32) == 15) || ((i%32) == 14)) &&
                                        (i < (srcHeight-18)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 15) || ((i%16) == 14)) &&
                                        (i < (srcHeight-18)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else // mc
                            {
                                if ((((i%32) == 31) || ((i%32) == 30)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 15) || ((i%16) == 14)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                     vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                     vbuff[j] = temp_round; // 12 bit
                            }
                            
                        }
                        else if (src_agent == 2) // pp
                        {
                            if (spec_type == 1) // WMV
                            {
                                if ((((i%32) == 15) || ((i%32) == 14)) &&
                                        (i < (srcHeight-18)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 15) || ((i%16) == 14)) &&
                                        (i < (srcHeight-18)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                            {
                                if ((((i%64) == 51) || ((i%64) == 50)) &&
                                        (i < (srcHeight-14)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%32) == 27) || ((i%32) == 26)) &&
                                        (i < (srcHeight-6)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else // 264normal
                            {
                                if ((((i%32) == 23) || ((i%32) == 22)) &&
                                        (i < (srcHeight-10)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 11) || ((i%16) == 10)) &&
                                        (i < (srcHeight-6)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                        }
                        else // mc
                        {
                            if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                            {
                                if ((((i%64) == 63) || ((i%64) == 62)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%32) == 31) || ((i%32) == 30)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                     vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                     vbuff[j] = temp_round; // 12 bit
                            }
                            else
                            {
                                if ((((i%32) == 31) || ((i%32) == 30)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 15) || ((i%16) == 14)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                     vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                     vbuff[j] = temp_round; // 12 bit
                            }
                        }
                    }
                }
                tg_line_cnt = (next_OF == 0) ? (tg_line_cnt+2) : tg_line_cnt;
                if (tg_line_cnt >= targetHeight)
                    break;
            }
            
            while (tg_line_cnt < targetHeight)
            {
                c= (c+M)%2048;
                next_c = (c+M)%2048;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = c;
                //weight_n = next_c;
            
                for (j=0; j<targetWidth; j++)
                {
                    if (OF == 0 && next_OF == 0)
                        temp = weight * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j] + 
                            (2048-weight) * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j];
                    else if (OF == 0 && next_OF == 1)
                        temp = factor * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j] +
                            (2048-weight) * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j];
                    else if (OF == 1 && next_OF == 0)
                        temp = weight * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j] + 128 * vbuff[j];
                    else
                        temp = factor * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j] + 128 * vbuff[j];
        
                    // 12-bit vertical result
                    temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                    // 8-bit vertical result
                    v_result = (temp_round + 8) / 0x10;
            
                    if (next_OF == 0)
                        targetData[(tg_line_cnt+v_ofst_real)*tgbufLen + h_ofst + j] = (UCHAR)v_result; // 8 bit
                    else
                        vbuff[j] = temp_round; // 12 bit
                }
                tg_line_cnt = (next_OF == 0) ? (tg_line_cnt+2) : tg_line_cnt;
            }
        }
    }
}
#else

int vbuff[4096];

void vdscl (UINT32 u4InstID, int srcWidth, int srcHeight, int targetWidth, int targetHeight, 
        UCHAR *srcData, UCHAR *targetData, int component, int pic_stru,
        int tgbufLen, int h_ofst, int v_ofst,
        int src_agent, int spec_type, int mbaff, int fg_src,
        int src_h_ofst, int src_v_ofst, int scl_h_ofst, int scl_v_ofst,
        int lk_en, int luma_key)
// component: 0 => Y, 1 => C
{
    int srcbufLen;
    UCHAR *tempbuffer;
    int factor, factor_eval;
    int M, c, next_c, OF, next_OF;
    int weight; //, weight_n;
    int tg_pxl_cnt, tg_line_cnt;
    int i, j, round;
    int temp, temp_round, temp2, temp_round2;
    int hbuff = 0;
    int hbuff2 = 0;
    int h_result, h_result2;
    int v_result;
    int v_ofst_real;
        
#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8550)
    int tmp_src_v_ofst = 0;
#endif
    int v_region_0 = 0;
    int v_region_1 = 0;
    int rgn_change = 0;
        
    srcbufLen = ((srcWidth+15)/16)*16;
    tempbuffer = (UCHAR*)_pucVDSCLWork1Sa[u4InstID];

    if ((pic_stru == 1) || (pic_stru == 2))
        v_ofst_real = v_ofst/2;
    else
        v_ofst_real = v_ofst;
        
    // Horizontal down scaling ---------------------------------------------------
    factor = (0x8000 * targetWidth + srcWidth/2) / srcWidth;
    M = 0x8000 - factor ;
    c = 0;
    next_c = 0;
	factor_eval = (factor >> 4);
    for (j=0; j<srcHeight; j++)
    {
        tg_pxl_cnt = 0;
        hbuff = 0;
        //c = 0;
        // @06102008
		c = scl_h_ofst;
		next_c = 0;
        
        rgn_change = 0;        
        if (component == 0) // Y
        {
            //for (i=1; i<srcWidth; i++)
			// @06102008
			for (i=(1+src_h_ofst); i<srcWidth; i++)
            {
                c= (c+M)%0x8000;
                next_c = (c+M)%0x8000;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = (c >> 4);
                //weight_n = (next_c >> 4);
    
                if (OF == 0 && next_OF == 0)
                    temp = weight * srcData[j*srcbufLen+i] + 
                        (2048-weight) * srcData[j*srcbufLen+i-1];
                else if (OF == 0 && next_OF == 1)
                    temp = factor_eval * srcData[j*srcbufLen+i] +
                        (2048-weight) * srcData[j*srcbufLen+i-1];
                else if (OF == 1 && next_OF == 0)
                    temp = weight * srcData[j*srcbufLen+i] + 128 * hbuff;
                else
                    temp = factor_eval * srcData[j*srcbufLen+i] + 128 * hbuff;
    
                v_region_0 = srcData[j*srcbufLen+i-1] > luma_key;
                v_region_1 = srcData[j*srcbufLen+i]   > luma_key;
                if (((i%16) == 0) && (OF == 1))
                    rgn_change = 1;
                else if (((i%16) == 0) && (v_region_0 == v_region_1))
                    rgn_change = 0;
                else if (v_region_0 != v_region_1)
                    rgn_change = 1;
    
                // 12-bit horizontal result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                // 8-bit horizontal result (after rounding)
                h_result = (temp_round + 8) / 0x10;
    
                if (tg_pxl_cnt < targetWidth)
                {
                    if (next_OF == 0)
                    {
                        if (lk_en && (OF == 1) && (rgn_change == 1))
                        {
                            tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = srcData[j*srcbufLen+i];
                            rgn_change = 0;
                         }
                        else if (lk_en && (rgn_change == 1))
                        {
                            tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = srcData[j*srcbufLen+i-1];
                            rgn_change = 0;
                        }
                        else
                        tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = (UINT8) h_result; // 8 bit
                        
                        tg_pxl_cnt = tg_pxl_cnt + 1;
                    }
                    else
                        hbuff = temp_round; // 12 bit
                }
                else
                    break;

                // debug
                /*
                if ((tg_pxl_cnt == targetWidth) && (next_OF == 0))
                {
                    printf ("h_result(%d, %d) = %3x\n", i, j, h_result);
                }*/ 
                /*
                if ((j == 15) && (tg_pxl_cnt >= (targetWidth - 4)))
                {
                    printf ("h_result(%d, %d) = %3x, srcData = %3x, hbuff = %3x\n", 
                            i, j, h_result, srcData[j*srcbufLen+srcWidth-1], hbuff);
                }
                */
                /*
                if ((i>333) && (i<338) && (j>61) && (j<65))
                {
                    printf ("hbuff(%d, %d) = %3x, hcoeff = %3x, \n", i, j, hbuff, c);
                }
                */
                // debug
            }
            while (tg_pxl_cnt < targetWidth)
            {
                c= (c+M)%0x8000;
                next_c = (c+M)%0x8000;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = (c >> 4);
                //weight_n = (next_c >> 4);
				
                if (OF == 0 && next_OF == 0)
                    temp = 2048 * srcData[j*srcbufLen+srcWidth-1];
                else if (OF == 0 && next_OF == 1)
                    temp = (2048-weight+factor_eval) * srcData[j*srcbufLen+srcWidth-1];
                else if (OF == 1 && next_OF == 0)
                    temp = weight * srcData[j*srcbufLen+srcWidth-1] + 128 * hbuff;
                else
                    temp = factor_eval * srcData[j*srcbufLen+srcWidth-1] + 128 * hbuff;
    
                // 12-bit horizontal result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                // 8-bit horizontal result (after rounding)
                h_result = (temp_round + 8) / 0x10;
    
                if (next_OF == 0)
                {
                    if (lk_en && (OF == 1) && (rgn_change == 1))
                    {                           
                        tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = srcData[j*srcbufLen+srcWidth-1];
                        rgn_change = 0;
                    }
                    else if (lk_en && (rgn_change == 1))
                    {
                        tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = srcData[j*srcbufLen+srcWidth-1];
                        rgn_change = 0;
                    }                    
                    else
                    tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = (UINT8) h_result; // 8 bit
                    tg_pxl_cnt = tg_pxl_cnt + 1;
                }
                else
                    hbuff = temp_round; // 12 bit
                // debug
                /*
                if ((j == 15) && (tg_pxl_cnt == targetWidth) && (next_OF == 0))
                {
                    printf ("h_result(%d, %d) = %3x, srcData = %3x, hbuff = %3x\n", 
                            i, j, h_result, srcData[j*srcbufLen+srcWidth-1], hbuff);
                }
                */
                // debug
            }
        }
        else // C
        {
            for (i=2*(1+src_h_ofst); i<srcWidth; i=i+2)
            {
                c= (c+M)%0x8000;
                next_c = (c+M)%0x8000;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = (c >> 4);
                //weight_n = (next_c >> 4);
    
                if (OF == 0 && next_OF == 0)
                {
                    temp = weight * srcData[j*srcbufLen+i] + 
                        (2048-weight) * srcData[j*srcbufLen+i-2];
                    temp2 = weight * srcData[j*srcbufLen+i+1] + 
                        (2048-weight) * srcData[j*srcbufLen+i-1];
                }
                else if (OF == 0 && next_OF == 1)
                {
                    temp = factor_eval * srcData[j*srcbufLen+i] +
                        (2048-weight) * srcData[j*srcbufLen+i-2];
                    temp2 = factor_eval * srcData[j*srcbufLen+i+1] +
                        (2048-weight) * srcData[j*srcbufLen+i-1];
                }
                else if (OF == 1 && next_OF == 0)
                {
                    temp = weight * srcData[j*srcbufLen+i] + 128 * hbuff;
                    temp2 = weight * srcData[j*srcbufLen+i+1] + 128 * hbuff2;
                }
                else
                {
                    temp = factor_eval * srcData[j*srcbufLen+i] + 128 * hbuff;
                    temp2 = factor_eval * srcData[j*srcbufLen+i+1] + 128 * hbuff2;
                }
    
                // 12-bit horizontal result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                temp_round2 = (temp2 >= 0x7f800) ? 0xff0 : (temp2 / 0x80);
                // 8-bit horizontal result (after rounding)
                h_result = (temp_round + 8) / 0x10;
                h_result2 = (temp_round2 + 8) / 0x10;
    
                if (tg_pxl_cnt < targetWidth)
                {
                    if (next_OF == 0)
                    {
                        tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = (UINT8)h_result; // 8 bit
                        tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst + 1] = (UINT8)h_result2; // 8 bit
                        tg_pxl_cnt = tg_pxl_cnt + 2;
                    }
                    else
                    {
                        hbuff = temp_round; // 12 bit
                        hbuff2 = temp_round2; // 12 bit
                    }
                }
                else
                    break;
            }

            while (tg_pxl_cnt < targetWidth)
            {
                c= (c+M)%0x8000;
                next_c = (c+M)%0x8000;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = (c >> 4);
                //weight_n = (next_c >> 4);

                if (OF == 0 && next_OF == 0)
                {
                    temp = 2048 * srcData[j*srcbufLen+srcWidth-2];
                    temp2 = 2048 * srcData[j*srcbufLen+srcWidth-1];
                }
                else if (OF == 0 && next_OF == 1)
                {
                    temp = (2048-weight+factor_eval) * srcData[j*srcbufLen+srcWidth-2];
                    temp2 = (2048-weight+factor_eval) * srcData[j*srcbufLen+srcWidth-1];
                }
                else if (OF == 1 && next_OF == 0)
                {
                    temp = weight * srcData[j*srcbufLen+srcWidth-2] + 128 * hbuff;
                    temp2 = weight * srcData[j*srcbufLen+srcWidth-1] + 128 * hbuff2;
                }
                else
                {
                    temp = factor_eval * srcData[j*srcbufLen+srcWidth-2] + 128 * hbuff;
                    temp2 = factor_eval * srcData[j*srcbufLen+srcWidth-1] + 128 * hbuff2;
                }
    
                // 12-bit horizontal result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                temp_round2 = (temp2 >= 0x7f800) ? 0xff0 : (temp2 / 0x80);
                // 8-bit horizontal result (after rounding)
                h_result = (temp_round + 8) / 0x10;
                h_result2 = (temp_round2 + 8) / 0x10;

                if (next_OF == 0)
                {
                    tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst] = (UINT8)h_result; // 8 bit
                    tempbuffer[j*tgbufLen + tg_pxl_cnt + h_ofst + 1] = (UINT8)h_result2; // 8 bit
                    tg_pxl_cnt = tg_pxl_cnt + 2;
                }
                else
                {
                    hbuff = temp_round; // 12 bit
                    hbuff2 = temp_round2; // 12 bit
                }
            }
        }
    }
    
    // Vertical down scaling ---------------------------------------------------
    factor = (0x8000 * targetHeight + srcHeight/2) / srcHeight;
    M = 0x8000 - factor ;
    //c = 0;
    // @06102008
	c = scl_v_ofst;
	next_c = 0;
    tg_line_cnt = 0;
	factor_eval = (factor >> 4);

    if ((pic_stru != 3) && (pic_stru != 4)) // single picture
    {
        //for (i=1; i<srcHeight; i++)
		for (i=(1+src_v_ofst); i<srcHeight; i++)
        {
            c= (c+M)%0x8000;
            next_c = (c+M)%0x8000;
            OF = (c<M) ? 1 : 0;
            next_OF = (next_c<M) ? 1 : 0;
            weight = (c >> 4);
            //weight_n = (next_c >> 4);
			        
            for (j=0; j<targetWidth; j++)
            {
                if (OF == 0 && next_OF == 0)
                    temp = weight * tempbuffer[i*tgbufLen+h_ofst+j] + 
                        (2048-weight) * tempbuffer[(i-1)*tgbufLen+h_ofst+j];
                else if (OF == 0 && next_OF == 1)
                    temp = factor_eval * tempbuffer[i*tgbufLen+h_ofst+j] +
                        (2048-weight) * tempbuffer[(i-1)*tgbufLen+h_ofst+j];
                else if (OF == 1 && next_OF == 0)
                    temp = weight * tempbuffer[i*tgbufLen+h_ofst+j] + 128 * vbuff[j];
                else
                    temp = factor_eval * tempbuffer[i*tgbufLen+h_ofst+j] + 128 * vbuff[j];
    
                // 12-bit vertical result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                // 8-bit vertical result
                v_result = (temp_round + 8) / 0x10;
        
                if (next_OF == 0)
                    targetData[(tg_line_cnt + v_ofst_real)*tgbufLen + h_ofst + j] = (UINT8)v_result; // 8 bit
                else
                {   
                    if (src_agent == 4) // fgt
                    {
                        if (fg_src == 1) // pp
                        {
                            if (((i%16) == 7) && (i < (srcHeight-9)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 7) && (i < (srcHeight-9)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                        else // mc
                        {
                            if (((i%16) == 15) && (i < (srcHeight-1)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 7) && (i < (srcHeight-1)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                    }
                    else if (src_agent == 2) // pp
                    {
                        if (spec_type == 1) // WMV
                        {
                            if (((i%16) == 7) && (i < (srcHeight-9)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 7) && (i < (srcHeight-9)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                        else if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                        {
#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)                        
                            if (((i%32) == 25) && (i < (srcHeight-7)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%16) == 13) && (i < (srcHeight-3)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
#else
                            if (((i%32) == 23) && (i < (srcHeight-9)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%16) == 11) && (i < (srcHeight-5)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
#endif
                        }
                        else if (spec_type == 2)// 264normal
                        {
                            if (((i%16) == 11) && (i < (srcHeight-5)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 5) && (i < (srcHeight-3)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                        else // mp2, mp4
                        {
#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)
                            if (((i%16) == 11) && (i < (srcHeight-5)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 3) && (i < (srcHeight-5)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit                        	
#else
                            if (((i%16) == 7) && (i < (srcHeight-9)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 3) && (i < (srcHeight-5)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit                            
#endif                                
                        }
                    }
                    else // mc
                    {
                        if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                        {
                            if (((i%32) == 31) && (i < (srcHeight-1)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%16) == 15) && (i < (srcHeight-1)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                        else
                        {
                            if (((i%16) == 15) && (i < (srcHeight-1)) && (component == 0)) // Y
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else if (((i%8) == 7) && (i < (srcHeight-1)) && (component == 1)) // C
                                vbuff[j] = v_result * 0x10; // 12 bit
                            else
                                vbuff[j] = temp_round; // 12 bit
                        }
                    }
                }
    
                // debug
                /*
                if ((j>187) && (j<193) && (i>61) && (i<65))
                    printf ("vbuff(%d, %d) = %3x\n", j, i, vbuff[j]);
                */
                // debug
            }
            tg_line_cnt = (next_OF == 0) ? (tg_line_cnt+1) : tg_line_cnt;
            if (tg_line_cnt >= targetHeight)
                break;
        }
        
        while (tg_line_cnt < targetHeight)
        {
            c= (c+M)%0x8000;
            next_c = (c+M)%0x8000;
            OF = (c<M) ? 1 : 0;
            next_OF = (next_c<M) ? 1 : 0;
            weight = (c >> 4);
            //weight_n = (next_c >> 4);
			        
            for (j=0; j<targetWidth; j++)
            {
                if (OF == 0 && next_OF == 0)
                    temp = weight * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j] + 
                        (2048-weight) * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j];
                else if (OF == 0 && next_OF == 1)
                    temp = factor_eval * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j] +
                        (2048-weight) * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j];
                else if (OF == 1 && next_OF == 0)
                    temp = weight * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j] + 128 * vbuff[j];
                else
                    temp = factor_eval * tempbuffer[(srcHeight-1)*tgbufLen+h_ofst+j] + 128 * vbuff[j];
    
                // 12-bit vertical result
                temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                // 8-bit vertical result
                v_result = (temp_round + 8) / 0x10;
        
                if (next_OF == 0)
                    targetData[(tg_line_cnt+v_ofst_real)*tgbufLen + h_ofst + j] = (UINT8)v_result; // 8 bit
                else
                    vbuff[j] = temp_round; // 12 bit
            }
            tg_line_cnt = (next_OF == 0) ? (tg_line_cnt+1) : tg_line_cnt;
        }
    }
    else // interlaced frame
    {
#if (CONFIG_CHIP_VER_CURR < CONFIG_CHIP_VER_MT8550)    
              //Only used under 8550
		if (pic_stru == 4) // both fields
#endif		
			src_v_ofst = src_v_ofst * 2;

        for (round=2; round<=3; round++)
        {
            // c = 0;
#if (CONFIG_CHIP_VER_CURR < CONFIG_CHIP_VER_MT8550)            
			// @07022008
			c = scl_v_ofst;
#else
                     // @02262009
                     if ((round == 2) && (pic_stru == 3)) // top field of two field mode
                     {
                         tmp_src_v_ofst = src_v_ofst;
                         src_v_ofst = 0;
                     }
                     else if ((round == 3) && (pic_stru == 3)) // bot field of two field mode
                     {
                         src_v_ofst = tmp_src_v_ofst;
                     }

                     if ((round == 2) && (pic_stru == 3)) // top field of two field mode
                         c = 0;
                     else
                         c = scl_v_ofst;
#endif
            next_c = 0;
            tg_line_cnt = round-2;
			
            for (i=(round+src_v_ofst); i<srcHeight; i=i+2)
            {
                c= (c+M)%0x8000;
                next_c = (c+M)%0x8000;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = (c >> 4);
                //weight_n = (next_c >> 4);
                
                for (j=0; j<targetWidth; j++)
                {
                    if (OF == 0 && next_OF == 0)
                        temp = weight * tempbuffer[i*tgbufLen+h_ofst+j] + 
                            (2048-weight) * tempbuffer[(i-2)*tgbufLen+h_ofst+j];
                    else if (OF == 0 && next_OF == 1)
                        temp = factor_eval * tempbuffer[i*tgbufLen+h_ofst+j] +
                            (2048-weight) * tempbuffer[(i-2)*tgbufLen+h_ofst+j];
                    else if (OF == 1 && next_OF == 0)
                        temp = weight * tempbuffer[i*tgbufLen+h_ofst+j] + 128 * vbuff[j];
                    else
                        temp = factor_eval * tempbuffer[i*tgbufLen+h_ofst+j] + 128 * vbuff[j];
        
                    // 12-bit vertical result
                    temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                    // 8-bit vertical result
                    v_result = (temp_round + 8) / 0x10;
            
                    if (next_OF == 0)
                        targetData[(tg_line_cnt+v_ofst_real)*tgbufLen + h_ofst + j] = (UINT8)v_result; // 8 bit
                    else if (pic_stru == 3)
                    {
                        if (src_agent == 4) // fgt
                        {
                            if (fg_src == 1) // pp
                            {
                                if ((((i%16) == 7) || ((i%16) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 7) || ((i%8) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else // mc
                            {
                                if ((((i%16) == 15) || ((i%16) == 14)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 7) || ((i%8) == 6)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            
                        }
                        else if (src_agent == 2) // pp
                        {
                            if (spec_type == 1) // WMV
                            {
                                if ((((i%16) == 7) || ((i%16) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 7) || ((i%8) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                            {
#if (CONFIG_CHIP_VER_CURR < CONFIG_CHIP_VER_MT8550)                            
                                if ((((i%32) == 25) || ((i%32) == 24)) &&
                                        (i < (srcHeight-8)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 13) || ((i%16) == 12)) &&
                                        (i < (srcHeight-4)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
#else
                                if ((((i%32) == 23) || ((i%32) == 22)) &&
                                        (i < (srcHeight-10)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 11) || ((i%16) == 10)) &&
                                        (i < (srcHeight-6)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
#endif
                            }
                            else if (spec_type == 2)// 264normal
                            {
                                if ((((i%16) == 11) || ((i%16) == 10)) &&
                                        (i < (srcHeight-6)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 5) || ((i%8) == 4)) &&
                                        (i < (srcHeight-4)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else // mp2, mp4
                            {
#if (CONFIG_CHIP_VER_CURR < CONFIG_CHIP_VER_MT8550)
                                if ((((i%16) == 11) || ((i%16) == 10)) &&
                                        (i < (srcHeight-6)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 3) || ((i%8) == 2)) &&
                                        (i < (srcHeight-6)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit                            	
#else
                                if ((((i%16) == 7) || ((i%16) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 3) || ((i%8) == 2)) &&
                                        (i < (srcHeight-6)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit                                
#endif                                    
                            }
                        }
                        else // mc
                        {
                            if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                            {
                                if ((((i%32) == 31) || ((i%32) == 30)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 15) || ((i%16) == 14)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else
                            {
                                if ((((i%16) == 15) || ((i%16) == 14)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%8) == 7) || ((i%8) == 6)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                        }
                    }
                    else // both top & bottom fields, modified by cyc118 at 01202006
                    {
                        if (src_agent == 4) // fgt
                        {
                            if (fg_src == 1) // pp
                            {
                                if ((((i%32) == 15) || ((i%32) == 14)) &&
                                        (i < (srcHeight-18)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 15) || ((i%16) == 14)) &&
                                        (i < (srcHeight-18)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else // mc
                            {
                                if ((((i%32) == 31) || ((i%32) == 30)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 15) || ((i%16) == 14)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                     vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                     vbuff[j] = temp_round; // 12 bit
                            }
                            
                        }
                        else if (src_agent == 2) // pp
                        {
                            if (spec_type == 1) // WMV
                            {
                                if ((((i%32) == 15) || ((i%32) == 14)) &&
                                        (i < (srcHeight-18)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 15) || ((i%16) == 14)) &&
                                        (i < (srcHeight-18)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                            {
#if (CONFIG_CHIP_VER_CURR < CONFIG_CHIP_VER_MT8550)                            
                                if ((((i%64) == 51) || ((i%64) == 50)) &&
                                        (i < (srcHeight-14)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%32) == 27) || ((i%32) == 26)) &&
                                        (i < (srcHeight-6)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
#else
                                if ((((i%64) == 47) || ((i%64) == 46)) &&
                                        (i < (srcHeight-18)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%32) == 23) || ((i%32) == 22)) &&
                                        (i < (srcHeight-10)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
#endif
                            }
                            else if (spec_type == 2)// 264normal
                            {
                                if ((((i%32) == 23) || ((i%32) == 22)) &&
                                        (i < (srcHeight-10)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 11) || ((i%16) == 10)) &&
                                        (i < (srcHeight-6)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
                            }
                            else // mp2, mp4
                            {
#if (CONFIG_CHIP_VER_CURR < CONFIG_CHIP_VER_MT8550)                            
                                if ((((i%32) == 23) || ((i%32) == 22)) &&
                                        (i < (srcHeight-10)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 7) || ((i%16) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit
#else
                                if ((((i%32) == 15) || ((i%32) == 14)) &&
                                        (i < (srcHeight-18)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 7) || ((i%16) == 6)) &&
                                        (i < (srcHeight-10)) && (component == 1)) // C
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                    vbuff[j] = temp_round; // 12 bit 
#endif                                                                	
                            }
                        }
                        else // mc
                        {
                            if ((spec_type == 2) && (mbaff == 1)) // 264mbaff
                            {
                                if ((((i%64) == 63) || ((i%64) == 62)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%32) == 31) || ((i%32) == 30)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                     vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                     vbuff[j] = temp_round; // 12 bit
                            }
                            else
                            {
                                if ((((i%32) == 31) || ((i%32) == 30)) && 
                                        (i < (srcHeight-2)) && (component == 0)) // Y
                                    vbuff[j] = v_result * 0x10; // 12 bit
                                else if ((((i%16) == 15) || ((i%16) == 14)) && 
                                        (i < (srcHeight-2)) && (component == 1))
                                     vbuff[j] = v_result * 0x10; // 12 bit
                                else
                                     vbuff[j] = temp_round; // 12 bit
                            }
                        }
                    }
        
                    // debug
                    /*
                    if ((j>187) && (j<193) && (i>61) && (i<65))
                        printf ("vbuff(%d, %d) = %3x\n", j, i, vbuff[j]);
                    */
                    // debug
                }
                tg_line_cnt = (next_OF == 0) ? (tg_line_cnt+2) : tg_line_cnt;
                if (tg_line_cnt >= targetHeight)
                    break;
            }
            
            while (tg_line_cnt < targetHeight)
            {
                c= (c+M)%0x8000;
                next_c = (c+M)%0x8000;
                OF = (c<M) ? 1 : 0;
                next_OF = (next_c<M) ? 1 : 0;
                weight = (c >> 4);
                //weight_n = (next_c >> 4);
                
                for (j=0; j<targetWidth; j++)
                {
                    if (OF == 0 && next_OF == 0)
                        temp = weight * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j] + 
                            (2048-weight) * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j];
                    else if (OF == 0 && next_OF == 1)
                        temp = factor_eval * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j] +
                            (2048-weight) * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j];
                    else if (OF == 1 && next_OF == 0)
                        temp = weight * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j] + 128 * vbuff[j];
                    else
                        temp = factor_eval * tempbuffer[(srcHeight+round-4)*tgbufLen+h_ofst+j] + 128 * vbuff[j];
        
                    // 12-bit vertical result
                    temp_round = (temp >= 0x7f800) ? 0xff0 : (temp / 0x80);
                    // 8-bit vertical result
                    v_result = (temp_round + 8) / 0x10;
            
                    if (next_OF == 0)
                        targetData[(tg_line_cnt+v_ofst_real)*tgbufLen + h_ofst + j] = (UINT8)v_result; // 8 bit
                    else
                        vbuff[j] = temp_round; // 12 bit
                }
                tg_line_cnt = (next_OF == 0) ? (tg_line_cnt+2) : tg_line_cnt;
            }
        }
    }
}
#endif

// *********************************************************************
// Function    : BOOL fgCompH264ChkSumGolden(UINT32 u4InstID)
// Description : write check sum in rec file
// Parameter   : None
// Return      : TRUE pass, FALSE err
// *********************************************************************
BOOL fgCompDvChkSumGolden(UINT32 u4InstID)
{
    BOOL fgIsCompNoErr,fgOpen;
    UINT32 *pu4GoldenCheckSum;
    UINT32 u4Temp;

    u4Temp = sprintf(_bTempStr1[u4InstID], _bFileStr1[u4InstID][0]);
    _tTempFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tTempFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tTempFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tTempFileInfo[u4InstID].u4FileLength = 0; 

    u4Temp += sprintf(_bTempStr1[u4InstID] + u4Temp, "%d_chksum.bin", _u4FileCnt[u4InstID]);
    fgOpen = fgOpenPCFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
    pu4GoldenCheckSum = (UINT32 *)_pucDumpYBuf[u4InstID];
    if(fgOpen)
    {
        if((!fgIsFrmPic(u4InstID)) && (_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)) //Only 2nd field pic needs offset 256
        {
            pu4GoldenCheckSum += MAX_CHKSUM_NUM;
        }
        else
        {
            pu4GoldenCheckSum += 0;
        }   
        //????????????
        //fgIsCompNoErr = fgVDEC_HAL_DV_VDec_CompCheckSum(&_u4DumpChksum[u4InstID][0], pu4GoldenCheckSum);      
    }
    else
    {
        fgIsCompNoErr = FALSE;
    }

    return fgIsCompNoErr;
}


// *********************************************************************
// Function    : void vDvWrData2PC(UINT32 u4InstID, BYTE *ptAddr, UINT32 u4Size)
// Description : Write the decoded data to PC for compare
// Parameter   : None
// Return      : None
// *********************************************************************
void vDvWrData2PC(UINT32 u4InstID, UCHAR *ptAddr)
{
#if ((!defined(COMP_HW_CHKSUM)) || defined(DOWN_SCALE_SUPPORT) || defined(FGT_SUPPORT))
  UINT32 u4Cnt;
  UINT32 u4Upd;
  UINT32 u4Pic32XSize;
  UCHAR *pbDecBuf;
  INT32 i;
#endif
  BOOL fgDecErr,fgOpen;
  char strMessage[256];
  UINT32 u4Size;

  _ptCurrFBufInfo[u4InstID] = &_ptFBufInfo[u4InstID][0];
  _ptCurrFBufInfo[u4InstID]->ucFBufStatus = FRAME;
  _ptCurrFBufInfo[u4InstID]->u4W = _tVerPic[u4InstID].u4W;
  _ptCurrFBufInfo[u4InstID]->u4H = _tVerPic[u4InstID].u4H;
  u4Size = _tVerPic[u4InstID].u4W * _tVerPic[u4InstID].u4H;

  //_tFileListRecInfo.fpFile = fopen(_FileList_Rec,"a+t");
  strncpy(_tFileListRecInfo[u4InstID].bFileName, _FileList_Rec[u4InstID], sizeof(_FileList_Rec[u4InstID]));
  vClrDecFlag(u4InstID, DEC_FLAG_CHG_FBUF);
  if((_ptCurrFBufInfo[u4InstID]->ucFBufStatus != FRAME)
       && (_ptCurrFBufInfo[u4InstID]->ucFBufStatus == _ucPrevDecFld[u4InstID]))
  {
#ifdef REDEC    
    if(_u4ReDecCnt[u4InstID] == 0)
#endif    
    {
      _u4FileCnt[u4InstID] ++;
      vSetDecFlag(u4InstID, DEC_FLAG_CHG_FBUF);
    }
  }


  fgDecErr = FALSE;

#ifdef GEN_HW_CHKSUM
  vWrite2PC(u4InstID, 9, NULL);
#endif

#if (defined(COMP_HW_CHKSUM) && (!defined(DOWN_SCALE_SUPPORT)) && (!defined(FGT_SUPPORT)))
	/*
    if(!fgCompDvChkSumGolden(u4InstID))
    {
      fgDecErr = TRUE;
      vVDecOutputDebugString("Check sum comparison mismatch\n");
    }
    */
#else // compare pixel by pixel
  // Y compare
  _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
  _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
  _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
  _tFBufFileInfo[u4InstID].u4FileLength = 0;  
  // Y decoded data Compare
  vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], _u4FileCnt[u4InstID]);
  #ifdef EXT_COMPARE 
  _tFBufFileInfo[u4InstID].u4FileLength = (((_ptCurrFBufInfo[u4InstID]->u4W + 15)>>4)<<4) *(((_ptCurrFBufInfo[u4InstID]->u4H + 31)>>5)<<5);
  #else
#ifdef DIRECT_DEC
  if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
#endif    
  {
  #ifdef  SATA_HDD_READ_SUPPORT
    fgOpenHDDFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);
  #else
    fgOpenPCFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]); 
  #endif
    _u4GoldenYSize[u4InstID] = _tFBufFileInfo[u4InstID].u4RealGetBytes;
  }
  #endif

#ifdef DOWN_SCALE_SUPPORT
  u4Upd = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight % 32;
  u4Pic32XSize = _tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgWidth * (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight - u4Upd);
#else
  u4Upd = _ptCurrFBufInfo[u4InstID]->u4H % 32; // 16 or 0
  u4Pic32XSize = _ptCurrFBufInfo[u4InstID]->u4W * (_ptCurrFBufInfo[u4InstID]->u4H - u4Upd);
#endif

  u4Cnt = 0; 
#ifdef EXT_COMPARE 
  if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
  {
  #ifdef DIRECT_DEC
    if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
  #endif    
    {
      vWrite2PC(u4InstID, 5, _pucDecWorkBuf[u4InstID]);
    }
  }
#else
#if defined(DOWN_SCALE_SUPPORT)
  pbDecBuf = _pucVDSCLBuf[u4InstID];
#elif defined(FGT_SUPPORT)
  pbDecBuf = _pucFGTBuf[u4InstID];
#else
  pbDecBuf = _pucPic0Y[u4InstID]; //_pucDecWorkBuf;
#endif  

#ifdef DIRECT_DEC
  if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
#endif    
  {
    for(i=0; i< _u4GoldenYSize[u4InstID]; i++)
    {
      if((_pucDumpYBuf[u4InstID][i] != pbDecBuf[i]))
      {
        u4Cnt ++;
        vVDecOutputDebugString("Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpYBuf[u4InstID][i]);
        sprintf(strMessage,"Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpYBuf[u4InstID][i]);
        fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
        //fprintf(_tFileListRecInfo.fpFile, "Y Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i], _pucDumpBuf[i]);

        //disable temporary?
        fgDecErr = TRUE;
        //vDumpReg();  // mark by ginny
        //vWrite2PC(_u4VDecID, 2, _pucDecWorkBuf);

        //disable temporary
	//vWrite2PC(_u4VDecID, 2, pbDecBuf);
#if defined(FGT_SUPPORT) || defined(DOWN_SCALE_SUPPORT)
        vWrite2PC(u4InstID, 5, _pucDecWorkBuf[u4InstID]);
#endif        
        break;        
      }
    }
    vVDecOutputDebugString("\nY Data Compare Over!!! Total bytes [0x%.8x] & error [%d]\n", _u4GoldenYSize[u4InstID], u4Cnt);
  }
#endif

  // CbCr compare
  //if(!fgIsMonoPic(_u4VDecID))
  {
    // CbCr decoded data Compare
    _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpCBuf[u4InstID];
    _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_C_SZ;  
    _tFBufFileInfo[u4InstID].u4FileLength = 0;
    vConcateStr(_bFileStr1[u4InstID][4], _bFileStr1[u4InstID][0], _bFileAddStrC[0], _u4FileCnt[u4InstID]);
  #ifdef EXT_COMPARE 
  _tFBufFileInfo[u4InstID].u4FileLength = (((_ptCurrFBufInfo[u4InstID]->u4W + 15)>>4)<<4) *(((_ptCurrFBufInfo[u4InstID]->u4H + 31)>>5)<<5) >>1;
  #else
    #ifdef DIRECT_DEC
    if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
    #endif    
    {  
    #ifdef  SATA_HDD_READ_SUPPORT
      fgOpenHDDFile(u4InstID, _bFileStr1[u4InstID][4],"r+b", &_tFBufFileInfo[u4InstID]);
    #else
      fgOpenPCFile(u4InstID, _bFileStr1[u4InstID][4],"r+b", &_tFBufFileInfo[u4InstID]);
    #endif
      _u4GoldenCSize[u4InstID] = _tFBufFileInfo[u4InstID].u4RealGetBytes;
    }
  #endif    
#ifdef DOWN_SCALE_SUPPORT
  u4Upd = (_tVerMpvDecPrm[u4InstID].rDownScalerPrm.u4TrgHeight >> 1) % 16;
#else
  //u4Upd = (_ptCurrFBufInfo->u4H >> 1) % 16;
#endif  
    u4Pic32XSize >>= 1;    
    u4Cnt = 0; 
#ifdef EXT_COMPARE  
    if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
    {
    #ifdef DIRECT_DEC
      if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
    #endif 
      {
        vWrite2PC(u4InstID, 6, _pucDecWorkBuf[u4InstID]+u4Size);
      }
    }
#else    
  #if defined(DOWN_SCALE_SUPPORT)
    pbDecBuf = _pucVDSCLBuf[u4InstID];
  #elif defined(FGT_SUPPORT)
    pbDecBuf = _pucFGTBuf[u4InstID];
  #else
    pbDecBuf = _pucPic0C[u4InstID]; //_pucDecWorkBuf;
  #endif  
  
  #ifdef DIRECT_DEC
    if((_u4FileCnt[u4InstID] >= _u4StartCompPicNum[u4InstID]) && ( _u4FileCnt[u4InstID] <= _u4EndCompPicNum[u4InstID]))
  #endif      
    {
      for(i=0; i< _u4GoldenCSize[u4InstID]; i++)
      {
        if(_pucDumpCBuf[u4InstID][i] != pbDecBuf[i])
        {
          u4Cnt ++ ;
          vVDecOutputDebugString("CbCr Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i + u4Size], _pucDumpCBuf[u4InstID][i]);
          sprintf(strMessage,"CbCr Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i + u4Size], _pucDumpCBuf[u4InstID][i]);
          fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
          //fprintf(_tFileListRecInfo.fpFile, "CbCr Data Mismatch at [0x%.8x] = 0x%.2x, Golden = 0x%.2x !!! \n", i, pbDecBuf[i + u4Size], _pucDumpBuf[i]);

	  //disable temporary?
          fgDecErr = TRUE;        
          
          //vDumpReg();     // mark by ginny   
          //vWrite2PC(_u4VDecID, 3, _pucDecWorkBuf+u4Size);
          
	  //disable temporary
          //vWrite2PC(_u4VDecID, 3, pbDecBuf+u4Size);
          
#if defined(FGT_SUPPORT) || defined(DOWN_SCALE_SUPPORT)
          vWrite2PC(u4InstID, 6, _pucDecWorkBuf[u4InstID]+u4Size);
#endif
          break;        
        }
      }
      vVDecOutputDebugString("CbCr Data Compare Over!!! Total bytes [0x%.8x] & error [%d]\n", _u4GoldenCSize[u4InstID], u4Cnt);
    }
#endif    
  }
#endif

  vVDecOutputDebugString("Pic count to [%d]\n", _u4FileCnt[u4InstID]);  
  sprintf(strMessage,"[%d], ", _u4FileCnt[u4InstID]);  
  fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
  //fprintf(_tFileListRecInfo.fpFile, "[%d], ", _u4FileCnt[_u4VDecID]);  


   _ptCurrFBufInfo[u4InstID]->u4FrameCnt = _u4FileCnt[u4InstID];
  if(_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME)
  {
#ifdef REDEC    
    if(_u4ReDecCnt[u4InstID] == 0)
#endif    
      _u4FileCnt[u4InstID] ++;
      vSetDecFlag(u4InstID, DEC_FLAG_CHG_FBUF);
  }
  //else
  {
    _ucPrevDecFld[u4InstID] = _ptCurrFBufInfo[u4InstID]->ucFBufStatus;
  }

  // Check if still pic needed compare
#if (defined(COMP_HW_CHKSUM) && (!defined(DOWN_SCALE_SUPPORT)) && (!defined(FGT_SUPPORT)))
    _tTempFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tTempFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
    _tTempFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
    _tTempFileInfo[u4InstID].u4FileLength = 0; 
    vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], "_chksum.bin\0", _u4FileCnt[u4InstID]);
    fgOpen = fgOpenFile(u4InstID, _bTempStr1[u4InstID],"r+b", &_tTempFileInfo[u4InstID]);
#else
  _tFBufFileInfo[u4InstID].fgGetFileInfo = FALSE;  
  _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID];
  _tFBufFileInfo[u4InstID].u4TargetSz = GOLD_Y_SZ;  
  _tFBufFileInfo[u4InstID].u4FileLength = 4;      
  vConcateStr(_bFileStr1[u4InstID][3], _bFileStr1[u4InstID][0], _bFileAddStrY[0], _u4FileCnt[u4InstID]);
  fgOpen = fgOpenFile(u4InstID, _bFileStr1[u4InstID][3],"r+b", &_tFBufFileInfo[u4InstID]);
#endif
  if((fgOpen == FALSE) ||(fgDecErr == TRUE) || (_fgVDecErr[u4InstID] == TRUE))
  {
    sprintf(strMessage, "%s", "\n");
    fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
    //fprintf(_tFileListRecInfo.fpFile, "\n");  
    // break decode
    if(fgOpen == FALSE)
    {
      sprintf(strMessage," Compare Finish==> Pic count to [%d] \n", _u4FileCnt[u4InstID] - 1);   
      fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
      //fprintf(_tFileListRecInfo.fpFile, " Compare Finish==> Pic count to [%d] \n", _u4FileCnt[_u4VDecID] - 1);   
      if(_u4FileCnt[u4InstID] == 1)
      {
        if(fgOpen == FALSE)
        {
          vVDecOutputDebugString("real NULL\n");
        }
      }
      else //flush DPB buffer
      {
        vFlushDPB(u4InstID, &_tVerMpvDecPrm[u4InstID], FALSE);
      }
    }
    else
    {
      sprintf(strMessage," Error ==> Pic count to [%d] \n", _u4FileCnt[u4InstID] - 1);     
      fgWrMsg2PC(strMessage,strlen(strMessage),8,&_tFileListRecInfo[u4InstID]);
      //fprintf(_tFileListRecInfo.fpFile, " Error ==> Pic count to [%d] \n", _u4FileCnt[_u4VDecID] - 1);         
    }
    _u4VerBitCount[u4InstID] = 0xffffffff;
  }
}

void reset_dec_counter(UINT32 u4InstID)
{
    u4H264Cnt = 0;
    fgH264YCrcOpen = FALSE;

    fgYCrcOpen = FALSE;
    fgCbCrCrcOpen = FALSE;
    u4FilePicNum = 0;
    u4FileTotalPicNum = 0;
    u4FilePicCont_noVOP = 0;
    u4FirstErrFrm = 0xFFFFFFFF;
    _u4VerBitCount[u4InstID] = 0xFFFFFFFF;
    _u4FileCnt[u4InstID] = 0;
}

