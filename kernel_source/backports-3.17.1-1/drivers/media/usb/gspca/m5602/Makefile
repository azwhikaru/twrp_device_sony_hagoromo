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
obj-$(CPTCFG_USB_M5602) += gspca_m5602.o

gspca_m5602-objs := m5602_core.o \
		    m5602_ov9650.o \
		    m5602_ov7660.o \
		    m5602_mt9m111.o \
		    m5602_po1030.o \
		    m5602_s5k83a.o \
		    m5602_s5k4aa.o

ccflags-y += -I$(backport_srctree)/drivers/media/usb/gspca
