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
#ifndef __BACKPORT_LINUX_POLL_H
#define __BACKPORT_LINUX_POLL_H
#include_next <linux/poll.h>
#include <linux/version.h>

#if  LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
#define poll_does_not_wait LINUX_BACKPORT(poll_does_not_wait)
static inline bool poll_does_not_wait(const poll_table *p)
{
	return p == NULL || p->qproc == NULL;
}

#define poll_requested_events LINUX_BACKPORT(poll_requested_events)
static inline unsigned long poll_requested_events(const poll_table *p)
{
	return p ? p->key : ~0UL;
}
#endif /* < 3.4 */

#endif /* __BACKPORT_LINUX_POLL_H */
