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

#ifeq ($(MTK_GPU_CHIP), MALI400MP1)
#ccflags-y += -DMALI_SUPPORT
#ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/gpu/mali/mali/common
#ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/gpu/mali/mali/include
#ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/gpu/mali/mali/linux
#endif

ccflags-$(CONFIG_ZRAM) += -I$(srctree)/drivers/staging/zram
ccflags-$(CONFIG_ZSMALLOC) += -I$(srctree)/drivers/staging/zsmalloc

obj-y += mlog.o
mlog-y := mlog_dump.o mlog_logger.o
