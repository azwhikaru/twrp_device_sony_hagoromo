/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 *  cxd224x-i2c.c - cxd224x NFC i2c driver
 *
 * Copyright (C) 2013,2015 Sony Corporation.
 * Copyright (C) 2012 Broadcom Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/poll.h>
#include <linux/version.h>
#include <linux/dma-mapping.h>

#include <linux/nfc/cxd224x.h>
#include <linux/wakelock.h>

#include <linux/sched.h>

#include <mach/mt_gpio.h>
#include <mach/irqs.h>
#include <mach/eint.h>
#include <cust_eint.h>
#include <nfc_board_devs.h>

#define MODULE_NAME                   "cxd224x-i2c"

#define CXD224X_WAKE_LOCK_NAME        "cxd224x-i2c"        /* wake lock for HOSTINT */
#define CXD224X_WAKE_LOCK_NAME_LP     "cxd224x-i2c-lp"     /* wake lock for low-power-mode */
#define CXD224X_WAKE_LOCK_TIMEOUT     (10)    /* wake lock timeout for HOSTINT (sec) */
#define CXD224X_WAKE_LOCK_TIMEOUT_LP  (3)     /* wake lock timeout for low-power-mode (sec) */

/* do not change below */
#define MAX_BUFFER_SIZE          (780)

/* Read data */
#define PACKET_HEADER_SIZE_NCI   (3)
#define PACKET_HEADER_SIZE_HCI   (3)
#define PACKET_TYPE_NCI          (16)
#define PACKET_TYPE_HCIEV        (4)
#define MAX_PACKET_SIZE          (PACKET_HEADER_SIZE_NCI + 255)

/* Retry Threshold*/
#define RST_RETRY_CNT            (3)

//for debug
//#define CXD224X_DEBUG_ENABLE    1
#if CXD224X_DEBUG_ENABLE
#define DBG_MSG(format, arg...)  printk(KERN_WARNING "%s: " format "\n",             MODULE_NAME, ## arg)
#else /*CXD224X_DEBUG_ENABLE*/
#define DBG_MSG(format, arg...)  do {} while (0)
#endif /* CXD224X_DEBUG_ENABLE */

static int ioctl_reset_flg = 0;
static int probe_exec = 0; 
static int first_irq_handler = 0; 

struct cxd224x_dev *cxd224x_dev_ptr = NULL;
struct cxd224x_platform_data *platform_data;

struct cxd224x_dev {
    wait_queue_head_t read_wq;
    struct mutex read_mutex;
    struct i2c_client *client;
    struct miscdevice cxd224x_device;
    unsigned int rst_gpio;
    unsigned int en_gpio;
    unsigned int pon_gpio;
    unsigned int xint_gpio;
    bool irq_enabled;
    spinlock_t irq_enabled_lock;
    unsigned int count_irq;
    struct wake_lock wakelock;     /* wake lock for HOSTINT */
    struct wake_lock wakelock_lp;  /* wake lock for low-power-mode */
};

static void cxd224x_init_stat(struct cxd224x_dev *cxd224x_dev)
{
    DBG_MSG("== %s called", __func__);
    cxd224x_dev->count_irq = 0;
}

static void cxd224x_disable_irq(struct cxd224x_dev *cxd224x_dev)
{
    unsigned long flags;

    DBG_MSG("== %s called", __func__);

    spin_lock_irqsave(&cxd224x_dev->irq_enabled_lock, flags);
    if (cxd224x_dev->irq_enabled) {
        mt_eint_mask(cxd224x_dev->client->irq);
        cxd224x_dev->irq_enabled = false;
    }
    spin_unlock_irqrestore(&cxd224x_dev->irq_enabled_lock, flags);
}

static void cxd224x_enable_irq(struct cxd224x_dev *cxd224x_dev)
{
    unsigned long flags;
    
    DBG_MSG("== %s called", __func__);
    
    spin_lock_irqsave(&cxd224x_dev->irq_enabled_lock, flags);
    if (!cxd224x_dev->irq_enabled) {
        cxd224x_dev->irq_enabled = true;
        mt_eint_unmask(cxd224x_dev->client->irq);
    }
    spin_unlock_irqrestore(&cxd224x_dev->irq_enabled_lock, flags);
}

static void cxd224x_dev_irq_handler(void)
{
    struct cxd224x_dev *cxd224x_dev = cxd224x_dev_ptr;
    unsigned long flags;

    DBG_MSG("== %s called", __func__);

    first_irq_handler = 1;
    if(probe_exec == 0){
        spin_lock_irqsave(&cxd224x_dev->irq_enabled_lock, flags);
        cxd224x_dev->count_irq++;
        spin_unlock_irqrestore(&cxd224x_dev->irq_enabled_lock, flags);
        wake_up(&cxd224x_dev->read_wq);
    }
}

