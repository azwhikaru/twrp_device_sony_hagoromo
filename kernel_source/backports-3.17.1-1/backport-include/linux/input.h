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
#ifndef __BACKPORT_INPUT_H
#define __BACKPORT_INPUT_H
#include_next <linux/input.h>

#ifndef KEY_WIMAX
#define KEY_WIMAX		246
#endif

#ifndef KEY_WPS_BUTTON
#define KEY_WPS_BUTTON		0x211
#endif

#ifndef KEY_RFKILL
#define KEY_RFKILL		247
#endif

#ifndef SW_RFKILL_ALL
#define SW_RFKILL_ALL           0x03
#endif

#endif /* __BACKPORT_INPUT_H */
