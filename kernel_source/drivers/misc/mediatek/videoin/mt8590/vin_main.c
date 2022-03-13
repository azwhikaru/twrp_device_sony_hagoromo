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
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/byteorder/generic.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include "mach/irqs.h"
#include "vin_hal.h"
#include "vin_main.h"
#include "typedef.h"
#include "vin_drv_if.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"
#include "hal_io.h"
#include "vin_hw.h"


#define VDDIN_DELAY_DIV      1 

static struct task_struct *videoin_main_task = NULL;
atomic_t videoin_main_event = ATOMIC_INIT(0);
wait_queue_head_t videoin_timer_wq;
atomic_t videoin_timer_event = ATOMIC_INIT(0);

static cBYTE _ucVinInitiated = 0;
static cBYTE _aucVinThreadDestroy;
static UINT32 _u4VinDelayCnt;
extern VIN_CTRL_INFO_T _rVinCtrlInfo;
static VIN_FB_INFO_T _rVinFBInfo;
unsigned char state_cur = 0xff;
unsigned char state_pre = 0xff;


BOOL fgVinUpdateBuff(void)
{
    BOOL fgRet = TRUE;
	VIDEO_IN_BUFFER_INFO VinBuf;
	if ((fgHal_VinIsTopField()&& (fgIsProgress(_rVinCtrlInfo.rVinCfg.eInputRes))) || //progressive
        (fgHal_VinIsTopField() && !fgIsProgress(_rVinCtrlInfo.rVinCfg.eInputRes) && fgIsSDInterlace(_rVinCtrlInfo.rVinCfg.eInputRes)) || //SD interlace
        (!fgHal_VinIsTopField() && !fgIsProgress(_rVinCtrlInfo.rVinCfg.eInputRes) && !fgIsSDInterlace(_rVinCtrlInfo.rVinCfg.eInputRes))) {
		if (vFillBuf(&VinBuf) == 0) {
			// can not get buffer ,return error
			vHal_VinEnableDramIf(TRUE);
			fgRet = TRUE;
		} else {
			fgRet = FALSE;
		}
	}
	return fgRet;
}
static void vVinHWStop(void)
{
    printk("enter %s\n",__FUNCTION__);
    vHal_VinSetEnable(FALSE);
}

static irqreturn_t videoin_Vsync_Isr(int irq, void *dev_id)
{
	int bufcnt = 0;
    bufcnt = bReadRegVIN(RW_VDI_HSCALE)&0xFFF;
	VIDEO_IN_LOG("vdoin_wrap_v_counter bufcnt = %d,_rVinCtrlInfo.pb_Type = %d\n",bufcnt,_rVinCtrlInfo.pb_Type);
	vclear_vin_irq();
    if(fgHal_VinIsTopField())
    	VIDEO_IN_LOG("Top field\n");
    else
    	VIDEO_IN_LOG("Bottom  field\n");
	
	if(_rVinCtrlInfo.pb_Type == VIN_PB_TYPE_START_CMD) {
	    if(fgVinUpdateBuff()) {
			//setevent to userspace
			state_cur = VIDEO_IN_SEND_BUFFER_TO_HW_NOTIFY;
    	} else {
			//vVinHWStop();
		}
	}
    atomic_set(&videoin_timer_event, 1);
    wake_up_interruptible(&videoin_timer_wq);
	
	return IRQ_HANDLED;
}

