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
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>
#if defined (CONFIG_RAETH_TSO)
#include <linux/tcp.h>
#include <net/ipv6.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <linux/in.h>
#include <linux/ppp_defs.h>
#include <linux/if_pppox.h>
#endif
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/rt2880/rt_mmap.h>
#include <mach/mt_boot.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_spm.h>
#include <mach/mt_spm_mtcmos.h>
#include <mach/mt_spm_sleep.h>
#include <mach/pmic_mt6323_sw.h>
#include <mach/eint.h>

#include "ra_reg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ioctl.h"

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../net/nat/hw_nat/ra_nat.h"
#endif
#if defined(CONFIG_RAETH_PDMA_DVT)
#include "dvt/raether_pdma_dvt.h"
#endif  /* CONFIG_RAETH_PDMA_DVT */
#include "ra_phy.h"

#if defined (TASKLET_WORKQUEUE_SW)
int init_schedule;
int working_schedule;
#endif

#ifdef CONFIG_RAETH_NAPI
static int raeth_clean(struct napi_struct *napi, int budget);

static int raeth_eth_recv(struct net_device* dev, int *work_done, int work_to_do);
#else
static int raeth_eth_recv(struct net_device* dev);
#endif

#if !defined(CONFIG_RA_NAT_NONE)
extern int (*ra_sw_nat_hook_rx)(struct sk_buff *skb);
extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
#endif

#if defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE)
#include <asm/mipsregs.h>
extern int (*ra_classifier_hook_tx)(struct sk_buff *skb, unsigned long cur_cycle);
extern int (*ra_classifier_hook_rx)(struct sk_buff *skb, unsigned long cur_cycle);
#endif /* CONFIG_RA_CLASSIFIER */

#if defined (CONFIG_RAETH_NAPI) || defined (CONFIG_RAETH_QOS)
#undef DELAY_INT
#else
#define DELAY_INT	1
#endif

#if defined (CONFIG_RAETH_JUMBOFRAME)
#define	MAX_RX_LENGTH	4096
#else
#define	MAX_RX_LENGTH	1536
#endif

struct net_device		*dev_raether;

static int rx_dma_owner_idx; 
static int rx_dma_owner_idx0;
#if defined (CONFIG_RAETH_HW_LRO)
static int rx_dma_owner_lro1;
static int rx_dma_owner_lro2;
static int rx_dma_owner_lro3;
#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING)
static int rx_dma_owner_idx1;
#if defined(CONFIG_ARCH_MT8590)
static int rx_dma_owner_idx2;
static int rx_dma_owner_idx3;
#endif  /* CONFIG_ARCH_MT8590 */
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
int rx_calc_idx1;
#endif
#endif
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
int rx_calc_idx0;
#endif
static int pending_recv;
static struct PDMA_rxdesc	*rx_ring;
unsigned long tx_ring_full=0;

#if defined (CONFIG_ETHTOOL) /*&& defined (CONFIG_RAETH_ROUTER)*/
#include "ra_ethtool.h"
extern struct ethtool_ops	ra_ethtool_ops;
#endif // (CONFIG_ETHTOOL //

unsigned int M2Q_table[64] = {0};
unsigned int lan_wan_separate = 0;

#if defined(CONFIG_HW_SFQ)
unsigned int web_sfq_enable = 0;
#endif

extern int fe_dma_init(struct net_device *dev);
extern int raeth_resume_fe_dma(struct net_device *dev);
extern int ei_start_xmit(struct sk_buff* skb, struct net_device *dev, int gmac_no);
extern void ei_xmit_housekeeping(unsigned long unused);
extern inline int raeth_send(struct net_device* dev, struct sk_buff *skb, int gmac_no);
#if defined (CONFIG_RAETH_HW_LRO)
extern int fe_hw_lro_init(struct net_device *dev);
#endif  /* CONFIG_RAETH_HW_LRO */
static int raeth_open(struct net_device *dev);
static int raeth_close(struct net_device *dev);
void StarLinkStatusChange(struct net_device *dev);
static void raeth_clear_mac2_link_interrupt();
static void raeth_enable_mac2_link_interrupt();
static void raeth_disable_mac2_link_interrupt();


/*
 * Set the hardware MAC address.
 */
static int raeth_set_mac_addr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	if(netif_running(dev))
		return -EBUSY;

        raethMac2AddressSet(addr->sa_data);
	return 0;
}

int raeth_forward_config(struct net_device *dev)
{
	unsigned int	regVal, regCsg;

 	unsigned int	regVal2;
 
#ifdef CONFIG_RAETH_HW_VLAN_TX
	/* 
	 * VLAN_IDX 0 = VLAN_ID 0
	 * .........
	 * VLAN_IDX 15 = VLAN ID 15
	 *
	 */
	/* frame engine will push VLAN tag regarding to VIDX feild in Tx desc. */
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xa8) = 0x00010000;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xac) = 0x00030002;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xb0) = 0x00050004;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xb4) = 0x00070006;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xb8) = 0x00090008;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xbc) = 0x000b000a;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xc0) = 0x000d000c;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xc4) = 0x000f000e;
#endif

	regVal = sysRegRead(GDMA1_FWD_CFG);
	regCsg = sysRegRead(CDMA_CSG_CFG);

 	regVal2 = sysRegRead(GDMA2_FWD_CFG);
 
	//set unicast/multicast/broadcast frame to cpu
	regVal &= ~0xFFFF;
	regVal |= GDMA1_FWD_PORT;
	regCsg &= ~0x7;

#if defined (CONFIG_RAETH_SPECIAL_TAG)
	regVal |= (1 << 24); //GDM1_TCI_81xx
#endif


#ifdef CONFIG_RAETH_HW_VLAN_TX
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	dev->features |= NETIF_F_HW_VLAN_TX;
#else
	dev->features |= NETIF_F_HW_VLAN_CTAG_TX;
#endif
#endif
#ifdef CONFIG_RAETH_HW_VLAN_RX
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	dev->features |= NETIF_F_HW_VLAN_RX;
#else
	dev->features |= NETIF_F_HW_VLAN_CTAG_RX;
#endif
#endif

#ifdef CONFIG_RAETH_CHECKSUM_OFFLOAD
	//enable ipv4 header checksum check
	regVal |= GDM1_ICS_EN;
	regCsg |= ICS_GEN_EN;

	//enable tcp checksum check
	regVal |= GDM1_TCS_EN;
	regCsg |= TCS_GEN_EN;

	//enable udp checksum check
	regVal |= GDM1_UCS_EN;
	regCsg |= UCS_GEN_EN;

 	regVal2 &= ~0xFFFF;
	regVal2 |= GDMA2_FWD_PORT;
  
	regVal2 |= GDM1_ICS_EN;
	regVal2 |= GDM1_TCS_EN;
	regVal2 |= GDM1_UCS_EN;
 
#if defined (CONFIG_RAETH_HW_LRO) 
    dev->features |= NETIF_F_HW_CSUM;
#else
	dev->features |= NETIF_F_IP_CSUM; /* Can checksum TCP/UDP over IPv4 */
#endif  /* CONFIG_RAETH_HW_LRO */

#if defined (CONFIG_RAETH_TSO)
	dev->features |= NETIF_F_SG;
	dev->features |= NETIF_F_TSO;
#endif // CONFIG_RAETH_TSO //

#if defined (CONFIG_RAETH_TSOV6)
	dev->features |= NETIF_F_TSO6;
	dev->features |= NETIF_F_IPV6_CSUM; /* Can checksum TCP/UDP over IPv6 */
#endif // CONFIG_RAETH_TSOV6 //
#else // Checksum offload disabled

	//disable ipv4 header checksum check
	regVal &= ~GDM1_ICS_EN;
	regCsg &= ~ICS_GEN_EN;

	//disable tcp checksum check
	regVal &= ~GDM1_TCS_EN;
	regCsg &= ~TCS_GEN_EN;

	//disable udp checksum check
	regVal &= ~GDM1_UCS_EN;
	regCsg &= ~UCS_GEN_EN;

 	regVal2 &= ~GDM1_ICS_EN;
	regVal2 &= ~GDM1_TCS_EN;
	regVal2 &= ~GDM1_UCS_EN;
 
	dev->features &= ~NETIF_F_IP_CSUM; /* disable checksum TCP/UDP over IPv4 */
#endif // CONFIG_RAETH_CHECKSUM_OFFLOAD //

#ifdef CONFIG_RAETH_JUMBOFRAME
	regVal |= GDM1_JMB_EN;
 	regVal2 |= GDM1_JMB_EN;
 #endif

	sysRegWrite(GDMA1_FWD_CFG, regVal);
	sysRegWrite(CDMA_CSG_CFG, regCsg);
 	sysRegWrite(GDMA2_FWD_CFG, regVal2);
 
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
        dev->vlan_features = dev->features;
#endif

	/*
	 *FE_RST_GLO register definition -
	 *Bit 0: PSE Rest
	 *Reset PSE after re-programming PSE_FQ_CFG.
	 */
	regVal = 0x1;
	sysRegWrite(FE_RST_GL, regVal);
	sysRegWrite(FE_RST_GL, 0);	// update for RSTCTL issue

	regCsg = sysRegRead(CDMA_CSG_CFG);
	printk("CDMA_CSG_CFG = %0X\n",regCsg);
	regVal = sysRegRead(GDMA1_FWD_CFG);
	printk("GDMA1_FWD_CFG = %0X\n",regVal);

 	regVal = sysRegRead(GDMA2_FWD_CFG);
	printk("GDMA2_FWD_CFG = %0X\n",regVal);
	return 1;
}

