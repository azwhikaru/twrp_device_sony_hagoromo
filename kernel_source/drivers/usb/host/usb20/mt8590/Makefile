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
#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs
#

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m3_emmc")
ccflags-y += -DMTK_USB_NO_BATTERY
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m3_ldvt")
ccflags-y += -DMTK_USB_NO_BATTERY
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_emmc")
ccflags-y += -DMTK_USB_NO_BATTERY
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_nand")
ccflags-y += -DMTK_USB_NO_BATTERY
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m2_ldvt")
ccflags-y += -DMTK_USB_NO_BATTERY
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p3_linux")
ccflags-y += -DMTK_USB_NO_BATTERY
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p3_nand")
ccflags-y += -DMTK_USB_NO_BATTERY
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p3_emmc")
ccflags-y += -DMTK_USB_NO_BATTERY
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p1_6630")
ccflags-y += -DID_PIN_USE_EX_EINT
ccflags-y += -DP1_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p1_emmc")
ccflags-y += -DID_PIN_USE_EX_EINT
ccflags-y += -DP1_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p1_ldvt")
ccflags-y += -DID_PIN_USE_EX_EINT
ccflags-y += -DP1_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_emmc5")
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_emmc")
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_linux")
ccflags-y += -DID_PIN_USE_EX_EINT
#ccflags-y += -DWITH_USB_ENTER_SUSPEND
#ccflags-y += -DUSB_SW_WITCH_MODE
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_em5linux")
#ccflags-y += -DID_PIN_USE_EX_EINT
#ccflags-y += -DUSB_OTG_LOW_POWER
#ccflags-y += -DUSB_SW_WITCH_MODE
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_nand")
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx2701p1_emmc")
ccflags-y += -DP2701_PROJECT
endif

#add for BBPSK1 (as the same as bx8590p2_linux)
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBPSK1_linux")
ccflags-y += -DUSB_SW_WITCH_MODE
ccflags-y += -DUSB_SW_DEVICE_MODE
ccflags-y += -DID_PIN_USE_EX_EINT
ccflags-y += -DWITH_USB_ENTER_SUSPEND
ccflags-y += -DP2_PROJECT
endif

#add for BBPSK2 (as the same as bx8590p2_linux)
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBPSK2_linux")
ccflags-y += -DUSB_SW_WITCH_MODE
ccflags-y += -DUSB_SW_DEVICE_MODE
ccflags-y += -DID_PIN_USE_EX_EINT
ccflags-y += -DWITH_USB_ENTER_SUSPEND
ccflags-y += -DP2_PROJECT
endif

#add for BBMini1 (as the same as bx8590p3_linux)
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBMini1_linux")
ccflags-y += -DMTK_USB_NO_BATTERY
endif

#add for BBMini2 (as the same as bx8590p2_linux)
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBMini2_linux")
ccflags-y += -DUSB_SW_WITCH_MODE
ccflags-y += -DUSB_SW_DEVICE_MODE
ccflags-y += -DID_PIN_USE_EX_EINT
ccflags-y += -DWITH_USB_ENTER_SUSPEND
ccflags-y += -DP2_PROJECT
endif

#add for BBDMP2
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBDMP2_linux")
ccflags-y += -DUSB_SW_WITCH_MODE
ccflags-y += -DID_PIN_USE_EX_EINT
ccflags-y += -DWITH_USB_ENTER_SUSPEND
endif

ccflags-y += -DMUSB_QMU_SUPPORT
#ccflags-y += -DMUSB_QMU_SUPPORT_HOST

#ccflags-y += -DMUSB_PORT0_HOST
#ccflags-y += -DMTK_USB_NO_BATTERY

include $(srctree)/drivers/misc/mediatek/Makefile.custom
ccflags-y += -Werror

obj-$(CONFIG_USB_MTK_HDRC)	:= usb20.o usb20_phy.o
obj-$(CONFIG_USB_MTK_HDRC)  += usb20_host.o
obj-$(CONFIG_USB_MTK_OTG)   += usb20_otg_if.o
obj-$(CONFIG_USB_MTK_DEBUG_FS)	+= usb20_phy_debugfs.o
ifeq ($(CONFIG_MTK_AEE_KDUMP),y)
obj-$(CONFIG_USB_MTK_HDRC) += usb_dump.o
endif
