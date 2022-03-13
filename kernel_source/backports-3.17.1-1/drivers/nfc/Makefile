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
# Makefile for nfc devices
#

obj-$(CPTCFG_NFC_PN544)		+= pn544/
obj-$(CPTCFG_NFC_MICROREAD)	+= microread/
obj-$(CPTCFG_NFC_PN533)		+= pn533.o
obj-$(CPTCFG_NFC_WILINK)	+= nfcwilink.o
obj-$(CPTCFG_NFC_MEI_PHY)	+= mei_phy.o
obj-$(CPTCFG_NFC_SIM)		+= nfcsim.o
obj-$(CPTCFG_NFC_PORT100)	+= port100.o
obj-$(CPTCFG_NFC_MRVL)		+= nfcmrvl/
obj-$(CPTCFG_NFC_TRF7970A)	+= trf7970a.o
obj-$(CPTCFG_NFC_ST21NFCA)  	+= st21nfca/
obj-$(CPTCFG_NFC_ST21NFCB)	+= st21nfcb/

ccflags-$(CONFIG_NFC_DEBUG) := -DDEBUG
