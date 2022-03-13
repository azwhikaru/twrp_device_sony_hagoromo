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
// ---------------------------------------------------------------------------
#ifndef     VIDEO_IN_IF_H
#define     VIDEO_IN_IF_H

#include <linux/hdmi_common.h>

#define BUF_COUNT		20
#define  VIN_GET_INFO_TYPE_RES        (0x1<<0)
#define  VIN_GET_INFO_TYPE_ASR        (0x1<<1)
#define  VIN_GET_INFO_TYPE_DATA      (0x1<<2)
#define  VIN_GET_INFO_TYPE_SIGNAL    (0x1<<3)
#define  VIN_GET_INFO_TYPE_SRC_COLORSPACE (0x1<<4)
#define  VIN_GET_INFO_TYPE_SRC_DEEPCOLOR (0x1<<5)


#define  VIN_GET_INFO_TYPE_ALL \
(VIN_GET_INFO_TYPE_RES | VIN_GET_INFO_TYPE_ASR | VIN_GET_INFO_TYPE_DATA|VIN_GET_INFO_TYPE_SIGNAL|VIN_GET_INFO_TYPE_SRC_COLORSPACE|VIN_GET_INFO_TYPE_SRC_DEEPCOLOR)

typedef HDMI_RESOLUTION_MODE_T HDMI_RESOLUTION_T ;

#define fgIsYCbCr444(u1Res) ((u1Res == YC444_24BIT) ||\
								 (u1Res == YC444_30BIT) ||\
								 (u1Res == YC444_36BIT))

#define fgIsYCbCr422(u1Res)  ((u1Res == YC422_16BIT) ||\
									 (u1Res == YC422_20BIT) ||\
									 (u1Res == YC422_24BIT))


#define fgIs24HzFrameRate(u1Res) ((u1Res == RES_1080P23_976HZ) || (u1Res == RES_1080P24HZ)||\
                                  (u1Res == RES_3D_1080P23HZ_TB) || (u1Res == RES_3D_1080P24HZ_TB)||\
                                  (u1Res == RES_3D_720P24HZ_TB)|| (u1Res == RES_3D_720P23HZ_TB)||\
                                  (u1Res == RES_3D_720P24HZ_SBS_HALF)|| (u1Res == RES_3D_720P23HZ_SBS_HALF)||\
                                  (u1Res == RES_3D_720P24HZ_SBS_HALF)|| (u1Res == RES_3D_720P23HZ_SBS_HALF) )

#define fgIs25HzFrameRate(u1Res) ((u1Res == RES_1080P25HZ)||(u1Res == RES_720P25HZ)||\
	                                                   (u1Res == RES_3D_1080P25HZ_TB)||(u1Res == RES_3D_720P25HZ_TB)||\
	                                                   (u1Res == RES_3D_1080P25HZ_SBS_HALF)||(u1Res == RES_3D_720P25HZ_SBS_HALF))

#define fgIs30HzFrameRate(u1Res) ((u1Res == RES_1080P30HZ)||(u1Res == RES_720P30HZ)||\
	                                                   (u1Res == RES_3D_1080P30HZ_TB)||(u1Res == RES_3D_720P30HZ_TB)||\
	                                                   (u1Res == RES_3D_1080P30HZ_SBS_HALF)||(u1Res == RES_3D_720P30HZ_SBS_HALF)||\
	                                                   (u1Res == RES_1080P29_97HZ)||\
	                                                   (u1Res == RES_3D_1080P29HZ_TB)||\
	                                                   (u1Res == RES_3D_1080P29HZ_SBS_HALF) )

#define fgIs50HzFrameRate(u1Res) ((u1Res == RES_576I) || (u1Res == RES_576I_2880) ||\
                             (u1Res == RES_576P)|| (u1Res == RES_576P_1440) ||\
                             (u1Res == RES_576P_2880)|| (u1Res == RES_720P50HZ)||\
                             (u1Res == RES_1080I50HZ)||(u1Res == RES_1080P50HZ)||\
                             (u1Res == RES_2D_576I50HZ)||(u1Res == RES_3D_720P50HZ_TB)||\
                             (u1Res == RES_3D_576I50HZ_TB)||(u1Res == RES_3D_576I50HZ_SBS_HALF)||\
                             (u1Res == RES_3D_576I25HZ_TB)||(u1Res == RES_3D_576I25HZ_SBS_HALF)||\
                             (u1Res == RES_3D_576P50HZ_TB)||(u1Res == RES_3D_576P50HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080I50HZ_TB)||(u1Res == RES_3D_1080I50HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080I25HZ_TB)||(u1Res == RES_3D_1080I25HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080P50HZ_TB)||(u1Res == RES_3D_1080P50HZ_SBS_HALF)||\
                             (u1Res == RES_3D_720P50HZ_SBS_HALF))

#define fgIs60HzFrameRate(u1Res) ((u1Res == RES_480I) || (u1Res == RES_480I_2880) ||\
                             (u1Res == RES_480P)|| (u1Res == RES_480P_1440) ||\
                             (u1Res == RES_480P_2880)|| (u1Res == RES_720P60HZ)||\
                             (u1Res == RES_1080I60HZ)||(u1Res == RES_1080P60HZ)||\
                             (u1Res == RES_2D_480I60HZ)||(u1Res == RES_3D_720P60HZ_TB)||\
                             (u1Res == RES_3D_1080P60HZ_TB)||(u1Res == RES_3D_1080P60HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080I30HZ_TB)||(u1Res == RES_3D_1080I30HZ_SBS_HALF)||\
                             (u1Res == RES_3D_480I60HZ_TB)||(u1Res == RES_3D_480I60HZ_SBS_HALF)||\
                             (u1Res == RES_3D_480I30HZ_TB)||(u1Res == RES_3D_480I30HZ_SBS_HALF)||\
                             (u1Res == RES_3D_480P60HZ_TB)||(u1Res == RES_3D_480P60HZ_SBS_HALF)||\
                             (u1Res == RES_3D_720P60HZ_TB)||(u1Res == RES_3D_720P60HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080I60HZ_TB)||(u1Res == RES_2D_640x480HZ)||\
                             (u1Res == RES_3D_1080I60HZ_SBS_HALF))

#define fgIs48HzFrameRate(u1Res) ((u1Res == RES_3D_1080P23HZ) || (u1Res == RES_3D_1080P24HZ)||\
                                                           (u1Res == RES_3D_720P23HZ) || (u1Res == RES_3D_720P24HZ))

