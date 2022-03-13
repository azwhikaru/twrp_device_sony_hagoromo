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
# Makefile for the Linux NFC subsystem.
#

obj-$(CPTCFG_NFC) += nfc.o
obj-$(CPTCFG_NFC_NCI) += nci/
obj-$(CPTCFG_NFC_HCI) += hci/
obj-$(CPTCFG_NFC_DIGITAL) += nfc_digital.o

nfc-objs := core.o netlink.o af_nfc.o rawsock.o llcp_core.o llcp_commands.o \
		llcp_sock.o

nfc_digital-objs := digital_core.o digital_technology.o digital_dep.o
