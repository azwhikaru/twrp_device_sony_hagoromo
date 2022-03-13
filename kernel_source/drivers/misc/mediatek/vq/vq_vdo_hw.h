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

#ifndef VQ_VDO_HW_H
#define VQ_VDO_HW_H

#include "vq_def.h"

enum VDO_DI_MODE {
	VDO_FRAME_MODE,
	VDO_FIELD_MODE,
	VDO_INTRA_MODE_WITH_EDGE_PRESERVING,
	VDO_4FIELD_MA_MODE,
	VDO_FUSION_MODE,
	VDO_8FIELD_MA_MODE,
	VDO_DI_MODE_NUM
};

enum VDO_SRC_FORMAT {
	VDO_SRC_FMT_YUV420_BLK,
	VDO_SRC_FMT_YUV422_BLK,
	VDO_SRC_FMT_YUV420_SCL,
	VDO_SRC_FMT_YUV422_SCL,
	VDO_SRC_FMT_NUM
};

enum VDO_YUV420_START_LINE_MODE {
	VDO_YUV420_START_LINE_MODE0,
	VDO_YUV420_START_LINE_MODE1,
	VDO_YUV420_START_LINE_MODE2,
	VDO_YUV420_START_LINE_MODE3,
	VDO_YUV420_START_LINE_MODE4,
	VDO_YUV420_START_LINE_MODE_NUM
};

enum VDO_YUV422_START_LINE_MODE {
	VDO_YUV422_START_LINE_MODE0,
	VDO_YUV422_START_LINE_MODE1,
	VDO_YUV422_START_LINE_MODE2,
	VDO_YUV422_START_LINE_MODE3,
	VDO_YUV422_START_LINE_MODE_NUM
};

enum VDO_VERTICAL_FILTER_MODE {
	VDO_VERTICAL_FILTER_LINEAR,
	VDO_VERTICAL_FILTER_4TAP,
	VDO_VERTICAL_FILTER_8TAP,
	VDO_VERTICAL_FILTER_NUM
};

struct VDO_HW_SRAM_UTILIZATION_T {
	unsigned int HD_MEM_1920;
	unsigned int HD_MEM;
	unsigned int HD_EN;
};

struct VDO_START_LINE_T {
	unsigned int StartLineForY;
	unsigned int StartLineForC;
	unsigned int StarSubLineForY;
	unsigned int StarSubLineForC;
	unsigned int AlternatvieStartLineForY;
	unsigned int AlternatvieStartLineForC;
};

struct VQ_VDO_PARAM_T {
	unsigned char				u1EnableVerticalChromaDetect;
	unsigned char				u1EnablePreSharp;
	unsigned char				u1CurrentIsTop;
	unsigned char				u1TopFieldFirst;

	unsigned int				u4Width;
	unsigned int				u4Height;

	unsigned int				u4AddrYPrev;
	unsigned int				u4AddrYCurr;
	unsigned int				u4AddrYNext;

	unsigned int				u4AddrCbcrPrev;
	unsigned int				u4AddrCbcrCurr;
	unsigned int				u4AddrCbcrNext;

	enum VDO_DI_MODE		eDiMode;
	enum VDO_SRC_FORMAT		eSrcFmt;
	enum VQ_TIMING_TYPE_E		eTimingType;
};

extern int iVQ_Vdo_SetParam(struct VQ_VDO_PARAM_T *prParam);
extern int iVQ_Vdo_Enable(unsigned char u1Enable);

#endif /* VQ_VDO_HW_H */

