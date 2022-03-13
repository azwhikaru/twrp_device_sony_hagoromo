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
ifeq ($(CONFIG_MTK_WLAN_SUPPORT), y)

# Enable warning to error in all MTK subdir.
subdir-ccflags-y += -Werror

SUPPORT_MT6620 := no
SUPPORT_MT6628 := no

ifneq ($(MTK_COMBO_CHIP_MT6620),)
SUPPORT_MT6620 := yes
endif

ifneq ($(MTK_COMBO_CHIP_MT6628),)
SUPPORT_MT6628 := yes
endif

ifeq ($(SUPPORT_MT6620), yes)
    obj-$(CONFIG_MTK_COMBO_WIFI) += mt6620/
endif

ifeq ($(SUPPORT_MT6628), yes)
    obj-$(CONFIG_MTK_COMBO_WIFI) += mt6628/
endif


endif
