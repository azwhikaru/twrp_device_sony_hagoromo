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
# b43legacy core
b43legacy-y				+= main.o
b43legacy-y				+= ilt.o
b43legacy-y				+= phy.o
b43legacy-y				+= radio.o
b43legacy-y				+= sysfs.o
b43legacy-y				+= xmit.o
# b43 RFKILL button support
b43legacy-y				+= rfkill.o
# b43legacy LED support
b43legacy-$(CPTCFG_B43LEGACY_LEDS)	+= leds.o
# b43legacy debugging
b43legacy-$(CPTCFG_B43LEGACY_DEBUG)	+= debugfs.o
# b43legacy DMA and PIO
b43legacy-$(CPTCFG_B43LEGACY_DMA)	+= dma.o
b43legacy-$(CPTCFG_B43LEGACY_PIO)	+= pio.o

obj-$(CPTCFG_B43LEGACY)			+= b43legacy.o

