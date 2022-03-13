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

#include <linux/vmalloc.h>
#include <mach/m4u.h>

#include "vq_vdo_reg.h"
#include "vq_vdo_hw.h"


/*********************************** define ***********************************/
#define MA8F_BUFFER_ALIGN 2048

/* additional 128x6 bits */
#define MA8F_PER_BUF_SIZE (((768 * 576 / 2 / 8 / 2 + 6 * 128 / 8) + MA8F_BUFFER_ALIGN - 1) & ~(MA8F_BUFFER_ALIGN - 1))

/* the additional 1 for protection. CQ 121346 */
#define MA8F_BUFFER_TOTAL_SIZE (MA8F_PER_BUF_SIZE * 2 * (4 + 1) + (MA8F_BUFFER_ALIGN - 1))


/*********************************** variable *********************************/
#if VQ_CTP_TEST
static unsigned int _u4VdoFusionBufVa = 0x26a00000;
static unsigned int _u4VdoFusionBufMva = 0x26a00000;
#else
static unsigned int _u4VdoFusionBufVa;
static unsigned int _u4VdoFusionBufMva;
#endif

static unsigned short _u2VdoContrastThreshold       = 0x20;     /* 0x438[15:8] */
static unsigned short _u2VdoMovingThreshold         = 0x10;     /* 0x438[23:16] */
static unsigned short _u2VdoPullDownCombThreshold   = 0x03;     /* 0x4c4[22:16] */

static struct VDO_HW_SRAM_UTILIZATION_T rVdoHwSramUtil[] = {
	{0, 0, 0},
	{0, 0, 1},
	{0, 1, 0},
	{1, 1, 0}
};

static struct VDO_START_LINE_T rYUV420StartLine[] = {
	{0x07fb07fa, 0x07fd07fc, 0x80800000, 0x2060a0e0, 0x07fa07f9, 0x07fe07fd},
	{0x07fb07fa, 0x07fd07fc, 0x80800000, 0x2060a0e0, 0x07fa07f9, 0x07fe07fd},
	{0x07fa07f9, 0x07fd07fc, 0x00000000, 0x20602060, 0x07fa07f9, 0x07fd07fc},
	{0x07fd07fc, 0x07ff07fe, 0x80800000, 0xa0e02060, 0x07fc07fb, 0x07fe07fd},
	{0x07fb07fa, 0x07ff07fe, 0x80800000, 0xa0e02060, 0x07fa07f9, 0x07fe07fd}
};

/*static struct VDO_START_LINE_T rYUV422StartLine[] = {
	{0x07fb07fa,0x07fb07fa,0x80800000,0x80800000,0x07fa07f9,0x07fa07f9}, // 0
	{0x07fb07fa,0x07fb07fa,0x80800000,0x80800000,0x07fa07f9,0x07fa07f9}, // 1
	{0x07fa07f9,0x07fa07f9,0x00000000,0x00000000,0x07fa07f9,0x07fa07f9}, // 2
	{0x07fd07fc,0x07fb07fa,0x80800000,0x00008080,0x07fc07fb,0x07fc07fb}  // 3
};*/

/****************************** internal function *****************************/

