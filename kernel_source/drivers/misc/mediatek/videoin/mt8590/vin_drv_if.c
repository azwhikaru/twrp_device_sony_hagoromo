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
#include <mach/m4u.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/jiffies.h>
#include "vin_drv_if.h"
#include "vin_hal.h"
#include "vin_main.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"
#include "hdmi_rx_ctrl.h"
#include "hdmirx.h"

extern int m4u_do_mva_map_kernel(unsigned int mva, unsigned int size, int sec,
                        unsigned int* map_va, unsigned int* map_size);

VDOIN_DEV_CTRL_INFO_T _arVdoInDevCtrlInfo [VIN_DEVICE_MAX];
extern INPUT_DEVICE_INFO_T g_rDeviceCap[VIN_DEVICE_MAX];
VIN_CTRL_INFO_T _rVinCtrlInfo;
_VIN_QUEUE rQueue;
_VIN_QUEUE rQueue_backup;
int _bVideoInStart = 0;

INT32 VdoIn_VswSetInfo(INPUT_DEVICE_INFO_T* peDeviceInfo, VID_VDOIN_INFO_CHANGE_T e_info_change_type)
{
    switch (e_info_change_type)
    {
        case VID_VDOIN_INFO_CHANGE_RES:
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.eInputRes = peDeviceInfo->eInputRes;	   
            break;
        case VID_VDOIN_INFO_CHANGE_ASR:
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.eAspectRatio = peDeviceInfo->eAspectRatio;
            break;
		case VID_VDOIN_INFO_CHANGE_COLOR_SPACE:
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.esrccsp = peDeviceInfo->esrccsp;
			break;
        case VID_VDOIN_INFO_CHANGE_TYPE:
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.fgIsJpeg  = peDeviceInfo->fgIsJpeg;
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.fgIsCinema  = peDeviceInfo->fgIsCinema;
            break;
        case VID_VDOIN_INFO_CHANGE_ALL:
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.fgIsTimingOk  = peDeviceInfo->fgIsTimingOk;
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.eDeviceId  = peDeviceInfo->eDeviceId;
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.eInputRes  = peDeviceInfo->eInputRes;
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.ePinType  = peDeviceInfo->ePinType;
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.eInputMode  = peDeviceInfo->eInputMode;
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.eAspectRatio  = peDeviceInfo->eAspectRatio;
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.fgIsJpeg  = peDeviceInfo->fgIsJpeg;
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.fgIsCinema  = peDeviceInfo->fgIsCinema;
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.esrccsp = peDeviceInfo->esrccsp;
            _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.esrcdeepcolor = peDeviceInfo->esrcdeepcolor;
			printk("_arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.eAspectRatio =%d\r\n",
							   _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.eAspectRatio);
            break;
        default:
            break;
    }
    _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.fgVgaIsCeaType = peDeviceInfo->fgVgaIsCeaType;
    _arVdoInDevCtrlInfo[peDeviceInfo->eDeviceId].rVdoInDevInfo.fgNTSC60= peDeviceInfo->fgNTSC60;	
    return 0;
}

