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

EXTRA_CFLAGS += -DUSE_SSUSB_QMU

obj-y := musb_hdrc.o

musb_hdrc-y := musb_core.o
musb_hdrc-y += musb_gadget_ep0.o musb_gadget.o
musb_hdrc-y += musb_init.o ssusb_qmu.o mt_usb.o
musb_hdrc-$(CONFIG_DEBUG_FS) += musb_debugfs.o
musb_hdrc-y += mu3d_hal_qmu_drv.o \
              mu3d_hal_usb_drv.o \
              mu3d_hal_phy.o
