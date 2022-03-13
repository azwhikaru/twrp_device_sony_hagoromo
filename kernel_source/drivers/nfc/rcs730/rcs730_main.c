/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * Copyright (C) 2014,2015 Sony Corporation.
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
#include <linux/cdev.h>
#include <linux/of_gpio.h>
#include <linux/dma-mapping.h>
#include <linux/nfc/rcs730.h>
#include <linux/poll.h>

#include <linux/time.h>

#include <cust_eint.h>
#include <mach/mt_gpio.h>
#include <mach/irqs.h>
#include <mach/eint.h>
#include "rcs730_main.h"

#define  MAX_BUFFER_SIZE       512
#define  MAX_BUFFER_SIZE_IOCTL 16
#define  RCS730_ADDR_SIZE      2
#define  S730_XFER_MAX         16
#define  MODULE_NAME           "rcs730"


#define  DETECT_MAX_TRESHOLD (3)    /*  3  sec */
#define  DETECT_MIN_TRESHOLD (200)  /*200 usec */

//for debug
//#define NFCLINK_DEBUG_ENABLE  (1)
#ifdef NFCLINK_DEBUG_ENABLE
#define DBG_MSG(format, arg...)  printk(KERN_WARNING "%s: " format "\n",             MODULE_NAME, ## arg)
#else /*NFCLINK_DEBUG_ENABLE*/
#define DBG_MSG(format, arg...)  do {} while (0)
#endif /*NFCLINK_DEBUG_ENABLE*/

typedef enum
{
    NFC_CONFIG__NONE = 0,
    NFC_CONFIG__SYSOPTION,
    NFC_CONFIG__OPMODE,
    NFC_CONFIG__LITES_PMm,
    NFC_CONFIG__STBY_CONTROL,
    NFC_CONFIG__IRQ_MASK,
    NFC_CONFIG__RX_CONTROL,
    NFC_CONFIG__END,
} nfc_config_state;

struct rcs730_dev {
    struct mutex read_mutex;
    struct i2c_client *client;
    struct miscdevice rcs730_device;
    void (*conf_gpio) (void);
    unsigned int rfdet_gpio;
    unsigned int irq_gpio;
    atomic_t irq_enabled;
    atomic_t read_flag;
    bool cancel_read;
    spinlock_t irq_enabled_lock;
    wait_queue_head_t   read_wq;
    unsigned int count_irq;
};

#define NFC_LINK_RUNNING 0
static unsigned long open_allowed;

/* For DMA */
char *I2CDMAWriteBuf;
dma_addr_t I2CDMAWriteBuf_pa;  /* = NULL; */
char *I2CDMAReadBuf;
dma_addr_t I2CDMAReadBuf_pa;   /* = NULL; */
#define RCS730_WAKE_LOCK_TIMEOUT 10    /* wake lock timeout for HOSTINT (sec) */

typedef struct nfc_config_data
{
    nfc_config_state  state;
    unsigned char     data_length;
    unsigned short    regadd;
    unsigned char     data[S730_XFER_MAX];
} NFC_CONFIG_DATA;

const  NFC_CONFIG_DATA    nfc_config_tbl[] =     /* Setting Data Table */
{
     /* step */   /* data length */  /* register */         /* b7-0 */ /* b15-8 */ /* b23-16 */ /* b31-24 */
  {  NFC_CONFIG__NONE,           0,  REG__OPMODE,            { 0x00,      0x00,      0x00,      0x00 } },
  {  NFC_CONFIG__OPMODE,         6,  REG__OPMODE,            { 0x03,      0x00,      0x00,      0x00 } },    /* Lite-S mode */
  {  NFC_CONFIG__SYSOPTION,      3,  BLK__MC__SYS_OP,        { 0x01,      0x00,      0x00,      0x00 } },    /* MC SystemOption = NDEF support */
  {  NFC_CONFIG__LITES_PMm,      6,  REG__LITE_S_PMm,        { 0x01,      0x12,      0x00,      0x00 } },    /* Max Response Time */
  {  NFC_CONFIG__STBY_CONTROL,   6,  REG__STANDBY_CONTROL,   { 0x01,      0x00,      0xff,      0xff } },    /* AutoStandby Enable */
  {  NFC_CONFIG__IRQ_MASK,       6,  REG__INTERRUPT_MASK,    { 0x7d,      0x03,      0x0f,      0x80 } },    /* Interrupt Mask */  /* Enable "Received Polling" */
  {  NFC_CONFIG__RX_CONTROL,     6,  REG__TAG_RX_CONTROL,    { 0x01,      0x00,      0x00,      0x00 } },    /* Operation Enable */
  {  NFC_CONFIG__END,            0,  REG__OPMODE,            { 0x00,      0x00,      0x00,      0x00 } },
};

