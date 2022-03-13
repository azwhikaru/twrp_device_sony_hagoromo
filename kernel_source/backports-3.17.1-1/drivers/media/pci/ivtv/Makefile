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
ivtv-objs	:= ivtv-routing.o ivtv-cards.o ivtv-controls.o \
		   ivtv-driver.o ivtv-fileops.o ivtv-firmware.o \
		   ivtv-gpio.o ivtv-i2c.o ivtv-ioctl.o ivtv-irq.o \
		   ivtv-mailbox.o ivtv-queue.o ivtv-streams.o ivtv-udma.o \
		   ivtv-vbi.o ivtv-yuv.o
ivtv-alsa-objs := ivtv-alsa-main.o ivtv-alsa-pcm.o

obj-$(CPTCFG_VIDEO_IVTV) += ivtv.o
obj-$(CPTCFG_VIDEO_IVTV_ALSA) += ivtv-alsa.o
obj-$(CPTCFG_VIDEO_FB_IVTV) += ivtvfb.o

ccflags-y += -I$(backport_srctree)/drivers/media/i2c
ccflags-y += -I$(backport_srctree)/drivers/media/tuners
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-frontends

