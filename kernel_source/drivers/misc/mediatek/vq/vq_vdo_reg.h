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

#ifndef VQ_VDO_REG_H
#define VQ_VDO_REG_H

#include "vq_def.h"


/* vdo */
#define VQ_VDO_BASE                     (VQ_IO_BASE + 0x2000)

#define VDO_SCL_CTRL                         (VQ_VDO_BASE + 0x14c)
    #define VQ_VDO_W_VDO_SCL_CTRL(value) \
	VQ_REG_WRITE(VDO_SCL_CTRL, value)

#define VDO_8TAP_VALID                         (VQ_VDO_BASE + 0x194)
    #define VQ_VDO_W_VDO_8TAP_VALID(value) \
	VQ_REG_WRITE(VDO_8TAP_VALID, value)

#define VDO_8TAP_CTRL_04                         (VQ_VDO_BASE + 0x1a0)
    #define VQ_VDO_W_VDO_8TAP_CTRL_04(value) \
	VQ_REG_WRITE(VDO_8TAP_CTRL_04, value)

#define VDO_SHARP_CTRL_01                         (VQ_VDO_BASE + 0x1b0)
    #define VQ_VDO_W_VDO_SHARP_CTRL_01(value) \
	VQ_REG_WRITE(VDO_SHARP_CTRL_01, value)

#define VDO_SHARP_CTRL_02                         (VQ_VDO_BASE + 0x1b4)
    #define VQ_VDO_W_VDO_SHARP_CTRL_02(value) \
	VQ_REG_WRITE(VDO_SHARP_CTRL_02, value)

#define VDO_SHARP_CTRL_03                         (VQ_VDO_BASE + 0x1b8)
    #define VQ_VDO_W_VDO_SHARP_CTRL_03(value) \
	VQ_REG_WRITE(VDO_SHARP_CTRL_03, value)
    #define VQ_VDO_W_BYPASS_SHARP(value) \
	VQ_REG_WRITE_BITS(VDO_SHARP_CTRL_03, BITS(24, 24), 24, value)

#define VDOY1                         (VQ_VDO_BASE + 0x400)
    #define VQ_VDO_W_VDOY1(value) \
	VQ_REG_WRITE(VDOY1, value)

#define VDOC1                         (VQ_VDO_BASE + 0x404)
    #define VQ_VDO_W_VDOC1(value) \
	VQ_REG_WRITE(VDOC1, value)

#define VDOY2                         (VQ_VDO_BASE + 0x408)
    #define VQ_VDO_W_VDOY2(value) \
	VQ_REG_WRITE(VDOY2, value)

#define VDOC2                         (VQ_VDO_BASE + 0x40c)
    #define VQ_VDO_W_VDOC2(value) \
	VQ_REG_WRITE(VDOC2, value)

#define HBLOCK                         (VQ_VDO_BASE + 0x410)
    #define VQ_VDO_W_swap_off(value) \
	VQ_REG_WRITE_BITS(HBLOCK, BITS(31, 31), 31, value)
    #define VQ_VDO_W_HBLOCK_9_8(value) \
	VQ_REG_WRITE_BITS(HBLOCK, BITS(29, 28), 28, value)
    #define VQ_VDO_W_PICHEIGHT(value) \
	VQ_REG_WRITE_BITS(HBLOCK, BITS(26, 16), 16, value)
    #define VQ_VDO_W_DW_NEED(value) \
	VQ_REG_WRITE_BITS(HBLOCK, BITS(15, 8), 8, value)
    #define VQ_VDO_W_HBLOCK_7_0(value) \
	VQ_REG_WRITE_BITS(HBLOCK, BITS(7, 0), 0, value)

#define VSCALE                         (VQ_VDO_BASE + 0x414)
    #define VQ_VDO_W_VSCALE(value) \
	VQ_REG_WRITE(VSCALE, value)

#define STAMBR                         (VQ_VDO_BASE + 0x418)
    #define VQ_VDO_W_STAMBR(value) \
	VQ_REG_WRITE(STAMBR, value)

#define VMODE                         (VQ_VDO_BASE + 0x41c)
    #define VQ_VDO_W_VMODE(value) \
	VQ_REG_WRITE(VMODE, value)

