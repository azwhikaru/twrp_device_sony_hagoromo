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
/**
 * @file
 *   val_api_public.h
 *
 * @par Project:
 *   Video
 *
 * @par Description:
 *   Video Abstraction Layer API for internal use
 *
 * @par Author:
 *   Jackal Chen (mtk02532)
 *
 * @par $Revision: #1 $
 * @par $Modtime:$
 * @par $Log:$
 *
 */

#ifndef _VAL_API_PRIVATE_H_
#define _VAL_API_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "val_types_private.h"
#include "val_api_public.h"
#ifdef CONFIG_MTK_VIDEO_HEVC_SUPPORT
#include <linux/libmtk_cipher_export.h>
#endif

	VAL_RESULT_T eValInit(VAL_HANDLE_T *a_phHalHandle);
	VAL_RESULT_T eValDeInit(VAL_HANDLE_T *a_phHalHandle);

	VAL_RESULT_T eVideoIntMemAlloc(VAL_INTMEM_T *a_prParam, VAL_UINT32_T a_u4ParamSize);
	VAL_RESULT_T eVideoIntMemFree(VAL_INTMEM_T *a_prParam, VAL_UINT32_T a_u4ParamSize);

	VAL_RESULT_T eVideoCreateEvent(VAL_EVENT_T *a_prParam, VAL_UINT32_T a_u4ParamSize);
	VAL_RESULT_T eVideoSetEvent(VAL_EVENT_T *a_prParam, VAL_UINT32_T a_u4ParamSize);
	VAL_RESULT_T eVideoCloseEvent(VAL_EVENT_T *a_prParam, VAL_UINT32_T a_u4ParamSize);
	VAL_RESULT_T eVideoWaitEvent(VAL_EVENT_T *a_prParam, VAL_UINT32_T a_u4ParamSize);

	VAL_RESULT_T eVideoCreateMutex(VAL_MUTEX_T *a_prParam, VAL_UINT32_T a_u4ParamSize);
	VAL_RESULT_T eVideoCloseMutex(VAL_MUTEX_T *a_prParam, VAL_UINT32_T a_u4ParamSize);
	VAL_RESULT_T eVideoWaitMutex(VAL_MUTEX_T *a_prParam, VAL_UINT32_T a_u4ParamSize);
	VAL_RESULT_T eVideoReleaseMutex(VAL_MUTEX_T *a_prParam, VAL_UINT32_T a_u4ParamSize);

	VAL_RESULT_T eVideoMMAP(VAL_MMAP_T *a_prParam, VAL_UINT32_T a_u4ParamSize);
	VAL_RESULT_T eVideoUnMMAP(VAL_MMAP_T *a_prParam, VAL_UINT32_T a_u4ParamSize);

	VAL_RESULT_T eVideoInitLockHW(VAL_VCODEC_OAL_HW_REGISTER_T *prParam, int size);
	VAL_RESULT_T eVideoDeInitLockHW(VAL_VCODEC_OAL_HW_REGISTER_T *prParam, int size);

	VAL_RESULT_T eVideoVCodecCoreLoading(int CPUid, int *Loading);
	VAL_RESULT_T eVideoVCodecCoreNumber(int *CPUNums);

	VAL_RESULT_T eVideoConfigMCIPort(VAL_UINT32_T u4PortConfig, VAL_UINT32_T *pu4PortResult,
					 VAL_MEM_CODEC_T eMemCodec);

	VAL_UINT32_T eVideoHwM4UEnable(VAL_BOOL_T bEnable);	/* MTK_SEC_VIDEO_PATH_SUPPORT */
#ifdef CONFIG_MTK_VIDEO_HEVC_SUPPORT
	VAL_UINT32_T eVideoLibDecrypt(VIDEO_ENCRYPT_CODEC_T a_eVIDEO_ENCRYPT_CODEC);
#endif

#ifdef __cplusplus
}
#endif
#endif				/* #ifndef _VAL_API_PRIVATE_H_ */
