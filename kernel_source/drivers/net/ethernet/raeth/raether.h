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
#ifndef RA2882ETHEND_H
#define RA2882ETHEND_H

#include <linux/mii.h>
#include <linux/version.h>	/* check linux version for 2.4 and 2.6 compatibility */
#include <linux/interrupt.h>	/* for "struct tasklet_struct" in linux-3.10.14 */
#if defined (CONFIG_HW_SFQ)
#include <linux/ip.h>  
#include <linux/ipv6.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <asm/rt2880/rt_mmap.h>
#endif

#ifdef WORKQUEUE_BH
#include <linux/workqueue.h>
#endif // WORKQUEUE_BH //
#ifdef CONFIG_RAETH_LRO
#include <linux/inet_lro.h>
#endif
#include <linux/netdevice.h>

#ifdef DSP_VIA_NONCACHEABLE
#define ESRAM_BASE	0xa0800000	/* 0x0080-0000  ~ 0x00807FFF */
#else
#define ESRAM_BASE	0x80800000	/* 0x0080-0000  ~ 0x00807FFF */
#endif

#define RX_RING_BASE	((int)(ESRAM_BASE + 0x7000))
#define TX_RING_BASE	((int)(ESRAM_BASE + 0x7800))


#define NUM_TX_RINGS 	4
#ifdef MEMORY_OPTIMIZATION
#ifdef CONFIG_RAETH_ROUTER
#define NUM_RX_DESC     32 //128
#define NUM_TX_DESC    	32 //128
#elif CONFIG_RT_3052_ESW
#define NUM_RX_DESC     16 //64
#define NUM_TX_DESC     16 //64
#else
#define NUM_RX_DESC     32 //128
#define NUM_TX_DESC     32 //128
#endif
//#define NUM_RX_MAX_PROCESS 32
#define NUM_RX_MAX_PROCESS 32
#else
#if defined (CONFIG_RAETH_ROUTER)
#define NUM_RX_DESC     256
#define NUM_TX_DESC    	256
#elif defined (CONFIG_RT_3052_ESW)
#define NUM_RX_DESC     256
#define NUM_QRX_DESC NUM_RX_DESC
#define NUM_TX_DESC     256
#else
#define NUM_RX_DESC     256
#define NUM_QRX_DESC NUM_RX_DESC
#define NUM_TX_DESC     256
#endif

#define NUM_RX_MAX_PROCESS 16
#endif
#define DRV_NAME        "ra-eth"
#define DEV_NAME        "eth0"

#define GMAC0_OFFSET    0x28 
#define GMAC2_OFFSET    0x22

#define IRQ_ENET0	232

#if defined (CONFIG_RAETH_HW_LRO)
#define	HW_LRO_TIMER_UNIT   1
#define	HW_LRO_REFRESH_TIME 50000
#define	HW_LRO_MAX_AGG_CNT	17
#define	HW_LRO_AGG_DELTA	1
#if defined(CONFIG_RAETH_PDMA_DVT)
#define	MAX_LRO_RX_LENGTH	10240
#else
#define	MAX_LRO_RX_LENGTH	((HW_LRO_MAX_AGG_CNT + 1)*1500)
#endif
#define	HW_LRO_AGG_TIME	500
#define	HW_LRO_AGE_TIME	500
#define	HW_LRO_BW_THRE	        1000
#define	HW_LRO_PKT_INT_ALPHA    100
#endif  /* CONFIG_RAETH_HW_LRO */
#define FE_INT_STATUS_REG (*(volatile unsigned long *)(FE_INT_STATUS))
#define FE_INT_STATUS_CLEAN(reg) (*(volatile unsigned long *)(FE_INT_STATUS)) = reg

#define REALTEK_ETH_CTL
#ifdef REALTEK_ETH_CTL
int eee_contrl (u32 isEEEEnable);
#define DISABLE_AUTO_MDIX_FUNCTION (0)
#define ENABLE_AUTO_MDIX_FUNCTION  (1)
#define FORCE_MDI  (1)
#define FORCE_MDIX (0)
#endif /* REALTEK_ETH_CTL */

//#define RAETH_DEBUG
#ifdef RAETH_DEBUG
#define RAETH_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define RAETH_PRINT(fmt, args...) { }
#endif
enum mii_mode{
	RGMII = 0,
	MII,
	REVERSE_MII,
	RMII,
};

#define RAETH_VERSION	"v3.2"
enum wol_type{
	WOL_NONE = 0,
	MAC_WOL,
	PHY_WOL,
};

