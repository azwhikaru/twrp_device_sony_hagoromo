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
#ifndef B43_PHY_LCN_H_
#define B43_PHY_LCN_H_

#include "phy_common.h"


#define B43_PHY_LCN_AFE_CTL1			B43_PHY_OFDM(0x03B)
#define B43_PHY_LCN_AFE_CTL2			B43_PHY_OFDM(0x03C)
#define B43_PHY_LCN_RF_CTL1			B43_PHY_OFDM(0x04C)
#define B43_PHY_LCN_RF_CTL2			B43_PHY_OFDM(0x04D)
#define B43_PHY_LCN_TABLE_ADDR			B43_PHY_OFDM(0x055) /* Table address */
#define B43_PHY_LCN_TABLE_DATALO		B43_PHY_OFDM(0x056) /* Table data low */
#define B43_PHY_LCN_TABLE_DATAHI		B43_PHY_OFDM(0x057) /* Table data high */
#define B43_PHY_LCN_RF_CTL3			B43_PHY_OFDM(0x0B0)
#define B43_PHY_LCN_RF_CTL4			B43_PHY_OFDM(0x0B1)
#define B43_PHY_LCN_RF_CTL5			B43_PHY_OFDM(0x0B7)
#define B43_PHY_LCN_RF_CTL6			B43_PHY_OFDM(0x0F9)
#define B43_PHY_LCN_RF_CTL7			B43_PHY_OFDM(0x0FA)


struct b43_phy_lcn {
	bool hw_pwr_ctl;
	bool hw_pwr_ctl_capable;
	u8 tx_pwr_curr_idx;
};


struct b43_phy_operations;
extern const struct b43_phy_operations b43_phyops_lcn;

#endif /* B43_PHY_LCN_H_ */
