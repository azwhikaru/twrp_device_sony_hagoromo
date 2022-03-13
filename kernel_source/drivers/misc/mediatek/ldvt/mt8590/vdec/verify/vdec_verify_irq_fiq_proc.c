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
/**********************************************************************/
/***************           MTK CONFIDENTIAL            ****************/
/***************                                       ****************/
/***************   Description : MT8118 MTKPrintf      ****************/
/***************                 Procedure             ****************/
/***************                                       ****************/
/***************       Company : MediaTek Inc.         ****************/
/***************    Programmer : Ted Hu                ****************/
/**********************************************************************/
//#define _IRQ_FIQ_PROC_
#include "vdec_verify_irq_fiq_proc.h"
#include "vdec_verify_mpv_prov.h"


// *********************************************************************
// Function    : void vVldIrq0(void)
// Description : Clear picture info in frame buffer
// Parameter   : None
// Return      : None
// *********************************************************************
void vVldIrq0(void)
{
  _fgVDecComplete[0] = TRUE;
}

// *********************************************************************
// Function    : void vVldIrq1(void)
// Description : Clear picture info in frame buffer
// Parameter   : None
// Return      : None
// *********************************************************************
void vVldIrq1(void)
{
  _fgVDecComplete[1] = TRUE;
}


// *********************************************************************
// Function    : void vVDec0IrqProc(UINT16 u2Vector)
// Description : Irq Service routine.
// Parameter   : None
// Return      : None
// *********************************************************************
void vVDec0IrqProc(UINT16 u2Vector)
{
#ifndef IRQ_DISABLE    
//  BIM_ClearIrq(VECTOR_VDFUL);
  vVldIrq0();
#endif
}

// *********************************************************************
// Function    : void vVDec1IrqProc(void)
// Description : Irq Service routine.
// Parameter   : None
// Return      : None
// *********************************************************************
void vVDec1IrqProc(UINT16 u2Vector)
{
#ifndef IRQ_DISABLE    
//  BIM_ClearIrq(VECTOR_VDLIT);
  vVldIrq1();
#endif
}
