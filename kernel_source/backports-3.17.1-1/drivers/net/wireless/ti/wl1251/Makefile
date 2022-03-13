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
wl1251-objs		= main.o event.o tx.o rx.o ps.o cmd.o \
			  acx.o boot.o init.o debugfs.o io.o
wl1251_spi-objs		+= spi.o
wl1251_sdio-objs	+= sdio.o

obj-$(CPTCFG_WL1251)		+= wl1251.o
obj-$(CPTCFG_WL1251_SPI)	+= wl1251_spi.o
obj-$(CPTCFG_WL1251_SDIO)	+= wl1251_sdio.o

ccflags-y += -D__CHECK_ENDIAN__
