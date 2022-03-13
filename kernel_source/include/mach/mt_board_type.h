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
#ifndef _MT_BOARD_TYPE_H
#define _MT_BOARD_TYPE_H

#define GPIO_PHONE_EVB_DETECT (GPIO143|0x80000000)

/* MTK_POWER_EXT_DETECT */
enum mt_board_type {
	MT_BOARD_NONE = 0,
	MT_BOARD_EVB = 1,
	MT_BOARD_PHONE = 2
};

static DEFINE_SPINLOCK(mt_board_lock);

#endif
