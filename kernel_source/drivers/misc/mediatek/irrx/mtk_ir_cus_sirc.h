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
/* --------------------------------------------------------------- */
/*  Copyright 2016 SONY Corporation                                */
/* --------------------------------------------------------------- */

#ifndef __MTK_IR_CUS_SIRC_DEFINE_H__
#define __MTK_IR_CUS_SIRC_DEFINE_H__

#include "mtk_ir_cus_define.h"
#define SIRC_KEY_CODE(LENGTH,CUSTOM,DATA) (((LENGTH<<24)&0xff000000) | ((CUSTOM<<8)&0xffff00) | ((DATA)&0xff))
//bit32-bit24:sirc length;  bit23-bit8:custom code; bit8-bit0:keydata

#ifdef MTK_LK_IRRX_SUPPORT // platform/mt8127/lk/rule.mk
#include <platform/mtk_ir_lk_core.h>
#else
#include <media/rc-map.h>
#endif

#define SIRC_LENGTH_12 (0x0c)//12
#define SIRC_LENGTH_15 (0x0f)//15
#define SIRC_LENGTH_20 (0x14)//20

//define of customer code(also called category code)
#define SIRC_CUSTOMER_12BIT  (0x10) 
#define SIRC_CUSTOMER_15BIT  (0x90) 
#define SIRC_CUSTOMER_20BIT  (0x73a) 
#define SIRC_CUSTOMER_20BIT_DUAL  (0x410) 
#define SIRC_CUSTOMER_20BIT_TRIBLE  (0xc10) 

#define MTK_IR_MOUSE_SIRC_SWITCH_CODE 0x68

#define MTK_IR_SIRC_KEYPRESS_TIMEOUT 100

#define SIRCS_FILE_NAME "sircs_key"

#ifdef MTK_LK_IRRX_SUPPORT

// this table is using in lk,  for lk boot_menu select
static struct mtk_ir_lk_msg mtk_sirc_lk_table[] = {
	{0x78, KEY_UP},	
	{0x79, KEY_DOWN},	
	{0x7c, KEY_ENTER},	
};

#else

//this table is used in factory mode, for factory_mode menu select
static struct rc_map_table mtk_sirc_factory_table[] = {
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x78), KEY_VOLUMEUP},	
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x79), KEY_VOLUMEDOWN},	
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x7c), KEY_POWER},	
};

// this table is used in normal mode, for normal_boot 
static struct rc_map_table mtk_sirc_table[] = {

	{SIRC_KEY_CODE(SIRC_LENGTH_12,SIRC_CUSTOMER_12BIT,0x12), KEY_VOLUMEUP},
	{SIRC_KEY_CODE(SIRC_LENGTH_12,SIRC_CUSTOMER_12BIT,0x13), KEY_VOLUMEDOWN},
	{SIRC_KEY_CODE(SIRC_LENGTH_12,SIRC_CUSTOMER_12BIT,0x15), KEY_POWER},
	{SIRC_KEY_CODE(SIRC_LENGTH_15,SIRC_CUSTOMER_15BIT,0x69), KEY_1}, 
	{SIRC_KEY_CODE(SIRC_LENGTH_15,SIRC_CUSTOMER_15BIT,0x6a), KEY_1}, 
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x00), KEY_1}, 
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x01), KEY_2},
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x02), KEY_3},
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x03), KEY_4},
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x04), KEY_5},
    {SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x05), KEY_6},
    {SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x06), KEY_7},
    {SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x07), KEY_8},
    {SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x08), KEY_9},
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x09), KEY_0},	
    {SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x32), KEY_PLAYCD},
    {SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x39), KEY_PAUSECD},
    {SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x38), KEY_STOPCD},
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x7a), KEY_LEFT}, 
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x7b), KEY_RIGHT},
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x7c), KEY_ENTER},
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x19), KEY_HOMEPAGE},	
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT,0x7d), KEY_BACK}, 
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT_DUAL,0x29), KEY_SEARCH}, 
	{SIRC_KEY_CODE(SIRC_LENGTH_20,SIRC_CUSTOMER_20BIT_TRIBLE,0x7d),KEY_SPORT}, 


#if MTK_IRRX_AS_MOUSE_INPUT
    {0xffff, KEY_HELP},   // be carefule this key is used to send,but no response
#endif	 
};

#endif

#endif


