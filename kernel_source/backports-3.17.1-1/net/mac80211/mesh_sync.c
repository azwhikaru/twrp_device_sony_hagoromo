/*
 * Copyright 2011-2012, Pavel Zubarev <pavel.zubarev@gmail.com>
 * Copyright 2011-2012, Marco Porsch <marco.porsch@s2005.tu-chemnitz.de>
 * Copyright 2011-2012, cozybit Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "ieee80211_i.h"
#include "mesh.h"
#include "driver-ops.h"

/* This is not in the standard.  It represents a tolerable tbtt drift below
 * which we do no TSF adjustment.
 */
#define TOFFSET_MINIMUM_ADJUSTMENT 10

#define TOFFSET_MODE_ATH9K_HTC 0b01    /* for low speed bus IF (incl. USB) */

/* This is not in the standard.  It represents the maximum Toffset jump above
 * which we'll invalidate the Toffset setpoint and choose a new setpoint.  This
 * could be, for instance, in case a neighbor is restarted and its TSF counter
 * reset.
 */
#define TOFFSET_MAXIMUM_ADJUSTMENT 30000		/* 30 ms */

struct sync_method {
	u8 method;
	struct ieee80211_mesh_sync_ops ops;
};

/**
 * mesh_peer_tbtt_adjusting - check if an mp is currently adjusting its TBTT
 *
 * @ie: information elements of a management frame from the mesh peer
 */
static bool mesh_peer_tbtt_adjusting(struct ieee802_11_elems *ie)
{
	return (ie->mesh_config->meshconf_cap &
			IEEE80211_MESHCONF_CAPAB_TBTT_ADJUSTING) != 0;
}

void mesh_sync_adjust_tbtt(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	/* sdata->vif.bss_conf.beacon_int in 1024us units, 0.04% */
	u64 beacon_int_fraction = sdata->vif.bss_conf.beacon_int * 1024 / 2500;
	u64 tsf = 0;
	u64 tsfdelta = 0;
	/* Sony htc start */
	u64 toffset_min_set_latency;
	int toffset_mode;
	/* Sony htc end */

	spin_lock_bh(&ifmsh->sync_offset_lock);
	if (ifmsh->adjusting_tbtt) {
		ifmsh->sync_offset_clockdrift_max = 0;
		spin_unlock_bh(&ifmsh->sync_offset_lock);
		msync_dbg(sdata,
			"adjust_tbtt: skip clock drift adjustment for TBTT adjustment");
		return;
	}

	/* Sony htc start */
	/* This is not in the standard. It is an additional margin added to the
	 * Toffset setpoint to mitigate TSF overcorrection
	 * introduced by TSF adjustment latency. The value reflects the minimal
	 * unavoidable R/W latency that occurs *all times*. This value is definitely
	 * target hardware dependent value.
	 */
	toffset_mode = ifmsh->mshcfg.dot11Mesh_toffset_mode;
	/* Sony htc end */
	if (ifmsh->sync_offset_clockdrift_max < beacon_int_fraction) {
		msync_dbg(sdata, "TBTT : max clockdrift=%lld; adjusting\n",
			  (long long) ifmsh->sync_offset_clockdrift_max);
		tsfdelta = -ifmsh->sync_offset_clockdrift_max;
		ifmsh->sync_offset_clockdrift_max = 0;
	} else {
		msync_dbg(sdata, "TBTT : max clockdrift=%lld; adjusting by %llu\n",
			  (long long) ifmsh->sync_offset_clockdrift_max,
			  (unsigned long long) beacon_int_fraction);
		tsfdelta = -beacon_int_fraction;
		ifmsh->sync_offset_clockdrift_max -= beacon_int_fraction;
	}
	/* Sony htc start */
	/* Update to mitigate USB IF latency. */
	toffset_min_set_latency = ifmsh->mshcfg.dot11Mesh_toffset_set_latency;
	tsfdelta += toffset_min_set_latency;
	/* Sony htc end */
	spin_unlock_bh(&ifmsh->sync_offset_lock);

	if (toffset_mode != TOFFSET_MODE_ATH9K_HTC)
		spin_lock_bh(&ifmsh->sync_offset_lock);
	if (local->ops->delay_tsf)
		drv_delay_tsf(local, sdata, -tsfdelta);
	else {
		tsf = drv_get_tsf(local, sdata);
		if (tsf != -1ULL)
			drv_set_tsf(local, sdata, tsf + tsfdelta);
	}
	if (toffset_mode != TOFFSET_MODE_ATH9K_HTC)
		spin_unlock_bh(&ifmsh->sync_offset_lock);

	msync_dbg(sdata, "mesh_sync_adjust_tbtt(): tsf %llu tsfdelta %llu\n",
			tsf, tsfdelta);
}

