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
# drivers/video/mtk/Makefile
#
# Makefile for MT6516 video driver.
#
ifeq ($(CONFIG_MTK_HDMI_SUPPORT),y)
ifeq ($(CONFIG_CUSTOM_KERNEL_HDMI),nxp_tda19989)
ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/hdmitx/  \
             -I$(MTK_PATH_SOURCE)/drivers/hdmitx/	\
			 -I$(MTK_PATH_PLATFORM)/drivers/video/  \
             -I$(MTK_PATH_SOURCE)/drivers/video/	\
			-DTMFL_TDA19989 -DTMFL_NO_RTOS -DTMFL_LINUX_OS_KERNEL_DRIVER

obj-y +=						\
	hdmi_drv.o					\
	tmbslTDA9989_HDCP.o			\
	tmbslTDA9989_local.o		\
	tmbslTDA9989_State.o		\
	tmdlHdmiTx_local.o			\
	tmbslTDA9989_Edid.o			\
	tmbslTDA9989_InOut.o		\
	tmbslTDA9989_Misc.o			\
	tmdlHdmiTx.o				\
	tmdlHdmiTx_cfg.o			\
	I2C_api.o
endif
endif
obj-y += dummy.o
