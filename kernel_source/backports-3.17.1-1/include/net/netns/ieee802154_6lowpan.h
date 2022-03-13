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
/*
 * ieee802154 6lowpan in net namespaces
 */

#include <net/inet_frag.h>

#ifndef __NETNS_IEEE802154_6LOWPAN_H__
#define __NETNS_IEEE802154_6LOWPAN_H__

struct netns_sysctl_lowpan {
#ifdef CONFIG_SYSCTL
	struct ctl_table_header *frags_hdr;
#endif
};

struct netns_ieee802154_lowpan {
	struct netns_sysctl_lowpan sysctl;
	struct netns_frags	frags;
};

#endif
