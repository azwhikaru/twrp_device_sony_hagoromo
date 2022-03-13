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
obj-$(CPTCFG_VIDEO_GO7007) += go7007.o
obj-$(CPTCFG_VIDEO_GO7007_USB) += go7007-usb.o
obj-$(CPTCFG_VIDEO_GO7007_LOADER) += go7007-loader.o
obj-$(CPTCFG_VIDEO_GO7007_USB_S2250_BOARD) += s2250.o

go7007-y := go7007-v4l2.o go7007-driver.o go7007-i2c.o go7007-fw.o \
		snd-go7007.o

s2250-y := s2250-board.o

ccflags-$(CPTCFG_VIDEO_GO7007_LOADER:m=y) += -I$(backport_srctree)/drivers/media/common
