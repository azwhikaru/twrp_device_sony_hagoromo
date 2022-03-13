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
obj-$(CPTCFG_IWLEGACY)	+= iwlegacy.o
iwlegacy-objs 		:= common.o
iwlegacy-$(CPTCFG_IWLEGACY_DEBUGFS) += debug.o

iwlegacy-objs += $(iwlegacy-m)

# 4965
obj-$(CPTCFG_IWL4965)	+= iwl4965.o
iwl4965-objs		:= 4965.o 4965-mac.o 4965-rs.o 4965-calib.o
iwl4965-$(CPTCFG_IWLEGACY_DEBUGFS) += 4965-debug.o

# 3945
obj-$(CPTCFG_IWL3945)	+= iwl3945.o
iwl3945-objs		:= 3945-mac.o 3945.o 3945-rs.o
iwl3945-$(CPTCFG_IWLEGACY_DEBUGFS) += 3945-debug.o

ccflags-y += -D__CHECK_ENDIAN__
