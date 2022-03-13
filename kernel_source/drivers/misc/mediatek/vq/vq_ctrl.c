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


#include "vq_def.h"
#include "vq_dispfmt_hw.h"
#include "vq_vdo_hw.h"
#include "vq_nr_hw.h"
#include "vq_wc_hw.h"

#if (VQ_ION_SUPPORT)
#include <linux/ion_drv.h>
#endif

/*********************************** define ***********************************/

#define VQ_ADDR_SWITCH(ion, mva)        ((-1 == (ion))?(mva):(ion))
#if VQ_CTP_TEST
#define VQ_ALIGN_32(x) ((x + 31) / 32 * 32)
#endif

struct VQ_ADDR_SWITCH_PARAM_T {
	unsigned char			u1IsY;
	unsigned int			u4CalcAddr;
	unsigned int			u4YAddr;
	unsigned int			u4YSize;
	unsigned int			u4Mva;
	unsigned int			u4IonFd;
	unsigned int			u4Width;
	unsigned int			u4Height;
	enum VQ_COLOR_FMT_E	    eColoFmt;
};

/*********************************** variable *********************************/
unsigned int _vq_dbg_level = 1;

unsigned int _vq_Process_count = 0;

#if VQ_MVA_MAP_VA
unsigned int _vq_set_count = 1;
#endif

#if (VQ_ION_SUPPORT)
static struct ion_client *_vq_ion_client;
#endif

static struct VQ_DISPFMT_PATTERN_T	_rVqCmdDispfmtPattern = {0};
static enum VQ_DI_MODE_E		_eVqCmdDiMode = VQ_DI_MODE_MAX;
static enum VQ_FIELD_TYPE_E		_eVqCmdCurrField = VQ_FIELD_TYPE_MAX;
static unsigned char			_u1VqCmdTopFieldFirst = VQ_INVALID_DECIMAL;
static unsigned int			_u4VqCmdMnrLevel = VQ_INVALID_DECIMAL;
static unsigned int			_u4VqCmdBnrLevel = VQ_INVALID_DECIMAL;
static enum VQ_COLOR_FMT_E		_eVqCmdSrcColorFmt = VQ_COLOR_FMT_MAX;
static enum VQ_COLOR_FMT_E		_eVqCmdDstColorFmt = VQ_COLOR_FMT_MAX;

#if VQ_TIME_CHECK
unsigned int _au4VqTimeRec[VQ_TIME_CHECK_COUNT];
#endif

#if VQ_CTP_TEST
static char *_pcVqBaseFilePath = "D:\\tablet\\8590\\vq\\";
static char *_pcVqSrcFilePathPix = "pix\\";
static char *_pcVqSrcFilePathNr = "nr\\";
static char *_pcVqSrcFilePathSize = "size\\";
static char *_pcVqDstFilePath;
static char _acVqFileInputY[256] = {0};
static char _acVqFileInputC[256] = {0};
static char _acVqFileOutputY[256] = {0};
static char _acVqFileOutputC[256] = {0};
static char _acVqFileOutputReg[256] = {0};
static char _acVqFileOutputTempY[256] = {0};
static char _acVqFileOutputTempC[256] = {0};
static char _acVqFileOutputTempReg[256] = {0};
#endif

#if VQ_SLT
static const unsigned int _u4VqStaticInputY[] = {
#include "vq_320_240_y.txt"
};
static const unsigned int _u4VqStaticInputYSize = sizeof(_u4VqStaticInputY) / sizeof(_u4VqStaticInputY[0]);

static const unsigned int _u4VqStaticInputC[] = {
#include "vq_320_240_c.txt"
};
static const unsigned int _u4VqStaticInputCSize = sizeof(_u4VqStaticInputC) / sizeof(_u4VqStaticInputC[0]);
#endif

/****************************** internal function *****************************/
#if (VQ_ION_SUPPORT)
static int iVQ_IonInit(void)
{
	if (!_vq_ion_client && g_ion_device) {

		_vq_ion_client = ion_client_create(g_ion_device, "vq");

		if (!_vq_ion_client) {

			VQ_Printf(VQ_LOG_ERROR, "[E] create ion client failed\n");
			return VQ_RET_ERR_EXCEPTION;
		}

		VQ_Printf(VQ_LOG_DEFAULT, "[D] create ion client to 0x%x\n", _vq_ion_client);
	}

	return VQ_RET_OK;
}

static int iVQ_IonToAddr(unsigned int u4IonFd, unsigned int *pu4Addr, unsigned int *pu4Size)
{
	/*struct ion_handle *handle = NULL;*/

	if (NULL == _vq_ion_client) {

		VQ_Printf(VQ_LOG_ERROR, "[E] ion client is null\n");
		return VQ_RET_ERR_EXCEPTION;
	}

	if (VQ_INVALID_DW == u4IonFd) {

		VQ_Printf(VQ_LOG_ERROR, "[E] ion fd 0x%x is invalid\n", u4IonFd);
		return VQ_RET_ERR_EXCEPTION;
	}

	/*handle = ion_import_dma_buf(_vq_ion_client, u4IonFd);
	if (NULL == handle) {

		VQ_Printf(VQ_LOG_ERROR, "ion[0x%x] to handle is null\n", u4IonFd);
		return VQ_RET_ERR_EXCEPTION;
	}*/

	ion_phys(_vq_ion_client, u4IonFd, pu4Addr, pu4Size);

	VQ_Printf(VQ_LOG_ADDRESS, "[ADDR] ion fd[0x%x] handle[0x%x] address[0x%x] size[%d]\n",
		u4IonFd, u4IonFd, pu4Addr, pu4Size);

	return VQ_RET_OK;
}
#endif

#if VQ_CTP_TEST
static int iVQ_ReadFile(char *pcFile, unsigned int u4Addr, unsigned int u4ReadSize)
{
	SFILE *ptFile = NULL;
	unsigned int  u4ResultSize;

	if (NULL == pcFile) {

		VQ_Printf(VQ_LOG_CTP, "[CTP] read file is null\n");
		VQ_ASSERT(0);
	}

	if (0 == u4Addr) {

		VQ_Printf(VQ_LOG_CTP, "[CTP] read buf addr is null\n");
		VQ_ASSERT(0);
	}

	ptFile = sfopen(pcFile, "rb");
	if (NULL == ptFile) {

		VQ_Printf(VQ_LOG_CTP, "[CTP] open file fail %s\n", pcFile);
		VQ_ASSERT(0);
	}

	u4ResultSize = sfread((char *)u4Addr, sizeof(unsigned char), u4ReadSize, ptFile);

	sfclose(ptFile);

	if (u4ResultSize != u4ReadSize) {

		VQ_Printf(VQ_LOG_CTP, "[CTP] read file fail, addr = 0x%x, ReadSize %d, ResultSize %d\n",
			u4Addr, u4ReadSize, u4ResultSize);
		VQ_ASSERT(0);
	}

	VQ_Printf(VQ_LOG_CTP, "[CTP] read file success, addr = 0x%x, ReadSize %d, %s\n", u4Addr, u4ReadSize, pcFile);

	return VQ_RET_OK;
}

