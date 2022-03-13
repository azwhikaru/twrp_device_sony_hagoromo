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
obj-$(CPTCFG_SOC_CAMERA)		+= soc_camera.o soc_mediabus.o
obj-$(CPTCFG_SOC_CAMERA_SCALE_CROP)	+= soc_scale_crop.o

# a platform subdevice driver stub, allowing to support cameras by adding a
# couple of callback functions to the board code
obj-$(CPTCFG_SOC_CAMERA_PLATFORM)	+= soc_camera_platform.o

# soc-camera host drivers have to be linked after camera drivers
obj-$(CPTCFG_VIDEO_ATMEL_ISI)		+= atmel-isi.o
obj-$(CPTCFG_VIDEO_MX2)			+= mx2_camera.o
obj-$(CPTCFG_VIDEO_MX3)			+= mx3_camera.o
obj-$(CPTCFG_VIDEO_OMAP1)		+= omap1_camera.o
obj-$(CPTCFG_VIDEO_PXA27x)		+= pxa_camera.o
obj-$(CPTCFG_VIDEO_SH_MOBILE_CEU)	+= sh_mobile_ceu_camera.o
obj-$(CPTCFG_VIDEO_SH_MOBILE_CSI2)	+= sh_mobile_csi2.o
obj-$(CPTCFG_VIDEO_RCAR_VIN)		+= rcar_vin.o
