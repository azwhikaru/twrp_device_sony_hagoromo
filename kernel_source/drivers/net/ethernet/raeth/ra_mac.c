/*
 * Copyright 2016 Sony Corporation
 * File Changed on 2016-10-17
 */
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
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <linux/irq.h>
#include <linux/ctype.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,4)
#include <asm/system.h>
#include <linux/mca.h>
#endif
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/dma.h>

#include <asm/rt2880/surfboardint.h>	/* for cp0 reg access, added by bobtseng */

#include <linux/errno.h>
#include <linux/init.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/seq_file.h>
#include <linux/mii.h>


#if defined(CONFIG_RAETH_LRO)
#include <linux/inet_lro.h>
#endif

#include "ra_reg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ethtool.h"
#if defined(CONFIG_RAETH_PDMA_DVT)
#include "dvt/raether_pdma_dvt.h"
#endif  //#if defined(CONFIG_RAETH_PDMA_DVT)

extern struct net_device *dev_raether;

#if defined(CONFIG_RAETH_TSO)
int txd_cnt[MAX_SKB_FRAGS/2 + 1];
int tso_cnt[16];
#endif

#if defined(CONFIG_RAETH_LRO)
#define MAX_AGGR 64
#define MAX_DESC  8
int lro_stats_cnt[MAX_AGGR + 1];
int lro_flush_cnt[MAX_AGGR + 1];
int lro_len_cnt1[16];
//int lro_len_cnt2[16];
int aggregated[MAX_DESC];
int lro_aggregated;
int lro_flushed;
int lro_nodesc;
int force_flush;
int tot_called1;
int tot_called2;
#endif

#if defined(CONFIG_RAETH_HW_LRO)
#define HW_LRO_RING_NUM 3
#define MAX_HW_LRO_AGGR 64
unsigned int hw_lro_agg_num_cnt[HW_LRO_RING_NUM][MAX_HW_LRO_AGGR + 1];
unsigned int hw_lro_agg_size_cnt[HW_LRO_RING_NUM][16];
unsigned int hw_lro_tot_agg_cnt[HW_LRO_RING_NUM];
unsigned int hw_lro_tot_flush_cnt[HW_LRO_RING_NUM];
#if defined(CONFIG_RAETH_HW_LRO_REASON_DBG)
unsigned int hw_lro_agg_flush_cnt[HW_LRO_RING_NUM];
unsigned int hw_lro_age_flush_cnt[HW_LRO_RING_NUM];
unsigned int hw_lro_seq_flush_cnt[HW_LRO_RING_NUM];
unsigned int hw_lro_timestamp_flush_cnt[HW_LRO_RING_NUM];
unsigned int hw_lro_norule_flush_cnt[HW_LRO_RING_NUM];
#endif  /* CONFIG_RAETH_HW_LRO_REASON_DBG */
#endif  /* CONFIG_RAETH_HW_LRO */

#if defined(CONFIG_RAETH_QDMA)
extern unsigned int M2Q_table[64];
extern struct QDMA_txdesc *free_head; 
#endif
#if defined (CONFIG_ARCH_MT8590)
extern struct SFQ_table *sfq0;
extern struct SFQ_table *sfq1;
extern struct SFQ_table *sfq2;
extern struct SFQ_table *sfq3;
#endif

#if defined(CONFIG_USER_SNMPD)
static int ra_snmp_seq_show(struct seq_file *seq, void *v)
{
	seq_printf(seq, "rx counters: %x %x %x %x %x %x %x\n", sysRegRead(GDMA_RX_GBCNT0), sysRegRead(GDMA_RX_GPCNT0),sysRegRead(GDMA_RX_OERCNT0), sysRegRead(GDMA_RX_FERCNT0), sysRegRead(GDMA_RX_SERCNT0), sysRegRead(GDMA_RX_LERCNT0), sysRegRead(GDMA_RX_CERCNT0));

	seq_printf(seq, "fc config: %x %x %x %x\n", sysRegRead(CDMA_FC_CFG), sysRegRead(GDMA1_FC_CFG), PDMA_FC_CFG, sysRegRead(PDMA_FC_CFG));

	seq_printf(seq, "scheduler: %x %x %x\n", sysRegRead(GDMA1_SCH_CFG), sysRegRead(GDMA2_SCH_CFG), sysRegRead(PDMA_SCH_CFG));

	seq_printf(seq, "ports: %x %x %x %x %x %x\n", sysRegRead(PORT0_PKCOUNT), sysRegRead(PORT1_PKCOUNT), sysRegRead(PORT2_PKCOUNT), sysRegRead(PORT3_PKCOUNT), sysRegRead(PORT4_PKCOUNT), sysRegRead(PORT5_PKCOUNT));

	return 0;
}

static int ra_snmp_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_snmp_seq_show, NULL);
}

static const struct file_operations ra_snmp_seq_fops = {
	.owner	 = THIS_MODULE,
	.open	 = ra_snmp_seq_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = single_release
};
#endif

void raeth_stop(END_DEVICE *ei_local)
{
	unsigned int regValue;
	printk("%s()...", __FUNCTION__);

	regValue = sysRegRead(DMA_GLO_CFG);
	regValue &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(DMA_GLO_CFG, regValue);
    	
	printk("Done\n");	
	// printk("Done0x%x...\n", readreg(DMA_GLO_CFG));
}

void ei_irq_clear(void)
{
        sysRegWrite(FE_INT_STATUS, 0xFFFFFFFF);
}

void feEnableInterrupt()
{
	unsigned int regValue = sysRegRead(FE_INT_ENABLE);
	RAETH_PRINT("FE_INT_ENABLE -- : 0x%08x\n", regValue);
		
	sysRegWrite(FE_INT_ENABLE, regValue);
}

void raethMac2AddressSet(unsigned char p[6])
{
	unsigned long regValue;

	regValue = (p[0] << 8) | (p[1]);
	sysRegWrite(GDMA2_MAC_ADRH, regValue);

	regValue = (p[2] << 24) | (p[3] <<16) | (p[4] << 8) | p[5];
	sysRegWrite(GDMA2_MAC_ADRL, regValue);

	printk("GDMA2_MAC_ADRH -- : 0x%08x\n", sysRegRead(GDMA2_MAC_ADRH));
	printk("GDMA2_MAC_ADRL -- : 0x%08x\n", sysRegRead(GDMA2_MAC_ADRL));	    
	return;
}

/**
 * hard_init - Called by raeth_probe to inititialize network device
 * @dev: device pointer
 *
 * ethdev_init initilize dev->priv and set to END_DEVICE structure
 *
 */
void ethtool_init(struct net_device *dev)
{
#if defined (CONFIG_ETHTOOL) /*&& defined (CONFIG_RAETH_ROUTER)*/
	END_DEVICE *ei_local = netdev_priv(dev);

	// init mii structure
	ei_local->mii_info.dev = dev;
	ei_local->mii_info.mdio_read = mdio_read;
	ei_local->mii_info.mdio_write = mdio_write;
	ei_local->mii_info.phy_id_mask = 0x1f;
	ei_local->mii_info.reg_num_mask = 0x1f;
	ei_local->mii_info.supports_gmii = mii_check_gmii_support(&ei_local->mii_info);
	// TODO:   phy_id: 0~4
	ei_local->mii_info.phy_id = 1;
#endif
	return;
}

/*
 *	Routine Name : get_idx(mode, index)
 *	Description: calculate ring usage for tx/rx rings
 *	Mode 1 : Tx Ring 
 *	Mode 2 : Rx Ring
 */
int get_ring_usage(int mode, int i)
{
	unsigned long tx_ctx_idx, tx_dtx_idx, tx_usage;
	unsigned long rx_calc_idx, rx_drx_idx, rx_usage;

	struct PDMA_rxdesc* rxring;
	struct PDMA_txdesc* txring;

	END_DEVICE *ei_local = netdev_priv(dev_raether);


	if (mode == 2 ) {
		/* cpu point to the next descriptor of rx dma ring */
	        rx_calc_idx = *(unsigned long*)RX_CALC_IDX0;
	        rx_drx_idx = *(unsigned long*)RX_DRX_IDX0;
		rxring = (struct PDMA_rxdesc*)RX_BASE_PTR0;
		
		rx_usage = (rx_drx_idx - rx_calc_idx -1 + NUM_RX_DESC) % NUM_RX_DESC;
		if ( rx_calc_idx == rx_drx_idx ) {
		    if ( rxring[rx_drx_idx].rxd_info2.DDONE_bit == 1)
			tx_usage = NUM_RX_DESC;
		    else
			tx_usage = 0;
		}
		return rx_usage;
	}

	
	switch (i) {
		case 0:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX0;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX0;
				txring = ei_local->tx_ring0;
				break;
#if defined(CONFIG_RAETH_QOS)
		case 1:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX1;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX1;
				txring = ei_local->tx_ring1;
				break;
		case 2:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX2;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX2;
				txring = ei_local->tx_ring2;
				break;
		case 3:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX3;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX3;
				txring = ei_local->tx_ring3;
				break;
#endif
		default:
			printk("get_tx_idx failed %d %d\n", mode, i);
			return 0;
	};

	tx_usage = (tx_ctx_idx - tx_dtx_idx + NUM_TX_DESC) % NUM_TX_DESC;
	if ( tx_ctx_idx == tx_dtx_idx ) {
		if ( txring[tx_ctx_idx].txd_info2.DDONE_bit == 1)
			tx_usage = 0;
		else
			tx_usage = NUM_TX_DESC;
	}
	return tx_usage;

}

#if defined(CONFIG_RAETH_QOS)
void dump_qos(struct seq_file *s)
{
	int usage;
	int i;

	seq_printf(s, "\n-----Raeth QOS -----\n\n");

	for ( i = 0; i < 4; i++)  {
		usage = get_ring_usage(1,i);
		seq_printf(s, "Tx Ring%d Usage : %d/%d\n", i, usage, NUM_TX_DESC);
	}

	usage = get_ring_usage(2,0);
	seq_printf(s, "RX Usage : %d/%d\n\n", usage, NUM_RX_DESC);
	seq_printf(s, "GDMA1_FC_CFG(0x%08x)  : 0x%08x\n", GDMA1_FC_CFG, sysRegRead(GDMA1_FC_CFG));
	seq_printf(s, "GDMA2_FC_CFG(0x%08x)  : 0x%08x\n", GDMA2_FC_CFG, sysRegRead(GDMA2_FC_CFG));
	seq_printf(s, "PDMA_FC_CFG(0x%08x)  : 0x%08x\n", PDMA_FC_CFG, sysRegRead(PDMA_FC_CFG));
	seq_printf(s, "PSE_FQ_CFG(0x%08x)  : 0x%08x\n", PSE_FQ_CFG, sysRegRead(PSE_FQ_CFG));

	seq_printf(s, "\n\nTX_CTX_IDX0    : 0x%08x\n", sysRegRead(TX_CTX_IDX0));	
	seq_printf(s, "TX_DTX_IDX0    : 0x%08x\n", sysRegRead(TX_DTX_IDX0));
	seq_printf(s, "TX_CTX_IDX1    : 0x%08x\n", sysRegRead(TX_CTX_IDX1));	
	seq_printf(s, "TX_DTX_IDX1    : 0x%08x\n", sysRegRead(TX_DTX_IDX1));
	seq_printf(s, "TX_CTX_IDX2    : 0x%08x\n", sysRegRead(TX_CTX_IDX2));	
	seq_printf(s, "TX_DTX_IDX2    : 0x%08x\n", sysRegRead(TX_DTX_IDX2));
	seq_printf(s, "TX_CTX_IDX3    : 0x%08x\n", sysRegRead(TX_CTX_IDX3));
	seq_printf(s, "TX_DTX_IDX3    : 0x%08x\n", sysRegRead(TX_DTX_IDX3));
	seq_printf(s, "RX_CALC_IDX0   : 0x%08x\n", sysRegRead(RX_CALC_IDX0));
	seq_printf(s, "RX_DRX_IDX0    : 0x%08x\n", sysRegRead(RX_DRX_IDX0));

	seq_printf(s, "\n------------------------------\n\n");
}
#endif

