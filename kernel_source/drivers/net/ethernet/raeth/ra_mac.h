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
#ifndef RA_MAC_H
#define RA_MAC_H

struct proc_file {
	const char * const name;
	struct file_operations *fops;
};

void raeth_stop(END_DEVICE *ei_local);
void raethMac2AddressSet(unsigned char p[6]);
void ethtool_init(struct net_device *dev);

void feEnableInterrupt(void);

void dump_qos(void);
void dump_reg(struct seq_file *s);
void dump_cp0(void);

int debug_proc_init(void);
void debug_proc_exit(void);

int TsoLenUpdate(int tso_len);
int NumOfTxdUpdate(int num_of_txd);

#ifdef CONFIG_RAETH_LRO
int LroStatsUpdate(struct net_lro_mgr *lro_mgr, bool all_flushed);
#endif
#ifdef CONFIG_RAETH_HW_LRO
int HwLroStatsUpdate(unsigned int ring_num, unsigned int agg_cnt, unsigned int agg_size);
#if defined(CONFIG_RAETH_HW_LRO_REASON_DBG)
#define HW_LRO_AGG_FLUSH        (1)
#define HW_LRO_AGE_FLUSH        (2)
#define HW_LRO_NOT_IN_SEQ_FLUSH (3)
#define HW_LRO_TIMESTAMP_FLUSH  (4)
#define HW_LRO_NON_RULE_FLUSH   (5)
int HwLroFlushStatsUpdate(unsigned int ring_num, unsigned int flush_reason);
#endif  /* CONFIG_RAETH_HW_LRO_REASON_DBG */
typedef int (*HWLRO_DBG_FUNC)(int par1, int par2);
int hwlro_agg_cnt_ctrl(int par1, int par2);
int hwlro_agg_time_ctrl(int par1, int par2);
int hwlro_age_time_ctrl(int par1, int par2);
int hwlro_pkt_int_alpha_ctrl(int par1, int par2);
int hwlro_threshold_ctrl(int par1, int par2);
int hwlro_fix_setting_switch_ctrl(int par1, int par2);
#endif  /* CONFIG_RAETH_HW_LRO */
int getnext(const char *src, int separator, char *dest);
int str_to_ip(unsigned int *ip, const char *str);

#if defined(CONFIG_RAETH_PDMA_DVT)
typedef int (*PDMA_DBG_FUNC)(int par1, int par2);
#endif  //#if defined(CONFIG_RAETH_PDMA_DVT)
#endif
