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
#include "vdec_hw_common.h"
#include "vdec_hal_errcode.h"
#include "vdec_hal_if_vp9.h"
#include "vdec_hal_if_common.h"


#if CONFIG_DRV_VERIFY_SUPPORT
#include "../verify/vdec_verify_general.h"
#include "../verify/vdec_verify_mpv_prov.h"

#ifdef CONFIG_TV_DRV_VFY 
#include <mach/cache_operation.h>
#endif // CONFIG_TV_DRV_VFY

#if (!CONFIG_DRV_LINUX)
#include <stdio.h>
#include <string.h>
#endif


#endif
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>


//extern int VP9_RegDump(UINT32 u4Base, UINT32 u4Start, UINT32 u4End , UINT32 frame_number, BOOL bDecodeDone);

int VP9_RegDump(UINT32 u4Base, UINT32 u4Start, UINT32 u4End , UINT32 frame_number, BOOL bDecodeDone)
{ 
	int ret,i;
    	UINT32 u4Value;   

    if (bDecodeDone)
    {
        printk("================== Decode Done register dump ==================\n");
    }
    else
    {
        printk("================== Before trigger decode register dump ==================\n");
    }

    for ( i = u4Start ; i <= u4End ; i++ )
    {
        u4Value = u4ReadReg( u4Base+i*4 );
        
        if ( u4Base == VLD_REG_OFFSET0 )
            printk( "VLD[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == VDEC_BS2_OFFSET0 )
            printk( "VLD0_BS2[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == MC_REG_OFFSET0 )
            printk( "MC[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == VP9_VLD_REG_OFFSET0 )
            printk( "VP9_VLD[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == VP9_VLD_REG_OFFSET1 )
            printk( "VP9_VLD1[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == HEVC_PP_REG_OFFSET0 )
            printk( "PP[%d] = 0x%08.0X    (Addr: 0x%08.0X)   ",i,u4Value, u4Base + 4*i);
        if ( u4Base == HEVC_MV_REG_OFFSET0 )
            printk( "MV[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == HEVC_MISC_REG_OFFSET0 )
            printk( "MISC[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == VLD_TOP_REG_OFFSET0 )
            printk( "VLD_TOP[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == VLD_REG_OFFSET1 )
            printk( "VLD1[%d] = 0x%08.0X   (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
       // if ( u4Base == VDEC_BS2_OFFSET1 )
       //     printk( "VLD1_BS2[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == MC_REG_OFFSET1 )
            printk( "MC1[%d] = 0x%08.0X   (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == HEVC_VLD_REG_OFFSET1 )
            printk( "HEVC_VLD1[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == HEVC_PP_REG_OFFSET1 )
            printk( "PP1[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == HEVC_MV_REG_OFFSET1 )
            printk( "MV1[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == HEVC_MISC_REG_OFFSET1 )
            printk( "MISC1[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == VLD_TOP_REG_OFFSET1 )
            printk( "VLD_TOP1[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == MVDEC_TOP_OFFSET0 )
            printk( "VLD_MCore[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == LAE_VLD_OFFSET0 )
            printk( "LAE_VLD_OFFSET0[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == LAE_BS2_OFFSET0)
            printk( "LAE_BS2[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == LAE_VLDTOP_OFFSET0 )
            printk( "LAE_VLDTOP_OFFSET0[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == LAE_AVCVLD_OFFSET0 )
            printk( "LAE_AVCVLD_OFFSET0[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == LAE_MISC_OFFSET0 )
            printk( "LAE_MISC_OFFSET0[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == LAE_HEVCVLD_OFFSET0 )
            printk( "LAE_HEVCVLD_OFFSET0[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        //if ( u4Base == LAE_LARB_OFFSET0 )
        //    printk( "LAE_LARB_OFFSET0[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == LAE_VP9_VLD_OFFSET0 )
            printk( "LAE_VP9_VLD_OFFSET0[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);
        if ( u4Base == LAE_MV_OFFSET0)
            printk( "LAE_MV_OFFSET0[%d] = 0x%08.0X    (Addr: 0x%08.0X)  ",i,u4Value, u4Base + 4*i);

        if((i - u4Start)%2)
        {
            printk("\n");
        }        
    }
    printk("\n");
    return 0;
}
extern INT32 x_sema_lock_timeout (HANDLE_T  h_sema_hdl, UINT32    ui4_time);
//#include "U_os.h"
BOOL vVerVP9WaitDecEnd(VP9_COMMON_T* prCommon, UINT32 u4InstID)
{
    BOOL fgIsTimeOut = 0;
    INT32 i4Ret = 0;
	//UCHAR ucTryCount = 50;
	//do
	{
		i4Ret = x_sema_lock_timeout(*(prCommon->p_hVdecDecDone), 500);
		/*In verify code, we just use _hDecodeFinSema[VDEC_VLD0]*/
		//ucTryCount--;
    }//while((i4Ret != 0));

	if(i4Ret)
	{	
	    goto TIME_OUT;
	}
	else
	{
	    goto DECODE_DONE;
	}
TIME_OUT:
	VP9_PRINT_INFO("\n!!!!!! Decode int timeout !!!!!!\n\n");
	fgIsTimeOut = 1;
	return fgIsTimeOut;
	
DECODE_DONE:
	VP9_PRINT_INFO("[%d] Decode Done...\n", prCommon->u4InstID);	
	fgIsTimeOut = 0;
	return fgIsTimeOut;

}
void vVDEC_HAL_VP9_SW_Reset(UINT32 u4CoreId)
{    
    UINT32 u4Ret = 1;
    UINT32 u4FPEG_Ver;
    VP9_PRINT_INFO("%s", __FUNCTION__);
    
    if( u4CoreId == VP9_LAE_ID ) //LAE sw reset
    {
        vVP9RISCRead_MCore_TOP(14 , &u4Ret, u4CoreId);
        vVP9RISCWrite_MCore_TOP(14, (u4Ret | (1 << 24)) ,u4CoreId);
    }
	
    // ASYNC hw function, repeatedly called for delay 
    vVP9RISCWrite_VLD(66, 0x101, u4CoreId);
    vVP9RISCWrite_VLD(66, 0x101, u4CoreId);
    vVP9RISCWrite_VLD(66, 0x101, u4CoreId);
    vVP9RISCWrite_VLD(66, 0x101, u4CoreId);
    vVP9RISCWrite_VLD(66, 0x101, u4CoreId);

    // [vpx]
    vVP9RISCWrite_MISC(33,0x2,u4CoreId); // mid sys_clk selection

    // temporal all truned on
    vVP9RISCWrite_MISC(50, 0xFDFF, u4CoreId);
    vVP9RISCWrite_MISC(51, 0x71E31180, u4CoreId);
    vVP9RISCWrite_MISC(94, 0xFFFFF0C1, u4CoreId);
	//------realchip----------//
    vVP9RISCWrite_MISC( 61, 0, u4CoreId);
    vWriteGconReg(6*4, 1);
     vVDecWriteDV( u4CoreId, 59*4, 1);
	 //=====realchip----------//
	
    vVP9RISCRead_MISC(RW_PDN_CTRL, &u4Ret, u4CoreId);
    
    vVP9RISCWrite_VLD(66, 0x0, u4CoreId);

    // enable CRC Check ,1 mc_out, 3: pp_out
    vVP9RISCWrite_MISC(1, 0x3, u4CoreId);
    vVP9RISCWrite_VP9_VLD(41, 0x1, u4CoreId);
    
    // COUNT TBL Clear
    vVP9RISCWrite_VP9_VLD(106, 1, u4CoreId);
    
     // polling
    vVP9RISCRead_VP9_VLD(106, &u4Ret, u4CoreId);

    // [vpx]
    // need to add count break, avoid infinite loop
    while( ((u4Ret) & 0x1) == 1)
    {
        vVP9RISCRead_VP9_VLD(106, &u4Ret, u4CoreId);
    }   
    

    vVP9RISCRead_VLD_TOP(51, &u4FPEG_Ver, 0);
    VP9_PRINT_INFO("Bit-File Version  %d\n", u4FPEG_Ver);
	
    if( u4CoreId == VP9_LAE_ID ) //LAE sw reset
    {
        vVP9RISCRead_MCore_TOP(14 , &u4Ret, u4CoreId);
        vVP9RISCWrite_MCore_TOP(14, (u4Ret & ~(1 << 24)), u4CoreId);
    }
    
    return;
}

void vVDEC_HAL_VP9_Mcore_Init(UINT32 u4LaeBufAddr, UINT32 u4ErrBufAddr)
{    
    VP9_PRINT_INFO("Init Multi Core!!\n");
    
    //reset mcore, core 0/1
    vVP9RISCWrite_MCore_TOP(7, 0x1, 0);
    vVP9RISCWrite_MCore_TOP(7, 0x0, 0);
    
    // set addr
    vVP9RISCWrite_MCore_TOP(8, PHYSICAL(u4LaeBufAddr), 0);
    vVP9RISCWrite_MCore_TOP(9, PHYSICAL(u4ErrBufAddr), 0);
    
    vVP9RISCWrite_MCore_TOP(16, PHYSICAL(u4LaeBufAddr), 0);
    vVP9RISCWrite_MCore_TOP(17, PHYSICAL(u4ErrBufAddr), 0);

    vVP9RISCWrite_MCore_TOP(18, 0x120, 0); // 288, unit is 16 bytes
    //vVP9RISCWrite_MCore_TOP(24, 0x1, 0);
    vVP9RISCWrite_MCore_TOP(26, 0x22, 0);
    return;
}

void vVDEC_HAL_VP9_Mcore_Enable(BOOL fgEnable)
{
    if(fgEnable)
    {
        vVP9RISCWrite_MCore_TOP(24, 1, 0);
    }
    else
    {
        vVP9RISCWrite_MCore_TOP(24, 0, 0);    
    }
}

// **************************************************************************
// Function : INT32 i4VDEC_HAL_VP8_InitBarrelShifter(UINT32 u4BSID, UINT32 u4VDecID, VDEC_INFO_VP8_FRM_HDR_T *prVp8BSInitPrm, BOOL fgIsVC1);
// Description :Initialize barrel shifter with byte alignment
// Parameter : u4ReadPointer : set read pointer value
//                 u4WrtePointer : set write pointer value
//                 u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : =0: success.
//                  <0: fail.
// **************************************************************************
INT32 i4VDEC_HAL_VP9_InitBarrelShifter(UINT32 u4CoreId, ULONG u4VldRp, ULONG u4VldRpEnd, ULONG u4FifoStart, ULONG u4FifoEnd)
{
    UINT32 u4RetRegValue;
    ULONG u4VldRptr;
    ULONG u4VldWptr;
    static UINT32 u4Inited = 0;    
    VP9_PRINT_INFO("VP9_InitBarrelShifter frame:[0x%lx -- 0x%lx] fifo:[0x%lx -- 0x%lx]", u4VldRp, u4VldRpEnd, u4FifoStart, u4FifoEnd);

    u4VldRptr = PHYSICAL(u4VldRp);

    //set fifo start & end
    vVP9RISCWrite_VLD( 45, ((PHYSICAL(u4FifoStart)) >> 6), u4CoreId);
    vVP9RISCWrite_VLD( 46, ((PHYSICAL(u4FifoEnd)) >> 6), u4CoreId);
    vVP9RISCWrite_BS2( 45, ((PHYSICAL(u4FifoStart)) >> 6), u4CoreId);
    vVP9RISCWrite_BS2( 46, ((PHYSICAL(u4FifoEnd)) >> 6), u4CoreId);
    
#if 1
    vVP9RISCRead_VLD(59, &u4RetRegValue, u4CoreId);
    vVP9RISCWrite_VLD(59, u4RetRegValue | 0x10000000, u4CoreId);
    vVP9RISCRead_BS2(59, &u4RetRegValue, u4CoreId);
    vVP9RISCWrite_BS2(59, u4RetRegValue | 0x10000000, u4CoreId);    
#endif

    // polling Sram stable 
    vVP9RISCRead_VLD( 61 , &u4RetRegValue, u4CoreId);

    if( ((u4RetRegValue >> 15 ) & 0x1) == 1) 
    {
       // polling VLD_61  [0] == 1
        vVP9RISCRead_VLD( 61 , &u4RetRegValue, u4CoreId );
        while ( (u4RetRegValue & 0x1)   !=  1)    
            vVP9RISCRead_VLD( 61 , &u4RetRegValue , u4CoreId);
    }

    // read pointer
    vVP9RISCWrite_VLD( 44, u4VldRptr, u4CoreId);
    u4VldWptr = ((u4VldRpEnd + 512) >= u4FifoEnd) ? (u4VldRpEnd + 512 - VP9_V_FIFO_SZ): (u4VldRpEnd + 512);
    vVP9RISCWrite_VLD( 68, PHYSICAL(u4VldWptr), u4CoreId);

    //BITstream DMA async_FIFO  local reset
    vVP9RISCRead_VLD( 66 , &u4RetRegValue, u4CoreId );
    // for delay
    vVP9RISCWrite_VLD( 66, u4RetRegValue |(1 << 8), u4CoreId);
    vVP9RISCWrite_VLD( 66, u4RetRegValue |(1 << 8), u4CoreId);
    vVP9RISCWrite_VLD( 66, u4RetRegValue |(1 << 8), u4CoreId);
    vVP9RISCWrite_VLD( 66, u4RetRegValue |(1 << 8), u4CoreId);
    vVP9RISCWrite_VLD( 66, u4RetRegValue |(1 << 8), u4CoreId);
    vVP9RISCWrite_VLD( 66, 0, u4CoreId);

    //initial fetch
    //vVP9RISCRead_VLD( 35 , &u4RetRegValue, u4CoreId );
    //vVP9RISCWrite_VLD( 35, u4RetRegValue |(1 << 20), u4CoreId );
    vVP9RISCWrite_VLD( 35, 0x100000, u4CoreId );
    vVP9RISCRead_VLD( 58 , &u4RetRegValue, u4CoreId );
    
    while ( (u4RetRegValue & 0x1) !=  1)
    {
        vVP9RISCRead_VLD( 58, &u4RetRegValue, u4CoreId );
    }

    //initial barrel shifter
    //vVP9RISCRead_VLD( 35 , &u4RetRegValue, u4CoreId );      
    //vVP9RISCWrite_VLD( 35, u4RetRegValue|(1 << 23), u4CoreId );
    vVP9RISCWrite_VLD( 35, 0x800000, u4CoreId );

    UINT32 u4RdptrLsb = u4VldRptr & 0x0F;
    UINT32 u4ShiftBytesCount = 0;
    while(u4ShiftBytesCount < u4RdptrLsb)
    {
        u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 8);
        u4ShiftBytesCount++;
    }
    //VERIFY(u4VDEC_HAL_VP9_Reset_Bits_Count(u4CoreId) == 0);
   u4VDEC_HAL_VP9_Reset_Bits_Count(u4CoreId);
    return 0;
}

void vVDEC_HAL_VP9_InitBool(UINT32 u4CoreId)
{
    UINT32 u4Ret = 0;
    VP9_PRINT_INFO("------%s", __FUNCTION__);
    vVP9RISCWrite_VP9_VLD(68, 1, u4CoreId);
    vVP9RISCRead_VP9_VLD(68, &u4Ret, u4CoreId);
    // [vpx]
    // check bit 16
    // need to add count break, avoid infinite loop    
    while (((u4Ret >> 16) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(68, &u4Ret, u4CoreId);
    }
    
    return;   
}

//read input window
UINT32 u4VDEC_HAL_VP9_Get_Input_Window(UINT32 u4CoreId)
{
    UINT32 u4Ret = 0;
    vVP9RISCRead_VP9_VLD(0 , &u4Ret, u4CoreId);
    return u4Ret;
}

UINT32 u4VDEC_HAL_VP9_Read_Literal_Raw(UINT32 u4CoreId, UINT32 u4ShiftBit)
{
    UINT32 u4Ret = 0;    
    UINT32 u4Temp = 0;
    // it's possible to shift 0 bit
    ASSERT(u4ShiftBit >= 0 && u4ShiftBit <= 32);
    
    vVP9RISCRead_VP9_VLD(0 , &u4Ret, u4CoreId);
    vVP9RISCRead_VP9_VLD(u4ShiftBit , &u4Temp, u4CoreId);
    u4Ret = (u4Ret >> (32 - u4ShiftBit));
    VP9_PRINT_VERBOSE("VP9 Read %d Bit(Raw) ret:0x%x", u4ShiftBit, u4Ret);
    return u4Ret;
}

INT32 i4VDEC_HAL_VP9_Read_Signed_Literal_Raw(UINT32 u4CoreId, UINT32 u4ShiftBit)
{
    INT32 i4Ret = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, u4ShiftBit);
    return u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId) ? -i4Ret : i4Ret;
}

UINT32 u4VDEC_HAL_VP9_Read_Bit_Raw(UINT32 u4CoreId)
{
    UINT32 u4Ret = 0;    
    UINT32 u4Temp = 0;
    vVP9RISCRead_VP9_VLD(0 , &u4Ret, u4CoreId);
    vVP9RISCRead_VP9_VLD(1 , &u4Temp, u4CoreId);
    u4Ret = (u4Ret >> 31);
    VP9_PRINT_VERBOSE("VP9 Read 1 Bit(Raw) ret:0x%x", u4Ret);
    return u4Ret;
}

UINT32 u4VDEC_HAL_VP9_Get_Vld_Read_Pointer(UINT32 u4CoreId)
{
    UINT32 u4Ret = 0;
    vVP9RISCRead_VLD(63 , &u4Ret, u4CoreId);
    VP9_PRINT_VERBOSE("VP9 get vld read pointer: 0x%x", u4Ret);
    return u4Ret;
}

UINT32 u4VDEC_HAL_VP9_Get_Bits_Count(UINT32 u4CoreId)
{
    UINT32 u4Ret = 0;
    vVP9RISCRead_VP9_VLD(78 , &u4Ret, u4CoreId);
    VP9_PRINT_VERBOSE("get bits count: %d", u4Ret);
    return u4Ret;
}

UINT32 u4VDEC_HAL_VP9_Reset_Bits_Count(UINT32 u4CoreId)
{    
    UINT32 u4Ret = 0;
    vVP9RISCWrite_VP9_VLD(81 , 0x1, u4CoreId);
    vVP9RISCWrite_VP9_VLD(81 , 0x0, u4CoreId);
    vVP9RISCRead_VP9_VLD(78 , &u4Ret, u4CoreId);
    VP9_PRINT_VERBOSE("VP9 Reset bits count");
    return u4Ret;
}

UINT32 u4VDEC_HAL_VP9_Read_Literal(UINT32 u4CoreId, UINT32 u4ShiftBit)
{
    UINT32 u4Ret = 0;
    vVP9RISCRead_VP9_VLD(32 + u4ShiftBit, &u4Ret, u4CoreId);    
    VP9_PRINT_VERBOSE("VP9 Read %d Bit(Compressed) ret:0x%x", u4ShiftBit, u4Ret);
    return u4Ret;
}

UINT32 u4VDEC_HAL_VP9_Read_Bit(UINT32 u4CoreId)
{
    UINT32 u4Ret = 0;
    vVP9RISCRead_VP9_VLD(32 + 1, &u4Ret, u4CoreId);    
    VP9_PRINT_VERBOSE("VP9 Read 1 Bit(Compressed) ret:0x%x", u4Ret);
    return u4Ret;
}

void vVDEC_HAL_VP9_Set_Segmentation(UINT32 u4CoreId, SEGMENTATION* pSeg, UINT32 u4SegAddr)
{    
    UINT32 u4Tmp;
    if (pSeg->enabled)
    {
        // initialize seg_id prob table
        u4Tmp = (pSeg->tree_probs[3] << 24) | (pSeg->tree_probs[2] << 16) | (pSeg->tree_probs[1] << 8) | (pSeg->tree_probs[0]);
        vVP9RISCWrite_VP9_VLD(124, u4Tmp, u4CoreId);
        u4Tmp = (pSeg->tree_probs[6] << 16) | (pSeg->tree_probs[5] << 8) | (pSeg->tree_probs[4]);
        vVP9RISCWrite_VP9_VLD(125, u4Tmp, u4CoreId);
        u4Tmp = (pSeg->pred_probs[2] << 16) | (pSeg->pred_probs[1] << 8) | (pSeg->pred_probs[0]);
        vVP9RISCWrite_VP9_VLD(126, u4Tmp, u4CoreId);

        vVP9RISCWrite_VLD_TOP(70, PHYSICAL(u4SegAddr), u4CoreId);
        vVP9RISCWrite_VLD_TOP(71, PHYSICAL(u4SegAddr), u4CoreId);
    }
}

#define ALIGN_POWER_OF_TWO(value, n) \
    (((value) + ((1 << (n)) - 1)) & ~((1 << (n)) - 1))
    
#define MIN_TILE_WIDTH_B64 4
#define MAX_TILE_WIDTH_B64 64

//static INLINE int mi_cols_aligned_to_sb(int n_mis) {
static int mi_cols_aligned_to_sb(int n_mis) {

    return ALIGN_POWER_OF_TWO(n_mis, MI_BLOCK_SIZE_LOG2);
}

static int get_tile_offset(int idx, int mis, int log2) {
    const int sb_cols = mi_cols_aligned_to_sb(mis) >> MI_BLOCK_SIZE_LOG2;
    const int offset = ((idx * sb_cols) >> log2) << MI_BLOCK_SIZE_LOG2;
    //return MIN(offset, mis);
    if(offset <mis)
		return offset;
	else
		return mis;
}

void vp9_tile_set_row(VP9_TILE_INFO_T *pTile, const VP9_COMMON_T *prCommon, int row) {
    pTile->i4MiRowStart = get_tile_offset(row, prCommon->u4MI_rows, prCommon->rUnCompressedHeader.u4Log2TileRows);
    pTile->i4MiRowEnd = get_tile_offset(row + 1, prCommon->u4MI_rows, prCommon->rUnCompressedHeader.u4Log2TileRows);
    VP9_PRINT_VERBOSE("vp9_tile_set_row %d, i4MiRowStart %d, i4MiRowEnd %d",row ,pTile->i4MiRowStart , pTile->i4MiRowEnd);
}

void vp9_tile_set_col(VP9_TILE_INFO_T *pTile, const VP9_COMMON_T *prCommon, int col) {
    pTile->i4MiColStart = get_tile_offset(col, prCommon->u4MI_cols, prCommon->rUnCompressedHeader.u4Log2TileCols);
    pTile->i4MiColEnd = get_tile_offset(col + 1, prCommon->u4MI_cols, prCommon->rUnCompressedHeader.u4Log2TileCols);
    VP9_PRINT_VERBOSE("vp9_tile_set_col %d, i4MiColStart %d, i4MiColEnd %d",col ,pTile->i4MiColStart , pTile->i4MiColEnd);
}

void vp9_tile_init(VP9_TILE_INFO_T *pTile, const VP9_COMMON_T *prCommon, int row, int col) {
    vp9_tile_set_row(pTile, prCommon, row);
    vp9_tile_set_col(pTile, prCommon, col);
}

// Reads the next tile returning its size and adjusting '*data' accordingly
// based on 'is_last'.
static UINT32 get_tile_buffer(ULONG ulDataEnd, ULONG u4FifoEnd, UINT32 u4IsLast, ULONG ulDataStart, TileBuffer *pTileBuf) 
{
    UINT32 u4Size = 0;
    UCHAR* pData = (UCHAR *)(ulDataStart);
    UINT32 u4Offset = 0;
    UINT32 u4LeftSize = 0;

    VP9_PRINT_INFO("get_tile_buffer -- data start 0x%x ", (UINT32)ulDataStart);
    if(ulDataStart > ulDataEnd)
    {
        //ringfifo case
        u4LeftSize = VP9_V_FIFO_SZ - (ulDataStart - ulDataEnd);
    }
    else
    {
        u4LeftSize = ulDataEnd - ulDataStart;
    }
    
    if (!u4IsLast) 
    {
        if(!(4 <= u4LeftSize))
        {
            VP9_PRINT_ERROR("Truncated packet or corrupt tile length!");
            return;
        }
       
        // Here is unsigned char ptr, access 1 byte each time
        // can not use pData[0] to get 4 bytes
        
        UINT32 u4Byte1 = (UINT32)(*(pData++));
        if(((ULONG)pData) == u4FifoEnd)  //qianqian@20150215
        {
            pData = (UCHAR*)(u4FifoEnd - VP9_V_FIFO_SZ);
        }
        UINT32 u4Byte2 = (UINT32)(*(pData++));
        if(((ULONG)pData) == u4FifoEnd)
        {
            pData = (UCHAR*)(u4FifoEnd - VP9_V_FIFO_SZ);
        }
        UINT32 u4Byte3 = (UINT32)(*(pData++));
        if(((ULONG)pData) == u4FifoEnd)
        {
            pData = (UCHAR*)(u4FifoEnd - VP9_V_FIFO_SZ);
        }
        UINT32 u4Byte4 = (UINT32)(*(pData++));
        if(((ULONG)pData) == u4FifoEnd)
        {
            pData = (UCHAR*)(u4FifoEnd - VP9_V_FIFO_SZ);
        }
        u4Size = u4Byte1 << 24 | u4Byte2 << 16 | u4Byte3 << 8 | u4Byte4;
        u4Offset += 4;
        
        //VP9_PRINT_INFO(" ##### Size:%x", u4Size);        
        //VP9_PRINT_INFO(" ##### u4DataEnd:%x", u4DataEnd);
        if (u4Size > u4LeftSize)  
        {
            VP9_PRINT_ERROR("Truncated packet or corrupt tile length!");
            return;
        }
    }
    else
    {
        u4Size = u4LeftSize;
    }
    
    pTileBuf->data = (UCHAR*)pData;
    pTileBuf->size = u4Size;

    //VP9_PRINT_INFO("Tile Info: Offset:0x%x, Size:%x, Colo:%d", pTileBuf->data, pTileBuf->size, pTileBuf->col);

    u4Offset += u4Size;

    return u4Offset;
}

void vVDEC_HAL_VP9_Set_Tile_Info(UINT32 u4CoreId, VP9_COMMON_T* prCommon)
{    
    UINT32 u4TileRows = 1 << prCommon->rUnCompressedHeader.u4Log2TileRows;
    UINT32 u4TileCols = 1 << prCommon->rUnCompressedHeader.u4Log2TileCols;
    UINT32 u4TileRow;
    UINT32 u4TileCol;
    UINT32 u4TileNum;
    UINT32 u4Tmp;
    UINT32 u4ActualTileRows = u4TileRows;
    VP9_INPUT_CTX_T* pInputCtx = &prCommon->rInputCtx;
    ULONG ulData = pInputCtx->ulVaFrameStart + pInputCtx->u4UnCompressSize + prCommon->rUnCompressedHeader.u4FirstPartitionSize;
    ULONG ulDataEnd = pInputCtx->ulVaFrameEnd;
    UINT32 u4Offset = 0;
    //VP9_PRINT_INFO("vVDEC_HAL_VP9_Set_Tile_Info data addr = 0x%x", u4Data);
    for (u4TileRow = 0; u4TileRow < u4TileRows; ++u4TileRow) 
    {
        for (u4TileCol = 0; u4TileCol < u4TileCols; ++u4TileCol) 
        {
            UINT32 u4IsLast = (u4TileRow == u4TileRows - 1) && (u4TileCol == u4TileCols - 1);
            ULONG ulDataStart = ulData + u4Offset;
            ulDataStart = (ulDataStart >= pInputCtx->ulVaFifoEnd) ? (ulDataStart - VP9_V_FIFO_SZ) : ulDataStart;
            TileBuffer * pTileBuf = &(prCommon->TILE_INFOS[u4TileRow][u4TileCol]);
            pTileBuf->col = u4TileCol;
            u4Offset += get_tile_buffer(ulDataEnd, pInputCtx->ulVaFifoEnd, u4IsLast, ulDataStart, pTileBuf);
        }
    }
    
      // Load all tile information into tile_data.
    for (u4TileRow = 0; u4TileRow < u4TileRows; ++u4TileRow) {
        for (u4TileCol = 0; u4TileCol < u4TileCols; ++u4TileCol) {
            VP9_TILE_INFO_T rTile;
            TileBuffer *pTileBuf = &(prCommon->TILE_INFOS[u4TileRow][u4TileCol]);
            vp9_tile_init(&rTile, prCommon, u4TileRow, u4TileCol);
            UINT32 u4Yy = ((((rTile.i4MiRowEnd - rTile.i4MiRowStart)+7)>>3));
            if (u4Yy == 0){
                if (u4TileCol == 0)
                    u4ActualTileRows = u4ActualTileRows - 1;
            }
        }
    }
    
    u4TileNum = u4TileCols * u4ActualTileRows;
    vVP9RISCWrite_VP9_VLD(49,  ((u4ActualTileRows-1)<<16) + (u4TileCols - 1), u4CoreId);
    vVP9RISCWrite_VP9_VLD(64, 0x0, u4CoreId);
    UINT32* u4TileDram = (UINT32 *)prCommon->rTileBuffer.ulBufVAddr; //qianqian@20150215
    for (u4TileRow = 0; u4TileRow < u4TileRows; ++u4TileRow)
    {
        VP9_TILE_INFO_T rTile;
        vp9_tile_set_row(&rTile, prCommon, u4TileRow);
        
        for (u4TileCol = 0; u4TileCol < u4TileCols; ++u4TileCol)
        {
            vp9_tile_set_col(&rTile, prCommon, u4TileCol);

            TileBuffer *pTileBuf = &(prCommon->TILE_INFOS[u4TileRow][u4TileCol]);
            UINT32 u4TileAddr = PHYSICAL(pTileBuf->data);
            UINT32 u4BitOffset = (u4TileAddr - PHYSICAL(pInputCtx->ulVaFifoStart)) << 3;
            UINT32 u4TileSize = pTileBuf->size;
            
            if (((((rTile.i4MiRowEnd - rTile.i4MiRowStart)+7)>>3)))
            {
                UINT32 u4RiscData1 = (u4TileSize << 3) + (u4BitOffset & 0x7f);
                UINT32 u4RiscData2 = (u4TileAddr >> 4) << 4;
                UINT32 u4RiscData3 = (u4TileAddr <<3) & 0x7F;
                UINT32 u4Yy = ((((rTile.i4MiRowEnd - rTile.i4MiRowStart)+7)>>3)-1);
                UINT32 u4Xx = ((((rTile.i4MiColEnd - rTile.i4MiColStart)+7)>>3)-1);
                UINT32 u4RiscData4 = ((u4Yy&0x1ff)<<6) + (u4Xx&0x3f);
                
                if(u4TileNum<=8)
                {
                    VP9_PRINT_VERBOSE("//--- Tile_X = %d, Tile_Y = %d, Addr:0x%x, Size:0x%x --- //", u4TileCol, u4TileRow, u4TileAddr, u4TileSize);
                    vVP9RISCWrite_VP9_VLD(65, u4RiscData1, u4CoreId); //info sram : size + bit_cnt(init)\n", (b_size<<3) + (b_offset & 0x7f) );
                    vVP9RISCWrite_VP9_VLD(65, u4RiscData2, u4CoreId); //info sram : base\n", ((b_offset>>7)&0x01ffffff)<<4); 
                    vVP9RISCWrite_VP9_VLD(65, u4RiscData3, u4CoreId);//info sram : bit_cnt\n", b_offset & 0x7f);
                    vVP9RISCWrite_VP9_VLD(65, u4RiscData4, u4CoreId); //info sram : range(8bits),offset(8bits),err(1bit),sb_y_m1(9bits)->(='h%x),sb_x_m1(6bits)->(='h%x)\n", tmp, yy, xx); 
                }
                else
                {
                    *u4TileDram = u4RiscData1;
                    *(u4TileDram + 1) = u4RiscData2;
                    *(u4TileDram + 2) = u4RiscData3;
                    *(u4TileDram + 3) = u4RiscData4;
                    u4TileDram += 4;
                }
                
            }
            else
            {            
                VP9_PRINT_VERBOSE("//--- Tile_X = %d, Tile_Y = %d --- SKIP //", u4TileCol, u4TileRow);
                if (u4TileCol == 0)
                    u4ActualTileRows = u4ActualTileRows - 1;
            }         
        }
    }
    u4Tmp = 0x100014;
    vVP9RISCWrite_VLD(80, u4Tmp, u4CoreId);//'h%x reinit_fetch_mode... \n", tmp,tmp);
    vVP9RISCWrite_BS2(80, u4Tmp, u4CoreId);//'h%x reinit_fetch_mode... \n", tmp,tmp);

    if(u4TileNum <= 8){
        u4Tmp = 0x10111;
    }
    else{
        vVP9RISCWrite_VLD_TOP(46, PHYSICAL(prCommon->rTileBuffer.ulBufVAddr), u4CoreId);
        u4Tmp = 0x00111;
    }    
    vVP9RISCWrite_VP9_VLD(47, u4Tmp, u4CoreId);//'h%x tile_use_sram,seg_fsm_enable,tile_fsm_enable,bs_fsm_enable\n", tmp,tmp);  
}

void vVDEC_HAL_VP9_Set_UnCompressed(UINT32 u4CoreId, VP9_UNCOMPRESSED_HEADER_T* prUnCompressed)
{
    VP9_LOOP_FILTER_INFO_T* prLoopFilter = &(prUnCompressed->rLoopFilter);
    UINT32 u4Tmp1 = prUnCompressed->u4Height;
    UINT32 u4Tmp2 = prUnCompressed->u4Width;
    UINT32 u4Tmp  = (u4Tmp1<<16)+(u4Tmp2);
    UINT32 u4PP_Filter_Level;
    UINT32 u4PP_Mode_Delta;
    UINT32 u4PP_Ref_Delta;    
    UINT32 u4Idx;
    UINT32 i;    
    
    u4PP_Mode_Delta = 0;
    u4PP_Ref_Delta = 0;
    u4PP_Filter_Level  = prLoopFilter->mode_ref_delta_enabled & 0xf;
    u4PP_Filter_Level |= (prLoopFilter->filter_level & 0xff) << 16;
    u4PP_Filter_Level |= (prLoopFilter->sharpness_level & 0xff) << 8;
    
    VP9_PRINT_VERBOSE("VP9 Set Uncompressed flags to register");
    vVP9RISCWrite_VLD_TOP(28, u4Tmp, u4CoreId);
    
    u4Tmp1 = (((u4Tmp1+63)>>6)-1);//SB_Heightml
    u4Tmp2 = (((u4Tmp2+63)>>6)-1);//SB_Widthml
    u4Tmp  = (u4Tmp1<<16)+(u4Tmp2);
    vVP9RISCWrite_VLD_TOP(26, u4Tmp, u4CoreId);
    vVP9RISCWrite_VP9_VLD(41, 1, u4CoreId);//vp9_flag;
    u4Tmp = (prUnCompressed->u4AllowHighPrecisionMv << 31) 
                | (prUnCompressed->eInterpFilterType << 24) 
                | (prUnCompressed->seg.temporal_update << 10)
                | (prUnCompressed->seg.update_map << 9)
                | (prUnCompressed->seg.enabled << 8)
                | (prUnCompressed->u4ErrResilenceMode << 4)
                | (prUnCompressed->u4FrameParallelDecodingMode << 3)
                | (prUnCompressed->u4Lossless<<2)
                | (prUnCompressed->u4IntraOnly <<1)
                | (prUnCompressed->u4FrameType);
    // Set I mode    
    if(prUnCompressed->u4IntraOnly && prUnCompressed->u4FrameType != KEY_FRAME) 
   {        
		VP9_PRINT_INFO("!!! VP9 Set IntraOnly & NOT Key Settings...");
		u4Tmp &= ~(1 << 0); 
    }
    vVP9RISCWrite_VP9_VLD(42, u4Tmp, u4CoreId);

    u4Tmp1 = 0;
    for(u4Idx=0; u4Idx<=7; u4Idx++)
    {  
        u4Tmp1 = u4Tmp1 + (prUnCompressed->seg.feature_data[u4Idx][2] << (u4Idx*2));
    }
    
    for(u4Idx=0; u4Idx<=7; u4Idx++)
    {  
        u4Tmp1 = u4Tmp1 + (((prUnCompressed->seg.feature_mask[u4Idx] & (1<<2)) >> 2) << (16 + u4Idx));
    }
    
    u4Tmp2 = 0;
    for(u4Idx=0; u4Idx<=7; u4Idx++)
    {  
        u4Tmp2 = u4Tmp2 + (((prUnCompressed->seg.feature_mask[u4Idx] & (1<<3)) >> 3) << u4Idx);   
    }
    u4Tmp  = ((u4Tmp1<<8)+(u4Tmp2));
    vVP9RISCWrite_VP9_VLD(43, u4Tmp, u4CoreId);

    if (prLoopFilter->mode_ref_delta_enabled) 
    {
       for (i = 0; i < MAX_REF_LF_DELTAS_VP9; i++)
         u4PP_Ref_Delta |= ((UINT32)(prLoopFilter->ref_deltas[i] & 0xff)) << i*8;

       for (i = 0; i < MAX_MODE_LF_DELTAS_VP9; i++)
         u4PP_Mode_Delta |= ((UINT32)(prLoopFilter->mode_deltas[i] & 0xff)) << i*8;
    }

    if( u4CoreId != VP9_LAE_ID)
    {
        vVP9RISCWrite_PP(54, u4PP_Filter_Level, u4CoreId);

        if(prLoopFilter->mode_ref_delta_enabled)
        {
            vVP9RISCWrite_PP(55, u4PP_Ref_Delta, u4CoreId);
            vVP9RISCWrite_PP(56, u4PP_Mode_Delta, u4CoreId);        
        }
    
        if(prUnCompressed->seg.enabled)
        {
            vVP9RISCWrite_PP( 57, prUnCompressed->seg.u4SegCtr, u4CoreId);
            vVP9RISCWrite_PP( 58, prUnCompressed->seg.u4SegFeature_0_3, u4CoreId);
            vVP9RISCWrite_PP( 59, prUnCompressed->seg.u4SegFeature_4_7, u4CoreId);
        }
    }
}

void vVDEC_HAL_VP9_Clear_Counts_Table(UINT32 u4CoreId)
{    
    VP9_PRINT_INFO("Clear Counts: %d", u4CoreId);
    vVP9RISCWrite_VP9_VLD(106, 1, u4CoreId);
    UINT32 u4Ret = 1;
    // [vpx]
    // check bit 0
    // need to add count break, avoid infinite loop
    while( (u4Ret & 0x1) != 0)
    {
        vVP9RISCRead_VP9_VLD(106, &u4Ret, u4CoreId);
    } 
    return;

}

void vVDEC_HAL_VP9_Set_TxMode(UINT32 u4CoreId, UINT32 u4TxMode)
{
    vVP9RISCWrite_VP9_VLD(45 , u4TxMode, u4CoreId);
}

void vVDEC_HAL_VP9_Get_Counts_Table(UINT32 u4CoreId, ULONG ulWorkingBuf, FRAME_COUNTS* pCounts)
{
    int a, b, c, d, e, f;
    int ii, tx;
    //vVDEC_HAL_VP9_Count_TBL_WDMA(u4CoreId, u4WorkingBuf);
    vVDEC_HAL_VP9_Count_TBL_Read_SRAM(u4CoreId, ulWorkingBuf);
    //get counts from sram
    //UINT32* aSRAM_DATA = (UINT32*)(VIRTUAL(u4WorkingBuf));
    //UINT32* aSRAM_DATA = (UINT32*)(u4WorkingBuf);
	 UINT32* aSRAM_DATA = (UINT32*)(ulWorkingBuf); //qianqian@20150215

    // eob_branch
    ii = 0;
    for(tx = 0; tx < 4; tx ++)
        for(a = 0; a < 2; a++)
            for(b = 0; b < 2; b ++)
                for(c = 0; c < 6; c++)
                    for(d = 0; d < (c==0 ? 3:6); d++)
                    {
                        pCounts->eob_branch[tx][a][b][c][d] = aSRAM_DATA[ii];
                        if(c == 0 && d == 2)  ii += 2;
                        else if(c == 5 && d == 5 && c == 5) ii += 3;
                        else ii++;
                    }

    aSRAM_DATA += 256*4;

    // coef
    ii = 0;
    for(tx = 0; tx < 4; tx ++)
        for(a = 0; a < 2; a++)
            for(b = 0; b < 2; b ++)
                for(c = 0; c < 6; c++)
                    for(d = 0; d < (c==0 ? 3:6); d++)
                        for(e = 0; e < 4; e++)
                        {
                            pCounts->coef[tx][a][b][c][d][e] = aSRAM_DATA[ii];
                            ii ++;
                        }
                        
   aSRAM_DATA += 4*(512+16);

    // intra_inter[4][2]
    memcpy(pCounts->intra_inter, aSRAM_DATA, 4*2*4);    
    aSRAM_DATA += 2*4;

    // comp_inter[5][2]    
    memcpy(pCounts->comp_inter, aSRAM_DATA, 10*4);
    aSRAM_DATA += 10 + 2;

    // comp_ref[5][2]    
    memcpy(pCounts->comp_ref, aSRAM_DATA, 10*4);
    aSRAM_DATA += 10 + 2;
    
    // single_ref[5][2][2]
    memcpy(pCounts->single_ref, aSRAM_DATA, 20*4);
    aSRAM_DATA += 20;
    
    // inter_mode[7][4]    
    memcpy(pCounts->inter_mode, aSRAM_DATA, 7*4*4);
    aSRAM_DATA += 7*4;
    
    // y_mode[10][4]
    for(a = 0; a < 4; a++)
    {        
        memcpy(pCounts->y_mode[a], aSRAM_DATA, 10*4);
        aSRAM_DATA += 10 + 2;
    }

    // uv_mode[10][10]
    memcpy(pCounts->uv_mode, aSRAM_DATA, 10*10*4);
    aSRAM_DATA += 10*10;
    
    // partition[16][4]
    memcpy(pCounts->partition, aSRAM_DATA, 16*4*4);
    aSRAM_DATA += 16*4;
    
    // switchable_interp[4][3]
    for(a = 0; a < 4; a++)
    {
        memcpy(pCounts->switchable_interp[a], aSRAM_DATA, 3*4);
        aSRAM_DATA += 4;
    }
    
    // tx.p8x8[2][2]    
    memcpy(pCounts->tx.p8x8, aSRAM_DATA, 4*4);
    aSRAM_DATA += 4;
    
    // tx.p16x16[2][3]
    for(a = 0; a < 2; a++)
    {        
        memcpy(pCounts->tx.p16x16[a], aSRAM_DATA, 3*4);
        aSRAM_DATA += 4;
    }
    
    // tx.p32x32[2][4]
    memcpy(pCounts->tx.p32x32, aSRAM_DATA, 8*4);    
    aSRAM_DATA += 8;

    
    // skip[3][2]
    for(a = 0; a < 3; a++)
    {        
        memcpy(pCounts->skip[a], aSRAM_DATA, 2*4);
        aSRAM_DATA += 4;
    }
    
    // mv.joint[4]
    memcpy(pCounts->mv.joints, aSRAM_DATA, 4*4);
    aSRAM_DATA += 4;
    
    //mv.comps[0].sign[2]
    memcpy(pCounts->mv.comps[0].sign, aSRAM_DATA, 2*4);
    aSRAM_DATA += 2;
    
    //mv.comps[0].class0[2]
    memcpy(pCounts->mv.comps[0].class0, aSRAM_DATA, 2*4);
    aSRAM_DATA += 2;
    
    //mv.comps[0].classes[11]
    memcpy(pCounts->mv.comps[0].classes, aSRAM_DATA, 11*4);
    aSRAM_DATA += 12;
    
    //mv.comps[0].bits[10][2]
    memcpy(pCounts->mv.comps[0].bits, aSRAM_DATA, 10*2*4);
    aSRAM_DATA += 10*2 + 4;
    
    //mv.comps[0].class0_fp[2][4]
    memcpy(pCounts->mv.comps[0].class0_fp, aSRAM_DATA, 2*4*4);
    aSRAM_DATA += 2*4;
    
    //mv.comps[0].fp[4]
    memcpy(pCounts->mv.comps[0].fp, aSRAM_DATA, 4*4);
    aSRAM_DATA += 4;
    
    //mv.comps[0].class0_hp[2]
    memcpy(pCounts->mv.comps[0].class0_hp, aSRAM_DATA, 2*4);
    aSRAM_DATA +=2;
    
    //mv.comps[0].hp[2]
    memcpy(pCounts->mv.comps[0].hp, aSRAM_DATA, 2*4);
    aSRAM_DATA += 2;
    
    //mv.comps[1].sign[2]
    memcpy(pCounts->mv.comps[1].sign, aSRAM_DATA, 2*4);
    aSRAM_DATA += 2;
    
    //mv.comps[1].class0[2]
    memcpy(pCounts->mv.comps[1].class0, aSRAM_DATA, 2*4);
    aSRAM_DATA += 2;
    
    //mv.comps[1].classes[11]
    memcpy(pCounts->mv.comps[1].classes, aSRAM_DATA, 11*4);
    aSRAM_DATA += 12;
    
    //mv.comps[1].bits[10][2]    
    memcpy(pCounts->mv.comps[1].bits, aSRAM_DATA, 10*2*4);
    aSRAM_DATA += 10*2 + 4;

    //mv.comps[1].class0_fp[2][4]
    memcpy(pCounts->mv.comps[1].class0_fp, aSRAM_DATA, 2*4*4);
    aSRAM_DATA += 2*4;
    
    //mv.comps[1].fp[4]
    memcpy(pCounts->mv.comps[1].fp, aSRAM_DATA, 4*4);
    aSRAM_DATA += 4;
    
    //mv.comps[1].class0_hp[2]
    memcpy(pCounts->mv.comps[1].class0_hp, aSRAM_DATA, 2*4);
    aSRAM_DATA += 2;

    //mv.comps[1].hp[2]
    memcpy(pCounts->mv.comps[1].hp, aSRAM_DATA, 2*4);
    aSRAM_DATA += 2;
    
    //HalFlushInvalidateDCache();
}

void vVDEC_HAL_VP9_Get_Probs_Table(UINT32 u4CoreId, ULONG ulWorkingBuf, FRAME_CONTEXT* prFrmCtx)
{
    UINT32 i4Idx;    
    UINT32 i4TxSz;
    UCHAR aDATA[20*24];
    
    INT32 a, b, c, d, e, f;    
    
    //vVDEC_HAL_VP9_Prob_TBL_WDMA(u4CoreId, u4WorkingBuf);
    vVDEC_HAL_VP9_Prob_TBL_Read_SRAM(u4CoreId, ulWorkingBuf);
    //printk("---->address = %x", u4WorkingBuf);
    //UINT32* aSRAM_DATA = (UINT32*)u4WorkingBuf;
	UINT32* aSRAM_DATA = (UINT32*)ulWorkingBuf; //qianqian@20150215
    //get coef from sram
    for(i4TxSz = TX_4X4; i4TxSz < TX_SIZES; i4TxSz++)
    {
        memset(aDATA, 0, sizeof(aDATA));
        for(i4Idx = 0; i4Idx < 20*24;  i4Idx += 4)
        {
            aDATA[i4Idx]  = (*aSRAM_DATA) & 0xFF;
            aDATA[i4Idx+1] = ((*aSRAM_DATA) >> 8) & 0xFF;
            aDATA[i4Idx+2] = ((*aSRAM_DATA) >> 16) & 0xFF;
            aDATA[i4Idx+3] = ((*aSRAM_DATA) >> 24) & 0xFF;
            aSRAM_DATA ++;
        }
        
        i4Idx = 0;
        for(a = 0; a < 2; a++)
            for(b = 0; b < 2; b ++)
                for(c = 0; c < 6; c++)
                    for(d = 0; d < (c==0 ? 3:6); d++)
                        for(e = 0; e < 3; e++)
                            {
                                prFrmCtx->coef_probs[i4TxSz][a][b][c][d][e] = aDATA[i4Idx];
                                //printk("[DEBUG]: coef_probs:%d %d\n", aDATA[i4Idx], i4Idx);
                                if(c == 0 && d == 2 && e == 2)
                                    i4Idx += 12;
                                else if(c != 0 && d == 5 && e == 2)
                                    i4Idx += 3;
                                else 
                                    i4Idx++;
                            }

    }

    // initialize y_mod_prob    
    memset(aDATA, 0, sizeof(aDATA));
    for(i4Idx = 0; i4Idx < 16*4; i4Idx+=4)
    {
        aDATA[i4Idx]  = *aSRAM_DATA & 0xFF;
        aDATA[i4Idx+1] = (*aSRAM_DATA >> 8) & 0xFF;
        aDATA[i4Idx+2] = (*aSRAM_DATA >> 16) & 0xFF;
        aDATA[i4Idx+3] = (*aSRAM_DATA >> 24) & 0xFF;          
        aSRAM_DATA ++;
    }

    i4Idx = 0;
      for(a = 0; a < 4; a++)
          for(b = 0; b < 9; b ++)
          {
            prFrmCtx->y_mode_prob[a][b] = aDATA[i4Idx];
            if(b == 8) 
                i4Idx += 8;
            else 
                i4Idx++;
          }

    // initialize switchable_interp_prob
    memset(aDATA, 0, sizeof(aDATA));
    for(i4Idx = 0; i4Idx < 16*4;)
      {
          aDATA[i4Idx]  = *aSRAM_DATA & 0xFF;
          aDATA[i4Idx+1] = (*aSRAM_DATA >> 8) & 0xFF;
          aDATA[i4Idx+2] = (*aSRAM_DATA >> 16) & 0xFF;
          aDATA[i4Idx+3] = (*aSRAM_DATA >> 24) & 0xFF;          
          aSRAM_DATA ++;
          i4Idx+=4;
      }

    i4Idx = 0;
      for(a = 0; a < 4; a++)
          for(b = 0; b < 2; b ++)
          {
            prFrmCtx->switchable_interp_prob[a][b] = aDATA[i4Idx];
            if(b == 1) 
                i4Idx += 15; 
            else
                i4Idx++;
          }

    //ignore seg
    aSRAM_DATA += 8;

    // initialize comp_inter prob
    prFrmCtx->comp_inter_prob[0] = *aSRAM_DATA & 0xFF;
    prFrmCtx->comp_inter_prob[1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->comp_inter_prob[2] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->comp_inter_prob[3] = (*aSRAM_DATA >> 24) & 0xFF;
    aSRAM_DATA ++;
    prFrmCtx->comp_inter_prob[4] = *aSRAM_DATA & 0xFF;
    aSRAM_DATA += 3;
    
    // initialize comp_ref prob
    prFrmCtx->comp_ref_prob[0] = *aSRAM_DATA & 0xFF;
    prFrmCtx->comp_ref_prob[1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->comp_ref_prob[2] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->comp_ref_prob[3] = (*aSRAM_DATA >> 24) & 0xFF;    
    aSRAM_DATA ++;
    prFrmCtx->comp_ref_prob[4] = *aSRAM_DATA & 0xFF;
    aSRAM_DATA += 3;

    // initialize single_ref prob
    prFrmCtx->single_ref_prob[0][0] = *aSRAM_DATA & 0xFF;    
    prFrmCtx->single_ref_prob[0][1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->single_ref_prob[1][0] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->single_ref_prob[1][1] = (*aSRAM_DATA >> 24) & 0xFF;
    
    aSRAM_DATA ++;
    prFrmCtx->single_ref_prob[2][0] = *aSRAM_DATA & 0xFF;    
    prFrmCtx->single_ref_prob[2][1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->single_ref_prob[3][0] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->single_ref_prob[3][1] = (*aSRAM_DATA >> 24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->single_ref_prob[4][0] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->single_ref_prob[4][1] = (*aSRAM_DATA >> 8) & 0xFF; 
    aSRAM_DATA +=2;

    // initialize joINT32 prob
    prFrmCtx->nmvc.joints[0] = (*aSRAM_DATA) & 0xFF;    
    prFrmCtx->nmvc.joints[1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.joints[2] = (*aSRAM_DATA >> 16) & 0xFF;
    aSRAM_DATA += 4;

    // initialize mvd prob
     // 110     
    prFrmCtx->nmvc.comps[0].sign = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[0].classes[0] = (*aSRAM_DATA >> 8) & 0xFF;    
    prFrmCtx->nmvc.comps[0].classes[1] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[0].classes[2] = (*aSRAM_DATA >> 24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[0].classes[3] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[0].classes[4] = (*aSRAM_DATA >> 8) & 0xFF;    
    prFrmCtx->nmvc.comps[0].classes[5] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[0].classes[6] = (*aSRAM_DATA >> 24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[0].classes[7] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[0].classes[8] = (*aSRAM_DATA >> 8) & 0xFF;    
    prFrmCtx->nmvc.comps[0].classes[9] = (*aSRAM_DATA >> 16) & 0xFF;
    aSRAM_DATA += 2;

    // 111
    prFrmCtx->nmvc.comps[1].sign = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[1].classes[0] = (*aSRAM_DATA >> 8) & 0xFF;    
    prFrmCtx->nmvc.comps[1].classes[1] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[1].classes[2] = (*aSRAM_DATA >> 24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[1].classes[3] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[1].classes[4] = (*aSRAM_DATA >> 8) & 0xFF;    
    prFrmCtx->nmvc.comps[1].classes[5] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[1].classes[6] = (*aSRAM_DATA >> 24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[1].classes[7] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[1].classes[8] = (*aSRAM_DATA >> 8) & 0xFF;    
    prFrmCtx->nmvc.comps[1].classes[9] = (*aSRAM_DATA >> 16) & 0xFF;
    aSRAM_DATA += 2;

    // 112
    prFrmCtx->nmvc.comps[0].class0[0] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[0].bits[0] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.comps[0].bits[1] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[0].bits[2] = (*aSRAM_DATA >>24) & 0xFF;
    aSRAM_DATA ++;


    prFrmCtx->nmvc.comps[0].bits[3] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[0].bits[4] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.comps[0].bits[5] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[0].bits[6] = (*aSRAM_DATA >>24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[0].bits[7] = (*aSRAM_DATA >> 0) & 0xFF;
    prFrmCtx->nmvc.comps[0].bits[8] = (*aSRAM_DATA >>8) & 0xFF;
    prFrmCtx->nmvc.comps[0].bits[9] = (*aSRAM_DATA >> 16) & 0xFF;
    aSRAM_DATA += 2;

    // 113
    prFrmCtx->nmvc.comps[1].class0[0] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[1].bits[0] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.comps[1].bits[1] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[1].bits[2] = (*aSRAM_DATA >>24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[1].bits[3] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[1].bits[4] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.comps[1].bits[5] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[1].bits[6] = (*aSRAM_DATA >>24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[1].bits[7] = (*aSRAM_DATA >> 0) & 0xFF;
    prFrmCtx->nmvc.comps[1].bits[8] = (*aSRAM_DATA >>8) & 0xFF;
    prFrmCtx->nmvc.comps[1].bits[9] = (*aSRAM_DATA >> 16) & 0xFF;
    aSRAM_DATA += 2;


     // 114
    prFrmCtx->nmvc.comps[0].class0_fp[0][0] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[0].class0_fp[0][1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.comps[0].class0_fp[0][2] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[0].class0_fp[1][0] = (*aSRAM_DATA >>24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[0].class0_fp[1][1] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[0].class0_fp[1][2] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.comps[0].fp[0] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[0].fp[1] = (*aSRAM_DATA >>24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[0].fp[2] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[0].class0_hp = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.comps[0].hp = (*aSRAM_DATA >> 16) & 0xFF;
    aSRAM_DATA += 2;
    
     // 115
    prFrmCtx->nmvc.comps[1].class0_fp[0][0] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[1].class0_fp[0][1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.comps[1].class0_fp[0][2] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[1].class0_fp[1][0] = (*aSRAM_DATA >> 24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[1].class0_fp[1][1] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[1].class0_fp[1][2] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.comps[1].fp[0] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->nmvc.comps[1].fp[1] = (*aSRAM_DATA >> 24) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->nmvc.comps[1].fp[2] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->nmvc.comps[1].class0_hp = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->nmvc.comps[1].hp = (*aSRAM_DATA >> 16) & 0xFF;
    aSRAM_DATA += 2;


    // initialize uv_mod_prob
    for(i4Idx = 0; i4Idx < 10; i4Idx++)
    {
        prFrmCtx->uv_mode_prob[i4Idx][0] = (*aSRAM_DATA) & 0xFF;        
        prFrmCtx->uv_mode_prob[i4Idx][1] = (*aSRAM_DATA >> 8) & 0xFF;
        prFrmCtx->uv_mode_prob[i4Idx][2] = (*aSRAM_DATA >> 16) & 0xFF;
        prFrmCtx->uv_mode_prob[i4Idx][3] = (*aSRAM_DATA >> 24) & 0xFF;
        aSRAM_DATA ++;
        
        prFrmCtx->uv_mode_prob[i4Idx][4] = (*aSRAM_DATA) & 0xFF;        
        prFrmCtx->uv_mode_prob[i4Idx][5] = (*aSRAM_DATA >> 8) & 0xFF;
        prFrmCtx->uv_mode_prob[i4Idx][6] = (*aSRAM_DATA >> 16) & 0xFF;
        prFrmCtx->uv_mode_prob[i4Idx][7] = (*aSRAM_DATA >> 24) & 0xFF;
        aSRAM_DATA ++;
        
        prFrmCtx->uv_mode_prob[i4Idx][8] = (*aSRAM_DATA) & 0xFF;
        prFrmCtx->uv_mode_prob[i4Idx][9] = (*aSRAM_DATA >> 8) & 0xFF;
        aSRAM_DATA += 2;
    }
    aSRAM_DATA += 8;

    // initialize partition prob
    for(a = 0; a < 16; a++)
    {
        prFrmCtx->partition_prob[a][0] = (*aSRAM_DATA) & 0xFF;        
       // printk("[DEBUG]: partition_prob:%d %d\n", aDATA[i4Idx], 0);
        prFrmCtx->partition_prob[a][1] = (*aSRAM_DATA >> 8) & 0xFF;        
       // printk("[DEBUG]: partition_prob:%d %d\n", aDATA[i4Idx], 1);
        prFrmCtx->partition_prob[a][2] = (*aSRAM_DATA >> 16) & 0xFF;
        //printk("[DEBUG]: partition_prob:%d %d\n", aDATA[i4Idx], 2);
        //prFrmCtx->partition_prob[a][3] = (*aSRAM_DATA >> 24) & 0xFF;        
        aSRAM_DATA ++;
    }
    
    // initialize inter_mode prob    
    for(a = 0; a < 7; a++)
    {
        prFrmCtx->inter_mode_probs[a][0] = (*aSRAM_DATA) & 0xFF;
        prFrmCtx->inter_mode_probs[a][1] = (*aSRAM_DATA >> 8) & 0xFF;
        prFrmCtx->inter_mode_probs[a][2] = (*aSRAM_DATA >> 16) & 0xFF;         
        aSRAM_DATA ++;
    }
    
    // initialize skip prob
    prFrmCtx->skip_probs[0] = (*aSRAM_DATA) & 0xFF;
    prFrmCtx->skip_probs[1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->skip_probs[2] = (*aSRAM_DATA >> 16) & 0xFF;
    aSRAM_DATA ++;

    // initialize tx prob
    prFrmCtx->tx_probs.p8x8[0][0] = *aSRAM_DATA & 0xFF;    
    aSRAM_DATA ++;
    prFrmCtx->tx_probs.p8x8[1][0] = *aSRAM_DATA & 0xFF;
    aSRAM_DATA ++;
    
    prFrmCtx->tx_probs.p16x16[0][0] = *aSRAM_DATA & 0xFF;    
    prFrmCtx->tx_probs.p16x16[0][1] = (*aSRAM_DATA >> 8) & 0xFF;
    aSRAM_DATA ++;
    
    prFrmCtx->tx_probs.p16x16[1][0] = *aSRAM_DATA & 0xFF;    
    prFrmCtx->tx_probs.p16x16[1][1] = (*aSRAM_DATA >> 8) & 0xFF;
    aSRAM_DATA ++;
    
    prFrmCtx->tx_probs.p32x32[0][0] = *aSRAM_DATA & 0xFF;
    prFrmCtx->tx_probs.p32x32[0][1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->tx_probs.p32x32[0][2] = (*aSRAM_DATA >> 16) & 0xFF;
    aSRAM_DATA ++;

    prFrmCtx->tx_probs.p32x32[1][0] = *aSRAM_DATA & 0xFF;
    prFrmCtx->tx_probs.p32x32[1][1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->tx_probs.p32x32[1][2] = (*aSRAM_DATA >> 16) & 0xFF;
    aSRAM_DATA ++;

    // initialize intra_inter prob
    prFrmCtx->intra_inter_prob[0] = *aSRAM_DATA & 0xFF;
    prFrmCtx->intra_inter_prob[1] = (*aSRAM_DATA >> 8) & 0xFF;
    prFrmCtx->intra_inter_prob[2] = (*aSRAM_DATA >> 16) & 0xFF;
    prFrmCtx->intra_inter_prob[3] = (*aSRAM_DATA >> 24) & 0xFF;
    
    aSRAM_DATA += 2;
    
//    HalFlushInvalidateDCache();
    return;
}

void vVDEC_HAL_VP9_Set_Probs_Table(UINT32 u4CoreId, FRAME_CONTEXT* prFrmCtx)
{
    INT32 a, b, c, d, e, f;
    
    INT32 aDATA[20*24];
    INT32 i4Idx;
    INT32 aSRAM_DATA[5*24];
    INT32 i4RISC_DATA;

    // initialize coeff prob table
    //printk("============initialize coeff prob table Start ====================\n");
    vVP9RISCWrite_VP9_VLD(102, 1<<16 | 0, u4CoreId);
    vVP9RISCWrite_VP9_VLD(104, 1<<16 | 0, u4CoreId);

    INT32 i4TxSz;
    for(i4TxSz = TX_4X4; i4TxSz < TX_SIZES; i4TxSz++)
    {
        memset(aDATA, 0, sizeof(aDATA));
        i4Idx = 0;
        for(a = 0; a < 2; a++)
          for(b = 0; b < 2; b ++)
              for(c = 0; c < 6; c++)
                  for(d = 0; d < (c==0 ? 3:6); d++)
                      for(e = 0; e < 3; e++)
                      {
                        aDATA[i4Idx] = prFrmCtx->coef_probs[i4TxSz][a][b][c][d][e];                        
                        //printk("[DEBUG]: coef_probs:%d %d\n", aDATA[i4Idx], i4Idx);
                        if(c == 0 && d == 2 && e == 2)  i4Idx += 12;
                        else if(c != 0 && d == 5 && e == 2) i4Idx += 3;
                        else i4Idx++;
                      }
        
        
        for(i4Idx = 0; i4Idx < 20*24; i4Idx+=4)
          {
            aSRAM_DATA[i4Idx/4] = (aDATA[i4Idx+3] << 24) |
                (aDATA[i4Idx+2] << 16) |
                (aDATA[i4Idx+1] << 8) |
                aDATA[i4Idx];

			 
          }

        for(i4Idx = 0; i4Idx < 5*24; i4Idx ++)
        {
          if(i4Idx%5 == 4)
              vVP9RISCWrite_VP9_VLD(105, aSRAM_DATA[i4Idx], u4CoreId);
            else
              vVP9RISCWrite_VP9_VLD(103, aSRAM_DATA[i4Idx], u4CoreId);
        }

    }

	//printk("============Yi Feng Test 1 ====================\n");

    // initialize uv_mod_prob
    i4RISC_DATA = (1 << 16) | (116*4);      
    vVP9RISCWrite_VP9_VLD(102, i4RISC_DATA, u4CoreId);
    for(i4Idx = 0; i4Idx < 10; i4Idx++)
    {
        i4RISC_DATA = (prFrmCtx->uv_mode_prob[i4Idx][3] << 24) |
            (prFrmCtx->uv_mode_prob[i4Idx][2] << 16) |
            (prFrmCtx->uv_mode_prob[i4Idx][1] << 8)  |
            prFrmCtx->uv_mode_prob[i4Idx][0];

        vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
        i4RISC_DATA = (prFrmCtx->uv_mode_prob[i4Idx][7] << 24) |
            (prFrmCtx->uv_mode_prob[i4Idx][6] << 16) |
            (prFrmCtx->uv_mode_prob[i4Idx][5] << 8)  |
            prFrmCtx->uv_mode_prob[i4Idx][4];
        vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
        i4RISC_DATA = //(prFrmCtx->uv_mode_prob[i4Idx][9] << 8)  |
            prFrmCtx->uv_mode_prob[i4Idx][8];
        vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
        vVP9RISCWrite_VP9_VLD(103, 0, u4CoreId);
    }

    // initialize y_mod_prob
    i4RISC_DATA = (1 << 16) | (96*4);

    vVP9RISCWrite_VP9_VLD(102, i4RISC_DATA, u4CoreId);
    memset(aDATA, 0, sizeof(aDATA));

    //printk("============Yi Feng Test 2 ====================\n");
    i4Idx = 0;
      for(a = 0; a < 4; a++)
          for(b = 0; b < 9; b ++)
          {
            aDATA[i4Idx] = prFrmCtx->y_mode_prob[a][b];
            if(b == 8) i4Idx += 8;
            else i4Idx++;
          }
        
    for(i4Idx = 0; i4Idx < 16*4; i4Idx+=4)
      {
        i4RISC_DATA = (aDATA[i4Idx+3] << 24) | (aDATA[i4Idx+2] << 16) | (aDATA[i4Idx+1] << 8) | aDATA[i4Idx];          
        vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
      }
      
    // initialize switchable_interp_prob
    i4RISC_DATA = (1 << 16) | (100*4);

    vVP9RISCWrite_VP9_VLD(102, i4RISC_DATA, u4CoreId);
    memset(aDATA, 0, sizeof(aDATA));

    i4Idx = 0;
      for(a = 0; a < 4; a++)
          for(b = 0; b < 2; b ++)
          {
            aDATA[i4Idx] = prFrmCtx->switchable_interp_prob[a][b];
            if(b == 1) 
                i4Idx += 15; 
            else
                i4Idx++;
          }
    for(i4Idx = 0; i4Idx < 16*4; i4Idx+=4)
      {
        i4RISC_DATA = (aDATA[i4Idx+3] << 24) | (aDATA[i4Idx+2] << 16) | (aDATA[i4Idx+1] << 8) | aDATA[i4Idx];          
        vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
      }


    // initialize partition_probs
    i4RISC_DATA = (1 << 16) | 96;

    vVP9RISCWrite_VP9_VLD(104, i4RISC_DATA, u4CoreId);
    memset(aDATA, 0, sizeof(aDATA));
    //printk("============Yi Feng Test 4 ====================\n");
    i4Idx = 0;
      for(a = 0; a < 16; a++)
      {
          for(b = 0; b < 3; b ++)
          {
            aDATA[b] = prFrmCtx->partition_prob[a][b];            
            //printk("[DEBUG]: partition_prob:%d %d\n", aDATA[b], b);
          }
        i4Idx = 0;
        i4RISC_DATA = (aDATA[i4Idx+3] << 24) | (aDATA[i4Idx+2] << 16) | (aDATA[i4Idx+1] << 8) | aDATA[i4Idx];          
        vVP9RISCWrite_VP9_VLD(105, i4RISC_DATA, u4CoreId);
      }

    // initialize intra_inter prob
    i4RISC_DATA = (1 << 16) | 126;

    vVP9RISCWrite_VP9_VLD(104, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->intra_inter_prob[3] << 24) | 
        (prFrmCtx->intra_inter_prob[2] << 16) | 
        (prFrmCtx->intra_inter_prob[1] << 8) | 
        prFrmCtx->intra_inter_prob[0];
    vVP9RISCWrite_VP9_VLD(105, i4RISC_DATA, u4CoreId);

    // initialize comp_inter prob
    i4RISC_DATA = (1 << 16) | (106*4);

    vVP9RISCWrite_VP9_VLD(102, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->comp_inter_prob[3] << 24) | (prFrmCtx->comp_inter_prob[2] << 16) | (prFrmCtx->comp_inter_prob[1] << 8) | prFrmCtx->comp_inter_prob[0];
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = prFrmCtx->comp_inter_prob[4];
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);

    // initialize comp_ref prob
    i4RISC_DATA = (1 << 16) | (107*4);
    vVP9RISCWrite_VP9_VLD(102, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->comp_ref_prob[3] << 24) | (prFrmCtx->comp_ref_prob[2] << 16) | (prFrmCtx->comp_ref_prob[1] << 8) | prFrmCtx->comp_ref_prob[0];
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = prFrmCtx->comp_ref_prob[4];
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);

    // initialize single_ref prob
    i4RISC_DATA = (1 << 16) | (108*4);
    vVP9RISCWrite_VP9_VLD(102, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->single_ref_prob[1][1] << 24) | (prFrmCtx->single_ref_prob[1][0] << 16) | (prFrmCtx->single_ref_prob[0][1] << 8) | prFrmCtx->single_ref_prob[0][0];
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->single_ref_prob[3][1] << 24) | (prFrmCtx->single_ref_prob[3][0] << 16) | (prFrmCtx->single_ref_prob[2][1] << 8) | prFrmCtx->single_ref_prob[2][0];
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->single_ref_prob[4][1] << 8) | prFrmCtx->single_ref_prob[4][0];
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);

    // initialize tx prob
    i4RISC_DATA = (1 << 16) | (120);      
    vVP9RISCWrite_VP9_VLD(104, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = prFrmCtx->tx_probs.p8x8[0][0];
    vVP9RISCWrite_VP9_VLD(105, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = prFrmCtx->tx_probs.p8x8[1][0];
    vVP9RISCWrite_VP9_VLD(105, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->tx_probs.p16x16[0][1] << 8) | prFrmCtx->tx_probs.p16x16[0][0];
    vVP9RISCWrite_VP9_VLD(105, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->tx_probs.p16x16[1][1] << 8) | prFrmCtx->tx_probs.p16x16[1][0];
    vVP9RISCWrite_VP9_VLD(105, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->tx_probs.p32x32[0][2] << 16) | (prFrmCtx->tx_probs.p32x32[0][1] << 8) | prFrmCtx->tx_probs.p32x32[0][0];
    vVP9RISCWrite_VP9_VLD(105, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->tx_probs.p32x32[1][2] << 16) | (prFrmCtx->tx_probs.p32x32[1][1] << 8) | prFrmCtx->tx_probs.p32x32[1][0];
    vVP9RISCWrite_VP9_VLD(105, i4RISC_DATA, u4CoreId);

    // initialize skip prob
    i4RISC_DATA = (1 << 16) | (119);
    vVP9RISCWrite_VP9_VLD(104, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = (prFrmCtx->skip_probs[2] << 16) | (prFrmCtx->skip_probs[1] << 8) | prFrmCtx->skip_probs[0];
    vVP9RISCWrite_VP9_VLD(105, i4RISC_DATA, u4CoreId);

    // initialize inter_mode prob
    i4RISC_DATA = (1 << 16) | (112);
    vVP9RISCWrite_VP9_VLD(104, i4RISC_DATA, u4CoreId);
      for(a = 0; a < 7; a++)
      {
        i4RISC_DATA = (prFrmCtx->inter_mode_probs[a][2] << 16) | (prFrmCtx->inter_mode_probs[a][1] << 8) | prFrmCtx->inter_mode_probs[a][0];
        vVP9RISCWrite_VP9_VLD(105, i4RISC_DATA, u4CoreId);
      }

    // initialize joINT32 prob
    i4RISC_DATA = (1 << 16) | (109*4);
    vVP9RISCWrite_VP9_VLD(102, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.joints[2]) << 16) |
        ((prFrmCtx->nmvc.joints[1]) << 8)  |
        (prFrmCtx->nmvc.joints[0]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);

    // initialize mvd prob
    i4RISC_DATA = (1 << 16) | (110*4);
    vVP9RISCWrite_VP9_VLD(102, i4RISC_DATA, u4CoreId);
      // 110
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[0].classes[2]) << 24) |
        ((prFrmCtx->nmvc.comps[0].classes[1]) << 16) |
        ((prFrmCtx->nmvc.comps[0].classes[0]) << 8)  |
        (prFrmCtx->nmvc.comps[0].sign);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[0].classes[6]) << 24) |
        ((prFrmCtx->nmvc.comps[0].classes[5]) << 16) |
        ((prFrmCtx->nmvc.comps[0].classes[4]) << 8)  |
        (prFrmCtx->nmvc.comps[0].classes[3]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[0].classes[9]) << 16) |
        ((prFrmCtx->nmvc.comps[0].classes[8]) << 8)  |
        (prFrmCtx->nmvc.comps[0].classes[7]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    vVP9RISCWrite_VP9_VLD(103, 0, u4CoreId);

      // 111
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[1].classes[2]) << 24) |
        ((prFrmCtx->nmvc.comps[1].classes[1]) << 16) |
        ((prFrmCtx->nmvc.comps[1].classes[0]) << 8)  |
        (prFrmCtx->nmvc.comps[1].sign);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[1].classes[6]) << 24) |
        ((prFrmCtx->nmvc.comps[1].classes[5]) << 16) |
        ((prFrmCtx->nmvc.comps[1].classes[4]) << 8)  |
        (prFrmCtx->nmvc.comps[1].classes[3]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[1].classes[9]) << 16) |
        ((prFrmCtx->nmvc.comps[1].classes[8]) << 8)  |
        (prFrmCtx->nmvc.comps[1].classes[7]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    vVP9RISCWrite_VP9_VLD(103, 0, u4CoreId);

      // 112
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[0].bits[2]) << 24) |
        ((prFrmCtx->nmvc.comps[0].bits[1]) << 16) |
        ((prFrmCtx->nmvc.comps[0].bits[0]) << 8)  |
        ((prFrmCtx->nmvc.comps[0].class0[0]));
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[0].bits[6]) << 24) |
        ((prFrmCtx->nmvc.comps[0].bits[5]) << 16) |
        ((prFrmCtx->nmvc.comps[0].bits[4]) << 8)  |
        (prFrmCtx->nmvc.comps[0].bits[3]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[0].bits[9]) << 16) |
        ((prFrmCtx->nmvc.comps[0].bits[8]) << 8)  |
        (prFrmCtx->nmvc.comps[0].bits[7]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    vVP9RISCWrite_VP9_VLD(103, 0, u4CoreId);

      // 113
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[1].bits[2]) << 24) |
        ((prFrmCtx->nmvc.comps[1].bits[1]) << 16) |
        ((prFrmCtx->nmvc.comps[1].bits[0]) << 8)  |
        ((prFrmCtx->nmvc.comps[1].class0[0]));
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[1].bits[6]) << 24) |
        ((prFrmCtx->nmvc.comps[1].bits[5]) << 16) |
        ((prFrmCtx->nmvc.comps[1].bits[4]) << 8)  |
        (prFrmCtx->nmvc.comps[1].bits[3]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[1].bits[9]) << 16) |
        ((prFrmCtx->nmvc.comps[1].bits[8]) << 8)  |
        (prFrmCtx->nmvc.comps[1].bits[7]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    vVP9RISCWrite_VP9_VLD(103, 0, u4CoreId);

      // 114
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[0].class0_fp[1][0]) << 24) |
        ((prFrmCtx->nmvc.comps[0].class0_fp[0][2]) << 16) |
        ((prFrmCtx->nmvc.comps[0].class0_fp[0][1]) << 8)  |
        (prFrmCtx->nmvc.comps[0].class0_fp[0][0]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[0].fp[1]) << 24) |
        ((prFrmCtx->nmvc.comps[0].fp[0]) << 16) |
        ((prFrmCtx->nmvc.comps[0].class0_fp[1][2]) << 8)  |
        (prFrmCtx->nmvc.comps[0].class0_fp[1][1]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[0].hp) << 16) |
        ((prFrmCtx->nmvc.comps[0].class0_hp) << 8)  |
        (prFrmCtx->nmvc.comps[0].fp[2]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    vVP9RISCWrite_VP9_VLD(103, 0, u4CoreId);

      // 115
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[1].class0_fp[1][0]) << 24) |
        ((prFrmCtx->nmvc.comps[1].class0_fp[0][2]) << 16) |
        ((prFrmCtx->nmvc.comps[1].class0_fp[0][1]) << 8)  |
        (prFrmCtx->nmvc.comps[1].class0_fp[0][0]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[1].fp[1]) << 24) |
        ((prFrmCtx->nmvc.comps[1].fp[0]) << 16) |
        ((prFrmCtx->nmvc.comps[1].class0_fp[1][2]) << 8)  |
        (prFrmCtx->nmvc.comps[1].class0_fp[1][1]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    i4RISC_DATA = ((prFrmCtx->nmvc.comps[1].hp) << 16) |
        ((prFrmCtx->nmvc.comps[1].class0_hp) << 8)  |
        (prFrmCtx->nmvc.comps[1].fp[2]);
    vVP9RISCWrite_VP9_VLD(103, i4RISC_DATA, u4CoreId);
    vVP9RISCWrite_VP9_VLD(103, 0, u4CoreId);
    //printk("============initialize coeff prob table end====================\n");
}

void vVDEC_HAL_VP9_UPDATE_TX_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;    
    UINT32 u4Ret;
    u4ChkBit = 0;
    u4Ret = 0;
    VP9_PRINT_VERBOSE("VP9 update tx probs");

    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
        
    // [vpx]
    // check bit 0
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }

    return;
}

void vVDEC_HAL_VP9_UPDATE_COEF_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;    
    UINT32 u4Ret;
    u4ChkBit = 1;
    u4Ret = 0;    
    VP9_PRINT_VERBOSE("VP9 update coef probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);

    // [vpx]
    // check bit 1
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }
    
    return;   
}

void vVDEC_HAL_VP9_UPDATE_MBSKIP_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;    
    UINT32 u4Ret;
    u4ChkBit = 2;
    u4Ret = 0;
    
    VP9_PRINT_VERBOSE("VP9 update mbskip probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 2
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }    
    return;   
}

void vVDEC_HAL_VP9_UPDATE_INTER_MODE_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;    
    UINT32 u4Ret;
    u4ChkBit = 3;
    u4Ret = 0;
    
    VP9_PRINT_VERBOSE("VP9 update inter mode probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);
    
    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 3
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }      
    return;   
}

void vVDEC_HAL_VP9_UPDATE_SWITCHABLE_INTERP_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;    
    UINT32 u4Ret;
    u4ChkBit = 4;
    u4Ret = 0;
    
    VP9_PRINT_VERBOSE("VP9 update switchable interp mode probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);
    
    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 4
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }          
    return;   
}

void vVDEC_HAL_VP9_UPDATE_INTRA_INTER_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;    
    UINT32 u4Ret;
    u4ChkBit = 5;
    u4Ret = 0;
    
    VP9_PRINT_VERBOSE("VP9 update intra inter probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 5
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }       
    return;   
}

void vVDEC_HAL_VP9_UPDATE_SINGLE_REF_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;    
    UINT32 u4Ret;
    u4ChkBit = 7;
    u4Ret = 0;
    
    VP9_PRINT_VERBOSE("VP9 update single ref probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);
    
    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 7
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }           
    return;   
}

void vVDEC_HAL_VP9_UPDATE_Y_MODE_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;    
    UINT32 u4Ret;
    u4ChkBit = 9;
    u4Ret = 0;
    
    VP9_PRINT_VERBOSE("VP9 update y mode probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 9
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }           
    return;
}

void vVDEC_HAL_VP9_UPDATE_PARTITION_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;    
    UINT32 u4Ret;
    u4ChkBit = 10;
    u4Ret = 0;

    VP9_PRINT_VERBOSE("VP9 update partition probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 10
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }            
    return;   
}

void vVDEC_HAL_VP9_UPDATE_MVD_INT_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;    
    UINT32 u4Ret;
    u4ChkBit = 11;
    u4Ret = 0;     
    
    VP9_PRINT_VERBOSE("VP9 update mvd int probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 11
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }     
    return;   
}

void vVDEC_HAL_VP9_UPDATE_MVD_FP_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;
    UINT32 u4Ret;
    u4ChkBit = 12;
    u4Ret = 0;
    
    VP9_PRINT_VERBOSE("VP9 update mvd fp probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 12
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    }         
    return;   
}

void vVDEC_HAL_VP9_UPDATE_MVD_HP_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;
    UINT32 u4Ret;
    u4ChkBit = 13;
    u4Ret = 0;
    
    VP9_PRINT_VERBOSE("VP9 update mvd hp probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 13
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    } 
    return;   
}

void vVDEC_HAL_VP9_UPDATE_COMP_REF_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;
    UINT32 u4Ret;
    u4ChkBit = 8;
    u4Ret = 0;
    
    VP9_PRINT_VERBOSE("VP9 update comp ref probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 13
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    } 
    return;    
}

void vVDEC_HAL_VP9_Prob_TBL_Read_SRAM(UINT32 u4CoreID, ULONG u4VP9_PROB_TBL_Addr)
{
	 UINT32 i = 0;
        UINT32 k = 0;
        UINT32 a0 = 0;
        UINT32 a1 = 0;
        UINT32 u4Data = 0;
        UINT32* aSRAM_DATA = (UINT32*)u4VP9_PROB_TBL_Addr;
           
        // coefficient parts
        for(i = 0; i< 96; i++)
        {
           // read 4 data from 103
           for( k = 0; k < 4; k++)
           {
               vVP9RISCWrite_VP9_VLD(102, a0, u4CoreID);
               vVP9RISCRead_VP9_VLD(103, &u4Data, u4CoreID);
               *aSRAM_DATA = u4Data;
               aSRAM_DATA++;
               a0++;
           }       
           
           vVP9RISCWrite_VP9_VLD(104, a1, u4CoreID); 
           vVP9RISCRead_VP9_VLD(105, &u4Data, u4CoreID);
           *aSRAM_DATA = u4Data;
           aSRAM_DATA++;
           a1++;
        }
        
        for(i = 96; i< 128; i++)
        {
            for( k = 0; k < 4; k++)
            {
                vVP9RISCWrite_VP9_VLD(102, a0, u4CoreID);
                vVP9RISCRead_VP9_VLD(103, &u4Data, u4CoreID);        
                *aSRAM_DATA = u4Data;
                aSRAM_DATA++;
                a0++;
            }
        }
        
        for(i = 96; i< 128; i++)
        {
            vVP9RISCWrite_VP9_VLD(104, a1, u4CoreID);
            vVP9RISCRead_VP9_VLD(105, &u4Data, u4CoreID);        
            *aSRAM_DATA = u4Data;
            aSRAM_DATA++;
            a1++;
        }   
}

void vVDEC_HAL_VP9_Prob_TBL_WDMA(UINT32 u4CoreID, ULONG ulVP9_PROB_TBL_Addr)
{
    UINT32 i;
    UINT32 u4Ret = 0;
    UINT32 u4ChkBit;
    
    vVP9RISCRead_VP9_VLD(123, &u4Ret, u4CoreID);
    memset((void *)ulVP9_PROB_TBL_Addr, 0, VP9_PROB_TBL_SZ);
    // set vp9_ctx_count_wdma_mode
    vVP9RISCWrite_VP9_VLD(119, 1 << 1, u4CoreID);
    vVP9RISCWrite_VP9_VLD(120, PHYSICAL(ulVP9_PROB_TBL_Addr), u4CoreID);

    vVP9RISCWrite_VP9_VLD(122, (1 << 1), u4CoreID);

    u4ChkBit = 0;

    // polling
    vVP9RISCRead_VP9_VLD(123, &u4Ret, u4CoreID);

    // [vpx]
    // check bit 0
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(123, &u4Ret, u4CoreID);
    }
	
}

void vVDEC_HAL_VP9_Count_TBL_Read_SRAM(UINT32 u4CoreID, ULONG u4VP9_COUNT_TBL_Addr)
{
	UINT32 i = 0;
        UINT32 u4Data = 0;
        UINT32* aSRAM_DATA = (UINT32*)u4VP9_COUNT_TBL_Addr;
           
        // coefficient parts
        for(i = 0; i< 144*4; i++)
        {
           vVP9RISCWrite_VP9_VLD(108, i, u4CoreID); 
           vVP9RISCRead_VP9_VLD(109, &u4Data, u4CoreID);
           *aSRAM_DATA = u4Data;
           aSRAM_DATA++;
        }
        
        for(i = (144*4); i< (256*4); i++)
        {
            *aSRAM_DATA = 0;
            aSRAM_DATA++;
        }
        
        for(i = 0; i< (256*4); i++)
        {
            vVP9RISCWrite_VP9_VLD(110, i, u4CoreID); 
            vVP9RISCRead_VP9_VLD(111, &u4Data, u4CoreID);
            *aSRAM_DATA = u4Data;
            aSRAM_DATA++;
        }
        
        for(i = 0; i< (256*4); i++)
        {
            vVP9RISCWrite_VP9_VLD(112, i, u4CoreID); 
            vVP9RISCRead_VP9_VLD(113, &u4Data, u4CoreID);
            *aSRAM_DATA = u4Data;
            aSRAM_DATA++;
        }
        
        for(i = 0; i< (136*4); i++)
        {
            vVP9RISCWrite_VP9_VLD(114, i, u4CoreID); 
            vVP9RISCRead_VP9_VLD(115, &u4Data, u4CoreID);
            *aSRAM_DATA = u4Data;
            aSRAM_DATA++;
        }
        
        for(i = (136*4); i< (256*4); i++)
        {       
            *aSRAM_DATA = 0;
            aSRAM_DATA++;
        }   
	
}

void vVDEC_HAL_VP9_Count_TBL_WDMA(UINT32 u4CoreID, ULONG u4VP9_COUNT_TBL_Addr)
{
    UINT32 i;
    UINT32 u4Ret = 0;
    UINT32 u4ChkBit;
 
    memset((void*)u4VP9_COUNT_TBL_Addr, 0, VP9_COUNT_TBL_SZ);   

    // set vp9_ctx_count_wdma_mode
    vVP9RISCWrite_VP9_VLD(119, 1, u4CoreID);
    vVP9RISCWrite_VP9_VLD(120, PHYSICAL(u4VP9_COUNT_TBL_Addr), u4CoreID);

    vVP9RISCWrite_VP9_VLD(122, (1 << 0), u4CoreID);

    u4ChkBit = 0;

    // polling
    vVP9RISCRead_VP9_VLD(123, &u4Ret, u4CoreID);
    
#if 0
    //break while
    u4Ret = 0x1 << u4ChkBit;     
#endif

    // [vpx]
    // check bit 0
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(123, &u4Ret, u4CoreID);
    } 
    
}

void vVDEC_HAL_VP9_UPDATE_COMP_INTER_PROBS(UINT32 u4CoreId)
{
    UINT32 u4ChkBit;
    UINT32 u4Ret;
    u4ChkBit = 6;
    u4Ret = 0;
    
    VP9_PRINT_VERBOSE("VP9 update comp inter probs");
    vVP9RISCWrite_VP9_VLD(50, (1 << u4ChkBit), u4CoreId);
    vVP9RISCWrite_VP9_VLD(50, 0, u4CoreId);

    // polling
    vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    
    // [vpx]
    // check bit 13
    // need to add count break, avoid infinite loop
    while( ((u4Ret >> u4ChkBit) & 0x1) != 1)
    {
        vVP9RISCRead_VP9_VLD(51, &u4Ret, u4CoreId);
    } 
    return;      
}

void vVDEC_HAL_VP9_Set_Compound_Ref(UINT32 u4CoreId, UINT32 u4CompoundParam)
{
    vVP9RISCWrite_VP9_VLD(44, u4CompoundParam, u4CoreId);
}

void vVDEC_HAL_VP9_Set_MVP_Enable(UINT32 u4CoreId, BOOL fgMVP_Enable)
{    
    UINT32 u4Ret = 0;
    
    vVP9RISCRead_MV(241, &u4Ret, u4CoreId);

    if(fgMVP_Enable)
    {
        u4Ret |= 1 << 0;
        vVP9RISCWrite_MV(241, u4Ret, u4CoreId);
    }
    else
    {
        u4Ret &= ~(1 << 0);
        vVP9RISCWrite_MV(241, u4Ret, u4CoreId);    
    }
}

void vVDEC_HAL_VP9_Set_MV_Buffer_Addr(UINT32 u4CoreId, UINT32 u4MVBufVAddr)
{
    vVP9RISCWrite_MV(240, PHYSICAL(u4MVBufVAddr) >> 4 , u4CoreId);  // mv_rd_buf
    vVP9RISCWrite_MV(131, PHYSICAL(u4MVBufVAddr) >> 4 , u4CoreId);  // mv_wr_buf
}

void vVDEC_HAL_VP9_Set_MC_DecodeBuf_Addr(UINT32 u4CoreId, UINT32 u4YAddr, UINT32 u4CAddr)
{
    vVP9RISCWrite_MC(137, PHYSICAL(u4YAddr) >> 9, u4CoreId);
    vVP9RISCWrite_MC(138, PHYSICAL(u4CAddr) >> 8, u4CoreId);
}

void vVDEC_HAL_VP9_Set_MC_RefBuf_Addr(UINT32 u4CoreId, VP9_REF_BUF_T rLastFrame, VP9_REF_BUF_T rGoldenFrame, 
                                      VP9_REF_BUF_T rARF_Frame)
{
    vVP9RISCWrite_MC(842, PHYSICAL(rLastFrame.prBuf->rBufY.ulBufVAddr), u4CoreId);
    vVP9RISCWrite_MC(845, PHYSICAL(rLastFrame.prBuf->rBufC.ulBufVAddr), u4CoreId);
    vVP9RISCWrite_MC(843, PHYSICAL(rGoldenFrame.prBuf->rBufY.ulBufVAddr), u4CoreId);
    vVP9RISCWrite_MC(846, PHYSICAL(rGoldenFrame.prBuf->rBufC.ulBufVAddr), u4CoreId);
    vVP9RISCWrite_MC(844, PHYSICAL(rARF_Frame.prBuf->rBufY.ulBufVAddr), u4CoreId);
    vVP9RISCWrite_MC(847, PHYSICAL(rARF_Frame.prBuf->rBufC.ulBufVAddr), u4CoreId);
}

void vVDEC_HAL_VP9_Set_MC_Ref_Scaling_Step(UINT32 u4CoreId, INT32 i4X_Step, INT32 i4Y_Step, UINT32 u4FrameIdx)
{   
    switch(u4FrameIdx)
    {
        case 0:
            vVP9RISCWrite_MC(848,(((i4X_Step) << 16) + (i4Y_Step)), u4CoreId);
            break;
        case 1:            
            vVP9RISCWrite_MC(849,(((i4X_Step) << 16) + (i4Y_Step)), u4CoreId);
            break;
        case 2:            
            vVP9RISCWrite_MC(850,(((i4X_Step) << 16) + (i4Y_Step)), u4CoreId);
            break;
        default:
            // Error..
            break;
    }
}

void vVDEC_HAL_VP9_Set_MC_Set_Scaling_Factor(UINT32 u4CoreId, INT32 i4ScalingFactor_X, INT32 i4ScalingFactor_Y, INT32 u4FrameIdx)
{
    switch(u4FrameIdx)
    {
        case 0:
            vVP9RISCWrite_MC(854, i4ScalingFactor_X, u4CoreId);
            vVP9RISCWrite_MC(855, i4ScalingFactor_Y, u4CoreId);
            break;
        case 1:            
            vVP9RISCWrite_MC(856, i4ScalingFactor_X, u4CoreId);
            vVP9RISCWrite_MC(857, i4ScalingFactor_Y, u4CoreId);
            break;
        case 2:            
            vVP9RISCWrite_MC(858, i4ScalingFactor_X, u4CoreId);
            vVP9RISCWrite_MC(859, i4ScalingFactor_Y, u4CoreId);
            break;
        default:
            // Error..
            break;
}

}
void vVDEC_HAL_VP9_Set_MC_Set_UMV(UINT32 u4CoreId, UINT32 u4UMV_Width, UINT32 u4UMV_Height, UINT32 u4FrameIdx)
{   
    switch(u4FrameIdx)
    {
        case 0:
            vVP9RISCWrite_MC(851,(((u4UMV_Width) << 16) + (u4UMV_Height)), u4CoreId);
            break;
        case 1:            
            vVP9RISCWrite_MC(852,(((u4UMV_Width) << 16) + (u4UMV_Height)), u4CoreId);
            break;
        case 2:            
            vVP9RISCWrite_MC(853,(((u4UMV_Width) << 16) + (u4UMV_Height)), u4CoreId);
            break;
        default:
            VP9_PRINT_ERROR("Set MC UMV Exceed case");
            break;
    }
}

void vVDEC_HAL_VP9_Set_MC_MAX_RRF_BLK_Size(UINT32 u4CoreId, INT32* pMaxRefBlk_X, INT32* pMaxRefBlk_Y )
{   
    //RISCWrite_MC(863, (((%d) << 24) + ((%d) << 20) + ((%d) << 16) + ((%d) << 8) + ((%d) << 4) + (%d)));
    //max_blk_ref0_x<<24 + max_blk_ref1_x<<20 + max_blk_ref2_x<<16 + max_blk_ref0_y<<8 + max_blk_ref1_y<<4 + max_blk_ref2_y\n"
    
    vVP9RISCWrite_MC(863,(((pMaxRefBlk_X[0]) << 24) + ((pMaxRefBlk_X[1]) << 20) + ((pMaxRefBlk_X[2]) << 16) + 
                    ((pMaxRefBlk_Y[0]) << 8) + ((pMaxRefBlk_Y[1]) << 4) + (pMaxRefBlk_Y[2])), u4CoreId);
}

void vVDEC_HAL_VP9_Set_MC_Ref_Pitch(UINT32 u4CoreId, UINT32 u4UMV_Width, UINT32 u4FrameIdx)
{
    //RISCWrite_MC(860, (%d));//pitch ref 0 \n", (((((cuch_mc_pat0.cuch_mc_pat_ref_umv_width[0] + 15)/16)+3)>>2)*4));
    //RISCWrite_MC(861, (%d));//pitch ref 1 \n", (((((cuch_mc_pat0.cuch_mc_pat_ref_umv_width[1] + 15)/16)+3)>>2)*4));
    //RISCWrite_MC(862, (%d));//pitch ref 2 \n", (((((cuch_mc_pat0.cuch_mc_pat_ref_umv_width[2] + 15)/16)+3)>>2)*4));
    switch(u4FrameIdx)
    {
        case 0:
            vVP9RISCWrite_MC(860, (((((u4UMV_Width + 15)/16)+3)>>2)*4), u4CoreId);
            break;
        case 1:
            vVP9RISCWrite_MC(861, (((((u4UMV_Width + 15)/16)+3)>>2)*4), u4CoreId);
            break;
        case 2:
            vVP9RISCWrite_MC(862, (((((u4UMV_Width + 15)/16)+3)>>2)*4), u4CoreId);
            break;
        default:
            VP9_PRINT_ERROR("Set Ref Pitch Exceed case");
            break;
    }
}

void vVDEC_HAL_VP9_Set_MC_MI_COLS_ROWS(UINT32 u4CoreId, UINT32 u4MI_rows, UINT32 u4MI_cols)
{   
    //"RISCWrite_MC(866, (((%d) << 16) + (%d)));//mi_cols<<16+mi_rows for RRF use    
    vVP9RISCWrite_MC(866, (((u4MI_cols) << 16) + (u4MI_rows)), u4CoreId);
}

void vVDEC_HAL_VP9_Set_MC_Ref_Scaling_Enable(UINT32 u4CoreId, UINT32 u4Frame0_Scaling_EN, UINT32 u4Frame1_Scaling_EN, 
                                             UINT32 u4Frame2_Scaling_EN)
{   
    //RISCWrite_MC(864, (((%d) << 8) + ((%d) << 4) + (%d)));
    vVP9RISCWrite_MC(864,(((u4Frame0_Scaling_EN) << 8) + ((u4Frame1_Scaling_EN) << 4) + (u4Frame2_Scaling_EN)), u4CoreId);
}

void vVDEC_HAL_VP9_Set_PP_EN(UINT32 u4CoreId, UINT32 u4Enable)
{   
    vVP9RISCWrite_MC(136, u4Enable, u4CoreId);
}

void vVDEC_HAL_VP9_Set_PP_DBK_EN(UINT32 u4CoreId, UINT32 u4Enable)
{   
    vVP9RISCWrite_MC(142, u4Enable, u4CoreId);
}

void vVDEC_HAL_VP9_Set_PP_WriteByPost(UINT32 u4CoreId, UINT32 u4Enable)
{   
    vVP9RISCWrite_MC(148, u4Enable, u4CoreId);
}

void vVDEC_HAL_VP9_Set_PP_MB_Width(UINT32 u4CoreId, UINT32 u4Width)
{   
    UINT32 u4MBWidth;
    
    u4MBWidth = ((u4Width  + 63) >> 6) * 64;
        
    vVP9RISCWrite_MC(139, u4MBWidth >> 4, u4CoreId);
}

void vVDEC_HAL_VP9_Set_PP_MB_LeftRightMostIdx(UINT32 u4CoreId, UINT32 u4Width)
{   
    UINT32 u4MBWidth;
    
    u4MBWidth = ((u4Width  + 63) >> 6) * 64;
        
    vVP9RISCWrite_MC(152, (u4MBWidth >> 4) - 1, u4CoreId);
}

void vVDEC_HAL_VP9_Set_PP_MB_UpDownMostIdx(UINT32 u4CoreId, UINT32 u4Height)
{   
    UINT32 u4MBHeight;
    
    u4MBHeight = ((u4Height  + 63) >> 6) * 64;
        
    vVP9RISCWrite_MC(153, (u4MBHeight >> 4) - 1, u4CoreId);
}

void vVDEC_HAL_VP9_Set_PP_Display_Range(UINT32 u4CoreId, UINT32 u4Width, UINT32 u4Height)
{   
   //  fprintf(pp_risc_fp,"RISCWrite_PP( 60,  %d); //0x%x\n",(cm->frame_to_show->y_crop_height << 16) + cm->frame_to_show->y_crop_width,
   //   (cm->frame_to_show->y_crop_height << 16) + cm->frame_to_show->y_crop_width);           
    vVP9RISCWrite_PP(60, (u4Height << 16) + u4Width , u4CoreId);
}

void vVDEC_HAL_VP9_Set_SQT_IQ_SRAM_EN(UINT32 u4CoreId, UINT32 u4Enable)
{   
    //RISCWrite_PP( 321, 1); //set iq_sram_32b_risc_ctrl_en=1
    vVP9RISCWrite_PP(321, u4Enable, u4CoreId);
}

void vVDEC_HAL_VP9_Set_SQT_Q_Table(UINT32 u4CoreId, UINT32 au4Dequant[MAX_SEGMENTS][4])
{   
    UINT32 i;
    UINT32 u4IQ_SRAM_WData;
    
    for(i = 0 ; i < MAX_SEGMENTS; i++) 
    {
        // Set Y dequant
        vVP9RISCWrite_VLD(152, i * 2, u4CoreId);
        u4IQ_SRAM_WData = (au4Dequant[i][1]<<16 | au4Dequant[i][0]);
        vVP9RISCWrite_VLD(153, u4IQ_SRAM_WData, u4CoreId);

        // Set UV dequant
        vVP9RISCWrite_VLD(152, (i * 2) + 1, u4CoreId);
        u4IQ_SRAM_WData = (au4Dequant[i][3] << 16 | au4Dequant[i][2]);
        vVP9RISCWrite_VLD( 153, u4IQ_SRAM_WData, u4CoreId);
    }
}

void vVDEC_HAL_VP9_UFO_Config(UINT32 u4CoreId,UINT32 u4Width,  UINT32 u4Height, UINT32 u4PIC_SIZE_Y, UINT32 u4PIC_SIZE_C,
                              UINT32 u4Y_LEN_Addr, UINT32 u4C_LEN_Addr)
{
    UINT32 u4Pic_w_h;
    UINT32 u4AlignedWidth;
    UINT32 u4AlignedHeight;

    u4AlignedWidth = ((u4Width + 63) >> 6) << 6;    
    u4AlignedHeight = ((u4Height + 63) >> 6) << 6;
    
    u4Pic_w_h = ((((u4Width) / 16) - 1) << 16) | (((u4Height) / 16) - 1);
    
    vVP9RISCWrite_MC(700, u4Pic_w_h, u4CoreId);
    
    // enable UFO mode
    vVP9RISCWrite_MC(664, 0x11, u4CoreId);
    
    vVP9RISCWrite_MC(698, PHYSICAL(u4Y_LEN_Addr), u4CoreId);
    vVP9RISCWrite_MC(699, PHYSICAL(u4C_LEN_Addr), u4CoreId);
        
    vVP9RISCWrite_MC(825, u4PIC_SIZE_Y, u4CoreId);
    vVP9RISCWrite_MC(826, u4PIC_SIZE_C, u4CoreId);     

    // YC sep, ChunChia comment this is default on
    vVP9RISCWrite_MC(727,  0x1, u4CoreId);
}

UINT32 u4VDEC_HAL_VP9_VDec_ReadCRC_Y0(UINT32 u4Offset, UINT32 u4CoreId)
{
    UINT32 u4Ret = 0;
    
    //vVP9RISCRead_MCore_TOP(193 + u4Offset, &u4Ret, u4CoreId);
    vVP9RISCRead_MISC(95 + u4Offset, &u4Ret, u4CoreId);
    
    return u4Ret;
}

UINT32 u4VDEC_HAL_VP9_VDec_ReadCRC_Y1(UINT32 u4Offset, UINT32 u4CoreId)
{  
    UINT32 u4Ret = 0;  
    
    vVP9RISCRead_MCore_TOP(201 + u4Offset, &u4Ret, u4CoreId);
    
    return u4Ret;
}

UINT32 u4VDEC_HAL_VP9_VDec_ReadCRC_C0(UINT32 u4Offset, UINT32 u4CoreId)
{
    UINT32 u4Ret = 0;
    
    //vVP9RISCRead_MCore_TOP(197 + u4Offset, &u4Ret, u4CoreId);
    vVP9RISCRead_MISC(99 + u4Offset, &u4Ret, u4CoreId);
    
    return u4Ret;
}

UINT32 u4VDEC_HAL_VP9_VDec_ReadCRC_C1(UINT32 u4Offset, UINT32 u4CoreId)
{    
    UINT32 u4Ret = 0;
    
    vVP9RISCRead_MCore_TOP(205 + u4Offset, &u4Ret, u4CoreId);
    
    return u4Ret;
}

UINT32 u4VDEC_HAL_VP9_VDec_ReadFinishFlag(UINT32 u4CoreId)
{
    UINT32 u4Ret = 0;
    
    vVP9RISCRead_MISC(41 , &u4Ret, u4CoreId);

    return ((u4Ret >> 16) & 0x1);
}

UINT32 u4VDEC_HAL_VP9_MCORE_VDec_ReadFinishFlag(UINT32 u4CoreId)
{
    UINT32 u4Ret = 0;
        
    vVP9RISCRead_MCore_TOP(3 , &u4Ret, u4CoreId);

    return ((u4Ret >> 16) & 0x1);
}

UINT32 VP9_Wait_LAE_Decode_Done(unsigned long  start_time, UINT32 u4CoreID)
{
    UINT32 u4Ret = 0;
    UINT32 i = 0;
    
    vVP9RISCRead_MISC( 41 , &u4Ret, u4CoreID);

    #if SIM_LOG
    #else
    while ( ((u4Ret>>16) & 0x1) !=  1)
    {
        vVP9RISCRead_MISC(41, &u4Ret, u4CoreID);

        if ( ( jiffies - start_time > 3700) )
        {
            printk("Polling LAE time out!!!\n");
            return 1;
        }
    }
    #endif

    return 0;
}

void vVDEC_HAL_VP9_VDec_DumpReg(UINT32 u4FrameNum, UINT32 u4DualCore,BOOL bDecodeDone)
{
	//printk("---ignore register, vVDEC_HAL_VP9_VDec_DumpReg---\n");
	//return ;
	
    VP9_RegDump (VLD_TOP_REG_OFFSET0, 0, 128, u4FrameNum, 1);   //VLD_TOP
    VP9_RegDump (VLD_REG_OFFSET0, 33, 255,  u4FrameNum, 1);     //VLD
    VP9_RegDump (MC_REG_OFFSET0, 0, 864, u4FrameNum, 1);        //MC                    
    VP9_RegDump (VP9_VLD_REG_OFFSET0, 0, 0, u4FrameNum, 1);     //Input Window
    VP9_RegDump (VP9_VLD_REG_OFFSET0, 41, 105, u4FrameNum, 1);  //VP9 VLD
    VP9_RegDump (HEVC_MISC_REG_OFFSET0, 0, 128, u4FrameNum, 1); //VLD_TOP
    VP9_RegDump (HEVC_PP_REG_OFFSET0, 0, 896, u4FrameNum, 1);   //PP                        
    VP9_RegDump (HEVC_MV_REG_OFFSET0, 0, 254, u4FrameNum, 1);   //MV
                        
    if(u4DualCore)
    {
        VP9_RegDump (VLD_TOP_REG_OFFSET1, 0, 128, u4FrameNum, 1);   //VLD_TOP
        VP9_RegDump (VLD_REG_OFFSET1, 33, 255,  u4FrameNum, 1);     //VLD
        VP9_RegDump (MC_REG_OFFSET1, 0, 864, u4FrameNum, 1);        //MC                    
        VP9_RegDump (VP9_VLD_REG_OFFSET1, 0, 0, u4FrameNum, 1);     //Input Window
        VP9_RegDump (VP9_VLD_REG_OFFSET1, 41, 105, u4FrameNum, 1);  //VP9 VLD
        VP9_RegDump (HEVC_MISC_REG_OFFSET1, 0, 128, u4FrameNum, 1); //VLD_TOP
        VP9_RegDump (HEVC_PP_REG_OFFSET1, 0, 896, u4FrameNum, 1);   //PP
        VP9_RegDump (HEVC_MV_REG_OFFSET1, 0, 254, u4FrameNum, 1);   //MV
                        
        VP9_RegDump (LAE_VLD_OFFSET0, 33, 255,  u4FrameNum, 1);     //VLD
        VP9_RegDump (LAE_VP9_VLD_OFFSET0, 0, 0, u4FrameNum, 1);     //Input Window
        VP9_RegDump (LAE_VP9_VLD_OFFSET0, 41, 105, u4FrameNum, 1);  //VP9 VLD
        VP9_RegDump (LAE_MV_OFFSET0, 0, 254, u4FrameNum, 1);        //MV
                            
        VP9_RegDump (MVDEC_TOP_OFFSET0, 0, 127, u4FrameNum, 1);     //MCORE
    }
    
}
