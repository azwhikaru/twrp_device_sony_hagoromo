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
#ifndef _BACKPORT_DMA_BUF_H__
#define _BACKPORT_DMA_BUF_H__
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
#include_next <linux/dma-buf.h>
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#define dma_buf_export(priv, ops, size, flags, resv)	\
	dma_buf_export(priv, ops, size, flags)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0)
#undef dma_buf_export
#define dma_buf_export(priv, ops, size, flags, resv)	\
	dma_buf_export_named(priv, ops, size, flags, KBUILD_MODNAME)
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0) */

#endif /* _BACKPORT_DMA_BUF_H__ */