#ifdef CONFIG_RAETH_NAPI
static int raeth_eth_recv(struct net_device* dev, int *work_done, int work_to_do)
#else
static int raeth_eth_recv(struct net_device* dev)
#endif
{
	struct sk_buff	*skb, *rx_skb;
	unsigned int	length = 0;
	unsigned long	RxProcessed;

	int bReschedule = 0;
	END_DEVICE* 	ei_local = netdev_priv(dev);
#if defined (CONFIG_RAETH_MULTIPLE_RX_RING) || defined (CONFIG_RAETH_HW_LRO)
	int rx_ring_no=0;
#endif

#if defined (CONFIG_RAETH_SPECIAL_TAG)
	struct vlan_ethhdr *veth=NULL;
#endif


	RxProcessed = 0;
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
	rx_dma_owner_idx0 = (rx_calc_idx0 + 1) % NUM_RX_DESC;
#else
	rx_dma_owner_idx0 = (sysRegRead(RAETH_RX_CALC_IDX0) + 1) % NUM_RX_DESC;
#endif

#if defined (CONFIG_32B_DESC)
	dma_cache_sync(NULL, &ei_local->rx_ring0[rx_dma_owner_idx0], sizeof(struct PDMA_rxdesc), DMA_FROM_DEVICE);
#endif
#if defined (CONFIG_RAETH_HW_LRO)
	rx_dma_owner_lro1 = (sysRegRead(RX_CALC_IDX1) + 1) % NUM_RX_DESC;
	rx_dma_owner_lro2 = (sysRegRead(RX_CALC_IDX2) + 1) % NUM_RX_DESC;
	rx_dma_owner_lro3 = (sysRegRead(RX_CALC_IDX3) + 1) % NUM_RX_DESC;
#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING)
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
	rx_dma_owner_idx1 = (rx_calc_idx1 + 1) % NUM_RX_DESC;
#else
	rx_dma_owner_idx1 = (sysRegRead(RX_CALC_IDX1) + 1) % NUM_RX_DESC;
#endif  /* CONFIG_RAETH_RW_PDMAPTR_FROM_VAR */
#if defined(CONFIG_ARCH_MT8590)
    rx_dma_owner_idx2 = (sysRegRead(RX_CALC_IDX2) + 1) % NUM_RX_DESC;
    rx_dma_owner_idx3 = (sysRegRead(RX_CALC_IDX3) + 1) % NUM_RX_DESC;
#endif
#if defined (CONFIG_32B_DESC)
	dma_cache_sync(NULL, &ei_local->rx_ring1[rx_dma_owner_idx1], sizeof(struct PDMA_rxdesc), DMA_FROM_DEVICE);
#endif
#endif
	for ( ; ; ) {


#ifdef CONFIG_RAETH_NAPI
                if(*work_done >= work_to_do)
                        break;
                (*work_done)++;
#else
		if (RxProcessed++ > NUM_RX_MAX_PROCESS)
                {
                        // need to reschedule rx handle
                        bReschedule = 1;
                        break;
                }
#endif


#if defined (CONFIG_RAETH_HW_LRO)
		if (ei_local->rx_ring3[rx_dma_owner_lro3].rxd_info2.DDONE_bit == 1)  {
		    rx_ring = ei_local->rx_ring3;
		    rx_dma_owner_idx = rx_dma_owner_lro3;
		//    printk("rx_dma_owner_lro3=%x\n",rx_dma_owner_lro3);
		    rx_ring_no=3;
		}
		else if (ei_local->rx_ring2[rx_dma_owner_lro2].rxd_info2.DDONE_bit == 1)  {
		    rx_ring = ei_local->rx_ring2;
		    rx_dma_owner_idx = rx_dma_owner_lro2;
		//    printk("rx_dma_owner_lro2=%x\n",rx_dma_owner_lro2);
		    rx_ring_no=2;
		}
		else if (ei_local->rx_ring1[rx_dma_owner_lro1].rxd_info2.DDONE_bit == 1)  {
		    rx_ring = ei_local->rx_ring1;
		    rx_dma_owner_idx = rx_dma_owner_lro1;
		//    printk("rx_dma_owner_lro1=%x\n",rx_dma_owner_lro1);
		    rx_ring_no=1;
		} 
		else if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info2.DDONE_bit == 1)  {
		    rx_ring = ei_local->rx_ring0;
		    rx_dma_owner_idx = rx_dma_owner_idx0;
		 //   printk("rx_dma_owner_idx0=%x\n",rx_dma_owner_idx0);
		    rx_ring_no=0;
		} else {
		    break;
		}
    #if defined (CONFIG_RAETH_HW_LRO_DBG)
        HwLroStatsUpdate(rx_ring_no, rx_ring[rx_dma_owner_idx].rxd_info2.LRO_AGG_CNT, \
            (rx_ring[rx_dma_owner_idx].rxd_info2.PLEN1 << 14) | rx_ring[rx_dma_owner_idx].rxd_info2.PLEN0);
    #endif
    #if defined(CONFIG_RAETH_HW_LRO_REASON_DBG)
        HwLroFlushStatsUpdate(rx_ring_no, rx_ring[rx_dma_owner_idx].rxd_info2.REV);
    #endif
#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING)
		if (ei_local->rx_ring1[rx_dma_owner_idx1].rxd_info2.DDONE_bit == 1)  {
		    rx_ring = ei_local->rx_ring1;
		    rx_dma_owner_idx = rx_dma_owner_idx1;
		//    printk("rx_dma_owner_idx1=%x\n",rx_dma_owner_idx1);
		    rx_ring_no=1;
		}
#if defined(CONFIG_ARCH_MT8590)
        else if (ei_local->rx_ring2[rx_dma_owner_idx2].rxd_info2.DDONE_bit == 1)  {
            rx_ring = ei_local->rx_ring2;
            rx_dma_owner_idx = rx_dma_owner_idx2;
        //    printk("rx_dma_owner_idx2=%x\n",rx_dma_owner_idx2);
            rx_ring_no=2;
        }
        else if (ei_local->rx_ring3[rx_dma_owner_idx3].rxd_info2.DDONE_bit == 1)  {
		    rx_ring = ei_local->rx_ring3;
		    rx_dma_owner_idx = rx_dma_owner_idx3;
		//    printk("rx_dma_owner_idx3=%x\n",rx_dma_owner_idx3);
		    rx_ring_no=3;
		}		
#endif  /* CONFIG_ARCH_MT8590 */
        else if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info2.DDONE_bit == 1)  {
		    rx_ring = ei_local->rx_ring0;
		    rx_dma_owner_idx = rx_dma_owner_idx0;
		 //   printk("rx_dma_owner_idx0=%x\n",rx_dma_owner_idx0);
		    rx_ring_no=0;
		} else {
		    break;
		}
#else

		if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info2.DDONE_bit == 1)  {
		    rx_ring = ei_local->rx_ring0;
		    rx_dma_owner_idx = rx_dma_owner_idx0;
		} else {
		    break;
		}
#endif

#if defined (CONFIG_32B_DESC)
		prefetch(&rx_ring[(rx_dma_owner_idx + 1) % NUM_RX_DESC]);
#endif
		/* skb processing */
#if defined (CONFIG_RAETH_HW_LRO)
        length = (rx_ring[rx_dma_owner_idx].rxd_info2.PLEN1 << 14) | rx_ring[rx_dma_owner_idx].rxd_info2.PLEN0;
#else
		length = rx_ring[rx_dma_owner_idx].rxd_info2.PLEN0;
#endif  /* CONFIG_RAETH_HW_LRO */

		dma_unmap_single(NULL, rx_ring[rx_dma_owner_idx].rxd_info1.PDP0,
			length, DMA_FROM_DEVICE);

#if defined (CONFIG_RAETH_HW_LRO)
		if(rx_ring_no==3) {
		    rx_skb = ei_local->netrx3_skbuf[rx_dma_owner_idx];
		    rx_skb->data = ei_local->netrx3_skbuf[rx_dma_owner_idx]->data;
		}
		else if(rx_ring_no==2) {
		    rx_skb = ei_local->netrx2_skbuf[rx_dma_owner_idx];
		    rx_skb->data = ei_local->netrx2_skbuf[rx_dma_owner_idx]->data;
		}
		else if(rx_ring_no==1) {
		    rx_skb = ei_local->netrx1_skbuf[rx_dma_owner_idx];
		    rx_skb->data = ei_local->netrx1_skbuf[rx_dma_owner_idx]->data;
		} 
		else {
		    rx_skb = ei_local->netrx0_skbuf[rx_dma_owner_idx];
		    rx_skb->data = ei_local->netrx0_skbuf[rx_dma_owner_idx]->data;
		}
    #if defined(CONFIG_RAETH_PDMA_DVT)
        raeth_pdma_lro_dvt( rx_ring_no, ei_local, rx_dma_owner_idx );
    #endif  /* CONFIG_RAETH_PDMA_DVT */
#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING)
		if(rx_ring_no==1) {
		    rx_skb = ei_local->netrx1_skbuf[rx_dma_owner_idx];
		    rx_skb->data = ei_local->netrx1_skbuf[rx_dma_owner_idx]->data;
		} 
#if defined(CONFIG_ARCH_MT8590)
		else if(rx_ring_no==2) {
		    rx_skb = ei_local->netrx2_skbuf[rx_dma_owner_idx];
		    rx_skb->data = ei_local->netrx2_skbuf[rx_dma_owner_idx]->data;
		}
        else if(rx_ring_no==3) {
		    rx_skb = ei_local->netrx3_skbuf[rx_dma_owner_idx];
		    rx_skb->data = ei_local->netrx3_skbuf[rx_dma_owner_idx]->data;
		}
#endif  /* CONFIG_ARCH_MT8590 */
        else {
		    rx_skb = ei_local->netrx0_skbuf[rx_dma_owner_idx];
		    rx_skb->data = ei_local->netrx0_skbuf[rx_dma_owner_idx]->data;
		}
    #if defined(CONFIG_RAETH_PDMA_DVT)
        raeth_pdma_lro_dvt( rx_ring_no, ei_local, rx_dma_owner_idx );
    #endif  /* CONFIG_RAETH_PDMA_DVT */
#else
		rx_skb = ei_local->netrx0_skbuf[rx_dma_owner_idx];
		rx_skb->data = ei_local->netrx0_skbuf[rx_dma_owner_idx]->data;
    #if defined(CONFIG_RAETH_PDMA_DVT)
        raeth_pdma_rx_desc_dvt( ei_local, rx_dma_owner_idx0 );
    #endif  /* CONFIG_RAETH_PDMA_DVT */
#endif
		rx_skb->len 	= length;
/*TODO*/
#if defined (CONFIG_RAETH_SCATTER_GATHER_RX_DMA)
		rx_skb->data += NET_IP_ALIGN;
#endif
		rx_skb->tail 	= rx_skb->data + length;

 		if(rx_ring[rx_dma_owner_idx].rxd_info4.SP == 2) {
 		    rx_skb->dev 	  = dev;
		    rx_skb->protocol	  = eth_type_trans(rx_skb,dev);
		}
 
#ifdef CONFIG_RAETH_CHECKSUM_OFFLOAD
#if defined (CONFIG_PDMA_NEW)
		if(rx_ring[rx_dma_owner_idx].rxd_info4.L4VLD) {
			rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
		}else {
		    rx_skb->ip_summed = CHECKSUM_NONE;
		}
#else
		if(rx_ring[rx_dma_owner_idx].rxd_info4.IPFVLD_bit) {
			rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
		}else { 
		    rx_skb->ip_summed = CHECKSUM_NONE;
		}
#endif
#else
		    rx_skb->ip_summed = CHECKSUM_NONE;
#endif

#if defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE)
		/* Qwert+
		 */
		if(ra_classifier_hook_rx!= NULL)
		{
#if defined(CONFIG_RALINK_EXTERNAL_TIMER)
			ra_classifier_hook_rx(rx_skb, (*((volatile u32 *)(0xB0000D08))&0x0FFFF));
#else			
			ra_classifier_hook_rx(rx_skb, read_c0_count());
#endif			
		}
#endif /* CONFIG_RA_CLASSIFIER */

