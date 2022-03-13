/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_tpm.c
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

static struct cxd3776er_tpm_interface * tpm_interface = NULL;

int cxd3776er_tpm_register_module(struct cxd3776er_tpm_interface * interface)
{
	print_trace("%s()\n",__FUNCTION__);

	tpm_interface = interface;

	return(0);
}

int cxd3776er_tpm_initialize(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(tpm_interface!=NULL){
		rv=tpm_interface->initialize();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_tpm_shutdown(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(tpm_interface!=NULL){
		rv=tpm_interface->shutdown();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_tpm_prepare(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(tpm_interface!=NULL){
		rv=tpm_interface->prepare();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_tpm_cleanup(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(tpm_interface!=NULL){
		rv=tpm_interface->cleanup();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_tpm_judge(
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

	if(tpm_interface!=NULL){
		rv=tpm_interface->judge(
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

int cxd3776er_tpm_off(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(tpm_interface!=NULL){
		rv=tpm_interface->off();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_tpm_set_temperature(int index)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(tpm_interface!=NULL){
		rv=tpm_interface->set_temperature(index);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_tpm_get_temperature(int * index)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	*index=0;

	if(tpm_interface!=NULL){
		rv=tpm_interface->get_temperature(index);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_tpm_set_base_gain(int left, int right)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(tpm_interface!=NULL){
		rv=tpm_interface->set_base_gain(left,right);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_tpm_get_base_gain(int * left, int * right)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	*left=0;
	*right=0;

	if(tpm_interface!=NULL){
		rv=tpm_interface->get_base_gain(left,right);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int cxd3776er_tpm_exit_base_gain_adjustment(int save)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(tpm_interface!=NULL){
		rv=tpm_interface->exit_base_gain_adjustment(save);
		if(rv<0)
			return(rv);
	}

	return(0);
}

