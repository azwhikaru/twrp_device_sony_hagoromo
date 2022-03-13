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
#ifndef	__VSR_DATA_XFER_H__
#define	__VSR_DATA_XFER_H__
#include "vsr_common.h"
#include "vsr_blk.h"

//#define USE_KTHREAD

#define XFER_INTERFACE_BUF_SIZE				10
#define RECV_CMD_BUF_SIZE					4
#define	VSR_WAIT_ACK_DELAY_TIME				(10*HZ)		//1000ms
#define	VSR_WAIT_XFER_OVER_DELAY_TIME		(60*HZ)		//60s

#define	XFER_DATA_OVER			(int)1


//#define	LINUX_SECTOR_SIZE_2048

#ifdef	LINUX_SECTOR_SIZE_2048
#define	REQ_SECTOR_TO_LOADER_BLOCK(sector)		((u32)(sector))
#else
#define	REQ_SECTOR_TO_LOADER_BLOCK(sector)		((u32)(sector >> 2))

static inline u32 byte_to_sector(u32 len)
{
	return (len % KERNEL_SECTOR_SIZE) ? (len >> 9) + 1 : (len >> 9);
}

static inline u32 sector_to_loader_block_addr(u32 sector_addr, u32 sector_len)
{
	return sector_addr >> 2;
}

static inline u16 sector_to_loader_block_len(u32 sector_addr, u32 sector_len)
{
	if (0 == (sector_addr % 4))
	{
		return (u16)(sector_len % 4 ? (sector_len >> 2) + 1 : (sector_len >> 2));
	}
	else
	{
		return (u16)((((sector_addr % 4) + sector_len) % 4) ? ((((sector_addr % 4) + sector_len) >> 2) + 1) : (((sector_addr % 4) + sector_len)) >> 2);
	}
}

#endif
#define	SIZE_TO_BLOCK_COUNT(size)				((u32)(size >> 11))
#define	BLOCK_COUNT_TO_SIZE(block)				((size_t)(block << 11))
#define	SECTOR_COUNT_TO_SIZE(sector)			((size_t)(sector << 9))

#define SM_IDLE								((u8)0)
#define	SM_SEND_READ_CMD					((u8)1)
#define	SM_WAITING_FOR_ACK					((u8)2)
#define	SM_RECVED_ACK						((u8)3)
#define	SM_WAITING_ACK_TIMEOUT				((u8)4)
#define	SM_CHECKING_ACK						((u8)5)
#define	SM_XFER_BEGIN						((u8)6)
#define	SM_BAD_CMD							((u8)7)
#define	SM_XFERRING_DATAS					((u8)8)
#define	SM_XFER_DATAS_OVER					((u8)9)
#define	SM_CHECK_READ_STATE					((u8)10)
#define	SM_XFER_DATAS_SUCCESS				((u8)11)
#define	SM_XFER_DATAS_FAILED				((u8)12)
#define	SM_XFER_DATAS_STOP					((u8)13)


typedef	int (*proc_cmd)(struct state_machine *, const u8 *);

struct optcode_proc_func {
	u8 optcode;
	proc_cmd proc_func;
};

typedef const u8 *(*make_cmd)(u8);
struct sm_send_cmd {
	u8 sm_state;
	int cmd_size;
	make_cmd make_cmd_func;
};


typedef	int (*proc_err_state)(struct state_machine *);

struct errcode_proc_func {
	u8 errcode;
	proc_err_state proc_func;
};

void state_machine_init(struct state_machine *sm);
int read_from_dev(struct vsr_dev *dev, char *buf, u32 sector_addr, u32 sector_len);
int proc_recv_data(void);

#ifdef USE_KTHREAD
wait_queue_head_t *get_recv_task_qh(void);
int recv_kthread_init(void);
int recv_kthread_stop(void);
#endif

#endif	/*__VSR_DATA_XFER_H__*/

