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
#ifndef ION_DEBUGGER_KERNEL_DEF
#define ION_DEBUGGER_KERNEL_DEF
#define BACKTRACE_SIZE 10
#include <linux/mutex.h>
struct ion_buffer_record {
	struct ion_buffer_record *next;
	void *buffer_address;
	struct ion_buffer *buffer;
	unsigned int heap_type;
	unsigned int size;
	union {
		void *priv_virt;
		unsigned long priv_phys;
	};
	struct ion_buffer_usage_record *buffer_using_list;
	struct ion_buffer_usage_record *buffer_freed_list;
	struct mutex ion_buffer_usage_mutex;
	struct ion_address_usage_record *address_using_list;
	struct ion_address_usage_record *address_freed_list;
	struct mutex ion_address_usage_mutex;
};
struct ion_process_record {
	struct ion_process_record *next;
	pid_t pid;
	pid_t group_id;
	unsigned int count;
	struct ion_address_usage_record *address_using_list;
	struct ion_address_usage_record *address_freed_list;
	struct mutex ion_address_usage_mutex;
	struct ion_fd_usage_record *fd_using_list;
	struct ion_fd_usage_record *fd_freed_list;
	struct mutex ion_fd_usage_mutex;
};
extern void *ion_get_list(unsigned int record_type, void *record, unsigned int list_type);
extern struct ion_buffer_record *ion_get_inuse_buffer_record(void);
extern struct ion_buffer_record *ion_get_freed_buffer_record(void);
/* extern struct ion_prcoess_record *ion_get_inuse_process_usage_record(void); */
extern struct ion_process_record *ion_get_freed_process_record(void);
extern struct ion_client_usage_record *ion_get_inuse_client_record(void);
extern struct ion_client_usage_record *ion_get_freed_client_record(void);
extern unsigned int ion_get_data_from_record(void *record, unsigned int data_type);
#endif
