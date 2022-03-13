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
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/jiffies.h>

#include "hdmi_rx_ctrl.h"
#include "dgi_if.h"
#include "vin_main.h"
#include "vin_hal.h"
#include "vin_drv_if.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"
#include "hdmi_rx_hw.h"
#include "hdmi_rx_dvi.h"
#include "hdmitable.h"
#include "hdmiedid.h"

extern HDMI_AV_INFO_T _stAvdAVInfo;
extern VDOIN_DEV_CTRL_INFO_T _arVdoInDevCtrlInfo [VIN_DEVICE_MAX];
INPUT_DEVICE_INFO_T g_rDeviceCap[VIN_DEVICE_MAX];
static VIDEO_IN_SRC_ID_E _HdmiRXPortSwitch = VIN_HDMI_1;
static VSW_UTIL_FUNCS vsw_util = {0};

BYTE _bHdmiRepeaterMode = HDMI_SOURCE_MODE;
BYTE _bAppHdmiRepeaterMode = HDMI_SOURCE_MODE;
extern BOOL hdmirxbypassmode;
extern UINT8   _bHDMICurrSwitch;
extern UINT8   _bAppHDMICurrSwitch ;
extern BOOL    _fgBDPModeChgRes;
extern BYTE _bHdmiRepeaterModeDelayCount;
extern UINT8   _bDviModeDetState;
extern HDMI_RESOLUTION_MODE_T _bVDITiming;
extern unsigned char state_cur;
extern unsigned char state_pre;

const char* szHdmiRxPordStatusStr[] =
{
	"HDMI_RX_STATE_PWER_5V_STATUS",
	"HDMI_RX_STATE_EDID_STATUS",
	"HDMI_RX_INFOFRAME_NOTIFY",
	"HDMI_RX_TIMING_STATUS_LOCK",
	"HDMI_RX_TIMING_STATUS_UNLOCK",
	"HDMI_RX_RESOLUTION_CHANGEING",
	"HDMI_RX_RESOLUTION_CHANGE_DONE",
	"HDMI_RX_ASR_INFO_CHANGE",
	"HDMI_RX_CSP_INFO_CHANGE",  //COLORSPACE
	"HDMI_RX_TYPE_INFO_CHANGE", //JPEG CINEMA
	"VIDEO_IN_QUEUE_BUFFER_FULL_NOTIFY", //QUEUE FULL
	"VIDEO_IN_QUEUE_BUFFER_NULL_NOTIFY",  //QUEUE NULL
	"VIDEO_IN_SEND_BUFFER_TO_HW_NOTIFY",
	"HDMI_RX_AUD_STATUS_LOCK",
	"HDMI_RX_AUD_STATUS_UNLOCK",
};

static void vsw_set_util_funcs(const VSW_UTIL_FUNCS *util)
{
    memcpy(&vsw_util, util, sizeof(VSW_UTIL_FUNCS));
}
void vNotifyAppState(unsigned char u1state)
{
	if (u1state != state_pre) {
		printk("u1state = %d, %s \n",u1state, szHdmiRxPordStatusStr[u1state]);
		state_pre = u1state;
	} else {
		return;
	}
	if (NULL == vsw_util.state_callback) {
		VSW_LOG("vsw_util isn't initialized\n");
		return;
	}
	switch(u1state)
	{
		case HDMI_RX_STATE_PWER_5V_STATUS:
			vsw_util.state_callback(HDMI_RX_STATE_PWER_5V_STATUS);
		break;

		case HDMI_RX_STATE_EDID_STATUS:
			vsw_util.state_callback(HDMI_RX_STATE_EDID_STATUS);
		break;

		case HDMI_RX_INFOFRAME_NOTIFY:
			vsw_util.state_callback(HDMI_RX_INFOFRAME_NOTIFY);
		break;

		case HDMI_RX_TIMING_STATUS_LOCK:
			vsw_util.state_callback(HDMI_RX_TIMING_STATUS_LOCK);
		break;

		case HDMI_RX_TIMING_STATUS_UNLOCK:
			vsw_util.state_callback(HDMI_RX_TIMING_STATUS_UNLOCK);
		break;

		case HDMI_RX_CSP_INFO_CHANGE:
			vsw_util.state_callback(HDMI_RX_CSP_INFO_CHANGE);
		break;
		
		case HDMI_RX_ASR_INFO_CHANGE:
			vsw_util.state_callback(HDMI_RX_ASR_INFO_CHANGE);
		break;

		case HDMI_RX_TYPE_INFO_CHANGE:
			vsw_util.state_callback(HDMI_RX_TYPE_INFO_CHANGE);
		break;

		case VIDEO_IN_QUEUE_BUFFER_FULL_NOTIFY:
			vsw_util.state_callback(VIDEO_IN_QUEUE_BUFFER_FULL_NOTIFY);
		break;

		case VIDEO_IN_QUEUE_BUFFER_NULL_NOTIFY:
			vsw_util.state_callback(VIDEO_IN_QUEUE_BUFFER_NULL_NOTIFY);
		break;

		case VIDEO_IN_SEND_BUFFER_TO_HW_NOTIFY:
			vsw_util.state_callback(VIDEO_IN_SEND_BUFFER_TO_HW_NOTIFY);
		break;

		case HDMI_RX_AUD_STATUS_LOCK:
			vsw_util.state_callback(HDMI_RX_AUD_STATUS_LOCK);
		break;

		case HDMI_RX_AUD_STATUS_UNLOCK:
			vsw_util.state_callback(HDMI_RX_AUD_STATUS_UNLOCK);
		break;

		default:
		break;

	}
}

