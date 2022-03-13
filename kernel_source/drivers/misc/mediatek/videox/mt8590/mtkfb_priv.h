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
#ifndef __MTKFB_PRIV_H
#define __MTKFB_PRIV_H

#if defined(MTK_OVERLAY_ENGINE_SUPPORT)
#include "disp_ovl_engine_api.h"
#endif
#include <linux/fb.h>

#if defined(MTK_OVERLAY_ENGINE_SUPPORT)
extern DISP_OVL_ENGINE_INSTANCE_HANDLE  mtkfb_instance;
#endif

extern unsigned int isAEEEnabled;

/*********************************************************************/
/********************** Used for V4L2 ********************************/
/*********************************************************************/
#if defined(CONFIG_MTK_V4L2_VIDEO_SUPPORT) && (CONFIG_MTK_V4L2_VIDEO_SUPPORT)
#define DISPLAY_V4L2 1
#else
#define DISPLAY_V4L2 0
#endif

#if DISPLAY_V4L2

#define MTKFB_QUEUE_OVERLAY_CONFIG_V4L2  _IOW('O', 240, struct fb_overlay_config)

typedef int (*HOOK_PAN_DISPLAY_IMPL)(struct fb_var_screeninfo *var, struct fb_info *info);
typedef void (*BUF_NOTIFY_CALLBACK)(int mva);
extern HOOK_PAN_DISPLAY_IMPL  mtkfb_hook_pan_display;
extern HOOK_PAN_DISPLAY_IMPL  mtkfb_1_hook_pan_display;
extern BOOL g_is_v4l2_active;
extern BUF_NOTIFY_CALLBACK    hdmitx_buf_receive_cb;
extern BUF_NOTIFY_CALLBACK    hdmitx_buf_remove_cb;
extern BUF_NOTIFY_CALLBACK    mtkfb_buf_receive_cb;
extern BUF_NOTIFY_CALLBACK    mtkfb_buf_remove_cb;

int mtkfb_1_update_res(struct fb_info *fbi, unsigned int xres, unsigned int yres);

#endif

#endif				/* __MTKFB_PRIV_H */