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


#ifndef VQ_NR_REG_H
#define VQ_NR_REG_H

#include "vq_def.h"


/* nr */
#define VQ_NR_BASE                      (VQ_IO_BASE + 0x5000)

#define HD_YS                           (VQ_NR_BASE + 0x000)
    #define VQ_NR_W_HD_YS(value) \
	VQ_REG_WRITE_BITS(HD_YS, BITS(27, 4), 4, value)

#define HD_CS                           (VQ_NR_BASE + 0x004)
    #define VQ_NR_W_HD_CS(value) \
	VQ_REG_WRITE_BITS(HD_CS, BITS(27, 4), 4, value)

#define HD_R_MAP                        (VQ_NR_BASE + 0x008)
    #define VQ_NR_W_HD_R_MAP_C(value) \
	VQ_REG_WRITE_BITS(HD_R_MAP, BITS(12, 8), 8, value)
    #define VQ_NR_W_HD_R_MAP_Y(value) \
	VQ_REG_WRITE_BITS(HD_R_MAP, BITS(4, 0), 0, value)

#define HD_TP_ON                        (VQ_NR_BASE + 0x018)
    #define VQ_NR_W_HD_TP_CB(value) \
	VQ_REG_WRITE_BITS(HD_TP_ON, BITS(15, 8), 8, value)
    #define VQ_NR_W_HD_TP_CR(value) \
	VQ_REG_WRITE_BITS(HD_TP_ON, BITS(7, 0), 0, value)

#define HD_ACTIVE                       (VQ_NR_BASE + 0x01C)
    #define VQ_NR_W_HD_VDEW(value) \
	VQ_REG_WRITE_BITS(HD_ACTIVE, BITS(30, 20), 20, value)
    #define VQ_NR_W_HD_HDEW(value) \
	VQ_REG_WRITE_BITS(HD_ACTIVE, BITS(9, 0), 0, value)

#define HD_LINE_OFFSET                  (VQ_NR_BASE + 0x028)
    #define VQ_NR_W_HD_LINE_OFFSET(value) \
	VQ_REG_WRITE_BITS(HD_LINE_OFFSET, BITS(6, 0), 0, value)

#define HD_MODE_CTRL                    (VQ_NR_BASE + 0x02C)
    #define VQ_NR_W_HD_BURST_READ_EN(value) \
	VQ_REG_WRITE_BITS(HD_MODE_CTRL, BITS(23, 23), 23, value)
    #define VQ_NR_W_HD_ADDR_SWAP(value) \
	VQ_REG_WRITE_BITS(HD_MODE_CTRL, BITS(22, 20), 20, value)
    #define VQ_NR_W_HD_OUT_I(value) \
	VQ_REG_WRITE_BITS(HD_MODE_CTRL, BITS(12, 12), 12, value)
    #define VQ_NR_W_HD_FIELD(value) \
	VQ_REG_WRITE_BITS(HD_MODE_CTRL, BITS(7, 7), 7, value)
    #define VQ_NR_W_HD_FIELD_CNT(value) \
	VQ_REG_WRITE_BITS(HD_MODE_CTRL, BITS(6, 6), 6, value)
    #define VQ_NR_W_HD_CHKSM_EN(value) \
	VQ_REG_WRITE_BITS(HD_MODE_CTRL, BITS(3, 3), 3, value)
    #define VQ_NR_W_HD_CHKSM_SHIFT(value) \
	VQ_REG_WRITE_BITS(HD_MODE_CTRL, BITS(2, 2), 2, value)

#define HD_TRIGGER                      (VQ_NR_BASE + 0x030)
    #define VQ_NR_W_HD_TGEN_RST(value) \
	VQ_REG_WRITE_BITS(HD_TRIGGER, BITS(2, 2), 2, value)
    #define VQ_NR_W_HD_SET_FIELD(value) \
	VQ_REG_WRITE_BITS(HD_TRIGGER, BITS(0, 0), 0, value)

#define HD_STA                          (VQ_NR_BASE + 0x03C)
    #define VQ_NR_W_HD_VCNT(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(26, 16), 26, value)
    #define VQ_NR_W_HD_FLD_ADJI(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(14, 14), 14, value)
    #define VQ_NR_W_HD_FLD_VEI(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(13, 13), 13, value)
    #define VQ_NR_W_HD_FLD_NOW(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(10, 10), 10, value)
    #define VQ_NR_W_HD_ADJ_INT(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(8, 8), 8, value)
    #define VQ_NR_W_HD_VE_INT(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(7, 7), 7, value)
    #define VQ_NR_W_HD_TB_ADJI(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(6, 6), 6, value)
    #define VQ_NR_W_HD_TB_VEI(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(5, 5), 5, value)
    #define VQ_NR_W_HD_UDFL(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(3, 3), 3, value)
    #define VQ_NR_W_HD_TB_NOW(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(2, 2), 2, value)
    #define VQ_NR_W_HD_VS_NOW(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(1, 1), 1, value)
    #define VQ_NR_W_HD_HS_NOW(value) \
	VQ_REG_WRITE_BITS(HD_STA, BITS(0, 0), 0, value)

#define HD_EN                           (VQ_NR_BASE + 0x040)
    #define VQ_NR_W_HD_EN(value) \
	VQ_REG_WRITE_BITS(HD_EN, BITS(0, 0), 0, value)

#define MISC                            (VQ_NR_BASE + 0x300)
    #define VQ_NR_W_DRAM_RST(value) \
	VQ_REG_WRITE_BITS(MISC, BITS(5, 5), 5, value)
    #define VQ_NR_W_HD_RST(value) \
	VQ_REG_WRITE_BITS(MISC, BITS(1, 1), 1, value)

