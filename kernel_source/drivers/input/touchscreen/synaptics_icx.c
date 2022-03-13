/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* drivers/input/keyboard/synaptics_i2c_rmi.c
 *
 * Copyright (C) 2007 Google, Inc.
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
/*
 * ChangeLog:
 *   2011,2012,2013,2016 changed by Sony Corporation
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>
#include <linux/input/ts_icx.h>
#include <linux/input/synaptics_icx.h>

#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>

#define ERROR(format, args...) \
	printk(KERN_ERR "ERR[%s()]" format, __func__, ##args)

#define DPRINTF(format, args...) \
	printk(KERN_INFO "[%s()]" format, __func__, ##args)

#define SYNAPTICS_ICX_TP_USE_I2C_TRANSFER
/* #define SYNAPTICS_ICX_TP_ROT_180 */

#define MULTI_NUM 5

/* real finger_size read from TP device is   5 =  8mm	 */
/*                              TP size is 480 = 50mm	 */
/* maximum xy point sending to app is 960 (= 480 * 2)    */
/* so finger_size sending to app should be done 30 times */
#define SYNAPTICS_ICX_TP_W_COEF		30
#define SYNAPTICS_ICX_TP_W			(15 * SYNAPTICS_ICX_TP_W_COEF)

#ifdef SYNAPTICS_ICX_TP_USE_I2C_TRANSFER
#define SYNAPTICS_ICX_TP_DMA_ALLOC_RETRY	5
#endif

#define SYNAPTICS_ICX_TS_I2C_TIMING	400

#define SYNAPTICS_ICX_TS_KEEPALIVE_PERIOD	500
#define SYNAPTICS_ICX_TS_STATUS_CODE_MASK	0x0F
#define SYNAPTICS_ICX_TS_DEVICE_FAILURE		0x03

#define SYNAPTICS_ICX_TS_IGNORE_PERIOD		30
#define SYNAPTICS_ICX_TS_NOTIFY_RELEASE		30
#define SYNAPTICS_ICX_TS_IGNORE_NUM_DEFAULT	3
#define SYNAPTICS_ICX_TS_VALID_NUM_DEFAULT	3

static struct workqueue_struct *synaptics_wq;

struct synaptics_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct hrtimer timer;
	struct work_struct  irq_work;
	struct delayed_work	keepalive_work;
	struct delayed_work	notify_release_work;
	struct mutex		lock;
	int reported_finger_count;
	int8_t sensitivity_adjust;
	unsigned long last_touch_time;
	int (*power)(int on);
	struct early_suspend early_suspend;
	int before_finger[MULTI_NUM];
	int dbgregp;
	int sleep_state;
	int is_suspend;
	unsigned int send_event_cnt;
	unsigned int ignore_num;
	unsigned int valid_num;
#ifdef SYNAPTICS_ICX_TP_USE_I2C_TRANSFER
	dma_addr_t buf_phys;
	void *buf_virt;
#endif
};

struct synaptics_ts_data *synaptics_icx_ts_data = NULL;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h);
static void synaptics_ts_late_resume(struct early_suspend *h);
#endif
static int synaptics_ts_sleep(struct synaptics_ts_data *ts, int en);

struct xyzw {
	uint8_t xh;
	uint8_t yh;
	uint8_t xyl;
	uint8_t w;
	uint8_t z;
};
struct ts_reg {
	uint8_t dev_stat;
	uint8_t int_stat;
	uint8_t finger[2];
	struct xyzw xyzw[MULTI_NUM];
	uint8_t gest0;
	uint8_t gest1;
#define GEST1_PALM_DETECT_MASK 0x01
};


