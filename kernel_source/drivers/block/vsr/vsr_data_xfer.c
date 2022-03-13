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
#include <asm/system.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/err.h>

#include "vsr_common.h"
#include "vsr_data_xfer.h"
#include "vsr_blk.h"
#include "vsr_cmd.h"
#include "vsr_interface.h"
#ifdef	UNIT_TEST
#include "vsr_unit_test.h"
#endif


extern struct vsr_dev *get_vsr_dev(void);
extern void proc_timer_out(struct state_machine *sm);

static char *recv_buf = NULL;
static u16 read_block_len = 0;
static u32 read_sector_addr = 0;
static u32 read_sector_len = 0;

static inline void set_recv_buf(char *buf)
{
	recv_buf = buf;
}

static inline char *get_recv_buf(void)
{
	return recv_buf;
}

static inline void set_read_block_len(u32 block_len)
{
	read_block_len = block_len;
}

static inline u16 get_read_block_len(void)
{
	return read_block_len;
}

static inline void set_read_sector_addr(u32 sector_addr)
{
	read_sector_addr = sector_addr;
}

static inline u32 get_read_sector_addr(void)
{
	return read_sector_addr;
}

static inline void set_read_sector_len(u32 sector_len)
{
	read_sector_len = sector_len;
}

static inline u32 get_read_sector_len(void)
{
	return read_sector_len;
}


void state_machine_init(struct state_machine *sm)
{
	CHECK_PRT_VOID(sm);
	spin_lock_init(&sm->lock);
	sm->state = SM_IDLE;
}

static inline void set_sm_state(struct state_machine *sm, u8 state)
{
	//spin_lock_irq(&sm->lock);
	sm->state = state;
	//spin_unlock_irq(&sm->lock);
}

static inline u8 get_sm_state(const struct state_machine *sm)
{
	return (u8)sm->state;
}

static inline void set_xfer_flag(struct vsr_dev *dev)
{
	spin_lock(&dev->lock);
	dev->xfer_over = XFER_DATA_OVER;
	spin_unlock(&dev->lock);
}

static inline int get_xfer_flag(struct vsr_dev *dev)
{
	return dev->xfer_over;
}

static void vsr_timer_init(struct vsr_dev *dev, void (*func)(u32), u32 data, u32 delay)
{
	init_timer(&dev->timer);
	dev->timer.function = func;
	dev->timer.data = data;
	dev->timer.expires  = jiffies + delay;
	add_timer(&dev->timer);
}

