/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_timer.c
 *
 * CXD3776ER CODEC driver
 *
 * Copyright (c) 2015 Sony Corporation
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

#include "cxd3776er_common.h"

static int timer_thread_main(void * dummy);

static int initialized = FALSE;
static struct task_struct * timer_thread = NULL;

int cxd3776er_timer_initialize(void)
{
	print_trace("%s()\n",__FUNCTION__);

	timer_thread=kthread_create(timer_thread_main,NULL,"cxd3776er_thread");
	if(IS_ERR(timer_thread)){
		print_fail("kthread_create(): code %ld error occurred.\n",PTR_ERR(timer_thread));
		back_trace();
		timer_thread=NULL;
		return(PTR_ERR(timer_thread));
	}

	wake_up_process(timer_thread);

	initialized=TRUE;

	return(0);
}

int cxd3776er_timer_finalize(void)
{
	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

	if(timer_thread!=NULL){
		kthread_stop(timer_thread);
		timer_thread=NULL;
	}

	initialized=FALSE;

	return(0);
}

static int timer_thread_main(void * dummy)
{
	int force=TRUE;

	print_trace("%s()\n",__FUNCTION__);

	while(1){

		if(kthread_should_stop())
			break;

#ifdef TRACE_PRINT_ON
		printk(KERN_INFO "####### CXD3776ER TIMER\n");
#endif
		cxd3776er_check_tpm_status(force);

		force=FALSE;

		msleep(2000);
	}

	return(0);
}