struct rcs730_dev *rcs730_dev_ptr = NULL;

static struct timeval rfdet_tv;

static void rcs730_dev_rfdet_handler(void)
{
    struct rcs730_dev *rcs730_dev = rcs730_dev_ptr;
    unsigned long flags;
    struct timeval crnt_tv;

//  pr_info("rcs730_dev_rfdet_handler start \n");
    DBG_MSG("== %s called", __func__);

    do_gettimeofday(&crnt_tv);
    DBG_MSG("== %d %d %d %d", crnt_tv.tv_sec, crnt_tv.tv_usec, rfdet_tv.tv_sec, rfdet_tv.tv_usec);

    rfdet_tv = crnt_tv;
}

static void rcs730_dev_irq_handler(void)
{
    struct rcs730_dev *rcs730_dev = rcs730_dev_ptr;
    unsigned long flags;
    struct timeval crnt_tv;
    struct timeval diff_tv;
    struct timeval tmp_rfdet_tv;

//  pr_info("rcs730_dev_irq_handler start \n");
    DBG_MSG("== %s called", __func__);

    tmp_rfdet_tv = rfdet_tv;
    do_gettimeofday(&crnt_tv);
    DBG_MSG("== crnt tv(%d sec  %d us) rfdet tv(%d sec  %d us) ", crnt_tv.tv_sec, crnt_tv.tv_usec, tmp_rfdet_tv.tv_sec, tmp_rfdet_tv.tv_usec);

    // Case RF Det interrupt not happened. Ignore this case.
    if (tmp_rfdet_tv.tv_sec == 0 && tmp_rfdet_tv.tv_usec == 0) {
        printk(KERN_ERR "%s: == %s RF Det interrupt not happened\n", MODULE_NAME, __func__);
        return;
    }

    if(tmp_rfdet_tv.tv_usec > crnt_tv.tv_usec){
        diff_tv.tv_sec  = crnt_tv.tv_sec - 1 - tmp_rfdet_tv.tv_sec;
        diff_tv.tv_usec = (1000000 - tmp_rfdet_tv.tv_usec) + crnt_tv.tv_usec;
    } else {
        diff_tv.tv_sec  = crnt_tv.tv_sec - tmp_rfdet_tv.tv_sec;
        diff_tv.tv_usec = crnt_tv.tv_usec - tmp_rfdet_tv.tv_usec;
    }

    DBG_MSG("== %s called  Diff tv(%d sec  %d us)", __func__, diff_tv.tv_sec, diff_tv.tv_usec);

    // ErrorCase.
    if (diff_tv.tv_sec == 0 && diff_tv.tv_usec < DETECT_MIN_TRESHOLD) {
        printk(KERN_ERR "%s: == %s ErrCase TimeInterval is close (0)sec (%d)usec \n", MODULE_NAME, __func__, diff_tv.tv_usec);
        return;
    }
    
    // Deside the complete the connection with wireless IF
    if (diff_tv.tv_sec < DETECT_MAX_TRESHOLD) {
        printk(KERN_WARNING "%s: == %s Diff Time value (%d)sec (%d)usec \n", MODULE_NAME, __func__, diff_tv.tv_sec, diff_tv.tv_usec);
        if(rcs730_dev != NULL){
            spin_lock_irqsave(&rcs730_dev->irq_enabled_lock, flags);
            rcs730_dev->count_irq++;
            spin_unlock_irqrestore(&rcs730_dev->irq_enabled_lock, flags);
            wake_up_interruptible(&rcs730_dev->read_wq);
        }
    } else {
        printk(KERN_ERR "%s: == %s ErrCase Diff TimeValue is TooLarge (%d) (%d) \n", MODULE_NAME, __func__, diff_tv.tv_sec, diff_tv.tv_usec);
    }

}

