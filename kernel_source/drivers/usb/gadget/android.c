/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * Gadget Driver for Android
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *         Benoit Goby <benoit@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#ifdef CONFIG_ENABLE_USBCONN
#include <linux/icx_usbconn.h>
#endif
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
/* @note CONFIG_CHARGER_BQ24262_WMPORT is defined, then CONFIG_ARCH_MT8590_ICX is also defined. */
#include <linux/power/bq24262_wmport.h>
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */

/* Add for HW/SW connect */
#include <linux/musb/mtk_musb.h>
#include <linux/musb11/mtk11_musb.h>
/* Add for HW/SW connect */

#include "gadget_chips.h"

#include "f_fs.c"
//#include "f_audio_source.c"
#include "f_mass_storage.c"
#include "f_adb.c"
#include "f_mtp.c"
#include "f_accessory.c"
#define USB_ETH_RNDIS y
#include "f_rndis.c"
#include "rndis.c"
#include "u_ether.c"

MODULE_AUTHOR("Mike Lockwood");
MODULE_DESCRIPTION("Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static const char longname[] = "Gadget Android";

/* Default vendor and product IDs, overridden by userspace */
#define VENDOR_ID		0x054C
#define PRODUCT_ID		0x0B8C
#define BCD_DEVICE		0x0100


/* Default manufacturer and product string , overridden by userspace */
#define MANUFACTURER_STRING "Sony Corporation"
#define PRODUCT_STRING "Sony Audio Type-B"

#define USB_LOG "USB"

#include "f_uac2.c"

#define pr_verb(fmt, ...) \
	no_printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)

struct android_usb_function {
	char *name;
	void *config;

	struct device *dev;
	char *dev_name;
	struct device_attribute **attributes;

	/* for android_dev.enabled_functions */
	struct list_head enabled_list;

	/* Optional: initialization during gadget bind */
	int (*init)(struct android_usb_function *, struct usb_composite_dev *);
	/* Optional: cleanup during gadget unbind */
	void (*cleanup)(struct android_usb_function *);
	/* Optional: called when the function is added the list of
	 *		enabled functions */
	void (*enable)(struct android_usb_function *);
	/* Optional: called when it is removed */
	void (*disable)(struct android_usb_function *);

	int (*bind_config)(struct android_usb_function *,
			   struct usb_configuration *);

	/* Optional: called when the configuration is removed */
	void (*unbind_config)(struct android_usb_function *,
			      struct usb_configuration *);
	/* Optional: handle ctrl requests before the device is configured */
	int (*ctrlrequest)(struct android_usb_function *,
					struct usb_composite_dev *,
					const struct usb_ctrlrequest *);
};

struct android_dev {
	struct android_usb_function **functions;
	struct list_head enabled_functions;
	struct usb_composite_dev *cdev;
	struct device *dev;

	bool enabled;
	int disable_depth;
	struct mutex mutex;
	bool connected;
	bool sw_connected;
#ifdef CONFIG_ENABLE_USBCONN
#ifdef USBCONN_SUSPEND_ENABLLE
	bool suspended;
	bool resumed;
#endif
#endif
	struct work_struct work;
	char ffs_aliases[256];
	int rezero_cmd;
};

static struct class *android_class;
static struct android_dev *_android_dev;
static int android_bind_config(struct usb_configuration *c);
static void android_unbind_config(struct usb_configuration *c);
static int android_setup_config(struct usb_configuration *c, const struct usb_ctrlrequest *ctrl);

#ifdef CONFIG_ENABLE_USBCONN
static struct conn_to_gadget_func *conn_to_gadget_func_tbl = NULL;
#endif

/* string IDs are assigned dynamically */
#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

static char manufacturer_string[256];
char product_string[256];
static char serial_string[256];