typedef struct end_device
{

    unsigned int        tx_cpu_owner_idx0;
    unsigned int        rx_cpu_owner_idx0;
    unsigned int        fe_int_status;
    unsigned int        tx_full; 
    
#if !defined (CONFIG_RAETH_QDMA)
    unsigned int	phy_tx_ring0;
#else
    /* QDMA Tx  PTR */
    struct sk_buff *free_skb[NUM_TX_DESC];
    unsigned int tx_dma_ptr;
    unsigned int tx_cpu_ptr;
    unsigned int free_txd_num;
    unsigned int free_txd_head;
    unsigned int free_txd_tail;	
    struct QDMA_txdesc *txd_pool;
    dma_addr_t phy_txd_pool;
    unsigned int txd_pool_info[NUM_TX_DESC];
    struct QDMA_txdesc *free_head;
    unsigned int phy_free_head;
    unsigned int *free_page_head;
    unsigned int phy_free_page_head;
    struct PDMA_rxdesc *qrx_ring;
    unsigned int phy_qrx_ring;
#endif

    unsigned int	phy_rx_ring0, phy_rx_ring1, phy_rx_ring2, phy_rx_ring3;

    //send signal to user application to notify link status changed
    struct work_struct  kill_sig_wq;

    struct work_struct  reset_task;
#ifdef WORKQUEUE_BH
    struct work_struct  rx_wq;
#else
#if defined (TASKLET_WORKQUEUE_SW)
    struct work_struct  rx_wq;
#endif
    struct              tasklet_struct     rx_tasklet;
    struct              tasklet_struct     tx_tasklet;
#endif // WORKQUEUE_BH //

#if defined(CONFIG_RAETH_QOS)
    struct		sk_buff *	   skb_free[NUM_TX_RINGS][NUM_TX_DESC];
    unsigned int	free_idx[NUM_TX_RINGS];
#else
    struct		sk_buff*	   skb_free[NUM_TX_DESC];
    unsigned int	free_idx;
#endif

    struct              net_device_stats stat;  /* The new statistics table. */
    spinlock_t          page_lock;              /* Page register locks */
    struct PDMA_txdesc *tx_ring0;
#if defined(CONFIG_RAETH_QOS)
    struct PDMA_txdesc *tx_ring1;
    struct PDMA_txdesc *tx_ring2;
    struct PDMA_txdesc *tx_ring3;
#endif
    struct PDMA_rxdesc *rx_ring0;
    struct sk_buff     *netrx0_skbuf[NUM_RX_DESC];
#if defined (CONFIG_RAETH_HW_LRO)
    struct PDMA_rxdesc *rx_ring3;
    struct sk_buff     *netrx3_skbuf[NUM_RX_DESC];
    struct PDMA_rxdesc *rx_ring2;
    struct sk_buff     *netrx2_skbuf[NUM_RX_DESC];
    struct PDMA_rxdesc *rx_ring1;
    struct sk_buff     *netrx1_skbuf[NUM_RX_DESC];
#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING)
    struct PDMA_rxdesc *rx_ring1;
    struct sk_buff     *netrx1_skbuf[NUM_RX_DESC];
    struct PDMA_rxdesc *rx_ring2;
    struct sk_buff     *netrx2_skbuf[NUM_RX_DESC];
    struct PDMA_rxdesc *rx_ring3;
    struct sk_buff     *netrx3_skbuf[NUM_RX_DESC];
#endif
#ifdef CONFIG_RAETH_NAPI
    atomic_t irq_sem;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    struct napi_struct napi;
#endif
#endif
#ifdef CONFIG_PSEUDO_SUPPORT
    struct net_device *PseudoDev;
    unsigned int isPseudo;
#endif
#if defined (CONFIG_ETHTOOL) /*&& defined (CONFIG_RAETH_ROUTER)*/
	struct mii_if_info	mii_info;
#endif
#ifdef CONFIG_RAETH_LRO
    struct lro_counters lro_counters;
    struct net_lro_mgr lro_mgr;
    struct net_lro_desc lro_arr[8];
#endif
#ifdef CONFIG_RAETH_HW_VLAN_RX
    struct vlan_group *vlgrp;
#endif
#if defined (CONFIG_RAETH_HW_LRO)
    struct work_struct hw_lro_wq;
    unsigned int hw_lro_pkt_interval[3];
    unsigned int hw_lro_alpha;  /* 0 < packet interval alpha <= 10 */
    #if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
    unsigned int hw_lro_fix_setting;  /* 0: dynamical AGG/AGE time, 1: fixed AGG/AGE time */
    #endif  /* CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG */
#endif  /* CONFIG_RAETH_HW_LRO */
	struct eth_phy_ops *phy_ops;
	enum wol_type wol;
#ifdef REALTEK_ETH_CTL
	u32 eeeStatus;   /* eee status */
#endif /* REALTEK_ETH_CTL */
	u32 linkUp;             /*link status */
	struct net_device *netdev;
} END_DEVICE, *pEND_DEVICE;

struct eth_phy_ops {
    u32 addr;      /* 0-31 */
    u32 phy_id;  /* value of phy reg3(identifier2) */
    void (*init)(END_DEVICE *ei_local);
    void (*suspend)(END_DEVICE *ei_local);
    void (*resume)(END_DEVICE *ei_local);
	enum mii_mode (*get_mii_type)(END_DEVICE *ei_local);
};

#define RAETH_HTABLE_SIZE		(512)
#define RAETH_HTABLE_SIZE_LIMIT		(RAETH_HTABLE_SIZE >> 1)

void ei_xmit_housekeeping(unsigned long data);

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);
u32 mii_mgr_cl45_set_address(u32 port_num, u32 dev_addr, u32 reg_addr);
u32 mii_mgr_read_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 *read_data);
u32 mii_mgr_write_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 write_data);
void fe_sw_init(void);

#endif