#define INT_CLR                         (VQ_NR_BASE + 0x30C)
    #define VQ_NR_W_FBIST_SET_RANDOM_7(value) \
	VQ_REG_WRITE_BITS(INT_CLR, BITS(15, 15), 15, value)
    #define VQ_NR_W_FBIST_SET_RANDOM_6(value) \
	VQ_REG_WRITE_BITS(INT_CLR, BITS(14, 14), 14, value)
    #define VQ_NR_W_FBIST_SET_RANDOM_5(value) \
	VQ_REG_WRITE_BITS(INT_CLR, BITS(13, 13), 13, value)
    #define VQ_NR_W_FBIST_SET_RANDOM_4(value) \
	VQ_REG_WRITE_BITS(INT_CLR, BITS(12, 12), 12, value)
    #define VQ_NR_W_FBIST_SET_RANDOM_3(value) \
	VQ_REG_WRITE_BITS(INT_CLR, BITS(11, 11), 11, value)
    #define VQ_NR_W_FBIST_SET_RANDOM_2(value) \
	VQ_REG_WRITE_BITS(INT_CLR, BITS(10, 10), 10, value)
    #define VQ_NR_W_FBIST_SET_RANDOM_1(value) \
	VQ_REG_WRITE_BITS(INT_CLR, BITS(9, 9), 9, value)
    #define VQ_NR_W_FBIST_SET_RANDOM_0(value) \
	VQ_REG_WRITE_BITS(INT_CLR, BITS(8, 8), 8, value)
    #define VQ_NR_W_HD_INT_CLR(value) \
	VQ_REG_WRITE_BITS(INT_CLR, BITS(4, 4), 4, value)
    #define VQ_NR_W_FBIST_TRG_STA(value) \
	VQ_REG_WRITE_BITS(INT_CLR, BITS(3, 3), 3, value)

#define NR_DM_00                        (VQ_NR_BASE + 0x400)
    #define VQ_NR_W_NR_WFIFO_LINE_OFFSET(value) \
	VQ_REG_WRITE_BITS(NR_DM_00, BITS(31, 24), 24, value)
    #define VQ_NR_W_NR_WFIFO_MAX_REQ_LEN(value) \
	VQ_REG_WRITE_BITS(NR_DM_00, BITS(15, 8), 8, value)
    #define VQ_NR_W_NR_WFIFO_ADDR_SWAP(value) \
	VQ_REG_WRITE_BITS(NR_DM_00, BITS(6, 4), 4, value)
    #define VQ_NR_W_NR_WFIFO_INTERLACE_MODE(value) \
	VQ_REG_WRITE_BITS(NR_DM_00, BITS(0, 0), 0, value)

#define NR_DM_01                        (VQ_NR_BASE + 0x404)
    #define VQ_NR_W_NR_WFIFO_VDEW(value) \
	VQ_REG_WRITE_BITS(NR_DM_01, BITS(26, 16), 16, value)
    #define VQ_NR_W_NR_WFIFO_HDEW(value) \
	VQ_REG_WRITE_BITS(NR_DM_01, BITS(10, 1), 10, value)

#define NR_WYSA                         (VQ_NR_BASE + 0x410)
    #define VQ_NR_W_NR_WFIFO_Y_START_ADDR(value) \
	VQ_REG_WRITE_BITS(NR_WYSA, BITS(30, 4), 4, value)

#define NR_WYEA                         (VQ_NR_BASE + 0x414)
    #define VQ_NR_W_NR_WFIFO_Y_END_ADDR(value) \
	VQ_REG_WRITE_BITS(NR_WYEA, BITS(30, 4), 4, value)

#define NR_WCSA                         (VQ_NR_BASE + 0x418)
    #define VQ_NR_W_NR_WFIFO_C_START_ADDR(value) \
	VQ_REG_WRITE_BITS(NR_WCSA, BITS(30, 4), 4, value)

#define NR_WCEA                         (VQ_NR_BASE + 0x41C)
    #define VQ_NR_W_NR_WFIFO_C_END_ADDR(value) \
	VQ_REG_WRITE_BITS(NR_WCEA, BITS(30, 4), 4, value)

#define NR_HBKSA                        (VQ_NR_BASE + 0x420)
    #define VQ_NR_W_NR_H_BK_METER_START_ADDR(value) \
	VQ_REG_WRITE_BITS(NR_HBKSA, BITS(30, 4), 4, value)

#define NR_VBKSA                        (VQ_NR_BASE + 0x424)
    #define VQ_NR_W_NR_V_BK_METER_START_ADDR(value) \
	VQ_REG_WRITE_BITS(NR_VBKSA, BITS(30, 4), 4, value)

#define NR_FVKEA                        (VQ_NR_BASE + 0x428)
    #define VQ_NR_W_NR_F_BK_METER_START_ADDR(value) \
	VQ_REG_WRITE_BITS(NR_FVKEA, BITS(30, 4), 4, value)

#define NR_MISC_00                      (VQ_NR_BASE + 0x42C)
    #define VQ_NR_W_NR_IN_CEN_RATIO(value) \
	VQ_REG_WRITE_BITS(NR_MISC_00, BITS(31, 30), 30, value)
    #define VQ_NR_W_NR_CORE_BYPASS(value) \
	VQ_REG_WRITE_BITS(NR_MISC_00, BITS(29, 29), 29, value)
    #define VQ_NR_W_NR_OUTPUT_DELAY_SEL(value) \
	VQ_REG_WRITE_BITS(NR_MISC_00, BITS(28, 28), 28, value)
    #define VQ_NR_W_NR_DELSEL(value) \
	VQ_REG_WRITE_BITS(NR_MISC_00, BITS(27, 26), 26, value)
    #define VQ_NR_W_NR_OUTPUT_BLANK_BLACK_EN(value) \
	VQ_REG_WRITE_BITS(NR_MISC_00, BITS(24, 24), 24, value)
    #define VQ_NR_W_NR_OUTPUT_BLANK_BLACK_Y(value) \
	VQ_REG_WRITE_BITS(NR_MISC_00, BITS(23, 16), 16, value)
    #define VQ_NR_W_NR_OUTPUT_BLANK_BLACK_CB(value) \
	VQ_REG_WRITE_BITS(NR_MISC_00, BITS(15, 8), 8, value)
    #define VQ_NR_W_NR_OUTPUT_BLANK_BLACK_CR(value) \
	VQ_REG_WRITE_BITS(NR_MISC_00, BITS(7, 0), 0, value)