#define YSL                         (VQ_VDO_BASE + 0x420)
    #define VQ_VDO_W_YSL(value) \
	VQ_REG_WRITE(YSL, value)

#define CSL                         (VQ_VDO_BASE + 0x424)
    #define VQ_VDO_W_CSL(value) \
	VQ_REG_WRITE(CSL, value)

#define YSSL                         (VQ_VDO_BASE + 0x428)
    #define VQ_VDO_W_YSSL(value) \
	VQ_REG_WRITE(YSSL, value)

#define CSSL                         (VQ_VDO_BASE + 0x42c)
    #define VQ_VDO_W_CSSL(value) \
	VQ_REG_WRITE(CSSL, value)

#define VDOCTRL                         (VQ_VDO_BASE + 0x430)
    #define VQ_VDO_W_VDOCTRL(value) \
	VQ_REG_WRITE(VDOCTRL, value)
    #define VQ_VDO_W_AFLD(value) \
	VQ_REG_WRITE_BITS(VDOCTRL, BITS(5, 5), 5, value)
    #define VQ_VDO_W_PFLD(value) \
	VQ_REG_WRITE_BITS(VDOCTRL, BITS(2, 2), 2, value)
    #define VQ_VDO_W_FRMC(value) \
	VQ_REG_WRITE_BITS(VDOCTRL, BITS(1, 1), 1, value)
    #define VQ_VDO_W_FRMY(value) \
	VQ_REG_WRITE_BITS(VDOCTRL, BITS(0, 0), 0, value)

#define VSLACK                         (VQ_VDO_BASE + 0x434)
    #define VQ_VDO_W_VSLACK(value) \
	VQ_REG_WRITE(VSLACK, value)

#define MP                         (VQ_VDO_BASE + 0x438)
    #define VQ_VDO_W_MP(value) \
	VQ_REG_WRITE(MP, value)
    #define VQ_VDO_W_MTHRD(value) \
	VQ_REG_WRITE_BITS(MP, BITS(23, 16), 16, value)
    #define VQ_VDO_W_CT_THRD(value) \
	VQ_REG_WRITE_BITS(MP, BITS(15, 8), 8, value)

#define VDORST                         (VQ_VDO_BASE + 0x43c)
    #define VQ_VDO_W_VDORST(value) \
	VQ_REG_WRITE(VDORST, value)

#define COMB_8F                         (VQ_VDO_BASE + 0x440)
    #define VQ_VDO_W_COMB_8F(value) \
	VQ_REG_WRITE(COMB_8F, value)

#define YSL2                         (VQ_VDO_BASE + 0x450)
    #define VQ_VDO_W_YSL2(value) \
	VQ_REG_WRITE(YSL2, value)

#define CSL2                         (VQ_VDO_BASE + 0x454)
    #define VQ_VDO_W_CSL2(value) \
	VQ_REG_WRITE(CSL2, value)

#define MBAVG1                         (VQ_VDO_BASE + 0x458)
    #define VQ_VDO_W_MBAVG1(value) \
	VQ_REG_WRITE(MBAVG1, value)

#define MBAVG2                         (VQ_VDO_BASE + 0x45c)
    #define VQ_VDO_W_MBAVG2(value) \
	VQ_REG_WRITE(MBAVG2, value)

#define CMB_CNT                          (VQ_VDO_BASE + 0x460)
    #define VQ_VDO_W_CMB_CNT(value) \
	VQ_REG_WRITE(CMB_CNT, value)

#define PS_MODE                         (VQ_VDO_BASE + 0x464)
    #define VQ_VDO_W_PS_MODE(value) \
	VQ_REG_WRITE(PS_MODE, value)

#define STA_POS                         (VQ_VDO_BASE + 0x46c)
    #define VQ_VDO_W_STA_POS(value) \
	VQ_REG_WRITE(STA_POS, value)

#define PS_FLT                         (VQ_VDO_BASE + 0x470)
    #define VQ_VDO_W_PS_FLT(value) \
	VQ_REG_WRITE(PS_FLT, value)

