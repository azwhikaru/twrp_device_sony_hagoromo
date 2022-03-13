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

#ifndef VQ_WC_REG_H
#define VQ_WC_REG_H

#include "vq_def.h"


/* write channel 1 */
#define VQ_WC_BASE                          (VQ_IO_BASE + 0x3000)

#define VDOIN_EN                            (VQ_WC_BASE + 0x000)
    #define VQ_WC_W_VDOIN_EN(value) \
	VQ_REG_WRITE(VDOIN_EN, value)
    #define VQ_WC_W_mode_422(value) \
	VQ_REG_WRITE_BITS(VDOIN_EN, BITS(25, 25), 25, value)
    #define VQ_WC_W_linear_enable(value) \
	VQ_REG_WRITE_BITS(VDOIN_EN, BITS(14, 14), 14, value)

#define VDOIN_MODE                          (VQ_WC_BASE + 0x004)
    #define VQ_WC_W_VDOIN_MODE(value) \
	VQ_REG_WRITE(VDOIN_MODE, value)

#define YBUF_ADDR                           (VQ_WC_BASE + 0x008)
    #define VQ_WC_W_YBUF0_ADDR(value) \
	VQ_REG_WRITE_BITS(YBUF_ADDR, BITS(29, 0), 0, value)

#define ACT_LINE                            (VQ_WC_BASE + 0x00C)
    #define VQ_WC_W_ACT_LINE(value) \
	VQ_REG_WRITE(ACT_LINE, value)
    #define VQ_WC_W_ACTLINE(value) \
	VQ_REG_WRITE_BITS(ACT_LINE, BITS(11, 0), 0, value)

#define CBUF_ADDR                           (VQ_WC_BASE + 0x010)
    #define VQ_WC_W_CBUF0_ADDR(value) \
	VQ_REG_WRITE_BITS(CBUF_ADDR, BITS(29, 0), 0, value)

#define DW_NEED                             (VQ_WC_BASE + 0x014)
    #define VQ_WC_W_DW_NEED_C_LINE(value) \
	VQ_REG_WRITE_BITS(DW_NEED, BITS(27, 16), 16, value)
    #define VQ_WC_W_DW_NEED_Y_LINE(value) \
	VQ_REG_WRITE_BITS(DW_NEED, BITS(11, 0), 0, value)

#define HPIXEL                              (VQ_WC_BASE + 0x018)
    #define VQ_WC_W_HPIXEL(value) \
	VQ_REG_WRITE(HPIXEL, value)

#define INPUT_CTRL                          (VQ_WC_BASE + 0x020)
    #define VQ_WC_W_INPUT_CTRL(value) \
	VQ_REG_WRITE(INPUT_CTRL, value)

#define WRAPPER_3D_SETTING                  (VQ_WC_BASE + 0x030)
    #define VQ_WC_W_swrst(value) \
	VQ_REG_WRITE_BITS(WRAPPER_3D_SETTING, BITS(31, 30), 30, value)

#define HCNT_SETTING                        (VQ_WC_BASE + 0x034)
    #define VQ_WC_W_HCNT_SETTING(value) \
	VQ_REG_WRITE(HCNT_SETTING, value)
    #define VQ_WC_W_HACTCNT(value) \
	VQ_REG_WRITE_BITS(HCNT_SETTING, BITS(28, 16), 16, value)

#define HCNT_SETTING_1                      (VQ_WC_BASE + 0x038)
    #define VQ_WC_W_CHCNT(value) \
	VQ_REG_WRITE_BITS(HCNT_SETTING_1, BITS(25, 16), 16, value)
    #define VQ_WC_W_YHCNT(value) \
	VQ_REG_WRITE_BITS(HCNT_SETTING_1, BITS(9, 0), 0, value)

#define VSCALE                              (VQ_WC_BASE + 0x03C)
    #define VQ_WC_W_VSCALE(value) \
	VQ_REG_WRITE(VSCALE, value)
    #define VQ_WC_W_bghsize_dw(value) \
	VQ_REG_WRITE_BITS(VSCALE, BITS(25, 16), 16, value)


#endif /* VQ_WC_REG_H */

