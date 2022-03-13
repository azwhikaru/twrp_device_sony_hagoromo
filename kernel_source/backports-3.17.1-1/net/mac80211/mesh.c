/*
 * Copyright (c) 2008, 2009 open80211s Ltd.
 * Authors:    Luis Carlos Cobo <luisca@cozybit.com>
 * 	       Javier Cardona <javier@cozybit.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/slab.h>
#include <asm/unaligned.h>
#include "ieee80211_i.h"
#include "wme.h"
#include "mesh.h"
#include "driver-ops.h"

static int mesh_allocated;
static struct kmem_cache *rm_cache;

/* mesh_bss_mtx protects updates; internal iface list uses RCU */
static DEFINE_MUTEX(mesh_bss_mtx);
static LIST_HEAD(mesh_bss_list);


bool mesh_action_is_path_sel(struct ieee80211_mgmt *mgmt)
{
	return (mgmt->u.action.u.mesh_action.action_code ==
			WLAN_MESH_ACTION_HWMP_PATH_SELECTION);
}

void ieee80211s_init(void)
{
	mesh_pathtbl_init();
	mesh_allocated = 1;
	rm_cache = kmem_cache_create("mesh_rmc", sizeof(struct rmc_entry),
				     0, 0, NULL);
}

void ieee80211s_stop(void)
{
	if (!mesh_allocated)
		return;
	mesh_pathtbl_unregister();
	kmem_cache_destroy(rm_cache);
}

static void ieee80211_mesh_housekeeping_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata = (void *) data;
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	set_bit(MESH_WORK_HOUSEKEEPING, &ifmsh->wrkq_flags);

	ieee80211_queue_work(&local->hw, &sdata->work);
}

/* iterate over ifaces in mbss to find the highest tx headroom */
static void mesh_bss_set_max_headroom(struct mesh_local_bss *mbss)
{
	struct ieee80211_sub_if_data *sdata;
	int max_headroom = 0;

	rcu_read_lock();
	list_for_each_entry_rcu(sdata, &mbss->if_list, u.mesh.if_list)
		if (sdata->local->tx_headroom > max_headroom)
			max_headroom = sdata->local->tx_headroom;

	/* elevate all netdevs to max headroom */
	/*  from "net/mac80211/iface.c" int ieee80211_if_add()
		ndev->needed_headroom = local->tx_headroom
					 + 4*6		   four MAC addresses
					 + 2 + 2 + 2 + 2   ctl, dur, seq, qos
					 + 6		   mesh
					 + 8		   rfc1042/bridge tunnel
					 - ETH_HLEN     ethernet hard_header_len
					 + IEEE80211_ENCRYPT_HEADROOM;
				       = local->tx_headroom + 40
	*/
	list_for_each_entry_rcu(sdata, &mbss->if_list, u.mesh.if_list) {
		sdata->dev->needed_headroom = max_headroom + 40;
		sdata->local->tx_headroom =max_headroom;
	}
	rcu_read_unlock();

	mbss->max_headroom = max_headroom;
}

static inline bool
mesh_bss_matches(struct ieee80211_sub_if_data *sdata,
		 struct mesh_setup *setup,
		 struct mesh_local_bss *mbss)
{
	return setup->shared &&
	       mbss->mesh_id_len == setup->mesh_id_len &&
	       memcmp(mbss->mesh_id, setup->mesh_id, mbss->mesh_id_len) == 0 &&
	       mbss->path_sel_proto == setup->path_sel_proto &&
	       mbss->path_metric == setup->path_metric &&
	       mbss->sync_method == setup->sync_method &&
	       mbss->is_secure == setup->is_secure &&
	       net_eq(wiphy_net(sdata->wdev.wiphy), mbss->net);
}

static struct mesh_local_bss * __must_check
mesh_bss_find(struct ieee80211_sub_if_data *sdata, struct mesh_setup *setup)
{
	struct mesh_local_bss *mbss;

	if (WARN_ON(!setup->mesh_id_len))
		return NULL;

	lockdep_assert_held(&mesh_bss_mtx);

	list_for_each_entry(mbss, &mesh_bss_list, list)
		if (mesh_bss_matches(sdata, setup, mbss))
			return mbss;

	return NULL;
}

static struct mesh_local_bss * __must_check
mesh_bss_create(struct ieee80211_sub_if_data *sdata, struct mesh_setup *setup)
{
	struct mesh_local_bss *mbss;

	if (WARN_ON(setup->mesh_id_len > IEEE80211_MAX_SSID_LEN))
		return NULL;

	mbss = kzalloc(sizeof(*mbss), GFP_KERNEL);
	if (!mbss)
		return NULL;

	if (mesh_rmc_init(mbss)) {
		kfree(mbss);
		return NULL;
	}

	INIT_LIST_HEAD(&mbss->list);
	INIT_LIST_HEAD(&mbss->if_list);

	mbss->mesh_id_len = setup->mesh_id_len;
	memcpy(mbss->mesh_id, setup->mesh_id, setup->mesh_id_len);
	mbss->path_metric = setup->path_metric;
	mbss->path_sel_proto = setup->path_sel_proto;
	mbss->sync_method = setup->sync_method;
	mbss->is_secure = setup->is_secure;
	mbss->net = wiphy_net(sdata->wdev.wiphy);
	return mbss;
}

static void mesh_bss_remove(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct mesh_local_bss *mbss = ifmsh->mesh_bss;

	if (!mbss)
		return;

	mutex_lock(&mesh_bss_mtx);
	list_del_rcu(&ifmsh->if_list);
	mesh_bss_set_max_headroom(mbss);
	synchronize_rcu();
	ifmsh->mesh_bss = NULL;

	/* free when no more devs have this mbss */
	if (list_empty(&mbss->if_list)) {
		list_del(&mbss->list);
		mesh_rmc_free(mbss);
		kfree(mbss);
	}
	mutex_unlock(&mesh_bss_mtx);
}

static
int mesh_bss_add(struct ieee80211_sub_if_data *sdata,
		 struct mesh_setup *setup)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct mesh_local_bss *mbss;
	int ret;

	if (WARN_ON(!setup->mesh_id_len))
		return -EINVAL;

	mutex_lock(&mesh_bss_mtx);
	mbss = mesh_bss_find(sdata, setup);
	if (!mbss || !setup->shared) {
		mbss = mesh_bss_create(sdata, setup);
		if (!mbss) {
			ret = -ENOMEM;
			goto out_fail;
		}
		if (!setup->shared)
			goto out_add;
		list_add(&mbss->list, &mesh_bss_list);
	}

out_add:
	ifmsh->mesh_bss = mbss;
	list_add_rcu(&ifmsh->if_list, &mbss->if_list);
	mesh_bss_set_max_headroom(mbss);
	ret = 0;

out_fail:
	mutex_unlock(&mesh_bss_mtx);
	return ret;
}

/**
 * mesh_bss_find_if - return the interface in the local mbss matching addr
 *
 * @mbss: mesh bss on this host
 * @addr: address to find in the interface list
 *
 * Returns an interface in mbss matching addr, or NULL if none found.
 */
struct ieee80211_sub_if_data *
mesh_bss_find_if(struct mesh_local_bss *mbss, const u8 *addr)
{
	struct ieee80211_sub_if_data *sdata;

	rcu_read_lock();
	list_for_each_entry_rcu(sdata, &mbss->if_list, u.mesh.if_list) {
		if (ether_addr_equal(addr, sdata->vif.addr)) {
			rcu_read_unlock();
			return sdata;
		}
	}
	rcu_read_unlock();
	return NULL;
}

/**
 * mesh_bss_matches_addr - check if the specified addr is in the local mbss
 *
 * @mbss: mesh bss on this host
 * @addr: address to find in the interface list
 *
 * Returns true if the addr is used by an interface in the mbss.
 */
bool mesh_bss_matches_addr(struct mesh_local_bss *mbss, const u8 *addr)
{
	return mesh_bss_find_if(mbss, addr) != NULL;
}

/**
 * mesh_bss_forward_tx - send a frame on all other interfaces in a bss
 *
 * @sdata: interface to exclude from tx
 * @skb: frame to send
 *
 * Forwards group-directed frames from sdata for tx on all other interfaces
 * which are participating in an mbss.
 */
void mesh_bss_forward_tx(struct ieee80211_sub_if_data *sdata,
			 struct sk_buff *skb)
{
	struct mesh_local_bss *mbss = mbss(sdata);
	struct ieee80211_hdr *fwd_hdr;
	struct sk_buff *fwd_skb;
	struct ieee80211_tx_info *info;
	struct ieee80211_sub_if_data *tmp_sdata;