void vVSWGetHDMIRXStatus(VSW_GET_INFO_COND_T HdmiRxStatus)
{
	unsigned char bData=0xff;
    INPUT_DEVICE_INFO_T rDeviceCap;

    memset(&rDeviceCap, 0, sizeof(INPUT_DEVICE_INFO_T));
	VSW_LOG("HdmiRxStatus = %d @ %d \n", HdmiRxStatus,__LINE__);

	if(fgIsVinInitDone())
	{
		printk("HdmiRxStatus = %d @ %d \n", HdmiRxStatus,__LINE__);
		switch (HdmiRxStatus)
		{
		case VSW_COMP_NFY_ERROR:
		case VSW_COMP_NFY_RESOLUTION_CHGING:
		default:
		break;

		case VSW_COMP_NFY_UNLOCK:
			_HdmiRXPortSwitch = VIN_DEVICE_UNKNOW;
			bData = HDMI_RX_TIMING_STATUS_UNLOCK;
			if(bData!=0xff)
				vNotifyAppState(bData);
		break;


		case VSW_COMP_NFY_RESOLUTION_CHG_DONE:
			vVSWGetRXInfo(&rDeviceCap);
			VdoIn_VswSetInfo(&rDeviceCap, VID_VDOIN_INFO_CHANGE_ALL);
			memcpy(&g_rDeviceCap[rDeviceCap.eDeviceId], &rDeviceCap, sizeof(INPUT_DEVICE_INFO_T));
			if( g_rDeviceCap[rDeviceCap.eDeviceId].fgIsTimingOk)
			{
				bData = HDMI_RX_TIMING_STATUS_LOCK;
				if(bData!=0xff)
					vNotifyAppState(bData);
			}
		break;

		case VSW_COMP_NFY_COLOR_SPACE_CHG:
			vVSWGetRXInfo(&rDeviceCap);
			VdoIn_VswSetInfo(&rDeviceCap, VID_VDOIN_INFO_CHANGE_ALL);
			memcpy(&g_rDeviceCap[rDeviceCap.eDeviceId], &rDeviceCap, sizeof(INPUT_DEVICE_INFO_T));
			bData = HDMI_RX_CSP_INFO_CHANGE;
			if(bData!=0xff)
				vNotifyAppState(bData);
		break;

		case VSW_COMP_NFY_ASPECT_CHG:
			vVSWGetRXInfo(&rDeviceCap);
			VdoIn_VswSetInfo(&rDeviceCap, VID_VDOIN_INFO_CHANGE_ASR);
			memcpy(& _arVdoInDevCtrlInfo[rDeviceCap.eDeviceId].rVdoInDevInfo, &rDeviceCap, sizeof(INPUT_DEVICE_INFO_T));
			bData = HDMI_RX_ASR_INFO_CHANGE;
			if(bData!=0xff)
				vNotifyAppState(bData);
		break;

		case VSW_COMP_NFY_JPEG_CHG:
		case VSW_COMP_NFY_CINEMA_CHG:
			vVSWGetRXInfo(&rDeviceCap);
			VdoIn_VswSetInfo(&rDeviceCap, VID_VDOIN_INFO_CHANGE_TYPE);
			bData = HDMI_RX_TYPE_INFO_CHANGE;
			if(bData!=0xff)
				vNotifyAppState(bData);
		break;
		}
	}
    return;
}