static unsigned int cxd224x_dev_poll(struct file *filp, poll_table *wait)
{
    struct cxd224x_dev *cxd224x_dev = filp->private_data;
    unsigned int mask = 0;
    unsigned long flags;

    DBG_MSG("== %s called", __func__);

    poll_wait(filp, &cxd224x_dev->read_wq, wait);

    spin_lock_irqsave(&cxd224x_dev->irq_enabled_lock, flags);
    if(ioctl_reset_flg == 1) {
        ioctl_reset_flg = 0;
        mask |= POLLERR;
    } else if (cxd224x_dev->count_irq > 0) {
        cxd224x_dev->count_irq--;
        mask |= POLLIN | POLLRDNORM;
    }
    spin_unlock_irqrestore(&cxd224x_dev->irq_enabled_lock, flags);
    
    if(mask) {
        wake_lock_timeout(&cxd224x_dev->wakelock, CXD224X_WAKE_LOCK_TIMEOUT*HZ);
    }

    return mask;
}

static ssize_t cxd224x_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    struct cxd224x_dev *cxd224x_dev = filp->private_data;
    int total =0;
    int len   =0;
    int ret   =0;
    char *dmabuf;
    dma_addr_t dmaphysaddr;

    DBG_MSG("== %s called", __func__);

    dmabuf = (char *)dma_alloc_coherent(NULL, MAX_BUFFER_SIZE, &dmaphysaddr, GFP_KERNEL);
    if (dmabuf == NULL) { 
        ret = -ENOMEM;
        goto err_exit;
    }

    if (count > MAX_BUFFER_SIZE) {
        count = MAX_BUFFER_SIZE;
    }

    mutex_lock(&cxd224x_dev->read_mutex);

    ret = mt_i2c_master_recv(cxd224x_dev->client, (char*)dmaphysaddr, 3, I2C_DMA_FLAG);
    if (ret == 3 && (dmabuf[0] != 0xff)) {
        total = ret;

        len = dmabuf[PACKET_HEADER_SIZE_NCI-1];

#if CXD224X_DEBUG_ENABLE
        dev_err(&cxd224x_dev->client->dev, "Readlen %d+%d\n", total, len);
#endif /* CXD224X_DEBUG_ENABLE */
        /** make sure full packet fits in the buffer **/
        if (len > 0 && (len + total) <= count) {
            /** read the remainder of the packet. **/
	    ret = mt_i2c_master_recv(cxd224x_dev->client, (char*)dmaphysaddr + total, len, I2C_DMA_FLAG);
            if (ret == len) {
                total += len;
            }
        }
    }

    mutex_unlock(&cxd224x_dev->read_mutex);

    if (total > count || copy_to_user(buf, dmabuf, total)) {
        dev_err(&cxd224x_dev->client->dev, "failed to copy to user space, total = %d\n", total);
        total = -EFAULT;
    }

    if(dmabuf != NULL){
        dma_free_coherent(NULL, MAX_BUFFER_SIZE, dmabuf, dmaphysaddr);
        dmabuf    = NULL;
        dmaphysaddr = 0;
    }

    return total;

err_exit:
    if(dmabuf != NULL){
        dma_free_coherent(NULL, MAX_BUFFER_SIZE, dmabuf, dmaphysaddr);
        dmabuf    = NULL;
        dmaphysaddr = 0;
    }
    return ret;
}

static ssize_t cxd224x_dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
    struct cxd224x_dev *cxd224x_dev = filp->private_data;
    int ret;

    char *dmabuf;
    dma_addr_t dmaphysaddr;

    DBG_MSG("== %s called", __func__);

    dmabuf = (char *)dma_alloc_coherent(NULL, MAX_BUFFER_SIZE, &dmaphysaddr, GFP_KERNEL);
    if (dmabuf == NULL) { 
        ret = -ENOMEM;
        goto err_exit;
    }

    if (count > MAX_BUFFER_SIZE) {
        dev_err(&cxd224x_dev->client->dev, "out of memory\n");
        ret = -ENOMEM;
        goto err_exit;
    }

    if (copy_from_user(dmabuf, buf, count)) {
        dev_err(&cxd224x_dev->client->dev, "failed to copy from user space\n");
        ret = -ENOMEM;
        goto err_exit;
    }
    
    /* Write data */
    mutex_lock(&cxd224x_dev->read_mutex);
    ret = mt_i2c_master_send(cxd224x_dev->client, (char*)dmaphysaddr, count, I2C_DMA_FLAG);
    if (ret != count) {
        dev_err(&cxd224x_dev->client->dev, "failed to write %d\n", ret);
        ret = -EIO;
    }
    mutex_unlock(&cxd224x_dev->read_mutex);
    

    if(dmabuf != NULL){
        dma_free_coherent(NULL, MAX_BUFFER_SIZE, dmabuf, dmaphysaddr);
        dmabuf    = NULL;
        dmaphysaddr = 0;
    }

    return ret;
    