	rcu_read_lock();
	list_for_each_entry_rcu(tmp_sdata, &mbss->if_list, u.mesh.if_list) {

		if (sdata == tmp_sdata)
			continue;

		fwd_skb = skb_copy(skb, GFP_ATOMIC);
		if (!fwd_skb)
			goto out;

		fwd_hdr =  (struct ieee80211_hdr *) fwd_skb->data;
		info = IEEE80211_SKB_CB(fwd_skb);
		memset(info, 0, sizeof(*info));
		info->flags |= IEEE80211_TX_INTFL_NEED_TXPROCESSING;
		info->control.vif = &tmp_sdata->vif;

		memcpy(fwd_hdr->addr2, tmp_sdata->vif.addr, ETH_ALEN);
		ieee80211_set_qos_hdr(tmp_sdata, fwd_skb);
		ieee80211_add_pending_skb(tmp_sdata->local, fwd_skb);
	}
out:
	rcu_read_unlock();
}

/**
 * mesh_matches_local - check if the config of a mesh point matches ours
 *
 * @sdata: local mesh subif
 * @ie: information elements of a management frame from the mesh peer
 *
 * This function checks if the mesh configuration of a mesh point matches the
 * local mesh configuration, i.e. if both nodes belong to the same mesh network.
 */
bool mesh_matches_local(struct ieee80211_sub_if_data *sdata,
			struct ieee802_11_elems *ie)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u32 basic_rates = 0;
	struct cfg80211_chan_def sta_chan_def;

	/*
	 * As support for each feature is added, check for matching
	 * - On mesh config capabilities
	 *   - Power Save Support En
	 *   - Sync support enabled
	 *   - Sync support active
	 *   - Sync support required from peer
	 *   - MDA enabled
	 * - Power management control on fc
	 */
	if (!(ifmsh->mesh_id_len == ie->mesh_id_len &&
	     memcmp(ifmsh->mesh_id, ie->mesh_id, ie->mesh_id_len) == 0 &&
	     (ifmsh->mesh_pp_id == ie->mesh_config->meshconf_psel) &&
	     (ifmsh->mesh_pm_id == ie->mesh_config->meshconf_pmetric) &&
	     (ifmsh->mesh_cc_id == ie->mesh_config->meshconf_congest) &&
	     (ifmsh->mesh_sp_id == ie->mesh_config->meshconf_synch) &&
	     (ifmsh->mesh_auth_id == ie->mesh_config->meshconf_auth)))
		return false;

	ieee80211_sta_get_rates(sdata, ie, ieee80211_get_sdata_band(sdata),
				&basic_rates);

	if (sdata->vif.bss_conf.basic_rates != basic_rates)
		return false;

	ieee80211_ht_oper_to_chandef(sdata->vif.bss_conf.chandef.chan,
				     ie->ht_operation, &sta_chan_def);

	if (!cfg80211_chandef_compatible(&sdata->vif.bss_conf.chandef,
					 &sta_chan_def))
		return false;

	return true;
}

/**
 * mesh_peer_accepts_plinks - check if an mp is willing to establish peer links
 *
 * @ie: information elements of a management frame from the mesh peer
 */
bool mesh_peer_accepts_plinks(struct ieee802_11_elems *ie)
{
	return (ie->mesh_config->meshconf_cap &
			IEEE80211_MESHCONF_CAPAB_ACCEPT_PLINKS) != 0;
}

/**
 * mesh_accept_plinks_update - update accepting_plink in local mesh beacons
 *
 * @sdata: mesh interface in which mesh beacons are going to be updated
 *
 * Returns: beacon changed flag if the beacon content changed.
 */
u32 mesh_accept_plinks_update(struct ieee80211_sub_if_data *sdata)
{
	bool free_plinks;
	u32 changed = 0;

	/* In case mesh_plink_free_count > 0 and mesh_plinktbl_capacity == 0,
	 * the mesh interface might be able to establish plinks with peers that
	 * are already on the table but are not on PLINK_ESTAB state. However,
	 * in general the mesh interface is not accepting peer link requests
	 * from new peers, and that must be reflected in the beacon
	 */
	free_plinks = mesh_plink_availables(sdata);

	if (free_plinks != sdata->u.mesh.accepting_plinks) {
		sdata->u.mesh.accepting_plinks = free_plinks;
		changed = BSS_CHANGED_BEACON;
	}

	return changed;
}

/*
 * mesh_sta_cleanup - clean up any mesh sta state
 *
 * @sta: mesh sta to clean up.
 */
void mesh_sta_cleanup(struct sta_info *sta)
{
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	u32 changed;

	/*
	 * maybe userspace handles peer allocation and peering, but in either
	 * case the beacon is still generated by the kernel and we might need
	 * an update.
	 */
	changed = mesh_accept_plinks_update(sdata);
	if (!sdata->u.mesh.user_mpm) {
		changed |= mesh_plink_deactivate(sta);
		del_timer_sync(&sta->plink_timer);
	}

	if (changed)
		ieee80211_mbss_info_change_notify(sdata, changed);
}

int mesh_rmc_init(struct mesh_local_bss *mbss)
{
	struct mesh_rmc *rmc;
	int i;

	rmc = kmalloc(sizeof(struct mesh_rmc), GFP_KERNEL);
	if (!rmc)
		return -ENOMEM;
	rmc->idx_mask = RMC_BUCKETS - 1;
	for (i = 0; i < RMC_BUCKETS; i++) {
		INIT_LIST_HEAD(&rmc->bucket[i]);
		rwlock_init(&rmc->bucket_lock[i]);
	}

	mbss->rmc = rmc;
	return 0;
}

void mesh_rmc_free(struct mesh_local_bss *mbss)
{
	struct mesh_rmc *rmc = mbss->rmc;
	struct rmc_entry *p, *n;
	int i;

	if (!rmc)
		return;

	for (i = 0; i < RMC_BUCKETS; i++) {
		list_for_each_entry_safe(p, n, &rmc->bucket[i], list) {
			list_del(&p->list);
			kmem_cache_free(rm_cache, p);
		}
	}

	kfree(rmc);
	mbss->rmc = NULL;
}

/**
 * mesh_rmc_check - Check frame in recent multicast cache and add if absent.
 *
 * @sa:		source address
 * @mesh_hdr:	mesh_header
 *
 * Returns: 0 if the frame is not in the cache, nonzero otherwise.
 *
 * Checks using the source address and the mesh sequence number if we have
 * received this frame lately. If the frame is not in the cache, it is added to
 * it.
 */
int mesh_rmc_check(struct mesh_local_bss *mbss,
		   const u8 *sa, struct ieee80211s_hdr *mesh_hdr)
{
	struct mesh_rmc *rmc = mbss->rmc;
	u32 seqnum = 0;
	int entries = 0, ret = 0;
	u8 idx;
	struct rmc_entry *p, *n;

	/* Don't care about endianness since only match matters */
	memcpy(&seqnum, &mesh_hdr->seqnum, sizeof(mesh_hdr->seqnum));
	idx = le32_to_cpu(mesh_hdr->seqnum) & rmc->idx_mask;

	read_lock(&rmc->bucket_lock[idx]);
	list_for_each_entry_safe(p, n, &rmc->bucket[idx], list) {
		++entries;
		if (time_after(jiffies, p->exp_time) ||
		    entries == RMC_QUEUE_MAX_LEN) {
			list_del(&p->list);
			kmem_cache_free(rm_cache, p);
			--entries;
		} else if ((seqnum == p->seqnum) &&
			    ether_addr_equal(sa, p->sa)) {
			ret = -1;
			break;
		}
	}
	read_unlock(&rmc->bucket_lock[idx]);

	if (ret)
		return ret;

	p = kmem_cache_alloc(rm_cache, GFP_ATOMIC);
	if (!p)
		return 0;

	write_lock(&rmc->bucket_lock[idx]);
	p->seqnum = seqnum;
	p->exp_time = jiffies + RMC_TIMEOUT;
	memcpy(p->sa, sa, ETH_ALEN);
	list_add(&p->list, &rmc->bucket[idx]);
	write_unlock(&rmc->bucket_lock[idx]);
	return ret;
}