/* String Table */
static struct usb_string strings_dev[] = {
	[STRING_MANUFACTURER_IDX].s = manufacturer_string,
	[STRING_PRODUCT_IDX].s = product_string,
	[STRING_SERIAL_IDX].s = serial_string,
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength              = sizeof(device_desc),
	.bDescriptorType      = USB_DT_DEVICE,
#ifdef CONFIG_USB_MU3D_DRV
	.bcdUSB               = __constant_cpu_to_le16(0x0300),
#else
	.bcdUSB               = __constant_cpu_to_le16(0x0200),
#endif
	.bDeviceClass         = USB_CLASS_PER_INTERFACE,
	.idVendor             = __constant_cpu_to_le16(VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(PRODUCT_ID),
	.bcdDevice            = __constant_cpu_to_le16(BCD_DEVICE),
	.bNumConfigurations   = 1,
};

static struct usb_configuration android_config_driver = {
	.label		= "android",
	.unbind		= android_unbind_config,
	.setup		= android_setup_config,
	.bConfigurationValue = 1,
	.bmAttributes	= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.MaxPower	= 2, /* 2ma */
};

#ifdef CONFIG_ENABLE_USBCONN
static bool notice_usbconn(struct android_dev *pdev)
{
	int set_status = 0;

#ifdef USBCONN_SUSPEND_ENABLLE
	bool continues = true;

	pr_debug("usbconn:%s(%d)start access usbconn(con:%d sus:%d res:%d func:%p)\n",
			 __FUNCTION__, __LINE__,
			 pdev->connected, pdev->suspended, pdev->resumed, conn_to_gadget_func_tbl);

	if (pdev->suspended == 1) {
		set_status = 0;
		pdev->suspended = 0;
		continues = false;
		pr_debug("usbconn:(%d) received suspend, set disconnect\n", __LINE__);
	} else if (pdev->resumed == 1) {
		set_status = 1;
		pdev->resumed = 0;
		continues = false;
		pr_debug("usbconn:(%d) received resume, set connect\n", __LINE__);
	} else if (pdev->connected) {
#else
	pr_debug("usbconn:%s(%d)start access usbconn(con:%d func:%p)\n",
			 __FUNCTION__, __LINE__, pdev->connected, conn_to_gadget_func_tbl);

	if (pdev->connected) {
#endif
		set_status = 1;
		pr_debug("usbconn:(%d) received and set connect\n", __LINE__);
	} else {
		set_status = 0;
		pr_debug("usbconn:(%d) received and set disconnect\n", __LINE__);
	}

	if (conn_to_gadget_func_tbl) {
		conn_to_gadget_func_tbl->change_status(set_status);
	} else {
		pr_debug("usbconn:(%d)not regist operation\n", __LINE__);
	}

#ifdef USBCONN_SUSPEND_ENABLLE
	pr_debug("usbconn:(%d)end access usbconn(con:%d sus:%d res:%d set:%d)\n",
			 __LINE__,
			 pdev->connected, pdev->suspended, pdev->resumed, set_status);

	return continues;
#else
	pr_debug("usbconn:(%d)end access usbconn(con:%d set:%d)\n",
			 __LINE__, pdev->connected, set_status);

	return true;
#endif
}
#endif

static void android_work(struct work_struct *data)
{
	struct android_dev *dev = container_of(data, struct android_dev, work);
	struct usb_composite_dev *cdev = dev->cdev;
	char *disconnected[2] = { "USB_STATE=DISCONNECTED", NULL };
	char *connected[2]    = { "USB_STATE=CONNECTED", NULL };
	char *configured[2]   = { "USB_STATE=CONFIGURED", NULL };

	/* Add for HW/SW connect */
	char *hwdisconnected[2] = { "USB_STATE=HWDISCONNECTED", NULL };
#if 0 // no need for AOSP branch
	char *hwconnected[2]    = { "USB_STATE=HWCONNECTED", NULL };
#endif
	/* Add for HW/SW connect */

	char *rezero_event[2] = { "USB_STATE=REZEROCMD", NULL };
	char *showcdrom_event[2] = { "USB_STATE=SHOWCDROMCMD", NULL };

	char **uevent_envp = NULL;
	char **uevent_envp_cdrom = NULL;
	unsigned long flags;
	/* Add for HW/SW connect */
	bool is_hwconnected = true;

#ifdef CONFIG_ENABLE_USBCONN
	if (dev->connected != dev->sw_connected) {
		if (notice_usbconn(dev) == false) {
			return;
		}
	}
#endif

	/* patch for ALPS00345130, if the disconnect followed by hw_disconnect, then the hw_disconnect
	will not notify the UsbDeviceManager due to that musb->g.speed == USB_SPEED_UNKNOWN*/
	if (!cdev){
		return ;
	}

	if(usb_cable_connected())
			is_hwconnected = true;
	else
			is_hwconnected = false;

	pr_verb(USB_LOG "%s: is_hwconnected=%d\n", __func__, is_hwconnected);
	/* Add for HW/SW connect */

	spin_lock_irqsave(&cdev->lock, flags);
	if (cdev->config) {
		uevent_envp = configured;
	} else if (dev->connected != dev->sw_connected) {
		uevent_envp = dev->connected ? connected : disconnected;
#if 0 // no need for AOSP branch
		//Modified for ALPS00421097, USB State conflict with BatteryChargerType
		//Cause the USB switching menu is shown but finish (unloaded) always because of the "ChargerType is AC or unknown"
		#if 0
		if(!is_hwconnected)
			uevent_envp = dev->connected ? hwconnected : hwdisconnected;
		#else
		//if "SEND the USB_STATE with hw_connected flag"
		if(!is_hwconnected)
			uevent_envp = is_hwconnected ? hwconnected : hwdisconnected;
		#endif
		//Modified for ALPS00421097, USB State conflict with BatteryChargerType
#endif
	
	} else if (!is_hwconnected) {
		uevent_envp = hwdisconnected;
	}

	dev->sw_connected = dev->connected;

	if (dev->rezero_cmd == 1) {
		uevent_envp_cdrom = rezero_event;
		dev->rezero_cmd = 0;
	} else if (dev->rezero_cmd == 2) {
		uevent_envp_cdrom = showcdrom_event;
		dev->rezero_cmd = 0;
	}

	spin_unlock_irqrestore(&cdev->lock, flags);

	if (uevent_envp) {
		kobject_uevent_env(&dev->dev->kobj, KOBJ_CHANGE, uevent_envp);
		/*pr_info(USB_LOG "%s: sent uevent %s\n",
			__func__, uevent_envp[0]);*/
	} else {
		/*pr_info(USB_LOG "%s: did not send uevent (%d %d %p)\n",
			__func__, dev->connected, dev->sw_connected, cdev->config);*/
	}

	if (uevent_envp_cdrom) {
		kobject_uevent_env(&dev->dev->kobj, KOBJ_CHANGE, uevent_envp_cdrom);
		/*pr_info(USB_LOG "%s: sent uevent %s\n", __func__, uevent_envp_cdrom[0]);*/
	} else {
		/*pr_info(USB_LOG "%s: did not send zero uevent\n", __func__);*/
	}

}

static void android_enable(struct android_dev *dev)
{
	struct usb_composite_dev *cdev = dev->cdev;

	if (WARN_ON(!dev->disable_depth))
		return;

	if (--dev->disable_depth == 0) {
		usb_add_config(cdev, &android_config_driver,
					android_bind_config);
		usb_gadget_connect(cdev->gadget);
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
/* @note CONFIG_CHARGER_BQ24262_WMPORT is defined, then CONFIG_ARCH_MT8590_ICX is also defined. */
	bq24262_wmport_usb_set_gadget_enable_event(true);
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */
	}
}

static void android_disable(struct android_dev *dev)
{
	struct usb_composite_dev *cdev = dev->cdev;

	if (dev->disable_depth++ == 0) {
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
/* @note CONFIG_CHARGER_BQ24262_WMPORT is defined, then CONFIG_ARCH_MT8590_ICX is also defined. */
		bq24262_wmport_usb_set_gadget_enable_event(false);
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */
		usb_gadget_disconnect(cdev->gadget);
		/* Cancel pending control requests */
		usb_ep_dequeue(cdev->gadget->ep0, cdev->req);
		usb_remove_config(cdev, &android_config_driver);
	}
}

/*-------------------------------------------------------------------------*/
/* Supported functions initialization */

struct functionfs_config {
	bool opened;
	bool enabled;
	struct ffs_data *data;
};

static int ffs_function_init(struct android_usb_function *f,
			     struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct functionfs_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;

	return functionfs_init();
}

static void ffs_function_cleanup(struct android_usb_function *f)
{
	functionfs_cleanup();
	kfree(f->config);
}

static void ffs_function_enable(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = f->config;

	config->enabled = true;

	/* Disable the gadget until the function is ready */
	if (!config->opened)
		android_disable(dev);
}

static void ffs_function_disable(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = f->config;

	config->enabled = false;

	/* Balance the disable that was called in closed_callback */
	if (!config->opened)
		android_enable(dev);
}

static int ffs_function_bind_config(struct android_usb_function *f,
				    struct usb_configuration *c)
{
	struct functionfs_config *config = f->config;
	return functionfs_bind_config(c->cdev, c, config->data);
}

static ssize_t
ffs_aliases_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct android_dev *dev = _android_dev;
	int ret;

	mutex_lock(&dev->mutex);
	ret = sprintf(buf, "%s\n", dev->ffs_aliases);
	mutex_unlock(&dev->mutex);

	return ret;
}

static ssize_t
ffs_aliases_store(struct device *pdev, struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct android_dev *dev = _android_dev;
	char buff[256];

	mutex_lock(&dev->mutex);

	if (dev->enabled) {
		mutex_unlock(&dev->mutex);
		return -EBUSY;
	}

	strlcpy(buff, buf, sizeof(buff));
	strlcpy(dev->ffs_aliases, strim(buff), sizeof(dev->ffs_aliases));

	mutex_unlock(&dev->mutex);

	return size;
}

static DEVICE_ATTR(aliases, S_IRUGO | S_IWUSR, ffs_aliases_show,
					       ffs_aliases_store);
static struct device_attribute *ffs_function_attributes[] = {
	&dev_attr_aliases,
	NULL
};

static struct android_usb_function ffs_function = {
	.name		= "ffs",
	.init		= ffs_function_init,
	.enable		= ffs_function_enable,
	.disable	= ffs_function_disable,
	.cleanup	= ffs_function_cleanup,
	.bind_config	= ffs_function_bind_config,
	.attributes	= ffs_function_attributes,
};

static int functionfs_ready_callback(struct ffs_data *ffs)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = ffs_function.config;
	int ret = 0;

	mutex_lock(&dev->mutex);

	ret = functionfs_bind(ffs, dev->cdev);
	if (ret)
		goto err;

	config->data = ffs;
	config->opened = true;

	if (config->enabled)
		android_enable(dev);

err:
	mutex_unlock(&dev->mutex);
	return ret;
}

static void functionfs_closed_callback(struct ffs_data *ffs)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = ffs_function.config;

	mutex_lock(&dev->mutex);

	if (config->enabled)
		android_disable(dev);

	config->opened = false;
	config->data = NULL;

	functionfs_unbind(ffs);

	mutex_unlock(&dev->mutex);
}

