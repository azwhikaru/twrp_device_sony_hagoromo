/*
 * Copyright 2016 Sony Corporation
 * File Changed on 2016-10-17
 */
/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <media/rc-core.h>
#include <linux/rtpm_prio.h>//  scher_rr
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/earlysuspend.h>

#include "mtk_ir_regs.h"
#include "mtk_ir_core.h"
#include "mtk_ir_common.h"
#include <mach/mt_clkmgr.h>
#include <mach/mt_irq.h>
#include <mach/memory.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>
#include <mach/mt_boot_common.h>
#include <mach/mt_pm_ldo.h>

#include <linux/timer.h>

#if MTK_IR_DEVICE_NODE
#include "mtk_ir_dev.h"
#endif
#define TWO_CONSECUTIVE_TIMES_CHECK

static int  mtk_ir_core_probe(struct platform_device *pdev);
static int  mtk_ir_core_remove(struct platform_device *pdev);
static int  mt_ir_core_power_enable(void);

static irqreturn_t mtk_ir_core_irq(int irq, void *dev_id);
static int mtk_ir_core_register_swirq(int trigger_type);
static void mtk_ir_core_free_swirq(void);
extern BOOTMODE get_boot_mode(void);
extern int isFromSuspend(void);
extern void fromSuspendFlag(int value);
#ifdef TWO_CONSECUTIVE_TIMES_CHECK
static void rc_keydown_2consecutive_check(struct rc_dev *dev, int scancode, u8 toggle,
                                          struct mtk_ir_core_platform_data * pdata );
#endif /* TWO_CONSECUTIVE_TIMES_CHECK */

#define LIRCBUF_SIZE 6

int ir_log_debug_on = 0; // for IR_LOG_DEBUG(...) log on or off

static char * mode = "normal";

/*
record last interrupt hang_up place for debug question
*/

static char   last_hang_place[64] ={0};


static volatile u32 _u4Curscancode = BTN_NONE;
static spinlock_t scancode_lock;

static struct timer_list g_ir_timer; // used for check pulse cnt register for workaound to solve clear ir stat
#define TIMER_PERIOD HZ    // 1s

static BOOL timer_log = FALSE;
static u32 detect_hung_timers = 0;

LIST_HEAD(mtk_ir_device_list);
#ifdef TWO_CONSECUTIVE_TIMES_CHECK
static int prev_scancode = 0;
static int curr_scancode = 0;
static BOOL isConsecutiveCheckingPressed = FALSE;
static struct timer_list expire_check_timer;
static spinlock_t saved_scancode_lock;
#endif /* TWO_CONSECUTIVE_TIMES_CHECK */


struct mtk_rc_core mtk_rc_core = {
 .irq = MT_IRRX_IRQ_ID,
 .irq_register = false,
 .rcdev = NULL,
 .dev_current = NULL,
 .drv = NULL,
 .k_thread = NULL,
 #if MTK_IRRX_AS_MOUSE_INPUT
 .p_devmouse = NULL,
 #endif
};

