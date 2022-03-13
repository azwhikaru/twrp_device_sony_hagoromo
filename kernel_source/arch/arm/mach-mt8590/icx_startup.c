/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* ICX startup thread.
 *
 * Copyright 2015 Sony Corporation
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifdef CONFIG_ICX_STARTUP

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/device.h>

#ifdef CONFIG_ICX_MMC_TEST
#define ICX_DBG(fmt, args...)
#define ICX_INF(fmt, args...)
#define ICX_ERR(fmt, args...)
#else
#define ICX_DBG(fmt, args...)     printk(KERN_DEBUG "%s(%d) " fmt, __func__, __LINE__, ##args)
#define ICX_INF(fmt, args...)     printk(KERN_INFO  "%s(%d) " fmt, __func__, __LINE__, ##args)
#define ICX_ERR(fmt, args...)     printk(KERN_ERR   "%s(%d) " fmt, __func__, __LINE__, ##args)
#endif

static struct task_struct *icx_startup_thread;

static int icx_startup_thread_func(void *p)
{
    ICX_INF("start\n");
    while(!kthread_should_stop()) {
        ICX_ERR("This is sample code by platform.\n");
        ICX_ERR("Please modify here by product.\n");
        msleep(1000);
    }
    ICX_INF("done\n");
    return 0;
}

static void icx_startup_stop(void)
{
    if(!icx_startup_thread) {
        ICX_ERR("already stopped\n");
        return;
    }

    // stop thread
    kthread_stop(icx_startup_thread);
    icx_startup_thread = 0;
}

static void icx_startup_start(void)
{
    if(icx_startup_thread) {
        ICX_ERR("already started\n");
        return;
    }

    // create thread
    icx_startup_thread = kthread_create(icx_startup_thread_func, NULL, "icx_startup_thread");
    if(IS_ERR(icx_startup_thread)) {
        ICX_ERR("err=%ld\n", PTR_ERR(icx_startup_thread));
        icx_startup_thread  = 0;
        return;
    }

    // wakeup thread
    wake_up_process(icx_startup_thread);
}

static ssize_t icx_startup_stop_store(struct device_driver* drv, const char *buf, size_t len)
{
    icx_startup_stop();
    return len;
}

DRIVER_ATTR(icx_startup_stop, 0222, NULL, icx_startup_stop_store);

void icx_startup_init(struct device_driver *drv)
{
    int err;

    err = driver_create_file(drv, &driver_attr_icx_startup_stop);
    if(err) {
        ICX_ERR("driver_create_file() err=%d\n", err);
        return;
    }

    icx_startup_start();
}

void icx_startup_exit(struct device_driver *drv)
{
    icx_startup_stop();
    driver_remove_file(drv, &driver_attr_icx_startup_stop);
}

EXPORT_SYMBOL(icx_startup_init);
EXPORT_SYMBOL(icx_startup_exit);

#endif // CONFIG_ICX_STARTUP