static int iVQ_WriteFile(char *pcFile, unsigned int u4Addr, unsigned int u4WriteSize)
{
	SFILE *ptFile = NULL;
	unsigned int  u4ResultSize;

	if (NULL == pcFile) {

		VQ_Printf(VQ_LOG_CTP, "[CTP] write file is null\n");
		VQ_ASSERT(0);
	}

	if (0 == u4Addr) {

		VQ_Printf(VQ_LOG_CTP, "[CTP] write buf addr is null\n");
		VQ_ASSERT(0);
	}

	ptFile = sfopen(pcFile, "wb");
	if (NULL == ptFile) {

		VQ_Printf(VQ_LOG_CTP, "[CTP] open file fail %s\n", pcFile);
		VQ_ASSERT(0);
	}

	u4ResultSize = sfwrite((char *)u4Addr, sizeof(unsigned char), u4WriteSize, ptFile);

	sfclose(ptFile);

	if (u4ResultSize != u4WriteSize) {

		VQ_Printf(VQ_LOG_CTP, "[CTP] write file fail, WriteSize %d, ResSize %d\n", u4WriteSize, u4ResultSize);
		VQ_ASSERT(0);
	}

	VQ_Printf(VQ_LOG_CTP, "[CTP] write file success, WriteSize %d, %s\n", u4WriteSize, pcFile);

	return VQ_RET_OK;
}

static int iVQ_IO_Read(
	struct VQ_PARAM_T *prVqParam,
	enum VQ_TIMING_TYPE_E eTimingType,
	unsigned int u4NrEnable,
	unsigned int u4YCDivideSize,
	unsigned int *pu4ReadSize,
	unsigned int *pu4WriteSize)
{
	#if VQ_SLT
	if ((0 == u4NrEnable) || (1 != prVqParam->u4SizeIdx) || (VQ_TIMING_TYPE_480P != eTimingType)) {

		VQ_Printf(VQ_LOG_CTP, "[CTP] slt request miss, Nr = %d, SizeIdx = %d, Timing = %d\n",
			u4NrEnable, prVqParam->u4SizeIdx, eTimingType);
		VQ_ASSERT(0);
	}
	#else
	if ((u4NrEnable) &&
	    (0 == prVqParam->u4SizeIdx) && ((VQ_TIMING_TYPE_480P != eTimingType) &&
	    (VQ_TIMING_TYPE_1080P != eTimingType))) {

		VQ_Printf(VQ_LOG_CTP, "[CTP] not support timing %d when NR enable\n", eTimingType);
		VQ_ASSERT(0);
	}
	#endif

	strcpy(_acVqFileInputY, _pcVqBaseFilePath);
	strcpy(_acVqFileInputC, _pcVqBaseFilePath);

	if (0 != prVqParam->u4SizeIdx) {

		strcat(_acVqFileInputY, _pcVqSrcFilePathSize);
		strcat(_acVqFileInputC, _pcVqSrcFilePathSize);

	} else if (0 != u4NrEnable) {

		strcat(_acVqFileInputY, _pcVqSrcFilePathNr);
		strcat(_acVqFileInputC, _pcVqSrcFilePathNr);

	} else {

		strcat(_acVqFileInputY, _pcVqSrcFilePathPix);
		strcat(_acVqFileInputC, _pcVqSrcFilePathPix);
	}

	strcpy(_acVqFileOutputTempY, _pcVqBaseFilePath);
	strcat(_acVqFileOutputTempY, _pcVqDstFilePath);
	strcpy(_acVqFileOutputTempC, _pcVqBaseFilePath);
	strcat(_acVqFileOutputTempC, _pcVqDstFilePath);
	strcpy(_acVqFileOutputTempReg, _pcVqBaseFilePath);
	strcat(_acVqFileOutputTempReg, _pcVqDstFilePath);

	strcat(_acVqFileOutputTempY, "vq-");
	strcat(_acVqFileOutputTempC, "vq-");
	strcat(_acVqFileOutputTempReg, "vq-");

	if (0 != prVqParam->u4SizeIdx) {

		if (1 == prVqParam->u4SizeIdx) {

			strcat(_acVqFileInputY, "320_240-y.bin");
			strcat(_acVqFileInputC, "320_240-c.bin");

			strcat(_acVqFileOutputTempY, "320_240-");
			strcat(_acVqFileOutputTempC, "320_240-");
			strcat(_acVqFileOutputTempReg, "320_240-");

		} else if (2 == prVqParam->u4SizeIdx) {

			strcat(_acVqFileInputY, "352_240-y.bin");
			strcat(_acVqFileInputC, "352_240-c.bin");

			strcat(_acVqFileOutputTempY, "352_240-");
			strcat(_acVqFileOutputTempC, "352_240-");
			strcat(_acVqFileOutputTempReg, "352_240-");

		} else if (3 == prVqParam->u4SizeIdx) {

			strcat(_acVqFileInputY, "800_352-y.bin");
			strcat(_acVqFileInputC, "800_352-c.bin");

			strcat(_acVqFileOutputTempY, "800_352-");
			strcat(_acVqFileOutputTempC, "800_352-");
			strcat(_acVqFileOutputTempReg, "800_352-");

		} else {

		    VQ_ASSERT(0);
		}

		if (VQ_TIMING_TYPE_480P == eTimingType) {

			strcat(_acVqFileOutputTempY, "480p-");
			strcat(_acVqFileOutputTempC, "480p-");
			strcat(_acVqFileOutputTempReg, "480p-");

		} else if (VQ_TIMING_TYPE_576P == eTimingType) {

			strcat(_acVqFileOutputTempY, "576p-");
			strcat(_acVqFileOutputTempC, "576p-");
			strcat(_acVqFileOutputTempReg, "576p-");

		} else if (VQ_TIMING_TYPE_720P == eTimingType) {

			strcat(_acVqFileOutputTempY, "720p-");
			strcat(_acVqFileOutputTempC, "720p-");
			strcat(_acVqFileOutputTempReg, "720p-");

		} else if (VQ_TIMING_TYPE_1080P == eTimingType) {

			strcat(_acVqFileOutputTempY, "1080p-");
			strcat(_acVqFileOutputTempC, "1080p-");
			strcat(_acVqFileOutputTempReg, "1080p-");

		} else {

		    VQ_ASSERT(0);
		}

	} else {

		if (VQ_TIMING_TYPE_480P == eTimingType) {

			strcat(_acVqFileInputY, "480p-y.bin");
			strcat(_acVqFileInputC, "480p-c.bin");

			strcat(_acVqFileOutputTempY, "480p-");
			strcat(_acVqFileOutputTempC, "480p-");
			strcat(_acVqFileOutputTempReg, "480p-");

		} else if (VQ_TIMING_TYPE_576P == eTimingType) {

			strcat(_acVqFileInputY, "576p-y.bin");
			strcat(_acVqFileInputC, "576p-c.bin");

			strcat(_acVqFileOutputTempY, "576p-");
			strcat(_acVqFileOutputTempC, "576p-");
			strcat(_acVqFileOutputTempReg, "576p-");

		} else if (VQ_TIMING_TYPE_720P == eTimingType) {

			strcat(_acVqFileInputY, "720p-y.bin");
			strcat(_acVqFileInputC, "720p-c.bin");

			strcat(_acVqFileOutputTempY, "720p-");
			strcat(_acVqFileOutputTempC, "720p-");
			strcat(_acVqFileOutputTempReg, "720p-");

		} else if (VQ_TIMING_TYPE_1080P == eTimingType) {

			strcat(_acVqFileInputY, "1080p-y.bin");
			strcat(_acVqFileInputC, "1080p-c.bin");

			strcat(_acVqFileOutputTempY, "1080p-");
			strcat(_acVqFileOutputTempC, "1080p-");
			strcat(_acVqFileOutputTempReg, "1080p-");

		} else {

		    VQ_ASSERT(0);
		}
	}

	*pu4ReadSize = prVqParam->u2SrcPicWidth * VQ_ALIGN_32(prVqParam->u2SrcPicHeight);
	*pu4WriteSize = prVqParam->u2SrcPicWidth * prVqParam->u2SrcPicHeight;

	#if VQ_SLT
	{
		unsigned int u4Idx = 0;

		for (u4Idx = 0; u4Idx < _u4VqStaticInputYSize; u4Idx++) {

			/*for keep brace*/
			*((unsigned int *)prVqParam->u4InputAddrMvaYCurr + u4Idx)  = _u4VqStaticInputY[u4Idx];
		}

		for (u4Idx = 0; u4Idx < _u4VqStaticInputCSize; u4Idx++) {

			/*for keep brace*/
			*((unsigned int *)prVqParam->u4InputAddrMvaCbcrCurr + u4Idx)  = _u4VqStaticInputC[u4Idx];
		}
	}
	#elif VQ_4FLD_TEST
	iVQ_ReadFile("D:\\tablet\\8590\\vq\\di\\di-480p-10-Y.bin", prVqParam->u4InputAddrMvaYPrev, *pu4ReadSize);
	iVQ_ReadFile("D:\\tablet\\8590\\vq\\di\\di-480p-10-C.bin", prVqParam->u4InputAddrMvaCbcrPrev, *pu4ReadSize / 2);
	iVQ_ReadFile("D:\\tablet\\8590\\vq\\di\\di-480p-11-Y.bin", prVqParam->u4InputAddrMvaYCurr, *pu4ReadSize);
	iVQ_ReadFile("D:\\tablet\\8590\\vq\\di\\di-480p-11-C.bin", prVqParam->u4InputAddrMvaCbcrCurr, *pu4ReadSize / 2);
	iVQ_ReadFile("D:\\tablet\\8590\\vq\\di\\di-480p-12-Y.bin", prVqParam->u4InputAddrMvaYNext, *pu4ReadSize);
	iVQ_ReadFile("D:\\tablet\\8590\\vq\\di\\di-480p-12-C.bin", prVqParam->u4InputAddrMvaCbcrNext, *pu4ReadSize / 2);
	#else
	iVQ_ReadFile(_acVqFileInputY, prVqParam->u4InputAddrMvaYCurr, *pu4ReadSize);
	iVQ_ReadFile(_acVqFileInputC, prVqParam->u4InputAddrMvaCbcrCurr, *pu4ReadSize / 2);
	#endif

	memset((void *)prVqParam->u4OutputAddrMvaY, 0x68, *pu4WriteSize);
	memset((void *)prVqParam->u4OutputAddrMvaCbcr, 0x86, *pu4WriteSize / u4YCDivideSize);
	VQ_Printf(VQ_LOG_CTP, "[CTP] clr buf Ysize = %d, Csize = %d\n", *pu4WriteSize, *pu4WriteSize / u4YCDivideSize);
}

