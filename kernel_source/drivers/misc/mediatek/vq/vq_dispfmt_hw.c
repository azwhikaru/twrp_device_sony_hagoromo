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

#include "vq_dispfmt_reg.h"
#include "vq_dispfmt_hw.h"

/*********************************** define ***********************************/

/* HVTOTAL */
/* 480P: 950X530 */
/* 576P: 950X650 */
/* 720P: 1650X750 */
/* 1080P:2200X1125 */

#define FMT_TOTAL_PIXEL_720_480_H       950
#define FMT_TOTAL_PIXEL_720_480_V       530
#define FMT_TOTAL_PIXEL_720_576_H       950
#define FMT_TOTAL_PIXEL_720_576_V       650
#define FMT_TOTAL_PIXEL_1280_720_H      1650
#define FMT_TOTAL_PIXEL_1280_720_V      750
#define FMT_TOTAL_PIXEL_1920_1080_H     2200
#define FMT_TOTAL_PIXEL_1920_1080_V     1125

#define FMT_SIZE_720_480_H              720
#define FMT_SIZE_720_480_V              480
#define FMT_SIZE_720_576_H              720
#define FMT_SIZE_720_576_V              576
#define FMT_SIZE_1280_720_H             1280
#define FMT_SIZE_1280_720_V             720
#define FMT_SIZE_1920_1080_H            1920
#define FMT_SIZE_1920_1080_V            1080

/* active */
/* 480P:  H 165 ~ 884     V  45 ~ (45 + 479) */
/* 576P:  H 165 ~ 884     V  45 ~ (45 + 575) */
/* 720P:  H 185 ~ 1646    V  26 ~ (26 + 719) */
/* 1080P: H 193 ~ 2112    V  32 ~ (32 + 1079) */

#define FMT_ACTIVE_720_480_H_START      165     /* resolved */
#define FMT_ACTIVE_720_480_H_END        (FMT_ACTIVE_720_480_H_START + (FMT_SIZE_720_480_H - 1))
#define FMT_ACTIVE_720_480_V_START      45      /* resolved */
#define FMT_ACTIVE_720_480_V_END        (FMT_ACTIVE_720_480_V_START + (FMT_SIZE_720_480_V - 1))

#define FMT_ACTIVE_720_576_H_START      165     /* resolved */
#define FMT_ACTIVE_720_576_H_END        (FMT_ACTIVE_720_576_H_START + (FMT_SIZE_720_576_H - 1))
#define FMT_ACTIVE_720_576_V_START      45      /* resolved */
#define FMT_ACTIVE_720_576_V_END        (FMT_ACTIVE_720_576_V_START + (FMT_SIZE_720_576_V - 1))

#define FMT_ACTIVE_1280_720_H_START     185
#define FMT_ACTIVE_1280_720_H_END       (FMT_ACTIVE_1280_720_H_START + (FMT_SIZE_1280_720_H - 1))   /* bug */
#define FMT_ACTIVE_1280_720_V_START     24
#define FMT_ACTIVE_1280_720_V_END       (FMT_ACTIVE_1280_720_V_START + (FMT_SIZE_1280_720_V - 1))

#define FMT_ACTIVE_1920_1080_H_START    193
#define FMT_ACTIVE_1920_1080_H_END      (FMT_ACTIVE_1920_1080_H_START + (FMT_SIZE_1920_1080_H - 1))
#define FMT_ACTIVE_1920_1080_V_START    32
#define FMT_ACTIVE_1920_1080_V_END      (FMT_ACTIVE_1920_1080_V_START + (FMT_SIZE_1920_1080_V - 1))

struct FMT_TIMING_INFO_T {
	#if 1   /* !VQ_WH_TIMING */
	unsigned int  u4SizeH;
	unsigned int  u4SizeV;
	#endif
	unsigned int  u4HStart;
	#if 1   /* !VQ_WH_TIMING */
	unsigned int  u4HEnd;
	#endif
	unsigned int  u4VStart;
	#if 1   /* !VQ_WH_TIMING */
	unsigned int  u4VEnd;
	#endif
	unsigned int  u4TotalPixelH;
	unsigned int  u4TotalPixelV;
};

