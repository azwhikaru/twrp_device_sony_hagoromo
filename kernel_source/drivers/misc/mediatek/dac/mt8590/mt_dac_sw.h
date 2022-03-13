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
#ifndef _MTK_DAC_SW_H
#define _MTK_DAC_SW_H

/*
 * channel is 0 for channel A; channel is 1 for channel B;
 * if voltage is 0xff, the output voltage is 1.4v; 
 * if voltage is 0x00, the output voltage is 0v;
 */
extern int mt_dac_set(u8 channel, u8 voltage);

#endif   /*_MTK_DAC_SW_H*/