err_exit:
    if(dmabuf != NULL){
        dma_free_coherent(NULL, MAX_BUFFER_SIZE, dmabuf, dmaphysaddr);
        dmabuf    = NULL;
        dmaphysaddr = 0;
    }
    return ret;
}

static int cxd224x_dev_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
    struct cxd224x_dev *cxd224x_dev;

    DBG_MSG("== %s called", __func__);

    cxd224x_dev = container_of(filp->private_data, struct cxd224x_dev, cxd224x_device);

    filp->private_data = cxd224x_dev;
    cxd224x_dev_ptr    = cxd224x_dev;

    cxd224x_init_stat(cxd224x_dev);
    cxd224x_enable_irq(cxd224x_dev);
    dev_info(&cxd224x_dev->client->dev, "%d,%d\n", imajor(inode), iminor(inode));

    return ret;
}

static long cxd224x_dev_unlocked_ioctl(struct file *filp,
                     unsigned int cmd, unsigned long arg)
{
    struct cxd224x_dev *cxd224x_dev = filp->private_data;

    DBG_MSG("== %s cmd(%d) called", __func__, cmd);

    switch (cmd) {
    case CXDNFC_RESET_CTL:
        mt_set_gpio_out(cxd224x_dev->rst_gpio, GPIO_OUT_ZERO);
        usleep_range(10100, 10200);
        mt_set_gpio_out(cxd224x_dev->rst_gpio, GPIO_OUT_ONE);
        cxd224x_dev->count_irq = 0;
        
        //for wake_up();
        ioctl_reset_flg = 1;
        wake_up(&cxd224x_dev->read_wq);
        break;
    case CXDNFC_WAKE_CTL:
        if (arg == ARG_PON_ON) {
            /* PON HIGH (normal power mode)*/
            wake_lock_timeout(&cxd224x_dev->wakelock_lp, CXD224X_WAKE_LOCK_TIMEOUT_LP*HZ);
            mt_set_gpio_out(cxd224x_dev->pon_gpio, GPIO_OUT_ONE);
        } else if (arg == ARG_PON_OFF) {
            /* PON LOW (low power mode) */
            mt_set_gpio_out(cxd224x_dev->pon_gpio,GPIO_OUT_ZERO);
            wake_unlock(&cxd224x_dev->wakelock_lp);
        } else {
            return -EINVAL;
        }
        break;
    default:
        dev_err(&cxd224x_dev->client->dev, "%s, unknown cmd (%x, %lx)\n", __func__, cmd, arg);
        return -EINVAL;
    }
    return 0;
}

static ssize_t show_cxd224x_access(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);
    char i2cbuf[3];
    int ret;

    //DBG_MSG("== %s called  -- i2c_master_recv test", __func__);
    printk(KERN_WARNING "%s: == %s called  -- i2c_master_recv test\n", MODULE_NAME, __func__);

    ret = mt_i2c_master_recv(client, (unsigned char *)i2cbuf, 3, 0);
    
    //DBG_MSG("== %s called  -- i2c_master_recv test ret(%d)", __func__, ret);
    printk(KERN_WARNING "%s: == %s called  -- i2c_master_recv test ret(%d)\n", MODULE_NAME, __func__, ret);

    if(ret < 0) {
        return -EIO;
    }

    return 0;
}
static ssize_t store_cxd224x_access(struct device *dev, struct device_attribute *attr, const char *buf,size_t size)
{
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);
    char i2cbuf[3] = { 0x01, 0x02, 0x03 };
    int ret;

    //DBG_MSG("== %s called  -- i2c_master_send test", __func__);
    printk(KERN_WARNING "%s: == %s called  -- i2c_master_send test\n", MODULE_NAME, __func__);

    ret = mt_i2c_master_send(client, (unsigned char *)i2cbuf, 3, 0);

    //DBG_MSG("== %s called  -- i2c_master_send ret(%d)", __func__, ret);
    printk(KERN_WARNING "%s: == %s called  -- i2c_master_send test ret(%d)\n", MODULE_NAME, __func__, ret);

    if(ret < 0){
        return -EIO;
    }

    return size;
}
static DEVICE_ATTR(cxd224x_access, 0664, show_cxd224x_access, store_cxd224x_access);

