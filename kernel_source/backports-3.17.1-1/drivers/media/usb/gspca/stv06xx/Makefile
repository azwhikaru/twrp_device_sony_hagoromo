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
obj-$(CPTCFG_USB_STV06XX) += gspca_stv06xx.o

gspca_stv06xx-objs := stv06xx.o \
		      stv06xx_vv6410.o \
		      stv06xx_hdcs.o \
		      stv06xx_pb0100.o \
		      stv06xx_st6422.o

ccflags-y += -I$(backport_srctree)/drivers/media/usb/gspca

