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
/*****************************************************************************/
/* Copyright (c) 2009 NXP Semiconductors BV                                  */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation, using version 2 of the License.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307       */
/* USA.                                                                      */
/*                                                                           */
/*****************************************************************************/
//#include <linux/autoconf.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/vmalloc.h>
#include <linux/disp_assert_layer.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/switch.h>

#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/mach-types.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <mach/dma.h>
#include <mach/irqs.h>
#include <asm/tlbflush.h>
#include <asm/page.h>

#include <mach/m4u.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_boot.h>

#include "tve_common.h"
#include "tve_drv.h"

#include "ddp_hal.h"
#include "DpDataType.h"
#include "mtkfb_info.h"
#include "ddp_drv.h"
#include "ddp_reg.h"
#include "ddp_rdma.h"

#include "tve_dev.h"
#include "disp_drv_platform.h"
#include "mtkfb_info.h"
#include "dpi_drv.h"
#include "tve_fb_ctrl.h"
#include "mtkfb_priv.h"

#ifdef MTK_SEC_VIDEO_PATH_SUPPORT
#include "tz_cross/trustzone.h"
#include "tz_cross/ta_mem.h"
#include <tz_cross/tz_ddp.h>
#include "trustzone/kree/system.h"
#include "trustzone/kree/mem.h"
#endif


typedef struct
{
    tve_video_buffer_info buffer_info;
    tve_buffer_sate buf_state;

    unsigned int idx;   ///fence count
    int fence;          ///fence fd
    struct ion_handle *hnd;
    unsigned int mva;
    unsigned int va;
    struct list_head list;
#if DISPLAY_V4L2
    BUF_NOTIFY_CALLBACK buf_receive; // Used for V4L2
    BUF_NOTIFY_CALLBACK buf_remove;  // Used for V4L2
#endif
} tve_video_buffer_list;


static int tve_log_on = 0;
static int tve_kthread_log_on = 0;

static int tve_buffer_dump_on = 0;
static int tve_show_color_bar = 0;

#ifdef MTK_FOR_CVBS_DITHERING

unsigned tve_rdma1_read_buffer_index = 0;
unsigned tve_rdma1_write_buffer_index = 0;
unsigned tve_write_buffer_index = 0;
unsigned tve_read_buffer_index = 0;

int tve_only_UI_filter_on = 0;

int tve_filter_on = 0;

#define DISP_INDEX_OFFSET 0xa000

static struct task_struct *tve_filter_task = NULL;

static wait_queue_head_t tve_filter_wq;
static atomic_t tve_filter_event = ATOMIC_INIT(0);

#endif

static int tve_rdmafpscnt = 0;
static int tve_dp_mutex_dst = -1;

#define CVBS_PATH_MUTEX_ID 2

enum DP_COLOR_ENUM rdma1_src_fmt = RDMA1_INTPUT_FORMAT;

unsigned rdma1_src_height = 0;

extern void tve_debug_init(void);
extern UINT8 _ucCVBSOutFmt;

#define MTK_TVE_FENCE_SUPPORT

#ifdef MTK_TVE_FENCE_SUPPORT
// Fence Sync Object
#include "mtk_sync.h"
spinlock_t tve_lock;
DEFINE_SPINLOCK(tve_lock);

static unsigned int tve_get_fence_counter(void);

static struct sw_sync_timeline *tve_create_timeline(void);

static int tve_create_fence(int *pfence, unsigned int *pvalue);

static void tve_release_fence(void);

unsigned int tve_timeline_inc(void);

static void tve_signal_fence(void);

static void tve_sync_init(void);

static void tve_sync_destroy(void);

#endif

static struct task_struct *tve_rdma_config_task = NULL;
static struct task_struct *tve_rdma_update_task = NULL;

static struct list_head  tve_buffer_list;

static wait_queue_head_t tve_rdma_config_wq;
static atomic_t tve_rdma_config_event = ATOMIC_INIT(0);

static wait_queue_head_t tve_rdma_update_wq;
static atomic_t tve_rdma_update_event = ATOMIC_INIT(0);

static unsigned int tve_rdma1_addr_shadowed = NULL;
static unsigned int tve_rdma1_addr_using = NULL;

CVBS_STATE cvbs_connect_state = CVBS_STATE_NO_DEVICE;

static DEFINE_SEMAPHORE(tve_update_mutex);


#define REGISTER_WRITE32(u4Addr, u4Val)     (*((volatile unsigned int*)(u4Addr)) = (u4Val))
#define REGISTER_READ32(u4Addr)             (*((volatile unsigned int*)(u4Addr)))

#define CVBS_DEVNAME "cvbs"
static dev_t cvbs_devno;
static struct cdev *cvbs_cdev;
static struct class *cvbs_class = NULL;

CVBS_PARAMS _s_cvbs_params = {0};
CVBS_PARAMS *cvbs_params = &_s_cvbs_params;

static struct switch_dev cvbs_switch_data;
static struct switch_dev cvbs_reschg;
static CVBS_DRIVER *cvbs_drv = NULL;
static int cvbs_probe(struct platform_device *pdev);
static int cvbs_remove(struct platform_device *pdev);
static void CVBS_Suspend(struct early_suspend *h);
static void CVBS_Resume(struct early_suspend *h);

static unsigned int tve_va = 0;
static unsigned int tve_mva_r = 0;

unsigned tve_recovery_read_buffer_index = 0;
unsigned tve_recovery_write_buffer_index = 0;

static unsigned int tve_temp_mva = 0;
static unsigned int tve_temp_va = 0;

#ifdef MTK_FOR_CVBS_DITHERING
static unsigned int tve_rdma1_mva = 0;
static unsigned int tve_rdma1_va = 0;
#endif

unsigned cvbs_res = 1; // 0 480i,1 576i
#define CVBS_MAX_ERS 1
static unsigned int tve_res_param_table[][3] =
{
    {720,   480,    60}, // HDMI_VIDEO_720x480p_60Hz
    {720,   576,    50}
};

extern unsigned int fb_pa ;
int tve_rdma_address_config(bool enable, tve_video_buffer_info buffer_info);
int tve_allocate_rdma1_buffer(void);
int tve_free_rdma1_buffer(void);
int tve_rdma_address_config_ex(bool enable);
static void tve_rdma1_irq_handler(unsigned int param);
static int tve_rdma_config_kthread(void *data);
static int tve_rdma_update_kthread(void *data);

#ifdef MTK_FOR_CVBS_DITHERING
static int tve_filter_kthread(void *data);
#endif

extern int m4u_do_mva_map_kernel(unsigned int mva, unsigned int size, int sec,
                        unsigned int* map_va, unsigned int* map_size);
extern int m4u_do_mva_unmap_kernel(unsigned int mva, unsigned int size, unsigned int va);
extern void cvbs_recchg_nofity(int index);


#define CVBS_EARLY_SUSPEND_LEVEL 99
static struct platform_driver cvbs_driver =
{
    .probe  = cvbs_probe,
    .remove = cvbs_remove,
    //.suspend = CVBS_Suspend,
    //.resume = CVBS_Resume,
    .driver = { .name = CVBS_DEVNAME },
};
static struct early_suspend cvbs_early_suspend_handler =
{
    .level = CVBS_EARLY_SUSPEND_LEVEL,
    .suspend = CVBS_Suspend,
    .resume = CVBS_Resume,
};

static struct platform_device cvbs_parent_device = {

	.name	  = CVBS_DEVNAME,
	.id		  = 0,
};

void Assert(const char* szExpress, const char* szFile, int i4Line)
{
	printk("\nAssertion fails at:\nFile: %s, line %d\n\n", szFile, (int)(i4Line));
	printk("\t%s\n\n", szExpress);
}


static ssize_t cvbs_core_show_info(struct device *dev,
	                                         struct device_attribute *attr, char *buf)
{
	struct attribute *pattr = &(attr->attr);

	UINT32 len = 0;
	UINT32 temp_len = 0;
	u32 vregstart = 0xf4017600;

    CVBS_PRINTF("[cvbs]%s,%d \n", __func__, __LINE__);

	CVBS_PRINTF("[cvbs] %s \n",pattr->name);

  if (strcmp(pattr->name, "register") == 0) // here used to dump register
  {
	 CVBS_PRINTF("-------------dump ir register-----------\n");

	 for (vregstart = 0xf4017600; vregstart <=0xf4017700;)
	 {
		CVBS_PRINTF("TVE:0x%08x\n",REGISTER_READ32(vregstart));
		vregstart += 4;
	 }
  }

    return len;
}
extern int tve_test_cmd(int psInput);
static ssize_t cvbs_core_store_info(struct device *dev,
                                            struct device_attribute *attr,
		                                    const char *buf, size_t count)
{
    int val;
	UINT32 reg;
	//UINT32 len = 0;
	//UINT32 temp_len = 0;
	UINT32 reg_start,lenth,reg_index;

	struct attribute *pattr = &(attr->attr);
	printk("input cmd %s \n",pattr->name);
	if (strcmp(pattr->name, "test_cmd") == 0)
	{
		sscanf(buf, "%d", &val);
		printk("input cmd %s val= %d\n",pattr->name,val);
		tve_test_cmd(val);
	}
    if (strcmp(pattr->name, "read") == 0)
	{
		sscanf(buf, "%x %x", &reg_start , &lenth);
		printk("read start address:0x%08x lenth:0x%x  %x\n",reg_start,lenth,reg_start+lenth);
		for (reg_index = reg_start; reg_index <(reg_start+lenth);)
		{
		  printk("read:0x%08x = 0x%08x\n",reg_index,REGISTER_READ32(reg_index));
		  reg_index += 0x4;
		  if(reg_index >(reg_start+lenth))
		  	break;
		}
		//tve_test_cmd(val);
		return count;
	}
	if (strcmp(pattr->name, "write") == 0)
	{
		sscanf(buf, "%x %x", &reg , &val);
		printk("write reg(0x%08x) =  val(0x%08x)\n", reg, val);
		REGISTER_WRITE32(reg, val);
		printk("read:0x%08x = 0x%08x\n",reg,REGISTER_READ32(reg));
		//tve_test_cmd(val);
		return count;
	}
	if (strcmp(pattr->name, "register") == 0)
	{
		sscanf(buf, "%x %x", &reg , &val);
		printk("write reg(0x%08x) =  val(0x%08x)\n", reg, val);
		REGISTER_WRITE32(reg, val);
		printk("read  reg(0x%08x) =  val(0x%08x)\n", reg, REGISTER_READ32(reg));
		return count;
	}

  return count;
}


static DEVICE_ATTR(debug_log, 0664, cvbs_core_show_info, cvbs_core_store_info);

static DEVICE_ATTR(register, 0664, cvbs_core_show_info, cvbs_core_store_info);
static DEVICE_ATTR(register_clk, 0664, cvbs_core_show_info, cvbs_core_store_info);
static DEVICE_ATTR(register_dpi0, 0664, cvbs_core_show_info, cvbs_core_store_info);
static DEVICE_ATTR(register_hdmipll, 0664, cvbs_core_show_info, cvbs_core_store_info);
static DEVICE_ATTR(register_usbvdac, 0664, cvbs_core_show_info, cvbs_core_store_info);
static DEVICE_ATTR(register_selfclk, 0664, cvbs_core_show_info, cvbs_core_store_info);
static DEVICE_ATTR(register_mipiclk, 0664, cvbs_core_show_info, cvbs_core_store_info);

static DEVICE_ATTR(test_cmd, 0664, cvbs_core_show_info, cvbs_core_store_info);

static DEVICE_ATTR(read, 0664, cvbs_core_show_info, cvbs_core_store_info);

static DEVICE_ATTR(write, 0664, cvbs_core_show_info, cvbs_core_store_info);

static DEVICE_ATTR(clock, 0224, NULL, cvbs_core_store_info);


static struct device_attribute *cvbs_attr_list[] = {
	&dev_attr_debug_log,
	&dev_attr_register,
	&dev_attr_register_usbvdac,
	&dev_attr_register_clk,
	&dev_attr_register_selfclk,
	&dev_attr_register_dpi0,
	&dev_attr_register_hdmipll,
	&dev_attr_register_mipiclk,
	&dev_attr_test_cmd,
	&dev_attr_read,
	&dev_attr_write,
	&dev_attr_clock,
};

