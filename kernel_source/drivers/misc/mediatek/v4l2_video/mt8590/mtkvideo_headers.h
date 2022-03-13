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
#ifndef __MTKVIDEO_HEADERS_H
#define __MTKVIDEO_HEADERS_H


/* linux header files */
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/vmalloc.h>
#include <linux/disp_assert_layer.h>
#include <linux/semaphore.h>
#include <linux/xlog.h>
#include <linux/mutex.h>
#include <linux/leds-mt65xx.h>
#include <linux/file.h>
#include <linux/ion_drv.h>
#include <linux/list.h>
#include <linux/types.h> // houlong
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/mach-types.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <mach/dma.h>
#include <mach/irqs.h>
#include <mach/m4u_port.h>
#include <linux/dma-mapping.h>
#include <mach/mt_boot.h>
#include <mach/mt_typedefs.h> // MTK
#include <linux/fb.h> // struct fb_info

/* v4l2 Header files */
#include <linux/videodev2.h>
#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-chip-ident.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-vmalloc.h>
#include <linux/v4l2-dv-timings.h>
#include <linux/mtk_v4l2.h>

// for MTK_HDMI_MAIN_PATH
#include "disp_drv_platform.h"

/* other header files */
#include "debug.h"
#include "disp_drv.h"
#include "ddp_hal.h"
#include "disp_drv_log.h"
#include "disp_hal.h"
#include "ddp_dpfd.h" // dpfd support

#include "mtkfb.h"
#include "mtkfb_console.h"
#include "mtkfb_info.h"
#include "ddp_ovl.h"
#include "mtkfb_priv.h"
#if defined(CONFIG_MTK_OVERLAY_ENGINE_SUPPORT) && (CONFIG_MTK_OVERLAY_ENGINE_SUPPORT)
#include "disp_ovl_engine_api.h"
#include "disp_ovl_engine_hw.h"
#include "disp_ovl_engine_core.h"
#include "disp_ovl_engine_dev.h"
#endif
#include "hdmitable.h"
#include "hdmitx.h"
#include "tve_common.h"
#include "tve_drv.h"
#if defined(CONFIG_MTK_FB_1) && (CONFIG_MTK_FB_1)
#include "mtkfb_1.h"
#endif
#include "m4u_priv.h"

/* internal header files */
#include "mtkvideo_debug.h"
#include "mtkvideo_impl.h"
#include "mtkvideo_adapter.h"

extern void dump_stack(void);

#define MTKVIDEO_OUTPUT_RESOLUTION_MAX_W 1920
#define MTKVIDEO_OUTPUT_RESOLUTION_MAX_H 1088
#define MTKVIDEO_MDP_BUFFER_NUMBER       3 /* Used as MDP destination buffer */
#define MTKVIDEO_OVL_BUFFER_NUMBER       4 /* Used as OVL output buffer */

#define MTKVIDEO_INVALID_FENCE      (-1)
#define MTKVIDEO_INVALID_ION_HANDLE (-1)

enum mtkvideo_v4l2_disp_scenario
{
    MTK_V4L2_DISP_LCD = 0, /* LCD displays primary UI and video */
    MTK_V4L2_DISP_LCD_HDMI = 1, /* LCD displays primary UI, HDMI displays external UI and video. */
    MTK_V4L2_DISP_LCD_CVBS = 2, /* LCD displays primary UI, CVBS displays external UI and video. */
};

/* Video output port */
/* Default is MTK_V4L2_OUTPUT_PORT_COUNT, so user must set video output port explicitly. */
enum mtkvideo_v4l2_output_port
{
    MTK_V4L2_OUTPUT_PORT_LCD = 0, /* Set output port as LCD will close HDMI and CVBS port. */
    MTK_V4L2_OUTPUT_PORT_HDMI,
    MTK_V4L2_OUTPUT_PORT_CVBS,
    MTK_V4L2_OUTPUT_PORT_COUNT,
};

struct mtkvideo_v4l2_timings_map
{
    struct v4l2_dv_timings t;
    HDMI_VIDEO_RESOLUTION h; // hdmi res
    int c; // cvbs res
};

