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
#ifndef __MTK_GPU_UTILITY_H__
#define __MTK_GPU_UTILITY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* returning false indicated no implement */

// unit: x bytes
bool mtk_get_gpu_memory_usage(unsigned int* pMemUsage);
bool mtk_get_gpu_page_cache(unsigned int* pPageCache);


/* unit: 0~100 % */
	bool mtk_get_gpu_loading(unsigned int *pLoading);
	bool mtk_get_gpu_GP_loading(unsigned int *pLoading);
	bool mtk_get_gpu_PP_loading(unsigned int *pLoading);
	bool mtk_get_gpu_power_loading(unsigned int *pLoading);

#ifdef __cplusplus
}
#endif
#endif
