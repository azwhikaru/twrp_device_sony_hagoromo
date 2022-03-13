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
/*
 * mtkvideo_adapter.h - V4L2 adapter layer for display interface wrapping.
 *
 */

#include "mtkvideo_adapter.h"


static struct file g_ovl_eng_file;
static struct file g_hdmitx_file;
static wait_queue_head_t mtkfb_0_update_wq;
static atomic_t mtkfb_0_update_event;
struct semaphore mtkfb_0_update_mutex;
static wait_queue_head_t mtkfb_1_update_wq;
static atomic_t mtkfb_1_update_event;
struct semaphore mtkfb_1_update_mutex;
struct semaphore mtkv_compose_mutex;

/**********************************************************************
**************** extern variables & functions
**********************************************************************/
extern struct fb_info *mtkfb_fbi; /* stands for /dev/graphics/fb0 */
#if defined(CONFIG_MTK_FB_1) && (CONFIG_MTK_FB_1)
extern struct fb_info *mtkfb_1_fbi; /* stands for /dev/grahpics/fb1 */
#endif

#if defined(CONFIG_MTK_HDMI_SUPPORT) && (CONFIG_MTK_HDMI_SUPPORT)
extern long hdmi_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static long hdmi_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    printk("[V4L2] not support HDMI output!!!\n");
    return -1;
}
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,7,0)
extern int mtkfb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg);
#else
extern int mtkfb_ioctl(struct file *file, struct fb_info *info, unsigned int cmd, unsigned long arg);
#endif
extern int disp_ovl_engine_open(struct inode *inode, struct file *file);
extern int disp_ovl_engine_release(struct inode *inode, struct file *file);
extern long disp_ovl_engine_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#if defined(CONFIG_MTK_TVE_SUPPORT) && (CONFIG_MTK_TVE_SUPPORT)
extern long cvbs_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static long cvbs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return -1;
}
#endif


static int g_only_test_do_cache = 1;
/**********************************************************************
**************** V4L2 adapter layer functions
**********************************************************************/
static int mtkvideo_adpt_fb_pan_display_impl(struct fb_var_screeninfo *var, struct fb_info *info)
{
    static int update_idx = 0;
    struct mtkfb_device *fbdev;
    UINT32 offset;
    UINT32 paStart;
    char *vaStart, *vaEnd;
    int ret = 0;
    int fb_pa = 0;
    unsigned int fb_idx;
    struct semaphore *sema;
    unsigned long long t;
    struct fb_overlay_layer *layer;
    wait_queue_head_t *wq;
    atomic_t *event;

    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);

    if (g_mtkvideo_fh.ref_cnt <= 0)
    {
        // "/dev/video0" is not opened, means V4L2 is not used.
        // return -1, then pan displan goes to normal flow for "/dev/graphics/fb0".
        return -1;
    }

    if (info == mtkfb_fbi)
    {
        // fb0
        fb_idx = 0;
        wq = &mtkfb_0_update_wq;
        event = &mtkfb_0_update_event;
        sema = &mtkfb_0_update_mutex;
        layer = &g_mtkvideo_fh.p.fb.layer;
    }
#if defined(CONFIG_MTK_FB_1) && (CONFIG_MTK_FB_1)
    else // if (info == mtkfb_1_fbi)
    {
        // fb1
        fb_idx = 1;
        wq = &mtkfb_1_update_wq;
        event = &mtkfb_1_update_event;
        sema = &mtkfb_1_update_mutex;
        layer = &g_mtkvideo_fh.p.fb_s.layer;

        if (MTK_V4L2_DISP_LCD == g_mtkvideo_fh.p.scenario)
        {
            MTKVIDEO_ERROR("no external output, no need update sub UI!!!");
            return -1;
        }
    }
