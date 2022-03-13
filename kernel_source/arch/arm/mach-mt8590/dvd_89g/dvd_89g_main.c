/*
 * Copyright (C) 2016 SONY Corporation.  All Rights Reserved
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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/cdev.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/io.h>
#include <linux/delay.h>

#include <mach/mt_gpio.h>
#include <mach/icx_pm_helper.h>
#include "dvd_89g_main.h"

struct dvd_89g_device {
    struct cdev cdev;
    int usb_state;
};

static struct dvd_89g_device *dvd_89g_dev;
struct class *class = NULL;

extern void musb11_id_pin_sw_work(bool host_mode);

/*
 * Major & Minor
 */
static int dvd_89g_major = 0;
static int dvd_89g_minor = 0;

/* driver entry routines */
static int dvd_89g_remove(struct platform_device *pdev);
static int dvd_89g_suspend(struct platform_device *pdev, pm_message_t state);
static int dvd_89g_resume(struct platform_device *pdev);
static void dvd_89g_shutdown(struct platform_device *pdev);

/* user entry routines */
static int dvd_89g_dev_open(struct file *file);
static int dvd_89g_dev_close(struct file *file);
static long dvd_89g_dev_ioctl(struct file *filp, unsigned int cmd, unsigned int arg);

static const struct file_operations dvd_89g_dev_fops = {
  .owner          = THIS_MODULE,
  .open           = dvd_89g_dev_open,
  .release        = dvd_89g_dev_close,
  .unlocked_ioctl = dvd_89g_dev_ioctl,
};

/* Local routines */
static int dvd_89g_usb_set_switch(unsigned int val);
static int dvd_89g_hdmi_set_power(unsigned int val);
static int dvd_89g_set_reset(unsigned int val);
static int dvd_89g_set_power(unsigned int val);
static int dvd_89g_usb_get_state(unsigned int arg);
static void dvd_89g_power_config(void);
static void dvd_89g_reset_config(void);
static void dvd_89g_hdmi5v_config(void);
static void dvd_89g_usb_sel_config(void);
static void dvd_89g_uart_config(void);
static int dvd_89g_usb_vbus_on(void);

static int dvd_89g_usb_vbus_on(void)
{
    int ret = 0;
    int vbusf = 0;

    vbusf = mt_get_gpio_out( DVD_89G_USB_VBUS1);

    if(vbusf == 0){
        msleep(400);
    }else{
        overcurrent_end_processing();
        ret = mt_set_gpio_out( DVD_89G_USB_VBUS1, GPIO_LOW);
        if(ret){
            pr_err("%s : DVD_89G_USB_VBUS Error : ret= %d\n", __func__, ret);
            goto vbus_err;
        }
        msleep(500);
    }

    overcurrent_start_processing();
    ret = mt_set_gpio_out( DVD_89G_USB_VBUS1, GPIO_HIGH);
    if(ret){
        pr_err("%s : DVD_89G_USB_VBUS Error : ret= %d\n", __func__, ret);
        goto vbus_err;
    }

    msleep(200);

    overcurrent_end_processing();
    ret = mt_set_gpio_out( DVD_89G_USB_VBUS1, GPIO_LOW);
    if(ret){
        pr_err("%s : DVD_89G_USB_VBUS Error : ret= %d\n", __func__, ret);
        goto vbus_err;
    }

    msleep(1000);

    return 0;

vbus_err:
    return 1;
}

