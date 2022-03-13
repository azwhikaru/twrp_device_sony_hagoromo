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
#ifndef _BACKPORTLINUX_MMC_HOST_H
#define _BACKPORTLINUX_MMC_HOST_H
#include_next <linux/mmc/host.h>
#include <linux/version.h>
#include <linux/mmc/card.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)
#define mmc_card_hs LINUX_BACKPORT(mmc_card_hs)
static inline int mmc_card_hs(struct mmc_card *card)
{
	return card->host->ios.timing == MMC_TIMING_SD_HS ||
		card->host->ios.timing == MMC_TIMING_MMC_HS;
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0) */

#endif /* _BACKPORTLINUX_MMC_HOST_H */