static char *mtk_fb_format_spy(unsigned int fmt)
{
#if 0
	MTK_FB_FORMAT_RGB565   = MAKE_MTK_FB_FORMAT_ID(1, 2),
    MTK_FB_FORMAT_RGB888   = MAKE_MTK_FB_FORMAT_ID(2, 3),
    MTK_FB_FORMAT_BGR888   = MAKE_MTK_FB_FORMAT_ID(3, 3),
    MTK_FB_FORMAT_ARGB8888 = MAKE_MTK_FB_FORMAT_ID(4, 4),
    MTK_FB_FORMAT_ABGR8888 = MAKE_MTK_FB_FORMAT_ID(5, 4),
    MTK_FB_FORMAT_YUV422   = MAKE_MTK_FB_FORMAT_ID(6, 2),
    MTK_FB_FORMAT_XRGB8888 = MAKE_MTK_FB_FORMAT_ID(7, 4),
    MTK_FB_FORMAT_XBGR8888 = MAKE_MTK_FB_FORMAT_ID(8, 4),
    MTK_FB_FORMAT_UYVY     = MAKE_MTK_FB_FORMAT_ID(9, 2),
    MTK_FB_FORMAT_YUV420_P = MAKE_MTK_FB_FORMAT_ID(10, 2),
    MTK_FB_FORMAT_YUY2	= MAKE_MTK_FB_FORMAT_ID(11, 2),
    #endif

	//printk("[DDP] %s fmt 0x%x \n",__func__,fmt);
#if 1
    switch (fmt)
    {
      	case MTK_FB_FORMAT_RGB565:
            return "MTK_FB_FORMAT_RGB565";

      	case MTK_FB_FORMAT_RGB888:
        	return "MTK_FB_FORMAT_RGB888";

		case MTK_FB_FORMAT_BGR888:
			return "MTK_FB_FORMAT_BGR888";

		case MTK_FB_FORMAT_ARGB8888:
			return "MTK_FB_FORMAT_ARGB8888";

		case MTK_FB_FORMAT_ABGR8888:
			return "MTK_FB_FORMAT_ABGR8888";

		case MTK_FB_FORMAT_YUV422:
			return "MTK_FB_FORMAT_YUV422";

		case MTK_FB_FORMAT_XRGB8888:
			return "MTK_FB_FORMAT_XRGB8888";

		case MTK_FB_FORMAT_XBGR8888:
			return "MTK_FB_FORMAT_XBGR8888";

		case MTK_FB_FORMAT_UYVY:
			return "MTK_FB_FORMAT_UYVY";

		case MTK_FB_FORMAT_YUV420_P:
			return "MTK_FB_FORMAT_YUV420_P";

		case MTK_FB_FORMAT_YUY2:
			return "MTK_FB_FORMAT_YUY2";

        default:
            return "unknown fmt";
    }
	#endif
}


static int cvbs_core_create_attr(struct device *dev)
{
    int idx, err = 0;
    int num = (int)(sizeof(cvbs_attr_list)/sizeof(cvbs_attr_list[0]));
    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) {
        if ((err = device_create_file(dev, cvbs_attr_list[idx])))
            break;
    }
    return err;
}
static void cvbs_core_remove_attr(struct device *dev)
{
  int idx ;
  int num = (int)(sizeof(cvbs_attr_list)/sizeof(cvbs_attr_list[0]));
    if (!dev)
        return ;

    for (idx = 0; idx < num; idx++) {
        device_remove_file(dev, cvbs_attr_list[idx]);
    }
}
long cvbs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	//void __user *argp = (void __user *)arg;
	int ret =0;
	TVE_LOG("cmd: 0x%x %d %s\n", cmd, cmd & 0xff,tve_ioctl_spy(cmd));
	switch(cmd)
	{
	    case CMD_TVE_GET_FORMAT:
		{
		char str[str_lenth];
		cvbs_drv->getfmt(str);
		CVBS_LOG("CMD_TVE_GET_FORMAT str = %s %d \n",str,sizeof(str));
		ret = copy_to_user((void __user *)arg,str,sizeof(str));
			if (ret) {
			CVBS_LOG("copy_to_user failed ret %d\n",ret);
			ret = -1;
			}
		}
		break;

	    case CMD_TVE_SET_ENABLE:
		CVBS_LOG("CMD_TVE_SET_ENABLE arg: %ld\n",arg);
		cvbs_drv->tve_enable(arg);
		break;

		case CMD_TVE_SET_TVE_POWER_ON:
		CVBS_LOG("CMD_TVE_SET_TVE_POWER_ON arg: %ld\n",arg);
		cvbs_drv->init();
		break;

		case CMD_TVE_GET_DEVNAME:
		{
			char device_name[str_lenth]={0};

			cvbs_drv->device_name(device_name);
			CVBS_LOG("CMD_TVE_GET_DEVNAME %s  %d\n",device_name,sizeof(device_name));
			if (copy_to_user((void __user *)arg, device_name, sizeof(device_name))) {
			CVBS_LOG("copy_to_user failed\n");
			ret = -1;
			}
			CVBS_LOG("CMD_TVE_GET_DEVNAME  %s\n",arg);
		}
		break;

		case CMD_TVE_SET_SUSPEND:
		CVBS_LOG("CMD_TVE_SET_SUSPEND arg: %ld\n",arg);
		switch_set_state(&cvbs_switch_data,CVBS_STATE_NO_DEVICE);
#if DISPLAY_V4L2
		tve_video_buffer_info buffer_info;
		disp_unregister_irq(DISP_MODULE_RDMA1, tve_rdma1_irq_handler);
		tve_rdma_address_config(false, buffer_info);
		TVE_Suspend();
#endif
		break;

  		case CMD_TVE_SET_RESUME:
		CVBS_LOG("CMD_TVE_SET_RESUME arg: %ld\n",arg);
		switch_set_state(&cvbs_switch_data,CVBS_STATE_ACTIVE);
#if DISPLAY_V4L2
		TVE_Resume();
		disp_register_irq(DISP_MODULE_RDMA1, tve_rdma1_irq_handler);
#endif
		break;

		case CMD_TVE_SET_FORMAT:
		{
			//CHAR str[10] = {0};
			//copy_from_user(str, (void __user *)arg, sizeof(str));
			TVE_MSG("CMD_TVE_SET_FORMAT arg:%ld,cvbs_res=%d\n",arg,cvbs_res);
			if(arg == 0 || arg == 2)//480i
			{
			    if(cvbs_res != 0)
                         {
			        cvbs_drv->setfmt(TVE_RES_480P);
				 if(cvbs_connect_state == CVBS_STATE_ACTIVE)
					 cvbs_recchg_nofity(0);
                              cvbs_res = 0;
                        }
			}
			else if(arg == 1 || arg == 3)//arg = 1 -->576i
			{
			    if(cvbs_res != 1)
                        {
			        cvbs_drv->setfmt(TVE_RES_576P);
				  if(cvbs_connect_state == CVBS_STATE_ACTIVE)
					cvbs_recchg_nofity(1);
                            cvbs_res =1;
                        }
			}
			else{
				TVE_ERR("CMD_TVE_SET_FORMAT arg:%ld is not exsit,arg should be 0 or 1\n",arg);
			}
			//switch_set_state(&cvbs_switch_data,CVBS_STATE_ACTIVE);
		}
		break;

		case CMD_TVE_SET_CPS:
		{
			IBC_CpsCommonInfoParamsDef cps;
			ret = copy_from_user(&cps,(void __user *)arg, sizeof(cps));
			if (ret)
			{
				CVBS_LOG("copy_from_user failed! line:%d  %d \n", __LINE__,ret);
			}
			printf_cps(&cps);
			CVBS_LOG("CMD_TVE_SET_CPS \n");
			cvbs_drv->set_cps(&cps);
		    break;
		}
		case CMD_TVE_SET_MV:
		CVBS_LOG("CMD_TVE_SET_MV \n");
		cvbs_drv->setmv((char)arg);
		break;
		case CMD_TVE_SET_ASPECT:
		CVBS_LOG("CMD_TVE_SET_ASPECT arg:%ld\n",arg);
		cvbs_drv->set_aspect(arg);
		break;
		case CMD_TVE_SET_LOG_ON:
		CVBS_LOG("CMD_TVE_SET_LOG_ON arg:%ld\n",arg);
	    cvbs_drv->log_enable((unsigned short)arg);
		break;

		case CMD_TVE_SET_COLORBAR:
		CVBS_LOG("CMD_TVE_SET_COLORBAR arg:%ld\n",arg);
		cvbs_drv->colorbar(arg);
		break;

		case CMD_TVE_SET_DPI0_CB:
		CVBS_LOG("CMD_TVE_SET_DPI0_CB arg:%ld\n",arg);
		cvbs_drv->setDPI0CB(arg);
		break;

		case CMD_TVE_SET_BRIGHTNESS:
		CVBS_LOG("CMD_TVE_SET_BRIGHTNESS\n");
		cvbs_drv->setbri(arg);
		break;

        case CMD_TVE_GET_TVE_STATUS:
		{
	#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
		TVE_STATUS_CONF_T tve_status;
		CVBS_LOG("CMD_TVE_GET_TVE_STATUS\n");
		cvbs_drv->get_tve_status(&tve_status);
		if (copy_to_user((void __user *)arg, &tve_status, sizeof(tve_status))) {
               CVBS_LOG("copy_to_user failed! line:%d \n", __LINE__);
               ret = -1;
           }
		break;
	#endif
        }
		case CMD_TVE_GET_UNIFY:
		CVBS_LOG("CMD_TVE_GET_UNIFY\n");
		break;

		case CMD_TVE_POST_VIDEO_BUFFER:
        {
            tve_video_buffer_list *pBuffList;
            tve_video_buffer_info video_buffer_info;

			memset(&video_buffer_info, 0, sizeof(video_buffer_info));

            video_buffer_info.src_fmt = MTK_FB_FORMAT_YUV422; //MTK_FB_FORMAT_ARGB8888;
            #if 0
            if( (p->is_enabled== false)|| (p->is_clock_on==false )|| IS_HDMI_NOT_ON())
            {
                RETIF(!p->is_enabled, 0);
                RETIF(!p->is_clock_on, -1);
                RETIF(IS_HDMI_NOT_ON(), 0);
            }
			#endif

            if(copy_from_user(&video_buffer_info, (void __user *)arg, sizeof(video_buffer_info)))
            {
                TVE_MSG("copy_from_user failed! line\n");
                ret = -EFAULT;
                break;
            }

            //DBG_OnTriggerHDMI();

            //if(p->is_clock_on)
            {
                TVE_LOG("CMD_TVE_POST_VIDEO_BUFFER: base:0x%x phy:0x%x, "
                    "fmt:0x%x(%s), pitch:0x%x, offset_x:0x%x offset_y:0x%x,"
                    "width:0x%x height:0x%x, next_idx:%d, identity:%d, "
                    "connected_type:%d, security:%d, fenceFd:%d\n",
                    video_buffer_info.src_base_addr,
                    video_buffer_info.src_phy_addr,
                    video_buffer_info.src_fmt,mtk_fb_format_spy(video_buffer_info.src_fmt),
                    video_buffer_info.src_pitch,
                    video_buffer_info.src_offset_x,
                    video_buffer_info.src_offset_y,
                    video_buffer_info.src_width,
                    video_buffer_info.src_height,
                    video_buffer_info.next_buff_idx,
                    video_buffer_info.identity,
                    video_buffer_info.connected_type,
                    video_buffer_info.security,
                    video_buffer_info.fenceFd);

                pBuffList = kmalloc(sizeof(tve_video_buffer_list), GFP_KERNEL);
                memset(pBuffList, 0 , sizeof(tve_video_buffer_list));
                spin_lock_bh(&tve_lock);
                pBuffList->buf_state = insert_new;
                pBuffList->fence = NULL;
                memcpy(&pBuffList->buffer_info, &video_buffer_info, sizeof(video_buffer_info));



                if (pBuffList->buffer_info.fenceFd >= 0)
                pBuffList->fence = sync_fence_fdget(pBuffList->buffer_info.fenceFd);

                DISP_PRINTF(DDP_FENCE2_LOG,"fence2:post fenceFd %d fence 0x%x %d\n",
                    pBuffList->buffer_info.fenceFd,pBuffList->fence,pBuffList->fence);

                INIT_LIST_HEAD(&pBuffList->list);
                list_add_tail(&pBuffList->list, &tve_buffer_list);
                spin_unlock_bh(&tve_lock);

				#if 0
                if (0)
                {
                    MMP_MetaDataBitmap_t Bitmap;
                    unsigned int mva_size, kernel_va, kernel_size;

                    mva_size = video_buffer_info.src_width*video_buffer_info.src_height*3;
                    m4u_do_mva_map_kernel(video_buffer_info.src_phy_addr, mva_size, 0, &kernel_va, &kernel_size);

                    TVE_LOG("video_buffer_info kernel_va: 0x%x kernel_size: 0x%x\n", kernel_va, kernel_size);

                    Bitmap.data1 = video_buffer_info.src_width;
                    Bitmap.data2 = video_buffer_info.src_pitch;
                    Bitmap.width = video_buffer_info.src_width;
                    Bitmap.height = video_buffer_info.src_height;
                    Bitmap.format = MMProfileBitmapRGB888;
                    Bitmap.start_pos = 0;
                    Bitmap.pitch = video_buffer_info.src_pitch * 3;
                    Bitmap.data_size = kernel_size;
                    Bitmap.down_sample_x = 10;
                    Bitmap.down_sample_y = 10;
                    Bitmap.pData = kernel_va;
                    Bitmap.bpp = 24;

                    if (kernel_size != 0)
                        MMProfileLogMetaBitmap(HDMI_MMP_Events.DDPKBitblt, MMProfileFlagPulse, &Bitmap);

                    m4u_do_mva_unmap_kernel(video_buffer_info.src_phy_addr, mva_size, kernel_va);
                }
				#endif

                if (tve_dp_mutex_dst < 0) {
                    atomic_set(&tve_rdma_config_event, 1);
                    wake_up_interruptible(&tve_rdma_config_wq);
                }
            }
            break;
        }

		case CMD_TVE_GET_DEV_INFO:
		{
	        int displayid = 0;
	        mtk_dispif_info_t tve_info;

	        //if (hdmi_bufferdump_on > 0)
	        {
	            //MMProfileLogEx(HDMI_MMP_Events.GetDevInfo, MMProfileFlagStart, p->is_enabled, p->is_clock_on);
	        }

	        TVE_LOG("DEV_INFO configuration get + \n");

	        if (copy_from_user(&displayid, (void __user *)arg, sizeof(displayid)))
	        {
	            //if (tve_bufferdump_on > 0)
	            {
	            //    MMProfileLogEx(HDMI_MMP_Events.GetDevInfo, MMProfileFlagEnd, 0xff, 0xff1);
	            }

	            TVE_ERR(": copy_from_user failed! line:%d \n", __LINE__);
	            return -EAGAIN;
	        }

	        if (displayid != MTKFB_DISPIF_CVBS || displayid != MTKFB_DISPIF_HDMI)
	        {
	            //if (tve_bufferdump_on > 0)
	            {
	            //    MMProfileLogEx(HDMI_MMP_Events.GetDevInfo, MMProfileFlagPulse, 0xff, 0xff2);
	            }
	            TVE_MSG(": invalid display id:%d \n", displayid);
	            ///return -EAGAIN;
	        }

	        memset(&tve_info, 0, sizeof(tve_info));
	        tve_info.displayFormat = cvbs_res;//;CVBSOutFmt
	        tve_info.displayHeight =(cvbs_res ? 576 : 480) ;//p->hdmi_height;
	        tve_info.displayWidth = 720;//p->hdmi_width;
	        tve_info.display_id = displayid;
		 #if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
               if(cvbs_connect_state == CVBS_STATE_ACTIVE)
                    tve_info.isConnected = 1;
               else
                    tve_info.isConnected = 0;
		#else
	        tve_info.isConnected = 1;
		#endif
               TVE_MSG("isConnected:%d\n", tve_info.isConnected);
	        tve_info.displayMode = DISPIF_MODE_COMMAND;
	        tve_info.displayType = TVE_DISPIF_TYPE;
	        tve_info.isHwVsyncAvailable = 1;
	        tve_info.vsyncFPS = 50;

	        TVE_MSG("get device info w %d h %d \n",
	        tve_info.displayWidth,tve_info.displayHeight);

	        if (copy_to_user((void __user *)arg, &tve_info, sizeof(tve_info)))
	        {
	            //if (tve_bufferdump_on > 0)
	            {
	               // MMProfileLogEx(HDMI_MMP_Events.GetDevInfo, MMProfileFlagEnd, 0xff, 0xff2);
	            }

	            TVE_ERR("copy_to_user failed! line:%d \n", __LINE__);
	            ret = -EFAULT;
	        }

	        //if (tve_bufferdump_on > 0)
	        {
	          //  MMProfileLogEx(HDMI_MMP_Events.GetDevInfo, MMProfileFlagEnd, p->is_enabled, tve_info.displayType);
	        }

	        TVE_LOG("DEV_INFO configuration get displayType-%d \n", tve_info.displayType);

	        break;
    	}

		case CMD_TVE_PREPARE_BUFFER:
        {
            tve_buffer_info tve_buffer;
            unsigned int value = 0;
            int fenceFd = MTK_TVE_NO_FENCE_FD;

            if (copy_from_user(&tve_buffer, (void __user *)arg, sizeof(tve_buffer))) {
                TVE_MSG(" copy_from_user failed! line:%d \n", __LINE__);
                ret = -EFAULT;
                break;
            }

            if (down_interruptible(&tve_update_mutex)) {
                TVE_MSG(" Warning can't get semaphore in\n");
                ret = -EFAULT;
                break;
            }

           // if(p->is_clock_on)
            {
                tve_create_fence(&fenceFd, &value);
            }
            //else
            {
            //    TVE_LOG(" : Error in hdmi_create_fence when is_clock_on is off\n");
            }

            DISP_PRINTF(DDP_FENCE2_LOG,"fence2:create %d value %d\n",
                    fenceFd,value);

            tve_buffer.fence_fd = fenceFd;
            tve_buffer.index = value;
            up(&tve_update_mutex);

            if (copy_to_user((void __user *)arg, &tve_buffer, sizeof(tve_buffer)))
            {
                TVE_MSG(" : copy_to_user error! line:%d \n", __LINE__);
                ret = -EFAULT;
            }

            break;
        }

	case CMD_TVE_POST_VIDEO_BUFFER_EX:
	{
        tve_video_buffer_info video_buffer_info;

        video_buffer_info.src_fmt = MTK_FB_FORMAT_RGB565; //MTK_FB_FORMAT_ARGB8888;
        unsigned int mva_size = 0;
        unsigned int buffer_offset = TVE_DISP_WIDTH * TVE_DISP_HEIGHT * RDMA1_SRC_BPP;

        if(copy_from_user(&video_buffer_info, (void __user *)arg, sizeof(video_buffer_info)))
        {
            TVE_MSG("copy_from_user failed! line\n");
            ret = -EFAULT;
            break;
        }


        //if(p->is_clock_on)
        {
            TVE_MSG("CMD_TVE_POST_VIDEO_BUFFER_EX: base:0x%x phy:0x%x, "
                "fmt:0x%x(%s), pitch:0x%x, offset_x:0x%x offset_y:0x%x,"
                "width:0x%x height:0x%x, next_idx:%d, identity:%d, "
                "connected_type:%d, security:%d, fenceFd:%d\n",
                video_buffer_info.src_base_addr,
                video_buffer_info.src_phy_addr,
                video_buffer_info.src_fmt,mtk_fb_format_spy(video_buffer_info.src_fmt),
                video_buffer_info.src_pitch,
                video_buffer_info.src_offset_x,
                video_buffer_info.src_offset_y,
                video_buffer_info.src_width,
                video_buffer_info.src_height,
                video_buffer_info.next_buff_idx,
                video_buffer_info.identity,
                video_buffer_info.connected_type,
                video_buffer_info.security,
                video_buffer_info.fenceFd);

            {
                mva_size = video_buffer_info.src_width * video_buffer_info.src_height * RDMA1_SRC_BPP;
                if(NULL == tve_va)
                {
                    tve_allocate_rdma1_buffer();
                }

                tve_recovery_write_buffer_index = (tve_recovery_write_buffer_index + 1) % 3;
                if(tve_recovery_write_buffer_index == tve_recovery_read_buffer_index)
                {
                    tve_recovery_write_buffer_index = (tve_recovery_write_buffer_index + 2) % 3;
                }
                memcpy((void*)tve_va + buffer_offset * tve_recovery_write_buffer_index, video_buffer_info.src_phy_addr, mva_size);
                __cpuc_flush_dcache_area((void*)tve_va + buffer_offset * tve_recovery_write_buffer_index, mva_size);
                tve_recovery_read_buffer_index = tve_recovery_write_buffer_index;

                video_buffer_info.src_phy_addr = tve_mva_r + buffer_offset * tve_recovery_read_buffer_index;
            }

            DISP_PRINTF(DDP_FENCE2_LOG, "fence2:post addr 0x%x\n",video_buffer_info.src_phy_addr);


            tve_rdma_address_config(1,video_buffer_info);
            }

            break;
        }
	   default:
		TVE_ERR(" invalid cmd\n");
	   break;
	}
    return 0;
}

