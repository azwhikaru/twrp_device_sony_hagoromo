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
include $(srctree)/drivers/misc/mediatek/Makefile.custom
AUXADC_PLATFORM_DRIVER_DIR :=  $(srctree)/drivers/misc/mediatek/auxadc/$(CONFIG_MTK_PLATFORM)
obj-y += mt_auxadc.o
obj-y += $(subst ",,$(CONFIG_MTK_PLATFORM))/
ccflags-y  += -I$(AUXADC_PLATFORM_DRIVER_DIR)
