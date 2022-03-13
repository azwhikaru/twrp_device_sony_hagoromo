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
p54common-objs			:= eeprom.o fwio.o txrx.o main.o
p54common-$(CPTCFG_P54_LEDS)	+= led.o

obj-$(CPTCFG_P54_COMMON)	+= p54common.o
obj-$(CPTCFG_P54_USB)		+= p54usb.o
obj-$(CPTCFG_P54_PCI)		+= p54pci.o
obj-$(CPTCFG_P54_SPI)		+= p54spi.o
