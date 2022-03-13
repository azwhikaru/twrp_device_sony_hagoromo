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
#ifndef __BACKPORT_WATCHDOG_H
#define __BACKPORT_WATCHDOG_H
#include_next <linux/watchdog.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,1,0)
#define watchdog_device LINUX_BACKPORT(watchdog_device)
struct watchdog_device {
};
#endif

#endif /* __BACKPORT_WATCHDOG_H */
