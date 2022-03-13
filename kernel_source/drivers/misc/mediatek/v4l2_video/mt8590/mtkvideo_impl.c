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
 * mtkvideo_impl.c - V4L2 display driver implementation.
 *
 */

#include "mtkvideo_impl.h"


/**********************************************************************
**************** extern variables & functions
**********************************************************************/
extern long hdmi_func(unsigned int cmd, unsigned long arg);


/**********************************************************************
**************** static variables
**********************************************************************/
static DEFINE_SEMAPHORE(mtkvideo_io_mutex);
/* Primary display path buffer handler variables */
static struct semaphore mtkvideo_bh_mutex;
static struct task_struct *mtkvideo_bh_task = NULL; /* Buffer Handler task */
static wait_queue_head_t mtkvideo_bh_wq;  /* Buffer Handler wait queue. */
static atomic_t mtkvideo_bh_event = ATOMIC_INIT(0);
/* Secondary display path buffer handler variables */
static struct semaphore mtkvideo_bh_sub_mutex;
static struct task_struct *mtkvideo_bh_sub_task = NULL; /* Buffer Handler task */
static wait_queue_head_t mtkvideo_bh_sub_wq;  /* Buffer Handler wait queue. */
static atomic_t mtkvideo_bh_sub_event = ATOMIC_INIT(0);
/* MDP process handler variables */
static struct semaphore mtkvideo_mdp_process_mutex;
static struct task_struct *mtkvideo_mdp_process_task = NULL; /* MDP process task */
static wait_queue_head_t mtkvideo_mdp_process_wq;  /* MDP process wait queue. */
static atomic_t mtkvideo_mdp_process_event = ATOMIC_INIT(0);

static unsigned int g_mtkvide_only_support_mva = 1;
/**********************************************************************
**************** V4L2 IOCTL Functions
**********************************************************************/
static int mtkvideo_ioctl_querycap(struct file *file, void  *fh,
                struct v4l2_capability *cap)
{
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;
    memcpy(cap, &p->v4l2_param.cap, sizeof(*cap));

    return 0;
}

static int mtkvideo_ioctl_s_crop(struct file *file, void *fh,
                const struct v4l2_crop *a)
{
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;

    /* set source crop */
    if (V4L2_BUF_TYPE_VIDEO_OVERLAY == a->type)
    {
        /* Need wait the queued buffer processing completed ??? */
        /* If the video source resolution maybe change. */
        p->v4l2_param.src_c = a->c;
    }
    else
    {
        return -EINVAL;
    }

    return 0;
}

static int mtkvideo_ioctl_s_ctrl(struct file *file, void *fh,
                struct v4l2_control *a)
{
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;

    if (V4L2_CID_ROTATE == a->id)
    {
        if (a->value % 90)
        {
            return -EINVAL;
        }

        p->v4l2_param.rotate = a->value;
    }

    return 0;
}

/* VIDIOC_G_FMT handlers */
static int mtkvideo_ioctl_g_fmt_vid_out_overlay(struct file *file, void *fh,
                struct v4l2_format *f)
{
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;

    /* get overlay output format: output window */
    if (V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY == f->type)
    {
        f->fmt.win = p->v4l2_param.win.fmt.win;
    }

    return 0;
}

static int mtkvideo_ioctl_g_fmt_vid_out(struct file *file, void *fh,
                struct v4l2_format *f)
{
    struct mtkvideo_drv_param *p;
    struct v4l2_format *pix_fmt;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;
    pix_fmt = &p->v4l2_param.pix;

    if (f->type != pix_fmt->type ||
        (V4L2_BUF_TYPE_VIDEO_OUTPUT != f->type &&
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE != f->type))
    {
        MTKVIDEO_ERROR("Unmatched type:%d, %d", f->type, pix_fmt->type);
        return -EINVAL;
    }

    /* get overlay source format: pixel format */
    f->fmt = pix_fmt->fmt;

    return 0;
}

static int mtkvideo_ioctl_s_fmt_vid_out_overlay(struct file *file, void *fh,
                struct v4l2_format *f)
{
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;

    /* set overlay output format: output window */
    if (V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY == f->type)
    {
        p->v4l2_param.win = *f;
    }

    return 0;
}

static int mtkvideo_ioctl_s_fmt_vid_out(struct file *file, void *fh,
                struct v4l2_format *f)
{
    struct mtkvideo_drv_param *p;
    struct v4l2_format *pix_fmt;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;
    pix_fmt = &p->v4l2_param.pix;

    if (V4L2_BUF_TYPE_VIDEO_OUTPUT != f->type &&
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE != f->type)
    {
        MTKVIDEO_ERROR("Unsupport type:%d", f->type);
        return -EINVAL;
    }

    /* set overlay source format: pixel format */
    /* Need wait the queued buffer processing completed ??? */
    /* If the video source resolution maybe change. */
    *pix_fmt = *f;

    // update q type
    p->q.q.type = f->type;

    return 0;
}

static int mtkvideo_ioctl_enum_output(struct file *file, void *fh,
                struct v4l2_output *a)
{
    /* vop: video output port */
    static const struct v4l2_output vop[MTK_V4L2_OUTPUT_PORT_COUNT] =
    {
        {
            .index = MTK_V4L2_OUTPUT_PORT_LCD,
            .name = "LCD",
            /* The type field maybe not so important as index. */
            /* To add new type V4L2_OUTPUT_TYPE_LCD may be more sensiable. */
            .type = V4L2_OUTPUT_TYPE_ANALOG,
            .capabilities = 0,
        },
        {
            .index = MTK_V4L2_OUTPUT_PORT_HDMI,
            .name = "HDMI",
            /* The type field maybe not so important as index. */
            /* To add new type V4L2_OUTPUT_TYPE_HDMI may be more sensiable. */
            .type = V4L2_OUTPUT_TYPE_ANALOG,
            .capabilities = V4L2_OUT_CAP_DV_TIMINGS,
        },
        {
            .index = MTK_V4L2_OUTPUT_PORT_CVBS,
            .name = "CVBS",
            .type = V4L2_OUTPUT_TYPE_ANALOG,
            .capabilities = V4L2_OUT_CAP_DV_TIMINGS,
        },
    };

    if (a->index >= sizeof(vop)/sizeof(vop[0]))
    {
        MTKVIDEO_MSG("Invalid output index:%d", a->index);
        return -EINVAL;
    }

    *a = vop[a->index];

    return 0;
}

static int mtkvideo_ioctl_s_output(struct file *file, void *fh, unsigned int i)
{
    int ret = 0;
    enum mtkvideo_v4l2_output_port cvop; /* current video output port */
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;

    /* sanity check */
    if (i >= MTK_V4L2_OUTPUT_PORT_COUNT)
    {
        MTKVIDEO_MSG("Set invalid output index:%d", i);
        return -EINVAL;
    }

    cvop = p->v4l2_param.vop;

    /* Do not set again if i is same to current port. */
    if (cvop == i)
    {
        if (0 == p->resumed) {
            MTKVIDEO_MSG("Set same output:%d", i);
            return 0;
        }
	else {
            p->resumed = 0;
        }
    }

    /* close the current output port if necessary */
    if (MTK_V4L2_OUTPUT_PORT_COUNT != cvop)
    {
        // houlong: temporary not disable external output
        // to do : will disable in 8590 when there is real platform.
#if 1
        if (MTK_V4L2_OUTPUT_PORT_LCD == cvop)
        {
           // do nothing
        }
        else if (MTK_V4L2_OUTPUT_PORT_HDMI == cvop)
        {
            // disable hdmi port
            ret = mtkvideo_adpt_hdmi_func(MTK_HDMI_AUDIO_VIDEO_ENABLE, 0);
        }
        else if (MTK_V4L2_OUTPUT_PORT_CVBS == cvop)
        {
            // disable CVBS port
            ret = mtkvideo_adpt_cvbs_func(CMD_TVE_SET_SUSPEND, 0);
        }
#endif
    }

    /* enable new output port */
    if (MTK_V4L2_OUTPUT_PORT_LCD == i)
    {
        // disable hdmi output
        // houlong: temporary not disable external output
        // to do : will disable in 8590 when there is real platform.
        ret = mtkvideo_adpt_hdmi_func(MTK_HDMI_AUDIO_VIDEO_ENABLE, 0);
        // to do disable CVBS
        ret = mtkvideo_adpt_cvbs_func(CMD_TVE_SET_SUSPEND, 0);

        p->scenario = MTK_V4L2_DISP_LCD;
    }
    else if (MTK_V4L2_OUTPUT_PORT_HDMI == i)
    {
        // enable hdmi port
        ret = mtkvideo_adpt_hdmi_func(MTK_HDMI_AUDIO_VIDEO_ENABLE, 1);
        p->scenario = MTK_V4L2_DISP_LCD_HDMI;
    }
    else if (MTK_V4L2_OUTPUT_PORT_CVBS == i)
    {
        // to do
        // enable CVBS port
        ret = mtkvideo_adpt_cvbs_func(CMD_TVE_SET_RESUME, 1);
        p->scenario = MTK_V4L2_DISP_LCD_CVBS;
    }

    if (ret >= 0)
    {
        p->v4l2_param.vop = i;
    }

    return ret;
}

