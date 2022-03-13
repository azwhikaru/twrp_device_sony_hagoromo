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

#ifndef VQ_CTRL_H
#define VQ_CTRL_H

#include <linux/ioctl.h>

/* default define 0 */
#define VQ_CLK_RESET            0
#define VQ_LOOP_TEST            0
#define VQ_CLK_OFF_AUTO         0
#define VQ_MVA_MAP_VA           0
#define VQ_LOG_ALL		0
#define VQ_SLT                  0
#define VQ_CTP_TEST             0
#define VQ_4FLD_TEST            0

/* default define 1 */
#define VQ_WAIT_IRQ             1
#define VQ_WH_TIMING            1
#define VQ_TIME_CHECK		1

#if !VQ_CTP_TEST

/*#include <mach/mt_typedefs.h>*/

#else

#include "string.h"
/*#define INT8            char
#define INT16           short
#define INT32           int
#define UINT8           unsigned char
#define UINT16          unsigned short
#define UINT32          unsigned int
#define CHAR            char
#define UCHAR           unsigned char*/
#define VOID            void
#define UINT64          UINT32
#define HANDLE_T        UINT32
#define NULL_HANDLE     0
#define NULL            0

#endif

#define VQ_INVALID_DW			0xffffffff

#define VQ_INVALID_DECIMAL		99

#define VQ_RET_OK			0
#define VQ_RET_ERR_PARAM		-1
#define VQ_RET_ERR_EXCEPTION		-2

#if VQ_TIME_CHECK
#define VQ_TIME_CHECK_COUNT		10
#endif

enum VQ_FIELD_TYPE_E {
	VQ_FIELD_TYPE_TOP,
	VQ_FIELD_TYPE_BOTTOM,
	VQ_FIELD_TYPE_MAX
};

enum VQ_DI_MODE_E {
	VQ_DI_MODE_FRAME,
	VQ_DI_MODE_4_FIELD,
	VQ_DI_MODE_FIELD,
	VQ_DI_MODE_MAX
};

enum VQ_COLOR_FMT_E {
	VQ_COLOR_FMT_420BLK,
	VQ_COLOR_FMT_420SCL,
	VQ_COLOR_FMT_422BLK,
	VQ_COLOR_FMT_422SCL,
	VQ_COLOR_FMT_MAX
};

struct VQ_PARAM_T {
	/* if -1, ion address is invalid and should use mva address */
	unsigned int				u4InputAddrIonYPrev;
	unsigned int				u4InputAddrIonYCurr;
	unsigned int				u4InputAddrIonYNext;
	unsigned int				u4InputAddrIonCbcrPrev;
	unsigned int				u4InputAddrIonCbcrCurr;
	unsigned int				u4InputAddrIonCbcrNext;
	unsigned int				u4OutputAddrIonY;
	unsigned int				u4OutputAddrIonCbcr;

	unsigned int				u4InputAddrIonYSizePrev;
	unsigned int				u4InputAddrIonYSizeCurr;
	unsigned int				u4InputAddrIonYSizeNext;
	unsigned int				u4OutputAddrIonYSize;

	unsigned int				u4InputAddrMvaYPrev;
	unsigned int				u4InputAddrMvaYCurr;
	unsigned int				u4InputAddrMvaYNext;
	unsigned int				u4InputAddrMvaCbcrPrev;
	unsigned int				u4InputAddrMvaCbcrCurr;
	unsigned int				u4InputAddrMvaCbcrNext;
	unsigned int				u4OutputAddrMvaY;
	unsigned int				u4OutputAddrMvaCbcr;

	unsigned int				u4InputAddrVaYPrev;
	unsigned int				u4InputAddrVaYCurr;
	unsigned int				u4InputAddrVaYNext;
	unsigned int				u4InputAddrVaCbcrPrev;
	unsigned int				u4InputAddrVaCbcrCurr;
	unsigned int				u4InputAddrVaCbcrNext;
	unsigned int				u4OutputAddrVaY;
	unsigned int				u4OutputAddrVaCbcr;

	unsigned short				u2SrcPicWidth;
	unsigned short				u2SrcPicHeight;
	unsigned short				u2SrcFrmWidth;
	unsigned short				u2SrcFrmHeight;

	enum VQ_DI_MODE_E			eDiMode;
	enum VQ_FIELD_TYPE_E			eCurrField;
	enum VQ_COLOR_FMT_E			eSrcColorFmt;
	enum VQ_COLOR_FMT_E			eDstColorFmt;

	unsigned char				u1TopFieldFirst;

	unsigned int				u4MnrLevel;
	unsigned int				u4BnrLevel;

#if VQ_CTP_TEST
	unsigned int				u4SizeIdx;
#endif

};

extern int iVQ_PowerSwitch(int iPowerOn);
extern int iVQ_Process(struct VQ_PARAM_T *prVqParam);
extern int iVQ_CmdSetDispfmtPattern(unsigned int set, unsigned int CmdEnable, unsigned int CmdWidth);
extern int iVQ_CmdSetDiMode(
	unsigned int set, unsigned int CmdDiMode, unsigned int CmdCurrentField, unsigned int CmdTopFieldFirst);
extern int iVQ_CmdSetNrlevel(unsigned int set, unsigned int CmdMnrLevel, unsigned int CmdBnrLevel);
extern int iVQ_CmdSetColorFmt(unsigned int set, unsigned int CmdSrcColorFmt, unsigned int CmdDstColorFmt);

#endif /* VQ_CTRL_H */

