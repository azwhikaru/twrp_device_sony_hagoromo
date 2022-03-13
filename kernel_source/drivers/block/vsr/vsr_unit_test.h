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
#ifndef	__VSR_UNIT_TEST_H__
#define	__VSR_UNIT_TEST_H__

struct vsr_frame {
	char *datas;
	int len;
};

int read_data_test(struct vsr_dev *dev);
void start_recv(void);
void stop_recv(void);
struct timer_list *get_test_timer(void);

#endif	/*__VSR_UNIT_TEST_H__*/