static int mtkvideo_ioctl_reqbufs(struct file *file, void *fh,
            struct v4l2_requestbuffers *b)
{
    int ret = 0;
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;

    if (b->type != p->v4l2_param.pix.type ||
        (V4L2_BUF_TYPE_VIDEO_OUTPUT != b->type &&
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE != b->type))
    {
        MTKVIDEO_ERROR("Unmatched type:%d, %d", b->type, p->v4l2_param.pix.type);
        return -EINVAL;
    }

    if (V4L2_MEMORY_USERPTR != b->memory)
    {
        MTKVIDEO_ERROR("Unsupport memory:%d", b->memory);
        return -EINVAL;
    }

    p->io_allowed = 1;
    ret = vb2_reqbufs(&p->q.q, b);

    return ret;
}

static int mtkvideo_ioctl_qbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
    int ret = 0;
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;

    if (!p->io_allowed)
    {
        MTKVIDEO_ERROR("No io_allowed");
        return -EACCES;
    }

    if (p->v4l2_param.pix.type != b->type)
    {
        MTKVIDEO_ERROR("Unmatched type:%d, %d", b->type, p->v4l2_param.pix.type);
        return -EINVAL;
    }

    ret = vb2_qbuf(&p->q.q, b);

    return ret;
}

static int mtkvideo_ioctl_dqbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
    int ret = 0;
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;

    if (!p->io_allowed)
    {
        MTKVIDEO_ERROR("No io_allowed");
        return -EACCES;
    }

    if (p->v4l2_param.pix.type != b->type)
    {
        MTKVIDEO_ERROR("Unmatched type:%d, %d", b->type, p->v4l2_param.pix.type);
        return -EINVAL;
    }

    ret = vb2_dqbuf(&p->q.q, b, file->f_flags & O_NONBLOCK);

    if (ret)
    {
        mtkvideo_bh_mdp_process_wakeup();
    }

    return ret;
}

static int mtkvideo_ioctl_streamon(struct file *file, void *fh,
                enum v4l2_buf_type i)
{
    int ret = 0;
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;

    if (!p->io_allowed)
    {
        MTKVIDEO_ERROR("No io_allowed");
        return -EACCES;
    }

    if (p->v4l2_param.pix.type != i)
    {
        MTKVIDEO_ERROR("Unmatched type:%d, %d", i, p->v4l2_param.pix.type);
        return -EINVAL;
    }

    ret = vb2_streamon(&p->q.q, i);

    return ret;
}

static int mtkvideo_ioctl_streamoff(struct file *file, void *fh,
                enum v4l2_buf_type i)
{
    int ret = 0;
    struct mtkvideo_drv_param *p;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;

    if (!p->io_allowed)
    {
        MTKVIDEO_ERROR("No io_allowed");
        return -EACCES;
    }

    if (p->v4l2_param.pix.type != i)
    {
        MTKVIDEO_ERROR("Unmatched type:%d, %d", i, p->v4l2_param.pix.type);
        return -EINVAL;
    }

    ret = vb2_streamoff(&p->q.q, i);

    return ret;
}

static const struct mtkvideo_v4l2_timings_map hdmi_cvbs_res_map[] =
{
    //{V4L2_DV_BT_CEA_640X480P59_94, HDMI_VIDEO_RESOLUTION_NUM, -1}, // 640x480, not support
    {V4L2_DV_BT_CEA_720X480I59_94, HDMI_VIDEO_720x480i_60Hz, TVE_RES_480I}, // 480i, HDMI not support
    {V4L2_DV_BT_CEA_720X480P59_94, HDMI_VIDEO_720x480p_60Hz, TVE_RES_480P}, // 480p
    {V4L2_DV_BT_CEA_720X576I50, HDMI_VIDEO_720x576i_50Hz, TVE_RES_576I},   // 576i, HDMI not support
    {V4L2_DV_BT_CEA_720X576P50, HDMI_VIDEO_720x576p_50Hz, TVE_RES_576P},  // 576p
    //{V4L2_DV_BT_CEA_1280X720P24, HDMI_VIDEO_RESOLUTION_NUM, -1}, // 720p24, not support
    //{V4L2_DV_BT_CEA_1280X720P25, HDMI_VIDEO_RESOLUTION_NUM, -1}, // 720p25, not support
    //{V4L2_DV_BT_CEA_1280X720P30, HDMI_VIDEO_RESOLUTION_NUM, -1}, // 720p30, not support
    {V4L2_DV_BT_CEA_1280X720P50, HDMI_VIDEO_1280x720p_50Hz, -1}, // 720p50
    {V4L2_DV_BT_CEA_1280X720P60, HDMI_VIDEO_1280x720p_60Hz, -1}, // 720p60
    //{??, HDMI_VIDEO_1920x1080p_23Hz, -1}, // v4l2 no definition.
    {V4L2_DV_BT_CEA_1920X1080P24, HDMI_VIDEO_1920x1080p_24Hz, -1}, // 1080p24
    {V4L2_DV_BT_CEA_1920X1080P25, HDMI_VIDEO_1920x1080p_25Hz, -1}, // 1080p25
    {V4L2_DV_BT_CEA_1920X1080P30, HDMI_VIDEO_1920x1080p_30Hz, -1}, // 1080p30
    {V4L2_DV_BT_CEA_1920X1080I50, HDMI_VIDEO_1920x1080i_50Hz, -1}, // 1080i50
    {V4L2_DV_BT_CEA_1920X1080P50, HDMI_VIDEO_1920x1080p_50Hz, -1}, // 1080p50
    {V4L2_DV_BT_CEA_1920X1080I60, HDMI_VIDEO_1920x1080i_60Hz, -1}, // 1080i60
    {V4L2_DV_BT_CEA_1920X1080P60, HDMI_VIDEO_1920x1080p_60Hz, -1}, // 1080p60
};

static int dv_timings_to_hdmi_cvbs_res(struct v4l2_dv_timings *t,
                enum mtkvideo_v4l2_output_port p)
{
    int i;
    int r;
    const int count = sizeof(hdmi_cvbs_res_map)/sizeof(hdmi_cvbs_res_map[0]);

    for (i = 0; i < count; i++)
    {
        if (v4l_match_dv_timings(t, &hdmi_cvbs_res_map[i].t, 0))
        {
            if (MTK_V4L2_OUTPUT_PORT_HDMI == p)
            {
                r = hdmi_cvbs_res_map[i].h;

                if (HDMI_VIDEO_RESOLUTION_NUM != r)
                {
                    MTKVIDEO_MSG("s dv timing %d match hdmi res %d.", i, r);
                }
                else
                {
                    MTKVIDEO_MSG("s dv timing %d match hdmi res %d, but not support", i, r);
                    r = HDMI_VIDEO_720x480p_60Hz; // set 480p as default
                }
            }
            else
            {
                r = hdmi_cvbs_res_map[i].c;

                if (-1 != r)
                {
                    MTKVIDEO_MSG("s dv timing %d match cvbs res %d.", i, r);
                }
                else
                {
                    MTKVIDEO_MSG("s dv timing %d match cvbs res %d, but not support", i, r);
                    r = TVE_RES_480P; // set 480p as default
                }
            }
            break;
        }
    }

    if (count == i)
    {
        MTKVIDEO_MSG("s dv timing not match");
        if (MTK_V4L2_OUTPUT_PORT_HDMI == p)
        {
            i = 1; // set 480p as default
            *t = hdmi_cvbs_res_map[i].t;
            r = hdmi_cvbs_res_map[i].h;
        }
        else
        {
            i = 1; // set 480p as default
            *t = hdmi_cvbs_res_map[i].t;
            r = hdmi_cvbs_res_map[i].c;
        }
    }

