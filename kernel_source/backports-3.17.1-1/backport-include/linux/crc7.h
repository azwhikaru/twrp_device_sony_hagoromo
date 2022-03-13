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
#ifndef _BACKPORT_LINUX_CRC7_H
#define _BACKPORT_LINUX_CRC7_H
#include_next <linux/crc7.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)
#define crc7_be LINUX_BACKPORT(crc7_be)
static inline u8 crc7_be(u8 crc, const u8 *buffer, size_t len)
{
	return crc7(crc, buffer, len) << 1;
}
#endif /* < 3.16 */

#endif /* _BACKPORT_LINUX_CRC7_H */