#ifndef SYNAPTICS_ICX_TP_USE_I2C_TRANSFER
static int synaptics_ts_get_data(struct synaptics_ts_data *ts, struct ts_reg *reg)
{
	uint8_t start_reg;
	int i;
	int ret;

	/* set address */
	start_reg = 0x14;
	ret = mt_i2c_master_send(ts->client, &start_reg, 1, 0);
	if (ret < 0) {
		ERROR("mt_i2c_master_send failed(%d)\n", ret);
		return ret;
	}

	ret = mt_i2c_master_recv(ts->client, &reg->int_stat, sizeof(reg->int_stat), 0);
	if (ret < 0) {
		return ret;
	}
	ret = mt_i2c_master_recv(ts->client, &reg->finger[0], sizeof(reg->finger), 0);
	if (ret < 0) {
		return ret;
	}
	for (i = 0; i < MULTI_NUM; i++) {
		ret = mt_i2c_master_recv(ts->client, &reg->xyzw[i], sizeof(reg->xyzw[i]), 0);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}
#endif

static void synaptics_ts_reset(struct synaptics_ts_data *ts)
{
	struct ts_icx_platform_data *pdata = ts->client->dev.platform_data;

	mt_set_gpio_out(pdata->xrst_gpio, GPIO_OUT_ZERO);
	msleep(35);
	mt_set_gpio_out(pdata->xrst_gpio, GPIO_OUT_ONE);
	msleep(100);
}

static void synaptics_ts_keepalive_work_func(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct synaptics_ts_data *ts = container_of(dwork, struct synaptics_ts_data, keepalive_work);
	uint8_t buf;
	int ret = 0;

	mutex_lock(&ts->lock);

	if (!ts->sleep_state) {

		ret = i2c_smbus_read_i2c_block_data(ts->client, 0x13, sizeof(buf), &buf);
		if ((ret < 0)
		 || ((buf & SYNAPTICS_ICX_TS_STATUS_CODE_MASK) == SYNAPTICS_ICX_TS_DEVICE_FAILURE)) {

			pr_err("[%s] ESD triggered(ret=%d, StatusCode=%02x)\n", __func__, ret, buf);

			ts->send_event_cnt = 0;

			/* reset */
			synaptics_ts_reset(ts);
		}

		queue_delayed_work(synaptics_wq, &ts->keepalive_work, SYNAPTICS_ICX_TS_KEEPALIVE_PERIOD);
	}

	mutex_unlock(&ts->lock);
}

static void synaptics_ts_notify_release_work_func(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct synaptics_ts_data *ts = container_of(dwork, struct synaptics_ts_data, notify_release_work);

	input_report_abs(ts->input_dev, ABS_PRESSURE, 0);
	input_report_key(ts->input_dev, BTN_TOUCH, 0);
	input_sync(ts->input_dev);

	ts->send_event_cnt = 0;
}

static void synaptics_ts_irq_work_func(struct work_struct *work)
{
	int i;
	int ret;
#ifdef SYNAPTICS_ICX_TP_USE_I2C_TRANSFER
	struct i2c_msg msg[2];
	uint8_t start_reg;
#endif
	struct synaptics_ts_data *ts =
		container_of(work, struct synaptics_ts_data, irq_work);
	struct ts_reg *reg;
	int finger;
	int finger_num;
	unsigned int delta;
#ifdef SYNAPTICS_ICX_TP_ROT_180
	struct ts_icx_platform_data *pdata = ts->client->dev.platform_data;
#endif

#ifdef SYNAPTICS_ICX_TP_USE_I2C_TRANSFER
	start_reg = 0x13;
	msg[0].addr = ts->client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &start_reg;
	msg[0].ext_flag = 0;
	msg[0].timing = SYNAPTICS_ICX_TS_I2C_TIMING;
	msg[1].addr = ts->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = sizeof(*reg);
	msg[1].buf = (void *)ts->buf_phys;
	msg[1].ext_flag = I2C_DMA_FLAG;
	msg[1].timing = SYNAPTICS_ICX_TS_I2C_TIMING;

	mutex_lock(&ts->lock);
	ret = i2c_transfer(ts->client->adapter, msg, 2);
	mutex_unlock(&ts->lock);
	if (ret < 0) {
		ERROR("i2c_transfer failed(%d)\n", ret);
		return;
	}

	reg = (struct ts_reg *)ts->buf_virt;
#else
	mutex_lock(&ts->lock);
	ret = synaptics_ts_get_data(ts, &reg);
	mutex_unlock(&ts->lock);
	if (ret < 0) {
		ERROR("failed to get TS data(%d)\n", ret);
		return;
	}
#endif

	if ((reg->dev_stat & SYNAPTICS_ICX_TS_STATUS_CODE_MASK) == SYNAPTICS_ICX_TS_DEVICE_FAILURE) {
		pr_err("[%s] ESD triggered(StatusCode=%02x), ignored the finger information\n",
			   __func__, reg->dev_stat);
		return;
	}

	delta = jiffies_to_msecs(jiffies - ts->last_touch_time);
	ts->last_touch_time = jiffies;
	if (delta >= SYNAPTICS_ICX_TS_IGNORE_PERIOD) {
		/* ignore the first touch */
		return;
	}

	finger_num = 0;
	for (i = 0; i < MULTI_NUM; i++) {
		int x, y, wx, wy, z;

		finger = (reg->finger[i / 4] >> (i % 4) * 2) & 0x3;
		if (finger)
			finger_num++;

		if (finger == 0 && ts->before_finger[i] == 0)
			continue;
		ts->before_finger[i] = finger;

		x = reg->xyzw[i].xh << 4 | (reg->xyzw[i].xyl & 0xf);
		y = reg->xyzw[i].yh << 4 | ((reg->xyzw[i].xyl >> 4) & 0xf);
		wx = (reg->xyzw[i].w & 0xf) * SYNAPTICS_ICX_TP_W_COEF;
		wy = ((reg->xyzw[i].w >> 4) & 0xf) * SYNAPTICS_ICX_TP_W_COEF;
		z = reg->xyzw[i].z;
#ifdef SYNAPTICS_ICX_TP_ROT_180
		x = pdata->max_x - x;
		y = pdata->max_y - y;
#endif

		if (!z)
			continue;
#if 0 /* debug */
		printk("i = %d, x = %d, y = %d, wx = %d, wy = %d, z = %d\n",
			i, x, y, wx, wy, z);
#endif
		if (ts->send_event_cnt < ts->valid_num) {
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, wy);
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MINOR, wx);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, wx);
			input_report_abs(ts->input_dev, ABS_PRESSURE, z);
			input_mt_sync(ts->input_dev);
		}
	}

	cancel_delayed_work_sync(&ts->notify_release_work);

	if (finger_num) {
		if (ts->send_event_cnt < ts->valid_num) {
			input_report_key(ts->input_dev, BTN_TOUCH, 1);
			input_sync(ts->input_dev);
		}

		ts->send_event_cnt++;
		if (ts->send_event_cnt > ts->ignore_num) {
			ts->send_event_cnt = 0;
		}

		queue_delayed_work(synaptics_wq,
						   &ts->notify_release_work,
						   SYNAPTICS_ICX_TS_NOTIFY_RELEASE);
	}
	else {
		input_report_abs(ts->input_dev, ABS_PRESSURE, 0);
		input_report_key(ts->input_dev, BTN_TOUCH, 0);
		input_sync(ts->input_dev);

		ts->send_event_cnt = 0;
	}
}

