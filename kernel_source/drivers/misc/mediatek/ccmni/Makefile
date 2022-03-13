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
ifneq ($(CONFIG_MTK_CCCI_EXT), y)
MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
#CCCI_PLATFORM_DRIVER_DIR :=  $(wildcard $(obj)/$(call to-root,$(obj))mediatek/platform/$(MTK_PLATFORM)/kernel/drivers/ccci)
CCCI_PLATFORM_DRIVER_DIR :=  $(srctree)/mediatek/platform/$(MTK_PLATFORM)/kernel/drivers/ccci
ifeq ($(CCCI_PLATFORM_DRIVER_DIR),)
$(error Invalid ccci platform path, please check)
endif
ifeq (y,$(CONFIG_MTK_NET_CCMNI))
obj-m	+= ccmni.o
ccmni-$(CONFIG_MTK_NET_CCMNI) := ccmni_pfp.o ccmni_net.o
ccflags-y += -I$(MTK_PATH_SOURCE)/drivers/ccci/include -I$(CCCI_PLATFORM_DRIVER_DIR)\
		 -I$(CCCI_PLATFORM_DRIVER_DIR)/include
else
obj-n += dummy.o
endif

else
obj- :=dummy.o
endif
