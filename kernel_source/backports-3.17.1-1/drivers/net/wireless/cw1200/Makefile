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
cw1200_core-y := \
		fwio.o \
		txrx.o \
		main.o \
		queue.o \
		hwio.o \
		bh.o \
		wsm.o \
		sta.o \
		scan.o \
		debug.o
cw1200_core-$(CONFIG_PM)	+= pm.o

# CFLAGS_sta.o += -DDEBUG

cw1200_wlan_sdio-y := cw1200_sdio.o
cw1200_wlan_spi-y := cw1200_spi.o

obj-$(CPTCFG_CW1200) += cw1200_core.o
obj-$(CPTCFG_CW1200_WLAN_SDIO) += cw1200_wlan_sdio.o
obj-$(CPTCFG_CW1200_WLAN_SPI) += cw1200_wlan_spi.o
