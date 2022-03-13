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
ccflags-y += -DMUSB11_QMU_HOST_BULK_RX_AUTO_SPLIT

#ccflags-y += -DMTK_USB11_VBUS_DETECT_SUPPORT
ccflags-y += -DMUSB11_ID_PIN_USE_EX_EINT
#ccflags-y += -DMUSB11_PORT1_HOST

obj-$(CONFIG_USB11_MTK_HDRC) += musb11_hdrc.o
obj-$(CONFIG_USB11_MTK_HDRC) += $(subst ",,$(CONFIG_MTK_PLATFORM))/

include $(srctree)/drivers/misc/mediatek/Makefile.custom

musb11_hdrc-y := musb11_core.o

musb11_hdrc-y					+= musb11_gadget_ep0.o musb11_gadget.o
musb11_hdrc-y					+= musb11_virthub.o musb11_host.o
musb11_hdrc-y					+= musb11_hsdma.o
musb11_hdrc-y					+= musb11_debugfs.o
musb11_hdrc-y  				+= mtk11_qmu.o musb11_qmu.o
