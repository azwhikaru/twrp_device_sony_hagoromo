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
#ifndef __BACKPORT_LINUX_CRED_H
#define __BACKPORT_LINUX_CRED_H
#include_next <linux/cred.h>
#include <linux/version.h>

#ifndef current_user_ns
#define current_user_ns()	(current->nsproxy->user_ns)
#endif

#endif /* __BACKPORT_LINUX_CRED_H */
