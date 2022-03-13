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
#ifndef __DRV_IF_UDVT_H
#define __DRV_IF_UDVT_H


#define FATS_WAITRESPONSE_ERR        0x00
#define FATS_WAITRESPONSE_YES        0x01
#define FATS_WAITRESPONSE_NO         0x02
#define FATS_WAITRESPONSE_TIMEOUT    0x03


typedef enum
{
	UDVT_TEST_PASS = 0,
	UDVT_TEST_FAIL,
}UDVT_TEST_RESULT_e;

extern void UDVT_IF_TestProgress(UINT32 u4Percent);
extern void UDVT_IF_SendResult(UDVT_TEST_RESULT_e TestResult);
extern UINT32 UDVT_IF_OpenFile(const char *filename,const char *mode);
extern UINT32 UDVT_IF_ReadFile(void *ptr,UINT32 size,UINT32 count,UINT32 stream);
extern UINT32 UDVT_IF_GetFileLength(UINT32 stream);
extern UINT32 UDVT_IF_CloseFile(UINT32 stream);
extern UINT32 UDVT_IF_SeekFile(UINT32 stream,INT32 offset,UINT32 origin);
extern UINT32 UDVT_IF_DebugCheckSum(void *ptr,UINT32 count);
extern UINT32 FATS_IF_WaitResponse(const char *pMsgStr,UINT32 timeout);
extern UINT32 UDVT_IF_WriteFile(void *ptr,UINT32 size,UINT32 count,UINT32 stream);
extern UINT32 FATS_IF_SaveLog(UINT32 logLvl,const char *pformat,...);

#endif // __DRV_IF_FTR_H