#define F_CTRL                         (VQ_VDO_BASE + 0x478)
    #define VQ_VDO_W_F_CTRL(value) \
	VQ_REG_WRITE(F_CTRL, value)
    #define VQ_VDO_W_INTRA_EDGEP_MODE(value) \
	VQ_REG_WRITE_BITS(F_CTRL, BITS(11, 11), 11, value)

#define VIDEO_OPTION                         (VQ_VDO_BASE + 0x47c)
    #define VQ_VDO_W_VIDEO_OPTION(value) \
	VQ_REG_WRITE(VIDEO_OPTION, value)

#define PTR_WF_Y                         (VQ_VDO_BASE + 0x480)
    #define VQ_VDO_W_PTR_WF_Y(value) \
	VQ_REG_WRITE(PTR_WF_Y, value)

#define PTR_ZF_Y                         (VQ_VDO_BASE + 0x484)
    #define VQ_VDO_W_PTR_ZF_Y(value) \
	VQ_REG_WRITE(PTR_ZF_Y, value)

#define FDIFF_TH3                         (VQ_VDO_BASE + 0x488)
    #define VQ_VDO_W_FDIFF_TH3(value) \
	VQ_REG_WRITE(FDIFF_TH3, value)
    #define VQ_VDO_W_MA4F(value) \
	VQ_REG_WRITE_BITS(FDIFF_TH3, BITS(24, 24), 24, value)

#define FDIFF_TH2                         (VQ_VDO_BASE + 0x48c)
    #define VQ_VDO_W_FDIFF_TH2(value) \
	VQ_REG_WRITE(FDIFF_TH2, value)

#define FDIFF_TH1                         (VQ_VDO_BASE + 0x490)
    #define VQ_VDO_W_FDIFF_TH1(value) \
	VQ_REG_WRITE(FDIFF_TH1, value)

#define TH_XZ_MIN                         (VQ_VDO_BASE + 0x494)
    #define VQ_VDO_W_TH_XZ_MIN(value) \
	VQ_REG_WRITE(TH_XZ_MIN, value)

#define FCH_YW                         (VQ_VDO_BASE + 0x4a8)
    #define VQ_VDO_W_FCH_YW(value) \
	VQ_REG_WRITE(FCH_YW, value)

#define CRM_SAW                         (VQ_VDO_BASE + 0x4ac)
    #define VQ_VDO_W_CRM_SAW(value) \
	VQ_REG_WRITE(CRM_SAW, value)

#define EDGE_CTL                         (VQ_VDO_BASE + 0x4b0)
    #define VQ_VDO_W_EDGE_CTL(value) \
	VQ_REG_WRITE(EDGE_CTL, value)

#define MD_ADV                         (VQ_VDO_BASE + 0x4c0)
    #define VQ_VDO_W_MD_ADV(value) \
	VQ_REG_WRITE(MD_ADV, value)
    #define VQ_VDO_W_MA8F_OR(value) \
	VQ_REG_WRITE_BITS(MD_ADV, BITS(25, 25), 25, value)

#define PD_FLD_LIKE                         (VQ_VDO_BASE + 0x4c4)
    #define VQ_VDO_W_PD_FLD_LIKE(value) \
	VQ_REG_WRITE(PD_FLD_LIKE, value)
    #define VQ_VDO_W_PD_COMB_TH(value) \
	VQ_REG_WRITE_BITS(PD_FLD_LIKE, BITS(22, 16), 16, value)

#define PD_REGION                         (VQ_VDO_BASE + 0x4cc)
    #define VQ_VDO_W_PD_REGION(value) \
	VQ_REG_WRITE(PD_REGION, value)

#define CHROMA_MD                         (VQ_VDO_BASE + 0x4d0)
    #define VQ_VDO_W_CHROMA_MD(value) \
	VQ_REG_WRITE(CHROMA_MD, value)

#define MBAVG3                         (VQ_VDO_BASE + 0x4d8)
    #define VQ_VDO_W_MBAVG3(value) \
	VQ_REG_WRITE(MBAVG3, value)

