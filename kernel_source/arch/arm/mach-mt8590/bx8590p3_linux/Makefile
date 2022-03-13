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

include $(srctree)/drivers/misc/mediatek/Makefile.custom

ifneq ($(CONFIG_ARCH_MTK_PROJECT),)
ARCH_MTK_PROJECT := $(shell echo $(CONFIG_ARCH_MTK_PROJECT))
obj-y   += core/
obj-y   += dct/
endif

#obj-y   +=  accelerometer/
#obj-y   +=  alsps/
obj-y   +=  cam_cal/
obj-y   +=  camera/
obj-y   +=  eeprom/
#obj-y   +=  flashlight/
#obj-y   +=  gyroscope/
obj-y   +=  headset/
obj-y   +=  imgsensor/
obj-y   +=  kpd/
#obj-y   +=  lcm/
ifneq ($(CONFIG_MTK_FB),)
obj-y   += leds/
else
obj-$(CONFIG_MTK_LEDS) += leds/
endif
obj-y   +=  lens/
#obj-y   +=  magnetometer/
obj-y   +=  sound/
obj-y   +=  ssw/
obj-$(CONFIG_MTK_TOUCHPANEL)   +=  touchpanel/
obj-y   +=  vibrator/

