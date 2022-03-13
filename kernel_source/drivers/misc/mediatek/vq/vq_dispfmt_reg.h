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


#ifndef VQ_DISPFMT_REG_H
#define VQ_DISPFMT_REG_H

#include "vq_def.h"



/* dispsys_cfg */
#define VQ_DISPSYSCFG_REG               (VQ_IO_BASE + 0x0000)

#define BDP_DISPYSY_DI_CONFIG           (VQ_DISPSYSCFG_REG + 0x008)
    #define VQ_DISPSYSCFG_W_disp1_src_sel(value) \
	VQ_REG_WRITE_BITS(BDP_DISPYSY_DI_CONFIG, BITS(2, 2), 2, value)

#define BDP_DISPSYS_DRAM_BRIDGE_CONFIG1 (VQ_DISPSYSCFG_REG + 0x010)
    #define VQ_DISPSYSCFG_W_awmmu(value) \
	VQ_REG_WRITE_BITS(BDP_DISPSYS_DRAM_BRIDGE_CONFIG1, BITS(7, 7), 7, value)
    #define VQ_DISPSYSCFG_W_armmu(value) \
	VQ_REG_WRITE_BITS(BDP_DISPSYS_DRAM_BRIDGE_CONFIG1, BITS(23, 23), 23, value)

#define BDP_DISPSYS_DISP_CLK_CONFIG1    (VQ_DISPSYSCFG_REG + 0x018)
    #define VQ_DISPSYSCFG_W_DISP_CLK_CONFIG1(value) \
	VQ_REG_WRITE_BITS(BDP_DISPSYS_DISP_CLK_CONFIG1, BITS(2, 0), 0, value)

#define BDP_DISPSYS_CG_CON0             (VQ_DISPSYSCFG_REG + 0x100)
    #define VQ_DISPSYSCFG_W_BDP_DISPSYS_CG_CON0(value) \
	VQ_REG_WRITE(BDP_DISPSYS_CG_CON0, value)

#define BDP_DISPSYS_CG_CLR0             (VQ_DISPSYSCFG_REG + 0x108)
    #define VQ_DISPSYSCFG_W_BDP_DISPSYS_CG_CLR0(value) \
	VQ_REG_WRITE(BDP_DISPSYS_CG_CLR0, value)

#define BDP_DISPSYS_CG_CLR1             (VQ_DISPSYSCFG_REG + 0x118)
    #define VQ_DISPSYSCFG_W_BDP_DISPSYS_CG_CLR1(value) \
	VQ_REG_WRITE(BDP_DISPSYS_CG_CLR1, value)

#define BDP_DISPSYS_HW_DCM_DIS0         (VQ_DISPSYSCFG_REG + 0x120)
    #define VQ_DISPSYSCFG_W_BDP_DISPSYS_HW_DCM_DIS0(value) \
	VQ_REG_WRITE(BDP_DISPSYS_HW_DCM_DIS0, value)

#define BDP_DISPSYS_SW_RST_B            (VQ_DISPSYSCFG_REG + 0x138)

/* dispfmt */
#define VQ_DISPFMT_REG                  (VQ_IO_BASE + 0x1000)

#define FMT_VALID_READY                 (VQ_DISPFMT_REG + 0x000)
    #define VQ_DISPFMT_W_HOR_VALID_END(value) \
	VQ_REG_WRITE_BITS(FMT_VALID_READY, BITS(12, 0), 0, value)
    #define VQ_DISPFMT_W_HOR_VALID_STAR(value) \
	VQ_REG_WRITE_BITS(FMT_VALID_READY, BITS(28, 16), 16, value)