/*********************************** variable *********************************/

static struct FMT_TIMING_INFO_T _arFmtTimingInfo[VQ_TIMING_TYPE_MAX] = {
	{
		FMT_SIZE_720_480_H,		FMT_SIZE_720_480_V,
		FMT_ACTIVE_720_480_H_START,	FMT_ACTIVE_720_480_H_END,
		FMT_ACTIVE_720_480_V_START,	FMT_ACTIVE_720_480_V_END,
		FMT_TOTAL_PIXEL_720_480_H,	FMT_TOTAL_PIXEL_720_480_V
	},
	{
		FMT_SIZE_720_576_H,		FMT_SIZE_720_576_V,
		FMT_ACTIVE_720_576_H_START,	FMT_ACTIVE_720_576_H_END,
		FMT_ACTIVE_720_576_V_START,	FMT_ACTIVE_720_576_V_END,
		FMT_TOTAL_PIXEL_720_576_H,	FMT_TOTAL_PIXEL_720_576_V
	},
	{
		FMT_SIZE_1280_720_H,		FMT_SIZE_1280_720_V,
		FMT_ACTIVE_1280_720_H_START,	FMT_ACTIVE_1280_720_H_END,
		FMT_ACTIVE_1280_720_V_START,	FMT_ACTIVE_1280_720_V_END,
		FMT_TOTAL_PIXEL_1280_720_H,	FMT_TOTAL_PIXEL_1280_720_V
	},
	{
		FMT_SIZE_1920_1080_H,		FMT_SIZE_1920_1080_V,
		FMT_ACTIVE_1920_1080_H_START,	FMT_ACTIVE_1920_1080_H_END,
		FMT_ACTIVE_1920_1080_V_START,	FMT_ACTIVE_1920_1080_V_END,
		FMT_TOTAL_PIXEL_1920_1080_H,	FMT_TOTAL_PIXEL_1920_1080_V
	}
};

#if VQ_LOOP_TEST
static unsigned int _u4VqLoop;
#endif

static unsigned int _u4VqRegIrqWrSt;
static unsigned int _u4EventWqIrqTimeoutCount;
static wait_queue_head_t _VqEventWqIrqHandle;
static atomic_t _VqEventWqIrqFlag = ATOMIC_INIT(0);

/****************************** internal function *****************************/

static irqreturn_t VQ_Dispfmt_IrqHandler(int irq, void *dev_id)
{
	unsigned int *pu4RegClr = VQ_IO_BASE + 0x0004;

	*pu4RegClr |= 0xF;

	#if VQ_TIME_CHECK
	VQ_TIME_REC(7);
	#endif

	atomic_set(&_VqEventWqIrqFlag, 1);
	wake_up_interruptible(&_VqEventWqIrqHandle);

	return IRQ_HANDLED;
}

static int iVQ_Dispfmt_ConfigDispSysCfg(struct VQ_DISPFMT_PARAM_T *prParam)
{
	/* enable m4u */
	VQ_DISPSYSCFG_W_awmmu(1);
	VQ_DISPSYSCFG_W_armmu(1);

	VQ_DISPSYSCFG_W_BDP_DISPSYS_CG_CLR0(0xffffffff);
	VQ_DISPSYSCFG_W_BDP_DISPSYS_CG_CLR1(0xffffffff);

	if (prParam->u1NrEnable) {

		/*for keep brace*/
		VQ_DISPSYSCFG_W_disp1_src_sel(1);

	} else {

		/*for keep brace*/
		VQ_DISPSYSCFG_W_disp1_src_sel(0);
	}

	return VQ_RET_OK;
}

