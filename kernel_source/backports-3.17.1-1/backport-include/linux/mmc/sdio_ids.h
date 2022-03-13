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
#ifndef __BACKPORT_MMC_SDIO_IDS_H
#define __BACKPORT_MMC_SDIO_IDS_H
#include <linux/version.h>
#include_next <linux/mmc/sdio_ids.h>

#ifndef SDIO_CLASS_BT_AMP
#define SDIO_CLASS_BT_AMP	0x09	/* Type-A Bluetooth AMP interface */
#endif

#ifndef SDIO_DEVICE_ID_MARVELL_8688WLAN
#define SDIO_DEVICE_ID_MARVELL_8688WLAN		0x9104
#endif

#endif /* __BACKPORT_MMC_SDIO_IDS_H */
