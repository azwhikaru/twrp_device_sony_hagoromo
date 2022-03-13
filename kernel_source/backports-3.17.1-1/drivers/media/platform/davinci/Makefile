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
# Makefile for the davinci video device drivers.
#

#VPIF Display driver
obj-$(CPTCFG_VIDEO_DAVINCI_VPIF_DISPLAY) += vpif.o vpif_display.o
#VPIF Capture driver
obj-$(CPTCFG_VIDEO_DAVINCI_VPIF_CAPTURE) += vpif.o vpif_capture.o

# Capture: DM6446 and DM355
obj-$(CPTCFG_VIDEO_DM6446_CCDC) += vpfe_capture.o vpss.o dm644x_ccdc.o
obj-$(CPTCFG_VIDEO_DM355_CCDC) += vpfe_capture.o vpss.o dm355_ccdc.o
obj-$(CPTCFG_VIDEO_DM365_ISIF) += vpfe_capture.o vpss.o isif.o
obj-$(CPTCFG_VIDEO_DAVINCI_VPBE_DISPLAY) += vpss.o vpbe.o vpbe_osd.o \
	vpbe_venc.o vpbe_display.o
