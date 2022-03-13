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
earth-pt1-objs := pt1.o va1j5jf8007s.o va1j5jf8007t.o

obj-$(CPTCFG_DVB_PT1) += earth-pt1.o

ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core -I$(backport_srctree)/drivers/media/dvb-frontends