#define fgIs100HzFrameRate(u1Res) ((u1Res == RES_3D_720P25HZ) || (u1Res == RES_3D_720P50HZ) ||\
                             (u1Res == RES_3D_576I50HZ)|| (u1Res == RES_3D_576P50HZ) ||\
                             (u1Res == RES_3D_1080P50HZ) ||\
                             (u1Res == RES_3D_1080I25HZ)|| (u1Res == RES_3D_1080I50HZ))

#define fgIs120HzFrameRate(u1Res) ((u1Res == RES_3D_720P30HZ) || (u1Res == RES_3D_720P60HZ) ||\
                             (u1Res == RES_3D_480I60HZ)|| (u1Res == RES_3D_480P60HZ) ||\
                             (u1Res == RES_3D_1080P60HZ) ||\
                             (u1Res == RES_3D_1080I30HZ)|| (u1Res == RES_3D_1080I60HZ))

#define fgIsProgress(u1Res) ((u1Res == RES_480P) || (u1Res == RES_576P) ||\
                             (u1Res == RES_480P_1440)|| (u1Res == RES_576P_1440)||\
                             (u1Res == RES_480P_2880)|| (u1Res == RES_576P_2880)||\
                             (u1Res == RES_720P60HZ)||(u1Res == RES_720P50HZ)||\
                             (u1Res == RES_1080P60HZ)||(u1Res == RES_1080P50HZ)||\
                             (u1Res == RES_1080P30HZ)|| (u1Res == RES_1080P25HZ)||\
                             (u1Res == RES_1080P24HZ)|| (u1Res == RES_1080P23_976HZ)||\
                             (u1Res == RES_1080P29_97HZ)||\
                             (u1Res == RES_3D_1080P23HZ)||(u1Res == RES_3D_1080P24HZ)||\
                             (u1Res == RES_3D_720P60HZ)||(u1Res == RES_3D_720P50HZ)||\
                             (u1Res == RES_3D_720P30HZ)||(u1Res == RES_3D_720P25HZ)||\
                             (u1Res == RES_3D_480P60HZ)||(u1Res == RES_3D_576P50HZ)||\
                             (u1Res == RES_2D_640x480HZ)||\
                             (u1Res == RES_3D_720P60HZ_TB)||\
                             (u1Res == RES_3D_720P50HZ_TB)||\
                             (u1Res == RES_3D_1080P23HZ_TB)||\
                             (u1Res == RES_3D_1080P24HZ_TB)||\
                             (u1Res == RES_2160P_23_976HZ)||\
                             (u1Res == RES_2160P_24HZ)||\
                             (u1Res == RES_2160P_25HZ)||\
                             (u1Res == RES_2160P_29_97HZ)||\
                             (u1Res == RES_2160P_30HZ)||\
                             (u1Res == RES_2161P_24HZ)||\
                             (u1Res == RES_720P30HZ)||\
                             (u1Res == RES_720P25HZ)||\
                             (u1Res == RES_720P24HZ)||\
                             (u1Res == RES_720P23HZ)||\
                             (u1Res == RES_3D_1080P60HZ)||\
                             (u1Res == RES_3D_1080P50HZ)||\
                             (u1Res == RES_3D_1080P30HZ)||\
                             (u1Res == RES_3D_1080P29HZ)||\
                             (u1Res == RES_3D_1080P25HZ)||\
                             (u1Res == RES_3D_720P24HZ)||\
                             (u1Res == RES_3D_720P23HZ)||\
                             (u1Res == RES_3D_1080P60HZ_TB)||\
                             (u1Res == RES_3D_1080P50HZ_TB)||\
                             (u1Res == RES_3D_1080P30HZ_TB)||\
                             (u1Res == RES_3D_1080P29HZ_TB)||\
                             (u1Res == RES_3D_1080P25HZ_TB)||\
                             (u1Res == RES_3D_720P30HZ_TB)||\
                             (u1Res == RES_3D_720P25HZ_TB)||\
                             (u1Res == RES_3D_720P24HZ_TB)||\
                             (u1Res == RES_3D_720P23HZ_TB)||\
                             (u1Res == RES_3D_576P50HZ_TB)||\
                             (u1Res == RES_3D_480P60HZ_TB)||\
                             (u1Res == RES_3D_1080P60HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080P50HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080P30HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080P29HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080P25HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080P24HZ_SBS_HALF)||\
                             (u1Res == RES_3D_1080P23HZ_SBS_HALF)||\
                             (u1Res == RES_3D_720P60HZ_SBS_HALF)||\
                             (u1Res == RES_3D_720P50HZ_SBS_HALF)||\
                             (u1Res == RES_3D_720P30HZ_SBS_HALF)||\
                             (u1Res == RES_3D_720P25HZ_SBS_HALF)||\
                             (u1Res == RES_3D_720P24HZ_SBS_HALF)||\
                             (u1Res == RES_3D_720P23HZ_SBS_HALF)||\
                             (u1Res == RES_3D_576P50HZ_SBS_HALF)||\
                             (u1Res == RES_3D_480P60HZ_SBS_HALF)  )

#define fgIs3DInterlaceRes(u1Res) ((u1Res == RES_3D_1080I60HZ) || (u1Res == RES_3D_1080I50HZ))
#define fgIs2FsVideo(u1Res) ((u1Res == RES_480I) || (u1Res == RES_576I) ||\
                             (u1Res == RES_480P_1440) || (u1Res == RES_576P_1440) ||\
                             (u1Res == RES_2D_480I60HZ)||(u1Res == RES_2D_576I50HZ)||\
                             (u1Res == RES_3D_480I60HZ_TB) ||(u1Res == RES_3D_480I60HZ_SBS_HALF)||\
                             (u1Res == RES_3D_480I30HZ_TB) ||(u1Res == RES_3D_480I30HZ_SBS_HALF)||\
                             (u1Res == RES_3D_576I50HZ_TB) ||(u1Res == RES_3D_576I50HZ_SBS_HALF)||\
                             (u1Res == RES_3D_576I25HZ_TB) ||(u1Res == RES_3D_576I25HZ_SBS_HALF)  )

#define fgIs4FsVideo(u1Res) ((u1Res == RES_480I_2880) || (u1Res == RES_576I_2880) ||\
								 (u1Res == RES_480P_2880) || (u1Res == RES_480P_2880))

#define fgIsSDInterlace(u1Res) ((u1Res == RES_480I) || (u1Res == RES_576I) ||\
									(u1Res == RES_480I_2880) || (u1Res == RES_576I_2880) ||\
									(u1Res == RES_2D_480I60HZ)||(u1Res == RES_2D_576I50HZ)||\
									(u1Res == RES_3D_480I60HZ_TB) ||(u1Res == RES_3D_480I60HZ_SBS_HALF)||\
									(u1Res == RES_3D_480I30HZ_TB) ||(u1Res == RES_3D_480I30HZ_SBS_HALF)||\
									(u1Res == RES_3D_576I50HZ_TB) ||(u1Res == RES_3D_576I50HZ_SBS_HALF)||\
									(u1Res == RES_3D_576I25HZ_TB) ||(u1Res == RES_3D_576I25HZ_SBS_HALF))

