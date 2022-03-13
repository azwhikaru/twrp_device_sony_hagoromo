/*
 * Copyright 2015  Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "ieee80211_i.h"
#include "mesh.h"
#include "driver-ops.h"

/* XXX: this is copied from mesh_sync.c. remove it later */
#ifndef TOFFSET_MODE_ATH9K_HTC
#define TOFFSET_MODE_ATH9K_HTC 0b01    /* for low speed bus IF (incl. USB) */
#endif

/* TODO: move to and merged with mesh_sync.c */
static void mesh_mbca_delay_tsf(struct ieee80211_sub_if_data *sdata, u64 delay)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_mesh *ifmsh = &sdata->u.mesh;
	u64 tsf = 0;

	/* Sony htc start */
	u64 toffset_min_set_latency;
	int toffset_mode;
	/* Sony htc end */

	spin_lock_bh(&ifmsh->sync_offset_lock);
	/* Sony htc start */
	/* This is not in the standard. It is an additional margin added to the
	 * Toffset setpoint to mitigate TSF overcorrection
	 * introduced by TSF adjustment latency. The value reflects the minimal
	 * unavoidable R/W latency that occurs *all times*. This value is definitely
	 * target hardware dependent value.
	 */
	toffset_mode = ifmsh->mshcfg.dot11Mesh_toffset_mode;
	toffset_min_set_latency = ifmsh->mshcfg.dot11Mesh_toffset_set_latency;
	delay += toffset_min_set_latency;
	/* Sony htc end */
	spin_unlock_bh(&ifmsh->sync_offset_lock);

	if (local->ops->delay_tsf) {
		drv_delay_tsf(local, sdata, delay);
	} else {
		if (toffset_mode != TOFFSET_MODE_ATH9K_HTC)
			spin_lock_bh(&ifmsh->sync_offset_lock);

		tsf = drv_get_tsf(local, sdata);
		if (delay && tsf != -1ULL)
			drv_set_tsf(local, sdata, tsf - delay);

		if (toffset_mode != TOFFSET_MODE_ATH9K_HTC)
			spin_unlock_bh(&ifmsh->sync_offset_lock);
	}

	msync_dbg(sdata, "mesh_mbca_delay_tsf(): tsf %llu delay %llu\n",
			tsf, delay);

}

void mesh_mbca_free_beacon_timimg_ie(struct beacon_timing_ie *ie)
{
	if (!ie)
		return;

	kfree(ie->ie_char);
	kfree(ie);
}

struct beacon_timing_ie *mesh_mbca_alloc_beacon_timimg_ie(u8 *ie,
		size_t ie_len)
{
	struct beacon_timing_ie *btie;

	btie = kzalloc(sizeof(*btie), GFP_KERNEL);
	if (!btie)
		return NULL;

	if (!ie || !ie_len)
		return btie;

	btie->ie_char = kzalloc(ie_len, GFP_KERNEL);
	if (!btie->ie_char) {
		kfree(btie);
		return NULL;
	}

	memcpy(btie->ie_char, ie, ie_len);
	btie->ie_len = ie_len;
	return btie;
}

void mesh_mbca_free_tbtt_adjust_setting(
		struct mbca_tbtt_adjust_setting *setting)
{
	if (!setting)
		return;

	mesh_mbca_free_beacon_timimg_ie(setting->bcn_timing_ie);
	kfree(setting);
}

struct mbca_tbtt_adjust_setting *mesh_mbca_alloc_tbtt_adjust_setting(
		struct ieee80211_if_mesh *ifmsh,
		enum nl80211_tbtt_adjust_action action,
		u64 delay, u8 *ie, size_t ie_len)
{
	struct mbca_tbtt_adjust_setting *setting;

	setting = kzalloc(sizeof(*setting), GFP_KERNEL);
	if (!setting)
		goto out_err;

	if (ie && ie_len) {
		setting->bcn_timing_ie =
			mesh_mbca_alloc_beacon_timimg_ie(ie, ie_len);
		if (!setting->bcn_timing_ie) {
			goto out_err;
		}
	}

	setting->action = action;
	setting->delay = delay;

	return setting;

out_err:
	mesh_mbca_free_tbtt_adjust_setting(setting);
	return NULL;
}

/**
 *  unset optimaization to use do_div() safely for ARM processors
 */
int __attribute__((optimize("O0"))) mesh_mbca_get_tick_to_next_adjustment(
		struct ieee80211_sub_if_data *sdata, unsigned long *tick)
{
	u64 now, bcn_mod, bcn_intval_usec, tick_u64;
	static const unsigned long USEC_PER_TICK = 1000000/HZ;

	now = drv_get_tsf(sdata->local, sdata);
	if (now == -1ULL) {
		printk(KERN_ERR "drv_get_tsf() failed\n");
		return -1;
	}

	bcn_intval_usec = sdata->vif.bss_conf.beacon_int << 10;  /* TU -> usec */
	bcn_mod = do_div(now, bcn_intval_usec);

	/* set deleyed_work at the middle of the next beacon interval */
	tick_u64 = (bcn_intval_usec + (bcn_intval_usec >> 1) - bcn_mod);
	/* printk(KERN_INFO "HZ %d, bcn_intval_usec %llu, bcn_mod %llu, tick_u64 %llu\n",
			HZ, bcn_intval_usec, bcn_mod, tick_u64); */
	do_div(tick_u64, USEC_PER_TICK);

	*tick = tick_u64;
	return 0;
}