static unsigned int rcs730_dev_poll(struct file *filp, poll_table *wait)
{
    struct rcs730_dev *rcs730_dev = filp->private_data;
    unsigned int mask = 0;
    unsigned long flags;

    char tmp[MAX_BUFFER_SIZE_IOCTL] = {0, };
    int count = 0;
    int retry = 0;
    int ret   = 0;
    struct i2c_msg rcs730_msg[2];
    unsigned short address = 0;

//  pr_info("start(file:%p wait:%p)", filp, wait);
    DBG_MSG("== %s called", __func__);

    poll_wait(filp, &rcs730_dev->read_wq, wait);

    spin_lock_irqsave(&rcs730_dev->irq_enabled_lock, flags);
    if (rcs730_dev->count_irq > 0) {
        rcs730_dev->count_irq--;
        mask |= POLLIN | POLLRDNORM;
        printk(KERN_WARNING "%s: == %s mask(%d) \n", MODULE_NAME, __func__, mask);
    }
    spin_unlock_irqrestore(&rcs730_dev->irq_enabled_lock, flags);

//  pr_info("exit(%d)", mask);

    return mask;
}

static int rcs730_write_proc(struct rcs730_dev *rcs730_dev, char *buf, int size)
{
    int ret = 0, retry = 2;

// pr_info("%s : writing %zu bytes \n", __func__, size);
    DBG_MSG("== %s called", __func__);

    /* Write data */
    retry = 2;
    do {
        retry--;
        ret = mt_i2c_master_send(rcs730_dev->client, (unsigned char *)I2CDMAWriteBuf_pa, size, I2C_DMA_FLAG);
        if (ret == size) {
            break;
        }
        usleep_range(6000, 10000); /* Retry, chip was in standby */
    } while (retry);

    return ret;

}

static ssize_t rcs730_dev_read(struct file *filp, char __user * buf, size_t count, loff_t *offset)
{
    struct rcs730_dev *rcs730_dev = filp->private_data;
    struct i2c_msg rcs730_msg[2];
    int    ret = 0, retry = 2;

//  pr_info("Read rcs730 driver start\n");
    DBG_MSG("== %s called", __func__);

    if(count > MAX_BUFFER_SIZE){
        count = MAX_BUFFER_SIZE;
    }

    if(copy_from_user(I2CDMAReadBuf, buf, RCS730_ADDR_SIZE)){
        printk("%s : failed to copy from user space\n", __func__);
        return -EFAULT;
    }

    rcs730_msg[0].addr     = rcs730_dev->client->addr;
    rcs730_msg[0].flags    = rcs730_dev->client->flags & I2C_M_TEN;
    rcs730_msg[0].len      = RCS730_ADDR_SIZE;
    rcs730_msg[0].buf      = (char *)I2CDMAReadBuf_pa;
    rcs730_msg[0].ext_flag = I2C_DMA_FLAG;
    rcs730_msg[0].timing   = 400;

    rcs730_msg[1].addr     = rcs730_dev->client->addr;
    rcs730_msg[1].flags    = rcs730_dev->client->flags & I2C_M_TEN;
    rcs730_msg[1].flags   |= I2C_M_RD;
    rcs730_msg[1].len      = count;
    rcs730_msg[1].buf      = (char *)I2CDMAReadBuf_pa;
    rcs730_msg[1].ext_flag = I2C_DMA_FLAG;
    rcs730_msg[1].timing   = 400;

    /* Write Address and Read Data */
    do {
        retry--;
        ret = i2c_transfer(rcs730_dev->client->adapter, rcs730_msg, 2);
        if(ret >= 0){
            break;
        }
        usleep_range(6000, 10000); /* Retry, chip was in standby */
    } while (retry);

    if(ret < 0){
        pr_err("%s: i2c_master_recv returned %d\n", __func__, ret);
        return ret;
    }

    if(copy_to_user(buf, I2CDMAReadBuf, count)){
        pr_err("%s : failed to copy to user space\n", __func__);
        return -EFAULT;
    }
    return ret;
}

static ssize_t rcs730_dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
    struct rcs730_dev *rcs730_dev;
    int ret = 0;

// pr_info("Write rcs730 driver start\n");
    DBG_MSG("== %s called", __func__);

    rcs730_dev = filp->private_data;

    if (count > MAX_BUFFER_SIZE){
        count = MAX_BUFFER_SIZE;
    }

    if(copy_from_user(I2CDMAWriteBuf, buf, count)){
        pr_err("%s : failed to copy from user space\n", __func__);
        return -EFAULT;
    }

    ret = rcs730_write_proc(rcs730_dev, I2CDMAWriteBuf, count);
    if(ret != count){
        pr_err("%s : rcs730_write_proc returned %d\n", __func__, ret);
        ret = -EIO;
    }

    return ret;
}