static int iVQ_Dispfmt_ConfigNr(struct VQ_DISPFMT_PARAM_T *prParam)
{
	if (prParam->u1NrEnable) {

		/* 0x010 */
		VQ_DISPFMT_W_BYPASS_NR(0);

		/* 0x080 */
		VQ_DISPFMT_W_ADJ_SYNC_EN(1);
		VQ_DISPFMT_W_NR_ADJ_FORWARD(0);
		VQ_DISPFMT_W_TOGGLE_OPTION(0);
		VQ_DISPFMT_W_HSYNC_DELAY(0x38);
		VQ_DISPFMT_W_VSYNC_DELAY(0x5);

		/* 0x084 */
		#if VQ_WH_TIMING
		VQ_DISPFMT_W_VSYNC_END(_arFmtTimingInfo[prParam->eTimingType].u4VStart + prParam->u4Height - 1);
		#else
		VQ_DISPFMT_W_VSYNC_END(_arFmtTimingInfo[prParam->eTimingType].u4VEnd);
		#endif
		VQ_DISPFMT_W_VSYNC_START(_arFmtTimingInfo[prParam->eTimingType].u4VStart - 1);
		VQ_DISPFMT_W_VSYNC_POLAR(1);
		VQ_DISPFMT_W_HSYNC_POLAR(0);
		VQ_DISPFMT_W_FIELD_POLAR(0);
		VQ_DISPFMT_W_DE_SELF(0);
		VQ_DISPFMT_W_ODD_V_START_OPT(0);
		VQ_DISPFMT_W_ODD_V_END_OPT(0);
		VQ_DISPFMT_W_EVEN_V_START_OPT(1);
		VQ_DISPFMT_W_EVEN_V_END_OPT(1);

		/* 0x088 */
		#if VQ_WH_TIMING
		VQ_DISPFMT_W_HOR_END(_arFmtTimingInfo[prParam->eTimingType].u4HStart + prParam->u4Width - 1 + 7);
		#else
		VQ_DISPFMT_W_HOR_END(_arFmtTimingInfo[prParam->eTimingType].u4HEnd + 7);
		#endif
		VQ_DISPFMT_W_HOR_START(_arFmtTimingInfo[prParam->eTimingType].u4HStart + 7);

		/* 0x08C */
		#if VQ_WH_TIMING
		VQ_DISPFMT_W_VO_END(_arFmtTimingInfo[prParam->eTimingType].u4VStart + prParam->u4Height - 1);
		#else
		VQ_DISPFMT_W_VO_END(_arFmtTimingInfo[prParam->eTimingType].u4VEnd);
		#endif
		VQ_DISPFMT_W_VO_START(_arFmtTimingInfo[prParam->eTimingType].u4VStart);

		/* 0x090 */
		#if VQ_WH_TIMING
		VQ_DISPFMT_W_VE_END(_arFmtTimingInfo[prParam->eTimingType].u4VStart + prParam->u4Height - 1);
		#else
		VQ_DISPFMT_W_VE_END(_arFmtTimingInfo[prParam->eTimingType].u4VEnd);
		#endif
		VQ_DISPFMT_W_VE_START(_arFmtTimingInfo[prParam->eTimingType].u4VStart);

	} else {

		/* 0x010 */
		VQ_DISPFMT_W_BYPASS_NR(1);

		/* 0x080 */
		VQ_DISPFMT_W_ADJ_SYNC_EN(0);
		VQ_DISPFMT_W_NR_ADJ_FORWARD(0);
		VQ_DISPFMT_W_TOGGLE_OPTION(0);
		VQ_DISPFMT_W_HSYNC_DELAY(0);
		VQ_DISPFMT_W_VSYNC_DELAY(0);

		/* 0x084 */
		VQ_DISPFMT_W_VSYNC_END(0);
		VQ_DISPFMT_W_VSYNC_START(0);
		VQ_DISPFMT_W_VSYNC_POLAR(0);
		VQ_DISPFMT_W_HSYNC_POLAR(0);
		VQ_DISPFMT_W_FIELD_POLAR(0);
		VQ_DISPFMT_W_DE_SELF(0);
		VQ_DISPFMT_W_ODD_V_START_OPT(0);
		VQ_DISPFMT_W_ODD_V_END_OPT(0);
		VQ_DISPFMT_W_EVEN_V_START_OPT(0);
		VQ_DISPFMT_W_EVEN_V_END_OPT(0);

		/* 0x088 */
		VQ_DISPFMT_W_HOR_END(0);
		VQ_DISPFMT_W_HOR_START(0);

		/* 0x08C */
		VQ_DISPFMT_W_VO_END(0);
		VQ_DISPFMT_W_VO_START(0);

		/* 0x090 */
		VQ_DISPFMT_W_VE_END(0);
		VQ_DISPFMT_W_VE_START(0);
	}

	return VQ_RET_OK;
}