void dump_reg(struct seq_file *s)
{
	int fe_int_enable;
	int rx_usage;
	int dly_int_cfg;
	int rx_base_ptr0;
	int rx_max_cnt0;
	int rx_calc_idx0;
	int rx_drx_idx0;
#if !defined (CONFIG_RAETH_QDMA)
	int tx_usage;
	int tx_base_ptr[4];
	int tx_max_cnt[4];
	int tx_ctx_idx[4];
	int tx_dtx_idx[4];
	int i;
#endif

	fe_int_enable = sysRegRead(FE_INT_ENABLE);
    rx_usage = get_ring_usage(2,0);

	dly_int_cfg = sysRegRead(DLY_INT_CFG);
	
#if !defined (CONFIG_RAETH_QDMA)
	tx_usage = get_ring_usage(1,0);

	tx_base_ptr[0] = sysRegRead(TX_BASE_PTR0);
	tx_max_cnt[0] = sysRegRead(TX_MAX_CNT0);
	tx_ctx_idx[0] = sysRegRead(TX_CTX_IDX0);
	tx_dtx_idx[0] = sysRegRead(TX_DTX_IDX0);
	
	tx_base_ptr[1] = sysRegRead(TX_BASE_PTR1);
	tx_max_cnt[1] = sysRegRead(TX_MAX_CNT1);
	tx_ctx_idx[1] = sysRegRead(TX_CTX_IDX1);
	tx_dtx_idx[1] = sysRegRead(TX_DTX_IDX1);

	tx_base_ptr[2] = sysRegRead(TX_BASE_PTR2);
	tx_max_cnt[2] = sysRegRead(TX_MAX_CNT2);
	tx_ctx_idx[2] = sysRegRead(TX_CTX_IDX2);
	tx_dtx_idx[2] = sysRegRead(TX_DTX_IDX2);
	
	tx_base_ptr[3] = sysRegRead(TX_BASE_PTR3);
	tx_max_cnt[3] = sysRegRead(TX_MAX_CNT3);
	tx_ctx_idx[3] = sysRegRead(TX_CTX_IDX3);
	tx_dtx_idx[3] = sysRegRead(TX_DTX_IDX3);
#endif

	rx_base_ptr0 = sysRegRead(RX_BASE_PTR0);
	rx_max_cnt0 = sysRegRead(RX_MAX_CNT0);
	rx_calc_idx0 = sysRegRead(RX_CALC_IDX0);
	rx_drx_idx0 = sysRegRead(RX_DRX_IDX0);

	seq_printf(s, "\n\nFE_INT_ENABLE:\t0x%08x\n", fe_int_enable);
	seq_printf(s, "FE_INT_STATUS:\t0x%08x\n", sysRegRead(FE_INT_STATUS));
#if !defined (CONFIG_RAETH_QDMA)
	seq_printf(s, "TxRing PktCnt:\t%d/%d\n", tx_usage, NUM_TX_DESC);
#endif
	seq_printf(s, "RxRing PktCnt:\t%d/%d\n\n", rx_usage, NUM_RX_DESC);
	seq_printf(s, "DLY_INT_CFG:\t0x%08x\n", dly_int_cfg);

#if !defined (CONFIG_RAETH_QDMA)	
	for(i=0;i<4;i++) {
		seq_printf(s, "TX_BASE_PTR%d:\t0x%08x\n", i, tx_base_ptr[i]);	
		seq_printf(s, "TX_MAX_CNT%d:\t0x%08x\n", i, tx_max_cnt[i]);	
		seq_printf(s, "TX_CTX_IDX%d:\t0x%08x\n", i, tx_ctx_idx[i]);
		seq_printf(s, "TX_DTX_IDX%d:\t0x%08x\n", i, tx_dtx_idx[i]);
	}
#endif

	seq_printf(s, "RX_BASE_PTR0:\t0x%08x\n", rx_base_ptr0);	
	seq_printf(s, "RX_MAX_CNT0:\t0x%08x\n", rx_max_cnt0);	
	seq_printf(s, "RX_CALC_IDX0:\t0x%08x\n", rx_calc_idx0);
	seq_printf(s, "RX_DRX_IDX0:\t0x%08x\n", rx_drx_idx0);
	
#if defined (CONFIG_ETHTOOL) && defined (CONFIG_RAETH_ROUTER)
	seq_printf(s, "The current PHY address selected by ethtool is %d\n", get_current_phy_address());
#endif

}

struct proc_dir_entry *procRegDir;

#if defined(CONFIG_RAETH_HW_LRO) || defined (CONFIG_RAETH_MULTIPLE_RX_RING)
const static HWLRO_DBG_FUNC hw_lro_dbg_func[] =
{
    [0] = hwlro_agg_cnt_ctrl,
    [1] = hwlro_agg_time_ctrl,
    [2] = hwlro_age_time_ctrl,
    [3] = hwlro_pkt_int_alpha_ctrl,
    [4] = hwlro_threshold_ctrl,
    [5] = hwlro_fix_setting_switch_ctrl,
};
#endif  /* CONFIG_RAETH_HW_LRO */

#if defined(CONFIG_RAETH_PDMA_DVT)
const static PDMA_DBG_FUNC pdma_dvt_dbg_func[] =
{
    [0] = pdma_dvt_show_ctrl,
    [1] = pdma_dvt_test_rx_ctrl,
    [2] = pdma_dvt_test_tx_ctrl,
    [3] = pdma_dvt_test_debug_ctrl,
    [4] = pdma_dvt_test_lro_ctrl,
};
#endif  // #if defined(CONFIG_RAETH_PDMA_DVT)

int RegReadMain(struct seq_file *seq, void *v)
{
	dump_reg(seq);
	return 0;
}

static void *seq_SkbFree_start(struct seq_file *seq, loff_t *pos)
{
	if (*pos < NUM_TX_DESC)
		return pos;
	return NULL;
}

static void *seq_SkbFree_next(struct seq_file *seq, void *v, loff_t *pos)
{
	(*pos)++;
	if (*pos >= NUM_TX_DESC)
		return NULL;
	return pos;
}

static void seq_SkbFree_stop(struct seq_file *seq, void *v)
{
	/* Nothing to do */
}

static int seq_SkbFree_show(struct seq_file *seq, void *v)
{
	int i = *(loff_t *) v;
	END_DEVICE *ei_local = netdev_priv(dev_raether);

	seq_printf(seq, "%d: %08x\n",i,  *(int *)&ei_local->skb_free[i]);

	return 0;
}

static const struct seq_operations seq_skb_free_ops = {
	.start = seq_SkbFree_start,
	.next  = seq_SkbFree_next,
	.stop  = seq_SkbFree_stop,
	.show  = seq_SkbFree_show
};

static int skb_free_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &seq_skb_free_ops);
}

static const struct file_operations skb_free_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= skb_free_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= seq_release
};

#if defined (CONFIG_RAETH_QDMA)
int QDMARead(struct seq_file *seq, void *v)
{
	unsigned int temp,i;
	unsigned int sw_fq, hw_fq;
	unsigned int min_en, min_rate, max_en, max_rate, sch, weight;
	unsigned int queue, tx_des_cnt, hw_resv, sw_resv, queue_head, queue_tail;
	struct net_device *dev = dev_raether;
        END_DEVICE *ei_local = netdev_priv(dev);

	seq_printf(seq, "==== General Information ====\n");
	temp = sysRegRead(QDMA_FQ_CNT);
	sw_fq = (temp&0xFFFF0000)>>16;
	hw_fq = (temp&0x0000FFFF);
	seq_printf(seq, "SW TXD: %d/%d; HW TXD: %d/%d\n", sw_fq, NUM_TX_DESC, hw_fq,NUM_QDMA_PAGE);
	seq_printf(seq, "SW TXD virtual start address: 0x%08x\n", ei_local->txd_pool);
	seq_printf(seq, "HW TXD virtual start address: 0x%08x\n\n", free_head);

	seq_printf(seq, "==== Scheduler Information ====\n");
	temp = sysRegRead(QDMA_TX_SCH);
	max_en = (temp&0x00000800)>>11;
	max_rate = (temp&0x000007F0)>>4;
	for(i=0;i<(temp&0x0000000F);i++)
		max_rate *= 10;
	seq_printf(seq, "SCH1 rate control:%d. Rate is %dKbps.\n", max_en, max_rate);
	max_en = (temp&0x08000000)>>27;
	max_rate = (temp&0x07F00000)>>20;
	for(i=0;i<(temp&0x000F0000);i++)
		max_rate *= 10;
	seq_printf(seq, "SCH2 rate control:%d. Rate is %dKbps.\n\n", max_en, max_rate);

	seq_printf(seq, "==== Physical Queue Information ====\n");
	for (queue = 0; queue < 16; queue++){
		temp = sysRegRead(QTX_CFG_0 + 0x10 * queue);
		tx_des_cnt = (temp & 0xffff0000) >> 16;
		hw_resv = (temp & 0xff00) >> 8;
		sw_resv = (temp & 0xff);
		temp = sysRegRead(QTX_CFG_0 +(0x10 * queue) + 0x4);
		sch = (temp >> 31) + 1 ;
		min_en = (temp & 0x8000000) >> 27;
		min_rate = (temp & 0x7f00000) >> 20;
		for (i = 0; i< (temp & 0xf0000) >> 16; i++)
			min_rate *= 10;
		max_en = (temp & 0x800) >> 11;
		max_rate = (temp & 0x7f0) >> 4;
		for (i = 0; i< (temp & 0xf); i++)
			max_rate *= 10;
		weight = (temp & 0xf000) >> 12;
		queue_head = sysRegRead(QTX_HEAD_0 + 0x10 * queue);
		queue_tail = sysRegRead(QTX_TAIL_0 + 0x10 * queue);

		seq_printf(seq, "Queue#%d Information:\n", queue);
		seq_printf(seq, "%d packets in the queue; head address is 0x%08x, tail address is 0x%08x.\n", tx_des_cnt, queue_head, queue_tail);
		seq_printf(seq, "HW_RESV: %d; SW_RESV: %d; SCH: %d; Weighting: %d\n", hw_resv, sw_resv, sch, weight);
		seq_printf(seq, "Min_Rate_En is %d, Min_Rate is %dKbps; Max_Rate_En is %d, Max_Rate is %dKbps.\n\n", min_en, min_rate, max_en, max_rate);
	}
#if defined (CONFIG_ARCH_MT8590)
	seq_printf(seq, "==== Virtual Queue Information ====\n");
	seq_printf(seq, "VQTX_TB_BASE_0:0x%08x;VQTX_TB_BASE_1:0x%08x;VQTX_TB_BASE_2:0x%08x;VQTX_TB_BASE_3:0x%08x\n", \
			sfq0, sfq1, sfq2, sfq3);
	temp = sysRegRead(VQTX_NUM);
	seq_printf(seq, "VQTX_NUM_0:0x%01x;VQTX_NUM_1:0x%01x;VQTX_NUM_2:0x%01x;VQTX_NUM_3:0x%01x\n\n", \
			temp&0xF, (temp&0xF0)>>4, (temp&0xF00)>>8, (temp&0xF000)>>12);

#endif

	seq_printf(seq, "==== Flow Control Information ====\n");
	temp = sysRegRead(QDMA_FC_THRES);
	seq_printf(seq, "SW_DROP_EN:%x; SW_DROP_FFA:%d; SW_DROP_MODE:%d\n", \
			(temp&0x1000000)>>24, (temp&0x200000)>>25, (temp&0x30000000)>>28);
	seq_printf(seq, "WH_DROP_EN:%x; HW_DROP_FFA:%d; HW_DROP_MODE:%d\n", \
			(temp&0x10000)>>16, (temp&0x2000)>>17, (temp&0x300000)>>20);
#if defined (CONFIG_ARCH_MT8590)
	seq_printf(seq, "SW_DROP_FSTVQ_MODE:%d;SW_DROP_FSTVQ:%d\n", \
			(temp&0xC0000000)>>30, (temp&0x08000000)>>27);
	seq_printf(seq, "HW_DROP_FSTVQ_MODE:%d;HW_DROP_FSTVQ:%d\n", \
			(temp&0xC00000)>>22, (temp&0x080000)>>19);
#endif

	seq_printf(seq, "\n==== FSM Information\n");
	temp = sysRegRead(QDMA_DMA);
#if defined (CONFIG_ARCH_MT8590)
	seq_printf(seq, "VQTB_FSM:0x%01x\n", (temp&0x0F000000)>>24);
#endif
	seq_printf(seq, "FQ_FSM:0x%01x\n", (temp&0x000F0000)>>16);
	seq_printf(seq, "TX_FSM:0x%01x\n", (temp&0x00000F00)>>12);
	seq_printf(seq, "RX_FSM:0x%01x\n\n", (temp&0x0000000f));

	seq_printf(seq, "==== M2Q Information ====\n");
	for (i = 0; i < 64; i+=8){
		seq_printf(seq, " (%d,%d)(%d,%d)(%d,%d)(%d,%d)(%d,%d)(%d,%d)(%d,%d)(%d,%d)\n",
				i, M2Q_table[i], i+1, M2Q_table[i+1], i+2, M2Q_table[i+2], i+3, M2Q_table[i+3],
				i+4, M2Q_table[i+4], i+5, M2Q_table[i+5], i+6, M2Q_table[i+6], i+7, M2Q_table[i+7]);
	}

	return 0;

}