void vResInfoToUserSpace(VID_VDOIN_RES_INFO_T *pe_resolution_info, HDMI_RESOLUTION_MODE_T  eInputRes)
{
	UINT8 u13DModeType = 0;
	printk("[HDMI RX] eInputRes = %d\n ",eInputRes);
	if((eInputRes == RES_3D_1080P23HZ) ||                                 
	(eInputRes == RES_3D_1080P24HZ) ||                                 
	(eInputRes == RES_3D_720P60HZ) ||                                  
	(eInputRes == RES_3D_720P50HZ) ||                                  
	(eInputRes == RES_3D_720P30HZ) ||                                  
	(eInputRes == RES_3D_720P25HZ) ||                                  
	(eInputRes == RES_3D_576P50HZ) ||                                  
	(eInputRes == RES_3D_480P60HZ) ||                                  
	(eInputRes == RES_3D_1080I60HZ) ||                                 
	(eInputRes == RES_3D_1080I50HZ) ||                                 
	(eInputRes == RES_3D_1080I30HZ) ||                                 
	(eInputRes == RES_3D_1080I25HZ) ||                                 
	(eInputRes == RES_3D_576I25HZ) ||                                  
	(eInputRes == RES_3D_480I30HZ) ||                                  
	(eInputRes == RES_3D_576I50HZ) ||                                  
	(eInputRes == RES_3D_480I60HZ) ||                                  
	(eInputRes == RES_3D_1080P60HZ) ||                                 
	(eInputRes == RES_3D_1080P50HZ) ||                                 
	(eInputRes == RES_3D_1080P30HZ) ||                                 
	(eInputRes == RES_3D_1080P25HZ) ||                                 
	(eInputRes == RES_3D_720P24HZ) ||                                  
	(eInputRes == RES_3D_720P23HZ) )
	{
		u13DModeType = 1;  // frame packet
		pe_resolution_info->ui2_3d_type = VDOIN_3D_FP;
	}
	else if( (eInputRes == RES_3D_1080P60HZ_TB) ||                               
	(eInputRes == RES_3D_1080P50HZ_TB) ||                               
	(eInputRes == RES_3D_1080P30HZ_TB) ||                               
	(eInputRes == RES_3D_1080P25HZ_TB) || 
	(eInputRes == RES_3D_1080P23HZ_TB) || 
	(eInputRes == RES_3D_1080P24HZ_TB) ||                                   
	(eInputRes == RES_3D_1080I60HZ_TB) ||                               
	(eInputRes == RES_3D_1080I50HZ_TB) ||                               
	(eInputRes == RES_3D_1080I30HZ_TB) ||                               
	(eInputRes == RES_3D_1080I25HZ_TB) || 
	(eInputRes == RES_3D_720P60HZ_TB) ||                                
	(eInputRes == RES_3D_720P50HZ_TB) ||                                    
	(eInputRes == RES_3D_720P30HZ_TB) ||                                
	(eInputRes == RES_3D_720P25HZ_TB) ||                                
	(eInputRes == RES_3D_720P24HZ_TB) ||                                
	(eInputRes == RES_3D_720P23HZ_TB) ||                                
	(eInputRes == RES_3D_576P50HZ_TB) ||                                
	(eInputRes == RES_3D_576I25HZ_TB) ||                                
	(eInputRes == RES_3D_576I50HZ_TB) ||                                
	(eInputRes == RES_3D_480P60HZ_TB) ||                                
	(eInputRes == RES_3D_480I30HZ_TB) ||                                
	(eInputRes == RES_3D_480I60HZ_TB) )
	{
		u13DModeType = 1;  // Top and Bottom
		pe_resolution_info->ui2_3d_type = VDOIN_3D_TAB;
	}
	else if ( (eInputRes == RES_3D_1080P60HZ_SBS_HALF) ||                        
	(eInputRes == RES_3D_1080P50HZ_SBS_HALF) ||                        
	(eInputRes == RES_3D_1080P30HZ_SBS_HALF) ||                        
	(eInputRes == RES_3D_1080P25HZ_SBS_HALF) ||                        
	(eInputRes == RES_3D_1080P24HZ_SBS_HALF) ||                        
	(eInputRes == RES_3D_1080P23HZ_SBS_HALF) || 
	(eInputRes == RES_3D_1080I60HZ_SBS_HALF) ||                        
	(eInputRes == RES_3D_1080I50HZ_SBS_HALF) ||                            
	(eInputRes == RES_3D_1080I30HZ_SBS_HALF) ||                        
	(eInputRes == RES_3D_1080I25HZ_SBS_HALF) ||                        
	(eInputRes == RES_3D_720P60HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_720P50HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_720P30HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_720P25HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_720P24HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_720P23HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_576P50HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_576I25HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_576I50HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_480P60HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_480I30HZ_SBS_HALF) ||                         
	(eInputRes == RES_3D_480I60HZ_SBS_HALF))
	{
		u13DModeType = 1;  // Side by Side Half
		pe_resolution_info->ui2_3d_type = VDOIN_3D_SBS;
	}
	else
	{
		u13DModeType = 0;  // no 3D
		pe_resolution_info->ui2_3d_type = VDOIN_3D_NO;
	}


	switch(eInputRes)
	{
		case RES_2D_640x480HZ:
		pe_resolution_info->ui4_h_total = 800;
		pe_resolution_info->ui4_v_total = 525;
		pe_resolution_info->ui4_h_active = 640;
		pe_resolution_info->ui4_v_activel = 480;
		pe_resolution_info->u4_framerate = 60;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break; 
		
		case RES_480I:
		case RES_2D_480I60HZ:	   
		case RES_3D_480I30HZ:	   	
		case RES_3D_480I30HZ_TB:
		case RES_3D_480I30HZ_SBS_HALF:
		case RES_3D_480I60HZ:
		case RES_3D_480I60HZ_TB:
		case RES_3D_480I60HZ_SBS_HALF:
		pe_resolution_info->ui4_h_total = 1716;
		pe_resolution_info->ui4_v_total = 525;
		pe_resolution_info->ui4_h_active = 720;
		pe_resolution_info->ui4_v_activel = 480;
		pe_resolution_info->u4_framerate = 60;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = FALSE;
		break;
		
		case RES_480I_2880:
		pe_resolution_info->ui4_h_total = 3432;
		pe_resolution_info->ui4_v_total = 525;
		pe_resolution_info->ui4_h_active = 720;
		pe_resolution_info->ui4_v_activel = 480;
		pe_resolution_info->u4_framerate = 60;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = FALSE;
		break;
		
		case RES_480P:
		case RES_3D_480P60HZ:	   	
		case RES_3D_480P60HZ_TB:
		case RES_3D_480P60HZ_SBS_HALF:
		pe_resolution_info->ui4_h_total = 858;
		pe_resolution_info->ui4_v_total = 525;
		pe_resolution_info->ui4_h_active = 720;
		pe_resolution_info->ui4_v_activel = 480;
		pe_resolution_info->u4_framerate = 60;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_480P_1440:
		pe_resolution_info->ui4_h_total = 1716;
		pe_resolution_info->ui4_v_total = 525;
		pe_resolution_info->ui4_h_active = 720;
		pe_resolution_info->ui4_v_activel = 480;
		pe_resolution_info->u4_framerate = 60;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_480P_2880:
		pe_resolution_info->ui4_h_total = 3432;
		pe_resolution_info->ui4_v_total = 525;
		pe_resolution_info->ui4_h_active = 720;
		pe_resolution_info->ui4_v_activel = 480;
		pe_resolution_info->u4_framerate = 60;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_576I:
		case RES_2D_576I50HZ:	   	
		case RES_3D_576I25HZ:	
		case RES_3D_576I25HZ_TB:
		case RES_3D_576I25HZ_SBS_HALF:	   	
		case RES_3D_576I50HZ:		   	
		case RES_3D_576I50HZ_TB:
		case RES_3D_576I50HZ_SBS_HALF:
		pe_resolution_info->ui4_h_total = 1728;
		pe_resolution_info->ui4_v_total = 625;
		pe_resolution_info->ui4_h_active = 720;
		pe_resolution_info->ui4_v_activel = 576;
		pe_resolution_info->u4_framerate = 50;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = FALSE;
		break;
		
		case RES_576I_2880:
		pe_resolution_info->ui4_h_total = 3456;
		pe_resolution_info->ui4_v_total = 625;
		pe_resolution_info->ui4_h_active = 720;
		pe_resolution_info->ui4_v_activel = 576;
		pe_resolution_info->u4_framerate = 50;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = FALSE;
		break;
		
		case  RES_576P:
		case RES_3D_576P50HZ:	   	
		case RES_3D_576P50HZ_TB:
		case RES_3D_576P50HZ_SBS_HALF:
		pe_resolution_info->ui4_h_total = 864;
		pe_resolution_info->ui4_v_total = 625;
		pe_resolution_info->ui4_h_active = 720;
		pe_resolution_info->ui4_v_activel = 576;
		pe_resolution_info->u4_framerate = 50;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_576P_1440:
		pe_resolution_info->ui4_h_total = 1728;
		pe_resolution_info->ui4_v_total = 625;
		pe_resolution_info->ui4_h_active = 720;
		pe_resolution_info->ui4_v_activel = 576;
		pe_resolution_info->u4_framerate = 50;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_576P_2880:
		pe_resolution_info->ui4_h_total = 3456;
		pe_resolution_info->ui4_v_total = 625;
		pe_resolution_info->ui4_h_active = 720;
		pe_resolution_info->ui4_v_activel = 576;
		pe_resolution_info->u4_framerate = 50;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_720P60HZ:
		case RES_3D_720P60HZ:	   	
		case RES_3D_720P60HZ_TB: 
		case RES_3D_720P60HZ_SBS_HALF:
		pe_resolution_info->ui4_h_total = 1650;
		pe_resolution_info->ui4_v_total = 750;
		pe_resolution_info->ui4_h_active = 1280;
		pe_resolution_info->ui4_v_activel = 720;
		pe_resolution_info->u4_framerate = 60;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_720P50HZ:
		case RES_3D_720P50HZ:	   	
		case RES_3D_720P50HZ_TB: 
		case RES_3D_720P50HZ_SBS_HALF:
		pe_resolution_info->ui4_h_total = 1980;
		pe_resolution_info->ui4_v_total = 750;
		pe_resolution_info->ui4_h_active = 1280;
		pe_resolution_info->ui4_v_activel = 720;
		pe_resolution_info->u4_framerate = 50;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_720P30HZ:
		case RES_3D_720P30HZ:	 	
		case RES_3D_720P30HZ_TB:
		case RES_3D_720P30HZ_SBS_HALF:
		pe_resolution_info->ui4_h_total = 3300;
		pe_resolution_info->ui4_v_total = 750;
		pe_resolution_info->ui4_h_active = 1280;
		pe_resolution_info->ui4_v_activel = 720;
		pe_resolution_info->u4_framerate = 30;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;	    
		break;	
		
		case RES_720P25HZ:
		case RES_3D_720P25HZ:	 	
		case RES_3D_720P25HZ_TB:
		case RES_3D_720P25HZ_SBS_HALF:
		pe_resolution_info->ui4_h_total = 3960;
		pe_resolution_info->ui4_v_total = 750;
		pe_resolution_info->ui4_h_active = 1280;
		pe_resolution_info->ui4_v_activel = 720;
		pe_resolution_info->u4_framerate = 25;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;	    
		break;	
		
		case RES_720P24HZ:
		case RES_3D_720P24HZ:	 
		case RES_3D_720P24HZ_TB:
		case RES_3D_720P24HZ_SBS_HALF:
		pe_resolution_info->ui4_h_total = 3300;
		pe_resolution_info->ui4_v_total = 750;
		pe_resolution_info->ui4_h_active = 1280;
		pe_resolution_info->ui4_v_activel = 720;
		pe_resolution_info->u4_framerate = 24;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;	    
		break;
		
		case RES_720P23HZ:
		case RES_3D_720P23HZ:	 
		case RES_3D_720P23HZ_TB:
		case RES_3D_720P23HZ_SBS_HALF:
		pe_resolution_info->ui4_h_total = 3300;
		pe_resolution_info->ui4_v_total = 750;
		pe_resolution_info->ui4_h_active = 1280;
		pe_resolution_info->ui4_v_activel = 720;
		pe_resolution_info->u4_framerate = 23;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_1080I60HZ:
		case RES_3D_1080I30HZ:	
		case RES_3D_1080I30HZ_TB:
		case RES_3D_1080I30HZ_SBS_HALF:	   	
		case RES_3D_1080I60HZ:	   	
		case RES_3D_1080I60HZ_TB:
		case RES_3D_1080I60HZ_SBS_HALF: 
		pe_resolution_info->ui4_h_total = 2200;
		pe_resolution_info->ui4_v_total = 1125;
		pe_resolution_info->ui4_h_active = 1920;
		pe_resolution_info->ui4_v_activel = 1080;
		pe_resolution_info->u4_framerate = 60;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = FALSE;
		break;
		
		case RES_1080I50HZ:
		case RES_3D_1080I25HZ:	   
		case RES_3D_1080I25HZ_TB:
		case RES_3D_1080I25HZ_SBS_HALF:	   	
		case RES_3D_1080I50HZ:	   	
		case RES_3D_1080I50HZ_TB:
		case RES_3D_1080I50HZ_SBS_HALF: 
		pe_resolution_info->ui4_h_total = 2640;
		pe_resolution_info->ui4_v_total = 1125;
		pe_resolution_info->ui4_h_active = 1920;
		pe_resolution_info->ui4_v_activel = 1080;
		pe_resolution_info->u4_framerate = 50;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = FALSE;
		break;
		
		case RES_1080P60HZ:
		case RES_3D_1080P60HZ:	   	
		case RES_3D_1080P60HZ_TB:
		case RES_3D_1080P60HZ_SBS_HALF:		
		pe_resolution_info->ui4_h_total = 2200;
		pe_resolution_info->ui4_v_total = 1125;
		pe_resolution_info->ui4_h_active = 1920;
		pe_resolution_info->ui4_v_activel = 1080;
		pe_resolution_info->u4_framerate = 60;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;	
		
		case RES_1080P50HZ:
		case RES_3D_1080P50HZ:	   	
		case RES_3D_1080P50HZ_TB:
		case RES_3D_1080P50HZ_SBS_HALF:	
		pe_resolution_info->ui4_h_total = 2640;
		pe_resolution_info->ui4_v_total = 1125;
		pe_resolution_info->ui4_h_active = 1920;
		pe_resolution_info->ui4_v_activel = 1080;
		pe_resolution_info->u4_framerate = 50;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_1080P30HZ:
		case RES_1080P29_97HZ:	 
		case RES_3D_1080P30HZ:	   	
		case RES_3D_1080P30HZ_TB:
		case RES_3D_1080P30HZ_SBS_HALF:		
		pe_resolution_info->ui4_h_total = 2200;
		pe_resolution_info->ui4_v_total = 1125;
		pe_resolution_info->ui4_h_active = 1920;
		pe_resolution_info->ui4_v_activel = 1080;
		pe_resolution_info->u4_framerate = 30;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_1080P25HZ:
		case RES_3D_1080P25HZ:	   	
		case RES_3D_1080P25HZ_TB:
		case RES_3D_1080P25HZ_SBS_HALF:		
		pe_resolution_info->ui4_h_total = 2640;
		pe_resolution_info->ui4_v_total = 1125;
		pe_resolution_info->ui4_h_active = 1920;
		pe_resolution_info->ui4_v_activel = 1080;
		pe_resolution_info->u4_framerate = 25;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_1080P24HZ:
		case RES_3D_1080P24HZ:	   	
		case RES_3D_1080P24HZ_TB: 
		case RES_3D_1080P24HZ_SBS_HALF: 
		pe_resolution_info->ui4_h_total = 2750;
		pe_resolution_info->ui4_v_total = 1125;
		pe_resolution_info->ui4_h_active = 1920;
		pe_resolution_info->ui4_v_activel = 1080;
		pe_resolution_info->u4_framerate = 24;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		case RES_1080P23_976HZ:
		case RES_3D_1080P23HZ:	   	
		case RES_3D_1080P23HZ_TB: 
		case RES_3D_1080P23HZ_SBS_HALF: 	   	
		pe_resolution_info->ui4_h_total = 2750;
		pe_resolution_info->ui4_v_total = 1125;
		pe_resolution_info->ui4_h_active = 1920;
		pe_resolution_info->ui4_v_activel = 1080;
		pe_resolution_info->u4_framerate = 23;
		pe_resolution_info->b_is_3d = (u13DModeType>0)?TRUE:FALSE;   // = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;

		
		case RES_PANEL_AUO_B089AW01:
		case RES_MODE_NUM:
		case RES_AUTO:
		pe_resolution_info->ui4_h_total = 5555;
		pe_resolution_info->ui4_v_total = 5555;
		pe_resolution_info->ui4_h_active = 5555;
		pe_resolution_info->ui4_v_activel = 5555;
		pe_resolution_info->u4_framerate = 5555;
		pe_resolution_info->b_is_3d = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
		
		default:
		pe_resolution_info->ui4_h_total = 5555;
		pe_resolution_info->ui4_v_total = 5555;
		pe_resolution_info->ui4_h_active = 5555;
		pe_resolution_info->ui4_v_activel = 5555;
		pe_resolution_info->u4_framerate = 5555;
		pe_resolution_info->b_is_3d = FALSE;
		pe_resolution_info->b_is_progressive = TRUE;
		break;
	}
}

