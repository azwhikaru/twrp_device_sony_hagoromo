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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *    auxadc.h
 *
 * Project:
 * --------
 *   MT6573 DVT
 *
 * Description:
 * ------------
 *   This file is for Auxiliary ADC Unit.
 *
 * Author:
 * -------
 *  Myron Li
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
 
#ifndef MT8320_LDVT_VDEC_TS_H
#define MT8320_LDVT_VDEC_TS_H

#include <mach/mt_irq.h>


#define MT8320_VDEC_IRQ          (82)

#endif
struct file *openFile(char *path,int flag,int mode);
int readFileOffset(struct file *fp,char *buf,int readlen, UINT32 u4Offset );
int readFileSize(struct file *fp,char *buf,int readlen ) ;
int readFile(struct file *fp,char *buf,int readlen );
int closeFile(struct file *fp);
void initKernelEnv(void); 

