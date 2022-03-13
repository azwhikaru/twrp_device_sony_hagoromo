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
#ifndef __BACKPORT__LINUX_USB_CH9_H
#define __BACKPORT__LINUX_USB_CH9_H

#include <linux/version.h>
#include_next <linux/usb/ch9.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
#include <linux/types.h>    /* __u8 etc */
#include <asm/byteorder.h>  /* le16_to_cpu */

/**
 * usb_endpoint_maxp - get endpoint's max packet size
 * @epd: endpoint to be checked
 *
 * Returns @epd's max packet
 */
#define usb_endpoint_maxp LINUX_BACKPORT(usb_endpoint_maxp)
static inline int usb_endpoint_maxp(const struct usb_endpoint_descriptor *epd)
{
	return __le16_to_cpu(epd->wMaxPacketSize);
}
#endif /* < 3.2 */

#endif /* __BACKPORT__LINUX_USB_CH9_H */
