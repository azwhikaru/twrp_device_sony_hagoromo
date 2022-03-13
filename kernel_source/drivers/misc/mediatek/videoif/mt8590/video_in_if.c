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
#include <generated/autoconf.h>
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
#include <mach/mt_spm_mtcmos.h>

#include <mach/mt_boot.h>
#include "hdmi_rx_ctrl.h"
#include "vin_drv_if.h"

#define VSW_DEVNAME    "vsw"
size_t vsw_log_on = 0x800;


#undef OUTREG32
#define OUTREG32(x, y) {__OUTREG32((x),(y))}
#define __OUTREG32(x,y) {*(unsigned int*)(x)=(y);}

#define RETIF(cond, rslt)             if ((cond)){VSW_LOG("return in %d\n",__LINE__);return (rslt);}
#define RET_VOID_IF_NFY(cond)         if ((cond)){VSW_LOG("return in %d\n",__LINE__);return;}
#define RETIF_NOLOG(cond, rslt)       if ((cond)){return (rslt);}
#define RET_VOID_IF_NOLOG(cond)       if ((cond)){return;}
#define RETIFNOT(cond, rslt)          if (!(cond)){VSW_LOG("return in %d\n",__LINE__);return (rslt);}

#define ALIGN_TO(x, n)  \
    (((x) + ((n) - 1)) & ~((n) - 1))


static BOOL vsw_drv_init_context(void);

static VSW_DRIVER *vsw_drv = NULL;
static dev_t vsw_devno;
static struct cdev *vsw_cdev;
static struct class *vsw_class = NULL;

static struct switch_dev vsw_switch_data;

static char *_vsw_ioctl_spy(unsigned int cmd)
{
	switch(cmd)
	{
		case MTK_SET_VIDEO_IN_SRC_TYPE:
			return "MTK_SET_VIDEO_IN_SRC_TYPE";
			break;

		case MTK_GET_HDMI_RX_EDID:
			return "MTK_GET_HDMI_RX_EDID";
			break;

		case MTK_GET_HDMI_IN_PWR_STATUS:
			return "MTK_GET_HDMI_IN_PWR_STATUS";
			break;

		case MTK_SET_HDMI_RX_MODE_STATUS:
			return "MTK_SET_HDMI_RX_MODE_STATUS";
			break;

		case MTK_SET_HDMI_RX_EDID_VALUE:
			return "MTK_SET_HDMI_RX_EDID_VALUE";
			break;

		case MTK_GET_VIDEO_IN_CONFIG_INFO:
			return "MTK_GET_VIDEO_IN_CONFIG_INFO";
			break;

		case MTK_SET_VIDEO_IN_START_CMD:
			return "MTK_SET_VIDEO_IN_START_CMD";
			break;

		case MTK_SET_VIDEO_IN_STOP_CMD:
			return "MTK_SET_VIDEO_IN_STOP_CMD";
			break;

		case MTK_SET_VIDEO_IN_REQBUFS:
			return "MTK_SET_VIDEO_IN_REQBUFS";
			break;

		case MTK_SET_VIDEO_IN_INITBUF:
			return "MTK_SET_VIDEO_IN_INITBUF";
			break;

		case MTK_SET_VIDEO_IN_DQBUF:
			return "MTK_SET_VIDEO_IN_DQBUF";
			break;

		case MTK_SET_VIDEO_IN_QBUF:
			return "MTK_SET_VIDEO_IN_QBUF";
			break;

		case MTK_SET_HDMIRX_POWER_ENABLE:
			return "MTK_SET_HDMIRX_POWER_ENABLE";
			break;

		case MTK_SET_HDMIRX_POWER_DISABLE:
			return "MTK_SET_HDMIRX_POWER_DISABLE";
			break;

		case MTK_HDMI_RX_HDCP_KEY:
			return "MTK_HDMI_RX_HDCP_KEY";
			break;

		case MTK_VIDEO_IN_STREAMON:
			return "MTK_VIDEO_IN_STREAMON";
			break;

		case MTK_VIDEO_IN_STREAMOFF:
			return "MTK_VIDEO_IN_STREAMOFF";
			break;

		case MTK_SET_HDMI_RX_AUD_MODE:
			return "MTK_SET_HDMI_RX_AUD_MODE";
			break;

		case MTK_GET_HDMI_RX_AUD_INFO:
			return "MTK_GET_HDMI_RX_AUD_INFO";
			break;

		case MTK_ENABLE_HDMI_RX_AUD_TASK:
			return "MTK_ENABLE_HDMI_RX_AUD_TASK";
			break;
		default:
			return "unknown ioctl command";
			break;
	}

}