static int iVQ_IO_Write(
	unsigned int u4NrEnable,
	unsigned int u4WriteAddrY,
	unsigned int u4WriteAddrC,
	unsigned int u4WriteSize,
	unsigned int u4YCDivideSize,
	enum VQ_DI_MODE_E    eDiMode,
	enum VQ_FIELD_TYPE_E eCurrFld,
	unsigned char           u1TopFldFst)
{
	strcpy(_acVqFileOutputY, _acVqFileOutputTempY);
	strcpy(_acVqFileOutputC, _acVqFileOutputTempC);
	strcpy(_acVqFileOutputReg, _acVqFileOutputTempReg);

	if (u4NrEnable) {

		strcat(_acVqFileOutputY, "nr_on-");
		strcat(_acVqFileOutputC, "nr_on-");
		strcat(_acVqFileOutputReg, "nr_on-");

	} else {

		strcat(_acVqFileOutputY, "nr_off-");
		strcat(_acVqFileOutputC, "nr_off-");
		strcat(_acVqFileOutputReg, "nr_off-");
	}

	if (VQ_DI_MODE_FRAME == eDiMode) {

		strcat(_acVqFileOutputY, "di_frm-");
		strcat(_acVqFileOutputC, "di_frm-");
		strcat(_acVqFileOutputReg, "di_frm-");

	} else if (VQ_DI_MODE_4_FIELD == eDiMode) {

		strcat(_acVqFileOutputY, "di_4_fld-");
		strcat(_acVqFileOutputC, "di_4_fld-");
		strcat(_acVqFileOutputReg, "di_4_fld-");

	} else if (VQ_DI_MODE_FIELD == eDiMode) {

		strcat(_acVqFileOutputY, "di_fld-");
		strcat(_acVqFileOutputC, "di_fld-");
		strcat(_acVqFileOutputReg, "di_fld-");

	} else {

		VQ_Printf(VQ_LOG_ERROR, "[E] invalid DiMode %d\n", eDiMode);
		VQ_ASSERT;
	}

	if (VQ_FIELD_TYPE_TOP == eCurrFld) {

		strcat(_acVqFileOutputY, "fld_top-");
		strcat(_acVqFileOutputC, "fld_top-");
		strcat(_acVqFileOutputReg, "fld_top-");

	} else if (VQ_FIELD_TYPE_BOTTOM == eCurrFld) {

		strcat(_acVqFileOutputY, "fld_bottom-");
		strcat(_acVqFileOutputC, "fld_bottom-");
		strcat(_acVqFileOutputReg, "fld_bottom-");

	} else {

		VQ_Printf(VQ_LOG_ERROR, "[E] invalid CurrFld %d\n", eCurrFld);
		VQ_ASSERT;
	}

	if (0 != u1TopFldFst) {

		strcat(_acVqFileOutputY, "top_fst-");
		strcat(_acVqFileOutputC, "top_fst-");
		strcat(_acVqFileOutputReg, "top_fst-");

	} else {

		strcat(_acVqFileOutputY, "bottom_fst-");
		strcat(_acVqFileOutputC, "bottom_fst-");
		strcat(_acVqFileOutputReg, "bottom_fst-");
	}

	strcat(_acVqFileOutputY, "y.bin");
	strcat(_acVqFileOutputC, "c.bin");
	strcat(_acVqFileOutputReg, "reg.bin");

	#if !(VQ_SLT)
	iVQ_WriteFile(_acVqFileOutputY, u4WriteAddrY, u4WriteSize);
	iVQ_WriteFile(_acVqFileOutputC, u4WriteAddrC, u4WriteSize / u4YCDivideSize);
	iVQ_WriteFile(_acVqFileOutputReg, 0x1C000000, 0x6000);
	#endif
}
#endif