#define FMT_NR_VSYNC_CTRL_IN            (VQ_DISPFMT_REG + 0x004)
    #define VQ_DISPFMT_W_NR_IN_VSYNC_END(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(10, 0), 0, value)
    #define VQ_DISPFMT_W_NR_IN_VSYNC_START(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(22, 12), 12, value)
    #define VQ_DISPFMT_W_NR_IN_VSYNC_POLAR(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(24, 24), 24, value)
    #define VQ_DISPFMT_W_NR_IN_HSYNC_POLAR(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(25, 25), 25, value)
    #define VQ_DISPFMT_W_NR_IN_FIELD_POLAR(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(26, 26), 26, value)
    #define VQ_DISPFMT_W_NR_IN_DE_SELF(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(27, 27), 27, value)
    #define VQ_DISPFMT_W_NR_IN_ODD_V_START_OPT(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(28, 28), 28, value)
    #define VQ_DISPFMT_W_NR_IN_ODD_V_END_OPT(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(29, 29), 29, value)
    #define VQ_DISPFMT_W_NR_IN_EVEN_V_START_OPT(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(30, 30), 30, value)
    #define VQ_DISPFMT_W_NR_IN_EVEN_V_END_OPT(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(31, 31), 31, value)


#define FMT_NR_VALID_READY              (VQ_DISPFMT_REG + 0x008)
    #define VQ_DISPFMT_W_HOR_NR_VALID_END(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VALID_READY, BITS(12, 0), 0, value)
    #define VQ_DISPFMT_W_HOR_NR_VALID_STAR(value) \
	VQ_REG_WRITE_BITS(FMT_NR_VALID_READY, BITS(28, 16), 16, value)

#define FMT_NR_IN_HOR                   (VQ_DISPFMT_REG + 0x00C)
    #define VQ_DISPFMT_W_NR_IN_HOR_END(value) \
	VQ_REG_WRITE_BITS(FMT_NR_IN_HOR, BITS(12, 0), 0, value)
    #define VQ_DISPFMT_W_NR_IN_HOR_STAT(value) \
	VQ_REG_WRITE_BITS(FMT_NR_IN_HOR, BITS(28, 16), 16, value)

#define FMT_DI_PATH_CONFIG              (VQ_DISPFMT_REG + 0x010)
    #define VQ_DISPFMT_W_BYPASS_NR(value) \
	VQ_REG_WRITE_BITS(FMT_DI_PATH_CONFIG, BITS(0, 0), 0, value)

#define COLOR_BAR_CTRL                  (VQ_DISPFMT_REG + 0x014)
    #define VQ_DISPFMT_W_COLOR_BAR_ON(value) \
	VQ_REG_WRITE_BITS(COLOR_BAR_CTRL, BITS(0, 0), 0, value)
    #define VQ_DISPFMT_W_COLOR_BAR_TYPE(value) \
	VQ_REG_WRITE_BITS(COLOR_BAR_CTRL, BITS(1, 1), 1, value)
    #define VQ_DISPFMT_W_COLOR_BAR_WIDHT(value) \
	VQ_REG_WRITE_BITS(COLOR_BAR_CTRL, BITS(23, 16), 16, value)
    #define VQ_DISPFMT_W_ENABLE_422_444(value) \
	VQ_REG_WRITE_BITS(COLOR_BAR_CTRL, BITS(31, 31), 31, value)

#define DI_AGENT_CTRL                   (VQ_DISPFMT_REG + 0x018)
    #define VQ_DISPFMT_W_DI_AGENT_TRIG(value) \
	VQ_REG_WRITE_BITS(DI_AGENT_CTRL, BITS(0, 0), 0, value)

#define WAIT_VDO_WDMA_START             (VQ_DISPFMT_REG + 0x01C)
    #define VQ_DISPFMT_W_FMT_H_WAIT_VDO_WDMA_START(value) \
	VQ_REG_WRITE_BITS(WAIT_VDO_WDMA_START, BITS(12, 0), 0, value)
    #define VQ_DISPFMT_W_NR_H_WAIT_VDO_WDMA_START(value) \
	VQ_REG_WRITE_BITS(WAIT_VDO_WDMA_START, BITS(28, 16), 16, value)