#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
		if(ra_sw_nat_hook_rx != NULL) {
		    FOE_MAGIC_TAG(rx_skb)= FOE_MAGIC_GE;
		    *(uint32_t *)(FOE_INFO_START_ADDR(rx_skb)+2) = *(uint32_t *)&rx_ring[rx_dma_owner_idx].rxd_info4;
		    FOE_ALG(rx_skb) = 0;
		}
#endif

		/* We have to check the free memory size is big enough
		 * before pass the packet to cpu*/
#if defined (CONFIG_RAETH_SKB_RECYCLE_2K)
#if defined (CONFIG_RAETH_HW_LRO)
            if( rx_ring != ei_local->rx_ring0 )
                skb = __dev_alloc_skb(MAX_LRO_RX_LENGTH + NET_IP_ALIGN, GFP_ATOMIC);
            else
#endif  /* CONFIG_RAETH_HW_LRO */
                skb = skbmgr_dev_alloc_skb2k();
#else
#if defined (CONFIG_RAETH_HW_LRO)
        if( rx_ring != ei_local->rx_ring0 )
            skb = __dev_alloc_skb(MAX_LRO_RX_LENGTH + NET_IP_ALIGN, GFP_ATOMIC);
        else
#endif  /* CONFIG_RAETH_HW_LRO */
    		skb = __dev_alloc_skb(MAX_RX_LENGTH + NET_IP_ALIGN, GFP_ATOMIC);
#endif

		if (unlikely(skb == NULL))
		{
			printk(KERN_ERR "skb not available...\n");
			//if (rx_ring[rx_dma_owner_idx].rxd_info4.SP == 2) //GMAC2
			ei_local->stat.rx_dropped++;
            bReschedule = 1;
			break;
		}
#if !defined (CONFIG_RAETH_SCATTER_GATHER_RX_DMA)
		skb_reserve(skb, NET_IP_ALIGN);
#endif

#if defined (CONFIG_RAETH_SPECIAL_TAG)
		// port0: 0x8100 => 0x8100 0001
		// port1: 0x8101 => 0x8100 0002
		// port2: 0x8102 => 0x8100 0003
		// port3: 0x8103 => 0x8100 0004
		// port4: 0x8104 => 0x8100 0005
		// port5: 0x8105 => 0x8100 0006
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
		veth = (struct vlan_ethhdr *)(rx_skb->mac_header);
#else
		veth = (struct vlan_ethhdr *)(rx_skb->mac.raw);
#endif
		/*donot check 0x81 due to MT7530 SPEC*/
		//if((veth->h_vlan_proto & 0xFF) == 0x81) 
		{
		    veth->h_vlan_TCI = htons( (((veth->h_vlan_proto >> 8) & 0xF) + 1) );
		    rx_skb->protocol = veth->h_vlan_proto = htons(ETH_P_8021Q);
		}
#endif

/* ra_sw_nat_hook_rx return 1 --> continue
 * ra_sw_nat_hook_rx return 0 --> FWD & without netif_rx
 */
#if !defined(CONFIG_RA_NAT_NONE)
         if((ra_sw_nat_hook_rx == NULL) || 
	    (ra_sw_nat_hook_rx!= NULL && ra_sw_nat_hook_rx(rx_skb)))
#endif
         {
#ifdef CONFIG_RAETH_NAPI
                netif_receive_skb(rx_skb);
#else
#ifdef CONFIG_RAETH_HW_VLAN_RX
	        if(ei_local->vlgrp && rx_ring[rx_dma_owner_idx].rxd_info2.TAG) {
			vlan_hwaccel_rx(rx_skb, ei_local->vlgrp, rx_ring[rx_dma_owner_idx].rxd_info3.VID);
		} else {
			netif_rx(rx_skb);
		}
#else
#ifdef CONFIG_RAETH_CPU_LOOPBACK
                skb_push(rx_skb,ETH_HLEN);
                ei_start_xmit(rx_skb, dev, 1);
#else		
                netif_rx(rx_skb);
#endif
#endif
#endif
         }

		ei_local->stat.rx_packets++;
		ei_local->stat.rx_bytes += length;


#if defined (CONFIG_RAETH_SCATTER_GATHER_RX_DMA)
#if defined (CONFIG_RAETH_HW_LRO)
        if( rx_ring != ei_local->rx_ring0 ){
            rx_ring[rx_dma_owner_idx].rxd_info2.PLEN0 = SET_ADMA_RX_LEN0(MAX_LRO_RX_LENGTH);
            rx_ring[rx_dma_owner_idx].rxd_info2.PLEN1 = SET_ADMA_RX_LEN1(MAX_LRO_RX_LENGTH >> 14);
        }
        else
#endif  /* CONFIG_RAETH_HW_LRO */
    		rx_ring[rx_dma_owner_idx].rxd_info2.PLEN0 = MAX_RX_LENGTH;
		rx_ring[rx_dma_owner_idx].rxd_info2.LS0 = 0;
#endif
		rx_ring[rx_dma_owner_idx].rxd_info2.DDONE_bit = 0;
#if defined (CONFIG_RAETH_HW_LRO)
        if( rx_ring != ei_local->rx_ring0 )
            rx_ring[rx_dma_owner_idx].rxd_info1.PDP0 = dma_map_single(NULL, skb->data, MAX_LRO_RX_LENGTH, PCI_DMA_FROMDEVICE);
        else
#endif  /* CONFIG_RAETH_HW_LRO */
		rx_ring[rx_dma_owner_idx].rxd_info1.PDP0 = dma_map_single(NULL, skb->data, MAX_RX_LENGTH, PCI_DMA_FROMDEVICE);
#ifdef CONFIG_32B_DESC
		dma_cache_sync(NULL, &rx_ring[rx_dma_owner_idx], sizeof(struct PDMA_rxdesc), DMA_TO_DEVICE);
#endif
		/*  Move point to next RXD which wants to alloc*/
#if defined (CONFIG_RAETH_HW_LRO)
		if(rx_ring_no==3) {
		    sysRegWrite(RAETH_RX_CALC_IDX3, rx_dma_owner_idx);
		    ei_local->netrx3_skbuf[rx_dma_owner_idx] = skb;
		}
		else if(rx_ring_no==2) {
		    sysRegWrite(RAETH_RX_CALC_IDX2, rx_dma_owner_idx);
		    ei_local->netrx2_skbuf[rx_dma_owner_idx] = skb;
		}
		else if(rx_ring_no==1) {
		    sysRegWrite(RAETH_RX_CALC_IDX1, rx_dma_owner_idx);
		    ei_local->netrx1_skbuf[rx_dma_owner_idx] = skb;
		}
		else if(rx_ring_no==0) {
		    sysRegWrite(RAETH_RX_CALC_IDX0, rx_dma_owner_idx);
		    ei_local->netrx0_skbuf[rx_dma_owner_idx] = skb;
		}
#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING)
		if(rx_ring_no==0) {
		    sysRegWrite(RAETH_RX_CALC_IDX0, rx_dma_owner_idx);
		    ei_local->netrx0_skbuf[rx_dma_owner_idx] = skb;
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
		    rx_calc_idx0 = rx_dma_owner_idx;
#endif
		}
#if defined(CONFIG_ARCH_MT8590)
        else if(rx_ring_no==3) {
		    sysRegWrite(RAETH_RX_CALC_IDX3, rx_dma_owner_idx);
		    ei_local->netrx3_skbuf[rx_dma_owner_idx] = skb;
		}
		else if(rx_ring_no==2) {
		    sysRegWrite(RAETH_RX_CALC_IDX2, rx_dma_owner_idx);
		    ei_local->netrx2_skbuf[rx_dma_owner_idx] = skb;
		}
#endif  /* CONFIG_ARCH_MT8590 */
        else {
		    sysRegWrite(RAETH_RX_CALC_IDX1, rx_dma_owner_idx);
		    ei_local->netrx1_skbuf[rx_dma_owner_idx] = skb;
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
		    rx_calc_idx1 = rx_dma_owner_idx;
#endif
		}
#else
		sysRegWrite(RAETH_RX_CALC_IDX0, rx_dma_owner_idx);
		ei_local->netrx0_skbuf[rx_dma_owner_idx] = skb;
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
		rx_calc_idx0 = rx_dma_owner_idx;
#endif
#endif

		
		/* Update to Next packet point that was received.
		 */
#if defined (CONFIG_RAETH_HW_LRO)
		if(rx_ring_no==3)
			rx_dma_owner_lro3 = (sysRegRead(RAETH_RX_CALC_IDX3) + 1) % NUM_RX_DESC;
		else if(rx_ring_no==2)
			rx_dma_owner_lro2 = (sysRegRead(RAETH_RX_CALC_IDX2) + 1) % NUM_RX_DESC;
		else if(rx_ring_no==1)
			rx_dma_owner_lro1 = (sysRegRead(RAETH_RX_CALC_IDX1) + 1) % NUM_RX_DESC;
		else if(rx_ring_no==0)
			rx_dma_owner_idx0 = (sysRegRead(RAETH_RX_CALC_IDX0) + 1) % NUM_RX_DESC;
		else {
		}
#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING)
		if(rx_ring_no==0) {
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
			rx_dma_owner_idx0 = (rx_dma_owner_idx + 1) % NUM_RX_DESC;
#else
			rx_dma_owner_idx0 = (sysRegRead(RAETH_RX_CALC_IDX0) + 1) % NUM_RX_DESC;
#endif
#if defined(CONFIG_ARCH_MT8590)
        }else if(rx_ring_no==3) {
            rx_dma_owner_idx3 = (sysRegRead(RAETH_RX_CALC_IDX3) + 1) % NUM_RX_DESC;
        }else if(rx_ring_no==2) {
            rx_dma_owner_idx2 = (sysRegRead(RAETH_RX_CALC_IDX2) + 1) % NUM_RX_DESC;
#endif  /* CONFIG_ARCH_MT8590 */
		}else {
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
			rx_dma_owner_idx1 = (rx_dma_owner_idx + 1) % NUM_RX_DESC;
#else
			rx_dma_owner_idx1 = (sysRegRead(RAETH_RX_CALC_IDX1) + 1) % NUM_RX_DESC;
#endif
		}
#else
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
		rx_dma_owner_idx0 = (rx_dma_owner_idx + 1) % NUM_RX_DESC;
#else
		rx_dma_owner_idx0 = (sysRegRead(RAETH_RX_CALC_IDX0) + 1) % NUM_RX_DESC;
#endif
#endif
	}	/* for */

	return bReschedule;
}


/**********************************************************************
*
* raeth_get_stats - gather packet information for management plane
*
*Pass net_device_stats to the upper layer.
*
*
*RETURNS: pointer to net_device_stats
***********************************************************************/

static struct net_device_stats *raeth_get_stats(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	return &ei_local->stat;
}


/**********************************************************************
*
* raeth_receive_workq - process the next incoming packet
* 
* Handle one incoming packet.  The packet is checked for errors and sent
* to the upper layer.
*
* RETURNS: OK on success or ERROR.
***********************************************************************/

