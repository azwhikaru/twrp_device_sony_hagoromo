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
ccflags-y += -I$(src)/../linux/include \
						 -I$(src)/../linux/pri/include \
						 -I$(src)/../core/include \
						 -I$(src)/../include \
						 -I$(src)/ \
						 -I$(src)/../../../btif/inc \
						 -I$(src)/../../../common_detect
						 
ccflags-y += -I$(MTK_ROOT)/kernel/drivers/eemcs/
ccflags-y += -I$(MTK_ROOT)/kernel/drivers/conn_md/include

obj-y += wmt_core.o \
         wmt_ctrl.o \
         wmt_func.o \
         wmt_ic_soc.o \
         wmt_lib.o \
         wmt_conf.o \
         btm_core.o \
         dbg_core.o \
         psm_core.o \
         stp_core.o

endif