BOOL fgVinSetCfgInfo(INPUT_DEVICE_INFO_T rInfo)
{
    memcpy(&_rVinCtrlInfo.rVinCfg, &rInfo, sizeof(INPUT_DEVICE_INFO_T));

    if((rInfo.eInputRes >= RES_MODE_NUM) || 
        (rInfo.eInputMode == FMT_INVALID) ||
        (rInfo.ePinType == NON_USED) ||
        (rInfo.ePinType > YC444_36BIT) ||
        (rInfo.rVdoInWDramType.eVdoInDramFmt > VIN_422) ||
        (rInfo.rVdoInWDramType.eVdoInAddrMode > VIN_LINEAR) ||
        (rInfo.rVdoInWDramType.eVdoInSwapMode > VIN_SWAP_MODE_3))
    {
        printk("[VDOIN] Error paramenters!\n");
        return FALSE;
    }
    printk("rInfo.eInputRes = %d,rInfo.rVdoInWDramType.eVdoInDramFmt = %d,rInfo.rVdoInWDramType.eVdoInAddrMode = %d,rInfo.rVdoInWDramType.eVdoInSwapMode = %d\n" ,\
		   rInfo.eInputRes,rInfo.rVdoInWDramType.eVdoInDramFmt,rInfo.rVdoInWDramType.eVdoInAddrMode,rInfo.rVdoInWDramType.eVdoInSwapMode);
    vHal_VinSetDramMode(rInfo.rVdoInWDramType.eVdoInDramFmt,rInfo.rVdoInWDramType.eVdoInAddrMode,rInfo.rVdoInWDramType.eVdoInSwapMode );
    
    vHal_VinSetPara(rInfo.eInputRes, rInfo.rVdoInWDramType.eVdoInDramFmt, 
                    rInfo.eInputMode, rInfo.ePinType,rInfo.fgVgaIsCeaType);

    if(fgVideoIsNtsc(rInfo.eInputRes))
    {
        _rVinFBInfo.u1TvSystem = TVS_NTSC;
    }
    else
    {
        _rVinFBInfo.u1TvSystem = TVS_PAL;
    }

    if((rInfo.rVdoInWDramType.eVdoInDramFmt == VIN_422) 
        && (rInfo.rVdoInWDramType.eVdoInAddrMode == VIN_LINEAR))
    {
        _rVinFBInfo.eFBufType = FBT_422_RS;
    } 
    else if((rInfo.rVdoInWDramType.eVdoInDramFmt == VIN_420) 
        && (rInfo.rVdoInWDramType.eVdoInAddrMode == VIN_LINEAR))
    {
        _rVinFBInfo.eFBufType = FBT_420_RS;
    }    

    switch(rInfo.eInputRes)
    {
        case RES_480I:
        case RES_480I_2880:    
        case RES_1080I60HZ:   
        case RES_3D_1080I60HZ:  
        case RES_3D_1080I60HZ_TB:
        case RES_3D_1080I60HZ_SBS_HALF:    
			if(rInfo.fgNTSC60)
			{
              _rVinFBInfo.u4Duration = 3000;
              _rVinFBInfo.u2FrameRate = FRC_30;
			}
			else
			{
              _rVinFBInfo.u4Duration = 3003;
              _rVinFBInfo.u2FrameRate = FRC_29_97;
			}
            break;
	    case RES_1080P29_97HZ:
		case RES_3D_1080P29HZ:
	    case RES_3D_1080P29HZ_TB:
	    case RES_3D_1080P29HZ_SBS_HALF:
            _rVinFBInfo.u4Duration = 3003;
            _rVinFBInfo.u2FrameRate = FRC_29_97;
            break;
            
        case RES_576I:
        case RES_576I_2880:
        case RES_1080I50HZ:   
        case RES_3D_1080I50HZ:
        case RES_3D_1080I50HZ_TB:
        case RES_3D_1080I50HZ_SBS_HALF:
        case RES_1080P25HZ:  
	 	case RES_3D_1080P25HZ:  
        case RES_3D_1080P25HZ_TB:
        case RES_3D_1080P25HZ_SBS_HALF:
        case RES_720P25HZ:
        case RES_3D_720P25HZ:
        case RES_3D_720P25HZ_TB:
        case RES_3D_720P25HZ_SBS_HALF:
            _rVinFBInfo.u4Duration = 3600;
            _rVinFBInfo.u2FrameRate = FRC_25;
            break;        

        case RES_480P:
        case RES_480P_1440:    
        case RES_480P_2880: 
        case RES_720P60HZ:
        case RES_3D_720P60HZ:  
        case RES_3D_720P60HZ_TB:    
        case RES_3D_720P60HZ_SBS_HALF:
        case RES_1080P60HZ:
        case RES_3D_1080P60HZ:
        case RES_3D_1080P60HZ_TB:
        case RES_3D_1080P60HZ_SBS_HALF:
        case RES_2D_640x480HZ:
			if(rInfo.fgNTSC60)
			{
				_rVinFBInfo.u4Duration = (3000 >> 1);
				_rVinFBInfo.u2FrameRate = FRC_60;
			}
			else
			{
				_rVinFBInfo.u4Duration = (3003 >> 1);
				_rVinFBInfo.u2FrameRate = FRC_59_94;
			}
            break;  

        case RES_576P:
        case RES_576P_1440:    
        case RES_576P_2880:   
        case RES_720P50HZ:
        case RES_3D_720P50HZ:    
        case RES_3D_720P50HZ_TB:    
        case RES_3D_720P50HZ_SBS_HALF:
        case RES_1080P50HZ:
        case RES_3D_1080P50HZ:
        case RES_3D_1080P50HZ_TB:
        case RES_3D_1080P50HZ_SBS_HALF:
            _rVinFBInfo.u4Duration = (3600 >> 1);
            _rVinFBInfo.u2FrameRate = FRC_50;
            break;      

        case RES_1080P30HZ:     
		case RES_3D_1080P30HZ:
	    case RES_3D_1080P30HZ_TB:
	    case RES_3D_1080P30HZ_SBS_HALF:
            _rVinFBInfo.u4Duration = 3000;
            _rVinFBInfo.u2FrameRate = FRC_30;
            break;      
        case RES_720P30HZ:
	    case RES_3D_720P30HZ:
	    case RES_3D_720P30HZ_TB:
	    case RES_3D_720P30HZ_SBS_HALF:
			if(rInfo.fgNTSC60)
			{
	            _rVinFBInfo.u4Duration = 3000;
	            _rVinFBInfo.u2FrameRate = FRC_30;
			}
			else
			{
	            _rVinFBInfo.u4Duration = 3003;
	            _rVinFBInfo.u2FrameRate = FRC_29_97;
			}
            break;      

        case RES_1080P24HZ: 
        case RES_3D_1080P24HZ:    
        case RES_3D_1080P24HZ_TB:
        case RES_3D_1080P24HZ_SBS_HALF:
        case RES_720P24HZ:
        case RES_3D_720P24HZ:
        case RES_3D_720P24HZ_TB:
        case RES_3D_720P24HZ_SBS_HALF:
            _rVinFBInfo.u4Duration = 3750;
            _rVinFBInfo.u2FrameRate = FRC_24;
            break;      

        case RES_1080P23_976HZ:     
        case RES_3D_1080P23HZ:    
        case RES_3D_1080P23HZ_TB:    
        case RES_3D_1080P23HZ_SBS_HALF:
        case RES_720P23HZ:
        case RES_3D_720P23HZ:
        case RES_3D_720P23HZ_TB:
        case RES_3D_720P23HZ_SBS_HALF:
            _rVinFBInfo.u4Duration = 3753;
            _rVinFBInfo.u2FrameRate = FRC_23_976;
            break;  
			
        default:
            break;
    }

    switch(rInfo.eVdoInAR)
    {
        case SRC_ASP_UNKNOW:
        case SRC_ASP_UNDEFINED:       
            _rVinFBInfo.eAspectRatio = SRC_ASP_16_9_FULL;
            break;            

        default:     
            _rVinFBInfo.eAspectRatio = rInfo.eVdoInAR;
            break;
    }

    _rVinFBInfo.fgIs3DVideo = fgIs3DFPOutput(rInfo.eInputRes) ? TRUE : FALSE;
    if(fgIs24HzFrameRate(rInfo.eInputRes))
    {
        _u4VinDelayCnt = 24 >> VDDIN_DELAY_DIV;
    }
    else if(fgIs25HzFrameRate(rInfo.eInputRes))
    {
        _u4VinDelayCnt = 25 >> VDDIN_DELAY_DIV;
    }
    else if(fgIs30HzFrameRate(rInfo.eInputRes))
    {
        _u4VinDelayCnt = 30 >> VDDIN_DELAY_DIV;
    }
    else if(fgIs48HzFrameRate(rInfo.eInputRes))
    {
        _u4VinDelayCnt = 48 >> VDDIN_DELAY_DIV;
    }
    else if(fgIs50HzFrameRate(rInfo.eInputRes))
    {
        _u4VinDelayCnt = 50 >> VDDIN_DELAY_DIV;
    }
    else if(fgIs60HzFrameRate(rInfo.eInputRes))
    {
        _u4VinDelayCnt = 60 >> VDDIN_DELAY_DIV;
    }
    else
    {
        _u4VinDelayCnt = 60 >> VDDIN_DELAY_DIV;
    }
    
    return TRUE;
}


