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
cx25840-objs    := cx25840-core.o cx25840-audio.o cx25840-firmware.o \
		   cx25840-vbi.o cx25840-ir.o

obj-$(CPTCFG_VIDEO_CX25840) += cx25840.o

ccflags-y += -I$(backport_srctree)/drivers/media/i2c