#if VQ_SLT
static int iVQ_SumCompare(
		unsigned int u4WriteAddrY,
		unsigned int u4WriteAddrC,
		unsigned int u4WriteSize,
		unsigned int u4YCDivideSize)
{
	unsigned int  u4IdxY = 0;
	unsigned int  u4IdxC = 0;
	unsigned int  u4SumY = 0;
	unsigned int  u4SumC = 0;

	unsigned int  u4GoldenSumY = 0x30F9C72E;
	unsigned int  u4GoldenSumC = 0xD2D2C000;

	for (u4IdxY = 0; u4IdxY < (u4WriteSize / 4); u4IdxY++) {

		/*for keep brace*/
		u4SumY += *((unsigned int *)u4WriteAddrY + u4IdxY);
	}

	for (u4IdxC = 0; u4IdxC < (u4WriteSize / u4YCDivideSize / 4); u4IdxC++) {

		/*for keep brace*/
		u4SumC += *((unsigned int *)u4WriteAddrC + u4IdxC);
	}

	VQ_Printf(VQ_LOG_CTP, "[CTP] SumY = 0x%x, SumC = 0x%x, [0x%x, 0x%x, %d, %d], [0x%x, 0x%x, %d, %d]\n",
		u4SumY,
		u4SumC,
		u4WriteAddrY,
		((unsigned int *)u4WriteAddrY + u4IdxY),
		u4IdxY,
		(u4WriteSize / 4),
		u4WriteAddrC,
		((unsigned int *)u4WriteAddrC + u4IdxC),
		u4IdxC,
		(u4WriteSize / u4YCDivideSize / 4));

	if ((u4GoldenSumY != u4SumY) || (u4GoldenSumC != u4SumC)) {

		VQ_Printf(VQ_LOG_CTP,
			"[CTP] sum compare fail, Sum GoldenY = 0x%x, CurrY = 0x%x, GoldenC = 0x%x, CurrC = 0x%x\n",
			u4GoldenSumY, u4SumY, u4GoldenSumC, u4SumC);

		return VQ_RET_ERR_EXCEPTION;

	} else {

		VQ_Printf(VQ_LOG_CTP,
			"[CTP] sum compare OK, Sum GoldenY = 0x%x, CurrY = 0x%x, GoldenC = 0x%x, CurrC = 0x%x\n",
			u4GoldenSumY, u4SumY, u4GoldenSumC, u4SumC);

		return VQ_RET_OK;
	}
}
#endif

static int iVQ_AddrSwitch(struct VQ_ADDR_SWITCH_PARAM_T *prParam)
{
	unsigned int u4Size = VQ_INVALID_DW;

	prParam->u4CalcAddr = VQ_INVALID_DW;

	if (VQ_INVALID_DW != prParam->u4Mva) {

		prParam->u4CalcAddr = prParam->u4Mva;

	} else if (VQ_INVALID_DW != prParam->u4IonFd) {

		#if (VQ_ION_SUPPORT)
		iVQ_IonToAddr(prParam->u4IonFd, &(prParam->u4CalcAddr), &u4Size);
		#endif

		if ((VQ_COLOR_FMT_420BLK == prParam->eColoFmt) || (VQ_COLOR_FMT_420SCL == prParam->eColoFmt)) {

			if (u4Size < (prParam->u4Width * prParam->u4Height * 3 / 2)) {

				VQ_Printf(VQ_LOG_ERROR,
					"[E] ion[0x%x] addr[0x%x] size[%d] error, W[%d] H[%d] color[%d] IsY[%d]\n",
					prParam->u4IonFd,
					prParam->u4CalcAddr,
					u4Size,
					prParam->u4Width,
					prParam->u4Height,
					prParam->eColoFmt,
					prParam->u1IsY);

				return VQ_RET_ERR_EXCEPTION;
			}

		} else if ((VQ_COLOR_FMT_422BLK == prParam->eColoFmt) || (VQ_COLOR_FMT_422SCL == prParam->eColoFmt)) {

			if (u4Size < (prParam->u4Width * prParam->u4Height * 2)) {

				VQ_Printf(VQ_LOG_ERROR,
					"[E] ion[0x%x] addr[0x%x] size[%d] error, W[%d] H[%d] color[%d] IsY[%d]\n",
					prParam->u4IonFd,
					prParam->u4CalcAddr,
					u4Size,
					prParam->u4Width,
					prParam->u4Height,
					prParam->eColoFmt,
					prParam->u1IsY);

				return VQ_RET_ERR_EXCEPTION;
			}

		} else {

			VQ_Printf(VQ_LOG_ERROR,
				"[E] invalid color[%d] when ion[0x%x] addr[0x%x] size[%d], W[%d] H[%d] IsY[%d]\n",
				prParam->eColoFmt,
				prParam->u4IonFd,
				prParam->u4CalcAddr,
				u4Size,
				prParam->u4Width,
				prParam->u4Height,
				prParam->u1IsY);

			return VQ_RET_ERR_EXCEPTION;
		}

	} else if ((0 == prParam->u1IsY) &&
		   (VQ_INVALID_DW != prParam->u4YAddr) &&
		   (VQ_INVALID_DW != prParam->u4YSize)) {

		prParam->u4CalcAddr = prParam->u4YAddr + prParam->u4YSize;

	} else {

		VQ_Printf(VQ_LOG_ERROR,
			"[E] IsY[%d], mva[0x%x], ion[0x%x], YAddr[0x%x], YSize[%d], W[%d], H[%d], color[%d]\n",
			prParam->u1IsY,
			prParam->u4Mva,
			prParam->u4IonFd,
			prParam->u4YAddr,
			prParam->u4YSize,
			prParam->u4Height,
			prParam->u4Height,
			prParam->eColoFmt);

		return VQ_RET_ERR_EXCEPTION;
	}

	return VQ_RET_OK;
}

/******************************* global function ******************************/

int iVQ_PowerSwitch(int iPowerOn)
{
	if (0 == iPowerOn) {

		/*for keep brace*/
		/*	*/

	} else {

		#if (VQ_ION_SUPPORT)
		iVQ_IonInit();
		#endif
	}

	iVQ_Vdo_Enable(iPowerOn);

	return VQ_RET_OK;
}