void vVinProcess(VIN_PB_TYPE_T pbCmdType)
{
    switch(pbCmdType)
    {
        case VIN_PB_TYPE_START_CMD:
			printk("[enter %s]\n",__FUNCTION__);
            vhal_vin_clk_enable(TRUE);
			vHal_VinSetEnable(TRUE); //Enable HW
            break;
            
        case VIN_PB_TYPE_STOP_CMD:
			printk("[enter %s]\n",__FUNCTION__);
            vVinHWStop();   // stop Vsync interrupt
            vhal_vin_clk_enable(FALSE);
            vRstVariable(); //reset variable
            break;  
			
        default:    
            break;
    }

}

static int videoin_tmr_kthread(void *data)
{
	struct sched_param param = { .sched_priority = RTPM_PRIO_CAMERA_PREVIEW };
	sched_setscheduler(current, SCHED_RR, &param);
	
	for( ;; ) {
		wait_event_interruptible(videoin_timer_wq, atomic_read(&videoin_timer_event));
		atomic_set(&videoin_timer_event,0);
		if(state_cur!=0xff)
			vNotifyAppState(state_cur);
		msleep(20);
        if (kthread_should_stop())
            break;
	}
	return 0;
}

INT32 i4Videoin_Exit(void)
{
    _aucVinThreadDestroy = 1;
    _ucVinInitiated = 0;
    vVinHWStop();
    return 0;
}

