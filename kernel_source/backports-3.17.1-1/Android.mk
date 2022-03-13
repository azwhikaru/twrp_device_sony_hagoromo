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

TOP_PATH := $(shell pwd)
KLIB := $(TOP_PATH)/kernel
KLIB_BUILD := $(TOP_PATH)/$(PRODUCT_OUT)/obj/KERNEL_OBJ/

BACKPORT_VERSION := 3.17.1-1
BACKPORT_KO_STRIP := $(KERNEL_CROSS_COMPILE)strip --strip-debug
BACKPORT_KO_SRC_PATH := kernel/backports-$(BACKPORT_VERSION)
BACKPORT_KO_FILES :=
BACKPORT_KO_FILES += $(BACKPORT_KO_SRC_PATH)/compat/compat.ko
BACKPORT_KO_FILES += $(BACKPORT_KO_SRC_PATH)/net/mac80211/mac80211.ko
BACKPORT_KO_FILES += $(BACKPORT_KO_SRC_PATH)/net/wireless/cfg80211.ko
BACKPORT_KO_FILES += $(BACKPORT_KO_SRC_PATH)/drivers/net/wireless/mt_soc/conn_soc/mt_wifi/wlan/wlan_mt.ko
BACKPORT_KO_EXTFILES :=
BACKPORT_KO_EXTFILES += $(BACKPORT_KO_SRC_PATH)/drivers/net/wireless/mt66xx/mt6630/wlan/wlan_mt6630.ko
BACKPORT_KO_INSTALL_PATH := $(TOP_PATH)/$(PRODUCT_OUT)/system/lib/modules/

define install-backport-objs
 test -e $1 && $(BACKPORT_KO_STRIP) -o $(BACKPORT_KO_INSTALL_PATH)/$$(basename $1 .ko)_bp.ko $1 || exit 1 ;
endef

define install-backport-extobjs
 test -e $1 && $(BACKPORT_KO_STRIP) -o $(BACKPORT_KO_INSTALL_PATH)/$$(basename $1 .ko)_bp.ko $1 || echo skip. ;
endef

.PHONY: backports

$(BACKPORT_KO_INSTALL_PATH):
	mkdir -p $@

backports: $(INSTALLED_KERNEL_TARGET) $(BACKPORT_KO_INSTALL_PATH)
	make -C $(BACKPORT_KO_SRC_PATH) KLIB=$(KLIB) KLIB_BUILD=$(KLIB_BUILD)
	$(foreach i,$(BACKPORT_KO_FILES),    $(call install-backport-objs,$i))
	$(foreach i,$(BACKPORT_KO_EXTFILES), $(call install-backport-extobjs,$i))

#.PHONY: clean-backports
#clean-backports:
#       $(hide)cd kernel/backports-3.17.1-1 && make clean
#
#clean: clean-backports
