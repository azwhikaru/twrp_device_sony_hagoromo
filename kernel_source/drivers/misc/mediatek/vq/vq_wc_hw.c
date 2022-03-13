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

#include "vq_wc_reg.h"
#include "vq_wc_hw.h"


/*********************************** define ***********************************/


/*********************************** variable *********************************/


/*********************************** function *********************************/

int iVQ_WC_SetParam(struct VQ_WC_PARAM_T *prParam)
{
	VQ_WC_W_VDOIN_EN(0x00000000);   /* 0x000 */
	VQ_WC_W_VDOIN_EN(0x8200400d);   /* 0x000 */

	/* 0x000[25] */
	if (VQ_COLOR_FMT_420BLK == prParam->eDstColorFmt) {

		VQ_WC_W_mode_422(0);
		VQ_WC_W_linear_enable(0);

	} else if (VQ_COLOR_FMT_420SCL == prParam->eDstColorFmt) {

		VQ_WC_W_mode_422(0);
		VQ_WC_W_linear_enable(1);

	} else if (VQ_COLOR_FMT_422BLK == prParam->eDstColorFmt) {

		VQ_WC_W_mode_422(1);
		VQ_WC_W_linear_enable(0);

	} else if (VQ_COLOR_FMT_422SCL == prParam->eDstColorFmt) {

		VQ_WC_W_mode_422(1);
		VQ_WC_W_linear_enable(1);

	} else {

		VQ_Printf(VQ_LOG_ERROR, "[E] not support dst color %d\n", prParam->eDstColorFmt);
		return VQ_RET_ERR_PARAM;
	}

	VQ_WC_W_VDOIN_MODE(0x38000040); /* 0x004 */

	VQ_WC_W_YBUF0_ADDR((prParam->u4OutputAddrY >> 2));   /* 0x008 */

	VQ_WC_W_ACT_LINE(0xf20811df); /* 0x00C */

#if 1   /* VQ_WH_TIMING */
	VQ_WC_W_ACTLINE((prParam->u4Height - 1));  /* 0x00C[11:0] */
#else
	VQ_WC_W_ACTLINE((_arFmtTimingInfo[prParam->eTimingType].u4SizeV - 1));  /* 0x00C[11:0] */
#endif

	VQ_WC_W_CBUF0_ADDR((prParam->u4OutputAddrCbcr >> 2));  /* 0x010 */

#if 1   /* VQ_WH_TIMING */
	if ((VQ_COLOR_FMT_420BLK == prParam->eDstColorFmt) || (VQ_COLOR_FMT_420SCL == prParam->eDstColorFmt)) {

		/*for keep brace*/
		VQ_WC_W_DW_NEED_C_LINE(((prParam->u4Height / 2) - 1));   /* 0x014[27:16] */

	} else if ((VQ_COLOR_FMT_422BLK == prParam->eDstColorFmt) || (VQ_COLOR_FMT_422SCL == prParam->eDstColorFmt)) {

		/*for keep brace*/
		VQ_WC_W_DW_NEED_C_LINE((prParam->u4Height - 1));   /* 0x014[27:16] */
	}

	VQ_WC_W_DW_NEED_Y_LINE((prParam->u4Height - 1));   /* 0x014[11:0] */
#else
	VQ_WC_W_DW_NEED_C_LINE((_arFmtTimingInfo[prParam->eTimingType].u4SizeV - 1));   /* 0x014[27:16] */
	VQ_WC_W_DW_NEED_Y_LINE((_arFmtTimingInfo[prParam->eTimingType].u4SizeV - 1));   /* 0x014[11:0] */
#endif

	VQ_WC_W_HPIXEL(0x0001003b); /* 0x018 */

	VQ_WC_W_INPUT_CTRL(0x0400c024); /* 0x020 */

	/* vFmt_WriteReg(0x1c003024, 0x0000002d); */

	VQ_WC_W_HCNT_SETTING(0x02d00359); /* 0x034 */

#if 1   /* VQ_WH_TIMING */
	VQ_WC_W_HACTCNT(prParam->u4Width);  /* 0x034[28:16] */
#else
	VQ_WC_W_HACTCNT(_arFmtTimingInfo[prParam->eTimingType].u4SizeH);  /* 0x034[28:16] */
#endif

#if 1   /* VQ_WH_TIMING */
	VQ_WC_W_CHCNT((prParam->u4Width / 16 - 1));    /* 0x038[25:16] */
	VQ_WC_W_YHCNT((prParam->u4Width / 16 - 1));    /* 0x038[9:0] */
#else
	VQ_WC_W_CHCNT((_arFmtTimingInfo[prParam->eTimingType].u4SizeH / 16 - 1));    /* 0x038[25:16] */
	VQ_WC_W_YHCNT((_arFmtTimingInfo[prParam->eTimingType].u4SizeH / 16 - 1));    /* 0x038[9:0] */
#endif

	VQ_WC_W_VSCALE(0x002d40e3);   /* 0x03C */
#if 1   /* VQ_WH_TIMING */
	VQ_WC_W_bghsize_dw((prParam->u4Width / 16));   /* 0x03C[25:16] */
#else
	VQ_WC_W_bghsize_dw((_arFmtTimingInfo[prParam->eTimingType].u4SizeH / 16));   /* 0x03C[25:16] */
#endif

	VQ_WC_W_swrst(0x3);    /* 0x030 */
	VQ_WC_W_swrst(0x0);    /* 0x030 */

	return VQ_RET_OK;
}


