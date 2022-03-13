/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3778gf_switch.c
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
#define CONFIG_SWITCH_CXD3778GF
static int initialized = FALSE;

#ifdef CONFIG_SWITCH_CXD3778GF

static struct switch_dev headphone_switch_dev = {
	.name = "cxd3778gf_h2w",
};

static struct switch_dev antenna_switch_dev = {
	.name = "cxd3778gf_antenna",
};

#endif

int cxd3778gf_switch_initialize(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

#ifdef CONFIG_SWITCH_CXD3778GF

	rv=switch_dev_register(&headphone_switch_dev);
	if(rv<0){
		print_fail("switch_dev_register(headphone): code %d error occurred.\n",rv);
		back_trace();
		return(rv);
	}

	rv=switch_dev_register(&antenna_switch_dev);
	if(rv<0){
		print_fail("switch_dev_register(antenna): code %d error occurred.\n",rv);
		back_trace();
		switch_dev_unregister(&headphone_switch_dev);
		return(rv);
	}

#endif

	initialized=TRUE;

	return(0);
}

int cxd3778gf_switch_finalize(void)
{
	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

#ifdef CONFIG_SWITCH_CXD3778GF

	switch_dev_unregister(&antenna_switch_dev);
	switch_dev_unregister(&headphone_switch_dev);

#endif

	initialized=FALSE;

	return(0);
}

int cxd3778gf_switch_set_headphone_value(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	switch_set_state(&headphone_switch_dev,value);

	return(0);
}

int cxd3778gf_switch_set_antenna_value(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	switch_set_state(&antenna_switch_dev,value);

	return(0);
}

