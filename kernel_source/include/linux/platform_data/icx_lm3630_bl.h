/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 *  Copyright 2012,2013,2015 SONY corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef _LM3630_H_
#define _LM3630_H_

struct lm3630_bl_platform_data {

	int pwm_id;
	int blen_gpio;
};

#define LM3630_REG_CONTROL 0x00
#define LM3630_REG_CONFIGURATION 0x01
#define LM3630_REG_BOOST_CONTROL 0x02
#define LM3630_REG_BRIGHTNESS_A 0x03
#define LM3630_REG_BRIGHTNESS_B 0x04
#define LM3630_REG_CURRENT_A 0x05
#define LM3630_REG_CURRENT_B 0x06
#define LM3630_REG_ON_OFF_RAMP 0x07
#define LM3630_REG_RUN_RAMP 0x08
#define LM3630_REG_INTERRUPT_STATUS 0x09
#define LM3630_REG_INTERRUPT_ENABLE 0x0A
#define LM3630_REG_FAULT_STATUS 0x0B
#define LM3630_REG_SOFTWARE_RESET 0x0F
#define LM3630_REG_PWM_OUT_LOW 0x12
#define LM3630_REG_PWM_OUT_HIGH 0x13
#define LM3630_REG_REVISION 0x1F
#define LM3630_REG_FILTER_STRENGTH 0x50

#define LM3630_NAME "icx_lm3630"

void icx_lm3630_enable(void);

#endif /* _LM3630_H_ */
