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
#ifndef __BACKPORT_PLATFORM_DEVICE_H
#define __BACKPORT_PLATFORM_DEVICE_H

#include_next <linux/platform_device.h>
#include <linux/version.h>

#ifndef module_platform_driver_probe
#define module_platform_driver_probe(__platform_driver, __platform_probe) \
static int __init __platform_driver##_init(void) \
{ \
	return platform_driver_probe(&(__platform_driver), \
				     __platform_probe);    \
} \
module_init(__platform_driver##_init); \
static void __exit __platform_driver##_exit(void) \
{ \
	platform_driver_unregister(&(__platform_driver)); \
} \
module_exit(__platform_driver##_exit);
#endif

#ifndef PLATFORM_DEVID_NONE
#define PLATFORM_DEVID_NONE	(-1)
#endif

#ifndef PLATFORM_DEVID_AUTO
#define PLATFORM_DEVID_AUTO	(-1)
#endif

#ifndef module_platform_driver
#define module_platform_driver(__platform_driver) \
        module_driver(__platform_driver, platform_driver_register, \
                        platform_driver_unregister)
#endif

#endif /* __BACKPORT_PLATFORM_DEVICE_H */