void vsw_state_callback(VSW_STATE state)
{
    static VSW_STATE g_cur_state;
    RET_VOID_IF_NFY(g_cur_state == state);
	VSW_LOG("%s, state = %d vsw_switch_data.state=%d, @L%d\n", __func__, state, vsw_switch_data.state, __LINE__);
    switch(state)
    {
        case HDMI_RX_STATE_PWER_5V_STATUS:
            switch_set_state(&vsw_switch_data, HDMI_RX_STATE_PWER_5V_STATUS);
            break;

        case HDMI_RX_STATE_EDID_STATUS:
            switch_set_state(&vsw_switch_data, HDMI_RX_STATE_EDID_STATUS);
            break;

		case HDMI_RX_INFOFRAME_NOTIFY:
            switch_set_state(&vsw_switch_data, HDMI_RX_INFOFRAME_NOTIFY);
		break;

		case HDMI_RX_TIMING_STATUS_LOCK:
			switch_set_state(&vsw_switch_data, HDMI_RX_TIMING_STATUS_LOCK);
			break;

		case HDMI_RX_TIMING_STATUS_UNLOCK:
			switch_set_state(&vsw_switch_data, HDMI_RX_TIMING_STATUS_UNLOCK);
			break;

		case HDMI_RX_RESOLUTION_CHANGEING:
			switch_set_state(&vsw_switch_data, HDMI_RX_RESOLUTION_CHANGEING);
			break;

		case HDMI_RX_RESOLUTION_CHANGE_DONE:
			switch_set_state(&vsw_switch_data, HDMI_RX_RESOLUTION_CHANGE_DONE);
			break;

		case HDMI_RX_ASR_INFO_CHANGE:
			switch_set_state(&vsw_switch_data, HDMI_RX_ASR_INFO_CHANGE);
			break;

		case HDMI_RX_CSP_INFO_CHANGE:
			switch_set_state(&vsw_switch_data, HDMI_RX_CSP_INFO_CHANGE);
			break;

		case VIDEO_IN_QUEUE_BUFFER_FULL_NOTIFY:
			switch_set_state(&vsw_switch_data, VIDEO_IN_QUEUE_BUFFER_FULL_NOTIFY);
			break;

		case VIDEO_IN_QUEUE_BUFFER_NULL_NOTIFY:
			switch_set_state(&vsw_switch_data, VIDEO_IN_QUEUE_BUFFER_NULL_NOTIFY);
			break;

		case VIDEO_IN_SEND_BUFFER_TO_HW_NOTIFY:
			switch_set_state(&vsw_switch_data, VIDEO_IN_SEND_BUFFER_TO_HW_NOTIFY);
			break;

		case HDMI_RX_AUD_STATUS_LOCK:
			switch_set_state(&vsw_switch_data, HDMI_RX_AUD_STATUS_LOCK);
			break;

		case HDMI_RX_AUD_STATUS_UNLOCK:
			switch_set_state(&vsw_switch_data, HDMI_RX_AUD_STATUS_UNLOCK);
			break;

        default:
            printk("[hdmi rx]%s, hdmirx_state not support\n", __func__);
            break;
    }
    g_cur_state = state;
}

static int vsw_release(struct inode *inode, struct file *file)
{
    return 0;
}

static int vsw_open(struct inode *inode, struct file *file)
{
    return 0;
}

void vsw_log_enable(unsigned short enable)
{
    printk("hdmirx log 0x%x\n", enable);
    vsw_log_on = enable;
    //vsw_drv->log_enable(enable);
}
void hdmirx_power_on(void)
{
	vsw_drv->hdmirx_power_on();
}
void hdmirx_power_off(void)
{
	vsw_drv->hdmirx_power_off();
}