void vAspInfoToUserSpace(VID_VDOIN_SRC_ASPECT_RATIO_T *pe_aspect_ratio, SOURCE_ASPECT_RATIO_T  eAspectRatio)
{

    switch (eAspectRatio)
    {
       case SRC_ASP_UNKNOW:
          *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_UNKNOWN;
       break;
       case SRC_ASP_1_1:
           *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_1_1;
       break;
       case SRC_ASP_4_3_FULL:
           *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_4_3_P_S;
       break;
       case SRC_ASP_14_9_LB:
       case SRC_ASP_14_9_LB_T:
       case SRC_ASP_16_9_LB:
       case SRC_ASP_16_9_LB_T:
       case SRC_ASP_16_9_LB_G:
          *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_16_9_L_B;
       break;
       case SRC_ASP_14_9_FULL:    
       case SRC_ASP_16_9_FULL:
          *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_16_9_FULL;
       break;
       case SRC_ASP_221_1:
          *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_2_21_1;
       break;
       case SRC_ASP_16_9_PS:
           *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_16_9_P_S;
       break;
       case SRC_ASP_UNDEFINED:
           *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_UNDEFINED;
       break;
       case SRC_ASP_CUSTOMIZED:
           *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_CUSTOMIZED;
       break;
       case SRC_ASP_MAX:
           *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_UNKNOWN;
       break;
	   default:
		   *pe_aspect_ratio = VID_VIDIN_SRC_ASPECT_RATIO_UNKNOWN;
	   break;
    }
}