int mesh_add_meshconf_ie(struct ieee80211_sub_if_data *sdata,
			 struct sk_buff *skb)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u8 *pos, neighbors;
	u8 meshconf_len = sizeof(struct ieee80211_meshconf_ie);

	if (skb_tailroom(skb) < 2 + meshconf_len)
		return -ENOMEM;

	pos = skb_put(skb, 2 + meshconf_len);
	*pos++ = WLAN_EID_MESH_CONFIG;
	*pos++ = meshconf_len;

	/* save a pointer for quick updates in pre-tbtt */
	ifmsh->meshconf_offset = pos - skb->data;

	/* Active path selection protocol ID */
	*pos++ = ifmsh->mesh_pp_id;
	/* Active path selection metric ID   */
	*pos++ = ifmsh->mesh_pm_id;
	/* Congestion control mode identifier */
	*pos++ = ifmsh->mesh_cc_id;
	/* Synchronization protocol identifier */
	*pos++ = ifmsh->mesh_sp_id;
	/* Authentication Protocol identifier */
	*pos++ = ifmsh->mesh_auth_id;
	/* Mesh Formation Info - number of neighbors */
	neighbors = atomic_read(&ifmsh->estab_plinks);
	neighbors = min_t(int, neighbors, IEEE80211_MAX_MESH_PEERINGS);
	*pos++ = neighbors << 1;
	/* Mesh capability */
	*pos = 0x00;
	*pos |= ifmsh->mshcfg.dot11MeshForwarding ?
			IEEE80211_MESHCONF_CAPAB_FORWARDING : 0x00;
	*pos |= ifmsh->accepting_plinks ?
			IEEE80211_MESHCONF_CAPAB_ACCEPT_PLINKS : 0x00;
	/* Mesh PS mode. See IEEE802.11-2012 8.4.2.100.8 */
	*pos |= ifmsh->ps_peers_deep_sleep ?
			IEEE80211_MESHCONF_CAPAB_POWER_SAVE_LEVEL : 0x00;
	/* MBCA */
	*pos |= (ifmsh->mbca_executor != NL80211_MBCA_EXECUTOR_INACTIVATED) ?
			IEEE80211_MESHCONF_CAPAB_MBCA_ENABLED : 0x00;
	*pos++ |= ifmsh->adjusting_tbtt ?
			IEEE80211_MESHCONF_CAPAB_TBTT_ADJUSTING : 0x00;
	return 0;
}

int mesh_add_meshid_ie(struct ieee80211_sub_if_data *sdata, struct sk_buff *skb)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u8 *pos;

	if (skb_tailroom(skb) < 2 + ifmsh->mesh_id_len)
		return -ENOMEM;

	pos = skb_put(skb, 2 + ifmsh->mesh_id_len);
	*pos++ = WLAN_EID_MESH_ID;
	*pos++ = ifmsh->mesh_id_len;
	if (ifmsh->mesh_id_len)
		memcpy(pos, ifmsh->mesh_id, ifmsh->mesh_id_len);

	return 0;
}

static int mesh_add_awake_window_ie(struct ieee80211_sub_if_data *sdata,
				    struct sk_buff *skb)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u8 *pos;

	/* see IEEE802.11-2012 13.14.6 */
	if (ifmsh->ps_peers_light_sleep == 0 &&
	    ifmsh->ps_peers_deep_sleep == 0 &&
	    ifmsh->nonpeer_pm == NL80211_MESH_POWER_ACTIVE)
		return 0;

	if (skb_tailroom(skb) < 4)
		return -ENOMEM;

	pos = skb_put(skb, 2 + 2);
	*pos++ = WLAN_EID_MESH_AWAKE_WINDOW;
	*pos++ = 2;
	put_unaligned_le16(ifmsh->mshcfg.dot11MeshAwakeWindowDuration, pos);

	return 0;
}

int mesh_add_vendor_ies(struct ieee80211_sub_if_data *sdata,
			struct sk_buff *skb)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u8 offset, len;
	const u8 *data;

	if (!ifmsh->ie || !ifmsh->ie_len)
		return 0;

	/* fast-forward to vendor IEs */
	offset = ieee80211_ie_split_vendor(ifmsh->ie, ifmsh->ie_len, 0);

	if (offset) {
		len = ifmsh->ie_len - offset;
		data = ifmsh->ie + offset;
		if (skb_tailroom(skb) < len)
			return -ENOMEM;
		memcpy(skb_put(skb, len), data, len);
	}

	return 0;
}

int mesh_add_rsn_ie(struct ieee80211_sub_if_data *sdata, struct sk_buff *skb)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u8 len = 0;
	const u8 *data;

	if (!ifmsh->ie || !ifmsh->ie_len)
		return 0;

	/* find RSN IE */
	data = cfg80211_find_ie(WLAN_EID_RSN, ifmsh->ie, ifmsh->ie_len);
	if (!data)
		return 0;

	len = data[1] + 2;

	if (skb_tailroom(skb) < len)
		return -ENOMEM;
	memcpy(skb_put(skb, len), data, len);

	return 0;
}

static int mesh_add_ds_params_ie(struct ieee80211_sub_if_data *sdata,
				 struct sk_buff *skb)
{
	struct ieee80211_chanctx_conf *chanctx_conf;
	struct ieee80211_channel *chan;
	u8 *pos;

	if (skb_tailroom(skb) < 3)
		return -ENOMEM;

	rcu_read_lock();
	chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
	if (WARN_ON(!chanctx_conf)) {
		rcu_read_unlock();
		return -EINVAL;
	}
	chan = chanctx_conf->def.chan;
	rcu_read_unlock();

	if (ieee80211_get_sdata_band(sdata) != IEEE80211_BAND_2GHZ)
		return 0;

	pos = skb_put(skb, 2 + 1);
	*pos++ = WLAN_EID_DS_PARAMS;
	*pos++ = 1;
	*pos++ = ieee80211_frequency_to_channel(chan->center_freq);

	return 0;
}

int mesh_add_ht_cap_ie(struct ieee80211_sub_if_data *sdata,
		       struct sk_buff *skb)
{
	struct ieee80211_local *local = sdata->local;
	enum ieee80211_band band = ieee80211_get_sdata_band(sdata);
	struct ieee80211_supported_band *sband;
	u8 *pos;

	sband = local->hw.wiphy->bands[band];
	if (!sband->ht_cap.ht_supported ||
	    sdata->vif.bss_conf.chandef.width == NL80211_CHAN_WIDTH_20_NOHT ||
	    sdata->vif.bss_conf.chandef.width == NL80211_CHAN_WIDTH_5 ||
	    sdata->vif.bss_conf.chandef.width == NL80211_CHAN_WIDTH_10)
		return 0;

	if (skb_tailroom(skb) < 2 + sizeof(struct ieee80211_ht_cap))
		return -ENOMEM;

	pos = skb_put(skb, 2 + sizeof(struct ieee80211_ht_cap));
	ieee80211_ie_build_ht_cap(pos, &sband->ht_cap, sband->ht_cap.cap);

	return 0;
}

int mesh_add_ht_oper_ie(struct ieee80211_sub_if_data *sdata,
			struct sk_buff *skb)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_chanctx_conf *chanctx_conf;
	struct ieee80211_channel *channel;
	enum nl80211_channel_type channel_type =
		cfg80211_get_chandef_type(&sdata->vif.bss_conf.chandef);
	struct ieee80211_supported_band *sband;
	struct ieee80211_sta_ht_cap *ht_cap;
	u8 *pos;

	rcu_read_lock();
	chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
	if (WARN_ON(!chanctx_conf)) {
		rcu_read_unlock();
		return -EINVAL;
	}
	channel = chanctx_conf->def.chan;
	rcu_read_unlock();

	sband = local->hw.wiphy->bands[channel->band];
	ht_cap = &sband->ht_cap;

	if (!ht_cap->ht_supported || channel_type == NL80211_CHAN_NO_HT)
		return 0;

	if (skb_tailroom(skb) < 2 + sizeof(struct ieee80211_ht_operation))
		return -ENOMEM;

	pos = skb_put(skb, 2 + sizeof(struct ieee80211_ht_operation));
	ieee80211_ie_build_ht_oper(pos, ht_cap, &sdata->vif.bss_conf.chandef,
				   sdata->vif.bss_conf.ht_operation_mode);

	return 0;
}

int mesh_add_beacon_timing_ie(struct ieee80211_sub_if_data *sdata,
		struct sk_buff *skb, struct beacon_timing_ie *bcn_timing_ie)
{
	u8 *pos;

	if (bcn_timing_ie == NULL || bcn_timing_ie->ie_len == 0)
		return 0;

	if (skb_tailroom(skb) < bcn_timing_ie->ie_len)
		return -ENOMEM;

	pos = skb_put(skb, bcn_timing_ie->ie_len);
	memcpy(pos, bcn_timing_ie->ie_char, bcn_timing_ie->ie_len);
	return 0;
}


