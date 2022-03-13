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
#ifndef _COMPAT_LINUX_EEPROM_93CX6_H
#define _COMPAT_LINUX_EEPROM_93CX6_H 1

#include_next <linux/eeprom_93cx6.h>

#ifndef PCI_EEPROM_WIDTH_93C86
#define PCI_EEPROM_WIDTH_93C86	8
#endif /* PCI_EEPROM_WIDTH_93C86 */

#endif	/* _COMPAT_LINUX_EEPROM_93CX6_H */