static void synaptics_ts_irq_handler(void)
{
	struct synaptics_ts_data *ts = synaptics_icx_ts_data;

	queue_work(synaptics_wq, &ts->irq_work);
}

static int synaptics_ts_sleep(struct synaptics_ts_data *ts, int en)
{
	int ret;
	int tmp;

	/* F01_RMI_CTRL0 */
	ret = i2c_smbus_read_byte_data(ts->client, 0x3b);
	if (ret < 0) {
		pr_err("ERR: ts_sleep read %d\n", ret);
		return -1;
	}
	tmp = ret & ~0x7;
	tmp |= en ? 0x1 : 0x4;
	ret = i2c_smbus_write_byte_data(ts->client, 0x3b, tmp);

	if (ret < 0) {
		pr_err("ERR: ts_sleep write %d\n", ret);
		return -1;
	}

	ts->sleep_state = en;

	return 0;
}


static ssize_t dbgregp_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	if (!buf)
		return -EINVAL;
	return snprintf(buf, PAGE_SIZE, "0x%x\n", ts->dbgregp);
}

static ssize_t dbgregp_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	long tmp;

	if (!buf)
		return -EINVAL;

	if (kstrtol(buf, 0, &tmp))
		return -EINVAL;

	ts->dbgregp = tmp;
	return count;
}

