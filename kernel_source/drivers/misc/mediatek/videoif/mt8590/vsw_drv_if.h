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
#ifndef     VSW_DRV_IF_H
#define     VSW_DRV_IF_H

#include "hdmi_rx_task.h"
#include "typedef.h"

extern size_t vsw_log_on ;

#define hdmirxedidlog        (0x1)
#define hdmirxhotpluglog     (0x2)
#define hdmirxhdcplog        (0x4)
#define hdmirxhvtotallog     (0x10)
#define hdmirxinfoframelog   (0x20)
#define hdmirxdeepcolorlog   (0x40)
#define hdmirxsyncdetaillog  (0x80)
#define hdmirxdeflog          (0x100)
#define hdmirxdrvlog          (0x200)
#define vindrvlog             (0x400)
#define vswlog                (0x800)

#define hdmirxalllog         (0x3bf)

//////////////////////////////////////////////////HDMI RX EDID ////////////////////////////////////////////////////
#define HDMI_RX_EDID_LOG(fmt, arg...) \
	do { \
		if (vsw_log_on&hdmirxedidlog) {printk("[hdmi_rx_edid]%s,%d ", __func__, __LINE__); printk(fmt, ##arg);} \
	}while (0)

#define HDMI_RX_EDID_FUNC()	\
	do { \
		if(vsw_log_on&hdmirxedidlog) {printk("[hdmi_rx_edid] %s\n", __func__);} \
	}while (0)

/////////////////////////////////// HDMI RX HOTPLUG///////////////////////////////////////////////////////
#define HDMI_RX_HOTPLUG_LOG(fmt, arg...) \
	do { \
		if (vsw_log_on&hdmirxhotpluglog) {printk("[hdmi_rx_hotplug]%s,%d ", __func__, __LINE__); printk(fmt, ##arg);} \
	}while (0)

#define HDMI_RX_HOTPLUG_FUNC()	\
	do { \
		if(vsw_log_on&hdmirxhotpluglog) {printk("[hdmi_rx_hotplug] %s\n", __func__);} \
	}while (0)

/////////////////////////////////// HDMI RX HDCP ///////////////////////////////////////////////////////
#define HDMI_RX_HDCP_LOG(fmt, arg...) \
	do { \
		if (vsw_log_on&hdmirxhdcplog) {printk("[hdmi_rx_hdcp]%s,%d ", __func__, __LINE__); printk(fmt, ##arg);} \
	}while (0)

#define HDMI_RX_HDCP_FUNC()	\
	do { \
		if(vsw_log_on&hdmirxhdcplog) {printk("[hdmi_rx_hdcp] %s\n", __func__);} \
	}while (0)

/////////////////////////////////// HDMI RX HV TOTAL ///////////////////////////////////////////////////////
#define HDMI_RX_HVTOTAL_LOG(fmt, arg...) \
	do { \
		if (vsw_log_on&hdmirxhvtotallog) {printk("[hdmi_rx_hvtotal]%s,%d ", __func__, __LINE__); printk(fmt, ##arg);} \
	}while (0)

#define HDMI_RX_HVTOTAL_FUNC()	\
	do { \
		if(vsw_log_on&hdmirxhvtotallog) {printk("[hdmi_rx_hvtotal] %s\n", __func__);} \
	}while (0)

/////////////////////////////////// HDMI RX INFOFRAME///////////////////////////////////////////////////////
#define HDMI_RX_INFOFRAME_LOG(fmt, arg...) \
	do { \
		if (vsw_log_on&hdmirxinfoframelog) {printk("[hdmi_rx_infoframe]%s,%d ", __func__, __LINE__); printk(fmt, ##arg);} \
	}while (0)

#define HDMI_RX_INFOFRAME_FUNC()	\
	do { \
		if(vsw_log_on&hdmirxinfoframelog) {printk("[hdmi_rx_infoframe] %s\n", __func__);} \
	}while (0)

/////////////////////////////////// HDMI RX DEEPCOLOR///////////////////////////////////////////////////////
#define HDMI_RX_DEEPCOLOR_LOG(fmt, arg...) \
	do { \
		if (vsw_log_on&hdmirxdeepcolorlog) {printk("[hdmi_rx_deepcolor]%s,%d ", __func__, __LINE__); printk(fmt, ##arg);} \
	}while (0)

#define HDMI_RX_DEEPCOLOR_FUNC()	\
	do { \
		if(vsw_log_on&hdmirxdeepcolorlog) {printk("[hdmi_rx_deepcolor] %s\n", __func__);} \
	}while (0)

/////////////////////////////////// HDMI RX SYNC DETAIL///////////////////////////////////////////////////////
#define HDMI_RX_SYNC_LOG(fmt, arg...) \
	do { \
		if (vsw_log_on&hdmirxsyncdetaillog) {printk("[hdmi_rx_sync_detail]%s,%d ", __func__, __LINE__); printk(fmt, ##arg);} \
	}while (0)

#define HDMI_RX_SYNC_FUNC()	\
	do { \
		if(vsw_log_on&hdmirxsyncdetaillog) {printk("[hdmi_rx_sync_detail] %s\n", __func__);} \
	}while (0)


#define HDMIRX_LOG(fmt, arg...) \
	do { \
		if (vsw_log_on&hdmirxdeflog) {printk("[hdmirx]#%d ", __LINE__); printk(fmt, ##arg);} \
	}while (0)

#define HDMIRX_DRV_FUNC()	\
	do { \
		if(vsw_log_on&hdmirxdrvlog) {printk("[hdmirx_video] %s\n", __func__);} \
	}while (0)

#define VIDEO_IN_LOG(fmt, arg...)	\
	do { \
		if (vsw_log_on&vindrvlog) {printk("[videoin]#%d ", __LINE__); printk(fmt, ##arg);} \
	}while (0)

#define VSW_LOG(fmt, arg...)	\
			do { \
				if (vsw_log_on&vswlog) {printk("[vsw]#%d ", __LINE__); printk(fmt, ##arg);} \
			}while (0)


typedef struct
{
    void (*setvsw_util_funcs)(const VSW_UTIL_FUNCS *util);
    int (*hdmirxinit)(void);
    int (*videoinit)(void);
    int (*videouninit)(void);
    void (*videoinstart)(void);
    void (*videoinstop)(void);
	int (*getvideoininfo)(VID_VDOIN_ALL_INFO_T* pVdoInInfo, UINT32 eVdoInInfoType);
	int (*hdmirx_power_on)(void);
	void (*hdmirx_power_off)(void);
    int (*probe)(void);
    int (*uninit)(void);
    void (*enter)(void);
    void (*exit)(void);
    void (*suspend)(void);
    void (*resume)(void);
    int (*setvideoinsrcid)(VIDEO_IN_SRC_ID_E eVdoInDrvDevice);
	void (*sethdmirxmode)(HDMI_RX_STATUS_MODE eMode,enum HDMI_SWITCH_NO hdmiinport);
	void (*sethdmirxaudmode)(HDMI_RX_AUD_MODE bHdmiAudMode);
	void (*sethdmrxaudtask)(bool fgenable);
    int (*gethdmirxedid)(HDMI_RX_EDID_T *pv_get_info);
	void(*sethdmirxedidvalue)(unsigned char *pv_set_info);
	int (*gethdmirxpwrstatus)(unsigned long *prData);
	void (*hdmirxstatus)(void);
    void (*dump)(void);
	void (*read)(unsigned int u2Reg, unsigned int *p4Data);
    void (*write)(unsigned int u2Reg, unsigned int u4Data);
	void (*log_enable)(u16 enable);
	int (*vRequestBuffer)(VIDEO_IN_REQBUF *pRequestBuffer);
	int (*vInitBuffer)(VIDEO_IN_BUFFER_INFO *pVideoInBuffer);
	int (*vQBuf)(UINT8 idx);
	int (*vDQBuf)(VIDEO_IN_BUFFER_INFO *pVideoInBuffer);
	int (*vFillBuf)();
    int (*vStreamOn)();
    int (*vStreamOff)();
} VSW_DRIVER;

VSW_DRIVER* VSW_GetDriver(void);


int vSendBufferToQueue(VIDEO_IN_BUFFER_INFO* prBuf);
void vsw_log_enable(unsigned short enable);
INT32 i4Vsw_Init(void);
void video_in_start_cmd(void);
void video_in_stop_cmd(void);
INT32 video_in_config_info_get(VID_VDOIN_ALL_INFO_T* pVideoInInfo);
void vconfig_hdmirx_sys(unsigned char bRes);
void vNotifyAppState(unsigned char u1state);
extern BOOL fgIsHdmiRepeater(void);
extern void vVSWGetRXInfo(INPUT_DEVICE_INFO_T *pv_get_info);
extern void vVSWGetHDMIRXStatus(VSW_GET_INFO_COND_T HdmiRxStatus);


#endif