static int dvd_89g_usb_set_switch(unsigned int val)
{
    int ret = 0;

    if(val == 1){
        musb11_id_pin_sw_work(false);
        ret = mt_set_gpio_out( DVD_89G_USB_SEL, GPIO_LOW);
        if(ret){
            pr_err("%s : DVD_89G_USB_SEL : ret= %d\n", __func__, ret);
            goto usb_switch_err;
        }
    }else{
        musb11_id_pin_sw_work(true);
        ret = mt_set_gpio_out( DVD_89G_USB_SEL, GPIO_HIGH);
        if(ret){
            pr_err("%s : DVD_89G_USB_SEL : ret= %d\n", __func__, ret);
            goto usb_switch_err;
        }
    }

    ret = dvd_89g_usb_vbus_on();
    if(ret){
        pr_err("%s : DVD_89G_USB_VBUS Error : ret= %d\n", __func__, ret);
        goto usb_switch_err;
    }

    ret = mt_set_gpio_out( DVD_89G_USB_SEL, val);
    if(ret){
        pr_err("%s : DVD_89G_USB_SEL : ret= %d\n", __func__, ret);
        goto usb_switch_err;
    }
    
    msleep(1000);

    overcurrent_start_processing();
    ret = mt_set_gpio_out( DVD_89G_USB_VBUS1, GPIO_HIGH);
    if(ret){
        pr_err("%s : DVD_89G_USB_VBUS Error : ret= %d\n", __func__, ret);
        goto usb_switch_err;
    }

    dvd_89g_dev->usb_state = val;
    
    return 0;

usb_switch_err:
    return -1;
}

static int dvd_89g_start(void)
{
    int ret = 0;

    msleep(107);

    ret = dvd_89g_set_power(GPIO_HIGH);
    if(ret){
        pr_err("%s:DVD Power On Error ret = %d\n", __func__, ret);
        goto dvd_start_err;
    }

    ret = dvd_89g_hdmi_set_power(GPIO_HIGH);
    if(ret){
        pr_err("%s:HDMI Power On Error ret = %d\n", __func__, ret);
        goto dvd_start_err;
    }

    msleep(230);
    ret = dvd_89g_set_reset(GPIO_HIGH);
    if(ret){
        pr_err("%s:Reset Error ret = %d\n", __func__, ret);
        goto dvd_start_err;
    }

    return 0;

dvd_start_err:
    return -1;
}

static int dvd_89g_stop(void)
{
    int ret = 0;

    // Reset OFF
    ret = dvd_89g_set_reset(GPIO_LOW);
    if(ret){
        pr_err("%s:Reset Error ret = %d\n", __func__, ret);
        goto dvd_stop_err;
    }

    msleep(200);

    // Power OFF
    ret = dvd_89g_set_power(GPIO_LOW);
    if(ret){
        pr_err("%s:DVD Power On Error ret = %d\n", __func__, ret);
        goto dvd_stop_err;
    }

    // HDMI OFF
    ret = dvd_89g_hdmi_set_power(GPIO_LOW);
    if(ret){
        pr_err("%s:HDMI Power On Error ret = %d \n", __func__, ret);
        goto dvd_stop_err;
    }

    return 0;

dvd_stop_err:
    return -1;
}

static int dvd_89g_hdmi_set_power(unsigned int val)
{
    int ret = 0;

    /* hdmi power */
    ret = mt_set_gpio_out( DVD_89G_HDMI_5V, val);
    if(ret){
        pr_err("%s : DVD_89G_HDMI_5V : ret= %d\n", __func__, ret);
    }
    
    return ret;
}

static int dvd_89g_set_reset(unsigned int val)
{
    int ret = 0;

    /* 89g reset */
    ret = mt_set_gpio_out(DVD_89G_RESET, val);
    if(ret){
        pr_err("%s : DVD_89G_RESET : ret= %d\n", __func__, ret);
    }
    
    return ret;
}

static int dvd_89g_set_power(unsigned int val)
{
    int ret = 0;

    /* 89g Power */
    ret = mt_set_gpio_out( DVD_89G_POWER, val);
    if(ret){
        pr_err("%s : DVD_89G_POWER : ret= %d\n", __func__, ret);
    }
    
    return ret;
}

static int dvd_89g_usb_get_state(unsigned int arg)
{
    int info = 0;
    int ret = 0;
    
    /* usb state */
    info = mt_get_gpio_out(DVD_89G_USB_SEL);
    ret = copy_to_user((int *)arg, &info, sizeof(int));
    if(ret){
        pr_err("%s:copy_to_user Error\n", __func__);
        return -EFAULT;
    }
    
    return ret;
}

static int dvd_89g_dev_open(struct file *file)
{
    pr_info("%s : Start\n", __func__);
    return 0;
}

static int dvd_89g_dev_close(struct file *file)
{
    pr_info("%s : Start\n", __func__);
    return 0;
}

