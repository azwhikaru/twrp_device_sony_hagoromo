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
#ifndef __BACKPORT_LINUX_GENETLINK_H
#define __BACKPORT_LINUX_GENETLINK_H
#include_next <linux/genetlink.h>

/* This backports:
 *
 * commit e9412c37082b5c932e83364aaed0c38c2ce33acb
 * Author: Neil Horman <nhorman@tuxdriver.com>
 * Date:   Tue May 29 09:30:41 2012 +0000
 *
 *     genetlink: Build a generic netlink family module alias
 */
#ifndef MODULE_ALIAS_GENL_FAMILY
#define MODULE_ALIAS_GENL_FAMILY(family)\
 MODULE_ALIAS_NET_PF_PROTO_NAME(PF_NETLINK, NETLINK_GENERIC, "-family-" family)
#endif

#endif /* __BACKPORT_LINUX_GENETLINK_H */
