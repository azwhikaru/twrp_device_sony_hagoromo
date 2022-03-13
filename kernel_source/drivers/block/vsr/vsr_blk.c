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
#include <asm/system.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/workqueue.h>
#include <linux/err.h>

#include "vsr_common.h"
#include "vsr_data_xfer.h"
#include "vsr_interface.h"
#include "vsr_blk.h"
#ifdef	UNIT_TEST
#include "vsr_unit_test.h"
#endif

static atomic_t open_flag = ATOMIC_INIT(1);
static struct vsr_dev vdev;

struct vsr_dev *get_vsr_dev(void)
{
	return &vdev;
}

static int vsr_open(struct block_device *dev, fmode_t mode)
{
	if (!atomic_dec_and_test(&open_flag))
	{
		atomic_inc(&open_flag);
		VSR_WAN("Can not open vsr twice!");
		return -EBUSY;
	}

	VSR_DBG("Open vsr success!");
	return 0;
}
static void vsr_release(struct gendisk *dev, fmode_t mode)
{
	atomic_inc(&open_flag);
	VSR_DBG("Release vsr!");
	return 0;
}

static int vsr_ioctl(struct block_device *dev, fmode_t mode, unsigned cmd, unsigned long arg)
{
	switch (cmd){
	case OPEN_USB_FOR_VSR:
		VSR_DBG("Open USB success!");
		set_read_call_back_func();
		return open_xfer_intf();
		break;
	case CLOSE_USB_FOR_VSR:
		VSR_DBG("Close USB");
		clr_read_call_back_func();
		return close_xfer_intf();
		break;
	case SET_VSR_LOG_EN:
		VSR_DBG("log enable...");
		VSR_DBG("Input arg is: 0x%08x", (u32)arg);
		set_vsr_log_en((u32)arg);
		break;
	case GET_VSR_LOG_EN:
		VSR_DBG("get log enable info...");
		return (int)get_vsr_log_en();
		break;
	default:
		break;
	}

	return 0;
}

static struct block_device_operations vsr_fops = 
{
	.owner		= THIS_MODULE,
	.open		= vsr_open,
	.release 	= vsr_release,
	.ioctl		= vsr_ioctl,
};

static int vsr_transfer(struct vsr_dev *dev, u32 sector, u32 nsect, char *buf, int is_write)
{
	VSR_DBG("(%u/%u) buffer:%p [%s]\n",
			 sector, nsect, 
			 buf, is_write ? "write" : "read");

	if (is_write)
	{
		return 0;
	}
	else
	{
#ifndef	UNIT_TEST
		VSR_DBG("read_from_dev!");
		return read_from_dev(dev, buf, sector, nsect);
#endif
	}
}



//#define DATA_TEST
#ifdef DATA_TEST
static void show_mem_data(void *mem, unsigned int len)
{
	int i;
	u8 *data = (u8 *)mem;
	
	for (i = 0; i < len; i++)
	{
		if (0 == i % 32)
			printk("%04d: ", i);

		printk("%02x ", data[i]);

		if (0 == (i+1) % 32)
			printk("\n");
	}
}
#endif

static int vsr_make_request(struct request_queue *queue, struct bio *bio)
{
	int i;
	int ret = 0;
	struct bio_vec *bvec;
	void *disk_mem;
	void *bvec_mem;
	u32 sect_cnt = 0;
	u32 sect_len = 0;
	struct vsr_dev *dev = queue->queuedata;
	VSR_DBG("vsr_make_request!");
	
#if 1
	bio_for_each_segment(bvec, bio, i)
	{
		bvec_mem = kmap(bvec->bv_page) + bvec->bv_offset;
		sect_len = byte_to_sector(bvec->bv_len);
		ret = vsr_transfer(dev, bio->bi_sector + sect_cnt, sect_len, bvec_mem, bio_data_dir(bio));

		if (0 > ret)
		{
			kunmap(bvec->bv_page);
			VSR_ERR("vsr transfer error(%d)!", ret);
			goto err;
		}
		
#ifdef DATA_TEST		
		show_mem_data(bvec_mem, 2048);
#endif
		sect_cnt += sect_len;
		kunmap(bvec->bv_page);
		
		if (sect_cnt >= bio_sectors(bio))
			break;
	}

	bio_endio(bio, 0);
	return 0;
err:
	bio_endio(bio, -EIO);
#endif
	return ret;
}

static void vsr_request(struct request_queue *queue)
{
	struct request *req;
	VSR_DBG("come in!");
	
	while ((req = blk_peek_request(queue)) != NULL)
	{
		struct vsr_dev *dev = req->rq_disk->private_data;
		VSR_DBG("get request!");
		VSR_DBG("get dev addr = 0x%p!", dev);
		blk_start_request(req);

		if ((req->cmd_type != REQ_TYPE_FS) || (rq_data_dir(req) == WRITE))
		{
			printk(KERN_NOTICE "Skip non-fs request\n");
			__blk_end_request(req, -EIO, blk_rq_cur_bytes(req));
			continue;
		}
		
		VSR_DBG("effect request!");
		VSR_DBG("do_blk_req %p: cmd %p, sec %lx, "
					 "(%u/%u) buffer:%p [%s]\n",
					 req, req->cmd, (unsigned long)blk_rq_pos(req),
					 blk_rq_cur_sectors(req), blk_rq_sectors(req),
					 req->buffer, rq_data_dir(req) ? "write" : "read");

		vsr_transfer(dev, blk_rq_pos(req), blk_rq_sectors(req), req->buffer, rq_data_dir(req));
		__blk_end_request(req, 0, blk_rq_cur_bytes(req));
	}
}