static int iVQ_Dispfmt_ConfigTimingType(struct VQ_DISPFMT_PARAM_T *prParam)
{
	/* 0x000 */
	VQ_DISPFMT_W_HOR_VALID_STAR(_arFmtTimingInfo[prParam->eTimingType].u4HStart + 7);
	#if VQ_WH_TIMING
	VQ_DISPFMT_W_HOR_VALID_END(_arFmtTimingInfo[prParam->eTimingType].u4HStart + prParam->u4Width - 1 + 7);
	#else
	VQ_DISPFMT_W_HOR_VALID_END(_arFmtTimingInfo[prParam->eTimingType].u4HEnd + 7);
	#endif

	/* 0x004 */
	#if VQ_WH_TIMING
	VQ_DISPFMT_W_NR_IN_VSYNC_END(_arFmtTimingInfo[prParam->eTimingType].u4VStart + prParam->u4Height - 1);
	#else
	VQ_DISPFMT_W_NR_IN_VSYNC_END(_arFmtTimingInfo[prParam->eTimingType].u4VEnd);
	#endif
	VQ_DISPFMT_W_NR_IN_VSYNC_START(_arFmtTimingInfo[prParam->eTimingType].u4VStart - 1);
	VQ_DISPFMT_W_NR_IN_VSYNC_POLAR(1);
	VQ_DISPFMT_W_NR_IN_HSYNC_POLAR(1);
	VQ_DISPFMT_W_NR_IN_FIELD_POLAR(0);
	VQ_DISPFMT_W_NR_IN_DE_SELF(0);
	VQ_DISPFMT_W_NR_IN_ODD_V_START_OPT(0);
	VQ_DISPFMT_W_NR_IN_ODD_V_END_OPT(0);
	VQ_DISPFMT_W_NR_IN_EVEN_V_START_OPT(1);
	VQ_DISPFMT_W_NR_IN_EVEN_V_END_OPT(1);

	/* 0x008 */
	VQ_DISPFMT_W_HOR_NR_VALID_STAR(_arFmtTimingInfo[prParam->eTimingType].u4HStart + 7);
	#if VQ_WH_TIMING
	VQ_DISPFMT_W_HOR_NR_VALID_END(_arFmtTimingInfo[prParam->eTimingType].u4HStart + prParam->u4Width - 1 + 7);
	#else
	VQ_DISPFMT_W_HOR_NR_VALID_END(_arFmtTimingInfo[prParam->eTimingType].u4HEnd + 7);
	#endif

	/* 0x00C */
	VQ_DISPFMT_W_NR_IN_HOR_STAT(_arFmtTimingInfo[prParam->eTimingType].u4HStart + 7);
	#if VQ_WH_TIMING
	VQ_DISPFMT_W_NR_IN_HOR_END(_arFmtTimingInfo[prParam->eTimingType].u4HStart + prParam->u4Width - 1 + 7);
	#else
	VQ_DISPFMT_W_NR_IN_HOR_END(_arFmtTimingInfo[prParam->eTimingType].u4HEnd + 7);
	#endif

	/* 0x014 */
	VQ_DISPFMT_W_COLOR_BAR_ON(prParam->rPattern.u1Enable);
	if (prParam->rPattern.u1Enable) {

		VQ_DISPFMT_W_COLOR_BAR_TYPE(prParam->rPattern.u2Type);
		VQ_DISPFMT_W_COLOR_BAR_WIDHT(prParam->rPattern.u2Width);
		VQ_DISPFMT_W_ENABLE_422_444(1);

	} else {

		VQ_DISPFMT_W_ENABLE_422_444(0);
	}

	/* 0x094 */
	VQ_DISPFMT_W_HSYNWIDTH(0x20);
	VQ_DISPFMT_W_VSYNWIDTH(0xd);
	VQ_DISPFMT_W_HD_TP((VQ_TIMING_TYPE_720P == prParam->eTimingType)?1:0);
	VQ_DISPFMT_W_HD_ON((VQ_TIMING_TYPE_720P <= prParam->eTimingType)?1:0);
	VQ_DISPFMT_W_PRGS(1);
	VQ_DISPFMT_W_PRGS_AUTOFLD(0);
	VQ_DISPFMT_W_PRGS_INVFLD(0);
	VQ_DISPFMT_W_YUV_RST_OPT(0);
	VQ_DISPFMT_W_PRGS_FLD(0);
	VQ_DISPFMT_W_NEW_SD_144MHz(0);
	VQ_DISPFMT_W_NEW_SD_MODE(0);
	VQ_DISPFMT_W_NEW_SD_USE_EVEN(0);
	VQ_DISPFMT_W_TVMODE(0);

	/* 0x09C */
	#if VQ_WH_TIMING
	VQ_DISPFMT_W_PXLLEN(prParam->u4Width);
	#else
	VQ_DISPFMT_W_PXLLEN(_arFmtTimingInfo[prParam->eTimingType].u4SizeH);
	#endif

	VQ_DISPFMT_W_RIGHT_SKIP(0);

	/* 0x0A0 */
	#if VQ_WH_TIMING
	VQ_DISPFMT_W_HACTEND(_arFmtTimingInfo[prParam->eTimingType].u4HStart + prParam->u4Width - 1);
	#else
	VQ_DISPFMT_W_HACTEND(_arFmtTimingInfo[prParam->eTimingType].u4HEnd);
	#endif
	VQ_DISPFMT_W_HACTBGN(_arFmtTimingInfo[prParam->eTimingType].u4HStart);

	/* 0x0A4 */
	#if VQ_WH_TIMING
	VQ_DISPFMT_W_VOACTEND(_arFmtTimingInfo[prParam->eTimingType].u4VStart + prParam->u4Height - 1);
	#else
	VQ_DISPFMT_W_VOACTEND(_arFmtTimingInfo[prParam->eTimingType].u4VEnd);
	#endif
	VQ_DISPFMT_W_VOACTBGN(_arFmtTimingInfo[prParam->eTimingType].u4VStart);
	VQ_DISPFMT_W_HIDE_OST(0);

	/* 0x0A8 */
	#if VQ_WH_TIMING
	VQ_DISPFMT_W_VEACTEND(_arFmtTimingInfo[prParam->eTimingType].u4VStart + prParam->u4Height - 1);
	#else
	VQ_DISPFMT_W_VEACTEND(_arFmtTimingInfo[prParam->eTimingType].u4VEnd);
	#endif
	VQ_DISPFMT_W_VEACTBGN(_arFmtTimingInfo[prParam->eTimingType].u4VStart);
	VQ_DISPFMT_W_HIDE_EST(0);

	/* 0x0AC */
	VQ_DISPFMT_W_VDO1_EN(1);
	VQ_DISPFMT_W_FMTM(1);
	VQ_DISPFMT_W_HPOR(0);
	VQ_DISPFMT_W_VPOR(0);
	VQ_DISPFMT_W_C_RST_SEL(0);
	VQ_DISPFMT_W_PXLSEL(0);
	VQ_DISPFMT_W_FTRST(0);
	VQ_DISPFMT_W_FTRST(1);
	VQ_DISPFMT_W_SHVSYN(0);
	VQ_DISPFMT_W_SYN_DEL(0);
	VQ_DISPFMT_W_UVSW(0);
	VQ_DISPFMT_W_BLACK(0);
	VQ_DISPFMT_W_PFOFF(1);
	VQ_DISPFMT_W_HW_OPT(0);

	/* 0x0B0 */
	VQ_DISPFMT_W_Horizontal_Scaling(0x01000001);

	/* 0x0B4 */
	VQ_DISPFMT_W_BIY(8);
	VQ_DISPFMT_W_BICB(8);
	VQ_DISPFMT_W_BICR(8);
	VQ_DISPFMT_W_PF2OFF(0);
	VQ_DISPFMT_W_HIDE_L(0);

	/* 0x0B8 */
	VQ_DISPFMT_W_BGY(1);
	VQ_DISPFMT_W_BGCB(2);
	VQ_DISPFMT_W_BGCR(3);

	/* vFmt_WriteReg(0x1c0010bc, 0x00000000); */

	/* 0x0C8 */
	VQ_DISPFMT_W_NEW_SCL_MODE_CTRL(0x00000000);

	/* 0x0CC */
	VQ_DISPFMT_W_Dispfmt_Configure(0x00100000);

	/* 0x0D0 */
	VQ_DISPFMT_W_0D0_11_0(_arFmtTimingInfo[prParam->eTimingType].u4TotalPixelV);
	VQ_DISPFMT_W_0D0_28_16(_arFmtTimingInfo[prParam->eTimingType].u4TotalPixelH);
	VQ_DISPFMT_W_0D0_31_31(1);

	/* 0x0D4 */
	VQ_DISPFMT_W_V_TOTAL(_arFmtTimingInfo[prParam->eTimingType].u4TotalPixelV);
	VQ_DISPFMT_W_H_TOTAL(_arFmtTimingInfo[prParam->eTimingType].u4TotalPixelH);
	VQ_DISPFMT_W_ADJ_T(1);

	VQ_DISPFMT_W_Multi_Ratio(0x00000000);

	/* vFmt_WriteReg(0x1c0010f4, 0x00000000); */

	VQ_DISPFMT_W_C110(0);
	VQ_DISPFMT_W_OLD_CHROMA(0);
	VQ_DISPFMT_W_EN_235_255(0);
	VQ_DISPFMT_W_FROM_235_TO_255(0);

	/* vFmt_WriteReg(0x1c0013b0, 0x00000001);  //check */

	return VQ_RET_OK;
}