static int qdma_open(struct inode *inode, struct file *file)
{
	return single_open(file, QDMARead, NULL);
}

static const struct file_operations qdma_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= qdma_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};
#endif

int TxRingRead(struct seq_file *seq, void *v)
{
	END_DEVICE *ei_local = netdev_priv(dev_raether);
	struct PDMA_txdesc *tx_ring;
	int i = 0;

	tx_ring = kmalloc(sizeof(struct PDMA_txdesc) * NUM_TX_DESC, GFP_KERNEL);
        if(tx_ring==NULL){
		seq_printf(seq, " allocate temp tx_ring fail.\n");
		return 0;
	}

	for (i=0; i < NUM_TX_DESC; i++) {
		tx_ring[i] = ei_local->tx_ring0[i];
        }
	
	for (i=0; i < NUM_TX_DESC; i++) {
#ifdef CONFIG_32B_DESC
		seq_printf(seq, "%d: %08x %08x %08x %08x %08x %08x %08x %08x\n",i,  *(int *)&tx_ring[i].txd_info1, 
				*(int *)&tx_ring[i].txd_info2, *(int *)&tx_ring[i].txd_info3, 
				*(int *)&tx_ring[i].txd_info4, *(int *)&tx_ring[i].txd_info5, 
				*(int *)&tx_ring[i].txd_info6, *(int *)&tx_ring[i].txd_info7,
				*(int *)&tx_ring[i].txd_info8);
#else
		seq_printf(seq, "%d: %08x %08x %08x %08x\n",i,  *(int *)&tx_ring[i].txd_info1, *(int *)&tx_ring[i].txd_info2, 
				*(int *)&tx_ring[i].txd_info3, *(int *)&tx_ring[i].txd_info4);
#endif
	}

	kfree(tx_ring);
	return 0;
}

static int tx_ring_open(struct inode *inode, struct file *file)
{
#if !defined (CONFIG_RAETH_QDMA)
	return single_open(file, TxRingRead, NULL);
#else
	return single_open(file, QDMARead, NULL);
#endif
}

static const struct file_operations tx_ring_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= tx_ring_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};

int RxRingRead(struct seq_file *seq, void *v)
{
	END_DEVICE *ei_local = netdev_priv(dev_raether);
	struct PDMA_rxdesc *rx_ring;
	int i = 0;

	rx_ring = kmalloc(sizeof(struct PDMA_rxdesc) * NUM_RX_DESC, GFP_KERNEL);
	if(rx_ring==NULL){
		seq_printf(seq, " allocate temp rx_ring fail.\n");
		return 0;
	}

	for (i=0; i < NUM_RX_DESC; i++) {
		memcpy(&rx_ring[i], &ei_local->rx_ring0[i], sizeof(struct PDMA_rxdesc));
	}
	
	for (i=0; i < NUM_RX_DESC; i++) {
#ifdef CONFIG_32B_DESC
		seq_printf(seq, "%d: %08x %08x %08x %08x %08x %08x %08x %08x\n",i,  *(int *)&rx_ring[i].rxd_info1,
				*(int *)&rx_ring[i].rxd_info2, *(int *)&rx_ring[i].rxd_info3,
				*(int *)&rx_ring[i].rxd_info4, *(int *)&rx_ring[i].rxd_info5,
				*(int *)&rx_ring[i].rxd_info6, *(int *)&rx_ring[i].rxd_info7,
				*(int *)&rx_ring[i].rxd_info8);
#else
		seq_printf(seq, "%d: %08x %08x %08x %08x\n",i,  *(int *)&rx_ring[i].rxd_info1, *(int *)&rx_ring[i].rxd_info2, 
				*(int *)&rx_ring[i].rxd_info3, *(int *)&rx_ring[i].rxd_info4);
#endif
        }

	kfree(rx_ring);
	return 0;
}

static int rx_ring_open(struct inode *inode, struct file *file)
{
	return single_open(file, RxRingRead, NULL);
}

static const struct file_operations rx_ring_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= rx_ring_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};

#if defined(CONFIG_RAETH_HW_LRO) || defined (CONFIG_RAETH_MULTIPLE_RX_RING)
int RxLRORingRead(struct seq_file *seq, void *v, struct PDMA_rxdesc *rx_ring_p)
{
	struct PDMA_rxdesc *rx_ring;
	int i = 0;

	rx_ring = kmalloc(sizeof(struct PDMA_rxdesc) * NUM_RX_DESC, GFP_KERNEL);
	if(rx_ring==NULL){
		seq_printf(seq, " allocate temp rx_ring fail.\n");
		return 0;
	}

	for (i=0; i < NUM_RX_DESC; i++) {
		memcpy(&rx_ring[i], &rx_ring_p[i], sizeof(struct PDMA_rxdesc));
	}
	
	for (i=0; i < NUM_RX_DESC; i++) {
#ifdef CONFIG_32B_DESC
		seq_printf(seq, "%d: %08x %08x %08x %08x %08x %08x %08x %08x\n",i,  *(int *)&rx_ring[i].rxd_info1,
				*(int *)&rx_ring[i].rxd_info2, *(int *)&rx_ring[i].rxd_info3,
				*(int *)&rx_ring[i].rxd_info4, *(int *)&rx_ring[i].rxd_info5,
				*(int *)&rx_ring[i].rxd_info6, *(int *)&rx_ring[i].rxd_info7,
				*(int *)&rx_ring[i].rxd_info8);
#else
		seq_printf(seq, "%d: %08x %08x %08x %08x\n",i,  *(int *)&rx_ring[i].rxd_info1, *(int *)&rx_ring[i].rxd_info2, 
				*(int *)&rx_ring[i].rxd_info3, *(int *)&rx_ring[i].rxd_info4);
#endif
    }

	kfree(rx_ring);
	return 0;
}

int RxRing1Read(struct seq_file *seq, void *v)
{
	END_DEVICE *ei_local = netdev_priv(dev_raether);
    RxLRORingRead(seq, v, ei_local->rx_ring1);

    return 0;
}

int RxRing2Read(struct seq_file *seq, void *v)
{
	END_DEVICE *ei_local = netdev_priv(dev_raether);
    RxLRORingRead(seq, v, ei_local->rx_ring2);

    return 0;
}

int RxRing3Read(struct seq_file *seq, void *v)
{
	END_DEVICE *ei_local = netdev_priv(dev_raether);
    RxLRORingRead(seq, v, ei_local->rx_ring3);

    return 0;
}

static int rx_ring1_open(struct inode *inode, struct file *file)
{
	return single_open(file, RxRing1Read, NULL);
}

static int rx_ring2_open(struct inode *inode, struct file *file)
{
	return single_open(file, RxRing2Read, NULL);
}

static int rx_ring3_open(struct inode *inode, struct file *file)
{
	return single_open(file, RxRing3Read, NULL);
}

static const struct file_operations rx_ring1_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= rx_ring1_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};

static const struct file_operations rx_ring2_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= rx_ring2_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};

static const struct file_operations rx_ring3_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= rx_ring3_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};
#endif  /* CONFIG_RAETH_HW_LRO */

#if defined(CONFIG_RAETH_TSO)

int NumOfTxdUpdate(int num_of_txd)
{

	txd_cnt[num_of_txd]++;

	return 0;	
}

static void *seq_TsoTxdNum_start(struct seq_file *seq, loff_t *pos)
{
	seq_printf(seq, "TXD | Count\n");
	if (*pos < (MAX_SKB_FRAGS/2 + 1))
		return pos;
	return NULL;
}

static void *seq_TsoTxdNum_next(struct seq_file *seq, void *v, loff_t *pos)
{
	(*pos)++;
	if (*pos >= (MAX_SKB_FRAGS/2 + 1))
		return NULL;
	return pos;
}

static void seq_TsoTxdNum_stop(struct seq_file *seq, void *v)
{
	/* Nothing to do */
}

static int seq_TsoTxdNum_show(struct seq_file *seq, void *v)
{
	int i = *(loff_t *) v;
	seq_printf(seq, "%d: %d\n",i , txd_cnt[i]);

	return 0;
}

ssize_t NumOfTxdWrite(struct file *file, const char __user *buffer, 
		      size_t count, loff_t *data)
{
	memset(txd_cnt, 0, sizeof(txd_cnt));
        printk("clear txd cnt table\n");

	return count;
}

int TsoLenUpdate(int tso_len)
{

	if(tso_len > 70000) {
		tso_cnt[14]++;
	}else if(tso_len >  65000) {
		tso_cnt[13]++;
	}else if(tso_len >  60000) {
		tso_cnt[12]++;
	}else if(tso_len >  55000) {
		tso_cnt[11]++;
	}else if(tso_len >  50000) {
		tso_cnt[10]++;
	}else if(tso_len >  45000) {
		tso_cnt[9]++;
	}else if(tso_len > 40000) {
		tso_cnt[8]++;
	}else if(tso_len > 35000) {
		tso_cnt[7]++;
	}else if(tso_len > 30000) {
		tso_cnt[6]++;
	}else if(tso_len > 25000) {
		tso_cnt[5]++;
	}else if(tso_len > 20000) {
		tso_cnt[4]++;
	}else if(tso_len > 15000) {
		tso_cnt[3]++;
	}else if(tso_len > 10000) {
		tso_cnt[2]++;
	}else if(tso_len > 5000) {
		tso_cnt[1]++;
	}else {
		tso_cnt[0]++;
	}

	return 0;	
}