    return r;
}

static int mtkvideo_ioctl_s_dv_timings(struct file *file, void *fh,
        struct v4l2_dv_timings *timings)
{
    enum mtkvideo_v4l2_output_port vop;
    struct mtkvideo_drv_param *p;
    int r;

    MTKVIDEO_API_ENTRY();

    p = (struct mtkvideo_drv_param *)fh;
    vop = p->v4l2_param.vop;

    /* Only HDMI and CVBS port support timing switch. */
    if (MTK_V4L2_OUTPUT_PORT_HDMI != vop &&
        MTK_V4L2_OUTPUT_PORT_CVBS != vop)
    {
        return -EINVAL;
    }

    if (timings->type != V4L2_DV_BT_656_1120)
    {
        return -EINVAL;
    }

    if (vop >= MTK_V4L2_OUTPUT_PORT_COUNT)
    {
        MTKVIDEO_MSG("invalid output index:%d, please set out port first", vop);
        return -EINVAL;
    }

    r = dv_timings_to_hdmi_cvbs_res(timings, vop);
    p->v4l2_param.timing.t = *timings;
    p->ovl_s.width = timings->bt.width;
    p->ovl_s.height = timings->bt.height;

    if (MTK_V4L2_OUTPUT_PORT_HDMI == vop)
    {
        mtkvideo_adpt_hdmi_func(MTK_HDMI_VIDEO_CONFIG, r);
    }
    else
    {
		mtkvideo_adpt_cvbs_func(CMD_TVE_SET_FORMAT, r);
    }

    mtkvideo_adpt_notify_res_change(vop, timings->bt.width, timings->bt.height);

    return 0;
}

const struct v4l2_ioctl_ops g_mtkvideo_ioctl_ops = {
    .vidioc_querycap            = mtkvideo_ioctl_querycap,
    .vidioc_s_crop              = mtkvideo_ioctl_s_crop,
    .vidioc_s_ctrl              = mtkvideo_ioctl_s_ctrl,
    // VIDIOC_G_FMT start =>
    // v4l_g_fmt calls different api via v4l2_format::type.
    .vidioc_g_fmt_vid_out_overlay   = mtkvideo_ioctl_g_fmt_vid_out_overlay,
    .vidioc_g_fmt_vid_out       = mtkvideo_ioctl_g_fmt_vid_out,
    .vidioc_g_fmt_vid_out_mplane = mtkvideo_ioctl_g_fmt_vid_out,
    // VIDIOC_G_FMT end <=
    // VIDIOC_S_FMT start =>
    // v4l_s_fmt calls different api via v4l2_format::type.
    .vidioc_s_fmt_vid_out_overlay   = mtkvideo_ioctl_s_fmt_vid_out_overlay,
    .vidioc_s_fmt_vid_out       = mtkvideo_ioctl_s_fmt_vid_out,
    .vidioc_s_fmt_vid_out_mplane = mtkvideo_ioctl_s_fmt_vid_out,
    // VIDIOC_S_FMT end <=
    .vidioc_enum_output         = mtkvideo_ioctl_enum_output, // for reference.
    .vidioc_s_output             = mtkvideo_ioctl_s_output,
    .vidioc_reqbufs             = mtkvideo_ioctl_reqbufs,
    .vidioc_qbuf                = mtkvideo_ioctl_qbuf,
    .vidioc_dqbuf               = mtkvideo_ioctl_dqbuf,
    .vidioc_streamon            = mtkvideo_ioctl_streamon,
    .vidioc_streamoff           = mtkvideo_ioctl_streamoff,
    // .s_effect                ???
    // .g_effect               ???
    //
    .vidioc_s_dv_timings            = mtkvideo_ioctl_s_dv_timings, // houlong
    // expended
    //.vidioc_s_effect
    //.vidioc_g_effect
};

/**********************************************************************
**************** V4L2 File Operations Functions
**********************************************************************/

/*
 * mtkvideo_fops_open: It creates object of file handle structure and stores it in
 * private_data member of filepointer
 */
static int mtkvideo_fops_open(struct file *filep)
{
    int ret = 0;
    struct vb2_queue *q;

    MTKVIDEO_API_ENTRY();

    if (down_interruptible(&mtkvideo_io_mutex))
    {
        MTKVIDEO_ERROR("down error");
        return -EFAULT;
    }

    filep->private_data = (void*)&g_mtkvideo_fh.p;

    if (0 == g_mtkvideo_fh.ref_cnt)
    {
        /* Primary display width/height cannot be initialized in mtkvideo_param_init.
         * Because display driver maybe not initialzied when calling it.
         */
        g_mtkvideo_fh.p.ovl.width = DISP_GetScreenWidth();
        g_mtkvideo_fh.p.ovl.height = DISP_GetScreenHeight();

        g_mtkvideo_fh.p.io_allowed = 0;

        /* Init vb2_queue */
        q = &g_mtkvideo_fh.p.q.q;
        q->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        q->io_modes = VB2_USERPTR;
        q->ops = &g_mtkvideo_qobs;
        #if 1
        q->mem_ops = &g_mtkvideo_vb2_memops;
        #else
        q->mem_ops = &vb2_vmalloc_memops;
        #endif
        q->timestamp_type = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
        q->buf_struct_size = sizeof(struct mtkvideo_vb2_buffer);
        q->drv_priv = (void *)&g_mtkvideo_fh.p.q;
        ret = vb2_queue_init(q);
        spin_lock_init(&g_mtkvideo_fh.p.q.lock);
        INIT_LIST_HEAD(&g_mtkvideo_fh.p.q.list);

        mtkvideo_adpt_sync_state(1);
    }

    if (0 != ret)
    {
        MTKVIDEO_MSG("vb2 queue init failed :0x%x", ret);
    }
    else
    {
        g_mtkvideo_fh.ref_cnt++;
    }

    up(&mtkvideo_io_mutex);

    return ret;
}

/*
 * mtkvideo_fops_release: This function deletes buffer queue, frees the buffers and
 * the file handle
 */
static int mtkvideo_fops_release(struct file *filep)
{
    MTKVIDEO_API_ENTRY();

    if (down_interruptible(&mtkvideo_io_mutex))
    {
        MTKVIDEO_ERROR("down error");
        return -EFAULT;
    }

    g_mtkvideo_fh.ref_cnt--;

    if (0 == g_mtkvideo_fh.ref_cnt)
    {
        mtkvideo_adpt_sync_state(0);
        //list_del_init(struct list_head * entry);
        vb2_queue_release(&g_mtkvideo_fh.p.q.q);
        g_mtkvideo_fh.p.io_allowed = 0;
    }

    up(&mtkvideo_io_mutex);

    return 0;
}

/*
 * mtkvideo_fops_mmap: It is used to map kernel space buffers into user spaces
 * @Houlong: not used if memory type is V4L2_MEMORY_USERPTR.
 */
static int mtkvideo_fops_mmap(struct file *filep, struct vm_area_struct *vma)
{
    int ret = 0;

    MTKVIDEO_API_ENTRY();

    //ret = vb2_mmap(&common->buffer_queue, vma);

    return ret;
}

/*
 * mtkvideo_fops_poll: It is used for select/poll system call
 */
static unsigned int mtkvideo_fops_poll(struct file *filep, poll_table *wait)
{
    int ret = 0;

    MTKVIDEO_API_ENTRY();

    //ret = vb2_poll(&common->buffer_queue, filep, wait);

    return ret;
}

const struct v4l2_file_operations g_mtkvideo_fops = {
    .owner      = THIS_MODULE,
    .open       = mtkvideo_fops_open,
    .release    = mtkvideo_fops_release,
    .unlocked_ioctl = video_ioctl2,
    .mmap       = mtkvideo_fops_mmap,
    //.munmap     = mtkvideo_fops_munmap,
    .poll       = mtkvideo_fops_poll,
};