#endif

    MTKVIDEO_LOG("fb%d,xoffset=%u, yoffset=%u, xres=%u, yres=%u, xresv=%u, yresv=%u",
        fb_idx,
        var->xoffset, var->yoffset,
        info->var.xres, info->var.yres,
        info->var.xres_virtual,
        info->var.yres_virtual);

    if(down_interruptible(sema))
    {
        MTKVIDEO_ERROR("down error");
    }

    fbdev = (struct mtkfb_device *)info->par;
    fb_pa = fbdev->fb_pa_base;

    info->var.yoffset = var->yoffset;

    offset = var->yoffset * info->fix.line_length;
    paStart = fb_pa + offset;
    vaStart = info->screen_base + offset;
    vaEnd   = vaStart + info->var.yres * info->fix.line_length;

    layer->layer_id = 1; // UI layer is 1
    layer->layer_enable = TRUE;
    layer->src_base_addr = (void *)((unsigned int)vaStart);
    layer->src_phy_addr = (void *)paStart;
    layer->src_direct_link = 0;
    switch(var->bits_per_pixel)
    {
    case 16:
        layer->src_fmt = MTK_FB_FORMAT_RGB565;
        layer->alpha_enable = 0;
        layer->alpha = 0;
        break;
    case 24:
        layer->src_fmt = MTK_FB_FORMAT_RGB888;
        layer->alpha_enable = 0;
        layer->alpha = 0;
        break;
    case 32:
        layer->src_fmt = MTK_FB_FORMAT_XRGB8888; // responding to eARGB8888, OVL_INPUT_FORMAT_ARGB8888
        layer->alpha_enable = 1;
        layer->alpha = 255;
        break;
    default:
        MTKVIDEO_ERROR("Invalid color format bpp: 0x%d\n", var->bits_per_pixel);
        up(sema);
        return -1;
    }
    layer->src_use_color_key = 0;
    layer->src_color_key = 0xFF;
    layer->src_pitch = ALIGN_TO(var->xres, disphal_get_fb_alignment());
    layer->src_offset_x = 0;
    layer->src_offset_y = 0;
    layer->src_width = var->xres;
    layer->src_height = var->yres;
    layer->tgt_offset_x = 0;
    layer->tgt_offset_y = 0;
    layer->tgt_width = var->xres;
    layer->tgt_height = var->yres;
    layer->layer_rotation = MTK_FB_ORIENTATION_0;
    layer->layer_type = LAYER_2D;
    layer->video_rotation = MTK_FB_ORIENTATION_0;
    layer->isTdshp = TRUE;  // set to 1, will go through tdshp first, then layer blending, then to color
    layer->next_buff_idx = -1;
    layer->identity = 0;
    layer->connected_type = 0;
    layer->security = LAYER_NORMAL_BUFFER;
    layer->fence_fd = -1;
    layer->ion_fd = -1;

    up(sema);

    t = update_idx++;

    // wake up ovl to compose
    if (0 == fb_idx)
    {
        mtkvideo_bh_fb_0_update(t);
    }
    else
    {
        /* this is just for test, because test program draw UI by CPU. */
        /* !!!!!!!!!!! NOTE */
        if (g_only_test_do_cache)
        {
            m4u_dma_cache_maint(DISP_WDMA,
                (void const *)vaStart,
                info->var.yres * info->fix.line_length,
                DMA_BIDIRECTIONAL);
        }

        mtkvideo_bh_fb_1_update(t);
    }

    // wait display ok
    atomic_set(event, 0);
    // timeout: 1/4 second.
    ret = wait_event_interruptible_timeout(*wq, atomic_read(event), HZ/4);

    if (!ret)
    {
        MTKVIDEO_ERROR("wait update fb%d timeout.", fb_idx);
    }

    // !!! ALWAYS RETURN 0 FOR V4L2.
    // !!! always return 0 for v4l2.
    return 0;
}

int mtkvideo_adpt_sync_state(int active)
{
    if (active)
    {
        g_is_v4l2_active = TRUE;
        mtkfb_hook_pan_display = mtkvideo_adpt_fb_pan_display_impl;
#if defined(CONFIG_MTK_FB_1) && (CONFIG_MTK_FB_1)
        mtkfb_1_hook_pan_display = mtkvideo_adpt_fb_pan_display_impl;
#endif
    }
    else
    {
        g_is_v4l2_active = FALSE;
        mtkfb_hook_pan_display = NULL;
#if defined(CONFIG_MTK_FB_1) && (CONFIG_MTK_FB_1)
        mtkfb_1_hook_pan_display = NULL;
#endif
    }

    MTKVIDEO_MSG("v4l2 state is %s", active ? "active" : "inactive");

    return 0;
}