ssize_t TsoLenWrite(struct file *file, const char __user *buffer,
		    size_t count, loff_t *data)
{
	memset(tso_cnt, 0, sizeof(tso_cnt));
        printk("clear tso cnt table\n");

	return count;
}

static void *seq_TsoLen_start(struct seq_file *seq, loff_t *pos)
{
	seq_printf(seq, " Length  | Count\n");
	if (*pos < 15)
		return pos;
	return NULL;
}

static void *seq_TsoLen_next(struct seq_file *seq, void *v, loff_t *pos)
{
	(*pos)++;
	if (*pos >= 15)
		return NULL;
	return pos;
}

static void seq_TsoLen_stop(struct seq_file *seq, void *v)
{
	/* Nothing to do */
}

static int seq_TsoLen_show(struct seq_file *seq, void *v)
{
	int i = *(loff_t *) v;

	seq_printf(seq, "%d~%d: %d\n", i*5000, (i+1)*5000, tso_cnt[i]);

	return 0;
}

static const struct seq_operations seq_tso_txd_num_ops = {
	.start = seq_TsoTxdNum_start,
	.next  = seq_TsoTxdNum_next,
	.stop  = seq_TsoTxdNum_stop,
	.show  = seq_TsoTxdNum_show
};

static int tso_txd_num_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &seq_tso_txd_num_ops);
}

static struct file_operations tso_txd_num_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= tso_txd_num_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.write		= NumOfTxdWrite,
	.release 	= seq_release
};

static const struct seq_operations seq_tso_len_ops = {
	.start = seq_TsoLen_start,
	.next  = seq_TsoLen_next,
	.stop  = seq_TsoLen_stop,
	.show  = seq_TsoLen_show
};

static int tso_len_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &seq_tso_len_ops);
}

static struct file_operations tso_len_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= tso_len_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.write		= TsoLenWrite,
	.release 	= seq_release
};
#endif

#if defined(CONFIG_RAETH_LRO)
static int LroLenUpdate(struct net_lro_desc *lro_desc)
{
	int len_idx;

	if(lro_desc->ip_tot_len > 65000) {
		len_idx = 13;
	}else if(lro_desc->ip_tot_len > 60000) {
		len_idx = 12;
	}else if(lro_desc->ip_tot_len > 55000) {
		len_idx = 11;
	}else if(lro_desc->ip_tot_len > 50000) {
		len_idx = 10;
	}else if(lro_desc->ip_tot_len > 45000) {
		len_idx = 9;
	}else if(lro_desc->ip_tot_len > 40000) {
		len_idx = 8;
	}else if(lro_desc->ip_tot_len > 35000) {
		len_idx = 7;
	}else if(lro_desc->ip_tot_len > 30000) {
		len_idx = 6;
	}else if(lro_desc->ip_tot_len > 25000) {
		len_idx = 5;
	}else if(lro_desc->ip_tot_len > 20000) {
		len_idx = 4;
	}else if(lro_desc->ip_tot_len > 15000) {
		len_idx = 3;
	}else if(lro_desc->ip_tot_len > 10000) {
		len_idx = 2;
	}else if(lro_desc->ip_tot_len > 5000) {
		len_idx = 1;
	}else {
		len_idx = 0;
	}

	return len_idx;
}
int LroStatsUpdate(struct net_lro_mgr *lro_mgr, bool all_flushed)
{
	struct net_lro_desc *tmp;
	int len_idx;
	int i, j; 
	
	if (all_flushed) {
		for (i=0; i< MAX_DESC; i++) {
			tmp = & lro_mgr->lro_arr[i];
			if (tmp->pkt_aggr_cnt !=0) {
				for(j=0; j<=MAX_AGGR; j++) {
					if(tmp->pkt_aggr_cnt == j) {
						lro_flush_cnt[j]++;
					}
				}
				len_idx = LroLenUpdate(tmp);
			       	lro_len_cnt1[len_idx]++;
				tot_called1++;
			}
			aggregated[i] = 0;
		}
	} else {
		if (lro_flushed != lro_mgr->stats.flushed) {
			if (lro_aggregated != lro_mgr->stats.aggregated) {
				for (i=0; i<MAX_DESC; i++) {
					tmp = &lro_mgr->lro_arr[i];
					if ((aggregated[i]!= tmp->pkt_aggr_cnt) 
							&& (tmp->pkt_aggr_cnt == 0)) {
						aggregated[i] ++;
						for (j=0; j<=MAX_AGGR; j++) {
							if (aggregated[i] == j) {
								lro_stats_cnt[j] ++;
							}
						}
						aggregated[i] = 0;
						//len_idx = LroLenUpdate(tmp);
			       			//lro_len_cnt2[len_idx]++;
						tot_called2++;
					}
				}
			} else {
				for (i=0; i<MAX_DESC; i++) {
					tmp = &lro_mgr->lro_arr[i];
					if ((aggregated[i] != 0) && (tmp->pkt_aggr_cnt==0)) {
						for (j=0; j<=MAX_AGGR; j++) {
							if (aggregated[i] == j) {
								lro_stats_cnt[j] ++;
							}
						}
						aggregated[i] = 0;
						//len_idx = LroLenUpdate(tmp);
			       			//lro_len_cnt2[len_idx]++;
						force_flush ++;
						tot_called2++;
					}
				}
			}
		} else {
			if (lro_aggregated != lro_mgr->stats.aggregated) {
				for (i=0; i<MAX_DESC; i++) {
					tmp = &lro_mgr->lro_arr[i];
					if (tmp->active) {
						if (aggregated[i] != tmp->pkt_aggr_cnt)
							aggregated[i] = tmp->pkt_aggr_cnt;
					} else
						aggregated[i] = 0;
				}
			} 
		}

	}

	lro_aggregated = lro_mgr->stats.aggregated;
	lro_flushed = lro_mgr->stats.flushed;
	lro_nodesc = lro_mgr->stats.no_desc;

	return 0;
		
}


ssize_t LroStatsWrite(struct file *file, const char __user *buffer, 
		      size_t count, loff_t *data)
{
	memset(lro_stats_cnt, 0, sizeof(lro_stats_cnt));
	memset(lro_flush_cnt, 0, sizeof(lro_flush_cnt));
	memset(lro_len_cnt1, 0, sizeof(lro_len_cnt1));
	//memset(lro_len_cnt2, 0, sizeof(lro_len_cnt2));
	memset(aggregated, 0, sizeof(aggregated));
	lro_aggregated = 0;
	lro_flushed = 0;
	lro_nodesc = 0;
	force_flush = 0;
	tot_called1 = 0;
	tot_called2 = 0;
        printk("clear lro  cnt table\n");

	return count;
}

int LroStatsRead(struct seq_file *seq, void *v)
{
	int i;
	int tot_cnt=0;
	int tot_aggr=0;
	int ave_aggr=0;
	
	seq_printf(seq, "LRO statistic dump:\n");
	seq_printf(seq, "Cnt:   Kernel | Driver\n");
	for(i=0; i<=MAX_AGGR; i++) {
		tot_cnt = tot_cnt + lro_stats_cnt[i] + lro_flush_cnt[i];
		seq_printf(seq, " %d :      %d        %d\n", i, lro_stats_cnt[i], lro_flush_cnt[i]);
		tot_aggr = tot_aggr + i * (lro_stats_cnt[i] + lro_flush_cnt[i]);
	}
	ave_aggr = lro_aggregated/lro_flushed;
	seq_printf(seq, "Total aggregated pkt: %d\n", lro_aggregated);
	seq_printf(seq, "Flushed pkt: %d  %d\n", lro_flushed, force_flush);
	seq_printf(seq, "Average flush cnt:  %d\n", ave_aggr);
	seq_printf(seq, "No descriptor pkt: %d\n\n\n", lro_nodesc);

	seq_printf(seq, "Driver flush pkt len:\n");
	seq_printf(seq, " Length  | Count\n");
	for(i=0; i<15; i++) {
		seq_printf(seq, "%d~%d: %d\n", i*5000, (i+1)*5000, lro_len_cnt1[i]);
	}
	seq_printf(seq, "Kernel flush: %d;  Driver flush: %d\n", tot_called2, tot_called1);
	return 0;
}

static int lro_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, LroStatsRead, NULL);
}

static struct file_operations lro_stats_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= lro_stats_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.write		= LroStatsWrite,
	.release 	= single_release
};
#endif

int getnext(const char *src, int separator, char *dest)
{
    char *c;
    int len;

    if ( (src == NULL) || (dest == NULL) ) {
        return -1;
    }

    c = strchr(src, separator);
    if (c == NULL) {
        strcpy(dest, src);
        return -1;
    }
    len = c - src;
    strncpy(dest, src, len);
    dest[len] = '\0';
    return len + 1;
}

int str_to_ip(unsigned int *ip, const char *str)
{
    int len;
    const char *ptr = str;
    char buf[128];
    unsigned char c[4];
    int i;

    for (i = 0; i < 3; ++i) {
        if ((len = getnext(ptr, '.', buf)) == -1) {
            return 1; /* parse error */
        }
        c[i] = simple_strtoul(buf, NULL, 10);
        ptr += len;
    }
    c[3] = simple_strtoul(ptr, NULL, 0);
    *ip = (c[0]<<24) + (c[1]<<16) + (c[2]<<8) + c[3];
    return 0;
}

#if defined(CONFIG_RAETH_HW_LRO)
static int HwLroLenUpdate(unsigned int agg_size)
{
	int len_idx;

	if(agg_size > 65000) {
		len_idx = 13;
	}else if(agg_size > 60000) {
		len_idx = 12;
	}else if(agg_size > 55000) {
		len_idx = 11;
	}else if(agg_size > 50000) {
		len_idx = 10;
	}else if(agg_size > 45000) {
		len_idx = 9;
	}else if(agg_size > 40000) {
		len_idx = 8;
	}else if(agg_size > 35000) {
		len_idx = 7;
	}else if(agg_size > 30000) {
		len_idx = 6;
	}else if(agg_size > 25000) {
		len_idx = 5;
	}else if(agg_size > 20000) {
		len_idx = 4;
	}else if(agg_size > 15000) {
		len_idx = 3;
	}else if(agg_size > 10000) {
		len_idx = 2;
	}else if(agg_size > 5000) {
		len_idx = 1;
	}else {
		len_idx = 0;
	}

	return len_idx;
}

int HwLroStatsUpdate(unsigned int ring_num, unsigned int agg_cnt, unsigned int agg_size)
{
    if( (ring_num > 0) && (ring_num < 4) )
    {
        hw_lro_agg_size_cnt[ring_num-1][HwLroLenUpdate(agg_size)]++;
        hw_lro_agg_num_cnt[ring_num-1][agg_cnt]++;
        hw_lro_tot_flush_cnt[ring_num-1]++;
        hw_lro_tot_agg_cnt[ring_num-1] += agg_cnt;
    }

    return 0;
}