/**********************************************************************
**************** V4L2 vb2 queue operation Functions
**********************************************************************/
/*
* @queue_setup:    called from VIDIOC_REQBUFS and VIDIOC_CREATE_BUFS
*          handlers before memory allocation, or, if
*          *num_planes != 0, after the allocation to verify a
*          smaller number of buffers. Driver should return
*          the required number of buffers in *num_buffers, the
*          required number of planes per buffer in *num_planes; the
*          size of each plane should be set in the sizes[] array
*          and optional per-plane allocator specific context in the
*          alloc_ctxs[] array. When called from VIDIOC_REQBUFS,
*          fmt == NULL, the driver has to use the currently
*          configured format and *num_buffers is the total number
*          of buffers, that are being allocated. When called from
*          VIDIOC_CREATE_BUFS, fmt != NULL and it describes the
*          target frame format. In this case *num_buffers are being
*          allocated additionally to q->num_buffers.
*/
static int mtkvideo_qops_queue_setup(struct vb2_queue *vq,
                const struct v4l2_format *fmt,
                unsigned int *nbuffers, unsigned int *nplanes,
                unsigned int sizes[], void *alloc_ctxs[])
{
    struct mtkvideo_vb2_queue *q = vb2_get_drv_priv(vq);
    struct mtkvideo_drv_param *p = container_of(q, struct mtkvideo_drv_param, q);
    struct v4l2_format *pix_fmt = &p->v4l2_param.pix;

    MTKVIDEO_API_ENTRY();

    if (V4L2_BUF_TYPE_VIDEO_OUTPUT == pix_fmt->type)
    {
        *nplanes = 1;
    }
    else if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == pix_fmt->type)
    {
        *nplanes = pix_fmt->fmt.pix_mp.num_planes;
    }
    else
    {
        MTKVIDEO_MSG("src fmt type is not set!");
        return -1;
    }

    return 0;
}

/*
* @wait_prepare:   release any locks taken while calling vb2 functions;
*          it is called before an ioctl needs to wait for a new
*          buffer to arrive; required to avoid a deadlock in
*          blocking access type
*/
static void mtkvideo_qops_wait_prepare(struct vb2_queue *vq)
{
    MTKVIDEO_API_ENTRY();
    // do nothing
}

/*
* @wait_finish:    reacquire all locks released in the previous callback;
*          required to continue operation after sleeping while
*          waiting for a new buffer to arrive
*/
static void mtkvideo_qops_wait_finish(struct vb2_queue *vq)
{
    MTKVIDEO_API_ENTRY();
    // do nothing
}

/*
* @buf_init:       called once after allocating a buffer (in MMAP case)
*          or after acquiring a new USERPTR buffer; drivers may
*          perform additional buffer-related initialization;
*          initialization failure (return != 0) will prevent
*          queue setup from completing successfully; optional
*/
static int mtkvideo_qops_buf_init(struct vb2_buffer *vb)
{
    struct mtkvideo_vb2_buffer *buf = container_of(vb,
            struct mtkvideo_vb2_buffer, b);

    MTKVIDEO_API_ENTRY();

    INIT_LIST_HEAD(&buf->list);
    memset(&buf->u_va, 0, sizeof(buf->u_va));
    memset(&buf->k_va, 0, sizeof(buf->k_va));
    memset(&buf->mva, 0, sizeof(buf->mva));
    memset(&buf->length, 0, sizeof(buf->length));
    memset(&buf->handle, 0, sizeof(buf->handle));

    return 0;
}

/*
* @buf_prepare:    called every time the buffer is queued from userspace
*          and from the VIDIOC_PREPARE_BUF ioctl; drivers may
*          perform any initialization required before each hardware
*          operation in this callback; if an error is returned, the
*          buffer will not be queued in driver; optional
*/
static int mtkvideo_qops_buf_prepare(struct vb2_buffer *vb)
{
    struct mtkvideo_vb2_queue *q = vb2_get_drv_priv(vb->vb2_queue);
    struct mtkvideo_vb2_buffer *b = container_of(vb, struct mtkvideo_vb2_buffer, b);
    struct mtkvideo_drv_param *p = container_of(q, struct mtkvideo_drv_param, q);
    struct v4l2_format *pix_fmt = &p->v4l2_param.pix;
    int i, nplanes;

    MTKVIDEO_API_ENTRY();

    if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == pix_fmt->type)
    {
        nplanes = pix_fmt->fmt.pix_mp.num_planes;
    }
    else // if (V4L2_BUF_TYPE_VIDEO_OUTPUT == pix_fmt->type)
    {
        nplanes = 1;
    }

    // map user_va to mva
    if (0 == vb->v4l2_buf.flags)
    {
        for (i = 0; i < nplanes; i++)
        {
            int kernel_size;
            // va
            //b->u_va[i] = vb->v4l2_planes[i].m.userptr;
            b->length[i] = vb->v4l2_planes[i].length;
            b->handle[i] = (struct ion_handle *)NULL;

            if (0 == vb->v4l2_planes[i].m.userptr || 0 >= b->length[i])
            {
                MTKVIDEO_ERROR("NULL pointer or zero length.");
                return -ENOMEM;
            }

#if 1
            // User only passes mva to driver.
            b->mva[i] = vb->v4l2_planes[i].m.userptr;

#if 0 /* MAP MVA to VA is just for debug */
            m4u_do_mva_map_kernel(b->mva[i], b->length[i], 0, &b->k_va[i], &kernel_size);

            if (0 == kernel_size || 0 == b->k_va[i])
            {
                MTKVIDEO_ERROR("map MVA to va failed!");
                b->k_va[i] = 0;
            }
#endif
#else
            if (m4u_alloc_mva(MDP_WDMA,
                        b->user_va[i],
                        b->length[i],
                        0,
                        0,
                        &b->mva[i]))
            {
                MTKVIDEO_ERROR("m4u_alloc_mva fail.");
                return -ENOMEM;
            }

            m4u_dma_cache_maint(MDP_WDMA,
                    (void const *)b->user_va[i],
                    b->length[i],
                    DMA_BIDIRECTIONAL);
#endif
        }
    }
    else if (1 == vb->v4l2_buf.flags)
    {
        // ion

    }

    return 0;
}

/*
* @buf_finish:     called before every dequeue of the buffer back to
*          userspace; drivers may perform any operations required
*          before userspace accesses the buffer; optional
*/
static int mtkvideo_qops_buf_finish(struct vb2_buffer *vb)
{
    struct mtkvideo_vb2_queue *q = vb2_get_drv_priv(vb->vb2_queue);
    struct mtkvideo_vb2_buffer *b = container_of(vb, struct mtkvideo_vb2_buffer, b);
    struct mtkvideo_drv_param *p = container_of(q, struct mtkvideo_drv_param, q);
    struct v4l2_format *pix_fmt = &p->v4l2_param.pix;
    int i, nplanes;

    MTKVIDEO_API_ENTRY();

    if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == pix_fmt->type)
    {
        nplanes = pix_fmt->fmt.pix_mp.num_planes;
    }
    else // if (V4L2_BUF_TYPE_VIDEO_OUTPUT == pix_fmt->type)
    {
        nplanes = 1;
    }

    if (NULL == b->handle)
    {
        // map mva to user_va
        for (i = 0; i < nplanes; i++)
        {
#if 1
            // User only passes mva to driver.
            if (b->k_va[i])
            {
                m4u_do_mva_unmap_kernel(b->mva[i], b->length[i], b->k_va[i]);
            }

#else
            if (b->user_va[i] && b->mva[i])
            {
                m4u_dealloc_mva(MDP_WDMA,
                    b->user_va[i],
                    b->length[i],
                    b->mva[i]);
            }
#endif

            b->k_va[i] = 0;
            b->mva[i] = 0;
        }
    }
    else
    {
        // ion
    }

    return 0;
}

/*
* @buf_cleanup:    called once before the buffer is freed; drivers may
*          perform any additional cleanup; optional
*/
static void mtkvideo_qops_buf_cleanup(struct vb2_buffer *vb)
{
    struct mtkvideo_vb2_queue *q = vb2_get_drv_priv(vb->vb2_queue);
    struct mtkvideo_vb2_buffer *b = container_of(vb, struct mtkvideo_vb2_buffer, b);

    MTKVIDEO_API_ENTRY();

    spin_lock_bh(&q->lock);
    if (VB2_BUF_STATE_ACTIVE == vb->state)
    {
        list_del_init(&b->list);
    }
    spin_unlock_bh(&q->lock);
}

/*
* @start_streaming:    called once to enter 'streaming' state; the driver may
*          receive buffers with @buf_queue callback before
*          @start_streaming is called; the driver gets the number
*          of already queued buffers in count parameter; driver
*          can return an error if hardware fails or not enough
*          buffers has been queued, in such case all buffers that
*          have been already given by the @buf_queue callback are
*          invalidated.
*/
static int mtkvideo_qops_start_streaming(struct vb2_queue *vq, unsigned int count)
{
    struct mtkvideo_vb2_queue *q = vb2_get_drv_priv(vq);

    MTKVIDEO_API_ENTRY();

    // abnormal if empty ???
    spin_lock_bh(&q->lock);
    if (list_empty(&q->list))
    {
        MTKVIDEO_LOG("list is empty when start streaming.");
    }
    spin_unlock_bh(&q->lock);

    return 0;
}

