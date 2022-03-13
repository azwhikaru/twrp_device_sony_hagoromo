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
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>
#if defined(CONFIG_RAETH_TSO)
#include <linux/tcp.h>
#include <net/ipv6.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <linux/in.h>
#include <linux/ppp_defs.h>
#include <linux/if_pppox.h>
#endif
#if defined(CONFIG_RAETH_LRO)
#include <linux/inet_lro.h>
#endif
#include <linux/delay.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
#include <linux/sched.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 0)
#include <asm/rt2880/rt_mmap.h>
#else
#include <linux/libata-compat.h>
#endif

#include "../ra_reg.h"
#include "../raether.h"
#include "../ra_mac.h"
#include "../ra_ioctl.h"
#include "raether_pdma_dvt.h"

/* Global variables */
static unsigned int g_pdma_dvt_show_config;
static unsigned int g_pdma_dvt_rx_test_config;
static unsigned int g_pdma_dvt_tx_test_config;
static unsigned int g_pdma_dvt_debug_test_config;
static unsigned int g_pdma_dvt_lro_test_config;

unsigned int g_pdma_dev_lanport = 0;
unsigned int g_pdma_dev_wanport = 0;

void skb_dump(struct sk_buff *sk)
{
	unsigned int i;

	printk("skb_dump: from %s with len %d (%d) headroom=%d tailroom=%d\n",
	       sk->dev ? sk->dev->name : "ip stack", sk->len, sk->truesize,
	       skb_headroom(sk), skb_tailroom(sk));

	/* for(i=(unsigned int)sk->head;i<=(unsigned int)sk->tail;i++) { */
	/* for(i=(unsigned int)sk->head;i<=(unsigned int)sk->data+20;i++) { */
	for (i = (unsigned int)sk->head; i <= (unsigned int)sk->data + 60; i++) {
		if ((i % 20) == 0)
			printk("\n");
		if (i == (unsigned int)sk->data)
			printk("{");
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 21)
		if (i == (unsigned int)sk->transport_header)
			printk("#");
		if (i == (unsigned int)sk->network_header)
			printk("|");
		if (i == (unsigned int)sk->mac_header)
			printk("*");
#else
		if (i == (unsigned int)sk->h.raw)
			printk("#");
		if (i == (unsigned int)sk->nh.raw)
			printk("|");
		if (i == (unsigned int)sk->mac.raw)
			printk("*");
#endif
		printk("%02X-", *((unsigned char *)i));
		if (i == (unsigned int)sk->tail)
			printk("}");
	}
	printk("\n");
}

#if defined(CONFIG_RAETH_HW_LRO)
/* PDMA LRO test functions start */
int pdma_lro_disable_dvt(void)
{
	unsigned int regVal = 0;

	printk("pdma_lro_disable_dvt()\n");

	/* 1. Invalid LRO ring1~3 */
	SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 0);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 0);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING3, 0);

	/* 2 Polling relinguish */
	while (sysRegRead(ADMA_LRO_CTRL_DW0) & PDMA_LRO_RELINGUISH) {;
	}

	/* 3. Disable LRO */
	regVal = sysRegRead(ADMA_LRO_CTRL_DW0);
	regVal &= ~(PDMA_LRO_EN);
	sysRegWrite(ADMA_LRO_CTRL_DW0, regVal);

#if 0
	/* 4. Disable non-lro multiple rx */
	SET_PDMA_NON_LRO_MULTI_EN(0);

	/* 5.1. Set GDM1 to ring0 */
	SET_GDM_PID1_RXID_SEL(0);
	/* 5.2. Set GDM2 to ring0 */
	SET_GDM_PID2_RXID_SEL(0);
#endif

	return 0;
}

int pdma_lro_force_aggre_dvt(void)
{
	unsigned int regVal = 0;
	unsigned int ip;

	printk("pdma_lro_force_aggre_dvt()\n");

/* pdma rx ring1 */
	/* 1. Set RX ring mode to force port */
	SET_PDMA_RXRING_MODE(ADMA_RX_RING1, PDMA_RX_FORCE_PORT);

	/* 2. Configure lro ring */
	/* 2.1 set src/destination TCP ports */
	SET_PDMA_RXRING_TCP_SRC_PORT(ADMA_RX_RING1, 3423);
	SET_PDMA_RXRING_TCP_DEST_PORT(ADMA_RX_RING1, 2301);
	/* 2.2 set src/destination IPs */
	str_to_ip(&ip, "10.10.10.3");
	sysRegWrite(LRO_RX_RING1_SIP_DW0, ip);
	str_to_ip(&ip, "10.10.10.100");
	sysRegWrite(LRO_RX_RING1_DIP_DW0, ip);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING1, 1);

	/* 2.3 Valid LRO ring */
	SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 1);

	/* 2.4 Set AGE timer */
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING1, 0);

	/* 2.5 Set max AGG timer */
	SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING1, 0);

	/* 2.6 Set max LRO agg count */
	SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING1, HW_LRO_MAX_AGG_CNT);

	/* 3. IPv4 checksum update enable */
	SET_PDMA_LRO_IPV4_CSUM_UPDATE_EN(1);

	/* 4. Polling relinguish */
	while (sysRegRead(ADMA_LRO_CTRL_DW0) & PDMA_LRO_RELINGUISH) {;
	}

	/* 5. Enable LRO */
	regVal = sysRegRead(ADMA_LRO_CTRL_DW0);
	regVal |= PDMA_LRO_EN;
	sysRegWrite(ADMA_LRO_CTRL_DW0, regVal);

	return 0;
}

