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
# DVM
obj-$(CPTCFG_IWLDVM)	+= iwldvm.o
iwldvm-objs		+= main.o rs.o mac80211.o ucode.o tx.o
iwldvm-objs		+= lib.o calib.o tt.o sta.o rx.o

iwldvm-objs		+= power.o
iwldvm-objs		+= scan.o
iwldvm-objs		+= rxon.o devices.o

iwldvm-$(CPTCFG_IWLWIFI_LEDS) += led.o
iwldvm-$(CPTCFG_IWLWIFI_DEBUGFS) += debugfs.o

ccflags-y += -D__CHECK_ENDIAN__ -I$(src)/../
