/*
 * Copyright 2016 Sony Corporation
 * File Changed on 2016-10-17
 */
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

#ifndef __MTK_IR_RECV_H__
#define __MTK_IR_RECV_H__

#include <media/lirc_dev.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/types.h>
#include <linux/input.h>
#include "mtk_ir_common.h"

#define IR_BUS    BUS_HOST
#define IR_VERSION 11
#define IR_PRODUCT 11
#define IR_VENDOR 11


#define RC_MAP_MTK_NEC "rc_map_mtk_nec"
#define RC_MAP_MTK_RC6 "rc_map_mtk_rc6"
#define RC_MAP_MTK_RC5 "rc_map_mtk_rc5"
#define RC_MAP_MTK_SIRC "rc_map_mtk_sirc"
#define RC_MAP_MTK_RSTEP "rc_map_mtk_rstep"



#define MTK_IR_DRIVER_NAME	"mtk_ir"

#define MTK_INPUT_NEC_DEVICE_NAME	"NEC_Remote_Controller" // here is for input device name 
#define MTK_INPUT_RC6_DEVICE_NAME	"RC6_Remote_Controller" // here is for input device name 
#define MTK_INPUT_RC5_DEVICE_NAME	"RC5_Remote_Controller" // here is for input device name 
#define MTK_INPUT_SIRC_DEVICE_NAME	"sircs_key"             // here is for input device name 
#define MTK_INPUT_RSTEP_DEVICE_NAME	"RSTEP_Remote_Controller" // here is for input device name 


#define SEMA_STATE_LOCK   (0)
#define SEMA_STATE_UNLOCK (1)

#define SEMA_LOCK_OK                    ((int)   0)
#define SEMA_LOCK_TIMEOUT               ((int)  -1)
#define SEMA_LOCK_FAIL                  ((int)  -2)


struct mtk_rc_core {

	int irq;          // ir irq number	
	bool irq_register; // whether ir irq has been registered
    struct rc_dev *rcdev;//  current rcdev
    struct lirc_driver *drv; // lirc_driver
    struct platform_device *dev_current; // current activing device    
    struct task_struct * k_thread ;   // input thread

	#if MTK_IRRX_AS_MOUSE_INPUT
	struct input_dev *p_devmouse;// IR as mouse
	#endif
   
};

struct mtk_ir_device
{
	struct list_head list;
	struct platform_device dev_platform;
};

struct mtk_ir_core_platform_data {
	
	const char * input_name;      // /proc/bus/devices/input/input_name
	struct rc_map_list *p_map_list;   // rc map list
	int i4_keypress_timeout;
	int (*init_hw)(void);         // init ir_hw
	int (*uninit_hw)(void);       //uint ir_hw	
	u32 (*ir_hw_decode)(void * preserve); // decode function. preserve for future use
    u32 (*get_customer_code)(void );//get customer code function
    u32 (*set_customer_code)(void * preserve);//set customer code function
    
	void (*early_suspend)(void * preserve);
	void (*late_resume)(void * preserve);

	int (*suspend)(void * preserve);
	int (*resume)(void * preserve);		
	#if MTK_IRRX_AS_MOUSE_INPUT
	struct mtk_ir_mouse_code mouse_code;// IRRX as mouse code
	char mousename[32];
	#endif
		
};

extern struct mtk_rc_core mtk_rc_core;

extern struct mtk_ir_device mtk_ir_dev_nec;
extern struct mtk_ir_device mtk_ir_dev_rc6;
extern struct mtk_ir_device mtk_ir_dev_rc5;
extern struct mtk_ir_device mtk_ir_dev_sirc;
extern struct mtk_ir_device mtk_ir_dev_rstep;



extern struct list_head mtk_ir_device_list;

 
	 

extern void	release(struct device *dev);


extern int mtk_ir_core_create_thread (
							 int (*threadfn)(void *data),
							 void *data,
							 const char* ps_name,
							 struct task_struct ** p_struct_out,
							 unsigned int  ui4_priority );

extern void mtk_ir_core_send_scancode(u32 scancode);
extern void mtk_ir_core_send_mapcode(u32 mapcode, int stat);

extern void mtk_ir_core_send_mapcode_auto_up(u32 mapcode,u32 ms);

extern void mtk_ir_core_get_msg_by_scancode(u32 scancode, struct mtk_ir_msg *msg);
extern void mtk_ir_core_clear_hwirq_stat(void);
extern MTK_IR_MODE mtk_ir_core_getmode(void);
extern void rc_set_keypress_timeout(int i4value);
extern void mtk_ir_core_disable_hwirq(void);

#if MTK_IRRX_AS_MOUSE_INPUT
extern MTK_IR_DEVICE_MODE mtk_ir_mouse_get_device_mode(void);
extern void mtk_ir_mouse_set_device_mode(MTK_IR_DEVICE_MODE devmode);
extern struct input_dev * mtk_ir_mouse_register_input(struct platform_device *pdev);
extern int mtk_ir_mouse_proc_key(u32 scancode, struct mtk_rc_core *p_mtk_rc_core );
extern void mtk_ir_mouse_unregister_input(struct input_dev * dev);

extern int mtk_ir_mouse_get_x_smallstep(void);
extern int mtk_ir_mouse_get_y_smallstep(void);
extern int mtk_ir_mouse_get_x_largestep(void);
extern int mtk_ir_mouse_get_y_largestep(void);

extern void mtk_ir_mouse_set_x_smallstep(int xs);
extern void mtk_ir_mouse_set_y_smallstep(int ys);
extern void mtk_ir_mouse_set_x_largestep(int xl);
extern void mtk_ir_mouse_set_y_largestep(int yl);

#endif


#endif
