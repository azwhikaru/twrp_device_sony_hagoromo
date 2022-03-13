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
 /******************************************************************************
*[File]             rx_io.c
*[Version]          v0.1
*[Revision Date]    2009-04-18
*[Author]           Kenny Hsieh
*[Description]
*    source file for hdmi general control routine
*
*
******************************************************************************/
#if 1//(DRV_SUPPORT_HDMI_RX)

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
#include <cust_eint.h>
#include "cust_gpio_usage.h"
#include "mach/eint.h"
#include "mach/irqs.h"

#include <mach/devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_boot.h>

#include "rx_io.h"
#include "hal_io.h"
#include "hdmirx.h"
#include "typedef.h"


void Delay5MS(UINT32 count)
{
	UINT32 u4Index; 	
	for(u4Index=0;u4Index<count;u4Index++)
	{
		msleep(1);
	}
}

void vHalSetRxPort1HPDLevel(BOOL fgHighLevel)
{
	if(fgHighLevel)
	{
#ifdef GPIO_HDMI_HPD_CBUS_RX_PIN
		printk("[enter %s @L%d]fgHighLevel = %d\n",__FUNCTION__,__LINE__,fgHighLevel);
		mt_set_gpio_mode(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_MODE_00);  
		mt_set_gpio_dir(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_OUT_ONE);
#endif
	}
	else
	{
#ifdef GPIO_HDMI_HPD_CBUS_RX_PIN
		printk("[enter %s @L%d]fgHighLevel = %d\n",__FUNCTION__,__LINE__,fgHighLevel);
		mt_set_gpio_mode(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_MODE_00);  
		mt_set_gpio_dir(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_OUT_ZERO);
#endif
	}
}

void vHalSetRxPort2HPDLevel(BOOL fgHighLevel)
{
	if(fgHighLevel)
	{
#ifdef GPIO_HDMI_HPD_CBUS_RX_PIN
		printk("[enter %s]fgHighLevel = %d\n",__FUNCTION__,fgHighLevel);
		mt_set_gpio_mode(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_MODE_00);  
		mt_set_gpio_dir(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_OUT_ONE);
#endif
	}
	else
	{
#ifdef GPIO_HDMI_HPD_CBUS_RX_PIN
		mt_set_gpio_mode(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_MODE_00);  
		mt_set_gpio_dir(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_OUT_ZERO);
#endif
	}
}

#endif//#ifdef DRV_SUPPORT_HDMI_RX
