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


#include "vq_nr_reg.h"
#include "vq_nr_hw.h"


/*********************************** define ***********************************/

#define VQ_NR_LEVEL_MAX                         16

#define VQ_NR_LEVEL_RANGE_CNT                   3

#define NR_MESS_FILTER_TYPE_CNT                 3
#define NR_MESS_FILTER_THL_CNT                  64
#define NR_MESS_FILTER_TYPE_VALUE(level)        (NR_MESS_FILTER_TYPE_CNT * level / VQ_NR_LEVEL_MAX)
#define NR_MESS_FILTER_THL_VALUE(level)         ((NR_MESS_FILTER_THL_CNT / VQ_NR_LEVEL_MAX) * level)

#define NR_BLENDING_LEVEL_CNT                   15
#define NR_BLENDING_LEVEL_VALUE(level)          (NR_BLENDING_LEVEL_CNT * level / VQ_NR_LEVEL_MAX)

/****************************** internal function *****************************/

static int iVQ_Nr_InitSNR(void)
{
	VQ_NR_W_SNRSM00(0x00000000);     /* 0xD40 */
	VQ_NR_W_SNRSM01(0x00000000);     /* 0xD44 */
	VQ_NR_W_SNR00(0x00050064);       /* 0xD80 */
	VQ_NR_W_SNR01(0x07800438);       /* 0xD84 */
	VQ_NR_W_SNR02(0x01680404);       /* 0xD88 */
	VQ_NR_W_SNR03(0x01808806);       /* 0xD8C */
	VQ_NR_W_SNR04(0x4092C864);       /* 0xD90 */
	VQ_NR_W_SNR05(0x00990C04);       /* 0xD94 */
	VQ_NR_W_SNR06(0x32140602);       /* 0xD98 */
	VQ_NR_W_SNR07(0x04040404);       /* 0xD9C */
	VQ_NR_W_SNR08(0x4C82D820);       /* 0xDA0 */
	VQ_NR_W_SNR09(0x31220C08);       /* 0xDA4 */
	VQ_NR_W_SNR0A(0x0E0A0404);       /* 0xDA8 */
	VQ_NR_W_SNR0B(0x04040808);       /* 0xDAC */
	VQ_NR_W_SNR18(0x140A140A);       /* 0xDE0 */
	VQ_NR_W_SNR1A(0xFF000000);       /* 0xDE8 */
	VQ_NR_W_SNR1B(0x003F7E20);       /* 0xDEC */
	VQ_NR_W_SNR1C(0x00014000);       /* 0xDF0 */
	VQ_NR_W_SNR1D(0x00000012);       /* 0xDF4 */
	VQ_NR_W_SNR1E(0x00000000);       /* 0xDF8 */
	VQ_NR_W_SNRSB00(0x02C1B025);     /* 0xDFC */
	VQ_NR_W_SNR1F(0x02C00C4F);       /* 0xE00 */
	VQ_NR_W_SNR20(0x00000100);       /* 0xE04 */
	VQ_NR_W_SNR21(0x3C230523);       /* 0xE08 */
	VQ_NR_W_SNR22(0x01622C96);       /* 0xE0C */
	VQ_NR_W_SNR27(0x4C230523);       /* 0xE20 */
	VQ_NR_W_SNR28(0x01642C96);       /* 0xE24 */
	VQ_NR_W_SNR2B(0x6C050523);       /* 0xE30 */
	VQ_NR_W_SNR2C(0x01662C96);       /* 0xE34 */
	VQ_NR_W_SNR2D(0x6C050523);       /* 0xE38 */
	VQ_NR_W_SNR2E(0x01662C96);       /* 0xE3C */
	VQ_NR_W_SNR2F(0x6C080223);       /* 0xE40 */
	VQ_NR_W_SNR30(0x01662C96);       /* 0xE44 */
	VQ_NR_W_SNR37(0x48084808);       /* 0xE60 */
	VQ_NR_W_SNR38(0x48084808);       /* 0xE64 */
	VQ_NR_W_SNR3B(0x48084808);       /* 0xE70 */
	VQ_NR_W_SNR3C(0x48084808);       /* 0xE74 */
	VQ_NR_W_SNR3F(0x48084808);       /* 0xE80 */
	VQ_NR_W_SNR40(0x48084808);       /* 0xE84 */
	VQ_NR_W_SNR55(0x48084808);       /* 0xED8 */
	VQ_NR_W_SNR56(0x48084808);       /* 0xEDC */
	VQ_NR_W_SNR57(0x48084808);       /* 0xEE0 */
	VQ_NR_W_SNR58(0x48084808);       /* 0xEE4 */
	VQ_NR_W_SNRSM11(0x00000006);     /* 0xEEC */
	VQ_NR_W_SNRSM12(0x80008000);     /* 0xEF0 */
	VQ_NR_W_SNRSM13(0x00000000);     /* 0xEF4 */
	VQ_NR_W_SNR92(0x00000000);       /* 0xEF8 */
	VQ_NR_W_SNR59(0x00000048);       /* 0xEFC */
	VQ_NR_W_SNR5A(0x04040404);       /* 0xF00 */
	VQ_NR_W_SNR5B(0xDBC04C38);       /* 0xF04 */
	VQ_NR_W_SNR5C(0xE020E200);       /* 0xF08 */
	VQ_NR_W_SNR5E(0x0FEDAEC0);       /* 0xF10 */
	VQ_NR_W_SNR60(0xCC962A00);       /* 0xF18 */
	VQ_NR_W_SNR63(0x00000430);       /* 0xF24 */
	VQ_NR_W_SNR64(0x00000000);       /* 0xF28 */
	VQ_NR_W_SNR65(0x0C190255);       /* 0xF2C */
	VQ_NR_W_SNR66(0x000C0404);       /* 0xF30 */
	VQ_NR_W_SNR67(0x00000000);       /* 0xF34 */
	VQ_NR_W_SNR68(0x00000000);       /* 0xF38 */
	VQ_NR_W_SNR69(0x00000000);       /* 0xF3C */
	VQ_NR_W_SNR6A(0x07F40202);       /* 0xF40 */
	VQ_NR_W_SNR6C(0x00000555);       /* 0xF48 */
	VQ_NR_W_SNR6E(0xA0000008);       /* 0xF50 */
	VQ_NR_W_SNR6F(0x00200810);       /* 0xF54 */
	VQ_NR_W_SNR70(0xFF6C3C8C);       /* 0xF58 */
	VQ_NR_W_SNR71(0x7864A483);       /* 0xF60 */
	VQ_NR_W_SNR72(0x77408155);       /* 0xF64 */
	VQ_NR_W_SNR73(0xA898886A);       /* 0xF68 */
	VQ_NR_W_SNR77(0xE020B027);       /* 0xF78 */
	VQ_NR_W_SNR78(0x90330000);       /* 0xF7C */
	VQ_NR_W_SNRBK00(0xD4024088);     /* 0xF84 */
	VQ_NR_W_SNRBK01(0x00078000);     /* 0xF88 */
	VQ_NR_W_SNRBK02(0x00005800);     /* 0xF8C */
	VQ_NR_W_SNRBK03(0x74C11000);     /* 0xF90 */
	VQ_NR_W_SNRBK04(0x00014000);     /* 0xF94 */
	VQ_NR_W_SNR7E(0x00004111);       /* 0xF98 */
	VQ_NR_W_SNR7F(0x44440000);       /* 0xF9C */
	VQ_NR_W_SNR80(0x00004111);       /* 0xFA0 */
	VQ_NR_W_SNR81(0x41114111);       /* 0xFA4 */
	VQ_NR_W_SNR83(0x40040000);       /* 0xFAC */
	VQ_NR_W_SNR84(0x00000000);       /* 0xFB0 */
	VQ_NR_W_SNR85(0x00000000);       /* 0xFB4 */
	VQ_NR_W_SNR86(0x0000FFFF);       /* 0xFB8 */
	VQ_NR_W_SNR87(0xFFFF0000);       /* 0xFBC */
	VQ_NR_W_SNR88(0x0000FFFF);       /* 0xFC0 */
	VQ_NR_W_SNR89(0xFFFFFFFF);       /* 0xFC4 */
	VQ_NR_W_SNR8B(0xF00F0000);       /* 0xFCC */
	VQ_NR_W_SNR8C(0x0000FFF0);       /* 0xFD0 */
	VQ_NR_W_SNR8D(0x00000000);       /* 0xFD4 */
	VQ_NR_W_SNR8E(0x70000000);       /* 0xFD8 */
	VQ_NR_W_SNRBK05(0x00000005);     /* 0xFDC */
	VQ_NR_W_SNRBK06(0x00000027);     /* 0xFE0 */
	VQ_NR_W_SNRBK07(0x00000005);     /* 0xFE4 */
	VQ_NR_W_SNRBK08(0x00000032);     /* 0xFE8 */

	return VQ_RET_OK;
}