static u64 ath9k_htc_rx_timestamp_correction(u64 t_r, u64 t_r_org)
{
	u64 ret;
	ret = (t_r & 0x7fffULL) | (t_r_org & ~0x7fffULL);
	if ((t_r & 0x7fffULL) > (t_r_org & 0x7fffULL)){
		ret -= 0x8000ULL;        // subtract carry bit
	}
	return ret;
}

static void mesh_sync_offset_rx_bcn_presp(struct ieee80211_sub_if_data *sdata,
				   u16 stype,
				   struct ieee80211_mgmt *mgmt,
				   struct ieee802_11_elems *elems,
				   struct ieee80211_rx_status *rx_status)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;
	u64 t_t, t_r;
	s64 t_clockdrift;
	/* Sony htc start */
	u64 t_r_org;
	/* This is not in the standard. It is a margin added to the
	 * Toffset setpoint to mitigate TSF overcorrection
	 * introduced by TSF adjustment latency. As a result, tolerable tbtt drift
	 * becomes TOFFSET_MINUMUM_ADJUSTMENT + TOOFSET_SET_MARGIN.
	 */
	u64 toffset_set_margin;
	/* Sony htc end */

	WARN_ON(ifmsh->mesh_sp_id != IEEE80211_SYNC_METHOD_NEIGHBOR_OFFSET);

	/* standard mentions only beacons */
	if (stype != IEEE80211_STYPE_BEACON)
		return;

	/*
	 * Get time when timestamp field was received.  If we don't
	 * have rx timestamps, then use current tsf as an approximation.
	 * drv_get_tsf() must be called before entering the rcu-read
	 * section.
	 */
	t_r = drv_get_tsf(local, sdata);
	t_r_org = t_r;

	rcu_read_lock();
	sta = sta_info_get(sdata, mgmt->sa);
	if (!sta)
		goto no_sync;

	/* check offset sync conditions (13.13.2.2.1)
	 *
	 * TODO also sync to
	 * dot11MeshNbrOffsetMaxNeighbor non-peer non-MBSS neighbors
	 */

	if (elems->mesh_config && mesh_peer_tbtt_adjusting(elems)) {
		set_sta_flag(sta, WLAN_STA_TBTT_ADJUSTING);
		clear_sta_flag(sta, WLAN_STA_TOFFSET_KNOWN);
		msync_dbg(sdata, "STA %pM : is adjusting TBTT\n",
			  sta->sta.addr);
		goto no_sync;
	}
	clear_sta_flag(sta, WLAN_STA_TBTT_ADJUSTING);

	if (ifmsh->adjusting_tbtt) {
		msync_dbg(sdata,
			"offset_rx_bcn_presp: skip clock drift adjustment for TBTT adjustment");
		ifmsh->sync_offset_clockdrift_max = 0;
		goto no_sync;
	}

	if (ieee80211_have_rx_timestamp(rx_status)) {
		/* workaround for ath9k_htc */
		if (ifmsh->mshcfg.dot11Mesh_toffset_mode
				== TOFFSET_MODE_ATH9K_HTC)
			rx_status->mactime
				= ath9k_htc_rx_timestamp_correction(rx_status->mactime,
						t_r_org);
		/* time when timestamp field was received */
		t_r = ieee80211_calculate_rx_timestamp(local, rx_status,
						       24 + 12 +
						       elems->total_len +
						       FCS_LEN,
						       24);
	}

	/* Timing offset calculation (see 13.13.2.2.2) */
	t_t = le64_to_cpu(mgmt->u.beacon.timestamp);
	sta->t_offset = t_t - t_r;

	if (test_sta_flag(sta, WLAN_STA_TOFFSET_KNOWN)) {
		/* for sony debug to trace ath9k_htc behavior... */
		msync_dbg(sdata, "STA %pM : t_r_org=%lld, t_r=%lld, t_t=%lld\n",
			  sta->sta.addr,
			  (long long) t_r_org,
			  (long long) t_r,
			  (long long) t_t);
		/* for sony debug to here... */

		t_clockdrift = sta->t_offset_setpoint - sta->t_offset;
		msync_dbg(sdata,
			  "STA %pM : sta->t_offset=%lld, sta->t_offset_setpoint=%lld, t_clockdrift=%lld\n",
			  sta->sta.addr, (long long) sta->t_offset,
			  (long long) sta->t_offset_setpoint,
			  (long long) t_clockdrift);

		if (t_clockdrift > TOFFSET_MAXIMUM_ADJUSTMENT ||
		    t_clockdrift < -TOFFSET_MAXIMUM_ADJUSTMENT) {
			msync_dbg(sdata,
				  "STA %pM : t_clockdrift=%lld too large, setpoint reset\n",
				  sta->sta.addr,
				  (long long) t_clockdrift);
			clear_sta_flag(sta, WLAN_STA_TOFFSET_KNOWN);
			goto no_sync;
		}

		spin_lock_bh(&ifmsh->sync_offset_lock);
		if (t_clockdrift > ifmsh->sync_offset_clockdrift_max)
			ifmsh->sync_offset_clockdrift_max = t_clockdrift;
		spin_unlock_bh(&ifmsh->sync_offset_lock);
	} else {
		toffset_set_margin =
		    ifmsh->mshcfg.dot11Mesh_toffset_set_margin;
		sta->t_offset_setpoint = sta->t_offset - toffset_set_margin;
		set_sta_flag(sta, WLAN_STA_TOFFSET_KNOWN);
		msync_dbg(sdata, "STA %pM : offset was invalid, "
			  " sta->t_offset=%lld, offset_set_margin=%lld\n",
			  sta->sta.addr,
			  (long long) sta->t_offset,
			  toffset_set_margin);
	}

