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
#ifndef __DONGLE_HDMI_H__
#define __DONGLE_HDMI_H__

#include <linux/notifier.h>

enum DONGLE_HDMI_STATUS {
	DONGLE_HDMI_POWER_ON = 1,
	DONGLE_HDMI_POWER_OFF,
	DONGLE_HDMI_MAx
};

/*Register to slimport driver*/
int hdmi_driver_notifier_register(struct notifier_block *nb);
int hdmi_driver_notifier_unregister(struct notifier_block *nb);

#endif /*__DONGLE_HDMI_H__*/