int iVQ_Process(struct VQ_PARAM_T *prVqParam)
{
	#if VQ_CTP_TEST
	unsigned int			u4ReadSize;
	unsigned int			u4WriteSize;
	unsigned int			u4YCDivideSize;
	#endif

	int				iRet = VQ_RET_OK;

	struct VQ_VDO_PARAM_T		rVdoParam;
	struct VQ_DISPFMT_PARAM_T	rDispfmtParam;
	struct VQ_NR_PARAM_T		rNrParam;
	struct VQ_WC_PARAM_T		rWcParam;

	enum VQ_TIMING_TYPE_E		eTimingType;

#if VQ_MVA_MAP_VA
	UINT32 u4VaInYPrevSize = 0;
	UINT32 u4VaInYCurrSize = 0;
	UINT32 u4VaInYNextSize = 0;
	UINT32 u4VaInCPrevSize = 0;
	UINT32 u4VaInCCurrSize = 0;
	UINT32 u4VaInCNextSize = 0;
	UINT32 u4VaOutYSize = 0;
	UINT32 u4VaOutCSize = 0;

	UINT32 u4MapSrcYsize = 0;
	UINT32 u4MapSrcCsize = 0;
	UINT32 u4MapDstYsize = 0;
	UINT32 u4MapDstCsize = 0;
#endif

	prVqParam->eDstColorFmt = VQ_COLOR_FMT_420BLK;

	_vq_Process_count++;

	/*  */
	if ((TIMING_SIZE_720_480_H >= prVqParam->u2SrcPicWidth) &&
	    (TIMING_SIZE_720_480_V >= prVqParam->u2SrcPicHeight)) {

		eTimingType = VQ_TIMING_TYPE_480P;

	} else if ((TIMING_SIZE_720_576_H >= prVqParam->u2SrcPicWidth) &&
		   (TIMING_SIZE_720_576_V >= prVqParam->u2SrcPicHeight)) {

		eTimingType = VQ_TIMING_TYPE_576P;

	} else if ((TIMING_SIZE_1280_720_H >= prVqParam->u2SrcPicWidth) &&
		   (TIMING_SIZE_1280_720_V >= prVqParam->u2SrcPicHeight)) {

		eTimingType = VQ_TIMING_TYPE_720P;

	} else if ((TIMING_SIZE_1920_1080_H >= prVqParam->u2SrcPicWidth) &&
		   (TIMING_SIZE_1920_1080_V >= prVqParam->u2SrcPicHeight)) {

		eTimingType = VQ_TIMING_TYPE_1080P;

	} else {

		/* error */
		VQ_Printf(VQ_LOG_ERROR, "[E] not support width = %d, height = %d\n",
			prVqParam->u2SrcPicWidth, prVqParam->u2SrcPicHeight);

		return VQ_RET_ERR_PARAM;

	}

	{
		/* cmd set */
		if (VQ_DI_MODE_MAX != _eVqCmdDiMode) {

			prVqParam->eDiMode = _eVqCmdDiMode;
			VQ_Printf(VQ_LOG_PARAM, "[P] take cmd DiMode = %d\n", _eVqCmdDiMode);
		}

		if (VQ_FIELD_TYPE_MAX != _eVqCmdCurrField) {

			prVqParam->eCurrField = _eVqCmdCurrField;
			VQ_Printf(VQ_LOG_PARAM, "[P] take cmd CurrField = %d\n", _eVqCmdCurrField);
		}

		if (VQ_INVALID_DECIMAL != _u1VqCmdTopFieldFirst) {

			prVqParam->u1TopFieldFirst = _u1VqCmdTopFieldFirst;
			VQ_Printf(VQ_LOG_PARAM, "[P] take cmd TopFieldFirst = %d\n", _u1VqCmdTopFieldFirst);
		}

		if (VQ_INVALID_DECIMAL != _u4VqCmdMnrLevel) {

			prVqParam->u4MnrLevel = _u4VqCmdMnrLevel;
			VQ_Printf(VQ_LOG_PARAM, "[P] take cmd MnrLevel = %d\n", _u4VqCmdMnrLevel);
		}

		if (VQ_INVALID_DECIMAL != _u4VqCmdBnrLevel) {

			prVqParam->u4BnrLevel = _u4VqCmdBnrLevel;
			VQ_Printf(VQ_LOG_PARAM, "[P] take cmd BnrLevel = %d\n", _u4VqCmdBnrLevel);
		}

		if (VQ_COLOR_FMT_MAX != _eVqCmdSrcColorFmt) {

			prVqParam->eSrcColorFmt = _eVqCmdSrcColorFmt;
			VQ_Printf(VQ_LOG_PARAM, "[P] take cmd SrcColorFmt = %d\n", _eVqCmdSrcColorFmt);
		}

		if (VQ_COLOR_FMT_MAX != _eVqCmdDstColorFmt) {

			prVqParam->eDstColorFmt = _eVqCmdDstColorFmt;
			VQ_Printf(VQ_LOG_PARAM, "[P] take cmd DstColorFmt = %d\n", _eVqCmdDstColorFmt);
		}
	}

	if (VQ_COLOR_FMT_420BLK != prVqParam->eSrcColorFmt) {

		VQ_Printf(VQ_LOG_ERROR, "[E] not support src color format\n", prVqParam->eSrcColorFmt);
		return VQ_RET_ERR_PARAM;
	}

	if (VQ_COLOR_FMT_422SCL < prVqParam->eDstColorFmt) {

		VQ_Printf(VQ_LOG_ERROR, "[E] not support dst color format\n", prVqParam->eDstColorFmt);
		return VQ_RET_ERR_PARAM;
	}

	VQ_Printf(VQ_LOG_PARAM,
		"[PARAM] cnt[%d], Timing[%d], Pic[%d, %d], Frm[%d, %d], DI[%d, %d, %d], Col[%d, %d], Nr[%d, %d]\n",
		_vq_Process_count,
		eTimingType,
		prVqParam->u2SrcPicWidth,
		prVqParam->u2SrcPicHeight,
		prVqParam->u2SrcFrmWidth,
		prVqParam->u2SrcFrmHeight,
		prVqParam->eDiMode,
		prVqParam->eCurrField,
		prVqParam->u1TopFieldFirst,
		prVqParam->eSrcColorFmt,
		prVqParam->eDstColorFmt,
		prVqParam->u4MnrLevel,
		prVqParam->u4BnrLevel);

	VQ_Printf(VQ_LOG_ADDRESS,
		"[ADDR] ion[0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x]\n",
		prVqParam->u4InputAddrIonYPrev,
		prVqParam->u4InputAddrIonYCurr,
		prVqParam->u4InputAddrIonYNext,
		prVqParam->u4InputAddrIonCbcrPrev,
		prVqParam->u4InputAddrIonCbcrCurr,
		prVqParam->u4InputAddrIonCbcrNext,
		prVqParam->u4OutputAddrIonY,
		prVqParam->u4OutputAddrIonCbcr,
		prVqParam->u4InputAddrIonYSizePrev,
		prVqParam->u4InputAddrIonYSizeCurr,
		prVqParam->u4InputAddrIonYSizeNext,
		prVqParam->u4OutputAddrIonYSize);

	VQ_Printf(VQ_LOG_ADDRESS,
		"[ADDR] mva[0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x]\n",
		prVqParam->u4InputAddrMvaYPrev,
		prVqParam->u4InputAddrMvaYCurr,
		prVqParam->u4InputAddrMvaYNext,
		prVqParam->u4InputAddrMvaCbcrPrev,
		prVqParam->u4InputAddrMvaCbcrCurr,
		prVqParam->u4InputAddrMvaCbcrNext,
		prVqParam->u4OutputAddrMvaY,
		prVqParam->u4OutputAddrMvaCbcr);

	VQ_Printf(VQ_LOG_ADDRESS,
		"[ADDR] va[0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x]\n",
		prVqParam->u4InputAddrVaYPrev,
		prVqParam->u4InputAddrVaYCurr,
		prVqParam->u4InputAddrVaYNext,
		prVqParam->u4InputAddrVaCbcrPrev,
		prVqParam->u4InputAddrVaCbcrCurr,
		prVqParam->u4InputAddrVaCbcrNext,
		prVqParam->u4OutputAddrVaY,
		prVqParam->u4OutputAddrVaCbcr);

	#if VQ_TIME_CHECK
	VQ_TIME_REC(1);
	#endif

	#if (VQ_ION_SUPPORT)
	iVQ_IonInit();
	#endif

	/* clock on */
	iVQ_Dispfmt_ClkOn();

	#if VQ_TIME_CHECK
	VQ_TIME_REC(2);
	#endif

	/* config dispfmt */
	rDispfmtParam.eTimingType = eTimingType;
	rDispfmtParam.u4Width = prVqParam->u2SrcPicWidth;
	rDispfmtParam.u4Height = prVqParam->u2SrcPicHeight;
	rDispfmtParam.u1NrEnable = (((0 == prVqParam->u4MnrLevel) && (0 == prVqParam->u4BnrLevel))?0:1);
	rDispfmtParam.rPattern.u1Enable = _rVqCmdDispfmtPattern.u1Enable;
	rDispfmtParam.rPattern.u2Width = _rVqCmdDispfmtPattern.u2Width;
	rDispfmtParam.rPattern.u1422_444 = _rVqCmdDispfmtPattern.u1422_444;
	rDispfmtParam.rPattern.u2Type = _rVqCmdDispfmtPattern.u2Type;

	iVQ_Dispfmt_SetParam(&rDispfmtParam);

	#if VQ_TIME_CHECK
	VQ_TIME_REC(3);
	#endif

	/* config vdo */
	switch (prVqParam->eDiMode) {
	case VQ_DI_MODE_FRAME:
		rVdoParam.eDiMode = VDO_FRAME_MODE;
		break;
	case VQ_DI_MODE_4_FIELD:
		rVdoParam.eDiMode = VDO_FUSION_MODE;
		break;
	case VQ_DI_MODE_FIELD:
		rVdoParam.eDiMode = VDO_FIELD_MODE;
		break;
	default:
		VQ_Printf(VQ_LOG_ERROR, "[E] map di mode fail, input %d\n", prVqParam->eDiMode);
		break;
	}

	VQ_Printf(VQ_LOG_FLOW, "[F] map DiMode %d to vdo mode %d\n", prVqParam->eDiMode, rVdoParam.eDiMode);

	rVdoParam.eSrcFmt = prVqParam->eSrcColorFmt;
	rVdoParam.u1CurrentIsTop = (VQ_FIELD_TYPE_TOP == prVqParam->eCurrField)?1:0;
	rVdoParam.u1EnablePreSharp = 0;
	rVdoParam.u1EnableVerticalChromaDetect = 0;
	rVdoParam.u1TopFieldFirst = prVqParam->u1TopFieldFirst;
	rVdoParam.u4Width = prVqParam->u2SrcPicWidth;
	rVdoParam.u4Height = prVqParam->u2SrcPicHeight;

	{
		struct VQ_ADDR_SWITCH_PARAM_T  rParam;

		rParam.u4YAddr = VQ_INVALID_DW;
		rParam.u4YSize = VQ_INVALID_DW;
		rParam.u4Mva = prVqParam->u4InputAddrMvaYPrev;
		rParam.u4IonFd = prVqParam->u4InputAddrIonYPrev;
		rParam.u4Width = prVqParam->u2SrcFrmWidth;
		rParam.u4Height = prVqParam->u2SrcFrmHeight;
		rParam.eColoFmt = prVqParam->eSrcColorFmt;
		rParam.u1IsY = 1;
		iVQ_AddrSwitch(&rParam);
		rVdoParam.u4AddrYPrev = rParam.u4CalcAddr;

		rParam.u4YAddr = VQ_INVALID_DW;
		rParam.u4YSize = VQ_INVALID_DW;
		rParam.u4Mva = prVqParam->u4InputAddrMvaYCurr;
		rParam.u4IonFd = prVqParam->u4InputAddrIonYCurr;
		rParam.u4Width = prVqParam->u2SrcFrmWidth;
		rParam.u4Height = prVqParam->u2SrcFrmHeight;
		rParam.eColoFmt = prVqParam->eSrcColorFmt;
		rParam.u1IsY = 1;
		iVQ_AddrSwitch(&rParam);
		rVdoParam.u4AddrYCurr = rParam.u4CalcAddr;

		rParam.u4YAddr = VQ_INVALID_DW;
		rParam.u4YSize = VQ_INVALID_DW;
		rParam.u4Mva = prVqParam->u4InputAddrMvaYNext;
		rParam.u4IonFd = prVqParam->u4InputAddrIonYNext;
		rParam.u4Width = prVqParam->u2SrcFrmWidth;
		rParam.u4Height = prVqParam->u2SrcFrmHeight;
		rParam.eColoFmt = prVqParam->eSrcColorFmt;
		rParam.u1IsY = 1;
		iVQ_AddrSwitch(&rParam);
		rVdoParam.u4AddrYNext = rParam.u4CalcAddr;

		rParam.u4YAddr = rVdoParam.u4AddrYPrev;
		rParam.u4YSize = prVqParam->u4InputAddrIonYSizePrev;
		rParam.u4Mva = prVqParam->u4InputAddrMvaCbcrPrev;
		rParam.u4IonFd = prVqParam->u4InputAddrIonCbcrPrev;
		rParam.u4Width = prVqParam->u2SrcFrmWidth;
		rParam.u4Height = prVqParam->u2SrcFrmHeight;
		rParam.eColoFmt = prVqParam->eSrcColorFmt;
		rParam.u1IsY = 0;
		iVQ_AddrSwitch(&rParam);
		rVdoParam.u4AddrCbcrPrev = rParam.u4CalcAddr;

		rParam.u4YAddr = rVdoParam.u4AddrYCurr;
		rParam.u4YSize = prVqParam->u4InputAddrIonYSizeCurr;
		rParam.u4Mva = prVqParam->u4InputAddrMvaCbcrCurr;
		rParam.u4IonFd = prVqParam->u4InputAddrIonCbcrCurr;
		rParam.u4Width = prVqParam->u2SrcFrmWidth;
		rParam.u4Height = prVqParam->u2SrcFrmHeight;
		rParam.eColoFmt = prVqParam->eSrcColorFmt;
		rParam.u1IsY = 0;
		iVQ_AddrSwitch(&rParam);
		rVdoParam.u4AddrCbcrCurr = rParam.u4CalcAddr;

		rParam.u4YAddr = rVdoParam.u4AddrYNext;
		rParam.u4YSize = prVqParam->u4InputAddrIonYSizeNext;
		rParam.u4Mva = prVqParam->u4InputAddrMvaCbcrNext;
		rParam.u4IonFd = prVqParam->u4InputAddrIonCbcrNext;
		rParam.u4Width = prVqParam->u2SrcFrmWidth;
		rParam.u4Height = prVqParam->u2SrcFrmHeight;
		rParam.eColoFmt = prVqParam->eSrcColorFmt;
		rParam.u1IsY = 0;
		iVQ_AddrSwitch(&rParam);
		rVdoParam.u4AddrCbcrNext = rParam.u4CalcAddr;
	}

	iVQ_Vdo_SetParam(&rVdoParam);

	#if VQ_TIME_CHECK
	VQ_TIME_REC(4);
	#endif

	/* config nr */
	rNrParam.u4BnrLevel = prVqParam->u4BnrLevel;
	rNrParam.u4MnrLevel = prVqParam->u4MnrLevel;

	iVQ_Nr_SetParam(&rNrParam);

	#if VQ_TIME_CHECK
	VQ_TIME_REC(5);
	#endif

	/* config write channel */
	rWcParam.eTimingType = eTimingType;
	rWcParam.eDstColorFmt = prVqParam->eDstColorFmt;
	rWcParam.u4Width = prVqParam->u2SrcPicWidth;
	rWcParam.u4Height = prVqParam->u2SrcPicHeight;

	{
		struct VQ_ADDR_SWITCH_PARAM_T  rParam;

		rParam.u4YAddr = VQ_INVALID_DW;
		rParam.u4YSize = VQ_INVALID_DW;
		rParam.u4Mva = prVqParam->u4OutputAddrMvaY;
		rParam.u4IonFd = prVqParam->u4OutputAddrIonY;
		rParam.u4Width = prVqParam->u2SrcFrmWidth;
		rParam.u4Height = prVqParam->u2SrcFrmHeight;
		rParam.eColoFmt = prVqParam->eDstColorFmt;
		rParam.u1IsY = 1;
		iVQ_AddrSwitch(&rParam);
		rWcParam.u4OutputAddrY = rParam.u4CalcAddr;

		rParam.u4YAddr = rWcParam.u4OutputAddrY;
		rParam.u4YSize = prVqParam->u4OutputAddrIonYSize;
		rParam.u4Mva = prVqParam->u4OutputAddrMvaCbcr;
		rParam.u4IonFd = prVqParam->u4OutputAddrIonCbcr;
		rParam.u4Width = prVqParam->u2SrcFrmWidth;
		rParam.u4Height = prVqParam->u2SrcFrmHeight;
		rParam.eColoFmt = prVqParam->eDstColorFmt;
		rParam.u1IsY = 0;
		iVQ_AddrSwitch(&rParam);
		rWcParam.u4OutputAddrCbcr = rParam.u4CalcAddr;
	}

	iVQ_WC_SetParam(&rWcParam);

	#if VQ_MVA_MAP_VA
	{
		u4MapSrcYsize = prVqParam->u2SrcFrmWidth * prVqParam->u2SrcFrmHeight;

		if ((VQ_COLOR_FMT_422BLK == prVqParam->eSrcColorFmt) ||
		    (VQ_COLOR_FMT_422SCL == prVqParam->eSrcColorFmt)) {

			u4MapSrcCsize = prVqParam->u2SrcFrmWidth * prVqParam->u2SrcFrmHeight;

		} else {

			u4MapSrcCsize = prVqParam->u2SrcFrmWidth * prVqParam->u2SrcFrmHeight / 2;
		}

		u4MapDstYsize = prVqParam->u2SrcFrmWidth * prVqParam->u2SrcFrmHeight;

		if ((VQ_COLOR_FMT_422BLK == prVqParam->eDstColorFmt) ||
		    (VQ_COLOR_FMT_422SCL == prVqParam->eDstColorFmt)) {

			u4MapDstCsize = prVqParam->u2SrcFrmWidth * prVqParam->u2SrcFrmHeight;

		} else {

			u4MapDstCsize = prVqParam->u2SrcFrmWidth * prVqParam->u2SrcFrmHeight / 2;
		}

		m4u_mva_map_kernel(rVdoParam.u4AddrYPrev, u4MapSrcYsize, 0,
				   &(prVqParam->u4InputAddrVaYPrev), &u4VaInYPrevSize);

		m4u_mva_map_kernel(rVdoParam.u4AddrYCurr, u4MapSrcYsize, 0,
				   &(prVqParam->u4InputAddrVaYCurr), &u4VaInYCurrSize);

		m4u_mva_map_kernel(rVdoParam.u4AddrYNext, u4MapSrcYsize, 0,
				   &(prVqParam->u4InputAddrVaYNext), &u4VaInYNextSize);

		m4u_mva_map_kernel(rVdoParam.u4AddrCbcrPrev, u4MapSrcCsize, 0,
				   &(prVqParam->u4InputAddrVaCbcrPrev), &u4VaInCPrevSize);

		m4u_mva_map_kernel(rVdoParam.u4AddrCbcrCurr, u4MapSrcCsize, 0,
				   &(prVqParam->u4InputAddrVaCbcrCurr), &u4VaInCCurrSize);

		m4u_mva_map_kernel(rVdoParam.u4AddrCbcrNext, u4MapSrcCsize, 0,
				   &(prVqParam->u4InputAddrVaCbcrNext), &u4VaInCNextSize);

		m4u_mva_map_kernel(rWcParam.u4OutputAddrY, u4MapDstYsize, 0,
				   &(prVqParam->u4OutputAddrVaY), &u4VaOutYSize);

		m4u_mva_map_kernel(rWcParam.u4OutputAddrCbcr, u4MapDstCsize, 0,
				   &(prVqParam->u4OutputAddrVaCbcr), &u4VaOutCSize);

		VQ_Printf(VQ_LOG_ADDRESS,
			"[ADDR] map va addr, in[0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x], out[0x%x, 0x%x]\n",
			prVqParam->u4InputAddrVaYPrev,
			prVqParam->u4InputAddrVaYCurr,
			prVqParam->u4InputAddrVaYNext,
			prVqParam->u4InputAddrVaCbcrPrev,
			prVqParam->u4InputAddrVaCbcrCurr,
			prVqParam->u4InputAddrVaCbcrNext,
			prVqParam->u4OutputAddrVaY,
			prVqParam->u4OutputAddrVaCbcr);

		VQ_Printf(VQ_LOG_ADDRESS, "[ADDR] map mva size, in[%d, %d], out[%d, %d]\n",
			u4MapSrcYsize, u4MapSrcCsize, u4MapDstYsize, u4MapDstCsize);

		VQ_Printf(VQ_LOG_ADDRESS, "[ADDR] map va size, in[%d, %d, %d, %d, %d, %d], out[%d, %d]\n",
			u4VaInYPrevSize, u4VaInYCurrSize, u4VaInYNextSize,
			u4VaInCPrevSize, u4VaInCCurrSize, u4VaInCNextSize,
			u4VaOutYSize, u4VaOutCSize);
	}
	#endif

	#if VQ_CTP_TEST
	remap_mem_range(0x83000000, 0x83000000, (2*256*1024*1024), MT_MEMORY);

	cache_disable();

	if (VQ_COLOR_FMT_420SCL == prVqParam->eDstColorFmt) {

		u4YCDivideSize = 2;

	} else if (VQ_COLOR_FMT_422SCL == prVqParam->eDstColorFmt) {

		u4YCDivideSize = 1;

	} else {

		VQ_Printf(VQ_LOG_CTP, "[CTP] not support dst color format\n", prVqParam->eDstColorFmt);
		VQ_ASSERT(0);
	}

	iVQ_IO_Read(prVqParam, eTimingType, rDispfmtParam.u1NrEnable, u4YCDivideSize, &u4ReadSize, &u4WriteSize);
	#endif

#if VQ_WAIT_IRQ
	/* reg irq */
	iVQ_Dispfmt_IrqReg(1);
#endif

	/* triggle hw */
	iVQ_Dispfmt_Triggle(&rDispfmtParam);

	/* clock off */
	iVQ_Dispfmt_ClkOff();

#if VQ_MVA_MAP_VA
	if (_vq_set_count == _vq_Process_count) {

		/*for keep brace*/
		VQ_Printf(VQ_LOG_FLOW, "[F] _vq_set_count == _vq_Process_count == %d\n", _vq_Process_count);
	}

	m4u_mva_unmap_kernel(rVdoParam.u4AddrYPrev, u4MapSrcYsize, prVqParam->u4InputAddrVaYPrev);

	m4u_mva_unmap_kernel(rVdoParam.u4AddrYCurr, u4MapSrcYsize, prVqParam->u4InputAddrVaYCurr);

	m4u_mva_unmap_kernel(rVdoParam.u4AddrYNext, u4MapSrcYsize, prVqParam->u4InputAddrVaYNext);

	m4u_mva_unmap_kernel(rVdoParam.u4AddrCbcrPrev, u4MapSrcCsize, prVqParam->u4InputAddrVaCbcrPrev);

	m4u_mva_unmap_kernel(rVdoParam.u4AddrCbcrCurr, u4MapSrcCsize, prVqParam->u4InputAddrVaCbcrCurr);

	m4u_mva_unmap_kernel(rVdoParam.u4AddrCbcrNext, u4MapSrcCsize, prVqParam->u4InputAddrVaCbcrNext);

	m4u_mva_unmap_kernel(rWcParam.u4OutputAddrY, u4MapDstYsize, prVqParam->u4OutputAddrVaY);

	m4u_mva_unmap_kernel(rWcParam.u4OutputAddrCbcr, u4MapDstCsize, prVqParam->u4OutputAddrVaCbcr);
#endif

	#if VQ_CTP_TEST
	iVQ_IO_Write(
		rDispfmtParam.u1NrEnable, prVqParam->u4OutputAddrMvaY, prVqParam->u4OutputAddrMvaCbcr,
		u4WriteSize, u4YCDivideSize,
		prVqParam->eDiMode, prVqParam->eCurrField, prVqParam->u1TopFieldFirst);
	#endif

	#if VQ_SLT
	iRet = iVQ_SumCompare(prVqParam->u4OutputAddrMvaY, prVqParam->u4OutputAddrMvaCbcr, u4WriteSize, u4YCDivideSize);
	#endif

	VQ_Printf(VQ_LOG_FLOW, "[F] cnt[%d], process end\n", _vq_Process_count);

	return iRet;
}

