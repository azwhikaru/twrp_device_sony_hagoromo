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

ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/dgi/  \
             -I$(MTK_PATH_SOURCE)/drivers/dgi/    \
             -I$(MTK_PATH_PLATFORM)/drivers/dgi/  \
             -I$(MTK_PATH_SOURCE)/drivers/dgi/    \
             -I$(srctree)/drivers/misc/mediatek/hdmirx/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/videoin/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/videoif/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)   
             
obj-y	:= dgi_if.o dgi_hal.o rgb2hdmi.o




