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
include $(srctree)/drivers/misc/mediatek/combo/Makefile.combo


MTK_FM_RX_AUDIO_FLAG := $(addprefix -D, $(shell echo $(CONFIG_MTK_FM_RX_AUDIO) | tr a-z A-Z))
subdir-ccflags-y += $(MTK_FM_RX_AUDIO_FLAG)



obj-y   +=  aee/
obj-y   +=  boot/
obj-$(CONFIG_MTK_BTIF)   +=  btif/
obj-$(CONFIG_ARCH_MT6595)   +=  cmdq/
obj-$(CONFIG_ARCH_MT8135)   +=  dumpEfuseInfo/
obj-$(CONFIG_ARCH_MT6595)   +=  dum-char/
obj-$(CONFIG_ARCH_MT8127)   +=  dum-char/
obj-$(CONFIG_ARCH_MT8590)   +=  dum-char/
obj-$(CONFIG_MALI_MT8127)   +=  sync/
obj-y   +=  eint/
obj-y   +=  gcpu/
obj-y   +=  gdma/
obj-$(CONFIG_MTK_LEGACY_GPIO_DRIVER)    +=      gpio/
obj-y   +=  jpeg/
#obj-y   +=  kernel/
obj-$(CONFIG_ARCH_MT6595)   +=  keypad/
obj-$(CONFIG_ARCH_MT8127)   +=  keypad/
obj-$(CONFIG_ARCH_MT8590)   +=  keypad/
obj-y   +=  lcm/
#obj-$(CONFIG_ARCH_MT8135)   +=  lcm/
#obj-$(CONFIG_ARCH_MT8127)   +=  lcm/
obj-y   +=  m4u/
obj-$(CONFIG_ZRAM) +=  mlog/
obj-y   +=  mmp/
obj-$(CONFIG_MTKPASR)       +=  mtkpasr/
obj-y   +=  mtprof/
obj-$(CONFIG_USB_MU3D_DRV)  +=  mu3d/
obj-$(CONFIG_MU3_PHY)       +=  mu3phy/
obj-y   +=  pa/
obj-$(CONFIG_MTK_PMIC_WRAP)   +=  pmic_wrap/
obj-$(CONFIG_ARCH_MT6595)   +=  pmt/
obj-$(CONFIG_ARCH_MT8127)   +=  pmt/
obj-$(CONFIG_ARCH_MT8590)   +=  pmt/
obj-$(CONFIG_ARCH_MT6595)   +=  power_gs/
obj-$(CONFIG_ARCH_MT8127)   +=  power_gs/
obj-$(CONFIG_ARCH_MT8590)   +=  power_gs/
obj-y   +=  sched/
obj-y   +=  selinux_warning/
obj-y   +=  thermal/
obj-y   +=  touchpanel/
obj-y   +=  uart/
obj-$(CONFIG_ARCH_MT8135)   +=  video/$(MTK_PLATFORM)/
obj-$(CONFIG_ARCH_MT8127)   +=  video/$(MTK_PLATFORM)/
ifeq ($(CONFIG_MTK_FB),y)
obj-$(CONFIG_ARCH_MT8590)   +=  video/$(MTK_PLATFORM)/
endif
obj-$(CONFIG_MTK_FB)   +=  videox/
obj-$(CONFIG_MTK_VQ_SUPPORT)   +=  vq/
obj-y   +=  wdk/
obj-$(CONFIG_MTK_XHCI)   +=  xhci/

ifneq ($(wildcard $(srctree)/$(src)/met/),)
obj-y += met/
endif

#obj-y   +=  hdmitx/
obj-y	+= mmp/
obj-y	+= ram_console/
obj-y   += thermal/
obj-y   += videocodec/
obj-y   += xlog/
obj-y   += wdk/
obj-y   += sound/
obj-y   += btcvsd/
obj-y   += evdo_support/
#obj-y	+= mt_logger/
#obj-y   +=  hdmitx/
obj-y	+= mmp/
obj-y	+= ram_console/
obj-y   += thermal/
obj-y   += videocodec/
obj-y   += xlog/
obj-y   += wdk/
obj-y   += evdo_support/
#obj-y	+= mt_logger/
ifeq ($(CONFIG_MTK_HDMI_SUPPORT),y)
obj-$(CONFIG_MTK_FB)   += hdmi/
obj-$(CONFIG_ARCH_MT6595)   += ext_disp/
endif
obj-$(CONFIG_MTK_TVE_SUPPORT)   +=  tve/
ifneq ($(CONFIG_MTK_FPGA),y)
obj-y   +=  accdet/
obj-y   +=  almk/
obj-y   +=  auxadc/
obj-y   +=  dac/
# gator-driver is not custom-release and will be removed
obj-y   += $(call all-subdirs-with-makefile,gator-driver)
obj-y   +=  ccci/
obj-y   +=  ccmni/
obj-y   +=  dbg_dump/
obj-y   +=  dual_ccci/
obj-y   +=  eccci/
obj-y   +=  emd_ctl/
obj-$(CONFIG_MTK_ETM) += etm/
obj-y   +=  fmradio/
obj-y   +=  freqhopping/
obj-y   +=  gps/
obj-y   +=  hwmon/
obj-y   +=  leds/
obj-y   +=  mhl/
obj-$(CONFIG_MTK_NFC)   += nfc/
obj-y   +=  pwm/
obj-y   +=  ram_console/
obj-y   +=  rtc/
obj-y   +=  sync/
#MTK in-house tee, need to be prior to kernel for fiq_smp_call support
obj-$(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) += trustzone/
obj-$(CONFIG_MTK_C4A_SUPPORT) += cast_auth/
obj-y	+= kernel/
obj-y   +=  vibrator/
obj-y   +=  videocodec/


