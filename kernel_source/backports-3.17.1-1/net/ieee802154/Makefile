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
obj-$(CPTCFG_IEEE802154) += ieee802154.o af_802154.o
obj-$(CPTCFG_IEEE802154_6LOWPAN) += ieee802154_6lowpan.o

ieee802154_6lowpan-y := 6lowpan_rtnl.o reassembly.o
ieee802154-y := netlink.o nl-mac.o nl-phy.o nl_policy.o wpan-class.o \
                header_ops.o
af_802154-y := af_ieee802154.o raw.o dgram.o

ccflags-y += -D__CHECK_ENDIAN__