int read_from_dev(struct vsr_dev *dev, char *buf, u32 sector_addr, u32 sector_len)
{
#ifdef	UNIT_TEST
	int i;
#endif
	int ret = 0;
	u32 block_addr = 0;
	u16 block_len = 0;
	CMD_READ_DATA *cmd_read = NULL;
	CHECK_PRT_INT(dev);
	CHECK_PRT_INT(buf);

	if (0 == sector_len)
	{
		VSR_ERR("read cmd NO_DATA!");
		return NO_DATA;
	}

	set_read_sector_addr(sector_addr);
	set_read_sector_len(sector_len);
	block_addr = sector_to_loader_block_addr(sector_addr, sector_len);
	block_len = sector_to_loader_block_len(sector_addr, sector_len);
	
	VSR_DBG("ready for read dev!");
	set_recv_buf(buf);
	set_read_block_len(block_len);
	cmd_read = (CMD_READ_DATA *)make_cmd_read_data(block_addr, block_len);
	CHECK_PRT_INT(cmd_read);
	VSR_DBG("before check SM_IDLE!");

	if (SM_IDLE != get_sm_state(&dev->stmch))
	{
		VSR_DBG("Waiting for sm is idle!");
		ret = 0;
		goto out;
	}
	VSR_DBG("set state machine SM_SEND_READ_CMD!");
	set_sm_state(&dev->stmch, SM_SEND_READ_CMD);

	
	VSR_DBG("ready for send cmd!");
	set_sm_state(&dev->stmch, SM_WAITING_FOR_ACK);
	if (sizeof(CMD_READ_DATA) != send_cmd((u8 *)cmd_read, sizeof(CMD_READ_DATA)))
	{
		VSR_ERR("Send cmd failed!");
		ret = -SEND_CMD_ERR;
		goto out;
	}

	VSR_DBG("send cmd success!");
#ifdef	TIMER_OUT
	vsr_timer_init(dev, proc_timer_out, (u32)&dev->stmch, VSR_WAIT_ACK_DELAY_TIME);
#endif	
	VSR_DBG("waiting for xfer data over!");
#ifdef UNIT_TEST
	start_recv();
#endif

	if (wait_event_interruptible(dev->wait_xfer, (XFER_DATA_OVER == dev->xfer_over)))
	{
		VSR_ERR("Waiting for xfer data error!");
		ret = -WAIT_XFER_DATA_ERR;
		goto out;
	}

	if (SM_XFER_DATAS_SUCCESS != get_sm_state(&dev->stmch))
	{
		VSR_ERR("Bad xfer loop!");
		ret = -BAD_XFER_LOOP;
		goto out;
	}

#ifdef UNIT_TEST
	stop_recv();
#endif	
#if	0
	for (i = 0; i < 2048; i++)
	{
		DEBUG("recv_buf[%d] = %d\n", i, recv_buf[i]);
	}
#endif

out:
	set_sm_state(&dev->stmch, SM_IDLE);
	dev->xfer_over = 0;
	//close_xfer_intf();
	VSR_DBG("read data over!\n\n\n");
	return ret;
}

static inline int _set_sm_state_dbg(struct state_machine *sm, u8 state, const char *info)
{
	set_sm_state(sm, state);
	VSR_DBG("%s", info);
	return state;
}

static inline int _set_sm_state_err(struct state_machine *sm, u8 state, const char *info)
{
	set_sm_state(sm, state);
	VSR_ERR("%s", info);
	return state;
}

static inline int _set_sm_state_wan(struct state_machine *sm, u8 state, const char *info)
{
	set_sm_state(sm, state);
	VSR_WAN("%s", info);
	return state;
}

static int xfer_datas(struct state_machine *sm, size_t total_size, size_t cur_recv_size)
{
	int ret = 0;
	CHECK_PRT_INT(sm);

	if ((SM_XFER_BEGIN != get_sm_state(sm)) && (SM_XFERRING_DATAS != get_sm_state(sm)))
	{
		VSR_DBG("Nothing to do!");
		return 0;
	}

	if (cur_recv_size >= total_size)
	{
		ret = _set_sm_state_dbg(sm, SM_XFER_DATAS_OVER, "Xfer datas over!");
	}
	else
	{
		ret = _set_sm_state_dbg(sm, SM_XFERRING_DATAS, "Xferring datas!");
	}

	return ret;
}

static int proc_err_bad_cmd(struct state_machine *sm)
{
	VSR_ERR("[ERR PROC]:Bad cmd!");
	return 0;
}

static int proc_err_xfer_failed(struct state_machine *sm)
{
	VSR_ERR("[ERR PROC]:Xfer data failed!");
	return 0;
}

static int proc_err_ack_timeout(struct state_machine *sm)
{
	VSR_ERR("[ERR PROC]:Ack timeout!");
	return 0;
}

static int xfer_data_stop(struct state_machine *sm)
{
	VSR_ERR("[ERR PROC]:xfer data stop!");
	return 0;
}

static const struct errcode_proc_func errcode_proc_func_map[] = {
	{SM_BAD_CMD, proc_err_bad_cmd},
	{SM_XFER_DATAS_FAILED, proc_err_xfer_failed},
	{SM_WAITING_ACK_TIMEOUT, proc_err_ack_timeout},
	{SM_XFER_DATAS_STOP, xfer_data_stop},
};