static void ieee80211_mesh_path_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata =
		(struct ieee80211_sub_if_data *) data;

	ieee80211_queue_work(&sdata->local->hw, &sdata->work);
}

static void ieee80211_mesh_path_root_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata =
		(struct ieee80211_sub_if_data *) data;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	set_bit(MESH_WORK_ROOT, &ifmsh->wrkq_flags);

	ieee80211_queue_work(&sdata->local->hw, &sdata->work);
}

void ieee80211_mesh_root_setup(struct ieee80211_if_mesh *ifmsh)
{
	if (ifmsh->mshcfg.dot11MeshHWMPRootMode > IEEE80211_ROOTMODE_ROOT)
		set_bit(MESH_WORK_ROOT, &ifmsh->wrkq_flags);
	else {
		clear_bit(MESH_WORK_ROOT, &ifmsh->wrkq_flags);
		/* stop running timer */
		del_timer_sync(&ifmsh->mesh_path_root_timer);
	}
}

/**
 * ieee80211_fill_mesh_addresses - fill addresses of a locally originated mesh frame
 * @hdr:	802.11 frame header
 * @fc:		frame control field
 * @meshda:	destination address in the mesh
 * @meshsa:	source address address in the mesh.  Same as TA, as frame is
 *              locally originated.
 * @unicast:    force using unicast 4 addr format
 *
 * Return the length of the 802.11 (does not include a mesh control header)
 */
int ieee80211_fill_mesh_addresses(struct ieee80211_hdr *hdr, __le16 *fc,
				  const u8 *meshda, const u8 *meshsa, bool unicast)
{
	if (is_multicast_ether_addr(meshda) && !unicast) {
		*fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS);
		/* DA TA SA */
		memcpy(hdr->addr1, meshda, ETH_ALEN);
		memcpy(hdr->addr2, meshsa, ETH_ALEN);
		memcpy(hdr->addr3, meshsa, ETH_ALEN);
		return 24;
	} else {
		*fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS | IEEE80211_FCTL_TODS);
		/* RA TA DA SA */
		memset(hdr->addr1, 0, ETH_ALEN);   /* RA is resolved later */
		memcpy(hdr->addr2, meshsa, ETH_ALEN);
		memcpy(hdr->addr3, meshda, ETH_ALEN);
		memcpy(hdr->addr4, meshsa, ETH_ALEN);
		return 30;
	}
}

/**
 * ieee80211_new_mesh_header - create a new mesh header
 * @sdata:	mesh interface to be used
 * @meshhdr:    uninitialized mesh header
 * @addr4or5:   1st address in the ae header, which may correspond to address 4
 *              (if addr6 is NULL) or address 5 (if addr6 is present). It may
 *              be NULL.
 * @addr6:	2nd address in the ae header, which corresponds to addr6 of the
 *              mesh frame
 *
 * Return the header length.
 */
int ieee80211_new_mesh_header(struct ieee80211_sub_if_data *sdata,
			      struct ieee80211s_hdr *meshhdr,
			      const char *addr4or5, const char *addr6)
{
	if (WARN_ON(!addr4or5 && addr6))
		return 0;

	memset(meshhdr, 0, sizeof(*meshhdr));

	meshhdr->ttl = sdata->u.mesh.mshcfg.dot11MeshTTL;

	/* FIXME: racy -- TX on multiple queues can be concurrent */
	put_unaligned(cpu_to_le32(sdata->u.mesh.mesh_seqnum), &meshhdr->seqnum);
	sdata->u.mesh.mesh_seqnum++;

	if (addr4or5 && !addr6) {
		meshhdr->flags |= MESH_FLAGS_AE_A4;
		memcpy(meshhdr->eaddr1, addr4or5, ETH_ALEN);
		return 2 * ETH_ALEN;
	} else if (addr4or5 && addr6) {
		meshhdr->flags |= MESH_FLAGS_AE_A5_A6;
		memcpy(meshhdr->eaddr1, addr4or5, ETH_ALEN);
		memcpy(meshhdr->eaddr2, addr6, ETH_ALEN);
		return 3 * ETH_ALEN;
	}

	return ETH_ALEN;
}

static void ieee80211_mesh_housekeeping(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u32 changed;

	if (ifmsh->mshcfg.plink_timeout > 0)
		ieee80211_sta_expire(sdata, ifmsh->mshcfg.plink_timeout * HZ);
	mesh_path_expire(sdata);

	changed = mesh_accept_plinks_update(sdata);
	ieee80211_mbss_info_change_notify(sdata, changed);

	mod_timer(&ifmsh->housekeeping_timer,
		  round_jiffies(jiffies +
				IEEE80211_MESH_HOUSEKEEPING_INTERVAL));
}

static void ieee80211_mesh_rootpath(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u32 interval;

	mesh_path_tx_root_frame(sdata);

	if (ifmsh->mshcfg.dot11MeshHWMPRootMode == IEEE80211_PROACTIVE_RANN)
		interval = ifmsh->mshcfg.dot11MeshHWMPRannInterval;
	else
		interval = ifmsh->mshcfg.dot11MeshHWMProotInterval;

	mod_timer(&ifmsh->mesh_path_root_timer,
		  round_jiffies(TU_TO_EXP_TIME(interval)));
}

static int
ieee80211_mesh_build_beacon(struct ieee80211_if_mesh *ifmsh)
{
	struct beacon_data *bcn;
	int head_len, tail_len;
	struct sk_buff *skb;
	struct ieee80211_mgmt *mgmt;
	struct ieee80211_chanctx_conf *chanctx_conf;
	struct mesh_csa_settings *csa;
	enum ieee80211_band band;
	u8 *pos;
	struct ieee80211_sub_if_data *sdata;
	int hdr_len = offsetof(struct ieee80211_mgmt, u.beacon) +
		      sizeof(mgmt->u.beacon);
	struct beacon_timing_ie *bcn_timing_ie = ifmsh->bcn_timing_ie;


	sdata = container_of(ifmsh, struct ieee80211_sub_if_data, u.mesh);
	rcu_read_lock();
	chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
	band = chanctx_conf->def.chan->band;
	rcu_read_unlock();

	head_len = hdr_len +
		   2 + /* NULL SSID */
		   /* Channel Switch Announcement */
		   2 + sizeof(struct ieee80211_channel_sw_ie) +
		   /* Mesh Channel Swith Parameters */
		   2 + sizeof(struct ieee80211_mesh_chansw_params_ie) +
		   2 + 8 + /* supported rates */
		   ((ieee80211_get_sdata_band(sdata) == IEEE80211_BAND_2GHZ) ? (2 + 3) : 0);/* DS params */
	tail_len = 2 + (IEEE80211_MAX_SUPP_RATES - 8) +
		   2 + sizeof(struct ieee80211_ht_cap) +
		   2 + sizeof(struct ieee80211_ht_operation) +
		   2 + ifmsh->mesh_id_len +
		   2 + sizeof(struct ieee80211_meshconf_ie) +
		   2 + sizeof(__le16) + /* awake window */
		   ((bcn_timing_ie != NULL) ? (2 + bcn_timing_ie->ie_len) : 0) +  /* beacon timing */
		   ifmsh->ie_len;

	bcn = kzalloc(sizeof(*bcn) + head_len + tail_len, GFP_KERNEL);
	/* need an skb for IE builders to operate on */
	skb = dev_alloc_skb(max(head_len, tail_len));

	if (!bcn || !skb)
		goto out_free;

	/*
	 * pointers go into the block we allocated,
	 * memory is | beacon_data | head | tail |
	 */
	bcn->head = ((u8 *) bcn) + sizeof(*bcn);

	/* fill in the head */
	mgmt = (struct ieee80211_mgmt *) skb_put(skb, hdr_len);
	memset(mgmt, 0, hdr_len);
	mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT |
					  IEEE80211_STYPE_BEACON);
	eth_broadcast_addr(mgmt->da);
	memcpy(mgmt->sa, sdata->vif.addr, ETH_ALEN);
	memcpy(mgmt->bssid, sdata->vif.addr, ETH_ALEN);
	ieee80211_mps_set_frame_flags(sdata, NULL, (void *) mgmt);
	mgmt->u.beacon.beacon_int =
		cpu_to_le16(sdata->vif.bss_conf.beacon_int);
	mgmt->u.beacon.capab_info |= cpu_to_le16(
		sdata->u.mesh.security ? WLAN_CAPABILITY_PRIVACY : 0);

	pos = skb_put(skb, 2);
	*pos++ = WLAN_EID_SSID;
	*pos++ = 0x0;

	rcu_read_lock();
	csa = rcu_dereference(ifmsh->csa);
	if (csa) {
		pos = skb_put(skb, 13);
		memset(pos, 0, 13);
		*pos++ = WLAN_EID_CHANNEL_SWITCH;
		*pos++ = 3;
		*pos++ = 0x0;
		*pos++ = ieee80211_frequency_to_channel(
				csa->settings.chandef.chan->center_freq);
		bcn->csa_counter_offsets[0] = hdr_len + 6;
		*pos++ = csa->settings.count;
		*pos++ = WLAN_EID_CHAN_SWITCH_PARAM;
		*pos++ = 6;
		if (ifmsh->csa_role == IEEE80211_MESH_CSA_ROLE_INIT) {
			*pos++ = ifmsh->mshcfg.dot11MeshTTL;
			*pos |= WLAN_EID_CHAN_SWITCH_PARAM_INITIATOR;
		} else {
			*pos++ = ifmsh->chsw_ttl;
		}
		*pos++ |= csa->settings.block_tx ?
			  WLAN_EID_CHAN_SWITCH_PARAM_TX_RESTRICT : 0x00;
		put_unaligned_le16(WLAN_REASON_MESH_CHAN, pos);
		pos += 2;
		put_unaligned_le16(ifmsh->pre_value, pos);
		pos += 2;
	}
	rcu_read_unlock();

	if (ieee80211_add_srates_ie(sdata, skb, true, band) ||
	    mesh_add_ds_params_ie(sdata, skb))
		goto out_free;

	bcn->head_len = skb->len;
	memcpy(bcn->head, skb->data, bcn->head_len);

	/* now the tail */
	skb_trim(skb, 0);
	bcn->tail = bcn->head + bcn->head_len;

	if (ieee80211_add_ext_srates_ie(sdata, skb, true, band) ||
	    mesh_add_rsn_ie(sdata, skb) ||
	    mesh_add_ht_cap_ie(sdata, skb) ||
	    mesh_add_ht_oper_ie(sdata, skb) ||
	    mesh_add_meshid_ie(sdata, skb) ||
	    mesh_add_meshconf_ie(sdata, skb) ||
	    mesh_add_awake_window_ie(sdata, skb) ||
	    mesh_add_beacon_timing_ie(sdata, skb, bcn_timing_ie) ||
	    mesh_add_vendor_ies(sdata, skb))
		goto out_free;

	bcn->tail_len = skb->len;
	memcpy(bcn->tail, skb->data, bcn->tail_len);
	bcn->meshconf = (struct ieee80211_meshconf_ie *)
					(bcn->tail + ifmsh->meshconf_offset);

	dev_kfree_skb(skb);
	rcu_assign_pointer(ifmsh->beacon, bcn);
	return 0;