int iVQ_CmdSetDispfmtPattern(unsigned int set, unsigned int CmdEnable, unsigned int CmdWidth)
{
	if (set) {

		_rVqCmdDispfmtPattern.u1Enable = CmdEnable;
		_rVqCmdDispfmtPattern.u2Width = CmdWidth;

		VQ_Printf(VQ_LOG_CMD, "[CMD] set disp_pattern [%d, %d, %d, %d]\n",
			_rVqCmdDispfmtPattern.u1Enable,
			_rVqCmdDispfmtPattern.u2Width,
			_rVqCmdDispfmtPattern.u2Type,
			_rVqCmdDispfmtPattern.u1422_444);

	} else {

		VQ_Printf(VQ_LOG_CMD, "[CMD] current disp_pattern [%d, %d, %d, %d]\n",
			_rVqCmdDispfmtPattern.u1Enable,
			_rVqCmdDispfmtPattern.u2Width,
			_rVqCmdDispfmtPattern.u2Type,
			_rVqCmdDispfmtPattern.u1422_444);
	}

	return VQ_RET_OK;
}

int iVQ_CmdSetDiMode(
	unsigned int set, unsigned int CmdDiMode, unsigned int CmdCurrentField, unsigned int CmdTopFieldFirst)
{
	if (set) {

		_eVqCmdDiMode = CmdDiMode;
		_eVqCmdCurrField = CmdCurrentField;
		_u1VqCmdTopFieldFirst = CmdTopFieldFirst;

		VQ_Printf(VQ_LOG_CMD, "[CMD] set DiMode [%d, %d, %d]\n",
			_eVqCmdDiMode, _eVqCmdCurrField, _u1VqCmdTopFieldFirst);

	} else {

		VQ_Printf(VQ_LOG_CMD, "[CMD] current DiMode [%d, %d, %d]\n",
			_eVqCmdDiMode, _eVqCmdCurrField, _u1VqCmdTopFieldFirst);
	}

	return VQ_RET_OK;
}

