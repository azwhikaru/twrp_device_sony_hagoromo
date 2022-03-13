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
# Makefile for the Linux Bluetooth subsystem.
#

obj-$(CPTCFG_BT)	+= bluetooth.o
obj-$(CPTCFG_BT_RFCOMM)	+= rfcomm/
obj-$(CPTCFG_BT_BNEP)	+= bnep/
obj-$(CPTCFG_BT_CMTP)	+= cmtp/
obj-$(CPTCFG_BT_HIDP)	+= hidp/
obj-$(CPTCFG_BT_6LOWPAN) += bluetooth_6lowpan.o

bluetooth_6lowpan-y := 6lowpan.o

bluetooth-y := af_bluetooth.o hci_core.o hci_conn.o hci_event.o mgmt.o \
	hci_sock.o hci_sysfs.o l2cap_core.o l2cap_sock.o smp.o sco.o lib.o \
	a2mp.o amp.o

subdir-ccflags-y += -D__CHECK_ENDIAN__
