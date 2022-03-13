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
rsi_91x-y			+= rsi_91x_main.o
rsi_91x-y			+= rsi_91x_core.o
rsi_91x-y			+= rsi_91x_mac80211.o
rsi_91x-y			+= rsi_91x_mgmt.o
rsi_91x-y			+= rsi_91x_pkt.o
rsi_91x-$(CPTCFG_RSI_DEBUGFS)	+= rsi_91x_debugfs.o

rsi_usb-y			+= rsi_91x_usb.o rsi_91x_usb_ops.o
rsi_sdio-y			+= rsi_91x_sdio.o rsi_91x_sdio_ops.o
obj-$(CPTCFG_RSI_91X) 		+= rsi_91x.o
obj-$(CPTCFG_RSI_SDIO)		+= rsi_sdio.o
obj-$(CPTCFG_RSI_USB)		+= rsi_usb.o