void VdoIn_Switch(VIDEO_IN_SRC_ID_E eVdoInDrvDevice)
{
    printk("VDOIN: eVdoInDrvDevice = 0x%x\n", eVdoInDrvDevice);
	vin_hal_set_src_type(eVdoInDrvDevice);
}

void vHdmiRepeaterAudMode(HDMI_RX_AUD_MODE bHdmiAudMode)
{
	printk("[HDMI RX AUD] bHdmiAudMode = %d\n",bHdmiAudMode);
	if(HDMI_REPEATER_AUDIO_DRAM_MODE == bHdmiAudMode)
	{
		printk("[HDMI RX] HDMI_REPEATER_AUDIO_DRAM_MODE\n");
		hdmirxbypassmode = FALSE;
	}
	else if(HDMI_REPEATER_AUDIO_BYPASS_MODE == bHdmiAudMode)
	{
		printk("[HDMI RX] HDMI_REPEATER_AUDIO_BYPASS_MODE\n");
		hdmirxbypassmode = TRUE;
	}
}

void vEnableHdmiRepeaterAudTask(bool enable)
{
    vEnableHdmiRxAudTask(enable);

}
void vHdmiRepeaterMode(HDMI_RX_STATUS_MODE bHdmiMode,enum HDMI_SWITCH_NO hdmiinport)
{

	printk("[HDMI RX] bHdmiMode = %d,hdmiinport = %d\n",bHdmiMode,hdmiinport);
    if(_bHdmiRepeaterMode != bHdmiMode)
    {
        hdmi_tmdsonoff(0);
        if(_bHdmiRepeaterMode == HDMI_SOURCE_MODE)
        {
            _bHdmiRepeaterModeDelayCount = 0;
        }
        if(bHdmiMode == HDMI_SOURCE_MODE)
        {
			vHal_Disable_DGI_In();
        }
        _fgBDPModeChgRes = TRUE;
    }
    _bAppHdmiRepeaterMode = bHdmiMode;

   	if(bHdmiMode == HDMI_SOURCE_MODE)
	{
		vHalEnableINTR2_CKDT(FALSE);
		_bAppHdmiRepeaterMode = HDMI_SOURCE_MODE;
		_bAppHDMICurrSwitch = HDMI_SWITCH_INIT;
		vHalHDMIRxEnableVsyncInt(FALSE);
		_bVDITiming = 106;
		_bDviModeDetState = 0;
		printk("[HDMI RX] HDMI_SOURCE_MODE\n");
	}
	else if(bHdmiMode == HDMI_REPEATER_VIDEO_BYPASS_MODE)
	{
		printk("[HDMI RX] HDMI_REPEATER_VIDEO_BYPASS_MODE\n");
		_arVdoInDevCtrlInfo [_bAppHDMICurrSwitch].fgIsByPass = 1;
        if(hdmiinport == HDMI_SWITCH_1)
            _bAppHDMICurrSwitch = HDMI_SWITCH_1;
        else
            _bAppHDMICurrSwitch = HDMI_SWITCH_2;
	}
	else if(bHdmiMode == HDMI_REPEATER_VIDEO_DRAM_MODE)
	{
		printk("[HDMI RX] HDMI_REPEATER_VIDEO_DRAM_MODE\n");
		_arVdoInDevCtrlInfo [_bAppHDMICurrSwitch].fgIsByPass = 0;
        if(hdmiinport == HDMI_SWITCH_1)
            _bAppHDMICurrSwitch = HDMI_SWITCH_1;
        else
            _bAppHDMICurrSwitch = HDMI_SWITCH_2;
	}
}

