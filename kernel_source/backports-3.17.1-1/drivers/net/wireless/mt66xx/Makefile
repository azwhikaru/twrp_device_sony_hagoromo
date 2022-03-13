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
$(info enter mt_66xx)

SUPPORT_MT6630 := no

$(info MTK_COMBO_CHIP = $(CONFIG_MTK_COMBO_CHIP))

SUPPORT_MT6630 := $(if $(findstring MT6630, $(CONFIG_MTK_COMBO_CHIP)),yes,no)

$(info SUPPORT_MT6630 = $(SUPPORT_MT6630))

ifeq ($(SUPPORT_MT6630),yes)
    obj-$(CONFIG_MTK_COMBO_WIFI) += mt6630/
    #obj-m += mt6630/
    $(info include mt6630)
endif
