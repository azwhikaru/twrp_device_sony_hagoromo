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
#ifndef __BACKPORT_SEQ_FILE_H
#define __BACKPORT_SEQ_FILE_H
#include_next <linux/seq_file.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
#include <linux/user_namespace.h>
#include <linux/file.h>
#include <linux/fs.h>
#ifdef CONFIG_USER_NS
static inline struct user_namespace *seq_user_ns(struct seq_file *seq)
{
	struct file *f = container_of((void *) seq, struct file, private_data);

	return f->f_cred->user_ns;
}
#else
static inline struct user_namespace *seq_user_ns(struct seq_file *seq)
{
	extern struct user_namespace init_user_ns;
	return &init_user_ns;
}
#endif /* CONFIG_USER_NS */
#endif /* < 3.7 */

#endif /* __BACKPORT_SEQ_FILE_H */
