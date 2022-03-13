/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf_interrupt.c
 *
 * CXD3774GF CODEC driver
 *
 * Copyright (c) 2013 Sony Corporation
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

#include "cxd3774gf_common.h"

static irqreturn_t cxd3774gf_interrupt(int irq, void * data);

static int initialized = FALSE;

int cxd3774gf_interrupt_initialize(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);
#if 0
#ifdef USE_PCM_MONITORING_MUTE

	irq_set_irq_type(
		cxd3774gf_get_xpcm_det_irq(),
		IRQ_TYPE_EDGE_BOTH
	);

	rv=request_irq(
		cxd3774gf_get_xpcm_det_irq(),
		cxd3774gf_interrupt,
		0,
		"cxd3774gf_xpcm_det",
		NULL
	);
	if(rv<0){
		print_fail("request_irq(): code %d error occured.\n",rv);
		back_trace();
		return(rv);
	}

#endif
#endif
	initialized=TRUE;

	return(0);
}

int cxd3774gf_interrupt_finalize(void)
{
	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

#ifdef USE_PCM_MONITORING_MUTE

//	free_irq(cxd3774gf_get_xpcm_det_irq(),NULL);

#endif

	initialized=FALSE;

	return(0);
}

static irqreturn_t cxd3774gf_interrupt(int irq, void * data)
{
#ifdef TRACE_PRINT_ON
	printk(KERN_INFO "####### cxd3774gf INTERRUPT\n");
#endif

	cxd3774gf_handle_pcm_event();

	return(IRQ_HANDLED);
}
