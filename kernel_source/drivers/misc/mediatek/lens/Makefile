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
include $(srctree)/drivers/misc/mediatek/Makefile.custom

obj-y += lens.o

ifeq ($(CONFIG_MTK_LENS_DUMMYLENS_SUPPORT),y)
ccflags-y += -DDUMMY_LENS
lens-y += dummy_lens/dummy_lens.o
endif

ifeq ($(CONFIG_MTK_LENS_DW9718AF_SUPPORT),y)
ccflags-y += -DDW9718AF
lens-y += dw9718af/DW9718AF.o
endif