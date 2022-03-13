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
#ifndef __BACKPORT_SOCKET_H
#define __BACKPORT_SOCKET_H
#include_next <linux/socket.h>

#ifndef SOL_NFC
/*
 * backport SOL_NFC -- see commit:
 * NFC: llcp: Implement socket options
 */
#define SOL_NFC		280
#endif

#ifndef __sockaddr_check_size
#define __sockaddr_check_size(size)	\
	BUILD_BUG_ON(((size) > sizeof(struct __kernel_sockaddr_storage)))
#endif

#endif /* __BACKPORT_SOCKET_H */