#define CRC                             (VQ_DISPFMT_REG + 0x050)
    #define VQ_DISPFMT_W_CRC_INIT(value) \
	VQ_REG_WRITE_BITS(CRC, BITS(0, 0), 0, value)
    #define VQ_DISPFMT_W_CRC_CLEAR(value) \
	VQ_REG_WRITE_BITS(CRC, BITS(1, 1), 1, value)
    #define VQ_DISPFMT_W_CRC_RESULT(value) \
	VQ_REG_WRITE_BITS(CRC, BITS(31, 8), 8, value)

#define DERING                          (VQ_DISPFMT_REG + 0x054)
    #define VQ_DISPFMT_W_DERING_THRESHOLD_Y(value) \
	VQ_REG_WRITE_BITS(DERING, BITS(11, 0), 0, value)
    #define VQ_DISPFMT_W_DERING_THRESHOLD_C(value) \
	VQ_REG_WRITE_BITS(DERING, BITS(23, 12), 12, value)
    #define VQ_DISPFMT_W_DERING_TRANS(value) \
	VQ_REG_WRITE_BITS(DERING, BITS(27, 24), 24, value)
    #define VQ_DISPFMT_W_DERING_EN_Y(value) \
	VQ_REG_WRITE_BITS(DERING, BITS(28, 28), 28, value)
    #define VQ_DISPFMT_W_DERING_EN_C(value) \
	VQ_REG_WRITE_BITS(DERING, BITS(29, 29), 29, value)

#define BORDER_CONTROL                  (VQ_DISPFMT_REG + 0x058)
    #define VQ_DISPFMT_W_BORDER_X_WIDTH(value) \
	VQ_REG_WRITE_BITS(BORDER_CONTROL, BITS(21, 16), 16, value)
    #define VQ_DISPFMT_W_BORDER_Y_WIDTH(value) \
	VQ_REG_WRITE_BITS(BORDER_CONTROL, BITS(29, 24), 24, value)
    #define VQ_DISPFMT_W_BD_ON(value) \
	VQ_REG_WRITE_BITS(BORDER_CONTROL, BITS(31, 31), 31, value)

#define Border_color                    (VQ_DISPFMT_REG + 0x05C)
    #define VQ_DISPFMT_W_BDY(value) \
	VQ_REG_WRITE_BITS(Border_color, BITS(7, 2), 2, value)
    #define VQ_DISPFMT_W_BDCB(value) \
	VQ_REG_WRITE_BITS(Border_color, BITS(15, 10), 10, value)
    #define VQ_DISPFMT_W_BDCR(value) \
	VQ_REG_WRITE_BITS(Border_color, BITS(23, 18), 18, value)

#define DOWN_CONTROL                    (VQ_DISPFMT_REG + 0x06C)

#define Down_scaler_range_0             (VQ_DISPFMT_REG + 0x070)

#define Down_scaler_range_1             (VQ_DISPFMT_REG + 0x074)

#define Down_scaler_range_2             (VQ_DISPFMT_REG + 0x078)

#define Down_scaler_output_range        (VQ_DISPFMT_REG + 0x07C)

#define NR_CONTROL                      (VQ_DISPFMT_REG + 0x080)
    #define VQ_DISPFMT_W_ADJ_SYNC_EN(value) \
	VQ_REG_WRITE_BITS(NR_CONTROL, BITS(0, 0), 0, value)
    #define VQ_DISPFMT_W_NR_ADJ_FORWARD(value) \
	VQ_REG_WRITE_BITS(NR_CONTROL, BITS(1, 1), 1, value)
    #define VQ_DISPFMT_W_TOGGLE_OPTION(value) \
	VQ_REG_WRITE_BITS(NR_CONTROL, BITS(2, 2), 2, value)
    #define VQ_DISPFMT_W_HSYNC_DELAY(value) \
	VQ_REG_WRITE_BITS(NR_CONTROL, BITS(19, 8), 8, value)
    #define VQ_DISPFMT_W_VSYNC_DELAY(value) \
	VQ_REG_WRITE_BITS(NR_CONTROL, BITS(31, 20), 20, value)