static long hdmi_repeater_ioctl_setting(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	unsigned long pv_get_pwr_info;
	HDMI_RX_EDID_T pv_set_edid_info;
	VID_VDOIN_ALL_INFO_T VideoInCfgInfo;
	VIDEO_IN_SRC_ID_E eSrcId;
	VIDEO_IN_REQBUF videoInReqBuf;
	VIDEO_IN_BUFFER_INFO videoInBuffer;
	UINT32 videoInBufferIndex;
	HDMIRX_HDCP_KEY rxkey;
	HDMI_IN_PARAM_T hdmiin_param;
	HDMI_IN_AUD_PARAM_T hdmiinaud_param;
	bool                enrxaudtask;
    printk("[vsw] vsw ioctl= %s(%d), arg = %lu\n", _vsw_ioctl_spy(cmd), cmd & 0xff, arg);

    switch (cmd)
	{
	 case MTK_SET_VIDEO_IN_SRC_TYPE:
		 VSW_LOG("MTK_SET_HDMI_REPEATER_STATUS \r\n");
		if (copy_from_user(&eSrcId, (void __user *)arg, sizeof(eSrcId)))
		{
			VSW_LOG("copy_from_user failed! line:%d \n", __LINE__);
			ret = -EFAULT;
		}
		else
			vsw_drv->setvideoinsrcid(eSrcId);
		 break;

	 case MTK_GET_HDMI_RX_EDID:
		 VSW_LOG("MTK_GET_HDMI_RX_EDID \r\n");
		 vsw_drv->gethdmirxedid(&pv_set_edid_info);
		if (copy_to_user((void __user *)arg, &pv_set_edid_info, sizeof(pv_set_edid_info)))
		{
			VSW_LOG("copy_from_user failed! line:%d \n", __LINE__);
			ret = -EFAULT;
		}
		break;

	case MTK_SET_HDMI_RX_EDID_VALUE:
		VSW_LOG("MTK_SET_HDMI_RX_EDID_VALUE \r\n");
		if (copy_from_user(&pv_set_edid_info, (void __user *)arg, sizeof(pv_set_edid_info)))
		{
			VSW_LOG("copy_from_user failed! line:%d \n", __LINE__);
			ret = -EFAULT;
		}
		else
			vsw_drv->sethdmirxedidvalue(&pv_set_edid_info);
	break;

	 case MTK_GET_HDMI_IN_PWR_STATUS:
		 VSW_LOG("MTK_GET_HDMI_IN_PWR_STATUS \r\n");
		 vHdmiRxPwrStatus(&pv_get_pwr_info);
		 if (copy_to_user((void __user *)arg, &pv_get_pwr_info, sizeof(pv_get_pwr_info)))
		 {
			 VSW_LOG("copy_from_user failed! line:%d \n", __LINE__);
			 ret = -EFAULT;
		 }
		 break;

	case MTK_SET_HDMI_RX_MODE_STATUS:
		if (copy_from_user(&hdmiin_param, (void *)arg, sizeof(hdmiin_param)))
		{
			ret = -EFAULT;
		}
		else
 		vsw_drv->sethdmirxmode(hdmiin_param.rxmode,hdmiin_param.rxswitchport);
		break;
	case  MTK_SET_HDMI_RX_AUD_MODE:
		if (copy_from_user(&hdmiinaud_param, (void *)arg, sizeof(hdmiinaud_param)))
		{
			ret = -EFAULT;
		}
		else
		vsw_drv->sethdmirxaudmode(hdmiinaud_param.rxaudmode);
		break;

       case MTK_ENABLE_HDMI_RX_AUD_TASK:
       	     	if (copy_from_user(&enrxaudtask, (void *)arg, sizeof(enrxaudtask)))
		{
			ret = -EFAULT;
		}
		else
		vsw_drv->sethdmrxaudtask(enrxaudtask);
		break;
	case  MTK_GET_HDMI_RX_AUD_INFO:
		HDMIRX_LOG("MTK_GET_HDMI_RX_AUD_INFO \r\n");
		audio_para.chsts[0] = u1HalHDMIRxAudioCHSTAT0();
		audio_para.chsts[1] = u1HalHDMIRxAudioCHSTAT1();
		audio_para.chsts[2] = u1HalHDMIRxAudioCHSTAT2();
		audio_para.chsts[3] = u1HalHDMIRxAudioCHSTAT3();
		audio_para.chsts[4] = u1HalHDMIRxAudioCHSTAT4();
		printk("chsts0= 0x%x.\n",audio_para.chsts[0]);
		printk("chsts1= 0x%x.\n",audio_para.chsts[1]);
		printk("chsts2= 0x%x.\n",audio_para.chsts[2]);
		printk("chsts3= 0x%x.\n",audio_para.chsts[3]);
		printk("chsts4= 0x%x.\n",audio_para.chsts[4]);
		if (copy_to_user((void __user *)arg, &audio_para, sizeof(audio_para)))
		{
			HDMIRX_LOG("copy_from_user failed! line:%d \n", __LINE__);
			ret = -EFAULT;
		}
		break;

	case MTK_GET_VIDEO_IN_CONFIG_INFO:
		VSW_LOG("MTK_GET_VIDEO_IN_CONFIG_INFO\n");
		if (copy_from_user(&VideoInCfgInfo, (void *)arg, sizeof(VID_VDOIN_ALL_INFO_T)))
		{
			ret = -EFAULT;
		}
		video_in_config_info_get(&VideoInCfgInfo);
		if (copy_to_user((void *)arg,(void *)&VideoInCfgInfo, sizeof(VID_VDOIN_ALL_INFO_T)))
		{
			ret = -EFAULT;
		}
		break;

	case MTK_SET_VIDEO_IN_START_CMD:
 		vsw_drv->videoinstart();
		break;

	case MTK_SET_VIDEO_IN_STOP_CMD:
 		vsw_drv->videoinstop();
		break;

	case MTK_SET_VIDEO_IN_REQBUFS:
		if (copy_from_user(&videoInReqBuf, (void __user *)arg, sizeof(VIDEO_IN_REQBUF))) {
			ret = -EFAULT;
		} else {

			// Request video in's information
			vsw_drv->vRequestBuffer(&videoInReqBuf);
			// Copy the actual video in's information to user space
			if (copy_to_user((void __user *)arg, &videoInReqBuf, sizeof(VIDEO_IN_REQBUF))) {
				ret = -EFAULT;
			}
		}
	   	break;

	case MTK_SET_VIDEO_IN_QBUF:
		if (copy_from_user(&videoInBuffer, (void __user *)arg, sizeof(VIDEO_IN_BUFFER_INFO))) {
			ret = -EFAULT;
		} else {
		    VSW_LOG("videoInBuffer.u4BufIndex = %d, %s()\n",videoInBuffer.u4BufIndex, __FUNCTION__);
			vsw_drv->vQBuf(videoInBuffer.u4BufIndex);
		}
		break;

	case MTK_SET_VIDEO_IN_DQBUF:
		vsw_drv->vDQBuf(&videoInBuffer);
		if (copy_to_user((void __user *)arg, &videoInBuffer, sizeof(VIDEO_IN_BUFFER_INFO))) {
			ret = -EFAULT;
		}
		break;

	case MTK_SET_VIDEO_IN_INITBUF:
		if (copy_from_user(&videoInBuffer, (void __user *)arg, sizeof(VIDEO_IN_BUFFER_INFO))) {
			ret = -EFAULT;
		} else {
			vsw_drv->vInitBuffer(&videoInBuffer);
		}
		break;
    case MTK_VIDEO_IN_STREAMON:
        vsw_drv->vStreamOn();
        break;
    case MTK_VIDEO_IN_STREAMOFF:
        vsw_drv->vStreamOff();
        break;

	case MTK_SET_HDMIRX_POWER_ENABLE:
		hdmirx_power_on();
		break;

	case MTK_SET_HDMIRX_POWER_DISABLE:
		hdmirx_power_off();
		break;

	case MTK_HDMI_RX_HDCP_KEY:
	{
		if (copy_from_user(&rxkey, (void __user *)arg, sizeof(rxkey))) {
			VSW_LOG("copy_from_user failed! line:%d \n", __LINE__);
			ret = -EFAULT;
		} else {
			hdmirx_hdcpkey((unsigned char*)&rxkey);
		}
		break;
	}

	 default:
		 break;
	}
    VSW_LOG("ioctl finished\n");
	return ret;

}

