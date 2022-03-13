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
#ifndef __NET_WOW_H__
#define __NET_WOW_H__

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/sched.h>

struct wow_skb_parm {
	char magic[4];
	char pad[44];
};

#define WOWCB(skb) ((struct wow_skb_parm *)((skb)->cb))

struct wow_sniffer {
	struct work_struct wow_work;
	struct sk_buff_head q;
};

struct wow_pkt_info {
	u8 pkt_type;
	u16 l3_proto;
	u32 saddr;
	u32 daddr;
	u16 frag_off;
	u8 l4_proto;
	u16 sport;
	u16 dport;
	char procname[TASK_COMM_LEN];
	unsigned int count;
	struct list_head link;
};

extern void tag_wow_skb(struct sk_buff *skb);

#endif /* __NET_WOW_H__ */
