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
#include <linux/timer.h>

#include "vsr_common.h"
#include "vsr_data_xfer.h"
#include "vsr_cmd.h"
#include "vsr_unit_test.h"

#define LOAD_DATA_SIZE		512
#define RECV_BUF_SIZE		(1 << 10 << 10)
#define TEST_TIMER_DELAY	(HZ/1000)

static char test_recv_buf[RECV_BUF_SIZE] = {0};
static const u32 test_sector = 4 << 10;
static const u32 test_nsect = 4;
static struct timer_list test_timer;

struct timer_list *get_test_timer(void)
{
	return &test_timer;
}


static const CMD_READ_ACK cmd_ack_effect = {
	.head = CMD_HEAD,
	.optcode = CMD_OPTCODE_READ_ACK,
	.ackcode = CMD_ACKCODE_EFFECT_CMD,
	.tail = CMD_TAIL,
};

static const CMD_READ_ACK cmd_ack_bad = {
	.head = CMD_HEAD,
	.optcode = CMD_OPTCODE_READ_ACK,
	.ackcode = CMD_ACKCODE_BAD_CMD,
	.tail = CMD_TAIL,
};


static const CMD_READ_STATE cmd_state_idle = {
	.head = CMD_HEAD,
	.optcode = CMD_OPTCODE_READ_STATE,
	.state = CMD_STATE_IDLE,
	.tail = CMD_TAIL,
};

static const CMD_READ_STATE cmd_state_end = {
	.head = CMD_HEAD,
	.optcode = CMD_OPTCODE_READ_STATE,
	.state = CMD_STATE_END,
	.tail = CMD_TAIL,
};

static const CMD_READ_STATE cmd_state_failed = {
	.head = CMD_HEAD,
	.optcode = CMD_OPTCODE_READ_STATE,
	.state = CMD_STATE_FAILED,
	.tail = CMD_TAIL,
};

static const CMD_READ_STATE cmd_state_tray_out = {
	.head = CMD_HEAD,
	.optcode = CMD_OPTCODE_READ_STATE,
	.state = CMD_STATE_TRAY_OUT,
	.tail = CMD_TAIL,
};

static const char load_datas0[LOAD_DATA_SIZE] = {[0 ... LOAD_DATA_SIZE-1] = 1};
static const char load_datas1[LOAD_DATA_SIZE] = {[0 ... LOAD_DATA_SIZE-1] = 2};
static const char load_datas2[LOAD_DATA_SIZE] = {[0 ... LOAD_DATA_SIZE-1] = 3};
static const char load_datas3[LOAD_DATA_SIZE] = {[0 ... LOAD_DATA_SIZE-1] = 4};


static const struct vsr_frame vack_effect = {.datas = (char *)&cmd_ack_effect, .len = sizeof(CMD_READ_ACK),};
static const struct vsr_frame vack_bad = {.datas = (char *)&cmd_ack_bad, .len = sizeof(CMD_READ_ACK),};

static const struct vsr_frame vstate_idle = {.datas = (char *)&cmd_state_idle, .len = sizeof(CMD_READ_STATE),};
static const struct vsr_frame vstate_end = {.datas = (char *)&cmd_state_end, .len = sizeof(CMD_READ_STATE),};
static const struct vsr_frame vstate_failed = {.datas = (char *)&cmd_state_failed, .len = sizeof(CMD_READ_STATE),};
static const struct vsr_frame vstate_tray_out = {.datas = (char *)&cmd_state_tray_out, .len = sizeof(CMD_READ_STATE),};


static const struct vsr_frame vdatas0 = {.datas = (char *)load_datas0, .len = sizeof(load_datas0)};
static const struct vsr_frame vdatas1 = {.datas = (char *)load_datas1, .len = sizeof(load_datas1)};
static const struct vsr_frame vdatas2 = {.datas = (char *)load_datas2, .len = sizeof(load_datas2)};
static const struct vsr_frame vdatas3 = {.datas = (char *)load_datas3, .len = sizeof(load_datas3)};

static const struct vsr_frame *frame_list_normal_end[] = {&vack_effect, &vdatas0, &vdatas1, &vdatas2, &vdatas3, &vstate_end};
static const struct vsr_frame *frame_list_state_failed[] = {&vack_effect, &vdatas0, &vdatas1, &vdatas2, &vstate_failed};
static const struct vsr_frame *frame_list_state_tray_out[] = {&vack_effect, &vdatas0, &vdatas1, &vdatas2, &vstate_tray_out};
static const struct vsr_frame *frame_list_bad_ack[] = {&vack_bad, &vdatas0};
static const struct vsr_frame *frame_list_no_ack[] = {NULL};
static const struct vsr_frame *frame_list_no_ack1[] = {&vdatas0, &vdatas1, &vstate_end};
static const struct vsr_frame *frame_list_err0[] = {&vdatas0, &vdatas1, &vdatas2, &vdatas3, &vstate_end, 
													&vack_effect, &vdatas0, &vdatas1, &vdatas2, &vdatas3, &vstate_end};
static const struct vsr_frame *frame_list_err1[] = {&vack_effect, &vdatas0, &vdatas1, &vdatas2, &vstate_end};

#define frame_list_xxx frame_list_err1

int read_data_test(struct vsr_dev *dev)
{
	return read_from_dev(dev, test_recv_buf, test_sector, test_nsect);
}

static void test_timer_init(struct timer_list *timer, void (*func)(u32), u32 data, u32 delay)
{
	init_timer(timer);
	timer->function = func;
	timer->data = data;
	timer->expires  = jiffies + delay;
	add_timer(timer);
}

int usb_write_datas(const u8 *cmd, int len)
{
	int i;
	DEBUG("Write cmd is:");

	for (i = 0; i < len; i++)
	{
		DEBUG("0x%02x ", cmd[i]);
	}
	DEBUG("\n");
	return 0;
}

struct vsr_frame *get_frame(const struct vsr_frame *frame_list[], int size)
{
	static int frame_index = 0;
	struct vsr_frame *pframe = NULL;

	if (0 >= size)
	{
		return NULL;
	}
	
	VSR_DBG("Current frame is: %d/%d.", (frame_index + 1), size);
	if (frame_index < size)
	{
		pframe = (struct vsr_frame *)frame_list[frame_index];
		frame_index++;
		return pframe;
	}

	frame_index = 0;
	return NULL;
}

int usb_read_datas(u8 *buf, int len)
{
	struct vsr_frame *pframe = NULL;
	int list_size = sizeof(frame_list_xxx)/sizeof(frame_list_xxx[0]);
	
	pframe = get_frame(frame_list_xxx, list_size);

	if (NULL == pframe)
	{
		VSR_DBG("No frame!");
		return 0;
	}
	
	memcpy((char *)buf, pframe->datas, pframe->len);
	VSR_DBG("Recieve effect datas!");
	return pframe->len;
}

int proc_recv_data_test(void)
{
	int ret;
	ret = proc_recv_data();
	test_timer.expires  = jiffies + TEST_TIMER_DELAY;
	add_timer(&test_timer);
	return ret;
}

void start_recv(void)
{
	test_timer_init(&test_timer, proc_recv_data_test, 0, TEST_TIMER_DELAY);
	VSR_DBG("Start recv timer!");
}

void stop_recv(void)
{
	del_timer_sync(&test_timer);
	VSR_DBG("Stop recv timer!");
}
