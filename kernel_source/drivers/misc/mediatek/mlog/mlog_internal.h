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
#ifndef _MLOG_INTERNAL_H
#define _MLOG_INTERNAL_H

#include <linux/printk.h>

#ifndef CONFIG_ICX_SILENT_LOG
#define MLOG_DEBUG
#endif

#ifdef MLOG_DEBUG
#define MLOG_PRINTK(args...)    pr_info(args)
#else
#define MLOG_PRINTK(args...)    do { } while (0)
#endif

#endif
