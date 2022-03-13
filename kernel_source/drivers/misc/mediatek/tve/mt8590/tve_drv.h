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
#ifndef _TVE_DRV_H_
#define _TVE_DRV_H_

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/kernel.h>
#else
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include "tve_common.h"
#include "tve_ctrl.h"
#include "mtkfb_info.h"



#define TVE_LOG(fmt, arg...) \
    do { \
        if (tve_log_on) {printk("[cvbs] %s #%d "fmt,__func__, __LINE__, ##arg); } \
    }while (0)
#define TVE_KTHREAD_LOG(fmt, arg...) \
    do { \
        if (tve_kthread_log_on) {printk("[cvbs_kthread]#%d "fmt, __LINE__, ##arg); } \
    }while (0)
#define TVE_FUNC()    \
    do { \
        /*if(tve_log_on)*/ printk("[cvbs] %s\n", __func__); \
    }while (0)

#define TVE_LINE()    \
    do { \
        if (tve_log_on) {printk("[cvbs] %s,%d ", __func__, __LINE__); } \
    }while (0)

#define TVE_MSG(fmt, arg...) \
    do { \
        {printk("[cvbs] %s #%d "fmt,__func__, __LINE__,##arg); } \
    }while (0)
    
#define TVE_ERR(fmt, arg...) \
    do { \
        {printk("[cvbs] %s #%d error #%d "fmt,__func__, __LINE__,##arg); } \
    }while (0)
	
typedef struct
{
    void *src_base_addr;
    void *src_phy_addr;
    int src_fmt;
    unsigned int  src_pitch;
    unsigned int  src_offset_x, src_offset_y;
    unsigned int  src_width, src_height;

    int next_buff_idx;
    int identity;
    int connected_type;
    unsigned int security;
    int fenceFd;
} tve_video_buffer_info;



typedef struct
{
    //  Input
    int ion_fd;
    // Output
    unsigned int index; //fence count
    int fence_fd;   //fence fd
} tve_buffer_info;

typedef enum
{
    create_new,
    insert_new,
    reg_configed,
    reg_updated,
    buf_read_done,
    buf_dropped
} tve_buffer_sate;


//tve  start: power on-->setfmt

#define TVE_IO_MAGIC   'T'

#define TVE_IOW(num, dtype)     _IOW(TVE_IO_MAGIC, num, dtype)
#define TVE_IOR(num, dtype)     _IOR(TVE_IO_MAGIC, num, dtype)
#define TVE_IOWR(num, dtype)    _IOWR(TVE_IO_MAGIC, num, dtype)
#define TVE_IO(num)             _IO(TVE_IO_MAGIC, num)


#define CMD_TVE_SET_FORMAT      _IOW(TVE_IO_MAGIC, 0, int)   
#define CMD_TVE_SET_LOG_ON      _IOW(TVE_IO_MAGIC, 1, int)   
#define CMD_TVE_GET_STATUS      _IOW(TVE_IO_MAGIC, 2, int)   
#define CMD_TVE_SET_COLORBAR    _IOW(TVE_IO_MAGIC, 3, int)   
#define CMD_TVE_SET_BRIGHTNESS  _IOW(TVE_IO_MAGIC, 4, int)   
#define CMD_TVE_GET_UNIFY       _IOW(TVE_IO_MAGIC, 5, int) 
#define CMD_TVE_GET_DEVNAME     _IOR(TVE_IO_MAGIC, 6, char)
#define CMD_TVE_SET_ENABLE      _IOW(TVE_IO_MAGIC, 7, int)  //just for enable dac and encoder  
#define CMD_TVE_SET_TVE_POWER_ON   _IOW(TVE_IO_MAGIC, 8, int)
#define CMD_TVE_SET_RESUME      _IOW(TVE_IO_MAGIC, 9, int)
#define CMD_TVE_SET_SUSPEND     _IOW(TVE_IO_MAGIC, 10, int)
#define CMD_TVE_GET_TVE_STATUS    _IOR(TVE_IO_MAGIC, 11, TVE_STATUS_CONF_T)
#define CMD_TVE_SET_DPI0_CB    _IOW(TVE_IO_MAGIC, 12, int) 
#define CMD_TVE_SET_MV        _IOW(TVE_IO_MAGIC, 13, int) 
#define CMD_TVE_GET_FORMAT     _IOR(TVE_IO_MAGIC, 14, char)
#define CMD_TVE_SET_ASPECT     _IOW(TVE_IO_MAGIC, 15, int)
#define CMD_TVE_SET_CPS        _IOW(TVE_IO_MAGIC, 16, IBC_CpsCommonInfoParamsDef)

#define CMD_TVE_POST_VIDEO_BUFFER              TVE_IOW(20,  tve_video_buffer_info)
#define CMD_TVE_GET_DEV_INFO                   TVE_IOWR(35, mtk_dispif_info_t)
#define CMD_TVE_PREPARE_BUFFER                 TVE_IOW(36, tve_buffer_info)
#define CMD_TVE_POST_VIDEO_BUFFER_EX              TVE_IOW(37,  tve_video_buffer_info)


#define MTK_TVE_NO_FENCE_FD        ((int)(-1)) //((int)(~0U>>1))
#define MTK_TVE_NO_ION_FD        ((int)(-1))   //((int)(~0U>>1))

static char *tve_ioctl_spy(unsigned int cmd)
{
#if 1
    switch (cmd)
    {
      	case CMD_TVE_SET_FORMAT:
            return "CMD_TVE_SET_FORMAT";
			
      	case CMD_TVE_SET_LOG_ON:
        	return "CMD_TVE_SET_LOG_ON";

		case CMD_TVE_GET_STATUS:
			return "CMD_TVE_GET_STATUS";

		case CMD_TVE_SET_COLORBAR:
			return "CMD_TVE_SET_COLORBAR";

		case CMD_TVE_SET_BRIGHTNESS:
			return "CMD_TVE_SET_BRIGHTNESS";

		case CMD_TVE_GET_UNIFY:
			return "CMD_TVE_GET_UNIFY";

		case CMD_TVE_GET_DEVNAME:
			return "CMD_TVE_GET_DEVNAME";
			
		case CMD_TVE_SET_ENABLE:
			return "CMD_TVE_SET_ENABLE";
			
		case CMD_TVE_SET_TVE_POWER_ON:
			return "CMD_TVE_SET_TVE_POWER_ON";
			
		case CMD_TVE_SET_RESUME:
			return "CMD_TVE_SET_RESUME";
			
		case CMD_TVE_SET_SUSPEND:
			return "CMD_TVE_SET_SUSPEND";
		
		case CMD_TVE_GET_TVE_STATUS:
			return "CMD_TVE_GET_TVE_STATUS";
		
		case CMD_TVE_SET_DPI0_CB:
			return "CMD_TVE_SET_DPI0_CB";
		
		case CMD_TVE_SET_MV:
			return "CMD_TVE_SET_MV";
		
		case CMD_TVE_GET_FORMAT:
			return "CMD_TVE_GET_FORMAT";

		case CMD_TVE_SET_ASPECT:
			return "CMD_TVE_SET_ASPECT";

		case CMD_TVE_SET_CPS:
			return "CMD_TVE_SET_CPS";
			
        case CMD_TVE_POST_VIDEO_BUFFER:
            return "CMD_TVE_POST_VIDEO_BUFFER";

        case CMD_TVE_GET_DEV_INFO:
            return "CMD_TVE_GET_DEV_INFO";

        case CMD_TVE_PREPARE_BUFFER:
            return "CMD_TVE_PREPARE_BUFFER";        

        default:
            return "unknown ioctl command";
    }
	#endif
}


#define str_lenth 100
enum cvbs_report_state
{
    NO_DEVICE = 0,
    cvbs_PLUGIN = 1,
};

typedef enum{
	CVBS_STATE_NO_DEVICE,
	CVBS_STATE_ACTIVE,
	CVBS_STATE_PLUGIN_ONLY,
}CVBS_STATE;

#if 1
typedef struct
{
	unsigned int width;
	unsigned int height;

    /* timing parameters */
    unsigned int hsync_pulse_width;
    unsigned int hsync_back_porch;
    unsigned int hsync_front_porch;
    unsigned int vsync_pulse_width;
    unsigned int vsync_back_porch;
    unsigned int vsync_front_porch;
    
    /* intermediate buffers parameters */
    unsigned int intermediat_buffer_num; // 2..3

    int is_force_awake;
    int is_force_landscape;

    unsigned int scaling_factor; // determine the scaling of output screen size, valid value 0~10
                                 // 0 means no scaling, 5 means scaling to 95%, 10 means 90%  
}CVBS_PARAMS;

typedef struct
{
    void (*set_reset_pin)(unsigned int value);
    int  (*set_gpio_out)(unsigned int gpio, unsigned int value);
    void (*udelay)(unsigned int us);
    void (*mdelay)(unsigned int ms);
    void (*wait_transfer_done)(void);
	void (*state_callback)(CVBS_STATE state);
}CVBS_UTIL_FUNCS;

typedef struct
{
    void (*set_util_funcs)(const CVBS_UTIL_FUNCS *util);
    void (*get_params)(CVBS_PARAMS *params);

    void (*init)(void);
    int (*enter)(void);
    int (*exit)(void);
    void (*suspend)(void);
    void (*resume)(void);
	void (*tve_enable)(unsigned short flag);
	TVE_STATUS_T (*tve_power)(UCHAR enable);
	TVE_STATUS_T (*tve_reset)(void);
	TVE_STATUS_T (*setbri)(UCHAR level);
	TVE_STATUS_T (*setcont)(UCHAR level);
	TVE_STATUS_T (*sethue)(UCHAR level);
	TVE_STATUS_T (*setsat)(UCHAR level);
    void (*dump)(void);
	void (*read)(unsigned int u2Reg, unsigned int *p4Data);
    void (*write)(unsigned int u2Reg, unsigned int u4Data);
	void (*device_name)(char* str);
    void (*log_enable)(unsigned short enable);
	void (*setfmt)(int fmt);
       void (*setinitfmt)(int fmt);
	void (*setDPI0CB)(unsigned short enable);
	void (*colorbar)(UCHAR enable);
        UINT32 (*setmv)(UCHAR ucAps);
	void (*getfmt)(char* str);
        UINT32 (*set_aspect)(TVE_ASPECT_RATIO_T index);
        UINT32 (*set_cps)(IBC_CpsCommonInfoParamsDef* cps);
#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
	void (*get_tve_status)(TVE_STATUS_CONF_T* tve_status);
    void (*cvbs_autodetect_init)(void);
#endif
} CVBS_DRIVER;
const CVBS_DRIVER* CVBS_GetDriver(void);
#endif
#endif
