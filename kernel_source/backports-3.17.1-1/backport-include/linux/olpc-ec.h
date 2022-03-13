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
#ifndef _COMPAT_LINUX_OLPC_EC_H
#define _COMPAT_LINUX_OLPC_EC_H

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
#include_next <linux/olpc-ec.h>
#endif /* (LINUX_VERSION_CODE > KERNEL_VERSION(3,6,0)) */

#endif	/* _COMPAT_LINUX_OLPC_EC_H */