static void *functionfs_acquire_dev_callback(const char *dev_name)
{
	return 0;
}

static void functionfs_release_dev_callback(struct ffs_data *ffs_data)
{
}

struct adb_data {
	bool opened;
	bool enabled;
};

static int
adb_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct adb_data), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;

	return adb_setup();
}

static void adb_function_cleanup(struct android_usb_function *f)
{
	adb_cleanup();
	kfree(f->config);
}

static int
adb_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	return adb_bind_config(c);
}

static void adb_android_function_enable(struct android_usb_function *f)
{
/* This patch cause WHQL fail */
#if 0
	struct android_dev *dev = _android_dev;
	struct adb_data *data = f->config;

	data->enabled = true;

	/* Disable the gadget until adbd is ready */
	if (!data->opened)
		android_disable(dev);
#endif
}

static void adb_android_function_disable(struct android_usb_function *f)
{
/* This patch cause WHQL fail */
#if 0
	struct android_dev *dev = _android_dev;
	struct adb_data *data = f->config;

	data->enabled = false;

	/* Balance the disable that was called in closed_callback */
	if (!data->opened)
		android_enable(dev);
#endif
}

static struct android_usb_function adb_function = {
	.name		= "adb",
	.enable		= adb_android_function_enable,
	.disable	= adb_android_function_disable,
	.init		= adb_function_init,
	.cleanup	= adb_function_cleanup,
	.bind_config	= adb_function_bind_config,
};

static void adb_ready_callback(void)
{
/* This patch cause WHQL fail */
#if 0
	struct android_dev *dev = _android_dev;
	struct adb_data *data = adb_function.config;

	mutex_lock(&dev->mutex);

	data->opened = true;

	if (data->enabled)
		android_enable(dev);

	mutex_unlock(&dev->mutex);
#endif
}

static void adb_closed_callback(void)
{
/* This patch cause WHQL fail */
#if 0
	struct android_dev *dev = _android_dev;
	struct adb_data *data = adb_function.config;

	mutex_lock(&dev->mutex);

	data->opened = false;

	if (data->enabled)
		android_disable(dev);

	mutex_unlock(&dev->mutex);
#endif
}

#define MAX_ACM_INSTANCES 4
struct acm_function_config {
	int instances;
	int instances_on;
	struct usb_function *f_acm[MAX_ACM_INSTANCES];
	struct usb_function_instance *f_acm_inst[MAX_ACM_INSTANCES];
	int port_index;
};

static int
acm_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	int i;
	int ret;
	struct acm_function_config *config;

	config = kzalloc(sizeof(struct acm_function_config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;
	f->config = config;

	for (i = 0; i < MAX_ACM_INSTANCES; i++) {
		config->f_acm_inst[i] = usb_get_function_instance("acm");
		if (IS_ERR(config->f_acm_inst[i])) {
			ret = PTR_ERR(config->f_acm_inst[i]);
			goto err_usb_get_function_instance;
		}
		config->f_acm[i] = usb_get_function(config->f_acm_inst[i]);
		if (IS_ERR(config->f_acm[i])) {
			ret = PTR_ERR(config->f_acm[i]);
			goto err_usb_get_function;
		}
	}
	return 0;
err_usb_get_function_instance:
	while (i-- > 0) {
		usb_put_function(config->f_acm[i]);
err_usb_get_function:
		usb_put_function_instance(config->f_acm_inst[i]);
	}
	return ret;
}

static void acm_function_cleanup(struct android_usb_function *f)
{
	int i;
	struct acm_function_config *config = f->config;

	for (i = 0; i < MAX_ACM_INSTANCES; i++) {
		usb_put_function(config->f_acm[i]);
		usb_put_function_instance(config->f_acm_inst[i]);
	}
	kfree(f->config);
	f->config = NULL;
}

static int
acm_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	int i;
	int ret = 0;
	struct acm_function_config *config = f->config;

	config->instances_on = config->instances;
	for (i = 0; i < config->instances_on; i++) {
		ret = usb_add_function(c, config->f_acm[i]);
		if (ret) {
			pr_err("Could not bind acm%u config\n", i);
			goto err_usb_add_function;
		}
	}

	return 0;

err_usb_add_function:
	while (i-- > 0)
		usb_remove_function(c, config->f_acm[i]);
	return ret;
}

static void acm_function_unbind_config(struct android_usb_function *f,
				       struct usb_configuration *c)
{
	int i;
	struct acm_function_config *config = f->config;

	/*for (i = 0; i < config->instances_on; i++)
		usb_remove_function(c, config->f_acm[i]);*/
}

static ssize_t acm_instances_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct acm_function_config *config = f->config;
	return sprintf(buf, "%d\n", config->instances);
}

static ssize_t acm_instances_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct acm_function_config *config = f->config;
	int value;

	sscanf(buf, "%d", &value);
	if (value > MAX_ACM_INSTANCES)
		value = MAX_ACM_INSTANCES;
	config->instances = value;
	return size;
}

static DEVICE_ATTR(instances, S_IRUGO | S_IWUSR, acm_instances_show,
						 acm_instances_store);

static ssize_t acm_port_index_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct acm_function_config *config = f->config;
	return sprintf(buf, "%d\n", config->port_index);
}

static ssize_t acm_port_index_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct acm_function_config *config = f->config;
	int value, ret;

	ret = sscanf(buf, "%d", &value);
	config->port_index = (ret != 1 || value > MAX_ACM_INSTANCES) ? 0 : value;

	return size;
}

static DEVICE_ATTR(port_index, S_IRUGO | S_IWUSR, acm_port_index_show,
						 acm_port_index_store);

static struct device_attribute *acm_function_attributes[] = {
	&dev_attr_instances,
	&dev_attr_port_index, /*Only open the specific port*/
	NULL
};

static struct android_usb_function acm_function = {
	.name		= "acm",
	.init		= acm_function_init,
	.cleanup	= acm_function_cleanup,
	.bind_config	= acm_function_bind_config,
	.unbind_config	= acm_function_unbind_config,
	.attributes	= acm_function_attributes,
};


static int
mtp_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	return mtp_setup();
}

static void mtp_function_cleanup(struct android_usb_function *f)
{
	mtp_cleanup();
}

static int
mtp_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	return mtp_bind_config(c, false);
}

static int
ptp_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	/* nothing to do - initialization is handled by mtp_function_init */
	return 0;
}

static void ptp_function_cleanup(struct android_usb_function *f)
{
	/* nothing to do - cleanup is handled by mtp_function_cleanup */
}

static int
ptp_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	return mtp_bind_config(c, true);
}

static int mtp_function_ctrlrequest(struct android_usb_function *f,
					struct usb_composite_dev *cdev,
					const struct usb_ctrlrequest *c)
{
	return mtp_ctrlrequest(cdev, c);
}

static struct android_usb_function mtp_function = {
	.name		= "mtp",
	.init		= mtp_function_init,
	.cleanup	= mtp_function_cleanup,
	.bind_config	= mtp_function_bind_config,
	.ctrlrequest	= mtp_function_ctrlrequest,
};

/* PTP function is same as MTP with slightly different interface descriptor */
static struct android_usb_function ptp_function = {
	.name		= "ptp",
	.init		= ptp_function_init,
	.cleanup	= ptp_function_cleanup,
	.bind_config	= ptp_function_bind_config,
};


