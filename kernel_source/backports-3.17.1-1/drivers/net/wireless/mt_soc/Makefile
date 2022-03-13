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
#subdir-ccflags-y += -Werror

SUPPORT_CONNSYS:= no

$(info 'choose WLAN CHIP=$(CONFIG_MTK_WLAN_CHIP)')

$(info "check_connsys")
SUPPORT_CONNSYS := $(if $(findstring CONSYS,$(CONFIG_MTK_WLAN_CHIP)),yes,no)
$(info "check_connsys_end $(SUPPORT_CONNSYS)")

ifeq ($(SUPPORT_CONNSYS), yes)
	obj-$(CONFIG_MTK_CONNSYS_WIFI) += conn_soc/
    $(info "include conn_soc")
endif

endif
