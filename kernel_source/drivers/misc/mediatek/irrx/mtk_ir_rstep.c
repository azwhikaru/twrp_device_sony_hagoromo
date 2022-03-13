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

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>

#include "mtk_ir_common.h"
#include "mtk_ir_core.h"
#include "mtk_ir_cus_rstep.h" // include customer's key map
#include "mtk_ir_regs.h"    // include ir registers




#define MTK_RSTEP_CONFIG      (IRRX_CH_END_15 |IRRX_CH_IGSYN |  IRRX_CH_HWIR | IRRX_CH_ORDINV| IRRX_CH_RC5|IRRX_CH_RC5_1ST )
#define MTK_RSTEP_SAPERIOD    (0xA) 
#define MTK_RSTEP_THRESHOLD   (0x1) 


#define MTK_RSTEP_EXP_POWE_KEY1   0x00000000
#define MTK_RSTEP_EXP_POWE_KEY2   0x00000000

#define RSTEP_INFO_TO_BITCNT(u4Info)      ((u4Info & IRRX_CH_BITCNT_MASK)    >> IRRX_CH_BITCNT_BITSFT)

#define MTK_RSTEP_GET_CUSTOM(bdata0) ((bdata0>>4))
#define MTK_RSTEP_GET_ADDRESS(bdata0) ((bdata0 & 0xc)>>2)
#define MTK_RSTEP_GET_FRAME_TYPE(bdata0) (bdata0 & 0x3)
#define MTK_RSTEP_GET_KEYCODE(bdata1,bdata2)  \
               ((((bdata2 & 0x80)>>7) | ((bdata1 & 0x7f)<<1)) & 0xff)

#define INFO_TO_BITCNT(u4Info)      ((u4Info & IRRX_CH_BITCNT_MASK)    >> IRRX_CH_BITCNT_BITSFT)

//set deglitch with the min number. when glitch < (33*6 = 198us,ignore)
 static int mtk_ir_rstep_init_hw(void);
static int mtk_ir_rstep_uninit_hw(void);
static u32 mtk_ir_rstep_decode( void * preserve);

static u32 _u4PrevKey = BTN_NONE;   // pre key
static u32 _u4Rstep_customer_code = MTK_IR_RSTEP_CUSTOMER_CODE;


static struct rc_map_list mtk_rstep_map = {
	.map = {
		.scan    = mtk_rstep_table,   // here for custom to modify
		.size    = ARRAY_SIZE(mtk_rstep_table),
		.rc_type = RC_TYPE_OTHER,
		.name    = RC_MAP_MTK_RSTEP,
	}
};
 
static u32 mtk_ir_rstep_get_customer_code(void)
{
 	return _u4Rstep_customer_code;
}

static u32 mtk_ir_rstep_set_customer_code(void * preserve)
{
    u32 customer_code;
	if (preserve == NULL)
	{
		return 0;
	}
	_u4Rstep_customer_code = *((u32 *)preserve);

	IR_LOG_ALWAYS("_u4Rstep_customer_code = 0x%x\n",_u4Rstep_customer_code);
}
 
#ifdef CONFIG_HAS_EARLYSUSPEND

static void mtk_ir_rstep_early_suspend(void * preserve )
{
	IR_LOG_ALWAYS("\n");
}

static void mtk_ir_rstep_late_resume(void * preserve )
{
	IR_LOG_ALWAYS("\n");
}
#else

#define mtk_ir_rstep_early_suspend NULL
#define mtk_ir_rstep_late_resume NULL
#endif


#ifdef CONFIG_PM_SLEEP

static int mtk_ir_rstep_suspend(void * preserve )
{
	IR_LOG_ALWAYS("\n");
	return 0;
}

static int mtk_ir_rstep_resume(void * preserve )
{
	IR_LOG_ALWAYS("\n");
	return 0;
}

#else

#define mtk_ir_rstep_suspend NULL
#define mtk_ir_rstep_resume NULL

#endif

static struct mtk_ir_core_platform_data mtk_ir_pdata_rstep = {
	