void video_in_start_cmd(void)
{
    INT32 i4DevId;
    INPUT_DEVICE_INFO_T rVdoInInfo;

    for (i4DevId = 0; i4DevId < VIN_DEVICE_MAX; i4DevId++)
    {
	   if(fgSetVswLockDevice(i4DevId))
       {
           break;
       }
    }

    if(i4DevId >= VIN_DEVICE_MAX)
    {
        printk("VDOIN: Error Play Cmd, No Device locked\n");
        return;
    }
    printk("VDOIN: VdoIn_Play, DeviceID = 0x%x\n", i4DevId);
	vin_hal_set_src_type(i4DevId);
    rVdoInInfo.eInputMode = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eInputMode;
    rVdoInInfo.ePinType = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.ePinType;
    rVdoInInfo.eInputRes = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eInputRes;
    rVdoInInfo.fgVgaIsCeaType = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.fgVgaIsCeaType;
    rVdoInInfo.fgNTSC60= _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.fgNTSC60;
    rVdoInInfo.rVdoInWDramType.eVdoInAddrMode = VIN_LINEAR;
    rVdoInInfo.rVdoInWDramType.eVdoInSwapMode = VIN_SWAP_MODE_0;
    rVdoInInfo.rVdoInWDramType.eVdoInDramFmt = VIN_422;
    rVdoInInfo.eVdoInAR = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eAspectRatio;

    if(!(_arVdoInDevCtrlInfo[i4DevId].fgIsByPass))
    {
        printk( "eVdoInDramFmt = 0x%x\n", rVdoInInfo.rVdoInWDramType.eVdoInDramFmt);
        printk( "eVdoInAddrMode = 0x%x\n", rVdoInInfo.rVdoInWDramType.eVdoInAddrMode);
        printk("eVdoInSwapMode = 0x%x\n", rVdoInInfo.rVdoInWDramType.eVdoInSwapMode);
        printk("eVdoInResMode = 0x%x\n", rVdoInInfo.eInputRes);
        printk("eVdoInMode = 0x%x\n", rVdoInInfo.eInputMode);
        printk("eVdoInPinType = 0x%x\n", rVdoInInfo.ePinType);
        printk("eVdoInAR = 0x%x\n", rVdoInInfo.eVdoInAR);

        if(fgVinSetCfgInfo(rVdoInInfo))
        {
      		vVinSetPBType(VIN_PB_TYPE_START_CMD);
		}
    }
}

void video_in_stop_cmd(void)
{
    INT32 i4DevId;

    printk ("VDOIN: Video_In_Stop_CMD\n");

    for (i4DevId = 0; i4DevId < VIN_DEVICE_MAX; i4DevId++)
    {
	   if(fgSetVswLockDevice(i4DevId))
       {
           break;
       }
    }

    if(i4DevId >= VIN_DEVICE_MAX)
    {
        printk("VDOIN: Error Stop Cmd, No Device locked\n");
        return;
    }

    if(!(_arVdoInDevCtrlInfo[i4DevId].fgIsByPass))
    {
        vVinSetPBType(VIN_PB_TYPE_STOP_CMD);
    }
}

INT32 video_in_config_info_get(VID_VDOIN_ALL_INFO_T* pVideoInInfo)
{
    INT32 i4DevId;
 	UINT32 eVideoInInfoType;
    for (i4DevId = 0; i4DevId < VIN_DEVICE_MAX; i4DevId++)
    {
		if(fgSetVswLockDevice(i4DevId))
        {
            break;
        }
    }
    if(i4DevId >= VIN_DEVICE_MAX)
    {
        printk("VDOIN: Error Get Cmd, No Device locked\n");
        return (-1);
    }
	eVideoInInfoType = pVideoInInfo->eVddoInInfoType;
	
    printk ("video_in_config_info_get, eVideoInInfoType = 0x%x\n", eVideoInInfoType);
    if (eVideoInInfoType & VIN_GET_INFO_TYPE_RES)
    {
         vResInfoToUserSpace(&(pVideoInInfo->t_resolution_info), _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eInputRes);
		 printk ("htotal = %d,vtotal = %d,eInputRes = %d\n", \
		 		pVideoInInfo->t_resolution_info.ui4_h_total,pVideoInInfo->t_resolution_info.ui4_v_total,_arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eInputRes);
    }
    if (eVideoInInfoType & VIN_GET_INFO_TYPE_ASR)
    {
		printk ("_arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eAspectRatio = %d\n", \
			   _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eAspectRatio);
         vAspInfoToUserSpace(&(pVideoInInfo->e_aspect_ratio), _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eAspectRatio);
    }
    if (eVideoInInfoType & VIN_GET_INFO_TYPE_SRC_COLORSPACE)
    {
		printk ("_arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.esrccsp = %d\n", \
			   _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.esrccsp);
         pVideoInInfo->e_src_csp = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.esrccsp;
    }
    if (eVideoInInfoType & VIN_GET_INFO_TYPE_SRC_DEEPCOLOR)
    {
		printk ("_arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.esrcdeepcolor = %d\n", \
			   _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.esrcdeepcolor);
   		pVideoInInfo->e_src_deepcolor = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.esrcdeepcolor;
    }
    if (eVideoInInfoType & VIN_GET_INFO_TYPE_DATA)
    {
         pVideoInInfo->t_data_info.b_is_jpeg = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.fgIsJpeg;
         pVideoInInfo->t_data_info.b_is_cinema = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.fgIsCinema;
    }
    if (eVideoInInfoType & VIN_GET_INFO_TYPE_SIGNAL)
    {
		printk ("_arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eInputMode = %d,_arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.ePinType = %d\n", \
		   _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eInputMode,_arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.ePinType);
         pVideoInInfo->t_signal_info.eVdoinMode = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.eInputMode;
         pVideoInInfo->t_signal_info.eVdoinDataFmt = _arVdoInDevCtrlInfo[i4DevId].rVdoInDevInfo.ePinType;
    }
    return 0;
}

