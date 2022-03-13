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

#ifndef __MTK_IR_CUS_RC5_DEFINE_H__
#define __MTK_IR_CUS_RC5_DEFINE_H__

#include "mtk_ir_cus_define.h"
 
#ifdef MTK_LK_IRRX_SUPPORT // platform/mt8127/lk/rule.mk
#include <platform/mtk_ir_lk_core.h>

#else
#include <media/rc-map.h>
#endif
	 
	 
#ifdef MTK_LK_IRRX_SUPPORT
	 
	 // this table is using in lk,	for lk boot_menu select
static struct mtk_ir_lk_msg mtk_rc5_lk_table[] = 
{
	{0x06, KEY_UP}, 
	{0x07, KEY_DOWN},	 
	{0x5c, KEY_ENTER},  
};
	 
#else


static struct rc_map_table mtk_rc5_factory_table[] = {
	{0x06, KEY_VOLUMEDOWN}, 
	{0x07, KEY_VOLUMEUP}, 	
	{0x5c, KEY_POWER},	
};


static struct rc_map_table mtk_rc5_table[] = {
	{0x00, KEY_X}, 
	{0x01, KEY_ESC}, 
	{0x02, KEY_W}, 
	{0x03, KEY_LEFT}, 
	{0x04, KEY_SELECT},
	{0x05, KEY_RIGHT}, 
	{0x06, KEY_VOLUMEDOWN}, 
	{0x07, KEY_VOLUMEUP}, 
	{0x0e, KEY_POWER}, 
	{0x4c, KEY_HOMEPAGE}, 
	{0x5c, KEY_ENTER}, 
	
   #if MTK_IRRX_AS_MOUSE_INPUT
    {0xffff, KEY_HELP},   // be carefule this key is used to send,but no response
   #endif	
};
 

#define MTK_IR_RC5_CUSTOMER_CODE  0x00 //here is rc5's customer code

#define MTK_IR_MOUSE_RC5_SWITCH_CODE 0x00

#define MTK_IR_RC5_KEYPRESS_TIMEOUT 140


#endif

#endif