static proc_err_state get_err_proc_func(const u8 errcode)
{
	int i;
	int map_count = ARRAY_SIZE(errcode_proc_func_map);

	for (i = 0; i < map_count; i++)
	{
		if (errcode == errcode_proc_func_map[i].errcode)
		{
			return errcode_proc_func_map[i].proc_func;
		}
	}
	return NULL;
}

static int proc_err(struct state_machine *sm)
{
	proc_err_state proc_func = get_err_proc_func(get_sm_state(sm));
	if (!proc_func)
		return 0;
	
	proc_func(sm);
	return 1;
}

static inline void wake_up_read(void)
{
	get_vsr_dev()->xfer_over = XFER_DATA_OVER;
	wake_up_interruptible(&get_vsr_dev()->wait_xfer);
}

static inline void xfer_over_proc(struct state_machine *sm)
{
	if (1 == proc_err(sm))
	{
		wake_up_read();
	}
}

void proc_timer_out(struct state_machine *sm)
{
	if (SM_WAITING_FOR_ACK == get_sm_state(sm))
	{
		set_sm_state(sm, SM_WAITING_ACK_TIMEOUT);
		xfer_over_proc(sm);
	}
}

static void xfer_success_wake_up(struct state_machine *sm)
{
	if (SM_XFER_DATAS_SUCCESS == get_sm_state(sm))
	{
		wake_up_read();
	}
}

/*********************************************************************
 *process received cmd
 *********************************************************************/
static int proc_cmd_ack(struct state_machine *sm, const u8 *ack)
{
	int ret = 0;
	char ack_code = 0;
	CHECK_PRT_INT(sm);

	if (SM_WAITING_FOR_ACK != get_sm_state(sm))
	{
		VSR_DBG("Nothing to do!");
		return 0;
	}
	
#ifdef	TIMER_OUT
	if (SM_WAITING_ACK_TIMEOUT == get_sm_state(sm))
	{
		VSR_DBG("Nothing to do!");
		return 0;
	}
	
	del_timer_sync(&(get_vsr_dev()->timer));
	VSR_DBG("del_timer_sync!");
#endif

	ack_code = (char)get_ackcode((const CMD_READ_ACK *)ack);
	VSR_DBG("ack_code is: 0x%02x", ack_code);
	
	if (CMD_ACKCODE_BAD_CMD == ack_code)
	{
		ret = _set_sm_state_err(sm, SM_BAD_CMD, "Send bad cmd to dev!");
		ret = -ret;
	}
	else if (CMD_ACKCODE_EFFECT_CMD == ack_code)
	{
		ret = _set_sm_state_dbg(sm, SM_XFER_BEGIN, "Start to xfer datas!");
	}
	else
	{
		ret = _set_sm_state_err(sm, SM_BAD_CMD, "Recv bad ack cmd!");
		ret = -BAD_CMD;
	}

	return ret;
}

static int proc_cmd_state(struct state_machine *sm, const u8 *state)
{
	int ret = 0;
	char statecode = 0;
	CHECK_PRT_INT(sm);

	if ((SM_XFER_DATAS_OVER != get_sm_state(sm)) && (SM_XFERRING_DATAS != get_sm_state(sm)))
	{
		VSR_DBG("Nothing to do!");
		return 0;
	}
	
	statecode = (char)get_read_state((const CMD_READ_STATE *)state);

	if ((CMD_STATE_END == statecode) && (SM_XFER_DATAS_OVER == get_sm_state(sm)))
	{
		ret = _set_sm_state_dbg(sm, SM_XFER_DATAS_SUCCESS, "Xfer data success!");
	}
	else if ((CMD_STATE_END == statecode) && (SM_XFERRING_DATAS == get_sm_state(sm)))
	{
		ret = _set_sm_state_err(sm, SM_XFER_DATAS_FAILED, "Xfer data error stop!");
		ret = -ret;
	}
	else if (CMD_STATE_FAILED == statecode) 
	{
		ret = _set_sm_state_err(sm, SM_XFER_DATAS_FAILED, "Xfer data failed!");
		ret = -ret;
	}
	else if (CMD_STATE_TRAY_OUT == statecode) 
	{
		ret = _set_sm_state_wan(sm, SM_XFER_DATAS_STOP, "Tray out in xferring data!");
		ret = -ret;
	}
	else
	{
		ret = _set_sm_state_err(sm, SM_BAD_CMD, "Recv bad read state cmd!");
		ret = -BAD_CMD;
	}

	return ret;
}

