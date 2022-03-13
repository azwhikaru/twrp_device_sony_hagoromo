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
obj-pvrusb2-sysfs-$(CPTCFG_VIDEO_PVRUSB2_SYSFS) := pvrusb2-sysfs.o
obj-pvrusb2-debugifc-$(CPTCFG_VIDEO_PVRUSB2_DEBUGIFC) := pvrusb2-debugifc.o
obj-pvrusb2-dvb-$(CPTCFG_VIDEO_PVRUSB2_DVB) := pvrusb2-dvb.o

pvrusb2-objs	:= pvrusb2-i2c-core.o \
		   pvrusb2-audio.o \
		   pvrusb2-encoder.o pvrusb2-video-v4l.o \
		   pvrusb2-eeprom.o \
		   pvrusb2-main.o pvrusb2-hdw.o pvrusb2-v4l2.o \
		   pvrusb2-ctrl.o pvrusb2-std.o pvrusb2-devattr.o \
		   pvrusb2-context.o pvrusb2-io.o pvrusb2-ioread.o \
		   pvrusb2-cx2584x-v4l.o pvrusb2-wm8775.o \
		   pvrusb2-cs53l32a.o \
		   $(obj-pvrusb2-dvb-y) \
		   $(obj-pvrusb2-sysfs-y) $(obj-pvrusb2-debugifc-y)

obj-$(CPTCFG_VIDEO_PVRUSB2) += pvrusb2.o

ccflags-y += -I$(backport_srctree)/drivers/media/i2c
ccflags-y += -I$(backport_srctree)/drivers/media/tuners
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-frontends
