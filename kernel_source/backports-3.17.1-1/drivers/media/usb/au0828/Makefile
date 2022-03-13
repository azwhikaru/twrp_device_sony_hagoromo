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
au0828-objs	:= au0828-core.o au0828-i2c.o au0828-cards.o au0828-dvb.o

ifeq ($(CPTCFG_VIDEO_AU0828_V4L2),y)
  au0828-objs   += au0828-video.o au0828-vbi.o
endif

ifeq ($(CPTCFG_VIDEO_AU0828_RC),y)
  au0828-objs   += au0828-input.o
endif

obj-$(CPTCFG_VIDEO_AU0828) += au0828.o

ccflags-y += -I$(backport_srctree)/drivers/media/tuners
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-frontends

ccflags-y += $(extra-cflags-y) $(extra-cflags-m)
