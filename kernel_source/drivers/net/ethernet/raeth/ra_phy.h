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
#ifndef _RA_PHY_H
#define _RA_PHY_H

#include "raether.h"

#define PHY_REG_IDENTFIR2   (3)      /* Reg3: PHY Identifier 2 */
#define PHYID2_RTL8201FR    (0xC816) /*Realtek RTL8201FR */
#define PHYID2_SMSC8710A    (0xC0F1) /*SMSC LAN8710A */
#define PHYID2_DM9162_XMII  (0xB8A0)    /*davicom 9162 mii/rmii */  

extern int ra_detect_phy(END_DEVICE *ei_local);

#endif /* _RA_PHY_H */
