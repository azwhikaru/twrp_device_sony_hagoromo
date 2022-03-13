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

ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/hdmirx/  \
             -I$(MTK_PATH_SOURCE)/drivers/hdmirx/    \
             -I$(MTK_PATH_PLATFORM)/drivers/hdmirx/  \
             -I$(MTK_PATH_SOURCE)/drivers/hdmirx/    \
             -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/videoin/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/videoif/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/dgi/$(CONFIG_MTK_PLATFORM) \
             -I$(srctree)/sound/soc/mediatek/$(CONFIG_MTK_PLATFORM)
                
obj-y	:= hdmirx.o mt_hdmirx_debug.o hdmi_rx_ctrl.o hdmi_rx_task.o hdmi_rx_dvi.o hdmi_rx_hal.o edid_data.o edid_eeprom.o hal_io.o vga_table.o rx_io.o hdmi_rx_cmd.o hdmi_rx_aud_task.o
ifeq ($(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT),y)
ccflags-y += -I$(srctree)/include/trustzone/
obj-y += hdmi_rx_ca.o
endif



