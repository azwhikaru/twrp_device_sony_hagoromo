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
#ifndef _BACKPORT_NET_ADDRCONF_H
#define _BACKPORT_NET_ADDRCONF_H 1

#include_next <net/addrconf.h>

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0)
static inline bool ipv6_addr_is_solict_mult(const struct in6_addr *addr)
{
#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS) && BITS_PER_LONG == 64
	__u64 *p = (__u64 *)addr;
	return ((p[0] ^ cpu_to_be64(0xff02000000000000UL)) |
		((p[1] ^ cpu_to_be64(0x00000001ff000000UL)) &
		 cpu_to_be64(0xffffffffff000000UL))) == 0UL;
#else
	return ((addr->s6_addr32[0] ^ htonl(0xff020000)) |
		addr->s6_addr32[1] |
		(addr->s6_addr32[2] ^ htonl(0x00000001)) |
		(addr->s6_addr[12] ^ 0xff)) == 0;
#endif
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0) */

#endif	/* _BACKPORT_NET_ADDRCONF_H */
