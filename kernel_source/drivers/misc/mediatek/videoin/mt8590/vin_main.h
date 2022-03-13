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
#ifndef _VIN_MAIN_H_
#define _VIN_MAIN_H_

#include "typedef.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"
#include "vin_drv_if.h"
#include "hal_io.h"


#define TVS_NTSC       1
#define TVS_PAL         2
// frame_rate_code in Table 6-4 of 13818-2
#define FRC_23_976   1
#define FRC_24           2
#define FRC_25           3
#define FRC_29_97     4
#define FRC_30           5
#define FRC_50           6
#define FRC_59_94     7
#define FRC_60           8

#define HANDLE_T UINT32
#define VIN_INVALID_ADDR		0xFFFFFFFF


typedef UINT32 EV_GRP_EVENT_T;

typedef enum _VIN_EVENT_T
{
    VIN_EV_NONE                = ((EV_GRP_EVENT_T) 0),
    VIN_EV_STATE_M_START       = ((EV_GRP_EVENT_T) (1) << 0),
    VIN_EV_GET_FIRST_FB        = ((EV_GRP_EVENT_T) (1) << 1),
    VIN_EV_NOR_PROC            = ((EV_GRP_EVENT_T) (1) << 2),
    VIN_EV_WAIT_FB_IN          = ((EV_GRP_EVENT_T) (1) << 3),
    VIN_EVENT_CMD              = ((EV_GRP_EVENT_T) (1) << 4),
    VDEC_EVENT_MAX             = 0xFF,
    
} VIN_EVENT_T;

/* / Supported frame buffer type in driver layer */
typedef enum {
	FBT_420_RS = 0,	/* /< YCbCr 420 raster scan */
	FBT_420_BK,	/* /< YCbCr 420 block */
	FBT_422_RS,	/* /< YCbCr 422 raster scan */
	FBT_422_BK,	/* /< YCbCr 422 block */
	FBT_420_BK_YCBIND,	/* /< YCbCr 420 block, Y C memory are bound, for H.264 request */
	FBT_420_BK_YONLY,	/* /< YCbCr 420 block, Y memory only, no CbCr, for H.264 request */
	FBT_WORKSPACE,	/* /< One continue memory, like JPEG working space */
	FBT_PBBUF,	/* /< One continue memory, overlay with one HD main buffer */
	FBT_BGIMG	/* / < One continue memory, overlay with whole sub buffer */
} DRV_FBTYPE;

typedef struct _VIN_FB_INFO_T
{
    UINT32                u4Duration;
    UINT16                u2FrameRate;
    UINT8                 u1TvSystem;
    DRV_FBTYPE            eFBufType;
    SOURCE_ASPECT_RATIO_T eAspectRatio;
    BOOL                  fgIs3DVideo;
    UINT32                u43DFlag;
    
} VIN_FB_INFO_T;

INT32 i4Vin_Init(void);
INT32 i4VinUnInit(void);
BOOL fgIsVinInitDone(void);
void vRstVariable(void);
BOOL fgVinSetCfgInfo(INPUT_DEVICE_INFO_T rInfo);
void vVinSetCmd(VIN_PB_TYPE_T pbCmdType);
void vVinProcess(VIN_PB_TYPE_T pbCmdType);
void video_in_internal_init(void);
void video_in_internal_uninit(void);

#endif

