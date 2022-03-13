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
#ifndef __BACKPORT_MMC_SDIO_H
#define __BACKPORT_MMC_SDIO_H
#include <linux/version.h>
#include_next <linux/mmc/sdio.h>

/* backports b4625dab */
#ifndef SDIO_CCCR_REV_3_00
#define  SDIO_CCCR_REV_3_00    3       /* CCCR/FBR Version 3.00 */
#endif
#ifndef SDIO_SDIO_REV_3_00
#define  SDIO_SDIO_REV_3_00    4       /* SDIO Spec Version 3.00 */
#endif

#ifndef SDIO_BUS_ECSI
#define  SDIO_BUS_ECSI		0x20	/* Enable continuous SPI interrupt */
#endif
#ifndef SDIO_BUS_SCSI
#define  SDIO_BUS_SCSI		0x40	/* Support continuous SPI interrupt */
#endif

#endif /* __BACKPORT_MMC_SDIO_H */