void vVinSetPBType(VIN_PB_TYPE_T rPBType)
{
    switch(rPBType)
    {
        case VIN_PB_TYPE_START_CMD:
			_rVinCtrlInfo.pb_Type = VIN_PB_TYPE_START_CMD;
            vVinProcess(VIN_PB_TYPE_START_CMD);
            break;
            
        case VIN_PB_TYPE_STOP_CMD:
			_rVinCtrlInfo.pb_Type = VIN_PB_TYPE_STOP_CMD;
            vVinProcess(VIN_PB_TYPE_STOP_CMD);
            break;     

        default:
            break;
    }
}

BOOL fgSetVswLockDevice(VIDEO_IN_SRC_ID_E eDeviceId)
{
    if(g_rDeviceCap[eDeviceId].fgIsTimingOk)
    {
        return TRUE;
    }
    return FALSE;
}
// driver write
// app read
// app send
/*
** Description: This function performs to request the information of the buffer that driver wants
** 
** Note: Now only the max buffer count is checked
*/
int vRequestBuffer(VIDEO_IN_REQBUF *pRequestBuffer) {
	int i = 0;
	if (NULL == pRequestBuffer) {
		printk("VinBufQueue, Argument is NULL in %s()\n", __FUNCTION__);
		// Return error
		return -1;
	}

	ENTER_FUNC();

    // Clear Buffer queue
    vReset();
	
	// tell user the max buffer count that driver wants
	if (pRequestBuffer->u4BufCount > BUF_COUNT) {
		pRequestBuffer->u4BufCount = BUF_COUNT;
	}

	rQueue.maxCount = pRequestBuffer->u4BufCount;

	printQueueInfo();
	for (i = 0; i < rQueue.maxCount; i++) {
		printAddrInfo(i);
	}
	
	EXIT_FUNC();
	return 0;
}

