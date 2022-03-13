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
#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
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

#include <mach/devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_boot.h>


#include "tz_cross/trustzone.h"
#include "tz_cross/ta_test.h"
#include "tz_cross/ta_mem.h"
#include "trustzone/kree/system.h"
#include "trustzone/kree/mem.h"
//#include "kree_int.h"

#include "tz_cross/ta_drmkey.h"
#include "tz_cross/keyblock.h"

#include "tz_cross/ta_hdmirx.h"
#include "hdmi_rx_ca.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"



KREE_SESSION_HANDLE ca_hdmirx_handle = NULL;

bool fgCaHDMIRXCreate(void)
{
	TZ_RESULT tz_ret = 0;

    if(get_boot_mode() != FACTORY_BOOT) {
	tz_ret = KREE_CreateSession(TZ_TA_HDMIRX_UUID, &ca_hdmirx_handle);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		// Should provide strerror style error string in UREE.
		printk("Create ca_hdmirx_handle Error: %d\n", tz_ret);
		return FALSE;
	}
	printk("[HDMI]Create ca_hdmirx_handle ok: %d\n", tz_ret);
    }

	
	return TRUE;
}
bool fgCaHDMIRXClose(void)
{
	TZ_RESULT tz_ret = 0;

	tz_ret = KREE_CloseSession(ca_hdmirx_handle);
	if (tz_ret != TZ_RESULT_SUCCESS)
	{
		// Should provide strerror style error string in UREE.
		printk("Close ca_hdmirx_handle Error: %d\n", tz_ret);
		return FALSE;
	}
	printk("[HDMI]Close ca_hdmirx_handle ok: %d\n", tz_ret);
	return TRUE;
}

bool fgCaHDMIRXInstallHdcpKey(unsigned char *pdata,unsigned int u4Len)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[2];
	unsigned char *ptr;
	unsigned int i;
	
	if(ca_hdmirx_handle == NULL)
	{
		printk("[HDMIRX] TEE ca_hdmirx_handle=NULL\n");
		return FALSE ;
	}

	printk("[HDMIRX]fgCaHDMIRxInstallHdcpKey,%d\n",u4Len);

	if(u4Len >= 512)
	{
		return FALSE;
	}
	ptr = (unsigned char *)kmalloc(u4Len, GFP_KERNEL);

	for(i=0;i<u4Len;i++) {
		ptr[i] = pdata[i];
	}

	param[0].mem.buffer = ptr;
	param[0].mem.size = u4Len;
	param[1].value.a = u4Len;
	param[1].value.b = 0;

	
	printk("[HDMIRX]fgCaHDMIRxInstallHdcpKey,L%d\n",__LINE__);
	
	tz_ret = KREE_TeeServiceCall(ca_hdmirx_handle, HDMIRX_TA_INSTALL_HDCP_KEY, 
            TZ_ParamTypes2(TZPT_MEM_INPUT, TZPT_VALUE_INPUT), param);
	
	printk("[HDMIRX]fgCaHDMIRxInstallHdcpKey,L%d\n",__LINE__);
	
	for(i = 0; i < param[0].mem.size; i += 8)
	{
		printk("[fgCaHDMIRXInstallHdcpKeykey0] = %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n", i, \
			((char *)param[0].mem.buffer)[i], ((char *)param[0].mem.buffer)[i+1], ((char *)param[0].mem.buffer)[i+2], \
			((char *)param[0].mem.buffer)[i+3], ((char *)param[0].mem.buffer)[i+4], ((char *)param[0].mem.buffer)[i+5], \
			((char *)param[0].mem.buffer)[i+6], ((char *)param[0].mem.buffer)[i+7]);
	}
	for(i = 0; i < param[1].mem.size; i += 8)
	{
		printk("[fgCaHDMIRXInstallHdcpKey1]key = %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n", i, \
			((char *)param[1].mem.buffer)[i], ((char *)param[1].mem.buffer)[i+1], ((char *)param[1].mem.buffer)[i+2], \
			((char *)param[1].mem.buffer)[i+3], ((char *)param[1].mem.buffer)[i+4], ((char *)param[1].mem.buffer)[i+5], \
			((char *)param[1].mem.buffer)[i+6], ((char *)param[1].mem.buffer)[i+7]);
	}
	if (tz_ret != TZ_RESULT_SUCCESS)
	{
		HDMIRX_LOG("[HDMI]CA HDMI_TA_INSTALL_HDCP_KEY err:%X\n",tz_ret);
		return FALSE;
	}
	
	kfree(ptr);
	return TRUE;
}

#endif
