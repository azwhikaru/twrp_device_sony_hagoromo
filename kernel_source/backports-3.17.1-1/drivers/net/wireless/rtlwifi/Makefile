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
obj-$(CPTCFG_RTLWIFI) 		+= rtlwifi.o
rtlwifi-objs	:=		\
		base.o		\
		cam.o		\
		core.o		\
		debug.o		\
		efuse.o		\
		ps.o		\
		rc.o		\
		regd.o		\
		stats.o

rtl8192c_common-objs +=		\

obj-$(CPTCFG_RTLWIFI_PCI)	+= rtl_pci.o
rtl_pci-objs	:=		pci.o

obj-$(CPTCFG_RTLWIFI_USB)	+= rtl_usb.o
rtl_usb-objs	:=		usb.o

obj-$(CPTCFG_RTL8192C_COMMON)	+= rtl8192c/
obj-$(CPTCFG_RTL8192CE)		+= rtl8192ce/
obj-$(CPTCFG_RTL8192CU)		+= rtl8192cu/
obj-$(CPTCFG_RTL8192SE)		+= rtl8192se/
obj-$(CPTCFG_RTL8192DE)		+= rtl8192de/
obj-$(CPTCFG_RTL8723AE)		+= rtl8723ae/
obj-$(CPTCFG_RTL8723BE)		+= rtl8723be/
obj-$(CPTCFG_RTL8188EE)		+= rtl8188ee/
obj-$(CPTCFG_RTLBTCOEXIST)	+= btcoexist/
obj-$(CPTCFG_RTL8723_COMMON)	+= rtl8723com/

ccflags-y += -D__CHECK_ENDIAN__