int vInitBuffer(VIDEO_IN_BUFFER_INFO *pVideoInBuffer) {
	int i = 0;
    unsigned int u4KernelYAddr = 0;
    unsigned int u4KernelCAddr = 0;
    unsigned int u4KernelYAddrSize = 0;
    unsigned int u4KernelCAddrSize = 0;
	if (NULL == pVideoInBuffer || 0 == rQueue.maxCount) {
		// Return error
		printk("VinBufQueue, Videoin Buffer queue isn't initialized in %s()\n", __FUNCTION__);
		return -1;
	}
	ENTER_FUNC();
	if (pVideoInBuffer->u4BufIndex >= rQueue.maxCount) {
		// Return error
		printk("VinBufQueue, index(%d) is out of maxCount(%d)\n in %s()\n", pVideoInBuffer->u4BufIndex, rQueue.maxCount, __FUNCTION__);
		return -1;
	} else {
		printk("VinBufQueue, Set the addrInfo to %d element in queue in %s()\n", rQueue.totalCount, __FUNCTION__);
		#if 0 // change user mva address to kernel va address
        m4u_do_mva_map_kernel(pVideoInBuffer->vAddrInfo.u4YAddr, pVideoInBuffer->vAddrInfo.u4YBufferSize, \
            0, &u4KernelYAddr, &u4KernelYAddrSize);
        if (0 == u4KernelYAddr || 0 == u4KernelYAddrSize) {
            printk("VinBufQueue, Failed to convert Ymva(0x%x) to kernel Ymva(0x%x)\n", \
                pVideoInBuffer->vAddrInfo.u4YAddr, \
                u4KernelYAddr);
        }
        m4u_do_mva_map_kernel(pVideoInBuffer->vAddrInfo.u4CAddr, pVideoInBuffer->vAddrInfo.u4CBufferSize, \
            0, &u4KernelCAddr, &u4KernelCAddrSize);
        if (0 == u4KernelCAddrSize || 0 == u4KernelCAddr) {
            printk("VinBufQueue, Failed to convert Cmva(0x%x) to kernel Cmva(0x%x)\n", \
                pVideoInBuffer->vAddrInfo.u4CAddr, \
                u4KernelCAddr);
        }

        printk("VinBufQueue, convert Ymva(0x%x) to kernel Ymva(0x%x)\n", pVideoInBuffer->vAddrInfo.u4YAddr, u4KernelYAddr);
        printk("VinBufQueue, convert Cmva(0x%x) to kernel Cmva(0x%x)\n", pVideoInBuffer->vAddrInfo.u4CAddr, u4KernelCAddr);
        #endif
		memcpy(&(rQueue.rFBAddrInfo[rQueue.totalCount++]), &(pVideoInBuffer->vAddrInfo), sizeof(VIDEO_IN_ADDR));
		memcpy(&(rQueue_backup.rFBAddrInfo[rQueue_backup.totalCount++]), &(pVideoInBuffer->vAddrInfo), sizeof(VIDEO_IN_ADDR));
        //if (1 == rQueue.totalCount) {
           // vHal_VinSetBufPtr(&rQueue.rFBAddrInfo[0]);
        //}
	}

	printQueueInfo();
	for (i = 0; i < rQueue.totalCount; i++) {
		printAddrInfo(i);
	}

	EXIT_FUNC();

	return 0;
}