UINT32 v_convert_res_to_pixel_clk(BYTE bres,BOOL fgntsc60)
{
    UINT32 u4PixelClk;

    switch(bres)
    {
    case RES_2D_640x480HZ://39
        if(fgntsc60)
            u4PixelClk = 25200000;
        else
            u4PixelClk = 25175000;
        break;
    case RES_480I:
    case RES_480P: //2
    case RES_3D_480P60HZ_TB://80
    case RES_3D_480I30HZ_TB://81
    case RES_3D_480I60HZ_TB://82
    case RES_3D_480P60HZ_SBS_HALF://101
    case RES_3D_480I30HZ_SBS_HALF://102
    case RES_3D_480I60HZ_SBS_HALF://103
        if(fgntsc60)
            u4PixelClk = 27027000;
        else
            u4PixelClk = 27000000;
        break;
    case RES_576I: //1
    case RES_576P: //3
    case RES_3D_576P50HZ_TB://77
    case RES_3D_576I25HZ_TB://78
    case RES_3D_576I50HZ_TB://79
    case RES_3D_576P50HZ_SBS_HALF://98
    case RES_3D_576I25HZ_SBS_HALF://99
    case RES_3D_576I50HZ_SBS_HALF://100
        u4PixelClk = 27000000;
        break;
    case RES_PANEL_AUO_B089AW01: //40 //Total: 1344x625: Act: 1024x600: Frm: 60Hz: Clk: 50.4MHz
        u4PixelClk = 50400000;
        break;
    case RES_480I_2880://16
    case RES_480P_1440://4
    case RES_3D_480P60HZ: //28: 480p120hz
    case RES_3D_480I30HZ: //34: 480i120hz
    case RES_3D_480I60HZ: //36: 480i120hz
    case RES_2D_480I60HZ: //37: 480i60hz
        if(fgntsc60)
            u4PixelClk = 27027000*2;
        else
            u4PixelClk = 27000000*2;
        break;
    case RES_576I_2880://17
    case RES_576P_1440://5
    case RES_3D_576P50HZ: //27: 576p100hz
    case RES_3D_576I25HZ: //33: 576i100hz
    case RES_3D_576I50HZ: //35: 576i100hz
    case RES_2D_576I50HZ: //38: 576i50hz
        u4PixelClk = 27000000*2;
        break;
    case RES_720P23HZ: // 56
    case RES_3D_720P23HZ_TB://76
    case RES_3D_720P23HZ_SBS_HALF://97
        if(fgntsc60)
            u4PixelClk = 59400000;
        else
            u4PixelClk = 59340600;
        break;
    case RES_720P24HZ: // 55
    case RES_3D_720P24HZ_TB://75
    case RES_3D_720P24HZ_SBS_HALF://96
        if(fgntsc60)
            u4PixelClk = 59400000;
        else
            u4PixelClk = 59340600;
        break;
    case RES_720P60HZ://8
    case RES_1080I60HZ://10
    case RES_1080P23_976HZ: //19: 1080P23.976hz
    case RES_1080P29_97HZ: //20: 1080P29.97hz
    case RES_3D_720P60HZ_TB:  //41
    case RES_3D_1080P23HZ_TB: //45
    case RES_3D_1080I60HZ_SBS_HALF://43
    case RES_3D_1080I60HZ_TB: //69
    case RES_3D_1080P29HZ_TB://67
    case RES_3D_1080P23HZ_SBS_HALF://89
    case RES_3D_720P60HZ_SBS_HALF://92
    case RES_3D_1080P29HZ_SBS_HALF://86
    case RES_1080P30HZ://14
    case RES_1080P24HZ://18
    case RES_720P30HZ: // 53
    case RES_3D_1080P24HZ_TB://46
    case RES_3D_1080P30HZ_TB: //66
    case RES_3D_720P30HZ_SBS_HALF://94
    case RES_3D_1080I30HZ_TB://71
    case RES_3D_1080P24HZ_SBS_HALF://88
    case RES_3D_1080I30HZ_SBS_HALF://90
    case RES_3D_1080P30HZ_SBS_HALF://85
    case RES_3D_720P30HZ_TB://73
        if(fgntsc60)
            u4PixelClk = 74250000;
        else
            u4PixelClk = 74176000;
        break;
    case RES_720P50HZ://9
    case RES_1080I50HZ://11
    case RES_1080P25HZ: //15
    case RES_720P25HZ: // 54
    case RES_3D_720P50HZ_TB:  //42
    case RES_3D_1080I50HZ_SBS_HALF://44
    case RES_3D_1080P25HZ_TB: //68
    case RES_3D_1080I50HZ_TB://70
    case RES_3D_1080I25HZ_TB://72
    case RES_3D_720P25HZ_TB://74
    case RES_3D_1080P25HZ_SBS_HALF://87
    case RES_3D_1080I25HZ_SBS_HALF://91
    case RES_3D_720P50HZ_SBS_HALF://93
    case RES_3D_720P25HZ_SBS_HALF://95
        u4PixelClk = 74250000;
        break;

    case RES_480P_2880://6
        if(fgntsc60)
            u4PixelClk = 27027000*4;
        else
            u4PixelClk = 27000000*4;
        break;
    case RES_576P_2880://7
        u4PixelClk = 27000000*4;
        break;

    case RES_3D_720P23HZ: //63
        if(fgntsc60)
            u4PixelClk = 59400000*2;
        else
            u4PixelClk = 59340600*2;
        break;

    case RES_3D_720P24HZ: //62
        if(fgntsc60)
            u4PixelClk = 59400000*2;
        else
            u4PixelClk = 59340600*2;
        break;

    case RES_1080P60HZ://12
    case RES_3D_1080P23HZ: //21: 1080p47.952Hz
    case RES_3D_720P60HZ: //23: 720p120hz
    case RES_3D_1080I60HZ: //29: 1080i120hz
    case RES_3D_1080I30HZ: //31: 1080i120hz
    case RES_3D_1080P29HZ://60
    case RES_3D_1080P60HZ_TB: //64
    case RES_3D_1080P60HZ_SBS_HALF://83
    case RES_3D_1080P30HZ://59
    case RES_3D_720P30HZ: //25: 720p120hz
    case RES_3D_1080P24HZ: //22: 1080p48hz
        if(fgntsc60)
            u4PixelClk = 74250000*2;
        else
            u4PixelClk = 74176000*2;
        break;

    case RES_1080P50HZ://13
    case RES_3D_1080P50HZ_TB: //65
    case RES_3D_720P50HZ: //24: 720p100hz
    case RES_3D_720P25HZ: //26: 720p100hz
    case RES_3D_1080I50HZ: //30: 1080i100hz
    case RES_3D_1080I25HZ: //32: 1080i100hz
    case RES_3D_1080P25HZ://61
    case RES_3D_1080P50HZ_SBS_HALF://84
        u4PixelClk = 74250000*2;
        break;

    default:
        u4PixelClk = 27000000;
        break;
    }

    return u4PixelClk;
}