no_sync:
	rcu_read_unlock();
}

static void mesh_sync_offset_adjust_tbtt(struct ieee80211_sub_if_data *sdata,
					 struct beacon_data *beacon)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;

	WARN_ON(ifmsh->mesh_sp_id != IEEE80211_SYNC_METHOD_NEIGHBOR_OFFSET);
	WARN_ON(!rcu_read_lock_held());

	spin_lock_bh(&ifmsh->sync_offset_lock);

	if (ifmsh->sync_offset_clockdrift_max > TOFFSET_MINIMUM_ADJUSTMENT) {
		/* Since ajusting the tsf here would
		 * require a possibly blocking call
		 * to the driver tsf setter, we punt
		 * the tsf adjustment to the mesh tasklet
		 */
		msync_dbg(sdata,
			  "TBTT: kicking off clock drift adjustment with clockdrift_max=%lld\n",
			  ifmsh->sync_offset_clockdrift_max);
		set_bit(MESH_WORK_DRIFT_ADJUST, &ifmsh->wrkq_flags);
	} else {
		ifmsh->sync_offset_clockdrift_max = 0;
	}
	spin_unlock_bh(&ifmsh->sync_offset_lock);
}

/**
 * The following section is for userspace handled synchronization.
 */

/* MBCA hack: remove static */
/* static void mesh_sync_offset_rx_bcn_presp_user(struct ieee80211_sub_if_data *sdata, */
void mesh_sync_offset_rx_bcn_presp_user(struct ieee80211_sub_if_data *sdata,
				   u16 stype,
				   struct ieee80211_mgmt *mgmt,
				   struct ieee802_11_elems *elems,
				   struct ieee80211_rx_status *rx_status)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;
	u64 t_t, t_r;
	u64 t_r_org;
	u8 meshconf_cap = 0;
	struct mesh_sync_request sync_req;

	//WARN_ON(ifmsh->mesh_sp_id != IEEE80211_SYNC_METHOD_USER_HANDLED);
	WARN_ON(ifmsh->mesh_sp_id != IEEE80211_SYNC_METHOD_NEIGHBOR_OFFSET);

	/* standard mentions only beacons */
	if (stype != IEEE80211_STYPE_BEACON)
		return;

	if (elems->mesh_config)
		meshconf_cap = elems->mesh_config->meshconf_cap;

	/* The current tsf is a first approximation for the timestamp
	 * for the received beacon.  Further down we try to get a
	 * better value from the rx_status->mactime field if
	 * available. Also we have to call drv_get_tsf() before
	 * entering the rcu-read section.*/
	t_r = drv_get_tsf(local, sdata);
	t_r_org = t_r;

	rcu_read_lock();
	sta = sta_info_get(sdata, mgmt->sa);
	if (!sta)
		goto no_sync;

	/* check offset sync conditions (13.13.2.2.1)
	 *
	 * TODO also sync to
	 * dot11MeshNbrOffsetMaxNeighbor non-peer non-MBSS neighbors
	 */

	if (ieee80211_have_rx_timestamp(rx_status)) {
		/* workaround for ath9k_htc */
		if (ifmsh->mshcfg.dot11Mesh_toffset_mode
				== TOFFSET_MODE_ATH9K_HTC)
			rx_status->mactime
				= ath9k_htc_rx_timestamp_correction(rx_status->mactime,
						t_r_org);
		/* time when timestamp field was received */
		t_r = ieee80211_calculate_rx_timestamp(local, rx_status,
						       24 + 12 +
						       elems->total_len +
						       FCS_LEN,
						       24);
	}

	t_t = le64_to_cpu(mgmt->u.beacon.timestamp);

	// notify to userspace as BCN RX event
	sync_req.addr = sta->sta.addr;
	sync_req.recv_ts = t_r;
	sync_req.hdr_ts = t_t;
	sync_req.beacon_int = mgmt->u.beacon.beacon_int;
	sync_req.mesh_cap = meshconf_cap;
	sync_req.bcn_timing_len = elems->bcn_timing_len;
	sync_req.bcn_timing = elems->bcn_timing;
	sync_req.plink_state = sta->plink_state;
	sync_req.llid = sta->llid;
	sync_req.plid = sta->plid;

	cfg80211_mesh_sync_request(&sdata->wdev, &sync_req, GFP_ATOMIC);

	msync_dbg(sdata,
                 "BCN rcvd from STA %pM : recv_ts=%lld hdr_ts=%lld\n",
                 sta->sta.addr, t_r, t_t);

