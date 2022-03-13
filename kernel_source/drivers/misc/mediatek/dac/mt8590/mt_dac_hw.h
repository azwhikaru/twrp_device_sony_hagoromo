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
#ifndef _MTK_DAC_HW_H
#define _MTK_DAC_HW_H

#define DAC_BASE 0xF0010000

#define MIPI_8BDAC_40                     (DAC_BASE + 0x440)
#define MIPI_8BDAC_44                     (DAC_BASE + 0x444)
#define MIPI_8BDAC_38                     (DAC_BASE + 0x438)
#define MIPI_8BDAC_3C                     (DAC_BASE + 0x43c)
#define MIPI_8BDAC_ANA30                     (DAC_BASE + 0x430)
#define MIPI_8BDAC_ANA34                     (DAC_BASE + 0x434)


#define DEBUG_MON_CON0 0xF0209E20
#define CLK_8BDAC_CFG 0xF0000150
#define CLK_CFG_15 0xF00000F0

#endif	 /*_MTK_DAC_HW_H*/