int vStreamOn() {

	ENTER_FUNC();
	if (0 == rQueue.totalCount ){
		// Return error
		printk("VinBufQueue, Videoin Buffer queue isn't initialized in %s()\n", __FUNCTION__);
		return -1;
	} else if (1 == _bVideoInStart) {
		printk("VinBufQueue, Videoin is already on in %s()\n", __FUNCTION__);
    } else {
        /*normal mode ok case */
    }
    _bVideoInStart = 1;
	EXIT_FUNC();
    
    return 0;
}

int vStreamOff() {

	ENTER_FUNC();
	if (0 == rQueue.totalCount ){
		// Return error
		printk("VinBufQueue, Videoin Buffer queue isn't initialized in %s()\n", __FUNCTION__);
		return -1;
	} else if (1 != _bVideoInStart) {
		printk("VinBufQueue, Videoin isn't on in %s()\n", __FUNCTION__);
    } else {
        /*normal mode ok case */
    }
    
    _bVideoInStart = 0;
	EXIT_FUNC();
    
    return 0;
}

/*
** Description: This function performs to release the buffer pointed by idx
** 
** Parameters:
**	idx: index of the buffer that should be released
*/
int vQBuf(UINT8 idx) {
	int i = 0;

	ENTER_FUNC();
	
    if (_bVideoInStart != 1) {
		// Return error
		printk("VinBufQueue, STREAMON command isn't called by userspace in %s()\n", __FUNCTION__);
		return -1;
    }
    
	if (rQueue.totalCount == 0) {
		// Return error, maybe this queue isn't initialized
		printk("VinBufQueue, Videoin Buffer queue isn't initialized in %s()\n", __FUNCTION__);
		return -1;
	} else if (idx != rQueue.readIndex) {
		// This buffer pointed by rQueue.readIndex isn't consumed
		// Should warn user or return error
		printk("VinBufQueue, the index(%d) doesn't match the readindex(%d) in queue in %s()\n", idx, rQueue.readIndex, __FUNCTION__);
		return -1;
	} else {
		rQueue.readIndex++;
		rQueue.usedCount--;
		rQueue.readIndex %= rQueue.totalCount;
		printk("rQueue.readIndex  the index(%d) in %s(%d)\n", rQueue.readIndex, __FUNCTION__,__LINE__);

		if (0 == rQueue.usedCount) {
			// Buffer queue is empty, should notify this event to user space if you need
			printk("VinBufQueue, Videoin Buffer queue is empty in %s(), should inform user\n", __FUNCTION__);
		}
	}

	//printQueueInfo();
	//for (i = 0; i < rQueue.totalCount; i++) {
	//	printAddrInfo(i);
	//}
	EXIT_FUNC();

	return 0;
}

