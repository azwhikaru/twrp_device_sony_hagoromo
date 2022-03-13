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
cx25821-y   := cx25821-core.o cx25821-cards.o cx25821-i2c.o \
		       cx25821-gpio.o cx25821-medusa-video.o \
		       cx25821-video.o cx25821-video-upstream.o

obj-$(CPTCFG_VIDEO_CX25821) += cx25821.o
obj-$(CPTCFG_VIDEO_CX25821_ALSA) += cx25821-alsa.o

ccflags-y += -I$(backport_srctree)/drivers/media/i2c
ccflags-y += -I$(backport_srctree)/drivers/media/common
