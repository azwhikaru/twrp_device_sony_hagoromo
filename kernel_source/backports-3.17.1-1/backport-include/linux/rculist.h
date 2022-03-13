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
#ifndef __BACKPORT_RCULIST_H
#define __BACKPORT_RCULIST_H
#include_next <linux/rculist.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0)
#include <backport/magic.h>
#define hlist_for_each_entry_rcu4(tpos, pos, head, member)		\
	for (pos = rcu_dereference_raw(hlist_first_rcu(head));		\
	     pos &&							\
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1; });\
	     pos = rcu_dereference_raw(hlist_next_rcu(pos)))

#define hlist_for_each_entry_rcu3(pos, head, member)				\
	for (pos = hlist_entry_safe (rcu_dereference_raw(hlist_first_rcu(head)),\
			typeof(*(pos)), member);				\
		pos;								\
		pos = hlist_entry_safe(rcu_dereference_raw(hlist_next_rcu(	\
			&(pos)->member)), typeof(*(pos)), member))

#undef hlist_for_each_entry_rcu
#define hlist_for_each_entry_rcu(...) \
	macro_dispatcher(hlist_for_each_entry_rcu, __VA_ARGS__)(__VA_ARGS__)
#endif /* < 3.9 */

#ifndef list_for_each_entry_continue_rcu
#define list_for_each_entry_continue_rcu(pos, head, member) 		\
	for (pos = list_entry_rcu(pos->member.next, typeof(*pos), member); \
	     prefetch(pos->member.next), &pos->member != (head);	\
	     pos = list_entry_rcu(pos->member.next, typeof(*pos), member))
#endif

#ifndef list_entry_rcu
#define list_entry_rcu(ptr, type, member) \
	container_of(rcu_dereference(ptr), type, member)
#endif

#endif /* __BACKPORT_RCULIST_H */