static long dvd_89g_dev_ioctl(struct file *filp, unsigned int cmd, unsigned int arg)
{
    int ret = 0;

    switch (cmd) {
    case DVD_89G_IOCTL_START:
        ret = dvd_89g_start();
        break;
    case DVD_89G_IOCTL_STOP:
        ret = dvd_89g_stop();
        break;
    case DVD_89G_IOCTL_USB_SWITCH:
        ret = dvd_89g_usb_set_switch(arg);
        break;
    case DVD_89G_IOCTL_USB_GET_STATE:
        ret = dvd_89g_usb_get_state(arg);
        break;
    case DVD_89G_IOCTL_RESTART:
        ret = dvd_89g_stop();
        if(ret){
            pr_err("%s: Restart stop err ret = %d\n", __func__, ret);
            return -EINVAL;
        }

        ret = dvd_89g_start();
        if(ret){
            pr_err("%s: Restart Start err ret = %d\n", __func__, ret);
            return -EINVAL;
        }
        break;
    default:
        pr_err("%s bad ioctl not command %u\n", __func__, cmd);
        return -EINVAL;
   }

    return ret;
}

static void dvd_89g_power_config(void)
{
    mt_set_gpio_pull_enable(DVD_89G_POWER, GPIO_PULL_DISABLE);
    mt_set_gpio_pull_select(DVD_89G_POWER, GPIO_PULL_DOWN);
    mt_set_gpio_dir(DVD_89G_POWER, GPIO_DIR_OUT);
    mt_set_gpio_mode(DVD_89G_POWER, GPIO_MODE_GPIO);
}

static void dvd_89g_reset_config(void)
{
    mt_set_gpio_pull_enable(DVD_89G_RESET, GPIO_PULL_DISABLE);
    mt_set_gpio_pull_select(DVD_89G_RESET, GPIO_PULL_DOWN);
    mt_set_gpio_dir(DVD_89G_RESET, GPIO_DIR_OUT);
    mt_set_gpio_mode(DVD_89G_RESET, GPIO_MODE_GPIO);
}

static void dvd_89g_hdmi5v_config(void)
{
    mt_set_gpio_pull_enable(DVD_89G_HDMI_5V, GPIO_PULL_DISABLE);
    mt_set_gpio_pull_select(DVD_89G_HDMI_5V, GPIO_PULL_DOWN);
    mt_set_gpio_dir(DVD_89G_HDMI_5V, GPIO_DIR_OUT);
    mt_set_gpio_mode(DVD_89G_HDMI_5V, GPIO_MODE_GPIO);
}

static void dvd_89g_usb_sel_config(void)
{
    mt_set_gpio_pull_enable(DVD_89G_USB_SEL, GPIO_PULL_DISABLE);
    mt_set_gpio_pull_select(DVD_89G_USB_SEL, GPIO_PULL_DOWN);
    mt_set_gpio_dir(DVD_89G_USB_SEL, GPIO_DIR_OUT);
    mt_set_gpio_mode(DVD_89G_USB_SEL, GPIO_MODE_GPIO);
}

static void dvd_89g_uart_config(void)
{
    mt_set_gpio_pull_enable(DVD_89G_UART_URXD1, GPIO_PULL_DISABLE);
    mt_set_gpio_pull_select(DVD_89G_UART_URXD1, GPIO_PULL_DOWN);
    mt_set_gpio_dir(DVD_89G_UART_URXD1, GPIO_DIR_IN);
    mt_set_gpio_ies(DVD_89G_UART_URXD1, GPIO_IES_DISABLE);
    mt_set_gpio_mode(DVD_89G_UART_URXD1, GPIO_MODE_01);
    
    mt_set_gpio_pull_enable(DVD_89G_UART_UTXD1, GPIO_PULL_DISABLE);
    mt_set_gpio_pull_select(DVD_89G_UART_UTXD1, GPIO_PULL_DOWN);
    mt_set_gpio_dir(DVD_89G_UART_UTXD1, GPIO_DIR_OUT);
    mt_set_gpio_ies(DVD_89G_UART_UTXD1, GPIO_IES_DISABLE);
    mt_set_gpio_mode(DVD_89G_UART_UTXD1, GPIO_MODE_01);
}

