/* rc-dvb0700-big.c - Keytable for devices in dvb0700
 *
 * Copyright (c) 2010 by Mauro Carvalho Chehab <mchehab@redhat.com>
 *
 * TODO: This table is a real mess, as it merges RC codes from several
 * devices into a big table. It also has both RC-5 and NEC codes inside.
 * It should be broken into small tables, and the protocols should properly
 * be indentificated.
 *
 * The table were imported from dib0700_devices.c.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
 /* --------------------------------------------------------------- */
 /*  Copyright 2016 SONY Corporation                                */
 /* --------------------------------------------------------------- */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>

#include "mtk_ir_common.h"
#include "mtk_ir_core.h"
#include "mtk_ir_cus_sirc.h" // include customer's key map
#include "mtk_ir_regs.h"    // include ir registers

#include "../../../../../vendor/sony/icx_key/driver/icx_key.h"
#include <linux/cdev.h>

#define MTK_SIRC_CONFIG      (IRRX_CH_END_15 + IRRX_CH_HWIR +IRRX_CH_ORDINV )
#define MTK_SIRC_SAPERIOD    (0x002) //
#define MTK_SIRC_THRESHOLD   (0x604) //bit[12:8]for  deglitch  200/31.25=6

#define MTK_SIRC_EXP_POWE_KEY1   0x00000000
#define MTK_SIRC_EXP_POWE_KEY2   0x00000000

#define IRRX_SIRC_BITCNT12 (u32)(0xc)
#define IRRX_SIRC_BITCNT15 (u32)(0xf)
#define IRRX_SIRC_BITCNT20 (u32)(0x14)
#define INFO_TO_BITCNT(u4Info)      ((u4Info & IRRX_CH_BITCNT_MASK)    >> IRRX_CH_BITCNT_BITSFT)

extern int sircs_get_start_code(void);

//set deglitch with the min number. when glitch < (33*6 = 198us,ignore)
static int mtk_ir_sirc_init_hw(void);
static int mtk_ir_sirc_uninit_hw(void);
static char Reverse1Byte(char ucSrc);
u32 mtk_ir_sirc_decode( void * preserve);

static u32 _u4PrevKey = BTN_NONE;   // pre key
static u32 _u4Sirc_customer_code_12bit = SIRC_CUSTOMER_12BIT;
static u32 _u4Sirc_customer_code_15bit = SIRC_CUSTOMER_15BIT;
static u32 _u4Sirc_customer_code_20bit = SIRC_CUSTOMER_20BIT;
static u32 _u4Sirc_customer_code_20bit_dual = SIRC_CUSTOMER_20BIT_DUAL;
static u32 _u4Sirc_customer_code_20bit_trible = SIRC_CUSTOMER_20BIT_TRIBLE;

static int  cdev_sircs_key_probe(void);
static void cdev_sircs_key_release(void);
static long cdev_sircs_key_ioctl(struct file *filp, unsigned int cmd, unsigned int arg);

static dev_t            m_devid;
static struct cdev      m_cdev;
static struct class *class = NULL;
static int sircs_char_dev_init = 0;

static struct rc_map_list mtk_sirc_map = {
	.map = {
		.scan    = mtk_sirc_table,   // here for custom to modify
		.size    = ARRAY_SIZE(mtk_sirc_table),
		.rc_type = RC_TYPE_SONY20,
		.name    = RC_MAP_MTK_SIRC,
	}
};
static char Reverse1Byte(char ucSrc)
{
  char ucRet=0;
  int i;
  char ucTemp;
  for(i=0;i<8;i++)
  {
 	 ucTemp=1<<i;
 	 if(ucSrc&ucTemp)
 	 {
 		 ucRet+=1<<(7-i);
 	 }
  }
 
  return ucRet;
}
 