int iVQ_CmdSetNrlevel(unsigned int set, unsigned int CmdMnrLevel, unsigned int CmdBnrLevel)
{
	if (set) {

		_u4VqCmdMnrLevel = CmdMnrLevel;
		_u4VqCmdBnrLevel = CmdBnrLevel;

		VQ_Printf(VQ_LOG_CMD, "[CMD] set nr level [%d, %d]\n", _u4VqCmdMnrLevel, _u4VqCmdBnrLevel);

	} else {

		VQ_Printf(VQ_LOG_CMD, "[CMD] current nr level [%d, %d]\n", _u4VqCmdMnrLevel, _u4VqCmdBnrLevel);
	}

	return VQ_RET_OK;
}

int iVQ_CmdSetColorFmt(unsigned int set, unsigned int CmdSrcColorFmt, unsigned int CmdDstColorFmt)
{
	if (set) {

		_eVqCmdSrcColorFmt = CmdSrcColorFmt;
		_eVqCmdDstColorFmt = CmdDstColorFmt;

		VQ_Printf(VQ_LOG_CMD, "[CMD] set color fmt [%d, %d]\n", _eVqCmdSrcColorFmt, _eVqCmdDstColorFmt);

	} else {

		VQ_Printf(VQ_LOG_CMD, "[CMD] current color fmt [%d, %d]\n", _eVqCmdSrcColorFmt, _eVqCmdDstColorFmt);
	}

	return VQ_RET_OK;
}

