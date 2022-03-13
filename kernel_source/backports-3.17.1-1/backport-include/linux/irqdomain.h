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
#ifndef __BACKPORT_LINUX_IRQDOMAIN_H
#define __BACKPORT_LINUX_IRQDOMAIN_H
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
#include_next <linux/irqdomain.h>
#endif

#endif /* __BACKPORT_LINUX_IRQDOMAIN_H */