out_free:
	kfree(bcn);
	dev_kfree_skb(skb);
	return -ENOMEM;
}

static int
ieee80211_mesh_rebuild_beacon(struct ieee80211_sub_if_data *sdata)
{
	struct beacon_data *old_bcn;
	int ret;

	old_bcn = rcu_dereference_protected(sdata->u.mesh.beacon,
					    lockdep_is_held(&sdata->wdev.mtx));
	ret = ieee80211_mesh_build_beacon(&sdata->u.mesh);
	if (ret)
		/* just reuse old beacon */
		return ret;

	if (old_bcn)
		kfree_rcu(old_bcn, rcu_head);
	return 0;
}

void ieee80211_mbss_info_change_notify(struct ieee80211_sub_if_data *sdata,
				       u32 changed)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	unsigned long bits = changed;
	u32 bit;

	if (!bits)
		return;

	/* if we race with running work, worst case this work becomes a noop */
	for_each_set_bit(bit, &bits, sizeof(changed) * BITS_PER_BYTE)
		set_bit(bit, &ifmsh->mbss_changed);
	set_bit(MESH_WORK_MBSS_CHANGED, &ifmsh->wrkq_flags);
	ieee80211_queue_work(&sdata->local->hw, &sdata->work);
}

int ieee80211_start_mesh(struct ieee80211_sub_if_data *sdata)
{
	struct mesh_setup setup = {};
	int ret;

	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct ieee80211_local *local = sdata->local;
	u32 changed = BSS_CHANGED_BEACON |
		      BSS_CHANGED_BEACON_ENABLED |
		      BSS_CHANGED_HT |
		      BSS_CHANGED_BASIC_RATES |
		      BSS_CHANGED_BEACON_INT;

	local->fif_other_bss++;
	/* mesh ifaces must set allmulti to forward mcast traffic */
	atomic_inc(&local->iff_allmultis);
	ieee80211_configure_filter(local);

	ifmsh->mesh_cc_id = 0;	/* Disabled */
	/* sony extension starts */
	/* register sync ops from extensible synchronization framework */
	if (local->hw.flags2 & IEEE80211_HW_CLOCK_SYNC)
		ifmsh->sync_executor = NL80211_MESH_SYNC_EXECUTOR_HW;
	ifmsh->sync_ops = ieee80211_mesh_sync_ops_get(ifmsh->mesh_sp_id,
					ifmsh->sync_executor);
	printk(KERN_INFO "ifmsh->sync_executor %d\n", ifmsh->sync_executor);
	/* sony extension ends */

	ifmsh->adjusting_tbtt = false;
	ifmsh->sync_offset_clockdrift_max = 0;
	set_bit(MESH_WORK_HOUSEKEEPING, &ifmsh->wrkq_flags);
	ieee80211_mesh_root_setup(ifmsh);
	ieee80211_queue_work(&local->hw, &sdata->work);
	sdata->vif.bss_conf.ht_operation_mode =
				ifmsh->mshcfg.ht_opmode;
	sdata->vif.bss_conf.enable_beacon = true;
	/* sony extension starts */
	sdata->vif.bss_conf.mesh_awake_window_duration =
			sdata->u.mesh.mshcfg.dot11MeshAwakeWindowDuration;
	/* sony extension ends */

	changed |= ieee80211_mps_local_status_update(sdata);

	if (ieee80211_mesh_build_beacon(ifmsh)) {
		ieee80211_stop_mesh(sdata);
		return -ENOMEM;
	}

	setup.mesh_id_len = ifmsh->mesh_id_len;
	setup.mesh_id = ifmsh->mesh_id;
	setup.path_sel_proto = ifmsh->mesh_pp_id;
	setup.sync_method = ifmsh->mesh_pm_id;
	setup.path_metric = ifmsh->mesh_cc_id;
	setup.is_secure = ifmsh->security & IEEE80211_MESH_SEC_SECURED;
	setup.shared = ifmsh->share_mbss;

	ret = mesh_bss_add(sdata, &setup);
	if (ret) {
		ieee80211_stop_mesh(sdata);
		return ret;
	}

	ieee80211_recalc_dtim(local, sdata);
	ieee80211_bss_info_change_notify(sdata, changed);

	netif_carrier_on(sdata->dev);
	return 0;
}

void ieee80211_stop_mesh(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct beacon_data *bcn;

	netif_carrier_off(sdata->dev);

	/* stop the beacon */
	ifmsh->mesh_id_len = 0;
	sdata->vif.bss_conf.enable_beacon = false;
	clear_bit(SDATA_STATE_OFFCHANNEL_BEACON_STOPPED, &sdata->state);
	ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_BEACON_ENABLED);
	bcn = rcu_dereference_protected(ifmsh->beacon,
					lockdep_is_held(&sdata->wdev.mtx));
	RCU_INIT_POINTER(ifmsh->beacon, NULL);
	kfree_rcu(bcn, rcu_head);

	/* flush STAs and mpaths on this iface */
	sta_info_flush(sdata);
	mesh_path_flush_by_iface(sdata);

	/* free all potentially still buffered group-addressed frames */
	local->total_ps_buffered -= skb_queue_len(&ifmsh->ps.bc_buf);
	skb_queue_purge(&ifmsh->ps.bc_buf);

	del_timer_sync(&sdata->u.mesh.housekeeping_timer);
	del_timer_sync(&sdata->u.mesh.mesh_path_root_timer);
	del_timer_sync(&sdata->u.mesh.mesh_path_timer);
	ieee80211_mps_teardown(sdata);

	mesh_mbca_teardown(ifmsh);

	/* clear any mesh work (for next join) we may have accrued */
	ifmsh->wrkq_flags = 0;
	ifmsh->mbss_changed = 0;

	local->fif_other_bss--;
	atomic_dec(&local->iff_allmultis);
	ieee80211_configure_filter(local);

	netif_tx_stop_all_queues(sdata->dev);

	mesh_bss_remove(sdata);
}