#define fgIsHDRes(u1Res) ((u1Res == RES_720P30HZ)||(u1Res == RES_720P25HZ)||(u1Res == RES_720P24HZ)||\
											   (u1Res == RES_720P23HZ)||(u1Res == RES_720P60HZ)||(u1Res == RES_720P50HZ)||\
											   (u1Res == RES_1080I60HZ)||(u1Res == RES_1080I50HZ)||(u1Res == RES_1080P60HZ)||\
											   (u1Res == RES_1080P50HZ)||(u1Res == RES_1080P30HZ)||(u1Res == RES_1080P25HZ)||\
											   (u1Res == RES_1080P24HZ)||(u1Res == RES_1080P23_976HZ)||(u1Res == RES_1080P29_97HZ) ||\
											   (u1Res == RES_3D_1080P60HZ) ||(u1Res == RES_3D_1080P50HZ) ||(u1Res == RES_3D_1080P30HZ) ||\
											   (u1Res == RES_3D_1080P29HZ) ||(u1Res == RES_3D_1080P25HZ) ||(u1Res == RES_3D_1080P24HZ) ||\
											   (u1Res == RES_3D_1080P23HZ) ||(u1Res == RES_3D_720P60HZ) ||(u1Res == RES_3D_720P50HZ) ||\
											   (u1Res == RES_3D_720P30HZ) ||(u1Res == RES_3D_720P25HZ) ||(u1Res == RES_3D_720P24HZ) ||\
											   (u1Res == RES_3D_720P23HZ) ||(u1Res == RES_3D_1080I60HZ) ||(u1Res == RES_3D_1080I50HZ) ||\
											   (u1Res == RES_3D_1080I30HZ) ||(u1Res == RES_3D_1080I25HZ)||\
											   (u1Res == RES_3D_1080P60HZ_TB) ||(u1Res == RES_3D_1080P50HZ_TB) ||(u1Res == RES_3D_1080P30HZ_TB) ||\
											   (u1Res == RES_3D_1080P29HZ_TB) ||(u1Res == RES_3D_1080P25HZ_TB) ||(u1Res == RES_3D_1080P24HZ_TB) ||\
											   (u1Res == RES_3D_1080P23HZ_TB) ||(u1Res == RES_3D_720P60HZ_TB) ||(u1Res == RES_3D_720P50HZ_TB) ||\
											   (u1Res == RES_3D_720P30HZ_TB) ||(u1Res == RES_3D_720P25HZ_TB) ||(u1Res == RES_3D_720P24HZ_TB) ||\
											   (u1Res == RES_3D_720P23HZ_TB) ||(u1Res == RES_3D_1080I60HZ_TB) ||(u1Res == RES_3D_1080I50HZ_TB) ||\
											   (u1Res == RES_3D_1080I30HZ_TB) ||(u1Res == RES_3D_1080I25HZ_TB)||\
											   (u1Res == RES_3D_1080P60HZ_SBS_HALF) ||(u1Res == RES_3D_1080P50HZ_SBS_HALF) ||(u1Res == RES_3D_1080P30HZ_SBS_HALF) ||\
											   (u1Res == RES_3D_1080P29HZ_SBS_HALF) ||(u1Res == RES_3D_1080P25HZ_SBS_HALF) ||(u1Res == RES_3D_1080P24HZ_SBS_HALF) ||\
											   (u1Res == RES_3D_1080P23HZ_SBS_HALF) ||(u1Res == RES_3D_720P60HZ_SBS_HALF) ||(u1Res == RES_3D_720P50HZ_SBS_HALF) ||\
											   (u1Res == RES_3D_720P30HZ_SBS_HALF) ||(u1Res == RES_3D_720P25HZ_SBS_HALF) ||(u1Res == RES_3D_720P24HZ_SBS_HALF) ||\
											   (u1Res == RES_3D_720P23HZ_SBS_HALF) ||(u1Res == RES_3D_1080I60HZ_SBS_HALF) ||(u1Res == RES_3D_1080I50HZ_SBS_HALF) ||\
											   (u1Res == RES_3D_1080I30HZ_SBS_HALF) ||(u1Res == RES_3D_1080I25HZ_SBS_HALF)||\
											   (u1Res == RES_2160P_24HZ) ||(u1Res == RES_2160P_25HZ) ||(u1Res == RES_2160P_30HZ)||(u1Res == RES_2161P_24HZ)||(u1Res == RES_2160P_23_976HZ)||(u1Res == RES_2160P_29_97HZ))


#define fgIs3DTABOutput(u1Res) ((u1Res == RES_3D_1080P60HZ_TB) || (u1Res == RES_3D_1080P50HZ_TB) || (u1Res == RES_3D_1080P30HZ_TB) ||(u1Res == RES_3D_1080P29HZ_TB)||\
		(u1Res == RES_3D_1080P25HZ_TB) || (u1Res == RES_3D_1080P24HZ_TB) || (u1Res == RES_3D_1080P23HZ_TB) ||\
		(u1Res == RES_3D_720P60HZ_TB) || (u1Res == RES_3D_720P50HZ_TB) || (u1Res == RES_3D_720P30HZ_TB) ||\
		(u1Res == RES_3D_720P25HZ_TB) || (u1Res == RES_3D_720P24HZ_TB) || (u1Res == RES_3D_720P23HZ_TB) ||\
		(u1Res == RES_3D_1080I60HZ_TB) || (u1Res == RES_3D_1080I50HZ_TB) ||\
		(u1Res == RES_3D_1080I30HZ_TB) ||(u1Res == RES_3D_1080I25HZ_TB) ||\
		(u1Res == RES_3D_576P50HZ_TB) || (u1Res == RES_3D_576I50HZ_TB) || (u1Res == RES_3D_576I25HZ_TB) ||\
		(u1Res == RES_3D_480P60HZ_TB) || (u1Res == RES_3D_480I60HZ_TB) || (u1Res == RES_3D_480I30HZ_TB))

