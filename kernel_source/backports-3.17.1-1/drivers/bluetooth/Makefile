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
#
# Makefile for the Linux Bluetooth HCI device drivers.
#

obj-$(CPTCFG_BT_HCIVHCI)	+= hci_vhci.o
obj-$(CPTCFG_BT_HCIUART)	+= hci_uart.o
obj-$(CPTCFG_BT_HCIBCM203X)	+= bcm203x.o
obj-$(CPTCFG_BT_HCIBPA10X)	+= bpa10x.o
obj-$(CPTCFG_BT_HCIBFUSB)	+= bfusb.o
obj-$(CPTCFG_BT_HCIDTL1)	+= dtl1_cs.o
obj-$(CPTCFG_BT_HCIBT3C)	+= bt3c_cs.o
obj-$(CPTCFG_BT_HCIBLUECARD)	+= bluecard_cs.o
obj-$(CPTCFG_BT_HCIBTUART)	+= btuart_cs.o

obj-$(CPTCFG_BT_HCIBTUSB)	+= btusb.o
obj-$(CPTCFG_BT_HCIBTSDIO)	+= btsdio.o

obj-$(CPTCFG_BT_ATH3K)		+= ath3k.o
obj-$(CPTCFG_BT_MRVL)		+= btmrvl.o
obj-$(CPTCFG_BT_MRVL_SDIO)	+= btmrvl_sdio.o
obj-$(CPTCFG_BT_WILINK)		+= btwilink.o

btmrvl-y			:= btmrvl_main.o
btmrvl-$(CONFIG_DEBUG_FS)	+= btmrvl_debugfs.o

hci_uart-y				:= hci_ldisc.o
hci_uart-$(CPTCFG_BT_HCIUART_H4)	+= hci_h4.o
hci_uart-$(CPTCFG_BT_HCIUART_BCSP)	+= hci_bcsp.o
hci_uart-$(CPTCFG_BT_HCIUART_LL)	+= hci_ll.o
hci_uart-$(CPTCFG_BT_HCIUART_ATH3K)	+= hci_ath.o
hci_uart-$(CPTCFG_BT_HCIUART_3WIRE)	+= hci_h5.o
hci_uart-objs				:= $(hci_uart-y)

ccflags-y += -D__CHECK_ENDIAN__