ifneq ($(CONFIG_MTK_COMBO),)
	subdir-ccflags-y += -DMTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT
	ifeq ($(CONFIG_MTK_COMBO),y)
		subdir-ccflags-y += -DMTK_WCN_REMOVE_KERNEL_MODULE
	endif

	BUILD_COMBO := no
	BUILD_SOC := no

	ifneq ($(findstring MT6620E3,$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_COMBO := yes
		subdir-ccflags-y += -DMTK_WCN_COMBO_CHIP_SUPPORT
		subdir-ccflags-y += -DMT6620
	endif
	ifneq ($(findstring MT6628,$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_COMBO := yes
		subdir-ccflags-y += -DMTK_WCN_COMBO_CHIP_SUPPORT
		subdir-ccflags-y += -DMT6628
	endif
$(warning combo_chip $(CONFIG_MTK_COMBO_CHIP))
$(warning combo_chip filter $(findstring MT6630,$(CONFIG_MTK_COMBO_CHIP)))
	ifneq ($(findstring MT6630,$(CONFIG_MTK_COMBO_CHIP)),)
$(warning build combo)
		BUILD_COMBO := yes
		subdir-ccflags-y += -DMTK_WCN_COMBO_CHIP_SUPPORT
		subdir-ccflags-y += -DMT6630
	endif
	ifeq ($(BUILD_COMBO), yes)
		obj-y   +=  combo/
	endif

	ifneq ($(findstring CONSYS_6572,$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
	ifneq ($(findstring CONSYS_6582,$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
	ifneq ($(findstring CONSYS_6582,$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
	ifneq ($(findstring CONSYS_6592,$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
	ifneq ($(findstring CONSYS_8127,$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
$(warning combo_chip findstring $(findstring CONSYS_8590,$(CONFIG_MTK_COMBO_CHIP)))
	ifneq ($(findstring CONSYS_8590,$(CONFIG_MTK_COMBO_CHIP)),)
$(warning build soc)
		BUILD_SOC := yes
	endif
	ifeq ($(BUILD_SOC), yes)
		obj-y   +=  conn_soc/
	endif
	obj-y   +=  common_detect/
endif

endif#!CONFIG_MTK_FPGA



ifneq ($(CONFIG_MTK_PLATFORM),)
MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
obj-$(CONFIG_HAVE_CMMB_FEATURE)   += cmmbdrv/$(MTK_PLATFORM)/
obj-y   += devapc/$(MTK_PLATFORM)/
obj-y   += devinfo/$(MTK_PLATFORM)/
obj-$(CONFIG_MTK_FB)   += dispsys/$(MTK_PLATFORM)/
obj-$(CONFIG_MTK_LDVT) += ldvt/$(MTK_PLATFORM)/
#obj-y   += masp/$(MTK_PLATFORM)/
obj-y   += mmc-host/$(MTK_PLATFORM)/
#obj-$(CONFIG_MTK_MMC)	+= pmt/
obj-$(CONFIG_MTK_MTD_NAND)	+= nand/
#obj-y   += power-loss-test/$(MTK_PLATFORM)/
#obj-y   += power/$(MTK_PLATFORM)/
obj-y   += power/
obj-y   += smi/$(MTK_PLATFORM)/
obj-y   += spi/$(MTK_PLATFORM)/
obj-y   += wdt/$(MTK_PLATFORM)/
endif

obj-y   += camera/
#obj-y   += cam_cal/
obj-y   += imgsensor/src
#obj-y   += eeprom/
obj-y   += flashlight/src
ifeq ($(CONFIG_CUSTOM_KERNEL_FLASHLIGHT),"dummy_flashlight")
obj-y   += flashlight/dummy_flashlight/
endif
ifeq ($(CONFIG_CUSTOM_KERNEL_FLASHLIGHT),"constant_flashlight")
obj-y   += flashlight/constant_flashlight/
endif

# obj-y   += lens/
obj-y   += mrdump/
#obj-y   +=  hibcore/
obj-y   +=  i2c/
obj-$(CONFIG_MTK_IRRX_SUPPORT) += irrx/

ifeq ($(CONFIG_MTK_HDMIRX_SUPPORT),y)
obj-y		+= hdmirx/
obj-y		+= dgi/
obj-y		+= videoif/
obj-y		+= videoin/
endif

ifeq ($(CONFIG_MTK_V4L2_VIDEO_SUPPORT),y)
obj-y   +=  v4l2_video/
endif

obj-y		+= cpu_stress/

ifeq ($(CONFIG_MTK_EEPROM_AT24),y)
obj-y   += eeprom/at24/
endif
