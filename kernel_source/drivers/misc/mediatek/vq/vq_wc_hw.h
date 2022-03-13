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

#ifndef VQ_WC_HW_H
#define VQ_WC_HW_H

#include "vq_def.h"

struct VQ_WC_PARAM_T {
	unsigned int				u4Width;
	unsigned int				u4Height;
	unsigned int				u4OutputAddrY;
	unsigned int				u4OutputAddrCbcr;

	enum VQ_TIMING_TYPE_E			eTimingType;
	enum VQ_COLOR_FMT_E			eDstColorFmt;
};

extern int iVQ_WC_SetParam(struct VQ_WC_PARAM_T *prParam);

#endif /* VQ_WC_HW_H */