#define HD_MODE                         (VQ_VDO_BASE + 0x4e0)
    #define VQ_VDO_W_HD_MODE(value) \
	VQ_REG_WRITE(HD_MODE, value)
    #define VQ_VDO_W_HD_EN(value) \
	VQ_REG_WRITE_BITS(HD_MODE, BITS(24, 24), 24, value)
    #define VQ_VDO_W_HD_MEM(value) \
	VQ_REG_WRITE_BITS(HD_MODE, BITS(22, 22), 22, value)
    #define VQ_VDO_W_HD_MEM_1920(value) \
	VQ_REG_WRITE_BITS(HD_MODE, BITS(21, 21), 21, value)
    #define VQ_VDO_W_HD_LINE_MODE(value) \
	VQ_REG_WRITE_BITS(HD_MODE, BITS(20, 20), 20, value)
    #define VQ_VDO_W_DW_NEED_HD(value) \
	VQ_REG_WRITE_BITS(HD_MODE, BITS(8, 0), 0, value)

#define PTR_AF_Y                         (VQ_VDO_BASE + 0x4ec)
    #define VQ_VDO_W_PTR_AF_Y(value) \
	VQ_REG_WRITE(PTR_AF_Y, value)

#define WMV_DISABLE                         (VQ_VDO_BASE + 0x4f8)
    #define VQ_VDO_W_WMV_DISABLE(value) \
	VQ_REG_WRITE(WMV_DISABLE, value)

#define PTR_ZF_YC                         (VQ_VDO_BASE + 0x4fc)
    #define VQ_VDO_W_PTR_ZF_YC(value) \
	VQ_REG_WRITE(PTR_ZF_YC, value)

#define METRIC_00                         (VQ_VDO_BASE + 0x700)
    #define VQ_VDO_W_METRIC_00(value) \
	VQ_REG_WRITE(METRIC_00, value)

#define MDDI_FUSION_00                         (VQ_VDO_BASE + 0x800)
    #define VQ_VDO_W_MDDI_FUSION_00(value) \
	VQ_REG_WRITE(MDDI_FUSION_00, value)
    #define VQ_VDO_W_IFUSION_EN(value) \
	VQ_REG_WRITE_BITS(MDDI_FUSION_00, BITS(0, 0), 0, value)

#define MDDI_FUSION_08                         (VQ_VDO_BASE + 0x820)
    #define VQ_VDO_W_MDDI_FUSION_08(value) \
	VQ_REG_WRITE(MDDI_FUSION_08, value)

#define MDDI_FUSION_1A                         (VQ_VDO_BASE + 0x868)
    #define VQ_VDO_W_MDDI_FUSION_1A(value) \
	VQ_REG_WRITE(MDDI_FUSION_1A, value)

#define MDDI_FUSION_1C                         (VQ_VDO_BASE + 0x870)
    #define VQ_VDO_W_MDDI_FUSION_1C(value) \
	VQ_REG_WRITE(MDDI_FUSION_1C, value)
    #define VQ_VDO_W_en_lmr(value) \
	VQ_REG_WRITE_BITS(MDDI_FUSION_1C, BITS(29, 29), 29, value)
    #define VQ_VDO_W_en_lmw(value) \
	VQ_REG_WRITE_BITS(MDDI_FUSION_1C, BITS(28, 28), 28, value)
    #define VQ_VDO_W_fusion_flag_addr_base(value) \
	VQ_REG_WRITE_BITS(MDDI_FUSION_1C, BITS(27, 0), 0, value)

#define MDDI_FUSION_1D                         (VQ_VDO_BASE + 0x874)
    #define VQ_VDO_W_MDDI_FUSION_1D(value) \
	VQ_REG_WRITE(MDDI_FUSION_1D, value)
    #define VQ_VDO_W_da_flag_waddr_hi_limit(value) \
	VQ_REG_WRITE_BITS(MDDI_FUSION_1D, BITS(27, 0), 0, value)

#define MDDI_FUSION_1E                         (VQ_VDO_BASE + 0x878)
    #define VQ_VDO_W_MDDI_FUSION_1E(value) \
	VQ_REG_WRITE(MDDI_FUSION_1E, value)
    #define VQ_VDO_W_da_flag_waddr_lo_limit(value) \
	VQ_REG_WRITE_BITS(MDDI_FUSION_1E, BITS(27, 0), 0, value)

