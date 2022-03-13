/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * Copyright 2009,2015 Sony Corporation.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/*****************/
/*@ header_files */
/*****************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/uart/mtk_uart.h>
#include <linux/uart/mtk_uart_intf.h>

#include <asm/atomic.h>
#include <asm/uaccess.h>

#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_core.h>
#include <mach/icx_cradle.h>

// #include <cust_eint.h>
#include "mt_auxadc.h"

/****************/
/*@ definitions */
/****************/

MODULE_AUTHOR("sony corporation");
MODULE_DESCRIPTION("cradle device driver");
MODULE_LICENSE("GPL");

/* debug switch */
/* #define TRACE_PRINT_ON */
/* #define VALUE_PRINT_ON  */

/* node major number (0=dynamic) */
#define MAJOR_NUMBER 0

/* accessary check delay time at queued [tick] */
#define ACCESSARY_CHECK_DELAY_TIME (10/(1000/HZ)) /* 10ms */

/* chattering check counts */
#define CHATTERING_CHECK_COUNTS 5

/* chattering check interval [tick] */
#define CHATTERING_CHECK_INTERVAL (10/(1000/HZ)) /* 10ms */

/* accessary id stable time [tick] */
#define ACCESSARY_ID_STABLE_TIME (100/(1000/HZ)) /* 100ms */

/* serial wait timeout (accessary check done) [tick] */
#define SERIAL_WAIT_TIMEOUT (300/(1000/HZ)) /* 300ms */

/* if enabled, not sleep in pcm direct */
#define ENABLE_STAY_AWAKE_MODE

/* keep alive time [ms] */
#define KEEP_ALIVE_TIME 500

/* important message level */
#define KERN_REPORT KERN_ERR//KERN_WARNING

/* general define */
#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif
#define ON  1
#define OFF 0

/*tmp*/
#define ICX_CRADLE_TXD_STATE_HI 1
#define ICX_CRADLE_TXD_STATE_LO 0

/* trace print macro */
#ifdef TRACE_PRINT_ON
#define trace(fmt, args...) printk(KERN_ERR "[icx_cradle][%d]  " fmt, __LINE__, ##args)
#else
    #define trace(fmt, args...)
#endif

/* value print macro */
#ifdef VALUE_PRINT_ON
#define value(fmt, args...) printk(KERN_ERR "[icx_cradle][%d]    " fmt, __LINE__, ##args)
#else
    #define value(fmt, args...)
#endif

/* device structure */
struct cradle_device_info {

    /* common */
    int                       node_major;
    struct cdev               cdev;
    int                       cdev_error;
    struct  class         * class_error;
    struct  device       * dev_error;
    int                       irq_error;
    struct semaphore          semaphore;
    struct workqueue_struct * workqueue;
    icx_cradle_state_t        state;
    icx_cradle_state_t        state_for_power;

    /* work queue */
    struct delayed_work work_accessary_check;
    int                 accessary_check_disabled;

    /* wait queue for poll */
    wait_queue_head_t waitqueue_poll;
    int               report_count;

    /* wait queue for serial driver */
    wait_queue_head_t waitqueue_serial;
    int               serial_wait_done;
    int txd_state;
};

/****************/
/*@ proto_types */
/****************/

/* driver_entry_routines */
static int  __init    icx_cradle_init(void);
static void __exit    icx_cradle_exit(void);
static int            icx_cradle_probe(struct platform_device * dummy);
static int   icx_cradle_remove(struct platform_device * dummy);
module_init(icx_cradle_init);
module_exit(icx_cradle_exit);


/* user_entry_routines */
static int icx_cradle_open(
    struct inode * inode,
    struct file * file
);
static int icx_cradle_release(
    struct inode * inode,
    struct file * file
);
static long icx_cradle_ioctl(
    struct file * file,
    unsigned int cmd,
    unsigned long arg
);
static u_int icx_cradle_poll(
    struct file * file,
    struct poll_table_struct * wait
);

/* susres_entry_routines */
static int  icx_cradle_cansleep(struct platform_device * dummy);
static int  icx_cradle_suspend(struct platform_device * dummy, pm_message_t state);
static int  icx_cradle_resume(struct platform_device * dummy);
static void icx_cradle_shutdown(struct platform_device *dummy);