long mtkvideo_adpt_disp_init(void)
{
    int ret;

    MTKVIDEO_API_ENTRY();

    memset(&g_ovl_eng_file, 0, sizeof(g_ovl_eng_file));

    ret = disp_ovl_engine_open(NULL, &g_ovl_eng_file);
    if (0 != ret)
    {
        MTKVIDEO_MSG("ovl engine open failed:%d", ret);
    }

    /* Normal hdmtx file handle doesn't use private_data.
       So set 1 to differ normal handle. */
    memset(&g_hdmitx_file, 0, sizeof(g_hdmitx_file));
    g_hdmitx_file.private_data = (void *)1;
#if defined(CONFIG_MTK_HDMI_SUPPORT)    
    hdmitx_buf_receive_cb = mtkvideo_buf_before_display_notify;
    hdmitx_buf_remove_cb = mtkvideo_buf_after_display_notify;
#endif
    mtkfb_buf_receive_cb = mtkvideo_buf_before_display_notify;
    mtkfb_buf_remove_cb = mtkvideo_buf_after_display_notify;

    init_waitqueue_head(&mtkfb_0_update_wq);
    atomic_set(&mtkfb_0_update_event, 0);
    sema_init(&mtkfb_0_update_mutex, 1);
    init_waitqueue_head(&mtkfb_1_update_wq);
    atomic_set(&mtkfb_1_update_event, 0);
    sema_init(&mtkfb_1_update_mutex, 1);
    sema_init(&mtkv_compose_mutex, 1);

    return ret;
}

long mtkvideo_adpt_disp_uninit(void)
{
    int ret;

    MTKVIDEO_API_ENTRY();

    ret = disp_ovl_engine_release(NULL, &g_ovl_eng_file);
    if (0 != ret)
    {
        MTKVIDEO_MSG("ovl engine release failed:%d", ret);
    }

    return ret;
}

long mtkvideo_adpt_hdmi_func(unsigned int cmd, unsigned long arg)
{
    long ret;

    MTKVIDEO_API_ENTRY();

    ret = hdmi_ioctl(&g_hdmitx_file, cmd, arg);

    return ret;
}

long mtkvideo_adpt_cvbs_func(unsigned int cmd, unsigned long arg)
{
    long ret;

    MTKVIDEO_API_ENTRY();

    ret = cvbs_ioctl(NULL, cmd, arg);

    return ret;
}

int mtkvideo_adpt_mtkfb_func(unsigned int cmd, unsigned long arg)
{
    int ret;
    struct fb_info *i = mtkfb_fbi;

    MTKVIDEO_API_ENTRY();

    #if LINUX_VERSION_CODE > KERNEL_VERSION(3,7,0)
    ret = mtkfb_ioctl(i, cmd, arg);
    #else
    ret = mtkfb_ioctl(NULL, i, cmd, arg);
    #endif

    return ret;
}

/* display overlay engine  */
long mtkvideo_adpt_mtk_disp(unsigned int cmd, unsigned long arg)
{
    long ret;

    MTKVIDEO_API_ENTRY();

    ret = disp_ovl_engine_unlocked_ioctl(&g_ovl_eng_file, cmd, arg);

    return ret;
}

