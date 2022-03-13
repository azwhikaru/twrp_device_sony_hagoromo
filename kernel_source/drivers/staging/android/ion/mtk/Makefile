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
#obj-$(CONFIG_ION_MTK) += ion_drv.o ion_mm_heap_old.o ion_sys_contig_heap.o
obj-$(CONFIG_ION_MTK) += ion_drv.o ion_mm_heap.o ion_mm_carveout_heap.o
ccflags-$(CONFIG_ION_MTK)  += -I$(srctree)/drivers/staging/android/ion #fro kernel-3.10 ion
ccflags-$(CONFIG_ION_MTK)  += -I$(srctree)/drivers/gpu/ion  #for kernrl-3.4 ion

ifeq ($(CONFIG_MT_ENG_BUILD), y)
ccflags-y += -DION_RUNTIME_DEBUGGER=1
else
ccflags-y += -DION_RUNTIME_DEBUGGER=0
endif

ccflags-y += -I$(srctree)/include -I$(srctree)/include/trustzone

ifeq ($(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT), y)
ccflags-y += -DION_SUPPORT_TEE=1
else
ccflags-y += -DION_SUPPORT_TEE=0
endif

#obj-y   +=  ion_test_drv/
