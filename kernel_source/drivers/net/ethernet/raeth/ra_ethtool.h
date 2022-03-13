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
#ifndef RA_ETHTOOL_H
#define RA_ETHTOOL_H

/* ethtool related */
unsigned char get_current_phy_address(void);
int mdio_read(struct net_device *dev, int phy_id, int location);
void mdio_write(struct net_device *dev, int phy_id, int location, int value);

/* for pseudo interface */
int mdio_virt_read(struct net_device *dev, int phy_id, int location);
void mdio_virt_write(struct net_device *dev, int phy_id, int location, int value);

#endif