static const struct optcode_proc_func optcode_proc_func_map[] = {
	{CMD_OPTCODE_READ_ACK, proc_cmd_ack},
	{CMD_OPTCODE_READ_STATE, proc_cmd_state},
};

static proc_cmd get_cmd_proc_func(const u8 optcode)
{
	int i;
	int map_count = ARRAY_SIZE(optcode_proc_func_map);

	for (i = 0; i < map_count; i++)
	{
		if (optcode == optcode_proc_func_map[i].optcode)
		{
			return optcode_proc_func_map[i].proc_func;
		}
	}
	return NULL;

}
/*********************************************************************
 *process received cmd end
 *********************************************************************/

#ifdef ADD_SYNC_CMD
/*********************************************************************
 *BENTEN send cmd in some state for sync
 *********************************************************************/
struct sm_send_cmd sm_send_cmd_map[] = {
		{SM_XFER_BEGIN, 		sizeof(CMD_START_XFER), (u8 *)make_cmd_start_xfer},
		{SM_XFER_DATAS_OVER, 	sizeof(CMD_READ_OVER),	(u8 *)make_cmd_read_over},
};

static struct sm_send_cmd *get_sync_cmd(u8 sm_state)
{
	int i;
	int map_count = ARRAY_SIZE(sm_send_cmd_map);

	for (i = 0; i < map_count; i++)
	{
		if (sm_state == sm_send_cmd_map[i].sm_state)
		{
			return &sm_send_cmd_map[i];
		}
	}
	return NULL;
}

static int send_sync_cmd(const struct state_machine *sm, u8 sync_flag)
{
	int ret = 0;
	u8 sm_state = 0;
	struct sm_send_cmd *cmd = NULL;

	sm_state = get_sm_state(sm);
	
	if ((SM_XFER_BEGIN != sm_state) && (SM_XFER_DATAS_OVER != sm_state))
		return 0;
	
	cmd = get_sync_cmd(sm_state);
	CHECK_PRT_INT(cmd);
	
	if (cmd->cmd_size != send_cmd(cmd->make_cmd_func(sync_flag), cmd->cmd_size))
	{
		VSR_ERR("Send sync cmd failed!");
		ret = _set_sm_state_err(sm, SM_XFER_DATAS_FAILED, "Xfer data error stop!");
		ret = -ret;
		goto err;
	}

err:
	return ret;
}
/*********************************************************************
 *BENTEN send cmd in some state for sync end
 *********************************************************************/
#endif

static u8 *get_cur_recv_buf(const struct state_machine *sm, const u8 *cmd, const u8 *base, 
														size_t total_size, size_t cur_recv_size)
{
	u8 *datas = NULL;
	
	if ((SM_WAITING_FOR_ACK == get_sm_state(sm)) || (SM_XFER_DATAS_OVER == get_sm_state(sm)))
	{
		datas = (u8 *)cmd;
		VSR_DBG("use cmd buf!");
	}
	else if (SM_XFER_BEGIN == get_sm_state(sm))
	{
		datas = (u8 *)base;
		VSR_DBG("use data buf!");
	}
	else if (SM_XFERRING_DATAS == get_sm_state(sm))
	{
		if (total_size > cur_recv_size)
		{
			datas = (u8 *)base + cur_recv_size;
			VSR_DBG("[SM_XFERRING_DATAS]:use data buf!");
		}
		else
		{
			datas = (u8 *)cmd;
			VSR_DBG("[SM_XFERRING_DATAS]:use cmd buf!");
		}
	}
	else
	{
		VSR_DBG("Non buf be used, sm state is: %d!", get_sm_state(sm));
	}
	return datas;
}