/******************************* global function ******************************/

int iVQ_Dispfmt_SetParam(struct VQ_DISPFMT_PARAM_T *prParam)
{
	iVQ_Dispfmt_ConfigDispSysCfg(prParam);
	iVQ_Dispfmt_ConfigNr(prParam);
	iVQ_Dispfmt_ConfigTimingType(prParam);

	return VQ_RET_OK;
}

int iVQ_Dispfmt_ClkOn(void)
{
	VQ_Printf(VQ_LOG_FLOW, "[F] start clock on including spm_mtcmos_ctrl_bdpsys(STA_POWER_ON)\n");

	spm_mtcmos_ctrl_bdpsys(STA_POWER_ON);

	#if VQ_CLK_RESET
		VQ_REG_WRITE_MASK(BDP_DISPSYS_SW_RST_B, 0x00000000, 0x00000017);    /* 0x1c000138 */
	#endif

	/*VQ_REG_WRITE_MASK(BDP_DISPSYS_CG_CON0, 0x00000000, 0xFFFE0000);     clock on */
	enable_clock(MT_CG_DISPFMT_27M, "vq");
	enable_clock(MT_CG_DISPFMT_27M_VDOUT, "vq");
	enable_clock(MT_CG_DISPFMT_27_74_74, "vq");
	enable_clock(MT_CG_DISPFMT_2FS, "vq");
	enable_clock(MT_CG_DISPFMT_2FS_2FS74_148, "vq");
	enable_clock(MT_CG_DISPFMT_BCLK, "vq");
	enable_clock(MT_CG_VDO_DRAM, "vq");
	enable_clock(MT_CG_VDO_2FS, "vq");
	enable_clock(MT_CG_VDO_BCLK, "vq");
	enable_clock(MT_CG_WR_CHANNEL_DI_PXL, "vq");
	enable_clock(MT_CG_WR_CHANNEL_DI_DRAM, "vq");
	enable_clock(MT_CG_WR_CHANNEL_DI_BCLK, "vq");
	enable_clock(MT_CG_NR_PXL, "vq");
	enable_clock(MT_CG_NR_DRAM, "vq");
	enable_clock(MT_CG_NR_BCLK, "vq");

	VQ_REG_WRITE_MASK(BDP_DISPSYS_HW_DCM_DIS0, 0x000007FF, 0x000007FF); /* disable dcm clock */

	VQ_DISPSYSCFG_W_DISP_CLK_CONFIG1(0x1);  /* nr clk */

	#if VQ_CLK_RESET
		VQ_REG_WRITE_MASK(BDP_DISPSYS_SW_RST_B, 0x00000017, 0x00000017);    /* 0x1c000138 */
	#endif

	VQ_Printf(VQ_LOG_FLOW, "[F] end clock on\n");
}