#define NR_VSYNC_CONTROL                (VQ_DISPFMT_REG + 0x084)
    #define VQ_DISPFMT_W_VSYNC_END(value) \
	VQ_REG_WRITE_BITS(NR_VSYNC_CONTROL, BITS(10, 0), 0, value)
    #define VQ_DISPFMT_W_VSYNC_START(value) \
	VQ_REG_WRITE_BITS(NR_VSYNC_CONTROL, BITS(22, 12), 12, value)
    #define VQ_DISPFMT_W_VSYNC_POLAR(value) \
	VQ_REG_WRITE_BITS(NR_VSYNC_CONTROL, BITS(24, 24), 24, value)
    #define VQ_DISPFMT_W_HSYNC_POLAR(value) \
	VQ_REG_WRITE_BITS(NR_VSYNC_CONTROL, BITS(25, 25), 25, value)
    #define VQ_DISPFMT_W_FIELD_POLAR(value) \
	VQ_REG_WRITE_BITS(NR_VSYNC_CONTROL, BITS(26, 26), 26, value)
    #define VQ_DISPFMT_W_DE_SELF(value) \
	VQ_REG_WRITE_BITS(NR_VSYNC_CONTROL, BITS(27, 27), 27, value)
    #define VQ_DISPFMT_W_ODD_V_START_OPT(value) \
	VQ_REG_WRITE_BITS(NR_VSYNC_CONTROL, BITS(28, 28), 28, value)
    #define VQ_DISPFMT_W_ODD_V_END_OPT(value) \
	VQ_REG_WRITE_BITS(NR_VSYNC_CONTROL, BITS(29, 29), 29, value)
    #define VQ_DISPFMT_W_EVEN_V_START_OPT(value) \
	VQ_REG_WRITE_BITS(NR_VSYNC_CONTROL, BITS(30, 30), 30, value)
    #define VQ_DISPFMT_W_EVEN_V_END_OPT(value) \
	VQ_REG_WRITE_BITS(NR_VSYNC_CONTROL, BITS(31, 31), 31, value)

#define NR_HORIZONTAL_CONTROL           (VQ_DISPFMT_REG + 0x088)
    #define VQ_DISPFMT_W_HOR_END(value) \
	VQ_REG_WRITE_BITS(NR_HORIZONTAL_CONTROL, BITS(11, 0), 0, value)
    #define VQ_DISPFMT_W_HOR_START(value) \
	VQ_REG_WRITE_BITS(NR_HORIZONTAL_CONTROL, BITS(27, 16), 16, value)

#define NR_ODD_VERTICAL_CONTRL          (VQ_DISPFMT_REG + 0x08C)
    #define VQ_DISPFMT_W_VO_END(value) \
	VQ_REG_WRITE_BITS(NR_ODD_VERTICAL_CONTRL, BITS(10, 0), 0, value)
    #define VQ_DISPFMT_W_VO_START(value) \
	VQ_REG_WRITE_BITS(NR_ODD_VERTICAL_CONTRL, BITS(26, 16), 16, value)

#define NR_EVEN_VERTICAL_CONTRL         (VQ_DISPFMT_REG + 0x090)
    #define VQ_DISPFMT_W_VE_END(value) \
	VQ_REG_WRITE_BITS(NR_EVEN_VERTICAL_CONTRL, BITS(10, 0), 0, value)
    #define VQ_DISPFMT_W_VE_START(value) \
	VQ_REG_WRITE_BITS(NR_EVEN_VERTICAL_CONTRL, BITS(26, 16), 16, value)

