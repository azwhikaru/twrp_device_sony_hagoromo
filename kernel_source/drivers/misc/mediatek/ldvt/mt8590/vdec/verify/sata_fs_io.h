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
/** @file sata_fs_io.h
 */


#ifndef SATA_FS_IO_H
#define SATA_FS_IO_H


//#include "x_common.h"
//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Configurations
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Prototype  of inter-file functions
//-----------------------------------------------------------------------------
BOOL fgHDDFsMount (UINT32 u4InstID);
BOOL fgHDDFsUnMount (UINT32 u4InstID);

BOOL fgHDDFsOpenFile(  UINT32 u4InstID,
    CHAR *strFileName,
    INT32* pi4FileId);

BOOL fgHDDFsCloseFile(  UINT32 i4FileId);
BOOL fgHDDFsReadFile(  UINT32 u4InstID,
    CHAR *strFileName,
    void* pvAddr,
    UINT32 u4Offset,
    UINT32 u4Length,
    UINT32 *pu4RealReadSize,
    UINT32 *pu4TotalFileLength,
    INT32* pi4FileId);
BOOL fgHDDFsWriteFile(CHAR *strFileName,
	void* pvAddr,
	UINT32 u4Length);

UINT32 u4HDDFsGetFileSize(INT32 *pi4FileId);
int vdecopenFile(char *path,int flag,int mode);
int vdecwriteFile(int fp,char *buf,int writelen);
int vdeccloseFile(int fp);


#endif  // SATA_FS_IO_H