static long vsw_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long r;

    r = hdmi_repeater_ioctl_setting(file, cmd, arg);

    return r;
}

extern int mt_hdmirx_delete_attr(struct device *dev);

static int vsw_remove(struct platform_device *pdev)
{

   int err;
   err = mt_hdmirx_delete_attr(&(pdev->dev));
   if (err)
	   printk("delete attr\n");
    return 0;
}


static BOOL vsw_drv_init_context(void)
{

    VSW_LOG("%s\n", __func__);
    static const VSW_UTIL_FUNCS vsw_utils =
    {
        .state_callback         = vsw_state_callback,
    };

	if (vsw_drv != NULL)
	{
		return TRUE;
	}

    vsw_drv = (VSW_DRIVER*)VSW_GetDriver();

	if (NULL == vsw_drv)
	{
		return FALSE;
	}

    vsw_drv->setvsw_util_funcs(&vsw_utils);
	return TRUE;
}

struct file_operations vsw_fops =
{
    .owner   = THIS_MODULE,
    .unlocked_ioctl   = vsw_ioctl,
    .open    = vsw_open,
    .release = vsw_release,
};
extern int mt_hdmirx_create_attr(struct device *dev);

static int vsw_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct class_device *class_dev = NULL;

    VSW_LOG("%s\n", __func__);

    /* Allocate device number for hdmiRX driver */
    ret = alloc_chrdev_region(&vsw_devno, 0, 1, VSW_DEVNAME);

    if (ret)
    {
        printk("[hdmirx]alloc_chrdev_region fail\n");
        return -1;
    }

    /* For character driver register to system, device number binded to file operations */
    vsw_cdev = cdev_alloc();
    vsw_cdev->owner = THIS_MODULE;
    vsw_cdev->ops = &vsw_fops;
    ret = cdev_add(vsw_cdev, vsw_devno, 1);

    /* For device number binded to device name(hdmirx), one class is corresponeded to one node */
    vsw_class = class_create(THIS_MODULE, VSW_DEVNAME);
    /* mknod /dev/hdmirx */
    class_dev = (struct class_device *)device_create(vsw_class, NULL, vsw_devno, NULL, VSW_DEVNAME);
    ret = mt_hdmirx_create_attr(&(pdev->dev));
	if (ret)
		printk("create attribute\n");

    if (!vsw_drv_init_context())
    {
        printk("%s, vsw_drv_init_context fail\n", __func__);
        return 1;
    }

    VSW_LOG("[%s] current=0x%08x\n", __func__, (unsigned int)current);

    return 0;
}