static u32 mtk_ir_sirc_get_customer_code(void)
{
	IR_LOG_ALWAYS("_u4Sirc_customer_code_20 = 0x%x\n",_u4Sirc_customer_code_20bit);
 	return _u4Sirc_customer_code_20bit;
}

static u32 mtk_ir_sirc_set_customer_code(void * preserve)
{
	if (preserve == NULL)
	{
		return 0;
	}
	_u4Sirc_customer_code_20bit= *((u32 *)preserve);

	IR_LOG_ALWAYS("_u4Sirc_customer_code_20 = 0x%x\n",_u4Sirc_customer_code_20bit);
}
 
#ifdef CONFIG_HAS_EARLYSUSPEND

static void mtk_ir_sirc_early_suspend(void * preserve )
{
//	IR_LOG_ALWAYS("\n");
}

static void mtk_ir_sirc_late_resume(void * preserve )
{
//	IR_LOG_ALWAYS("\n");
}
#else

#define mtk_ir_sirc_early_suspend NULL
#define mtk_ir_sirc_late_resume NULL
#endif


#ifdef CONFIG_PM_SLEEP

static int mtk_ir_sirc_suspend(void * preserve )
{
//	IR_LOG_ALWAYS("\n");
	return 0;
}

static int mtk_ir_sirc_resume(void * preserve )
{
//	IR_LOG_ALWAYS("\n");
	return 0;
}

#else

#define mtk_ir_sirc_suspend NULL
#define mtk_ir_sirc_resume NULL

#endif

static struct mtk_ir_core_platform_data mtk_ir_pdata_sirc = {
	
	.input_name = MTK_INPUT_SIRC_DEVICE_NAME,
	.p_map_list = &mtk_sirc_map,
	.i4_keypress_timeout = MTK_IR_SIRC_KEYPRESS_TIMEOUT,
	.init_hw = mtk_ir_sirc_init_hw,
	.uninit_hw = mtk_ir_sirc_uninit_hw,
	.ir_hw_decode = mtk_ir_sirc_decode,
	.get_customer_code = mtk_ir_sirc_get_customer_code,
	.set_customer_code = mtk_ir_sirc_set_customer_code,
	
	#ifdef CONFIG_HAS_EARLYSUSPEND
	.early_suspend = mtk_ir_sirc_early_suspend,
	.late_resume =   mtk_ir_sirc_late_resume,
	#endif

	#ifdef CONFIG_PM_SLEEP
    .suspend = mtk_ir_sirc_suspend,
    .resume = mtk_ir_sirc_resume,
	#endif
	
};

static struct file_operations ir_sircs_fops={
  .unlocked_ioctl = cdev_sircs_key_ioctl,
  .compat_ioctl   = cdev_sircs_key_ioctl,
};

static long cdev_sircs_key_ioctl(struct file *filp, unsigned int cmd, unsigned int arg)
{

    switch( cmd ) {
      case ICX_KEY_IOCTL_CMD_GETCOUNT:
      {
          return sizeof(mtk_sirc_table) / sizeof(mtk_sirc_table[0]);
      }
      case ICX_KEY_IOCTL_CMD_GETINFO:
      {
        int i = 0;
        int *sircs_key = 0;
        int size = sizeof(mtk_sirc_table) / sizeof(mtk_sirc_table[0]);
        
        sircs_key = kzalloc(size * sizeof(int), GFP_KERNEL);
        
        if(sircs_key == NULL){
            printk("%s:kzalloc err!!!\n",__func__);
            return -1;
        }
        
        for(i = 0; i < size; i++){
            sircs_key[i] = mtk_sirc_table[i].keycode;
        }

        if( copy_to_user((void __user *)arg, &sircs_key[0], (size * sizeof(int)) )) {
            kfree(sircs_key);
            return -EFAULT;
        }

        kfree(sircs_key);
        break;
      }
      case ICX_KEY_IOCTL_CMD_GET_RESUME_KEY:
      {
        int start_key = -EFAULT;
        int scancode = 0;
        int i = 0;
        int size = sizeof(mtk_sirc_table) / sizeof(mtk_sirc_table[0]);
        
        scancode = sircs_get_start_code();
        
        if(scancode <= 0 || scancode == BTN_NONE){
            return -EFAULT; //Bad address
        }
        
        for (i = 0; i < size; i++)
        { 
           if (mtk_sirc_table[i].scancode == scancode)
           {
              start_key = mtk_sirc_table[i].keycode;
		      break;
	       }	
        }

        return start_key;
      }
      default:
        return -EINVAL; //invalid argument
    }

    return 0;
}

