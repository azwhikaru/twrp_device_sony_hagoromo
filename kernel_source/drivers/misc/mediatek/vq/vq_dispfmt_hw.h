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

#ifndef VQ_DISPFMT_HW_H
#define VQ_DISPFMT_HW_H

#include "vq_def.h"

struct VQ_DISPFMT_PATTERN_T {
	unsigned char     u1Enable;
	unsigned char     u1422_444;
	unsigned short    u2Type;
	unsigned short    u2Width;
};

struct VQ_DISPFMT_PARAM_T {
	unsigned char				u1NrEnable;

	unsigned int				u4Width;
	unsigned int				u4Height;

	enum VQ_TIMING_TYPE_E			eTimingType;

	struct VQ_DISPFMT_PATTERN_T		rPattern;
};

extern int iVQ_Dispfmt_SetParam(struct VQ_DISPFMT_PARAM_T *prParam);
extern int iVQ_Dispfmt_ClkOn(void);
extern int iVQ_Dispfmt_ClkOff(void);
extern int iVQ_Dispfmt_IrqReg(unsigned char u1Enable);
extern int iVQ_Dispfmt_Triggle(struct VQ_DISPFMT_PARAM_T *prParam);

#endif /* VQ_DISPFMT_HW_H */

