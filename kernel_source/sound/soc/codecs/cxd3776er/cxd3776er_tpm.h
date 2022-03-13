/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_tpm.h
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

#ifndef _CXD3776ER_TPM_HEADER_
#define _CXD3776ER_TPM_HEADER_

int cxd3776er_tpm_register_module(struct cxd3776er_tpm_interface * interface);
int cxd3776er_tpm_initialize(void);
int cxd3776er_tpm_shutdown(void);
int cxd3776er_tpm_prepare(void);
int cxd3776er_tpm_cleanup(void);
int cxd3776er_tpm_judge(
	int tpm_mode,
	int output_path,
	int heaphone_amp,
	int tpm_status,
	int sdin_mix
);
int cxd3776er_tpm_off(void);
int cxd3776er_tpm_set_temperature(int index);
int cxd3776er_tpm_get_temperature(int * index);
int cxd3776er_tpm_set_base_gain(int left, int right);
int cxd3776er_tpm_get_base_gain(int * left, int * right);
int cxd3776er_tpm_exit_base_gain_adjustment(int save);

#endif