no_sync:
	rcu_read_unlock();
}

void mesh_sync_adjust_tbtt_user(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u64 tsf = 0;
	u64 tsfdelta;
	int toffset_mode = ifmsh->mshcfg.dot11Mesh_toffset_mode;
	struct mesh_sync_request sync_req;

	tsfdelta = -ifmsh->sync_offset_clockdrift_max;

	// adjust own TSF value as ordred
	// workaround for ath9k_htc
	if (toffset_mode != TOFFSET_MODE_ATH9K_HTC)
		spin_lock_bh(&ifmsh->sync_offset_lock);
	if (local->ops->delay_tsf)
		drv_delay_tsf(local, sdata, -tsfdelta);
	else {
		tsf = drv_get_tsf(local, sdata);
		if (tsf != -1ULL)
			drv_set_tsf(local, sdata, tsf + tsfdelta);
	}
	if (toffset_mode != TOFFSET_MODE_ATH9K_HTC)
		spin_unlock_bh(&ifmsh->sync_offset_lock);

	// notify to userspace as BCN TX event
	memset(&sync_req, 0, sizeof(sync_req));
	sync_req.addr = sdata->vif.addr;
	sync_req.recv_ts = ifmsh->sync_offset_clockdrift_max;
	cfg80211_mesh_sync_request(&sdata->wdev, &sync_req, GFP_ATOMIC);

	msync_dbg(sdata, "mesh_sync_adjust_tbtt_user(): tsf %llu tsfdelta %llu\n",
			tsf, tsfdelta);
}