BOOL fgHdckUseSlaveMode(void)
{
    return TRUE;
    BOOL fgHdckUseSlaveMode = FALSE;
    UINT32 u4VdoInputRes,u4InputPixelClk;
    UINT32 u4VdoOutputRes,u4OutputPixelClk;
    BOOL fgInputHD, fgOutputHD, fgInputOutput;
    u4VdoInputRes = u4GetHdmiRxRes();
    u4VdoOutputRes = _stAvdAVInfo.e_resolution;
    u4InputPixelClk = v_convert_res_to_pixel_clk(u4VdoInputRes,fgRxInputNtsc60());
    u4OutputPixelClk = v_convert_res_to_pixel_clk(u4VdoOutputRes,fgRxInputNtsc60());
    fgInputHD = fgIsHDRes(u4VdoInputRes);
    fgOutputHD = fgIsHDRes(u4VdoOutputRes);
    fgInputOutput = (fgInputHD&&fgOutputHD)||((!fgInputHD)&&(!fgOutputHD));

    if(fgHdmiRepeaterIsDramMode())
    {
        if((((u4OutputPixelClk%u4InputPixelClk)==0)||((u4InputPixelClk%u4OutputPixelClk)==0))&&fgInputOutput)
        {
            fgHdckUseSlaveMode = TRUE;//Use slave mode
        }
    }
    printk("[PMX] HDCK slave Mode = %d\n",fgHdckUseSlaveMode);
    return fgHdckUseSlaveMode;
}

