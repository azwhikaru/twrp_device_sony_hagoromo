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
#ifndef __VSR_BLK_H__
#define __VSR_BLK_H__
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#define	VSR_WORK_QUEUE	"vsr_work_queue"
extern struct state_machine stmch;

#define MD_NOQUEUE	0
#define MD_SIMPLE	1
#define MD_FULL		2

#define	VSR_MAJOR				0
#define	VSR_DEV_NAME			"vsr"
#define	KERNEL_SECTOR_SIZE		512
#define	DISK_CAPACITY_SIZE		(((1 << 10) << 10) << 2)			//4M


struct state_machine {
	u8 state;
	spinlock_t	lock;
};

struct vsr_dev {
	int	major;
	int minor;
	u8 *disk_data;
	struct state_machine stmch;
	struct timer_list timer;
	spinlock_t lock;
	wait_queue_head_t wait_xfer;
	struct workqueue_struct *vsr_work_queue;
	struct delayed_work	recv_work;
	int	xfer_over;
	struct gendisk *gdk;
	struct request_queue *rq_queue;
	struct class *dev_class;
};
#endif	/*__VSR_BLK_H__*/