#define Mode_Control                    (VQ_DISPFMT_REG + 0x094)
    #define VQ_DISPFMT_W_HSYNWIDTH(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(7, 0), 0, value)
    #define VQ_DISPFMT_W_VSYNWIDTH(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(12, 8), 8, value)
    #define VQ_DISPFMT_W_HD_TP(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(13, 13), 13, value)
    #define VQ_DISPFMT_W_HD_ON(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(14, 14), 14, value)
    #define VQ_DISPFMT_W_PRGS(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(15, 15), 15, value)
    #define VQ_DISPFMT_W_PRGS_AUTOFLD(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(17, 17), 17, value)
    #define VQ_DISPFMT_W_PRGS_INVFLD(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(18, 18), 18, value)
    #define VQ_DISPFMT_W_YUV_RST_OPT(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(20, 20), 20, value)
    #define VQ_DISPFMT_W_PRGS_FLD(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(24, 24), 24, value)
    #define VQ_DISPFMT_W_NEW_SD_144MHz(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(27, 27), 27, value)
    #define VQ_DISPFMT_W_NEW_SD_MODE(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(28, 28), 28, value)
    #define VQ_DISPFMT_W_NEW_SD_USE_EVEN(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(29, 29), 29, value)
    #define VQ_DISPFMT_W_TVMODE(value) \
	VQ_REG_WRITE_BITS(Mode_Control, BITS(31, 30), 30, value)

#define VSYN_Offset                     (VQ_DISPFMT_REG + 0x098)
    #define VQ_DISPFMT_W_PF_ADV(value) \
	VQ_REG_WRITE_BITS(VSYN_Offset, BITS(19, 16), 16, value)

#define Pixel_Length                    (VQ_DISPFMT_REG + 0x09C)
    #define VQ_DISPFMT_W_PXLLEN(value) \
	VQ_REG_WRITE_BITS(Pixel_Length, BITS(12, 0), 0, value)
    #define VQ_DISPFMT_W_RIGHT_SKIP(value) \
	VQ_REG_WRITE_BITS(Pixel_Length, BITS(19, 16), 16, value)

#define Horizontal_Active_Zone          (VQ_DISPFMT_REG + 0x0A0)
    #define VQ_DISPFMT_W_HACTEND(value) \
	VQ_REG_WRITE_BITS(Horizontal_Active_Zone, BITS(12, 0), 0, value)
    #define VQ_DISPFMT_W_HACTBGN(value) \
	VQ_REG_WRITE_BITS(Horizontal_Active_Zone, BITS(28, 16), 16, value)

#define Vertical_Odd_Active_Zone        (VQ_DISPFMT_REG + 0x0A4)
    #define VQ_DISPFMT_W_VOACTEND(value) \
	VQ_REG_WRITE_BITS(Vertical_Odd_Active_Zone, BITS(11, 0), 0, value)
    #define VQ_DISPFMT_W_VOACTBGN(value) \
	VQ_REG_WRITE_BITS(Vertical_Odd_Active_Zone, BITS(27, 16), 16, value)
    #define VQ_DISPFMT_W_HIDE_OST(value) \
	VQ_REG_WRITE_BITS(Vertical_Odd_Active_Zone, BITS(31, 28), 28, value)

#define Vertical_Even_Active_Zone       (VQ_DISPFMT_REG + 0x0A8)
    #define VQ_DISPFMT_W_VEACTEND(value) \
	VQ_REG_WRITE_BITS(Vertical_Even_Active_Zone, BITS(11, 0), 0, value)
    #define VQ_DISPFMT_W_VEACTBGN(value) \
	VQ_REG_WRITE_BITS(Vertical_Even_Active_Zone, BITS(27, 16), 16, value)
    #define VQ_DISPFMT_W_HIDE_EST(value) \
	VQ_REG_WRITE_BITS(Vertical_Even_Active_Zone, BITS(31, 28), 28, value)