struct rndis_function_config {
	u8      ethaddr[ETH_ALEN];
	u32     vendorID;
	char	manufacturer[256];
	/* "Wireless" RNDIS; auto-detected by Windows */
	bool	wceis;
	struct eth_dev *dev;
};

static int
rndis_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct rndis_function_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;
	return 0;
}

static void rndis_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
	f->config = NULL;
}

static int
rndis_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	int ret;
	struct eth_dev *dev;
	struct rndis_function_config *rndis = f->config;

	pr_debug(USB_LOG "%s:\n", __func__);

	if (!rndis) {
		pr_err("%s: rndis_pdata\n", __func__);
		return -1;
	}

	pr_info("%s MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
		rndis->ethaddr[0], rndis->ethaddr[1], rndis->ethaddr[2],
		rndis->ethaddr[3], rndis->ethaddr[4], rndis->ethaddr[5]);

	dev = gether_setup_name(c->cdev->gadget, rndis->ethaddr, "rndis");
	if (IS_ERR(dev)) {
		ret = PTR_ERR(dev);
		pr_err("%s: gether_setup failed\n", __func__);
		return ret;
	}
	rndis->dev = dev;

	if (rndis->wceis) {
		/* "Wireless" RNDIS; auto-detected by Windows */
		rndis_iad_descriptor.bFunctionClass =
						USB_CLASS_WIRELESS_CONTROLLER;
		rndis_iad_descriptor.bFunctionSubClass = 0x01;
		rndis_iad_descriptor.bFunctionProtocol = 0x03;
		rndis_control_intf.bInterfaceClass =
						USB_CLASS_WIRELESS_CONTROLLER;
		rndis_control_intf.bInterfaceSubClass =	 0x01;
		rndis_control_intf.bInterfaceProtocol =	 0x03;
	}

	return rndis_bind_config_vendor(c, rndis->ethaddr, rndis->vendorID,
					   rndis->manufacturer, rndis->dev);
}

static void rndis_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct rndis_function_config *rndis = f->config;
	gether_cleanup(rndis->dev);
}

static ssize_t rndis_manufacturer_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	return sprintf(buf, "%s\n", config->manufacturer);
}

static ssize_t rndis_manufacturer_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;

	if (size >= sizeof(config->manufacturer))
		return -EINVAL;
	if (sscanf(buf, "%s", config->manufacturer) == 1)
		return size;
	return -1;
}

static DEVICE_ATTR(manufacturer, S_IRUGO | S_IWUSR, rndis_manufacturer_show,
						    rndis_manufacturer_store);

static ssize_t rndis_wceis_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	return sprintf(buf, "%d\n", config->wceis);
}

static ssize_t rndis_wceis_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%d", &value) == 1) {
		config->wceis = value;
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(wceis, S_IRUGO | S_IWUSR, rndis_wceis_show,
					     rndis_wceis_store);

static ssize_t rndis_ethaddr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *rndis = f->config;
	return sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		rndis->ethaddr[0], rndis->ethaddr[1], rndis->ethaddr[2],
		rndis->ethaddr[3], rndis->ethaddr[4], rndis->ethaddr[5]);
}

static ssize_t rndis_ethaddr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *rndis = f->config;

	if (sscanf(buf, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		    (int *)&rndis->ethaddr[0], (int *)&rndis->ethaddr[1],
		    (int *)&rndis->ethaddr[2], (int *)&rndis->ethaddr[3],
		    (int *)&rndis->ethaddr[4], (int *)&rndis->ethaddr[5]) == 6)
		return size;
	return -EINVAL;
}

static DEVICE_ATTR(ethaddr, S_IRUGO | S_IWUSR, rndis_ethaddr_show,
					       rndis_ethaddr_store);

static ssize_t rndis_vendorID_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	return sprintf(buf, "%04x\n", config->vendorID);
}

static ssize_t rndis_vendorID_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%04x", &value) == 1) {
		config->vendorID = value;
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(vendorID, S_IRUGO | S_IWUSR, rndis_vendorID_show,
						rndis_vendorID_store);

static struct device_attribute *rndis_function_attributes[] = {
	&dev_attr_manufacturer,
	&dev_attr_wceis,
	&dev_attr_ethaddr,
	&dev_attr_vendorID,
	NULL
};

static struct android_usb_function rndis_function = {
	.name		= "rndis",
	.init		= rndis_function_init,
	.cleanup	= rndis_function_cleanup,
	.bind_config	= rndis_function_bind_config,
	.unbind_config	= rndis_function_unbind_config,
	.attributes	= rndis_function_attributes,
};


struct mass_storage_function_config {
	struct fsg_config fsg;
	struct fsg_common *common;
};

static int mass_storage_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	struct mass_storage_function_config *config;
	struct fsg_common *common;
	int err;
	int i;

	config = kzalloc(sizeof(struct mass_storage_function_config),
								GFP_KERNEL);
	if (!config)
		return -ENOMEM;

#if CONFIG_ARCH_MT8590_ICX
#define NLUN_STORAGE 8
#else
#ifdef MTK_MULTI_STORAGE_SUPPORT
#ifdef MTK_SHARED_SDCARD
#define NLUN_STORAGE 1
#else
#define NLUN_STORAGE 2
#endif
#else
#define NLUN_STORAGE 1
#endif
#endif

	config->fsg.nluns = NLUN_STORAGE;

	for (i = 0; i < config->fsg.nluns; i++) {
		config->fsg.luns[i].removable = 1;
		config->fsg.luns[i].nofua = 1;
	}

	common = fsg_common_init(NULL, cdev, &config->fsg);
	if (IS_ERR(common)) {
		kfree(config);
		return PTR_ERR(common);
	}

	err = sysfs_create_link(&f->dev->kobj,
				&common->luns[0].dev.kobj,
				"lun");
	if (err) {
		kfree(config);
		return err;
	}

	/*
	 * "i" starts from "1", cuz dont want to change the naming of
	 * the original path of "lun0".
	 */
	for (i = 1; i < config->fsg.nluns; i++) {
		char string_lun[5] = {0};

		sprintf(string_lun, "lun%d", i);

		err = sysfs_create_link(&f->dev->kobj,
				&common->luns[i].dev.kobj,
				string_lun);
		if (err) {
			kfree(config);
			return err;
		}
	}

	config->common = common;
	f->config = config;
	return 0;
}

static void mass_storage_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
	f->config = NULL;
}

static int mass_storage_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct mass_storage_function_config *config = f->config;
	return fsg_bind_config(c->cdev, c, config->common);
}

static ssize_t mass_storage_inquiry_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	return sprintf(buf, "%s\n", config->common->inquiry_string);
}

static ssize_t mass_storage_inquiry_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	if (size >= sizeof(config->common->inquiry_string))
		return -EINVAL;
	if (sscanf(buf, "%s", config->common->inquiry_string) != 1)
		return -EINVAL;
	return size;
}

static DEVICE_ATTR(inquiry_string, S_IRUGO | S_IWUSR,
					mass_storage_inquiry_show,
					mass_storage_inquiry_store);

#ifdef CONFIG_ARCH_MT8590_ICX
static ssize_t mass_storage_nluns_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	return snprintf(buf, 3, "%1d\n", config->common->nluns);
}

static ssize_t mass_storage_nluns_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	if (size >= sizeof(config->common->nluns))
		return -EINVAL;
	if (sscanf(buf, "%1d", &config->common->nluns) != 1)
		return -EINVAL;
	if (config->common->nluns > config->fsg.nluns)
		config->common->nluns = config->fsg.nluns;
	return size;
}

static DEVICE_ATTR(nluns, S_IRUGO | S_IWUSR,
					mass_storage_nluns_show,
					mass_storage_nluns_store);
