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
# WMT HAL driver for MT6582
# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.

ifneq ($(KERNELRELEASE),)
  include $(srctree)/drivers/misc/mediatek/Makefile.custom

  ccflags-y += -I$(src)/include \
               -I$(srctree)/drivers/misc/mediatek/conn_soc/common/linux/include \
               -I$(srctree)/drivers/misc/mediatek/conn_soc/common/include \
               -I$(srctree)/drivers/misc/mediatek/conn_soc/common/linux/include \
               -I$(srctree)/arch/arm/mach-$(MTK_PLATFORM)/include/$(CONFIG_ARCH_MTK_PROJECT)

  obj-y += mtk_wcn_consys_hw.o
  obj-y += wmt_plat_alps.o
# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD  := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