static struct platform_driver vsw_driver =
{
    .probe  = vsw_probe,
    .remove = vsw_remove,
    .driver = { .name = VSW_DEVNAME }
};

static int __init vsw_init(void)   //include hdmi rx && video in
{
    int err_code = 0;

    VSW_LOG("%s\n", __func__);

    HDMIRX_DBG_Init();

    if (platform_driver_register(&vsw_driver))
    {
        printk("[vsw]failed to register vsw driver\n");
        return -1;
    }

    if (!vsw_drv_init_context())
    {
        printk("[hdmirx]%s, vsw_drv_init_context fail\n", __func__);
        return 1;
    }

    vsw_drv->hdmirxinit();
	vsw_drv->videoinit();

    vsw_switch_data.name = "rx_hdmi_vsw";
    vsw_switch_data.index = 0;
    vsw_switch_data.state = 0;
    err_code = switch_dev_register(&vsw_switch_data);

	if (err_code)
	{
		printk("switch_dev_register returned:%d!\n", err_code);
		return 1;
	}

    return err_code;
}

static void __exit vsw_exit(void)
{
    device_destroy(vsw_class, vsw_devno);
    class_destroy(vsw_class);
    cdev_del(vsw_cdev);
    unregister_chrdev_region(vsw_devno, 1);
}

module_init(vsw_init);
module_exit(vsw_exit);
MODULE_AUTHOR("MingMing.FU <mingming.fu@mediatek.com>");
MODULE_DESCRIPTION("VDIEO SWITCH Driver");
MODULE_LICENSE("GPL");