static void mesh_sync_offset_adjust_tbtt_user(struct ieee80211_sub_if_data *sdata,
					      struct beacon_data *beacon)
{
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	struct mesh_sync_request sync_req;

	WARN_ON(ifmsh->mesh_sp_id != IEEE80211_SYNC_METHOD_NEIGHBOR_OFFSET);
	BUG_ON(!rcu_read_lock_held());

	// only add work queue
	spin_lock_bh(&ifmsh->sync_offset_lock);
	if (ifmsh->sync_offset_clockdrift_max != 0) {
		msync_dbg(sdata,
			  "TBTT: kicking off clock drift adjustment with clockdrift_max=%lld\n",
			  ifmsh->sync_offset_clockdrift_max);
		set_bit(MESH_WORK_DRIFT_ADJUST, &ifmsh->wrkq_flags);
		spin_unlock_bh(&ifmsh->sync_offset_lock);
	} else {
		spin_unlock_bh(&ifmsh->sync_offset_lock);
		// notify to userspace as BCN TX event

		memset(&sync_req, 0, sizeof(sync_req));
		sync_req.addr = sdata->vif.addr;
		sync_req.recv_ts = ifmsh->sync_offset_clockdrift_max;
		cfg80211_mesh_sync_request(&sdata->wdev, &sync_req, GFP_ATOMIC);
	}
	return;
}

static const struct sync_method sync_methods[] = {
	{
		.method = IEEE80211_SYNC_METHOD_NEIGHBOR_OFFSET,
		.ops = {
			.rx_bcn_presp = &mesh_sync_offset_rx_bcn_presp,
			.adjust_tbtt = &mesh_sync_offset_adjust_tbtt,
			.adjust_tbtt_setter = &mesh_sync_adjust_tbtt,
		}
	},
};

static const struct ieee80211_mesh_sync_ops user_sync_ops = {
	.rx_bcn_presp = &mesh_sync_offset_rx_bcn_presp_user,
	.adjust_tbtt = &mesh_sync_offset_adjust_tbtt_user,
	.adjust_tbtt_setter = &mesh_sync_adjust_tbtt_user,
};

const struct ieee80211_mesh_sync_ops *ieee80211_mesh_sync_ops_get(u8 method,
		enum nl80211_mesh_sync_executor sync_executor)
{
	int i;

	if (sync_executor == NL80211_MESH_SYNC_EXECUTOR_HW)
		return NULL;

	if (sync_executor == NL80211_MESH_SYNC_EXECUTOR_USERSPACE)
		return &user_sync_ops;

	/* then, NL80211_MESH_SYNC_EXECUTOR_KERNEL */
	for (i = 0 ; i < ARRAY_SIZE(sync_methods); ++i) {
		if (sync_methods[i].method == method)
			return &sync_methods[i].ops;
	}
	return NULL;
}
