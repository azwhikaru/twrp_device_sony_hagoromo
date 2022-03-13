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
#ifndef	__VSR_INTERFACE_H__
#define	__VSR_INTERFACE_H__

#ifdef	UNIT_TEST
//#include "vsr_unit_test.h"
extern int usb_write_datas(const u8 *cmd, int len);
extern int usb_read_datas(u8 *buf, int len);
#else
//from usb by joson
extern int MTK_USB_Open(void);
extern int MTK_USB_Close(void);
extern int MTK_USB_Write(char *buffer, unsigned size);
extern void MTK_USB_Register_Read_Callback(void* function);
extern int MTK_USB_Read(char *buffer, unsigned size);
extern int usb_write_datas(const u8 *cmd, int len);
#endif

static inline int open_xfer_intf(void)
{
#ifndef	UNIT_TEST
	return MTK_USB_Open();
#endif
}

static inline int close_xfer_intf(void)
{
#ifndef	UNIT_TEST
	return MTK_USB_Close();
#endif
}

static inline int send_cmd(const u8 *cmd, int len)
{
#ifdef	UNIT_TEST
	usb_write_datas(cmd, len);
	return 0;
#else
	//usb_write_datas(cmd, len);
	return MTK_USB_Write((char *)cmd, (u32)len);
#endif
}

static inline int read_datas(u8 *buf, int len)
{
#ifdef	UNIT_TEST
	return usb_read_datas(buf, len);
#else
	return MTK_USB_Read((char *)buf, (u32)len);
#endif
	//return 0;
}

void call_proc_recv_data(void);

static inline void set_read_call_back_func(void)
{
#ifndef	UNIT_TEST
	MTK_USB_Register_Read_Callback(call_proc_recv_data);
#endif
}

static inline void clr_read_call_back_func(void)
{
#ifndef	UNIT_TEST
	MTK_USB_Register_Read_Callback(NULL);
#endif
}

#endif	/*__VSR_INTERFACE_H__*/
