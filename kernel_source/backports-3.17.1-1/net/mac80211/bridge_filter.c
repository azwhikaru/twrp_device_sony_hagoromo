/*
 * Copyright (C) 2013 Sony Corporation
 * Author: Yoshihiko.Ikenaga@jp.sony.com
 */

#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rcupdate.h>
#include <linux/export.h>
#include <linux/ieee80211.h>
#include <net/mac80211.h>
#include <net/ieee80211_radiotap.h>
#include <asm/unaligned.h>

#include "ieee80211_i.h"

#define BF_PACKET_PASS      0
#define BF_PACKET_DROP      (1<<0)

void mac80211_bridge_filter_set(struct sk_buff *skb, int fwd)
{
	if (fwd) {
		skb->bridge_filter = BF_PACKET_PASS;
	} else {
		skb->bridge_filter = BF_PACKET_DROP;
	}
}

int mac80211_bridge_filter_is_drop(const struct sk_buff *skb)
{
	return (skb->bridge_filter & BF_PACKET_DROP);
}

EXPORT_SYMBOL(mac80211_bridge_filter_is_drop);