static const struct file_operations cxd224x_dev_fops = {
    .owner  = THIS_MODULE,
    .llseek = no_llseek,
    .poll   = cxd224x_dev_poll,
    .read   = cxd224x_dev_read,
    .write  = cxd224x_dev_write,
    .open   = cxd224x_dev_open,
    .unlocked_ioctl = cxd224x_dev_unlocked_ioctl
};

static int cxd224x_probe(struct i2c_client *client,
               const struct i2c_device_id *id)
{
    int ret;
    int retry;
    struct cxd224x_dev *cxd224x_dev;

    DBG_MSG("== %s called", __func__);

    platform_data = client->dev.platform_data;
    probe_exec = 1;

    dev_info(&client->dev, "%s, probing cxd224x driver flags = %x\n", __func__, client->flags);
    if (platform_data == NULL) {
        dev_err(&client->dev, "nfc probe fail\n");
        ret = -ENODEV;
        goto err_exit;
    }

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        dev_err(&client->dev, "need I2C_FUNC_I2C\n");
        ret = -ENODEV;
        goto err_exit;
    }

    cxd224x_dev = kzalloc(sizeof(*cxd224x_dev), GFP_KERNEL);
    if (cxd224x_dev == NULL) {
        dev_err(&client->dev, "failed to allocate memory for module data\n");
        ret = -ENOMEM;
        goto err_exit;
    }

    cxd224x_dev->xint_gpio = platform_data->xint_gpio;
    cxd224x_dev->en_gpio   = platform_data->en_gpio;
    cxd224x_dev->rst_gpio  = platform_data->rst_gpio;
    cxd224x_dev->pon_gpio  = platform_data->pon_gpio;
    cxd224x_dev->client    = client;

    wake_lock_init(&cxd224x_dev->wakelock, WAKE_LOCK_SUSPEND, CXD224X_WAKE_LOCK_NAME);
    wake_lock_init(&cxd224x_dev->wakelock_lp, WAKE_LOCK_SUSPEND, CXD224X_WAKE_LOCK_NAME_LP);

    /* init mutex and queues */
    init_waitqueue_head(&cxd224x_dev->read_wq);
    mutex_init(&cxd224x_dev->read_mutex);
    spin_lock_init(&cxd224x_dev->irq_enabled_lock);

    cxd224x_dev->cxd224x_device.minor = MISC_DYNAMIC_MINOR;
    cxd224x_dev->cxd224x_device.name  = MODULE_NAME;
    cxd224x_dev->cxd224x_device.fops  = &cxd224x_dev_fops;

    ret = misc_register(&cxd224x_dev->cxd224x_device);
    if (ret) {
        dev_err(&client->dev, "misc_register failed\n");
        goto err_misc_register;
    }

    i2c_set_clientdata(client, cxd224x_dev);
    dev_info(&client->dev, "%s, probing cxd224x driver exited successfully\n", __func__);

    /* Initialize GPIO Setting */
    /* explicitly Disable Eint */
    mt_eint_mask(CUST_EINT_IRQ_SONY_DMP_NFC_NUM);

    /* LDO_EN GPIO */
#ifdef CONFIG_CXD224X_NFC_VEN
    mt_set_gpio_mode(platform_data->en_gpio, GPIO_MODE_00);
    mt_set_gpio_out(platform_data->en_gpio, GPIO_OUT_ZERO);
    mt_set_gpio_dir(platform_data->en_gpio, GPIO_DIR_OUT);