int mtkvideo_adpt_buf_init(struct mtkvideo_buf_info *bi)
{
    unsigned int i;
    unsigned int alloc_size, buf_size;
    unsigned int w, h, bpp, pages;
    struct mtkvideo_buffer *b;

    MTKVIDEO_API_ENTRY();

    MTKVIDEO_ASSERT(bi && (0 == bi->base_va));

    w = MTKVIDEO_OUTPUT_RESOLUTION_MAX_W; /* MAX */
    h = MTKVIDEO_OUTPUT_RESOLUTION_MAX_H; /* MAX */
    //bpp = 2; /* MDP output is YUYV (YUV 422 raster scan, one planes. */
    bpp = 3; /* OVL engine output is RGB888 after composing layers */
    pages = MTKVIDEO_MDP_BUFFER_NUMBER + MTKVIDEO_OVL_BUFFER_NUMBER;
    buf_size = w * h * bpp;
    alloc_size = buf_size * pages;
    MTKVIDEO_LOG("alloc adpt buf: w=%d, h=%d, bpp=%d, cnt=%d", w, h, bpp, pages);

    // alloc virtual memory
    bi->base_va = (unsigned int)vmalloc(alloc_size);

    if (!bi->base_va)
    {
        MTKVIDEO_LOG("vmalloc %dbytes fail", alloc_size);
        return -ENOMEM;
    }

    if (m4u_alloc_mva(MDP_WDMA,
                bi->base_va,
                alloc_size,
                0,
                0,
                &bi->base_mva))
    {
        vfree((void *)bi->base_va);
        bi->base_va = 0;
        MTKVIDEO_LOG("m4u_alloc_mva for v4l2 mdp fail.");
        return -ENOMEM;
    }

    m4u_dma_cache_maint(MDP_WDMA,
            (void const *)bi->base_va,
            alloc_size,
            DMA_BIDIRECTIONAL);

    bi->alloc_size = alloc_size;

    /* mdp buffer info init */
    bi->mdp_bi.buf_size = buf_size;
    b = &bi->mdp_bi.buf[0];

    for (i = 0; i < MTKVIDEO_MDP_BUFFER_NUMBER; i++)
    {
        // assign buffer address
        b[i].va = bi->base_va + buf_size * i;
        b[i].mva = bi->base_mva + buf_size * i;
        //
        b[i].state = MTKV_BUF_UNUSED;
        // init buffer protection semaphore
        sema_init(&b[i].lock, 1);
    }

    spin_lock_init(&bi->mdp_bi.idx_lock);

    bi->mdp_bi.widx = MTKVIDEO_MDP_BUFFER_NUMBER;
    bi->mdp_bi.ridx = MTKVIDEO_MDP_BUFFER_NUMBER;
    bi->mdp_bi.next_widx = MTKVIDEO_MDP_BUFFER_NUMBER;

    /* ovl buffer info init */
    bi->ovl_bi.buf_size = buf_size;
    spin_lock_init(&bi->ovl_bi.lock);
    b = &bi->ovl_bi.buf[0];

    for (i = 0; i < MTKVIDEO_OVL_BUFFER_NUMBER; i++)
    {
        // assign buffer address
        b[i].va = bi->base_va + buf_size * (MTKVIDEO_MDP_BUFFER_NUMBER + i);
        b[i].mva = bi->base_mva + buf_size * (MTKVIDEO_MDP_BUFFER_NUMBER + i);
        b[i].state = MTKV_BUF_UNUSED;
        // init buffer protection semaphore
        sema_init(&b[i].lock, 1);
    }

    MTKVIDEO_LOG("M4U alloc mva: 0x%x va: 0x%x size: 0x%x",
            bi->base_mva, bi->base_va, alloc_size);

    return 0;
}

int mtkvideo_adpt_buf_uninit(struct mtkvideo_buf_info *bi)
{
    int ret = 0;

    MTKVIDEO_API_ENTRY();

    MTKVIDEO_ASSERT(bi);

    if (bi->base_va)
    {
        m4u_dealloc_mva(MDP_WDMA,
            bi->base_va,
            bi->alloc_size,
            bi->base_mva);

        vfree((void *)bi->base_va);
    }

    memset(bi, 0, sizeof(*bi));

    return ret;
}

int mtkvideo_adpt_ovl_init(struct mtkvideo_ovl_param *ovl)
{
    unsigned int i;

    MTKVIDEO_API_ENTRY();

    memset(ovl, 0, sizeof(*ovl));

    for (i = 0; i < HW_OVERLAY_COUNT; i++)
    {
        ovl->layers[i].layer_id = i;
        ovl->layers[i].layer_enable = 0;
        ovl->layers[i].alpha_enable = 0;
        ovl->layers[i].alpha = 0;
        ovl->layers[i].fence_fd = -1;
        ovl->layers[i].ion_fd = -1;
        sema_init(&ovl->lock[i], 1);
        ovl->out.ion_fd = -1;
    }

    return 0;
}

int mtkvideo_adpt_ovl_uninit(struct mtkvideo_ovl_param *ovl)
{
    MTKVIDEO_API_ENTRY();
    // do nothing

    return 0;
}