#ifndef CONFIG_RAETH_NAPI
#if defined WORKQUEUE_BH || defined (TASKLET_WORKQUEUE_SW)
void raeth_receive_workq(struct work_struct *work)
#else
void ei_receive(unsigned long unused)  // device structure
#endif // WORKQUEUE_BH //
{
	struct net_device *dev = dev_raether;
	END_DEVICE *ei_local = netdev_priv(dev);
	unsigned long reg_int_mask=0;
	int bReschedule=0;


	if(tx_ring_full==0){
		bReschedule = raeth_eth_recv(dev);
		if(bReschedule)
		{
#ifdef WORKQUEUE_BH
			schedule_work(&ei_local->rx_wq);
#else
#if defined (TASKLET_WORKQUEUE_SW)
			if (working_schedule == 1)
				schedule_work(&ei_local->rx_wq);
			else
#endif
			tasklet_hi_schedule(&ei_local->rx_tasklet);
#endif // WORKQUEUE_BH //
		}else{
			reg_int_mask=sysRegRead(RAETH_FE_INT_ENABLE);
#if defined(DELAY_INT)
			sysRegWrite(RAETH_FE_INT_ENABLE, reg_int_mask| RX_DLY_INT);
#else
			sysRegWrite(RAETH_FE_INT_ENABLE, (reg_int_mask | RX_DONE_INT0 | RX_DONE_INT1));
#endif
#ifdef CONFIG_RAETH_QDMA
			reg_int_mask=sysRegRead(QFE_INT_ENABLE);
#if defined(DELAY_INT)
			sysRegWrite(QFE_INT_ENABLE, reg_int_mask| RX_DLY_INT);
#else
			sysRegWrite(QFE_INT_ENABLE, (reg_int_mask | RX_DONE_INT0 | RX_DONE_INT1));
#endif

#endif			
			
		}
	}else{
#ifdef WORKQUEUE_BH
                schedule_work(&ei_local->rx_wq);
#else
#if defined (TASKLET_WORKQUEUE_SW)
		if (working_schedule == 1)
			schedule_work(&ei_local->rx_wq);
		else
#endif
                tasklet_schedule(&ei_local->rx_tasklet);
#endif // WORKQUEUE_BH //
	}
}
#endif

#if defined (CONFIG_RAETH_HW_LRO)
void raeth_hw_lro_auto_adj(unsigned int index, END_DEVICE* ei_local)
{    
    unsigned int entry;
    unsigned int pkt_cnt;
    unsigned int tick_cnt;
    unsigned int duration_us;
    unsigned int byte_cnt;

    /* read packet count statitics of the auto-learn table */
    entry = index  + 68;
    sysRegWrite( PDMA_FE_ALT_CF8, entry );
    pkt_cnt = sysRegRead(PDMA_FE_ALT_SGL_CFC) & 0xfff;
    tick_cnt = (sysRegRead(PDMA_FE_ALT_SGL_CFC) >> 16) & 0xffff;
#if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
    printk("[HW LRO] raeth_hw_lro_auto_adj(): pkt_cnt[%d]=%d, tick_cnt[%d]=%d\n", index, pkt_cnt, index, tick_cnt);
    printk("[HW LRO] raeth_hw_lro_auto_adj(): packet_interval[%d]=%d (ticks/pkt)\n", index, tick_cnt/pkt_cnt);
#endif    

    /* read byte count statitics of the auto-learn table */
    entry = index  + 64;
    sysRegWrite( PDMA_FE_ALT_CF8, entry );
    byte_cnt = sysRegRead(PDMA_FE_ALT_SGL_CFC);
#if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
    printk("[HW LRO] raeth_hw_lro_auto_adj(): byte_cnt[%d]=%d\n", index, byte_cnt);
#endif

    /* calculate the packet interval of the rx flow */
    duration_us = tick_cnt * HW_LRO_TIMER_UNIT;
    ei_local->hw_lro_pkt_interval[index - 1] = (duration_us/pkt_cnt) * ei_local->hw_lro_alpha / 100;
#if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
    printk("[HW LRO] raeth_hw_lro_auto_adj(): packet_interval[%d]=%d (20us)\n", index, duration_us/pkt_cnt);
#endif    

#if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
    if ( !ei_local->hw_lro_fix_setting ){
#endif
    /* adjust age_time, agg_time for the lro ring */
	if(ei_local->hw_lro_pkt_interval[index - 1] > 0){
		SET_PDMA_RXRING_AGE_TIME(index, (ei_local->hw_lro_pkt_interval[index - 1] * HW_LRO_MAX_AGG_CNT));
		SET_PDMA_RXRING_AGG_TIME(index, (ei_local->hw_lro_pkt_interval[index - 1] * HW_LRO_AGG_DELTA));
	}
	else{
		SET_PDMA_RXRING_AGE_TIME(index, HW_LRO_MAX_AGG_CNT);
		SET_PDMA_RXRING_AGG_TIME(index, HW_LRO_AGG_DELTA);
	}
#if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
    }
#endif
}

void raeth_hw_lro_workq(struct work_struct *work)
{
    END_DEVICE *ei_local;
    unsigned int reg_int_val;
    unsigned int reg_int_mask;

    ei_local = container_of(work, struct end_device, hw_lro_wq);

    reg_int_val = sysRegRead(RAETH_FE_INT_STATUS);
#if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
    printk("[HW LRO] raeth_hw_lro_workq(): RAETH_FE_INT_STATUS=0x%x\n", reg_int_val);
#endif
    if((reg_int_val & ALT_RPLC_INT3)){
#if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
        printk("[HW LRO] ALT_RPLC_INT3 occurred!\n");
#endif
        sysRegWrite(RAETH_FE_INT_STATUS, ALT_RPLC_INT3);
        raeth_hw_lro_auto_adj(3, ei_local);
    }
    if((reg_int_val & ALT_RPLC_INT2)){
#if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
        printk("[HW LRO] ALT_RPLC_INT2 occurred!\n");
#endif
        sysRegWrite(RAETH_FE_INT_STATUS, ALT_RPLC_INT2);
        raeth_hw_lro_auto_adj(2, ei_local);
    }
    if((reg_int_val & ALT_RPLC_INT1)){
#if defined (CONFIG_RAETH_HW_LRO_AUTO_ADJ_DBG)
        printk("[HW LRO] ALT_RPLC_INT1 occurred!\n");
#endif
        sysRegWrite(RAETH_FE_INT_STATUS, ALT_RPLC_INT1);
        raeth_hw_lro_auto_adj(1, ei_local);
    }

    /* unmask interrupts of rx flow to hw lor rings */
    reg_int_mask = sysRegRead(RAETH_FE_INT_ENABLE);    
    sysRegWrite(RAETH_FE_INT_ENABLE, reg_int_mask | ALT_RPLC_INT3 | ALT_RPLC_INT2 | ALT_RPLC_INT1);
}
#endif  /* CONFIG_RAETH_HW_LRO */

#ifdef CONFIG_RAETH_NAPI
static int raeth_clean(struct napi_struct *napi, int budget)
{
	struct net_device *netdev=dev_raether;
    int work_to_do = budget;
	END_DEVICE *ei_local =netdev_priv(netdev);
    int work_done = 0;
	unsigned long reg_int_mask=0;

	ei_xmit_housekeeping(0);

	raeth_eth_recv(netdev, &work_done, work_to_do);

        /* if no Tx and not enough Rx work done, exit the polling mode */
    if(( (work_done < work_to_do)) || !netif_running(netdev)) {
		napi_complete(&ei_local->napi);

		atomic_dec_and_test(&ei_local->irq_sem);

		sysRegWrite(RAETH_FE_INT_STATUS, RAETH_FE_INT_ALL);		// ack all fe interrupts
    		reg_int_mask=sysRegRead(RAETH_FE_INT_ENABLE);

#ifdef DELAY_INT
		sysRegWrite(RAETH_FE_INT_ENABLE, reg_int_mask |RAETH_FE_INT_DLY_INIT);  // init delay interrupt only
#else
		sysRegWrite(RAETH_FE_INT_ENABLE,reg_int_mask | RAETH_FE_INT_SETTING);
#endif

#ifdef CONFIG_RAETH_QDMA
		sysRegWrite(QFE_INT_STATUS, QFE_INT_ALL);
		reg_int_mask=sysRegRead(QFE_INT_ENABLE);
#ifdef DELAY_INT
        sysRegWrite(QFE_INT_ENABLE, reg_int_mask |QFE_INT_DLY_INIT);  // init delay interrupt only
#else
        sysRegWrite(QFE_INT_ENABLE,reg_int_mask | (RX_DONE_INT0 | RX_DONE_INT1 | RLS_DONE_INT));
#endif
#endif // CONFIG_RAETH_QDMA //

        return 0;
    }

    return 1;
}

#endif

void StarLinkStatusChange(struct net_device *dev)
{
	/* This function shall be called only when PHY_AUTO_POLL is enabled */
	u32 val = 0;
	u32 speed = 0;

	END_DEVICE *ei_local = netdev_priv(dev);

	val = sysRegRead(MAC2_SR);

	if (ei_local->linkUp != ((val & MAC2_LINK_STATUS)?1UL:0UL)) {
		ei_local->linkUp = (val & MAC2_LINK_STATUS)?1UL:0UL;
		printk("Link status: %s\n", ei_local->linkUp? "Up":"Down");
		if (ei_local->linkUp) {
			speed = ((val >> MAC2_STA_SPD_OFFSET) & MAC2_STA_SPD_MASK);
			printk("%s Duplex - %s Mbps mode\n",
				   (val & MAC2_DPX_STATUS)?"Full":"Half",
				   !speed? "10":(speed==1?"100":(speed==2?"1000":"unknown")));
			printk("TX flow control:%s, RX flow control:%s\n",
					(val & MAC2_STA_TXFC)?"On":"Off",
					(val & MAC2_STA_RXFC)?"On":"Off");
			netif_carrier_on(dev);
		} else {
			netif_carrier_off(dev);
		}
	}
}

/**
 * raeth_interrupt - handle controler interrupt
 *
 * This routine is called at interrupt level in response to an interrupt from
 * the controller.
 *
 * RETURNS: N/A.
 */
