/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_dmfb.c
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

static struct cxd3776er_dmfb_interface * dmfb_interface = NULL;

int cxd3776er_dmfb_register_module(struct cxd3776er_dmfb_interface * interface)
{
	print_trace("%s()\n",__FUNCTION__);

	dmfb_interface = interface;

	return(0);
}

int cxd3776er_dmfb_initialize(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->initialize();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_dmfb_shutdown(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->shutdown();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_dmfb_prepare(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->prepare();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_dmfb_cleanup(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->cleanup();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_dmfb_judge(
	int noise_cancel_mode,
	int output_path,
	int output_select,
	int tpm_status,
	int sdin_mix
)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	rv=0;

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->judge(
			noise_cancel_mode,
			output_path,
			output_select,
			tpm_status,
			sdin_mix
		);
		if(rv<0)
			return(rv);
	}

//	print_debug("noise cancel: mode=%d, status=%d\n",mode,rv);

	return(rv);
}

int cxd3776er_dmfb_off(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->off();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_dmfb_set_user_gain(int index)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->set_user_gain(index);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_dmfb_get_user_gain(int * index)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	*index=0;

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->get_user_gain(index);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_dmfb_set_base_gain(int left, int right)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->set_base_gain(left,right);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_dmfb_get_base_gain(int * left, int * right)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	*left=0;
	*right=0;

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->get_base_gain(left,right);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_dmfb_exit_base_gain_adjustment(int save)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(dmfb_interface!=NULL){
		rv=dmfb_interface->exit_base_gain_adjustment(save);
		if(rv<0)
			return(rv);
	}

	return(0);
}