static unsigned int map_v4l2_format_2_dp_format(unsigned int v4l2_format, unsigned int *y_stride, unsigned int *uv_stride)
{
    unsigned int dpFormat;
    unsigned int stride_y, stride_c;

    // stride information: reference blit.cpp, set_config caculation.
    switch (v4l2_format)
    {
    case V4L2_PIX_FMT_NV12_MTK_BLK:
        /* video decode hw output color format */
        /* NV12: Y/CbCr 420 block, two planes */
        dpFormat = eNV12_BLK_K;
        stride_y = 32;
        stride_c = 16;
        break;
    case V4L2_PIX_FMT_NV16:
        /* video decode hw output is processed by HW DI or NR */
        /* NV12 -> NV16 */
        /* NV16: Y/CbCr 422 raster scan, two planes*/
        dpFormat = eNV16_K;
        stride_y = 2;
        stride_c = 1;
        break;
    case V4L2_PIX_FMT_NV61:
        /* video decode hw output is processed by HW DI or NR */
        /* NV12 -> NV16 */
        /* NV16: Y/CbCr 422 raster scan, two planes*/
        dpFormat = eNV61_K;
        stride_y = 2;
        stride_c = 1;
        break;
    case V4L2_PIX_FMT_NV12:
        dpFormat = eNV12_K;
        stride_y = 2;
        stride_c = 1;
        break;
    case V4L2_PIX_FMT_NV21:
        dpFormat = eNV21_K;
        stride_y = 2;
        stride_c = 1;
        break;
    case V4L2_PIX_FMT_YUV420M:
        dpFormat = eYUV_420_3P_K;
        stride_y = 1;
        break;
    case V4L2_PIX_FMT_YVU420M:
        dpFormat = eYUV_420_3P_YVU_K;
        stride_y = 1;
        break;
    default:
        dpFormat = -1;
        stride_y = 0;
        break;
    }

    if (y_stride)
    {
        *y_stride = stride_y;
    }

    if (uv_stride)
    {
        *uv_stride = stride_c;
    }

    return dpFormat;
}

//
// TODO: srcX,srcY, dstX, dstY are not ZERO.
// Fox example: the display window is not full screen.
int mtkvideo_adpt_mdp_process(struct mtkvideo_drv_param *p, struct mtkvideo_vb2_buffer *b)
{
    struct v4l2_rect src_r;
    struct v4l2_rect dst_r;
    unsigned int src_format, y_stride, uv_stride;
    DdpkBitbltConfig pddp;
    struct v4l2_format *pix_fmt;
    int i, nplanes;

    MTKVIDEO_API_ENTRY();
    MTKVIDEO_ASSERT(p && b);

    pix_fmt = &p->v4l2_param.pix;

    if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == pix_fmt->type)
    {
        nplanes = pix_fmt->fmt.pix_mp.num_planes;
    }
    else // if (V4L2_BUF_TYPE_VIDEO_OUTPUT == pix_fmt->type)
    {
        nplanes = 1;
    }

    memset((void *)&pddp, 0, sizeof(DdpkBitbltConfig));

    src_format = map_v4l2_format_2_dp_format(p->v4l2_param.pix.fmt.pix.pixelformat, &y_stride, &uv_stride);
    if (-1 == src_format)
    {
        MTKVIDEO_MSG("Error src color format=%x", p->v4l2_param.pix.fmt.pix.pixelformat);
        return -1;
    }

    src_r.left = 0;
    src_r.top = 0;
    src_r.width = p->v4l2_param.pix.fmt.pix.width;
    src_r.height = p->v4l2_param.pix.fmt.pix.height;

    if (V4L2_PIX_FMT_NV12_MTK_BLK == p->v4l2_param.pix.fmt.pix.pixelformat)
    {
        src_r.width += 15;
        src_r.width &= ~15;
        src_r.height += 31;
        src_r.height &= ~31;
    }

    pddp.srcX = src_r.left;
    pddp.srcY = src_r.top;
    pddp.srcW = src_r.width;
    pddp.srcH = src_r.height;
    pddp.srcWStride = src_r.width;
    pddp.srcHStride = src_r.height;

    for (i = 0; i < nplanes; i++)
    {
        pddp.srcAddr[i] = b->mva[i];
        pddp.srcBufferSize[i] = b->length[i];
    }

    pddp.srcFormat = src_format;
    pddp.srcPlaneNum = nplanes;
    pddp.srcMemType = DPFD_MEMORY_MVA;

    pddp.m_cropXStart =  pddp.srcX ; //source crop x offset
    pddp.m_cropYStart =  pddp.srcY ; //source crop y offset
    pddp.m_cropWidth =   pddp.srcW ; //source crop width offset
    pddp.m_cropHeight =  pddp.srcH ; //source crop height offset

    dst_r = p->v4l2_param.win.fmt.win.w;
    pddp.dstFormat = eYUY2_K ; // YUYV

    pddp.orientation = p->v4l2_param.rotate;
    pddp.dstX = dst_r.left;
    pddp.dstY = dst_r.top;
    pddp.dstW = ALIGN_TO(dst_r.width, 4);
    pddp.dstH = ALIGN_TO(dst_r.height, 4);
    pddp.dstWStride = pddp.dstX + pddp.dstW;
    pddp.dstHStride = pddp.dstY + pddp.dstH;
    pddp.pitch = pddp.dstW * 2;

    pddp.dstPlaneNum = 1;
    pddp.dstMemType = DPFD_MEMORY_MVA;


    pddp.dstAddr[0] = p->bi.mdp_bi.buf[p->bi.mdp_bi.next_widx].mva;
    pddp.dstBufferSize[0] = p->bi.mdp_bi.buf_size;
    DDPK_Bitblt_Config(DDPK_CH_HDMI_0, &pddp);
    DDPK_Bitblt(DDPK_CH_HDMI_0);

    return 0;
}