void vconfig_hdmirx_sys(unsigned char bRes)
{
    if(fgHdckUseSlaveMode())
	{
		bRes = u4GetHdmiRxRes();
		vRxWriteRegMsk(0x209000,0,(1<<24));
		vRxWriteRegMsk(0x4000904,(1<<16),(1<<16));

		if(v_convert_res_to_pixel_clk(bRes,fgRxInputNtsc60())<=27027000)
		{
			vRxWriteReg(0x209000,0x7E00E131);
			udelay(20);
			vRxWriteReg(0x209000,0x7E00E133);
			udelay(20);
			vRxWriteReg(0x209000,0x7F00E133);
			udelay(20);
			vRxWriteReg(0x20925C,0x00000003);
			udelay(10);
			vRxWriteReg(0x20925C,0x00000001);
			udelay(10);
			vRxWriteReg(0x209254,0x80100000);
			vRxWriteReg(0x209250,0x00000141);
			vRxWriteRegMsk(0x209004,0,(1<<12)|(1<<11)|(1<<10));
			vRxWriteRegMsk(0x80,(1<<24)|(1<<25),(1<<24)|(1<<25));
		}
		else if(v_convert_res_to_pixel_clk(bRes,fgRxInputNtsc60())<=54054000)
		{
			vRxWriteRegMsk(0x209004,(1<<10),(1<<12)|(1<<11)|(1<<10));
			vRxWriteReg(0x209000,0x7E00E131);
			udelay(20);
			vRxWriteReg(0x209000,0x7E00E133);
			udelay(20);
			vRxWriteReg(0x209000,0x7F00E133);
			udelay(20);
			vRxWriteReg(0x20925C,0x00000003);
			udelay(10);
			vRxWriteReg(0x20925C,0x00000001);
			udelay(10);
			vRxWriteReg(0x209254,0x80100000);
			vRxWriteReg(0x209250,0x00000141);
			vRxWriteRegMsk(0x80,(1<<24),(1<<24)|(1<<25));
		}
		else if(v_convert_res_to_pixel_clk(bRes,fgRxInputNtsc60())<=74250000)
		{
		    printk("[enter %s]  74M\n",__FUNCTION__);
			vRxWriteRegMsk(0x209004,(1<<11),(1<<12)|(1<<11)|(1<<10));
			vRxWriteReg(0x209000,0x7E00E131);
			udelay(20);
			vRxWriteReg(0x209000,0x7E00E133);
			udelay(20);
			vRxWriteReg(0x209000,0x7F00E133);
			udelay(20);
			vRxWriteReg(0x20925C,0x00000003);
			udelay(10);
			vRxWriteReg(0x20925C,0x00000001);
			udelay(10);
			vRxWriteReg(0x209254,0x80100000);
			vRxWriteReg(0x209250,0x00000131);
			vRxWriteRegMsk(0x80,(1<<25),(1<<24)|(1<<25));
		}
		else if(v_convert_res_to_pixel_clk(bRes,fgRxInputNtsc60())<=108108000)
		{
			vRxWriteReg(0x209000,0x7E00E131);
			udelay(20);
			vRxWriteReg(0x209000,0x7E00E133);
			udelay(20);
			vRxWriteReg(0x209000,0x7F00E133);
			udelay(20);
			vRxWriteReg(0x20925C,0x00000003);
			udelay(10);
			vRxWriteReg(0x20925C,0x00000001);
			udelay(10);
			vRxWriteReg(0x209254,0x80100000);
			vRxWriteReg(0x209250,0x00000141);
			vRxWriteRegMsk(0x209004,(1<<11),(1<<12)|(1<<11)|(1<<10));
			vRxWriteRegMsk(0x80,(1<<24),(1<<24)|(1<<25));
		}
		else if(v_convert_res_to_pixel_clk(bRes,fgRxInputNtsc60())<=148500000)
		{

			printk("[enter %s]	148M\n",__FUNCTION__);
			vRxWriteReg(0x209000,0x7E00E131);
			udelay(20);
			vRxWriteReg(0x209000,0x7E00E133);
			udelay(20);
			vRxWriteReg(0x209000,0x7F00E133);
			udelay(20);
			vRxWriteReg(0x20925C,0x00000003);
			udelay(10);
			vRxWriteReg(0x20925C,0x00000001);
			udelay(10);
			vRxWriteReg(0x209254,0x80100000);
			vRxWriteReg(0x209250,0x00000131);
			vRxWriteRegMsk(0x209004,(1<<11)|(1<<10),(1<<12)|(1<<11)|(1<<10));
			vRxWriteRegMsk(0x80,(1<<24),(1<<24)|(1<<25));
		}
		else
		{
			vRxWriteReg(0x209000,0x7E00E131);
			udelay(20);
			vRxWriteReg(0x209000,0x7E00E133);
			udelay(20);
			vRxWriteReg(0x209000,0x7F00E133);
			udelay(20);
			vRxWriteReg(0x20925C,0x00000003);
			udelay(10);
			vRxWriteReg(0x20925C,0x00000001);
			udelay(10);
			vRxWriteReg(0x209254,0x80100000);
			vRxWriteReg(0x209250,0x00000141);
			vRxWriteRegMsk(0x209004,0,(1<<12)|(1<<11)|(1<<10));
			vRxWriteRegMsk(0x80,(1<<24)|(1<<25),(1<<24)|(1<<25));
		}
	}
}