#define fgIs3DSBSOutput(u1Res) ((u1Res == RES_3D_1080P60HZ_SBS_HALF) || (u1Res == RES_3D_1080P50HZ_SBS_HALF) || (u1Res == RES_3D_1080P30HZ_SBS_HALF) ||(u1Res == RES_3D_1080P29HZ_SBS_HALF)||\
		(u1Res == RES_3D_1080P25HZ_SBS_HALF) || (u1Res == RES_3D_1080P24HZ_SBS_HALF) || (u1Res == RES_3D_1080P23HZ_SBS_HALF) ||\
		(u1Res == RES_3D_720P60HZ_SBS_HALF) || (u1Res == RES_3D_720P50HZ_SBS_HALF) || (u1Res == RES_3D_720P30HZ_SBS_HALF) ||\
		(u1Res == RES_3D_720P25HZ_SBS_HALF) || (u1Res == RES_3D_720P24HZ_SBS_HALF) || (u1Res == RES_3D_720P23HZ_SBS_HALF) ||\
		(u1Res == RES_3D_1080I60HZ_SBS_HALF) || (u1Res == RES_3D_1080I50HZ_SBS_HALF) ||\
		(u1Res == RES_3D_1080I30HZ_SBS_HALF) ||(u1Res == RES_3D_1080I25HZ_SBS_HALF) ||\
		(u1Res == RES_3D_576P50HZ_SBS_HALF) || (u1Res == RES_3D_576I50HZ_SBS_HALF) || (u1Res == RES_3D_576I25HZ_SBS_HALF) ||\
		(u1Res == RES_3D_480P60HZ_SBS_HALF) || (u1Res == RES_3D_480I60HZ_SBS_HALF) || (u1Res == RES_3D_480I30HZ_SBS_HALF))

#define fgIs3DOutput(u1Res) ((u1Res == RES_3D_1080P60HZ) || (u1Res == RES_3D_1080P50HZ) || (u1Res == RES_3D_1080P30HZ) ||(u1Res == RES_3D_1080P29HZ)||\
		(u1Res == RES_3D_1080P25HZ) || (u1Res == RES_3D_1080P24HZ) || (u1Res == RES_3D_1080P23HZ) ||\
		(u1Res == RES_3D_720P60HZ) || (u1Res == RES_3D_720P50HZ) || (u1Res == RES_3D_720P30HZ) ||\
		(u1Res == RES_3D_720P25HZ) || (u1Res == RES_3D_720P24HZ) || (u1Res == RES_3D_720P23HZ) ||\
		(u1Res == RES_3D_1080I60HZ) || (u1Res == RES_3D_1080I50HZ) ||\
		(u1Res == RES_3D_1080I30HZ) ||(u1Res == RES_3D_1080I25HZ) ||\
		(u1Res == RES_3D_576P50HZ) || (u1Res == RES_3D_576I50HZ) || (u1Res == RES_3D_576I25HZ) ||\
		(u1Res == RES_3D_480P60HZ) || (u1Res == RES_3D_480I60HZ) || (u1Res == RES_3D_480I30HZ) ||\
		(u1Res == RES_3D_1080P60HZ_TB) || (u1Res == RES_3D_1080P50HZ_TB) || (u1Res == RES_3D_1080P30HZ_TB) ||(u1Res == RES_3D_1080P29HZ_TB)||\
		(u1Res == RES_3D_1080P25HZ_TB) || (u1Res == RES_3D_1080P24HZ_TB) || (u1Res == RES_3D_1080P23HZ_TB) ||\
		(u1Res == RES_3D_720P60HZ_TB) || (u1Res == RES_3D_720P50HZ_TB) || (u1Res == RES_3D_720P30HZ_TB) ||\
		(u1Res == RES_3D_720P25HZ_TB) || (u1Res == RES_3D_720P24HZ_TB) || (u1Res == RES_3D_720P23HZ_TB) ||\
		(u1Res == RES_3D_1080I60HZ_TB) || (u1Res == RES_3D_1080I50HZ_TB) ||\
		(u1Res == RES_3D_1080I30HZ_TB) ||(u1Res == RES_3D_1080I25HZ_TB) ||\
		(u1Res == RES_3D_576P50HZ_TB) || (u1Res == RES_3D_576I50HZ_TB) || (u1Res == RES_3D_576I25HZ_TB) ||\
		(u1Res == RES_3D_480P60HZ_TB) || (u1Res == RES_3D_480I60HZ_TB) || (u1Res == RES_3D_480I30HZ_TB) ||\
		(u1Res == RES_3D_1080P60HZ_SBS_HALF) || (u1Res == RES_3D_1080P50HZ_SBS_HALF) || (u1Res == RES_3D_1080P30HZ_SBS_HALF) ||(u1Res == RES_3D_1080P29HZ_SBS_HALF)||\
		(u1Res == RES_3D_1080P25HZ_SBS_HALF) || (u1Res == RES_3D_1080P24HZ_SBS_HALF) || (u1Res == RES_3D_1080P23HZ_SBS_HALF) ||\
		(u1Res == RES_3D_720P60HZ_SBS_HALF) || (u1Res == RES_3D_720P50HZ_SBS_HALF) || (u1Res == RES_3D_720P30HZ_SBS_HALF) ||\
		(u1Res == RES_3D_720P25HZ_SBS_HALF) || (u1Res == RES_3D_720P24HZ_SBS_HALF) || (u1Res == RES_3D_720P23HZ_SBS_HALF) ||\
		(u1Res == RES_3D_1080I60HZ_SBS_HALF) || (u1Res == RES_3D_1080I50HZ_SBS_HALF)||\
		(u1Res == RES_3D_1080I30HZ_SBS_HALF) ||(u1Res == RES_3D_1080I25HZ_SBS_HALF) ||\
		(u1Res == RES_3D_576P50HZ_SBS_HALF) || (u1Res == RES_3D_576I50HZ_SBS_HALF) || (u1Res == RES_3D_576I25HZ_SBS_HALF) ||\
		(u1Res == RES_3D_480P60HZ_SBS_HALF) || (u1Res == RES_3D_480I60HZ_SBS_HALF) || (u1Res == RES_3D_480I30HZ_SBS_HALF))

#define fgIs3DFPOutput(u1Res) (fgIs3DOutput(u1Res)&&(!fgIs3DSBSOutput(u1Res))&&(!fgIs3DTABOutput(u1Res)))