int mtkvideo_adpt_compose(struct mtkvideo_ovl_param *ovl)
{
    int i, ret;
    DISP_OVL_ENGINE_FILE_HANDLE_MAP *map;
    DISP_OVL_ENGINE_INSTANCE_HANDLE handle;

    map = (DISP_OVL_ENGINE_FILE_HANDLE_MAP *)g_ovl_eng_file.private_data;
    handle = map->mHandle;

    if(down_interruptible(&mtkv_compose_mutex))
    {
        MTKVIDEO_ERROR("down error");
        return -1;
    }

    for (i = 0; i < HW_OVERLAY_COUNT; i++)
    {
        ret = Disp_Ovl_Engine_Set_layer_info(handle, &ovl->layers[i]);
        if (OVL_OK != ret)
        {
            MTKVIDEO_ERROR("Set Layer[%d] failed.", i);
            break;
        }
    }

    if (OVL_OK == ret)
    {
        ret = Disp_Ovl_Engine_Set_Overlayed_Buffer(handle, &ovl->out);
        if (OVL_OK != ret)
        {
            MTKVIDEO_ERROR("Set out buffer failed.");
        }
    }

    if (OVL_OK == ret)
    {
        ret = Disp_Ovl_Engine_Trigger_Overlay(handle);
        if (OVL_OK != ret)
        {
            MTKVIDEO_ERROR("Trigger OVL failed.");
        }
    }

    if (OVL_OK == ret)
    {
        ret = Disp_Ovl_Engine_Wait_Overlay_Complete(handle, HZ);
        if (OVL_OK != ret)
        {
            MTKVIDEO_ERROR("Wait OVL complete failed.");
        }
    }

    up(&mtkv_compose_mutex);

    return ret;
}

static void mtkvideo_adpt_configure_hdmi(hdmi_video_buffer_info *config, int dw, int dh,
    int format, int pitch, bool is_secure, void* mva)
{
    config->src_base_addr  = 0;
 	config->src_phy_addr   = mva;
 	config->src_pitch      = pitch;
    config->src_fmt        = format;
    config->src_offset_x   = 0;
    config->src_offset_y   = 0;
    config->src_width      = dw;
    config->src_height     = dh;

    // Security
    if(is_secure) {
       config->security = LAYER_SECURE_BUFFER;
    }
    else {
        config->security = LAYER_NORMAL_BUFFER;
    }
}