static ssize_t dbgreg_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	int ret;

	if (!buf)
		return -EINVAL;
	ret = i2c_smbus_read_byte_data(ts->client, ts->dbgregp);
	if (ret < 0)
		return ret;

	return snprintf(buf, PAGE_SIZE, "0x%x\n", ret);
}

static ssize_t dbgreg_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	int ret;
	long tmp;

	if (!buf)
		return -EINVAL;

	if (kstrtol(buf, 0, &tmp))
		return -EINVAL;

	ret = i2c_smbus_write_byte_data(ts->client, ts->dbgregp, tmp);
	if (ret < 0)
		return ret;
	return count;
}

static ssize_t dump_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	int ret;
	int i;
	char buf2[16];

	if (!buf)
		return -EINVAL;
	buf[0] = '\0';
	strlcat(buf, "      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n",
		PAGE_SIZE - 1);

	for (i = 0; i < 256; i++) {
		if ((i & 0xf) == 0) {
			snprintf(buf2, sizeof(buf2), "%02x   ", i);
			strlcat(buf, buf2, PAGE_SIZE - 1);
		}

		ret = i2c_smbus_read_byte_data(ts->client, i);
		if (ret < 0)
			strlcat(buf, "**:", PAGE_SIZE - 1);
		else {
			snprintf(buf2, sizeof(buf2), "%02x:", ret);
			strlcat(buf, buf2, PAGE_SIZE - 1);
		}
		if ((i & 0xf) == 0xf)
			strlcat(buf, "\n", PAGE_SIZE - 1);
	}
	return strlen(buf);
}

static ssize_t xrst_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	struct ts_icx_platform_data *pdata = ts->client->dev.platform_data;
	int ret;

	if (!buf)
		return -EINVAL;

	ret = mt_get_gpio_out(pdata->xrst_gpio);

	return snprintf(buf, PAGE_SIZE, "%d\n", ret);
}

static ssize_t sleep_store(struct device *dev,
						   struct device_attribute *attr,
					       const char *buf, size_t count)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	int ret;
	long tmp;

	if (!buf)
		return -EINVAL;

	if (kstrtol(buf, 0, &tmp))
		return -EINVAL;

	mutex_lock(&ts->lock);

	if (tmp) {
		cancel_delayed_work_sync(&ts->keepalive_work);
		ret = synaptics_ts_sleep(ts, 1);
	}
	else {
		if (ts->is_suspend == 0) {
			ret = synaptics_ts_sleep(ts, 0);
			queue_delayed_work(synaptics_wq,
							   &ts->keepalive_work,
							   SYNAPTICS_ICX_TS_KEEPALIVE_PERIOD);

			ts->send_event_cnt = 0;
		}
	}

	mutex_unlock(&ts->lock);

	if (ret < 0)
		return ret;
	else
		return count;
}

static ssize_t sleep_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	struct ts_icx_platform_data *pdata = ts->client->dev.platform_data;

	if (!buf)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%d\n", ts->sleep_state);
}

static ssize_t xrst_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	struct ts_icx_platform_data *pdata = ts->client->dev.platform_data;
	int ret;
	long tmp;

	if (!buf)
		return -EINVAL;

	if (kstrtol(buf, 0, &tmp))
		return -EINVAL;

	if (tmp)
		ret = mt_set_gpio_out(pdata->xrst_gpio, GPIO_OUT_ONE);
	else
		ret = mt_set_gpio_out(pdata->xrst_gpio, GPIO_OUT_ZERO);

	return ret;
}

static ssize_t ignore_num_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);

	if (!buf)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%d\n", ts->ignore_num);
}

static ssize_t ignore_num_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	long tmp;

	if (!buf)
		return -EINVAL;

	if (kstrtol(buf, 0, &tmp))
		return -EINVAL;

	if (tmp >= 0) {
		ts->ignore_num = tmp;
	}

	return count;
}

static DEVICE_ATTR(regp, 0600, dbgregp_show, dbgregp_store);
static DEVICE_ATTR(regv, 0600, dbgreg_show, dbgreg_store);
static DEVICE_ATTR(dump, 0400, dump_show, NULL);
static DEVICE_ATTR(xrst, 0600, xrst_show, xrst_store);
static DEVICE_ATTR(sleep, 0600, sleep_show, sleep_store);
static DEVICE_ATTR(ignore_num, 0600, ignore_num_show, ignore_num_store);

