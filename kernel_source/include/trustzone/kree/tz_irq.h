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
/*
 * IRQ/FIQ for TrustZone
 */

#ifndef __KREE_TZ_IRQ_H__
#define __KREE_TZ_IRQ_H__

#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT

void kree_irq_init(void);
int kree_set_fiq(int irq, unsigned long irq_flags);
void kree_enable_fiq(int irq);
void kree_disable_fiq(int irq);
void kree_query_fiq(int irq, int *enable, int *pending);
unsigned int kree_fiq_get_intack(void);
void kree_fiq_eoi(unsigned int iar);
int kree_raise_softfiq(unsigned int mask, unsigned int irq);
void kree_irq_mask_all(unsigned int *pmask, unsigned int size);
void kree_irq_mask_restore(unsigned int *pmask, unsigned int size);

#else

#define kree_set_fiq(irq, irq_flags)     -1
#define kree_enable_fiq(irq)
#define kree_disable_fiq(irq)

#endif				/* CONFIG_MTK_IN_HOUSE_TEE_SUPPORT */

#endif				/* __KREE_TZ_IRQ_H__ */