#define NR_MISC_01                      (VQ_NR_BASE + 0x43C)
    #define VQ_NR_W_NR_MON_SEL(value) \
	VQ_REG_WRITE_BITS(NR_MISC_01, BITS(15, 8), 8, value)
    #define VQ_NR_W_NR_RANDOM_EN(value) \
	VQ_REG_WRITE_BITS(NR_MISC_01, BITS(3, 3), 3, value)
    #define VQ_NR_W_NR_WFIFO_WDLE_CHECK_EN(value) \
	VQ_REG_WRITE_BITS(NR_MISC_01, BITS(2, 2), 2, value)
    #define VQ_NR_W_NR_WFIFO_CHKSM_SHIFT(value) \
	VQ_REG_WRITE_BITS(NR_MISC_01, BITS(1, 1), 1, value)
    #define VQ_NR_W_NR_WFIFO_CHKSM_EN(value) \
	VQ_REG_WRITE_BITS(NR_MISC_01, BITS(0, 0), 0, value)

#define TOP_MAIN_00                     (VQ_NR_BASE + 0x800)
    #define VQ_NR_W_c_m2h_inv(value) \
	VQ_REG_WRITE_BITS(TOP_MAIN_00, BITS(23, 23), 23, value)
    #define VQ_NR_W_c_m2v_inv(value) \
	VQ_REG_WRITE_BITS(TOP_MAIN_00, BITS(22, 22), 22, value)
    #define VQ_NR_W_c_m2f_inv(value) \
	VQ_REG_WRITE_BITS(TOP_MAIN_00, BITS(21, 21), 21, value)
    #define VQ_NR_W_c_crc_sel(value) \
	VQ_REG_WRITE_BITS(TOP_MAIN_00, BITS(3, 2), 2, value)

#define TOP_MAIN_01                     (VQ_NR_BASE + 0x804)
    #define VQ_NR_W_c_rst_ctrl(value) \
	VQ_REG_WRITE_BITS(TOP_MAIN_01, BITS(31, 24), 24, value)
    #define VQ_NR_W_c_dbg_out_sel(value) \
	VQ_REG_WRITE_BITS(TOP_MAIN_01, BITS(4, 0), 0, value)

#define YCBCR2YC_MAIN_00                (VQ_NR_BASE + 0x874)
    #define VQ_NR_W_c_ycbcr2yc_left_padding(value) \
	VQ_REG_WRITE_BITS(YCBCR2YC_MAIN_00, BITS(7, 7), 7, value)
    #define VQ_NR_W_OUTPUT_422_ENABLE(value) \
	VQ_REG_WRITE_BITS(YCBCR2YC_MAIN_00, BITS(6, 6), 6, value)
    #define VQ_NR_W_c_ycbcr2yc_even_extend(value) \
	VQ_REG_WRITE_BITS(YCBCR2YC_MAIN_00, BITS(5, 5), 5, value)
    #define VQ_NR_W_c_ycbcr2yc_no_sync_block(value) \
	VQ_REG_WRITE_BITS(YCBCR2YC_MAIN_00, BITS(4, 4), 4, value)
    #define VQ_NR_W_c_ycbcr2yc_keep_last_data(value) \
	VQ_REG_WRITE_BITS(YCBCR2YC_MAIN_00, BITS(3, 3), 3, value)
    #define VQ_NR_W_c_ycbcr2yc_hsync_black(value) \
	VQ_REG_WRITE_BITS(YCBCR2YC_MAIN_00, BITS(2, 2), 2, value)
    #define VQ_NR_W_c_ycbcr2yc_filter_on(value) \
	VQ_REG_WRITE_BITS(YCBCR2YC_MAIN_00, BITS(1, 1), 1, value)
    #define VQ_NR_W_INPUT_444_ENABLE(value) \
	VQ_REG_WRITE_BITS(YCBCR2YC_MAIN_00, BITS(0, 0), 0, value)

#define CRC_CFG                         (VQ_NR_BASE + 0x8A4)
    #define VQ_NR_W_c_crc_disable_f(value) \
	VQ_REG_WRITE_BITS(CRC_CFG, BITS(10, 10), 10, value)
    #define VQ_NR_W_c_crc_422(value) \
	VQ_REG_WRITE_BITS(CRC_CFG, BITS(8, 8), 8, value)
    #define VQ_NR_W_c_crc_en_sel(value) \
	VQ_REG_WRITE_BITS(CRC_CFG, BITS(7, 6), 6, value)
    #define VQ_NR_W_c_crc_vs_sel_1(value) \
	VQ_REG_WRITE_BITS(CRC_CFG, BITS(5, 5), 5, value)
    #define VQ_NR_W_c_crc_vs_sel_0(value) \
	VQ_REG_WRITE_BITS(CRC_CFG, BITS(4, 4), 4, value)
    #define VQ_NR_W_c_crc_vend_sel(value) \
	VQ_REG_WRITE_BITS(CRC_CFG, BITS(3, 3), 3, value)
    #define VQ_NR_W_c_crc_vstart_sel(value) \
	VQ_REG_WRITE_BITS(CRC_CFG, BITS(2, 2), 2, value)
    #define VQ_NR_W_c_crc_clr(value) \
	VQ_REG_WRITE_BITS(CRC_CFG, BITS(1, 1), 1, value)
    #define VQ_NR_W_c_crc_start(value) \
	VQ_REG_WRITE_BITS(CRC_CFG, BITS(0, 0), 0, value)

#define NR_3DNR00                          (VQ_NR_BASE + 0xC00)
    #define VQ_NR_W_c_src_420(value) \
	VQ_REG_WRITE_BITS(NR_3DNR00, BITS(29, 29), 29, value)
    #define VQ_NR_W_c_nr_src_sel(value) \
	VQ_REG_WRITE_BITS(NR_3DNR00, BITS(28, 28), 28, value)
    #define VQ_NR_W_c_sw_init(value) \
	VQ_REG_WRITE_BITS(NR_3DNR00, BITS(27, 27), 27, value)
    #define VQ_NR_W_c_v_bound_protect(value) \
	VQ_REG_WRITE_BITS(NR_3DNR00, BITS(26, 26), 26, value)
    #define VQ_NR_W_c_line_buffer_repeat_right(value) \
	VQ_REG_WRITE_BITS(NR_3DNR00, BITS(12, 12), 12, value)
    #define VQ_NR_W_c_line_buffer_half_width(value) \
	VQ_REG_WRITE_BITS(NR_3DNR00, BITS(9, 0), 0, value)

