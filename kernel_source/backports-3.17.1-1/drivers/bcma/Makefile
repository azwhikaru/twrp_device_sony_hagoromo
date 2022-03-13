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
bcma-y					+= main.o scan.o core.o sprom.o
bcma-y					+= driver_chipcommon.o driver_chipcommon_pmu.o
bcma-$(CPTCFG_BCMA_SFLASH)		+= driver_chipcommon_sflash.o
bcma-$(CPTCFG_BCMA_NFLASH)		+= driver_chipcommon_nflash.o
bcma-y					+= driver_pci.o
bcma-y					+= driver_pcie2.o
bcma-$(CPTCFG_BCMA_DRIVER_PCI_HOSTMODE)	+= driver_pci_host.o
bcma-$(CPTCFG_BCMA_DRIVER_MIPS)		+= driver_mips.o
bcma-$(CPTCFG_BCMA_DRIVER_GMAC_CMN)	+= driver_gmac_cmn.o
bcma-$(CPTCFG_BCMA_DRIVER_GPIO)		+= driver_gpio.o
bcma-$(CPTCFG_BCMA_HOST_PCI)		+= host_pci.o
bcma-$(CPTCFG_BCMA_HOST_SOC)		+= host_soc.o
obj-$(CPTCFG_BCMA)			+= bcma.o

ccflags-$(CPTCFG_BCMA_DEBUG)		:= -DDEBUG
