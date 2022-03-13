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
/* Show controls */
#define PDMA_SHOW_RX_DESC   (1 << 1)
#define PDMA_SHOW_TX_DESC   (1 << 2)
#define PDMA_SHOW_DETAIL_RX_DESC   (1 << 3)
#define PDMA_SHOW_DETAIL_TX_DESC   (1 << 4)

/* Rx test controls */
#define PDMA_TEST_RX_UDF     (1 << 1)
#define PDMA_TEST_RX_IPV6    (1 << 2)
#define PDMA_TEST_RX_IPV4    (1 << 3)
#define PDMA_TEST_RX_IPV4F   (1 << 4)
#define PDMA_TEST_RX_L4VLD   (1 << 5)
#define PDMA_TEST_RX_L4F     (1 << 6)
#define PDMA_TEST_RX_SPORT   (1 << 7)
#define PDMA_TEST_RX_VID_ON  (1 << 8)
#define PDMA_TEST_RX_VID_OFF (1 << 9)

/* Tx test controls */
#define PDMA_TEST_TX_LAN_SPORT   (1 << 1)
#define PDMA_TEST_TX_WAN_SPORT   (1 << 2)
#define PDMA_TEST_TX_VLAN_ON     (1 << 3)
#define PDMA_TEST_TX_VLAN_OFF    (1 << 4)
#define PDMA_TEST_TX_VLAN_ZERO   (1 << 5)
#define PDMA_TEST_TX_VLAN_MAX    (1 << 6)
#define PDMA_TEST_TX_PDMA_LPK    (1 << 31)

/* Debug controls */
#define PDMA_TEST_TSO_DEBUG      (1 << 1)

/* LRO test controls */
typedef int (*PDMA_LRO_DVT_FUNC) (void);

#define PDMA_TEST_LRO_DISABLE           (0)
#define PDMA_TEST_LRO_FORCE_PORT        (1)
#define PDMA_TEST_LRO_AUTO_LEARN        (2)
#define PDMA_TEST_LRO_AUTO_IPV6         (3)
#define PDMA_TEST_LRO_AUTO_MYIP         (4)
#define PDMA_TEST_LRO_FORCE_AGGREGATE   (5)
#define PDMA_TEST_NON_LRO_PORT_ID       (6)
#define PDMA_TEST_NON_LRO_STAG          (7)
#define PDMA_TEST_NON_LRO_VLAN          (8)
#define PDMA_TEST_NON_LRO_TCP_ACK       (9)
#define PDMA_TEST_NON_LRO_PRI1          (10)
#define PDMA_TEST_NON_LRO_PRI2          (11)
#define PDMA_TEST_LRO_DLY_INT0          (12)
#define PDMA_TEST_LRO_DLY_INT1          (13)
#define PDMA_TEST_LRO_DLY_INT2          (14)
#define PDMA_TEST_LRO_DLY_INT3          (15)

void skb_dump(struct sk_buff *sk);

int pdma_dvt_show_ctrl(int par1, int par2);
int pdma_dvt_test_rx_ctrl(int par1, int par2);
int pdma_dvt_test_tx_ctrl(int par1, int par2);
int pdma_dvt_test_debug_ctrl(int par1, int par2);
int pdma_dvt_test_lro_ctrl(int par1, int par2);

unsigned int pdma_dvt_get_show_config(void);
unsigned int pdma_dvt_get_rx_test_config(void);
unsigned int pdma_dvt_get_tx_test_config(void);
unsigned int pdma_dvt_get_debug_test_config(void);
unsigned int pdma_dvt_get_lro_test_config(void);
void pdma_dvt_reset_config(void);

void raeth_pdma_rx_desc_dvt(END_DEVICE *ei_local, int rx_dma_owner_idx0);
void raeth_pdma_tx_vlan_dvt(END_DEVICE *ei_local,
			    unsigned long tx_cpu_owner_idx0);
void raeth_pdma_tx_desc_dvt(END_DEVICE *ei_local,
			    unsigned long tx_cpu_owner_idx0);

void raeth_pdma_lro_dvt(int rx_ring_no, END_DEVICE *ei_local,
			int rx_dma_owner_idx0);
void raeth_pdma_lro_dly_int_dvt(void);
void pdma_dvt_set_dma_mode(void);

