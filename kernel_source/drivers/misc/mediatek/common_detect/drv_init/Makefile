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
ifneq ($(KERNELRELEASE),)

ifeq ($(CONFIG_MTK_COMBO),y)
	ccflags-y += -I$(src)/inc/
	ccflags-y += -I$(src)/../
	obj-y += conn_drv_init.o
	obj-y += common_drv_init.o
	obj-y += bluetooth_drv_init.o
	obj-y += gps_drv_init.o
	obj-y += fm_drv_init.o
	obj-y += wlan_drv_init.o
ifneq ($(findstring MT6630,$(CONFIG_MTK_COMBO_CHIP)),)
	obj-y += ant_drv_init.o
endif
	ifneq ($(findstring MT6630,$(CONFIG_MTK_COMBO_CHIP)),)
		ccflags-y += -DMT6630
	endif
	ifneq ($(findstring MT6620,$(CONFIG_MTK_COMBO_CHIP)),)
		ccflags-y += -DMT6620
	endif
	ifneq ($(findstring MT6628,$(CONFIG_MTK_COMBO_CHIP)),)
		ccflags-y += -DMT6628
	endif
endif

else
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD  := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