#endif
    /* XINT GPIO */
    mt_set_gpio_mode(platform_data->xint_gpio, GPIO_MODE_00);
    mt_set_gpio_dir(platform_data->xint_gpio, GPIO_DIR_IN);
    mt_set_gpio_pull_select(platform_data->xint_gpio, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(platform_data->xint_gpio, GPIO_PULL_ENABLE);

    /* RST GPIO */
    mt_set_gpio_mode(platform_data->rst_gpio, GPIO_MODE_00);
    mt_set_gpio_out(platform_data->rst_gpio, GPIO_OUT_ZERO);
    mt_set_gpio_dir(platform_data->rst_gpio, GPIO_DIR_OUT);
  
    /* PON GPIO */
    mt_set_gpio_mode(platform_data->pon_gpio, GPIO_MODE_00);
    mt_set_gpio_out(platform_data->pon_gpio, GPIO_OUT_ZERO);
    mt_set_gpio_dir(platform_data->pon_gpio, GPIO_DIR_OUT);

    /* Eint registration */
    cxd224x_dev->irq_enabled = true;
    mt_eint_set_polarity(CUST_EINT_IRQ_SONY_DMP_NFC_NUM,MT_POLARITY_LOW);
    mt_eint_set_sens(CUST_EINT_IRQ_SONY_DMP_NFC_NUM,MT_EDGE_SENSITIVE);
    mt_eint_registration(CUST_EINT_IRQ_SONY_DMP_NFC_NUM, CUST_EINT_IRQ_SONY_DMP_NFC_TYPE, cxd224x_dev_irq_handler,1);

    /* LDO_EN GPIO */
#ifdef CONFIG_CXD224X_NFC_VEN
    mt_set_gpio_out(platform_data->en_gpio, GPIO_OUT_ONE);
#endif
    usleep_range(10100, 10200);

    /* After the reset , if an interrupt did not enter , 
       it is better that you retry several times. */
    for(retry=0; retry < RST_RETRY_CNT; retry++){
        /* RST GPIO */
        mt_set_gpio_out(platform_data->rst_gpio, GPIO_OUT_ONE);
        usleep_range(10100, 10200);

        if(first_irq_handler == 1){
            /* PON GPIO */
            mt_set_gpio_out(platform_data->pon_gpio, GPIO_OUT_ZERO);
            break;
        }
    }
    if(retry == RST_RETRY_CNT){
        goto err_misc_register;
    }

    device_create_file(&(client->dev), &dev_attr_cxd224x_access);
    cxd224x_disable_irq(cxd224x_dev);
    probe_exec = 0;

    return 0;

err_misc_register:
    mutex_destroy(&cxd224x_dev->read_mutex);
    kfree(cxd224x_dev);
err_exit:
    probe_exec = 0;

    return ret;
}

static int cxd224x_remove(struct i2c_client *client)
{
    struct cxd224x_dev *cxd224x_dev;

    DBG_MSG("== %s called", __func__);

    cxd224x_dev = i2c_get_clientdata(client);

    if(cxd224x_dev == NULL){
        pr_err("%s : i2c_get_clientdata failed \n", __func__);
        return 0;
    }

    mt_eint_registration(CUST_EINT_IRQ_SONY_DMP_NFC_NUM, CUST_EINT_IRQ_SONY_DMP_NFC_TYPE, NULL, false);

    device_remove_file(&(client->dev), &dev_attr_cxd224x_access);
    wake_lock_destroy(&cxd224x_dev->wakelock);
    wake_lock_destroy(&cxd224x_dev->wakelock_lp);

    misc_deregister(&cxd224x_dev->cxd224x_device);
//  Accessed after shutdown to become kernelpanic.    
//    mutex_destroy(&cxd224x_dev->read_mutex);
//    kfree(cxd224x_dev);

    return 0;
}

static int cxd224x_shutdown(struct i2c_client *client)
{
    DBG_MSG("== %s called", __func__);
    
    cxd224x_remove(client);
    return 0;
}

#ifdef CONFIG_PM
static int cxd224x_suspend(struct device *dev)
{
    DBG_MSG("== %s called", __func__);
    
    return 0;
}

static int cxd224x_resume(struct device *dev)
{
    DBG_MSG("== %s called", __func__);
    
    return 0;
}

static const struct dev_pm_ops cxd224x_pm_ops = {
    .suspend        = cxd224x_suspend,
    .resume         = cxd224x_resume,
};
#endif

static const struct i2c_device_id cxd224x_id[] = {
    {MODULE_NAME, 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, cxd224x_id);

static struct i2c_driver cxd224x_driver = {
    .id_table = cxd224x_id,
    .probe    = cxd224x_probe,
    .remove   = cxd224x_remove,
    .shutdown = cxd224x_shutdown,
    .driver = {
        .owner = THIS_MODULE,
        .name  = MODULE_NAME,
#ifdef CONFIG_PM
        .pm    = &cxd224x_pm_ops,
#endif
    },
};

/*
 * module load/unload record keeping
 */

static int __init cxd224x_dev_init(void)
{
    return i2c_add_driver(&cxd224x_driver);
}
module_init(cxd224x_dev_init);

static void __exit cxd224x_dev_exit(void)
{
    i2c_del_driver(&cxd224x_driver);
}
module_exit(cxd224x_dev_exit);

MODULE_AUTHOR("Sony");
MODULE_DESCRIPTION("NFC cxd224x driver");
MODULE_LICENSE("GPL");