#define NR_3DNR0D                          (VQ_NR_BASE + 0xC34)
    #define VQ_NR_W_c_line_buffer_mode(value) \
	VQ_REG_WRITE_BITS(NR_3DNR0D, BITS(31, 30), 30, value)
    #define VQ_NR_W_c_line_buffer_manual_length(value) \
	VQ_REG_WRITE_BITS(NR_3DNR0D, BITS(29, 20), 20, value)

#define SNRSM00                         (VQ_NR_BASE + 0xD40)
    #define VQ_NR_W_SNRSM00(value) \
	VQ_REG_WRITE(SNRSM00, value)

#define SNRSM01                         (VQ_NR_BASE + 0xD44)
    #define VQ_NR_W_SNRSM01(value) \
	VQ_REG_WRITE(SNRSM01, value)

#define SNR00                           (VQ_NR_BASE + 0xD80)
    #define VQ_NR_W_SNR00(value) \
	VQ_REG_WRITE(SNR00, value)

#define SNR01                           (VQ_NR_BASE + 0xD84)
    #define VQ_NR_W_SNR01(value) \
	VQ_REG_WRITE(SNR01, value)

#define SNR02                           (VQ_NR_BASE + 0xD88)
    #define VQ_NR_W_SNR02(value) \
	VQ_REG_WRITE(SNR02, value)

#define SNR03                           (VQ_NR_BASE + 0xD8C)
    #define VQ_NR_W_SNR03(value) \
	VQ_REG_WRITE(SNR03, value)

#define SNR04                           (VQ_NR_BASE + 0xD90)
    #define VQ_NR_W_SNR04(value) \
	VQ_REG_WRITE(SNR04, value)

#define SNR05                           (VQ_NR_BASE + 0xD94)
    #define VQ_NR_W_SNR05(value) \
	VQ_REG_WRITE(SNR05, value)

#define SNR06                           (VQ_NR_BASE + 0xD98)
    #define VQ_NR_W_SNR06(value) \
	VQ_REG_WRITE(SNR06, value)

#define SNR07                           (VQ_NR_BASE + 0xD9C)
    #define VQ_NR_W_SNR07(value) \
	VQ_REG_WRITE(SNR07, value)

#define SNR08                           (VQ_NR_BASE + 0xDA0)
    #define VQ_NR_W_SNR08(value) \
	VQ_REG_WRITE(SNR08, value)

#define SNR09                           (VQ_NR_BASE + 0xDA4)
    #define VQ_NR_W_SNR09(value) \
	VQ_REG_WRITE(SNR09, value)

#define SNR0A                           (VQ_NR_BASE + 0xDA8)
    #define VQ_NR_W_SNR0A(value) \
	VQ_REG_WRITE(SNR0A, value)

#define SNR0B                           (VQ_NR_BASE + 0xDAC)
    #define VQ_NR_W_SNR0B(value) \
	VQ_REG_WRITE(SNR0B, value)

#define SNR18                           (VQ_NR_BASE + 0xDE0)
    #define VQ_NR_W_SNR18(value) \
	VQ_REG_WRITE(SNR18, value)

#define SNR1A                           (VQ_NR_BASE + 0xDE8)
    #define VQ_NR_W_SNR1A(value) \
	VQ_REG_WRITE(SNR1A, value)

#define SNR1B                           (VQ_NR_BASE + 0xDEC)
    #define VQ_NR_W_SNR1B(value) \
	VQ_REG_WRITE(SNR1B, value)

#define SNR1C                           (VQ_NR_BASE + 0xDF0)
    #define VQ_NR_W_SNR1C(value) \
	VQ_REG_WRITE(SNR1C, value)

#define SNR1D                           (VQ_NR_BASE + 0xDF4)
    #define VQ_NR_W_SNR1D(value) \
	VQ_REG_WRITE(SNR1D, value)

#define SNR1E                           (VQ_NR_BASE + 0xDF8)
    #define VQ_NR_W_SNR1E(value) \
	VQ_REG_WRITE(SNR1E, value)

#define SNRSB00                         (VQ_NR_BASE + 0xDFC)
    #define VQ_NR_W_SNRSB00(value) \
	VQ_REG_WRITE(SNRSB00, value)

#define SNR1F                           (VQ_NR_BASE + 0xE00)
    #define VQ_NR_W_SNR1F(value) \
	VQ_REG_WRITE(SNR1F, value)
    #define VQ_NR_W_R_2D_ENABLE(value) \
	VQ_REG_WRITE_BITS(SNR1F, BITS(0, 0), 0, value)
    #define VQ_NR_W_R_BLOCK_PDP(value) \
	VQ_REG_WRITE_BITS(SNR1F, BITS(7, 7), 7, value)

#define SNR20                           (VQ_NR_BASE + 0xE04)
    #define VQ_NR_W_SNR20(value) \
	VQ_REG_WRITE(SNR20, value)

#define SNR21                           (VQ_NR_BASE + 0xE08)
    #define VQ_NR_W_SNR21(value) \
	VQ_REG_WRITE(SNR21, value)

#define SNR22                           (VQ_NR_BASE + 0xE0C)
    #define VQ_NR_W_SNR22(value) \
	VQ_REG_WRITE(SNR22, value)

#define SNR27                           (VQ_NR_BASE + 0xE20)
    #define VQ_NR_W_SNR27(value) \
	VQ_REG_WRITE(SNR27, value)

#define SNR28                           (VQ_NR_BASE + 0xE24)
    #define VQ_NR_W_SNR28(value) \
	VQ_REG_WRITE(SNR28, value)

#define SNR2B                           (VQ_NR_BASE + 0xE30)
    #define VQ_NR_W_SNR2B(value) \
	VQ_REG_WRITE(SNR2B, value)

#define SNR2C                           (VQ_NR_BASE + 0xE34)
    #define VQ_NR_W_SNR2C(value) \
	VQ_REG_WRITE(SNR2C, value)

