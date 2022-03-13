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
mantis_core-objs :=	mantis_ioc.o	\
			mantis_uart.o	\
			mantis_dma.o	\
			mantis_pci.o	\
			mantis_i2c.o	\
			mantis_dvb.o	\
			mantis_evm.o	\
			mantis_hif.o	\
			mantis_ca.o	\
			mantis_pcmcia.o	\
			mantis_input.o

mantis-objs	:=	mantis_cards.o	\
			mantis_vp1033.o	\
			mantis_vp1034.o	\
			mantis_vp1041.o	\
			mantis_vp2033.o	\
			mantis_vp2040.o	\
			mantis_vp3030.o

hopper-objs	:=	hopper_cards.o	\
			hopper_vp3028.o

obj-$(CPTCFG_MANTIS_CORE)	+= mantis_core.o
obj-$(CPTCFG_DVB_MANTIS)	+= mantis.o
obj-$(CPTCFG_DVB_HOPPER)	+= hopper.o

ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core/ -I$(backport_srctree)/drivers/media/dvb-frontends/