static irqreturn_t raeth_interrupt(int irq, void *dev_id)
{
#if !defined(CONFIG_RAETH_NAPI)
	unsigned long reg_int_val;
	unsigned long reg_int_mask=0;
	unsigned int recv = 0;
	unsigned int transmit __maybe_unused = 0;
	unsigned long flags;
	unsigned long reg_int_link_status;
#endif

	struct net_device *dev = (struct net_device *) dev_id;
	END_DEVICE *ei_local = netdev_priv(dev);

	if (dev == NULL)
	{
		printk (KERN_ERR "net_interrupt(): irq %x for unknown device.\n", IRQ_ENET0);
		return IRQ_NONE;
	}

#ifdef CONFIG_RAETH_NAPI
        if (napi_schedule_prep(&ei_local->napi)) {
	        atomic_inc(&ei_local->irq_sem);
			sysRegWrite(RAETH_FE_INT_ENABLE, 0);
#ifdef CONFIG_RAETH_QDMA		
			sysRegWrite(QFE_INT_ENABLE, 0);
#endif
		__napi_schedule(&ei_local->napi);

        }
#else

	spin_lock_irqsave(&(ei_local->page_lock), flags);
	reg_int_val = sysRegRead(RAETH_FE_INT_STATUS);
#ifdef CONFIG_RAETH_QDMA	
	reg_int_val |= sysRegRead(QFE_INT_STATUS);
#endif

#if defined (DELAY_INT)
	if((reg_int_val & RX_DLY_INT))
		recv = 1;
	
	if (reg_int_val & RAETH_TX_DLY_INT)
		transmit = 1;

#if defined(CONFIG_RAETH_PDMA_DVT)
    raeth_pdma_lro_dly_int_dvt();
#endif  /* CONFIG_RAETH_PDMA_DVT */

#else
	if((reg_int_val & RX_DONE_INT0))
		recv = 1;

#if defined (CONFIG_RAETH_HW_LRO) 
	if((reg_int_val & RX_DONE_INT3))
		recv = 3;
	if((reg_int_val & RX_DONE_INT2))
		recv = 2;
	if((reg_int_val & RX_DONE_INT1))
		recv = 1;
#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING) 
#if defined(CONFIG_ARCH_MT8590)    
    if((reg_int_val & RX_DONE_INT3))
        recv = 3;
    if((reg_int_val & RX_DONE_INT2))
        recv = 2;
#endif  /* CONFIG_ARCH_MT8590 */
	if((reg_int_val & RX_DONE_INT1))
		recv = 1;
#endif

	if (reg_int_val & RAETH_TX_DONE_INT0)
		transmit |= RAETH_TX_DONE_INT0;
#if defined (CONFIG_RAETH_QOS)
	if (reg_int_val & TX_DONE_INT1)
		transmit |= TX_DONE_INT1;
	if (reg_int_val & TX_DONE_INT2)
		transmit |= TX_DONE_INT2;
	if (reg_int_val & TX_DONE_INT3)
		transmit |= TX_DONE_INT3;
#endif //CONFIG_RAETH_QOS

#endif //DELAY_INT

#if defined (DELAY_INT)
	sysRegWrite(RAETH_FE_INT_STATUS, RAETH_FE_INT_DLY_INIT);
#else
	sysRegWrite(RAETH_FE_INT_STATUS, RAETH_FE_INT_ALL);
#endif
#ifdef CONFIG_RAETH_QDMA
#if defined (DELAY_INT)
	sysRegWrite(QFE_INT_STATUS, QFE_INT_DLY_INIT);
#else
	sysRegWrite(QFE_INT_STATUS, QFE_INT_ALL);
#endif
#endif	

#if defined (CONFIG_RAETH_HW_LRO)
    if( reg_int_val & (ALT_RPLC_INT3 | ALT_RPLC_INT2 | ALT_RPLC_INT1) ){
        /* mask interrupts of rx flow to hw lor rings */
        reg_int_mask = sysRegRead(RAETH_FE_INT_ENABLE);
        sysRegWrite(RAETH_FE_INT_ENABLE, reg_int_mask & ~(ALT_RPLC_INT3 | ALT_RPLC_INT2 | ALT_RPLC_INT1));
        schedule_work(&ei_local->hw_lro_wq);
    }
#endif  /* CONFIG_RAETH_HW_LRO */

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
	if(transmit)
		ei_xmit_housekeeping(0);
#else
		ei_xmit_housekeeping(0);
#endif

	if (((recv == 1) || (pending_recv ==1)) && (tx_ring_full==0))
	{
		reg_int_mask = sysRegRead(RAETH_FE_INT_ENABLE);
#if defined (DELAY_INT)
		sysRegWrite(RAETH_FE_INT_ENABLE, reg_int_mask & ~(RX_DLY_INT));
#else
		sysRegWrite(RAETH_FE_INT_ENABLE, reg_int_mask & ~(RX_DONE_INT0 | RX_DONE_INT1));
#endif //DELAY_INT
#ifdef CONFIG_RAETH_QDMA		
		reg_int_mask = sysRegRead(QFE_INT_ENABLE);
#if defined (DELAY_INT)
		sysRegWrite(QFE_INT_ENABLE, reg_int_mask & ~(RX_DLY_INT));
#else
		sysRegWrite(QFE_INT_ENABLE, reg_int_mask & ~(RX_DONE_INT0 | RX_DONE_INT1));
#endif //DELAY_INT
#endif

		pending_recv=0;
#ifdef WORKQUEUE_BH
		schedule_work(&ei_local->rx_wq);
#else
#if defined (TASKLET_WORKQUEUE_SW)
		if (working_schedule == 1)
			schedule_work(&ei_local->rx_wq);
		else
#endif
		tasklet_hi_schedule(&ei_local->rx_tasklet);
#endif // WORKQUEUE_BH //
	} 
	else if (recv == 1 && tx_ring_full==1) 
	{
		pending_recv=1;
	}


	reg_int_link_status = sysRegRead(FE_INT_LINK_STATUS);
	if (reg_int_link_status & BIT(25)) /* Port status change */ {
			printk("port status change.\n");
			StarLinkStatusChange(dev);
			raeth_clear_mac2_link_interrupt();
		}


	spin_unlock_irqrestore(&(ei_local->page_lock), flags);
#endif

	return IRQ_HANDLED;
}

static int raeth_start_xmit_fake(struct sk_buff* skb, struct net_device *dev)
{
    if (!(dev->flags & IFF_UP)) {
		dev_kfree_skb_any(skb);
		return 0;
    }

    skb->dev = dev;
    return ei_start_xmit(skb, dev, 2);
}

int raeth_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
#if defined(CONFIG_RAETH_QDMA)
	esw_reg reg;
#endif

	int ret = 0;
	END_DEVICE *ei_local = netdev_priv(dev);
	ra_mii_ioctl_data mii;
	spin_lock_irq(&ei_local->page_lock);
	printk("raeth_ioctl cmd(0x%x)\n", cmd);
	switch (cmd) {
#if defined(CONFIG_RAETH_QDMA)
#define _HQOS_REG(x)	(*((volatile u32 *)(RALINK_FRAME_ENGINE_BASE + QDMA_RELATED + x)))
		case RAETH_QDMA_REG_READ:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_HQOS_MAX) {
				ret = -EINVAL;
				break;
			}
			reg.val = _HQOS_REG(reg.off);
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
		case RAETH_QDMA_REG_WRITE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_HQOS_MAX) {
				ret = -EINVAL;
				break;
			}
			_HQOS_REG(reg.off) = reg.val;
			break;
		case RAETH_QDMA_QUEUE_MAPPING:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
				if((reg.off&0x100) == 0x100){
					lan_wan_separate = 1;
					reg.off &= 0xff;
				}else{
					lan_wan_separate = 0;
				}
			M2Q_table[reg.off] = reg.val;
	  	break;
#if defined(CONFIG_HW_SFQ)
		case RAETH_QDMA_SFQ_WEB_ENABLE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if((reg.val) == 0x1){
				web_sfq_enable = 1;

			}else{
				web_sfq_enable = 0;
			}
		break;
#endif	  	
	  	
#endif	  	
		case RAETH_MII_READ:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_read(mii.phy_id, mii.reg_num, &mii.val_out);
			printk("phy %d, reg %d, val 0x%x\n", mii.phy_id, mii.reg_num, mii.val_out);
			copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
			break;

		case RAETH_MII_WRITE:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			printk("phy %d, reg %d, val 0x%x\n", mii.phy_id, mii.reg_num, mii.val_in);
			mii_mgr_write(mii.phy_id, mii.reg_num, mii.val_in);
			break;
		case RAETH_MII_READ_CL45:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_read_cl45(mii.port_num, mii.dev_addr, mii.reg_addr, &mii.val_out);
			copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
			break;
		case RAETH_MII_WRITE_CL45:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_write_cl45(mii.port_num, mii.dev_addr, mii.reg_addr, mii.val_in);
			break;
		default:
			ret = -EOPNOTSUPP;
			break;

	}

	spin_unlock_irq(&ei_local->page_lock);
	return ret;
}

/*
 * Set new MTU size
 * Change the mtu of Raeth Ethernet Device
 */
static int raeth_change_mtu(struct net_device *dev, int new_mtu)
{
	END_DEVICE *ei_local = netdev_priv(dev);

	if ( ei_local == NULL ) {
		printk(KERN_EMERG "%s: raeth_change_mtu passed a non-existent private pointer from net_dev!\n", dev->name);
		return -ENXIO;
	}

	if ( (new_mtu > 4096) || (new_mtu < 64)) {
		return -EINVAL;
	}

#ifndef CONFIG_RAETH_JUMBOFRAME
	if ( new_mtu > 1500 ) {
		return -EINVAL;
	}
#endif

	dev->mtu = new_mtu;

	return 0;
}

#ifdef CONFIG_RAETH_HW_VLAN_RX
static void raeth_vlan_rx_register(struct net_device *dev, struct vlan_group *grp)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	
	ei_local->vlgrp = grp;

	/* enable HW VLAN RX */
	sysRegWrite(CDMP_EG_CTRL, 1);

}
#endif


/* reset frame engine */
void raeth_fe_reset(void)
{
	u32 val;

	val = sysRegRead(RSTCTRL);

	val = val | RALINK_FE_RST;
	sysRegWrite(RSTCTRL, val);

	val = val & ~(RALINK_FE_RST);

	sysRegWrite(RSTCTRL, val);
}

  
static void raeth_reset_task(struct work_struct *work)
{
	struct net_device *dev = dev_raether;

	raeth_close(dev);
	raeth_open(dev);

	return;
}

void raeth_tx_timeout(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);

	schedule_work(&ei_local->reset_task);
}

static void raeth_setup_statistics(END_DEVICE* ei_local)
{
	ei_local->stat.tx_packets	= 0;
	ei_local->stat.tx_bytes 	= 0;
	ei_local->stat.tx_dropped 	= 0;
	ei_local->stat.tx_errors	= 0;
	ei_local->stat.tx_aborted_errors= 0;
	ei_local->stat.tx_carrier_errors= 0;
	ei_local->stat.tx_fifo_errors	= 0;
	ei_local->stat.tx_heartbeat_errors = 0;
	ei_local->stat.tx_window_errors	= 0;

	ei_local->stat.rx_packets	= 0;
	ei_local->stat.rx_bytes 	= 0;
	ei_local->stat.rx_dropped 	= 0;
	ei_local->stat.rx_errors	= 0;
	ei_local->stat.rx_length_errors = 0;
	ei_local->stat.rx_over_errors	= 0;
	ei_local->stat.rx_crc_errors	= 0;
	ei_local->stat.rx_frame_errors	= 0;
	ei_local->stat.rx_fifo_errors	= 0;
	ei_local->stat.rx_missed_errors	= 0;

	ei_local->stat.collisions	= 0;
#if defined (CONFIG_RAETH_QOS)
	ei_local->tx3_full = 0;
	ei_local->tx2_full = 0;
	ei_local->tx1_full = 0;
	ei_local->tx0_full = 0;
#else
	ei_local->tx_full = 0;
#endif
#ifdef CONFIG_RAETH_NAPI
	atomic_set(&ei_local->irq_sem, 1);
#endif

}