#define  fgVideoIsNtsc(ucFmt)  ((ucFmt == RES_480I) || (ucFmt == RES_480P) ||\
	                                            (ucFmt == RES_480P_1440)|| (ucFmt == RES_480P_2880)||\
	                                            (ucFmt == RES_720P60HZ)|| (ucFmt == RES_1080I60HZ)||\
	                                            (ucFmt == RES_720P30HZ) || (ucFmt == RES_720P23HZ) ||\
	                                            (ucFmt == RES_1080P60HZ)|| /*(ucFmt == RES_1080P30HZ)||*/\
	                                            (ucFmt == RES_480I_2880) || (ucFmt == RES_1080P23_976HZ)||\
	                                            (ucFmt == RES_1080P29_97HZ)|| (ucFmt == RES_3D_1080P23HZ)||\
	                                            (ucFmt == RES_3D_1080P29HZ)|| (ucFmt == RES_3D_720P60HZ)||\
	                                            (ucFmt == RES_3D_1080P60HZ)||(ucFmt == RES_3D_1080P29HZ)||\
	                                            (ucFmt == RES_3D_720P30HZ)||(ucFmt == RES_3D_720P23HZ)||\
	                                            (ucFmt == RES_3D_480P60HZ)||\
	                                            (ucFmt == RES_3D_1080I60HZ)|| (ucFmt == RES_3D_1080I30HZ)||\
	                                            (ucFmt == RES_3D_480I30HZ)|| (ucFmt == RES_3D_480I60HZ)||\
	                                            (ucFmt == RES_2D_480I60HZ) || (ucFmt == RES_PANEL_AUO_B089AW01)||\
	                                            (ucFmt == RES_2D_640x480HZ) ||\
	                                            (ucFmt == RES_3D_720P60HZ_TB) || (ucFmt == RES_3D_1080I60HZ_SBS_HALF)||\
	                                            (ucFmt == RES_3D_720P30HZ_TB)||(ucFmt == RES_3D_720P23HZ_TB)||\
	                                            (ucFmt == RES_3D_1080P60HZ_TB)||(ucFmt == RES_3D_1080P29HZ_TB)||\
	                                            (ucFmt == RES_3D_1080I60HZ_TB)||(ucFmt == RES_3D_1080I30HZ_TB)||\
	                                            (ucFmt == RES_3D_720P60HZ_SBS_HALF)||(ucFmt == RES_3D_720P30HZ_SBS_HALF)||\
	                                            (ucFmt == RES_3D_1080P23HZ_TB)||(ucFmt == RES_3D_720P23HZ_SBS_HALF)||\
	                                            (ucFmt == RES_3D_1080I30HZ_SBS_HALF)||(ucFmt == RES_3D_1080P23HZ_SBS_HALF)||\
	                                            (ucFmt == RES_3D_1080P60HZ_SBS_HALF)||(ucFmt == RES_3D_1080P29HZ_SBS_HALF)||\
	                                            (ucFmt == RES_3D_1080P23HZ_TB) ||\
	                                            (ucFmt == RES_2160P_23_976HZ)|| (ucFmt == RES_2160P_29_97HZ))


typedef struct _HDMIRX_SINK_AV_CAP_T {
	unsigned int ui4_sink_cea_ntsc_resolution;//use HDMI_SINK_VIDEO_RES_T
	unsigned int ui4_sink_cea_pal_resolution;//use HDMI_SINK_VIDEO_RES_T
	unsigned int ui4_sink_org_cea_ntsc_resolution;//use HDMI_SINK_VIDEO_RES_T
	unsigned int ui4_sink_org_cea_pal_resolution;//use HDMI_SINK_VIDEO_RES_T
	unsigned int ui4_sink_dtd_ntsc_resolution;//use HDMI_SINK_VIDEO_RES_T
	unsigned int ui4_sink_dtd_pal_resolution;//use HDMI_SINK_VIDEO_RES_T
	unsigned int ui4_sink_1st_dtd_ntsc_resolution;//use HDMI_SINK_VIDEO_RES_T
	unsigned int ui4_sink_1st_dtd_pal_resolution;//use HDMI_SINK_VIDEO_RES_T
	unsigned int ui4_sink_native_ntsc_resolution;//use HDMI_SINK_VIDEO_RES_T
	unsigned int ui4_sink_native_pal_resolution;//use HDMI_SINK_VIDEO_RES_T
	unsigned short ui2_sink_colorimetry;//use HDMI_SINK_VIDEO_COLORIMETRY_T
	unsigned short ui2_sink_vcdb_data; //use HDMI_SINK_VCDB_T
	unsigned short ui2_sink_aud_dec;//HDMI_SINK_AUDIO_DECODER_T
	unsigned char ui1_sink_dsd_ch_num;
	unsigned int ui4_sink_dts_fs;
	unsigned char
	ui1_sink_dts_ch_sampling[7];//n: channel number index, value: each bit means sampling rate for this channel number (SINK_AUDIO_32k..)
	unsigned char
	ui1_sink_pcm_ch_sampling[7];//n: channel number index, value: each bit means sampling rate for this channel number (SINK_AUDIO_32k..)
	unsigned char
	ui1_sink_pcm_bit_size[7];////n: channel number index, value: each bit means bit size for this channel number
	unsigned char
	ui1_sink_dst_ch_sampling[7];//n: channel number index, value: each bit means sampling rate for this channel number (SINK_AUDIO_32k..)
	unsigned char
	ui1_sink_dsd_ch_sampling[7];//n: channel number index, value: each bit means sampling rate for this channel number (SINK_AUDIO_32k..)
	unsigned char ui1_sink_org_pcm_ch_sampling[7];//original PCM sampling data
	unsigned char ui1_sink_org_pcm_bit_size[7];//original PCM bit Size

	unsigned short ui1_sink_max_tmds_clock;
	unsigned char ui1_sink_spk_allocation;
	unsigned char ui1_sink_content_cnc;
	unsigned char ui1_sink_p_latency_present;
	unsigned char ui1_sink_i_latency_present;
	unsigned char ui1_sink_p_audio_latency;
	unsigned char ui1_sink_p_video_latency;
	unsigned char ui1_sink_i_audio_latency;
	unsigned char ui1_sink_i_video_latency;
	unsigned char e_sink_rgb_color_bit;
	unsigned char e_sink_ycbcr_color_bit;
	unsigned char u1_sink_support_ai; //kenny add 2010/4/25
	unsigned char u1_sink_max_tmds; //kenny add 2010/4/25
	unsigned short ui2_edid_chksum_and_audio_sup;//HDMI_EDID_CHKSUM_AND_AUDIO_SUP_T
	unsigned short ui2_sink_cec_address;
	bool   b_sink_edid_ready;
	bool   b_sink_support_hdmi_mode;
	unsigned char ui1_ExtEdid_Revision;
	unsigned char ui1_Edid_Version;
	unsigned char ui1_Edid_Revision;
	unsigned char ui1_sink_support_ai;
	unsigned char ui1_Display_Horizontal_Size;
	unsigned char ui1_Display_Vertical_Size;
	bool   b_sink_hdmi_video_present;
	unsigned char  ui1_CNC;
	bool   b_sink_3D_present;
	unsigned int ui4_sink_cea_3D_resolution;
	unsigned short ui2_sink_3D_structure;
	unsigned int ui4_sink_cea_FP_SUP_3D_resolution;
	unsigned int ui4_sink_cea_TOB_SUP_3D_resolution;
	unsigned int ui4_sink_cea_SBS_SUP_3D_resolution;
	unsigned char _bMonitorName[13];
	unsigned short ui2_sink_ID_manufacturer_name;//(08H~09H)
	unsigned short ui2_sink_ID_product_code;           //(0aH~0bH)
	unsigned int ui4_sink_ID_serial_number;         //(0cH~0fH)
	unsigned char  ui1_sink_week_of_manufacture;   //(10H)
	unsigned char  ui1_sink_year_of_manufacture;   //(11H)  base on year 1990
	unsigned int ui4_sink_id_serial_number;
}  HDMIRX_SINK_AV_CAP_T;

