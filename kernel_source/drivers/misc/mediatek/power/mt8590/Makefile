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

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_emmc")
    obj-$(CONFIG_MTK_PMIC) += pmic_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_ldvt")
    obj-$(CONFIG_MTK_PMIC) += pmic_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_nand")
    obj-$(CONFIG_MTK_PMIC) += pmic_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p3_emmc")
    obj-$(CONFIG_MTK_PMIC) += pmic_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p3_nand")
    obj-$(CONFIG_MTK_PMIC) += pmic_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p3_linux")
    obj-$(CONFIG_MTK_PMIC) += pmic_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m3_emmc")
    obj-$(CONFIG_MTK_PMIC) += pmic_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_linux")
    obj-$(CONFIG_MTK_PMIC) += pmic_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m3_ldvt")
    obj-$(CONFIG_MTK_PMIC) += pmic_dummy.o
else
#add for BB-Mini1 (as the same as bx8590p3_linux)
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBMini1_linux")
    obj-$(CONFIG_MTK_PMIC) += pmic_dummy.o
else

ifeq ($(CONFIG_CHARGER_BQ24262_WMPORT),y)
	pmic_chr_type_det_source=
		obj-$(CONFIG_MTK_PMIC) += pmic_mt6323.o upmu_common.o  pmic_chr_type_det_icx.o
else
		obj-$(CONFIG_MTK_PMIC) += pmic_mt6323.o upmu_common.o  pmic_chr_type_det.o
endif

endif
endif
endif
endif
endif
endif
endif
endif
endif
endif

ifeq ($(MTK_MT6333_SUPPORT),yes)
    obj-$(CONFIG_MTK_PMIC) += mt6333.o
endif

ifeq ($(MTK_INTERNAL_MHL_SUPPORT), yes)
#ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/mhltx/
endif  

obj-$(CONFIG_MTK_SMART_BATTERY) += battery_meter_hal.o 

#ifeq ($(CONFIG_MTK_BQ27541_SUPPORT),y)
#    obj-$(CONFIG_MTK_SMART_BATTERY) += bq27541.o
#endif
ifeq ($(CONFIG_MTK_BQ24196_SUPPORT),y) 
    obj-$(CONFIG_MTK_SMART_BATTERY) += charging_hw_bq24196.o bq24196.o tbl_helper_func.o
else
ifeq ($(CONFIG_MTK_NCP1851_SUPPORT),y) 
    obj-$(CONFIG_MTK_SMART_BATTERY) += ncp1851.o charging_hw_ncp1851.o tbl_helper_func.o
else
ifeq ($(CONFIG_MTK_NCP1854_SUPPORT),y) 
    obj-$(CONFIG_MTK_SMART_BATTERY) += ncp1854.o charging_hw_ncp1854.o tbl_helper_func.o
else
ifeq ($(CONFIG_MTK_FAN5405_SUPPORT),y)
    obj-$(CONFIG_MTK_SMART_BATTERY) += fan5405.o charging_hw_fan5405.o
else
ifeq ($(CONFIG_MTK_BQ24158_SUPPORT),y)
    obj-$(CONFIG_MTK_SMART_BATTERY) += bq24158.o charging_hw_bq24158.o
else
ifeq ($(CONFIG_MTK_BQ24296_SUPPORT),y)
    obj-$(CONFIG_MTK_SMART_BATTERY) += bq24296.o charging_hw_bq24296.o tbl_helper_func.o
else
ifeq ($(CONFIG_MTK_BQ24195_SUPPORT),y)
    obj-$(CONFIG_MTK_SMART_BATTERY) += bq24195.o charging_hw_bq24195.o tbl_helper_func.o
else
ifeq ($(CONFIG_MTK_BQ25890_SUPPORT),y)
    obj-$(CONFIG_MTK_SMART_BATTERY) += bq25890.o charging_hw_bq25890.o tbl_helper_func.o
else
ifeq ($(CONFIG_MTK_SWCHR_SUPPORT),y)
    obj-$(CONFIG_MTK_SMART_BATTERY) += charging_hw_6333.o          
else
    obj-$(CONFIG_MTK_SMART_BATTERY) += charging_hw_pmic.o
endif
endif
endif
endif
endif
endif
endif
endif
endif