static int synaptics_ts_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	struct synaptics_ts_data *ts;
	int ret = 0;
	struct ts_icx_platform_data *pdata;
	unsigned long irqflags;
	uint8_t cfg_id[4];
#ifdef SYNAPTICS_ICX_TP_USE_I2C_TRANSFER
	int i;
#endif

	if (!client->dev.platform_data) {
		ERROR("need platformdata");
		return -EINVAL;
	}
	pdata = client->dev.platform_data;

	if (!client->irq) {
		ERROR("need irq");
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		ERROR("need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
	INIT_WORK(&ts->irq_work, synaptics_ts_irq_work_func);
	INIT_DELAYED_WORK(&ts->keepalive_work, synaptics_ts_keepalive_work_func);
	INIT_DELAYED_WORK(&ts->notify_release_work, synaptics_ts_notify_release_work_func);
	mutex_init(&ts->lock);
	ts->client = client;
	i2c_set_clientdata(client, ts);

	ts->power = pdata->power;
	if (ts->power) {
		ret = ts->power(1);
		if (ret < 0) {
			ERROR("power on failed\n");
			goto err_power_failed;
		}
	}

	ret = i2c_smbus_read_byte_data(ts->client, 0x84);
	if (ret < 0) {
		pr_err("detect failed %d\n", ret);
		goto err_detect_failed;
	}

	DPRINTF("reg[0x84] = 0x%02x\n", ret);

	ret = i2c_smbus_read_i2c_block_data(ts->client, 0x37, sizeof(cfg_id), cfg_id);
	if (ret < 0) {
		pr_err("couldn't read Customer Defined Config ID\n");
	} else {
		printk("%s: Config ID = %02x %02x %02x %02x\n",
			SYNAPTICS_ICX_NAME,
			cfg_id[0], cfg_id[1], cfg_id[2], cfg_id[3]);
	}

	ts->sensitivity_adjust = pdata->sensitivity_adjust;
	irqflags = pdata->irqflags;

	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		ERROR("Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = SYNAPTICS_ICX_NAME;
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ts->input_dev->keybit);

	input_set_abs_params(ts->input_dev, ABS_X,
	    pdata->min_x, pdata->max_x, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y,
	    pdata->min_y, pdata->max_y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X,
	    pdata->min_x, pdata->max_x, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y,
	    pdata->min_y, pdata->max_y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR,
	    0, SYNAPTICS_ICX_TP_W, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR,
	    0, SYNAPTICS_ICX_TP_W, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MINOR,
	    0, SYNAPTICS_ICX_TP_W, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_PRESSURE,
	    0, 255, 0, 0);

	input_set_events_per_packet(ts->input_dev, 37); /* (events/touch=7) * 5 + BTN_TOUCH + SYNC */

	ret = input_register_device(ts->input_dev);
	if (ret) {
		ERROR("Unable to register %s input device\n",
			ts->input_dev->name);
		goto err_input_register_device_failed;
	}

#ifdef SYNAPTICS_ICX_TP_USE_I2C_TRANSFER
	for (i = 0; i < SYNAPTICS_ICX_TP_DMA_ALLOC_RETRY; i++) {
		ts->buf_virt = dma_alloc_coherent(NULL, sizeof(struct ts_reg), &(ts->buf_phys), 0);
		if (ts->buf_virt != NULL) {
			break;
		}
	}
	if (i >= SYNAPTICS_ICX_TP_DMA_ALLOC_RETRY) {
		ERROR("memory allocation error\n");
		goto err_dma_alloc_coherent_failed;
	}
#endif

	synaptics_icx_ts_data = ts; 

	mt_eint_registration(client->irq, irqflags, synaptics_ts_irq_handler, 1);

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = synaptics_ts_early_suspend;
	ts->early_suspend.resume = synaptics_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	ts->sleep_state = 0;	/* default state: not in sleep */

	device_create_file(&client->dev, &dev_attr_regv);
	device_create_file(&client->dev, &dev_attr_regp);
	device_create_file(&client->dev, &dev_attr_dump);
	device_create_file(&client->dev, &dev_attr_xrst);
	device_create_file(&client->dev, &dev_attr_sleep);
	device_create_file(&client->dev, &dev_attr_ignore_num);

	ts->last_touch_time = jiffies;
	ts->ignore_num = SYNAPTICS_ICX_TS_IGNORE_NUM_DEFAULT;
	ts->valid_num = SYNAPTICS_ICX_TS_VALID_NUM_DEFAULT;

	queue_work(synaptics_wq, &ts->irq_work);
	queue_delayed_work(synaptics_wq, &ts->keepalive_work, SYNAPTICS_ICX_TS_KEEPALIVE_PERIOD);

	return 0;

err_dma_alloc_coherent_failed:
	input_unregister_device(ts->input_dev);

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
err_detect_failed:
err_power_failed:
	kfree(ts);
err_alloc_data_failed:
err_check_functionality_failed:
	return ret;
}

static int synaptics_ts_remove(struct i2c_client *client)
{
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);
	struct ts_icx_platform_data *pdata = client->dev.platform_data;

	mt_eint_mask(client->irq);
	unregister_early_suspend(&ts->early_suspend);
#ifdef SYNAPTICS_ICX_TP_USE_I2C_TRANSFER
	if (ts->buf_virt) {
		dma_free_coherent(NULL, sizeof(struct ts_reg), ts->buf_virt, ts->buf_phys);
	}
#endif
	mt_eint_registration(client->irq, pdata->irqflags, NULL, 0);
	cancel_work_sync(&ts->irq_work);
	cancel_delayed_work_sync(&ts->keepalive_work);
	cancel_delayed_work_sync(&ts->notify_release_work);
    if (synaptics_wq)
        destroy_workqueue(synaptics_wq);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int synaptics_ts_shutdown(struct i2c_client *client)
{
    return synaptics_ts_remove(client);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);

	mutex_lock(&ts->lock);

	mt_eint_mask(ts->client->irq);

	cancel_work_sync(&ts->irq_work);
	cancel_delayed_work_sync(&ts->keepalive_work);

	if (ts->power)
		ts->power(0);
	else
		synaptics_ts_sleep(ts, 1);

	ts->is_suspend = 1;

	mutex_unlock(&ts->lock);
}

