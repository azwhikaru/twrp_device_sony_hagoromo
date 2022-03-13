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
ccflags-y += -Werror

ifeq ($(CONFIG_ARCH_MT6595), y)
  #obj-$(CONFIG_PVR_RGX_MT6595) += mt6595/m1.3_ED2673322/
  obj-y += mt6595/
endif

ifeq ($(CONFIG_ARCH_MT8135), y)
  obj-$(CONFIG_PVR_RGX_MT8135) += mt8135/
endif
