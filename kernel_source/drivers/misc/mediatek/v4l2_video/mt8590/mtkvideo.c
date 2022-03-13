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
 * mtkvideo.c - V4L2 display driver implementation file.
 *
 */

#include "mtkvideo.h"

/**********************************************************************
******************* Global Variable Definitions
**********************************************************************/
struct mtkvideo_fh g_mtkvideo_fh;
static struct v4l2_device g_mtkvideo_v4l2_dev;
static struct video_device g_mtkvideo_device = {
    .name       = "mtkvideo_display_driver",
    .v4l2_dev   = &g_mtkvideo_v4l2_dev,
    .fops       = &g_mtkvideo_fops,
    .ioctl_ops  = &g_mtkvideo_ioctl_ops,
};

/**********************************************************************
******************* Function Definitions
**********************************************************************/

/* This API should be called after v4l2_device_register in probe function */
static void mtkvideo_param_init(struct mtkvideo_drv_param *p)
{
    struct v4l2_capability *c;

    MTKVIDEO_API_ENTRY();

    memset(p, 0, sizeof(struct mtkvideo_drv_param));
    /* Capabilities */
    c = &p->v4l2_param.cap;
    snprintf(c->driver, sizeof(c->driver), "%s", dev_name(g_mtkvideo_v4l2_dev.dev));
    // pCap->card is empty.
    snprintf(c->bus_info, sizeof(c->bus_info), "platform %s", dev_name(g_mtkvideo_v4l2_dev.dev));
    c->version = LINUX_VERSION_CODE;
    c->device_caps = V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_VIDEO_OVERLAY | V4L2_CAP_VIDEO_OUTPUT_OVERLAY | V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_STREAMING;
    c->capabilities = c->device_caps | V4L2_CAP_DEVICE_CAPS;

    p->v4l2_param.vop = MTK_V4L2_OUTPUT_PORT_COUNT;
    p->pVideoDevice = &g_mtkvideo_device;
    p->scenario = MTK_V4L2_DISP_LCD;
    p->resumed = 0;

    // alloc buffer
    mtkvideo_adpt_buf_init(&p->bi);
    // OVL param init
    mtkvideo_adpt_ovl_init(&p->ovl);
    mtkvideo_adpt_ovl_init(&p->ovl_s);
}

/* This API should be called in remove function */
static void mtkvideo_param_uninit(struct mtkvideo_drv_param *p)
{
    MTKVIDEO_API_ENTRY();

    mtkvideo_adpt_buf_uninit(&p->bi);
    mtkvideo_adpt_ovl_uninit(&p->ovl);
    mtkvideo_adpt_ovl_uninit(&p->ovl_s);
}

/*
 * mtkvideo_probe: This function creates device entries by register itself to the
 * V4L2 driver and initializes other variables.
 */
static __init int mtkvideo_probe(struct platform_device *pdev)
{
    static struct v4l2_device *v4l2_dev = &g_mtkvideo_v4l2_dev;
    static struct video_device *vfd = &g_mtkvideo_device;
    int err = 0;

    MTKVIDEO_API_ENTRY();

    /* Initialize field of vl2d device */
    memset(v4l2_dev, 0, sizeof(*v4l2_dev));
    //snprintf(v4l2_dev->name, sizeof(v4l2_dev->name), "%s", "mtk_v4l2_video");

    /* register v4l2 device */
    err = v4l2_device_register(&pdev->dev, v4l2_dev);
    if (err) {
        MTKVIDEO_MSG("Error registering v4l2 device:0x%x", err);
        return err;
    }

    /* Initialize field of video device */
    vfd->v4l2_dev = v4l2_dev;
    vfd->release = video_device_release_empty;
    vfd->vfl_dir = VFL_DIR_TX;
    snprintf(vfd->name, sizeof(vfd->name), "MTKVIDEO_Display_DRIVER_V%d", 0);

    /* register video device */
    err = video_register_device(vfd, VFL_TYPE_GRABBER, 0);
    if (err < 0)
    {
        v4l2_device_unregister(v4l2_dev);
    }

    /* Initialize driver parameters */
    g_mtkvideo_fh.ref_cnt = 0;
    mtkvideo_param_init(&g_mtkvideo_fh.p);
    /* Initialize display adapter layer */
    mtkvideo_adpt_disp_init();
    /* Initialize buffer handler */
    mtkvideo_bh_init(&g_mtkvideo_fh.p);

    return err;
}

/*
 * mtkvideo_remove: It un-registers V4L2 device.
 */
static int mtkvideo_remove(struct platform_device *device)
{
    MTKVIDEO_API_ENTRY();

    /* Uninitialize buffer handler */
    mtkvideo_bh_uninit();

    /* Uninitialize display adapter layer */
    mtkvideo_adpt_disp_uninit();

    /* Uninitialize driver parameters */
    mtkvideo_param_uninit(&g_mtkvideo_fh.p);

    /* */
    v4l2_device_unregister(&g_mtkvideo_v4l2_dev);
    /* Unregister video device */
    video_unregister_device(&g_mtkvideo_device);

    return 0;
}

#ifdef CONFIG_PM
static int mtkvideo_pm_suspend(struct device *dev)
{
    MTKVIDEO_API_ENTRY();

    return 0;
}

static int mtkvideo_pm_resume(struct device *dev)
{
    MTKVIDEO_API_ENTRY();

    g_mtkvideo_fh.p.resumed = 1;

    return 0;
}

static const struct dev_pm_ops g_mtkvideo_pm = {
    .suspend        = mtkvideo_pm_suspend,
    .resume         = mtkvideo_pm_resume,
};

#define mtkvideo_pm_ops (&g_mtkvideo_pm)
#else
#define mtkvideo_pm_ops NULL
#endif

static struct platform_driver g_mtkvideo_platform_driver = {
    .driver = {
            .name   = "mtk_v4l2_display", //MTKVIDEO_DRIVER,
            .owner  = THIS_MODULE,
            .pm = mtkvideo_pm_ops,
    },
    .probe  = mtkvideo_probe,
    .remove = mtkvideo_remove,
};

static struct platform_device g_mtkvideo_platform_device = {
    .name = "mtk_v4l2_display",
    .id   = 0,
};

static __init int mtkvideo_init(void)
{
    int ret;

    MTKVIDEO_API_ENTRY();

    ret = platform_device_register(&g_mtkvideo_platform_device);

    if (ret)
    {
        printk("[%s] return 0x%08X (%d)\n", __FUNCTION__, ret, ret);
    }

    ret = platform_driver_register(&g_mtkvideo_platform_driver);

    printk("[%s] return 0x%08X (%d)\n", __FUNCTION__, ret, ret);

    return ret;
}

/*
 * mtkvideo_cleanup: This function un-registers device and driver to the kernel,
 * frees requested irq handler and de-allocates memory allocated for channel
 * objects.
 */
static void mtkvideo_cleanup(void)
{
    MTKVIDEO_API_ENTRY();

    platform_driver_unregister(&g_mtkvideo_platform_driver);
    platform_device_unregister(&g_mtkvideo_platform_device);
}


late_initcall(mtkvideo_init);
module_exit(mtkvideo_cleanup);

MODULE_DESCRIPTION("MEDIATEK v4l2 video driver");
MODULE_AUTHOR("Houlong Wei <houlong.wei@mediatek.com>");
MODULE_LICENSE("GPL");