int iVQ_Dispfmt_ClkOff(void)
{
#if VQ_CLK_OFF_AUTO
	VQ_Printf(VQ_LOG_FLOW, "[F] start clock off\n");

	#if VQ_CLK_RESET
	VQ_REG_WRITE_MASK(BDP_DISPSYS_SW_RST_B, 0x00000000, 0x00000017);    /* 0x1c000138 */
	#endif

	/*VQ_REG_WRITE_MASK(BDP_DISPSYS_CG_CON0, 0xFFFE0000, 0xFFFE0000);	clock off */
	disable_clock(MT_CG_DISPFMT_27M, "vq");
	disable_clock(MT_CG_DISPFMT_27M_VDOUT, "vq");
	disable_clock(MT_CG_DISPFMT_27_74_74, "vq");
	disable_clock(MT_CG_DISPFMT_2FS, "vq");
	disable_clock(MT_CG_DISPFMT_2FS_2FS74_148, "vq");
	disable_clock(MT_CG_DISPFMT_BCLK, "vq");
	disable_clock(MT_CG_VDO_DRAM, "vq");
	disable_clock(MT_CG_VDO_2FS, "vq");
	disable_clock(MT_CG_VDO_BCLK, "vq");
	disable_clock(MT_CG_WR_CHANNEL_DI_PXL, "vq");
	disable_clock(MT_CG_WR_CHANNEL_DI_DRAM, "vq");
	disable_clock(MT_CG_WR_CHANNEL_DI_BCLK, "vq");
	disable_clock(MT_CG_NR_PXL, "vq");
	disable_clock(MT_CG_NR_DRAM, "vq");
	disable_clock(MT_CG_NR_BCLK, "vq");

	#if VQ_CLK_RESET
	VQ_REG_WRITE_MASK(BDP_DISPSYS_SW_RST_B, 0x00000017, 0x00000017);    /* 0x1c000138 */
	#endif

	/*spm_mtcmos_ctrl_bdpsys(STA_POWER_DOWN);*/

	VQ_Printf(VQ_LOG_FLOW, "[F] end clock off\n");
#endif
}