static int rcs730_dev_open(struct inode *inode, struct file *filp)
{
    struct rcs730_dev *rcs730_dev = container_of(filp->private_data, struct rcs730_dev, rcs730_device);

//  pr_info("Open rcs730 driver start\n");
    DBG_MSG("== %s called", __func__);

    if(test_and_set_bit(NFC_LINK_RUNNING, &open_allowed)){
          return -EBUSY;
    }

    filp->private_data = rcs730_dev;
    rcs730_dev_ptr = rcs730_dev;
    pr_debug("%s : %d,%d\n", __func__, imajor(inode), iminor(inode));

    return 0;
}

static int rcs730_dev_release(struct inode *inode, struct file *filp)
{
    //  pr_info("Release rcs730 driver start\n");
    DBG_MSG("== %s called", __func__);

    clear_bit(NFC_LINK_RUNNING, &open_allowed);

    return 0;
}

static long rcs730_dev_ioctl(struct file *filp,
         unsigned int cmd, unsigned long arg)
{
    int retry = 2;
    struct rcs730_dev *rcs730_dev = filp->private_data;
    char tmp[MAX_BUFFER_SIZE_IOCTL] = {0, };
    int ret = 0, count = 0, step = NFC_CONFIG__NONE;

//  pr_info("IOCTL rcs730 driver start\n");
    DBG_MSG("== %s called", __func__);
  
    switch (cmd) {
    case RCS730_IOCTL__INITIALIZE:
//  pr_info("IOCTL rcs730 Initialize\n");
        step = NFC_CONFIG__NONE;
        step++;
        while( nfc_config_tbl[step].state != NFC_CONFIG__END ){
            
            tmp[0] = 0x00;
            tmp[1] = 0x00;
            tmp[2] = 0x00;
            tmp[3] = 0x00;
            tmp[4] = 0x00;
            tmp[5] = 0x00;
            count = nfc_config_tbl[step].data_length;
            tmp[0] = (unsigned char)(((unsigned short)nfc_config_tbl[step].regadd)>>8);;  /* Upper */
            tmp[1] = (unsigned char)nfc_config_tbl[step].regadd;  /* Lower */
            if( count > 2 ){
                memcpy( &(tmp[2]), &(nfc_config_tbl[step].data[0]), count-2 );
            }
            
            retry = 2;
            do {
                
                retry--;
                ret = mt_i2c_master_send(rcs730_dev->client, (unsigned char *)tmp, count,0);
                
                if (ret == count) {
                    break;
                }
                usleep_range(6000, 10000); /* Retry, chip was in standby */
               
            } while (retry);
            
            usleep_range(6000, 10000); /* Need some interval for next transaction */
            step++;
        };
        break;
    case RCS730_IOCTL__SUSPEND:
//  pr_info("IOCTL rcs730 Suspend\n");
        count  = 6;
        tmp[0] = (unsigned char)(((unsigned short)REG__TAG_RX_CONTROL)>>8);  /* Upper */
        tmp[1] = (unsigned char)REG__TAG_RX_CONTROL;  /* Lower */
        tmp[2] = 0x00;
        tmp[3] = 0x00;
        tmp[4] = 0x00;
        tmp[5] = 0x00;
        
        /* Write data */
        retry = 2;
        do {
            retry--;
            ret = mt_i2c_master_send(rcs730_dev->client, (unsigned char *)tmp, count,0);
            if(ret == count){
                break;
            }
            usleep_range(6000, 10000); /* Retry, chip was in standby */
        } while (retry);
        break;
    case RCS730_IOCTL__RESUME:
//  pr_info("IOCTL rcs730 Resume\n");
        count  = 6;
        tmp[0] = (unsigned char)(((unsigned short)REG__TAG_RX_CONTROL)>>8);  /* Upper */
        tmp[1] = (unsigned char)REG__TAG_RX_CONTROL;  /* Lower */
        tmp[2] = 0x01;
        tmp[3] = 0x00;
        tmp[4] = 0x00;
        tmp[5] = 0x00;
        
        /* Write data */
        retry = 2;
        do {
            retry--;
            ret = mt_i2c_master_send(rcs730_dev->client, (unsigned char *)tmp, count,0);
            if (ret == count) {
                break;
            }
            usleep_range(6000, 10000); /* Retry, chip was in standby */
        } while (retry);
        break;
    default:
        pr_err("%s bad ioctl %u\n", __func__, cmd);
        return -EINVAL;
    }

  return 0;
}