struct mtkvideo_v4l2_param
{
    struct v4l2_capability cap; /* vidioc_querycap */
    struct v4l2_rect src_c;  /* vidioc_s_crop */
    int rotate; /* vidioc_s_ctrl, CID_ROTATE, 0, 90, 180, 270 */
    struct v4l2_format pix; /* vidioc_s_fmt, vidioc_g_fmt: overlay source pixel format */
    struct v4l2_format win; /* vidioc_s_fmt, vidioc_g_fmt: overlay output window */
    enum mtkvideo_v4l2_output_port vop; /* Video output port, initialize with unknown */
    struct mtkvideo_v4l2_timings_map timing;
    struct v4l2_requestbuffers reqbuf;
};

struct mtkvideo_vb2_buffer {
    struct vb2_buffer b;
    struct list_head list;
    /* user_va, mva, length are corresponding to b->v4l2_planes */
    unsigned int u_va[VIDEO_MAX_PLANES]; // User VA, not used currently
    unsigned int k_va[VIDEO_MAX_PLANES]; // kernel VA
    unsigned int mva[VIDEO_MAX_PLANES]; // MVA
    unsigned int length[VIDEO_MAX_PLANES];
    struct ion_handle *handle[VIDEO_MAX_PLANES];
};

struct mtkvideo_vb2_queue {
    struct vb2_queue q;
    struct list_head list; /* List of queued buffers */
    spinlock_t lock;            /* Used in video-buf */
};

enum mtkvideo_buffer_state
{
    MTKV_BUF_UNUSED = 1 << 0, /* Unused */
    MTKV_BUF_SELECTED,        /* Selected and will be filled. */
    MTKV_BUF_FILLED,          /* Filled and will be displayed. */
    MTKV_BUF_DISPLAYING,
};

struct mtkvideo_buffer
{
    unsigned int va;  /* virtual address */
    unsigned int mva; /* used for hw */
    enum mtkvideo_buffer_state state; /* buffer state */
    struct semaphore lock; /* lock used for buffer access protection */
    struct fb_overlay_layer layer; /* temparary variable */
};

struct mtkvideo_mdp_buf_info
{
    unsigned int buf_size; /* byte size of one buffer */
    struct mtkvideo_buffer buf[MTKVIDEO_MDP_BUFFER_NUMBER];
    unsigned int widx; /* MDP output buffer index */
    unsigned int ridx; /* ovl or rdma read index */
    unsigned int next_widx;
    spinlock_t   idx_lock; /* lock used for idx update */
};

struct mtkvideo_ovl_buf_info
{
    unsigned int buf_size; /* byte size of one buffer */
    spinlock_t   lock; /* lock used for buf state update */
    struct mtkvideo_buffer buf[MTKVIDEO_OVL_BUFFER_NUMBER];
};

struct mtkvideo_buf_info
{
    unsigned int base_va;
    unsigned int base_mva;
    unsigned int alloc_size; /* total alloc size */
    struct mtkvideo_mdp_buf_info mdp_bi; /* mdp buffer info */
    struct mtkvideo_ovl_buf_info ovl_bi; /* ovl buffer info */
};

struct mtkvideo_ovl_param
{
    //struct fb_overlay_config;
    int fence;
    int time;
    struct fb_overlay_layer layers[HW_OVERLAY_COUNT];
    struct semaphore lock[HW_OVERLAY_COUNT];
    struct disp_ovl_engine_config_mem_out_struct out;

    /* display width */
    int width;
    int height;
};

struct mtkvideo_fb_info
{
    unsigned long long update; /* update time */
    unsigned long long last_update; /* last update time */
    struct fb_overlay_layer layer; /* temparary variable */
};

struct mtkvideo_drv_param
{
    struct mtkvideo_v4l2_param v4l2_param;
    struct mtkvideo_vb2_queue q;
    struct video_device *pVideoDevice;
    enum mtkvideo_v4l2_disp_scenario scenario;
    int  io_allowed;
    struct mtkvideo_buf_info bi; /* buffer info */
    struct mtkvideo_ovl_param ovl; /* used for primary display path */
    struct mtkvideo_ovl_param ovl_s; /* used for secondary display path */
    struct mtkvideo_fb_info  fb;  /* used for primary display path */
    struct mtkvideo_fb_info  fb_s; /* used for secondary dispaly path */
    int resumed; /* resumed flag */
};

/* File handle structure */
struct mtkvideo_fh {
    int ref_cnt; /* reference count */
    struct mtkvideo_drv_param p;
};

extern struct mtkvideo_fh g_mtkvideo_fh;

#endif /* __MTKVIDEO_HEADERS_H */
