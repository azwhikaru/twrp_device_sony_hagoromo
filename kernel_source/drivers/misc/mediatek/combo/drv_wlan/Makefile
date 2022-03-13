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
SUPPORT_MT6620 := no
SUPPORT_MT6628 := no
SUPPORT_MT6630 := no

SUPPORT_MT6620 := $(if $(filter "MT6620", $(MTK_COMBO_CHIP)),yes,no)
SUPPORT_MT6628 := $(if $(filter "MT6628", $(MTK_COMBO_CHIP)),yes,no)
SUPPORT_MT6630 := $(if $(filter "MT6630", $(MTK_COMBO_CHIP)),yes,no)

ifeq ($(SUPPORT_MT6620), yes)
    obj-y += mt6620/wlan/
    $(warning include mt6620/wlan/)
endif

ifeq ($(SUPPORT_MT6628), yes)
    obj-y += mt6628/wlan/
    $(warning include mt6628/wlan/)
endif

ifeq ($(SUPPORT_MT6630), yes)
    obj-$(CONFIG_MTK_COMBO_WIFI) += mt6630/wlan/
    $(warning include mt6630/wlan/)
endif