typedef enum {
	HDMI_RX_STATE_PWER_5V_STATUS,
	HDMI_RX_STATE_EDID_STATUS,
	HDMI_RX_INFOFRAME_NOTIFY,
	HDMI_RX_TIMING_STATUS_LOCK,
	HDMI_RX_TIMING_STATUS_UNLOCK,
	HDMI_RX_RESOLUTION_CHANGEING,
	HDMI_RX_RESOLUTION_CHANGE_DONE,
	HDMI_RX_ASR_INFO_CHANGE,
	HDMI_RX_CSP_INFO_CHANGE,  //COLORSPACE
	HDMI_RX_TYPE_INFO_CHANGE, //JPEG CINEMA
	VIDEO_IN_QUEUE_BUFFER_FULL_NOTIFY, //QUEUE FULL
	VIDEO_IN_QUEUE_BUFFER_NULL_NOTIFY,  //QUEUE NULL
	VIDEO_IN_SEND_BUFFER_TO_HW_NOTIFY,
	HDMI_RX_AUD_STATUS_LOCK,
	HDMI_RX_AUD_STATUS_UNLOCK
} VSW_STATE;

typedef struct {
	void (*set_reset_pin)(unsigned int value);
	int (*set_gpio_out)(unsigned int gpio, unsigned int value);
	void (*udelay)(unsigned int us);
	void (*mdelay)(unsigned int ms);
	void (*wait_transfer_done)(void);
	void (*state_callback)(VSW_STATE state);
} VSW_UTIL_FUNCS;


typedef enum
{
	HDMI_SOURCE_MODE = 0,
	HDMI_REPEATER_VIDEO_BYPASS_MODE = 1,
	HDMI_REPEATER_VIDEO_DRAM_MODE = 2,
} HDMI_RX_STATUS_MODE;

typedef enum
{	HDMI_REPEATER_AUDIO_BYPASS_MODE = 0,
	HDMI_REPEATER_AUDIO_DRAM_MODE = 1,
} HDMI_RX_AUD_MODE;

typedef struct
{
	HDMI_RX_AUD_MODE rxaudmode;
}HDMI_IN_AUD_PARAM_T;

typedef enum {
	HDMI_RX_SERVICE_CMD = 0,
	HDMI_DISABLE_HDMI_RX_TASK_CMD,
	HDMI_RX_EG_WAKEUP_THREAD,
} HDMIRX_TASK_COMMAND_TYPE_T;

typedef enum {
	RX_DETECT_STATE = 0,
	RX_CHANGE_EDID_STATE = 1,
	RX_IDLE_STATE = 20,

} HDMI_RX_STATE_TYPE;

typedef struct {
	unsigned int u4YAddr; 	   ///< the address of Y component of the frame buffer
	unsigned int u4CAddr; 	   ///< the address of CbCr component of the frame buffer
	unsigned int u4YBufferSize;
	unsigned int u4CBufferSize;
} VIDEO_IN_ADDR, *PVIDEO_IN_BUFFER_INFO;

typedef struct {
	unsigned int u4BufCount;
} VIDEO_IN_REQBUF;

typedef struct {
	unsigned int u4BufIndex;
	unsigned int wch1mva;
	unsigned int wch1m4uva[4];
	VIDEO_IN_ADDR vAddrInfo;
	VIDEO_IN_ADDR vAddrInfo_va;
} VIDEO_IN_BUFFER_INFO;


typedef enum {
	HDMI_RX_SET_VIDEO_CHG_MODE = 0,
	HDMI_RX_SET_TX_TMDS,
	HDMI_RX_SET_TX_GCP,
	HDMI_RX_SET_TX_SEND_ACR,
} HDMI_RX_SET_AVD_COND_T;

typedef struct _HDMI_RX_EDID_T {
	unsigned char u1HdmiRxEDID[512];
} HDMI_RX_EDID_T;

typedef struct {
	unsigned char u1Hdcprxkey[384];
} HDMIRX_HDCP_KEY;

typedef enum {
	NON_USED = 0,
	YCMIX_8BIT,
	YCMIX_10BIT,
	YCMIX_12BIT,
	YC422_16BIT,
	YC422_20BIT,
	YC422_24BIT,
	YC444_24BIT,
	YC444_30BIT,
	YC444_36BIT,
} VIDEO_DGI_IN_FORMAT_T;

typedef enum {
	FMT_INVALID = 0,
	FMT_601_MODE,
	FMT_656_MODE,
} VIDEO_DGI_IN_MODE_T;

typedef enum {
	SRC_ASP_UNKNOW = 0,
	SRC_ASP_1_1,
	SRC_ASP_4_3_FULL,
	SRC_ASP_14_9_LB,
	SRC_ASP_14_9_LB_T,
	SRC_ASP_16_9_LB,
	SRC_ASP_16_9_LB_T,
	SRC_ASP_16_9_LB_G,
	SRC_ASP_14_9_FULL,
	SRC_ASP_16_9_FULL,
	SRC_ASP_221_1,
	SRC_ASP_16_9_PS,
	SRC_ASP_UNDEFINED,
	SRC_ASP_CUSTOMIZED,
	SRC_ASP_MAX
} SOURCE_ASPECT_RATIO_T;

typedef enum {
	HDMIRX_RGB = 0,
	HDMIRX_YCBCR_444,
	HDMIRX_YCBCR_422,
	HDMIRX_XV_YCC,
} HDMIRX_SOURCE_COLOR_SPACE_T;

typedef enum {
	HDMIRX_NO_DEEP_COLOR=0,
	HDMIRX_DEEP_COLOR_10_BIT,
	HDMIRX_DEEP_COLOR_12_BIT,
	HDMIRX_DEEP_COLOR_16_BIT 
} HDMIRX_SOURCE_DEEP_COLOR_T;

