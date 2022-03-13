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
#ifndef __BACKPORT_MMC_SDIO_FUNC_H
#define __BACKPORT_MMC_SDIO_FUNC_H
#include <linux/version.h>
#include_next <linux/mmc/sdio_func.h>

#ifndef dev_to_sdio_func
#define dev_to_sdio_func(d)	container_of(d, struct sdio_func, dev)
#endif

#endif /* __BACKPORT_MMC_SDIO_FUNC_H */
