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
#ifndef _XHCI_MTK_POWER_H
#define _XHCI_MTK_POWER_H

#include <linux/usb.h>
#include <xhci.h>

void enableXhciAllPortPower(struct xhci_hcd *xhci);
void disableXhciAllPortPower(struct xhci_hcd *xhci);
void enableAllClockPower(bool is_reset);
void disableAllClockPower(void);
void disablePortClockPower(int port_index, int port_rev);
void enablePortClockPower(int port_index, int port_rev);

#ifdef CONFIG_USB_MTK_DUALMODE
void mtk_switch2host(void);
void mtk_switch2device(bool skip);
#endif

#endif