static struct resource mtk_ir_resource[] = {
    {
        .start  = IRRX_BASE_PHY,
        .end    = IRRX_BASE_PHY_END,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = MT_IRRX_IRQ_ID,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device mtk_ir_dev_parent = {
	
	.name	  = MTK_IR_DRIVER_NAME,
	.id		  = -1,// here must be -1
	.num_resources  = ARRAY_SIZE(mtk_ir_resource),
    .resource       = mtk_ir_resource,
	.dev = {
	          .release       = release,
	        },
};


void release(struct device *dev)
{
    return;
}


MTK_IR_MODE mtk_ir_core_getmode(void)
{
   BOOTMODE boot_mode = get_boot_mode();
   IR_LOG_ALWAYS("bootmode = %d\n", boot_mode);

   if ((boot_mode == NORMAL_BOOT) ||
   	   (boot_mode == RECOVERY_BOOT))
   	{
		
   	   return MTK_IR_NORMAL;
   	}
    else if (boot_mode == FACTORY_BOOT)
   	{
   	  return MTK_IR_FACTORY;
   	}
	
    #if 0
	if (strcmp(mode,"factory") == 0)
	{
		return MTK_IR_FACTORY;
	}
	else if (strcmp(mode,"normal") == 0)
	{
		return MTK_IR_NORMAL;
	}
	#endif

	return MTK_IR_MAX;

}


#ifdef CONFIG_PM_SLEEP

static int mtk_ir_core_suspend(struct device *dev)
{

	int ret = 0;
	struct platform_device *pdev = to_platform_device(dev);
	struct mtk_ir_core_platform_data *pdata = (struct mtk_ir_core_platform_data *)(dev->platform_data);

	if (-1 == pdev->id) //  this is the parent device, so ignore it
	{
		ret = 0;
		goto end;
 	}

	if (!pdata || !(pdata->suspend) || !(pdata->resume))
 	{
 		IR_LOG_ALWAYS("%s, suspend arg wrong\n",pdata->input_name);
		ret = -1;
		goto end;
	}

	IO_WRITE32(0xF0001000,0x30,0x1<<9);
	IO_WRITE32(0xF0001000,0x30,0x0<<9);
	ret = pdata->init_hw();
	if (ret)
	{
		IR_LOG_ALWAYS(" fail to init_hw for ir_dev(%s) ret = %d!!!\n",
								pdata->input_name, ret);
		goto end;
	}

	ret = pdata->suspend(NULL);
	IR_LOG_ALWAYS("ret(%d)\n",ret);

end:

	return 0;
}

static int mtk_ir_core_resume(struct device *dev)
{
	int ret = 0;
	struct platform_device *pdev = to_platform_device(dev);
	struct mtk_ir_core_platform_data *pdata = (struct mtk_ir_core_platform_data *)(dev->platform_data);

	if (-1 == pdev->id)
	{
		ret = 0;
		goto end;
 	}

	if (!pdata || !(pdata->suspend) || !(pdata->resume))
 	{
 		IR_LOG_ALWAYS("%s, resume arg wrong\n",pdata->input_name);
		ret = -1;
		goto end;
	}

	ret = pdata->resume(NULL);
	IR_LOG_ALWAYS("ret(%d)\n",ret);

end:

	return 0;
}
#endif

static const struct dev_pm_ops mtk_ir_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(mtk_ir_core_suspend, mtk_ir_core_resume)

};


static struct platform_driver mtk_ir_core_driver = {
	.probe  = mtk_ir_core_probe,
	.remove = mtk_ir_core_remove,
	.driver = {
		.name   = MTK_IR_DRIVER_NAME,
		.owner  = THIS_MODULE,
		.pm	= &mtk_ir_pm_ops,
	},
};

#define IR_ASSERT_DEBUG 1

// for Assert to debug  ir driver code
void AssertIR(const char* szExpress, const char* szFile, int i4Line)
{
	printk(IR_LOG_TAG"\nAssertion fails at:\nFile: %s, line %d\n\n", szFile, (int)(i4Line));
	printk(IR_LOG_TAG"\t%s\n\n", szExpress);

#if IR_ASSERT_DEBUG
	dump_stack();
	panic("%s", szExpress);
#endif
}


// enable or disable clock
int mt_ir_core_clock_enable(void)
{
    IR_LOG_ALWAYS("enable IRRX clock\n");
	return enable_clock(MT_CG_INFRA_IRRX,"IRRX");
}

int mt_ir_core_clock_disable(void)
{
    IR_LOG_ALWAYS("disable IRRX clock\n");
	return disable_clock(MT_CG_INFRA_IRRX,"IRRX");
}

int mt_ir_core_power_enable(void)
{
    	IR_LOG_ALWAYS("mt_ir_core_power_enable\n");
		#ifdef MTK_PMIC_MT6397
		hwPowerOn(MT65XX_POWER_LDO_VMCH, VOL_3300, "NFI");
		#else
		hwPowerOn(MT6323_POWER_LDO_VMCH, VOL_3300, "NFI");
		#endif
		return 0;
}


// when write ir register ,for debug info
void mtk_ir_core_reg_write(u16 ui2Regs, u32 u4Data)
{
  // IR_W_REGS_LOG("[W]addr = 0x%04x, data = 0x%08x\n", IRRX_BASE_VIRTUAL + ui2Regs, u4Data);
   IO_WRITE32(IRRX_BASE_VIRTUAL, ui2Regs, u4Data);
   /*
   IR_LOG_ALWAYS("[R]addr = 0x%04x, data = 0x%08x\n",IRRX_BASE_VIRTUAL + ui2Regs,
   	                     IO_READ32(IRRX_BASE_VIRTUAL,ui2Regs ));
   	                     */
}


#define SPRINTF_DEV_ATTR(fmt, arg...)  \
	do { \
		 temp_len = sprintf(buf,fmt,##arg);	 \
		 buf += temp_len; \
		 len += temp_len; \
	}while (0)



static ssize_t mtk_ir_core_show_info(struct device *dev,
	                                         struct device_attribute *attr, char *buf)
{
   struct attribute *pattr = &(attr->attr);

   int len = 0;
   int temp_len = 0;
   u32 vregstart = IRRX_BASE_VIRTUAL;
   struct mtk_ir_core_platform_data * pdata = NULL;

   if (strcmp(pattr->name, "switch_dev") == 0)
   {
	   len = sprintf(buf,"used to switch ir device\n");
   }
   if (strcmp(pattr->name, "debug_log") == 0)
   {
      SPRINTF_DEV_ATTR("0: debug_log off \n 1: debug_log on \n");
   	  //len = sprintf(buf,"0: debug_log off \n 1: debug_log on \n");
   	  SPRINTF_DEV_ATTR("cur debug_log (%d)\n ",ir_log_debug_on);
   }
   if (strcmp(pattr->name, "register") == 0) // here used to dump register
   {
      SPRINTF_DEV_ATTR("-------------dump ir register-----------\n");

      for (; vregstart <=IRRX_BASE_VIRTUAL_END;)
      {
      	 SPRINTF_DEV_ATTR("0x%08x = 0x%08lx\n",vregstart,REGISTER_READ32(vregstart));
		 vregstart += 4;
      }
//	  SPRINTF_DEV_ATTR("0x%08x = 0x%08lx\n",0xf0005630,REGISTER_READ32(0xf0005630));
   }

   #if MTK_IR_DEVICE_NODE
   if (strcmp(pattr->name, "log_to") == 0)
   {

	  SPRINTF_DEV_ATTR("useage:\n0: log_to kernel  \n1: var netlink log_to userspace \n");
	  SPRINTF_DEV_ATTR("log_to=(%d)\n", mtk_ir_get_log_to());
	  mtk_ir_dev_show_info(&buf, &len);

   }
   #endif

    if (strcmp(pattr->name, "hungup") == 0)
	{
	
		SPRINTF_DEV_ATTR("%s \n",last_hang_place);

	}

	if (strcmp(pattr->name, "cuscode") == 0)
	{

		if (mtk_rc_core.dev_current != NULL)
		{
		    pdata = mtk_rc_core.dev_current->dev.platform_data;
			SPRINTF_DEV_ATTR("read cuscode(0x%08x) \n", pdata->get_customer_code());
		}
	}

	if (strcmp(pattr->name, "timer") == 0)
	{

		SPRINTF_DEV_ATTR("timer_log = %s\n", timer_log == FALSE?"FALSE":"TRUE");
    SPRINTF_DEV_ATTR("detect_hung_timers = %d\n",detect_hung_timers);
	}


   return len;
}


static ssize_t mtk_ir_core_store_debug(struct device *dev,
                                            struct device_attribute *attr,
		                                    const char *buf, size_t count)
{
   char * buffdata[24];
   int val ;
   struct attribute *pattr = &(attr->attr);
   memset(buffdata,0,24);

   if (strcmp(pattr->name, "clock") == 0)
   {
     sscanf(buf, "%d", &val);

   	 if (val)//enable clock
   	 {
   	 	mt_ir_core_clock_enable();
   	 }
	 else
	 {
	    mt_ir_core_clock_disable();
	 }

   }

   if (strcmp(pattr->name, "swirq") == 0)
   {
   	 sscanf(buf, "%d", &val);
	 if (1 == val)
	 {
		mtk_ir_core_register_swirq(IRQF_TRIGGER_LOW);
	 }
	 else if (2 == val)
	 {
	 	mtk_ir_core_register_swirq(IRQF_TRIGGER_HIGH);
	 }
	 else
	 {
	 	mtk_ir_core_free_swirq();
	 }

   }
   if (strcmp(pattr->name, "hwirq") == 0)
   {
   	 sscanf(buf, "%d", &val);
	 if (val)
	 {
	 	mtk_ir_core_clear_hwirq_stat();
	 }
   }
   return count;

}



static ssize_t mtk_ir_core_store_info(struct device *dev,
                                            struct device_attribute *attr,
		                                    const char *buf, size_t count)
{
    int var;
	u32 reg;
	unsigned long val;

	struct list_head *list_cursor;
    struct mtk_ir_device *entry;
	struct mtk_ir_core_platform_data * pdata;
	struct platform_device *pdev;
	struct attribute *pattr = &(attr->attr);	

   if (strcmp(pattr->name, "switch_dev") == 0)
   {
       sscanf(buf, "%d", &var);
	   goto switch_device_process;
   }

   if (strcmp(pattr->name, "debug_log") == 0)
   {
     sscanf(buf, "%d", &var);
   	 ir_log_debug_on = var;
	 return count;
   }

   if (strcmp(pattr->name, "register") == 0)
   {
	 sscanf(buf, "%x %lx", &reg , &val);
	 printk("write reg(0x%08x) =  val(0x%08lx)\n", reg, val);
	 REGISTER_WRITE32(reg, val);
	 printk("read  reg(0x%08x) =  val(0x%08lx)\n", reg, REGISTER_READ32(reg));
	 return count;
   }

   #if MTK_IR_DEVICE_NODE
   if (strcmp(pattr->name, "log_to") == 0)
   {
     sscanf(buf, "%d", &var);
     mtk_ir_set_log_to(var);
   	 return count;
   }
   #endif

   if (strcmp(pattr->name, "press_timeout") == 0)
   {
      sscanf(buf, "%d", &var);
 	 if (mtk_rc_core.dev_current != NULL)
 	 {
 	 	pdata = mtk_rc_core.dev_current->dev.platform_data;
		pdata->i4_keypress_timeout = var;
		IR_LOG_ALWAYS("%s, i4_keypress_timeout = %d\n ",pdata->input_name,pdata->i4_keypress_timeout);
		rc_set_keypress_timeout(pdata->i4_keypress_timeout);
 	 }
   	 return count;
   }


   if (strcmp(pattr->name, "cuscode") == 0)
	 {
	    sscanf(buf, "%x",&reg);
	    printk("write cuscode(0x%08x) \n", reg);

		if (mtk_rc_core.dev_current != NULL)
		{
			pdata = mtk_rc_core.dev_current->dev.platform_data;
			pdata->set_customer_code(&reg);
			printk("read cuscode(0x%08x)\n", pdata->get_customer_code());
		}
	    return count;
	 }


   if (strcmp(pattr->name, "timer") == 0)
   {
     sscanf(buf, "%d", &var);   //sscanf(buf, "%d", &val);  for klockwork issue
     if (var)
	 {
	 	timer_log = TRUE;
	 }
	 else
	 {
	 	timer_log = FALSE;
	 }
	 printk("timer_log = %s\n", timer_log == FALSE?"FALSE":"TRUE");
    return count;
   }


 switch_device_process:

	if (!list_empty(&mtk_ir_device_list))
    {
      list_for_each(list_cursor,&mtk_ir_device_list)
	  {
		entry = list_entry(list_cursor, struct mtk_ir_device, list);
		pdev = &(entry->dev_platform);
		if (var == (pdev->id)) // find the ok device to siwtch
		{
		   pdata = ((entry->dev_platform).dev).platform_data;
		   IR_LOG_ALWAYS("Matched devname(%s),id(%d)\n",pdev->name,pdev->id);
           IR_LOG_ALWAYS("input_devname(%s)\n", pdata->input_name);
		   platform_device_register(pdev);
		   break;
	     }
	   }
	}
    return count;	
	
}


static ssize_t mtk_ir_show_alldev_info(struct device *dev,
	                                         struct device_attribute *attr,
	                                         char *buf)
{
    struct list_head *list_cursor;
    struct mtk_ir_device *entry;
	struct mtk_ir_core_platform_data * pdata;
	struct rc_dev *rcdev;
	struct lirc_driver *drv;
	struct rc_map * pmap;
	struct rc_map_table *pscan;
	int index = 0;
	int len = 0;
	int temp_len = 0;		

	// show all mtk_ir_device's info
    if (!list_empty(&mtk_ir_device_list))
    {

      list_for_each(list_cursor,&mtk_ir_device_list)
	  {
		entry = list_entry(list_cursor, struct mtk_ir_device, list);

		SPRINTF_DEV_ATTR("------- devname(%s),id(%d)---------\n",
			         entry->dev_platform.name,entry->dev_platform.id);
		
		
		pdata = ((entry->dev_platform).dev).platform_data;
		pmap = &(pdata->p_map_list->map);
		pscan = pmap->scan;
		
		SPRINTF_DEV_ATTR( "input_devname(%s)\n", pdata->input_name);
		
	    SPRINTF_DEV_ATTR( "rc_map name(%s)\n", pmap->name);
		SPRINTF_DEV_ATTR("i4_keypress_timeout(%d)ms\n",pdata->i4_keypress_timeout);

		SPRINTF_DEV_ATTR("rc_map_items:\n");
		// here show customer's map, not really the finally map
		for (index = 0; index < (pmap->size); index ++)
        {
           SPRINTF_DEV_ATTR("{0x%x, %d}\n",
		   	       pscan[index].scancode, pscan[index].keycode);
        } 	
		ASSERT(pdata->get_customer_code != NULL);
		SPRINTF_DEV_ATTR("customer code = 0x%x\n",pdata->get_customer_code());
      }
    }
	SPRINTF_DEV_ATTR("irq = %d\n", mtk_rc_core.irq);
	SPRINTF_DEV_ATTR("irq_register = %s\n", mtk_rc_core.irq_register?"true":"false");
	if (mtk_rc_core.dev_current != NULL)
	{
	   SPRINTF_DEV_ATTR("current activing devname(%s),id(%d)\n",
	               mtk_rc_core.dev_current->name,mtk_rc_core.dev_current->id);

	}
	else
	{
	    SPRINTF_DEV_ATTR("no activing ir device \n");
	}
	rcdev =  mtk_rc_core.rcdev;
	drv = mtk_rc_core.drv;
	
	if (rcdev != NULL)
	{
		SPRINTF_DEV_ATTR("current rc_dev devname(%s),id(%d)\n",
	               mtk_rc_core.dev_current->name,mtk_rc_core.dev_current->id);
		SPRINTF_DEV_ATTR("current rc_dev input_devname(%s)\n",
	                 rcdev->input_name);
		// show current activiting device's really mappiing
	   if (likely((drv != NULL) && (rcdev != NULL))) // transfer code to     /dev/lirc
	   {
	       pmap =  &(rcdev->rc_map);
	       pscan = pmap->scan;	
		   for (index = 0; index < (pmap->len); index ++)
           {
              SPRINTF_DEV_ATTR("{0x%x, %d}\n",
		   	       pscan[index].scancode, pscan[index].keycode);
           } 		
	   }
	}
	
	SPRINTF_DEV_ATTR("insmod_mode = %s\n",mode);
	
	
	return len;
}

#if MTK_IRRX_AS_MOUSE_INPUT
static ssize_t mtk_ir_core_show_mouse_info(struct device *dev,
	                                         struct device_attribute *attr,
	                                         char *buf)
{

	struct mtk_ir_core_platform_data * pdata = NULL;
	struct mtk_ir_mouse_code *p_mousecode = NULL;
	struct list_head *list_cursor;
    struct mtk_ir_device *entry;
	int len = 0;
	int temp_len = 0;	
	
	SPRINTF_DEV_ATTR("g_ir_device_mode = %d\n",mtk_ir_mouse_get_device_mode());
	
	if (!list_empty(&mtk_ir_device_list))
    {

      list_for_each(list_cursor,&mtk_ir_device_list)
	  {
		entry = list_entry(list_cursor, struct mtk_ir_device, list);

		SPRINTF_DEV_ATTR("------- devname(%s),id(%d)---------\n",
			         entry->dev_platform.name,entry->dev_platform.id);		
		pdata = ((entry->dev_platform).dev).platform_data;				
		SPRINTF_DEV_ATTR( "input_mousename(%s)\n", pdata->mousename);		
	    p_mousecode = &(pdata->mouse_code);
		SPRINTF_DEV_ATTR("scanleft =  0x%x\n",p_mousecode->scanleft);
	    SPRINTF_DEV_ATTR("scanright = 0x%x\n",p_mousecode->scanright);
	    SPRINTF_DEV_ATTR("scanup = 0x%x\n",p_mousecode->scanup);
	    SPRINTF_DEV_ATTR("scandown = 0x%x\n",p_mousecode->scandown);
	    SPRINTF_DEV_ATTR("scanenter = 0x%x\n",p_mousecode->scanenter);
	    SPRINTF_DEV_ATTR("scanswitch = 0x%x\n",p_mousecode->scanswitch);		
      }
    } 	
	
	SPRINTF_DEV_ATTR("x_small_step = %d\n",mtk_ir_mouse_get_x_smallstep());
	SPRINTF_DEV_ATTR("y_small_step = %d\n",mtk_ir_mouse_get_y_smallstep());
	SPRINTF_DEV_ATTR("x_large_step = %d\n",mtk_ir_mouse_get_x_largestep());
	SPRINTF_DEV_ATTR("y_large_step = %d\n",mtk_ir_mouse_get_y_largestep());	
	return len;
}

static ssize_t mtk_ir_core_store_mouse_info(struct device *dev,
                                            struct device_attribute *attr,
		                                    const char *buf, size_t count)
{
    int varx = -1;
	int vary = -1;	
	
	struct list_head *list_cursor;
    struct mtk_ir_device *entry;
	struct mtk_ir_core_platform_data * pdata;
	struct platform_device *pdev;
	struct attribute *pattr = &(attr->attr);	
	
   if (strcmp(pattr->name, "mouse_x_y_small") == 0)
   {
       sscanf(buf, "%d %d", &varx,&vary);
	   mtk_ir_mouse_set_x_smallstep(varx);
	   mtk_ir_mouse_set_y_smallstep(vary);

   }
   if (strcmp(pattr->name, "mouse_x_y_large") == 0)
   {
       sscanf(buf, "%d %d", &varx,&vary);
	   mtk_ir_mouse_set_x_largestep(varx);
	   mtk_ir_mouse_set_y_largestep(vary);
   }

    return count;	
	
}

#endif

// show ,store curent ir device  info
//static DEVICE_ATTR(curdev_info,  0664, mtk_ir_core_show_info,   mtk_ir_core_store_info);

// show all  device  info
static DEVICE_ATTR(alldev_info,  0444, mtk_ir_show_alldev_info, NULL);

// switch device protocol
static DEVICE_ATTR(switch_dev, 0664, mtk_ir_core_show_info, mtk_ir_core_store_info);

static DEVICE_ATTR(debug_log, 0664, mtk_ir_core_show_info, mtk_ir_core_store_info);

static DEVICE_ATTR(register, 0664, mtk_ir_core_show_info, mtk_ir_core_store_info);

static DEVICE_ATTR(clock, 0220, NULL, mtk_ir_core_store_debug);

static DEVICE_ATTR(swirq, 0220, NULL, mtk_ir_core_store_debug);

static DEVICE_ATTR(hwirq, 0220, NULL, mtk_ir_core_store_debug);
static DEVICE_ATTR(press_timeout, 0220, NULL, mtk_ir_core_store_info);
static DEVICE_ATTR(hungup, 0444, mtk_ir_core_show_info, NULL);
static DEVICE_ATTR(cuscode, 0664, mtk_ir_core_show_info, mtk_ir_core_store_info);
static DEVICE_ATTR(timer, 0664, mtk_ir_core_show_info, mtk_ir_core_store_info);


#if MTK_IR_DEVICE_NODE
static DEVICE_ATTR(log_to, 0664, mtk_ir_core_show_info, mtk_ir_core_store_info);
#endif

#if MTK_IRRX_AS_MOUSE_INPUT
static DEVICE_ATTR(mouse, 0444, mtk_ir_core_show_mouse_info, NULL);
static DEVICE_ATTR(mouse_x_y_small, 0220, NULL, mtk_ir_core_store_mouse_info);
static DEVICE_ATTR(mouse_x_y_large, 0220, NULL, mtk_ir_core_store_mouse_info);


#endif



static struct device_attribute *ir_attr_list[] = {
    //&dev_attr_curdev_info,
	&dev_attr_alldev_info,
	&dev_attr_switch_dev,	
	&dev_attr_debug_log,	
	&dev_attr_register,		
	&dev_attr_clock,
	&dev_attr_swirq,
	&dev_attr_hwirq,
	&dev_attr_press_timeout,
	&dev_attr_hungup,
	&dev_attr_cuscode,
	&dev_attr_timer,
	
	#if MTK_IR_DEVICE_NODE
    &dev_attr_log_to,
	#endif
	
	#if MTK_IRRX_AS_MOUSE_INPUT
    &dev_attr_mouse,
    &dev_attr_mouse_x_y_small,
    &dev_attr_mouse_x_y_large,
	#endif
	
};

void mtk_ir_core_send_scancode(u32 scancode )
{
    unsigned long __flags;
	struct rc_dev *rcdev =  mtk_rc_core.rcdev;
	struct lirc_driver *drv = mtk_rc_core.drv;
	
    spin_lock_irqsave(&scancode_lock, __flags);
    _u4Curscancode = scancode;
    spin_unlock_irqrestore(&scancode_lock, __flags);
	IR_LOG_DEBUG("[irrx]:scancode 0x%x\n",_u4Curscancode);

	
	sprintf(last_hang_place,"%s--%d\n",__FUNCTION__,__LINE__);
	
	if (likely(mtk_rc_core.k_thread != NULL)) // transfer code to /dev/input/eventx
	{
	   	wake_up_process(mtk_rc_core.k_thread);
	}
	sprintf(last_hang_place,"%s--%d\n",__FUNCTION__,__LINE__);
		
    #if MTK_IR_DEVICE_NODE
	mtk_ir_dev_put_scancode(scancode);// transfer code to /dev/ir_dev
	#endif
	
	sprintf(last_hang_place,"%s--%d\n",__FUNCTION__,__LINE__);
	if (likely((drv != NULL) && (rcdev != NULL))) // transfer code to     /dev/lirc
	{
	     int index = 0;
	     struct rc_map * pmap =  &(rcdev->rc_map);
	     struct rc_map_table *pscan = pmap->scan;
		 struct mtk_ir_msg msg = {scancode,BTN_NONE }; // does not find any map_key, but still transfer data to lirc driver

		for (index = 0; index < (pmap->len); index ++)
        {
           if (pscan[index].scancode == scancode)
           {
     	      msg.scancode = scancode;
		      msg.keycode = pscan[index].keycode;
		      break;
	       }  	
        }
	   lirc_buffer_write(drv->rbuf,(unsigned char *) &msg);
	   wake_up(&(drv->rbuf->wait_poll));
	}
	sprintf(last_hang_place,"%s--%d\n",__FUNCTION__,__LINE__);
	
}

void mtk_ir_core_send_mapcode(u32 mapcode, int stat)
{
    struct rc_dev *rcdev =  mtk_rc_core.rcdev;
	IR_LOG_KEY("mapcode 0x%x \n",mapcode);
	
	if (rcdev == NULL)
		IR_LOG_ALWAYS("rcdev = NULL, send fail!!!\n");
	
	rc_keyup(rcdev);//first do key up for last key;
	input_report_key(rcdev->input_dev, mapcode, stat);
	input_sync(rcdev->input_dev);
	
}

void mtk_ir_core_send_mapcode_auto_up(u32 mapcode,u32 ms)
{
    struct rc_dev *rcdev =  mtk_rc_core.rcdev;
	IR_LOG_KEY("mapcode 0x%x \n",mapcode);
	
	if (rcdev == NULL)
		IR_LOG_ALWAYS("rcdev = NULL, send fail!!!\n");
	
	rc_keyup(rcdev);//first do key up for last key;
	input_report_key(rcdev->input_dev, mapcode, 1);
	input_sync(rcdev->input_dev);
	
    msleep(ms);
	
	input_report_key(rcdev->input_dev, mapcode, 0);
	input_sync(rcdev->input_dev);	
}

void mtk_ir_core_disable_hwirq(void)
{

	// disable interrupt
    IR_WRITE_MASK(IRRX_IRINT_EN,IRRX_INTEN_MASK,IRRX_INTCLR_OFFSET,0x0);
	dsb();
	
}


void mtk_ir_core_clear_hwirq_stat(void)
{

   u32 info = IR_READ32(IRRX_COUNT_HIGH_REG);

   IR_WRITE_MASK(IRRX_IRCLR,IRRX_IRCLR_MASK,IRRX_IRCLR_OFFSET,0x1); // clear irrx state machine
   dsb();

   IR_WRITE_MASK(IRRX_IRINT_CLR,IRRX_INTCLR_MASK,IRRX_INTCLR_OFFSET,0x1); // clear irrx eint stat
   dsb();

   do
   {
      info = IR_READ32(IRRX_COUNT_HIGH_REG);

   }while(info != 0);

   // enable ir interrupt
   IR_WRITE_MASK(IRRX_IRINT_EN,IRRX_INTEN_MASK,IRRX_INTCLR_OFFSET,0x1);
   dsb();

}

#if 0
static int mtk_ir_core_is_hwirq(void)
{

     return (IR_READ32(IRRX_INTSTAT_REG) & (1<<IRRX_INTSTAT_OFFSET));

}
#endif

//ir irq function
static irqreturn_t mtk_ir_core_irq(int irq, void *dev_id)
{
    struct platform_device * pdev = (struct platform_device *)dev_id;
 	
    u32 scancode = BTN_NONE;
	
	struct mtk_ir_core_platform_data *pdata ;	
	mtk_ir_core_disable_hwirq();
	

	#if 0
	if (mtk_ir_core_is_hwirq() == 0)
	{
	  IR_LOG_ALWAYS("not irrx irq!!!\n");
	  printk("0x%08x\n", IR_READ32(IRRX_INTSTAT_REG));
	  return IRQ_HANDLED ;
	}
	#endif
	
    sprintf(last_hang_place,"irq begin\n");
	ASSERT(pdev != NULL );
	pdata = pdev->dev.platform_data;
	ASSERT(pdata != NULL );	
	ASSERT(pdata->ir_hw_decode != NULL);
	
	sprintf(last_hang_place,"ir_hw_decode begin\n");
	scancode = pdata->ir_hw_decode(NULL);// get the scancode
		
	sprintf(last_hang_place,"send_scancode begin\n");
	if (BTN_INVALID_KEY != scancode)
	{
      mtk_ir_core_send_scancode(scancode);
	}
	
	mtk_ir_core_clear_hwirq_stat();	// make sure irq statu is sure clear
		
    sprintf(last_hang_place,"ok here\n");
	
    return IRQ_HANDLED ;
}



static int mtk_ir_core_register_swirq(int trigger_type)
{
    int ret;
	 struct mtk_ir_core_platform_data *pdata = NULL;
	ASSERT(mtk_rc_core.dev_current != NULL);	
    pdata = (mtk_rc_core.dev_current)->dev.platform_data;
	
	if (mtk_rc_core.irq_register == true)
	{
		IR_LOG_ALWAYS("irq number(%d) already registered\n", mtk_rc_core.irq);
		return 0;
	}
	
	IR_LOG_ALWAYS("reg irq number(%d)\n", mtk_rc_core.irq);	

	#if 1
	ret = request_irq(mtk_rc_core.irq,mtk_ir_core_irq,trigger_type,
	 	              pdata->input_name,(void *)(mtk_rc_core.dev_current));
	if (ret)
	{
		IR_LOG_ALWAYS("fail to request irq(%d), ir_dev(%s) ret = %d !!!\n",
			             mtk_rc_core.irq, pdata->input_name, ret);

		return -1;
	}
	mtk_rc_core.irq_register = true;

	#endif

	
	return 0;
}

static void mtk_ir_core_free_swirq(void)
{
 	
	if (mtk_rc_core.irq_register == false)
	{
		IR_LOG_ALWAYS("irq number(%d) already freed\n", mtk_rc_core.irq);
		return ;
	}
	IR_LOG_ALWAYS("free irq number(%d)\n", mtk_rc_core.irq);
	free_irq(mtk_rc_core.irq,(void *)(mtk_rc_core.dev_current)); // first free irq	
	mtk_rc_core.irq_register = false;
	return ;
}

/*
here is get message from initial kemap ,not rc-dev.map
*/
void mtk_ir_core_get_msg_by_scancode(u32 scancode, struct mtk_ir_msg *msg)
{
  struct platform_device *pdev = mtk_rc_core.dev_current;
  struct mtk_ir_core_platform_data *pdata ;
  struct rc_map_list *plist;
  struct rc_map *pmap;
  struct rc_map_table *pscan;
  unsigned int size;
  int index = 0;
  ASSERT(msg != NULL);
  ASSERT (pdev != NULL);
  pdata = pdev->dev.platform_data;
  ASSERT(pdata != NULL);
  plist = pdata->p_map_list;
  ASSERT(plist != NULL);
  pmap = &(plist->map);
  pscan = pmap->scan;
  ASSERT(pscan != NULL);
  size = pmap->size;

  msg->scancode = scancode;
  msg->keycode = BTN_NONE;	 // does not find any map_key, but still transfer data to userspace
  for (index = 0; index < size; index ++)
  {
     if (pscan[index].scancode == scancode)
     {
     	msg->scancode = scancode;
		msg->keycode = pscan[index].keycode;
		break;
	 }  	
  }
}
	


int mtk_ir_core_create_thread (
							 int (*threadfn)(void *data),
							 void *data,
							 const char* ps_name,
							 struct task_struct ** p_struct_out,
							 unsigned int  ui4_priority )
  {

	  struct sched_param param;
	  int ret = 0;
	  struct  task_struct * ptask;
	  ptask = kthread_create(threadfn, data, ps_name);

	  if (IS_ERR(ptask))
	  {
		  printk("unable to create kernel thread %s\n",ps_name );
		  *p_struct_out = NULL;
		  return -1;
	  }
	  param.sched_priority = ui4_priority;
	  ret = sched_setscheduler(ptask, SCHED_RR, &param);
	  *p_struct_out = ptask;
	  return 0;
  }


/*
mtk_ir_core_open()
for linux2.4 module

*/
static  int mtk_ir_core_open(void *data)
{
	return 0;
}

/*
mtk_ir_core_close()
for linux2.4 module

*/
static void mtk_ir_core_close(void *data)
{
	return ;
}

/*
mtk_lirc_fops's all functions is in lirc_dev.c

*/

static struct file_operations mtk_lirc_fops = {
	.owner		= THIS_MODULE,
	.write		= lirc_dev_fop_write,
	.unlocked_ioctl	= lirc_dev_fop_ioctl,
	.read		= lirc_dev_fop_read,
	.poll		= lirc_dev_fop_poll,
	.open		= lirc_dev_fop_open,
	.release	= lirc_dev_fop_close,
	.llseek		= no_llseek,
};


#ifdef TWO_CONSECUTIVE_TIMES_CHECK
static void release_expire_timer_function(unsigned long v)
{
    unsigned long __flags;
    spin_lock_irqsave(&saved_scancode_lock, __flags);
    prev_scancode = 0;
    spin_unlock_irqrestore(&saved_scancode_lock, __flags);
}

static void rc_keydown_2consecutive_check(struct rc_dev *rcdev, int scancode, u8 toggle,
                                          struct mtk_ir_core_platform_data * pdata )
{
    int tmp_i4_keypress_timeout = 0;
	int tmp_prev_scancode = 0;
    unsigned long __flags;

	tmp_i4_keypress_timeout = pdata->i4_keypress_timeout;

    /* check current pressed/released status */
	if (!(rcdev->keypressed))
	{
		/*
		 * if button status change
		 * Pressed -> Released
		 */
		if( TRUE == isConsecutiveCheckingPressed ) {
            spin_lock_irqsave(&saved_scancode_lock, __flags);
		    prev_scancode = 0;
            spin_unlock_irqrestore(&saved_scancode_lock, __flags);
		    isConsecutiveCheckingPressed = FALSE;
	    }
		mod_timer(&expire_check_timer, (jiffies + msecs_to_jiffies(tmp_i4_keypress_timeout)));
	}
    curr_scancode = scancode;
    
	spin_lock_irqsave(&saved_scancode_lock, __flags);
    tmp_prev_scancode = prev_scancode;
    spin_unlock_irqrestore(&saved_scancode_lock, __flags);

	if(tmp_prev_scancode == curr_scancode) {
		/* 2 consecutive times match */
		rc_keydown(rcdev,curr_scancode ,toggle);
		isConsecutiveCheckingPressed = TRUE;
		del_timer(&expire_check_timer);
	} else if(isFromSuspend()) {
		rc_keydown(rcdev,curr_scancode ,toggle);
		isConsecutiveCheckingPressed = TRUE;
		del_timer(&expire_check_timer);
		fromSuspendFlag(FALSE);
	}
	/* save current_scancode to use next check */
    spin_lock_irqsave(&saved_scancode_lock, __flags);
    prev_scancode = curr_scancode;
    spin_unlock_irqrestore(&saved_scancode_lock, __flags);

	return;
}
#endif /* TWO_CONSECUTIVE_TIMES_CHECK */
// for linux input system
static int mtk_ir_input_thread(void* pvArg)
{

    u32 u4CurKey = BTN_NONE;
	
	struct rc_dev *rcdev =  NULL;
	
	unsigned long __flags;
	
     while( !kthread_should_stop())
    {

	 IR_LOG_DEBUG(" mtk_ir_input_thread begin !!\n");
	 set_current_state(TASK_INTERRUPTIBLE);

	 if (u4CurKey == BTN_NONE) // does not have key
	 {
	    schedule();//
	 }
	 set_current_state(TASK_RUNNING);

	 spin_lock_irqsave(&scancode_lock, __flags);
       u4CurKey = _u4Curscancode;
	   _u4Curscancode = BTN_NONE;
     spin_unlock_irqrestore(&scancode_lock, __flags);
	

	 if (kthread_should_stop())// other place want to stop this thread;
		continue;

     IR_LOG_DEBUG("mtk_ir_input_thread get scancode: 0x%08x\n ",u4CurKey);
    rcdev = mtk_rc_core.rcdev;
	
	if (unlikely(rcdev == NULL))
	{
		IR_LOG_ALWAYS("rcdev = NULL!!!!!!!!!\n");
     	continue;
	}
	
	#if MTK_IRRX_AS_MOUSE_INPUT
	
     MTK_IR_DEVICE_MODE dev_mode = mtk_ir_mouse_get_device_mode();
     struct mtk_ir_core_platform_data * pdata = NULL;
	 pdata = mtk_rc_core.dev_current->dev.platform_data;

	 struct mtk_ir_mouse_code  *p_mousecode = &(pdata->mouse_code);

	 if (u4CurKey == p_mousecode->scanswitch)// this is the mode switch code
	 {
	 	IR_LOG_ALWAYS("switch mode code 0x%08x\n",u4CurKey);
		if (!(rcdev->keypressed))// last key was released, this is the first time switch code down.
		{
			if (dev_mode == MTK_IR_AS_IRRX)
			{
				mtk_ir_mouse_set_device_mode(MTK_IR_AS_MOUSE);
			}
			else
			{
				mtk_ir_mouse_set_device_mode(MTK_IR_AS_IRRX);
			}
		}
		/* this function will auto key up, when does not have key after 250ms
		here send 0xffff, because when we hold scanswitch key, we will not switch mode in every repeat
		interrupt, we only switch when switch key down,  but we need to kown ,whether switch code was the
		first time to down, so we send 0xffff.
		
		*/
#ifdef TWO_CONSECUTIVE_TIMES_CHECK
		rc_keydown_2consecutive_check(rcdev,0xffff ,0,pdata);// this function will auto key up, when does not have key after 250m
#else
		rc_keydown(rcdev,0xffff ,0);// this function will auto key up, when does not have key after 250m
#endif /* TWO_CONSECUTIVE_TIMES_CHECK */
	 }
	 else
	 {
       if (dev_mode == MTK_IR_AS_MOUSE) // current irrx as mouse
       {
       	    if (mtk_ir_mouse_proc_key(u4CurKey,&mtk_rc_core)) // this key is mouse event, process success
			{
				IR_LOG_ALWAYS("process mouse key\n");
			}
			else// this key is not mouse key ,so as ir key process
			{
				IR_LOG_ALWAYS("process ir key\n");
#ifdef TWO_CONSECUTIVE_TIMES_CHECK
	            rc_keydown_2consecutive_check(rcdev,u4CurKey ,0,pdata);// this function will auto key up, when does not have key after 250ms
#else
	            rc_keydown(rcdev,u4CurKey ,0);// this function will auto key up, when does not have key after 250ms
#endif /* TWO_CONSECUTIVE_TIMES_CHECK */
	     	}
	   }
	   else
	   {
#ifdef TWO_CONSECUTIVE_TIMES_CHECK
         rc_keydown_2consecutive_check(rcdev,u4CurKey ,0,pdata);// this function will auto key up, when does not have key after 250ms
#else
         rc_keydown(rcdev,u4CurKey ,0);// this function will auto key up, when does not have key after 250ms
#endif /* TWO_CONSECUTIVE_TIMES_CHECK */
	   }
	 }

	#else
	
#ifdef TWO_CONSECUTIVE_TIMES_CHECK
	   rc_keydown_2consecutive_check(rcdev,u4CurKey ,0,pdata);// this function will auto key up, when does not have key after 250ms
#else
	   rc_keydown(rcdev,u4CurKey ,0);// this function will auto key up, when does not have key after 250ms
#endif /* TWO_CONSECUTIVE_TIMES_CHECK */

	#endif
	
	 u4CurKey = BTN_NONE;

    }
	
	
    IR_LOG_ALWAYS("mtk_ir_input_thread exit success\n");
	return 0;
}




static int mtk_ir_core_create_attr(struct device *dev)
{
    int idx, err = 0;
    int num = (int)(sizeof(ir_attr_list)/sizeof(ir_attr_list[0]));
    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) {
        if ((err = device_create_file(dev, ir_attr_list[idx])))
            break;
    }
    return err;
}
static void mtk_ir_core_remove_attr(struct device *dev)
{
  int idx ;
  int num = (int)(sizeof(ir_attr_list)/sizeof(ir_attr_list[0]));
    if (!dev)
        return ;

    for (idx = 0; idx < num; idx++) {
        device_remove_file(dev, ir_attr_list[idx]);
    }
}

//register driver for /dev/lirc

static int mtk_ir_lirc_register(struct device *dev_parent)
{
	struct lirc_driver *drv;  // for lirc_driver
	struct lirc_buffer *rbuf; //  for store ir data
	int ret  = -ENOMEM;
	unsigned long features;

	drv = kzalloc(sizeof(struct lirc_driver), GFP_KERNEL);
	if (!drv)
	{
	  IR_LOG_ALWAYS("kzalloc lirc_driver fail!!!\n ");
	  return ret;
	}

	rbuf = kzalloc(sizeof(struct lirc_buffer), GFP_KERNEL);
	if (!rbuf)
	{
	  IR_LOG_ALWAYS("kzalloc lirc_buffer fail!!!\n ");
	  goto rbuf_alloc_failed;
	}

	ret = lirc_buffer_init(rbuf, MTK_IR_CHUNK_SIZE, LIRCBUF_SIZE);
	if (ret)
	{
	    IR_LOG_ALWAYS("lirc_buffer_init fail ret(%d) !!!\n", ret);
		goto rbuf_init_failed;
	}

	features = 0;
	
	snprintf(drv->name, sizeof(drv->name), "mtk_ir_core_lirc");
	
	drv->minor = -1;
	drv->features = features;
	drv->data = NULL;
	drv->rbuf = rbuf;
	drv->set_use_inc = mtk_ir_core_open;
	drv->set_use_dec = mtk_ir_core_close;
	drv->chunk_size = MTK_IR_CHUNK_SIZE ;
	drv->code_length = MTK_IR_CHUNK_SIZE;
	drv->fops = &mtk_lirc_fops;
	drv->dev = dev_parent;
	drv->owner = THIS_MODULE;

	drv->minor = lirc_register_driver(drv);
	if (drv->minor < 0) {
		ret = -ENODEV;
		IR_LOG_ALWAYS("lirc_register_driver fail ret(%d) !!!\n", ret);
		goto lirc_register_failed;
	}	
    spin_lock_init(&scancode_lock);
	ret = mtk_ir_core_create_thread( mtk_ir_input_thread,
						            NULL,
						            "mtk_ir_inp_thread",
                                    &(mtk_rc_core.k_thread),
                                    RTPM_PRIO_SCRN_UPDATE);
    if (ret)
    {
       IR_LOG_ALWAYS ("create mtk_ir_input_thread fail\n");
	   goto ir_inp_thread_fail;
    }

	
	IR_LOG_ALWAYS("lirc_register_driver succeed drv->minor(%d)\n",drv->minor);
	mtk_rc_core.drv = drv;// store lirc_driver pointor to mtk_rc_core.drv

	return 0;
	
ir_inp_thread_fail:
	 lirc_unregister_driver(drv->minor);
lirc_register_failed:
	lirc_buffer_free(rbuf);
rbuf_init_failed:
	kfree(rbuf);
rbuf_alloc_failed:
	kfree(drv);
	return ret;
}

//unregister driver for /dev/lirc

static int mtk_ir_lirc_unregister(struct mtk_rc_core * pdev)
{

	
	struct lirc_driver *drv = pdev->drv;
	if (pdev->k_thread) // first stop thread
	{
		kthread_stop(pdev->k_thread);
		pdev->k_thread = NULL;
	}
	
    if (drv)
	{
	  lirc_unregister_driver(drv->minor);
	  lirc_buffer_free(drv->rbuf);
	  kfree(drv);
	  pdev->drv = NULL;
    }
	return 0;
}

// this timer function is only for workaround ir's random hung issue
static void mtk_ir_timer_function(unsigned long v)
{
   u32 CHK_CNT_PULSE = 0;
   if (timer_log)
   {
   	 printk("%s\n",__FUNCTION__);
   }
   CHK_CNT_PULSE = ((IR_READ32(IRRX_EXPBCNT) >>IRRX_IRCHK_CNT_OFFSET) & IRRX_IRCHK_CNT);

   if (CHK_CNT_PULSE == IRRX_IRCHK_CNT) //then clear register
   {
     detect_hung_timers ++;
   	 IR_LOG_ALWAYS("CHK_CNT_PULSE = 0x%x,detect_hung_timers = %d\n",CHK_CNT_PULSE,detect_hung_timers);
	 if (detect_hung_timers > 1000)
	 {
	   detect_hung_timers = 0;
	 }
     mtk_ir_core_clear_hwirq_stat();
   }
   g_ir_timer.expires = jiffies + TIMER_PERIOD;
   add_timer(&g_ir_timer);
}


static int  mtk_ir_core_probe(struct platform_device *pdev)
{
	
	struct mtk_ir_core_platform_data *pdata = pdev->dev.platform_data;
	struct rc_dev *rcdev = NULL ;
	int ret = 0;

	/* id = -1, it is the mtk_ir_dev_parent,
	   only for register lirc_driver and begin mtk_ir_input_thread,
	   bause lirc_driver and mtk_ir_input_thread  are for all devices*/
    if (-1 == pdev->id)
    {
       mtk_ir_core_create_attr(&(pdev->dev)); // create device attribute
       ret = mtk_ir_lirc_register(&(pdev->dev));
	   if (ret)
	   {
		  IR_LOG_ALWAYS("mtk_ir_lirc_register fail ret(%d) !!!\n",ret);
	   }
	   return ret;
    }
		
	/* register really ir device nec or rc5 or rc6 ....*/
	ASSERT(pdata != NULL);
	ASSERT(pdata->init_hw != NULL);
	ASSERT(pdata->uninit_hw != NULL);
	ASSERT(pdata->p_map_list != NULL);
	ASSERT(pdata->ir_hw_decode != NULL);
	

	platform_device_unregister(mtk_rc_core.dev_current); // first unregister old device
	
	ret = pdata->init_hw(); // init this  ir's  hw
	if (ret)
	{
		IR_LOG_ALWAYS(" fail to init_hw for ir_dev(%s) ret = %d!!!\n",
			             pdata->input_name, ret);
		goto err_init_hw;		
	}
	
	rcdev = rc_allocate_device(); // alloc rc device and rc->input
	if (!rcdev) {
		ret = -ENOMEM;
		IR_LOG_ALWAYS("rc_allocate_device fail\n");
		goto err_allocate_device;
	}
  	
	rcdev->driver_type = RC_DRIVER_SCANCODE;
	rcdev->allowed_protos = pdata->p_map_list->map.rc_type;
	rcdev->input_name = pdata->input_name; // /proc/bus/input/devices
	rcdev->input_id.bustype = BUS_HOST;
	rcdev->input_id.version = IR_VERSION;
	rcdev->input_id.product = IR_PRODUCT;
	rcdev->input_id.vendor =  IR_VENDOR;	
	rcdev->driver_name = MTK_IR_DRIVER_NAME;
	rcdev->map_name = pdata->p_map_list->map.name;

	ret = rc_register_device(rcdev);
	if (ret < 0) {
		
		IR_LOG_ALWAYS( "failed to register rc device for ir_dev(%s) ret(%d)!!!\n",
			               pdata->input_name, ret);
		goto err_register_rc_device;
	}
	rc_set_keypress_timeout(pdata->i4_keypress_timeout);
	clear_bit(EV_MSC, rcdev->input_dev->evbit);
	clear_bit(MSC_SCAN, rcdev->input_dev->mscbit);
	
	#if MTK_IRRX_AS_MOUSE_INPUT
	mtk_rc_core.p_devmouse = mtk_ir_mouse_register_input(pdev);
	if (NULL == mtk_rc_core.p_devmouse)
	{
		IR_LOG_ALWAYS("fail to register ir_mouse device(%s)\n",pdata->mousename);
		goto err_register_mousedev;
		
	}
	#endif
	
	mtk_rc_core.rcdev = rcdev;
	mtk_rc_core.dev_current = pdev;	

	ret = mtk_ir_core_register_swirq(IRQF_TRIGGER_LOW);
	if (ret)
	{	
		goto err_request_irq;
	
	}
	if (g_ir_timer.function == NULL)
	{
        init_timer(&g_ir_timer);
	    g_ir_timer.function = mtk_ir_timer_function;
	    g_ir_timer.expires = jiffies + TIMER_PERIOD;
	    add_timer(&g_ir_timer);
	}
#ifdef TWO_CONSECUTIVE_TIMES_CHECK	
    spin_lock_init(&saved_scancode_lock);
	if (expire_check_timer.function == NULL)
	{
        init_timer(&expire_check_timer);
	    expire_check_timer.function = release_expire_timer_function;
	}
#endif /* TWO_CONSECUTIVE_TIMES_CHECK */
	return 0;

err_request_irq:
	rc_unregister_device(rcdev);
	rcdev = NULL;
err_register_mousedev:
err_register_rc_device:
	rc_free_device(rcdev);
	rcdev = NULL;
	mtk_rc_core.rcdev = NULL;	
err_allocate_device:	
    pdata->uninit_hw();
err_init_hw:

	return ret;
}

static int  mtk_ir_core_remove(struct platform_device *pdev)
{
   	struct mtk_ir_core_platform_data *pdata = pdev->dev.platform_data;
	struct rc_dev *rcdev = mtk_rc_core.rcdev;

	IR_LOG_ALWAYS ("mtk_ir_core_remove device name(%s)\n",
		                     kobject_name(&(pdev->dev).kobj));
	
	if (-1 == pdev->id) // it is mtk_ir_dev_parent, it must be the last paltform device to unregister
	{
		mtk_ir_lirc_unregister(&mtk_rc_core); // unregister lirc driver
		mtk_ir_core_remove_attr(&(pdev->dev));
		
		return 0;
	}
	
	if (mtk_rc_core.dev_current != pdev) // it is not the current active device, do nothing ,only prompt
	{
	    IR_LOG_ALWAYS ("mtk_ir_core_remove device name(%s) key_map_name(%s)\n",
			             kobject_name(&(pdev->dev).kobj), pdata->p_map_list->map.name );
		return 0;
	}

	if (rcdev != NULL) //  ok, it is the current active device
	{
	    IR_LOG_ALWAYS("unregister current active device: name(%s),key_map_name(%s)",
			            kobject_name(&(pdev->dev).kobj),pdata->p_map_list->map.name);

		mtk_ir_core_free_swirq();			
		pdata->uninit_hw();// first uinit this type ir device	
		#if MTK_IRRX_AS_MOUSE_INPUT
        if (mtk_rc_core.p_devmouse != NULL)
        {
           mtk_ir_mouse_unregister_input(mtk_rc_core.p_devmouse);
		   mtk_rc_core.p_devmouse = NULL;
        }
		#endif
		rc_unregister_device(rcdev); // unregister rcdev
		mtk_rc_core.rcdev = NULL;
		mtk_rc_core.dev_current = NULL;
	}
		
	return 0;
}


void mtk_ir_core_log_always(const char *fmt, ...)
{
     va_list args;
	
     #if MTK_IR_DEVICE_NODE
	 int size_data = 0;
	 char buff[IR_NETLINK_MSG_SIZE] = {0};
	 char *p = buff;
	 struct message_head head = {MESSAGE_NORMAL , 0};

     if (0 == mtk_ir_get_log_to()) // end key to kernel
	 {
	 if (ir_log_debug_on)
        {
   		  va_start(args, fmt);
		  vprintk(fmt, args);
		  va_end(args);
        }
	 }
	 else   // here is log to userspace
	 {
	     p += IR_NETLINK_MESSAGE_HEADER;
	     va_start(args, fmt);
	     size_data = vsnprintf(p, IR_NETLINK_MSG_SIZE -IR_NETLINK_MESSAGE_HEADER-1, fmt, args);
	     va_end(args);
		 p[size_data] = 0;
		 head.message_size = size_data + 1;		
		 memcpy(buff,&head, IR_NETLINK_MESSAGE_HEADER);

		 #if 0
		 printk("[debug]: %s size_data(%d)\n", p,size_data);
		 printk("[debug]: %d\n", IR_NETLINK_MESSAGE_HEADER + head.message_size);
         #endif

		 mtk_ir_netlink_msg_q_send(buff,IR_NETLINK_MESSAGE_HEADER + head.message_size);
	 }	
	 #else
	 if (ir_log_debug_on)
        {
    	  va_start(args, fmt);
    	  vprintk(fmt, args);
    	  va_end(args);
        }
	 #endif


}


#ifdef CONFIG_HAS_EARLYSUSPEND

static void mtk_ir_core_early_suspend(struct early_suspend *h)
{
     IR_LOG_ALWAYS("\n");

	 struct mtk_ir_core_platform_data *pdata = NULL;


     if (likely(mtk_rc_core.dev_current != NULL))
     {
          pdata = mtk_rc_core.dev_current->dev.platform_data;
		  ASSERT(pdata != NULL);
		  if (pdata->early_suspend)
		  {
		     pdata->early_suspend(NULL);
		  }
     }		

}

static void mtk_ir_core_late_resume(struct early_suspend *h)
{
	 IR_LOG_ALWAYS("\n");

     struct mtk_ir_core_platform_data *pdata = NULL;


     if (likely(mtk_rc_core.dev_current != NULL))
     {
          pdata = mtk_rc_core.dev_current->dev.platform_data;
		  ASSERT(pdata != NULL);
		  if (pdata->late_resume)
		  {
		     pdata->late_resume(NULL);
		  }
     }
}

static struct early_suspend mtk_ir_early_suspend_desc = {
	.level		= EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1,
	.suspend	= mtk_ir_core_early_suspend,
	.resume		= mtk_ir_core_late_resume,
};
#endif

static int __init mtk_ir_core_init(void)
{
    int ret ;

  	mt_ir_core_clock_enable();// enable ir clock;
    	
	mt_set_gpio_mode(GPIO46,GPIO_MODE_01); // enable irrx pin function

	mt_ir_core_power_enable();
	
	ret = platform_driver_register(&mtk_ir_core_driver); //first register driver

	if (ret)
	{
	   IR_LOG_ALWAYS("mtk_ir_core_driver register fail\n");	
	   goto fail;
	}
	
	ret = platform_device_register(&mtk_ir_dev_parent); // here register parent ir dev for lirc_driver and input_thread
	if (ret)
	{
	   IR_LOG_ALWAYS("mtk_ir_dev_parent register fail !!!\n");
	   goto fail;
	}
	
	#if (MTK_IRRX_PROTOCOL == MTK_IR_ID_NEC)
	ret = platform_device_register(&(mtk_ir_dev_nec.dev_platform)); //register  nec device
	#elif (MTK_IRRX_PROTOCOL == MTK_IR_ID_RC6)
	ret = platform_device_register(&(mtk_ir_dev_rc6.dev_platform)); //register  rc6 device
    #elif (MTK_IRRX_PROTOCOL == MTK_IR_ID_RC5)
	ret = platform_device_register(&(mtk_ir_dev_rc5.dev_platform)); //register  rc5 device
	#elif (MTK_IRRX_PROTOCOL == MTK_IR_ID_SIRC)
	ret = platform_device_register(&(mtk_ir_dev_sirc.dev_platform)); //register  sirc device
	#elif (MTK_IRRX_PROTOCOL == MTK_IR_ID_RSTEP)
	ret = platform_device_register(&(mtk_ir_dev_rstep.dev_platform)); //register  sirc device
	#endif

	if (ret)
	{
		IR_LOG_ALWAYS("mtk_ir_dev_nec register fail!!!\n");
		goto fail;
	}
	
	#if MTK_IR_DEVICE_NODE
    mtk_ir_dev_init();
	#endif

	INIT_LIST_HEAD(&(mtk_ir_dev_nec.list));
	list_add(&(mtk_ir_dev_nec.list),&mtk_ir_device_list);
	
    INIT_LIST_HEAD(&(mtk_ir_dev_rc6.list));
	list_add(&(mtk_ir_dev_rc6.list),&mtk_ir_device_list);
	
	INIT_LIST_HEAD(&(mtk_ir_dev_rc5.list));
	list_add(&(mtk_ir_dev_rc5.list),&mtk_ir_device_list);

    INIT_LIST_HEAD(&(mtk_ir_dev_sirc.list));
	list_add(&(mtk_ir_dev_sirc.list),&mtk_ir_device_list);

    INIT_LIST_HEAD(&(mtk_ir_dev_rstep.list));
	list_add(&(mtk_ir_dev_rstep.list),&mtk_ir_device_list);
	
#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&mtk_ir_early_suspend_desc);
#endif
	 	
	IR_LOG_ALWAYS ("mtk_ir_core_init success\n");
	
fail:
	return ret;
}

static void __exit mtk_ir_core_exit(void)
{
    del_timer(&g_ir_timer);
#ifdef TWO_CONSECUTIVE_TIMES_CHECK
    del_timer(&expire_check_timer);
#endif /* TWO_CONSECUTIVE_TIMES_CHECK */
    #if MTK_IR_DEVICE_NODE
	mtk_ir_dev_exit();
   	#endif	
	
	if (mtk_rc_core.dev_current)
	{
	  platform_device_unregister(mtk_rc_core.dev_current);
	  mtk_rc_core.dev_current = NULL;
	}
	
	platform_device_unregister(&mtk_ir_dev_parent);		
	platform_driver_unregister(&mtk_ir_core_driver);//for this ,all device will be unregister

	#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&mtk_ir_early_suspend_desc);
   #endif


}

module_init(mtk_ir_core_init);
module_exit(mtk_ir_core_exit);
module_param(mode,charp,S_IRUGO);

/*****************************************************************************/
MODULE_AUTHOR("ji.wang <ji.wang@mediatek.com>");
MODULE_DESCRIPTION(IR_LOG_TAG);
MODULE_LICENSE("GPL");