static int cdev_sircs_key_probe(void)
{
    int ret = 0;
    dev_t devid = 0;
    struct device *dev = NULL;

    /* get a major number */
    ret = alloc_chrdev_region(&m_devid, 0, 1, SIRCS_FILE_NAME);
    if(ret != 0){
        pr_err("%s: major number Error ret = %d \n", __func__, ret);
        return ret;
    }
    pr_info("major = %d\n", MAJOR(m_devid));

    /* initialize cdev */
    cdev_init(&m_cdev, &ir_sircs_fops);
    m_cdev.owner = THIS_MODULE;
    
    ret = cdev_add(&m_cdev, m_devid, 1);
    if(ret != 0){
        goto cdev_add_err;
    }
    
    class = class_create(THIS_MODULE, SIRCS_FILE_NAME);
    if (IS_ERR(class)){
       pr_err("class create error\n");
       goto class_create_err;
    }
    
    dev = device_create(class, NULL, m_devid, NULL, SIRCS_FILE_NAME);
    if (IS_ERR(dev)){
        pr_err(" device create Error \n");
        goto device_create_err;
    }
    
    return 0;

device_create_err:
  class_destroy( class );

class_create_err:
  cdev_del( &m_cdev );

cdev_add_err:
  unregister_chrdev_region(m_devid, 1);
  return -1;

}

static void cdev_sircs_key_release(void)
{
    device_destroy(class, m_devid);
    class_destroy(class);
    cdev_del( &m_cdev );
    unregister_chrdev_region(m_devid, 1);

    return;
}

struct mtk_ir_device  mtk_ir_dev_sirc = { 
   .dev_platform = {
   	  .name		  = MTK_IR_DRIVER_NAME, // here must be equal to 
	  .id		  = MTK_IR_ID_SIRC,
	  .dev = {
	           .platform_data = &mtk_ir_pdata_sirc,
			   .release       = release,
	         },
   	},
   	
};

static int mtk_ir_sirc_uninit_hw(void)
{   
   
    if(sircs_char_dev_init == 1){
        cdev_sircs_key_release();
        sircs_char_dev_init = 0;
    }
   
    // disable ir interrupt
	IR_WRITE_MASK(IRRX_IRINT_EN,IRRX_INTEN_MASK,IRRX_INTCLR_OFFSET,0x0); 
	mtk_ir_core_clear_hwirq_stat();  
	rc_map_unregister(&mtk_sirc_map);
	
	return 0;
}


