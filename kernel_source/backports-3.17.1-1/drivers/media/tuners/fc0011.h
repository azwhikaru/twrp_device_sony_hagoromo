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
#ifndef LINUX_FC0011_H_
#define LINUX_FC0011_H_

#include <linux/kconfig.h>
#include "dvb_frontend.h"


/** struct fc0011_config - fc0011 hardware config
 *
 * @i2c_address: I2C bus address.
 */
struct fc0011_config {
	u8 i2c_address;
};

/** enum fc0011_fe_callback_commands - Frontend callbacks
 *
 * @FC0011_FE_CALLBACK_POWER: Power on tuner hardware.
 * @FC0011_FE_CALLBACK_RESET: Request a tuner reset.
 */
enum fc0011_fe_callback_commands {
	FC0011_FE_CALLBACK_POWER,
	FC0011_FE_CALLBACK_RESET,
};

#if IS_ENABLED(CPTCFG_MEDIA_TUNER_FC0011)
struct dvb_frontend *fc0011_attach(struct dvb_frontend *fe,
				   struct i2c_adapter *i2c,
				   const struct fc0011_config *config);
#else
static inline
struct dvb_frontend *fc0011_attach(struct dvb_frontend *fe,
				   struct i2c_adapter *i2c,
				   const struct fc0011_config *config)
{
	dev_err(&i2c->dev, "fc0011 driver disabled in Kconfig\n");
	return NULL;
}
#endif

#endif /* LINUX_FC0011_H_ */
