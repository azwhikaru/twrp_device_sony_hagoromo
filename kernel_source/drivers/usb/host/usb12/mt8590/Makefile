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
ccflags-y += -DWITH_USB11_ENTER_SUSPEND
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_em5linux")
#ccflags-y += -DUSB_OTG_LOW_POWER
#ccflags-y += -DUSB11_SW_WITCH_MODE
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_nand")
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx2701p1_emmc")
ccflags-y += -DP2701_PROJECT
endif

#add sony build config
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBMini1_linux")
ccflags-y += -DMTK_USB_NO_BATTERY
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBPSK1_linux")
ccflags-y += -DMTK_USB11_VBUS_DETECT_SUPPORT
ccflags-y += -DUSB11_SW_WITCH_MODE
ccflags-y += -DWITH_USB11_ENTER_SUSPEND
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBPSK2_linux")
ccflags-y += -DWITH_USB11_ENTER_SUSPEND
ccflags-y += -DUSB11_SW_WITCH_MODE
#ccflags-y += -DMUSB11_PORT1_HOST
ccflags-y += -DP2_PROJECT
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBMini2_linux")
ccflags-y += -DWITH_USB11_ENTER_SUSPEND
ccflags-y += -DUSB11_SW_WITCH_MODE
#ccflags-y += -DMUSB11_PORT1_HOST
ccflags-y += -DP2_PROJECT
endif

ccflags-y += -DMUSB11_QMU_SUPPORT
ccflags-y += -DMUSB11_QMU_SUPPORT_HOST

#ccflags-y += -DMTK_USB11_VBUS_DETECT_SUPPORT
ccflags-y += -DMUSB11_ID_PIN_USE_EX_EINT
#ccflags-y += -DMUSB11_PORT1_HOST

include $(srctree)/drivers/misc/mediatek/Makefile.custom
ccflags-y += -Werror

obj-$(CONFIG_USB11_MTK_HDRC)	:= usb11.o usb11_phy.o
obj-$(CONFIG_USB11_MTK_HDRC)  += usb11_host.o

#obj-$(CONFIG_USB11_MTK_OTG)   += usb20_otg_if.o
#obj-$(CONFIG_USB11_MTK_DEBUG_FS)	+= usb20_phy_debugfs.o
ifeq ($(CONFIG_MTK_AEE_KDUMP),y)
#obj-$(CONFIG_MTK_USBFSH) += usb_dump.o
endif