#define SNR2D                           (VQ_NR_BASE + 0xE38)
    #define VQ_NR_W_SNR2D(value) \
	VQ_REG_WRITE(SNR2D, value)

#define SNR2E                           (VQ_NR_BASE + 0xE3C)
    #define VQ_NR_W_SNR2E(value) \
	VQ_REG_WRITE(SNR2E, value)

#define SNR2F                           (VQ_NR_BASE + 0xE40)
    #define VQ_NR_W_SNR2F(value) \
	VQ_REG_WRITE(SNR2F, value)

#define SNR30                           (VQ_NR_BASE + 0xE44)
    #define VQ_NR_W_SNR30(value) \
	VQ_REG_WRITE(SNR30, value)

#define SNR37                           (VQ_NR_BASE + 0xE60)
    #define VQ_NR_W_SNR37(value) \
	VQ_REG_WRITE(SNR37, value)
    #define VQ_NR_W_R_MESSSEL_SM_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR37, BITS(31, 30), 30, value)
    #define VQ_NR_W_R_MESSSFT_SM_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR37, BITS(29, 24), 24, value)
    #define VQ_NR_W_R_MESSTHL_SM_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR37, BITS(21, 16), 16, value)
    #define VQ_NR_W_R_MESSSEL_EDGE_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR37, BITS(15, 14), 14, value)
    #define VQ_NR_W_R_MESSSFT_EDGE_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR37, BITS(13, 8), 8, value)
    #define VQ_NR_W_R_MESSTHL_EDGE_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR37, BITS(5, 0), 0, value)

#define SNR38                           (VQ_NR_BASE + 0xE64)
    #define VQ_NR_W_SNR38(value) \
	VQ_REG_WRITE(SNR38, value)
    #define VQ_NR_W_R_MESSSEL_MESS_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR38, BITS(31, 30), 30, value)
    #define VQ_NR_W_R_MESSSFT_MESS_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR38, BITS(29, 24), 24, value)
    #define VQ_NR_W_R_MESSTHL_MESS_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR38, BITS(21, 16), 16, value)
    #define VQ_NR_W_R_MESSSEL_MOS_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR38, BITS(15, 14), 14, value)
    #define VQ_NR_W_R_MESSSFT_MOS_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR38, BITS(13, 8), 8, value)
    #define VQ_NR_W_R_MESSTHL_MOS_CO1MO(value) \
	VQ_REG_WRITE_BITS(SNR38, BITS(5, 0), 0, value)

#define SNR3B                           (VQ_NR_BASE + 0xE70)
    #define VQ_NR_W_SNR3B(value) \
	VQ_REG_WRITE(SNR3B, value)
    #define VQ_NR_W_R_MESSSEL_SM_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3B, BITS(31, 30), 30, value)
    #define VQ_NR_W_R_MESSSFT_SM_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3B, BITS(29, 24), 24, value)
    #define VQ_NR_W_R_MESSTHL_SM_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3B, BITS(21, 16), 16, value)
    #define VQ_NR_W_R_MESSSEL_EDGE_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3B, BITS(15, 14), 14, value)
    #define VQ_NR_W_R_MESSSFT_EDGE_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3B, BITS(13, 8), 8, value)
    #define VQ_NR_W_R_MESSTHL_EDGE_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3B, BITS(5, 0), 0, value)

#define SNR3C                           (VQ_NR_BASE + 0xE74)
    #define VQ_NR_W_SNR3C(value) \
	VQ_REG_WRITE(SNR3C, value)
    #define VQ_NR_W_R_MESSSEL_MESS_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3C, BITS(31, 30), 30, value)
    #define VQ_NR_W_R_MESSSFT_MESS_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3C, BITS(29, 24), 24, value)
    #define VQ_NR_W_R_MESSTHL_MESS_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3C, BITS(21, 16), 16, value)
    #define VQ_NR_W_R_MESSSEL_MOS_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3C, BITS(15, 14), 14, value)
    #define VQ_NR_W_R_MESSSFT_MOS_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3C, BITS(13, 8), 8, value)
    #define VQ_NR_W_R_MESSTHL_MOS_CO2MO(value) \
	VQ_REG_WRITE_BITS(SNR3C, BITS(5, 0), 0, value)

#define SNR3F                           (VQ_NR_BASE + 0xE80)
    #define VQ_NR_W_SNR3F(value) \
	VQ_REG_WRITE(SNR3F, value)
    #define VQ_NR_W_R_MESSSEL_SM_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR3F, BITS(31, 30), 30, value)
    #define VQ_NR_W_R_MESSSFT_SM_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR3F, BITS(29, 24), 24, value)
    #define VQ_NR_W_R_MESSTHL_SM_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR3F, BITS(21, 16), 16, value)
    #define VQ_NR_W_R_MESSSEL_EDGE_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR3F, BITS(15, 14), 14, value)
    #define VQ_NR_W_R_MESSSFT_EDGE_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR3F, BITS(13, 8), 8, value)
    #define VQ_NR_W_R_MESSTHL_EDGE_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR3F, BITS(5, 0), 0, value)

#define SNR40                           (VQ_NR_BASE + 0xE84)
    #define VQ_NR_W_SNR40(value) \
	VQ_REG_WRITE(SNR40, value)
    #define VQ_NR_W_R_MESSSEL_MESS_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR40, BITS(31, 30), 30, value)
    #define VQ_NR_W_R_MESSSFT_MESS_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR40, BITS(29, 24), 24, value)
    #define VQ_NR_W_R_MESSTHL_MESS_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR40, BITS(21, 16), 16, value)
    #define VQ_NR_W_R_MESSSEL_MOS_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR40, BITS(15, 14), 14, value)
    #define VQ_NR_W_R_MESSSFT_MOS_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR40, BITS(13, 8), 8, value)
    #define VQ_NR_W_R_MESSTHL_MOS_CO3MO(value) \
	VQ_REG_WRITE_BITS(SNR40, BITS(5, 0), 0, value)