	.input_name = MTK_INPUT_RSTEP_DEVICE_NAME,
	.p_map_list = &mtk_rstep_map,
	.i4_keypress_timeout = MTK_IR_RSTEP_KEYPRESS_TIMEOUT,
	.init_hw = mtk_ir_rstep_init_hw,
	.uninit_hw = mtk_ir_rstep_uninit_hw,
	.ir_hw_decode = mtk_ir_rstep_decode,
	.get_customer_code = mtk_ir_rstep_get_customer_code,
	.set_customer_code = mtk_ir_rstep_set_customer_code,
	
	#ifdef CONFIG_HAS_EARLYSUSPEND
	.early_suspend = mtk_ir_rstep_early_suspend,
	.late_resume =   mtk_ir_rstep_late_resume,
	#endif

	#ifdef CONFIG_PM_SLEEP
    .suspend = mtk_ir_rstep_suspend,
    .resume = mtk_ir_rstep_resume,
	#endif
	
};


struct mtk_ir_device  mtk_ir_dev_rstep = { 
   .dev_platform = {
   	  .name		  = MTK_IR_DRIVER_NAME, // here must be equal to 
	  .id		  = MTK_IR_ID_RSTEP,
	  .dev = {
	           .platform_data = &mtk_ir_pdata_rstep,
			   .release       = release,
	         },
   	},
   	
};

static int mtk_ir_rstep_uninit_hw(void)
{   
   
    // disable ir interrupt
	IR_WRITE_MASK(IRRX_IRINT_EN,IRRX_INTEN_MASK,IRRX_INTCLR_OFFSET,0x0); 
	mtk_ir_core_clear_hwirq_stat();  
	rc_map_unregister(&mtk_rstep_map);
	
	return 0;
}


static int mtk_ir_rstep_init_hw(void)
{  
   MTK_IR_MODE ir_mode = mtk_ir_core_getmode();

   IR_LOG_ALWAYS("ir_mode = %d\n",ir_mode);
   
   if (ir_mode == MTK_IR_FACTORY)
   {
   	 mtk_rstep_map.map.scan = mtk_rstep_factory_table;
	 mtk_rstep_map.map.size = ARRAY_SIZE(mtk_rstep_factory_table);
   }
   else
   {
   	 mtk_rstep_map.map.scan = mtk_rstep_table;
	 mtk_rstep_map.map.size = ARRAY_SIZE(mtk_rstep_table);
	 
	 #if MTK_IRRX_AS_MOUSE_INPUT
	 struct rc_map_table *p_table = mtk_rstep_map.map.scan;
	 int size = mtk_rstep_map.map.size;
	 int i = 0;
	 memset(&(mtk_ir_pdata_rstep.mouse_code),0xff, sizeof(mtk_ir_pdata_rstep.mouse_code));
	  
     for (; i< size; i++)
     {
     	if (p_table[i].keycode == KEY_LEFT)
     	{
     	    mtk_ir_pdata_rstep.mouse_code.scanleft = p_table[i].scancode;
			
     	}
		else if(p_table[i].keycode == KEY_RIGHT)
		{
		    mtk_ir_pdata_rstep.mouse_code.scanright = p_table[i].scancode;
		}
		else if (p_table[i].keycode == KEY_UP)
		{
			
			mtk_ir_pdata_rstep.mouse_code.scanup = p_table[i].scancode;
		}
		else if (p_table[i].keycode == KEY_DOWN)
		{
			
			mtk_ir_pdata_rstep.mouse_code.scandown = p_table[i].scancode;
		}
		else if (p_table[i].keycode == KEY_ENTER)
		{
			
			mtk_ir_pdata_rstep.mouse_code.scanenter = p_table[i].scancode;
		}
		
     }
	 
	 mtk_ir_pdata_rstep.mouse_code.scanswitch = MTK_IR_MOUSE_RSTEP_SWITCH_CODE;
	 mtk_ir_pdata_rstep.mousename[0] = '\0';
	 strcat(mtk_ir_pdata_rstep.mousename,mtk_ir_pdata_rstep.input_name);
	 strcat(mtk_ir_pdata_rstep.mousename,"_Mouse");	 	
     #endif
	 
   }
   
   rc_map_register(&mtk_rstep_map);

   //first setting power key//
  
   #if 1
   IR_WRITE32(IRRX_EXP_IRM1,MTK_RSTEP_EXP_POWE_KEY1);
   IR_WRITE32(IRRX_EXP_IRL1,MTK_RSTEP_EXP_POWE_KEY2);

   // disable interrupt 
   IR_WRITE_MASK(IRRX_IRINT_EN,IRRX_INTEN_MASK,IRRX_INTCLR_OFFSET,0x0);
   
   IR_WRITE32(IRRX_CONFIG_HIGH_REG,  MTK_RSTEP_CONFIG); 
   IR_WRITE32(IRRX_CONFIG_LOW_REG,  MTK_RSTEP_SAPERIOD);
   IR_WRITE32(IRRX_THRESHOLD_REG,  MTK_RSTEP_THRESHOLD);

   
   mtk_ir_core_clear_hwirq_stat(); 
   
   // enable ir interrupt
   IR_WRITE_MASK(IRRX_IRINT_EN,IRRX_INTEN_MASK,IRRX_INTCLR_OFFSET,0x1);
   #endif
	  
   return 0;
	
}

