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
#ifndef __MMPROFILE_H__
#define __MMPROFILE_H__
#include "mmprofile_static_event.h"

#define DEFAULT_MMP_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

#define MMProfileEventNameMaxLen 31

	typedef unsigned int MMP_Event;

	typedef enum {
		MMProfileFlagStart = 1,
		MMProfileFlagEnd = 2,
		MMProfileFlagPulse = 4,
		MMProfileFlagEventSeparator = 8,
		MMProfileFlagMax = 0xFFFFFFFF
	} MMP_LogType;

	typedef enum {
		MMProfileMetaStringMBS = 1,
		MMProfileMetaStringWCS,
		MMProfileMetaStructure,
		MMProfileMetaBitmap,
		MMProfileMetaRaw,
		MMProfileMetaUser = 0x10000000,
		MMProfileMetaUserM4UReg,
		MMProfileMetaMax = 0xFFFFFFFF
	} MMP_MetaDataType;

	typedef enum {
		MMProfileBitmapRGB565 = 1,
		MMProfileBitmapRGB888,
		MMProfileBitmapRGBA8888,
		MMProfileBitmapBGR888,
		MMProfileBitmapBGRA8888,
		MMProfileBitmapMax = 0xFFFFFFFF
	} MMP_PixelFormat;

	typedef struct {
		unsigned int data1;	/* data1 (user defined) */
		unsigned int data2;	/* data2 (user defined) */
		MMP_MetaDataType data_type;	/* meta data type */
		unsigned int size;	/* meta data size */
		void *pData;	/* meta data pointer */
	} MMP_MetaData_t;

	typedef struct {
		unsigned int data1;	/* data1 (user defined) */
		unsigned int data2;	/* data2 (user defined) */
		unsigned int struct_size;	/* structure size (bytes) */
		void *pData;	/* structure pointer */
		char struct_name[32];	/* structure name */
	} MMP_MetaDataStructure_t;

	typedef struct {
		unsigned int data1;	/* data1 (user defined) */
		unsigned int data2;	/* data2 (user defined) */
		unsigned int width;	/* image width */
		unsigned int height;	/* image height */
		MMP_PixelFormat format;	/* image pixel format */
		unsigned int start_pos;	/* start offset of image data (base on pData) */
		unsigned int bpp;	/* bits per pixel */
		int pitch;	/* image pitch (bytes per line) */
		unsigned int data_size;	/* image data size (bytes) */
		unsigned int down_sample_x;	/* horizontal down sample rate (>=1) */
		unsigned int down_sample_y;	/* vertical down sample rate (>=1) */
		void *pData;	/* image buffer address */
	} MMP_MetaDataBitmap_t;

	MMP_Event MMProfileRegisterEvent(MMP_Event parent, const char *name);
	MMP_Event MMProfileFindEvent(MMP_Event parent, const char *name);
	void MMProfileEnableEvent(MMP_Event event, int enable);
	void MMProfileEnableEventRecursive(MMP_Event event, int enable);
	int MMProfileQueryEnable(MMP_Event event);
	void MMProfileLog(MMP_Event event, MMP_LogType type);
	void MMProfileLogEx(MMP_Event event, MMP_LogType type, unsigned int data1,
			    unsigned int data2);
	int MMProfileLogMeta(MMP_Event event, MMP_LogType type, MMP_MetaData_t *pMetaData);
	int MMProfileLogMetaString(MMP_Event event, MMP_LogType type, const char *str);
	int MMProfileLogMetaStringEx(MMP_Event event, MMP_LogType type, unsigned int data1,
				     unsigned int data2, const char *str);
	int MMProfileLogMetaStructure(MMP_Event event, MMP_LogType type,
				      MMP_MetaDataStructure_t *pMetaData);
	int MMProfileLogMetaBitmap(MMP_Event event, MMP_LogType type,
				   MMP_MetaDataBitmap_t *pMetaData);

#define MMProfileLogStructure(event, type, pStruct, struct_type) \
{ \
    MMP_MetaDataStructure_t MetaData; \
    MetaData.data1 = 0; \
    MetaData.data2 = 0; \
    strcpy(MetaData.struct_name, #struct_type); \
    MetaData.struct_size = sizeof(struct_type); \
    MetaData.pData = (void *)(pStruct); \
    MMProfileLogMetaStructure(event, type, &MetaData); \
}

#ifdef __cplusplus
}
#endif
#endif