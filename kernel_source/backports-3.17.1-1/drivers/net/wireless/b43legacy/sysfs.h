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
#ifndef B43legacy_SYSFS_H_
#define B43legacy_SYSFS_H_

struct b43legacy_wldev;

int b43legacy_sysfs_register(struct b43legacy_wldev *dev);
void b43legacy_sysfs_unregister(struct b43legacy_wldev *dev);

#endif /* B43legacy_SYSFS_H_ */