static void vsr_full_request(struct request_queue *queue)
{

}

static int init_blk_request(struct vsr_dev *dev, int mode)
{
	int ret = 0;
	
	switch(mode) {
	case MD_NOQUEUE:
		VSR_DBG("MD_NOQUEUE");
		dev->rq_queue = blk_alloc_queue(GFP_KERNEL);
		if (!dev->rq_queue)
		{
			ret = -ENOMEM;
			goto err_queue;
		}

		blk_queue_make_request(dev->rq_queue, &vsr_make_request);
		break;
	case MD_SIMPLE:
		VSR_DBG("MD_SIMPLE");
		dev->rq_queue = blk_init_queue(vsr_request, &dev->lock);
		if (!dev->rq_queue)
		{
			ret = -ENOMEM;
			goto err_queue;
		}

		break;
	case MD_FULL:
		VSR_DBG("MD_FULL");
		dev->rq_queue = blk_init_queue(vsr_full_request, &dev->lock);
		if (!dev->rq_queue)
		{
			ret = -ENOMEM;
			goto err_queue;
		}
		break;
	default:
		break;
	}
	
err_queue:
	return ret;
}

static int __init vsr_init(void)
{
	int ret = 0;

	vdev.major = register_blkdev(VSR_MAJOR, VSR_DEV_NAME);
	VSR_DBG("vsr driver major is: %d", vdev.major);
	
	if (vdev.major <= 0)
	{
		ret = -EIO;
		goto out;
	}
	
	spin_lock_init(&vdev.lock);

	ret = init_blk_request(&vdev, MD_NOQUEUE);
	if (0 > ret)
	{
		VSR_ERR("init_blk_request failed!");
		goto err_queue;
	}
	//blk_queue_hardsect_size();
	VSR_DBG("init_blk_request success!");
	
	vdev.gdk = alloc_disk(1);
	if (!vdev.gdk)
	{
		ret = -ENOMEM;
		VSR_ERR("alloc_disk failed!");
		goto err_alloc;	
	}
	VSR_DBG("alloc_disk success!");
	
	vdev.gdk->major = vdev.major ;
	vdev.gdk->first_minor = 0;
	vdev.gdk->fops = &vsr_fops;
	vdev.gdk->queue = vdev.rq_queue;
	vdev.xfer_over = 0;
	vdev.gdk->private_data = &vdev;
	vdev.gdk->queue->queuedata = &vdev;
	sprintf(vdev.gdk->disk_name, VSR_DEV_NAME);
	set_capacity(vdev.gdk, DISK_CAPACITY_SIZE >> 9);

	init_waitqueue_head(&vdev.wait_xfer);
	state_machine_init(&vdev.stmch);
	add_disk(vdev.gdk);
	VSR_DBG("init dev success!");
	
	vdev.dev_class = class_create(THIS_MODULE, VSR_DEV_NAME);
	if (IS_ERR(vdev.dev_class))
	{
		VSR_ERR("device_create failed!");
		ret = -1;
		goto err_class;
	}
	device_create(vdev.dev_class, NULL, MKDEV(vdev.major, 0), NULL, VSR_DEV_NAME);
	VSR_DBG("device_create success!");
	
#ifdef	UNIT_TEST
	read_data_test(&vdev);
#else

#ifdef USE_KTHREAD
	init_waitqueue_head(get_recv_task_qh());
	recv_kthread_init();
#else
	vdev.vsr_work_queue = create_workqueue(VSR_WORK_QUEUE);
	INIT_DELAYED_WORK(&vdev.recv_work, proc_recv_data);
#endif

#endif

out:
	return ret;
err_class:
	del_gendisk(vdev.gdk);
	put_disk(vdev.gdk);
err_alloc:
	blk_cleanup_queue(vdev.rq_queue);
err_queue:
	unregister_blkdev(vdev.major, VSR_DEV_NAME);
	
}

static void __exit vsr_exit(void)
{
#ifndef	UNIT_TEST
	destroy_workqueue(vdev.vsr_work_queue);
#endif
#ifdef USE_KTHREAD
	recv_kthread_stop();
#endif	
	device_destroy(vdev.dev_class, MKDEV(vdev.major, 0));
	class_destroy(vdev.dev_class);
	
	del_gendisk(vdev.gdk);
	put_disk(vdev.gdk);

	blk_cleanup_queue(vdev.rq_queue);

	unregister_blkdev(vdev.major, VSR_DEV_NAME);
}

module_init(vsr_init);
module_exit(vsr_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WJing Wang");
MODULE_DESCRIPTION("virtual sr driver");