int iVQ_Dispfmt_IrqReg(unsigned char u1Enable)
{
	if ((0 == _u4VqRegIrqWrSt) && (0 != u1Enable)) {

		if (request_irq(BDP_WR_CHANNEL_DI_IRQ_B_ID,
				(irq_handler_t)VQ_Dispfmt_IrqHandler,
				IRQF_TRIGGER_LOW,
				"mtk_vq",
				NULL)) {

			VQ_Printf(VQ_LOG_ERROR, "[E] reg irq[%d] fail\n", BDP_WR_CHANNEL_DI_IRQ_B_ID);

			return VQ_RET_ERR_EXCEPTION;

		} else {

			_u4VqRegIrqWrSt = 1;

			init_waitqueue_head(&_VqEventWqIrqHandle);

			VQ_Printf(VQ_LOG_DEFAULT, "[D] reg irq[%d] success, init EventWqIrqHandle[%d]\n",
				BDP_WR_CHANNEL_DI_IRQ_B_ID, _VqEventWqIrqHandle);
		}

	} else if ((1 == _u4VqRegIrqWrSt) && (0 == u1Enable)) {

		free_irq(BDP_WR_CHANNEL_DI_IRQ_B_ID, 0);

		_u4VqRegIrqWrSt = 0;

		VQ_Printf(VQ_LOG_DEFAULT, "[D] free irq[%d] success\n", BDP_WR_CHANNEL_DI_IRQ_B_ID);

	}

	return VQ_RET_OK;
}

