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
obj-$(CPTCFG_ZD1211RW) += zd1211rw.o

zd1211rw-objs := zd_chip.o zd_mac.o \
		zd_rf_al2230.o zd_rf_rf2959.o \
		zd_rf_al7230b.o zd_rf_uw2453.o \
		zd_rf.o zd_usb.o

ccflags-$(CPTCFG_ZD1211RW_DEBUG) := -DDEBUG

