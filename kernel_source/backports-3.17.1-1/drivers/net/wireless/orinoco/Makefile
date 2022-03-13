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
# Makefile for the orinoco wireless device drivers.
#
orinoco-objs := main.o fw.o hw.o mic.o scan.o wext.o hermes_dld.o hermes.o cfg.o

obj-$(CPTCFG_HERMES)		+= orinoco.o
obj-$(CPTCFG_PCMCIA_HERMES)	+= orinoco_cs.o
obj-$(CPTCFG_APPLE_AIRPORT)	+= airport.o
obj-$(CPTCFG_PLX_HERMES)	+= orinoco_plx.o
obj-$(CPTCFG_PCI_HERMES)	+= orinoco_pci.o
obj-$(CPTCFG_TMD_HERMES)	+= orinoco_tmd.o
obj-$(CPTCFG_NORTEL_HERMES)	+= orinoco_nortel.o
obj-$(CPTCFG_PCMCIA_SPECTRUM)	+= spectrum_cs.o
obj-$(CPTCFG_ORINOCO_USB)	+= orinoco_usb.o

# Orinoco should be endian clean.
ccflags-y += -D__CHECK_ENDIAN__
