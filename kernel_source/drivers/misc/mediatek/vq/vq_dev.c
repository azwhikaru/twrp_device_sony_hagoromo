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

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <generated/autoconf.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/param.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <mach/mt_smi.h>

#include <linux/xlog.h>
#include <linux/proc_fs.h>  /* proc file use */
/* ION */
/* #include <linux/ion.h> */
#include <linux/ion_drv.h>
/* #include <mach/m4u.h> */

#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>

#include <linux/io.h>


#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_irq.h>
#include <mach/irqs.h>
#include <mach/mt_clkmgr.h> /* ???? */
#include <mach/mt_irq.h>
#include <mach/sync_write.h>

#include "debug.h"

#include "vq_dev.h"
#include "vq_def.h"
#include "vq_dbg.h"

/*********************************** define ***********************************/

#define VQ_DEVNAME "mtk_vq"

/*********************************** variable *********************************/

static dev_t vq_devno;
static struct cdev *vq_cdev;
static struct class *vq_class;

/*********************************** function *********************************/

static long vq_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned int ret = 0;

	VQ_Printf(VQ_LOG_IOCTL, "[IOCTL] cmd=0x%x, arg=0x%x\n", cmd, (unsigned int)arg);

	switch (cmd) {
	case VQ_IOCTL_POWER_SWITCH:
	{
		int iPowerSwitch = 0;

		if (copy_from_user(&iPowerSwitch, (void *)arg, sizeof(int))) {

			VQ_Printf(VQ_LOG_ERROR, "[E] invalid param in VQ_IOCTL_POWER_SWITCH when copy_from_user\n");
			return VQ_RET_ERR_PARAM;
		}

		VQ_Printf(VQ_LOG_IOCTL, "[IOCTL] set param for power switch\n");

		ret = iVQ_PowerSwitch(iPowerSwitch);

		break;
	}

	case VQ_IOCTL_PROCESS:
	{
		struct VQ_PARAM_T	rVqParam;

		if (copy_from_user(&rVqParam, (void *)arg, sizeof(struct VQ_PARAM_T))) {

			VQ_Printf(VQ_LOG_ERROR, "[E] invalid param in VQ_IOCTL_PROCESS when copy_from_user\n");
			return VQ_RET_ERR_PARAM;
		}

		VQ_Printf(VQ_LOG_IOCTL, "[IOCTL] set param for process\n");

		#if VQ_TIME_CHECK
		{
			UINT32 u4Idx = 0;
			for (u4Idx = 0; u4Idx < VQ_TIME_CHECK_COUNT; u4Idx++) {

				/*for keep brace*/
				_au4VqTimeRec[u4Idx] = 0;
			}
		}

		VQ_TIME_REC(0);
		#endif

		ret = iVQ_Process(&rVqParam);

		#if VQ_TIME_CHECK
		VQ_TIME_REC(9);

		#if VQ_WAIT_IRQ
		VQ_Printf(VQ_LOG_TIME, "[T] [%08d], [1-%08d, %08d, 3-%08d, %08d, 5-%08d, %08d, 7-%08d, %08d, 9-%08d]\n",
			(_au4VqTimeRec[9] - _au4VqTimeRec[0]),
			(_au4VqTimeRec[1] - _au4VqTimeRec[0]),
			(_au4VqTimeRec[2] - _au4VqTimeRec[1]),
			(_au4VqTimeRec[3] - _au4VqTimeRec[2]),
			(_au4VqTimeRec[4] - _au4VqTimeRec[3]),
			(_au4VqTimeRec[5] - _au4VqTimeRec[4]),
			(_au4VqTimeRec[6] - _au4VqTimeRec[5]),
			(_au4VqTimeRec[7] - _au4VqTimeRec[6]),
			(_au4VqTimeRec[8] - _au4VqTimeRec[7]),
			(_au4VqTimeRec[9] - _au4VqTimeRec[8]));
		#else
		VQ_Printf(VQ_LOG_TIME, "[T] [%08d], [1-%08d, %08d, 3-%08d, %08d, 5-%08d, %08d, 7-%08d, %08d, 9-%08d]\n",
			(_au4VqTimeRec[9] - _au4VqTimeRec[0]),
			(_au4VqTimeRec[1] - _au4VqTimeRec[0]),
			(_au4VqTimeRec[2] - _au4VqTimeRec[1]),
			(_au4VqTimeRec[3] - _au4VqTimeRec[2]),
			(_au4VqTimeRec[4] - _au4VqTimeRec[3]),
			(_au4VqTimeRec[5] - _au4VqTimeRec[4]),
			(_au4VqTimeRec[6] - _au4VqTimeRec[5]),
			(_au4VqTimeRec[7] - _au4VqTimeRec[6]),
			(_au4VqTimeRec[8] - _au4VqTimeRec[7]),
			(_au4VqTimeRec[9] - _au4VqTimeRec[8]));
		#endif

		#endif

		break;
	}

	default:
		VQ_Printf(VQ_LOG_ERROR, "[E] no such command %d\n", cmd);
		break;
	}

	return ret;
}