#define VIN_DEVICE_MAX (5)
typedef enum VIDEO_IN_SRC_ID_E {
	VIN_DEVICE_UNKNOW = 0,
	VIN_EXTERNAL_TVD  = 1,
	VIN_HDMI_1        = 2,
	VIN_HDMI_2        = 3,
	VIN_SRC_DGI,
} VIDEO_IN_SRC_ID_E;

enum HDMI_SWITCH_NO {
	HDMI_SWITCH_INIT = 0,
	HDMI_SWITCH_1,
	HDMI_SWITCH_2,
	HDMI_SWITCH_3,
	HDMI_SWITCH_4,
	HDMI_SWITCH_5,
	HDMI_SWITCH_6
};

typedef struct {
	HDMI_RX_STATUS_MODE rxmode;
	enum HDMI_SWITCH_NO rxswitchport;
}	HDMI_IN_PARAM_T;


typedef enum {
	VSW_COMP_NFY_ERROR = 0,
	VSW_COMP_NFY_UNLOCK = 1,
	VSW_COMP_NFY_RESOLUTION_CHGING,
	VSW_COMP_NFY_RESOLUTION_CHG_DONE,
	VSW_COMP_NFY_ASPECT_CHG,
	VSW_COMP_NFY_COLOR_SPACE_CHG,
	VSW_COMP_NFY_JPEG_CHG,
	VSW_COMP_NFY_CINEMA_CHG
} VSW_GET_INFO_COND_T;

typedef enum _VIN_DRAM_ADDRESS_MODE_T {
	VIN_BLOCK = 0,
	VIN_LINEAR = 1

} VIN_DRAM_ADDRESS_MODE_T;

typedef enum _VIN_DRAM_SWAP_MODE_T {
	VIN_SWAP_MODE_0 = 0,
	VIN_SWAP_MODE_1 = 1,
	VIN_SWAP_MODE_2 = 2,
	VIN_SWAP_MODE_3 = 3

} VIN_DRAM_SWAP_MODE_T;

typedef enum _VIN_DRAM_FORMAT_T {
	VIN_420 = 0,
	VIN_422 = 1

} VIN_DRAM_FORMAT_T;

typedef  enum {
	VIN_PB_TYPE_INVALID = 0,
	VIN_PB_TYPE_START_CMD,
	VIN_PB_TYPE_STOP_CMD,
} VIN_PB_TYPE_T;

typedef struct _VDOIN_CONFIG_WRITE_DRAM_TYPE_T {
	VIN_DRAM_ADDRESS_MODE_T eVdoInAddrMode;
	VIN_DRAM_SWAP_MODE_T eVdoInSwapMode;
	VIN_DRAM_FORMAT_T eVdoInDramFmt;

} VDOIN_CONFIG_WRITE_DRAM_TYPE_T;


typedef struct _INPUT_DEVICE_INFO_T {
	bool                   fgIsTimingOk;
	VIDEO_IN_SRC_ID_E      eDeviceId;
	HDMI_RESOLUTION_MODE_T  eInputRes;     //use EDID_VIDEO_RES_T, there are many resolution
	VIDEO_DGI_IN_FORMAT_T   ePinType;
	VIDEO_DGI_IN_MODE_T     eInputMode;
	SOURCE_ASPECT_RATIO_T  eAspectRatio;
	VDOIN_CONFIG_WRITE_DRAM_TYPE_T rVdoInWDramType;
	SOURCE_ASPECT_RATIO_T		   eVdoInAR;
	HDMIRX_SOURCE_COLOR_SPACE_T           esrccsp;
	HDMIRX_SOURCE_DEEP_COLOR_T            esrcdeepcolor;
	bool                   fgIsJpeg;
	bool                   fgIsCinema;
	bool                   fgVgaIsCeaType;
	bool                   fgNTSC60;//different from 59.94 or 23.976
} INPUT_DEVICE_INFO_T;

typedef struct DGI_VIDEO_IN_INFO_T {
	VIDEO_DGI_IN_FORMAT_T   ePinType;
	VIDEO_DGI_IN_MODE_T     eInputMode;
} DGI_VIDEO_IN_INFO_T;


typedef struct _VDOIN_DEV_CTRL_INFO_T {
	INPUT_DEVICE_INFO_T   rVdoInDevInfo;
	//BOOL   fgIsLock;
	bool   fgIsByPass;
}   VDOIN_DEV_CTRL_INFO_T;

typedef enum {
	VDOIN_3D_NO = 0,
	VDOIN_3D_FP,
	VDOIN_3D_SBS,
	VDOIN_3D_TAB,
} VDOIN_3D_TYPE_T;

typedef struct _VID_VDOIN_RES_INFO_T {
	unsigned int ui4_h_total;
	unsigned int ui4_v_total;
	unsigned int ui4_h_active;
	unsigned int ui4_v_activel;
	unsigned int u4_framerate;
	bool     b_is_3d;
	VDOIN_3D_TYPE_T ui2_3d_type;
	bool     b_is_progressive;
} VID_VDOIN_RES_INFO_T;

typedef enum {
	VID_VIDIN_SRC_ASPECT_RATIO_UNKNOWN = 0,
	VID_VIDIN_SRC_ASPECT_RATIO_1_1,                 //1:1
	VID_VIDIN_SRC_ASPECT_RATIO_4_3_P_S,         //4:3 PAN SCAN
	VID_VIDIN_SRC_ASPECT_RATIO_4_3_L_B,         //4:3 LETTER BOX
	VID_VIDIN_SRC_ASPECT_RATIO_16_9_P_S,        //16:9 PAN SCAN
	VID_VIDIN_SRC_ASPECT_RATIO_16_9_L_B,        //16:9 LETTER BOX
	VID_VIDIN_SRC_ASPECT_RATIO_16_9_FULL,       //16:9 FULL
	VID_VIDIN_SRC_ASPECT_RATIO_2_21_1,              //2.21:1
	VID_VIDIN_SRC_ASPECT_RATIO_UNDEFINED,       //FOR IFRAME DECODE
	VID_VIDIN_SRC_ASPECT_RATIO_CUSTOMIZED       //FOR ui2_sample_width & ui2_sample_heigh
} VID_VDOIN_SRC_ASPECT_RATIO_T;

typedef struct _VID_VDOIN_DATA_INFO_T {
	bool b_is_jpeg;
	bool b_is_cinema;
} VID_VDOIN_DATA_INFO_T;

typedef struct _VID_VDOIN_SIGNAL_INFO_T {
	VIDEO_DGI_IN_MODE_T   eVdoinMode;
	VIDEO_DGI_IN_FORMAT_T eVdoinDataFmt;
} VID_VDOIN_SIGNAL_INFO_T;

