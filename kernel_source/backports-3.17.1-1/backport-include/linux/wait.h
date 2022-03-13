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
#ifndef __BACKPORT_LINUX_WAIT_H
#define __BACKPORT_LINUX_WAIT_H
#include_next <linux/wait.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0)
extern int bit_wait(void *);
extern int bit_wait_io(void *);

static inline int
backport_wait_on_bit(void *word, int bit, unsigned mode)
{
	return wait_on_bit(word, bit, bit_wait, mode);
}

static inline int
backport_wait_on_bit_io(void *word, int bit, unsigned mode)
{
	return wait_on_bit(word, bit, bit_wait_io, mode);
}

#define wait_on_bit LINUX_BACKPORT(wait_on_bit)
#define wait_on_bit_io LINUX_BACKPORT(wait_on_bit_io)

#endif

#endif /* __BACKPORT_LINUX_WAIT_H */