#if defined(CONFIG_RAETH_HW_LRO_REASON_DBG)
int HwLroFlushStatsUpdate(unsigned int ring_num, unsigned int flush_reason)
{
    if( (ring_num > 0) && (ring_num < 4) )
    {
#if 1
        if ( (flush_reason & 0x7) == HW_LRO_AGG_FLUSH )
            hw_lro_agg_flush_cnt[ring_num-1]++;
        else if ( (flush_reason & 0x7) == HW_LRO_AGE_FLUSH )
            hw_lro_age_flush_cnt[ring_num-1]++;
        else if ( (flush_reason & 0x7) == HW_LRO_NOT_IN_SEQ_FLUSH )
            hw_lro_seq_flush_cnt[ring_num-1]++;
        else if ( (flush_reason & 0x7) == HW_LRO_TIMESTAMP_FLUSH )
            hw_lro_timestamp_flush_cnt[ring_num-1]++;
        else if ( (flush_reason & 0x7) == HW_LRO_NON_RULE_FLUSH )
            hw_lro_norule_flush_cnt[ring_num-1]++;
#else
        if ( flush_reason & BIT(4) )
            hw_lro_agg_flush_cnt[ring_num-1]++;
        else if ( flush_reason & BIT(3) )
            hw_lro_age_flush_cnt[ring_num-1]++;
        else if ( flush_reason & BIT(2) )
            hw_lro_seq_flush_cnt[ring_num-1]++;
        else if ( flush_reason & BIT(1) )
            hw_lro_timestamp_flush_cnt[ring_num-1]++;
        else if ( flush_reason & BIT(0) )
            hw_lro_norule_flush_cnt[ring_num-1]++;
#endif
    }

    return 0;
}
#endif  /* CONFIG_RAETH_HW_LRO_REASON_DBG */

ssize_t HwLroStatsWrite(struct file *file, const char __user *buffer, 
		      size_t count, loff_t *data)
{
    memset(hw_lro_agg_num_cnt, 0, sizeof(hw_lro_agg_num_cnt));
    memset(hw_lro_agg_size_cnt, 0, sizeof(hw_lro_agg_size_cnt));
    memset(hw_lro_tot_agg_cnt, 0, sizeof(hw_lro_tot_agg_cnt));
    memset(hw_lro_tot_flush_cnt, 0, sizeof(hw_lro_tot_flush_cnt));
#if defined(CONFIG_RAETH_HW_LRO_REASON_DBG)
    memset(hw_lro_agg_flush_cnt, 0, sizeof(hw_lro_agg_flush_cnt));
    memset(hw_lro_age_flush_cnt, 0, sizeof(hw_lro_age_flush_cnt));
    memset(hw_lro_seq_flush_cnt, 0, sizeof(hw_lro_seq_flush_cnt));
    memset(hw_lro_timestamp_flush_cnt, 0, sizeof(hw_lro_timestamp_flush_cnt));
    memset(hw_lro_norule_flush_cnt, 0, sizeof(hw_lro_norule_flush_cnt));
#endif  /* CONFIG_RAETH_HW_LRO_REASON_DBG */

    printk("clear hw lro cnt table\n");

	return count;
}

int HwLroStatsRead(struct seq_file *seq, void *v)
{
	int i;
	
	seq_printf(seq, "HW LRO statistic dump:\n");

    /* Agg number count */
	seq_printf(seq, "Cnt:   RING1 | RING2 | RING3 | Total\n");
	for(i=0; i<=MAX_HW_LRO_AGGR; i++) {
		seq_printf(seq, " %d :      %d        %d        %d        %d\n", 
            i, hw_lro_agg_num_cnt[0][i], hw_lro_agg_num_cnt[1][i], hw_lro_agg_num_cnt[2][i],
            hw_lro_agg_num_cnt[0][i]+hw_lro_agg_num_cnt[1][i]+hw_lro_agg_num_cnt[2][i]);
	}

    /* Total agg count */
    seq_printf(seq, "Total agg:   RING1 | RING2 | RING3 | Total\n");
    seq_printf(seq, "                %d      %d      %d      %d\n", 
        hw_lro_tot_agg_cnt[0], hw_lro_tot_agg_cnt[1], hw_lro_tot_agg_cnt[2],
        hw_lro_tot_agg_cnt[0]+hw_lro_tot_agg_cnt[1]+hw_lro_tot_agg_cnt[2]);

    /* Total flush count */
    seq_printf(seq, "Total flush:   RING1 | RING2 | RING3 | Total\n");
    seq_printf(seq, "                %d      %d      %d      %d\n", 
        hw_lro_tot_flush_cnt[0], hw_lro_tot_flush_cnt[1], hw_lro_tot_flush_cnt[2],
        hw_lro_tot_flush_cnt[0]+hw_lro_tot_flush_cnt[1]+hw_lro_tot_flush_cnt[2]);

    /* Avg agg count */
    seq_printf(seq, "Avg agg:   RING1 | RING2 | RING3 | Total\n");
    seq_printf(seq, "                %d      %d      %d      %d\n", 
        (hw_lro_tot_flush_cnt[0]) ? hw_lro_tot_agg_cnt[0]/hw_lro_tot_flush_cnt[0] : 0,
        (hw_lro_tot_flush_cnt[1]) ? hw_lro_tot_agg_cnt[1]/hw_lro_tot_flush_cnt[1] : 0,
        (hw_lro_tot_flush_cnt[2]) ? hw_lro_tot_agg_cnt[2]/hw_lro_tot_flush_cnt[2] : 0,
        (hw_lro_tot_flush_cnt[0]+hw_lro_tot_flush_cnt[1]+hw_lro_tot_flush_cnt[2]) ? \
        ((hw_lro_tot_agg_cnt[0]+hw_lro_tot_agg_cnt[1]+hw_lro_tot_agg_cnt[2])/(hw_lro_tot_flush_cnt[0]+hw_lro_tot_flush_cnt[1]+hw_lro_tot_flush_cnt[2])) : 0
    );

    /*  Statistics of aggregation size counts */
	seq_printf(seq, "HW LRO flush pkt len:\n");
	seq_printf(seq, " Length  | RING1  | RING2  | RING3  | Total\n");
	for(i=0; i<15; i++) {
		seq_printf(seq, "%d~%d: %d      %d      %d      %d\n", i*5000, (i+1)*5000, 
            hw_lro_agg_size_cnt[0][i], hw_lro_agg_size_cnt[1][i], hw_lro_agg_size_cnt[2][i],
            hw_lro_agg_size_cnt[0][i]+hw_lro_agg_size_cnt[1][i]+hw_lro_agg_size_cnt[2][i]);
	}
#if defined(CONFIG_RAETH_HW_LRO_REASON_DBG)
    seq_printf(seq, "Flush reason:   RING1 | RING2 | RING3 | Total\n");
    seq_printf(seq, "AGG timeout:      %d      %d      %d      %d\n", 
        hw_lro_agg_flush_cnt[0], hw_lro_agg_flush_cnt[1], hw_lro_agg_flush_cnt[2],
        (hw_lro_agg_flush_cnt[0]+hw_lro_agg_flush_cnt[1]+hw_lro_agg_flush_cnt[2])
    );
    seq_printf(seq, "AGE timeout:      %d      %d      %d      %d\n", 
        hw_lro_age_flush_cnt[0], hw_lro_age_flush_cnt[1], hw_lro_age_flush_cnt[2],
        (hw_lro_age_flush_cnt[0]+hw_lro_age_flush_cnt[1]+hw_lro_age_flush_cnt[2])
    );
    seq_printf(seq, "Not in-sequence:  %d      %d      %d      %d\n", 
        hw_lro_seq_flush_cnt[0], hw_lro_seq_flush_cnt[1], hw_lro_seq_flush_cnt[2],
        (hw_lro_seq_flush_cnt[0]+hw_lro_seq_flush_cnt[1]+hw_lro_seq_flush_cnt[2])
    );
    seq_printf(seq, "Timestamp:        %d      %d      %d      %d\n", 
        hw_lro_timestamp_flush_cnt[0], hw_lro_timestamp_flush_cnt[1], hw_lro_timestamp_flush_cnt[2],
        (hw_lro_timestamp_flush_cnt[0]+hw_lro_timestamp_flush_cnt[1]+hw_lro_timestamp_flush_cnt[2])
    );
    seq_printf(seq, "No LRO rule:      %d      %d      %d      %d\n", 
        hw_lro_norule_flush_cnt[0], hw_lro_norule_flush_cnt[1], hw_lro_norule_flush_cnt[2],
        (hw_lro_norule_flush_cnt[0]+hw_lro_norule_flush_cnt[1]+hw_lro_norule_flush_cnt[2])
    );
#endif  /* CONFIG_RAETH_HW_LRO_REASON_DBG */
    
	return 0;
}

static int hw_lro_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, HwLroStatsRead, NULL);
}

static struct file_operations hw_lro_stats_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= hw_lro_stats_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.write		= HwLroStatsWrite,
	.release 	= single_release
};

int hwlro_agg_cnt_ctrl(int par1, int par2)
{
    SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING1, par2);
    SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING2, par2);
    SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING3, par2);
    return 0;
}

int hwlro_agg_time_ctrl(int par1, int par2)
{
    SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING1, par2);
    SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING2, par2);
    SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING3, par2);
    return 0;
}

int hwlro_age_time_ctrl(int par1, int par2)
{
    SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING1, par2);
    SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING2, par2);
    SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING3, par2);
    return 0;
}

int hwlro_pkt_int_alpha_ctrl(int par1, int par2)
{
    END_DEVICE *ei_local = netdev_priv(dev_raether);

    ei_local->hw_lro_alpha = par2;
    printk("[hwlro_pkt_int_alpha_ctrl]ei_local->hw_lro_alpha = %d\n", ei_local->hw_lro_alpha);

    return 0;
}

int hwlro_threshold_ctrl(int par1, int par2)
{
    /* bandwidth threshold setting */
    SET_PDMA_LRO_BW_THRESHOLD(par2);
    return 0;
}

int hwlro_fix_setting_switch_ctrl(int par1, int par2)
{
#if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
    END_DEVICE *ei_local = netdev_priv(dev_raether);

    ei_local->hw_lro_fix_setting = par2;
    printk("[hwlro_pkt_int_alpha_ctrl]ei_local->hw_lro_fix_setting = %d\n", ei_local->hw_lro_fix_setting);
#endif  /* CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG */

    return 0;
}

ssize_t HwLroAutoTlbWrite(struct file *file, const char __user *buffer, 
		      size_t count, loff_t *data)
{
    char buf[32];
    char *pBuf;
    int len = count;
    int x = 0,y = 0;
    char *pToken = NULL;
    char *pDelimiter = " \t";

    printk("[HwLroAutoTlbWrite]write parameter len = %d\n\r", (int)len);
    if(len >= sizeof(buf)){
        printk("input handling fail!\n");
        len = sizeof(buf) - 1;
        return -1;
    }
    
    if(copy_from_user(buf, buffer, len)){
        return -EFAULT;
    }
    buf[len] = '\0';
    printk("[HwLroAutoTlbWrite]write parameter data = %s\n\r", buf);

    pBuf = buf;
    pToken = strsep(&pBuf, pDelimiter);
    x = NULL != pToken ? simple_strtol(pToken, NULL, 16) : 0;

    pToken = strsep(&pBuf, "\t\n ");
    if(pToken != NULL){
        y = NULL != pToken ? simple_strtol(pToken, NULL, 16) : 0;
        printk("y = 0x%08x \n\r", y);
    }

    if ( (sizeof(hw_lro_dbg_func)/sizeof(hw_lro_dbg_func[0]) > x) && NULL != hw_lro_dbg_func[x])
    {
        (*hw_lro_dbg_func[x])(x, y);
    }

	return count;
}