int pdma_lro_auto_aggre_dvt(void)
{
	unsigned int regVal = 0;
	unsigned int ip;

	printk("pdma_lro_auto_aggre_dvt()\n");

	/* 1.1 Set my IP_1 */
	str_to_ip(&ip, "10.10.10.254");
	sysRegWrite(LRO_RX_RING0_DIP_DW0, ip);
	sysRegWrite(LRO_RX_RING0_DIP_DW1, 0);
	sysRegWrite(LRO_RX_RING0_DIP_DW2, 0);
	sysRegWrite(LRO_RX_RING0_DIP_DW3, 0);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING0, 1);

	/* 1.2 Set my IP_2 */
	str_to_ip(&ip, "10.10.20.254");
	sysRegWrite(LRO_RX_RING1_DIP_DW0, ip);
	sysRegWrite(LRO_RX_RING1_DIP_DW1, 0);
	sysRegWrite(LRO_RX_RING1_DIP_DW2, 0);
	sysRegWrite(LRO_RX_RING1_DIP_DW3, 0);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING1, 1);

	/* 1.3 Set my IP_3 */
	sysRegWrite(LRO_RX_RING2_DIP_DW3, 0x20010238);
	sysRegWrite(LRO_RX_RING2_DIP_DW2, 0x08000000);
	sysRegWrite(LRO_RX_RING2_DIP_DW1, 0x00000000);
	sysRegWrite(LRO_RX_RING2_DIP_DW0, 0x00000254);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING2, 1);

	/* 1.4 Set my IP_4 */
	sysRegWrite(LRO_RX_RING3_DIP_DW3, 0x20010238);
	sysRegWrite(LRO_RX_RING3_DIP_DW2, 0x08010000);
	sysRegWrite(LRO_RX_RING3_DIP_DW1, 0x00000000);
	sysRegWrite(LRO_RX_RING3_DIP_DW0, 0x00000254);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING3, 1);

	/* 2.1 Set RX ring1~3 to auto-learn modes */
	SET_PDMA_RXRING_MODE(ADMA_RX_RING1, PDMA_RX_AUTO_LEARN);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING2, PDMA_RX_AUTO_LEARN);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING3, PDMA_RX_AUTO_LEARN);

	/* 2.2 Valid LRO ring */
	SET_PDMA_RXRING_VALID(ADMA_RX_RING0, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING3, 1);

	/* 2.3 Set AGE timer */
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING1, 0);
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING2, 0);
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING3, 0);

	/* 2.4 Set max AGG timer */
	SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING1, 0);
	SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING2, 0);
	SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING3, 0);

	/* 2.5 Set max LRO agg count */
	SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING1, HW_LRO_MAX_AGG_CNT);
	SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING2, HW_LRO_MAX_AGG_CNT);
	SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING3, HW_LRO_MAX_AGG_CNT);

	/* 3.0 IPv6 LRO enable */
	SET_PDMA_LRO_IPV6_EN(1);

	/* 3.1 IPv4 checksum update disable */
	SET_PDMA_LRO_IPV4_CSUM_UPDATE_EN(1);

	/* 3.2 switch priority comparision to byte count mode */
	SET_PDMA_LRO_ALT_SCORE_MODE(PDMA_LRO_ALT_BYTE_CNT_MODE);

	/* 3.3 bandwidth threshold setting */
	SET_PDMA_LRO_BW_THRESHOLD(0);

	/* 3.4 auto-learn score delta setting */
	sysRegWrite(LRO_ALT_SCORE_DELTA, 0);

	/* 3.5 Set ALT timer to 20us: (unit: 20us) */
	SET_PDMA_LRO_ALT_REFRESH_TIMER_UNIT(HW_LRO_TIMER_UNIT);
	/* 3.6 Set ALT refresh timer to 1 sec. (unit: 20us) */
	SET_PDMA_LRO_ALT_REFRESH_TIMER(HW_LRO_REFRESH_TIME);

	/* 4. Polling relinguish */
	while (sysRegRead(ADMA_LRO_CTRL_DW0) & PDMA_LRO_RELINGUISH) {;
	}

	/* 5. Enable LRO */
	regVal = sysRegRead(ADMA_LRO_CTRL_DW0);
	regVal |= PDMA_LRO_EN;
	sysRegWrite(ADMA_LRO_CTRL_DW0, regVal);

	return 0;
}

int pdma_lro_auto_ipv6_dvt(void)
{
	unsigned int regVal = 0;

	printk("pdma_lro_auto_ipv6_dvt()\n");

	/* 1. Set my IP */
	sysRegWrite(LRO_RX_RING1_DIP_DW3, 0x20010238);
	sysRegWrite(LRO_RX_RING1_DIP_DW2, 0x08000000);
	sysRegWrite(LRO_RX_RING1_DIP_DW1, 0x00000000);
	sysRegWrite(LRO_RX_RING1_DIP_DW0, 0x00000254);

	/* 2.1 Set RX ring1~3 to auto-learn modes */
	SET_PDMA_RXRING_MODE(ADMA_RX_RING1, PDMA_RX_AUTO_LEARN);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING2, PDMA_RX_AUTO_LEARN);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING3, PDMA_RX_AUTO_LEARN);

	/* 2.2 Valid LRO ring */
	SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING3, 1);

	/* 2.3 Set AGE timer */
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING1, HW_LRO_AGE_TIME);
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING2, HW_LRO_AGE_TIME);
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING3, HW_LRO_AGE_TIME);

	/* 3.0 IPv6 LRO enable */
	SET_PDMA_LRO_IPV6_EN(1);

	/* 3.1 IPv4 checksum update disable */
	SET_PDMA_LRO_IPV4_CSUM_UPDATE_EN(1);

	/* 3.2 switch priority comparision to byte count mode */
	SET_PDMA_LRO_ALT_SCORE_MODE(PDMA_LRO_ALT_BYTE_CNT_MODE);

	/* 3.3 bandwidth threshold setting */
	SET_PDMA_LRO_BW_THRESHOLD(0);

	/* 3.4 auto-learn score delta setting */
	sysRegWrite(LRO_ALT_SCORE_DELTA, 0);

	/* 3.5 Set ALT timer to 500us: (unit: 20us) */
	SET_PDMA_LRO_ALT_REFRESH_TIMER_UNIT(25);
	/* 3.6 Set ALT refresh timer to 1 sec. (unit: 500us) */
	SET_PDMA_LRO_ALT_REFRESH_TIMER(2000);

	/* 3.7 Set max AGG timer: 10 msec. */
	SET_PDMA_LRO_MAX_AGG_TIME(HW_LRO_AGG_TIME);

	/* 4. Polling relinguish */
	while (sysRegRead(ADMA_LRO_CTRL_DW0) & PDMA_LRO_RELINGUISH) {;
	}

	/* 5. Enable LRO */
	regVal = sysRegRead(ADMA_LRO_CTRL_DW0);
	regVal |= PDMA_LRO_EN;
	sysRegWrite(ADMA_LRO_CTRL_DW0, regVal);

	return 0;
}

