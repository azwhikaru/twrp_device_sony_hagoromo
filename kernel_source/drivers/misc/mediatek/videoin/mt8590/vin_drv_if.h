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
#ifndef __VIN_DRV_IF_H__
#define __VIN_DRV_IF_H__

#include "hdmirx.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"

typedef struct {
	UINT8 readIndex;
	UINT8 writeIndex;
	UINT8 usedCount;
	UINT8 totalCount;
	UINT8 maxCount;
	VIDEO_IN_ADDR rFBAddrInfo[BUF_COUNT];
} _VIN_QUEUE;

extern _VIN_QUEUE rQueue;
extern _VIN_QUEUE rQueue_backup;


#define printQueueInfo() 	VIDEO_IN_LOG("rQueue Info: readIdx(%d), writeIdx(%d), usedCount(%d), totalCount(%d), maxCount(%d)\n", \
									rQueue.readIndex, rQueue.writeIndex, rQueue.usedCount, rQueue.totalCount, rQueue.maxCount)
#define printAddrInfo(idx) VIDEO_IN_LOG("AddrInfo[%d]: YAddr(0x%x), CAddr(0x%x), YSize(0x%x), CSize(0x%x)\n", (idx), rQueue.rFBAddrInfo[(idx)].u4YAddr,rQueue.rFBAddrInfo[(idx)].u4CAddr, \
									rQueue.rFBAddrInfo[(idx)].u4YBufferSize, rQueue.rFBAddrInfo[(idx)].u4CBufferSize);
#define ENTER_FUNC() VIDEO_IN_LOG("VinBufQueue, enter into %s()\n", __FUNCTION__);
#define EXIT_FUNC() VIDEO_IN_LOG("VinBufQueue, exit from %s()\n", __FUNCTION__);

typedef enum
{
    VID_VDOIN_INFO_CHANGE_UNKNOWN = 0,
    VID_VDOIN_INFO_CHANGE_RES,
    VID_VDOIN_INFO_CHANGE_COLOR_SPACE,
    VID_VDOIN_INFO_CHANGE_ASR,
    VID_VDOIN_INFO_CHANGE_TYPE,
    VID_VDOIN_INFO_CHANGE_SRCDEEPCOLOR,
    VID_VDOIN_INFO_CHANGE_ALL,
}VID_VDOIN_INFO_CHANGE_T;

void vVinSetPBType(VIN_PB_TYPE_T rPBType);
BOOL fgSetVswLockDevice(VIDEO_IN_SRC_ID_E eDeviceId);
extern void vResInfoToUserSpace(VID_VDOIN_RES_INFO_T *pe_resolution_info, HDMI_RESOLUTION_MODE_T  eInputRes);
extern void vAspInfoToUserSpace(VID_VDOIN_SRC_ASPECT_RATIO_T *pe_aspect_ratio, SOURCE_ASPECT_RATIO_T  eAspectRatio);
INT32 VdoIn_VswSetInfo(INPUT_DEVICE_INFO_T* peDeviceInfo, VID_VDOIN_INFO_CHANGE_T e_info_change_type);
int vInitBuffer(VIDEO_IN_BUFFER_INFO *pVideoInBuffer);
int vDQBuf(VIDEO_IN_BUFFER_INFO *pVideoInBuffer);
int vRequestBuffer(VIDEO_IN_REQBUF *pRequestBuffer);
int vFillBuf();
int vReset();
int vQBuf(UINT8 idx);
int vStreamOn();
int vStreamOff();
void vShowvideoinstatus(void);
#endif