#define SNR55                           (VQ_NR_BASE + 0xED8)
    #define VQ_NR_W_SNR55(value) \
	VQ_REG_WRITE(SNR55, value)
    #define VQ_NR_W_R_MESSSEL_SM_BK(value) \
	VQ_REG_WRITE_BITS(SNR55, BITS(31, 30), 30, value)
    #define VQ_NR_W_R_MESSSFT_SM_BK(value) \
	VQ_REG_WRITE_BITS(SNR55, BITS(29, 24), 24, value)
    #define VQ_NR_W_R_MESSTHL_SM_BK(value) \
	VQ_REG_WRITE_BITS(SNR55, BITS(21, 16), 16, value)
    #define VQ_NR_W_R_MESSSEL_EDGE_BK(value) \
	VQ_REG_WRITE_BITS(SNR55, BITS(15, 14), 14, value)
    #define VQ_NR_W_R_MESSSFT_EDGE_BK(value) \
	VQ_REG_WRITE_BITS(SNR55, BITS(13, 8), 8, value)
    #define VQ_NR_W_R_MESSTHL_EDGE_BK(value) \
	VQ_REG_WRITE_BITS(SNR55, BITS(5, 0), 0, value)

#define SNR56                           (VQ_NR_BASE + 0xEDC)
    #define VQ_NR_W_SNR56(value) \
	VQ_REG_WRITE(SNR56, value)
    #define VQ_NR_W_R_MESSSEL_MESS_BK(value) \
	VQ_REG_WRITE_BITS(SNR56, BITS(31, 30), 30, value)
    #define VQ_NR_W_R_MESSSFT_MESS_BK(value) \
	VQ_REG_WRITE_BITS(SNR56, BITS(29, 24), 24, value)
    #define VQ_NR_W_R_MESSTHL_MESS_BK(value) \
	VQ_REG_WRITE_BITS(SNR56, BITS(21, 16), 16, value)
    #define VQ_NR_W_R_MESSSEL_MOS_BK(value) \
	VQ_REG_WRITE_BITS(SNR56, BITS(15, 14), 14, value)
    #define VQ_NR_W_R_MESSSFT_MOS_BK(value) \
	VQ_REG_WRITE_BITS(SNR56, BITS(13, 8), 8, value)
    #define VQ_NR_W_R_MESSTHL_MOS_BK(value) \
	VQ_REG_WRITE_BITS(SNR56, BITS(5, 0), 0, value)

#define SNR57                           (VQ_NR_BASE + 0xEE0)
    #define VQ_NR_W_SNR57(value) \
	VQ_REG_WRITE(SNR57, value)
    #define VQ_NR_W_R_MESSSEL_SM_DEF(value) \
	VQ_REG_WRITE_BITS(SNR57, BITS(31, 30), 30, value)
    #define VQ_NR_W_R_MESSSFT_SM_DEF(value) \
	VQ_REG_WRITE_BITS(SNR57, BITS(29, 24), 24, value)
    #define VQ_NR_W_R_MESSTHL_SM_DEF(value) \
	VQ_REG_WRITE_BITS(SNR57, BITS(21, 16), 16, value)
    #define VQ_NR_W_R_MESSSEL_EDGE_DEF(value) \
	VQ_REG_WRITE_BITS(SNR57, BITS(15, 14), 14, value)
    #define VQ_NR_W_R_MESSSFT_EDGE_DEF(value) \
	VQ_REG_WRITE_BITS(SNR57, BITS(13, 8), 8, value)
    #define VQ_NR_W_R_MESSTHL_EDGE_DEF(value) \
	VQ_REG_WRITE_BITS(SNR57, BITS(5, 0), 0, value)

#define SNR58                           (VQ_NR_BASE + 0xEE4)
    #define VQ_NR_W_SNR58(value) \
	VQ_REG_WRITE(SNR58, value)
    #define VQ_NR_W_R_MESSSEL_MESS_DEF(value) \
	VQ_REG_WRITE_BITS(SNR58, BITS(31, 30), 30, value)
    #define VQ_NR_W_R_MESSSFT_MESS_DEF(value) \
	VQ_REG_WRITE_BITS(SNR58, BITS(29, 24), 24, value)
    #define VQ_NR_W_R_MESSTHL_MESS_DEF(value) \
	VQ_REG_WRITE_BITS(SNR58, BITS(21, 16), 16, value)
    #define VQ_NR_W_R_MESSSEL_MOS_DEF(value) \
	VQ_REG_WRITE_BITS(SNR58, BITS(15, 14), 14, value)
    #define VQ_NR_W_R_MESSSFT_MOS_DEF(value) \
	VQ_REG_WRITE_BITS(SNR58, BITS(13, 8), 8, value)
    #define VQ_NR_W_R_MESSTHL_MOS_DEF(value) \
	VQ_REG_WRITE_BITS(SNR58, BITS(5, 0), 0, value)

#define SNRSM11                         (VQ_NR_BASE + 0xEEC)
    #define VQ_NR_W_SNRSM11(value) \
	VQ_REG_WRITE(SNRSM11, value)

#define SNRSM12                         (VQ_NR_BASE + 0xEF0)
    #define VQ_NR_W_SNRSM12(value) \
	VQ_REG_WRITE(SNRSM12, value)

#define SNRSM13                          (VQ_NR_BASE + 0xEF4)
    #define VQ_NR_W_SNRSM13(value) \
	VQ_REG_WRITE(SNRSM13, value)

#define SNR92                            (VQ_NR_BASE + 0xEF8)
    #define VQ_NR_W_SNR92(value) \
	VQ_REG_WRITE(SNR92, value)

#define SNR59                             (VQ_NR_BASE + 0xEFC)
    #define VQ_NR_W_SNR59(value) \
	VQ_REG_WRITE(SNR59, value)

#define SNR5A                             (VQ_NR_BASE + 0xF00)
    #define VQ_NR_W_SNR5A(value) \
	VQ_REG_WRITE(SNR5A, value)

#define SNR5B                             (VQ_NR_BASE + 0xF04)
    #define VQ_NR_W_SNR5B(value) \
	VQ_REG_WRITE(SNR5B, value)

#define SNR5C                             (VQ_NR_BASE + 0xF08)
    #define VQ_NR_W_SNR5C(value) \
	VQ_REG_WRITE(SNR5C, value)