int pdma_lro_auto_myIP_dvt(void)
{
	unsigned int regVal = 0;
	unsigned int ip;

	printk("pdma_lro_auto_myIP_dvt()\n");

	/* 1.1 Set my IP_1 */
	str_to_ip(&ip, "10.10.10.254");
	sysRegWrite(LRO_RX_RING0_DIP_DW0, ip);
	sysRegWrite(LRO_RX_RING0_DIP_DW1, 0);
	sysRegWrite(LRO_RX_RING0_DIP_DW2, 0);
	sysRegWrite(LRO_RX_RING0_DIP_DW3, 0);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING0, 1);
	/* 1.2 Set my IP_2 */
	str_to_ip(&ip, "10.10.20.254");
	sysRegWrite(LRO_RX_RING1_DIP_DW0, ip);
	sysRegWrite(LRO_RX_RING1_DIP_DW1, 0);
	sysRegWrite(LRO_RX_RING1_DIP_DW2, 0);
	sysRegWrite(LRO_RX_RING1_DIP_DW3, 0);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING1, 1);
	/* 1.3 Set my IP_3 */
	sysRegWrite(LRO_RX_RING2_DIP_DW3, 0x20010238);
	sysRegWrite(LRO_RX_RING2_DIP_DW2, 0x08000000);
	sysRegWrite(LRO_RX_RING2_DIP_DW1, 0x00000000);
	sysRegWrite(LRO_RX_RING2_DIP_DW0, 0x00000254);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING2, 1);
	/* 1.4 Set my IP_4 */
	sysRegWrite(LRO_RX_RING3_DIP_DW3, 0x20010238);
	sysRegWrite(LRO_RX_RING3_DIP_DW2, 0x08010000);
	sysRegWrite(LRO_RX_RING3_DIP_DW1, 0x00000000);
	sysRegWrite(LRO_RX_RING3_DIP_DW0, 0x00000254);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING3, 1);

	/* 2.1 Set RX ring1~3 to auto-learn modes */
	SET_PDMA_RXRING_MODE(ADMA_RX_RING1, PDMA_RX_AUTO_LEARN);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING2, PDMA_RX_AUTO_LEARN);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING3, PDMA_RX_AUTO_LEARN);

	/* 2.2 Valid LRO ring */
	SET_PDMA_RXRING_VALID(ADMA_RX_RING0, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING3, 1);

	/* 2.3 Set AGE timer */
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING1, HW_LRO_AGE_TIME);
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING2, HW_LRO_AGE_TIME);
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING3, HW_LRO_AGE_TIME);

	/* 3.0 IPv6 LRO enable */
	SET_PDMA_LRO_IPV6_EN(1);

	/* 3.1 IPv4 checksum update disable */
	SET_PDMA_LRO_IPV4_CSUM_UPDATE_EN(1);

	/* 3.2 switch priority comparision to byte count mode */
	SET_PDMA_LRO_ALT_SCORE_MODE(PDMA_LRO_ALT_BYTE_CNT_MODE);

	/* 3.3 bandwidth threshold setting */
	SET_PDMA_LRO_BW_THRESHOLD(0);

	/* 3.4 auto-learn score delta setting */
	sysRegWrite(LRO_ALT_SCORE_DELTA, 0);

	/* 3.5 Set ALT timer to 500us: (unit: 20us) */
	SET_PDMA_LRO_ALT_REFRESH_TIMER_UNIT(25);
	/* 3.6 Set ALT refresh timer to 1 sec. (unit: 500us) */
	SET_PDMA_LRO_ALT_REFRESH_TIMER(2000);

	/* 3.7 Set max AGG timer: 10 msec. */
	SET_PDMA_LRO_MAX_AGG_TIME(HW_LRO_AGG_TIME);

	/* 4. Polling relinguish */
	while (sysRegRead(ADMA_LRO_CTRL_DW0) & PDMA_LRO_RELINGUISH) {;
	}

	/* 5. Enable LRO */
	regVal = sysRegRead(ADMA_LRO_CTRL_DW0);
	regVal |= PDMA_LRO_EN;
	sysRegWrite(ADMA_LRO_CTRL_DW0, regVal);

	return 0;
}

int pdma_lro_dly_int_dvt(int index)
{
	unsigned int regVal = 0;
	unsigned int ip;

	printk("pdma_lro_dly_int_dvt(%d)\n", index);

#if 0
	/* 1.1 Set my IP_1 */
	/* str_to_ip( &ip, "10.10.10.254" ); */
	str_to_ip(&ip, "10.10.10.100");
	sysRegWrite(LRO_RX_RING0_DIP_DW0, ip);
	sysRegWrite(LRO_RX_RING0_DIP_DW1, 0);
	sysRegWrite(LRO_RX_RING0_DIP_DW2, 0);
	sysRegWrite(LRO_RX_RING0_DIP_DW3, 0);
#else
	/* 1.1 set src/destination TCP ports */
	SET_PDMA_RXRING_TCP_SRC_PORT(ADMA_RX_RING1, 3423);
	SET_PDMA_RXRING_TCP_DEST_PORT(ADMA_RX_RING1, 2301);
	SET_PDMA_RXRING_TCP_SRC_PORT(ADMA_RX_RING2, 3423);
	SET_PDMA_RXRING_TCP_DEST_PORT(ADMA_RX_RING2, 2301);
	SET_PDMA_RXRING_TCP_SRC_PORT(ADMA_RX_RING3, 3423);
	SET_PDMA_RXRING_TCP_DEST_PORT(ADMA_RX_RING3, 2301);
	/* 1.2 set src/destination IPs */
	str_to_ip(&ip, "10.10.10.3");
	sysRegWrite(LRO_RX_RING1_SIP_DW0, ip);
	str_to_ip(&ip, "10.10.10.100");
	sysRegWrite(LRO_RX_RING1_DIP_DW0, ip);
	str_to_ip(&ip, "10.10.10.3");
	sysRegWrite(LRO_RX_RING2_SIP_DW0, ip);
	str_to_ip(&ip, "10.10.10.100");
	sysRegWrite(LRO_RX_RING2_DIP_DW0, ip);
	str_to_ip(&ip, "10.10.10.3");
	sysRegWrite(LRO_RX_RING3_SIP_DW0, ip);
	str_to_ip(&ip, "10.10.10.100");
	sysRegWrite(LRO_RX_RING3_DIP_DW0, ip);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING1, 1);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING2, 1);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING3, 1);
#endif

	if (index == 0) {
		/* 1.2 Disable DLY_INT for lro ring */
		SET_PDMA_LRO_DLY_INT_EN(0);
	} else {
		/* 1.2 Enable DLY_INT for lro ring */
		SET_PDMA_LRO_DLY_INT_EN(1);
	}

	/* 1.3 LRO ring DLY_INT setting */
	if (index == 1) {
		sysRegWrite(LRO_RX1_DLY_INT, DELAY_INT_INIT);
	} else if (index == 2) {
		sysRegWrite(LRO_RX2_DLY_INT, DELAY_INT_INIT);
	} else if (index == 3) {
		sysRegWrite(LRO_RX3_DLY_INT, DELAY_INT_INIT);
	}
#if 0
	/* 2.1 Set RX rings to auto-learn modes */
	SET_PDMA_RXRING_MODE(ADMA_RX_RING1, PDMA_RX_AUTO_LEARN);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING2, PDMA_RX_AUTO_LEARN);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING3, PDMA_RX_AUTO_LEARN);
#else
	/* 2.0 set rx ring mode */
	SET_PDMA_RXRING_MODE(ADMA_RX_RING1, PDMA_RX_FORCE_PORT);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING2, PDMA_RX_FORCE_PORT);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING3, PDMA_RX_FORCE_PORT);

	/* 2.1 IPv4 force port mode */
	SET_PDMA_RXRING_IPV4_FORCE_MODE(ADMA_RX_RING1, 1);
	SET_PDMA_RXRING_IPV4_FORCE_MODE(ADMA_RX_RING2, 1);
	SET_PDMA_RXRING_IPV4_FORCE_MODE(ADMA_RX_RING3, 1);
