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
#ifndef     HDMI_RX_DVI_H
#define     HDMI_RX_DVI_H
#include "vga_timing.h"
#include <linux/irqreturn.h>

#define RANGE_CHECKING(a, b, offset)  ((UINT32)((a)+(offset)-(b)) <= ((offset)*2))
#define CCIR_decode_444	 0

irqreturn_t hdmirx_irq_handle(int irq, void *dev_id);
UINT8 u1GetRxCapturedTiming(void);
HDMI_RESOLUTION_MODE_T u4GetHdmiRxRes(void);
void vDviModeDetect(void);
void vDviChkModeChange(void);
BOOL fgCheckRxDetectDone(void);
UINT8 bDviStdTimingSearch(UINT8 bMode, UINT16   wDviHclk, UINT8 bDviVclk, UINT16 wDviHtotal,UINT16 wDviWidth);
void vHdmiRxDviStatus(void);
void vShowRxResoInfoStatus(void);
UINT8 bHDMI3DPacketVaild(void);
UINT16 wHDMI3DGetHVParameter(UINT8 bType);
void vDviInitial(void);
BOOL fgRxInputNtsc60(void);
HDMI_RESOLUTION_MODE_T vConvertHdmiRXResToVDORes(UINT8 u2Timing);
#endif
