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
cx18-objs    := cx18-driver.o cx18-cards.o cx18-i2c.o cx18-firmware.o cx18-gpio.o \
	cx18-queue.o cx18-streams.o cx18-fileops.o cx18-ioctl.o cx18-controls.o \
	cx18-mailbox.o cx18-vbi.o cx18-audio.o cx18-video.o cx18-irq.o \
	cx18-av-core.o cx18-av-audio.o cx18-av-firmware.o cx18-av-vbi.o cx18-scb.o \
	cx18-dvb.o cx18-io.o
cx18-alsa-objs := cx18-alsa-main.o cx18-alsa-pcm.o

obj-$(CPTCFG_VIDEO_CX18) += cx18.o
obj-$(CPTCFG_VIDEO_CX18_ALSA) += cx18-alsa.o

ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-frontends
ccflags-y += -I$(backport_srctree)/drivers/media/tuners