#endif

	/* 2.2 Valid LRO ring */
	SET_PDMA_RXRING_VALID(ADMA_RX_RING0, 1);
	if ((index == 0) || (index == 1)) {
		SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 1);
		SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 0);
		SET_PDMA_RXRING_VALID(ADMA_RX_RING3, 0);
	} else if (index == 2) {
		SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 0);
		SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 1);
		SET_PDMA_RXRING_VALID(ADMA_RX_RING3, 0);
	} else {
		SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 0);
		SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 0);
		SET_PDMA_RXRING_VALID(ADMA_RX_RING3, 1);
	}

	/* 2.3 Set AGE timer */
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING1, HW_LRO_AGE_TIME);
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING2, HW_LRO_AGE_TIME);
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING3, HW_LRO_AGE_TIME);

	/* 3.1 IPv4 checksum update enable */
	SET_PDMA_LRO_IPV4_CSUM_UPDATE_EN(1);

	/* 3.2 switch priority comparision to byte count mode */
	SET_PDMA_LRO_ALT_SCORE_MODE(PDMA_LRO_ALT_BYTE_CNT_MODE);

	/* 3.3 bandwidth threshold setting */
	SET_PDMA_LRO_BW_THRESHOLD(0);

	/* 3.4 auto-learn score delta setting */
	sysRegWrite(LRO_ALT_SCORE_DELTA, 0);

	/* 3.5 Set ALT timer to 500us: (unit: 20us) */
	SET_PDMA_LRO_ALT_REFRESH_TIMER_UNIT(25);
	/* 3.6 Set ALT refresh timer to 1 sec. (unit: 500us) */
	SET_PDMA_LRO_ALT_REFRESH_TIMER(2000);

	/* 3.7 Set max AGG timer */
	SET_PDMA_LRO_MAX_AGG_TIME(HW_LRO_AGG_TIME);

	/* 4. Polling relinguish */
	while (sysRegRead(ADMA_LRO_CTRL_DW0) & PDMA_LRO_RELINGUISH) {;
	}

	/* 5. Enable LRO */
	regVal = sysRegRead(ADMA_LRO_CTRL_DW0);
	regVal |= PDMA_LRO_EN;
	sysRegWrite(ADMA_LRO_CTRL_DW0, regVal);

	return 0;
}

int pdma_lro_dly_int0_dvt(void)
{
	return pdma_lro_dly_int_dvt(0);
}

int pdma_lro_dly_int1_dvt(void)
{
	return pdma_lro_dly_int_dvt(1);
}

int pdma_lro_dly_int2_dvt(void)
{
	return pdma_lro_dly_int_dvt(2);
}

int pdma_lro_dly_int3_dvt(void)
{
	return pdma_lro_dly_int_dvt(3);
}

#endif /* CONFIG_RAETH_HW_LRO */

#if defined(CONFIG_RAETH_MULTIPLE_RX_RING)
int pdma_gdm_rxid_config(void)
{
	unsigned int regVal = 0;

	printk("pdma_gdm_rxid_config()\n");

	/* 1. Set RX ring1~3 to pse modes */
	SET_PDMA_RXRING_MODE(ADMA_RX_RING1, PDMA_RX_PSE_MODE);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING2, PDMA_RX_PSE_MODE);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING3, PDMA_RX_PSE_MODE);

	/* 2. Enable non-lro multiple rx */
	SET_PDMA_NON_LRO_MULTI_EN(1);

	return 0;
}

int pdma_non_lro_portid_dvt(void)
{
	unsigned int regVal = 0;

	printk("pdma_non_lro_portid_dvt()\n");

	/* 1. Set GDM1 to ring3 */
	SET_GDM_PID1_RXID_SEL(3);
#if 0
	/* 2. Set GDM2 to ring1 */
	SET_GDM_PID2_RXID_SEL(1);
#endif

	/* 3. Set priority rule: pid */
	SET_GDM_RXID_PRI_SEL(GDM_PRI_PID);

	/* PDMA multi-rx enable */
	pdma_gdm_rxid_config();

	return 0;
}

int pdma_non_lro_stag_dvt(void)
{
	unsigned int regVal = 0;

	printk("pdma_non_lro_stag_dvt()\n");

	/* 1. Set STAG4 to ring0 */
	GDM_STAG_RXID_SEL(4, 0);
	/* 2. Set STAG3 to ring1 */
	GDM_STAG_RXID_SEL(3, 1);
	/* 3. Set STAG2 to ring2 */
	GDM_STAG_RXID_SEL(2, 2);
	/* 4. Set STAG1 to ring3 */
	GDM_STAG_RXID_SEL(1, 3);

	/* 5. Set priority rule: stag/pid */
	SET_GDM_RXID_PRI_SEL(GDM_PRI_PID);

	/* PDMA multi-rx enable */
	pdma_gdm_rxid_config();

	return 0;
}

int pdma_non_lro_vlan_dvt(void)
{
	unsigned int regVal = 0;

	printk("pdma_non_lro_vlan_dvt()\n");

	/* 1. Set vlan priority=3 to ring1 */
	SET_GDM_VLAN_PRI_RXID_SEL(3, 1);
	/* 2. Set vlan priority=2 to ring2 */
	SET_GDM_VLAN_PRI_RXID_SEL(2, 2);
	/* 3. Set vlan priority=1 to ring3 */
	SET_GDM_VLAN_PRI_RXID_SEL(1, 3);
	/* 4. Set vlan priority=0 to ring3 */
	SET_GDM_VLAN_PRI_RXID_SEL(0, 3);

	/* 1. Set vlan priority=4 to ring1 */
	SET_GDM_VLAN_PRI_RXID_SEL(4, 1);
	/* 2. Set vlan priority=5 to ring2 */
	SET_GDM_VLAN_PRI_RXID_SEL(5, 2);
	/* 3. Set vlan priority=6 to ring3 */
	SET_GDM_VLAN_PRI_RXID_SEL(6, 3);
	/* 4. Set vlan priority=7 to ring3 */
	SET_GDM_VLAN_PRI_RXID_SEL(7, 3);

	/* 4. Set priority rule: vlan > pid */
	SET_GDM_RXID_PRI_SEL(GDM_PRI_VLAN_PID);

	/* PDMA multi-rx enable */
	pdma_gdm_rxid_config();

	return 0;
}

int pdma_non_lro_tcpack_dvt(void)
{
	unsigned int regVal = 0;

	printk("pdma_non_lro_tcpack_dvt()\n");

	/* 1. Enable TCP ACK with zero payload check */
	SET_GDM_TCP_ACK_WZPC(1);
	/* 2. Set TCP ACK to ring3 */
	SET_GDM_TCP_ACK_RXID_SEL(3);

	/* 3. Set priority rule: ack > pid */
	SET_GDM_RXID_PRI_SEL(GDM_PRI_ACK_PID);

	/* PDMA multi-rx enable */
	pdma_gdm_rxid_config();

	return 0;
}

