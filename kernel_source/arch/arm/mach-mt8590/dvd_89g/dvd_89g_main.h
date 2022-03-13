/*
 * Driver include for the DVD 89G Driver.
 *
 * Copyright (C) 2016 Sony Corporation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _DVD_89G_MAIN_H_
#define _DVD_89G_MAIN_H_

#include <linux/ioctl.h>
#include <cust_gpio_usage.h>

#define DVD_89G_POWER      GPIO_DVD_89G_POWER_PIN
#define DVD_89G_RESET      GPIO_DVD_89G_RESET_PIN
#define DVD_89G_UART_URXD1 GPIO_UART_URXD2_PIN
#define DVD_89G_UART_UTXD1 GPIO_UART_UTXD2_PIN
#define DVD_89G_HDMI_5V    GPIO_DVD_89G_HDMI_5V_PIN
#define DVD_89G_USB_SEL    GPIO_DVD_89G_USB_SEL_PIN
#define DVD_89G_USB_VBUS1  GPIO_OTG_DRVVBUS1_PIN

#define GPIO_HIGH 1
#define GPIO_LOW  0

/* Major & Minor */
#define DVD_89G_CDEV_FILE_NAME "dvd_89g"

/* ioctl */
typedef enum
{
  DVD_89G_IOCTLNUM_NONE = 0,
  DVD_89G_IOCTLNUM_START,
  DVD_89G_IOCTLNUM_STOP,
  DVD_89G_IOCTLNUM_USB_SWITCH,
  DVD_89G_IOCTLNUM_USB_GET_STATE,
  DVD_89G_IOCTLNUM_RESTART,
} t_DVD_89G_ioctl;

#define DVD_89G_CHAR_BASE              'R'
#define DVD_89G_IO(num)                _IO(DVD_89G_CHAR_BASE, num)
#define DVD_89G_IOCTL_START            DVD_89G_IO(DVD_89G_IOCTLNUM_START)
#define DVD_89G_IOCTL_STOP             DVD_89G_IO(DVD_89G_IOCTLNUM_STOP)
#define DVD_89G_IOCTL_USB_SWITCH       DVD_89G_IO(DVD_89G_IOCTLNUM_USB_SWITCH)
#define DVD_89G_IOCTL_USB_GET_STATE    DVD_89G_IO(DVD_89G_IOCTLNUM_USB_GET_STATE)
#define DVD_89G_IOCTL_RESTART          DVD_89G_IO(DVD_89G_IOCTLNUM_RESTART)

#endif /* _DVD_89G_MAIN_H_ */