#define Video_Formatter_Control         (VQ_DISPFMT_REG + 0x0AC)
    #define VQ_DISPFMT_W_VDO1_EN(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(0, 0), 0, value)
    #define VQ_DISPFMT_W_FMTM(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(1, 1), 1, value)
    #define VQ_DISPFMT_W_HPOR(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(3, 3), 3, value)
    #define VQ_DISPFMT_W_VPOR(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(4, 4), 4, value)
    #define VQ_DISPFMT_W_C_RST_SEL(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(7, 7), 7, value)
    #define VQ_DISPFMT_W_PXLSEL(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(9, 8), 8, value)
    #define VQ_DISPFMT_W_FTRST(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(10, 10), 10, value)
    #define VQ_DISPFMT_W_SHVSYN(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(11, 11), 11, value)
    #define VQ_DISPFMT_W_SYN_DEL(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(15, 14), 14, value)
    #define VQ_DISPFMT_W_UVSW(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(19, 19), 19, value)
    #define VQ_DISPFMT_W_BLACK(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(25, 25), 25, value)
    #define VQ_DISPFMT_W_PFOFF(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(27, 27), 27, value)
    #define VQ_DISPFMT_W_HW_OPT(value) \
	VQ_REG_WRITE_BITS(Video_Formatter_Control, BITS(31, 28), 28, value)

#define Horizontal_Scaling              (VQ_DISPFMT_REG + 0x0B0)
    #define VQ_DISPFMT_W_Horizontal_Scaling(value) \
	VQ_REG_WRITE(Horizontal_Scaling, value)

#define Build_In_Color                  (VQ_DISPFMT_REG + 0x0B4)
    #define VQ_DISPFMT_W_BIY(value) \
	VQ_REG_WRITE_BITS(Build_In_Color, BITS(7, 4), 4, value)
    #define VQ_DISPFMT_W_BICB(value) \
	VQ_REG_WRITE_BITS(Build_In_Color, BITS(15, 12), 12, value)
    #define VQ_DISPFMT_W_BICR(value) \
	VQ_REG_WRITE_BITS(Build_In_Color, BITS(23, 20), 20, value)
    #define VQ_DISPFMT_W_PF2OFF(value) \
	VQ_REG_WRITE_BITS(Build_In_Color, BITS(24, 24), 24, value)
    #define VQ_DISPFMT_W_HIDE_L(value) \
	VQ_REG_WRITE_BITS(Build_In_Color, BITS(25, 25), 25, value)

#define Background_Color                (VQ_DISPFMT_REG + 0x0B8)
    #define VQ_DISPFMT_W_BGY(value) \
	VQ_REG_WRITE_BITS(Background_Color, BITS(7, 4), 4, value)
    #define VQ_DISPFMT_W_BGCB(value) \
	VQ_REG_WRITE_BITS(Background_Color, BITS(15, 12), 12, value)
    #define VQ_DISPFMT_W_BGCR(value) \
	VQ_REG_WRITE_BITS(Background_Color, BITS(23, 20), 20, value)

#define Y_value_limitation              (VQ_DISPFMT_REG + 0x0C4)
    #define VQ_DISPFMT_W_OLD_C_ACC(value) \
	VQ_REG_WRITE_BITS(Y_value_limitation, BITS(16, 16), 16, value)
    #define VQ_DISPFMT_W_Out_Sel2(value) \
	VQ_REG_WRITE_BITS(Y_value_limitation, BITS(18, 18), 18, value)
    #define VQ_DISPFMT_W_TVE_ND(value) \
	VQ_REG_WRITE_BITS(Y_value_limitation, BITS(20, 20), 20, value)
    #define VQ_DISPFMT_W_FIRST_PXL_LEAD(value) \
	VQ_REG_WRITE_BITS(Y_value_limitation, BITS(31, 24), 24, value)

#define NEW_SCL_MODE_CTRL               (VQ_DISPFMT_REG + 0x0C8)
    #define VQ_DISPFMT_W_NEW_SCL_MODE_CTRL(value) \
	VQ_REG_WRITE(NEW_SCL_MODE_CTRL, value)

#define Dispfmt_Configure               (VQ_DISPFMT_REG + 0x0CC)
    #define VQ_DISPFMT_W_Dispfmt_Configure(value) \
	VQ_REG_WRITE(Dispfmt_Configure, value)