int pdma_non_lro_pri1_dvt(void)
{
	unsigned int regVal = 0;

	printk("pdma_non_lro_pri1_dvt()\n");

	/* 1. Set GDM1 to ring0 */
	SET_GDM_PID1_RXID_SEL(0);

	/* 2.1 Disable TCP ACK with zero payload check */
	SET_GDM_TCP_ACK_WZPC(0);
	/* 2.2 Set TCP ACK to ring1 */
	SET_GDM_TCP_ACK_RXID_SEL(1);

	/* 3. Set vlan priority=1 to ring2 */
	SET_GDM_VLAN_PRI_RXID_SEL(1, 2);

	/* 4. Set priority rule: vlan > ack > pid */
	SET_GDM_RXID_PRI_SEL(GDM_PRI_VLAN_ACK_PID);

	/* PDMA multi-rx enable */
	pdma_gdm_rxid_config();

	return 0;
}

int pdma_non_lro_pri2_dvt(void)
{
	unsigned int regVal = 0;

	printk("pdma_non_lro_pri2_dvt()\n");

	/* 1. Set GDM1 to ring0 */
	SET_GDM_PID1_RXID_SEL(0);

	/* 2.1 Disable TCP ACK with zero payload check */
	SET_GDM_TCP_ACK_WZPC(0);
	/* 2.2 Set TCP ACK to ring1 */
	SET_GDM_TCP_ACK_RXID_SEL(1);

	/* 3. Set vlan priority=1 to ring2 */
	SET_GDM_VLAN_PRI_RXID_SEL(1, 2);

	/* 4. Set priority rule: ack > vlan > pid */
	SET_GDM_RXID_PRI_SEL(GDM_PRI_ACK_VLAN_PID);

	/* PDMA multi-rx enable */
	pdma_gdm_rxid_config();

	return 0;
}
#endif /* CONFIG_RAETH_MULTIPLE_RX_RING */
const static PDMA_LRO_DVT_FUNC pdma_dvt_lro_func[] = {
#if defined(CONFIG_RAETH_HW_LRO)
	[0] = pdma_lro_disable_dvt,	/* PDMA_TEST_LRO_DISABLE */
	[1] = pdma_lro_force_aggre_dvt,	/* PDMA_TEST_LRO_FORCE_PORT */
	[2] = pdma_lro_auto_aggre_dvt,	/* PDMA_TEST_LRO_AUTO_LEARN */
	[3] = pdma_lro_auto_ipv6_dvt,	/* PDMA_TEST_LRO_AUTO_IPV6 */
	[4] = pdma_lro_auto_myIP_dvt,	/* PDMA_TEST_LRO_AUTO_MYIP */
	[5] = pdma_lro_force_aggre_dvt,	/* PDMA_TEST_LRO_FORCE_AGGREGATE */
#endif /* CONFIG_RAETH_HW_LRO */
#if defined(CONFIG_RAETH_MULTIPLE_RX_RING)
	[6] = pdma_non_lro_portid_dvt,	/* PDMA_TEST_NON_LRO_PORT_ID */
	[7] = pdma_non_lro_stag_dvt,	/* PDMA_TEST_NON_LRO_STAG */
	[8] = pdma_non_lro_vlan_dvt,	/* PDMA_TEST_NON_LRO_VLAN */
	[9] = pdma_non_lro_tcpack_dvt,	/* PDMA_TEST_NON_LRO_TCP_ACK */
	[10] = pdma_non_lro_pri1_dvt,	/* PDMA_TEST_NON_LRO_PRI1 */
	[11] = pdma_non_lro_pri2_dvt,	/* PDMA_TEST_NON_LRO_PRI2 */
#endif /* CONFIG_RAETH_MULTIPLE_RX_RING */
#if defined(CONFIG_RAETH_HW_LRO)
	[12] = pdma_lro_dly_int0_dvt,	/* PDMA_TEST_LRO_DLY_INT0 */
	[13] = pdma_lro_dly_int1_dvt,	/* PDMA_TEST_LRO_DLY_INT1 */
	[14] = pdma_lro_dly_int2_dvt,	/* PDMA_TEST_LRO_DLY_INT2 */
	[15] = pdma_lro_dly_int3_dvt,	/* PDMA_TEST_LRO_DLY_INT3 */
#endif /* CONFIG_RAETH_HW_LRO */
};

/* PDMA LRO test functions end */

