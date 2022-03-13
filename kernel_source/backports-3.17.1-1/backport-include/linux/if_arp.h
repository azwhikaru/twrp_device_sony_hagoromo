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
#ifndef _BACKPORTS_LINUX_AF_ARP_H
#define _BACKPORTS_LINUX_AF_ARP_H 1

#include_next <linux/if_arp.h>

#ifndef ARPHRD_IEEE802154_MONITOR
#define ARPHRD_IEEE802154_MONITOR 805	/* IEEE 802.15.4 network monitor */
#endif

#ifndef ARPHRD_6LOWPAN
#define ARPHRD_6LOWPAN	825		/* IPv6 over LoWPAN             */
#endif

#endif /* _BACKPORTS_LINUX_AF_ARP_H */
