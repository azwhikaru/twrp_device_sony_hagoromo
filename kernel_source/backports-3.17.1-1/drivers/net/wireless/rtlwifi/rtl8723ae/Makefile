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
obj-m := rtl8723ae.o


rtl8723ae-objs :=		\
		dm.o		\
		fw.o		\
		hal_btc.o	\
		hal_bt_coexist.o\
		hw.o		\
		led.o		\
		phy.o		\
		pwrseq.o	\
		rf.o		\
		sw.o		\
		table.o		\
		trx.o		\


obj-$(CPTCFG_RTL8723AE) += rtl8723ae.o

ccflags-y += -D__CHECK_ENDIAN__