#if defined(CONFIG_RAETH_HW_LRO) || defined(CONFIG_RAETH_MULTIPLE_RX_RING)
void raeth_pdma_lro_dvt(int rx_ring_no, END_DEVICE *ei_local,
			int rx_dma_owner_idx0)
{
	if (pdma_dvt_get_show_config() & PDMA_SHOW_RX_DESC) {
		if (rx_ring_no == 1) {
			printk("------- eth_recv (ring1) --------\n");
			printk("rx_info1=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info1);
			printk("rx_info2=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info2);
			printk("rx_info3=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info3);
			printk("rx_info4=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info4);
			printk("-------------------------------\n");
		} else if (rx_ring_no == 2) {
			printk("------- eth_recv (ring2) --------\n");
			printk("rx_info1=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info1);
			printk("rx_info2=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info2);
			printk("rx_info3=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info3);
			printk("rx_info4=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info4);
			printk("-------------------------------\n");
		} else if (rx_ring_no == 3) {
			printk("------- eth_recv (ring3) --------\n");
			printk("rx_info1=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info1);
			printk("rx_info2=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info2);
			printk("rx_info3=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info3);
			printk("rx_info4=0x%x\n",
			       *(unsigned int *)
			       &ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info4);
			printk("-------------------------------\n");
		}
	}
	if ((pdma_dvt_get_show_config() & PDMA_SHOW_DETAIL_RX_DESC) ||
	    (pdma_dvt_get_lro_test_config()==PDMA_TEST_LRO_FORCE_PORT)) {
		if (rx_ring_no == 1) {
			printk("------- eth_recv (ring1) --------\n");
			printk("rx_info1.PDP0=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info1.PDP0);
			printk("rx_info2.DDONE_bit=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info2.DDONE_bit);
			printk("rx_info2.LS0=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info2.LS0);
			printk("rx_info2.PLEN0=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info2.PLEN0);
			printk("rx_info2.TAG=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info2.TAG);
#if defined(CONFIG_ARCH_MT8590)
			printk("rx_info2.LRO_AGG_CNT=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info2.LRO_AGG_CNT);
			printk("rx_info2.REV=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info2.REV);
#else
			printk("rx_info2.LS1=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info2.LS1);
#endif /* CONFIG_RAETH_HW_LRO */
			printk("rx_info2.PLEN1=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info2.PLEN1);
			printk("rx_info3.TPID=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info3.TPID);
			printk("rx_info3.VID=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info3.VID);
			printk("rx_info4.IP6=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info4.IP6);
			printk("rx_info4.IP4=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info4.IP4);
			printk("rx_info4.IP4F=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info4.IP4F);
			printk("rx_info4.TACK=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info4.TACK);
			printk("rx_info4.L4VLD=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info4.L4VLD);
			printk("rx_info4.L4F=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info4.L4F);
			printk("rx_info4.SPORT=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info4.SP);
			printk("rx_info4.CRSN=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info4.CRSN);
			printk("rx_info4.FOE_Entry=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info4.FOE_Entry);
			printk("-------------------------------\n");
		} else if (rx_ring_no == 2) {
			printk("------- eth_recv (ring2) --------\n");
			printk("rx_info1.PDP0=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info1.PDP0);
			printk("rx_info2.DDONE_bit=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info2.DDONE_bit);
			printk("rx_info2.LS0=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info2.LS0);
			printk("rx_info2.PLEN0=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info2.PLEN0);
			printk("rx_info2.TAG=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info2.TAG);
#if defined(CONFIG_ARCH_MT8590)
			printk("rx_info2.LRO_AGG_CNT=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info2.LRO_AGG_CNT);
			printk("rx_info2.REV=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info2.REV);
#else
			printk("rx_info2.LS1=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info2.LS1);
#endif /* CONFIG_RAETH_HW_LRO */
			printk("rx_info2.PLEN1=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info2.PLEN1);
			printk("rx_info3.TPID=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info3.TPID);
			printk("rx_info3.VID=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info3.VID);
			printk("rx_info4.IP6=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info4.IP6);
			printk("rx_info4.IP4=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info4.IP4);
			printk("rx_info4.IP4F=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info4.IP4F);
			printk("rx_info4.TACK=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info4.TACK);
			printk("rx_info4.L4VLD=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info4.L4VLD);
			printk("rx_info4.L4F=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info4.L4F);
			printk("rx_info4.SPORT=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info4.SP);
			printk("rx_info4.CRSN=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info4.CRSN);
			printk("rx_info4.FOE_Entry=0x%x\n",
			       ei_local->rx_ring2[rx_dma_owner_idx0].
			       rxd_info4.FOE_Entry);
			printk("-------------------------------\n");
		} else if (rx_ring_no == 3) {
			printk("------- eth_recv (ring3) --------\n");
			printk("rx_info1.PDP0=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info1.PDP0);
			printk("rx_info2.DDONE_bit=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info2.DDONE_bit);
			printk("rx_info2.LS0=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info2.LS0);
			printk("rx_info2.PLEN0=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info2.PLEN0);
			printk("rx_info2.TAG=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info2.TAG);
#if defined(CONFIG_ARCH_MT8590)
			printk("rx_info2.LRO_AGG_CNT=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info2.LRO_AGG_CNT);
			printk("rx_info2.REV=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info2.REV);
#else
			printk("rx_info2.LS1=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info2.LS1);
#endif /* CONFIG_RAETH_HW_LRO */
			printk("rx_info2.PLEN1=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info2.PLEN1);
			printk("rx_info3.TPID=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info3.TPID);
			printk("rx_info3.VID=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info3.VID);
			printk("rx_info4.IP6=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info4.IP6);
			printk("rx_info4.IP4=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info4.IP4);
			printk("rx_info4.IP4F=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info4.IP4F);
			printk("rx_info4.TACK=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info4.TACK);
			printk("rx_info4.L4VLD=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info4.L4VLD);
			printk("rx_info4.L4F=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info4.L4F);
			printk("rx_info4.SPORT=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info4.SP);
			printk("rx_info4.CRSN=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info4.CRSN);
			printk("rx_info4.FOE_Entry=0x%x\n",
			       ei_local->rx_ring3[rx_dma_owner_idx0].
			       rxd_info4.FOE_Entry);
			printk("-------------------------------\n");
		}
	}
	if (pdma_dvt_get_lro_test_config() == PDMA_TEST_LRO_FORCE_AGGREGATE) {
		if (rx_ring_no == 1) {
			printk("PASS!!! => RING1: rxd_info1.PDP0=0x%x\n",
			       ei_local->rx_ring1[rx_dma_owner_idx0].
			       rxd_info1.PDP0);
			skb_dump(ei_local->netrx1_skbuf[rx_dma_owner_idx0]);
			pdma_dvt_reset_config();
		}
	}
}
#endif

int pdma_dvt_show_ctrl(int par1, int par2)
{
	if (par2 == 0)
		g_pdma_dvt_show_config = 0;
	else
		g_pdma_dvt_show_config |= (1 << par2);

	return 0;
}

int pdma_dvt_test_rx_ctrl(int par1, int par2)
{
	if (par2 == 0)
		g_pdma_dvt_rx_test_config = 0;
	else
		g_pdma_dvt_rx_test_config |= (1 << par2);

	return 0;
}

int pdma_dvt_test_tx_ctrl(int par1, int par2)
{
	if (par2 == 0)
		g_pdma_dvt_tx_test_config = 0;
	else
		g_pdma_dvt_tx_test_config |= (1 << par2);

	return 0;
}

int pdma_dvt_test_debug_ctrl(int par1, int par2)
{
	if (par2 == 0)
		g_pdma_dvt_debug_test_config = 0;
	else
		g_pdma_dvt_debug_test_config |= (1 << par2);

	return 0;
}

int pdma_dvt_test_lro_ctrl(int par1, int par2)
{
	g_pdma_dvt_lro_test_config = par2;

#if defined(CONFIG_RAETH_HW_LRO) || defined(CONFIG_RAETH_MULTIPLE_RX_RING)
	if (pdma_dvt_lro_func[par2])
		(*pdma_dvt_lro_func[par2]) ();
#endif /* #if defined (CONFIG_RAETH_HW_LRO) */

	return 0;
}

unsigned int pdma_dvt_get_show_config()
{
	return g_pdma_dvt_show_config;
}

unsigned int pdma_dvt_get_rx_test_config()
{
	return g_pdma_dvt_rx_test_config;
}

unsigned int pdma_dvt_get_tx_test_config()
{
	return g_pdma_dvt_tx_test_config;
}

unsigned int pdma_dvt_get_debug_test_config()
{
	return g_pdma_dvt_debug_test_config;
}

unsigned int pdma_dvt_get_lro_test_config()
{
	return g_pdma_dvt_lro_test_config;
}

void pdma_dvt_reset_config()
{
	g_pdma_dvt_show_config = 0;
	g_pdma_dvt_rx_test_config = 0;
	g_pdma_dvt_tx_test_config = 0;
	g_pdma_dvt_lro_test_config = 0;
}

void raeth_pdma_rx_desc_dvt(END_DEVICE *ei_local, int rx_dma_owner_idx0)
{
#if 0
	unsigned int udf = 0;
#endif

	if (pdma_dvt_get_show_config() & PDMA_SHOW_RX_DESC) {
		printk("------- eth_recv --------\n");
		printk("rx_info1=0x%x\n",
		       *(unsigned int *)&ei_local->
		       rx_ring0[rx_dma_owner_idx0].rxd_info1);
		printk("rx_info2=0x%x\n",
		       *(unsigned int *)&ei_local->
		       rx_ring0[rx_dma_owner_idx0].rxd_info2);
		printk("rx_info3=0x%x\n",
		       *(unsigned int *)&ei_local->
		       rx_ring0[rx_dma_owner_idx0].rxd_info3);
		printk("rx_info4=0x%x\n",
		       *(unsigned int *)&ei_local->
		       rx_ring0[rx_dma_owner_idx0].rxd_info4);
		printk("-------------------------------\n");
	}
	if ((pdma_dvt_get_show_config() & PDMA_SHOW_DETAIL_RX_DESC) ||
	    pdma_dvt_get_rx_test_config()) {
		printk("------- eth_recv --------\n");
		printk("rx_info1.PDP0=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info1.PDP0);
		printk("rx_info2.DDONE_bit=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].
		       rxd_info2.DDONE_bit);
		printk("rx_info2.LS0=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info2.LS0);
		printk("rx_info2.PLEN0=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info2.PLEN0);
		printk("rx_info2.TAG=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info2.TAG);
#if defined(CONFIG_ARCH_MT8590)
		printk("rx_info2.LRO_AGG_CNT=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].
		       rxd_info2.LRO_AGG_CNT);
#else
		printk("rx_info2.LS1=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info2.LS1);
#endif /* CONFIG_RAETH_HW_LRO */
		printk("rx_info2.PLEN1=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info2.PLEN1);
		printk("rx_info3.TPID=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info3.TPID);
		printk("rx_info3.VID=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info3.VID);
#if 0
		printk("rx_info4.UDF=0x%x\n", udf);
#endif
		printk("rx_info4.IP6=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.IP6);
		printk("rx_info4.IP4=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.IP4);
		printk("rx_info4.IP4F=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.IP4F);
		printk("rx_info4.TACK=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.TACK);
		printk("rx_info4.L4VLD=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.L4VLD);
		printk("rx_info4.L4F=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.L4F);
		printk("rx_info4.SPORT=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.SP);
		printk("rx_info4.CRSN=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.CRSN);
		printk("rx_info4.FOE_Entry=0x%x\n",
		       ei_local->rx_ring0[rx_dma_owner_idx0].
		       rxd_info4.FOE_Entry);
		printk("-------------------------------\n");
	}
	if ((pdma_dvt_get_rx_test_config() & PDMA_TEST_RX_IPV6)) {
		if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.IP6) {
			printk("PASS!!! => rx_info4.IP6=0x%x\n",
			       ei_local->rx_ring0[rx_dma_owner_idx0].
			       rxd_info4.IP6);
			pdma_dvt_reset_config();
		}
	} else if ((pdma_dvt_get_rx_test_config() & PDMA_TEST_RX_IPV4)) {
		if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.IP4) {
			printk("PASS!!! => rx_info4.IP4=0x%x\n",
			       ei_local->rx_ring0[rx_dma_owner_idx0].
			       rxd_info4.IP4);
			pdma_dvt_reset_config();
		}
	} else if ((pdma_dvt_get_rx_test_config() & PDMA_TEST_RX_IPV4F)) {
		if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.IP4F) {
			printk("PASS!!! => rx_info4.IP4F=0x%x\n",
			       ei_local->rx_ring0[rx_dma_owner_idx0].
			       rxd_info4.IP4F);
			pdma_dvt_reset_config();
		}
	} else if ((pdma_dvt_get_rx_test_config() & PDMA_TEST_RX_L4VLD)) {
		if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.L4VLD) {
			printk("PASS!!! => rx_info4.L4VLD=0x%x\n",
			       ei_local->rx_ring0[rx_dma_owner_idx0].
			       rxd_info4.L4VLD);
			pdma_dvt_reset_config();
		}
	} else if ((pdma_dvt_get_rx_test_config() & PDMA_TEST_RX_L4F)) {
		if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.L4F) {
			printk("PASS!!! => rx_info4.L4F=0x%x\n",
			       ei_local->rx_ring0[rx_dma_owner_idx0].
			       rxd_info4.L4F);
			pdma_dvt_reset_config();
		}
	} else if ((pdma_dvt_get_rx_test_config() & PDMA_TEST_RX_SPORT)) {
		if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.SP == 1) {
			g_pdma_dev_lanport++;
		} else if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info4.SP ==
			   2) {
			g_pdma_dev_wanport++;
		}
		if (g_pdma_dev_lanport && g_pdma_dev_wanport) {
			printk
			    ("PASS!!! => g_pdma_dev_lanport=0x%x, g_pdma_dev_wanport=0x%x",
			     g_pdma_dev_lanport, g_pdma_dev_wanport);

			g_pdma_dev_lanport = 0;
			g_pdma_dev_wanport = 0;
			pdma_dvt_reset_config();
		}
	} else if ((pdma_dvt_get_rx_test_config() & PDMA_TEST_RX_VID_OFF)) {
		if (!ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info3.VID) {
			printk("PASS!!! => rxd_info3.VID=0x%x\n",
			       ei_local->rx_ring0[rx_dma_owner_idx0].
			       rxd_info3.VID);
			pdma_dvt_reset_config();
		}
	} else if ((pdma_dvt_get_rx_test_config() & PDMA_TEST_RX_VID_ON)) {
		printk("RX data: (PDP0=%x)\n",
		       (unsigned int)ei_local->
		       netrx0_skbuf[rx_dma_owner_idx0]->data);

		skb_dump(ei_local->netrx0_skbuf[rx_dma_owner_idx0]);

		if (ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info3.VID &&
		    ei_local->rx_ring0[rx_dma_owner_idx0].rxd_info2.TAG) {
			printk("PASS!!! => rxd_info2.TAG=0x%x\n",
			       ei_local->rx_ring0[rx_dma_owner_idx0].
			       rxd_info2.TAG);
			printk("PASS!!! => rxd_info3.VID=0x%x\n",
			       ei_local->rx_ring0[rx_dma_owner_idx0].
			       rxd_info3.VID);
			pdma_dvt_reset_config();
		}
	}
}

void raeth_pdma_tx_vlan_dvt(END_DEVICE *ei_local,
			    unsigned long tx_cpu_owner_idx0)
{
	if ((pdma_dvt_get_tx_test_config() & PDMA_TEST_TX_VLAN_ON)) {
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG = 0x10000 | 0xE007;	/* VLAN_TAG = 0x1E007 */
	} else if ((pdma_dvt_get_tx_test_config() & PDMA_TEST_TX_VLAN_ZERO)) {
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG = 0x10000 | 0x0000;	/* VLAN_TAG = 0x10000 */
	} else if ((pdma_dvt_get_tx_test_config() & PDMA_TEST_TX_VLAN_MAX)) {
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG = 0x10000 | 0xFFFF;	/* VLAN_TAG = 0x1FFFF */
	}
}

void raeth_pdma_tx_desc_dvt(END_DEVICE *ei_local,
			    unsigned long tx_cpu_owner_idx0)
{
	if (PDMA_TEST_RX_UDF == pdma_dvt_get_rx_test_config()) {
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FPORT = 4;	/* PPE */
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.UDF = 0x2F;
	}
	if (pdma_dvt_get_show_config() & PDMA_SHOW_TX_DESC) {
		printk("------- raeth_send --------\n");
		printk("tx_info1=%x\n",
		       *(unsigned int *)&ei_local->
		       tx_ring0[tx_cpu_owner_idx0].txd_info1);
		printk("tx_info2=%x\n",
		       *(unsigned int *)&ei_local->
		       tx_ring0[tx_cpu_owner_idx0].txd_info2);
		printk("tx_info3=%x\n",
		       *(unsigned int *)&ei_local->
		       tx_ring0[tx_cpu_owner_idx0].txd_info3);
		printk("tx_info4=%x\n",
		       *(unsigned int *)&ei_local->
		       tx_ring0[tx_cpu_owner_idx0].txd_info4);
		printk("--------------------------------\n");
	}
	if ((pdma_dvt_get_show_config() & PDMA_SHOW_DETAIL_TX_DESC) ||
	    pdma_dvt_get_tx_test_config()) {
		printk("------- raeth_send --------\n");
		printk("tx_info1.SDP0=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0);
		printk("tx_info2.DDONE_bit=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].
		       txd_info2.DDONE_bit);
		printk("tx_info2.LS0_bit=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.LS0_bit);
		printk("tx_info2.SDL0=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0);
		printk("tx_info2.BURST_bit=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].
		       txd_info2.BURST_bit);
		printk("tx_info2.LS1_bit=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.LS1_bit);
		printk("tx_info2.SDL1=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL1);
		printk("tx_info3.SDP1=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info3.SDP1);
		printk("tx_info4.TUI_CO=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.TUI_CO);
		printk("tx_info4.TSO=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.TSO);
		printk("tx_info4.FPORT=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FPORT);
		printk("tx_info4.UDF=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.UDF);
		printk("tx_info4.RESV=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.RESV);
		printk("tx_info4.VLAN_TAG=%x\n",
		       ei_local->tx_ring0[tx_cpu_owner_idx0].
		       txd_info4.VLAN_TAG);
		printk("--------------------------------\n");
	}
	if ((pdma_dvt_get_tx_test_config() & PDMA_TEST_TX_LAN_SPORT)) {
		if (ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FPORT == 1) {
			printk("PASS!!! => txd_info4.FPORT=0x%x\n",
			       ei_local->tx_ring0[tx_cpu_owner_idx0].
			       txd_info4.FPORT);
			pdma_dvt_reset_config();
		}
	} else if ((pdma_dvt_get_tx_test_config() & PDMA_TEST_TX_WAN_SPORT)) {
		if (ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FPORT == 2) {
			printk("PASS!!! => txd_info4.FPORT=0x%x\n",
			       ei_local->tx_ring0[tx_cpu_owner_idx0].
			       txd_info4.FPORT);
			pdma_dvt_reset_config();
		}
	} else if ((pdma_dvt_get_tx_test_config() & PDMA_TEST_TX_VLAN_ON)) {
		if (ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG) {
			printk("PASS!!! => txd_info4.VLAN_TAG=0x%x\n",
			       ei_local->tx_ring0[tx_cpu_owner_idx0].
			       txd_info4.VLAN_TAG);
			/* pdma_dvt_reset_config(); */
		}
	} else if ((pdma_dvt_get_tx_test_config() & PDMA_TEST_TX_VLAN_OFF)) {
		if (!ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG) {
			printk("PASS!!! => txd_info4.VLAN_TAG=0x%x\n",
			       ei_local->tx_ring0[tx_cpu_owner_idx0].
			       txd_info4.VLAN_TAG);
			pdma_dvt_reset_config();
		}
	} else if ((pdma_dvt_get_tx_test_config() & PDMA_TEST_TX_VLAN_ZERO)) {
		if (ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG) {
			printk("PASS!!! => txd_info4.VLAN_TAG=0x%x\n",
			       ei_local->tx_ring0[tx_cpu_owner_idx0].
			       txd_info4.VLAN_TAG);
			/* pdma_dvt_reset_config(); */
		}
	} else if ((pdma_dvt_get_tx_test_config() & PDMA_TEST_TX_VLAN_MAX)) {
		if (ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG) {
			printk("PASS!!! => txd_info4.VLAN_TAG=0x%x\n",
			       ei_local->tx_ring0[tx_cpu_owner_idx0].
			       txd_info4.VLAN_TAG);
			/* pdma_dvt_reset_config(); */
		}
	}
}

void raeth_pdma_lro_dly_int_dvt(void)
{
	unsigned int reg_int_val;

	reg_int_val = sysRegRead(RAETH_FE_INT_STATUS);

	if (pdma_dvt_get_lro_test_config() == PDMA_TEST_LRO_DLY_INT0) {
		if ((reg_int_val & RX_DLY_INT)) {
			printk("PASS!!! => reg_int_val=0x%x\n", reg_int_val);
			pdma_dvt_reset_config();
		}
	} else if (pdma_dvt_get_lro_test_config() == PDMA_TEST_LRO_DLY_INT1) {
		if ((reg_int_val & RING1_RX_DLY_INT)) {
			printk("PASS!!! => reg_int_val=0x%x\n", reg_int_val);
			pdma_dvt_reset_config();
		}
	} else if (pdma_dvt_get_lro_test_config() == PDMA_TEST_LRO_DLY_INT2) {
		if ((reg_int_val & RING2_RX_DLY_INT)) {
			printk("PASS!!! => reg_int_val=0x%x\n", reg_int_val);
			pdma_dvt_reset_config();
		}
	} else if (pdma_dvt_get_lro_test_config() == PDMA_TEST_LRO_DLY_INT3) {
		if ((reg_int_val & RING3_RX_DLY_INT)) {
			printk("PASS!!! => reg_int_val=0x%x\n", reg_int_val);
			pdma_dvt_reset_config();
		}
	}
}

void pdma_dvt_set_dma_mode(void)
{
#if defined(CONFIG_RAETH_PDMA_LEGACY_MODE)
	unsigned int regVal;
	regVal = sysRegRead(ADMA_LRO_CTRL_DW3);
	regVal &= ~(BIT(15));
	sysRegWrite(ADMA_LRO_CTRL_DW3, regVal);
#endif  /* CONFIG_RAETH_PDMA_DVT */	
}