static int mtk_ir_sirc_init_hw(void)
{  
   MTK_IR_MODE ir_mode = mtk_ir_core_getmode();

   IR_LOG_ALWAYS("ir_mode = %d\n",ir_mode);

   if(sircs_char_dev_init == 0){
      cdev_sircs_key_probe();
      sircs_char_dev_init = 1;
   }
   
   if (ir_mode == MTK_IR_FACTORY)
   {
   	 mtk_sirc_map.map.scan = mtk_sirc_factory_table;
	 mtk_sirc_map.map.size = ARRAY_SIZE(mtk_sirc_factory_table);
   }
   else
   {
   	 mtk_sirc_map.map.scan = mtk_sirc_table;
	 mtk_sirc_map.map.size = ARRAY_SIZE(mtk_sirc_table);
	 
	 #if MTK_IRRX_AS_MOUSE_INPUT
	 struct rc_map_table *p_table = mtk_sirc_map.map.scan;
	 int size = mtk_sirc_map.map.size;
	 int i = 0;
	 memset(&(mtk_ir_pdata_sirc.mouse_code),0xff, sizeof(mtk_ir_pdata_sirc.mouse_code));
	  
     for (; i< size; i++)
     {
     	if (p_table[i].keycode == KEY_LEFT)
     	{
     	    mtk_ir_pdata_sirc.mouse_code.scanleft = p_table[i].scancode;
			
     	}
		else if(p_table[i].keycode == KEY_RIGHT)
		{
		    mtk_ir_pdata_sirc.mouse_code.scanright = p_table[i].scancode;
		}
		else if (p_table[i].keycode == KEY_UP)
		{
			
			mtk_ir_pdata_sirc.mouse_code.scanup = p_table[i].scancode;
		}
		else if (p_table[i].keycode == KEY_DOWN)
		{
			
			mtk_ir_pdata_sirc.mouse_code.scandown = p_table[i].scancode;
		}
		else if (p_table[i].keycode == KEY_ENTER)
		{
			
			mtk_ir_pdata_sirc.mouse_code.scanenter = p_table[i].scancode;
		}
		
     }
	 
	 mtk_ir_pdata_sirc.mouse_code.scanswitch = MTK_IR_MOUSE_SIRC_SWITCH_CODE;
	 mtk_ir_pdata_sirc.mousename[0] = '\0';
	 strcat(mtk_ir_pdata_sirc.mousename,mtk_ir_pdata_sirc.input_name);
	 strcat(mtk_ir_pdata_sirc.mousename,"_Mouse");	 	
     #endif
	 
   }
   
   rc_map_register(&mtk_sirc_map);

   //first setting power key//
  
   #if 1
   IR_WRITE32(IRRX_EXPEN ,0x1);
   IR_WRITE32(IRRX_EXP_IRM1,MTK_SIRC_EXP_POWE_KEY1);
   IR_WRITE32(IRRX_EXP_IRL1,MTK_SIRC_EXP_POWE_KEY2);

   // disable interrupt 
   IR_WRITE_MASK(IRRX_IRINT_EN,IRRX_INTEN_MASK,IRRX_INTCLR_OFFSET,0x0);
   
   IR_WRITE32(IRRX_CONFIG_HIGH_REG,  MTK_SIRC_CONFIG); 
   IR_WRITE32(IRRX_CONFIG_LOW_REG,  MTK_SIRC_SAPERIOD);
   IR_WRITE32(IRRX_THRESHOLD_REG,  MTK_SIRC_THRESHOLD);

   
   mtk_ir_core_clear_hwirq_stat(); 
   
   // enable ir interrupt
   IR_WRITE_MASK(IRRX_IRINT_EN,IRRX_INTEN_MASK,IRRX_INTCLR_OFFSET,0x1);
   #endif
	  
   return 0;
	
}