void HwLroAutoTlbDump(struct seq_file *seq, unsigned int index)
{
    int i;
    struct PDMA_LRO_AUTO_TLB_INFO   pdma_lro_auto_tlb;
    unsigned int tlb_info[9];
    unsigned int dw_len, cnt, priority;
    unsigned int entry;

    if( index > 4 )
        index = index - 1;
    entry = (index * 9) + 1;

    /* read valid entries of the auto-learn table */
    sysRegWrite( PDMA_FE_ALT_CF8, entry );

    //seq_printf(seq, "\nEntry = %d\n", entry);
    for(i=0; i<9; i++){
        tlb_info[i] = sysRegRead(PDMA_FE_ALT_SEQ_CFC);
        //seq_printf(seq, "tlb_info[%d] = 0x%x\n", i, tlb_info[i]);
    }
    memcpy(&pdma_lro_auto_tlb, tlb_info, sizeof(struct PDMA_LRO_AUTO_TLB_INFO));

    dw_len = pdma_lro_auto_tlb.auto_tlb_info7.DW_LEN;
    cnt = pdma_lro_auto_tlb.auto_tlb_info6.CNT;

    if ( sysRegRead(ADMA_LRO_CTRL_DW0) & PDMA_LRO_ALT_SCORE_MODE )  /* packet count */
        priority = cnt;
    else    /* byte count */
        priority = dw_len;

    /* dump valid entries of the auto-learn table */
    if( index >= 4 )
        seq_printf(seq, "\n===== TABLE Entry: %d (Act) =====\n", index);
    else
        seq_printf(seq, "\n===== TABLE Entry: %d (LRU) =====\n", index);
    if( pdma_lro_auto_tlb.auto_tlb_info8.IPV4 ){
        seq_printf(seq, "SIP = 0x%x:0x%x:0x%x:0x%x (IPv4)\n", 
            pdma_lro_auto_tlb.auto_tlb_info4.SIP3,
            pdma_lro_auto_tlb.auto_tlb_info3.SIP2,
            pdma_lro_auto_tlb.auto_tlb_info2.SIP1,
            pdma_lro_auto_tlb.auto_tlb_info1.SIP0);
    }
    else{        
        seq_printf(seq, "SIP = 0x%x:0x%x:0x%x:0x%x (IPv6)\n", 
            pdma_lro_auto_tlb.auto_tlb_info4.SIP3,
            pdma_lro_auto_tlb.auto_tlb_info3.SIP2,
            pdma_lro_auto_tlb.auto_tlb_info2.SIP1,
            pdma_lro_auto_tlb.auto_tlb_info1.SIP0);
    }
    seq_printf(seq, "DIP_ID = %d\n", pdma_lro_auto_tlb.auto_tlb_info8.DIP_ID);
    seq_printf(seq, "TCP SPORT = %d | TCP DPORT = %d\n", 
        pdma_lro_auto_tlb.auto_tlb_info0.STP, 
        pdma_lro_auto_tlb.auto_tlb_info0.DTP);
    seq_printf(seq, "VLAN1 = %d | VLAN2 = %d | VLAN3 = %d | VLAN4 =%d \n", 
        pdma_lro_auto_tlb.auto_tlb_info5.VLAN_VID0,
        (pdma_lro_auto_tlb.auto_tlb_info5.VLAN_VID0 << 12),
        (pdma_lro_auto_tlb.auto_tlb_info5.VLAN_VID0 << 24),
        pdma_lro_auto_tlb.auto_tlb_info6.VLAN_VID1);
    seq_printf(seq, "TPUT = %d | FREQ = %d\n", dw_len, cnt);
    seq_printf(seq, "PRIORITY = %d\n", priority);
}

int HwLroAutoTlbRead(struct seq_file *seq, void *v)
{
	int i;
    unsigned int regVal;
#if defined(CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
    unsigned int regOp1, regOp2, regOp3, regOp4;
    unsigned int agg_cnt, agg_time, age_time;
#endif  /* CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG */

    /* Read valid entries of the auto-learn table */
    sysRegWrite(PDMA_FE_ALT_CF8, 0);
    regVal = sysRegRead(PDMA_FE_ALT_SEQ_CFC);

    seq_printf(seq, "HW LRO Auto-learn Table: (PDMA_LRO_ALT_CFC_RSEQ_DBG=0x%x)\n", regVal);

    for(i = 7; i >= 0; i--)
    {
        if( regVal & (1 << i) )
            HwLroAutoTlbDump(seq, i);
    }
    
#if defined(CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
    /* Read the agg_time/age_time/agg_cnt of LRO rings */
    seq_printf(seq, "\nHW LRO Ring Settings\n");
    for(i = 1; i <= 3; i++) 
    {
        regOp1 = sysRegRead( LRO_RX_RING0_CTRL_DW1 + (i * 0x40) );
        regOp2 = sysRegRead( LRO_RX_RING0_CTRL_DW2 + (i * 0x40) );
        regOp3 = sysRegRead( LRO_RX_RING0_CTRL_DW3 + (i * 0x40) );
        regOp4 = sysRegRead( ADMA_LRO_CTRL_DW2 );
        agg_cnt = ((regOp3 & 0x03) << PDMA_LRO_AGG_CNT_H_OFFSET) | ((regOp2 >> PDMA_LRO_RING_AGG_CNT1_OFFSET) & 0x3f);
        agg_time = (regOp2 >> PDMA_LRO_RING_AGG_OFFSET) & 0xffff;
        age_time = ((regOp2 & 0x03f) << PDMA_LRO_AGE_H_OFFSET) | ((regOp1 >> PDMA_LRO_RING_AGE1_OFFSET) & 0x3ff);
        seq_printf(seq, "Ring[%d]: MAX_AGG_CNT=%d, AGG_TIME=%d, AGE_TIME=%d, Threshold=%d\n", 
            i, agg_cnt, agg_time, age_time, regOp4);
    }
#endif  /* CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG */

	return 0;
}

static int hw_lro_auto_tlb_open(struct inode *inode, struct file *file)
{
	return single_open(file, HwLroAutoTlbRead, NULL);
}

static struct file_operations hw_lro_auto_tlb_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= hw_lro_auto_tlb_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.write		= HwLroAutoTlbWrite,
	.release 	= single_release
};
#endif  /* CONFIG_RAETH_HW_LRO */