/**
 * raeth_dev_init - pick up ethernet port at boot time
 * @dev: network device to probe
 *
 * This routine probe the ethernet port at boot time.
 *
 *
 */

int __init raeth_dev_init(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	struct sockaddr addr;
	unsigned char mac_addr_def[6] = {0x00, 0x0C, 0x43, 0x28, 0x80, 0x48};

	raeth_fe_reset();

	memcpy(addr.sa_data, mac_addr_def, 6);

#ifdef CONFIG_RAETH_NAPI
	netif_napi_add(dev, &ei_local->napi, raeth_clean, 128);
#endif
	raeth_set_mac_addr(dev, &addr);
	spin_lock_init(&ei_local->page_lock);
	ether_setup(dev);

	raeth_setup_statistics(ei_local);

#if defined (CONFIG_RAETH_HW_LRO) 
	dev->features |= NETIF_F_HW_CSUM;
#else
	dev->features |= NETIF_F_IP_CSUM; /* Can checksum TCP/UDP over IPv4 */
#endif  /* CONFIG_RAETH_HW_LRO */

#if defined (CONFIG_RAETH_TSO)
	dev->features |= NETIF_F_SG;
	dev->features |= NETIF_F_TSO;
#endif // CONFIG_RAETH_TSO //

#if defined (CONFIG_RAETH_TSOV6)
	dev->features |= NETIF_F_TSO6;
	dev->features |= NETIF_F_IPV6_CSUM; /* Can checksum TCP/UDP over IPv6 */
#endif 

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
	dev->vlan_features = dev->features;
#endif

	return 0;
}

static void raeth_enable_int(void)
{
	
#ifdef DELAY_INT
		sysRegWrite(RAETH_DLY_INT_CFG, DELAY_INT_INIT);
		sysRegWrite(RAETH_FE_INT_ENABLE, RAETH_FE_INT_DLY_INIT);
#if defined (CONFIG_RAETH_HW_LRO)
		sysRegWrite(RAETH_FE_INT_ENABLE, RAETH_FE_INT_DLY_INIT | ALT_RPLC_INT3 | ALT_RPLC_INT2 | ALT_RPLC_INT1);
#endif  /* CONFIG_RAETH_HW_LRO */
#else
		sysRegWrite(RAETH_FE_INT_ENABLE, RAETH_FE_INT_ALL);
#if defined (CONFIG_RAETH_HW_LRO)
		sysRegWrite(RAETH_FE_INT_ENABLE, RAETH_FE_INT_ALL | ALT_RPLC_INT3 | ALT_RPLC_INT2 | ALT_RPLC_INT1);
#endif  /* CONFIG_RAETH_HW_LRO */
#endif

#ifdef CONFIG_RAETH_QDMA
#ifdef DELAY_INT
		sysRegWrite(QDMA_DELAY_INT, DELAY_INT_INIT);
		sysRegWrite(QFE_INT_ENABLE, QFE_INT_DLY_INIT);
#else
		sysRegWrite(QFE_INT_ENABLE, QFE_INT_ALL);

#endif
#endif

		raeth_enable_mac2_link_interrupt();

}

static void raeth_disable_int(void)
{
		//sysRegWrite(RAETH_DLY_INT_CFG, DELAY_INT_INIT);
		sysRegWrite(RAETH_FE_INT_ENABLE, sysRegRead(RAETH_FE_INT_ENABLE) & (~(RAETH_FE_INT_DLY_INIT)));

		raeth_disable_mac2_link_interrupt();
}


static struct sk_buff *raeth_alloc_one_skb(int len)
{
		struct sk_buff *skb;
		
		skb = dev_alloc_skb(len + NET_IP_ALIGN);
		if (skb == NULL ) {
				printk("skbuff buffer allocation failed!");
		} else {
#if !defined (CONFIG_RAETH_SCATTER_GATHER_RX_DMA)
			skb_reserve(skb, NET_IP_ALIGN);
#endif
		}
		return skb;
}

static void raeth_set_mac2_mode(enum mii_mode mode)
{
	unsigned int regSysCfg1 = 0, regValue = 0;
	regSysCfg1 = sysRegRead(SYSCFG1);
	regSysCfg1 &= ~(0x3 << 14);
	
	printk("set Mac2 mode(%d)\n",mode);
	if (RGMII == mode) {
		regSysCfg1 |= 0x0 << 14; // GE2 RGMII Mode
	} else if (MII == mode) {
		regSysCfg1 |= 0x1 << 14; // GE2 MII Mode
	} else if (RMII == mode) {
		regSysCfg1 |= 0x3 << 14; // GE2 RMII Mode
		
		regValue = sysRegRead(MAC_GPC);
		regValue &= ~(0x3 << 24);
		regValue |= 0x2 << 24; // reference clock output from RXC
		sysRegWrite(MAC_GPC, regValue);
	}
	sysRegWrite(SYSCFG1, regSysCfg1);
}

static void raeth_clear_mac2_link_interrupt()
{
    /*clear mac2 port change interrupt*/
    sysRegWrite(FE_INT_LINK_STATUS,sysRegRead(FE_INT_LINK_STATUS) | (BIT(25)));
}

static void raeth_enable_mac2_link_interrupt()
{
    sysRegWrite(FE_INT_LINK_ENABLE,sysRegRead(FE_INT_LINK_ENABLE) | (BIT(25)));
}

static void raeth_disable_mac2_link_interrupt()
{
    sysRegWrite(FE_INT_LINK_ENABLE,sysRegRead(FE_INT_LINK_ENABLE) & (~(BIT(25))));
}

static void raeth_enable_auto_poll(int phyStart, int phyEnd)
{
	unsigned int regValue = 0;

	regValue = sysRegRead(ESW_PHY_POLLING);
	
	regValue |= (1<<31);
	regValue &= ~(0x1f);
	regValue &= ~(0x1f<<8);

	regValue |= (phyStart << 0);//setup PHY address for auto polling (Start Addr).
	regValue |= (phyEnd << 8);// setup PHY address for auto polling (End Addr).

	sysRegWrite(ESW_PHY_POLLING, regValue);
}

static void raeth_set_rx_mode(struct net_device *dev)
{
    unsigned long flags;
	u32 gdm2Filter = 0;

	gdm2Filter = sysRegRead(GDM2_FILTER_CTRL);
	gdm2Filter &= (~(3 << 0));
    if (dev->flags & IFF_PROMISC) {
        printk("%s: Promiscuous mode enabled.\n", dev->name);
		gdm2Filter |= (0 << 0);
    } else if ((netdev_mc_count(dev)) != 0){
		u32 hashIdx;
		u32 regAddr = 0;
        printk("%s: My Mac , Broadcast filter, Hash table filter.\n", dev->name);
	    if ((netdev_mc_count(dev) > RAETH_HTABLE_SIZE_LIMIT) || (dev->flags & IFF_ALLMULTI)) {
			printk("All hash address\n");
	        for (hashIdx = 0; hashIdx < 16; hashIdx++) {
				regAddr = GDM2_HASH_TBE_0 + hashIdx*8;
	            sysRegWrite(regAddr, 0xffffffff);;
	        }
	    } else {
	        struct netdev_hw_addr *ha;
			u32 hashReg = 0;
	        netdev_for_each_mc_addr(ha, dev) {
	            u32 hashAddr;
				u32 bit = 0;
	            hashAddr = (u32) (((ha->addr[0] & 0x1) << 8) + (u32) (ha->addr[5]));
				hashIdx = hashAddr/32;
				regAddr = GDM2_HASH_TBE_0 + hashIdx*8;
				bit =  hashAddr%32;
				printk("Hash address(%d)\n", hashAddr);

				hashReg = sysRegRead(regAddr);
	            sysRegWrite(regAddr, hashReg | (1 << bit));
	        }
	    }

		gdm2Filter |= (1 << 0);
    } else {
        printk("%s: My Mac and Broadcast filter.\n", dev->name);
		gdm2Filter |= (2 << 0);
    }

	sysRegWrite(GDM2_FILTER_CTRL, gdm2Filter);

}

static void raeth_mac_init(END_DEVICE *ei_local)
{
	int ge2PhyAddr, ge1PhyAddr = 0;
	enum mii_mode mii;

	ge2PhyAddr = ra_detect_phy(ei_local);
	if (ge2PhyAddr < 0) {
		printk("Cannot detect Ethernet PHY!!!\n\r");
	} else {
		ei_local->phy_ops->init(ei_local);
		ge1PhyAddr = (ge2PhyAddr == 0) ? 0x1f : (ge2PhyAddr - 1);
		raeth_enable_auto_poll(ge1PhyAddr, ge2PhyAddr);
		mii = ei_local->phy_ops->get_mii_type(ei_local);
		raeth_set_mac2_mode(mii);
	}
}
static int phy_wol_eint_num = 4;

static void raeth_phy_wol_eint_init(END_DEVICE *ei_local)
{
	if (!strncmp(CONFIG_ARCH_MTK_PROJECT, "bx8590p2",8)) {
		printk("raeth_phy_wol_eint_init set eint number(%d)\n", phy_wol_eint_num);
		mt_eint_set_sens(phy_wol_eint_num, MT_EDGE_SENSITIVE);
		mt_eint_set_polarity(phy_wol_eint_num, MT_EINT_POL_NEG);
		mt_eint_unmask(phy_wol_eint_num);
	}
}

static void raeth_wol_init(END_DEVICE *ei_local)
{
	if (WOL_NONE == ei_local->wol){
		printk("Not Support wol\n");
	} else if (MAC_WOL == ei_local->wol) {
		printk("MAC Support wol\n");
		spm_set_sleep_wakesrc(WAKE_SRC_ETHERNET, true, false);
	} else if (PHY_WOL == ei_local->wol){
		printk("PHY Support wol\n");
		raeth_phy_wol_eint_init(ei_local);
	}
}

/**
 * raeth_open - Open/Initialize the ethernet port.
 * @dev: network device to initialize
 *
 * This routine goes all-out, setting everything
 * up a new at each open, even though many of these registers should only need to be set once at boot.
 */
