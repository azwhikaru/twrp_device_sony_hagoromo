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
# Build as module
#obj-m += cast_auth.o
include $(srctree)/drivers/misc/mediatek/Makefile.custom

ccflags-y += -I$(srctree)/../vendor/mediatek/proprietary/external/trustzone/include/
ccflags-y += -I$(srctree)/include/ -I$(srctree)/include/trustzone/ 
ccflags-y += -I$(srctree)/ -Werror


obj-y += cast_auth.o
cast_auth-objs += cast_auth_ko.o cast_auth_mod.o
