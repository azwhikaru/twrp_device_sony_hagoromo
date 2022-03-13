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
obj-stk1160-ac97-$(CPTCFG_VIDEO_STK1160_AC97) := stk1160-ac97.o

stk1160-y := 	stk1160-core.o \
		stk1160-v4l.o \
		stk1160-video.o \
		stk1160-i2c.o \
		$(obj-stk1160-ac97-y)

obj-$(CPTCFG_VIDEO_STK1160) += stk1160.o

ccflags-y += -I$(backport_srctree)/drivers/media/i2c
