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
#include "../include/vdec_info_vp9.h"
#include "../include/vdec_info_common.h"
#include "vdec_hw_vp8.h"

#if CONFIG_DRV_VERIFY_SUPPORT
#include "../verify/vdec_verify_general.h"
#include "../verify/vdec_info_verify.h"
#include "../verify/vdec_verify_mpv_prov.h"

#if (!CONFIG_DRV_LINUX)
#include <string.h>
#include <stdio.h>
#endif

#endif

#define SIM_LOG 0

/* RISC Pattern Common Part Settings...*/
void vVP9RISCWrite_MC(UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId )
{
    vWriteReg(MC_REG_OFFSET0 + u4Addr*4, u4Value);
	#if SIM_LOG
	printk("        RISCWrite_MC(%u, %-10u, %u); // 0x%08x\n", u4Addr, u4Value, u4CoreId, u4Value);
	#endif
}

void vVP9RISCRead_MC(UINT32 u4Addr, UINT32* pu4Value , UINT32 u4CoreId)
{
    (*pu4Value) = u4ReadReg(MC_REG_OFFSET0 + u4Addr*4);
   #if SIM_LOG
   printk("        RISCRead_MC(%u, %u); // 0x%08x\n", u4Addr, u4CoreId, (*pu4Value));
   #endif
}

void vVP9RISCWrite_MV(UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId)
{
     vWriteReg(AVC_MV_REG_OFFSET0 + u4Addr*4, u4Value);
    #if SIM_LOG
	printk("        RISCWrite_MV(%u, %-10u, %u); // 0x%08x\n", u4Addr, u4Value, u4CoreId, u4Value);
    #endif
}

void vVP9RISCRead_MV(UINT32 u4Addr, UINT32* pu4Value, UINT32 u4CoreId)
{
    (*pu4Value) = u4ReadReg( AVC_MV_REG_OFFSET0+u4Addr*4 );
    #if SIM_LOG
	printk("        vVP9RISCRead_MV( %u, %u); /* return 0x%08x */\n", u4Addr, u4CoreId, (*pu4Value));
   #endif
}

void vVP9RISCWrite_PP(UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId)
{
    vWriteReg( RM_VDEC_PP_BASE+ u4Addr *4, u4Value);
    #if SIM_LOG
	printk("        RISCWrite_PP(%u, %-10u, %u); // 0x%08x\n", u4Addr, u4Value, u4CoreId, u4Value);
    #endif
}

void vVP9RISCRead_PP(UINT32 u4Addr, UINT32* pu4Value, UINT32 u4CoreId )
{
    (*pu4Value) = u4ReadReg( RM_VDEC_PP_BASE+u4Addr*4 );
     #if SIM_LOG
    printk("        RISCRead_PP(%u, %u); // 0x%08x", u4Addr, u4CoreId, (*pu4Value));
	 #endif
}

void vVP9RISCWrite_VLD_TOP(UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId)
{
    vWriteReg( VLD_TOP_REG_OFFSET0+ u4Addr*4 , u4Value);
    #if SIM_LOG
	printk("        RISCWrite_VLD_TOP(%u, %-10u, %u); // 0x%08x\n", u4Addr, u4Value, u4CoreId, u4Value);
    #endif
}

void vVP9RISCRead_VLD_TOP(UINT32 u4Addr, UINT32* pu4Value, UINT32 u4CoreId )
{
     (*pu4Value) = u4ReadReg( VLD_TOP_REG_OFFSET0+ u4Addr*4  );
    #if SIM_LOG
    printk("        RISCRead_VLD_TOP(%u, %u); // 0x%08x\n", u4Addr, u4CoreId, (*pu4Value));
    #endif
}

void vVP9RISCWrite_VLD(UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId)
{
    vWriteReg( VLD_REG_OFFSET0+ u4Addr*4 , u4Value);

    #if SIM_LOG
    	printk("        RISCWrite_VLD(%u, %-10u, %u); // 0x%08x\n", u4Addr, u4Value, u4CoreId, u4Value);
    #endif
}

