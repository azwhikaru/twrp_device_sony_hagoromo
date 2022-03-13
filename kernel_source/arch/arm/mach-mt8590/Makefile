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

# Makefile for the linux kernel.
#

include $(srctree)/drivers/misc/mediatek/Makefile.custom

ccflags-y += -I$(srctree)/

# Fix-me: marked others for early porting
ifeq ($(MTK_INTERNAL), yes)
ifeq ($(MTK_USE_RESERVED_EXT_MEM), yes)
KBUILD_CFLAGS += -DMTK_USE_RESERVED_EXT_MEM
endif
endif

ifeq ($(MTK_SECURITY_SW_SUPPORT), yes)
ifeq ($(MTK_SEC_FASTBOOT_UNLOCK_SUPPORT), yes)
KBUILD_CFLAGS += -DMTK_SEC_FASTBOOT_UNLOCK_SUPPORT
endif
endif

ifeq ($(MTK_TB_WIFI_3G_MODE), WIFI_ONLY)
KBUILD_CFLAGS += -DMTK_TB_WIFI_3G_MODE_WIFI_ONLY
endif

obj-y := core.o irq.o timer.o mt_dbg.o
obj-y += mt_devs.o mt_gpt.o mt_boot.o board.o 
obj-y += ca7_timer.o
obj-y += mt_dma.o
obj-y += mt_reg_dump.o
obj-y += mt_audio_i2c_devs.o
obj-$(CONFIG_NFC_CXD224X_I2C) += mt_nfc_i2c_devs.o
obj-$(CONFIG_FELICA_RCS730) += mt_rcs730_i2c_devs.o
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBDMP2_linux")
obj-y += icx_bl_i2c_devs.o
obj-y += icx_radio_i2c_devs.o
endif
obj-$(CONFIG_RTC_DRV_BU9873)  += mt_rtc_i2c_devs.o
obj-$(CONFIG_ICX_PM_HELPER) += icx_power.o
obj-$(CONFIG_ICX_STARTUP) += icx_startup.o
obj-$(CONFIG_ENABLE_WMPORT) += icx_cradle_devs.o
obj-y += mt_pm_init.o
ifeq ($(CONFIG_SMP),y)
ccflags-y += -I$(srctree)
obj-y += mt-smp.o hotplug.o mt-headsmp.o mt_hotplug.o mt_hotplug_mechanism.o
endif
# TO-FIX add # for do early porting in JB migration
obj-y += eint.o mt_freqhopping.o
obj-$(CONFIG_MT65XX_TRACER) += pmu_v7.o mon_interface.o mt_mon.o mt_emi_bm.o
obj-y += emi_bwl.o emi_mpu.o
#obj-y += camera_isp.o
#obj-y += camera_sysram.o 
#obj-y += camera_pipe_mgr.o
#obj-y += camera_eis.o
#obj-y += camera_fdvt.o
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_emmc")
obj-y += mt_pm_ldo_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_ldvt")
obj-y += mt_pm_ldo_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_nand")
obj-y += mt_pm_ldo_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p3_emmc")
obj-y += mt_pm_ldo_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p3_nand")
obj-y += mt_pm_ldo_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p3_linux")
obj-y += mt_pm_ldo_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_linux")
obj-y += mt_pm_ldo_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m3_emmc")
obj-y += mt_pm_ldo_dummy.o
else
ifeq ($(CONFIG_ARCH_MTK_PROJECT), "bx8590m3_ldvt")
obj-y += mt_pm_ldo_dummy.o
else
#add for BBMini1 (as the same as bx8590p3_linux)
ifeq ($(CONFIG_ARCH_MTK_PROJECT), "BBMini1_linux")
obj-y += mt_pm_ldo_dummy.o
else
obj-y += mt_pm_ldo.o
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

obj-y += innercache.o mt_l2c.o
obj-$(CONFIG_MTK_CIRQ) += mt_cirq.o
ccflags-y += #-I$(MTK_PATH_PLATFORM)/drivers/power \
             #-I$(MTK_PATH_PLATFORM)/drivers/gpu/mali/mali/include \
             -I$(MTK_PATH_SOURCE)/drivers/wdk
#obj-y += mt_spm.o mt_spm_sleep.o mt_spm_idle.o mt_spm_mtcmos.o
obj-y += mt_spm.o mt_spm_sleep.o mt_spm_mtcmos.o
ccflags-$(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) += -I$(srctree)/include/trustzone/
obj-y += mt_sleep.o
#obj-y += mt_cpufreq.o mt_gpufreq.o mt_cpu_ss.o
obj-y += mt_cpufreq.o mt_cpu_ss.o
obj-y += mt_emifreq.o
obj-y += mt_dormant.o cpu_dormant.o
obj-y += mt_dcm.o mt_idle.o
obj-y += mt_clkmgr_common.o
obj-y += mt_clkmgr.o
obj-y += sbchk_base.o
obj-y += mtk_ccci_helper.o
obj-y += mtk_eemcs_helper.o

ifneq ($(CONFIG_MTK_CCCI_DEVICES),y)
ccflags-y  += -D__USING_DUMMY_CCCI_API__
endif

ifneq ($(MTK_TABLET_HARDWARE), )
ccflags-y += -DMTK_TABLET_PLATFORM=\"$(shell echo $(MTK_TABLET_HARDWARE) | tr a-z A-Z )\"
endif

obj-y += mt_ptp.o
obj-y += mssv.o

# Hibernation
AFLAGS_swsusp.o	:= -DTEXT_OFFSET=$(TEXT_OFFSET)
obj-$(CONFIG_HIBERNATION)	+= cpu_hibernate.o swsusp.o

KBUILD_AFLAGS	:=$(KBUILD_AFLAGS:-msoft-float=-Wa,-mfpu=neon)

ifeq ($(MTK_ALPS_BOX_SUPPORT), yes)
ccflags-y += -DMTK_ALPS_BOX_SUPPORT
endif

# Module project related part
obj-y += $(ARCH_MTK_PROJECT)/

#debug driver
obj-$(CONFIG_REGMON_DEBUG) += regmon.o
obj-$(CONFIG_ICX_DEBUG) += icx_debug.o

# nvp driver wrapper
obj-$(CONFIG_ICX_NVP_WRAPPER) += icx_nvp_wrapper.o

ifeq ($(CONFIG_RICHO_RICOH5B), y)
#ricoh5b tuner
obj-y += ricoh5b_i2c_devs.o
obj-y += ricoh5b_sound_devs.o
endif

#DVD 89G Driver
ifeq ($(CONFIG_ARCH_MTK_PROJECT), "BBMini2_linux")
obj-y += dvd_89g_devs.o
obj-y += dvd_89g/
endif
