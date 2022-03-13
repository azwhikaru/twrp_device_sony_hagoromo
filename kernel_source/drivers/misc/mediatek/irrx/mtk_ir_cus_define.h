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

#ifndef __MTK_IR_CUS_DEFINE_H__
#define __MTK_IR_CUS_DEFINE_H__

#define MTK_IR_ID_NEC 0
#define MTK_IR_ID_RC6 1
#define MTK_IR_ID_RC5 2
#define MTK_IR_ID_SIRC 3
#define MTK_IR_ID_RSTEP 4


#define MTK_IRRX_PROTOCOL MTK_IR_ID_SIRC


#ifdef MTK_LK_IRRX_SUPPORT  //using in lk
#define MTK_LK_DETECTED_REEPAT_TIMES  6
#define MTK_LK_WAIT_REPEAT_TIME_OUT   2000   //ms
#define MTK_LK_BOOT_SELECT_MENU_TIME_OUT 30 //s
#define MTK_LK_IRRX_USING_TIMER 0


#define MTK_LK_CUSTOMER_BOOT 0
#if  MTK_LK_CUSTOMER_BOOT 
#define MTK_LK_CUSTOMER_FASTLOGO_TIMES 5000
#endif

#endif


#define MTK_IRRX_AS_MOUSE_INPUT 1

#if MTK_IRRX_AS_MOUSE_INPUT
#define MOUSE_SMALL_X_STEP 10
#define MOUSE_SMALL_Y_STEP 10
#define MOUSE_LARGE_X_STEP 30
#define MOUSE_LARGE_Y_STEP 30

#endif

#endif