static int iVQ_Nr_LevelMessFilter(int iLevel)
{
	/* 0xDA4 */
	if ((VQ_NR_LEVEL_MAX / VQ_NR_LEVEL_RANGE_CNT) >= iLevel) {

		/*for keep brace*/
		VQ_NR_W_SNR09(0x31220C08);
	} else if ((VQ_NR_LEVEL_MAX / VQ_NR_LEVEL_RANGE_CNT * 2) >= iLevel) {

		/*for keep brace*/
		VQ_NR_W_SNR09(0x19220C08);

	} else {

		/*for keep brace*/
		VQ_NR_W_SNR09(0x0D220C08);
	}

	/* 0xE60 */
	VQ_NR_W_R_MESSSEL_SM_CO1MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_SM_CO1MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_SM_CO1MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSSEL_EDGE_CO1MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_EDGE_CO1MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_EDGE_CO1MO(NR_MESS_FILTER_THL_VALUE(iLevel));

	/* 0xE64 */
	VQ_NR_W_R_MESSSEL_MESS_CO1MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_MESS_CO1MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_MESS_CO1MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSSEL_MOS_CO1MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_MOS_CO1MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_MOS_CO1MO(NR_MESS_FILTER_THL_VALUE(iLevel));

	/* 0xE70 */
	VQ_NR_W_R_MESSSEL_SM_CO2MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_SM_CO2MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_SM_CO2MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSSEL_EDGE_CO2MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_EDGE_CO2MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_EDGE_CO2MO(NR_MESS_FILTER_THL_VALUE(iLevel));

	/* 0xE74 */
	VQ_NR_W_R_MESSSEL_MESS_CO2MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_MESS_CO2MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_MESS_CO2MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSSEL_MOS_CO2MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_MOS_CO2MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_MOS_CO2MO(NR_MESS_FILTER_THL_VALUE(iLevel));

	/* 0xE80 */
	VQ_NR_W_R_MESSSEL_SM_CO3MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_SM_CO3MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_SM_CO3MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSSEL_EDGE_CO3MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_EDGE_CO3MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_EDGE_CO3MO(NR_MESS_FILTER_THL_VALUE(iLevel));

	/* 0xE84 */
	VQ_NR_W_R_MESSSEL_MESS_CO3MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_MESS_CO3MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_MESS_CO3MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSSEL_MOS_CO3MO(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_MOS_CO3MO(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_MOS_CO3MO(NR_MESS_FILTER_THL_VALUE(iLevel));

	/* 0xED8 */
	VQ_NR_W_R_MESSSEL_SM_BK(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_SM_BK(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_SM_BK(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSSEL_EDGE_BK(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_EDGE_BK(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_EDGE_BK(NR_MESS_FILTER_THL_VALUE(iLevel));

	/* 0xEDC */
	VQ_NR_W_R_MESSSEL_MESS_BK(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_MESS_BK(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_MESS_BK(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSSEL_MOS_BK(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_MOS_BK(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_MOS_BK(NR_MESS_FILTER_THL_VALUE(iLevel));

	/* 0xEE0 */
	VQ_NR_W_R_MESSSEL_SM_DEF(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_SM_DEF(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_SM_DEF(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSSEL_EDGE_DEF(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_EDGE_DEF(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_EDGE_DEF(NR_MESS_FILTER_THL_VALUE(iLevel));

	/* 0xEE4 */
	VQ_NR_W_R_MESSSEL_MESS_DEF(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_MESS_DEF(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_MESS_DEF(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSSEL_MOS_DEF(NR_MESS_FILTER_TYPE_VALUE(iLevel));
	VQ_NR_W_R_MESSSFT_MOS_DEF(NR_MESS_FILTER_THL_VALUE(iLevel));
	VQ_NR_W_R_MESSTHL_MOS_DEF(NR_MESS_FILTER_THL_VALUE(iLevel));

	/* 0xF98 */
	VQ_NR_W_R_UIBLDLV_BK_DEF(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_SM_DEF(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_MESS_DEF(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_EDGE_DEF(NR_BLENDING_LEVEL_VALUE(iLevel));

	/* 0xF9C */
	VQ_NR_W_R_UIBLDLV_BK_BK(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_SM_BK(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_MESS_BK(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_EDGE_BK(NR_BLENDING_LEVEL_VALUE(iLevel));

	/* 0xFA0 */
	VQ_NR_W_R_UIBLDLV_BK_CO1(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_SM_CO1(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_MESS_CO1(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_EDGE_CO1(NR_BLENDING_LEVEL_VALUE(iLevel));

	/* 0xFA4 */
	VQ_NR_W_R_UIBLDLV_BK_CO2(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_SM_CO2(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_MESS_CO2(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_EDGE_CO2(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_BK_CO3(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_SM_CO3(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_MESS_CO3(NR_BLENDING_LEVEL_VALUE(iLevel));
	VQ_NR_W_R_UIBLDLV_EDGE_CO3(NR_BLENDING_LEVEL_VALUE(iLevel));

	return VQ_RET_OK;
}

static int iVQ_Nr_LevelMnr(unsigned int u4MnrLevel)
{
	if (0 == u4MnrLevel) {

		/* 0xFAC */
		VQ_NR_W_R_UIBLDLV_MOS_BK(0);
		VQ_NR_W_R_UIBLDLV_MOS_DEF(0);

		/* 0xFB0 */
		VQ_NR_W_R_UIBLENDOUTLV_C(0);
		VQ_NR_W_R_UIBLDLV_MOS_CO3(0);
		VQ_NR_W_R_UIBLDLV_MOS_CO2(0);
		VQ_NR_W_R_UIBLDLV_MOS_CO1(0);

	} else {

		/* 0xFAC */
		VQ_NR_W_R_UIBLDLV_MOS_BK(NR_BLENDING_LEVEL_VALUE(u4MnrLevel));
		VQ_NR_W_R_UIBLDLV_MOS_DEF(NR_BLENDING_LEVEL_VALUE(u4MnrLevel));

		/* 0xFB0 */
		VQ_NR_W_R_UIBLENDOUTLV_C(NR_BLENDING_LEVEL_VALUE(u4MnrLevel));
		VQ_NR_W_R_UIBLDLV_MOS_CO3(NR_BLENDING_LEVEL_VALUE(u4MnrLevel));
		VQ_NR_W_R_UIBLDLV_MOS_CO2(NR_BLENDING_LEVEL_VALUE(u4MnrLevel));
		VQ_NR_W_R_UIBLDLV_MOS_CO1(NR_BLENDING_LEVEL_VALUE(u4MnrLevel));
	}

	return VQ_RET_OK;
}

static int iVQ_Nr_LevelBnr(unsigned int u4BnrLevel)
{
	if (0 == u4BnrLevel) {

		/* 0xE00 */
		VQ_NR_W_R_BLOCK_PDP(0);

	} else {

		/* 0xE00 */
		VQ_NR_W_R_BLOCK_PDP(1);
	}

	return VQ_RET_OK;
}

static int iVQ_Nr_ConfigSwitch(struct VQ_NR_PARAM_T *prParam)
{
	/* 0x42C */
	VQ_NR_W_NR_IN_CEN_RATIO(0);
	VQ_NR_W_NR_CORE_BYPASS(0);
	VQ_NR_W_NR_OUTPUT_DELAY_SEL(1);
	VQ_NR_W_NR_DELSEL(0);
	VQ_NR_W_NR_OUTPUT_BLANK_BLACK_EN(1);
	VQ_NR_W_NR_OUTPUT_BLANK_BLACK_Y(0x10);
	VQ_NR_W_NR_OUTPUT_BLANK_BLACK_CB(0x80);
	VQ_NR_W_NR_OUTPUT_BLANK_BLACK_CR(0x80);

	/* 0xC00 */
	VQ_NR_W_c_src_420(0);
	VQ_NR_W_c_nr_src_sel(0);
	VQ_NR_W_c_sw_init(0);
	VQ_NR_W_c_v_bound_protect(1);
	VQ_NR_W_c_line_buffer_repeat_right(0);
	VQ_NR_W_c_line_buffer_half_width(0x3ff);

	/* 0x874 */
	VQ_NR_W_c_ycbcr2yc_left_padding(0);
	VQ_NR_W_c_ycbcr2yc_even_extend(0);
	VQ_NR_W_c_ycbcr2yc_no_sync_block(0);
	VQ_NR_W_c_ycbcr2yc_keep_last_data(0);
	VQ_NR_W_c_ycbcr2yc_hsync_black(0);
	VQ_NR_W_c_ycbcr2yc_filter_on(0);
	VQ_NR_W_INPUT_444_ENABLE(0);
	VQ_NR_W_OUTPUT_422_ENABLE(1);

	/* 0x800 */
	VQ_NR_W_c_crc_sel(1);

	return VQ_RET_OK;
}

static int iVQ_Nr_ConfigB2R(struct VQ_NR_PARAM_T *prParam)
{
	return VQ_RET_OK;
}

static int iVQ_Nr_ConfigDram(struct VQ_NR_PARAM_T *prParam)
{
	VQ_NR_W_NR_OUTPUT_DELAY_SEL(1);
	VQ_NR_W_NR_OUTPUT_BLANK_BLACK_EN(1);
	VQ_NR_W_NR_OUTPUT_BLANK_BLACK_Y(0x80);
	VQ_NR_W_NR_OUTPUT_BLANK_BLACK_CB(0x30);
	VQ_NR_W_NR_OUTPUT_BLANK_BLACK_CR(0xA0);

	return VQ_RET_OK;
}

static int iVQ_Nr_ConfigMain(struct VQ_NR_PARAM_T *prParam)
{
	VQ_NR_W_INPUT_444_ENABLE(0);  /* check */
	VQ_NR_W_OUTPUT_422_ENABLE(1);

	VQ_NR_W_c_src_420(0);
	VQ_NR_W_c_nr_src_sel(0);
	VQ_NR_W_c_sw_init(0);
	VQ_NR_W_c_v_bound_protect(1);

	VQ_NR_W_c_line_buffer_mode(0x3);
	VQ_NR_W_c_line_buffer_manual_length(0x3FF);

	return VQ_RET_OK;
}

static int iVQ_Nr_ConfigSNR(struct VQ_NR_PARAM_T *prParam)
{
	iVQ_Nr_InitSNR();

	if ((0 == prParam->u4BnrLevel) && (0 == prParam->u4MnrLevel)) {

		/*for keep brace*/
		VQ_NR_W_R_2D_ENABLE(0);

	} else {

		/*for keep brace*/
		VQ_NR_W_R_2D_ENABLE(1);
	}

	iVQ_Nr_LevelMnr(prParam->u4MnrLevel);

	iVQ_Nr_LevelBnr(prParam->u4BnrLevel);

	iVQ_Nr_LevelMessFilter(prParam->u4MnrLevel);

	return VQ_RET_OK;
}

static int iVQ_Nr_ConfigDemoMode(struct VQ_NR_PARAM_T *prParam)
{
	if (prParam->rDemo.u1EnableDemo) {

		/*	*/
		/*	*/
	}

	return VQ_RET_OK;
}

static int iVQ_Nr_ConfigCrc(struct VQ_NR_PARAM_T *prParam)
{
	if (prParam->rCrc.u1EnableCrc) {

		/*for keep brace*/
		VQ_NR_W_c_crc_sel(prParam->rCrc.u1CrcType);
	}

	return VQ_RET_OK;
}

static int iVQ_Nr_ConfigPattern(struct VQ_NR_PARAM_T *prParam)
{
	if (prParam->rPattern.u1EnablePattern) {

		/*for keep brace*/
		/*	*/
	}

	return VQ_RET_OK;
}

/******************************* global function ******************************/

int iVQ_Nr_SetParam(struct VQ_NR_PARAM_T *prParam)
{
	if ((0 != prParam->u4BnrLevel) || (0 != prParam->u4MnrLevel)) {

		iVQ_Nr_ConfigSwitch(prParam);

		iVQ_Nr_ConfigB2R(prParam);

		iVQ_Nr_ConfigDram(prParam);

		iVQ_Nr_ConfigMain(prParam);

		iVQ_Nr_ConfigSNR(prParam);

		iVQ_Nr_ConfigDemoMode(prParam);

		iVQ_Nr_ConfigCrc(prParam);

		iVQ_Nr_ConfigPattern(prParam);
	}

	return VQ_RET_OK;
}


