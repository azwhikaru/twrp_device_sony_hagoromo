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
img-ir-y			:= img-ir-core.o
img-ir-$(CPTCFG_IR_IMG_RAW)	+= img-ir-raw.o
img-ir-$(CPTCFG_IR_IMG_HW)	+= img-ir-hw.o
img-ir-$(CPTCFG_IR_IMG_NEC)	+= img-ir-nec.o
img-ir-$(CPTCFG_IR_IMG_JVC)	+= img-ir-jvc.o
img-ir-$(CPTCFG_IR_IMG_SONY)	+= img-ir-sony.o
img-ir-$(CPTCFG_IR_IMG_SHARP)	+= img-ir-sharp.o
img-ir-$(CPTCFG_IR_IMG_SANYO)	+= img-ir-sanyo.o
img-ir-objs			:= $(img-ir-y)

obj-$(CPTCFG_IR_IMG)		+= img-ir.o