/*
** Description: This function performs to retrive the buffer from the buffer queue
*/
int vDQBuf(VIDEO_IN_BUFFER_INFO *pVideoInBuffer) {
	int i = 0;

	ENTER_FUNC();
    if (_bVideoInStart != 1) {
		// Return error
		printk("VinBufQueue, STREAMON command isn't called by userspace in %s()\n", __FUNCTION__);
		return -1;
    }
    
	if (NULL == pVideoInBuffer || 0 == rQueue.totalCount) {
		// Return error
		printk("VinBufQueue, Videoin Buffer queue isn't initialized in %s()\n", __FUNCTION__);
		return -1;
	}
	memset(pVideoInBuffer, 0, sizeof(VIDEO_IN_BUFFER_INFO));
	if ( rQueue.usedCount <= 1) {
		// -1 means this buffer queue is empty
		printk("VinBufQueue, Videoin Buffer queue is empty in %s(), usedCount=%d\n", __FUNCTION__, rQueue.usedCount);
		pVideoInBuffer->u4BufIndex = -1;
	} else {
	    printk("VinBufQueue, Copy %d addrInfo in Videoin Buffer Queue to user space in %s()\n",rQueue.readIndex, __FUNCTION__);
		memcpy(&(pVideoInBuffer->vAddrInfo), &(rQueue.rFBAddrInfo[rQueue.readIndex]), sizeof(VIDEO_IN_ADDR));
		pVideoInBuffer->u4BufIndex = rQueue.readIndex;
	}
	EXIT_FUNC();

	return 0;
}