static bool
ieee80211_mesh_process_chnswitch(struct ieee80211_sub_if_data *sdata,
				 struct ieee802_11_elems *elems, bool beacon)
{
	struct cfg80211_csa_settings params;
	struct ieee80211_csa_ie csa_ie;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	enum ieee80211_band band = ieee80211_get_sdata_band(sdata);
	int err;
	u32 sta_flags;

	sdata_assert_lock(sdata);

	sta_flags = IEEE80211_STA_DISABLE_VHT;
	switch (sdata->vif.bss_conf.chandef.width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
		sta_flags |= IEEE80211_STA_DISABLE_HT;
	case NL80211_CHAN_WIDTH_20:
		sta_flags |= IEEE80211_STA_DISABLE_40MHZ;
		break;
	default:
		break;
	}

	memset(&params, 0, sizeof(params));
	memset(&csa_ie, 0, sizeof(csa_ie));
	err = ieee80211_parse_ch_switch_ie(sdata, elems, beacon, band,
					   sta_flags, sdata->vif.addr,
					   &csa_ie);
	if (err < 0)
		return false;
	if (err)
		return false;

	params.chandef = csa_ie.chandef;
	params.count = csa_ie.count;

	if (!cfg80211_chandef_usable(sdata->local->hw.wiphy, &params.chandef,
				     IEEE80211_CHAN_DISABLED)) {
		sdata_info(sdata,
			   "mesh STA %pM switches to unsupported channel (%d MHz, width:%d, CF1/2: %d/%d MHz), aborting\n",
			   sdata->vif.addr,
			   params.chandef.chan->center_freq,
			   params.chandef.width,
			   params.chandef.center_freq1,
			   params.chandef.center_freq2);
		return false;
	}

	err = cfg80211_chandef_dfs_required(sdata->local->hw.wiphy,
					    &params.chandef,
					    NL80211_IFTYPE_MESH_POINT);
	if (err < 0)
		return false;
	if (err > 0)
		/* TODO: DFS not (yet) supported */
		return false;

	params.radar_required = err;

	if (cfg80211_chandef_identical(&params.chandef,
				       &sdata->vif.bss_conf.chandef)) {
		mcsa_dbg(sdata,
			 "received csa with an identical chandef, ignoring\n");
		return true;
	}

	mcsa_dbg(sdata,
		 "received channel switch announcement to go to channel %d MHz\n",
		 params.chandef.chan->center_freq);

	params.block_tx = csa_ie.mode & WLAN_EID_CHAN_SWITCH_PARAM_TX_RESTRICT;
	if (beacon) {
		ifmsh->chsw_ttl = csa_ie.ttl - 1;
		if (ifmsh->pre_value >= csa_ie.pre_value)
			return false;
		ifmsh->pre_value = csa_ie.pre_value;
	}

	if (ifmsh->chsw_ttl >= ifmsh->mshcfg.dot11MeshTTL)
		return false;

	ifmsh->csa_role = IEEE80211_MESH_CSA_ROLE_REPEATER;

	if (ieee80211_channel_switch(sdata->local->hw.wiphy, sdata->dev,
				     &params) < 0)
		return false;

	return true;
}

static void
ieee80211_mesh_rx_probe_req(struct ieee80211_sub_if_data *sdata,
			    struct ieee80211_mgmt *mgmt, size_t len)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct sk_buff *presp;
	struct beacon_data *bcn;
	struct ieee80211_mgmt *hdr;
	struct ieee802_11_elems elems;
	size_t baselen;
	u8 *pos;

	pos = mgmt->u.probe_req.variable;
	baselen = (u8 *) pos - (u8 *) mgmt;
	if (baselen > len)
		return;

	ieee802_11_parse_elems(pos, len - baselen, false, &elems);

	if (!elems.mesh_id)
		return;

	/* 802.11-2012 10.1.4.3.2 */
	if ((!ether_addr_equal(mgmt->da, sdata->vif.addr) &&
	     !is_broadcast_ether_addr(mgmt->da)) ||
	    elems.ssid_len != 0)
		return;

	/* do not respond if meshid does not present int the probe request */
	if (elems.mesh_id_len == 0 && elems.mesh_id == NULL)
		return;

	if (elems.mesh_id_len != 0 &&
	    (elems.mesh_id_len != ifmsh->mesh_id_len ||
	     memcmp(elems.mesh_id, ifmsh->mesh_id, ifmsh->mesh_id_len)))
		return;

	rcu_read_lock();
	bcn = rcu_dereference(ifmsh->beacon);

	if (!bcn)
		goto out;

	presp = dev_alloc_skb(local->tx_headroom +
			      bcn->head_len + bcn->tail_len);
	if (!presp)
		goto out;

	skb_reserve(presp, local->tx_headroom);
	memcpy(skb_put(presp, bcn->head_len), bcn->head, bcn->head_len);
	memcpy(skb_put(presp, bcn->tail_len), bcn->tail, bcn->tail_len);
	hdr = (struct ieee80211_mgmt *) presp->data;
	hdr->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT |
					 IEEE80211_STYPE_PROBE_RESP);
	memcpy(hdr->da, mgmt->sa, ETH_ALEN);
	IEEE80211_SKB_CB(presp)->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;
	ieee80211_tx_skb(sdata, presp);
out:
	rcu_read_unlock();
}

static void ieee80211_mesh_rx_bcn_presp(struct ieee80211_sub_if_data *sdata,
					u16 stype,
					struct ieee80211_mgmt *mgmt,
					size_t len,
					struct ieee80211_rx_status *rx_status)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct ieee802_11_elems elems;
	struct ieee80211_channel *channel;
	size_t baselen;
	int freq;
	enum ieee80211_band band = rx_status->band;

	/* ignore ProbeResp to foreign address */
	if (stype == IEEE80211_STYPE_PROBE_RESP &&
	    !ether_addr_equal(mgmt->da, sdata->vif.addr))
		return;

	baselen = (u8 *) mgmt->u.probe_resp.variable - (u8 *) mgmt;
	if (baselen > len)
		return;

	ieee802_11_parse_elems(mgmt->u.probe_resp.variable, len - baselen,
			       false, &elems);

	/* ignore non-mesh or secure / unsecure mismatch */
	if ((!elems.mesh_id || !elems.mesh_config) ||
	    (elems.rsn && sdata->u.mesh.security == IEEE80211_MESH_SEC_NONE) ||
	    (!elems.rsn && sdata->u.mesh.security != IEEE80211_MESH_SEC_NONE))
		return;

	if (elems.ds_params)
		freq = ieee80211_channel_to_frequency(elems.ds_params[0], band);
	else
		freq = rx_status->freq;

	channel = ieee80211_get_channel(local->hw.wiphy, freq);

	if (!channel || channel->flags & IEEE80211_CHAN_DISABLED)
		return;

	if (mesh_matches_local(sdata, &elems)) {
		if (freq != sdata->vif.bss_conf.chandef.chan->center_freq) {
			printk(KERN_DEBUG "force quit, peer %pM is freq=%d, current op freq=%d\n",
					mgmt->sa,
					freq,
					sdata->vif.bss_conf.chandef.chan->center_freq);
			return;
		}
		mesh_neighbour_update(sdata, mgmt->sa, &elems);
	}
	if (ifmsh->sync_ops)
		ifmsh->sync_ops->rx_bcn_presp(sdata,
				stype, mgmt, &elems, rx_status);

	if (ifmsh->mbca_executor == NL80211_MBCA_EXECUTOR_USERSPACE)
		if (ifmsh->sync_executor != NL80211_MESH_SYNC_EXECUTOR_USERSPACE)
			mesh_sync_offset_rx_bcn_presp_user(sdata,
				stype, mgmt, &elems, rx_status);

	if (ifmsh->csa_role != IEEE80211_MESH_CSA_ROLE_INIT &&
	    !sdata->vif.csa_active)
		ieee80211_mesh_process_chnswitch(sdata, &elems, true);
}

