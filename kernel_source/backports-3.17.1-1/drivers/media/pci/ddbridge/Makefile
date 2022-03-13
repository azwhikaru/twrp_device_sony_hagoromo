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
#
# Makefile for the ddbridge device driver
#

ddbridge-objs := ddbridge-core.o

obj-$(CPTCFG_DVB_DDBRIDGE) += ddbridge.o

ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core/
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-frontends/
ccflags-y += -I$(backport_srctree)/drivers/media/tuners/

# For the staging CI driver cxd2099
ccflags-y += -I$(backport_srctree)/drivers/staging/media/cxd2099/