static int cvbs_release(struct inode *inode, struct file *file)
{
	CVBS_FUNC();
    return 0;
}

static int cvbs_open(struct inode *inode, struct file *file)
{
	CVBS_FUNC();
    return 0;
}

static void cvbs_udelay(unsigned int us)
{
    udelay(us);
}

static void cvbs_mdelay(unsigned int ms)
{
    msleep(ms);
}

void cvbs_state_callback(CVBS_STATE state)
{
	switch(state)
	{
		case CVBS_STATE_NO_DEVICE:
              printk("[cvbs]call_back:CVBS_STATE_NO_DEVICE\n");
		switch_set_state(&cvbs_switch_data,CVBS_STATE_NO_DEVICE);
              cvbs_connect_state = CVBS_STATE_NO_DEVICE;
		break;

		case CVBS_STATE_ACTIVE:
              printk("[cvbs]call_back:CVBS_STATE_ACTIVE\n");
		switch_set_state(&cvbs_switch_data,CVBS_STATE_ACTIVE);
              cvbs_connect_state = CVBS_STATE_ACTIVE;
		break;

		default:
		printk("no this state \n");
		break;
	}
}
void cvbs_recchg_nofity(int index)
{
        printk("cvbs_recchg_nofity %d \n",index);
        switch_set_state(&cvbs_reschg, 0xff);
        switch_set_state(&cvbs_reschg,1);
}
struct file_operations cvbs_fops =
{
    .owner   = THIS_MODULE,
    .unlocked_ioctl   = cvbs_ioctl,
    .open    = cvbs_open,
    .release = cvbs_release,
};

static BOOL cvbs_drv_init_context(void)
{
    static const CVBS_UTIL_FUNCS cvbs_utils =
    {
        .udelay                 = cvbs_udelay,
        .mdelay                 = cvbs_mdelay,
        .state_callback         = cvbs_state_callback,
    };

    if (cvbs_drv != NULL)
    {
        return TRUE;
    }


    cvbs_drv = (CVBS_DRIVER *)CVBS_GetDriver();

    if (NULL == cvbs_drv)
    {
        return FALSE;
    }

    cvbs_drv->set_util_funcs(&cvbs_utils);
    cvbs_drv->get_params(cvbs_params);

    return TRUE;
}

static void __exit cvbs_exit(void)
{
    TVE_PowerOff();

    cvbs_drv->exit();
    device_destroy(cvbs_class, cvbs_devno);
    class_destroy(cvbs_class);
    cdev_del(cvbs_cdev);
    unregister_chrdev_region(cvbs_devno, 1);
	platform_device_unregister(&cvbs_parent_device);
	platform_driver_unregister(&cvbs_driver);

}

static void CVBS_Suspend(struct early_suspend *h)
{
	TVE_Suspend();
}
static void CVBS_Resume(struct early_suspend *h)
{
	TVE_Resume();
}

static int cvbs_remove(struct platform_device *pdev)
{
#if 0
	if (-1 == pdev->id)
	{
		cvbs_core_remove_attr(&(pdev->dev));

		return 0;
	}
#endif

    return 0;
}

static int cvbs_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct class_device *class_dev = NULL;
#if 0
    if (-1 == pdev->id)
    {
        ret = cvbs_core_create_attr(&(pdev->dev)); // create device attribute
        if (ret)
	    {
	        printk("[CVBS]cvbs_core_create_attr fail\n");
	        return -1;
	    }
    }
#endif

    /* Allocate device number for cvbs driver */
    ret = alloc_chrdev_region(&cvbs_devno, 0, 1, CVBS_DEVNAME);

    if (ret)
    {
        printk("[CVBS]alloc_chrdev_region fail\n");
        return -1;
    }

    /* For character driver register to system, device number binded to file operations */
    cvbs_cdev = cdev_alloc();
    cvbs_cdev->owner = THIS_MODULE;
    cvbs_cdev->ops = &cvbs_fops;
    ret = cdev_add(cvbs_cdev, cvbs_devno, 1);

    /* For device number binded to device name, one class is corresponeded to one node */
    cvbs_class = class_create(THIS_MODULE, CVBS_DEVNAME);

    class_dev = (struct class_device *)device_create(cvbs_class, NULL, cvbs_devno, NULL, CVBS_DEVNAME);

    printk("[cvbs][%s] current=0x%08x\n", __func__, (unsigned int)current);

    if (!cvbs_drv_init_context())
    {
        printk("[cvbs]%s, cvbs_drv_init_context fail\n", __func__);
        return TVE_SET_ERROR;
    }

	//


    return 0;
}

extern void tve_path_init(void);


