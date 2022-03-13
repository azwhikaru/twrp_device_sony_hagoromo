/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3778gf_interrupt.c
 *
 * CXD3778GF CODEC driver
 *
 * Copyright (c) 2013-2016 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* #define TRACE_PRINT_ON */
/* #define DEBUG_PRINT_ON */
#define TRACE_TAG "------- "
#define DEBUG_TAG "        "

#include "cxd3778gf_common.h"

#define HP_DET_EINT 35 /* GPIO 53 */

static irqreturn_t cxd3778gf_interrupt(int irq, void * data);

static int initialized = FALSE;

static void hp_det_irq_handler(void);

int cxd3778gf_interrupt_initialize(unsigned int type, int headphone_detect_mode)
{
	int rv;
	int hp_detect_irq;

	print_trace("%s()\n",__FUNCTION__);

#if 0
#ifdef USE_PCM_MONITORING_MUTE

	irq_set_irq_type(
		cxd3778gf_get_xpcm_det_irq(),
		IRQ_TYPE_EDGE_BOTH
	);

	rv=request_irq(
		cxd3778gf_get_xpcm_det_irq(),
		cxd3778gf_interrupt,
		0,
		"cxd3778gf_xpcm_det",
		NULL
	);
	if(rv<0){
		print_fail("request_irq(): code %d error occured.\n",rv);
		back_trace();
		return(rv);
	}

#endif
#endif
	if (initialized==FALSE){
		if (type == TYPE_A && headphone_detect_mode == HEADPHONE_DETECT_INTERRUPT) {
			hp_detect_irq = cxd3778gf_get_hp_det_irq();
			//mt_eint_set_hw_debounce(hp_detect_irq, 32);
			if (hp_detect_irq == 0)
				mt_eint_registration(hp_detect_irq, EINTF_TRIGGER_LOW, hp_det_irq_handler, 1);
			else
				mt_eint_registration(hp_detect_irq, EINTF_TRIGGER_HIGH, hp_det_irq_handler, 1);

			initialized=TRUE;
		}
	}

	return(0);
}

int cxd3778gf_interrupt_finalize(unsigned int type, int headphone_detect_mode)
{
	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

#ifdef USE_PCM_MONITORING_MUTE

//	free_irq(cxd3778gf_get_xpcm_det_irq(),NULL);

#endif
	if (type == TYPE_A && headphone_detect_mode == HEADPHONE_DETECT_INTERRUPT) {
		mt_eint_registration(cxd3778gf_get_hp_det_irq(), EINTF_TRIGGER_HIGH, NULL, 0);
	}
	initialized=FALSE;

	return(0);
}

static irqreturn_t cxd3778gf_interrupt(int irq, void * data)
{
#ifdef TRACE_PRINT_ON
	printk(KERN_INFO "####### cxd3778gf INTERRUPT\n");
#endif

	cxd3778gf_handle_pcm_event();

	return(IRQ_HANDLED);
}

static void hp_det_irq_handler(void)
{
	static DEFINE_SPINLOCK(lock);
	unsigned long flags;

#ifdef TRACE_PRINT_ON
        printk(KERN_INFO "####### cxd3778gf INTERRUPT HPDET\n");
#endif

	spin_lock_irqsave(&lock, flags);
	if (cxd3778gf_get_hp_det_se_value() == 0) {
		mt_eint_set_polarity(cxd3778gf_get_hp_det_irq(), MT_POLARITY_LOW);
	} else {
		mt_eint_set_polarity(cxd3778gf_get_hp_det_irq(), MT_POLARITY_HIGH);
	}

	cxd3778gf_handle_hp_det_event();
	spin_unlock_irqrestore(&lock, flags);

	return;
}