u32  mtk_ir_sirc_decode( void * preserve)
{
    u32 u4BitCnt;   
    u32 u1Command, u1Device, u1Extended,sirc_key;
    u32 _au4IrRxData[2];   
    u32 _u4Info = IR_READ32(IRRX_COUNT_HIGH_REG);
    _au4IrRxData[0] = IR_READ32(IRRX_COUNT_MID_REG);
    _au4IrRxData[1] = IR_READ32(IRRX_COUNT_LOW_REG);
    
    char *pu1Data = (char *)_au4IrRxData;
    u4BitCnt = INFO_TO_BITCNT(_u4Info);

    if((0 != _au4IrRxData[0]) || (0 != _au4IrRxData[1]) || _u4Info != 0)
    {       
	    IR_LOG_KEY( "RxIsr Info:0x%08x data: 0x%08x%08x\n", _u4Info, _au4IrRxData[1], _au4IrRxData[0]);
    }
	else
	{
       IR_LOG_KEY("invalid key!!!\n");
		return BTN_INVALID_KEY; 
	}
	 switch (u4BitCnt)
    {
        case IRRX_SIRC_BITCNT12:
			          
            u1Command = (pu1Data[0] >> 1);
			u1Command = Reverse1Byte(u1Command<<1);
			
			u1Device = ((pu1Data[0] & 0x01) << 4) | ((pu1Data[1] & 0xF0) >> 4);
			u1Device = Reverse1Byte(u1Device<<3);
			sirc_key = SIRC_KEY_CODE(SIRC_LENGTH_12,u1Device,u1Command);
            IR_LOG_KEY("[irrx] SIRC:Received 12B Key: 0x%02x, Device = 0x%02x,sirc_key =0x%08x\n", u1Command, u1Device,sirc_key);
			/* Check GroupId. */
			if (u1Device != _u4Sirc_customer_code_12bit)
			{
				IR_LOG_KEY("Received 12B :invalid customer code 0x%x!!!\n",u1Device);
				_u4PrevKey = BTN_NONE;
				return BTN_NONE;
			}
			
			break;
          				
        case IRRX_SIRC_BITCNT15:
			
            u1Command = (pu1Data[0] >> 1);
			u1Command = Reverse1Byte(u1Command<<1);
			
            u1Device = ((pu1Data[0] & 0x01) << 7) | ((pu1Data[1] & 0xFE) >> 1);
            u1Device = Reverse1Byte(u1Device);
			sirc_key = SIRC_KEY_CODE(SIRC_LENGTH_15, u1Device,u1Command);
            IR_LOG_KEY("SIRC:Received 15B Key: 0x%02x, u1Device = 0x%02x,sirc_key =:0x%08x \n", u1Command,u1Device,sirc_key);
			/* Check GroupId. */
			if (u1Device != _u4Sirc_customer_code_15bit)
			{
				IR_LOG_KEY("Received 15B :invalid customer code 0x%x!!!\n",u1Device);
				_u4PrevKey = BTN_NONE;
				return BTN_NONE;
			}
			
			break;
    
        case IRRX_SIRC_BITCNT20:
			
            u1Command = (pu1Data[0] >> 1);
			u1Command = Reverse1Byte(u1Command<<1);
			
			u1Device = ((pu1Data[0] & 0x01) << 4) | ((pu1Data[1] & 0xF0) >> 4);
			u1Device = Reverse1Byte(u1Device<<3);

			u1Extended = ((pu1Data[1] & 0x0F) << 4) | ((pu1Data[2] & 0xF0) >> 4);
			u1Extended = Reverse1Byte(u1Extended);
			//IR_LOG_KEY("SIRC:Received 20B Device = 0x%x, u1Extended:0x%x\n", u1Device, u1Extended);
			u1Device = ((u1Extended&0xff) <<5) + ((u1Device)&0x1f);
			sirc_key = SIRC_KEY_CODE(SIRC_LENGTH_20, u1Device,u1Command);
			
	        IR_LOG_KEY("SIRC:Received 20B Key: 0x%x, Device = 0x%x, sirc_key:0x%08x\n", u1Command, u1Device,sirc_key);
			    /* Check GroupId. */
		    if ((u1Device != _u4Sirc_customer_code_20bit)&&(u1Device != _u4Sirc_customer_code_20bit_dual)\
				&&(u1Device != _u4Sirc_customer_code_20bit_trible))
				
		    {
		        IR_LOG_KEY("Received 20B :invalid customer code 0x%x!!!\n",u1Device);
				_u4PrevKey = BTN_NONE;
				return BTN_NONE;
		    }
			
			break;
			
         default:
            IR_LOG_KEY("BITCNT unmatch:not sirc key\n");
			return BTN_NONE;
            break;
    }
    return sirc_key;
	
}






