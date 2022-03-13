#
# Copyright (C) 2011-2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify it under the terms of the
# GNU General Public License version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with this program.
# If not, see <http://www.gnu.org/licenses/>.
#
obj-$(CONFIG_RAETH) += raeth.o
raeth-objs := ra_mac.o mii_mgr.o

ifeq ($(CONFIG_MTK_SMB_HOOK),y)
obj-y += smb_hook.o
obj-m += smb.o
smb-objs := smb_nf.o
endif

#EXTRA_CFLAGS   += -DCONFIG_RAETH_MULTIPLE_RX_RING

ifeq ($(CONFIG_RAETH_QOS),y)
raeth-objs += ra_qos.o
endif

ifeq ($(CONFIG_RAETH_QDMA),y)
raeth-objs += raether_qdma.o
endif

ifneq ($(CONFIG_RAETH_QDMA),y)
raeth-objs += raether_pdma.o
endif

raeth-objs += raether.o

ifeq ($(CONFIG_ETHTOOL),y)
raeth-objs += ra_ethtool.o
endif

ifeq ($(CONFIG_RAETH_PDMA_DVT),y)
raeth-objs += dvt/raether_pdma_dvt.o
obj-m += dvt/pkt_gen.o
obj-m += dvt/pkt_gen_udp_frag.o
obj-m += dvt/pkt_gen_tcp_frag.o
endif

ifeq ($(CONFIG_RAETH_HW_LRO),y)
raeth-objs += raether_hwlro.o
endif

ifeq ($(CONFIG_RAETH_GMAC2),y)
EXTRA_CFLAGS += -DCONFIG_PSEUDO_SUPPORT
endif

ifeq ($(CONFIG_ETH_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS   += -DMEMORY_OPTIMIZATION
endif

ifeq ($(CONFIG_RT2860V2_AP_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS   += -DMEMORY_OPTIMIZATION
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS   += -DWORKQUEUE_BH
endif

ifeq ($(CONFIG_TASKLET_WORKQUEUE_SW),y)
EXTRA_CFLAGS   += -DTASKLET_WORKQUEUE_SW
endif

raeth-objs += ra_phy.o
