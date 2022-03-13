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
cx88xx-objs	:= cx88-cards.o cx88-core.o cx88-i2c.o cx88-tvaudio.o \
		   cx88-dsp.o cx88-input.o
cx8800-objs	:= cx88-video.o cx88-vbi.o
cx8802-objs	:= cx88-mpeg.o

obj-$(CPTCFG_VIDEO_CX88) += cx88xx.o cx8800.o
obj-$(CPTCFG_VIDEO_CX88_MPEG) += cx8802.o
obj-$(CPTCFG_VIDEO_CX88_ALSA) += cx88-alsa.o
obj-$(CPTCFG_VIDEO_CX88_BLACKBIRD) += cx88-blackbird.o
obj-$(CPTCFG_VIDEO_CX88_DVB) += cx88-dvb.o
obj-$(CPTCFG_VIDEO_CX88_VP3054) += cx88-vp3054-i2c.o

ccflags-y += -I$(backport_srctree)/drivers/media/i2c
ccflags-y += -I$(backport_srctree)/drivers/media/common
ccflags-y += -I$(backport_srctree)/drivers/media/tuners
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-frontends
