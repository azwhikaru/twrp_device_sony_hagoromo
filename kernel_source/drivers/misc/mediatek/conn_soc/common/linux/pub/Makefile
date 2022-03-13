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
ifeq ($(CONFIG_MTK_COMBO), y)
ccflags-y += -I$(src)/../../linux/include \
						 -I$(src)/../../linux/pri/include \
						 -I$(src)/../../core/include \
						 -I$(src)/../../include \
						 -I$(src)/../ \
						 -I$(src)/../../../../common_detect
						 
ccflags-y += -I$(MTK_ROOT)/kernel/drivers/eemcs/
ccflags-y += -I$(MTK_ROOT)/kernel/drivers/conn_md/include
ccflags-y += -DWMT_CREATE_NODE_DYNAMIC=1

obj-y += osal.o \
         bgw_desense.o \
         wmt_idc.o
obj-$(CONFIG_MTK_COMBO_BT) += stp_chrdev_bt.o

obj-$(CONFIG_MTK_COMBO_GPS) += stp_chrdev_gps.o

obj-$(CONFIG_MTK_COMBO_WIFI) += wmt_chrdev_wifi.o
endif
