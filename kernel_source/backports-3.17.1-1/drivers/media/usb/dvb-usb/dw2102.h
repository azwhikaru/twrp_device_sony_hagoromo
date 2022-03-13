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
#ifndef _DW2102_H_
#define _DW2102_H_

#define DVB_USB_LOG_PREFIX "dw2102"
#include "dvb-usb.h"

#define deb_xfer(args...) dprintk(dvb_usb_dw2102_debug, 0x02, args)
#define deb_rc(args...)   dprintk(dvb_usb_dw2102_debug, 0x04, args)
#endif