/* other_entry_routines */
static void icx_cradle_interrupt(void);
static void icx_cradle_notify_serial_event(int type);

/* core_control_routines */
static int  queue_accessary_check_work(struct cradle_device_info * device);
static void do_accessary_check_work(struct work_struct * work);
static int  check_accessary_status(struct cradle_device_info * device);

/* basic_routines */
static void enable_rxd_interrupt(void);
static void disable_rxd_interrupt(void);
static int  get_rxd_status(void);
static int  get_accessary_id_1(int connect);
static int  get_accessary_id_2(int connect);
static void msleep_interruptible_local(int timeout);

/*********************/
/*@ global_variables */
/*********************/

static struct platform_driver Driver __refdata = {
    .driver.name =             ICX_CRADLE_NAME,
    .probe    =             icx_cradle_probe,
    .remove   =             icx_cradle_remove,
    .suspend  =             icx_cradle_suspend,
    .resume   =             icx_cradle_resume,
    .shutdown =             icx_cradle_shutdown,
};

struct file_operations FileOperations = {
    .owner   = THIS_MODULE,
    .open    = icx_cradle_open,
    .release = icx_cradle_release,
    .unlocked_ioctl   = icx_cradle_ioctl,
    .poll    = icx_cradle_poll,
};

static struct icx_cradle_driver_data *cradle_driver_static_data;

static struct cradle_device_info DeviceInfo;

static void (*notify_accessary_change)(void);

/**************************/
/*@ driver_entry_routines */
/**************************/

static int __init icx_cradle_init(void)
{
    int rv;

    trace("%s()\n",__FUNCTION__);

    printk(KERN_ERR "starting cradle driver.\n");

    rv=platform_driver_register(&Driver);
    if(rv!=0){
        printk(KERN_ERR "%s: platform_driver_register() error, rv = %d\n",__FUNCTION__,rv);
        return(rv);
    }

    return(0);
}

static void __exit icx_cradle_exit(void)
{
    trace("%s()\n",__FUNCTION__);

    platform_driver_unregister(&Driver);

    printk(KERN_INFO "stopping icx_cradle driver.\n");
}