#define MDDI_FUSION_20                         (VQ_VDO_BASE + 0x880)
    #define VQ_VDO_W_MDDI_FUSION_20(value) \
	VQ_REG_WRITE(MDDI_FUSION_20, value)

#define MDDI_FUSION_22                         (VQ_VDO_BASE + 0x888)
    #define VQ_VDO_W_MDDI_FUSION_22(value) \
	VQ_REG_WRITE(MDDI_FUSION_22, value)

#define MDDI_FUSION_23                         (VQ_VDO_BASE + 0x88c)
    #define VQ_VDO_W_MDDI_FUSION_23(value) \
	VQ_REG_WRITE(MDDI_FUSION_23, value)

#define MDDI_PE_00                         (VQ_VDO_BASE + 0x8b0)
    #define VQ_VDO_W_MDDI_PE_00(value) \
	VQ_REG_WRITE(MDDI_PE_00, value)

#define MDDI_PE_03                         (VQ_VDO_BASE + 0x8bc)
    #define VQ_VDO_W_MDDI_PE_03(value) \
	VQ_REG_WRITE(MDDI_PE_03, value)

#define VDO_CRC_00                         (VQ_VDO_BASE + 0xf00)
    #define VQ_VDO_W_VDO_CRC_00(value) \
	VQ_REG_WRITE(VDO_CRC_00, value)

#define VDO_PQ_OPTION                         (VQ_VDO_BASE + 0xf40)
    #define VQ_VDO_W_VDO_PQ_OPTION(value) \
	VQ_REG_WRITE(VDO_PQ_OPTION, value)

#define VDO_PQ_OPTION2                         (VQ_VDO_BASE + 0xf44)
    #define VQ_VDO_W_VDO_PQ_OPTION2(value) \
	VQ_REG_WRITE(VDO_PQ_OPTION2, value)

#define MDDI_FILM_02                         (VQ_VDO_BASE + 0xf60)
    #define VQ_VDO_W_MDDI_FILM_02(value) \
	VQ_REG_WRITE(MDDI_FILM_02, value)

#define CHROMA_SAW_CNT                         (VQ_VDO_BASE + 0xfe0)
    #define VQ_VDO_W_CHROMA_SAW_CNT(value) \
	VQ_REG_WRITE(CHROMA_SAW_CNT, value)

#define VDO_PQ_OPTION5                         (VQ_VDO_BASE + 0xfe4)
    #define VQ_VDO_W_VDO_PQ_OPTION5(value) \
	VQ_REG_WRITE(VDO_PQ_OPTION5, value)

#define DEMO_MODE                         (VQ_VDO_BASE + 0xfec)
    #define VQ_VDO_W_DEMO_MODE(value) \
	VQ_REG_WRITE(DEMO_MODE, value)

#define VDO_PQ_OPTION9                         (VQ_VDO_BASE + 0xff4)
    #define VQ_VDO_W_VDO_PQ_OPTION9(value) \
	VQ_REG_WRITE(VDO_PQ_OPTION9, value)
    #define VQ_VDO_W_chroma_multi_burst_threshold(value) \
	VQ_REG_WRITE_BITS(VDO_PQ_OPTION9, BITS(31, 24), 24, value)
    #define VQ_VDO_W_chroma_multi_burst_pixel_sel(value) \
	VQ_REG_WRITE_BITS(VDO_PQ_OPTION9, BITS(19, 19), 19, value)
    #define VQ_VDO_W_chroma_multi_burst_en(value) \
	VQ_REG_WRITE_BITS(VDO_PQ_OPTION9, BITS(18, 18), 18, value)

/* sample
#define                          (VQ_VDO_BASE + 0x)
    #define VQ_VDO_W_(value) \
	VQ_REG_WRITE(, value)
    #define VQ_VDO_W_(value) \
	VQ_REG_WRITE_BITS(, BITS(, ), , value)
    #define VQ_VDO_W_(value) \
	VQ_REG_WRITE_BITS(, BITS(, ), , value)
    #define VQ_VDO_W_(value) \
	VQ_REG_WRITE_BITS(, BITS(, ), , value)
*/

#endif /* VQ_VDO_REG_H */

