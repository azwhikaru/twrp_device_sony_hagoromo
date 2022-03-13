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
#ifndef __MTK_V4L2_H__
#define __MTK_V4L2_H__

#include <linux/videodev2.h>

#ifdef __cplusplus
extern "C" {
#endif

/* two planes -- one Y, one Cr + Cb interleaved  */
#define V4L2_PIX_FMT_NV12_MTK_BLK   v4l2_fourcc('M', 'K', '1', '2') /* 12  Y/CbCr 4:2:0 16x32 macroblock  */


#ifdef __cplusplus
}
#endif
#endif				/* __MTK_V4L2_H__ */