static int icx_cradle_probe(struct platform_device * dummy)
{
    struct cradle_device_info * device;
    dev_t node;
    int rv;

    trace("%s()\n",__FUNCTION__);

    struct icx_cradle_driver_data *data = dev_get_platdata(&dummy->dev);
    cradle_driver_static_data = data;
    device=&DeviceInfo;

    /* initialize device structure */
    memset(device,0,sizeof(struct cradle_device_info));
    device->cdev_error=-1;
    device->dev_error=-1;
    device->class_error=-1;
    device->irq_error=-1;

    device->txd_state = ICX_CRADLE_TXD_STATE_HI;

    /* initialize common parameter */
    sema_init((&device->semaphore),1);
    init_waitqueue_head(&device->waitqueue_poll);
    init_waitqueue_head(&device->waitqueue_serial);

    /* allocate major number */
    if(MAJOR_NUMBER!=0){
        node=MKDEV(MAJOR_NUMBER,0);
        rv=register_chrdev_region(node,1,ICX_CRADLE_NAME);
        if(rv<0){
            printk(
                KERN_ERR
                "%s: register_chrdev_region() error, rv = %d\n",
                __FUNCTION__,
                rv
            );
            icx_cradle_remove(dummy);
            return(rv);
        }
        device->node_major=MAJOR_NUMBER;
    }
    else{
        rv=alloc_chrdev_region(&node,0,1,ICX_CRADLE_NAME);
        if(rv<0){
            printk(
                KERN_ERR
                "%s: alloc_chrdev_region() error, rv = %d\n",
                __FUNCTION__,
                rv
            );
            icx_cradle_remove(dummy);
            return(rv);
        }
        device->node_major=MAJOR(node);
    }
    printk(KERN_ERR "%s: major number = %d\n",ICX_CRADLE_NAME,device->node_major);

    /* add driver */
    cdev_init(&device->cdev,&FileOperations);
    device->cdev.owner=THIS_MODULE;
    device->cdev_error=cdev_add(&device->cdev,node,1);
    if(device->cdev_error<0){
        printk(
            KERN_ERR
            "%s: cdev_add() error, rv = %d\n",
            __FUNCTION__,
            device->cdev_error
        );
        icx_cradle_remove(dummy);
        return(device->cdev_error);
    }


    /* create node */
    device->class_error= class_create( THIS_MODULE, ICX_CRADLE_NAME );
    if(IS_ERR(device->class_error)){
        printk(
            KERN_ERR
            "%s: class_create() error, rv = %d\n",
            __FUNCTION__,
            device->class_error
        );
        icx_cradle_remove(dummy);
        return(device->class_error);
    }
    device->dev_error=device_create(device->class_error,NULL, MKDEV(device->node_major,0), NULL,ICX_CRADLE_NAME);
    if(IS_ERR(device->dev_error)){
        printk(
            KERN_ERR
            "%s: device_create() error, rv = %d\n",
            __FUNCTION__,
            device->dev_error
        );
        icx_cradle_remove(dummy);
        return(device->dev_error);
    }
    #ifdef ENABLE_STAY_AWAKE_MODE
        printk(KERN_INFO "%s: enabled stay awake mode\n",ICX_CRADLE_NAME);
    #endif

    /* initialize workqueue */
    device->workqueue=create_singlethread_workqueue("cradle_wq");
    if(!device->workqueue){
        printk(
            KERN_ERR
            "%s: create_singlethread_workqueue() error\n",
            __FUNCTION__
        );
        icx_cradle_remove(dummy);
        return(-1);
    }
    INIT_DELAYED_WORK(&device->work_accessary_check,do_accessary_check_work);
    device->accessary_check_disabled=FALSE;

    //ACC_DET
    mt_set_gpio_mode(data->acc_det_gpio, GPIO_MODE_00);
    mt_set_gpio_dir(data->acc_det_gpio, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(data->acc_det_gpio, GPIO_PULL_DISABLE);

    //ACC_DET EINT 
    mt_eint_registration(data->acc_det_eint, EINTF_TRIGGER_RISING, icx_cradle_interrupt, 1);
    mt_eint_dis_debounce(data->acc_det_eint);

    //uart_tx
    mt_set_gpio_mode(data->uart_tx_gpio, GPIO_MODE_01);
    mt_set_gpio_dir(data->uart_tx_gpio, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(data->uart_tx_gpio, GPIO_PULL_DISABLE);
    //uart_rx
    mt_set_gpio_mode(data->uart_rx_gpio, GPIO_MODE_01);
    mt_set_gpio_dir(data->uart_rx_gpio, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(data->uart_rx_gpio, GPIO_PULL_DISABLE);

    /* setup notifier from serial */
    mtk_uart_register_event_notifier(icx_cradle_notify_serial_event);

    /* check current state */
    queue_accessary_check_work(device);

    return(0);
}

static int icx_cradle_remove(struct platform_device * dummy)
{
    struct cradle_device_info * device;

    trace("%s()\n",__FUNCTION__);

    device=&DeviceInfo;

    /* disable notifier from serial */
    mtk_uart_register_event_notifier(NULL);

    /* disable interrupt */
    disable_rxd_interrupt();

    /* destroy workqueue */
    device->accessary_check_disabled=TRUE;
    if(device->workqueue){
        cancel_delayed_work(&device->work_accessary_check);
        flush_workqueue(device->workqueue);
        destroy_workqueue(device->workqueue);
        device->workqueue=NULL;
    }
    /* delete node */
    //if(device->dev_error==0){
        device_destroy(device->class_error,MKDEV(device->node_major,0));
        device->dev_error=-1;
        class_destroy(device->class_error);
    //}

    /* delete cdev */
    if(device->cdev_error==0){
        cdev_del( &device->cdev );
        device->cdev_error=-1;
    }

    /* unregister major number */
    if(device->node_major!=0){
        unregister_chrdev_region(MKDEV(device->node_major,0),1);
        device->node_major=0;
    }

    return(0);
}

/************************/
/*@ user_entry_routines */
/************************/

static int icx_cradle_open(struct inode * inode, struct file * file)
{
    struct cradle_device_info * device;

    trace("%s()\n",__FUNCTION__);

    device=container_of(inode->i_cdev,struct cradle_device_info,cdev);

    file->private_data=device;
    return(0);
}

static int icx_cradle_release(struct inode * inode, struct file * file)
{
    trace("%s()\n",__FUNCTION__);

    return(0);
}

static long icx_cradle_ioctl(
    struct file *  file,
    unsigned int   command,
    unsigned long  arg
)
{
//    extern void set_uart_break_mode(int is_on);
    struct cradle_device_info * device;
    int rv;
    int val;

    trace("%s(%u)\n",__FUNCTION__,command);
    device=file->private_data;

    if(down_interruptible(&device->semaphore))
        return(-ERESTARTSYS);

    switch(command){

        case ICX_CRADLE_GET_STATE:
            printk("%s()[%d] ICX_CRADLE_GET_STATE\n",__FUNCTION__,__LINE__);
            rv=copy_to_user(
                (void*)arg,
                (void*)&device->state,
                _IOC_SIZE(command)
            );
            if(rv){
                printk(KERN_ERR "%s: copy_to_user() error, rv = %d\n",__FUNCTION__,rv);
                up(&device->semaphore);
                return(-EFAULT);
            }

            device->report_count=device->state.count;
            value("report_count = %d\n",device->report_count);

            break;

        case ICX_CRADLE_SET_SLEEP_SIGNAL:

            rv = copy_from_user((void*)&val, (void*)arg, _IOC_SIZE(command));
            if (rv) {
                printk(KERN_ERR "%s: copy_to_user() error, rv = %d\n", __FUNCTION__, rv);
                up(&device->semaphore);
                return(-EFAULT);
            }

            if (val != 0) {
                printk(KERN_INFO "[icx_cradle] TXD LO\n");
                device->txd_state = ICX_CRADLE_TXD_STATE_LO;
                //set_uart_break_mode(1);
            } else {
                printk(KERN_INFO "[icx_cradle] TXD HI\n");
                device->txd_state = ICX_CRADLE_TXD_STATE_HI;
                //set_uart_break_mode(0);
            }
            break;

        case ICX_CRADLE_GET_SLEEP_SIGNAL:

            rv = copy_to_user((void*)arg, (void*)&device->txd_state, _IOC_SIZE(command));
            if (rv) {
                printk(KERN_ERR "%s: copy_to_user() error, rv = %d\n", __FUNCTION__, rv);
                up(&device->semaphore);
                return(-EFAULT);
            }
            break;

        case ICX_CRADLE_DISABLE:
            printk( KERN_WARNING "%s: ICX_CRADLE_DISABLE called !!!!!!!!!!!!!!!!!\n",__FUNCTION__);
            break;

        case ICX_CRADLE_ENABLE:
            printk( KERN_WARNING "%s: ICX_CRADLE_ENABLE called !!!!!!!!!!!!!!!!!\n",__FUNCTION__);
            break;

        default:
            printk( KERN_WARNING "%s: illegal request\n",__FUNCTION__);
            up(&device->semaphore);
            return(-ENOTTY);
    }

    up(&device->semaphore);

    return(0);
}

static u_int icx_cradle_poll(
    struct file * file,
    struct poll_table_struct * wait
)
{
    struct cradle_device_info * device;
    int ret=0;

    /* trace("%s() start \n",__FUNCTION__); */

    device=file->private_data;

    down(&device->semaphore);

    poll_wait(file,&device->waitqueue_poll,wait);

    if(device->report_count!=device->state.count)
        ret=POLLIN|POLLRDNORM;

    up(&device->semaphore);

    return(ret);
}

/**************************/
/*@ susres_entry_routines */
/**************************/
static unsigned char cradle_suspended=FALSE;

static int icx_cradle_cansleep(struct platform_device * dummy)
{
    struct cradle_device_info * device;

    trace("%s()\n",__FUNCTION__);

    device=&DeviceInfo;

    #ifdef ENABLE_STAY_AWAKE_MODE
        if(device->state.connect)
            return(-EBUSY);
    #endif


    return(0);
}

static int icx_cradle_suspend(struct platform_device * dummy, pm_message_t state)
{
    struct cradle_device_info * device;

    trace("%s()\n",__FUNCTION__);

    device=&DeviceInfo;

    /* wait workqueu empty */
    cancel_delayed_work(&device->work_accessary_check);
    flush_workqueue(device->workqueue);

    /* disable do_work and new queuing (no wait) */
    device->accessary_check_disabled=TRUE;

    down(&device->semaphore);

    /* anyway notify disconnect if connected */
    if(device->state.connect){
        device->state.connect = 0;
        device->state.id      = 0;
        device->state.id2     = 0;
        device->state.count++;

        wake_up_interruptible(&device->waitqueue_poll);
        printk(
            KERN_REPORT
            "%s: disconnect(sus), event count = %d\n",
            ICX_CRADLE_NAME,
            device->state.count
        );
    }

    up(&device->semaphore);

    // if(emxx_pm_get_usb_suspend_state())
    //     disable_rxd_interrupt();
    // else
    //     enable_rxd_interrupt();

    enable_rxd_interrupt();

    cradle_suspended=TRUE;


    return(0);
}

static int icx_cradle_resume(struct platform_device * dummy)
{
    struct cradle_device_info * device;

    trace("%s()\n",__FUNCTION__);

    device=&DeviceInfo;

    if(cradle_suspended){

        /* wait workqueu empty */
        cancel_delayed_work(&device->work_accessary_check);
        flush_workqueue( device->workqueue );

        /* enable new queuing */
        device->accessary_check_disabled=FALSE;

        /* current state check */
        queue_accessary_check_work(device);

        cradle_suspended=FALSE;
    }

    return(0);
}

static void icx_cradle_shutdown( struct platform_device * non )
{
    struct cradle_device_info * device;

    trace("%s()\n",__FUNCTION__);

    device=&DeviceInfo;

    /* disable do_work and new queuing (no wait) */
    device->accessary_check_disabled=TRUE;

    /* do not call interrupt handler(include kernel handler) */
    disable_rxd_interrupt();

    /* delay (wait factor on if already RX-HIGH) */
    udelay(200);

}

/*************************/
/*@ other_entry_routines */
/*************************/

static void icx_cradle_interrupt(void)
{
    struct cradle_device_info * device;

    trace("%s()\n",__FUNCTION__);

    device=&DeviceInfo;

    queue_accessary_check_work(device);

    return;
}

static void icx_cradle_notify_serial_event(int type)
{
    struct cradle_device_info * device;

    trace("%s(%d)\n",__FUNCTION__,type);

    device=&DeviceInfo;

    switch(type){

        /* framing error, break interrupt */
        case MTK_UART_EVENT_TYPE_RD_ERR:
            queue_accessary_check_work(device);
            break;

        /* user open serial */
        case MTK_UART_EVENT_TYPE_OPEN:

            /* disconnect state */
            if(!device->state.connect)
                return;

            /* connect(now) and same id */
            if(( get_rxd_status() )
            && ( device->state.id  == get_accessary_id_1(1) )
            && ( device->state.id2 == get_accessary_id_2(1) )){
                return;
            }

            /* re-check state */

            /* enable interrupt with the best of care */
            enable_rxd_interrupt();

            /* clear wait flag */
            device->serial_wait_done=FALSE;

            /* check state */
            queue_accessary_check_work(device);

            /* wait done */
            wait_event_interruptible_timeout(
                device->waitqueue_serial,
                device->serial_wait_done,
                SERIAL_WAIT_TIMEOUT
            );
            break;

        default:
            break;
    }
}

void icx_cradle_register_event_notifier(void (*func)(void))
{
    trace("%s()\n",__FUNCTION__);

    notify_accessary_change=func;
}
EXPORT_SYMBOL(icx_cradle_register_event_notifier);

int icx_cradle_get_accessary_status(icx_cradle_state_t * state)
{
    struct cradle_device_info * device;

    trace("%s()\n",__FUNCTION__);

    device=&DeviceInfo;

    if(down_interruptible(&device->semaphore))
        return(-ERESTARTSYS);

    *state=device->state_for_power;

    up(&device->semaphore);

    return(0);
}
EXPORT_SYMBOL(icx_cradle_get_accessary_status);

/**************************/
/*@ core_control_routines */
/**************************/

static int queue_accessary_check_work(struct cradle_device_info * device)
{
    trace("%s()\n",__FUNCTION__);

    if(device->accessary_check_disabled)
        return(-1);

    queue_delayed_work(
        device->workqueue,
        &device->work_accessary_check,
        ACCESSARY_CHECK_DELAY_TIME
    );

    return(0);
}

static void do_accessary_check_work(struct work_struct * work)
{
    struct cradle_device_info * device;

    trace("%s()\n",__FUNCTION__);

    device=&DeviceInfo;

    if(check_accessary_status(device)>0){
        /* retry */
        queue_accessary_check_work(device);
    }
}

static int check_accessary_status( struct cradle_device_info * device )
{
    int connect;
    int id1;
    int id2;
    int i;

    trace("%s() \n",__FUNCTION__);

    if(device->accessary_check_disabled){
        return(-1);
    }

    /* first check */
    connect = get_rxd_status();
    id1     = get_accessary_id_1(connect);
    id2     = get_accessary_id_2(connect);

    /* if same state, no actioin */
    if((device->state.connect == connect)
    && (device->state.id  == id1 )
    && (device->state.id2 == id2 )){
        value("status = %d, id1 = %d, id2 = %d\n",connect,id1,id2);

        if(device->state_for_power.connect==1 && connect==0){
            down( &device->semaphore);
            device->state_for_power.connect = 0;
            device->state_for_power.id      = 0;
            device->state_for_power.id2     = 0;
            up( &device->semaphore);
            if(notify_accessary_change!=NULL)
                notify_accessary_change();
        }

        return(0);
    }

    /* rxd, id1 and id2 are stable ? */
    for(i=0; i<CHATTERING_CHECK_COUNTS-1; i++){

        msleep_interruptible_local(CHATTERING_CHECK_INTERVAL);

        if(device->accessary_check_disabled){
            return(-1);
        } 

        /* if rxd status changed, this pulse is ignore. */
        if(connect != get_rxd_status()){
            value("status = %d, id1 = %d, id2 = %d -> ignore\n",connect,id1,id2);
            return(0); /* ignore */
        }

        if(( id1 != get_accessary_id_1(connect) )
        || ( id2 != get_accessary_id_2(connect) )){
            value("status = %d, id1 = %d, id2 = %d -> retry\n",connect,id1,id2);
            return(1); /* retry */
        }
    }

    /* if connect then disable interrupt */
    if(connect)
        disable_rxd_interrupt();
    else
        enable_rxd_interrupt();

    /* id1 and id2 are stable ? */
    if(connect){

        msleep_interruptible_local(ACCESSARY_ID_STABLE_TIME);

        if(device->accessary_check_disabled){
            return(-1);
        }

        if(( id1 != get_accessary_id_1(connect) )
        || ( id2 != get_accessary_id_2(connect) )){
            enable_rxd_interrupt();
            value("status = %d, id1 = %d, id2 = %d -> retry\n",connect,id1,id2);
            return(1); /* retry */
        }
    }

    /* update info */

    down( &device->semaphore);

    device->state.connect = connect;
    device->state.id      = id1;
    device->state.id2     = id2;
    device->state.count++;

    device->state_for_power.connect = connect;
    device->state_for_power.id      = id1;
    device->state_for_power.id2     = id2;

    if(connect){
        printk(
            KERN_REPORT
            "%s: connect, ID1 = %d, ID2 = %d, event count = %d\n",
            ICX_CRADLE_NAME,
            device->state.id,
            device->state.id2,
            device->state.count
        );
    }
    else{
        printk(
            KERN_REPORT
            "%s: disconnect, event count = %d\n",
            ICX_CRADLE_NAME,
            device->state.count
        );
    }

    up( &device->semaphore);

    /* notify poll */
    wake_up_interruptible(&device->waitqueue_poll);

    /* notify open_serial */
    wake_up_interruptible(&device->waitqueue_serial);
    device->serial_wait_done=TRUE;

    /* notify event to icx_cradle */
    if(notify_accessary_change!=NULL)
        notify_accessary_change();

    return(0);
}

/*******************/
/*@ basic_routines */
/*******************/

static void enable_rxd_interrupt()
{
    trace("%s()\n",__FUNCTION__);
    mt_eint_ack(cradle_driver_static_data->acc_det_eint);
    mt_eint_unmask(cradle_driver_static_data->acc_det_eint);
}

static void disable_rxd_interrupt()
{
    trace("%s()\n",__FUNCTION__);
    mt_eint_mask(cradle_driver_static_data->acc_det_eint);
}

static int get_rxd_status()
{
    int ret=0;

    trace("%s()\n",__FUNCTION__);

    ret = mt_get_gpio_in(cradle_driver_static_data->acc_det_gpio);
    value("rxd status = %d\n",ret);

    return(ret);
}

static int get_md_adc_val(unsigned int num)
{
    int data[4] = { 0, 0, 0, 0 };
    int val = 0;
    int ret = 0;

    ret = IMM_GetOneChannelValue(num, data, &val);

    if (ret == 0)
        return val;
    else
        return -1;
}


static int get_accessary_id_1(int connect)
{
    int val;
    int ret;

    trace("%s(%d)\n",__FUNCTION__,connect);

    /* disconnect default */
    if(!connect){
        value("id1 = %d\n",0);
        return(0);
    }

    val=get_md_adc_val(cradle_driver_static_data->adc_ch_acc1);
    if(val < 0){
        printk(KERN_ERR "%s: get_md_adc_val(id1) error, val = %d\n",__FUNCTION__,val);
        return(-1);
    }

    value("id1 ad val = %u\n",val);

         if ( val <  65 ) ret = ICX_CRADLE_ID_0; /* about 0   */
    else if ( val <  209 ) ret = ICX_CRADLE_ID_1; /* about 23  */
    else if ( val <  351 ) ret = ICX_CRADLE_ID_2; /* about 45  */
    else if ( val <  502 ) ret = ICX_CRADLE_ID_3; /* about 62  */
    else if ( val <  709 ) ret = ICX_CRADLE_ID_4; /* about 82  */
    else if ( val < 970 ) ret = ICX_CRADLE_ID_5; /* about 106 */
    else if ( val < 1281 ) ret = ICX_CRADLE_ID_6; /* about 128 */
    else if ( val < 1666 ) ret = ICX_CRADLE_ID_7; /* about 151 */
    else if ( val < 2342 ) ret = ICX_CRADLE_ID_8; /* about 174 */
    else if ( val < 3237 ) ret = ICX_CRADLE_ID_9; /* about 211 */
    else if ( val < 3982 ) ret = ICX_CRADLE_ID_A; /* about 233 */
    else                  ret = ICX_CRADLE_ID_B; /* about 255 */

    value("id1 = %d\n",ret);

    return(ret);
}

static int get_accessary_id_2(int connect)
{
    int val;
    int ret;

    trace("%s(%d)\n",__FUNCTION__,connect);

    /* disconnect default */
    if(!connect){
        value("id2 = %d\n",0);
        return(0);
    }

    val=get_md_adc_val(cradle_driver_static_data->adc_ch_acc2);
    if(val < 0){
        printk(KERN_ERR "%s: get_md_adc_val(id2) error, val = %d\n",__FUNCTION__,val);
        return(-1);
    }

    value("id2 ad val = %u\n",val);

         if ( val <  65 ) ret = ICX_CRADLE_ID2_0; /* about 0   */
    else if ( val <  209 ) ret = ICX_CRADLE_ID2_1; /* about 23  */
    else if ( val <  351 ) ret = ICX_CRADLE_ID2_2; /* about 45  */
    else if ( val <  502 ) ret = ICX_CRADLE_ID2_3; /* about 62  */
    else if ( val <  709 ) ret = ICX_CRADLE_ID2_4; /* about 82  */
    else if ( val < 970 ) ret = ICX_CRADLE_ID2_5; /* about 106 */
    else if ( val < 1281 ) ret = ICX_CRADLE_ID2_6; /* about 128 */
    else if ( val < 1666 ) ret = ICX_CRADLE_ID2_7; /* about 151 */
    else if ( val < 2342 ) ret = ICX_CRADLE_ID2_8; /* about 174 */
    else if ( val < 3237 ) ret = ICX_CRADLE_ID2_9; /* about 211 */
    else if ( val < 3982 ) ret = ICX_CRADLE_ID2_A; /* about 233 */
    else                  ret = ICX_CRADLE_ID2_B; /* about 255 */

    value("id2 = %d\n",ret);

    return(ret);
}

static void msleep_interruptible_local(int timeout)
{
    trace("%s(%d)\n",__FUNCTION__,timeout);

    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(timeout);
}

