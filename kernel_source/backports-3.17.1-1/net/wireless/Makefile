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
obj-$(CPTCFG_CFG80211) += cfg80211.o
obj-$(CPTCFG_LIB80211) += lib80211.o
obj-$(CPTCFG_LIB80211_CRYPT_WEP) += lib80211_crypt_wep.o
obj-$(CPTCFG_LIB80211_CRYPT_CCMP) += lib80211_crypt_ccmp.o
obj-$(CPTCFG_LIB80211_CRYPT_TKIP) += lib80211_crypt_tkip.o

obj-$(CONFIG_WEXT_CORE) += wext-core.o
obj-$(CONFIG_WEXT_PROC) += wext-proc.o
obj-$(CONFIG_WEXT_SPY) += wext-spy.o
obj-$(CONFIG_WEXT_PRIV) += wext-priv.o

cfg80211-y += core.o sysfs.o radiotap.o util.o reg.o scan.o nl80211.o
cfg80211-y += mlme.o ibss.o sme.o chan.o ethtool.o mesh.o ap.o trace.o
cfg80211-$(CPTCFG_CFG80211_DEBUGFS) += debugfs.o
cfg80211-$(CPTCFG_CFG80211_WEXT) += wext-compat.o wext-sme.o
cfg80211-$(CPTCFG_CFG80211_INTERNAL_REGDB) += regdb.o

CFLAGS_trace.o := -I$(src)

ccflags-y += -D__CHECK_ENDIAN__

$(obj)/regdb.c: $(src)/db.txt $(src)/genregdb.awk
	@$(AWK) -f $(src)/genregdb.awk < $< > $@

clean-files := regdb.c