void do_finish_tbtt_adjustment(struct ieee80211_if_mesh *ifmsh,
		enum nl80211_tbtt_adjust_action action)
{
	struct ieee80211_sub_if_data *sdata =
		container_of(ifmsh, struct ieee80211_sub_if_data, u.mesh);
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;

	ifmsh->adjusting_tbtt = false;
	if (ifmsh->sync_executor == NL80211_MESH_SYNC_EXECUTOR_HW &&
			local->ops->enable_clock_sync) {
		drv_enable_clock_sync(local, sdata, true);
	}

	if (ifmsh->sync_ops &&
			ifmsh->sync_executor == NL80211_MESH_SYNC_EXECUTOR_KERNEL) {
		/* in case of kernel sync */
		rcu_read_lock();
		list_for_each_entry_rcu(sta, &sdata->local->sta_list, list) {
			if (sdata != sta->sdata)
				continue;
			clear_sta_flag(sta, WLAN_STA_TOFFSET_KNOWN);
		}
		rcu_read_unlock();
		spin_lock_bh(&ifmsh->sync_offset_lock);
		ifmsh->sync_offset_clockdrift_max = 0;
		spin_unlock_bh(&ifmsh->sync_offset_lock);
	}
	/* notify user space that TBTT adjusting finished */
	cfg80211_mesh_mbca_tbtt_adjust_event(&sdata->wdev, action, 0);
}


void mesh_mbca_work(struct work_struct *work)
{
	struct ieee80211_if_mesh *ifmsh =
		container_of(work, struct ieee80211_if_mesh, mbca_work.work);
	struct ieee80211_sub_if_data *sdata =
		container_of(ifmsh, struct ieee80211_sub_if_data, u.mesh);
	struct mbca_tbtt_adjust_setting* setting;
	unsigned long tick;

	if (test_and_clear_bit(MESH_WORK_MBCA_ADJUST_TBTT, &ifmsh->wrkq_flags)) {
		msync_dbg(sdata, "mesh_mbca_work: MESH_WORK_MBCA_ADJUST_TBTT\n");

		setting = list_first_entry_or_null(&ifmsh->mbca_tbtt_adjust_list,
				struct mbca_tbtt_adjust_setting, list);
		if (!setting) {
			printk(KERN_ERR "no entry in ifmsh->mbca_tbtt_adjust_list");
			return;
		}
		list_del(&setting->list);
		--ifmsh->mbca_tbtt_adjust_list_len;

		if (setting->action == NL80211_TBTT_ADJUST_ACTION_EXECUTE) {
			mesh_mbca_delay_tsf(sdata, setting->delay);

		} else {
			/* then, NL80211_TBTT_ADJUST_ACTION_FINISH */
			msync_dbg(sdata, "TBTT adjustment finished!\n");
			do_finish_tbtt_adjustment(ifmsh,
					NL80211_TBTT_ADJUST_ACTION_FINISH);
		}

		ifmsh->bcn_timing_ie = setting->bcn_timing_ie;
		setting->bcn_timing_ie = NULL;
		mesh_mbca_free_tbtt_adjust_setting(setting);

		ieee80211_mbss_info_change_notify(sdata, BSS_CHANGED_BEACON);

		/* schedule next adjusting if needed*/
		setting = list_first_entry_or_null(&ifmsh->mbca_tbtt_adjust_list,
				struct mbca_tbtt_adjust_setting, list);
		if (!setting) {
			return;
		}

		mesh_mbca_get_tick_to_next_adjustment(sdata, &tick);

		set_bit(MESH_WORK_MBCA_ADJUST_TBTT, &ifmsh->wrkq_flags);
		ieee80211_queue_delayed_work(&sdata->local->hw,
					&ifmsh->mbca_work, tick);
	}

}

int mesh_mbca_cancel_tbtt_adjustment(struct ieee80211_if_mesh *ifmsh,
		u8 *ie, size_t ie_len)
{
	struct ieee80211_sub_if_data *sdata =
		container_of(ifmsh, struct ieee80211_sub_if_data, u.mesh);
	struct beacon_timing_ie *bcn_timing_ie = NULL;

	/* assumed sdata is locked */

	msync_dbg(sdata, "TBTT adjustment cancelled!\n");

	if (!ifmsh->adjusting_tbtt)
		return 0;

	do_finish_tbtt_adjustment(ifmsh, NL80211_TBTT_ADJUST_ACTION_CANCEL);
	mesh_mbca_teardown(ifmsh);

	bcn_timing_ie = mesh_mbca_alloc_beacon_timimg_ie(ie, ie_len);
	if (!bcn_timing_ie)
		return -ENOMEM;

	mesh_mbca_free_beacon_timimg_ie(ifmsh->bcn_timing_ie);
	ifmsh->bcn_timing_ie = bcn_timing_ie;
	ieee80211_mbss_info_change_notify(sdata, BSS_CHANGED_BEACON);

	return 0;
}

void mesh_mbca_teardown(struct ieee80211_if_mesh *ifmsh)
{
	/* assumed sdata is locked */
	struct mbca_tbtt_adjust_setting *setting, *next;

	cancel_delayed_work_sync(&ifmsh->mbca_work);

	list_for_each_entry_safe(setting, next, &ifmsh->mbca_tbtt_adjust_list, list) {
		list_del(&setting->list);
		mesh_mbca_free_tbtt_adjust_setting(setting);
	}

	ifmsh->mbca_tbtt_adjust_list_len = 0;
}