#define SNR5E                             (VQ_NR_BASE + 0xF10)
    #define VQ_NR_W_SNR5E(value) \
	VQ_REG_WRITE(SNR5E, value)

#define SNR60                             (VQ_NR_BASE + 0xF18)
    #define VQ_NR_W_SNR60(value) \
	VQ_REG_WRITE(SNR60, value)

#define SNR63                             (VQ_NR_BASE + 0xF24)
    #define VQ_NR_W_SNR63(value) \
	VQ_REG_WRITE(SNR63, value)

#define SNR64                             (VQ_NR_BASE + 0xF28)
    #define VQ_NR_W_SNR64(value) \
	VQ_REG_WRITE(SNR64, value)

#define SNR65                             (VQ_NR_BASE + 0xF2C)
    #define VQ_NR_W_SNR65(value) \
	VQ_REG_WRITE(SNR65, value)

#define SNR66                             (VQ_NR_BASE + 0xF30)
    #define VQ_NR_W_SNR66(value) \
	VQ_REG_WRITE(SNR66, value)

#define SNR67                             (VQ_NR_BASE + 0xF34)
    #define VQ_NR_W_SNR67(value) \
	VQ_REG_WRITE(SNR67, value)

#define SNR68                             (VQ_NR_BASE + 0xF38)
    #define VQ_NR_W_SNR68(value) \
	VQ_REG_WRITE(SNR68, value)

#define SNR69                             (VQ_NR_BASE + 0xF3C)
    #define VQ_NR_W_SNR69(value) \
	VQ_REG_WRITE(SNR69, value)

#define SNR6A                             (VQ_NR_BASE + 0xF40)
    #define VQ_NR_W_SNR6A(value) \
	VQ_REG_WRITE(SNR6A, value)

#define SNR6C                             (VQ_NR_BASE + 0xF48)
    #define VQ_NR_W_SNR6C(value) \
	VQ_REG_WRITE(SNR6C, value)

#define SNR6E                             (VQ_NR_BASE + 0xF50)
    #define VQ_NR_W_SNR6E(value) \
	VQ_REG_WRITE(SNR6E, value)

#define SNR6F                             (VQ_NR_BASE + 0xF54)
    #define VQ_NR_W_SNR6F(value) \
	VQ_REG_WRITE(SNR6F, value)

#define SNR70                             (VQ_NR_BASE + 0xF58)
    #define VQ_NR_W_SNR70(value) \
	VQ_REG_WRITE(SNR70, value)

#define SNR71                             (VQ_NR_BASE + 0xF60)
    #define VQ_NR_W_SNR71(value) \
	VQ_REG_WRITE(SNR71, value)

#define SNR72                             (VQ_NR_BASE + 0xF64)
    #define VQ_NR_W_SNR72(value) \
	VQ_REG_WRITE(SNR72, value)

#define SNR73                             (VQ_NR_BASE + 0xF68)
    #define VQ_NR_W_SNR73(value) \
	VQ_REG_WRITE(SNR73, value)

#define SNR77                             (VQ_NR_BASE + 0xF78)
    #define VQ_NR_W_SNR77(value) \
	VQ_REG_WRITE(SNR77, value)

#define SNR78                             (VQ_NR_BASE + 0xF7C)
    #define VQ_NR_W_SNR78(value) \
	VQ_REG_WRITE(SNR78, value)

#define SNRBK00                           (VQ_NR_BASE + 0xF84)
    #define VQ_NR_W_SNRBK00(value) \
	VQ_REG_WRITE(SNRBK00, value)

#define SNRBK01                           (VQ_NR_BASE + 0xF88)
    #define VQ_NR_W_SNRBK01(value) \
	VQ_REG_WRITE(SNRBK01, value)

#define SNRBK02                           (VQ_NR_BASE + 0xF8C)
    #define VQ_NR_W_SNRBK02(value) \
	VQ_REG_WRITE(SNRBK02, value)

#define SNRBK03                           (VQ_NR_BASE + 0xF90)
    #define VQ_NR_W_SNRBK03(value) \
	VQ_REG_WRITE(SNRBK03, value)

#define SNRBK04                           (VQ_NR_BASE + 0xF94)
    #define VQ_NR_W_SNRBK04(value) \
	VQ_REG_WRITE(SNRBK04, value)

#define SNR7E                             (VQ_NR_BASE + 0xF98)
    #define VQ_NR_W_SNR7E(value) \
	VQ_REG_WRITE(SNR7E, value)
    #define VQ_NR_W_R_UIBLDLV_BK_DEF(value) \
	VQ_REG_WRITE_BITS(SNR7E, BITS(15, 12), 12, value)
    #define VQ_NR_W_R_UIBLDLV_SM_DEF(value) \
	VQ_REG_WRITE_BITS(SNR7E, BITS(11, 8), 8, value)
    #define VQ_NR_W_R_UIBLDLV_MESS_DEF(value) \
	VQ_REG_WRITE_BITS(SNR7E, BITS(7, 4), 4, value)
    #define VQ_NR_W_R_UIBLDLV_EDGE_DEF(value) \
	VQ_REG_WRITE_BITS(SNR7E, BITS(3, 0), 0, value)

#define SNR7F                             (VQ_NR_BASE + 0xF9C)
    #define VQ_NR_W_SNR7F(value) \
	VQ_REG_WRITE(SNR7F, value)
    #define VQ_NR_W_R_UIBLDLV_BK_BK(value) \
	VQ_REG_WRITE_BITS(SNR7F, BITS(31, 28), 28, value)
    #define VQ_NR_W_R_UIBLDLV_SM_BK(value) \
	VQ_REG_WRITE_BITS(SNR7F, BITS(27, 24), 24, value)
    #define VQ_NR_W_R_UIBLDLV_MESS_BK(value) \
	VQ_REG_WRITE_BITS(SNR7F, BITS(23, 20), 20, value)
    #define VQ_NR_W_R_UIBLDLV_EDGE_BK(value) \
	VQ_REG_WRITE_BITS(SNR7F, BITS(19, 16), 16, value)

