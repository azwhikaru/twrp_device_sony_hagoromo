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
# only support 6628 currently, do not support on-binary
ifneq ($(MTK_COMBO_CHIP_MT6628),)
# ---------------------------------------------------
# Compile Options
# ---------------------------------------------------
	ccflags-y += -DLINUX -DMT6628 -DDBG=1
	obj-$(CONFIG_MTK_COMBO_WIFI) += mtk_wifi_loopback.o
# ---------------------------------------------------
# Directory List
# ---------------------------------------------------
	mtk_wifi_loopback-objs  := fw_download.o mt6628-core.o
endif