#ifdef	MODIFY_SECTOR_OFFSET
static inline size_t data_offset_in_block(u32 sector_addr, u32 sector_len)
{
	return SECTOR_COUNT_TO_SIZE(sector_addr % 4);
}

static void modify_data_offset(u8 *buf, unsigned long len, size_t offset)
{
	int i = 0;
	u8 *dst_addr = buf;
	u8 *effect_data = buf + offset;

	while(len--)
	{
		*dst_addr++ = *effect_data++;
	}
}

static inline size_t modify_cur_recv_size(size_t cur_recv_size, size_t offset)
{
	return (cur_recv_size - offset);
}

static size_t modify_sector_offset_patch(u8 *buf, size_t cur_recv_size)
{
	u32 sector_addr = get_read_sector_addr();
	u32 sector_len = get_read_sector_len();
	size_t data_offset = data_offset_in_block(sector_addr, sector_len);

	if (0 == data_offset)
	{
		return cur_recv_size;
	}

	modify_data_offset(buf, SECTOR_COUNT_TO_SIZE(sector_len), data_offset);
	return modify_cur_recv_size(cur_recv_size, data_offset);
}
#endif

static int cmd_state_patch(size_t read_buf_size, size_t recv_size, u8 *datas, const struct state_machine *sm)
{
	int ret = 0;
	u8 *cmd = NULL;
	int cmd_state_size = 0;
	proc_cmd proc_func = NULL;

	if (SM_XFER_DATAS_OVER != get_sm_state(sm))
	{
		VSR_DBG("Nothing to do!");
		return ret;
	}

	cmd_state_size = sizeof(CMD_READ_STATE);
	if ((recv_size != read_buf_size) || ((recv_size % 2048) != cmd_state_size))
	{
		VSR_DBG("recv_size = %d read_buf_size = %d!", recv_size, read_buf_size);
		return ret;
	}
	
	cmd = datas + read_buf_size - cmd_state_size;
	ret = is_cmd(cmd, cmd_state_size);
	if (1 == ret)
	{
		VSR_DBG("proc CMD datas!");
		proc_func = get_cmd_proc_func(cmd[1]);
		CHECK_PRT_INT(proc_func);
		ret = proc_func(sm, cmd);
	}
	else if (-1 == ret)
	{
		VSR_ERR("Bad recv data!");
	}
	
	return ret;
}

