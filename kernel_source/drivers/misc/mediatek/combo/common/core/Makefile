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

ccflags-y += -I$(src)/../linux/include \
						 -I$(src)/../core/include \
						 -I$(src)/../include \
						 -I$(src)/ \
						 -I$(src)/../../../common_detect


ifeq ($(CONFIG_MTK_COMBO),y)
obj-y += wmt_core.o wmt_ctrl.o  wmt_func.o  wmt_lib.o wmt_conf.o wmt_dbg.o wmt_exp.o
obj-y	+= wmt_ic_6620.o
obj-y	+= wmt_ic_6628.o
obj-y	+= wmt_ic_6630.o

obj-y	+= stp_exp.o
obj-y	+= stp_core.o
obj-y	+= psm_core.o
obj-y	+= btm_core.o

endif