static int __init cvbs_init(void)
{
    int ret = 0;
   // int tmp_boot_mode;


    printk("[cvbs]%s\n", __func__);

#if 1
	// here register parent ir dev for lirc_driver and input_thread
	if (platform_device_register(&cvbs_parent_device))
	{
		printk("[cvbs]failed to register cvbs_parent_driver driver\n");
		return -1;
	}
#endif

    if (platform_driver_register(&cvbs_driver))
    {
        printk("[cvbs]failed to register cvbs_driver driver\n");
        return -1;
    }

	register_early_suspend(&cvbs_early_suspend_handler);

    if (!cvbs_drv_init_context())
    {
        printk("[cvbs]%s, cvbs_drv_init_context fail\n", __func__);
        return TVE_SET_ERROR;//CVBS_STATUS_NOT_IMPLEMENTED;
    }

    cvbs_connect_state = CVBS_STATE_NO_DEVICE;

    cvbs_switch_data.name = "cvbs";
    cvbs_switch_data.index = 0;
    cvbs_switch_data.state = NO_DEVICE;


    ret = switch_dev_register(&cvbs_switch_data);

    cvbs_reschg.name = "reschg_cvbs";
    cvbs_reschg.index = 0;
    cvbs_reschg.state = 0;

    ret = switch_dev_register(&cvbs_reschg);
    if (ret)
    {
        printk("[cvbs][CVBS]cvbs_switch_data_reschg returned:%d!\n", ret);
        return 1;
    }

	if(0)
	{
	   tve_rdma_address_config_ex(true);
	}


	// tve color bar 0x14017604[1] = 0x1
	// dpi0 cb 0x1400df00 = 0x41
	// dpi0 yuv444 0x1400d010[6] rgb2yuv = 0x0
	// dpi0 yuv444 0x1400d014[2:0] = 0x2
    //cvbs_drv->colorbar(TRUE);
    //cvbs_drv->setDPI0CB(FALSE);

    TVE_PowerOn();

	cvbs_drv->init();

    TVE_MSG("cvbs_drv 0x%x set cvbs format %d\n",cvbs_drv,cvbs_res);

        #if 1
        if(cvbs_drv)
        {
                unsigned cvbs_fmt = TVE_RES_576P;
                if(!cvbs_res)
                {
                        cvbs_fmt = TVE_RES_480P;
                }

                TVE_MSG("set cvbs format %d\n",cvbs_res);
                cvbs_drv->setinitfmt(cvbs_fmt);
        }
        #endif


#if DISPLAY_V4L2
	if (1)
	{
#ifdef MTK_TVE_FENCE_SUPPORT

			    tve_sync_init();


			    init_waitqueue_head(&tve_rdma_config_wq);
			    init_waitqueue_head(&tve_rdma_update_wq);
                #ifdef MTK_FOR_CVBS_DITHERING
                init_waitqueue_head(&tve_filter_wq);
                #endif


			   // init_waitqueue_head(&tve_vsync_wq);

			    INIT_LIST_HEAD(&tve_buffer_list);

		        if (!tve_rdma_config_task)
		        {
		            tve_rdma_config_task = kthread_create(tve_rdma_config_kthread, NULL, "tve_rdma_config_kthread");
		            wake_up_process(tve_rdma_config_task);
		        }

		        if (!tve_rdma_update_task)
		        {
		            tve_rdma_update_task = kthread_create(tve_rdma_update_kthread, NULL, "tve_rdma_update_kthread");
		            wake_up_process(tve_rdma_update_task);
		        }
                #ifdef MTK_FOR_CVBS_DITHERING
                if (!tve_filter_task)
                {
                    tve_filter_task = kthread_create(tve_filter_kthread, NULL, "tve_filter_kthread");
                    wake_up_process(tve_filter_task);
                }
                #endif

		        disp_register_irq(DISP_MODULE_RDMA1, tve_rdma1_irq_handler);

#endif
	}
#else
	if (0)
    {
        disp_register_irq(DISP_MODULE_RDMA1, tve_rdma1_irq_handler);

#ifdef MTK_TVE_FENCE_SUPPORT

	    tve_sync_init();
	    INIT_LIST_HEAD(&tve_buffer_list);

        if (!tve_rdma_config_task)
        {
            tve_rdma_config_task = kthread_create(tve_rdma_config_kthread, NULL, "tve_rdma_config_kthread");
            wake_up_process(tve_rdma_config_task);
        }

        if (!tve_rdma_update_task)
        {
            tve_rdma_update_task = kthread_create(tve_rdma_update_kthread, NULL, "tve_rdma_update_kthread");
            wake_up_process(tve_rdma_update_task);
        }

#endif
    }
#endif

    //cvbs_drv->setDPI0CB(FALSE);
	//cvbs_drv->init();
	#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
	cvbs_drv->cvbs_autodetect_init();
	cvbs_drv->enter();
	#endif

	tve_debug_init();

	//cvbs_state_callback(CVBS_STATE_ACTIVE);

#if (!DISPLAY_V4L2)
    tve_path_init();
#endif
    return 0;
}

#define MTK_CVBS_SHOW_COLOR_BAR 0
struct COLOR_BAR_YUV_DATA color_bar_yuv[COLOR_BAR_NO] =
{
#if MTK_CVBS_SHOW_COLOR_BAR // black screen
	{0x51,0x5a,0xf0},// R
	{0x90,0x36,0x22},// G
	{0x28,0xf0,0x6e},// B
	{0x00,0x00,0x00} // G
#else
	{0x10,0x80,0x80},// R
	{0x10,0x80,0x80},// G
	{0x10,0x80,0x80},// B
	{0x10,0x80,0x80} // G
#endif
};


struct COLOR_BAR_RGB_DATA color_bar_rgb[COLOR_BAR_NO] =
{
	{0xff,0x00,0x00},
	{0x00,0xff,0x00},
	{0x00,0x00,0xff},
	{0x00,0x00,0x00}
};

char *color_bar_name[COLOR_BAR_NO] =
{
	"red",
	"green",
	"blue",
	"other"
};

void tve_draw_color_bar(
	unsigned char *buf_addr,
	unsigned int width,
	unsigned int height,
	unsigned format
	)
{
	int i = 0;
	int j = 0;
	int DataSize;
	int pitch;
	int bpp;

	int color_bar_size;

	switch(format)
	{
		case MTK_FB_FORMAT_YUY2:
		case MTK_FB_FORMAT_YUV422:
			bpp = 2;
			break;
		case MTK_FB_FORMAT_RGB888:
		case MTK_FB_FORMAT_BGR888:
			bpp = 3;
			break;
		default :
			bpp = 2;
			break;
	}

	pitch = width*bpp;
	DataSize = pitch*height;
	color_bar_size= DataSize/(COLOR_BAR_NO+1);

	memset((void *) buf_addr, 0, DataSize);

#if MTK_CVBS_SHOW_COLOR_BAR
	if(MTK_FB_FORMAT_YUY2 == format)
	{
		for(i=0;i<COLOR_BAR_NO;++i)
		{
			for(j=i*color_bar_size;j<(i+1)*color_bar_size && j < DataSize;j+=4)
			{
				buf_addr[j]   = color_bar_yuv[i].Y;
				buf_addr[j+1] = color_bar_yuv[i].U;
				buf_addr[j+2] = color_bar_yuv[i].Y;
				buf_addr[j+3] = color_bar_yuv[i].V;
			}
			TVE_MSG(" set %s buffer %d line %d\n",color_bar_name[i],j,j/pitch);

		}
	}
	else if(MTK_FB_FORMAT_YUV422 == format)
	{
		for(i=0;i<COLOR_BAR_NO;++i)
		{
			for(j=i*color_bar_size;j<(i+1)*color_bar_size && j < DataSize;j+=4)
			{
				buf_addr[j]   = color_bar_yuv[i].U;
				buf_addr[j+1] = color_bar_yuv[i].Y;
				buf_addr[j+2] = color_bar_yuv[i].V;
				buf_addr[j+3] = color_bar_yuv[i].Y;
			}
			TVE_MSG(" set %s buffer %d line %d\n",color_bar_name[i],j,j/pitch);
		}

	}
	else if (MTK_FB_FORMAT_BGR888 == format)
	{
	    for(i=0;i<COLOR_BAR_NO;++i)
		{
			for(j=i*color_bar_size;j<(i+1)*color_bar_size && j < DataSize;j+=3)
			{
				buf_addr[j]   = color_bar_rgb[i].B;
				buf_addr[j+1] = color_bar_rgb[i].G;
				buf_addr[j+2] = color_bar_rgb[i].R;
			}
			TVE_MSG(" set %s buffer %d line %d\n",color_bar_name[i],j,j/pitch);
		}

	}
	else if (MTK_FB_FORMAT_RGB888 == format)
	{
	    for(i=0;i<COLOR_BAR_NO;++i)
		{
			for(j=i*color_bar_size;j<(i+1)*color_bar_size && j < DataSize;j+=3)
			{
				buf_addr[j]   = color_bar_rgb[i].R;
				buf_addr[j+1] = color_bar_rgb[i].G;
				buf_addr[j+2] = color_bar_rgb[i].B;
			}
			TVE_MSG(" set %s buffer %d line %d\n",color_bar_name[i],j,j/pitch);
		}

	}
#else
    for(j=0; j < DataSize;j+=4)
    {
    	buf_addr[j]   = color_bar_yuv[0].U; // YUV422 uyvy
    	buf_addr[j+1] = color_bar_yuv[0].Y;
    	buf_addr[j+2] = color_bar_yuv[0].V;
    	buf_addr[j+3] = color_bar_yuv[0].Y;
    }
#endif

	return;
}

void tve_draw_color_bar_ex(enum RDMA1_COLOR_BAR_TYPE rdma1_cb_fmt)
{
	bool enable = true;

	unsigned char *buf_addr;
	unsigned int width = TVE_DISP_WIDTH;
	unsigned int height = TVE_DISP_HEIGHT;
	unsigned format;

	tve_video_buffer_info buffer_info = {0};

    if (((void *) tve_va) == NULL)
	{
		int ret = -1;
		TVE_MSG("tve_va is null, allocate rdma1 read buffer\n");
		ret = tve_allocate_rdma1_buffer();

		if(ret < 0)
		{
			TVE_MSG(" allocate rdma1 read buffer fail,return directly\n");
			return ;
		}
	}

	switch(rdma1_cb_fmt)
	{
		case RDMA1_COLOR_BAR_YUYV:
			format = MTK_FB_FORMAT_YUY2;
			break;

		case RDMA1_COLOR_BAR_UYUV:
			format = MTK_FB_FORMAT_YUV422;
			break;

		case RDMA1_COLOR_BAR_RGB:
			format = MTK_FB_FORMAT_RGB888;
			break;

		case RDMA1_COLOR_BAR_BGR:
			format = MTK_FB_FORMAT_BGR888;
			break;

		default:
			format = eYUYV;
			break;

	}

	buf_addr = (unsigned char *)tve_va;

	buffer_info.src_base_addr = tve_mva_r;
	buffer_info.src_phy_addr = tve_mva_r;
	buffer_info.src_fmt = format;
	buffer_info.src_pitch = width;
	buffer_info.src_offset_x = 0;
	buffer_info.src_offset_y = 0;
	buffer_info.src_width = width;
	buffer_info.src_height = height;
	buffer_info.next_buff_idx = 0;
	buffer_info.identity = 0;
	buffer_info.connected_type = 0;
	buffer_info.security = 0;
	buffer_info.fenceFd = 0;


	tve_show_color_bar = 1;

	tve_draw_color_bar(	buf_addr,	width,height,	format	);

	tve_rdma_address_config(enable,  buffer_info);

}

#ifdef MTK_FOR_CVBS_DITHERING
int tve_allocate_filter_rdma1_buffer(void)
{

    if(tve_rdma1_va)
	{
		TVE_MSG(" tve_temp_va 0x%x already allocate\n",tve_rdma1_va);
        return 0;
    }
    char *buf_addr = NULL;
    unsigned int width = TVE_DISP_WIDTH;
	unsigned int height = TVE_DISP_HEIGHT;
    int PixelSize = width * height;
    int BufferSize = PixelSize * RDMA1_SRC_BPP * 6;
    M4U_PORT_STRUCT m4uport;
    unsigned rdma1_input_format = RDMA1_INTPUT_FORMAT;
	unsigned format = 0;

    tve_rdma1_va = (unsigned int) vmalloc(BufferSize);
    if (((void *) tve_rdma1_va) == NULL)
    {
        TVE_MSG("vmalloc tve_rdma1_va %d bytes fail!!!\n", BufferSize);
        return -1;
    }
	TVE_MSG(" tve_temp_va 0x%x\n",tve_rdma1_va);

    if (m4u_alloc_mva(DISP_RDMA1, tve_rdma1_va, BufferSize, 0, 0, &tve_rdma1_mva))
    {
        TVE_MSG("m4u_alloc_mva for tve_rdma1_mva fail\n");
        return -1;
    }

	TVE_MSG(" alloc_mva tve_rdma1_mva 0x%x\n",tve_rdma1_mva);

    memset((void *) &m4uport, 0, sizeof(M4U_PORT_STRUCT));
    m4uport.ePortID = DISP_RDMA1;
    m4uport.Virtuality = 1;
    m4uport.domain = 3;
    m4uport.Security = 0;
    m4uport.Distance = 1;
    m4uport.Direction = 0;
    m4u_config_port(&m4uport);

    TVE_MSG("tve_rdma1_va=0x%x, tve_rdma1_mva=0x%x\n", tve_rdma1_va, tve_rdma1_mva);

    buf_addr = (char *)tve_rdma1_va;

	switch(rdma1_input_format)
	{
		case eYUYV:
			format = MTK_FB_FORMAT_YUY2;
			break;

		case eUYVY:
			format = MTK_FB_FORMAT_YUV422;
			break;

		case eRGB888:
			format = MTK_FB_FORMAT_RGB888;
			break;

		case eBGR888:
			format = MTK_FB_FORMAT_BGR888;
			break;

		default:
			format = eYUYV;
			break;

	}
	tve_draw_color_bar(buf_addr,width,height,format);

    tve_temp_mva = tve_rdma1_mva + PixelSize * RDMA1_SRC_BPP * 3;
    tve_temp_va = tve_rdma1_va + PixelSize * RDMA1_SRC_BPP * 3;

    TVE_MSG("tve_temp_va=0x%x, tve_temp_mva=0x%x\n", tve_temp_va, tve_temp_mva);
}
#endif



