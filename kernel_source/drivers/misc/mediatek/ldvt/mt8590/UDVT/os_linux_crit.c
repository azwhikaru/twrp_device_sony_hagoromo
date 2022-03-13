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
#include <linux/irqflags.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/thread_info.h>
#include "x_assert.h"
#include "x_os.h"


#define INVALID_OWNER ((struct thread_info *)(NULL))

static struct thread_info *s_crit_owner = INVALID_OWNER;
static DEFINE_SPINLOCK(s_crit_lock);
static unsigned long s_crit_count;


CRIT_STATE_T
x_os_drv_crit_start(VOID)
{
    unsigned long flags;

    if (s_crit_owner != current_thread_info())
    {
        spin_lock_irqsave(&s_crit_lock, flags);
        s_crit_owner = current_thread_info();
        s_crit_count = 1;
        return (CRIT_STATE_T)(flags);
    }
    s_crit_count++;
    return (CRIT_STATE_T)(s_crit_count);
}


VOID
x_os_drv_crit_end(CRIT_STATE_T t_old_level)
{
    unsigned long flags = (unsigned long)(t_old_level);

    ASSERT(s_crit_owner == current_thread_info());
    s_crit_count--;
    if (s_crit_count != 0)
    {
        ASSERT(flags == s_crit_count + 1);
        return;
    }
    s_crit_owner = INVALID_OWNER;
    spin_unlock_irqrestore(&s_crit_lock, flags);
}


CRIT_STATE_T (* x_crit_start)(VOID) = x_os_drv_crit_start;
VOID (* x_crit_end)(CRIT_STATE_T  t_old_level) = x_os_drv_crit_end;


EXPORT_SYMBOL(x_crit_start);
EXPORT_SYMBOL(x_crit_end);


