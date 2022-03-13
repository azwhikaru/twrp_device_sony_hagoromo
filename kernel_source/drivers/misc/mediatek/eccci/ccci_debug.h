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
#ifndef __CCCI_DEBUG_H__
#define __CCCI_DEBUG_H__

/* tag defination */
#define CORE "cor"
#define BM "bfm"
#define NET "net"
#define CHAR "chr"
#define SYSFS "sfs"
#define KERN "ken"
#define IPC "ipc"
#define RPC "rpc"

extern unsigned int ccci_debug_enable;
#define CCCI_DBG_MSG(idx, tag, fmt, args...) \
do { \
	if (ccci_debug_enable == 1) \
		printk(KERN_DEBUG "[ccci/" tag "](%d)" fmt, (idx+1), ##args); \
	else if (ccci_debug_enable == 2) \
		printk(KERN_NOTICE "[ccci/" tag "](%d)" fmt, (idx+1), ##args); \
} while (0)
#define CCCI_INF_MSG(idx, tag, fmt, args...) printk(KERN_NOTICE"[ccci/" tag "](%d)" fmt, (idx+1), ##args)
#define CCCI_ERR_MSG(idx, tag, fmt, args...) printk(KERN_ERR "[ccci/err/" tag "](%d)" fmt, (idx+1), ##args)

/* only for ccci_dfo.c */
#define CCCI_DBG_COM_MSG(fmt, args...)		 CCCI_DBG_MSG(0, "com", fmt, ##args)
#define CCCI_INF_COM_MSG(fmt, args...)		 CCCI_INF_MSG(0, "com", fmt, ##args)
#define CCCI_ERR_COM_MSG(fmt, args...)		 CCCI_ERR_MSG(0, "com", fmt, ##args)

/* raw prink is only used to dump memory and statistic data */

/* #define CCCI_STATISTIC */
#define CCCI_STATISTIC_DUMP_INTERVAL 10	/* seconds */
#define CCCI_USE_NAPI

#endif				/* __CCCI_DEBUG_H__ */