void vVP9RISCRead_VLD(UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId)
{
	(*pu4Value) = u4ReadReg( VLD_REG_OFFSET0+ u4Addr*4 );
    #if SIM_LOG
    	printk("        RISCRead_VLD(%u, %u); // 0x%08x\n", u4Addr, u4CoreId, (*pu4Value));
    #endif
}

void vVP9RISCRead_MISC(UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId)
{
    (*pu4Value) = u4ReadReg( DV_REG_OFFSET0+ u4Addr*4 );
    #if SIM_LOG
	printk("        RISCRead_MISC(%u, %u); // 0x%08x\n", u4Addr, u4CoreId,(*pu4Value));
    #endif
}

void vVP9RISCWrite_MISC(UINT32 u4Addr, UINT32 u4Value , UINT32 u4CoreId)
{
    vWriteReg( DV_REG_OFFSET0+ u4Addr*4 , u4Value);
    #if SIM_LOG
	printk("        RISCWrite_MISC(%u, %-10u, %u); // 0x%08x\n", u4Addr, u4Value, u4CoreId, u4Value);
    #endif
}

void vVP9RISCRead_VDEC_TOP(UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId)
{
    (*pu4Value) = u4ReadReg(u4Addr*4 );
    #if SIM_LOG
	printk("        RISCRead_VDEC_TOP(%u, %u); // 0x%08x\n", u4Addr, u4CoreId, (*pu4Value));
   #endif
}

void vVP9RISCWrite_VDEC_TOP( UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId)
{
    vWriteReg(u4Addr*4 , u4Value);
    #if SIM_LOG
	printk ("          RISCWrite_VDEC_TOP(%u , %-10u, %u); // 0x%08x\n",u4Addr, u4Value, u4CoreId, u4Value);
    #endif
}

void vVP9RISCWrite_VP9_VLD(UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId)
{
    vWriteReg( VP9_VLD_REG_OFFSET0 +u4Addr*4 , u4Value);
    #if SIM_LOG
    printk("        RISCWrite_VP9_VLD(%u, %-10u, %u); // 0x%08x\n", u4Addr, u4Value, u4CoreId, u4Value);
   #endif
}

void vVP9RISCRead_VP9_VLD(UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId)
{
    (*pu4Value) = u4ReadReg( VP9_VLD_REG_OFFSET0+ u4Addr*4 );
    #if SIM_LOG
	printk("        RISCRead_VP9_VLD(%u, %u); // 0x%08x\n", u4Addr, u4CoreId, (*pu4Value));
    #endif
}

void vVP9RISCRead_BS2(UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId)
{
    (*pu4Value) = u4ReadReg( VDEC_BS2_OFFSET0+u4Addr*4 );
    #if SIM_LOG
	printk("        RISCRead_BS2(%u, %u); // 0x%08x\n", u4Addr, u4CoreId, (*pu4Value));
    #endif
}

void vVP9RISCWrite_BS2(UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId)
{
    vWriteReg( VDEC_BS2_OFFSET0+ u4Addr*4 , u4Value);
    #if SIM_LOG
	printk("        RISCWrite_BS2(%u, %-10u, %u); // 0x%08x\n", u4Addr, u4Value, u4CoreId, u4Value);    
    #endif
}

void vVP9RISCWrite_MCore_TOP( UINT32 u4Addr, UINT32 u4Value, UINT32 u4CoreId)
{
    vWriteReg( MVDEC_TOP_OFFSET0+ u4Addr*4 , u4Value);
    #if SIM_LOG
	printk ("          RISCWrite_MCORE_TOP(%u, %-10u, %u); // 0x%08x\n",u4Addr, u4Value, u4CoreId, u4Value);
    #endif
}

void vVP9RISCRead_MCore_TOP(UINT32 u4Addr , UINT32* pu4Value, UINT32 u4CoreId)
{
    (*pu4Value) = u4ReadReg( MVDEC_TOP_OFFSET0+ u4Addr*4 );
    #if SIM_LOG
	printk("        RISCRead_MCORE_TOP(%u, %u); // 0x%08x\n", u4Addr, u4CoreId, (*pu4Value));
    #endif
}