#if defined (CONFIG_MIPS)
int CP0RegRead(struct seq_file *seq, void *v)
{
	seq_printf(seq, "CP0 Register dump --\n");
	seq_printf(seq, "CP0_INDEX\t: 0x%08x\n", read_32bit_cp0_register(CP0_INDEX));
	seq_printf(seq, "CP0_RANDOM\t: 0x%08x\n", read_32bit_cp0_register(CP0_RANDOM));
	seq_printf(seq, "CP0_ENTRYLO0\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYLO0));
	seq_printf(seq, "CP0_ENTRYLO1\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYLO1));
	seq_printf(seq, "CP0_CONF\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONF));
	seq_printf(seq, "CP0_CONTEXT\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONTEXT));
	seq_printf(seq, "CP0_PAGEMASK\t: 0x%08x\n", read_32bit_cp0_register(CP0_PAGEMASK));
	seq_printf(seq, "CP0_WIRED\t: 0x%08x\n", read_32bit_cp0_register(CP0_WIRED));
	seq_printf(seq, "CP0_INFO\t: 0x%08x\n", read_32bit_cp0_register(CP0_INFO));
	seq_printf(seq, "CP0_BADVADDR\t: 0x%08x\n", read_32bit_cp0_register(CP0_BADVADDR));
	seq_printf(seq, "CP0_COUNT\t: 0x%08x\n", read_32bit_cp0_register(CP0_COUNT));
	seq_printf(seq, "CP0_ENTRYHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYHI));
	seq_printf(seq, "CP0_COMPARE\t: 0x%08x\n", read_32bit_cp0_register(CP0_COMPARE));
	seq_printf(seq, "CP0_STATUS\t: 0x%08x\n", read_32bit_cp0_register(CP0_STATUS));
	seq_printf(seq, "CP0_CAUSE\t: 0x%08x\n", read_32bit_cp0_register(CP0_CAUSE));
	seq_printf(seq, "CP0_EPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_EPC));
	seq_printf(seq, "CP0_PRID\t: 0x%08x\n", read_32bit_cp0_register(CP0_PRID));
	seq_printf(seq, "CP0_CONFIG\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONFIG));
	seq_printf(seq, "CP0_LLADDR\t: 0x%08x\n", read_32bit_cp0_register(CP0_LLADDR));
	seq_printf(seq, "CP0_WATCHLO\t: 0x%08x\n", read_32bit_cp0_register(CP0_WATCHLO));
	seq_printf(seq, "CP0_WATCHHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_WATCHHI));
	seq_printf(seq, "CP0_XCONTEXT\t: 0x%08x\n", read_32bit_cp0_register(CP0_XCONTEXT));
	seq_printf(seq, "CP0_FRAMEMASK\t: 0x%08x\n", read_32bit_cp0_register(CP0_FRAMEMASK));
	seq_printf(seq, "CP0_DIAGNOSTIC\t: 0x%08x\n", read_32bit_cp0_register(CP0_DIAGNOSTIC));
	seq_printf(seq, "CP0_DEBUG\t: 0x%08x\n", read_32bit_cp0_register(CP0_DEBUG));
	seq_printf(seq, "CP0_DEPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_DEPC));
	seq_printf(seq, "CP0_PERFORMANCE\t: 0x%08x\n", read_32bit_cp0_register(CP0_PERFORMANCE));
	seq_printf(seq, "CP0_ECC\t: 0x%08x\n", read_32bit_cp0_register(CP0_ECC));
	seq_printf(seq, "CP0_CACHEERR\t: 0x%08x\n", read_32bit_cp0_register(CP0_CACHEERR));
	seq_printf(seq, "CP0_TAGLO\t: 0x%08x\n", read_32bit_cp0_register(CP0_TAGLO));
	seq_printf(seq, "CP0_TAGHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_TAGHI));
	seq_printf(seq, "CP0_ERROREPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_ERROREPC));
	seq_printf(seq, "CP0_DESAVE\t: 0x%08x\n\n", read_32bit_cp0_register(CP0_DESAVE));

	return 0;
}

static int cp0_reg_open(struct inode *inode, struct file *file)
{
	return single_open(file, CP0RegRead, NULL);
}

static const struct file_operations cp0_reg_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= cp0_reg_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};
#endif

#if defined(CONFIG_RAETH_QOS)
static struct proc_dir_entry *procRaQOS, *procRaFeIntr, *procRaEswIntr;
extern uint32_t num_of_rxdone_intr;
extern uint32_t num_of_esw_intr;

int RaQOSRegRead(struct seq_file *seq, void *v)
{
	dump_qos(seq);
	return 0;
}

static int raeth_qos_open(struct inode *inode, struct file *file)
{
	return single_open(file, RaQOSRegRead, NULL);
}

static const struct file_operations raeth_qos_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= raeth_qos_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};
#endif

int EswCntRead(struct seq_file *seq, void *v)
{
	seq_printf(seq, "\n		  <<CPU>>			 \n");
	seq_printf(seq, "		    |				 \n");

	seq_printf(seq, "+-----------------------------------------------+\n");
	seq_printf(seq, "|		  <<PSE>>		        |\n");
	seq_printf(seq, "+-----------------------------------------------+\n");
	seq_printf(seq, "		   |				 \n");
	seq_printf(seq, "+-----------------------------------------------+\n");
	seq_printf(seq, "|		  <<GDMA>>		        |\n");
	seq_printf(seq, "| GDMA1_RX_GBCNT  : %010u (Rx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2400));	
	seq_printf(seq, "| GDMA1_RX_GPCNT  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2408));	
	seq_printf(seq, "| GDMA1_RX_VDPCNT : %010u (VID Drop Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x240C));	
	seq_printf(seq, "| GDMA1_RX_OERCNT : %010u (overflow error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2410));	
	seq_printf(seq, "| GDMA1_RX_FERCNT : %010u (FCS error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2414));	
	seq_printf(seq, "| GDMA1_RX_SERCNT : %010u (too short)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2418));	
	seq_printf(seq, "| GDMA1_RX_LERCNT : %010u (too long)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x241C));	
	seq_printf(seq, "| GDMA1_RX_CERCNT : %010u (checksum error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2420));	
	seq_printf(seq, "| GDMA1_RX_FCCNT  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2424));	
	seq_printf(seq, "| GDMA1_TX_SKIPCNT: %010u (about count)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2428));	
	seq_printf(seq, "| GDMA1_TX_COLCNT : %010u (collision count)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x242C));	
	seq_printf(seq, "| GDMA1_TX_GBCNT  : %010u (Tx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2430));	
	seq_printf(seq, "| GDMA1_TX_GPCNT  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2438));	
	seq_printf(seq, "| GDMA1_RX_ARLCNT : %010u (ARL Hash Table Drop Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x243C));	
	seq_printf(seq, "|						|\n");
	seq_printf(seq, "| GDMA2_RX_GBCNT  : %010u (Rx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2440));	
	seq_printf(seq, "| GDMA2_RX_GPCNT  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2448));	
	seq_printf(seq, "| GDMA2_RX_VDPCNT : %010u (VID Drop Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x244C));	
	seq_printf(seq, "| GDMA2_RX_OERCNT : %010u (overflow error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2450));	
	seq_printf(seq, "| GDMA2_RX_FERCNT : %010u (FCS error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2454));	
	seq_printf(seq, "| GDMA2_RX_SERCNT : %010u (too short)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2458));	
	seq_printf(seq, "| GDMA2_RX_LERCNT : %010u (too long)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x245C));	
	seq_printf(seq, "| GDMA2_RX_CERCNT : %010u (checksum error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2460));	
	seq_printf(seq, "| GDMA2_RX_FCCNT  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2464));	
	seq_printf(seq, "| GDMA2_TX_SKIPCNT: %010u (skip)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2468));	
	seq_printf(seq, "| GDMA2_TX_COLCNT : %010u (collision)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x246C));	
	seq_printf(seq, "| GDMA2_TX_GBCNT  : %010u (Tx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2470));	
	seq_printf(seq, "| GDMA2_TX_GPCNT  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2478));
	seq_printf(seq, "| GDMA2_RX_ARLCNT : %010u (ARL Hash Table Drop Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x247C));	
	seq_printf(seq, "+-----------------------------------------------+\n");
	seq_printf(seq, "\n");

	return 0;
}

static int switch_count_open(struct inode *inode, struct file *file)
{
	return single_open(file, EswCntRead, NULL);
}

static const struct file_operations switch_count_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= switch_count_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};

#if defined (CONFIG_ETHTOOL) /*&& defined (CONFIG_RAETH_ROUTER)*/
/*
 * proc write procedure
 */
static ssize_t change_phyid(struct file *file, const char __user *buffer, 
			    size_t count, loff_t *data)
{
	char buf[32];
	struct net_device *cur_dev_p;
	END_DEVICE *ei_local;
	char if_name[64];
	unsigned int phy_id;

	if (count > 32)
		count = 32;
	memset(buf, 0, 32);
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	/* determine interface name */
    strcpy(if_name, DEV_NAME);	/* "eth2" by default */
    if(isalpha(buf[0]))
		sscanf(buf, "%s %d", if_name, &phy_id);
	else
		phy_id = simple_strtol(buf, 0, 10);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	cur_dev_p = dev_get_by_name(&init_net, DEV_NAME);
#else
	cur_dev_p = dev_get_by_name(DEV_NAME);
#endif
	if (cur_dev_p == NULL)
		return -EFAULT;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	ei_local = netdev_priv(cur_dev_p);
#else
	ei_local = cur_dev_p->priv;
#endif	
	ei_local->mii_info.phy_id = (unsigned char)phy_id;
	return count;
}

#endif

static int gmac_open(struct inode *inode, struct file *file)
{
	return single_open(file, RegReadMain, NULL);
}

static struct file_operations gmac_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= gmac_open,
	.read	 	= seq_read,
	.llseek	 	= seq_lseek,
#if defined (CONFIG_ETHTOOL)
	.write		= change_phyid,
#endif
	.release 	= single_release
};

#if defined (TASKLET_WORKQUEUE_SW)
extern int init_schedule;
extern int working_schedule;
static int ScheduleRead(struct seq_file *seq, void *v)
{
	if (init_schedule == 1)
		seq_printf(seq, "Initialize Raeth with workqueque<%d>\n", init_schedule);
	else
		seq_printf(seq, "Initialize Raeth with tasklet<%d>\n", init_schedule);
	if (working_schedule == 1)
		seq_printf(seq, "Raeth is running at workqueque<%d>\n", working_schedule);
	else
		seq_printf(seq, "Raeth is running at tasklet<%d>\n", working_schedule);

	return 0;
}

static ssize_t ScheduleWrite(struct file *file, const char __user *buffer, 
		      size_t count, loff_t *data)
{
	char buf[2];
	int old;
	
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;
	old = init_schedule;
	init_schedule = simple_strtol(buf, 0, 10);
	printk("Change Raeth initial schedule from <%d> to <%d>\n! Not running schedule at present !\n", 
		old, init_schedule);

	return count;
}

static int schedule_switch_open(struct inode *inode, struct file *file)
{
	return single_open(file, ScheduleRead, NULL);
}

static const struct file_operations schedule_sw_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= schedule_switch_open,
	.read	 	= seq_read,
	.write 		= ScheduleWrite,
	.llseek	 	= seq_lseek,
	.release 	= single_release
};
#endif

#if defined(CONFIG_RAETH_PDMA_DVT)
static int PdmaDvtRead(struct seq_file *seq, void *v)
{
    seq_printf(seq, "g_pdma_dvt_show_config = 0x%x\n", pdma_dvt_get_show_config());
    seq_printf(seq, "g_pdma_dvt_rx_test_config = 0x%x\n", pdma_dvt_get_rx_test_config());
    seq_printf(seq, "g_pdma_dvt_tx_test_config = 0x%x\n", pdma_dvt_get_tx_test_config());
    
	return 0;
}

static int PdmaDvtOpen(struct inode *inode, struct file *file)
{
	return single_open(file, PdmaDvtRead, NULL);
}

static ssize_t PdmaDvtWrite(struct file *file, const char __user *buffer, 
		      size_t count, loff_t *data)
{
	char buf[32];
    char *pBuf;
    int len = count;
    int x = 0,y = 0;
    char *pToken = NULL;
    char *pDelimiter = " \t";

    printk("write parameter len = %d\n\r", (int)len);
    if(len >= sizeof(buf)){
        printk("input handling fail!\n");
        len = sizeof(buf) - 1;
        return -1;
    }
    
    if(copy_from_user(buf, buffer, len)){
        return -EFAULT;
    }
    buf[len] = '\0';
    printk("write parameter data = %s\n\r", buf);

    pBuf = buf;
    pToken = strsep(&pBuf, pDelimiter);
    x = NULL != pToken ? simple_strtol(pToken, NULL, 16) : 0;

    pToken = strsep(&pBuf, "\t\n ");
    if(pToken != NULL){
        y = NULL != pToken ? simple_strtol(pToken, NULL, 16) : 0;
        printk("y = 0x%08x \n\r", y);
    }

    if ( (sizeof(pdma_dvt_dbg_func)/sizeof(pdma_dvt_dbg_func[0]) > x) && NULL != pdma_dvt_dbg_func[x])
    {
        (*pdma_dvt_dbg_func[x])(x, y);
    }
    else
    {
        printk("no handler defined for command id(0x%08x)\n\r", x);
    }
    
    printk("x(0x%08x), y(0x%08x)\n", x, y);

    return len;
}

static const struct file_operations pdma_dev_sw_fops = {
	.owner 		= THIS_MODULE,
	.open       = PdmaDvtOpen,
	.read	 	= seq_read,
	.write 		= PdmaDvtWrite
};
#endif  //#if defined(CONFIG_RAETH_PDMA_DVT)

static END_DEVICE * get_ei_device(void)
{
	struct net_device *cur_dev_p;
	END_DEVICE *ei_local;
	cur_dev_p = dev_get_by_name(&init_net, DEV_NAME);
	if (cur_dev_p == NULL)
		return -EFAULT;

	ei_local = netdev_priv(cur_dev_p);

	return ei_local;
}

static int get_wol_status(struct seq_file *seq, void *v)
{
	END_DEVICE *ei_local;

	ei_local = get_ei_device();
	if (ei_local < 0) {
		seq_printf(seq, "Get net device error\n");
		return 0;
	}

	seq_printf(seq, "Wake On Lan (WOL) type is (%d)\n", ei_local->wol);

	return 0;
}

static int wol_write(struct file *file, const char __user *buffer,
		      size_t count, loff_t *data)
{
	END_DEVICE *ei_local;
    char *buf;

    buf = kmalloc(count + 1, GFP_KERNEL);
    if(copy_from_user(buf, buffer, count))
            return -EFAULT;

	buf[count] = '\0';

	ei_local = get_ei_device();
	if (ei_local < 0) {
		printk("Get net device error\n");
		return 0;
	}

	ei_local->wol = buf[0] - '0';
	printk("Wake On Lan (WOL) type is (%d)\n", ei_local->wol);

    kfree(buf);
    return count;
}

static int wol_open(struct inode *inode, struct file *file)
{
	return single_open(file, get_wol_status, NULL);
}

static const struct file_operations wol_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= wol_open,
	.read	 	= seq_read,
	.write		= wol_write
};

#ifdef REALTEK_ETH_CTL
static int get_realtek_eth_mdi_status(struct seq_file *seq, void *v)
{
	END_DEVICE *ei_local;
	u32    val  = 0;
	u32    page = 0;
	ei_local = get_ei_device();
	if (ei_local < 0) {
		seq_printf(seq, "Get net device error\n");
		return 0;
	}
	
	if(ei_local->phy_ops == NULL)
	{
		printk("please initialize eth first.\n");
	    return 0;
    }

	mii_mgr_read(ei_local->phy_ops->addr,  31, &page);
	mii_mgr_write(ei_local->phy_ops->addr, 31, 0);//set to page0
	mii_mgr_read(ei_local->phy_ops->addr,  28, &val);
	mii_mgr_write(ei_local->phy_ops->addr, 31, page);//set to saved page

	if(val & 0x0004) {
	    seq_printf(seq, "MDI is auto \n");
	} else {
	    seq_printf(seq, "MDI is (%s)\n", (val & 0x0002)?"FORCE MDI":"FORCE MDIX");
    }
	return 0;
}

static int get_realtek_eth_eee_status(struct seq_file *seq, void *v)
{
	END_DEVICE *ei_local;

	ei_local = get_ei_device();
	if (ei_local < 0) {
		seq_printf(seq, "Get net device error\n");
		return 0;
	}

	seq_printf(seq, "EEE is (%s)\n", ei_local->eeeStatus?"enable":"disable");

	return 0;
}

static int get_realtek_regdump(struct seq_file *seq, void *v)
{
    u32 phy_register = 0;
    u32 phy_address  = 0;
    u32 temp=0;
    u32 save_page=0;
    u32 r31=0;
    int i = 0;

    mii_mgr_read(phy_address,31, &save_page);
    printk("START==========\n");
    mii_mgr_write(phy_address, 31, 0); //set page 0
    mii_mgr_read(phy_address,31, &temp); // verify
    printk("page %d\n",temp);

    /* register 0~6 */
    for(i = 0 ; i < 7 ; i++)
    {
	    mii_mgr_read(phy_address,i, &temp);
	    printk("Register<%02d>: %04X \n",i, temp);
	}
	/* register 13,14 */
	for(i = 13 ; i < 15 ; i++)
	{
	    mii_mgr_read(phy_address,i, &temp);
	    printk("Register<%02d>: %04X \n",i, temp);
	}
	/* register 24 28 30 */
    {
        mii_mgr_read(phy_address,24, &temp);
        printk("Register<%02d>: %04X \n",24, temp);
        mii_mgr_read(phy_address,28, &temp);
        printk("Register<%02d>: %04X \n",28, temp);
        mii_mgr_read(phy_address,30, &temp);
        printk("Register<%02d>: %04X \n",30, temp);
    }

    printk("===============\n");
    mii_mgr_write(phy_address, 31, 4); //set page 4
    mii_mgr_read(phy_address,31, &temp); // verify
    printk("page %d\n",temp);
    /* register 16 21 */
    {
 	   mii_mgr_read(phy_address,16, &temp);
 	   printk("Register<%02d>: %04X \n",16, temp);
 	    mii_mgr_read(phy_address,21, &temp);
 	   printk("Register<%02d>: %04X \n",21, temp);
 	}

    printk("===============\n");
    mii_mgr_write(phy_address, 31, 7); //set page 7
    mii_mgr_read(phy_address,31, &temp); // verify
    printk("page %d\n",temp);
    /* register 16~20 */
	for(i = 16 ; i < 21 ; i++)
	{
	    mii_mgr_read(phy_address,i, &temp);
	    printk("Register<%02d>: %04X \n",i, temp);
	}

	/* register 23 24 */
	{
	     mii_mgr_read(phy_address,23, &temp);
	     printk("Register<%02d>: %04X \n",23, temp);
	     mii_mgr_read(phy_address,24, &temp);
	     printk("Register<%02d>: %04X \n",24, temp);
	}
	
	mii_mgr_write(phy_address, 31, save_page); //set page from save_page
	printk("page %d return\n",save_page);
	printk("============END\n");

	printk("\n");
}

static int realtek_eth_mdi_open(struct inode *inode, struct file *file)
{
	return single_open(file, get_realtek_eth_mdi_status, NULL);
}

static int realtek_eth_eee_open(struct inode *inode, struct file *file)
{
	return single_open(file, get_realtek_eth_eee_status, NULL);
}

static int realtek_regdump_open(struct inode *inode, struct file *file)
{
	return single_open(file, get_realtek_regdump, NULL);
}

static int mdi_write(struct file *file, const char __user *buffer,
		      size_t count, loff_t *data)
{
	END_DEVICE *ei_local;
	u32    newval = 0;
	u32    page   = 0;
	u32    mdi_change = 0;
    char *buf;

    buf = kmalloc(count + 1, GFP_KERNEL);
    if(copy_from_user(buf, buffer, count))
	{
	    if(buf != NULL)
		    kfree(buf);
        return -EFAULT;
	}

	buf[count] = '\0';

	ei_local = get_ei_device();
	if (ei_local < 0) {
		printk("Get net device error\n");
	    if(buf != NULL)
		    kfree(buf);
		return 0;
	}

	mdi_change = buf[0] - '0';
	kfree(buf);

	switch(mdi_change) {
	case 0 :
		newval = ENABLE_AUTO_MDIX_FUNCTION << 2;
		break;
	case 1 :
        newval = (DISABLE_AUTO_MDIX_FUNCTION << 2) | (FORCE_MDI << 1);
		break;
	case 2 :
        newval = (DISABLE_AUTO_MDIX_FUNCTION << 2) | (FORCE_MDIX << 1);
		break;
	default :
		newval = ENABLE_AUTO_MDIX_FUNCTION << 2;
		break;
	}
	
	if(ei_local->phy_ops == NULL)
	{
		printk("please initialize eth first.\n");
	    return;
    }

	mii_mgr_read(ei_local->phy_ops->addr,  31, &page);
	mii_mgr_write(ei_local->phy_ops->addr, 31, 0);//set to page0
    mii_mgr_write(ei_local->phy_ops->addr, 28, newval);
	mii_mgr_write(ei_local->phy_ops->addr, 31, page);//set to saved page

    return count;
}

static void eee_enable (END_DEVICE *ei_local)
{
	u32    page = 0;
	if(ei_local->phy_ops == NULL)
	{
		printk("please initialize eth first.\n");
	    return;
    }
	mii_mgr_read(ei_local->phy_ops->addr,  31, &page);

	mii_mgr_write(ei_local->phy_ops->addr, 31, 0x0004);//set to page4
	mii_mgr_write(ei_local->phy_ops->addr, 16, 0x7377);// EEE_nway_enable

	mii_mgr_write(ei_local->phy_ops->addr, 31, 0x0000);//set to page0
	mii_mgr_write(ei_local->phy_ops->addr, 13, 0x0007);//Set Address mode and MMD Device = 7

	mii_mgr_write(ei_local->phy_ops->addr, 14, 0x003C);//Set Address Value
	
	mii_mgr_write(ei_local->phy_ops->addr, 13, 0x4007);//Set Data mode and MMD Device = 7
	
	mii_mgr_write(ei_local->phy_ops->addr, 14, 0x0002);//turn on 100BASE-TX EEE capability
	
	mii_mgr_write(ei_local->phy_ops->addr,  0, 0x1200);//Restart Auto-Negotiation
	
	mii_mgr_write(ei_local->phy_ops->addr, 31, page);//set to saved page

	ei_local->eeeStatus = 1; /* eee enable */
}

static void eee_disable (END_DEVICE *ei_local)
{
	u32    page = 0;

	if(ei_local->phy_ops == NULL)
	{
		printk("please initialize eth first.\n");
	    return;
    }
	mii_mgr_read(ei_local->phy_ops->addr,  31, &page);

	mii_mgr_write(ei_local->phy_ops->addr, 31, 0x0004);//set to page4
	mii_mgr_write(ei_local->phy_ops->addr, 16, 0x4077);// EEE_nway_disable

	mii_mgr_write(ei_local->phy_ops->addr, 31, 0x0000);//set to page0
	mii_mgr_write(ei_local->phy_ops->addr, 13, 0x0007);//Set Address mode and MMD Device = 7

	mii_mgr_write(ei_local->phy_ops->addr, 14, 0x003C);//Set Address Value
	
	mii_mgr_write(ei_local->phy_ops->addr, 13, 0x4007);//Set Data mode and MMD Device = 7
	
	mii_mgr_write(ei_local->phy_ops->addr, 14, 0x0000);//turn off 100BASE-TX EEE capability
	
	mii_mgr_write(ei_local->phy_ops->addr,  0, 0x1200);//Restart Auto-Negotiation
	
	mii_mgr_write(ei_local->phy_ops->addr, 31, page);//set to saved page
	
	ei_local->eeeStatus = 0; /* eee disable */
}

int eee_contrl (u32 isEEEEnable)
{
	END_DEVICE *ei_local;

	ei_local = get_ei_device();
	if (ei_local < 0) {
		return -1;
	}

    if( 1 == isEEEEnable )
		eee_enable(ei_local);
	else
		eee_disable(ei_local);

	return 0;
}

static int eee_write(struct file *file, const char __user *buffer,
		      size_t count, loff_t *data)
{
	END_DEVICE *ei_local;
	u32    eee_status = 0;
    char *buf;

    buf = kmalloc(count + 1, GFP_KERNEL);
    if(copy_from_user(buf, buffer, count))
	{
	    if(buf != NULL)
		    kfree(buf);
        return -EFAULT;
	}

	buf[count] = '\0';

	ei_local = get_ei_device();
	if (ei_local < 0) {
		printk("Get net device error\n");
	    if(buf != NULL)
		    kfree(buf);
		return 0;
	}

	eee_status = buf[0] - '0';
    kfree(buf);

    eee_contrl(eee_status==1?1:0);
    return count;
}

static const struct file_operations mdi_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= realtek_eth_mdi_open,
	.read	 	= seq_read,
	.write		= mdi_write
};

static const struct file_operations eee_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= realtek_eth_eee_open,
	.read	 	= seq_read,
	.write		= eee_write
};

static const struct file_operations regdump_fops = {
	.owner 		= THIS_MODULE,
	.open	 	= realtek_regdump_open,
	.read	 	= seq_read,
	.write		= NULL 
};
#endif /* REALTEK_ETH_CTL */

static struct proc_file proc_file_list[] = 
{
	{PROCREG_GMAC, &gmac_fops},
	{PROCREG_SKBFREE, &skb_free_fops},
	{PROCREG_TXRING, &tx_ring_fops},
	{PROCREG_RXRING, &rx_ring_fops},
#if defined (CONFIG_RAETH_HW_LRO) || defined (CONFIG_RAETH_MULTIPLE_RX_RING)
	{PROCREG_RXRING1, &rx_ring1_fops},
	{PROCREG_RXRING2, &rx_ring2_fops},
	{PROCREG_RXRING3, &rx_ring3_fops},
#endif
#if defined(CONFIG_RAETH_TSO)
	{PROCREG_NUM_OF_TXD, &tso_txd_num_fops},
	{PROCREG_TSO_LEN, &tso_len_fops},
#endif
#if defined(CONFIG_RAETH_LRO)
	{PROCREG_LRO_STATS, &lro_stats_fops},
#endif
#if defined(CONFIG_RAETH_HW_LRO)
	{PROCREG_HW_LRO_STATS, &hw_lro_stats_fops},
	{PROCREG_HW_LRO_AUTO_TLB, &hw_lro_auto_tlb_fops},
#endif
#if defined(CONFIG_RAETH_QOS)
	{PROCREG_RAQOS, &raeth_qos_fops},
#endif
#if defined(CONFIG_USER_SNMPD)
	{PROCREG_SNMP, &ra_snmp_seq_fops},
#endif
#if defined (TASKLET_WORKQUEUE_SW)
	{PROCREG_SCHE, &schedule_sw_fops},
#endif
#if defined(CONFIG_RAETH_PDMA_DVT)
	{PROCREG_PDMA_DVT, &pdma_dev_sw_fops},
#endif
	{PROCREG_ESW_CNT, &switch_count_fops},
	{"wol", &wol_fops},
#ifdef REALTEK_ETH_CTL
	{"mdi", &mdi_fops},
	{"eee", &eee_fops},
	{"dumpreg", &regdump_fops},
#endif /* REALTEK_ETH_CTL */
};

int debug_proc_init(void)
{
	int i =0;
	
    if (procRegDir == NULL)
		procRegDir = proc_mkdir(PROCREG_DIR, NULL);

    for (i=0; i<ARRAY_SIZE(proc_file_list); i++) {
	    if (!(proc_create(proc_file_list[i].name, 0, procRegDir, proc_file_list[i].fops)))
		    printk("!! FAIL to create %s PROC !!\n", proc_file_list[0].name);
    }

    printk(KERN_ALERT "PROC INIT OK!\n");
    return 0;
}

void debug_proc_exit(void)
{
	int i =0;

    for (i=0; i<ARRAY_SIZE(proc_file_list); i++) {
    	remove_proc_entry(proc_file_list[0].name, procRegDir);
    }

    printk(KERN_ALERT "proc exit\n");
}
EXPORT_SYMBOL(procRegDir);