/*
** Description: This function is called by interrupt handler
*/
int vFillBuf() {
	int i = 0;
	
    if (_bVideoInStart != 1) {
		// Return error
		//printk("VinBufQueue, STREAMON command isn't called by userspace in %s()\n", __FUNCTION__);
		return -1;
    }
    
	if (0 == rQueue.totalCount ){
		// Return error
		//printk("VinBufQueue, Videoin Buffer queue isn't initialized in %s()\n", __FUNCTION__);
		return -1;
	}

	//ENTER_FUNC();
	if (rQueue.usedCount >= rQueue.totalCount) {
		// Queue is over
		// Should inform this event to user space
	   // printk("VinBufQueue, queue is over, usedCount=%d, totalCount=%d\n", rQueue.usedCount, rQueue.totalCount);
		return -1;
	} else {
	    VIDEO_IN_LOG("VinBufQueue, Fill %d buffer\n", rQueue.writeIndex);
	    if (rQueue.rFBAddrInfo[rQueue.writeIndex].u4CAddr != rQueue_backup.rFBAddrInfo[rQueue.writeIndex].u4CAddr
			|| rQueue.rFBAddrInfo[rQueue.writeIndex].u4YAddr != rQueue_backup.rFBAddrInfo[rQueue.writeIndex].u4YAddr) {
			printk("VinBufQueue, Fill %d buffer error(0x%x, 0x%x) != (0x%x, 0x%x)\n", \
				rQueue.writeIndex, \
				rQueue.rFBAddrInfo[rQueue.writeIndex].u4YAddr, \
				rQueue.rFBAddrInfo[rQueue.writeIndex].u4CAddr, \
				rQueue_backup.rFBAddrInfo[rQueue.writeIndex].u4YAddr, \
				rQueue_backup.rFBAddrInfo[rQueue.writeIndex].u4CAddr);
			return -1;
		}
		vHal_VinSetBufPtr(&rQueue.rFBAddrInfo[rQueue.writeIndex]);
		rQueue.writeIndex++;
		rQueue.writeIndex %= rQueue.totalCount;
		rQueue.usedCount++;
	}
/*
	printQueueInfo();
	for (i = 0; i < rQueue.totalCount; i++) {
		printAddrInfo(i);
	}*/
	//EXIT_FUNC();

	return 0;
}

int vReset() {
    vStreamOff();
    memset(&rQueue, 0, sizeof(rQueue));
    
    return 0;
}

void vShowvideoinstatus(void)
{
    int i = 0;
	printQueueInfo();
	for (i = 0; i < rQueue.totalCount; i++) {
		printAddrInfo(i);
	}
}
