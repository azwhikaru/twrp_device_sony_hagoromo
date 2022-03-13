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
#ifndef __BACKPORT_LINUX_IRQ_H
#define __BACKPORT_LINUX_IRQ_H
#include_next <linux/irq.h>

#ifdef CONFIG_HAVE_GENERIC_HARDIRQS
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0)
#define irq_get_trigger_type LINUX_BACKPORT(irq_get_trigger_type)
static inline u32 irq_get_trigger_type(unsigned int irq)
{
	struct irq_data *d = irq_get_irq_data(irq);
	return d ? irqd_get_trigger_type(d) : 0;
}
#endif
#endif /* CONFIG_HAVE_GENERIC_HARDIRQS */

#endif /* __BACKPORT_LINUX_IRQ_H */