#endif

#ifdef MTK_BICR_SUPPORT

static ssize_t mass_storage_bicr_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	return sprintf(buf, "%d\n", config->common->bicr);
}

static ssize_t mass_storage_bicr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	if (size >= sizeof(config->common->bicr))
		return -EINVAL;
	if (sscanf(buf, "%d", &config->common->bicr) != 1)
		return -EINVAL;

	/* Set Lun[0] is a CDROM when enable bicr.*/
	if (!strcmp(buf, "1"))
		config->common->luns[0].cdrom = 1;
	else {
		/*Reset the value. Clean the cdrom's parameters*/
		config->common->luns[0].cdrom = 0;
		config->common->luns[0].blkbits = 0;
		config->common->luns[0].blksize = 0;
		config->common->luns[0].num_sectors = 0;
	}

	return size;
}

static DEVICE_ATTR(bicr, S_IRUGO | S_IWUSR,
					mass_storage_bicr_show,
					mass_storage_bicr_store);

#endif

static struct device_attribute *mass_storage_function_attributes[] = {
	&dev_attr_inquiry_string,
#ifdef MTK_BICR_SUPPORT
	&dev_attr_bicr,
#endif
#ifdef CONFIG_ARCH_MT8590_ICX
	&dev_attr_nluns,
#endif
	NULL
};

static struct android_usb_function mass_storage_function = {
	.name		= "mass_storage",
	.init		= mass_storage_function_init,
	.cleanup	= mass_storage_function_cleanup,
	.bind_config	= mass_storage_function_bind_config,
	.attributes	= mass_storage_function_attributes,
};


static int accessory_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	return acc_setup();
}

static void accessory_function_cleanup(struct android_usb_function *f)
{
	acc_cleanup();
}

static int accessory_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	return acc_bind_config(c);
}

static int accessory_function_ctrlrequest(struct android_usb_function *f,
						struct usb_composite_dev *cdev,
						const struct usb_ctrlrequest *c)
{
	return acc_ctrlrequest(cdev, c);
}

static struct android_usb_function accessory_function = {
	.name		= "accessory",
	.init		= accessory_function_init,
	.cleanup	= accessory_function_cleanup,
	.bind_config	= accessory_function_bind_config,
	.ctrlrequest	= accessory_function_ctrlrequest,
};

#if 0
static int audio_source_function_init(struct android_usb_function *f,
			struct usb_composite_dev *cdev)
{
	struct audio_source_config *config;

	config = kzalloc(sizeof(struct audio_source_config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;
	config->card = -1;
	config->device = -1;
	f->config = config;
	return 0;
}

static void audio_source_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
}

static int audio_source_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct audio_source_config *config = f->config;

	return audio_source_bind_config(c, config);
}

static void audio_source_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct audio_source_config *config = f->config;

	config->card = -1;
	config->device = -1;
}

static ssize_t audio_source_pcm_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct audio_source_config *config = f->config;

	/* print PCM card and device numbers */
	return sprintf(buf, "%d %d\n", config->card, config->device);
}

static DEVICE_ATTR(pcm, S_IRUGO, audio_source_pcm_show, NULL);

static struct device_attribute *audio_source_function_attributes[] = {
	&dev_attr_pcm,
	NULL
};

static struct android_usb_function audio_source_function = {
	.name		= "audio_source",
	.init		= audio_source_function_init,
	.cleanup	= audio_source_function_cleanup,
	.bind_config	= audio_source_function_bind_config,
	.unbind_config	= audio_source_function_unbind_config,
	.attributes	= audio_source_function_attributes,
};
#endif

static int audio_function_init(struct android_usb_function *f,
			struct usb_composite_dev *cdev)
{
	pr_err(USB_LOG "init");
	
	return 0;
}
static void audio_function_cleanup(struct android_usb_function *f)
{
	pr_err(USB_LOG "cleanup");
}

static int audio_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	pr_err(USB_LOG "bind interface=%d\n",c->next_interface_id);
	
	return audio_bind_config(c);
}

static void audio_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{

	pr_err(USB_LOG "unbind");

	uac2_unbind_config(c);
}

static ssize_t
audio_f_plus_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return snprintf(buf, 3, "%1d\n", f_plus);
}

static ssize_t
audio_f_plus_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int value;
	if (sscanf(buf, "%1d", &value) == 1) {
		if (value <= 5)
		{
			f_plus = value;
		}
		return size;
	}
	return -1;
}

static DEVICE_ATTR(f_plus, S_IRUGO | S_IWUSR , audio_f_plus_show, audio_f_plus_store);


static ssize_t
audio_f_minus_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return snprintf(buf, 3, "%1d\n", f_minus);
}

static ssize_t
audio_f_minus_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	unsigned int value;
	if (sscanf(buf, "%1d", &value) == 1) {
		if (value <= 5)
		{
			f_minus = value;
		}
		return size;
	}
	return -1;
}

static DEVICE_ATTR(f_minus, S_IRUGO | S_IWUSR , audio_f_minus_show, audio_f_minus_store);


static ssize_t
audio_f_thresh_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return snprintf(buf, 5, "%3d\n", f_thresh);
}

static ssize_t
audio_f_thresh_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	unsigned int value;
	if (sscanf(buf, "%3d", &value) <= 3) {
		if (value <= 100)
		{
			f_thresh = value;
		}
		return size;
	}
	return -1;
}

static DEVICE_ATTR(f_thresh, S_IRUGO | S_IWUSR , audio_f_thresh_show, audio_f_thresh_store);

static ssize_t
audio_f_start_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return snprintf(buf, 6, "%4d\n", f_start);
}

static ssize_t
audio_f_start_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	unsigned int value;
	if (sscanf(buf, "%4d", &value) <= 4) {
		if (value <= 1000)
		{
			f_start = value;
		}
		return size;
	}
	return -1;
}

static DEVICE_ATTR(f_start, S_IRUGO | S_IWUSR , audio_f_start_show, audio_f_start_store);

static struct device_attribute *audio_function_attributes[] = {
	&dev_attr_f_plus,
	&dev_attr_f_minus,
	&dev_attr_f_thresh,
	&dev_attr_f_start,
	NULL
};

static struct android_usb_function audio_function = {
	.name		= "audio_func",
	.init		= audio_function_init,
	.cleanup	= audio_function_cleanup,
	.bind_config	= audio_function_bind_config,
	.unbind_config	= audio_function_unbind_config,
	.attributes	= audio_function_attributes,
};

static struct android_usb_function *supported_functions[] = {
	&ffs_function,
	&adb_function,
	&acm_function,
	&mtp_function,
	&ptp_function,
	&rndis_function,
	&mass_storage_function,
	&accessory_function,
	&audio_function,
	//&audio_source_function,
	NULL
};


