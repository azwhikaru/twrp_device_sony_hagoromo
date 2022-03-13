/*
* Copyright (C) 2011-2015 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _MTK_DRV_MT8135_MHL_SLT_H
#define _MTK_DRV_MT8135_MHL_SLT_H

//#define MTK_TVE_LOG_DEBUG
//#define TVE_TEST_CLOCK_SEL_MIPI_OR_TVD

#define CHECK_TVE_NTSC_VERIFY				0x00
#define CHECK_TVE_PAL_VERIFY				0x01
#define CHECK_TVE_TEST_PATTERN				0x02
#define CHECK_TVE_MACROVERSION_TEST		    0x03
#define CHECK_DPI0_TEST_PATTERN				0x04
#define CHECK_DPI1_TEST_PATTERN				0x05
#define CHECK_MONITOR_OUTPUTA				0x06
#define CHECK_MONITOR_OUTPUTB				0x07
#define CHECK_TVE_DEBUG_LOG				    0x08

//extern void hdmi_config_pll(unsigned int resolutionmode);
//extern void dpi_setting_res(unsigned char arg);
#endif  
