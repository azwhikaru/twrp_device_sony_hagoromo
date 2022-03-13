/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * bu9873.h - platform_data for the bu9873 rtc driver
 *
 * Copyright 2015 Sony Corporation
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _LINUX_BU9873_H
#define _LINUX_BU9873_H

#include <linux/types.h>

/* Register map */
#define BU9873_REG_SECOND   0x00    /*Second Counter*/
#define BU9873_REG_MINUTE   0x01    /*Minute Counter*/
#define BU9873_REG_HOUR     0x02    /*Hour Counter*/
#define BU9873_REG_DAY_OF_WEEK      0x03    /*Day-of-week Counter*/
#define BU9873_REG_DAY      0x04    /*Day Counter*/
#define BU9873_REG_MONTH        0x05    /*Month Counter*/
#define BU9873_REG_YEAR     0x06    /*Year Counter*/
#define BU9873_REG_TIME_TRIMMING        0x07 /*Time Trimming Register*/
#define BU9873_REG_ALARM_A_MINUTE       0x08    /*Alarm_A_Minute Register*/
#define BU9873_REG_ALARM_A_HOUR     0x09    /*Alarm_A_Hour Register*/
#define BU9873_REG_ALARM_A_DAY_OF_WEEK      0x0A    /*Alarm_A_Day-of-week Register*/
#define BU9873_REG_ALARM_B_MINUTE       0x0B    /*Alarm_B_Minute Register*/
#define BU9873_REG_ALARM_B_HOUR     0x0C    /*Alarm_B_Hour Register*/
#define BU9873_REG_ALARM_B_DAY_OF_WEEK      0x0D    /*Alarm_B_Day-of-week Register*/

#define BU9873_REG_CONTOROL_1       0x0E    /*Control Register 1*/
#define BU9873_REG_CONTOROL_1_AALE  0x80 /*ALARM_A_ENABLE*/
#define BU9873_REG_CONTOROL_1_BALE  0x40 /*ALARM_B_ENABLE*/
#define BU9873_REG_CONTOROL_1_CT        0x07    /*Periodic interrupt function selection*/

#define BU9873_REG_CONTOROL_2           0x0F/*Control Register 2*/
#define BU9873_REG_CONTOROL_2_12B_24        0x20    /*12B/24-hour mode selection bit*/
#define BU9873_REG_CONTOROL_2_ADJ       0x10    /*Second Adjust Bit*/
#define BU9873_REG_CONTOROL_2_XSTEP     0x10    /*Oscillator Halt Sensing Bit*/
#define BU9873_REG_CONTOROL_2_CLENB     0x08    /*Clock Output Bit*/
#define BU9873_REG_CONTOROL_2_CTFG      0x04    /*Periodic Interrupt Flag Bit*/
#define BU9873_REG_CONTOROL_2_AAFG      0x02    /*Alarm A Interrupt Flag Bit*/
#define BU9873_REG_CONTOROL_2_BAFG      0x01    /*Alarm B Interrupt Flag Bit*/

#define BU9873_I2C_INTERNAL_ADDR_MASK 0xf0
#define BU9873_I2C_TRANS_FORMAT_WRITE 0x0
#define BU9873_I2C_TRANS_FORMAT_CHANGE_FORMAT 0x4

struct bu9873_platform_data {
    unsigned int  rtc_intrb_gpio;
    unsigned int rtc_intrb_eint;
    int rtc_i2c_timing;
};

#define BU9873_NAME "rtc-bu9873"

#define BU9873_YEAR_OFFSET 100

/*
 * tm->tm_year is elapsed years from 1900
 * and this module is can handle 2000 - 2099 years
 * So min and max is following
*/
#define BU9873_YEAR_MIN 100 
#define BU9873_YEAR_MAX 199


#endif /* _LINUX_BU9873_H */