static int raeth_open(struct net_device *dev)
{
	int i;
	unsigned long flags;

	END_DEVICE *ei_local;
	struct net_device * raethDev = NULL;
	int err;

	if (!try_module_get(THIS_MODULE))
	{
		printk("%s: Cannot reserve module\n", __FUNCTION__);
		return -1;
	}

	printk("Raeth %s (",RAETH_VERSION);
#if defined (CONFIG_RAETH_NAPI)
	printk("NAPI\n");
#elif defined (CONFIG_RA_NETWORK_TASKLET_BH)
	printk("Tasklet");
#elif defined (CONFIG_RA_NETWORK_WORKQUEUE_BH)
	printk("Workqueue");
#endif

#if defined (CONFIG_RAETH_SKB_RECYCLE_2K)
	printk(",SkbRecycle");
#endif
	printk(" )\n");

	raethDev = dev;

	ei_local = netdev_priv(raethDev); // get device pointer from System

	if (ei_local == NULL)
	{
		printk(KERN_EMERG "%s: raeth_open passed a non-existent device!\n", dev->name);
		return -ENXIO;
	}

	/* init carrier to off */
	ei_local->linkUp = 0;
	netif_carrier_off(dev);

	/* receiving packet buffer allocation - NUM_RX_DESC x MAX_RX_LENGTH */
	for ( i = 0; i < NUM_RX_DESC; i++) {
		ei_local->netrx0_skbuf[i] = raeth_alloc_one_skb(MAX_RX_LENGTH);

#if defined (CONFIG_RAETH_HW_LRO) 
		ei_local->netrx3_skbuf[i] = raeth_alloc_one_skb(MAX_LRO_RX_LENGTH);

		ei_local->netrx1_skbuf[i] = raeth_alloc_one_skb(MAX_LRO_RX_LENGTH);

#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING) 

		ei_local->netrx3_skbuf[i] = raeth_alloc_one_skb(MAX_RX_LENGTH);

		ei_local->netrx2_skbuf[i] = raeth_alloc_one_skb(MAX_RX_LENGTH);

		ei_local->netrx1_skbuf[i] = raeth_alloc_one_skb(MAX_RX_LENGTH);
#endif
	}
	
	fe_dma_init(raethDev);
	
#if defined (CONFIG_RAETH_HW_LRO)
	fe_hw_lro_init(raethDev);
#endif  /* CONFIG_RAETH_HW_LRO */

	if ( raethDev->dev_addr != NULL) {
		raethMac2AddressSet((void *)(raethDev->dev_addr));
	} else {
		printk("dev->dev_addr is empty !\n");
	} 

	spin_lock_irqsave(&(ei_local->page_lock), flags);

	INIT_WORK(&ei_local->reset_task, raeth_reset_task);
	
#ifdef WORKQUEUE_BH
#ifndef CONFIG_RAETH_NAPI
	INIT_WORK(&ei_local->rx_wq, raeth_receive_workq);
#endif // CONFIG_RAETH_NAPI //
#else
#ifndef CONFIG_RAETH_NAPI
#if defined (TASKLET_WORKQUEUE_SW)
	working_schedule = init_schedule;
	INIT_WORK(&ei_local->rx_wq, raeth_receive_workq);
	tasklet_init(&ei_local->rx_tasklet, raeth_receive_workq, 0);
#else
	tasklet_init(&ei_local->rx_tasklet, ei_receive, 0);
#endif
#endif // CONFIG_RAETH_NAPI //
#endif // WORKQUEUE_BH //

#ifdef CONFIG_RAETH_NAPI
	atomic_dec(&ei_local->irq_sem);
	napi_enable(&ei_local->napi);
#endif

	spin_unlock_irqrestore(&(ei_local->page_lock), flags);

#if defined (CONFIG_RAETH_HW_LRO)
	INIT_WORK(&ei_local->hw_lro_wq, raeth_hw_lro_workq);
#endif  /* CONFIG_RAETH_HW_LRO */

	err = request_irq( raethDev->irq, raeth_interrupt, IRQF_TRIGGER_LOW, raethDev->name, raethDev);
	if (err)
		return err;

	raeth_forward_config(raethDev);

	raeth_set_rx_mode(dev);

	raeth_mac_init(ei_local);

	raeth_wol_init(ei_local);

	if (sysRegRead(FE_INT_LINK_STATUS) & BIT(25)) {
		StarLinkStatusChange(dev);
		sysRegWrite(FE_INT_LINK_STATUS,sysRegRead(FE_INT_LINK_STATUS) | (BIT(25)));
	}

	if (WOL_NONE == ei_local->wol) {
		printk("If the platforms need to use PHY_WOL,\n\
			echo 2 > /proc/ethernet/wol must be executed before ifconfig eth0 up.\n");
		printk("If the platforms need to use MAC_WOL,\n\
			echo 1 > /proc/ethernet/wol must be executed before ifconfig eth0 up.\n");
	}
	raeth_enable_int();
	netif_start_queue(dev);

	return 0;
}

/**
 * raeth_close - shut down network device
 * @dev: network device to clear
 *
 * This routine shut down network device.
 *
 *
 */
static int raeth_close(struct net_device *dev)
{
	int i;
	END_DEVICE *ei_local = netdev_priv(dev);	// device pointer

	netif_stop_queue(dev);
	raeth_disable_int();
	raeth_stop(ei_local);
	free_irq(dev->irq, dev);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	cancel_work_sync(&ei_local->reset_task);
#endif

#ifdef WORKQUEUE_BH
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	cancel_work_sync(&ei_local->rx_wq);
#endif
#else
#if defined (TASKLET_WORKQUEUE_SW)
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	cancel_work_sync(&ei_local->rx_wq);
#endif
#endif
	tasklet_kill(&ei_local->tx_tasklet);
	tasklet_kill(&ei_local->rx_tasklet);
#endif // WORKQUEUE_BH //

#ifdef CONFIG_RAETH_NAPI
	atomic_inc(&ei_local->irq_sem);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
        napi_disable(&ei_local->napi);
#else
        netif_poll_disable(dev);
#endif
#endif


#if defined (CONFIG_RAETH_HW_LRO)
    cancel_work_sync(&ei_local->hw_lro_wq);
#endif  /* CONFIG_RAETH_HW_LRO */   

        for ( i = 0; i < NUM_RX_DESC; i++)
        {
                if (ei_local->netrx0_skbuf[i] != NULL) {
                        dev_kfree_skb(ei_local->netrx0_skbuf[i]);
			ei_local->netrx0_skbuf[i] = NULL;
		}
#if defined (CONFIG_RAETH_HW_LRO)
                if (ei_local->netrx3_skbuf[i] != NULL) {
                        dev_kfree_skb(ei_local->netrx3_skbuf[i]);
			ei_local->netrx3_skbuf[i] = NULL;
		}
                if (ei_local->netrx2_skbuf[i] != NULL) {
                        dev_kfree_skb(ei_local->netrx2_skbuf[i]);
			ei_local->netrx2_skbuf[i] = NULL;
		}
                if (ei_local->netrx1_skbuf[i] != NULL) {
                        dev_kfree_skb(ei_local->netrx1_skbuf[i]);
			ei_local->netrx1_skbuf[i] = NULL;
		}
#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING)
#if defined(CONFIG_ARCH_MT8590)
                if (ei_local->netrx3_skbuf[i] != NULL) {
                        dev_kfree_skb(ei_local->netrx3_skbuf[i]);
			ei_local->netrx3_skbuf[i] = NULL;
		}
                if (ei_local->netrx2_skbuf[i] != NULL) {
                        dev_kfree_skb(ei_local->netrx2_skbuf[i]);
			ei_local->netrx2_skbuf[i] = NULL;
		}
#endif  /* CONFIG_ARCH_MT8590 */
                if (ei_local->netrx1_skbuf[i] != NULL) {
                        dev_kfree_skb(ei_local->netrx1_skbuf[i]);
			ei_local->netrx1_skbuf[i] = NULL;
		}
#endif
        }

	for ( i = 0; i < NUM_TX_DESC; i++)
	{
		if((ei_local->skb_free[i]!=(struct  sk_buff *)0xFFFFFFFF) && (ei_local->skb_free[i]!= 0))
		{
			dev_kfree_skb_any(ei_local->skb_free[i]);
		}
	}

	/* TX Ring */
#ifdef CONFIG_RAETH_QDMA
       if (ei_local->txd_pool != NULL) {
	   pci_free_consistent(NULL, NUM_TX_DESC*sizeof(struct QDMA_txdesc), ei_local->txd_pool, ei_local->phy_txd_pool);
       }
       if (ei_local->free_head != NULL){
	       pci_free_consistent(NULL, NUM_QDMA_PAGE * sizeof(struct QDMA_txdesc), ei_local->free_head, ei_local->phy_free_head);
       }
       if (ei_local->free_page_head != NULL){
	       pci_free_consistent(NULL, NUM_QDMA_PAGE * QDMA_PAGE_SIZE, ei_local->free_page_head, ei_local->phy_free_page_head);
       }
#else	
       if (ei_local->tx_ring0 != NULL) {
	   pci_free_consistent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), ei_local->tx_ring0, ei_local->phy_tx_ring0);
	   ei_local->tx_ring0 = NULL;
       }
#endif       

#if defined (CONFIG_RAETH_QOS)
       if (ei_local->tx_ring1 != NULL) {
	   pci_free_consistent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), ei_local->tx_ring1, ei_local->phy_tx_ring1);
       }

       if (ei_local->tx_ring2 != NULL) {
	   pci_free_consistent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), ei_local->tx_ring2, ei_local->phy_tx_ring2);
       }

       if (ei_local->tx_ring3 != NULL) {
	   pci_free_consistent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), ei_local->tx_ring3, ei_local->phy_tx_ring3);
       }
#endif
	/* RX Ring */
#ifdef CONFIG_32B_DESC
       kfree(ei_local->rx_ring0);
#else
        pci_free_consistent(NULL, NUM_RX_DESC*sizeof(struct PDMA_rxdesc), ei_local->rx_ring0, ei_local->phy_rx_ring0);
	ei_local->rx_ring0 = NULL;
#endif
#if defined CONFIG_RAETH_QDMA && !defined(CONFIG_RAETH_QDMATX_QDMARX)	
#ifdef CONFIG_32B_DESC
	kfree(ei_local->qrx_ring);
#else
	pci_free_consistent(NULL, NUM_QRX_DESC*sizeof(struct PDMA_rxdesc), ei_local->qrx_ring, ei_local->phy_qrx_ring);
#endif
#endif	
#if defined (CONFIG_RAETH_HW_LRO)
        pci_free_consistent(NULL, NUM_RX_DESC*sizeof(struct PDMA_rxdesc), ei_local->rx_ring3, ei_local->phy_rx_ring3);
        pci_free_consistent(NULL, NUM_RX_DESC*sizeof(struct PDMA_rxdesc), ei_local->rx_ring2, ei_local->phy_rx_ring2);
        pci_free_consistent(NULL, NUM_RX_DESC*sizeof(struct PDMA_rxdesc), ei_local->rx_ring1, ei_local->phy_rx_ring1);
#elif defined (CONFIG_RAETH_MULTIPLE_RX_RING)
#ifdef CONFIG_32B_DESC
	kfree(ei_local->rx_ring1);