/*
* @stop_streaming: called when 'streaming' state must be disabled; driver
*          should stop any DMA transactions or wait until they
*          finish and give back all buffers it got from buf_queue()
*          callback; may use vb2_wait_for_all_buffers() function
*/
static int mtkvideo_qops_stop_streaming(struct vb2_queue *vq)
{
    struct mtkvideo_vb2_queue *q = vb2_get_drv_priv(vq);
    struct mtkvideo_vb2_buffer *b;

    MTKVIDEO_API_ENTRY();

    if (!vb2_is_streaming(vq))
    {
        // return directly if streaming is not started.
        return 0;
    }

    spin_lock_bh(&q->lock);
    while (!list_empty(&q->list))
    {
        b = list_first_entry(&q->list, struct mtkvideo_vb2_buffer, list);
        list_del(&b->list);
        vb2_buffer_done(&b->b, VB2_BUF_STATE_ERROR);
    }
    spin_unlock_bh(&q->lock);

    /* uninit MDP buffer information */
    // to do

    return 0;
}

/*
* @buf_queue:      passes buffer vb to the driver; driver may start
*          hardware operation on this buffer; driver should give
*          the buffer back by calling vb2_buffer_done() function;
*          it is allways called after calling STREAMON ioctl;
*          might be called before start_streaming callback if user
*          pre-queued buffers before calling STREAMON
*/
static void mtkvideo_qops_buf_queue(struct vb2_buffer *vb)
{
    struct mtkvideo_vb2_queue *q = vb2_get_drv_priv(vb->vb2_queue);
    struct mtkvideo_vb2_buffer *b = container_of(vb, struct mtkvideo_vb2_buffer, b);

    MTKVIDEO_API_ENTRY();

    spin_lock_bh(&q->lock);
    list_add_tail(&b->list, &q->list);
    spin_unlock_bh(&q->lock);

    mtkvideo_bh_mdp_process_wakeup();
}

const struct vb2_ops g_mtkvideo_qobs = {
    .queue_setup     = mtkvideo_qops_queue_setup,
    .wait_prepare    = mtkvideo_qops_wait_prepare,
    .wait_finish     = mtkvideo_qops_wait_finish,
    .buf_init        = mtkvideo_qops_buf_init,
    .buf_prepare     = mtkvideo_qops_buf_prepare,
    .buf_finish      = mtkvideo_qops_buf_finish,
    .buf_cleanup     = mtkvideo_qops_buf_cleanup,
    .start_streaming = mtkvideo_qops_start_streaming,
    .stop_streaming  = mtkvideo_qops_stop_streaming,
    .buf_queue       = mtkvideo_qops_buf_queue,
};

/**********************************************************************
**************** V4L2 vb2 memory handling/memory allocator operations
**********************************************************************/
#if 0
/**
 * struct vb2_mem_ops - memory handling/memory allocator operations
 * @alloc:	allocate video memory and, optionally, allocator private data,
 *		return NULL on failure or a pointer to allocator private,
 *		per-buffer data on success; the returned private structure
 *		will then be passed as buf_priv argument to other ops in this
 *		structure. Additional gfp_flags to use when allocating the
 *		are also passed to this operation. These flags are from the
 *		gfp_flags field of vb2_queue.
 */
void *mtkvideo_vb2_memops_alloc(void *alloc_ctx, unsigned long size, gfp_t gfp_flags)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return NULL;
    }

    return vb2_vmalloc_memops.alloc(alloc_ctx, size, gfp_flags);
}

/*
 * @put:	inform the allocator that the buffer will no longer be used;
 *		usually will result in the allocator freeing the buffer (if
 *		no other users of this buffer are present); the buf_priv
 *		argument is the allocator private per-buffer structure
 *		previously returned from the alloc callback
 */
void mtkvideo_vb2_memops_put(void *buf_priv)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return ;
    }

    vb2_vmalloc_memops.put(buf_priv);
}
#endif

/*
 * @get_userptr: acquire userspace memory for a hardware operation; used for
 *		 USERPTR memory types; vaddr is the address passed to the
 *		 videobuf layer when queuing a video buffer of USERPTR type;
 *		 should return an allocator private per-buffer structure
 *		 associated with the buffer on success, NULL on failure;
 *		 the returned private structure will then be passed as buf_priv
 *		 argument to other ops in this structure
 */
void* mtkvideo_vb2_memops_get_userptr(void *alloc_ctx, unsigned long vaddr,
					unsigned long size, int write)
{
    MTKVIDEO_API_ENTRY();

    if (g_mtkvide_only_support_mva)
    {
        // we only support MVA as input, this api cheats the v4l2-core when its being called.
        return (void *)vaddr;
    }

    return vb2_vmalloc_memops.get_userptr(alloc_ctx, vaddr, size, write);
}

/*
 * @put_userptr: inform the allocator that a USERPTR buffer will no longer
 *		 be used
 */
void mtkvideo_vb2_memops_put_userptr(void *buf_priv)
{
    MTKVIDEO_API_ENTRY();

    if (g_mtkvide_only_support_mva)
    {
	    // we only support MVA as input, this api cheats the v4l2-core when its being called.
        return;
    }

    vb2_vmalloc_memops.put_userptr(buf_priv);
}

#if 0
/*
 * @attach_dmabuf: attach a shared struct dma_buf for a hardware operation;
 *		   used for DMABUF memory types; alloc_ctx is the alloc context
 *		   dbuf is the shared dma_buf; returns NULL on failure;
 *		   allocator private per-buffer structure on success;
 *		   this needs to be used for further accesses to the buffer
 */
void* mtkvideo_vb2_memops_attach_dmabuf(void *alloc_ctx, struct dma_buf *dbuf,
				unsigned long size, int write)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return NULL;
    }

    return vb2_vmalloc_memops.attach_dmabuf(alloc_ctx, dbuf, size, write);
}


/*
 * @detach_dmabuf: inform the exporter of the buffer that the current DMABUF
 *		   buffer is no longer used; the buf_priv argument is the
 *		   allocator private per-buffer structure previously returned
 *		   from the attach_dmabuf callback
 */
void mtkvideo_vb2_memops_detach_dmabuf(void *buf_priv)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return ;
    }

    vb2_vmalloc_memops.detach_dmabuf(buf_priv);
}

/*
 * @map_dmabuf: request for access to the dmabuf from allocator; the allocator
 *		of dmabuf is informed that this driver is going to use the
 *		dmabuf
 */
int mtkvideo_vb2_memops_map_dmabuf(void *buf_priv)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return 0;
    }

    return vb2_vmalloc_memops.map_dmabuf(buf_priv);
}

/*
 * @unmap_dmabuf: releases access control to the dmabuf - allocator is notified
 *		  that this driver is done using the dmabuf for now
 */
void mtkvideo_vb2_memops_unmap_dmabuf(void *buf_priv)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return ;
    }

    vb2_vmalloc_memops.unmap_dmabuf(buf_priv);
}

/*
 * @prepare:	called every time the buffer is passed from userspace to the
 *		driver, useful for cache synchronisation, optional
 */
void mtkvideo_vb2_memops_prepare(void *buf_priv)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return ;
    }

    //vb2_vmalloc_memops.prepare(buf_priv);
}

/*
 * @finish:	called every time the buffer is passed back from the driver
 *		to the userspace, also optional
 */
void mtkvideo_vb2_memops_finish(void *buf_priv)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return ;
    }

    //vb2_vmalloc_memops.finish(buf_priv);
}

/*
 * @vaddr:	return a kernel virtual address to a given memory buffer
 *		associated with the passed private structure or NULL if no
 *		such mapping exists
 */
void* mtkvideo_vb2_memops_vaddr(void *buf_priv)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return NULL;
    }

    return vb2_vmalloc_memops.vaddr(buf_priv);
}

/*
 * @cookie:	return allocator specific cookie for a given memory buffer
 *		associated with the passed private structure or NULL if not
 *		available
 */
void* mtkvideo_vb2_memops_cookie(void *buf_priv)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return NULL;
    }

    //return vb2_vmalloc_memops.cookie(buf_priv);
    return NULL;
}
#endif