typedef struct _VID_VDOIN_ALL_INFO_T {
	unsigned int eVddoInInfoType;
	VID_VDOIN_RES_INFO_T              t_resolution_info;
	VID_VDOIN_DATA_INFO_T             t_data_info;
	VID_VDOIN_SRC_ASPECT_RATIO_T      e_aspect_ratio;
	VID_VDOIN_SIGNAL_INFO_T           t_signal_info;
	HDMIRX_SOURCE_COLOR_SPACE_T           	  e_src_csp;
	HDMIRX_SOURCE_DEEP_COLOR_T               e_src_deepcolor;
} VID_VDOIN_ALL_INFO_T;

typedef struct _VIN_InPUT_QUEUE {
	int BufReadIndex;
	int BufWriteIndex;
	int FBCount;
	VIDEO_IN_BUFFER_INFO rFBAddrInfo[BUF_COUNT];
} _VIN_InPUT_QUEUE;

typedef struct _VIN_OUTPUT_QUEUE {
	int RdIdx;                            ///< Read index
	int WrIdx;
	int FBCount;                           ///< Write index
	VIDEO_IN_BUFFER_INFO rFBAddrInfo[BUF_COUNT];
} _VIN_OUTPUT_QUEUE;


typedef struct _VIN_CTRL_INFO_T {
	INPUT_DEVICE_INFO_T  rVinCfg;
	_VIN_InPUT_QUEUE rInputQueue;
	_VIN_OUTPUT_QUEUE rOutputQueue;
	VIN_PB_TYPE_T pb_Type;
} VIN_CTRL_INFO_T;


typedef struct {
	bool pcmchinfo;
	bool dsdaudio;
	bool hbraudio;
	unsigned char   hdmirxaudfs;
	unsigned char	chsts[5];
} HDMIRX_AUDIO_INFO;

#define SINK_YCBCR_444 (1<<0)
#define SINK_YCBCR_422 (1<<1)
#define SINK_XV_YCC709 (1<<2)
#define SINK_XV_YCC601 (1<<3)
#define SINK_METADATA0 (1<<4)
#define SINK_METADATA1 (1<<5)
#define SINK_METADATA2 (1<<6)
#define SINK_RGB       (1<<7)

#define ACP_TYPE_GENERAL_AUDIO  0x00
#define ACP_TYPE_IEC60958                0x01
#define ACP_TYPE_DVD_AUDIO           0x02
#define ACP_TYPE_SACD                       0x03
#define ACP_LOST_DISABLE                  0xFF

#define   HDMI_SINK_AUDIO_DEC_LPCM        (1<<0)
#define   HDMI_SINK_AUDIO_DEC_AC3         (1<<1)
#define   HDMI_SINK_AUDIO_DEC_MPEG1       (1<<2)
#define   HDMI_SINK_AUDIO_DEC_MP3         (1<<3)
#define   HDMI_SINK_AUDIO_DEC_MPEG2       (1<<4)
#define   HDMI_SINK_AUDIO_DEC_AAC         (1<<5)
#define   HDMI_SINK_AUDIO_DEC_DTS         (1<<6)
#define   HDMI_SINK_AUDIO_DEC_ATRAC       (1<<7)
#define   HDMI_SINK_AUDIO_DEC_DSD         (1<<8)
#define   HDMI_SINK_AUDIO_DEC_DOLBY_PLUS   (1<<9)
#define   HDMI_SINK_AUDIO_DEC_DTS_HD      (1<<10)
#define   HDMI_SINK_AUDIO_DEC_MAT_MLP     (1<<11)
#define   HDMI_SINK_AUDIO_DEC_DST         (1<<12)
#define   HDMI_SINK_AUDIO_DEC_WMA         (1<<13)

#define HDMIRX_IOW(num, dtype)     _IOW('H', num, dtype)
#define HDMIRX_IOR(num, dtype)     _IOR('H', num, dtype)
#define HDMIRX_IOWR(num, dtype)    _IOWR('H', num, dtype)
#define HDMIRX_IO(num)             _IO('H', num)

#define MTK_SET_VIDEO_IN_SRC_TYPE                HDMIRX_IOWR(1, VIDEO_IN_SRC_ID_E)
#define MTK_GET_HDMI_RX_EDID                     HDMIRX_IOWR(2, HDMI_RX_EDID_T)
#define MTK_GET_HDMI_IN_PWR_STATUS               HDMIRX_IOWR(3, int)
#define MTK_SET_HDMI_RX_MODE_STATUS              HDMIRX_IOWR(4, HDMI_IN_PARAM_T)
#define MTK_SET_HDMI_RX_EDID_VALUE               HDMIRX_IOWR(5, HDMI_RX_EDID_T)
#define MTK_GET_VIDEO_IN_CONFIG_INFO             HDMIRX_IOWR(6, VID_VDOIN_ALL_INFO_T)
#define MTK_SET_VIDEO_IN_START_CMD               HDMIRX_IOWR(7, int)
#define MTK_SET_VIDEO_IN_STOP_CMD                HDMIRX_IOWR(8, int)
#define MTK_SET_VIDEO_IN_REQBUFS				 HDMIRX_IOWR(9, VIDEO_IN_REQBUF)
#define MTK_SET_VIDEO_IN_QBUF					 HDMIRX_IOWR(10, unsigned int)
#define MTK_SET_VIDEO_IN_DQBUF					 HDMIRX_IOWR(11, VIDEO_IN_BUFFER_INFO)
#define MTK_SET_VIDEO_IN_INITBUF				 HDMIRX_IOWR(12, VIDEO_IN_BUFFER_INFO)
#define MTK_SET_HDMIRX_POWER_ENABLE              HDMIRX_IOWR(13, int)
#define MTK_SET_HDMIRX_POWER_DISABLE             HDMIRX_IOWR(14, int)
#define MTK_HDMI_RX_HDCP_KEY                     HDMIRX_IOWR(15, HDMIRX_HDCP_KEY)
#define MTK_VIDEO_IN_STREAMON                    HDMIRX_IOWR(16, int)
#define MTK_VIDEO_IN_STREAMOFF                   HDMIRX_IOWR(17, int)
#define MTK_SET_HDMI_RX_AUD_MODE                 HDMIRX_IOWR(18, HDMI_IN_AUD_PARAM_T)
#define MTK_GET_HDMI_RX_AUD_INFO                 HDMIRX_IOWR(19, HDMIRX_AUDIO_INFO)
#define MTK_ENABLE_HDMI_RX_AUD_TASK              HDMIRX_IOWR(20, bool)



#endif