static u8 cmd_buf[XFER_INTERFACE_BUF_SIZE];
int proc_recv_data(void)
{
	int ret = 0;
	u8 *base = NULL;
	u8 *datas = NULL;
	size_t recv_size;
	static size_t cur_recv_size = 0;
	static size_t effect_cur_recv_size = 0;
	size_t space_size;
	size_t total_size;
	size_t effect_total_size;
	size_t read_buf_size;
	u8 *cmd = cmd_buf;
	proc_cmd proc_func = NULL;
	struct state_machine *sm = NULL;
	
	VSR_DBG("process recv data!");
	total_size = BLOCK_COUNT_TO_SIZE(get_read_block_len());
	effect_total_size = SECTOR_COUNT_TO_SIZE(get_read_sector_len());
	base = get_recv_buf();
	CHECK_PRT_INT(base);
	sm = &get_vsr_dev()->stmch;

	datas = get_cur_recv_buf(sm, cmd, base, total_size, effect_cur_recv_size);
	CHECK_PRT_INT(datas);
	space_size = total_size - cur_recv_size;
	read_buf_size = space_size + sizeof(CMD_READ_STATE);
	VSR_DBG("space_size = %u!", space_size);
	
	recv_size = read_datas(datas, read_buf_size);
	VSR_DBG("recv_size = %u!", recv_size);
	ret = is_cmd(datas, recv_size);
	
	if (1 == ret)
	{
		VSR_DBG("proc CMD datas!");
		proc_func = get_cmd_proc_func(datas[1]);
		CHECK_PRT_INT(proc_func);
		ret = proc_func(sm, datas);
	}
	else if (-1 == ret)
	{
		VSR_ERR("Bad recv data!");
	}
	else
	{
		cur_recv_size += recv_size;
#ifdef	MODIFY_SECTOR_OFFSET
		effect_cur_recv_size = modify_sector_offset_patch(datas, cur_recv_size);
#else
		effect_cur_recv_size = cur_recv_size;
#endif
		ret = xfer_datas(sm, total_size, cur_recv_size);
		
		if (SM_XFER_DATAS_OVER == ret)
		{
			VSR_DBG("effect_total_recv_size = %u!", effect_cur_recv_size);
			cur_recv_size = 0;
			effect_cur_recv_size = 0;
		}
		else if (0 == ret)
		{
			cur_recv_size -= recv_size;
			VSR_DBG("current state is: %d", get_sm_state(sm));
			VSR_WAN("Invalid recv data!");
		}
		else if (ret < 0)
		{
			VSR_ERR("Xfer datas error!");
		}
		else
		{
			VSR_DBG(".");
		}
	}

#ifdef ADD_SYNC_CMD
	send_sync_cmd(sm, CMD_SYNC_FLAG_OK);
#else
	cmd_state_patch(read_buf_size, recv_size, datas, sm);
#endif
	xfer_over_proc(sm);
	xfer_success_wake_up(sm);
	return ret;
}

#ifdef USE_KTHREAD
static int recv_flag = 0;
static wait_queue_head_t wait_recv_task;
static struct task_struct *vsr_recv_task = NULL;

wait_queue_head_t *get_recv_task_qh(void)
{
	return &wait_recv_task;
}

static void wake_up_recv_kthread(void)
{
	recv_flag = 1;
	wake_up_interruptible(&wait_recv_task);
}

static int kthread_func_proc_recv_data(void *data)
{
	while(1)
	{
		set_current_state(TASK_UNINTERRUPTIBLE);

		if(kthread_should_stop())
			break;

		if (wait_event_interruptible(wait_recv_task, recv_flag))
		{
			VSR_ERR("Waiting for recv data error!");
			return -1;
		}
		recv_flag = 0;
		proc_recv_data();
	}
	return 0;
}

int recv_kthread_init(void)
{
	int err;
	vsr_recv_task = kthread_create(kthread_func_proc_recv_data, NULL, "vsr_recv_task");

	if(IS_ERR(vsr_recv_task))
	{
		VSR_ERR("Unable to start kernel thread!");
		err = PTR_ERR(vsr_recv_task);
		vsr_recv_task = NULL;
		return err;
	}

	wake_up_process(vsr_recv_task);
	return 0;
}

int recv_kthread_stop(void)
{
	if (vsr_recv_task)
	{
		kthread_stop(vsr_recv_task);
		vsr_recv_task = NULL;
	}
}

#endif

void call_proc_recv_data(void)
{
	VSR_DBG("recv data!");
#ifdef USE_KTHREAD
	wake_up_recv_kthread();
#else
	//queue_work(get_vsr_dev()->vsr_work_queue, &get_vsr_dev()->recv_work);
	queue_delayed_work(get_vsr_dev()->vsr_work_queue, &get_vsr_dev()->recv_work, 0);
	//queue_work_on(WORK_CPU_UNBOUND, get_vsr_dev()->vsr_work_queue, &get_vsr_dev()->recv_work);
#endif
}