#define SNR80                             (VQ_NR_BASE + 0xFA0)
    #define VQ_NR_W_SNR80(value) \
	VQ_REG_WRITE(SNR80, value)
    #define VQ_NR_W_R_UIBLDLV_BK_CO1(value) \
	VQ_REG_WRITE_BITS(SNR80, BITS(15, 12), 12, value)
    #define VQ_NR_W_R_UIBLDLV_SM_CO1(value) \
	VQ_REG_WRITE_BITS(SNR80, BITS(11, 8), 8, value)
    #define VQ_NR_W_R_UIBLDLV_MESS_CO1(value) \
	VQ_REG_WRITE_BITS(SNR80, BITS(7, 4), 4, value)
    #define VQ_NR_W_R_UIBLDLV_EDGE_CO1(value) \
	VQ_REG_WRITE_BITS(SNR80, BITS(3, 0), 0, value)

#define SNR81                             (VQ_NR_BASE + 0xFA4)
    #define VQ_NR_W_SNR81(value) \
	VQ_REG_WRITE(SNR81, value)
    #define VQ_NR_W_R_UIBLDLV_BK_CO2(value) \
	VQ_REG_WRITE_BITS(SNR81, BITS(31, 28), 28, value)
    #define VQ_NR_W_R_UIBLDLV_SM_CO2(value) \
	VQ_REG_WRITE_BITS(SNR81, BITS(27, 24), 24, value)
    #define VQ_NR_W_R_UIBLDLV_MESS_CO2(value) \
	VQ_REG_WRITE_BITS(SNR81, BITS(23, 20), 20, value)
    #define VQ_NR_W_R_UIBLDLV_EDGE_CO2(value) \
	VQ_REG_WRITE_BITS(SNR81, BITS(19, 16), 16, value)
    #define VQ_NR_W_R_UIBLDLV_BK_CO3(value) \
	VQ_REG_WRITE_BITS(SNR81, BITS(15, 12), 12, value)
    #define VQ_NR_W_R_UIBLDLV_SM_CO3(value) \
	VQ_REG_WRITE_BITS(SNR81, BITS(11, 8), 8, value)
    #define VQ_NR_W_R_UIBLDLV_MESS_CO3(value) \
	VQ_REG_WRITE_BITS(SNR81, BITS(7, 4), 4, value)
    #define VQ_NR_W_R_UIBLDLV_EDGE_CO3(value) \
	VQ_REG_WRITE_BITS(SNR81, BITS(3, 0), 0, value)

#define SNR83                             (VQ_NR_BASE + 0xFAC)
    #define VQ_NR_W_SNR83(value) \
	VQ_REG_WRITE(SNR83, value)
    #define VQ_NR_W_R_UIBLDLV_MOS_BK(value) \
	VQ_REG_WRITE_BITS(SNR83, BITS(31, 28), 28, value)
    #define VQ_NR_W_R_UIBLDLV_MOS_DEF(value) \
	VQ_REG_WRITE_BITS(SNR83, BITS(19, 16), 16, value)

#define SNR84                             (VQ_NR_BASE + 0xFB0)
    #define VQ_NR_W_SNR84(value) \
	VQ_REG_WRITE(SNR84, value)
    #define VQ_NR_W_R_UIBLENDOUTLV_C(value) \
	VQ_REG_WRITE_BITS(SNR84, BITS(31, 28), 28, value)
    #define VQ_NR_W_R_UIBLDLV_MOS_CO3(value) \
	VQ_REG_WRITE_BITS(SNR84, BITS(15, 12), 12, value)
    #define VQ_NR_W_R_UIBLDLV_MOS_CO2(value) \
	VQ_REG_WRITE_BITS(SNR84, BITS(11, 8), 8, value)
    #define VQ_NR_W_R_UIBLDLV_MOS_CO1(value) \
	VQ_REG_WRITE_BITS(SNR84, BITS(7, 4), 4, value)

#define SNR85                             (VQ_NR_BASE + 0xFB4)
    #define VQ_NR_W_SNR85(value) \
	VQ_REG_WRITE(SNR85, value)

#define SNR86                             (VQ_NR_BASE + 0xFB8)
    #define VQ_NR_W_SNR86(value) \
	VQ_REG_WRITE(SNR86, value)

#define SNR87                             (VQ_NR_BASE + 0xFBC)
    #define VQ_NR_W_SNR87(value) \
	VQ_REG_WRITE(SNR87, value)

#define SNR88                             (VQ_NR_BASE + 0xFC0)
    #define VQ_NR_W_SNR88(value) \
	VQ_REG_WRITE(SNR88, value)

#define SNR89                             (VQ_NR_BASE + 0xFC4)
    #define VQ_NR_W_SNR89(value) \
	VQ_REG_WRITE(SNR89, value)

#define SNR8B                             (VQ_NR_BASE + 0xFCC)
    #define VQ_NR_W_SNR8B(value) \
	VQ_REG_WRITE(SNR8B, value)

#define SNR8C                             (VQ_NR_BASE + 0xFD0)
    #define VQ_NR_W_SNR8C(value) \
	VQ_REG_WRITE(SNR8C, value)

#define SNR8D                             (VQ_NR_BASE + 0xFD4)
    #define VQ_NR_W_SNR8D(value) \
	VQ_REG_WRITE(SNR8D, value)

#define SNR8E                             (VQ_NR_BASE + 0xFD8)
    #define VQ_NR_W_SNR8E(value) \
	VQ_REG_WRITE(SNR8E, value)

#define SNRBK05                           (VQ_NR_BASE + 0xFDC)
    #define VQ_NR_W_SNRBK05(value) \
	VQ_REG_WRITE(SNRBK05, value)

#define SNRBK06                           (VQ_NR_BASE + 0xFE0)
    #define VQ_NR_W_SNRBK06(value) \
	VQ_REG_WRITE(SNRBK06, value)

#define SNRBK07                           (VQ_NR_BASE + 0xFE4)
    #define VQ_NR_W_SNRBK07(value) \
	VQ_REG_WRITE(SNRBK07, value)

#define SNRBK08                           (VQ_NR_BASE + 0xFE8)
    #define VQ_NR_W_SNRBK08(value) \
	VQ_REG_WRITE(SNRBK08, value)

#endif /* VQ_NR_REG_H */

