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
# CONN-MD Bridge Driver
# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)

    ccflags-y += -I$(MTK_ROOT)/kernel/drivers/eemcs/
    ccflags-y += -I$(src)/include/

    obj-y	+= conn_md_bridge.o
    conn_md_bridge-objs := conn_md.o
    conn_md_bridge-objs += conn_md_exp.o
    conn_md_bridge-objs += conn_md_log.o
    conn_md_bridge-objs += conn_md_dump.o
    conn_md_bridge-objs += conn_md_dbg.o
    conn_md_bridge-objs += conn_md_test.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD  := $(shell pwd)

default:
	$(MAKE) -C

endif