int tve_allocate_rdma1_buffer(void)
{
    int i = 0;
	int j = 0;
    char *buf_addr;
    M4U_PORT_STRUCT m4uport;
	unsigned int width = TVE_DISP_WIDTH;
	unsigned int height = TVE_DISP_HEIGHT;
    int PixelSize = width * height;
    int DataSize = PixelSize * RDMA1_SRC_BPP;
    int BufferSize = DataSize * RDMA1_FRAME_BUFFER_NUM;
	int pitch = width*RDMA1_SRC_BPP;
	int color_bar_size = DataSize/(COLOR_BAR_NO+1);
	unsigned rdma1_input_format = RDMA1_INTPUT_FORMAT;
	unsigned format;

    TVE_FUNC();

	if(tve_va)
	{
		TVE_MSG(" tve_va 0x%x already allocate\n",tve_va);
    return 0;
}

	TVE_MSG(" PixelSize %d DataSize %d BufferSize %d\n",PixelSize,DataSize,BufferSize);

    tve_va = (unsigned int) vmalloc(BufferSize);


    if (((void *) tve_va) == NULL)
    {
        TVE_MSG("vmalloc %d bytes fail!!!\n", BufferSize);
        return -1;
    }

	TVE_MSG(" tve_va 0x%x\n",tve_va);

    //memset((void *) tve_va, 0, BufferSize);

	TVE_MSG(" memset tve_va 0x%x\n",tve_va);

    buf_addr = (char *)tve_va;

	#if 1

	switch(rdma1_input_format)
	{
		case eYUYV:
			format = MTK_FB_FORMAT_YUY2;
			break;

		case eUYVY:
			format = MTK_FB_FORMAT_YUV422;
			break;

		case eRGB888:
			format = MTK_FB_FORMAT_RGB888;
			break;

		case eBGR888:
			format = MTK_FB_FORMAT_BGR888;
			break;

		default:
			format = eYUYV;
			break;

	}
	tve_draw_color_bar(buf_addr,width,height,format);
	#else
	if(eYUYV == rdma1_input_format)
	{
		for (i=0; i<DataSize/4; i+=4)
	    {
	       buf_addr[i] =   0x51;
	       buf_addr[i+1] = 0x5A;
	       buf_addr[i+2] = 0x51;
	       buf_addr[i+3] = 0xF0;
	    }

		TVE_MSG(" set r buffer %d line %d\n",i,i/pitch);

	    for (; i<DataSize*2/4; i+=4)
	    {
	       buf_addr[i] =   0x90;
	       buf_addr[i+1] = 0x36;
	       buf_addr[i+2] = 0x90;
	       buf_addr[i+3] = 0x22;

	    }

		TVE_MSG(" set g buffer %d line %d\n",i,i/pitch);

	    for (; i<DataSize*3/4; i+=4)
	    {
	       buf_addr[i] =   0x28;
	       buf_addr[i+1] = 0xF0;
	       buf_addr[i+2] = 0x28;
	       buf_addr[i+3] = 0x6E;

	    }
		TVE_MSG(" set b buffer %d line %d\n",i,i/pitch);
	}
	else if(eUYVY == rdma1_input_format)
	{
		for(i=0;i<COLOR_BAR_NO;++i)
		{
			for(j=i*color_bar_size;j<(i+1)*color_bar_size && j < DataSize;j+=4)
			{
				buf_addr[j]   = color_bar_yuv[i].U;
				buf_addr[j+1] = color_bar_yuv[i].Y;
				buf_addr[j+2] = color_bar_yuv[i].V;
				buf_addr[j+3] = color_bar_yuv[i].Y;
			}
			TVE_MSG(" set %s buffer %d line %d\n",color_bar_name[i],j,j/pitch);

		}

	}
	else if (eRGB888 == rdma1_input_format)
	{
	    for (i=0; i<DataSize/3; i+=3)
	    {
	       buf_addr[i] = 0xff;
	       buf_addr[i+1] = 0x0;
	       buf_addr[i+2] = 0x0;
	    }

		TVE_MSG(" set r buffer %d line %d\n",i,i/pitch);

	    for (; i<DataSize*2/3; i+=3)
	    {
	       buf_addr[i] = 0x0;
	       buf_addr[i+1] = 0xff;
	       buf_addr[i+2] = 0x0;
	    }

		TVE_MSG(" set g buffer %d line %d\n",i,i/pitch);

	    for (; i<DataSize; i+=3)
	    {
	       buf_addr[i] = 0x0;
	       buf_addr[i+1] = 0x0;
	       buf_addr[i+2] = 0xff;
	    }
		TVE_MSG(" set b buffer %d line %d\n",i,i/pitch);

	}
	#endif
    //RDMA1
    if (m4u_alloc_mva(DISP_RDMA1, tve_va, BufferSize, 0, 0, &tve_mva_r))
    {
        TVE_MSG("m4u_alloc_mva for tve_mva_r fail\n");
        return -1;
    }

	TVE_MSG(" alloc_mva tve_mva_r 0x%x\n",tve_mva_r);

    memset((void *) &m4uport, 0, sizeof(M4U_PORT_STRUCT));
    m4uport.ePortID = DISP_RDMA1;
    m4uport.Virtuality = 1;
    m4uport.domain = 3;
    m4uport.Security = 0;
    m4uport.Distance = 1;
    m4uport.Direction = 0;
    m4u_config_port(&m4uport);

    TVE_MSG("tve_va=0x%08x, tve_mva_r=0x%08x\n", tve_va, tve_mva_r);

    return 0;
}

int tve_free_rdma1_buffer(void)
{
    int tve_va_size = TVE_DISP_WIDTH * TVE_DISP_HEIGHT*RDMA1_SRC_BPP*RDMA1_FRAME_BUFFER_NUM;

    #ifdef MTK_FOR_CVBS_DITHERING
    int tve_temp_va_size = TVE_DISP_WIDTH * TVE_DISP_HEIGHT*RDMA1_SRC_BPP * 6;
    #endif

    //free tve_va & tve_mva
    TVE_MSG("Free tve_va and tve_mva\n");

    if (tve_mva_r)
    {
        M4U_PORT_STRUCT m4uport;
        m4uport.ePortID =  DISP_RDMA1;
        m4uport.Virtuality = 1;
        m4uport.domain = 3;
        m4uport.Security = 0;
        m4uport.Distance = 1;
        m4uport.Direction = 0;
        m4u_config_port(&m4uport);

        m4u_dealloc_mva(DISP_RDMA1,
                        tve_va,
                        tve_va_size,
                        tve_mva_r);
        tve_mva_r = 0;
    }

    if (tve_va)
    {
        vfree((void *) tve_va);
        tve_va = 0;
    }
#ifdef MTK_FOR_CVBS_DITHERING
        if (tve_rdma1_mva)
        {
            M4U_PORT_STRUCT m4uport;
            m4uport.ePortID =  DISP_RDMA1;
            m4uport.Virtuality = 1;
            m4uport.domain = 3;
            m4uport.Security = 0;
            m4uport.Distance = 1;
            m4uport.Direction = 0;
            m4u_config_port(&m4uport);

            m4u_dealloc_mva(DISP_RDMA1,
                            tve_rdma1_mva,
                            tve_temp_va_size,
                            tve_rdma1_mva);
            tve_rdma1_mva = 0;
            tve_temp_mva = 0;
        }

        if (tve_rdma1_va)
        {
            vfree((void *) tve_rdma1_va);
            tve_rdma1_va = 0;
            tve_temp_va = 0;
        }
#endif


    return 0;
}



extern char* saved_command_line;

int tve_rdma_address_config_ex(bool enable)
{
		   tve_video_buffer_info buffer_info = {0};
#if 0 //DISPLAY_V4L2
	unsigned rdma_idx = 0;
#else
		   unsigned rdma_idx = 1;
#endif
		   unsigned orig_target_line ;
		   unsigned target_line ;
        unsigned disp_w = TVE_DISP_WIDTH;
        unsigned disp_h = TVE_DISP_HEIGHT;
        //unsigned cvbs_res = 1;
            char *ptr = NULL;

            unsigned rdma1_src_addr = tve_mva_r;


        #ifdef MTK_FOR_CVBS_DITHERING
                tve_allocate_filter_rdma1_buffer();
        #endif

			tve_allocate_rdma1_buffer();

            TVE_MSG(" cmdline %s \n",saved_command_line);

            #if 1

            ptr = strstr(saved_command_line, "rdma1_src=");
            if(ptr == NULL){
                rdma1_src_addr = tve_mva_r;
                TVE_MSG(" can not get rdma1_src from uboot\n");
            }
            else{
                ptr += 10;
                rdma1_src_addr = simple_strtol(ptr, NULL, 10);
                if(0 == rdma1_src_addr)
                {
                    rdma1_src_addr = tve_mva_r;
                }
            }

        TVE_MSG(" get rdma1_src_addr 0x%x from uboot\n",rdma1_src_addr);


        ptr = strstr(saved_command_line, "cvbs_res=");
        if(ptr == NULL){
                cvbs_res = 1;
                TVE_MSG(" can not get cvbs_res from uboot\n");
        }
        else{
                ptr += 9;
                cvbs_res = simple_strtol(ptr, NULL, 10);
        }

        TVE_MSG(" get cvbs_res %d from uboot\n",cvbs_res);

       // _ucCVBSOutFmt = cvbs_res;
        disp_w = tve_get_disp_width( cvbs_res);
        disp_h = tve_get_disp_height( cvbs_res);

        TVE_MSG(" tve disp w %d h %d\n",disp_w,disp_h);

            #else
            rdma1_src_addr = tve_mva_r;
            #endif





			buffer_info.src_base_addr = rdma1_src_addr;
			buffer_info.src_phy_addr = rdma1_src_addr;
			buffer_info.src_fmt = rdma1_src_fmt = MTK_FB_FORMAT_RGB565;//RDMA1_SRC_FORMAT;
	buffer_info.src_pitch = disp_w;
			buffer_info.src_offset_x = 0;
			buffer_info.src_offset_y = 0;
	buffer_info.src_width = disp_w;
	buffer_info.src_height = disp_h;
			buffer_info.next_buff_idx = 0;
			buffer_info.identity = 0;
			buffer_info.connected_type = 0;
			buffer_info.security = 0;
			buffer_info.fenceFd = 0;

			orig_target_line = RDMAGetTargetLine(rdma_idx);
			target_line = buffer_info.src_height*4/5;

			TVE_LOG("rdma%d target line %d\n",rdma_idx,orig_target_line);

			tve_rdma_address_config(enable,  buffer_info);

			if(orig_target_line > target_line)
			{
				orig_target_line = target_line ;
				TVE_LOG("set rdma%d target line %d tve_dp_mutex_dst %d\n",rdma_idx,target_line,tve_dp_mutex_dst);

				if(tve_dp_mutex_dst = CVBS_PATH_MUTEX_ID)
				{
			        disp_path_get_mutex_(tve_dp_mutex_dst);

					RDMASetTargetLine(rdma_idx, target_line);

			        disp_path_release_mutex_(tve_dp_mutex_dst);
				}

			}

			if (1)
		    {

#ifdef MTK_TVE_FENCE_SUPPORT

			    tve_sync_init();


			    init_waitqueue_head(&tve_rdma_config_wq);
			    init_waitqueue_head(&tve_rdma_update_wq);
                #ifdef MTK_FOR_CVBS_DITHERING
                init_waitqueue_head(&tve_filter_wq);
                #endif


			   // init_waitqueue_head(&tve_vsync_wq);

			    INIT_LIST_HEAD(&tve_buffer_list);

		        if (!tve_rdma_config_task)
		        {
		            tve_rdma_config_task = kthread_create(tve_rdma_config_kthread, NULL, "tve_rdma_config_kthread");
		            wake_up_process(tve_rdma_config_task);
		        }

		        if (!tve_rdma_update_task)
		        {
		            tve_rdma_update_task = kthread_create(tve_rdma_update_kthread, NULL, "tve_rdma_update_kthread");
		            wake_up_process(tve_rdma_update_task);
		        }
                #ifdef MTK_FOR_CVBS_DITHERING
                if (!tve_filter_task)
                {
                    tve_filter_task = kthread_create(tve_filter_kthread, NULL, "tve_filter_kthread");
                    wake_up_process(tve_filter_task);
                }
                #endif

		        disp_register_irq(DISP_MODULE_RDMA1, tve_rdma1_irq_handler);

#endif
		    }
}
void tve_set_dump_on(unsigned dump_on)
{
        tve_buffer_dump_on = dump_on;

        TVE_MSG(" set tve_buffer_dump_on %d\n",dump_on);
}

#ifdef MTK_FOR_CVBS_DITHERING
void tve_set_filter_on(unsigned filter_on)
{
    tve_filter_on = filter_on;

    TVE_MSG("set tve_filter_on (%d)\n", filter_on);
}

