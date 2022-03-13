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
#ifndef _TVE_FB_CTRL_H_
#define _TVE_FB_CTRL_H_

#include <linux/mmprofile.h>


struct COLOR_BAR_YUV_DATA{
	unsigned char Y;
	unsigned char U;
	unsigned char V;
};

struct COLOR_BAR_RGB_DATA{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

enum COLOR_BAR_ID{
	COLOR_BAR_RED = 0,
	COLOR_BAR_GREEN,
	COLOR_BAR_BLUE,
	COLOR_BAR_NO,
};

enum RDMA1_COLOR_BAR_TYPE{
	RDMA1_COLOR_BAR_YUYV = 0,
	RDMA1_COLOR_BAR_UYUV,
	RDMA1_COLOR_BAR_RGB,
	RDMA1_COLOR_BAR_BGR,
	RDMA1_COLOR_BAR_NO
};



struct TVE_MMP_EVENTS
{
    MMP_Event HDMI;
    MMP_Event DDPKBitblt;
    MMP_Event OverlayDone;
    MMP_Event SwitchRDMABuffer;
    MMP_Event SwitchOverlayBuffer;
    MMP_Event StopOverlayBuffer;
    MMP_Event RDMA1RegisterUpdated;
    MMP_Event WDMA1RegisterUpdated;
    MMP_Event WaitVSync;
    MMP_Event BufferPost;
    MMP_Event BufferInsert;
    MMP_Event BufferAdd;
    MMP_Event BufferUsed;
    MMP_Event BufferRemove;
    MMP_Event FenceCreate;
    MMP_Event FenceSignal;
    MMP_Event HDMIState;
    MMP_Event GetDevInfo;
    MMP_Event ErrorInfo;
    MMP_Event MutexErr;
    MMP_Event BufferCfg;
    MMP_Event BufferUpdate;
} ;

unsigned int tve_get_disp_width(unsigned res);
unsigned int tve_get_disp_height(unsigned res);

#endif