u32  mtk_ir_rstep_decode( void * preserve)
{
    u32 _au4IrRxData[2];   
   u32 _u4Info = IR_READ32(IRRX_COUNT_HIGH_REG);
   _au4IrRxData[0] = IR_READ32(IRRX_COUNT_MID_REG);
   _au4IrRxData[1] = IR_READ32(IRRX_COUNT_LOW_REG);
    
    char *pu1Data = (char *)_au4IrRxData;
    u16 u4BitCnt = RSTEP_INFO_TO_BITCNT(_u4Info);
    u16 u2RSTEPCustom = MTK_RSTEP_GET_CUSTOM(pu1Data[0]);
    u16 u2RSTEPAddress = MTK_RSTEP_GET_ADDRESS(pu1Data[0]);
    u16 u2RSTEPFrametype = MTK_RSTEP_GET_FRAME_TYPE(pu1Data[0]);
    u32 u4RSTEPkey = MTK_RSTEP_GET_KEYCODE(pu1Data[1], pu1Data[2]);
    
    if((0 != _au4IrRxData[0]) || (0 != _au4IrRxData[1]) || _u4Info != 0)
    {
       
	    IR_LOG_KEY( "RxIsr Info:0x%08x data: 0x%08x%08x\n", _u4Info,
	 	                          _au4IrRxData[1], _au4IrRxData[0]);
    }
	else
	{
       IR_LOG_KEY("invalid key!!!\n");
		return BTN_INVALID_KEY; 
	}
    if(MTK_IR_RSTEP_BIT_COUNT!= u4BitCnt) 
    {
          IR_LOG_KEY( "invalid key:BitCnt ERROR:0x%x.\n",u4BitCnt);
          return BTN_INVALID_KEY; 
    }
    if(_u4Rstep_customer_code!= u2RSTEPCustom) 
    {
          IR_LOG_KEY( "invalid key:Custom Code ERROR:0x%x.please modify mtk_ir_cust_rsetp.h\n",u2RSTEPCustom);
          return BTN_INVALID_KEY; 
    }
    if(MTK_IR_RSTEP_ADDRESS != u2RSTEPAddress) 
    {
          IR_LOG_KEY( "invalid key:RSTEP_ADDRESS ERROR:0x%x.\n",u2RSTEPAddress);
          return BTN_INVALID_KEY; 
    }
    if(MTK_IR_RSTEP_FRAME_TYPE != u2RSTEPFrametype) 
    {
          IR_LOG_KEY( "invalid key:RSTEP_ADDRESS ERROR:0x%x.this is not rstep reomte control!\n",u2RSTEPFrametype);
          return BTN_INVALID_KEY; 
    }
    
    IR_LOG_KEY( "R_STEP decoder:BitCnt: 0x%x,Custom: 0x%x, Address: 0x%x, Frametype: 0x%x, Keycode; 0x%x\n", 
              u4BitCnt, u2RSTEPCustom,u2RSTEPAddress,u2RSTEPFrametype,u4RSTEPkey);
    

 #if 0   /* Check GroupId. */
    if (u2RSTEPCustom != _u4Rstep_customer_code)
    {
        IR_LOG_KEY("invalid customer code 0x%x!!!\n",u1Device);
		_u4PrevKey = BTN_NONE;
		return BTN_NONE;
    }
 #endif
    return u4RSTEPkey;
	
}






