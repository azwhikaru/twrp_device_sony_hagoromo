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
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>

#include "tve_common.h"
#include "tve_drv.h"

#if 1
static CVBS_UTIL_FUNCS cvbs_util = {0};
#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
static struct timer_list r_auto_detect_timer;
static uint32_t gCVBS_CHK_INTERVAL = 100; //1000;

static struct task_struct *cvbs_timer_task = NULL;
wait_queue_head_t cvbs_timer_wq;
atomic_t cvbs_timer_event = ATOMIC_INIT(0);

#define ANALOG_DAC_COUNTER_THRESHOD 3
#define ANALOG_DAC_STATUS 0x1
UINT32 _ucCurrentAnalogDACStatus = ANALOG_DAC_STATUS;
UINT32 _ucPreAnalogDACStatus = 0xFF;
UINT32 _ucRecAnalogDACStatus = ANALOG_DAC_STATUS;
UINT32 _ucAnalogDACStatusCounter = 0;

extern void TVE_GetPowerDacStatus(UCHAR ucTveId);
static int cvbs_timer_kthread(void *data);
#endif
extern UINT32 TVE_HalGetPowerDacStatus(void);
extern BOOL cvbs_auto_detect_enable;

static void cvbs_set_util_funcs(const CVBS_UTIL_FUNCS *util)
{
    memcpy(&cvbs_util, util, sizeof(CVBS_UTIL_FUNCS));
}
/*----------------------------------------------------------------------------*/

static void cvbs_get_params(CVBS_PARAMS *params)
{
    memset(params, 0, sizeof(CVBS_PARAMS));
    
   //add should init first parameter here
}

static void cvbs_devices_name(char*str)
{
  strcpy(str,CVBS_DRV);
  CVBS_LOG("str= %s \n",str);
}

static int cvbs_internal_enter(void)
{
 
    CVBS_DEF_LOG("[cvbs]cvbs_internal_enter\n");
    
    if(tve_get_auto_detect_status() == TRUE)
    {
        CVBS_DEF_LOG("[cvbs] auto detect enable\n");
        cvbs_auto_detect_enable = TRUE;
    }
    else
    {
        CVBS_DEF_LOG("[cvbs] auto detect disable\n");
        cvbs_auto_detect_enable = FALSE;
    }
    
#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
	init_waitqueue_head(&cvbs_timer_wq);
	cvbs_timer_task = kthread_create(cvbs_timer_kthread, NULL, "cvbs_timer_kthread"); 
	wake_up_process(cvbs_timer_task);
#endif	
	return 0;

}

static int cvbs_internal_exit(void)
{
   CVBS_FUNC();
#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
   	if(r_auto_detect_timer.function)
	{
	 del_timer_sync(&r_auto_detect_timer);
	}
	memset((void*)&r_auto_detect_timer, 0, sizeof(r_auto_detect_timer));
#endif
    return 0;
}

#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
void CVBS_polling(unsigned long n)
{
    atomic_set(&cvbs_timer_event, 1);
    wake_up_interruptible(&cvbs_timer_wq);
    mod_timer(&r_auto_detect_timer, jiffies + gCVBS_CHK_INTERVAL/(1000/HZ));

}
void cvbs_timer_impl(void)
{
   TVE_GetPowerDacStatus(TVE_2);
}

void timer_init(void)
{
	CVBS_FUNC();
	memset((void*)&r_auto_detect_timer, 0, sizeof(r_auto_detect_timer));
	r_auto_detect_timer.expires  = jiffies + 1000/(1000/HZ);   // wait 1s to stable
	r_auto_detect_timer.function = CVBS_polling;	 
	r_auto_detect_timer.data	 = 0;
	init_timer(&r_auto_detect_timer);
	add_timer(&r_auto_detect_timer);
}
static int cvbs_timer_kthread(void *data)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_CAMERA_PREVIEW };
    sched_setscheduler(current, SCHED_RR, &param);
    
    for( ;; ) {
        wait_event_interruptible(cvbs_timer_wq, atomic_read(&cvbs_timer_event));
	    atomic_set(&cvbs_timer_event,0);
        cvbs_timer_impl();
        if (kthread_should_stop())
            break;
    }

    return 0;
}

