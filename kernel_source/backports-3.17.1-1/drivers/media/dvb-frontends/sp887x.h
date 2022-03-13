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
/*
   Driver for the Spase sp887x demodulator
*/

#ifndef SP887X_H
#define SP887X_H

#include <linux/dvb/frontend.h>
#include <linux/firmware.h>

struct sp887x_config
{
	/* the demodulator's i2c address */
	u8 demod_address;

	/* request firmware for device */
	int (*request_firmware)(struct dvb_frontend* fe, const struct firmware **fw, char* name);
};

#if IS_ENABLED(CPTCFG_DVB_SP887X)
extern struct dvb_frontend* sp887x_attach(const struct sp887x_config* config,
					  struct i2c_adapter* i2c);
#else
static inline struct dvb_frontend* sp887x_attach(const struct sp887x_config* config,
					  struct i2c_adapter* i2c)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif // CPTCFG_DVB_SP887X

#endif // SP887X_H
