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
obj-$(CPTCFG_USB_GL860) += gspca_gl860.o

gspca_gl860-objs := gl860.o \
		    gl860-mi1320.o \
		    gl860-ov2640.o \
		    gl860-ov9655.o \
		    gl860-mi2020.o

ccflags-y += -I$(backport_srctree)/drivers/media/usb/gspca

