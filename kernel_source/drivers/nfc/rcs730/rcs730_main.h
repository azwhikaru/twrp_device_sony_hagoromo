/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * Driver include for the RCS730 NFC chip.
 *
 * Copyright (C) 2014 Sony Corporation.
 *
 * Author:  Hiroki Matsuda <Hiroki.Matsuda@jp.sony.com>
 * Contact: Hiroki Matsuda <Hiroki.Matsuda@jp.sony.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _RCS730_MAIN_H_
#define _RCS730_MAIN_H_

#include <linux/ioctl.h>

/* Felica Link Registers */
#define  BLK__FELICA_LITE_S           0x0000
#define  BLK__MC__SYS_OP              0x0883
#define  BLK__HOST_THROUGH            0x0c00
#define  BLK__HOST_THROUGH_RESPONSE   0x0c0A

#define  REG__OPMODE                  0x0B00
#define  REG__TAG_TX_CONTROL          0x0B04
#define  REG__TAG_RX_CONTROL          0x0B08
#define  REG__RF_STATUS               0x0B0C
#define  REG__I2C_SLAVE_ADDRESS       0x0B10
#define  REG__I2C_BUFFER_CONTROL      0x0B14
#define  REG__I2C_STATUS              0x0B18
#define  REG__INTERRUPT_MASK          0x0B20
#define  REG__INTERRUPT_RAW_STATUS    0x0B24
#define  REG__INTERRUPT_STATUS        0x0B28
#define  REG__INTERRUPT_CLEAR         0x0B2C
#define  REG__WRITE_PROTECT           0x0B30
#define  REG__STANDBY_CONTROL         0x0B34
#define  REG__INITIALIZATION_CONTROL  0x0B38
#define  REG__HOST_IF_SECURITY        0x0B40
#define  REG__HOST_IF_WCNT            0x0B44
#define  REG__RF_PARAMETER            0x0B50
#define  REG__LITE_S_HT_CONFIG        0x0B60
#define  REG__LITE_S_PMm              0x0B64
#define  REG__PLUG_CONFIG1            0x0B80
#define  REG__PLUG_CONFIG2            0x0B84
#define  REG__PLUG_CONFIG3            0x0B88
#define  REG__NFC_DEP_CONFIG          0x0BA0
#define  REG__NFC_DEP_PMm1            0x0BA4
#define  REG__NFC_DEP_PMm2            0x0BA8
#define  REG__RW_CONFIG               0x0BC0
#define  REG__RW_CONTROL              0x0BC4
#define  REG__RW_TIMEOUT              0x0BC8


/* ioctl */
typedef enum
{
  RCS730_IOCTLNUM__NONE = 0,
  RCS730_IOCTLNUM__INITIALIZE, /* Initialize RCS730 Registers */
  RCS730_IOCTLNUM__SUSPEND,    /* Disable wireless interface */
  RCS730_IOCTLNUM__RESUME,     /* Enable wireless interface */
} t_rcs730_ioctl;

#define RCS730_CHAR_BASE           'R'
#define RCS730_IO(num)             _IO(RCS730_CHAR_BASE, num)
#define RCS730_IOR(num, dtype)     _IOR(RCS730_CHAR_BASE, num, dtype)
#define RCS730_IOW(num, dtype)     _IOW(RCS730_CHAR_BASE, num, dtype)
#define RCS730_IOCTL__INITIALIZE   RCS730_IO(RCS730_IOCTLNUM__INITIALIZE)
#define RCS730_IOCTL__SUSPEND      RCS730_IO(RCS730_IOCTLNUM__SUSPEND)
#define RCS730_IOCTL__RESUME       RCS730_IO(RCS730_IOCTLNUM__RESUME)

#endif /* _RCS730_MAIN_H_ */