int mtkvideo_adpt_send_buf_to_hdmi(struct mtkvideo_ovl_param *p)
{
    hdmi_video_buffer_info layer;
    //hdmi_buffer_info hdmi_buffer;
    int ret, pitch, format;

    memset(&layer, 0, sizeof(hdmi_video_buffer_info));
    //memset(&hdmi_buffer, 0, sizeof(hdmi_buffer_info));

    if (MTK_FB_FORMAT_RGB888 != p->out.outFormat)
    {
        MTKVIDEO_ERROR("rdma only receive RGB888 format by SW");
        return -1;
    }
    else
    {
        //pitch = p->width * 3;
        pitch = p->width;
        format = RDMA_INPUT_FORMAT_RGB888;
    }

    //pitch = p->width * mtkvideo_adpt_get_layer_pitch(p->out.outFormat);

    mtkvideo_adpt_configure_hdmi(&layer,
        p->width, p->height, format,
        pitch, p->out.security, (void *)p->out.dstAddr);

    layer.fenceFd = -1; //ovl_job->dst_buf->acquire_fence_fd;

    // V4L2 no need call PREPARE_BUFFER, because it only creates fence.
#if 0
    if((ret = mtkvideo_adpt_hdmi_func(MTK_HDMI_PREPARE_BUFFER, (unsigned long)&hdmi_buffer)) < 0)
    {
        MTKVIDEO_ERROR("hdmi prepare buffer failed:%d", ret);
        //hdmi_buffer.fence_fd = -1;
    }
    else
#endif
    {
        if((ret = mtkvideo_adpt_hdmi_func(MTK_HDMI_POST_VIDEO_BUFFER, (unsigned long)&layer)) < 0)
        {
            MTKVIDEO_ERROR("hdmi post buffer failed:%d", ret);
            //hdmi_buffer.fence_fd = -1;
        }
    }

    return ret;
}


int mtkvideo_adpt_send_buf_to_cvbs(struct mtkvideo_ovl_param *p)
{
    static tve_video_buffer_info buffer;
    int ret, pitch, format;

    memset(&buffer, 0, sizeof(tve_video_buffer_info));

    buffer.src_base_addr  = 0;
 	buffer.src_phy_addr   = (void *)p->out.dstAddr;
 	buffer.src_pitch      = p->width;
    buffer.src_fmt        = p->out.outFormat;
    buffer.src_offset_x   = 0;
    buffer.src_offset_y   = 0;
    buffer.src_width      = p->width;
    buffer.src_height     = p->height;

    if(p->out.security) {
	    buffer.security = LAYER_SECURE_BUFFER;
    }
    else {
        buffer.security = LAYER_NORMAL_BUFFER;
    }

    buffer.fenceFd = -1;


    // JUST FOR DEBUG
    if (buffer.src_phy_addr == 0)
    {
        MTKVIDEO_ERROR("Skip NULL buffer to CVBS");
		return 0;
    }
	else
	{
        MTKVIDEO_ERROR("Send buffer to CVBS: mva=%08X", buffer.src_phy_addr);
	}

    // V4L2 no need call PREPARE_BUFFER, because it only creates fence.
	ret = mtkvideo_adpt_cvbs_func(CMD_TVE_POST_VIDEO_BUFFER, (unsigned long)&buffer);

    return ret;
}

int mtkvideo_adpt_send_buf_to_lcd_ex(struct mtkvideo_ovl_param *p)
{
    struct fb_overlay_config config;
    int i, ret;

    memset(&config, 0, sizeof(config));
    config.fence = -1;

    for (i = 0; i < HW_OVERLAY_COUNT; i++)
    {
        config.layers[i] = p->layers[i];
    }

    if((ret = mtkvideo_adpt_mtkfb_func(MTKFB_QUEUE_OVERLAY_CONFIG_V4L2, (unsigned long)&config)) < 0)
    {
        MTKVIDEO_ERROR("mtkfb queue config failed:%d", ret);
    }

    return ret;
}