#else
#if defined(CONFIG_ARCH_MT8590)
        pci_free_consistent(NULL, NUM_RX_DESC*sizeof(struct PDMA_rxdesc), ei_local->rx_ring3, ei_local->phy_rx_ring3);
        pci_free_consistent(NULL, NUM_RX_DESC*sizeof(struct PDMA_rxdesc), ei_local->rx_ring2, ei_local->phy_rx_ring2);
#endif  /* CONFIG_ARCH_MT8590 */
        pci_free_consistent(NULL, NUM_RX_DESC*sizeof(struct PDMA_rxdesc), ei_local->rx_ring1, ei_local->phy_rx_ring1);
#endif
#endif

	printk("Free TX/RX Ring Memory!\n");

	raeth_fe_reset();

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	module_put(THIS_MODULE);
#else
	MOD_DEC_USE_COUNT;
#endif
	return 0;
}

static const struct net_device_ops raeth_netdev_ops = {
        .ndo_init               = raeth_dev_init,
        .ndo_open               = raeth_open,
        .ndo_stop               = raeth_close,
        .ndo_start_xmit         = raeth_start_xmit_fake,
        .ndo_get_stats          = raeth_get_stats,
        .ndo_set_mac_address    = raeth_set_mac_addr,
        .ndo_change_mtu         = raeth_change_mtu,
        .ndo_do_ioctl           = raeth_ioctl,
        .ndo_validate_addr      = eth_validate_addr,
#ifdef CONFIG_RAETH_HW_VLAN_RX
		.ndo_vlan_rx_register   = raeth_vlan_rx_register,
#endif
#ifdef CONFIG_NET_POLL_CONTROLLER
        .ndo_poll_controller    = raeth_clean,
#endif
		.ndo_set_rx_mode = raeth_set_rx_mode,
};

void raeth_ethifsys_init(void)
{
	enable_pll(ETHPLL, "ethpll");
	/*=========================================================================*/
	/* Power on ETHDMASYS and HIFSYS*/
	/*=========================================================================*/
	/* Power on ETHDMASYS*/
	enable_subsys(SYS_ETH, "ethdmasys");
	enable_subsys(SYS_HIF, "hifsys");
}

static int raeth_set_mac_wol(bool enable)
{
	unsigned int regValue = 0;	
	
	if (enable) {
		regValue = sysRegRead(MAC2_WOL);
		
		regValue |= (WOL_INT_CLR | WOL_INT_EN | WOL_EN);
		
		sysRegWrite(MAC2_WOL, regValue);

	} else {
		unsigned int regValue = 0;
		
		regValue = sysRegRead(MAC2_WOL);
		
		regValue &= ~(WOL_INT_EN | WOL_EN);
		
		sysRegWrite(MAC2_WOL, regValue);
		
	}
}

static int raeth_wait_tx_complete(struct net_device *dev)
{
	int check_count = 0;
	END_DEVICE *ei_local = netdev_priv(dev);
	if (ei_local == NULL)	{
		printk(KERN_EMERG "%s: FUNC(%s) passed a non-existent device!\n",
			dev->name, __FUNCTION__);
		return -ENXIO;
	}
	if(NULL != ei_local->tx_ring0) {
		while (0 == ei_local->tx_ring0[sysRegRead(TX_CTX_IDX0)].txd_info2.DDONE_bit) {
			mdelay(1);
			check_count++;
			if (check_count > 10){
				printk("wait the data of current descriptor sent completely timeout.\n");
				return -1;
			}
		}
	}
	printk("\nFUNC(%s),LINE(%d),send the data of current tx descriptor completely.\n",
		__FUNCTION__,__LINE__);
	printk("Function(%s)done!\n",__FUNCTION__);
	return 0;
}

static int raeth_resume_hw_setting(struct net_device *dev)
{
	int ge2PhyAddr, ge1PhyAddr = 0;
	enum mii_mode mii;
	END_DEVICE *ei_local = netdev_priv(dev);
	if (ei_local == NULL)	{
		printk(KERN_EMERG "%s: FUNC(%s) passed a non-existent device!\n",
			dev->name, __FUNCTION__);
		return -ENXIO;
	}
	raeth_fe_reset();
	netif_stop_queue(dev);
	raeth_resume_fe_dma(dev);

	if ( dev->dev_addr != NULL) {
		raethMac2AddressSet((void *)(dev->dev_addr));
	} else {
		printk("dev->dev_addr is empty !\n");
	}

	raeth_enable_int();
	raeth_forward_config(dev);
	raeth_set_rx_mode(dev);

	if (NULL != ei_local->phy_ops) {
		ge2PhyAddr = ei_local->phy_ops->addr;
		ei_local->phy_ops->init(ei_local);
		ge1PhyAddr = (ge2PhyAddr == 0) ? 0x1f : (ge2PhyAddr - 1);
		raeth_enable_auto_poll(ge1PhyAddr, ge2PhyAddr);
		mii = ei_local->phy_ops->get_mii_type(ei_local);
		raeth_set_mac2_mode(mii);
	}
	netif_start_queue(dev);

	printk("Function(%s)done!\n",__FUNCTION__);
	return 0;
}


static int raeth_resume(struct platform_device *pdev)
{
    struct net_device *netdev = platform_get_drvdata(pdev);
	END_DEVICE *ei_local = netdev_priv(netdev);

	if (WOL_NONE == ei_local->wol) {
		printk("raeth_resume:Not Support wol\n");
		enable_pll(ETHPLL, "ethpll");
		enable_subsys(SYS_ETH, "ethdmasys");
		enable_subsys(SYS_HIF, "hifsys");
		raeth_resume_hw_setting(netdev);
	} else if (MAC_WOL == ei_local->wol){
		printk("raeth_resume:Mac Support wol\n");
		enable_subsys(SYS_HIF, "hifsys");
		raeth_set_mac_wol(false);
		pmic_hwrs_release(CTRL_EXT_PMIC_EN);
		pmic_hwrs_release(VOLT_VTCXO_PMU);
	} else if (PHY_WOL == ei_local->wol){
		printk("raeth_resume:PHY Support wol\n");
		pmic_hwrs_release(CTRL_EXT_PMIC_EN);
		if (NULL != ei_local->phy_ops) {
			ei_local->phy_ops->resume(ei_local);
		}
	}
    return 0;
}

static int raeth_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct net_device *netdev = platform_get_drvdata(pdev);
	END_DEVICE *ei_local = netdev_priv(netdev);

	if (WOL_NONE == ei_local->wol){
		printk("raeth_suspend:Not Support wol\n");
		raeth_wait_tx_complete(netdev);
		disable_subsys(SYS_HIF, "hifsys");
		disable_subsys(SYS_ETH, "ethdmasys");
		disable_pll(ETHPLL, "ethpll");
	} else if (MAC_WOL == ei_local->wol) {
		printk("raeth_suspend:MAC Support wol\n");
		raeth_set_mac_wol(true);
		disable_subsys(SYS_HIF, "hifsys");
		pmic_hwrs_claim(VOLT_VTCXO_PMU);//26M clock
		pmic_hwrs_claim(CTRL_EXT_PMIC_EN);
	} else if (PHY_WOL == ei_local->wol){
		printk("raeth_suspend:PHY Support wol\n");
		if (NULL != ei_local->phy_ops) {
			ei_local->phy_ops->suspend(ei_local);
		}
		pmic_hwrs_claim(CTRL_EXT_PMIC_EN);
	}
    return 0;
}

static int raeth_probe(struct platform_device *pdev)
{
	int ret;
	struct net_device *dev = alloc_etherdev(sizeof(END_DEVICE));
	CHIP_SW_VER ver = mt_get_chip_sw_ver();
	enum wol_type wol = WOL_NONE;
	END_DEVICE *ei_local = netdev_priv(dev);
	
	printk("function(%s)\n", __FUNCTION__);

	if (!dev)
		return -ENOMEM;

	strcpy(dev->name, DEV_NAME);
	dev->irq  = IRQ_ENET0;
	dev->addr_len = 6;
	dev->base_addr = RALINK_FRAME_ENGINE_BASE;

	dev->netdev_ops 	= &raeth_netdev_ops;
	dev->watchdog_timeo = (5*HZ);

#if defined (CONFIG_ETHTOOL)
	/* net_device structure Init */
	ethtool_init(dev);
	dev->ethtool_ops	= &ra_ethtool_ops;
#endif

#ifdef CONFIG_RAETH_NAPI
	printk("NAPI enable, Tx Ring = %d, Rx Ring = %d\n", NUM_TX_DESC, NUM_RX_DESC);
#endif

	/* Register net device for the driver */
	if ( register_netdev(dev) != 0) {
		printk(KERN_WARNING " " __FILE__ ": No ethernet port found.\n");
		return -ENXIO;
	}

	ret = debug_proc_init();

	dev_raether = dev;
    SET_NETDEV_DEV(dev, &pdev->dev);
    platform_set_drvdata(pdev, dev);
	
	raeth_ethifsys_init();
	raeth_dev_init(dev);

	ei_local->wol = wol;
	ei_local->netdev = dev;

	return ret;
}
static int raeth_remove(struct platform_device *pdev)
{
	struct net_device *dev = dev_raether;
	END_DEVICE *ei_local;
	
	printk("function(%s), line(%d)\n", __FUNCTION__, __LINE__);
	ei_local = netdev_priv(dev);

	unregister_netdev(dev);
	RAETH_PRINT("Free ei_local and unregister netdev...\n");

	free_netdev(dev);
	debug_proc_exit();
	return 0;
}
static struct platform_device *raeth_pdev;

static struct platform_driver raeth_pdrv = {
    .driver = {
        .name = DRV_NAME,
        .owner = THIS_MODULE,
    },
    .probe = raeth_probe,
    .suspend = raeth_suspend,
    .resume = raeth_resume,
    .remove = raeth_remove,
};

/**
 * raeth_init - Module Init code
 *
 * Called by kernel to register net_device
 *
 */
int __init raeth_init(void)
{
	  int	  err;
	
	  printk("%s ...\n", __FUNCTION__);
	
	  err = platform_driver_register(&raeth_pdrv);
	  if (err) {
		  printk("%s ...\n", __FUNCTION__);
		  return err;
	  }
	  printk("%s ...\n", __FUNCTION__);
	  raeth_pdev = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	  if (IS_ERR(raeth_pdev)) {
		  err = PTR_ERR(raeth_pdev);
		  goto unreg_platform_driver;
	  }
	  
	  printk("%s success...\n", __FUNCTION__);
	  return 0;
	
	unreg_platform_driver:
	  platform_driver_unregister(&raeth_pdrv);
	  return err;
}


/**
 * raeth_exit - Module Exit code
 *
 * Cmd 'rmmod' will invode the routine to exit the module
 *
 */
void raeth_exit(void)
{
	printk("%s ...\n", __FUNCTION__);
	platform_device_unregister(raeth_pdev);
	platform_driver_unregister(&raeth_pdrv);
	printk("%s ...\n", __FUNCTION__);
}
module_init(raeth_init);
module_exit(raeth_exit);
MODULE_LICENSE("GPL");
