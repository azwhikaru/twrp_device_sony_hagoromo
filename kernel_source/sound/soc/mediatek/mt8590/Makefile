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

# ccflags-y += -DDEBUG

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bt8590m1_emmc")
ccflags-y += -DSND_SOC_MTK_BT8590M1_EMMC
endif
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m1_emmc")
ccflags-y += -DSND_SOC_MTK_BX8590M1_EMMC
endif
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m1_ldvt")
ccflags-y += -DSND_SOC_MTK_BX8590M1_LDVT
endif
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590m1_linux")
ccflags-y += -DSND_SOC_MTK_BX8590M1_LINUX
endif
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"bx8590p2_linux")
ccflags-y += -DSND_SOC_MTK_BX8590P2_LINUX
endif
#add for BBPoP (as the same as bx8590m1_linux)
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBPoP_linux")
ccflags-y += -DSND_SOC_MTK_BX8590M1_LINUX
endif
#add for BBPSK1,2,Mini2 (as the same as bx8590p2_linux)
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBPSK1_linux")
ccflags-y += -DSND_SOC_MTK_BX8590P2_LINUX
endif
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBPSK2_linux")
ccflags-y += -DSND_SOC_MTK_BX8590P2_LINUX
endif
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"BBMini2_linux")
ccflags-y += -DSND_SOC_MTK_BX8590P2_LINUX
endif
ifeq ("$(MTK_CM4_SUPPORT)","yes")
ccflags-y += -DSND_SOC_MTK_CM4_SUPPORT
endif

# machine driver
obj-$(CONFIG_SND_SOC_MT8590) += demo-machine.o
obj-$(CONFIG_SND_SOC_ICX_BOARD) += icx-machine.o

# platform driver
# platform
obj-$(CONFIG_SND_SOC_MT8590) += mt8590-lp-audio.o
# platform
obj-$(CONFIG_SND_SOC_MT8590) += mt8590-audio.o mt8590-hdmi-control.o 
# cpu dai
obj-$(CONFIG_SND_SOC_MT8590) += mt8590-dai.o
# hardware
obj-$(CONFIG_SND_SOC_MT8590) += mt8590-afe.o

# codec driver
obj-$(CONFIG_SND_SOC_MT8590) += dummy-codec.o pcm1795.o pcm4202.o pcm1861.o pcm5102a.o
obj-$(CONFIG_SND_SOC_ICX_BOARD) += pcm1808.o

# others
obj-$(CONFIG_SND_SOC_MT8590) += mt8590-asrc.o mt8590-audrtc.o mt8590-register-debug.o

#debug driver
obj-$(CONFIG_SND_SOC_MT8590) += mt8590-afe-regmon.o