static int android_init_functions(struct android_usb_function **functions,
				  struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct android_usb_function *f;
	struct device_attribute **attrs;
	struct device_attribute *attr;
	int err;
	int index = 0;

	for (; (f = *functions++); index++) {
		f->dev_name = kasprintf(GFP_KERNEL, "f_%s", f->name);
		/* Added for USB Develpment debug, more log for more debuging help */
		pr_debug(USB_LOG "%s: f->dev_name = %s, f->name = %s\n",
			 __func__, f->dev_name, f->name);
		/* Added for USB Develpment debug, more log for more debuging help */
		f->dev = device_create(android_class, dev->dev,
				MKDEV(0, index), f, f->dev_name);
		if (IS_ERR(f->dev)) {
			pr_err("%s: Failed to create dev %s", __func__,
							f->dev_name);
			err = PTR_ERR(f->dev);
			goto err_create;
		}

		if (f->init) {
			err = f->init(f, cdev);
			if (err) {
				pr_err("%s: Failed to init %s", __func__,
								f->name);
				goto err_out;
			} else {
				pr_debug(USB_LOG "%s: init %s success!!\n",
					 __func__, f->name);
			}
		}

		attrs = f->attributes;
		if (attrs) {
			while ((attr = *attrs++) && !err)
				err = device_create_file(f->dev, attr);
		}
		if (err) {
			pr_err("%s: Failed to create function %s attributes",
					__func__, f->name);
			goto err_out;
		}
	}
	return 0;

err_out:
	device_destroy(android_class, f->dev->devt);
err_create:
	kfree(f->dev_name);
	return err;
}

static void android_cleanup_functions(struct android_usb_function **functions)
{
	struct android_usb_function *f;

	while (*functions) {
		f = *functions++;

		if (f->dev) {
			device_destroy(android_class, f->dev->devt);
			kfree(f->dev_name);
		}

		if (f->cleanup)
			f->cleanup(f);
	}
}

static int
android_bind_enabled_functions(struct android_dev *dev,
			       struct usb_configuration *c)
{
	struct android_usb_function *f;
	int ret;

	/* Added for USB Develpment debug, more log for more debuging help */
	pr_debug(USB_LOG "%s: ", __func__);
	/* Added for USB Develpment debug, more log for more debuging help */

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		pr_debug("bind_config function '%s'/%p\n", f->name, f);
		ret = f->bind_config(f, c);
		if (ret) {
			pr_err("%s: %s failed", __func__, f->name);
			return ret;
		}
	}
	return 0;
}

static void
android_unbind_enabled_functions(struct android_dev *dev,
			       struct usb_configuration *c)
{
	struct android_usb_function *f;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		pr_debug("unbind_config function '%s'/%p\n", f->name, f);
		if (f->unbind_config)
			f->unbind_config(f, c);
	}
}

static int android_enable_function(struct android_dev *dev, char *name)
{
	struct android_usb_function **functions = dev->functions;
	struct android_usb_function *f;
	while ((f = *functions++)) {

		/* Added for USB Develpment debug, more log for more debuging help */
		pr_debug(USB_LOG "%s: name = %s, f->name=%s\n",
			 __func__, name, f->name);
		/* Added for USB Develpment debug, more log for more debuging help */
		if (!strcmp(name, f->name)) {
			list_add_tail(&f->enabled_list,
						&dev->enabled_functions);
			return 0;
		}
	}
	return -EINVAL;
}

/*-------------------------------------------------------------------------*/
/* /sys/class/android_usb/android%d/ interface */

static ssize_t
functions_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct android_usb_function *f;
	char *buff = buf;

	/* Added for USB Develpment debug, more log for more debuging help */
	pr_debug(USB_LOG "%s: ", __func__);
	/* Added for USB Develpment debug, more log for more debuging help */

	mutex_lock(&dev->mutex);

	list_for_each_entry(f, &dev->enabled_functions, enabled_list)
		buff += sprintf(buff, "%s,", f->name);

	mutex_unlock(&dev->mutex);

	if (buff != buf)
		*(buff-1) = '\n';
	return buff - buf;
}

static ssize_t
functions_store(struct device *pdev, struct device_attribute *attr,
			       const char *buff, size_t size)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	char *name;
	char buf[256], *b;
	char aliases[256], *a;
	int err;
	int is_ffs;
	int ffs_enabled = 0;

	mutex_lock(&dev->mutex);

	if (dev->enabled) {
		mutex_unlock(&dev->mutex);
		return -EBUSY;
	}

	INIT_LIST_HEAD(&dev->enabled_functions);

	/* Added for USB Develpment debug, more log for more debuging help */
	pr_debug(USB_LOG "%s:\n", __func__);
	/* Added for USB Develpment debug, more log for more debuging help */

	strlcpy(buf, buff, sizeof(buf));
	b = strim(buf);

	while (b) {
		name = strsep(&b, ",");

		/* Added for USB Develpment debug, more log for more debuging help */
		/*pr_info(USB_LOG "%s: name = %s\n", __func__, name);*/
		/* Added for USB Develpment debug, more log for more debuging help */

		if (!name)
			continue;

		is_ffs = 0;
		strlcpy(aliases, dev->ffs_aliases, sizeof(aliases));
		a = aliases;

		while (a) {
			char *alias = strsep(&a, ",");
			if (alias && !strcmp(name, alias)) {
				is_ffs = 1;
				break;
			}
		}

		if (is_ffs) {
			if (ffs_enabled)
				continue;
			err = android_enable_function(dev, "ffs");
			if (err)
				pr_err("android_usb: Cannot enable ffs (%d)",
									err);
			else
				ffs_enabled = 1;
			continue;
		}

		err = android_enable_function(dev, name);
		if (err)
			pr_err("android_usb: Cannot enable '%s' (%d)",
							   name, err);
	}

	mutex_unlock(&dev->mutex);

	return size;
}

static ssize_t enable_show(struct device *pdev, struct device_attribute *attr,
			   char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	return sprintf(buf, "%d\n", dev->enabled);
}

static ssize_t enable_store(struct device *pdev, struct device_attribute *attr,
			    const char *buff, size_t size)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct usb_composite_dev *cdev = dev->cdev;
	struct android_usb_function *f;
	int enabled = 0;


	if (!cdev)
		return -ENODEV;

	mutex_lock(&dev->mutex);

	/* Added for USB Develpment debug, more log for more debuging help */
	pr_debug(USB_LOG "%s: device_attr->attr.name: %s\n", __func__, attr->attr.name);
	/* Added for USB Develpment debug, more log for more debuging help */

	sscanf(buff, "%d", &enabled);
	if (enabled && !dev->enabled) {
		/*
		 * Update values in composite driver's copy of
		 * device descriptor.
		 */
		cdev->desc.idVendor = device_desc.idVendor;
		cdev->desc.idProduct = device_desc.idProduct;
		cdev->desc.bcdDevice = device_desc.bcdDevice;
		cdev->desc.bDeviceClass = device_desc.bDeviceClass;
		cdev->desc.bDeviceSubClass = device_desc.bDeviceSubClass;
		cdev->desc.bDeviceProtocol = device_desc.bDeviceProtocol;

		/* special case for meta mode */
		if (serial_string[0] == 0x0) {
			cdev->desc.iSerialNumber = 0;
		} else {
			cdev->desc.iSerialNumber = device_desc.iSerialNumber;
		}

		list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
			pr_debug("enable function '%s'/%p\n", f->name, f);
			if (f->enable)
				f->enable(f);
		}
		android_enable(dev);
		dev->enabled = true;

		/* Added for USB Develpment debug, more log for more debuging help */
		/*pr_info(USB_LOG "%s: enable 0->1 case, device_desc.idVendor = 0x%x, device_desc.idProduct = 0x%x,\n",
			__func__, device_desc.idVendor, device_desc.idProduct);*/
		/* Added for USB Develpment debug, more log for more debuging help */

	} else if (!enabled && dev->enabled) {

		/* Added for USB Develpment debug, more log for more debuging help */
		/*pr_info(USB_LOG "%s: enable 1->0 case, device_desc.idVendor = 0x%x, device_desc.idProduct = 0x%x,\n",
			__func__, device_desc.idVendor, device_desc.idProduct);*/
		/* Added for USB Develpment debug, more log for more debuging help */

		android_disable(dev);
		list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
			if (f->disable)
				f->disable(f);
		}
		dev->enabled = false;
	} else {
		/*pr_err("android_usb: already %s\n",
				dev->enabled ? "enabled" : "disabled");*/
		/* Add for HW/SW connect */
		if (!usb_cable_connected()) {
			schedule_work(&dev->work);
			pr_verb(USB_LOG "%s: enable 0->0 case - no usb cable", __func__);
		}
		/* Add for HW/SW connect */
	}

	mutex_unlock(&dev->mutex);
	return size;
}