int ieee80211_mesh_finish_csa(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct mesh_csa_settings *tmp_csa_settings;
	int ret = 0;
	int changed = 0;

	/* Reset the TTL value and Initiator flag */
	ifmsh->csa_role = IEEE80211_MESH_CSA_ROLE_NONE;
	ifmsh->chsw_ttl = 0;

	/* Remove the CSA and MCSP elements from the beacon */
	tmp_csa_settings = rcu_dereference(ifmsh->csa);
	RCU_INIT_POINTER(ifmsh->csa, NULL);
	if (tmp_csa_settings)
		kfree_rcu(tmp_csa_settings, rcu_head);
	ret = ieee80211_mesh_rebuild_beacon(sdata);
	if (ret)
		return -EINVAL;

	changed |= BSS_CHANGED_BEACON;

	mcsa_dbg(sdata, "complete switching to center freq %d MHz",
		 sdata->vif.bss_conf.chandef.chan->center_freq);
	return changed;
}

int ieee80211_mesh_csa_beacon(struct ieee80211_sub_if_data *sdata,
			      struct cfg80211_csa_settings *csa_settings)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct mesh_csa_settings *tmp_csa_settings;
	int ret = 0;

	tmp_csa_settings = kmalloc(sizeof(*tmp_csa_settings),
				   GFP_ATOMIC);
	if (!tmp_csa_settings)
		return -ENOMEM;

	memcpy(&tmp_csa_settings->settings, csa_settings,
	       sizeof(struct cfg80211_csa_settings));

	rcu_assign_pointer(ifmsh->csa, tmp_csa_settings);

	ret = ieee80211_mesh_rebuild_beacon(sdata);
	if (ret) {
		tmp_csa_settings = rcu_dereference(ifmsh->csa);
		RCU_INIT_POINTER(ifmsh->csa, NULL);
		kfree_rcu(tmp_csa_settings, rcu_head);
		return ret;
	}

	return BSS_CHANGED_BEACON;
}

static int mesh_fwd_csa_frame(struct ieee80211_sub_if_data *sdata,
			       struct ieee80211_mgmt *mgmt, size_t len)
{
	struct ieee80211_mgmt *mgmt_fwd;
	struct sk_buff *skb;
	struct ieee80211_local *local = sdata->local;
	u8 *pos = mgmt->u.action.u.chan_switch.variable;
	size_t offset_ttl;

	skb = dev_alloc_skb(local->tx_headroom + len);
	if (!skb)
		return -ENOMEM;
	skb_reserve(skb, local->tx_headroom);
	mgmt_fwd = (struct ieee80211_mgmt *) skb_put(skb, len);

	/* offset_ttl is based on whether the secondary channel
	 * offset is available or not. Subtract 1 from the mesh TTL
	 * and disable the initiator flag before forwarding.
	 */
	offset_ttl = (len < 42) ? 7 : 10;
	*(pos + offset_ttl) -= 1;
	*(pos + offset_ttl + 1) &= ~WLAN_EID_CHAN_SWITCH_PARAM_INITIATOR;

	memcpy(mgmt_fwd, mgmt, len);
	eth_broadcast_addr(mgmt_fwd->da);
	memcpy(mgmt_fwd->sa, sdata->vif.addr, ETH_ALEN);
	memcpy(mgmt_fwd->bssid, sdata->vif.addr, ETH_ALEN);

	ieee80211_tx_skb(sdata, skb);
	return 0;
}

static void mesh_rx_csa_frame(struct ieee80211_sub_if_data *sdata,
			      struct ieee80211_mgmt *mgmt, size_t len)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct ieee802_11_elems elems;
	u16 pre_value;
	bool fwd_csa = true;
	size_t baselen;
	u8 *pos;

	if (mgmt->u.action.u.measurement.action_code !=
	    WLAN_ACTION_SPCT_CHL_SWITCH)
		return;

	pos = mgmt->u.action.u.chan_switch.variable;
	baselen = offsetof(struct ieee80211_mgmt,
			   u.action.u.chan_switch.variable);
	ieee802_11_parse_elems(pos, len - baselen, false, &elems);

	ifmsh->chsw_ttl = elems.mesh_chansw_params_ie->mesh_ttl;
	if (!--ifmsh->chsw_ttl)
		fwd_csa = false;

	pre_value = le16_to_cpu(elems.mesh_chansw_params_ie->mesh_pre_value);
	if (ifmsh->pre_value >= pre_value)
		return;

	ifmsh->pre_value = pre_value;

	if (!sdata->vif.csa_active &&
	    !ieee80211_mesh_process_chnswitch(sdata, &elems, false)) {
		mcsa_dbg(sdata, "Failed to process CSA action frame");
		return;
	}

	/* forward or re-broadcast the CSA frame */
	if (fwd_csa) {
		if (mesh_fwd_csa_frame(sdata, mgmt, len) < 0)
			mcsa_dbg(sdata, "Failed to forward the CSA frame");
	}
}

static void ieee80211_mesh_rx_mgmt_action(struct ieee80211_sub_if_data *sdata,
					  struct ieee80211_mgmt *mgmt,
					  size_t len,
					  struct ieee80211_rx_status *rx_status)
{
	switch (mgmt->u.action.category) {
	case WLAN_CATEGORY_SELF_PROTECTED:
		switch (mgmt->u.action.u.self_prot.action_code) {
		case WLAN_SP_MESH_PEERING_OPEN:
		case WLAN_SP_MESH_PEERING_CLOSE:
		case WLAN_SP_MESH_PEERING_CONFIRM:
			mesh_rx_plink_frame(sdata, mgmt, len, rx_status);
			break;
		}
		break;
	case WLAN_CATEGORY_MESH_ACTION:
		if (mesh_action_is_path_sel(mgmt))
			mesh_rx_path_sel_frame(sdata, mgmt, len);
		break;
	case WLAN_CATEGORY_SPECTRUM_MGMT:
		mesh_rx_csa_frame(sdata, mgmt, len);
		break;
	}
}

void ieee80211_mesh_rx_queued_mgmt(struct ieee80211_sub_if_data *sdata,
				   struct sk_buff *skb)
{
	struct ieee80211_rx_status *rx_status;
	struct ieee80211_mgmt *mgmt;
	u16 stype;

	sdata_lock(sdata);

	/* mesh already went down */
	if (!sdata->u.mesh.mesh_id_len)
		goto out;

	rx_status = IEEE80211_SKB_RXCB(skb);
	mgmt = (struct ieee80211_mgmt *) skb->data;
	stype = le16_to_cpu(mgmt->frame_control) & IEEE80211_FCTL_STYPE;

	switch (stype) {
	case IEEE80211_STYPE_PROBE_RESP:
	case IEEE80211_STYPE_BEACON:
		ieee80211_mesh_rx_bcn_presp(sdata, stype, mgmt, skb->len,
					    rx_status);
		break;
	case IEEE80211_STYPE_PROBE_REQ:
		ieee80211_mesh_rx_probe_req(sdata, mgmt, skb->len);
		break;
	case IEEE80211_STYPE_ACTION:
		ieee80211_mesh_rx_mgmt_action(sdata, mgmt, skb->len, rx_status);
		break;
	}
out:
	sdata_unlock(sdata);
}

static void mesh_bss_info_changed(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u32 bit, changed = 0;

	for_each_set_bit(bit, &ifmsh->mbss_changed,
			 sizeof(changed) * BITS_PER_BYTE) {
		clear_bit(bit, &ifmsh->mbss_changed);
		changed |= BIT(bit);
	}

	if (sdata->vif.bss_conf.enable_beacon &&
	    (changed & (BSS_CHANGED_BEACON |
			BSS_CHANGED_HT |
			BSS_CHANGED_BASIC_RATES |
			BSS_CHANGED_BEACON_INT)))
		if (ieee80211_mesh_rebuild_beacon(sdata))
			return;

	ieee80211_bss_info_change_notify(sdata, changed);
}