static int dvd_89g_probe(struct platform_device *pdev)
{
    int ret = 0;
    dev_t devid = 0;
    struct device *dev = NULL;

    pr_info("%s : Start\n", __func__);

    ret = alloc_chrdev_region(&devid, dvd_89g_minor, 1, DVD_89G_CDEV_FILE_NAME);
    if(ret != 0){
        pr_err("%s: major number Error ret = %d \n", __func__, ret);
        return ret;
    }
    dvd_89g_major = MAJOR(devid);
    
    dvd_89g_dev = kmalloc(sizeof(struct dvd_89g_device), GFP_KERNEL);
    if (dvd_89g_dev == NULL) {
        pr_err("%s:failed to allocate \n",__func__);
        ret = -ENOMEM;
        goto cdev_add_err;
    }
    
    /* initialize cdev */
    cdev_init(&dvd_89g_dev->cdev, &dvd_89g_dev_fops);
    dvd_89g_dev->cdev.owner = THIS_MODULE;
    dvd_89g_dev->cdev.ops = &dvd_89g_dev_fops;
    
    ret = cdev_add(&dvd_89g_dev->cdev, devid, 1);
    if(ret != 0){
        goto cdev_add_err;
    }
    
    class = class_create(THIS_MODULE, DVD_89G_CDEV_FILE_NAME);
    
    if (IS_ERR(class)){
       pr_err("class create error\n");
       goto cdev_add_err;
    }
    
    dev = device_create(class, NULL, devid, NULL, DVD_89G_CDEV_FILE_NAME);
    if (IS_ERR(dev)){
        pr_err(" device create Error \n");
        goto cdev_add_err;
    }
    
    dvd_89g_dev->usb_state = 0;

    /* 89G Reset */
    dvd_89g_reset_config();

    /* 89G Power */
    dvd_89g_power_config();

    /*  89G HDMI 5V */
    dvd_89g_hdmi5v_config();

    /*  89G USB SEL*/
    dvd_89g_usb_sel_config();

    /* 89G_UART */
    dvd_89g_uart_config();

    return 0;

cdev_add_err:
    kfree(dvd_89g_dev);
    unregister_chrdev_region(devid, 1);

    return -1;
}

static int dvd_89g_remove(struct platform_device *pdev)
{
    dev_t dev = 0;
    
    /* get device file */
    dev = MKDEV(dvd_89g_major, dvd_89g_minor);

    device_destroy(class, dev);
    class_destroy(class);
    cdev_del(&dvd_89g_dev->cdev);

    /* major & minor */
    unregister_chrdev_region(dev, 1);
    kfree(dvd_89g_dev);

    return 0;
}

static int dvd_89g_suspend(struct platform_device *pdev, pm_message_t state)
{
    pr_info("%s:DVD 89G Suspend \n",__func__);
    return 0;
}

static int dvd_89g_resume(struct platform_device *pdev)
{
    pr_info("%s:DVD 89G Resume \n",__func__);
    return 0;
}

static void dvd_89g_shutdown(struct platform_device *pdev)
{
    pr_info("%s : Shutdown\n", __func__);
    dvd_89g_remove(pdev);
}

static struct platform_driver dvd_89g_driver = {
    .probe    = dvd_89g_probe,
    .remove   = dvd_89g_remove,
    .suspend  = dvd_89g_suspend,
    .resume   = dvd_89g_resume,
    .shutdown = dvd_89g_shutdown,
    .driver   = {
      .name  = "dvd_89g",
      .owner  = THIS_MODULE,
    },
};

static int __init dvd_89g_init(void)
{
    pr_info("%s : Start\n", __func__);
 
    return platform_driver_register(&dvd_89g_driver);
}

static void __exit dvd_89g_exit(void)
{
    pr_info("%s : Start\n", __func__);

    platform_driver_unregister(&dvd_89g_driver);
}

module_init(dvd_89g_init);
module_exit(dvd_89g_exit);

MODULE_DESCRIPTION("dvd 89G driver");
MODULE_LICENSE("GPL");