int iVQ_Dispfmt_Triggle(struct VQ_DISPFMT_PARAM_T *prParam)
{
#if VQ_LOOP_TEST
	VQ_Printf(VQ_LOG_CTP, "[CTP] will triggle loop[%d]\n", ++_u4VqLoop);
#else
	VQ_Printf(VQ_LOG_FLOW, "[F] will triggle\n");
#endif

	#if VQ_TIME_CHECK
	VQ_TIME_REC(6);
	#endif

	/* 0x018 */
	VQ_DISPFMT_W_DI_AGENT_TRIG(1);
	VQ_DISPFMT_W_DI_AGENT_TRIG(0);

#if VQ_WAIT_IRQ

	while (1) {

		if (wait_event_interruptible_timeout(
				_VqEventWqIrqHandle, atomic_read(&_VqEventWqIrqFlag), HZ / 10) == 0) {

			_u4EventWqIrqTimeoutCount++;

			VQ_Printf(VQ_LOG_ERROR,
				"[E] Irq timeout, Id[%d], f[%d], c[%d], St[%d], Hdl[0x%x]\n",
				BDP_WR_CHANNEL_DI_IRQ_B_ID,
				_VqEventWqIrqFlag,
				_u4EventWqIrqTimeoutCount,
				_u4VqRegIrqWrSt,
				_VqEventWqIrqHandle);

		} else {

			#if VQ_TIME_CHECK
			VQ_TIME_REC(8);
			#endif

			VQ_Printf(VQ_LOG_IRQ,
				"[IRQ] Irq OK, Id[%d], St[%d], Handle[%d], Flag[%d], count[%d], irq_reg[0x%x]\n",
				BDP_WR_CHANNEL_DI_IRQ_B_ID,
				_u4VqRegIrqWrSt,
				_VqEventWqIrqHandle,
				_VqEventWqIrqFlag,
				_u4EventWqIrqTimeoutCount,
				*((unsigned int *)(VQ_IO_BASE + 0x0000)));

			_u4EventWqIrqTimeoutCount = 0;

			break;
		}
	}

	atomic_set(&_VqEventWqIrqFlag, 0);

#else

	{
		volatile unsigned int u4RegIrq = 0;
		volatile unsigned int u4RegClk = 0;
		volatile unsigned int u4RegDcm = 0;
		unsigned int u4Loop   = 0;
		unsigned int *pu4RegIrq = VQ_IO_BASE + 0x0000;
		unsigned int *pu4RegClr = VQ_IO_BASE + 0x0004;
		unsigned int *pu4RegClk = VQ_IO_BASE + 0x0100;
		unsigned int *pu4RegDcm = VQ_IO_BASE + 0x0120;

		while (1) {

			u4RegIrq = *pu4RegIrq;
			u4RegClk = *pu4RegClk;
			u4RegDcm = *pu4RegDcm;

			if (0 == (u4RegIrq & (0x8))) {

				#if VQ_TIME_CHECK
				VQ_TIME_REC(7);
				#endif

				*pu4RegClr |= 0xF;

				#if VQ_CTP_TEST
				VQ_Printf(VQ_LOG_CTP,
					"[CTP] irq clr loop[%d], 0x%x = 0x%x, 0x%x = 0x%x, 0x%x = 0x%x, 0x%x = 0x%x\n",
					u4Loop,
					pu4RegIrq, u4RegIrq,
					pu4RegClr, *pu4RegClr,
					pu4RegClk, u4RegClk,
					pu4RegDcm, u4RegDcm);
				#else
				VQ_Printf(VQ_LOG_IRQ, "[IRQ] irq clr loop[%d], 0x%x = 0x%x, 0x%x = 0x%x\n",
					u4Loop,
					pu4RegIrq, u4RegIrq,
					pu4RegClr, *pu4RegClr);
				#endif

				break;
			}

			u4Loop++;

			if (0 == (u4Loop % 10000000)) {

				VQ_Printf(VQ_LOG_IRQ, "[IRQ] irq loop[%d], 0x%x = 0x%x, 0x%x = 0x%x, 0x%x = 0x%x\n",
					u4Loop,
					pu4RegIrq, u4RegIrq,
					pu4RegClk, u4RegClk,
					pu4RegDcm, u4RegDcm);
			}
		}

		#if VQ_TIME_CHECK
		VQ_TIME_REC(8);
		#endif

	}
#endif

	VQ_DISPFMT_W_DI_AGENT_TRIG(1);

	return VQ_RET_OK;
}