#endif

#if 0
int tve_rdma_address_config(bool enable, tve_video_buffer_info buffer_info)
{
    int rdmaInputFormat;
    int rdmaInputsize;
    unsigned int offset;
    unsigned int SourceAddr;
    struct disp_path_config_struct config = {0};
    bool need_config;


    if (enable)
    {

        rdmaInputFormat = eYUYV;
        rdmaInputsize = 2;

		switch(buffer_info.src_fmt)
		{
			case MTK_FB_FORMAT_ARGB8888:
				rdmaInputsize = 4;
            	rdmaInputFormat = eARGB8888;
				break;
			case MTK_FB_FORMAT_RGB888:
	            rdmaInputsize = 3;
	            rdmaInputFormat = eRGB888;
				break;
			case MTK_FB_FORMAT_YUV422:
	            rdmaInputsize = 2;
	            rdmaInputFormat = eYUYV;
			break;
			default:
		        TVE_MSG("[cvbs] src_fmt :0x%x  !\n",buffer_info.src_fmt);

				break;
		}

        TVE_MSG("[cvbs] src_fmt : 0x%x rdmaInputsize %d rdmaInputFormat 0x%x!\n",
			buffer_info.src_fmt,rdmaInputsize,rdmaInputFormat);

        offset = (buffer_info.src_pitch - buffer_info.src_width) / 2 * rdmaInputsize;
        SourceAddr = (unsigned int)buffer_info.src_phy_addr
                                      + buffer_info.src_offset_y * buffer_info.src_pitch * rdmaInputsize
                                      + buffer_info.src_offset_x * rdmaInputsize + offset;


		TVE_MSG("[cvbs] src_phy_addr 0x%x offset 0x%x SourceAddr 0x%x!\n",
			buffer_info.src_phy_addr,offset,SourceAddr);

        config.addr = SourceAddr;

        config.srcWidth = buffer_info.src_width;
        config.srcHeight = buffer_info.src_height;
        config.bgROI.width = buffer_info.src_width;
        config.bgROI.height = buffer_info.src_height;
        config.srcROI.width = buffer_info.src_width;
        config.srcROI.height = buffer_info.src_height;
        config.srcROI.x = 0;
        config.srcROI.y = 0;
        config.bgROI.x = 0;
        config.bgROI.y = 0;
        config.bgColor = 0x0;   // background color

        config.inFormat = rdmaInputFormat;
        config.pitch = buffer_info.src_pitch * rdmaInputsize;

        config.outFormat = RDMA1_OUTPUT_FORMAT;


        /*sub path rdma1->dpi0*/
        config.srcModule = DISP_MODULE_RDMA1;
		config.dstModule = DISP_MODULE_DPI0;


        need_config = true;

        if (tve_dp_mutex_dst <= 0)
        {
            tve_dp_mutex_dst = 2;
            tve_rdmafpscnt = 0;
        }
        else
        {
            need_config = false;
        }

        tve_rdmafpscnt++;

        ///disp_path_get_mutex_(dp_mutex_dst);

        if (true == need_config)
        {
    		#if 0
            M4U_PORT_STRUCT m4uport;
            memset((void *) &m4uport, 0, sizeof(M4U_PORT_STRUCT));
            m4uport.ePortID = DISP_RDMA1;
            m4uport.Virtuality = 1;
            m4uport.domain = 3;
            m4uport.Security = 0;
            m4uport.Distance = 1;
            m4uport.Direction = 0;
            m4u_config_port(&m4uport);
    		#endif


            disp_path_config_(&config, tve_dp_mutex_dst);

            ///DPI_CHECK_RET(HDMI_DPI(_EnableColorBar)());
            //DPI_CHECK_RET(HDMI_DPI(_EnableClk)());
            //DPI_CHECK_RET(HDMI_DPI(_DumpRegisters)());
        }


        disp_path_get_mutex_(tve_dp_mutex_dst);
        DISP_REG_SET(0xa000 + DISP_REG_RDMA_MEM_START_ADDR, config.addr);
        disp_path_release_mutex_(tve_dp_mutex_dst);

        ///disp_path_release_mutex_(dp_mutex_dst);

        #if 0
        {
            MMP_MetaDataBitmap_t Bitmap;
            Bitmap.data1 = config.addr;
            Bitmap.data2 = buffer_info.src_base_addr;
            Bitmap.width = buffer_info.src_width;
            Bitmap.height = buffer_info.src_height;
            Bitmap.format = MMProfileBitmapBGRA8888;
            Bitmap.start_pos = 0;
            Bitmap.pitch = buffer_info.src_width * 4;

            Bitmap.data_size = Bitmap.pitch * Bitmap.height;
            Bitmap.down_sample_x = 10;
            Bitmap.down_sample_y = 10;
            Bitmap.pData = buffer_info.src_base_addr;
            Bitmap.bpp = 32;

            MMProfileLogMetaBitmap(HDMI_MMP_Events.BufferCfg, MMProfileFlagPulse, &Bitmap);
        }
        #endif

    }
    else
    {
        if (-1 != tve_dp_mutex_dst)
        {
            //FIXME: release mutex timeout
            TVE_LOG("Stop RDMA1>DPI0\n");
            disp_path_get_mutex_(tve_dp_mutex_dst);

            ///DISP_REG_SET_FIELD(1 << dp_mutex_src , DISP_REG_CONFIG_MUTEX_INTEN,  1);
            RDMAStop(1);
            RDMAReset(1);
            disp_path_release_mutex_(tve_dp_mutex_dst);

            //disp_unlock_mutex(dp_mutex_dst);
            tve_dp_mutex_dst = -1;
        }
    }

    return 0;
}
#else

#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT
extern void disp_register_intr(unsigned int irq, unsigned int secure);
extern KREE_SESSION_HANDLE ddp_session_handle(void);
//extern unsigned int gRdma1Secure;

static unsigned int gRDMASecure = 0;
#endif

//extern struct HDMI_MMP_Events_t HDMI_MMP_Events;

unsigned int tve_get_disp_width(unsigned res)
{
    if(res > CVBS_MAX_ERS)
    {
    	TVE_MSG("[LK] error %s, #%d res %d\n", __func__, __LINE__,res);
    }

    return tve_res_param_table[res][0];
}

unsigned int tve_get_disp_height(unsigned res)
{
    if(res > CVBS_MAX_ERS)
    {
    	TVE_MSG("[LK] error %s, #%d res %d\n", __func__, __LINE__,res);
    }

    return tve_res_param_table[res][1];
}

int tve_rdma_address_config(bool enable, tve_video_buffer_info buffer_info)
{
    unsigned int offset;
#if 0 //DISPLAY_V4L2
	unsigned int rdma_idx = 0;
#else
	unsigned int rdma_idx = 1;
#endif
    unsigned int SourceAddr;
    struct disp_path_config_struct config = {0};

    #ifdef MTK_FOR_CVBS_DITHERING
    unsigned int kernel_temp_va = 0, kernel_temp_size = 0;
    #endif
    unsigned int mva_size = 0, kernel_va = 0, kernel_size = 0;

    unsigned int rdmaOutFormat = RDMA1_OUTPUT_FORMAT;

 	int rdmaInputFormat = RDMA1_INTPUT_FORMAT; ///bpp == 2 ? RDMA_INPUT_FORMAT_UYVY : RDMA_INPUT_FORMAT_RGB888;
    unsigned int rdmaInputsize = RDMA1_SRC_BPP;
	unsigned int rdmaPitch = RDMA1_SRC_BPP;
    bool need_config = true;

	enum DPI0_INPUT_COLOR_FORMAT dpi0_input_fmt;
	enum DPI0_INPUT_COLOR_FORMAT dpi0_output_fmt = DPI0_OUTPUT_YUV;

	switch(buffer_info.src_fmt)
	{
	    case MTK_FB_FORMAT_RGB565:
	        rdmaInputFormat = eRGB565;
			rdmaInputsize = 2;
			rdmaOutFormat = RDMA_OUTPUT_FORMAT_YUV444;
			dpi0_input_fmt = DPI0_INPUT_YUV;
			//dpi0_output_fmt = DPI0_OUTPUT_YUV;
			break;

		case MTK_FB_FORMAT_YUY2:
			rdmaInputFormat = eYUY2;
			rdmaInputsize = 2;
			rdmaOutFormat = RDMA_OUTPUT_FORMAT_YUV444;
			dpi0_input_fmt = DPI0_INPUT_YUV;
			//dpi0_output_fmt = DPI0_OUTPUT_YUV;
			break;

		case MTK_FB_FORMAT_YUV422:
			rdmaInputFormat = eUYVY;
			rdmaInputsize = 2;
			rdmaOutFormat = RDMA_OUTPUT_FORMAT_YUV444;
			dpi0_input_fmt = DPI0_INPUT_YUV;
			//dpi0_output_fmt = DPI0_OUTPUT_YUV;

			break;
		case MTK_FB_FORMAT_RGB888:
			rdmaInputFormat = eRGB888;
			rdmaInputsize = 3;
			rdmaOutFormat = RDMA_OUTPUT_FORMAT_ARGB;
			dpi0_input_fmt = DPI0_INPUT_RGB;
			//dpi0_output_fmt = DPI0_OUTPUT_RGB;
			break;
		case MTK_FB_FORMAT_BGR888:
			rdmaInputFormat = eBGR888;
			rdmaInputsize = 3;
			rdmaOutFormat = RDMA_OUTPUT_FORMAT_ARGB;
			dpi0_input_fmt = DPI0_INPUT_RGB;
			//dpi0_output_fmt = DPI0_OUTPUT_RGB;
			break;
		default:
			TVE_MSG(" unknow format 0x%x \n ",buffer_info.src_fmt);
			break;
	}

    TVE_LOG(" buffer_info: base:0x%x phy:0x%x, "
                    "fmt:0x%x(%s), pitch:0x%x, offset_x:0x%x offset_y:0x%x,"
                    "width:0x%x height:0x%x, next_idx:%d, identity:%d, "
                    "connected_type:%d, security:%d, fenceFd:%d\n",
                    buffer_info.src_base_addr,
                    buffer_info.src_phy_addr,
                    buffer_info.src_fmt, mtk_fb_format_spy(buffer_info.src_fmt),
                    buffer_info.src_pitch,
                    buffer_info.src_offset_x,
                    buffer_info.src_offset_y,
                    buffer_info.src_width,
                    buffer_info.src_height,
                    buffer_info.next_buff_idx,
                    buffer_info.identity,
                    buffer_info.connected_type,
                    buffer_info.security,
                    buffer_info.fenceFd);


	rdmaPitch = buffer_info.src_width*rdmaInputsize;
    mva_size = rdmaPitch*buffer_info.src_height;

    if(enable)
    {

        need_config = true;

       // if (p->is_clock_on == false) {
          //  TVE_LOG(" clock stoped enable(%d), is_clock_on(%d)\n", enable, p->is_clock_on);
          //  return -1;
        //}
       #ifdef MTK_FOR_CVBS_DITHERING
       if (tve_rdma1_va && tve_filter_on && tve_only_UI_filter_on)
       {
           m4u_do_mva_map_kernel(buffer_info.src_phy_addr, mva_size, 0, &kernel_temp_va, &kernel_temp_size);
           if(kernel_temp_size != 0)
           {
               tve_write_buffer_index = (tve_write_buffer_index + 1)%3;
               if(tve_write_buffer_index == tve_read_buffer_index)
               {
                   tve_write_buffer_index = (tve_write_buffer_index + 2)%3;
               }
               memcpy((void*)tve_temp_va + tve_write_buffer_index * TVE_DISP_WIDTH * TVE_DISP_HEIGHT * RDMA1_SRC_BPP, (void*)kernel_temp_va, kernel_temp_size);
               tve_read_buffer_index = tve_write_buffer_index;

               atomic_set(&tve_filter_event, 1);
               wake_up_interruptible(&tve_filter_wq);
           }
           m4u_do_mva_unmap_kernel(buffer_info.src_phy_addr, mva_size, kernel_temp_va);

       }
       #endif

        if (tve_buffer_dump_on == 7)
        {
            m4u_do_mva_map_kernel(buffer_info.src_phy_addr, mva_size, 0, &kernel_va, &kernel_size);

            TVE_MSG("video_buffer_info kernel_va: 0x%x kernel_size: 0x%x\n", kernel_va, kernel_size);

			#if 0
		    MMP_MetaDataBitmap_t Bitmap;

            Bitmap.data1 = buffer_info.src_width;
            Bitmap.data2 = buffer_info.src_pitch;
            Bitmap.width = buffer_info.src_width;
            Bitmap.height = buffer_info.src_height;
            Bitmap.format = (rdmaInputsize==4)?MMProfileBitmapRGBA8888:MMProfileBitmapRGB888;
            Bitmap.start_pos = 0;
            Bitmap.pitch = buffer_info.src_pitch * rdmaInputsize;
            Bitmap.data_size = kernel_size;
            Bitmap.down_sample_x = 10;
            Bitmap.down_sample_y = 10;
            Bitmap.pData = kernel_va;
            Bitmap.bpp = 8*rdmaInputsize;
			#endif

            if (kernel_size != 0)
            {
                memcpy((void *)tve_va, (void *)kernel_va, kernel_size);

	            TVE_MSG("copy tve_va 0x%x kernel_va 0x%x\n", tve_va,kernel_va);

                //MMProfileLogMetaBitmap(HDMI_MMP_Events.DDPKBitblt, MMProfileFlagPulse, &Bitmap);
            }
        }

        offset = (buffer_info.src_pitch - buffer_info.src_width) / 2 * rdmaInputsize;
#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT
        if(buffer_info.security == 1)
        {
            SourceAddr = (unsigned int)buffer_info.src_phy_addr;    //This will be a secure buffer handle
            TVE_LOG(" Get secure buffer, handle = %d\n", SourceAddr);
        }
        else
#endif
        {
            SourceAddr = (unsigned int)buffer_info.src_phy_addr;
            TVE_LOG(" Get normal buffer, address = 0x%x\n", SourceAddr);
        }

        // Config RDMA->DPI1
        if (tve_buffer_dump_on == 7)
    	{
            config.addr = tve_mva_r;
    	}
		else if(tve_show_color_bar == 1)
		{
			config.addr = tve_mva_r;
		}
        #ifdef MTK_FOR_CVBS_DITHERING
        else if(kernel_temp_size != 0)
        {
            config.addr = tve_rdma1_mva + tve_rdma1_read_buffer_index * TVE_DISP_WIDTH * TVE_DISP_HEIGHT * RDMA1_SRC_BPP;
        }
        #endif
        else
    	{
            config.addr = SourceAddr;//;
    	}

        config.srcWidth = buffer_info.src_width;
        config.srcHeight = buffer_info.src_height;
		#if 0 // DISPLAY_V4L2
		config.srcModule = DISP_MODULE_RDMA0;
		#else
        config.srcModule = DISP_MODULE_RDMA1;
		#endif
        config.inFormat = rdmaInputFormat;
        config.pitch = buffer_info.src_width * rdmaInputsize;
        config.outFormat = rdmaOutFormat;
        config.dstModule = DISP_MODULE_DPI0;


        if(tve_dp_mutex_dst <= 0)
        {
            tve_dp_mutex_dst = 2;
            tve_rdmafpscnt=0;
        }
        else
            need_config = false;

        tve_rdmafpscnt++;

		if(need_config)
		{
		    unsigned int mutexSof = 2;

	   		DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(tve_dp_mutex_dst), (1<<12)); //rdma1


	        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(tve_dp_mutex_dst), mutexSof);
		}

		disp_path_get_mutex_(tve_dp_mutex_dst);


