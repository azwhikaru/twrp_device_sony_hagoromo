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
#ifndef _HDMI_RX_TASK_H_
#define _HDMI_RX_TASK_H_

#include "typedef.h"


#define HDMI_RX_TIMER_5MS 1  //timer is 5 ticket 
#define HDMI_RX_TIMER_10MS 2
#define HDMI_RX_TIMER_15MS 3
#define HDMI_RX_TIMER_20MS 4
#define HDMI_RX_TIMER_100MS 20
#define HDMI_RX_TIMER_200MS 40
#define HDMI_RX_TIMER_400MS 80
#define HDMI_RX_TIMER_1S 200

typedef struct
{
  unsigned long    u4Seconds;                  //Number of seconds from startup
  unsigned long    u4Micros;                   //Remainder in microsecond
} HAL_TIME_T;
int hdmi_rx_internal_init(void);
int hdmi_rx_tmr_isr(void);
int hdmirx_internal_power_on(void);
void hdmirx_internal_power_off(void);
signed int hdmi_rx_uninit(void);
void hdmi_rx_repeatermode_exit(void);
void hdmi_rx_suspend(void);
void hdmi_rx_resume(void);
void hdmirx_reg_dump(void);
void hdmirx_read(unsigned int u2Reg, unsigned int *p4Data);
void hdmirx_write(unsigned int u2Reg, unsigned int u4Data);
void hdmi_rx_repeatermode_enter(void);
void vEnableHdmiRxTask(UINT8 u1Enable);
void vDumpHdmiRxEdid(UINT8 u1Edid);
void vSet640x480PEnable(BYTE bType);
void vIssueHdmiRxUpdateEdidCmd(UINT8 u1EdidReady);
void vTxSetKsvReady(BYTE bTxDownStream, UINT16 u2TxBStatus, BYTE *prbTxBksv, BYTE *prbTxKsvlist, BOOL fgTxVMatch);
#endif
