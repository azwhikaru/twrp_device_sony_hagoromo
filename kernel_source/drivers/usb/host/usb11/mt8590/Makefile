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
obj-$(CONFIG_MTK_USBFSH) := musbfsh_core.o musbfsh_mt65xx.o

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_emmc5")
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_emmc")
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_linux")
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_em5linux")
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_nand")
ccflags-y += -DP2_PROJECT
endif
include $(srctree)/drivers/misc/mediatek/Makefile.custom
ccflags-y += -I$(srctree)/drivers/usb/host/usb11/
ccflags-y += -I$(srctree)/drivers/usb/host/usb11/$(CONFIG_MTK_PLATFORM)/
ccflags-y += -I$(srctree)/include/linux/
ccflags-y += -Werror