static const struct file_operations rcs730_dev_fops = {
    .owner = THIS_MODULE,
    .poll           = rcs730_dev_poll,
    .read           = rcs730_dev_read,
    .write          = rcs730_dev_write,
    .open           = rcs730_dev_open,
    .release        = rcs730_dev_release,
    .unlocked_ioctl = rcs730_dev_ioctl,
};


static int rcs730_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;
    struct rcs730_platform_data *platform_data;
    struct rcs730_dev *rcs730_dev;

//  pr_info("%s : Start\n", __func__);
    DBG_MSG("== %s called", __func__);

    //reset the Timeval for RFDet
    memset(&rfdet_tv, 0, sizeof(rfdet_tv));

    platform_data = client->dev.platform_data;
    if (platform_data == NULL) {
        dev_err(&client->dev, "nfc probe fail\n");
        return -ENODEV;
    }

    /* RFDET GPIO */
    mt_set_gpio_mode(platform_data->rfdet_gpio, GPIO_MODE_00);
    mt_set_gpio_dir(platform_data->rfdet_gpio, GPIO_DIR_IN);
    mt_set_gpio_pull_select(platform_data->rfdet_gpio, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(platform_data->rfdet_gpio, GPIO_PULL_ENABLE);

    /* IRQ GPIO */
    mt_set_gpio_mode(platform_data->irq_gpio, GPIO_MODE_00);
    mt_set_gpio_dir(platform_data->irq_gpio, GPIO_DIR_IN);
    mt_set_gpio_pull_select(platform_data->irq_gpio, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(platform_data->irq_gpio, GPIO_PULL_ENABLE);

    rcs730_dev = kzalloc(sizeof(*rcs730_dev), GFP_KERNEL);
    if (rcs730_dev == NULL) {
        dev_err(&client->dev, "failed to allocate memory for module data\n");
        ret = -ENOMEM;
        return ret;
    }

    rcs730_dev->rfdet_gpio = platform_data->rfdet_gpio;
    rcs730_dev->irq_gpio   = platform_data->irq_gpio;
    rcs730_dev->client     = client;

    init_waitqueue_head(&rcs730_dev->read_wq);
    spin_lock_init(&rcs730_dev->irq_enabled_lock);
    rcs730_dev->count_irq = 0;

    mutex_init(&rcs730_dev->read_mutex);

    I2CDMAWriteBuf = (char *)dma_alloc_coherent(NULL, MAX_BUFFER_SIZE, &I2CDMAWriteBuf_pa, GFP_KERNEL);
    if (I2CDMAWriteBuf == NULL) { 
        ret = -ENOMEM;
        goto err_exit;
    }
    I2CDMAReadBuf = (char *)dma_alloc_coherent(NULL, MAX_BUFFER_SIZE, &I2CDMAReadBuf_pa,GFP_KERNEL);
    if (I2CDMAReadBuf == NULL) { 
        ret = -ENOMEM;
        goto err_exit;
    }

    rcs730_dev->rcs730_device.minor = MISC_DYNAMIC_MINOR;
    rcs730_dev->rcs730_device.name = "rcs730";
    rcs730_dev->rcs730_device.fops = &rcs730_dev_fops;

    ret = misc_register(&rcs730_dev->rcs730_device);
    if (ret) {
        pr_err("%s : misc_register failed\n", __FILE__);
        goto err_exit;
    }
    
    i2c_set_clientdata(client, rcs730_dev);

    mt_eint_set_polarity(CUST_EINT_RFDET_SONY_DMP_RCS730_NUM,MT_POLARITY_LOW);
    mt_eint_set_sens(CUST_EINT_RFDET_SONY_DMP_RCS730_NUM,MT_EDGE_SENSITIVE);
    mt_eint_registration(CUST_EINT_RFDET_SONY_DMP_RCS730_NUM, CUST_EINT_RFDET_SONY_DMP_RCS730_TYPE, rcs730_dev_rfdet_handler,1);

    mt_eint_set_polarity(CUST_EINT_IRQ_SONY_DMP_RCS730_NUM,MT_POLARITY_LOW);
    mt_eint_set_sens(CUST_EINT_IRQ_SONY_DMP_RCS730_NUM,MT_EDGE_SENSITIVE);
    mt_eint_registration(CUST_EINT_IRQ_SONY_DMP_RCS730_NUM, CUST_EINT_IRQ_SONY_DMP_RCS730_TYPE,rcs730_dev_irq_handler,1); 

    return 0;

err_exit:
    if(I2CDMAWriteBuf != NULL){
        dma_free_coherent(NULL, MAX_BUFFER_SIZE, I2CDMAWriteBuf, I2CDMAWriteBuf_pa);
        I2CDMAWriteBuf    = NULL;
        I2CDMAWriteBuf_pa = 0;
    }

    if(I2CDMAReadBuf != NULL){
        dma_free_coherent(NULL, MAX_BUFFER_SIZE, I2CDMAReadBuf, I2CDMAReadBuf_pa);
        I2CDMAReadBuf    = NULL;
        I2CDMAReadBuf_pa = 0;
    }

    mutex_destroy(&rcs730_dev->read_mutex);
    kfree(rcs730_dev);

    return ret;
}

static int rcs730_remove(struct i2c_client *client)
{
    struct rcs730_dev *rcs730_dev;

//  pr_info("%s : Start\n", __func__);
    DBG_MSG("== %s called", __func__);

    rcs730_dev = i2c_get_clientdata(client);
    mt_eint_registration(CUST_EINT_RFDET_SONY_DMP_RCS730_NUM, CUST_EINT_RFDET_SONY_DMP_RCS730_TYPE, NULL,false);
    mt_eint_registration(CUST_EINT_IRQ_SONY_DMP_RCS730_NUM, CUST_EINT_IRQ_SONY_DMP_RCS730_TYPE,NULL,false); 
    misc_deregister(&rcs730_dev->rcs730_device);
//  Accessed after shutdown to become kernelpanic.
//    mutex_destroy(&rcs730_dev->read_mutex);
//    kfree(rcs730_dev);
    dma_free_coherent(NULL, MAX_BUFFER_SIZE, I2CDMAWriteBuf, I2CDMAWriteBuf_pa);
    I2CDMAWriteBuf = NULL;
    I2CDMAWriteBuf_pa = 0;
    dma_free_coherent(NULL, MAX_BUFFER_SIZE, I2CDMAReadBuf, I2CDMAReadBuf_pa);
    I2CDMAReadBuf = NULL;
    I2CDMAReadBuf_pa = 0;

    return 0;
}

static int rcs730_shutdown(struct i2c_client *client)
{
    DBG_MSG("== %s called", __func__);
    
    rcs730_remove(client);
    
    return 0;
}

#ifdef CONFIG_PM
static int rcs730_suspend(struct device *dev)
{

//  pr_info("rcs730_suspend start\n");
    DBG_MSG("== %s called", __func__);

    return 0;
}

static int rcs730_resume(struct device *dev)
{    
//  pr_info("rcs730_resume start\n");
    DBG_MSG("== %s called", __func__);
    
    return 0;
}

static const struct dev_pm_ops rcs730_pm_ops = {
    .suspend        = rcs730_suspend,
    .resume         = rcs730_resume,
};
#endif

static const struct i2c_device_id rcs730_id[] = {
    {MODULE_NAME, 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, rcs730_id);


static struct i2c_driver rcs730_driver = {
    .id_table = rcs730_id,
    .probe    = rcs730_probe,
    .remove   = rcs730_remove,
    .shutdown = rcs730_shutdown,
    .driver   = {
        .owner = THIS_MODULE,
        .name  =  MODULE_NAME,
#ifdef CONFIG_PM
        .pm = &rcs730_pm_ops,
#endif
    },
};

/*
 * module load/unload record keeping
 */

static int __init rcs730_dev_init(void)
{
//  pr_info("Loading rcs730 driver\n");
    return i2c_add_driver(&rcs730_driver);
}
module_init(rcs730_dev_init);

static void __exit rcs730_dev_exit(void)
{
//  pr_info("Unloading rcs730 driver\n");
    i2c_del_driver(&rcs730_driver);
}
module_exit(rcs730_dev_exit);

MODULE_AUTHOR("Sony");
MODULE_DESCRIPTION("Felica rcs730 driver");
MODULE_LICENSE("GPL");

