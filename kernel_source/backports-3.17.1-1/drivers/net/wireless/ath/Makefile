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
obj-$(CPTCFG_ATH5K)		+= ath5k/
obj-$(CPTCFG_ATH9K_HW)		+= ath9k/
obj-$(CPTCFG_CARL9170)		+= carl9170/
obj-$(CPTCFG_ATH6KL)		+= ath6kl/
obj-$(CPTCFG_AR5523)		+= ar5523/
obj-$(CPTCFG_WIL6210)		+= wil6210/
obj-$(CPTCFG_ATH10K)		+= ath10k/
obj-$(CPTCFG_WCN36XX)		+= wcn36xx/

obj-$(CPTCFG_ATH_COMMON)	+= ath.o

ath-objs :=	main.o \
		regd.o \
		hw.o \
		key.o \
		dfs_pattern_detector.o \
		dfs_pri_detector.o

ath-$(CPTCFG_ATH_DEBUG) += debug.o
ccflags-y += -D__CHECK_ENDIAN__
