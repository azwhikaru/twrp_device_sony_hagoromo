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

#add kernel source code path as head file seach path
ccflags-y += -I$(srctree)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/auxadc/

obj-$(CONFIG_MTK_MMC) += dbg.o
obj-$(CONFIG_MTK_MMC) += mt_sd_misc.o sd.o
obj-$(CONFIG_MTK_MMC) += sdio_autok.o sdio_autok_proc.o ettagent.o


# kdump/kpainc interface move to lk 
ifeq ($(CONFIG_MTK_AEE_IPANIC),y)
obj-$(CONFIG_MTK_MMC) += mt_dump.o
endif

ifeq ($(CONFIG_MTK_EMMC_SUPPORT_OTP),yes)
obj-$(CONFIG_MTK_MMC) += emmc_otp.o
endif

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
ccflags-y += -DFEATURE_MET_MMC_INDEX
endif