#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT
        if ((true == need_config) || (gRDMASecure != buffer_info.security))
#else
        if(true == need_config)
#endif
        {
#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT
            if (gRDMASecure != buffer_info.security)
            {
                //HDMI_LOG("Disable HDMI DPI Clock\n");
                //DPI_CHECK_RET(HDMI_DPI(_DisableClk)());
                gRDMASecure = buffer_info.security;
            }
            disp_path_config_(&config, tve_dp_mutex_dst);
            if(buffer_info.security)
            {
                MTEEC_PARAM param[4];
                unsigned int paramTypes;
                TZ_RESULT ret;
                //HDMI_LOG("Config M4U Security path\n");
                disp_register_intr(MT6582_DISP_RDMA1_IRQ_ID, 0);
                param[0].value.a = DISP_RDMA1;
                param[1].value.a = buffer_info.security;
                paramTypes = TZ_ParamTypes2(TZPT_VALUE_INPUT,TZPT_VALUE_INPUT);
                ret = KREE_TeeServiceCall(ddp_session_handle(), TZCMD_DDP_SET_SECURE_MODE, paramTypes, param);
                if(ret!= TZ_RESULT_SUCCESS)
                {
                    TVE_LOG("KREE_TeeServiceCall(TZCMD_DDP_SET_SECURE_MODE) fail, ret=%d \n", ret);
                }
            }
            else
            {
#endif
	            M4U_PORT_STRUCT m4uport;
	            TVE_LOG("Config M4U normal path\n");
	            memset((void *) &m4uport, 0, sizeof(M4U_PORT_STRUCT));
	            m4uport.ePortID = DISP_RDMA1;
	            m4uport.Virtuality = 1;
	            m4uport.domain = 3;
	            m4uport.Security = 0;
	            m4uport.Distance = 1;
	            m4uport.Direction = 0;
	            m4u_config_port(&m4uport);
#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT
            }
#endif

#ifndef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT
            disp_path_config_(&config, tve_dp_mutex_dst);

			if(rdma1_src_fmt!=rdmaInputFormat)
			{
				//rdma1_src_fmt=rdmaInputFormat;

				TVE_MSG("RDMA%d src format change: 0x%x -> 0x%x \n",
						rdma_idx, rdma1_src_fmt,rdmaInputFormat);
			}

                if(rdma1_src_height!=config.srcHeight)
		{
		        unsigned target_line = config.srcHeight*4/5;
	                RDMASetTargetLine(rdma_idx, target_line);
		}
#endif
            //DPI_CHECK_RET(HDMI_DPI(_EnableClk)());

        }
        else
        {
#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT
            if(buffer_info.security)
            {
                MTEEC_PARAM param[4];
                unsigned int paramTypes;
                TZ_RESULT ret;

                param[0].value.a = (uint32_t) config.addr;
                paramTypes = TZ_ParamTypes1(TZPT_VALUE_INPUT);
                TVE_LOG("Rdma config handle=0x%x \n", param[0].value.a);

                ret = KREE_TeeServiceCall(ddp_session_handle(), 71/*TZCMD_DDP_RDMA1_ADDR_CONFIG*/, paramTypes, param);
                if(ret!= TZ_RESULT_SUCCESS)
                {
                    TVE_LOG("TZCMD_DDP_RDMA_ADDR_CONFIG fail, ret=%d \n", ret);
                }
            }
            else
#endif
			{
	            DISP_REG_SET(0xa000 + DISP_REG_RDMA_MEM_START_ADDR, config.addr);
                     tve_rdma1_addr_shadowed = config.addr;

			if(rdma1_src_fmt!=rdmaInputFormat || rdma1_src_height!=config.srcHeight)
				{
	                        TVE_MSG("RDMA%d src format change: 0x%x -> 0x%x addr 0x%x,w 0x%x h 0x%x -> 0x%x,pitch 0x%x rdmaOutFormat 0x%x\n",
						rdma_idx, rdma1_src_fmt,rdmaInputFormat,config.addr,
					config.srcWidth,rdma1_src_height,config.srcHeight,rdmaPitch,rdmaOutFormat);

					//rdma1_src_fmt = rdmaInputFormat;

					RDMAConfig(rdma_idx,
                			RDMA_MODE_MEMORY,       	 // mem mode
                			rdmaInputFormat,    				 // inputFormat
                			config.addr,               // display lk logo when entering kernel
                			rdmaOutFormat,     // output format
                			rdmaPitch,     // pitch, eRGB888
                			config.srcWidth,       // width
                			config.srcHeight,      // height
                			0,                           // byte swap
                			0);

        			if(rdma1_src_height!=config.srcHeight)
        			{
        			        unsigned target_line = config.srcHeight*4/5;
        		                RDMASetTargetLine(rdma_idx, target_line);
        			}

				}

	            TVE_LOG("config RDMA1 addr: 0x%x\n", config.addr);
			}
        }

        disp_path_release_mutex_(tve_dp_mutex_dst);

		if(rdma1_src_fmt!=rdmaInputFormat)
		{
			rdma1_src_fmt = rdmaInputFormat;

			DPI0_ConfigColorFormat(dpi0_input_fmt,dpi0_output_fmt);
		}

		if(rdma1_src_height!=config.srcHeight)
		{
		        TVE_MSG("RDMA%d src height change: 0x%x -> 0x%x \n",
		        rdma_idx, rdma1_src_height,config.srcHeight);

		        rdma1_src_height=config.srcHeight;
		}

    }
    else
    {
        if(-1 != tve_dp_mutex_dst)
        {
            //FIXME: release mutex timeout
            TVE_LOG("Stop RDMA1>DPI0\n");
            disp_path_get_mutex_(tve_dp_mutex_dst);

            DISP_REG_SET_FIELD(1 << tve_dp_mutex_dst , DISP_REG_CONFIG_MUTEX_INTEN,  1);
            RDMAStop(1);
            RDMAReset(1);
            disp_path_release_mutex_(tve_dp_mutex_dst);
            //disp_unlock_mutex(dp_mutex_dst);
            tve_dp_mutex_dst = -1;
        }
    }

	if(tve_buffer_dump_on == 7)
	{
    	m4u_do_mva_unmap_kernel(buffer_info.src_phy_addr, mva_size, kernel_va);
	}


    return 0;
}
#endif

#ifdef MTK_TVE_FENCE_SUPPORT
#define FENCE_STEP_COUNTER 1
DEFINE_MUTEX(tve_fence_mutex);
static atomic_t tve_timeline_counter = ATOMIC_INIT(0);
static atomic_t tve_fence_counter = ATOMIC_INIT(0);
static struct sw_sync_timeline *tve_timeline ;
static unsigned int tve_get_fence_counter(void)
{
    return atomic_add_return(FENCE_STEP_COUNTER, &tve_fence_counter);
}



static struct sw_sync_timeline *tve_create_timeline(void)
{
    char name[32];
    const char *prefix = "tve_timeline";
    sprintf(name, "%s", prefix);

    tve_timeline = timeline_create(name);

    if (tve_timeline == NULL)
    {
        printk(" error: cannot create timeline! \n");
    }
    else
    {
        TVE_MSG(" %s() %s\n",__func__, name);
    }

    return tve_timeline;
}

static int tve_create_fence(int *pfence, unsigned int *pvalue)
{
    //int fenceFd = MTK_HDMI_NO_FENCE_FD;
    //struct fence_data data;
    const char *prefix = "tve_fence";

    unsigned int value;
    char name[20];

    *pfence = MTK_TVE_NO_FENCE_FD;

    spin_lock_bh(&tve_lock);
    //data.value = tve_get_fence_counter();
    value = tve_get_fence_counter();
    spin_unlock_bh(&tve_lock);
    //sprintf(data.name, "%s-%d", prefix,  data.value);
    sprintf(name, "%s-%d", prefix,  value);

    if (tve_timeline != NULL)
    {
        int fd = get_unused_fd();
        struct sync_pt *pt;
        struct sync_fence *fence;
        if (fd < 0)
        {
            TVE_MSG(" : could not get a file description! line:%d \n", __LINE__);
            return fd;
        }

        pt = sw_sync_pt_create(tve_timeline, value);
        if (pt == NULL)
        {
            TVE_MSG(" : could not create sync point! line:%d \n", __LINE__);
            put_unused_fd(fd);
            return -ENOMEM;
        }

        name[sizeof(name) - 1] = '\0';
        fence = sync_fence_create(name, pt);
        if (fence == NULL)
        {
            TVE_MSG(" : could not create fence! line:%d \n", __LINE__);
            sync_pt_free(pt);
            put_unused_fd(fd);
            return -ENOMEM;
        }

        sync_fence_install(fence, fd);
        //fenceFd = fd;

        *pfence = fd;
        *pvalue = value;

        //MMProfileLogEx(HDMI_MMP_Events.FenceCreate, MMProfileFlagPulse, fd, value);
        TVE_LOG("%s fd: %d value: %d\n",__func__, fd, value);
    }
    else
    {
        TVE_MSG(" error: there is no Timeline to create Fence! \n");
        return -1;
    }

    return 0;
}



static void tve_release_fence(void)
{
    int inc = atomic_read(&tve_fence_counter) - atomic_read(&tve_timeline_counter);

    if (inc <= 0)
    {
        return ;
    }

    if (tve_timeline != NULL)
    {
        timeline_inc(tve_timeline, inc);
    }

    atomic_add(inc, &tve_timeline_counter);
    //MMProfileLogEx(HDMI_MMP_Events.FenceSignal, MMProfileFlagPulse, atomic_read(&tve_fence_counter), inc);

}

unsigned int tve_timeline_inc(void)
{
    unsigned int fence_cnt, timeline_cnt, inc;

    spin_lock_bh(&tve_lock);
    fence_cnt = atomic_read(&tve_fence_counter);
    timeline_cnt = atomic_read(&tve_timeline_counter);
    inc = fence_cnt - timeline_cnt;
    spin_unlock_bh(&tve_lock);

    if (inc < 0 || inc > 5)
    {
        //if (hdmi_bufferdump_on > 0)
        {
           // MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Timeline_Err, inc);
        }

        TVE_MSG("fence error: inc=%d, fence_cnt=%d, timeline_cnt=%d! \n", inc, fence_cnt, timeline_cnt);
        inc = 0;
    }

    spin_lock_bh(&tve_lock);
    atomic_add(1, &tve_timeline_counter);
    spin_unlock_bh(&tve_lock);
    return atomic_read(&tve_timeline_counter);
}

/**
 * step forward timeline
 * all fence(sync_point) will be signaled prior to it's counter
 * refer to {@link sw_sync_timeline_inc}
 */
