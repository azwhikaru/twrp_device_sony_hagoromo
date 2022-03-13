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
# drivers/video/mtk/Makefile
#
# Makefile for MT8590 video driver.
#
include $(srctree)/drivers/misc/mediatek/Makefile.custom

ccflags-y += -I$(srctree)/drivers/misc/mediatek/videoin/$(CONFIG_MTK_PLATFORM) \
             -I$(srctree)/drivers/misc/mediatek/hdmirx/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/videoif/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/dgi/$(CONFIG_MTK_PLATFORM)
obj-y	:= vin_drv_if.o vin_hal.o vin_main.o




