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
#ifndef	__VSR_CMD_H__
#define	__VSR_CMD_H__
#include <linux/kernel.h>
#include "vsr_common.h"

#define	CMD_HEAD					((u8)0xFE)
#define	CMD_TAIL					((u8)0xFA)
#define	CMD_OPTCODE_READ_DATA		((u8)0x28)
#define	CMD_OPTCODE_READ_ACK		((u8)0x29)
#ifdef ADD_SYNC_CMD
#define	CMD_OPTCODE_START_XFER		((u8)0x2A)
#define	CMD_OPTCODE_READ_OVER		((u8)0x2B)
#define	CMD_OPTCODE_READ_STATE		((u8)0x2C)
#else
#define	CMD_OPTCODE_READ_STATE		((u8)0x2A)
#endif

#define	CMD_ACKCODE_BAD_CMD			((u8)0x00)
#define	CMD_ACKCODE_EFFECT_CMD		((u8)0x01)

#define	CMD_SYNC_FLAG_OK			((u8)0x01)
#define	CMD_STOP_EXFER				((u8)0x00)
#define	CMD_START_EXFER				CMD_SYNC_FLAG_OK
#define	CMD_READ_OVER_RET			CMD_SYNC_FLAG_OK

#define	CMD_STATE_IDLE				((u8)0x00)
#define	CMD_STATE_END				((u8)0x01)
#define	CMD_STATE_FAILED			((u8)0x02)
#define	CMD_STATE_TRAY_OUT			((u8)0x03)

typedef struct {
	u8	head;
	u8	optcode;
	u8	addr[4];
	u8	length[2];
	u8	chksum;
	u8	tail;
}CMD_READ_DATA;

typedef struct {
	u8	head;
	u8	optcode;
	u8	ackcode;
	u8	tail;
}CMD_READ_ACK;

#ifdef ADD_SYNC_CMD
typedef struct {
	u8	head;
	u8	optcode;
	u8	flag;
	u8	tail;
}CMD_START_XFER;

typedef struct {
	u8	head;
	u8	optcode;
	u8	result;
	u8	tail;
}CMD_READ_OVER;
#endif

typedef struct {
	u8	head;
	u8	optcode;
	u8	state;
	u8	tail;
}CMD_READ_STATE;

struct optcode_tail_offset {
	u8	optcode;
	u8	offset;
};

const CMD_READ_DATA *make_cmd_read_data(u32 block_base, u16 block_count);
#ifdef ADD_SYNC_CMD
const CMD_START_XFER *make_cmd_start_xfer(u8 start_flag);
const CMD_READ_OVER *make_cmd_read_over(u8 read_ret);
#endif
int is_cmd(const u8 *cmd, int len);
const char get_ackcode(const CMD_READ_ACK *ack);
const char get_read_state(const CMD_READ_STATE *state);
#endif	/*__VSR_CMD_H__*/