/*
 * @num_users:	return the current number of users of a memory buffer;
 *		return 1 if the videobuf layer (or actually the driver using
 *		it) is the only user
 */
unsigned int mtkvideo_vb2_memops_num_users(void *buf_priv)
{
    MTKVIDEO_API_ENTRY();

    if (g_mtkvide_only_support_mva)
    {
        return 0;
    }

    return vb2_vmalloc_memops.num_users(buf_priv);
}

#if 0
/*
 * @mmap:	setup a userspace mapping for a given memory buffer under
 *		the provided virtual memory region
 *
 * Required ops for USERPTR types: get_userptr, put_userptr.
 * Required ops for MMAP types: alloc, put, num_users, mmap.
 * Required ops for read/write access types: alloc, put, num_users, vaddr
 * Required ops for DMABUF types: attach_dmabuf, detach_dmabuf, map_dmabuf,
 *				  unmap_dmabuf.
 */
int mtkvideo_vb2_memops_mmap(void *buf_priv, struct vm_area_struct *vma)
{
    MTKVIDEO_API_ENTRY();

    // this api is not used.

    if (g_mtkvide_only_support_mva)
    {
        return 0;
    }

    return vb2_vmalloc_memops.mmap(buf_priv, vma);
}
#endif

/**
 * struct vb2_mem_ops - memory handling/memory allocator operations
 */
const struct vb2_mem_ops g_mtkvideo_vb2_memops = {
	//.alloc		= mtkvideo_vb2_memops_alloc,
	//.put		= mtkvideo_vb2_memops_put,
	.get_userptr	= mtkvideo_vb2_memops_get_userptr,
	.put_userptr	= mtkvideo_vb2_memops_put_userptr,
	//.map_dmabuf	= mtkvideo_vb2_memops_map_dmabuf,
	//.unmap_dmabuf	= mtkvideo_vb2_memops_unmap_dmabuf,
	//.attach_dmabuf	= mtkvideo_vb2_memops_attach_dmabuf,
	//.detach_dmabuf	= mtkvideo_vb2_memops_detach_dmabuf,
	//.vaddr		= mtkvideo_vb2_memops_vaddr,
	//.mmap		= mtkvideo_vb2_memops_mmap,
	.num_users	= mtkvideo_vb2_memops_num_users,
};

/**********************************************************************
**************** V4L2 Buffer Handler Functions
**********************************************************************/
static int mtkvideo_bh_mdp_process_kthread(void *data)
{
    int ret, widx, next_widx;
    struct semaphore *lock;
    struct mtkvideo_drv_param *p;
    struct mtkvideo_vb2_queue *q;
    struct mtkvideo_vb2_buffer *b;
    struct mtkvideo_mdp_buf_info *bi;
    struct fb_overlay_layer *layer;
    struct v4l2_window *win;
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);

    MTKVIDEO_API_ENTRY();
    MTKVIDEO_ASSERT(data);

    p = (struct mtkvideo_drv_param *)data;
    q = &p->q;
    bi = &p->bi.mdp_bi;

    for (;;)
    {
        wait_event_interruptible(mtkvideo_mdp_process_wq, atomic_read(&mtkvideo_mdp_process_event));
        atomic_set(&mtkvideo_mdp_process_event, 0);

        if (down_interruptible(&mtkvideo_mdp_process_mutex))
        {
            MTKVIDEO_ERROR("down error");
            continue;
        }

        while (!list_empty(&q->list))
        {
            spin_lock_bh(&q->lock);
            b = list_first_entry(&q->list, struct mtkvideo_vb2_buffer, list);
            list_del(&b->list);
            b->b.state = VB2_BUF_STATE_ACTIVE;
            spin_unlock_bh(&q->lock);

            spin_lock_bh(&bi->idx_lock);
            widx = bi->widx;
            if (MTKVIDEO_MDP_BUFFER_NUMBER == widx)
            {
                next_widx = 0; /* will process the 1st buffer */
            }
            else
            {
                next_widx += 1;
                next_widx %= MTKVIDEO_MDP_BUFFER_NUMBER;
            }
            lock = &bi->buf[next_widx].lock;
            spin_unlock_bh(&bi->idx_lock);

            // check buf state
            do
            {
                if (MTKV_BUF_UNUSED != bi->buf[next_widx].state)
                {
                    MTKVIDEO_ERROR("no unused buf, wait OVL process, id=%d", next_widx);
                    mtkvideo_bh_wakeup(p->scenario);
                    msleep(1);
                    continue;
                }
            } while (0);

            if (down_interruptible(lock))
            {
                // error
                //vb2_buffer_done(&b->b, VB2_BUF_STATE_ERROR);
                MTKVIDEO_ERROR("down error");
                break;
            }

            bi->next_widx = next_widx;
            // call MDP process
            ret = mtkvideo_adpt_mdp_process(p, b);
            if (ret)
            {
                MTKVIDEO_ERROR("MDP process error");
                //up(lock);
                //break;
            }

            // fill layer info
            win = &p->v4l2_param.win.fmt.win;
            layer = &bi->buf[next_widx].layer;
            layer->layer_id = 0;
            layer->layer_enable = 1;
            layer->src_base_addr = (void *)bi->buf[next_widx].va;
            layer->src_phy_addr = (void *)bi->buf[next_widx].mva;
            layer->src_direct_link = 0;
            layer->src_fmt = MTK_FB_FORMAT_YUV422;
            layer->src_use_color_key = 0;
            layer->src_color_key = 0;
            layer->src_pitch = win->w.width; // pitch is pixel width with align?
            layer->src_offset_x = 0; // TBD
            layer->src_offset_y = 0; // TBD
            layer->src_width = win->w.width;
            layer->src_height = win->w.height;
            layer->tgt_offset_x = win->w.left;
            layer->tgt_offset_y = win->w.top;
            layer->tgt_width = win->w.width;
            layer->tgt_height = win->w.height;
            layer->layer_rotation = MTK_FB_ORIENTATION_0;
            layer->layer_type = LAYER_2D;
            layer->video_rotation = MTK_FB_ORIENTATION_0;
            layer->isTdshp = 0;
            layer->next_buff_idx = -1;
            layer->identity = 0; // just for debug ???
            layer->connected_type = -1;
            layer->security = LAYER_NORMAL_BUFFER;
            layer->fence_fd = -1;
            layer->ion_fd = -1;
            layer->alpha_enable = 0;
            layer->alpha = 0;

            spin_lock_bh(&bi->idx_lock);
            /* advance write index after MDP processed one buffer. */
            bi->widx = next_widx;
            spin_unlock_bh(&bi->idx_lock);

            bi->buf[bi->widx].state = MTKV_BUF_FILLED;
            up(lock);

            vb2_buffer_done(&b->b, VB2_BUF_STATE_DONE);

            mtkvideo_bh_wakeup(p->scenario);
        }

        up(&mtkvideo_mdp_process_mutex);

        if (kthread_should_stop())
        {
            break;
        }
    }

    mtkvideo_mdp_process_task = NULL;

    return 0;
}