int mtkvideo_adpt_send_buf_to_lcd(struct mtkvideo_ovl_param *p)
{
    struct fb_overlay_config config;
    struct fb_overlay_layer *layer;
    int i, ret;

    memset(&config, 0, sizeof(config));
    config.fence = -1;

    for (i = 0; i < HW_OVERLAY_COUNT; i++)
    {
        layer = &config.layers[i];

        layer->layer_enable = 0;
        layer->layer_id = i;
        layer->fence_fd = -1;
        layer->ion_fd = -1;
    }

    layer = &config.layers[0];

    layer->layer_enable = 0;
    layer->src_base_addr = (void *)0;
    layer->src_phy_addr = (void *)p->out.dstAddr;
    layer->src_direct_link = 0;
    layer->src_fmt        = p->out.outFormat;
    layer->src_use_color_key = 0;
    layer->src_color_key = 0;
    layer->src_pitch      = p->out.srcROI.width;
    layer->src_offset_x = p->out.srcROI.x;
    layer->src_offset_y = p->out.srcROI.y;
    layer->src_width = p->out.srcROI.width;
    layer->src_height= p->out.srcROI.height;
    layer->tgt_offset_x   = 0;
    layer->tgt_offset_y   = 0;
    layer->tgt_width      = p->width;
    layer->tgt_height     = p->height;
    layer->layer_rotation = MTK_FB_ORIENTATION_0;
    layer->layer_type = LAYER_2D;
    layer->video_rotation = MTK_FB_ORIENTATION_0;
    layer->isTdshp = 0;
    layer->next_buff_idx = -1;
    layer->identity = 0;
    layer->connected_type = -1;
    layer->security = LAYER_NORMAL_BUFFER;
    layer->fence_fd = -1;
    layer->ion_fd = -1;
    layer->alpha_enable = 0; // RGB888
    layer->alpha = 0;

    if((ret = mtkvideo_adpt_mtkfb_func(MTKFB_QUEUE_OVERLAY_CONFIG_V4L2, (unsigned long)&config)) < 0)
    {
        MTKVIDEO_ERROR("mtkfb queue config failed:%d", ret);
    }

    return ret;
}

void mtkvideo_buf_before_display_notify(int mva)
{
    unsigned int i;
    struct mtkvideo_buffer *ovl_buf;
    spinlock_t *lock;

    lock = &g_mtkvideo_fh.p.bi.ovl_bi.lock;
    ovl_buf = &g_mtkvideo_fh.p.bi.ovl_bi.buf[0];

    spin_lock_bh(lock);

    for (i = 0; i < MTKVIDEO_OVL_BUFFER_NUMBER; i++)
    {
        if (mva == ovl_buf[i].mva)
        {
            ovl_buf[i].state = MTKV_BUF_DISPLAYING;
            spin_unlock_bh(lock);
            MTKVIDEO_MSG("ovl_bi.buf[%d].state=DISPLAYING", i);
            return;
        }
    }

    spin_unlock_bh(lock);

    MTKVIDEO_MSG("mva=0x%x doesn't match ovl_bi.buf!!!!!", mva);
}

void mtkvideo_buf_after_display_notify(int mva)
{
    unsigned int i;
    struct mtkvideo_buffer *ovl_buf;
    spinlock_t *lock;

    lock = &g_mtkvideo_fh.p.bi.ovl_bi.lock;
    ovl_buf = &g_mtkvideo_fh.p.bi.ovl_bi.buf[0];

    spin_lock_bh(lock);

    for (i = 0; i < MTKVIDEO_OVL_BUFFER_NUMBER; i++)
    {
        if (mva == ovl_buf[i].mva)
        {
            ovl_buf[i].state = MTKV_BUF_UNUSED;
            spin_unlock_bh(lock);
            MTKVIDEO_MSG("ovl_bi.buf[%d].state=UNUSED", i);
            return;
        }
    }

    spin_unlock_bh(lock);

    MTKVIDEO_MSG("mva=0x%x doesn't match ovl_bi.buf!!!!!", mva);
}

int mtkvideo_adpt_notify_res_change(enum mtkvideo_v4l2_output_port port, unsigned int w, unsigned int h)
{
#if defined(CONFIG_MTK_FB_1) && (CONFIG_MTK_FB_1)

    struct semaphore *sema;

    MTKVIDEO_API_ENTRY();

    if (MTK_V4L2_OUTPUT_PORT_HDMI == port ||
        MTK_V4L2_OUTPUT_PORT_CVBS == port)
    {
        sema = &mtkfb_1_update_mutex;

        if(down_interruptible(sema))
        {
            MTKVIDEO_ERROR("down error");
        }

        mtkfb_1_update_res(mtkfb_1_fbi, w, h);

        up(sema);
    }

#endif

    return 0;
}

int mtkvideo_adpt_pan_display_done(int fbidx)
{
    wait_queue_head_t *wq;
    atomic_t *event;

    if (0 == fbidx)
    {
        wq = &mtkfb_0_update_wq;
        event = &mtkfb_0_update_event;
    }
    else
    {
        wq = &mtkfb_1_update_wq;
        event = &mtkfb_1_update_event;
    }

    atomic_set(event, 1);
    wake_up_interruptible(wq);

    return 0;
}

