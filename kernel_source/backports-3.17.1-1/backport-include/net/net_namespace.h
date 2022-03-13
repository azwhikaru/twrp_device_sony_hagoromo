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
#ifndef _COMPAT_NET_NET_NAMESPACE_H
#define _COMPAT_NET_NET_NAMESPACE_H 1

#include_next <net/net_namespace.h>

#if IS_ENABLED(CPTCFG_IEEE802154_6LOWPAN)
#include <linux/version.h>
#include <net/netns/ieee802154_6lowpan.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
/*
 * we provide backport for 6lowpan as per the dependencies file
 * down to 3.5 only.
 */
extern struct netns_ieee802154_lowpan ieee802154_lowpan;
struct netns_ieee802154_lowpan *net_ieee802154_lowpan(struct net *net);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)
/* This can be removed once and if this gets upstream */
static inline struct netns_ieee802154_lowpan *
net_ieee802154_lowpan(struct net *net)
{
	return &net->ieee802154_lowpan;
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0) */
#endif /* CPTCFG_IEEE802154_6LOWPAN */

#endif	/* _COMPAT_NET_NET_NAMESPACE_H */