#define DISPFMT_0D0                     (VQ_DISPFMT_REG + 0x0D0)
    #define VQ_DISPFMT_W_0D0_11_0(value) \
	VQ_REG_WRITE_BITS(DISPFMT_0D0, BITS(11, 0), 0, value)
    #define VQ_DISPFMT_W_0D0_28_16(value) \
	VQ_REG_WRITE_BITS(DISPFMT_0D0, BITS(28, 16), 16, value)
    #define VQ_DISPFMT_W_0D0_31_31(value) \
	VQ_REG_WRITE_BITS(DISPFMT_0D0, BITS(31, 31), 31, value)

#define Horizontal_Vertical_Total_pixel (VQ_DISPFMT_REG + 0x0D4)
    #define VQ_DISPFMT_W_V_TOTAL(value) \
	VQ_REG_WRITE_BITS(Horizontal_Vertical_Total_pixel, BITS(11, 0), 0, value)
    #define VQ_DISPFMT_W_H_TOTAL(value) \
	VQ_REG_WRITE_BITS(Horizontal_Vertical_Total_pixel, BITS(28, 16), 16, value)
    #define VQ_DISPFMT_W_ADJ_T(value) \
	VQ_REG_WRITE_BITS(Horizontal_Vertical_Total_pixel, BITS(31, 31), 31, value)

#define MULTI_INC                       (VQ_DISPFMT_REG + 0x0D8)

#define MULTI_DEC                       (VQ_DISPFMT_REG + 0x0DC)

#define NEW_SCL_MODE_CTRL2              (VQ_DISPFMT_REG + 0x0E0)

#define Multi_Ratio                     (VQ_DISPFMT_REG + 0x0E4)
    #define VQ_DISPFMT_W_Multi_Ratio(value) \
	VQ_REG_WRITE(Multi_Ratio, value)

#define MVDO_adjustment                 (VQ_DISPFMT_REG + 0x0E8)
    #define VQ_DISPFMT_W_HSYN_DELAY(value) \
	VQ_REG_WRITE_BITS(MVDO_adjustment, BITS(11, 0), 0, value)
    #define VQ_DISPFMT_W_VSYN_DELAY(value) \
	VQ_REG_WRITE_BITS(MVDO_adjustment, BITS(27, 16), 16, value)
    #define VQ_DISPFMT_W_ADJ_FORWARD(value) \
	VQ_REG_WRITE_BITS(MVDO_adjustment, BITS(31, 31), 31, value)

#define MIXER_CTRL                      (VQ_DISPFMT_REG + 0x0F8)
    #define VQ_DISPFMT_W_C110(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL, BITS(4, 4), 4, value)
    #define VQ_DISPFMT_W_OLD_CHROMA(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL, BITS(5, 5), 5, value)
    #define VQ_DISPFMT_W_EN_235_255(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL, BITS(6, 6), 6, value)
    #define VQ_DISPFMT_W_FROM_235_TO_255(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL, BITS(7, 7), 7, value)

#define MIXER_CTRL2                     (VQ_DISPFMT_REG + 0x0FC)
    #define VQ_DISPFMT_W_LUMA_KEY(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL2, BITS(11, 0), 0, value)

    #define VQ_DISPFMT_W_WINDOW_LINEAR_SIZE_SEL(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL2, BITS(17, 16), 16, value)

    #define VQ_DISPFMT_W_WINDOW_ACC_SIZE_SEL(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL2, BITS(19, 18), 18, value)

    #define VQ_DISPFMT_W_PRE_DATA_NEXT_LUMA_Y_OPTION(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL2, BITS(20, 20), 20, value)

    #define VQ_DISPFMT_W_NEXT_DATA_PRE_LUMA_Y_OPTION(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL2, BITS(21, 21), 21, value)

    #define VQ_DISPFMT_W_PRE_DATA_NEXT_LUMA_C_OPTION(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL2, BITS(22, 22), 22, value)

    #define VQ_DISPFMT_W_NEXT_DATA_PRE_LUMA_C_OPTION(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL2, BITS(23, 23), 23, value)

    #define VQ_DISPFMT_W_post_div2_sel(value) \
	VQ_REG_WRITE_BITS(MIXER_CTRL2, BITS(31, 31), 31, value)


#endif /* VQ_DISPFMT_REG_H */