static ssize_t state_show(struct device *pdev, struct device_attribute *attr,
			   char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct usb_composite_dev *cdev = dev->cdev;
	char *state = "DISCONNECTED";
	unsigned long flags;

	if (!cdev)
		goto out;

	spin_lock_irqsave(&cdev->lock, flags);
	if (cdev->config)
		state = "CONFIGURED";
	else if (dev->connected)
		state = "CONNECTED";
	spin_unlock_irqrestore(&cdev->lock, flags);
out:
	return sprintf(buf, "%s\n", state);
}

#define DESCRIPTOR_ATTR(field, format_string)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return sprintf(buf, format_string, device_desc.field);		\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)				\
{									\
	int value;							\
	if (sscanf(buf, format_string, &value) == 1) {			\
		device_desc.field = value;				\
		return size;						\
	}								\
	return -1;							\
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

#define DESCRIPTOR_STRING_ATTR(field, buffer)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return sprintf(buf, "%s\n", buffer);				\
}									\
static ssize_t								\ 
field ## _store(struct device *dev, struct device_attribute *attr,	\ 
		const char *buf, size_t size)				\ 
{									\ 
	char buff[256];							\ 
	if (size >= sizeof(buffer))					\ 
		return -EINVAL;						\ 
	strlcpy(buff, buf, sizeof(buff));				\ 
	strlcpy(buffer, strim(buff), sizeof(buffer));			\ 
	return size;							\ 
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

#define DESCRIPTOR_POW(field, format_string)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return sprintf(buf, format_string, android_config_driver.field);		\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)				\
{									\
	char buff[256];					\
	if (size >= sizeof(buffer))					\
		return -EINVAL;						\
	strlcpy(buff, buf, sizeof(buff));		\
	strlcpy(buffer, strim(buff), sizeof(buffer));	\
	return size;						\
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

#ifdef CONFIG_ARCH_MT8590_ICX
static ssize_t
bmAttributes_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	int value=0;
	if (android_config_driver.bmAttributes & USB_CONFIG_ATT_SELFPOWER) value = 1;
	return snprintf(buf, 3, "%1d\n", value);
}

static ssize_t
bmAttributes_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct android_dev *pdev = _android_dev;

	int value;
	if (sscanf(buf, "%1d", &value) == 1) {
		if (value == 0)
		{
			android_config_driver.bmAttributes = USB_CONFIG_ATT_ONE;
			usb_gadget_clear_selfpowered(pdev->cdev->gadget);
		}
		else
		{
			android_config_driver.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
			usb_gadget_set_selfpowered(pdev->cdev->gadget);
		}

		return size;
	}
	return -1;
}
static DEVICE_ATTR(SelfPowered, S_IRUGO | S_IWUSR, bmAttributes_show, bmAttributes_store);

static ssize_t
MaxPower_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return snprintf(buf, 5, "%3d\n", android_config_driver.MaxPower);
}

static ssize_t
MaxPower_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int value;
	if (sscanf(buf, "%3d", &value) <= 3) {
		if ((value >= 500) || (value < 0)) {
			value = 500;
		}
		android_config_driver.MaxPower = value;
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
		bq24262_wmport_usb_set_max_power_event(value);
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */
		return size;
	}
	return -1;
}
static DEVICE_ATTR(MaxPower, S_IRUGO | S_IWUSR, MaxPower_show, MaxPower_store);
#endif

DESCRIPTOR_ATTR(idVendor, "%04x\n")
DESCRIPTOR_ATTR(idProduct, "%04x\n")
DESCRIPTOR_ATTR(bcdDevice, "%04x\n")
DESCRIPTOR_ATTR(bDeviceClass, "%d\n")
DESCRIPTOR_ATTR(bDeviceSubClass, "%d\n")
DESCRIPTOR_ATTR(bDeviceProtocol, "%d\n")
DESCRIPTOR_STRING_ATTR(iManufacturer, manufacturer_string)
DESCRIPTOR_STRING_ATTR(iProduct, product_string)
DESCRIPTOR_STRING_ATTR(iSerial, serial_string)

static DEVICE_ATTR(functions, S_IRUGO | S_IWUSR, functions_show,
						 functions_store);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, enable_show, enable_store);
static DEVICE_ATTR(state, S_IRUGO, state_show, NULL);

static struct device_attribute *android_usb_attributes[] = {
	&dev_attr_idVendor,
	&dev_attr_idProduct,
	&dev_attr_bcdDevice,
	&dev_attr_bDeviceClass,
	&dev_attr_bDeviceSubClass,
	&dev_attr_bDeviceProtocol,
	&dev_attr_iManufacturer,
	&dev_attr_iProduct,
	&dev_attr_iSerial,
	&dev_attr_functions,
	&dev_attr_enable,
	&dev_attr_state,
#ifdef CONFIG_ARCH_MT8590_ICX
	&dev_attr_SelfPowered,
	&dev_attr_MaxPower,
#endif
	NULL
};

/*-------------------------------------------------------------------------*/
/* Composite driver */

static int android_bind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;
	int ret = 0;

	ret = android_bind_enabled_functions(dev, c);
	if (ret)
		return ret;

	return 0;
}

static void android_unbind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;

	android_unbind_enabled_functions(dev, c);
}

static int android_setup_config(struct usb_configuration *c, const struct usb_ctrlrequest *ctrl)
{
	int handled = -EINVAL;
	const u8 recip = ctrl->bRequestType & USB_RECIP_MASK;

	printk("%s bRequestType=%x, bRequest=%x, recip=%x\n", __func__, ctrl->bRequestType, ctrl->bRequest, recip);

	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		switch (ctrl->bRequest)	{
		case USB_REQ_CLEAR_FEATURE:
			switch (recip) {
			case USB_RECIP_DEVICE:
				switch (ctrl->wValue) {
				case USB_DEVICE_U1_ENABLE:
					handled = 1;
					printk("Clear Feature->U1 Enable\n");
				break;

				case USB_DEVICE_U2_ENABLE:
					handled = 1;
					printk("Clear Feature->U2 Enable\n");
				break;

				default:
					handled = -EINVAL;
				break;
				}
				break;
			default:
				handled = -EINVAL;
			break;
			}
		break;

		case USB_REQ_SET_FEATURE:
			switch (recip) {
			case USB_RECIP_DEVICE:
				switch (ctrl->wValue) {
				case USB_DEVICE_U1_ENABLE:
					printk("Set Feature->U1 Enable\n");
					handled = 1;
				break;
				case USB_DEVICE_U2_ENABLE:
					printk("Set Feature->U2 Enable\n");
					handled = 1;
				break;
				default:
					handled = -EINVAL;
				break;
				}
			break;

			default:
				handled = -EINVAL;
			break;
			}
		break;

		default:
			handled = -EINVAL;
		break;
		}
	}

	return handled;
}

