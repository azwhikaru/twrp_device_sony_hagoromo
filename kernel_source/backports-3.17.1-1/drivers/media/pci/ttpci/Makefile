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
# Makefile for the kernel SAA7146 FULL TS DVB device driver
# and the AV7110 DVB device driver
#

dvb-ttpci-objs := av7110_hw.o av7110_v4l.o av7110_av.o av7110_ca.o av7110.o av7110_ipack.o

ifdef CONFIG_INPUT_EVDEV
dvb-ttpci-objs += av7110_ir.o
endif

obj-$(CPTCFG_TTPCI_EEPROM) += ttpci-eeprom.o
obj-$(CPTCFG_DVB_BUDGET_CORE) += budget-core.o
obj-$(CPTCFG_DVB_BUDGET) += budget.o
obj-$(CPTCFG_DVB_BUDGET_AV) += budget-av.o
obj-$(CPTCFG_DVB_BUDGET_CI) += budget-ci.o
obj-$(CPTCFG_DVB_BUDGET_PATCH) += budget-patch.o
obj-$(CPTCFG_DVB_AV7110) += dvb-ttpci.o

ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core/ -I$(backport_srctree)/drivers/media/dvb-frontends/
ccflags-y += -I$(backport_srctree)/drivers/media/tuners
