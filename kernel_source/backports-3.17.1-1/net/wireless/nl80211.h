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
#ifndef __NET_WIRELESS_NL80211_H
#define __NET_WIRELESS_NL80211_H

#include "core.h"

int nl80211_init(void);
void nl80211_exit(void);
void nl80211_notify_wiphy(struct cfg80211_registered_device *rdev,
			  enum nl80211_commands cmd);
void nl80211_send_scan_start(struct cfg80211_registered_device *rdev,
			     struct wireless_dev *wdev);
struct sk_buff *nl80211_build_scan_msg(struct cfg80211_registered_device *rdev,
				       struct wireless_dev *wdev, bool aborted);
void nl80211_send_scan_result(struct cfg80211_registered_device *rdev,
			      struct sk_buff *msg);
void nl80211_send_sched_scan(struct cfg80211_registered_device *rdev,
			     struct net_device *netdev, u32 cmd);
void nl80211_send_sched_scan_results(struct cfg80211_registered_device *rdev,
				     struct net_device *netdev);
void nl80211_send_reg_change_event(struct regulatory_request *request);
void nl80211_send_rx_auth(struct cfg80211_registered_device *rdev,
			  struct net_device *netdev,
			  const u8 *buf, size_t len, gfp_t gfp);
void nl80211_send_rx_assoc(struct cfg80211_registered_device *rdev,
			   struct net_device *netdev,
			   const u8 *buf, size_t len, gfp_t gfp);
void nl80211_send_deauth(struct cfg80211_registered_device *rdev,
			 struct net_device *netdev,
			 const u8 *buf, size_t len, gfp_t gfp);
void nl80211_send_disassoc(struct cfg80211_registered_device *rdev,
			   struct net_device *netdev,
			   const u8 *buf, size_t len, gfp_t gfp);
void nl80211_send_auth_timeout(struct cfg80211_registered_device *rdev,
			       struct net_device *netdev,
			       const u8 *addr, gfp_t gfp);
void nl80211_send_assoc_timeout(struct cfg80211_registered_device *rdev,
				struct net_device *netdev,
				const u8 *addr, gfp_t gfp);
void nl80211_send_connect_result(struct cfg80211_registered_device *rdev,
				 struct net_device *netdev, const u8 *bssid,
				 const u8 *req_ie, size_t req_ie_len,
				 const u8 *resp_ie, size_t resp_ie_len,
				 u16 status, gfp_t gfp);
void nl80211_send_roamed(struct cfg80211_registered_device *rdev,
			 struct net_device *netdev, const u8 *bssid,
			 const u8 *req_ie, size_t req_ie_len,
			 const u8 *resp_ie, size_t resp_ie_len, gfp_t gfp);
void nl80211_send_disconnected(struct cfg80211_registered_device *rdev,
			       struct net_device *netdev, u16 reason,
			       const u8 *ie, size_t ie_len, bool from_ap);

void
nl80211_michael_mic_failure(struct cfg80211_registered_device *rdev,
			    struct net_device *netdev, const u8 *addr,
			    enum nl80211_key_type key_type,
			    int key_id, const u8 *tsc, gfp_t gfp);

void
nl80211_send_beacon_hint_event(struct wiphy *wiphy,
			       struct ieee80211_channel *channel_before,
			       struct ieee80211_channel *channel_after);

void nl80211_send_ibss_bssid(struct cfg80211_registered_device *rdev,
			     struct net_device *netdev, const u8 *bssid,
			     gfp_t gfp);

int nl80211_send_mgmt(struct cfg80211_registered_device *rdev,
		      struct wireless_dev *wdev, u32 nlpid,
		      int freq, int sig_dbm,
		      const u8 *buf, size_t len, u32 flags, gfp_t gfp);

void
nl80211_radar_notify(struct cfg80211_registered_device *rdev,
		     const struct cfg80211_chan_def *chandef,
		     enum nl80211_radar_event event,
		     struct net_device *netdev, gfp_t gfp);

void nl80211_send_ap_stopped(struct wireless_dev *wdev);

void cfg80211_rdev_free_coalesce(struct cfg80211_registered_device *rdev);

// sony specific command
void nl80211_mesh_discovery_request(struct cfg80211_registered_device *rdev,
		struct wireless_dev *wdev, const u8 *addr, const u8 *mpp, gfp_t gfp);

void nl80211_plink_broken(struct cfg80211_registered_device *rdev,
		struct wireless_dev *wdev, const u8 *addr, u16 reason, gfp_t gfp);

void nl80211_mesh_sync_request(struct cfg80211_registered_device *rdev,
			       struct wireless_dev *wdev,
			       struct mesh_sync_request *sync_req,
			       gfp_t gfp);

void nl80211_mesh_mbca_tbtt_adjust_event(
		struct cfg80211_registered_device *rdev,
		struct wireless_dev *wdev,
		enum nl80211_tbtt_adjust_action action, u64 delay);

#endif /* __NET_WIRELESS_NL80211_H */