static int mtkvideo_bh_kthread(void *data)
{
    int /*i,*/ ridx, widx, next_ridx/*, ret*/;
    int is_new_video, is_new_ui;
    struct semaphore *mdp_lock;
    struct mtkvideo_drv_param *p;
    struct mtkvideo_mdp_buf_info *mdp_bi;
    struct mtkvideo_ovl_buf_info *ovl_bi;
    spinlock_t *ovl_lock;
    struct mtkvideo_ovl_param *ovl;
    struct mtkvideo_fb_info   *fb;
    enum mtkvideo_v4l2_disp_scenario scenario;
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };

    sched_setscheduler(current, SCHED_RR, &param);

    MTKVIDEO_API_ENTRY();
    MTKVIDEO_ASSERT(data);

    p = (struct mtkvideo_drv_param *)data;
    mdp_bi = &p->bi.mdp_bi;
    ovl_bi = &p->bi.ovl_bi;
    ovl_lock = &p->bi.ovl_bi.lock;
    ovl = &p->ovl;
    fb  = &p->fb;

    for (;;)
    {
        wait_event_interruptible(mtkvideo_bh_wq, atomic_read(&mtkvideo_bh_event));
        atomic_set(&mtkvideo_bh_event, 0);

        if (down_interruptible(&mtkvideo_bh_mutex))
        {
            MTKVIDEO_ERROR("down error");
            continue;
        }

        do
        {
            is_new_video = 0;
            is_new_ui = 0;

            scenario = p->scenario;

            if (MTK_V4L2_DISP_LCD == scenario)
            {
                /**** check if video is necessary to be updated */
                spin_lock_bh(&mdp_bi->idx_lock);
                ridx = mdp_bi->ridx;
                widx = mdp_bi->widx;
                if (MTKVIDEO_MDP_BUFFER_NUMBER == widx)
                {
                    /* there is no video buffer coming */
                    next_ridx = MTKVIDEO_MDP_BUFFER_NUMBER - 1;
                }
                else
                {
                    if (MTKVIDEO_MDP_BUFFER_NUMBER == ridx)
                    {
                        /* will process the 1st buffer */
                        next_ridx = 0;
                        is_new_video = 1;
                    }
                    else if (ridx != widx)
                    {
                        /* advance read index because write index exceeds read index. */
                        next_ridx = ridx + 1;
                        next_ridx %= MTKVIDEO_MDP_BUFFER_NUMBER;
                        is_new_video = 1;
                    }
                    else
                    {
                        /* read index equals write index, do not advance read index. */
                        next_ridx = ridx;
                        //is_new_video = 0;
                    }
                }

                mdp_lock = &mdp_bi->buf[next_ridx].lock;
                spin_unlock_bh(&mdp_bi->idx_lock);

                if (MTKV_BUF_FILLED != mdp_bi->buf[next_ridx].state)
                {
                    MTKVIDEO_MSG("buf[%d] not filled\n", next_ridx);
                    is_new_video = 0; // force not update video layer.
                    ridx = widx; // force break the while loop.
                }

                if (is_new_video)
                {
                    if (down_interruptible(mdp_lock))
                    {
                        MTKVIDEO_ERROR("down error");
                        break;
                    }

                    ovl->layers[0] = mdp_bi->buf[next_ridx].layer;

                    up(mdp_lock);
                }
            }
            else
            {
                // disable video layer
                ovl->layers[0].layer_id = 0;
                ovl->layers[0].layer_enable = 0;
                ridx = widx; // force break the while loop.
            }

            /**** check if UI is necessary to be updated */
            if (down_interruptible(&mtkfb_0_update_mutex))
            {
                MTKVIDEO_ERROR("down error");
            }
            else
            {
                // temporary mark it,  TO DO: need fix!!!!
                if (fb->update != fb->last_update)
                {
                    is_new_ui = 1;
                    fb->last_update = fb->update;

                    ovl->layers[1] = fb->layer;
                }
            }

            up(&mtkfb_0_update_mutex);

#if 1
            if (is_new_video || is_new_ui)
            {
                mtkvideo_adpt_send_buf_to_lcd_ex(ovl);
            }

            if (is_new_video)
            {
                spin_lock_bh(&mdp_bi->idx_lock);
                mdp_bi->buf[next_ridx].state = MTKV_BUF_UNUSED;
                mdp_bi->ridx = next_ridx;
                ridx = next_ridx;
                spin_unlock_bh(&mdp_bi->idx_lock);
            }

            if (is_new_ui)
            {
                mtkvideo_adpt_pan_display_done(0);
            }
#else
            if (is_new_video || is_new_ui)
            {
                /**** Get free dst buffer */
                do
                {
                    spin_lock_bh(ovl_lock);

                    for (i = 0; i < MTKVIDEO_OVL_BUFFER_NUMBER; i++)
                    {
                        if (MTKV_BUF_UNUSED == ovl_bi->buf[i].state)
                        {
                            break;
                        }
                    }

                    if (i < MTKVIDEO_OVL_BUFFER_NUMBER)
                    {
                        ovl_bi->buf[i].state = MTKV_BUF_SELECTED;
                    }

                    spin_unlock_bh(ovl_lock);

                    if (MTKVIDEO_OVL_BUFFER_NUMBER == i)
                    {
                        MTKVIDEO_MSG("Get free out buf failed.");
                        msleep(1);
                        continue;
                    }
                } while(0);

                ovl->out.enable = 1;
                ovl->out.dirty = 1;
                ovl->out.outFormat = MTK_FB_FORMAT_RGB888;
                ovl->out.dstAddr = ovl_bi->buf[i].mva;
                ovl->out.srcROI.x = 0;
                ovl->out.srcROI.y = 0;
                ovl->out.srcROI.width = ovl->width;
                ovl->out.srcROI.height = ovl->height;
                ovl->out.security = LAYER_NORMAL_BUFFER;
                ovl->out.ion_fd = -1;

                /**** Compose video & ui layers */
                ret = mtkvideo_adpt_compose(ovl);

                spin_lock_bh(ovl_lock);
                ovl_bi->buf[i].state = MTKV_BUF_FILLED;
                spin_unlock_bh(ovl_lock);

                if (is_new_video)
                {
                    spin_lock_bh(&mdp_bi->idx_lock);
                    mdp_bi->buf[next_ridx].state = MTKV_BUF_UNUSED;
                    mdp_bi->ridx = next_ridx;
                    ridx = next_ridx;
                    spin_unlock_bh(&mdp_bi->idx_lock);
                }

                // temporary mark it,  TO DO: need fix!!!!
                if (is_new_ui)
                {
                    mtkvideo_adpt_pan_display_done(0);
                }

                /**** Queue composed buffer to primary display path */
                if (MTK_V4L2_DISP_LCD == scenario)
                {
                    ret = mtkvideo_adpt_send_buf_to_lcd(ovl);
                    if (ret < 0)
                    {
                        MTKVIDEO_ERROR("send buf[%d] to lcd failed!!!", i);
                    }
                }
            }
#endif
            /* break if read index equal write index */
        } while (ridx != widx);

        up(&mtkvideo_bh_mutex);

        if (kthread_should_stop())
        {
            break;
        }
    }

    mtkvideo_bh_task = NULL;

    return 0;
}