static void synaptics_ts_late_resume(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);

	mutex_lock(&ts->lock);

	if (ts->power)
		ts->power(1);
	else
		synaptics_ts_sleep(ts, 0);

	ts->is_suspend = 0;
	ts->send_event_cnt = 0;

	mt_eint_unmask(ts->client->irq);

	queue_work(synaptics_wq, &ts->irq_work);
	queue_delayed_work(synaptics_wq, &ts->keepalive_work, SYNAPTICS_ICX_TS_KEEPALIVE_PERIOD);

	mutex_unlock(&ts->lock);
}
#endif

static const struct i2c_device_id synaptics_ts_id[] = {
	{ SYNAPTICS_ICX_NAME, 0 },
	{ }
};

static struct i2c_driver synaptics_ts_driver = {
	.probe		= synaptics_ts_probe,
	.remove		= synaptics_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= synaptics_ts_suspend,
	.resume		= synaptics_ts_resume,
#endif
	.shutdown	= synaptics_ts_shutdown,
	.id_table	= synaptics_ts_id,
	.driver = {
		.name	= SYNAPTICS_ICX_NAME,
	},
};

static int __init synaptics_ts_init(void)
{
	int ret = 0;

	synaptics_wq = create_singlethread_workqueue("synaptics_wq");
	if (!synaptics_wq)
		return -ENOMEM;

	ret = i2c_add_driver(&synaptics_ts_driver);
	if (ret < 0)
		destroy_workqueue(synaptics_wq);

	return ret;
}

static void __exit synaptics_ts_exit(void)
{
	i2c_del_driver(&synaptics_ts_driver);
	if (synaptics_wq)
		destroy_workqueue(synaptics_wq);
}

module_init(synaptics_ts_init);
module_exit(synaptics_ts_exit);

MODULE_DESCRIPTION("Synaptics Touchscreen Driver");
MODULE_LICENSE("GPL");
