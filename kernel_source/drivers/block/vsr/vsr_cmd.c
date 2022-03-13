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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/err.h>

#include "vsr_common.h"
#include "vsr_cmd.h"

static CMD_READ_DATA cmd_read_data;
#ifdef ADD_SYNC_CMD
static CMD_START_XFER cmd_start_xfer;
static CMD_READ_OVER cmd_read_over;
#endif
static const struct optcode_tail_offset ot_offset_map[] = {
	{CMD_OPTCODE_READ_ACK, 3},
	{CMD_OPTCODE_READ_STATE, 3},
};

static inline u8 check_sum(const u8 *base, int len)
{
	int ret = 0xff;
	
	if (!base)
		return 0;

	ret = base[--len];
	while (len)
	{
		ret |= base[--len];
	}
	return ret;
}

const CMD_READ_DATA *make_cmd_read_data(u32 block_base, u16 block_count)
{
	cmd_read_data.head 			= CMD_HEAD;
	cmd_read_data.optcode 		= CMD_OPTCODE_READ_DATA;
	cmd_read_data.addr[0] 		= (block_base >> 24) & 0xFF;
	cmd_read_data.addr[1] 		= (block_base >> 16) & 0xFF;
	cmd_read_data.addr[2] 		= (block_base >> 8) & 0xFF;
	cmd_read_data.addr[3] 		= (block_base >> 0) & 0xFF;
	cmd_read_data.length[0] 		= (block_count >> 8) & 0xFF;
	cmd_read_data.length[1] 		= (block_count >> 0) & 0xFF;
	cmd_read_data.chksum		= check_sum((u8 *)&cmd_read_data + 2, sizeof(cmd_read_data.addr)+ sizeof(cmd_read_data.length));
	cmd_read_data.tail 			= CMD_TAIL;
	return &cmd_read_data;
}

#ifdef ADD_SYNC_CMD
const CMD_START_XFER *make_cmd_start_xfer(u8 start_flag)
{
	cmd_start_xfer.head 		= CMD_HEAD;
	cmd_start_xfer.optcode 		= CMD_OPTCODE_START_XFER;
	cmd_start_xfer.flag			= start_flag;
	cmd_start_xfer.tail 		= CMD_TAIL;
	return &cmd_start_xfer;
}

const CMD_READ_OVER *make_cmd_read_over(u8 read_ret)
{
	cmd_read_over.head 		= CMD_HEAD;
	cmd_read_over.optcode 	= CMD_OPTCODE_READ_OVER;
	cmd_read_over.result	= read_ret;
	cmd_read_over.tail 		= CMD_TAIL;
	return &cmd_read_over;
}
#endif

static int check_optcode(const u8 optcode)
{
	int i;
	int map_count = sizeof(ot_offset_map)/sizeof(ot_offset_map[0]);
	
	for (i = 0; i < map_count; i++)
	{
		if (optcode == ot_offset_map[i].optcode)
		{
			return ot_offset_map[i].offset;
		}
	}
	return 0;
}

int is_cmd(const u8 *cmd, int len)
{	
	int ret = 1;
	int tail_offset = 0;
	
	CHECK_PRT_INT(cmd);

	if (len <= 0)
	{
		VSR_ERR("Bad cmd!");
		return -1;
	}

	if (CMD_HEAD != cmd[0])
	{
		VSR_DBG("Not cmd!");
		ret = 0;
	}

	tail_offset = check_optcode(cmd[1]);
	if (0 == tail_offset)
	{
		VSR_DBG("Not cmd!");
		ret = 0;
	}

	if (CMD_TAIL != cmd[tail_offset])
	{
		ret = 0;
	}

	return ret;
}

const char get_ackcode(const CMD_READ_ACK *ack)
{
	CHECK_PRT_CHAR(ack);

	if ((CMD_HEAD != ack->head) || (CMD_TAIL != ack->tail))
	{
		VSR_ERR("Bad cmd!");
		return -BAD_CMD;
	}

	if (CMD_OPTCODE_READ_ACK != ack->optcode)
	{
		VSR_LOG("Not ack cmd!");
		return -ERR_OPTCODE;
	}
	
	return (const char)ack->ackcode;
}

const char get_read_state(const CMD_READ_STATE *state)
{
	CHECK_PRT_CHAR(state);

	if ((CMD_HEAD != state->head) || (CMD_TAIL != state->tail))
	{
		VSR_ERR("Bad cmd!");
		return -BAD_CMD;
	}

	if (CMD_OPTCODE_READ_STATE != state->optcode)
	{
		VSR_LOG("Not read state cmd!");
		return -ERR_OPTCODE;
	}
	
	return (const char)state->state;
}