static int mtkvideo_bh_sub_kthread(void *data)
{
    int i, ridx, widx, next_ridx, ret;
    int is_new_video, is_new_ui;
    struct semaphore *mdp_lock;
    struct mtkvideo_drv_param *p;
    struct mtkvideo_mdp_buf_info *mdp_bi;
    struct mtkvideo_ovl_buf_info *ovl_bi;
    spinlock_t *ovl_lock;
    struct mtkvideo_ovl_param *ovl;
    struct mtkvideo_fb_info   *fb;
    enum mtkvideo_v4l2_disp_scenario scenario;
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };

    sched_setscheduler(current, SCHED_RR, &param);

    MTKVIDEO_API_ENTRY();
    MTKVIDEO_ASSERT(data);

    p = (struct mtkvideo_drv_param *)data;
    mdp_bi = &p->bi.mdp_bi;
    ovl_bi = &p->bi.ovl_bi;
    ovl_lock = &p->bi.ovl_bi.lock;
    ovl = &p->ovl_s;
    fb  = &p->fb_s;

    for (;;)
    {
        wait_event_interruptible(mtkvideo_bh_sub_wq, atomic_read(&mtkvideo_bh_sub_event));
        atomic_set(&mtkvideo_bh_sub_event, 0);

        if (down_interruptible(&mtkvideo_bh_sub_mutex))
        {
            MTKVIDEO_ERROR("down error");
            continue;
        }

        do
        {
            is_new_video = 0;
            is_new_ui = 0;

            scenario = p->scenario;

            if (MTK_V4L2_DISP_LCD != scenario)
            {
                /**** check if video is necessary to be updated */
                spin_lock_bh(&mdp_bi->idx_lock);
                ridx = mdp_bi->ridx;
                widx = mdp_bi->widx;
                if (MTKVIDEO_MDP_BUFFER_NUMBER == widx)
                {
                    /* there is no video buffer coming */
                    next_ridx = MTKVIDEO_MDP_BUFFER_NUMBER - 1;
                }
                else
                {
                    if (MTKVIDEO_MDP_BUFFER_NUMBER == ridx)
                    {
                        /* will process the 1st buffer */
                        next_ridx = 0;
                        is_new_video = 1;
                    }
                    else if (ridx != widx)
                    {
                        /* advance read index because write index exceeds read index. */
                        next_ridx = ridx + 1;
                        next_ridx %= MTKVIDEO_MDP_BUFFER_NUMBER;
                        is_new_video = 1;
                    }
                    else
                    {
                        /* read index equals write index, do not advance read index. */
                        next_ridx = ridx;
                        //is_new_video = 0;
                    }
                }

                mdp_lock = &mdp_bi->buf[next_ridx].lock;
                spin_unlock_bh(&mdp_bi->idx_lock);

                if (MTKV_BUF_FILLED != mdp_bi->buf[next_ridx].state)
                {
                    MTKVIDEO_MSG("buf[%d] not filled\n", next_ridx);
                    is_new_video = 0; // force not update video layer.
                    ridx = widx; // force break the while loop.
                }

                if (is_new_video)
                {
                    if (down_interruptible(mdp_lock))
                    {
                        MTKVIDEO_ERROR("down error");
                        break;
                    }

                    ovl->layers[0] = mdp_bi->buf[next_ridx].layer;

                    up(mdp_lock);
                }
            }
            else
            {
                // disable video layer
                ovl->layers[0].layer_id = 0;
                ovl->layers[0].layer_enable = 0;
                ridx = widx; // force break the while loop.
            }

            /**** check if UI is necessary to be updated */
            if (down_interruptible(&mtkfb_1_update_mutex))
            {
                MTKVIDEO_ERROR("down error");
            }
            else
            {
                // temporary mark it,  TO DO: need fix!!!!
                if (fb->update != fb->last_update)
                {
                    is_new_ui = 1;
                    fb->last_update = fb->update;

                    ovl->layers[1] = fb->layer;
                }
            }

            up(&mtkfb_1_update_mutex);

            if (is_new_video || is_new_ui)
            {
                /**** Get free dst buffer */
                do
                {
                    spin_lock_bh(ovl_lock);

                    for (i = 0; i < MTKVIDEO_OVL_BUFFER_NUMBER; i++)
                    {
                        if (MTKV_BUF_UNUSED == ovl_bi->buf[i].state)
                        {
                            break;
                        }
                    }

                    if (i < MTKVIDEO_OVL_BUFFER_NUMBER)
                    {
                        ovl_bi->buf[i].state = MTKV_BUF_SELECTED;
                    }

                    spin_unlock_bh(ovl_lock);

                    if (MTKVIDEO_OVL_BUFFER_NUMBER == i)
                    {
                        MTKVIDEO_MSG("Get free out buf failed.");
                        msleep(1);
                        continue;
                    }
                } while(0);

                ovl->out.enable = 1;
                ovl->out.dirty = 1;

                if (MTK_V4L2_DISP_LCD_HDMI == scenario) {
                    ovl->out.outFormat = MTK_FB_FORMAT_RGB888; // HDMI only receive RGB888 format by SW.
                }
                else //if (MTK_V4L2_DISP_LCD_CVBS == scenario)
                {
					#if MTK_CVBS_FORMAT_YUV
                    ovl->out.outFormat = MTK_FB_FORMAT_YUV422;
					#else
                    ovl->out.outFormat = MTK_FB_FORMAT_RGB888;
					#endif
                }

                if (i >= MTKVIDEO_OVL_BUFFER_NUMBER) {
			/* to fix klockwork issue */
			i = 0;
                }

                ovl->out.dstAddr = ovl_bi->buf[i].mva;
                ovl->out.srcROI.x = 0;
                ovl->out.srcROI.y = 0;
                ovl->out.srcROI.width = ovl->width;
                ovl->out.srcROI.height = ovl->height;
                ovl->out.security = LAYER_NORMAL_BUFFER;
                ovl->out.ion_fd = -1;

                /**** Compose video & ui layers */
                ret = mtkvideo_adpt_compose(ovl);

                spin_lock_bh(ovl_lock);
                ovl_bi->buf[i].state = MTKV_BUF_FILLED;
                spin_unlock_bh(ovl_lock);

                if (is_new_video)
                {
                    spin_lock_bh(&mdp_bi->idx_lock);
                    mdp_bi->buf[next_ridx].state = MTKV_BUF_UNUSED;
                    mdp_bi->ridx = next_ridx;
                    ridx = next_ridx;
                    spin_unlock_bh(&mdp_bi->idx_lock);
                }

                // temporary mark it,  TO DO: need fix!!!!
                if (is_new_ui)
                {
                    mtkvideo_adpt_pan_display_done(1);
                }

                /**** Queue composed buffer to secondary display path */
                if (MTK_V4L2_DISP_LCD_HDMI == scenario)
                {
                    ret = mtkvideo_adpt_send_buf_to_hdmi(ovl);
                    if (ret < 0)
                    {
                        MTKVIDEO_ERROR("send buf[%d] to hdmi failed!!!", i);
                    }
                }
                else //if (MTK_V4L2_DISP_LCD_CVBS == scenario)
                {
                    ret = mtkvideo_adpt_send_buf_to_cvbs(ovl);
                    if (ret < 0)
                    {
                        MTKVIDEO_ERROR("send buf[%d] to hdmi failed!!!", i);
                    }
                }
            }

            /* break if read index equal write index */
        } while (ridx != widx);

        up(&mtkvideo_bh_sub_mutex);

        if (kthread_should_stop())
        {
            break;
        }
    }

    mtkvideo_bh_sub_task = NULL;

    return 0;
}


int mtkvideo_bh_init(struct mtkvideo_drv_param *p)
{
    MTKVIDEO_API_ENTRY();

    if (NULL == mtkvideo_bh_task)
    {
        sema_init(&mtkvideo_bh_mutex, 1);

        init_waitqueue_head(&mtkvideo_bh_wq);
        atomic_set(&mtkvideo_bh_event, 0);

        mtkvideo_bh_task = kthread_create(mtkvideo_bh_kthread, (void *)p, "mtkv_bh");
        wake_up_process(mtkvideo_bh_task);
    }

    if (NULL == mtkvideo_bh_sub_task)
    {
        sema_init(&mtkvideo_bh_sub_mutex, 1);

        init_waitqueue_head(&mtkvideo_bh_sub_wq);
        atomic_set(&mtkvideo_bh_sub_event, 0);

        mtkvideo_bh_sub_task = kthread_create(mtkvideo_bh_sub_kthread, (void *)p, "mtkv_bh_s");
        wake_up_process(mtkvideo_bh_sub_task);
    }

    if (NULL == mtkvideo_mdp_process_task)
    {
        sema_init(&mtkvideo_mdp_process_mutex, 1);
        init_waitqueue_head(&mtkvideo_mdp_process_wq);
        atomic_set(&mtkvideo_mdp_process_event, 0);

        mtkvideo_mdp_process_task = kthread_create(mtkvideo_bh_mdp_process_kthread, (void *)p, "mtkv_mdp");
        wake_up_process(mtkvideo_mdp_process_task);
    }

    return 0;
}

int mtkvideo_bh_uninit(void)
{
    MTKVIDEO_API_ENTRY();

    // stop the thread
    if (mtkvideo_bh_task)
    {
        kthread_stop(mtkvideo_bh_task);
        mtkvideo_bh_task = NULL;
    }

    if (mtkvideo_bh_sub_task)
    {
        kthread_stop(mtkvideo_bh_sub_task);
        mtkvideo_bh_sub_task = NULL;
    }

    if (mtkvideo_mdp_process_task)
    {
        kthread_stop(mtkvideo_mdp_process_task);
        mtkvideo_mdp_process_task = NULL;
    }

    return 0;
}

void mtkvideo_bh_fb_0_update(unsigned long long t)
{
    // MTKVIDEO_API_ENTRY();
    if (t > 0)
    {
        g_mtkvideo_fh.p.fb.update = t;
    }

    atomic_set(&mtkvideo_bh_event, 1);
    wake_up_interruptible(&mtkvideo_bh_wq);
}

void mtkvideo_bh_fb_1_update(unsigned long long t)
{
    // MTKVIDEO_API_ENTRY();
    if (t > 0)
    {
        g_mtkvideo_fh.p.fb_s.update = t;
    }

    atomic_set(&mtkvideo_bh_sub_event, 1);
    wake_up_interruptible(&mtkvideo_bh_sub_wq);
}

void mtkvideo_bh_wakeup(int disp_scenario)
{
    // MTKVIDEO_API_ENTRY();
    if (MTK_V4L2_DISP_LCD == disp_scenario)
    {
        mtkvideo_bh_fb_0_update(0LL);
    }
    else
    {
        mtkvideo_bh_fb_1_update(0LL);
    }
}

void mtkvideo_bh_mdp_process_wakeup(void)
{
    // MTKVIDEO_API_ENTRY();
    atomic_set(&mtkvideo_mdp_process_event, 1);
    wake_up_interruptible(&mtkvideo_mdp_process_wq);
}