static int vq_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t vq_read(struct file *file, char __user *data, size_t len, loff_t *ppos)
{
	return 0;
}

static int vq_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int vq_flush(struct file *file , fl_owner_t a_id)
{
	return 0;
}

/* remap register to user space */
static int vq_mmap(struct file *file, struct vm_area_struct *a_pstVMArea)
{
	a_pstVMArea->vm_page_prot = pgprot_noncached(a_pstVMArea->vm_page_prot);

	if (remap_pfn_range(
		a_pstVMArea,
		a_pstVMArea->vm_start,
		a_pstVMArea->vm_pgoff,
		(a_pstVMArea->vm_end - a_pstVMArea->vm_start),
		a_pstVMArea->vm_page_prot)) {

		VQ_Printf(VQ_LOG_ERROR, "[E] MMAP failed!!\n");
		return -1;
	}

	return 0;
}

/* Kernel interface */
static const struct file_operations vq_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= vq_unlocked_ioctl,
	.open		= vq_open,
	.release	= vq_release,
	.flush		= vq_flush,
	.read		= vq_read,
	.mmap		= vq_mmap
};

static int vq_probe(struct platform_device *pdev)
{
	struct class_device;

	int ret;
	struct class_device *class_dev = NULL;

	VQ_Printf(VQ_LOG_DEFAULT, "\n [D] vq driver probe...\n\n");
	ret = alloc_chrdev_region(&vq_devno, 0, 1, VQ_DEVNAME);

	if (ret) {

		/*for keep brace*/
		VQ_Printf(VQ_LOG_ERROR, "[E] Can't Get Major number for VQ Device\n");

	} else {

		VQ_Printf(VQ_LOG_DEFAULT, "[D] Get VQ Device Major number (%d)\n", vq_devno);
	}

	vq_cdev = cdev_alloc();
	vq_cdev->owner = THIS_MODULE;
	vq_cdev->ops = &vq_fops;

	ret = cdev_add(vq_cdev, vq_devno, 1);

	vq_class = class_create(THIS_MODULE, VQ_DEVNAME);
	class_dev = (struct class_device *)device_create(vq_class, NULL, vq_devno, NULL, VQ_DEVNAME);

	/* Register IRQ */

	VQ_Printf(VQ_LOG_DEFAULT, "[D] VQ Probe Done\n");
	NOT_REFERENCED(class_dev);
	return 0;
}

static int vq_remove(struct platform_device *pdev)
{
	/* disable IRQ */

	return 0;
}

static void vq_shutdown(struct platform_device *pdev)
{
	/* Nothing yet */
}

static int vq_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	VQ_Printf(VQ_LOG_DEFAULT, "[D] vq suspend start\n");

	return VQ_RET_OK;
}

static int vq_resume(struct platform_device *pdev)
{
	VQ_Printf(VQ_LOG_DEFAULT, "[D] vq resume start\n");

	return VQ_RET_OK;
}

static struct platform_driver vq_driver = {
	.probe		= vq_probe,
	.remove		= vq_remove,
	.shutdown	= vq_shutdown,
	.suspend	= vq_suspend,
	.resume		= vq_resume,
	.driver		= {
		.name = VQ_DEVNAME,
		/* .pm   = &vq_pm_ops, */
	},
};

static struct platform_device vq_device = {
	.name = "mtk_vq",
	.id   = 0,
};

static int __init vq_init(void)
{
	int ret;

	VQ_Printf(VQ_LOG_DEFAULT, "[D] VQ init start\n");

	ret = platform_device_register(&vq_device);
	if (ret) {

		/*for keep brace*/
		VQ_Printf(VQ_LOG_ERROR, "[E] [%s] return 0x%08X (%d)\n", __func__, ret, ret);
	}

	if (platform_driver_register(&vq_driver)) {

		VQ_Printf(VQ_LOG_ERROR, "[E] failed to register vq driver\n");
		ret = -ENODEV;
		return ret;
	}

	vq_debug_init();

	VQ_Printf(VQ_LOG_DEFAULT, "[D] VQ init end\n");

	return 0;
}

static void __exit vq_exit(void)
{
	VQ_Printf(VQ_LOG_DEFAULT, "[D] VQ exit start\n");

	cdev_del(vq_cdev);
	unregister_chrdev_region(vq_devno, 1);

	vq_debug_exit();

	platform_driver_unregister(&vq_driver);
	platform_device_unregister(&vq_device);

	device_destroy(vq_class, vq_devno);
	class_destroy(vq_class);

	VQ_Printf(VQ_LOG_DEFAULT, "[D] VQ exit end\n");
}

late_initcall(vq_init);
module_exit(vq_exit);