INT32 i4Vsw_Init(void)
{
	UINT8 bCnt;
	for(bCnt = 0; bCnt < VIN_DEVICE_MAX; bCnt++)
	{
		memset(&g_rDeviceCap[bCnt], 0, sizeof(INPUT_DEVICE_INFO_T));
		g_rDeviceCap[bCnt].eDeviceId = VIN_DEVICE_UNKNOW;
	}
	_HdmiRXPortSwitch = VIN_DEVICE_UNKNOW;

	vVSWGetRXInfo(&g_rDeviceCap[VIN_HDMI_1]);
	if(g_rDeviceCap[VIN_HDMI_1].fgIsTimingOk)
	{
		VdoIn_VswSetInfo(&g_rDeviceCap[VIN_HDMI_1], VID_VDOIN_INFO_CHANGE_ALL);
	}
	return 0;
}

void video_in_internal_init(void)
{
    i4Vin_Init();
    i4Vsw_Init();
    memset(_arVdoInDevCtrlInfo, 0, sizeof(VDOIN_DEV_CTRL_INFO_T) * VIN_DEVICE_MAX);
}

void video_in_internal_uninit(void)
{
    i4VinUnInit();
    memset(_arVdoInDevCtrlInfo, 0, sizeof(VDOIN_DEV_CTRL_INFO_T) * VIN_DEVICE_MAX);
}

VSW_DRIVER* VSW_GetDriver(void)
{
	static VSW_DRIVER VSW_DRV =
	{
		.setvsw_util_funcs     = vsw_set_util_funcs,
		.hdmirxinit			   = hdmi_rx_internal_init,
		.videoinit			   = video_in_internal_init,
		.videouninit           = video_in_internal_uninit,
		.videoinstart          = video_in_start_cmd,
		.videoinstop           = video_in_stop_cmd,
		.getvideoininfo        = video_in_config_info_get,
		.hdmirx_power_on       = hdmirx_internal_power_on,
		.hdmirx_power_off      = hdmirx_internal_power_off,
		.probe			       = hdmi_rx_tmr_isr,
		.uninit                = hdmi_rx_uninit,
		.enter                 = hdmi_rx_repeatermode_enter,
        .exit                  = hdmi_rx_repeatermode_exit,
		.suspend               = hdmi_rx_suspend,
		.resume                = hdmi_rx_resume,
		.setvideoinsrcid       = VdoIn_Switch,
		.sethdmirxmode         = vHdmiRepeaterMode,
		.sethdmirxaudmode      = vHdmiRepeaterAudMode,
		.sethdmrxaudtask       = vEnableHdmiRepeaterAudTask,
		.gethdmirxedid         = vGETHDMIRxEDID,
		.sethdmirxedidvalue    = vUserSetHDMIRxEDIDToDrv,
		.gethdmirxpwrstatus    = vHdmiRxPwrStatus,
		.hdmirxstatus          = vShowHDMIRxStatus,
		.dump                  = hdmirx_reg_dump,
		.read                  = hdmirx_read,
		.write                 = hdmirx_write,
		.log_enable            = vsw_log_enable,
		.vInitBuffer		   = vInitBuffer,
		.vRequestBuffer		   = vRequestBuffer,
		.vQBuf				   = vQBuf,
		.vDQBuf 			   = vDQBuf,
		.vFillBuf              = vFillBuf,
		.vStreamOn             = vStreamOn,
		.vStreamOff            = vStreamOff,

	};
	return &VSW_DRV;
}
EXPORT_SYMBOL(VSW_GetDriver);