void TVE_GetPowerDacStatus(UCHAR ucTveId)
{
	UINT32 ucStatus;
	BOOL _fgNFYStatus = FALSE;

       if(tve_detect_is_pending() == TRUE)
       {
            CVBS_AUTO_DETECT_LOG("tve detect pending\n");
            return;
       }

	// here need to confirm 
	ucStatus = TVE_HalGetPowerDacStatus() & 0x1;	// ignore the Y/C status
	CVBS_AUTO_DETECT_LOG("ucStatus %d\n",ucStatus);
	if(ucStatus == _ucRecAnalogDACStatus)
	{
		_ucAnalogDACStatusCounter++;

		if(_ucAnalogDACStatusCounter > ANALOG_DAC_COUNTER_THRESHOD)
		{
			_ucCurrentAnalogDACStatus = ucStatus;
			
			_fgNFYStatus = TRUE;
		}
	}
	else
	{
		_ucRecAnalogDACStatus = ucStatus;
		_ucAnalogDACStatusCounter = 0;

		_fgNFYStatus = FALSE;
	}
		
	switch(ucTveId)
	{
		case TVE_1:
		case TVE_2:
			if(_fgNFYStatus)
			{				

				if(cvbs_util.state_callback!= NULL)
				{					
					if(_ucCurrentAnalogDACStatus != _ucPreAnalogDACStatus)
					{
						CVBS_DEF_LOG("[cvbs]PreStatus = 0x%x, CurrentStatus = 0x%x\n", _ucPreAnalogDACStatus, _ucCurrentAnalogDACStatus);
						cvbs_util.state_callback((_ucCurrentAnalogDACStatus ? CVBS_STATE_NO_DEVICE : CVBS_STATE_ACTIVE));
					}
				}				
				_ucPreAnalogDACStatus = _ucCurrentAnalogDACStatus;
			}
			break;

		default:
			break;
	}
}
#endif

const CVBS_DRIVER* CVBS_GetDriver(void)
{
	static const CVBS_DRIVER cvbs_drv =
	{
		.set_util_funcs   = cvbs_set_util_funcs,
		.get_params       = cvbs_get_params,
		.init             = cvbs_drv_init,
		.enter            = cvbs_internal_enter,
        .exit             = cvbs_internal_exit,
		.suspend          = NULL,
		.resume           = NULL,
		//.video_config	  = NULL,
		.tve_enable  	  = TveEnable,
		.tve_power		  = TVE_DACPower,
		.tve_reset		  = TVE_Reset,
		.setbri           = TVE_SetBrightness,
		.setcont          = TVE_SetContrast,
		.sethue           = TVE_SetHue,
		.setsat           = TVE_SetSaturation,
		.dump             = NULL,
		.read             = NULL,
		.write            = NULL,
		.device_name      = cvbs_devices_name,
		//.get_state        = NULL,
		.log_enable       = Tve_LogEnable,
		.setfmt 		  = TveSetFmt,
		.setinitfmt 		  = TveSetInitFmt,
		.setDPI0CB        = TveSetDPI0Colorbar,
		.colorbar         = TVE_SetColorBar,
		.setmv            = TVE_SetMacroVision,		
		.getfmt 		  = TveGetFmt,
		.set_aspect       = TVE_SetAspectRatio,
		.set_cps          = TVE_SetCps,  
		#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
		.get_tve_status   = TVE_GetTveStatus,
		.cvbs_autodetect_init = timer_init,
		#endif
	};	
	return &cvbs_drv;
}
EXPORT_SYMBOL(CVBS_GetDriver);
#endif