static int iVQ_Vdo_ConfigBase(struct VQ_VDO_PARAM_T *prParam)
{
	if ((VDO_FRAME_MODE != prParam->eDiMode)) {

		if (VQ_TIMING_TYPE_720P < prParam->eTimingType) {	/* 1080p no 8 tap */

			VQ_VDO_W_VDO_SCL_CTRL(0x00212f00);  /* 0x14c[25:24] y c 8 tap,full hd no 8tap */

		} else {

			VQ_VDO_W_VDO_SCL_CTRL(0x03212f00);
		}

	} else {

		VQ_VDO_W_VDO_SCL_CTRL(0x00212f00);
	}

	VQ_VDO_W_VDO_8TAP_VALID(0x01e00002);  /* 0x194 */
	VQ_VDO_W_VDO_8TAP_CTRL_04(0x00000000);  /* 0x1a0 */
	VQ_VDO_W_VDO_SHARP_CTRL_01(0x40ffff80);  /* 0x1b0 */
	VQ_VDO_W_VDO_SHARP_CTRL_02(0x00822020);  /* 0x1b4 */
	/* VQ_VDO_W_VDO_SHARP_CTRL_03(0x01108080);    //0x1b8 */

	VQ_VDO_W_VDOY1(0x26200000);  /* 0x400      //Y - luma */
	VQ_VDO_W_VDOC1(0x26300000);  /* 0x404      //Y - chroma */
	VQ_VDO_W_VDOY2(0x26200000);  /* 0x408      //X - luma */
	VQ_VDO_W_VDOC2(0x26300000);  /* 0x40c      //X - chroma */

	/* 0x410 */
	VQ_VDO_W_swap_off(0);        /* check */
	VQ_VDO_W_HBLOCK_9_8(0);      /* check */
	VQ_VDO_W_PICHEIGHT(prParam->u4Height);          /* check */
	VQ_VDO_W_DW_NEED(prParam->u4Width / 4);         /* check */
	VQ_VDO_W_HBLOCK_7_0(prParam->u4Width / 8);      /* check */

	VQ_VDO_W_VSCALE(0x80000800);  /* 0x414 */
	VQ_VDO_W_STAMBR(0x1e1e0000);  /* 0x418 */
	VQ_VDO_W_VMODE(0x00600003);  /* 0x41c */
	VQ_VDO_W_YSL(0x0ffd0ffc);  /* 0x420 */
	VQ_VDO_W_CSL(0x0fff0ffe);  /* 0x424 */
	VQ_VDO_W_YSSL(0x80800000);  /* 0x428 */
	VQ_VDO_W_CSSL(0xa0e02060);  /* 0x42c */
	VQ_VDO_W_VDOCTRL(0x00000204);  /* 0x430 */
	VQ_VDO_W_VSLACK(0x14c81414);  /* 0x434 */
	VQ_VDO_W_MP(0x06000e07);  /* 0x438 */
	VQ_VDO_W_VDORST(0x000000ff);  /* 0x43c */
	VQ_VDO_W_VDORST(0x00000000);  /* 0x43c */
	VQ_VDO_W_COMB_8F(0x2002ff00);  /* 0x440 */
	VQ_VDO_W_YSL2(0x0ffc0ffb);  /* 0x450 */
	VQ_VDO_W_CSL2(0x0ffe0ffd);  /* 0x454 */
	VQ_VDO_W_MBAVG1(0x00800000);  /* 0x458 */
	VQ_VDO_W_MBAVG2(0x00840000);  /* 0x45c */
	VQ_VDO_W_CMB_CNT(0x00000001);  /* 0x460 */
	VQ_VDO_W_CMB_CNT(0x00000000);  /* 0x460 */
	VQ_VDO_W_PS_MODE(0x00008000);  /* 0x464 */
	VQ_VDO_W_STA_POS(0x05010000);  /* 0x46c */
	VQ_VDO_W_PS_FLT(0x10000000);  /* 0x470 */

	if ((VDO_FRAME_MODE != prParam->eDiMode)) {

		if (VQ_TIMING_TYPE_720P < prParam->eTimingType) {

			/*for keep brace*/
			VQ_VDO_W_F_CTRL(0x00000081);  /* 0x478[7]  4tap for full hd 1080 */

		} else {

			/*for keep brace*/
			VQ_VDO_W_F_CTRL(0x00000001);  /* 0x478 */
		}

	} else {

		/*for keep brace*/
		VQ_VDO_W_F_CTRL(0x00000000);  /* 0x478 */
	}

	VQ_VDO_W_VIDEO_OPTION(0x04000000);  /* 0x47c //check */
	VQ_VDO_W_PTR_WF_Y(0x26000000);  /* 0x480      //W - luma */
	VQ_VDO_W_PTR_ZF_Y(0x26400000);  /* 0x484      //Z - luma */
	VQ_VDO_W_FDIFF_TH3(0x4b0186a0);  /* 0x488 */
	VQ_VDO_W_FDIFF_TH2(0x5b011940);  /* 0x48c */
	VQ_VDO_W_FDIFF_TH1(0x0000c350);  /* 0x490 */
	VQ_VDO_W_TH_XZ_MIN(0x00012003);  /* 0x494 */
	VQ_VDO_W_FCH_YW(0x00036000);  /* 0x4a8 */
	VQ_VDO_W_CRM_SAW(0x02000020);  /* 0x4ac */
	VQ_VDO_W_EDGE_CTL(0x0332323c);  /* 0x4b0 */
	VQ_VDO_W_MD_ADV(0x02332093);  /* 0x4c0 */
	VQ_VDO_W_PD_FLD_LIKE(0x01030510);  /* 0x4c4 */
	VQ_VDO_W_PD_REGION(0x00007601);  /* 0x4cc */
	VQ_VDO_W_CHROMA_MD(0x10001433);  /* 0x4d0 */
	VQ_VDO_W_MBAVG3(0x00800000);  /* 0x4d8 */

	VQ_VDO_W_HD_MODE(0xa20000b4);  /* 0x4e0 */
#if 1   /* VQ_WH_TIMING */
	VQ_VDO_W_HD_MEM(((prParam->eTimingType > VQ_TIMING_TYPE_576P)?1:0));
	VQ_VDO_W_HD_MEM_1920(((prParam->eTimingType > VQ_TIMING_TYPE_720P)?1:0));
	VQ_VDO_W_DW_NEED_HD((prParam->u4Width / 4));    /* check */
#else
	VQ_VDO_W_HD_MEM(((_arFmtTimingInfo[prParam->eTimingType].u4SizeH > 720)?1:0));
	VQ_VDO_W_HD_MEM_1920(((_arFmtTimingInfo[prParam->eTimingType].u4SizeH > 1280)?1:0));
	VQ_VDO_W_DW_NEED_HD((_arFmtTimingInfo[prParam->eTimingType].u4SizeH / 4));
#endif

	VQ_VDO_W_PTR_AF_Y(0x26100000);  /* 0x4ec      //W - chroma */
	VQ_VDO_W_WMV_DISABLE(0xffffffff);  /* 0x4f8 */
	VQ_VDO_W_PTR_ZF_YC(0x26500000);  /* 0x4fc      //Z - chroma */

	VQ_VDO_W_METRIC_00(0x780f1001);  /* 0x700 */

	VQ_VDO_W_MDDI_FUSION_00(0x00000000);  /* 0x800 */
	VQ_VDO_W_MDDI_FUSION_08(0x14140041);  /* 0x820 */
	VQ_VDO_W_MDDI_FUSION_1A(0x00000000);  /* 0x868 */
	VQ_VDO_W_MDDI_FUSION_1C(0x00388000);  /* 0x870 */
	VQ_VDO_W_MDDI_FUSION_20(0x00040000);  /* 0x880 */
	VQ_VDO_W_MDDI_FUSION_22(0xc0000400);  /* 0x888 */
	VQ_VDO_W_MDDI_FUSION_23(0x0020fffe);  /* 0x88c */
	VQ_VDO_W_MDDI_PE_00(0x00000005);  /* 0x8b0 */
	VQ_VDO_W_MDDI_PE_03(0x00054444);  /* 0x8bc */

	VQ_VDO_W_VDO_CRC_00(0x10000000);  /* 0xf00 */
	VQ_WriteReg(VQ_IO_BASE + 0x2f20, 0x00000000);  /* 0xf20 //check */
	VQ_WriteReg(VQ_IO_BASE + 0x2f24, 0x00c800c8);  /* 0xf24 //check */
	VQ_WriteReg(VQ_IO_BASE + 0x2f28, 0x03070206);  /* 0xf28 //check */
	VQ_WriteReg(VQ_IO_BASE + 0x2f2c, 0x05070406);  /* 0xf2c //check */
	VQ_VDO_W_VDO_PQ_OPTION(0x00e0802b);  /* 0xf40 */
	VQ_VDO_W_VDO_PQ_OPTION2(0x1e007800);  /* 0xf44 */
	VQ_WriteReg(VQ_IO_BASE + 0x2f4c, 0x00f29a0a);  /* 0xf4c //check */
	VQ_VDO_W_MDDI_FILM_02(0x00033535);  /* 0xf60 */
	VQ_VDO_W_CHROMA_SAW_CNT(0x00ff00ff);  /* 0xfe0 */
	VQ_VDO_W_VDO_PQ_OPTION5(0x8050001e);  /* 0xfe4 */
	VQ_VDO_W_DEMO_MODE(0x00000000);  /* 0xfec */
	VQ_VDO_W_VDO_PQ_OPTION9(0x18060032);  /* 0xff4 */

	VQ_VDO_W_VDO_CRC_00(0x20000000);  /* 0xf00 */
	VQ_VDO_W_VDO_CRC_00(0x00000000);  /* 0xf00 */
	VQ_VDO_W_VDO_CRC_00(0x10000000);  /* 0xf00 */
	VQ_VDO_W_VDO_CRC_00(0x00000000);  /* 0xf00 */

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigDIMode(enum VDO_DI_MODE eDiMode)
{
	if (VDO_FRAME_MODE == eDiMode) {

		VQ_VDO_W_FRMC(0x1);  /* 0x430[1:1] */
		VQ_VDO_W_FRMY(0x1); /* 0x430[0:0] */
		VQ_VDO_W_MA4F(0x0);  /* 0x488[24] */
		VQ_VDO_W_MA8F_OR(0x0);  /* 0x4c0[25] */
		VQ_VDO_W_IFUSION_EN(0x0);  /* 0x800[0] */

	} else if (VDO_FIELD_MODE == eDiMode) {

		VQ_VDO_W_FRMC(0x0);  /* 0x430[1:1] */
		VQ_VDO_W_FRMY(0x0); /* 0x430[0:0] */
		VQ_VDO_W_INTRA_EDGEP_MODE(0x0);  /* 0x478[11] */
		VQ_VDO_W_MA4F(0x0);  /* 0x488[24] */
		VQ_VDO_W_MA8F_OR(0x0);  /* 0x4c0[25] */
		VQ_VDO_W_IFUSION_EN(0x0);  /* 0x800[0] */

	} else if (VDO_INTRA_MODE_WITH_EDGE_PRESERVING == eDiMode) {

		VQ_VDO_W_FRMC(0x0);  /* 0x430[1:1] */
		VQ_VDO_W_FRMY(0x0); /* 0x430[0:0] */
		VQ_VDO_W_INTRA_EDGEP_MODE(0x1);  /* 0x478[11] */
		VQ_VDO_W_MA4F(0x0);  /* 0x488[24] */
		VQ_VDO_W_MA8F_OR(0x0);  /* 0x4c0[25] */
		VQ_VDO_W_IFUSION_EN(0x0);  /* 0x800[0] */

	} else if (VDO_4FIELD_MA_MODE == eDiMode) {

		VQ_VDO_W_MA4F(0x1);  /* 0x488[24] */
		VQ_VDO_W_MA8F_OR(0x0);  /* 0x4c0[25] */
		VQ_VDO_W_IFUSION_EN(0x0);  /* 0x800[0] */

	} else if (VDO_8FIELD_MA_MODE == eDiMode) {

		VQ_VDO_W_MA4F(0x1);   /* 0x488[24]  // 4fld */
		VQ_VDO_W_MA8F_OR(0x1);   /* 0x4c0[25]  // 8fld */
		VQ_VDO_W_IFUSION_EN(0x0);  /* 0x800[0] */

	} else if (VDO_FUSION_MODE == eDiMode) {

		VQ_VDO_W_MA4F(0x1);  /* 0x488[24] */
		VQ_VDO_W_MA8F_OR(0x0);  /* 0x4c0[25] */
		VQ_VDO_W_IFUSION_EN(0x1);  /* 0x800[0] */
	}

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigSrcsize(unsigned int u4Width, unsigned int u4Height)
{
	VQ_VDO_W_HBLOCK_7_0(u4Width / 8);   /* 410[7:0] */

	VQ_VDO_W_DW_NEED(u4Width / 4);  /* 0x410[15:8] */

	VQ_VDO_W_PICHEIGHT(u4Height); /* 0x410[26:16] */

	VQ_VDO_W_DW_NEED_HD(u4Width / 4); /* 0x4e0[8:0] */

	VQ_VDO_W_HD_LINE_MODE(((u4Height >= 720) ? 1 : 0)); /* 0x4e0[20] //check */

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigHwSramUtil(enum VDO_DI_MODE eDiMode, unsigned int u4Width)
{
	unsigned int u4Hw_Sram_Util = 0;
	unsigned int HD_MEM_1920;	/* 0x4e0[21] */
	unsigned int HD_MEM;		/* 0x4e0[22] */
	unsigned int HD_EN;		/* 0x4e0[24] */

	if (u4Width <= 720) {

		/*for keep brace*/
		u4Hw_Sram_Util = 0;

	} else if ((u4Width > 720) && (u4Width <= 1920) && (eDiMode == VDO_FRAME_MODE || eDiMode == VDO_FIELD_MODE)) {

		/* check */
		/*for keep brace*/
		u4Hw_Sram_Util = 1;

	} else if ((u4Width > 720) && (u4Width <= 1280) && (eDiMode >= VDO_4FIELD_MA_MODE)) {

		/*for keep brace*/
		u4Hw_Sram_Util = 2;

	} else if ((u4Width > 1280) && (u4Width <= 1920) && (eDiMode >= VDO_4FIELD_MA_MODE)) {

		/*for keep brace*/
		u4Hw_Sram_Util = 3;

	} else {

		/*for keep brace*/
		VQ_Printf(VQ_LOG_ERROR, "[E] vdo sram config fail, w = %d, di = %d\n", u4Width, eDiMode);
	}

	VQ_Printf(VQ_LOG_FLOW, "[F] vdo sram config to %d, w = %d, di = %d\n", u4Hw_Sram_Util, u4Width, eDiMode);

	HD_MEM_1920 = rVdoHwSramUtil[u4Hw_Sram_Util].HD_MEM_1920;
	HD_MEM = rVdoHwSramUtil[u4Hw_Sram_Util].HD_MEM;
	HD_EN = rVdoHwSramUtil[u4Hw_Sram_Util].HD_EN;

	VQ_VDO_W_HD_MEM_1920(HD_MEM_1920);	/* 0x4e0[21] */
	VQ_VDO_W_HD_MEM(HD_MEM);		/* 0x4e0[22] */
	VQ_VDO_W_HD_EN(HD_EN);			/* 0x4e0[24] */

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigStartLine(enum VDO_DI_MODE eDiMode,
				   unsigned int u4Width,
				   enum VDO_VERTICAL_FILTER_MODE eVFilterMode,
				   enum VDO_SRC_FORMAT eSrcFmt)
{
	unsigned int StartLineForY = 0;
	unsigned int StartLineForC = 0;
	unsigned int StarSubLineForY = 0;
	unsigned int StarSubLineForC = 0;
	unsigned int AlternatvieStartLineForY = 0;
	unsigned int AlternatvieStartLineForC = 0;

	if (eSrcFmt == VDO_SRC_FMT_YUV420_BLK || eSrcFmt == VDO_SRC_FMT_YUV420_SCL) {

		enum VDO_YUV420_START_LINE_MODE eYUV420StartLineMode = VDO_YUV420_START_LINE_MODE0;

		if (
		/* SD, field/4field, no 4/8tap */
		((u4Width <= 720) && (eDiMode >= VDO_FIELD_MODE) && (eVFilterMode == VDO_VERTICAL_FILTER_LINEAR))
		/* HD, field/4field, no 4tap */
		|| ((u4Width > 720) && (eDiMode >= VDO_FIELD_MODE) && (eVFilterMode == VDO_VERTICAL_FILTER_LINEAR))
		/* HD, frame mode/4field, 4tap */
		|| ((u4Width > 720) &&
			(eDiMode == VDO_FRAME_MODE || eDiMode >= VDO_4FIELD_MA_MODE) &&
			(eVFilterMode == VDO_VERTICAL_FILTER_4TAP))) {

			eYUV420StartLineMode = VDO_YUV420_START_LINE_MODE1;

		} else if (
		    /* SD , frame, no 4/8tap */
		    /* HD, frame, no 4tap */
		    ((eDiMode == VDO_FRAME_MODE) && (eVFilterMode == VDO_VERTICAL_FILTER_LINEAR))) {

			eYUV420StartLineMode = VDO_YUV420_START_LINE_MODE2;

		} else if (
		    /* SD,frame/4fld, 4/8tap */
		    ((u4Width <= 720) &&
		     (eDiMode == VDO_FRAME_MODE || eDiMode >= VDO_4FIELD_MA_MODE) &&
		     (eVFilterMode >= VDO_VERTICAL_FILTER_4TAP))) {

			eYUV420StartLineMode = VDO_YUV420_START_LINE_MODE3;

		} else if (
		    /* 720p ,frame/4fld, 8tap */
		    ((u4Width > 720 && u4Width <= 1280) &&
		     (eDiMode == VDO_FRAME_MODE || eDiMode >= VDO_4FIELD_MA_MODE) &&
		     (eVFilterMode == VDO_VERTICAL_FILTER_8TAP))) {

			eYUV420StartLineMode = VDO_YUV420_START_LINE_MODE4;

		} else {

			/*	*/
			/*	*/
		}

		StartLineForY = rYUV420StartLine[eYUV420StartLineMode].StartLineForY;
		StartLineForC = rYUV420StartLine[eYUV420StartLineMode].StartLineForC;
		StarSubLineForY = rYUV420StartLine[eYUV420StartLineMode].StarSubLineForY;
		StarSubLineForC = rYUV420StartLine[eYUV420StartLineMode].StarSubLineForC;
		AlternatvieStartLineForY = rYUV420StartLine[eYUV420StartLineMode].AlternatvieStartLineForY;
		AlternatvieStartLineForC = rYUV420StartLine[eYUV420StartLineMode].AlternatvieStartLineForC;

	} else if (eSrcFmt == VDO_SRC_FMT_YUV422_BLK || eSrcFmt == VDO_SRC_FMT_YUV422_SCL) {

		/*for keep brace*/
		/* enum VDO_YUV422_START_LINE_MODE eYUV422StartLineMode = VDO_YUV422_START_LINE_MODE0; */
	}

	VQ_VDO_W_YSL(StartLineForY);   /* 0x420 */
	VQ_VDO_W_CSL(StartLineForC);   /* 0x424 */
	VQ_VDO_W_YSSL(StarSubLineForY); /* 0x428 */
	VQ_VDO_W_CSSL(StarSubLineForC); /* 0x42C */
	VQ_VDO_W_YSL2(AlternatvieStartLineForY);    /* 0x450 */
	VQ_VDO_W_CSL2(AlternatvieStartLineForC);    /* 0x454 */

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigMA8F(unsigned char bIsOn, unsigned int AddrW, unsigned int Addr1, unsigned int Addr2)
{
#if 0
	if (bIsOn) {

		vVdo_WriteReg(0x1c002458, PHYSICAL(Addr1));
		vVdo_WriteReg(0x1c00245c, PHYSICAL(Addr2));
		vVdo_WriteReg(0x1c0024d8, PHYSICAL(AddrW));

		vVdo_WriteReg(0x1c002f54, PHYSICAL(AddrW));
		vVdo_WriteReg(0x1c0024dc, PHYSICAL(Addr2));
	}

	vVdo_WriteRegMask(0x1c002440, bIsOn << 29, 0x1 << 29); /* DYN_8F enable dynamic 8-field motion detection */
	vVdo_WriteRegMask(0x1c002488, bIsOn << 31, 0x1 << 31); /* 6fld */
	vVdo_WriteRegMask(0x1c0024c0, bIsOn << 25, 0x1 << 25); /* 8fld */
	vVdo_WriteRegMask(0x1c0024c0, 0x3 << 16, 0xf << 16); /* u4VAC_6F,SD */

	vVdo_WriteRegMask(0x1c00247c, 0 << 12, 0x1 << 12); /* fgPROT_WR_STA */
	vVdo_WriteRegMask(0x1c00247c, 0 << 13, 0x1 << 13); /* fgPROT_WR_END */

	if (bIsOn) {

		/*for keep brace*/
		vVdo_WriteRegMask(0x1c002494, 0x48 << 24, 0xff << 24); /* test mode */

	} else {

		/*for keep brace*/
		vVdo_WriteRegMask(0x1c002494, 0x00 << 24, 0xff << 24); /* test mode */
	}
#endif

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigDeintWXYZ(unsigned int *pu4AddrY, unsigned int *pu4AddrC)
{
	if ((pu4AddrY) && (pu4AddrC)) {

		if ((pu4AddrY[0] == 0) ||
		    (pu4AddrY[1] == 0) ||
		    (pu4AddrY[2] == 0) ||
		    (pu4AddrY[3] == 0) ||
		    (pu4AddrC[0] == 0) ||
		    (pu4AddrC[1] == 0) ||
		    (pu4AddrC[2] == 0) ||
		    (pu4AddrC[3] == 0)) {

			/*for keep brace*/
			/*  */
		}

		VQ_VDO_W_PTR_WF_Y((pu4AddrY[0] >> 2));     /* 0x480   // W */
		VQ_VDO_W_VDOY2((pu4AddrY[1] >> 2));        /* 0x408   // X */
		VQ_VDO_W_VDOY1((pu4AddrY[2] >> 2));        /* 0x400   // Y */
		VQ_VDO_W_PTR_ZF_Y((pu4AddrY[3] >> 2));     /* 0x484   // Z */

		VQ_VDO_W_PTR_AF_Y((pu4AddrC[0] >> 2));     /* 0x4ec   // w */
		VQ_VDO_W_VDOC2((pu4AddrC[1] >> 2));        /* 0x40c   // x */
		VQ_VDO_W_VDOC1((pu4AddrC[2] >> 2));        /* 0x404   // y */
		VQ_VDO_W_PTR_ZF_YC((pu4AddrC[3] >> 2));    /* 0x4fc   // z */
	}

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigAcsTop(unsigned char u1AcsTop)
{
	VQ_VDO_W_PFLD((!u1AcsTop)); /* 0x430[2] */
	VQ_VDO_W_AFLD((!u1AcsTop)); /* 0x430[5] */

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigFusionDramMode(unsigned char u1FusionEnable, unsigned int AddrW)
{
	VQ_VDO_W_IFUSION_EN(u1FusionEnable);    /* 0x800[0] */

	/* AddrW = (AddrW) ? PHYSICAL(AddrW) : 0;    //check */

	if (u1FusionEnable) {

		VQ_VDO_W_fusion_flag_addr_base(AddrW / 16);   /* 0x870[27:0] */
		VQ_VDO_W_da_flag_waddr_hi_limit(((AddrW + MA8F_BUFFER_TOTAL_SIZE)) / 16);    /* 0x874[27:0] */
		VQ_VDO_W_da_flag_waddr_lo_limit(AddrW / 16);   /* 0x878[27:0] */
	}

	VQ_VDO_W_en_lmw(u1FusionEnable);    /* 0x870[28] */
	VQ_VDO_W_en_lmr(u1FusionEnable);    /* 0x870[29] */

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigDIThreshold(
		unsigned short u2CtThrehold, unsigned short u2MovingThrehold, unsigned short u2Pd_Threhold)
{
	VQ_VDO_W_CT_THRD(u2CtThrehold);     /* 0x438[15:8] */
	VQ_VDO_W_MTHRD(u2MovingThrehold);   /* 0x438[23:16] */
	VQ_VDO_W_PD_COMB_TH(u2Pd_Threhold); /* 0x4C4[22:16] */

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigVerticalChromaDetect(unsigned char bIsOn, unsigned char bRange , unsigned int u4Threhold)
{
	VQ_VDO_W_chroma_multi_burst_en(bIsOn);              /* 0xFF4[18] */
	VQ_VDO_W_chroma_multi_burst_pixel_sel(bRange);      /* 0xFF4[19] */
	VQ_VDO_W_chroma_multi_burst_threshold(u4Threhold);  /* 0xFF4[31:24] */

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigSharpPrePara(unsigned char bypass)
{
	VQ_VDO_W_BYPASS_SHARP(bypass);  /* 0x1B8[24] */

	return VQ_RET_OK;
}

static int iVQ_Vdo_ConfigBuffer(unsigned char u1EnableBuffer)
{
	if (0 == u1EnableBuffer) {

		if ((0 != _u4VdoFusionBufVa) && (0 != _u4VdoFusionBufMva)) {

			if (m4u_dealloc_mva(BDP_VDO, _u4VdoFusionBufVa, MA8F_BUFFER_TOTAL_SIZE, _u4VdoFusionBufMva)) {

				VQ_Printf(VQ_LOG_ERROR, "[E] m4u_dealloc_mva fail, Va = 0x%x, Mva= 0x%x\n",
					_u4VdoFusionBufVa, _u4VdoFusionBufMva);

				VQ_ASSERT;
				return VQ_RET_ERR_EXCEPTION;
			}

			vfree(_u4VdoFusionBufVa);

		} else if (0 == (_u4VdoFusionBufVa && _u4VdoFusionBufMva)) {

			VQ_Printf(VQ_LOG_ERROR, "[E] VdoFusionBuf exception, Va = 0x%x, Mva= 0x%x\n",
				_u4VdoFusionBufVa, _u4VdoFusionBufMva);

			VQ_ASSERT;
			return VQ_RET_ERR_EXCEPTION;
		}

	} else {

		if ((0 == _u4VdoFusionBufVa) && (0 == _u4VdoFusionBufMva)) {

			_u4VdoFusionBufVa = (unsigned int)vmalloc(MA8F_BUFFER_TOTAL_SIZE);

			if (0 == _u4VdoFusionBufVa) {

				VQ_Printf(VQ_LOG_ERROR, "[E] VdoFusionBufVa vmalloc fail\n");

				VQ_ASSERT;
				return VQ_RET_ERR_EXCEPTION;

			} else {

				if (m4u_alloc_mva(
						BDP_VDO,
						_u4VdoFusionBufVa,
						MA8F_BUFFER_TOTAL_SIZE,
						0,
						0,
						&_u4VdoFusionBufMva)) {

					VQ_Printf(VQ_LOG_ERROR, "[E] m4u_alloc_mva fail, Va = 0x%x\n",
						_u4VdoFusionBufVa);

					VQ_ASSERT;
					return VQ_RET_ERR_EXCEPTION;

				} else {

					VQ_Printf(VQ_LOG_DEFAULT, "[D] FusionBuf alloc success, Va = 0x%x, Mva= 0x%x\n",
						_u4VdoFusionBufVa, _u4VdoFusionBufMva);
				}
			}

		} else if (0 == (_u4VdoFusionBufVa && _u4VdoFusionBufMva)) {

			VQ_Printf(VQ_LOG_ERROR, "[E] VdoFusionBuf exception, Va = 0x%x, Mva= 0x%x\n",
				_u4VdoFusionBufVa, _u4VdoFusionBufMva);

			VQ_ASSERT;
			return VQ_RET_ERR_EXCEPTION;
		}
	}

	return VQ_RET_OK;
}

/******************************* global function ******************************/

int iVQ_Vdo_SetParam(struct VQ_VDO_PARAM_T *prParam)
{
	unsigned int  au4AddrY[4];
	unsigned int  au4AddrC[4];

	iVQ_Vdo_ConfigBase(prParam);

	iVQ_Vdo_ConfigDIMode(prParam->eDiMode);

	iVQ_Vdo_ConfigSrcsize(prParam->u4Width, prParam->u4Height);

	iVQ_Vdo_ConfigHwSramUtil(prParam->eDiMode, prParam->u4Width);

	iVQ_Vdo_ConfigStartLine(prParam->eDiMode, prParam->u4Width, VDO_VERTICAL_FILTER_4TAP, prParam->eSrcFmt);

	if (0 != prParam->u1CurrentIsTop) {

		au4AddrY[0] = prParam->u4AddrYPrev;
		au4AddrY[1] = prParam->u4AddrYCurr;
		au4AddrY[2] = prParam->u4AddrYCurr;
		au4AddrY[3] = prParam->u4AddrYNext;

		au4AddrC[0] = prParam->u4AddrCbcrPrev;
		au4AddrC[1] = prParam->u4AddrCbcrCurr;
		au4AddrC[2] = prParam->u4AddrCbcrCurr;
		au4AddrC[3] = prParam->u4AddrCbcrNext;

	} else {

		au4AddrY[0] = prParam->u4AddrYPrev;
		au4AddrY[1] = prParam->u4AddrYPrev;
		au4AddrY[2] = prParam->u4AddrYCurr;
		au4AddrY[3] = prParam->u4AddrYCurr;

		au4AddrC[0] = prParam->u4AddrCbcrPrev;
		au4AddrC[1] = prParam->u4AddrCbcrPrev;
		au4AddrC[2] = prParam->u4AddrCbcrCurr;
		au4AddrC[3] = prParam->u4AddrCbcrCurr;
	}

	iVQ_Vdo_ConfigDeintWXYZ(au4AddrY, au4AddrC);

	iVQ_Vdo_ConfigAcsTop(prParam->u1CurrentIsTop);

	if (VDO_FUSION_MODE == prParam->eDiMode) {

		iVQ_Vdo_ConfigBuffer(1);

		iVQ_Vdo_ConfigFusionDramMode(1, _u4VdoFusionBufMva);

	} else {

		iVQ_Vdo_ConfigBuffer(0);

		iVQ_Vdo_ConfigFusionDramMode(0, _u4VdoFusionBufMva);
	}

	iVQ_Vdo_ConfigDIThreshold(_u2VdoContrastThreshold, _u2VdoMovingThreshold, _u2VdoPullDownCombThreshold);

	iVQ_Vdo_ConfigVerticalChromaDetect(prParam->u1EnableVerticalChromaDetect, 0, 0);

	iVQ_Vdo_ConfigSharpPrePara(!(prParam->u1EnablePreSharp));

	iVQ_Vdo_ConfigMA8F(0, 0, 0, 0);

	return VQ_RET_OK;
}

int iVQ_Vdo_Enable(unsigned char u1Enable)
{
	iVQ_Vdo_ConfigBuffer(u1Enable);
}

