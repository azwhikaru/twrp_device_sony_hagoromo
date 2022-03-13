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
ath5k-y				+= caps.o
ath5k-y				+= initvals.o
ath5k-y				+= eeprom.o
ath5k-y				+= gpio.o
ath5k-y				+= desc.o
ath5k-y				+= dma.o
ath5k-y				+= qcu.o
ath5k-y				+= pcu.o
ath5k-y				+= phy.o
ath5k-y				+= reset.o
ath5k-y				+= attach.o
ath5k-y				+= base.o
CFLAGS_base.o			+= -I$(src)
ath5k-y				+= led.o
ath5k-y				+= rfkill.o
ath5k-y				+= ani.o
ath5k-y				+= sysfs.o
ath5k-y				+= mac80211-ops.o
ath5k-$(CPTCFG_ATH5K_DEBUG)	+= debug.o
ath5k-$(CPTCFG_ATH5K_AHB)	+= ahb.o
ath5k-$(CPTCFG_ATH5K_PCI)	+= pci.o
obj-$(CPTCFG_ATH5K)		+= ath5k.o