INT32 i4Vin_Init(void)
{
	if(_ucVinInitiated == 0)
	{
		vRstVariable();
		_ucVinInitiated = 1;
		printk("enter %s @L%d _ucVinInitiated = %d\n",__FUNCTION__,__LINE__,_ucVinInitiated);
		vHal_VinSetEnable(FALSE);
		if (request_irq(MT_VIN_IRQ_ID,videoin_Vsync_Isr, IRQF_TRIGGER_LOW, "mtkvideoin", NULL) < 0) {
			return -1;
		}
		init_waitqueue_head(&videoin_timer_wq);
		videoin_main_task = kthread_create(videoin_tmr_kthread, NULL, "hdmirx_timer_kthread"); 
		wake_up_process(videoin_main_task);
	}
	else
	{
		return -1;
	}

	return 0;
}


INT32 i4VinUnInit(void)
{
    _ucVinInitiated = 0;
	vVinHWStop();	
    return 0;
}

void vRstVariable(void)
{
    memset (&_rVinFBInfo, 0, sizeof (VIN_FB_INFO_T));
    memset (&_rVinCtrlInfo, 0, sizeof (VIN_CTRL_INFO_T));
	memset(&rQueue, 0, sizeof(_VIN_QUEUE));
	memset(&rQueue_backup, 0, sizeof(_VIN_QUEUE));
}

BOOL fgIsVinInitDone(void)
{   
    return (_ucVinInitiated > 0) ? TRUE : FALSE;
}

