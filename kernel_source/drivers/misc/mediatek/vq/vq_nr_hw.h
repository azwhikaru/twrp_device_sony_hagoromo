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

#ifndef VQ_NR_HW_H
#define VQ_NR_HW_H

#include "vq_def.h"

struct VQ_NR_PATTERN_T {
	unsigned char			u1EnablePattern;
	unsigned int			u4ColorY;
	unsigned int			u4ColorC;
};

struct VQ_NR_CRC_T {
	unsigned char			u1EnableCrc;
	unsigned char			u1CrcType;
};

struct VQ_NR_DEMO_T {
	unsigned char			u1EnableDemo;
	unsigned char			u1DemoType;
};

struct VQ_NR_PARAM_T {
	unsigned int			u4MnrLevel;
	unsigned int			u4BnrLevel;

	struct VQ_NR_PATTERN_T		rPattern;
	struct VQ_NR_CRC_T		rCrc;
	struct VQ_NR_DEMO_T		rDemo;
};

extern int iVQ_Nr_SetParam(struct VQ_NR_PARAM_T *prParam);

#endif /* VQ_NR_HW_H */

