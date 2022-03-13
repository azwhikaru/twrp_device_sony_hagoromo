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
# Makefile for PN544 HCI based NFC driver
#

pn544_i2c-objs  = i2c.o
pn544_mei-objs  = mei.o

obj-$(CPTCFG_NFC_PN544)     += pn544.o
obj-$(CPTCFG_NFC_PN544_I2C) += pn544_i2c.o
obj-$(CPTCFG_NFC_PN544_MEI) += pn544_mei.o
