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
#ifndef __MTK_IR_CUS_NEC_DEFINE_H__
#define __MTK_IR_CUS_NEC_DEFINE_H__

#include "mtk_ir_cus_define.h"

#ifdef MTK_LK_IRRX_SUPPORT // platform/mt8127/lk/rule.mk
#include <platform/mtk_ir_lk_core.h>
#else
#include <media/rc-map.h>
#endif


#ifdef MTK_LK_IRRX_SUPPORT

// this table is using in lk,  for lk boot_menu select
static struct mtk_ir_lk_msg mtk_rstep_lk_table[] = {
	{0x19, KEY_UP},	
	{0x1a, KEY_DOWN},	
	{0x1e, KEY_ENTER},	
};

#else

//this table is used in factory mode, for factory_mode menu select
static struct rc_map_table mtk_rstep_factory_table[] = {
	{0x19, KEY_VOLUMEUP},	
	{0x1a, KEY_VOLUMEDOWN},	
	{0x1e, KEY_POWER},	
};


// this table is used in normal mode, for normal_boot 
static struct rc_map_table mtk_rstep_table[] = {
	{0x02, KEY_1}, 
	{0x05, KEY_2},
	{0x06, KEY_3},
	{0x09, KEY_4},
	{0x0a, KEY_5},
    {0x0d, KEY_6},
    {0x0e, KEY_7},
    {0x11, KEY_8},
    {0x12, KEY_9},	 
	{0x15, KEY_0},
    {0x16, KEY_POWER},
    {0x19, KEY_UP},	
	{0x1a, KEY_DOWN},
	{0x1d, KEY_LEFT}, 
	{0x1e, KEY_ENTER},
	{0x21, KEY_RIGHT},
	{0x22, KEY_BACKSPACE},
	{0x5a, KEY_HOMEPAGE},	
	{0x3a, KEY_BACK}, 

#if MTK_IRRX_AS_MOUSE_INPUT
    {0xffff, KEY_HELP},   // be carefule this key is used to send,but no response
#endif	 
};

#define MTK_IR_RSTEP_BIT_COUNT  0x19
#define MTK_IR_RSTEP_CUSTOMER_CODE  0x5 //here is nec's customer code
#define MTK_IR_RSTEP_ADDRESS  0x1
#define MTK_IR_RSTEP_FRAME_TYPE  0x2



#define MTK_IR_MOUSE_RSTEP_SWITCH_CODE 0x31

#define MTK_IR_RSTEP_KEYPRESS_TIMEOUT 140

#endif
#endif