static void tve_signal_fence(void)
{
    unsigned inc = 0;

    if (tve_timeline != NULL)
    {
        inc = 1;  ///hdmi_get_timeline_counter_inc();
        timeline_inc(tve_timeline, inc);

        //if (hdmi_bufferdump_on > 0)
        {
        //    MMProfileLogEx(HDMI_MMP_Events.FenceSignal, MMProfileFlagPulse, atomic_read(&tve_timeline_counter), atomic_read(&tve_fence_counter));
        }

        ///TVE_MSG("  %s:%d, tl %d, fd %d\n", hdmi_timeline->obj.name, hdmi_timeline->value, hdmi_timeline, fence_counter);
    }
    else
    {
        TVE_MSG(" no Timeline to inc tl %d, fd %d\n", atomic_read(&tve_timeline_counter), atomic_read(&tve_fence_counter));
    }


}

static void tve_sync_init(void)
{

    ///spin_lock_init(&tve_lock);
    tve_create_timeline();
    // Reset all counter to 0
    atomic_set(&tve_timeline_counter, 0);
    atomic_set(&tve_fence_counter, 0);
}

static void tve_sync_destroy(void)
{

    if (tve_timeline != NULL)
    {
        TVE_MSG(" destroy timeline %s:%d\n", tve_timeline->obj.name, tve_timeline->value);
        timeline_destroy(tve_timeline);
        tve_timeline = NULL;
    }

    // Reset all counter to 0
    atomic_set(&tve_timeline_counter, 0);
    atomic_set(&tve_fence_counter, 0);
}


#endif


void tve_buffer_to_rdma(void)
{
    TVE_KTHREAD_LOG("hdmi_buffer_to_RDMA\n");

    if (!list_empty(&tve_buffer_list)) {
        tve_video_buffer_list *pBuffList = NULL;
		int fence_wait = 0;

        spin_lock_bh(&tve_lock);
        pBuffList = list_first_entry(&tve_buffer_list, tve_video_buffer_list, list);

        while (pBuffList->buf_state != insert_new) {
            if (list_is_last(&pBuffList->list, &tve_buffer_list))
                break;
            pBuffList = list_entry(pBuffList->list.next, tve_video_buffer_list, list);
        }

        spin_unlock_bh(&tve_lock);
        if(pBuffList->fence)
        {
            fence_wait = sync_fence_wait(pBuffList->fence, 1000);
        }

        if ((pBuffList == NULL) || (pBuffList->buf_state != insert_new)
                ||(fence_wait < 0)
                || (tve_rdma_address_config(true, pBuffList->buffer_info) < 0) ) {

            TVE_KTHREAD_LOG(" pBuffList %x buf_state %d fence_wait %d\n", pBuffList,pBuffList->buf_state,fence_wait);

            if ((pBuffList != NULL) && (pBuffList->buf_state == insert_new))
            pBuffList->buf_state = buf_read_done;

        } else {
            spin_lock_bh(&tve_lock);
            pBuffList->buf_state = reg_configed;
            spin_unlock_bh(&tve_lock);
        }
    } else
        TVE_KTHREAD_LOG(" rdma config buffer is NULL\n");
}


void tve_buffer_state_update(void)
{
    int buf_sequence = 0;
    tve_video_buffer_list *pUpdateList = NULL;
    int remove_buffer_cnt = 0;

    if (!list_empty(&tve_buffer_list)) {
        tve_video_buffer_list *pBuffList = NULL;
	tve_video_buffer_list *pNextList = NULL;

        spin_lock_bh(&tve_lock);
        pBuffList = list_first_entry(&tve_buffer_list, tve_video_buffer_list, list);

        while (pBuffList) {
            if (pBuffList->buf_state == insert_new)
             {
                if (!list_is_last(&pBuffList->list, &tve_buffer_list))
                	pNextList = list_entry(pBuffList->list.next, tve_video_buffer_list, list);
                if (pNextList && pNextList->buf_state == insert_new) {
                	pBuffList->buf_state = buf_dropped;
                	pBuffList = pNextList;
                	pNextList = NULL;
                } else
                	break;
             }

            else if (pBuffList->buf_state == reg_configed) {
                buf_sequence++;
                pBuffList->buf_state = reg_updated;
                if (buf_sequence > 1)
                    pUpdateList->buf_state = buf_read_done;
                pUpdateList = pBuffList;

            } else if (pBuffList->buf_state == reg_updated)
                pBuffList->buf_state = buf_read_done;


            if (!list_is_last(&pBuffList->list, &tve_buffer_list))
                pBuffList = list_entry(pBuffList->list.next, tve_video_buffer_list, list);
            else
                pBuffList = NULL;
        }

        pBuffList = NULL;
        pBuffList = list_first_entry(&tve_buffer_list, tve_video_buffer_list, list);
        spin_unlock_bh(&tve_lock);

        while (!list_is_last(&pBuffList->list, &tve_buffer_list))
        {
            if (pBuffList && ((pBuffList->buf_state == buf_read_done) || (pBuffList->buf_state == buf_dropped)))
            {
		if (pBuffList->buffer_info.src_phy_addr != tve_rdma1_addr_using)
                {
                        spin_lock_bh(&tve_lock);
                        if(pBuffList->fence)
                        {
                            sync_fence_put(pBuffList->fence);
                            DISP_PRINTF(DDP_FENCE2_LOG,"fence2: fence put 0x%x\n",
                                        pBuffList->fence);
                            pBuffList->fence = NULL;
                        }

                        list_del(&pBuffList->list);
                        kfree(pBuffList);
                        pBuffList = NULL;
                        spin_unlock_bh(&tve_lock);

                        tve_timeline_inc();
                        tve_signal_fence();

                        remove_buffer_cnt++;

                        spin_lock_bh(&tve_lock);
                        pBuffList = list_first_entry(&tve_buffer_list, tve_video_buffer_list, list);
                        spin_unlock_bh(&tve_lock);
                }
                else
		{
			//pr_err("src_phy_addr(0x%08x) != rdma1_addr_using(0x%08x) so doesn't signal fence",pBuffList->buffer_info.src_phy_addr,rdma1_addr_using);
			break;
		}
            }
            else if(pBuffList && (pBuffList->buf_state == create_new))
            {
                //using_buf_cnt++;
                pBuffList = list_entry(pBuffList->list.next, tve_video_buffer_list, list);
            }
            else
                break;
        }

        if (remove_buffer_cnt > 1)
           TVE_LOG(" remove two buffer one time\n");

    }
}


void tve_remove_buffers(void)
{
    tve_video_buffer_list *pBuffList = NULL;

    while (!list_empty(&tve_buffer_list))
    {
        spin_lock_bh(&tve_lock);
        pBuffList = list_first_entry(&tve_buffer_list, tve_video_buffer_list, list);
        spin_unlock_bh(&tve_lock);

        spin_lock_bh(&tve_lock);
        sync_fence_put(pBuffList->fence);
        list_del(&pBuffList->list);
        kfree(pBuffList);
        pBuffList = NULL;
        spin_unlock_bh(&tve_lock);
    }

    tve_release_fence();
    TVE_LOG("fence stop rdma done\n");
}


static int tve_rdma_config_kthread(void *data)
{
    struct sched_param param = {.sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);

	TVE_FUNC();
    for (;;) {
        wait_event_interruptible(tve_rdma_config_wq, atomic_read(&tve_rdma_config_event));
        atomic_set(&tve_rdma_config_event, 0);

        if (down_interruptible(&tve_update_mutex)) {
            TVE_LOG(" can't get semaphore in\n");
            continue;
        }

        TVE_KTHREAD_LOG("  wake up\n");

        //if (p->is_clock_on == true) /* remove the first head here */
            tve_buffer_to_rdma();

        up(&tve_update_mutex);

        if (kthread_should_stop())
            break;
    }
    return 0;
}


static int tve_rdma_update_kthread(void *data)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);

    for (;;) {
        wait_event_interruptible(tve_rdma_update_wq, atomic_read(&tve_rdma_update_event));
        atomic_set(&tve_rdma_update_event, 0);

        if (down_interruptible(&tve_update_mutex)) {
            TVE_LOG(" can't get semaphore in\n");
            continue;
        }

       // if (p->is_clock_on == true)
            tve_buffer_state_update();
        //else
        //    tve_remove_buffers();

        up(&tve_update_mutex);

        if (kthread_should_stop())
        break;

    }
    return 0;
}

#ifdef MTK_FOR_CVBS_DITHERING
static int tve_filter_kthread(void *data)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);

    for (;;) {
        wait_event_interruptible(tve_filter_wq, atomic_read(&tve_filter_event));
        atomic_set(&tve_filter_event, 0);

        //printk("zhg filter_thread start\n");
        int height = tve_get_disp_height(cvbs_res);
        int width = tve_get_disp_width(cvbs_res);
        int height_end = height - 1;
        int h = 0;
        int w = 0;
        int pitch = width * RDMA1_SRC_BPP;
        tve_rdma1_write_buffer_index = (tve_rdma1_write_buffer_index + 1)%3;
        if(tve_rdma1_write_buffer_index == tve_rdma1_read_buffer_index)
        {
            tve_rdma1_write_buffer_index = (tve_rdma1_write_buffer_index + 2)%3;
        }
        unsigned char *src_temp_addr = (unsigned char *)tve_temp_va + tve_read_buffer_index * TVE_DISP_WIDTH * TVE_DISP_HEIGHT * RDMA1_SRC_BPP;
        unsigned char *tve_rdma1_addr = (unsigned char *)tve_rdma1_va + tve_rdma1_write_buffer_index * TVE_DISP_WIDTH * TVE_DISP_HEIGHT * RDMA1_SRC_BPP;
        memcpy(tve_rdma1_addr, src_temp_addr, pitch);
        for(h = 1; h < height_end; h ++)
        {
            for(w = 0; w < pitch; w ++)
            {
                tve_rdma1_addr[h*pitch+w] = (src_temp_addr[(h-1)*pitch+w]>>2) +
                    (src_temp_addr[h*pitch+w]>>1) + (src_temp_addr[(h+1)*pitch+w]>>2);
            }
        }
        memcpy(tve_rdma1_addr + pitch * height_end, src_temp_addr + pitch * height_end, pitch);
        __cpuc_flush_dcache_area(tve_rdma1_addr,pitch * height);
        tve_rdma1_read_buffer_index = tve_rdma1_write_buffer_index;

        if(-1 != tve_dp_mutex_dst)
        {
            disp_path_get_mutex_(tve_dp_mutex_dst);
            DISP_REG_SET(DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_START_ADDR, tve_rdma1_mva + tve_rdma1_read_buffer_index * TVE_DISP_WIDTH * TVE_DISP_HEIGHT * RDMA1_SRC_BPP);
            disp_path_release_mutex_(tve_dp_mutex_dst);
        }

        if (kthread_should_stop())
        break;

        //printk("zhg filter_thread end\n");

    }
    return 0;
}
#endif


static void tve_rdma1_irq_handler(unsigned int param)
{
   // RET_VOID_IF_NOLOG(!is_hdmi_active());

    ///RET_VOID_IF_NOLOG(!p->lcm_is_video_mode);
    /*
    param 0xf4012004[5:0] rdma interrupt status
    bit 0 register update 0x1
    1 frame start 0x2
    2 frame end   0x4
    3 EOF abnormal 0x8
    4 underflow 0x10
    5 target line 0x20
   */
    TVE_KTHREAD_LOG(" irq status %x\n", param);
    if (param & 0x20) // taget line 0x20
    {
        //MMProfileLogEx(HDMI_MMP_Events.RDMA1RegisterUpdated, MMProfileFlagPulse, param, 0x20);

        ///hdmi_update_buffer_switch();

        atomic_set(&tve_rdma_config_event, 1);
        wake_up_interruptible(&tve_rdma_config_wq);
    }

        if(param&(1<<2))  //frame done
        {
        	//pr_err("enter _rdma1_irq_handler frame done rdma1_addr_using=0x%08x rdma1_addr_shadowed=0x%08x",rdma1_addr_using, rdma1_addr_shadowed);
        	tve_rdma1_addr_using = tve_rdma1_addr_shadowed;
        }

#if 0
    if ((param & 2) && (hdmi_params->cabletype == MHL_SMB_CABLE)) // rdma1 register updated
    {
        //MMProfileLogEx(HDMI_MMP_Events.RDMA1RegisterUpdated, MMProfileFlagPulse, param, 2);

        hdmi_vsync_flag = 1;
        wake_up_interruptible(&hdmi_vsync_wq);

    }
#endif
    if (param & 1) // rdma1 register updated
    {
        //MMProfileLogEx(HDMI_MMP_Events.RDMA1RegisterUpdated, MMProfileFlagPulse, param, 1);

        atomic_set(&tve_rdma_update_event, 1);
        wake_up_interruptible(&tve_rdma_update_wq);

    }
}

//module_init(cvbs_init);
late_initcall(cvbs_init);
module_exit(cvbs_exit);
MODULE_AUTHOR("xuguo,jin<xuguo.jin@mediatek.com>");
MODULE_DESCRIPTION("cvbs Driver");
MODULE_LICENSE("GPL");