void ieee80211_mesh_work(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	sdata_lock(sdata);

	/* mesh already went down */
	if (!sdata->u.mesh.mesh_id_len)
		goto out;

	if (ifmsh->preq_queue_len &&
	    time_after(jiffies,
		       ifmsh->last_preq + msecs_to_jiffies(ifmsh->mshcfg.dot11MeshHWMPpreqMinInterval)))
		mesh_path_start_discovery(sdata);

	if (test_and_clear_bit(MESH_WORK_GROW_MPATH_TABLE, &ifmsh->wrkq_flags))
		mesh_mpath_table_grow();

	if (test_and_clear_bit(MESH_WORK_GROW_MPP_TABLE, &ifmsh->wrkq_flags))
		mesh_mpp_table_grow();

	if (test_and_clear_bit(MESH_WORK_HOUSEKEEPING, &ifmsh->wrkq_flags))
		ieee80211_mesh_housekeeping(sdata);

	if (test_and_clear_bit(MESH_WORK_ROOT, &ifmsh->wrkq_flags))
		ieee80211_mesh_rootpath(sdata);

	if (test_and_clear_bit(MESH_WORK_DRIFT_ADJUST, &ifmsh->wrkq_flags)) {
		if (ifmsh->sync_ops)
			ifmsh->sync_ops->adjust_tbtt_setter(sdata);
	}

	if (test_and_clear_bit(MESH_WORK_MBSS_CHANGED, &ifmsh->wrkq_flags))
		mesh_bss_info_changed(sdata);

	if (test_and_clear_bit(MESH_WORK_PS_SET_DRV_STA_PEER_PM, &ifmsh->wrkq_flags))
		ieee80211_mps_set_drv_sta_peer_pm(sdata);

	if (test_and_clear_bit(MESH_WORK_PS_START_FORCE_NONPEER_PM_ACTIVE, &ifmsh->wrkq_flags))
		ieee80211_mps_start_force_nonpeer_pm_active(sdata);

	if (test_and_clear_bit(MESH_WORK_PS_STOP_FORCE_NONPEER_PM_ACTIVE, &ifmsh->wrkq_flags))
		ieee80211_mps_stop_force_nonpeer_pm_active(sdata);
out:
	sdata_unlock(sdata);
}

void ieee80211_mesh_notify_scan_completed(struct ieee80211_local *local)
{
	struct ieee80211_sub_if_data *sdata;

	rcu_read_lock();
	list_for_each_entry_rcu(sdata, &local->interfaces, list)
		if (ieee80211_vif_is_mesh(&sdata->vif) &&
		    ieee80211_sdata_running(sdata))
			ieee80211_queue_work(&local->hw, &sdata->work);
	rcu_read_unlock();
}

void ieee80211_mesh_init_sdata(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	static u8 zero_addr[ETH_ALEN] = {};

	setup_timer(&ifmsh->housekeeping_timer,
		    ieee80211_mesh_housekeeping_timer,
		    (unsigned long) sdata);

	ifmsh->accepting_plinks = true;
	atomic_set(&ifmsh->mpaths, 0);
	ifmsh->last_preq = jiffies;
	ifmsh->next_perr = jiffies;
	ifmsh->csa_role = IEEE80211_MESH_CSA_ROLE_NONE;
	/* Allocate all mesh structures when creating the first mesh interface. */
	if (!mesh_allocated)
		ieee80211s_init();
	setup_timer(&ifmsh->mesh_path_timer,
		    ieee80211_mesh_path_timer,
		    (unsigned long) sdata);
	setup_timer(&ifmsh->mesh_path_root_timer,
		    ieee80211_mesh_path_root_timer,
		    (unsigned long) sdata);
	INIT_LIST_HEAD(&ifmsh->preq_queue.list);
	skb_queue_head_init(&ifmsh->ps.bc_buf);
	spin_lock_init(&ifmsh->mesh_preq_queue_lock);
	spin_lock_init(&ifmsh->sync_offset_lock);
	RCU_INIT_POINTER(ifmsh->beacon, NULL);
	ieee80211_mps_init(sdata);
	INIT_DELAYED_WORK(&ifmsh->mbca_work, mesh_mbca_work);
	INIT_LIST_HEAD(&ifmsh->mbca_tbtt_adjust_list);
	sdata->vif.bss_conf.bssid = zero_addr;
}

int ieee80211_calc_mesh_airtime_link_metric(struct ieee80211_sub_if_data *sdata,
		u8* addr, u32* metric)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info* sta;

	rcu_read_lock();
	sta = sta_info_get(sdata, addr);
	if (!sta) {
		rcu_read_unlock();
		return -EINVAL;
	}
	*metric = airtime_link_metric_get(local, sta);

	rcu_read_unlock();
	return 0;
}

int ieee80211_mesh_get_link_metric_base_info(struct ieee80211_sub_if_data *sdata,
		u8* addr, struct mesh_metric_base* mbase)
{
	struct sta_info* sta;

	rcu_read_lock();
	sta = sta_info_get(sdata, addr);
	if (!sta) {
		rcu_read_unlock();
		return -EINVAL;
	}
	mbase->ht_supported = sta->sta.ht_cap.ht_supported;
	mbase->fail_avg = sta->fail_avg;
	mbase->avg_rate = max(ewma_read(&sta->avg_rate), 10UL);
	mbase->last_signal = sta->last_signal;
	mbase->ref_rssi_value = sta->ref_rssi_value;
	mbase->ref_last_rate = sta->ref_last_rate;
	mbase->max_cap_rate = sta->max_cap_rate;
	mbase->max_rssi_for_max_rate = sta->max_rssi_for_max_rate;
	mbase->rssi_rate_cnt = sta->rssi_rate_cnt;
	mbase->ref_last_tx = sta->ref_last_tx;
	mbase->last_rx = sta->last_rx;
	mbase->current_time = jiffies;
	mbase->avg_signal = (s32) -ewma_read(&sta->avg_signal);
	mbase->is_sleeping = (sta->peer_pm == NL80211_MESH_POWER_LIGHT_SLEEP ||
			sta->peer_pm == NL80211_MESH_POWER_DEEP_SLEEP);

	rcu_read_unlock();

	mhwmp_dbg(sdata, " get link metric base info %pM\n", addr);
	mhwmp_dbg(sdata, "  ht_supported:%d fail_avg:%d avg_rate:%d last_signal:%d\n"
			 "  ref_rssi_value:%d ref_last_rate:%d max_cap_rate:%d\n"
			 "  max_rssi_for_max_rate:%d rssi_rate_cnt:%d ref_last_tx:%lld\n"
			 "  last_rx:%lld current_time:%lld avg_signal:%d is_sleep %d\n",
		mbase->ht_supported, mbase->fail_avg, mbase->avg_rate, mbase->last_signal,
		mbase->ref_rssi_value, mbase->ref_last_rate, mbase->max_cap_rate,
		mbase->max_rssi_for_max_rate, mbase->rssi_rate_cnt, mbase->ref_last_tx,
		mbase->last_rx, mbase->current_time, mbase->avg_signal,
		mbase->is_sleeping);

	return 0;
}

int ieee80211_mesh_set_link_metric_base_info(struct ieee80211_sub_if_data *sdata,
		u8* addr, u32 set_flags, struct mesh_metric_base* mbase)
{
	struct sta_info* sta;


	mhwmp_dbg(sdata, " set link metric base info %pM\n", addr);

	rcu_read_lock();
	sta = sta_info_get(sdata, addr);
	if (!sta) {
		rcu_read_unlock();
		return -EINVAL;
	}
	if (set_flags & METRIC_SET_MAX_CAP_RATE) {
		sta->max_cap_rate = (unsigned int)mbase->max_cap_rate;
		mhwmp_dbg(sdata, " sta->max_cap_rate = %d", sta->max_cap_rate);
	}
	if (set_flags & METRIC_SET_MAX_RSSI_FOR_MRATE) {
		sta->max_rssi_for_max_rate = (int)mbase->max_rssi_for_max_rate;
		mhwmp_dbg(sdata, " sta->max_rssi_for_max_rate = %d", sta->max_rssi_for_max_rate);
	}
	if (set_flags & METRIC_SET_AVG_RATE) {
		ewma_init(&sta->avg_rate, 1, 32);
		ewma_add(&sta->avg_rate, mbase->avg_rate);
		mhwmp_dbg(sdata, " sta->avg_rate = %d", (int)ewma_read(&sta->avg_rate));
	}
	if (set_flags & METRIC_SET_REF_RSSI_VALUE) {
		sta->ref_rssi_value = (int)mbase->ref_rssi_value;
		mhwmp_dbg(sdata, " sta->ref_rssi_value = %d", sta->ref_rssi_value);
	}
	if (set_flags & METRIC_SET_RSSI_RATE_CNT) {
		sta->rssi_rate_cnt = mbase->rssi_rate_cnt;
		mhwmp_dbg(sdata, " sta->rssi_rate_cnt = %d", sta->rssi_rate_cnt);
	}
	if (set_flags & METRIC_SET_FAIL_AVG) {
		sta->fail_avg = (unsigned int)mbase->fail_avg;
		mhwmp_dbg(sdata, " sta->fail_avg = %d", sta->fail_avg);
	}
	if (set_flags & METRIC_SET_REF_LAST_TX) {
		sta->ref_last_tx = (unsigned long)mbase->ref_last_tx;
		mhwmp_dbg(sdata, " sta->ref_last_tx = %lld", (long long int)sta->ref_last_tx);
	}
	rcu_read_unlock();
	return 0;
}