static int android_bind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct usb_gadget	*gadget = cdev->gadget;
	int			id, ret;

	/*
	 * Start disconnected. Userspace will connect the gadget once
	 * it is done configuring the functions.
	 */
	usb_gadget_disconnect(gadget);

	ret = android_init_functions(dev->functions, cdev);
	if (ret)
		return ret;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	/* Default strings - should be updated by userspace */
	strncpy(manufacturer_string, MANUFACTURER_STRING, sizeof(manufacturer_string) - 1);
	strncpy(product_string, PRODUCT_STRING, sizeof(product_string) - 1);
	strncpy(serial_string, "0123456789ABCDEF", sizeof(serial_string) - 1);

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_SERIAL_IDX].id = id;
	device_desc.iSerialNumber = id;

	usb_gadget_set_selfpowered(gadget);
	dev->cdev = cdev;

	return 0;
}

static int android_usb_unbind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;

	cancel_work_sync(&dev->work);
	android_cleanup_functions(dev->functions);
	return 0;
}

/* HACK: android needs to override setup for accessory to work */
static int (*composite_setup_func)(struct usb_gadget *gadget, const struct usb_ctrlrequest *c);
extern void composite_setup_complete(struct usb_ep *ep, struct usb_request *req);

static int
android_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *c)
{
	struct android_dev		*dev = _android_dev;
	struct usb_composite_dev	*cdev = get_gadget_data(gadget);
	struct usb_request		*req = cdev->req;
	struct android_usb_function	*f;
	int value = -EOPNOTSUPP;
	unsigned long flags;

	pr_debug("%s\n", __func__);

	req->zero = 0;
	req->complete = composite_setup_complete;
	req->length = 0;
	gadget->ep0->driver_data = cdev;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		if (f->ctrlrequest) {
			value = f->ctrlrequest(f, cdev, c);
			if (value >= 0)
				break;
		}
	}

	/* Special case the accessory function.
	 * It needs to handle control requests before it is enabled.
	 */
	if (value < 0)
		value = acc_ctrlrequest(cdev, c);

	if (value < 0)
		value = composite_setup_func(gadget, c);

	spin_lock_irqsave(&cdev->lock, flags);
	if (!dev->connected) {
		dev->connected = 1;
		schedule_work(&dev->work);
	} else if (c->bRequest == USB_REQ_SET_CONFIGURATION &&
						cdev->config) {
		schedule_work(&dev->work);
	}
	spin_unlock_irqrestore(&cdev->lock, flags);

	return value;
}

static void android_disconnect(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;

	/* Added for USB Develpment debug, more log for more debuging help */
	pr_verb(USB_LOG "%s:\n", __func__);
	/* Added for USB Develpment debug, more log for more debuging help */

	/* accessory HID support can be active while the
	   accessory function is not actually enabled,
	   so we need to inform it when we are disconnected.
	 */
	acc_disconnect();

	dev->connected = 0;
	schedule_work(&dev->work);

	/* Added for USB Develpment debug, more log for more debuging help */
	pr_verb(USB_LOG "%s: dev->connected = %d\n", __func__, dev->connected);
	/* Added for USB Develpment debug, more log for more debuging help */
}

#ifdef CONFIG_ENABLE_USBCONN
#ifdef USBCONN_SUSPEND_ENABLLE
static void android_suspend(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;

	pr_debug("usbconn:%s(%d) start\n", __FUNCTION__, __LINE__);

	dev->suspended = 1;
	dev->resumed = 0;
	schedule_work(&dev->work);

	pr_debug("usbconn:(%d) end\n", __LINE__);
}

static void android_resume(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;

	pr_debug("usbconn:%s(%d) start\n", __FUNCTION__, __LINE__);

	dev->suspended = 0;
	dev->resumed = 1;
	schedule_work(&dev->work);

	pr_debug("usbconn:(%d) end\n", __LINE__);
}
#endif
#endif

static struct usb_composite_driver android_usb_driver = {
	.name		= "android_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind		= android_bind,
	.unbind		= android_usb_unbind,
	.disconnect	= android_disconnect,
#ifdef CONFIG_ENABLE_USBCONN
#ifdef USBCONN_SUSPEND_ENABLLE
	.suspend	= android_suspend,
	.resume		= android_resume,
#endif
#endif
#ifdef CONFIG_USB_MU3D_DRV
	.max_speed	= USB_SPEED_SUPER
#else
	.max_speed	= USB_SPEED_HIGH
#endif
};

static int android_create_device(struct android_dev *dev)
{
	struct device_attribute **attrs = android_usb_attributes;
	struct device_attribute *attr;
	int err;

	dev->dev = device_create(android_class, NULL,
					MKDEV(0, 0), NULL, "android0");
	if (IS_ERR(dev->dev))
		return PTR_ERR(dev->dev);

	dev_set_drvdata(dev->dev, dev);

	while ((attr = *attrs++)) {
		err = device_create_file(dev->dev, attr);
		if (err) {
			device_destroy(android_class, dev->dev->devt);
			return err;
		}
	}
	return 0;
}


static int __init init(void)
{
	struct android_dev *dev;
	int err;

	android_class = class_create(THIS_MODULE, "android_usb");
	if (IS_ERR(android_class))
		return PTR_ERR(android_class);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		err = -ENOMEM;
		goto err_dev;
	}

	dev->disable_depth = 1;
	dev->functions = supported_functions;
	INIT_LIST_HEAD(&dev->enabled_functions);
	INIT_WORK(&dev->work, android_work);
	mutex_init(&dev->mutex);

	err = android_create_device(dev);
	if (err) {
		pr_err("%s: failed to create android device %d", __func__, err);
		goto err_create;
	}

	_android_dev = dev;

	err = usb_composite_probe(&android_usb_driver);
	if (err) {
		pr_err("%s: failed to probe driver %d", __func__, err);
		goto err_probe;
	}

	/* HACK: exchange composite's setup with ours */
	composite_setup_func = android_usb_driver.gadget_driver.setup;
	android_usb_driver.gadget_driver.setup = android_setup;

	return 0;

err_probe:
	device_destroy(android_class, dev->dev->devt);
err_create:
	kfree(dev);
err_dev:
	class_destroy(android_class);
	return err;
}
late_initcall(init);

static void __exit cleanup(void)
{
	usb_composite_unregister(&android_usb_driver);
	class_destroy(android_class);
	kfree(_android_dev);
	_android_dev = NULL;
}
module_exit(cleanup);

#ifdef CONFIG_USB_CAN_SLEEP_CONNECTING
bool usb_gadget_is_enabled(void)
{
	struct android_dev *dev = _android_dev;

	if (dev)
		return dev->enabled;

	return false;
}
#endif

#ifdef CONFIG_ENABLE_USBCONN
int gadget_bind_usbconn(void *ops)
{
	pr_debug("usbconn:%s(%d) start(%p)\n", __FUNCTION__, __LINE__, ops);

	if (ops == NULL) {
		return -EINVAL;
	}
	conn_to_gadget_func_tbl = (struct conn_to_gadget_func *)ops;

	pr_debug("usbconn:(%d) end\n", __LINE__);
	return 0;
}
EXPORT_SYMBOL(gadget_bind_usbconn);


bool get_usb_connect_status(void)
{
	struct android_dev *dev;
	bool status;


	pr_debug("usbconn:%s(%d) start\n", __FUNCTION__, __LINE__);

	dev = _android_dev;
	status = dev->sw_connected;

	pr_debug("usbconn:(%d) end(set:%d)\n", __LINE__, status);
	return status;
}
EXPORT_SYMBOL(get_usb_connect_status);
#endif

