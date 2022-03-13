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
# Display parameters shown under Settings -> About Phone

include $(srctree)/drivers/misc/mediatek/Makefile.custom

ifeq ($(CONFIG_MTK_MMC),y)
  CONFIG_MTK_USIF = y
else
	ifeq ($(CONFIG_MTK_MTD_NAND),y)
	CONFIG_MTK_USIF = y
	else
	CONFIG_MTK_USIF = n
	endif
endif

obj-$(CONFIG_MTK_USIF) := DumChar.o
ifeq ($(CONFIG_ARCH_MT8590_ICX),y)
DumChar-objs :=  dumchar.o
else
DumChar-objs :=  dumchar.o env.o
endif
