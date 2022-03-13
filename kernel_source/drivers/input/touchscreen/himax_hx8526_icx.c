/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-12-09
 */
/* drivers/input/touchscreen/himax_hx8526_icx.c
 *
 * Copyright (C) 2015 Sony Corporation
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

#include <asm/uaccess.h>
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
#include <linux/proc_fs.h>
#include <linux/input/ts_icx.h>
#include <linux/input/himax_hx8526_icx.h>

#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>

//#define HIMAX_HX8526_ICX_DEBUG
#define ERROR(format, args...) printk(KERN_ERR "ERR[%s()]" format, __func__, ##args)

#ifdef HIMAX_HX8526_ICX_DEBUG
#define DPRINTK(format, args...) printk(KERN_INFO "[%s()]" format, __func__, ##args)
#else
#define DPRINTK(format, args...)
#endif

#define HIMAX_HX8526_ICX_TP_ROT_180
/* #define HIMAX_HX8526_ICX_TP_FIXED_MULTI_NUM	(5) */

#define MAX_DATA_SIZE (128)

#define MULTI_NUM_MAX	10
#define MULTI_NUM_LIMIT	 5	/* the number of valid point */

#define COORDINATE_DATA_SIZE(n)	(n * 4)
#define FINGER_DATA_SIZE(n)		(((n + 3) / 4) * 4)
#define ID_INFO_DATA_SIZE		(4)

#define HIMAX_HX8526_ICX_TP_DMA_ALLOC_RETRY	5

/* real finger_size read from TP device is 120 = 20mm   */
/*                              TP size is 480 = 40mm   */
/* maximum xy point sending to app is 960 (= 480 * 2)   */
/* so finger_size sending to app should be done 4 times */
#define HIMAX_HX8526_TS_FINGER_SIZE_COEF	4
#define HIMAX_HX8526_TS_FINGER_SIZE			(120 * HIMAX_HX8526_TS_FINGER_SIZE_COEF)
#define HIMAX_HX8526_TS_Z_MAX		1

#define HIMAX_HX8526_TS_I2C_TIMING	400

#define HIMAX_HX8526_TS_KEEPALIVE_PERIOD	500
#define HIMAX_HX8526_TS_ESD_SUM				0x08

#define HIMAX_HX8526_TS_IGNORE_PERIOD		30
#define HIMAX_HX8526_TS_NOTIFY_RELEASE		30
#define HIMAX_HX8526_TS_IGNORE_NUM_DEFAULT	1

#define HIMAX_HX8526_TS_ERR_I2C				(-1)
#define HIMAX_HX8526_TS_ERR_SUM				(-2)
#define HIMAX_HX8526_TS_INVALID_DATA		(-3)

#define HIMAX_HX8526_TS_IDLE_MODE_ENABLE		(0x5F)
#define HIMAX_HX8526_TS_IDLE_MODE_DISABLE		(0x57)

static struct workqueue_struct *himax_hx8526_wq;

struct himax_hx8526_ts_data {
	struct i2c_client	   *client;
	struct input_dev	   *input_dev;
	struct work_struct		irq_work;
	struct delayed_work		keepalive_work;
	struct delayed_work		notify_release_work;
	struct mutex			lock;

	uint16_t				addr;
	int						before_finger[MULTI_NUM_MAX];
	unsigned long			last_touch_time;

	struct early_suspend	early_suspend;
	int 					(*power)(int on);

	uint8_t					multi_num;
	uint32_t				all_data_size;
	uint32_t				coordinate_data_size;
	uint32_t				finger_data_size;
	uint32_t				finger_data_offset;
	uint32_t				id_info_offset;

	dma_addr_t				buf_phys;
	void				   *buf_virt;

	int						dbgregp;
	int						sleep_state;
	int						is_suspend;
	unsigned int			send_event_cnt;
	unsigned int			ignore_num;

	/* versiont info */
	uint8_t					fw_ver_H;
	uint8_t					fw_ver_L;
	uint8_t					config_ver;

	/* TP info */
	uint32_t				rx_num;
	uint32_t				tx_num;
	uint32_t				x_res;
	uint32_t				y_res;
	uint8_t					xy_reverse;
	uint8_t					int_is_edge;
	uint8_t					idle_mode;
};

struct himax_hx8526_ts_data *himax_hx8526_icx_ts_data = NULL;

struct xy_coordinate_t {
	uint8_t xh;
	uint8_t xl;
	uint8_t yh;
	uint8_t yl;
};

struct id_info_t {
	uint8_t  point_count;
	uint8_t id1;
	uint8_t id2;
	uint8_t  sum;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void himax_hx8526_ts_early_suspend(struct early_suspend *h);
static void himax_hx8526_ts_late_resume(struct early_suspend *h);
#endif
static int  himax_hx8526_ts_sleep(struct himax_hx8526_ts_data *ts, int en);
static int  himax_hx8526_ts_poweron(struct himax_hx8526_ts_data *ts);
static int  himax_hx8526_ts_reinitialize(struct himax_hx8526_ts_data *ts);

/*=== Debug functions ================*/
static int  himax_touch_proc_init(void);
static void himax_touch_proc_deinit(void);
/*====================================*/

static void himax_hx8526_ts_reset(struct himax_hx8526_ts_data *ts)
{
	struct ts_icx_platform_data *pdata = ts->client->dev.platform_data;

	mt_set_gpio_out(pdata->xrst_gpio, GPIO_OUT_ZERO);
}

static void himax_hx8526_ts_unreset(struct himax_hx8526_ts_data *ts)
{
	struct ts_icx_platform_data *pdata = ts->client->dev.platform_data;

	mt_set_gpio_out(pdata->xrst_gpio, GPIO_OUT_ONE);
	msleep(20);
	mt_set_gpio_out(pdata->xrst_gpio, GPIO_OUT_ZERO);
	msleep(20);
	mt_set_gpio_out(pdata->xrst_gpio, GPIO_OUT_ONE);
	msleep(20);
}

static int himax_hx8526_ts_write(struct himax_hx8526_ts_data *ts, uint8_t length, uint8_t *data)
{
	struct i2c_msg msg;
	int ret;

	msg.addr	 = ts->client->addr;
	msg.flags	 = 0;
	msg.len		 = length;
	msg.buf		 = data;
	msg.ext_flag = 0;
	msg.timing	 = HIMAX_HX8526_TS_I2C_TIMING;

	ret = i2c_transfer(ts->client->adapter, &msg, 1);
	if (ret < 0) {
		pr_err("[HIMAX] failed to write(%d)\n", ret);
		return ret;
	}

	return 0;
}

static int himax_hx8526_ts_read(struct himax_hx8526_ts_data *ts, uint8_t command, uint8_t length, uint8_t *data)
{
	struct i2c_msg msg[2];
	int ret;

	msg[0].addr		= ts->client->addr;
	msg[0].flags	= 0;
	msg[0].len		= 1;
	msg[0].buf		= &command;
	msg[0].ext_flag	= 0;
	msg[0].timing	= HIMAX_HX8526_TS_I2C_TIMING;
	msg[1].addr		= ts->client->addr;
	msg[1].flags	= I2C_M_RD;
	msg[1].len		= length;
	msg[1].buf		= (void *)ts->buf_phys;
	msg[1].ext_flag = I2C_DMA_FLAG;
	msg[1].timing	= HIMAX_HX8526_TS_I2C_TIMING;

	ret = i2c_transfer(ts->client->adapter, msg, 2);
	if (ret < 0) {
		pr_err("[HIMAX] failed to read(%d)\n", ret);
		return ret;
	}

	memcpy(data, ts->buf_virt, length);
	
	return 0;
}

static int himax_hx8526_ts_read_touch_info(struct himax_hx8526_ts_data *ts, uint8_t *buf)
{
	uint8_t invalid_data;
	uint8_t sum;
	int i;
	int ret;

	ret = himax_hx8526_ts_read(ts, 0x86, ts->all_data_size, buf);
	if (ret < 0) {
		ERROR("failed to read touch information(%d)\n", ret);

		/* reinitialize */
		ret = himax_hx8526_ts_reinitialize(ts);
		if (ret < 0) {
			ERROR("TS reinitialize failure\n");
		}
		return HIMAX_HX8526_TS_ERR_I2C;
	}

	invalid_data = 1;
	sum = 0;
	for (i = 0; i < ts->all_data_size; i++) {
		if (buf[i] != 0xED) {
			invalid_data = 0;
		}
		sum += buf[i];
	}
	if (invalid_data) {
		/* all 0xED is invalid data */
		ERROR("TS invalid data recieved(%02Xh)\n", buf[0]);
		return HIMAX_HX8526_TS_INVALID_DATA;
	}
	if (sum != 0) {
		ERROR("TS data SUM error(%02Xh)\n", sum);

		/* reinitialize */
		ret = himax_hx8526_ts_reinitialize(ts);
		if (ret < 0) {
			ERROR("TS reinitialize failure\n");
		}
		return HIMAX_HX8526_TS_ERR_SUM;
	}

	return 0;
}

static int himax_hx8526_ts_enable_irq(struct himax_hx8526_ts_data *ts)
{
	uint8_t reg[MAX_DATA_SIZE];
	int ret = 0;

	ret = himax_hx8526_ts_read_touch_info(ts, reg);
	if (ret < 0) {
		ERROR("read touch info(%d)\n", ret);

		if (ret == HIMAX_HX8526_TS_INVALID_DATA) {
			ret = 0;
		}
	}

	ts->send_event_cnt = 0;

	return ret;
}

static void himax_hx8526_ts_keepalive_work_func(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct himax_hx8526_ts_data *ts = container_of(dwork, struct himax_hx8526_ts_data, keepalive_work);
	uint8_t buf;
	int ret = 0;

	mutex_lock(&ts->lock);

	if (!ts->sleep_state) {

		ret = himax_hx8526_ts_read(ts, 0xDC, sizeof(buf), &buf);
		if ((ret < 0) || !(buf & HIMAX_HX8526_TS_ESD_SUM)) {
			pr_err("[%s] ESD triggered(ret=%d, ESD_SUM=%02x)\n", __func__, ret, buf);

			/* reinitialize */
			ret = himax_hx8526_ts_reinitialize(ts);
			if (ret < 0) {
				pr_err("[%s] reinitialize failure\n", __func__);
			}
		}

		queue_delayed_work(himax_hx8526_wq, &ts->keepalive_work, HIMAX_HX8526_TS_KEEPALIVE_PERIOD);
	}

	mutex_unlock(&ts->lock);
}

static void himax_hx8526_ts_notify_release_work_func(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct himax_hx8526_ts_data *ts = container_of(dwork, struct himax_hx8526_ts_data, notify_release_work);

	input_report_abs(ts->input_dev, ABS_PRESSURE, 0);
	input_report_key(ts->input_dev, BTN_TOUCH, 0);
	input_sync(ts->input_dev);

	ts->send_event_cnt = 0;
}

static void himax_hx8526_ts_irq_work_func(struct work_struct *work)
{
	struct himax_hx8526_ts_data *ts = container_of(work, struct himax_hx8526_ts_data, irq_work);
#ifdef HIMAX_HX8526_ICX_TP_ROT_180
	struct ts_icx_platform_data *pdata = ts->client->dev.platform_data;
#endif
	uint8_t reg[MAX_DATA_SIZE];

	struct xy_coordinate_t *xy_coordinate_p;
	uint8_t 			   *finger_data_p;
	struct id_info_t	   *id_info_p;
	int 	 finger_num;
	uint8_t  finger;
	uint16_t id_info;
	uint32_t delta;
	int x, y, wx, wy, z;
	int i;
	int ret;

	mutex_lock(&ts->lock);
	ret = himax_hx8526_ts_read_touch_info(ts, reg);
	mutex_unlock(&ts->lock);
	if (ret < 0) {
		ERROR("read touch info(%d)\n", ret);
		return;
	}

	delta = jiffies_to_msecs(jiffies - ts->last_touch_time);
	ts->last_touch_time = jiffies;
	if (delta >= HIMAX_HX8526_TS_IGNORE_PERIOD) {
		/* ignore the first touch */
		return;
	}

	xy_coordinate_p = (struct xy_coordinate_t *)reg;
	finger_data_p   = reg + ts->finger_data_offset;
	id_info_p       = (struct id_info_t *)(reg + ts->id_info_offset);

	if ((id_info_p->id2 == 0xFF) && (id_info_p->id1 == 0xFF))
		id_info = 0;
	else
		id_info = (((uint16_t)id_info_p->id2 & 0x03) << 8) | (uint16_t)id_info_p->id1;
	finger_num = id_info_p->point_count & 0x0F;

	for (i = 0; i < ts->multi_num; i++) {
		finger = (id_info >> i) & 0x0001;

		if (finger == 0 && ts->before_finger[i] == 0)
			continue;
		ts->before_finger[i] = finger;

		x  = ((xy_coordinate_p[i].xh << 8) | xy_coordinate_p[i].xl) * 2;
		y  = ((xy_coordinate_p[i].yh << 8) | xy_coordinate_p[i].yl) * 2;
		wx = finger_data_p[i] * HIMAX_HX8526_TS_FINGER_SIZE_COEF;
		wy = wx;
		z  = finger;
#ifdef HIMAX_HX8526_ICX_TP_ROT_180
		x = pdata->max_x - x;
		y = pdata->max_y - y;
#endif

		if (!z)
			continue;
#if 0 /* debug */
		printk("i = %d, x = %d, y = %d, wx = %d, wy = %d, z = %d\n",
			i, x, y, wx, wy, z);
#endif
		if (ts->send_event_cnt == 0) {
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

	if (finger_num == 0x0F) {
		input_report_abs(ts->input_dev, ABS_PRESSURE, 0);
		input_report_key(ts->input_dev, BTN_TOUCH, 0);
		input_sync(ts->input_dev);

		ts->send_event_cnt = 0;
	}
	else {
		if (ts->send_event_cnt == 0) {
			input_report_key(ts->input_dev, BTN_TOUCH, 1);
			input_sync(ts->input_dev);
		}

		ts->send_event_cnt++;
		if (ts->send_event_cnt > ts->ignore_num) {
			ts->send_event_cnt = 0;
		}

		queue_delayed_work(himax_hx8526_wq,
						   &ts->notify_release_work,
						   HIMAX_HX8526_TS_NOTIFY_RELEASE);
	}
}

static void himax_hx8526_ts_irq_handler(void)
{
	struct  himax_hx8526_ts_data *ts =  himax_hx8526_icx_ts_data;

	queue_work(himax_hx8526_wq, &ts->irq_work);
}

static int himax_hx8526_ts_sleep_in(struct himax_hx8526_ts_data *ts)
{
	uint8_t reg;
	int ret = 0;

	reg = 0x82;
	ret = himax_hx8526_ts_write(ts, 1, &reg);
	if (ret < 0) {
		pr_err("Sensing Off error %d\n", ret);
		return ret;
	}
	msleep(50);

	reg = 0x80;
	ret = himax_hx8526_ts_write(ts, 1, &reg);
	if (ret < 0) {
		pr_err("Sleep In error %d\n", ret);
		return ret;
	}
	msleep(50);

	ts->sleep_state = 1;

	return 0;
}

static int himax_hx8526_ts_sleep_out(struct himax_hx8526_ts_data *ts)
{
	uint8_t reg;
	int ret = 0;

	reg = 0x83;
	ret = himax_hx8526_ts_write(ts, 1, &reg);
	if (ret < 0) {
		pr_err("Sensing On error(1) %d\n", ret);
		return ret;
	}
	msleep(50);

	reg = 0x81;
	ret = himax_hx8526_ts_write(ts, 1, &reg);
	if (ret < 0) {
		pr_err("Sleep Out error(1) %d\n", ret);
		return ret;
	}
	msleep(50);

	ts->sleep_state = 0;
	
	return 0;
}

static int himax_hx8526_ts_sleep(struct himax_hx8526_ts_data *ts, int en)
{
	int ret = 0;

	if (en) {
		if (ts->sleep_state == 0) {
			ret = himax_hx8526_ts_sleep_in(ts);
			if (ret < 0) {
				pr_err("ERR: ts_sleep in %d\n", ret);
				return ret;
			}
		}
	}
	else {
		if (ts->sleep_state == 1) {
			ret = himax_hx8526_ts_sleep_out(ts);
			if (ret < 0) {
				pr_err("ERR: ts_sleep out %d\n", ret);
				return ret;
			}
		}
		himax_hx8526_ts_enable_irq(ts);
	}

	return 0;
}


/******************/
/* Debug Routines */
/******************/

static ssize_t dbgregp_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);

	if (!buf)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "0x%x\n", ts->dbgregp);
}

static ssize_t dbgregp_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);
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
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);
	uint8_t val;
	int ret;

	if (!buf)
		return -EINVAL;

	ret = himax_hx8526_ts_read(ts, ts->dbgregp, sizeof(val), &val);
	if (ret < 0)
		return ret;

	return snprintf(buf, PAGE_SIZE, "0x%x\n", val);
}

static ssize_t dbgreg_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);
	int ret;
	uint8_t data[2];
	long tmp;

	if (!buf)
		return -EINVAL;

	if (kstrtol(buf, 0, &tmp))
		return -EINVAL;

	data[0] = ts->dbgregp;
	data[1] = tmp;
	ret = himax_hx8526_ts_write(ts, 2, data);
	if (ret < 0)
		return ret;
	return count;
}

static ssize_t xrst_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);
	struct ts_icx_platform_data *pdata = ts->client->dev.platform_data;
	int ret;

	if (!buf)
		return -EINVAL;

	ret = mt_get_gpio_out(pdata->xrst_gpio);

	return snprintf(buf, PAGE_SIZE, "%d\n", ret);
}

static ssize_t xrst_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);
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

	if (ret < 0)
		return ret;
	else
		return count;
}

static ssize_t sleep_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);

	if (!buf)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%d\n", ts->sleep_state);
}

static ssize_t sleep_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);
	int ret = 0;
	long tmp;

	if (!buf)
		return -EINVAL;

	if (kstrtol(buf, 0, &tmp))
		return -EINVAL;

	mutex_lock(&ts->lock);

	if (tmp) {
		if (ts->sleep_state == 0) {
			cancel_delayed_work_sync(&ts->keepalive_work);
			ret = himax_hx8526_ts_sleep_in(ts);
		}
	}
	else {
		if (ts->is_suspend == 0) {
			if (ts->sleep_state == 1) {
				ret = himax_hx8526_ts_sleep_out(ts);
				queue_delayed_work(himax_hx8526_wq,
								   &ts->keepalive_work,
								   HIMAX_HX8526_TS_KEEPALIVE_PERIOD);
			}
			himax_hx8526_ts_enable_irq(ts);
		}
	}

	mutex_unlock(&ts->lock);

	if (ret < 0)
		return ret;
	else
		return count;
}

static ssize_t clear_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);
	uint8_t reg = 0x88;
	int ret;

	ret = himax_hx8526_ts_write(ts, 1, &reg);

	if (ret < 0)
		return ret;
	else
		return count;
}

static ssize_t ignore_num_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);

	if (!buf)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%d\n", ts->ignore_num);
}

static ssize_t ignore_num_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct himax_hx8526_ts_data *ts = dev_get_drvdata(dev);
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
static DEVICE_ATTR(xrst, 0600, xrst_show, xrst_store);
static DEVICE_ATTR(sleep, 0600, sleep_show, sleep_store);
static DEVICE_ATTR(clear, 0200, NULL, clear_store);
static DEVICE_ATTR(ignore_num, 0600, ignore_num_show, ignore_num_store);


/*******/
/* I/F */
/*******/

static int himax_hx8526_ts_read_version(struct himax_hx8526_ts_data *ts)
{
	uint8_t buf[3];
	int ret = 0;

	/* read IC part number */
	ret = himax_hx8526_ts_read(ts, 0xD1, sizeof(buf), buf);
	if (ret < 0) {
		pr_err("[%s] couldn't read IC part number\n", __func__);
		return ret;
	} else {
		printk("%s: Device ID = %02x %02x %02x\n", HIMAX_HX8526_ICX_NAME,
												   buf[0], buf[1], buf[2]);
	}

	/* read FW version */
	ret = himax_hx8526_ts_read(ts, 0x33, 1, &buf[0]);
	if (ret < 0) {
		pr_err("[%s] failed to read FW version %d\n", __func__, ret);
		return ret;
	}
	ret = himax_hx8526_ts_read(ts, 0x32, 1, &buf[1]);
	if (ret < 0) {
		pr_err("[%s] failed to read FW version %d\n", __func__, ret);
		return ret;
	}
	printk("%s: FW version = %02x %02x\n", HIMAX_HX8526_ICX_NAME,
										   buf[0], buf[1]);
	ts->fw_ver_H = buf[0];
	ts->fw_ver_L = buf[1];

	/* read Config version */
	ret = himax_hx8526_ts_read(ts, 0x39, 1, &buf[0]);
	if (ret < 0) {
		pr_err("[%s] failed to read Config version %d\n", __func__, ret);
		return ret;
	}
	printk("%s: Config version = %02x\n", HIMAX_HX8526_ICX_NAME, buf[0]);
	ts->config_ver = buf[0];

	return 0;
}

static int himax_hx8526_ts_set_idle_mode(
	struct himax_hx8526_ts_data *ts,
	uint8_t mode)
{
	uint8_t data[3] = {0};
	int ret = 0;

	data[0] = 0x8C;
	data[1] = 0x14;
	ret = himax_hx8526_ts_write(ts, 2, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8C 14\n", __func__);
		return ret;
	}
	msleep(10);

	data[0] = 0x8B;
	data[1] = 0x00;
	data[2] = 0x02;
	ret = himax_hx8526_ts_write(ts, 3, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8B 00 02\n", __func__);
		return ret;
	}
	msleep(10);

	data[0] = 0x40;
	data[1] = mode;
	ret = himax_hx8526_ts_write(ts, 2, data);
	if (ret < 0) {
		pr_err("[%s] failed to set IDLE threshold\n", __func__);
		return ret;
	}

	data[0] = 0x8C;
	data[1] = 0x00;
	ret = himax_hx8526_ts_write(ts, 2, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8C 00\n", __func__);
		return ret;
	}
	msleep(10);

	return 0;
}

static int himax_hx8526_ts_initialize_param(struct himax_hx8526_ts_data *ts)
{
	int ret = 0;

	/* set IDLE mode */
	ret = himax_hx8526_ts_set_idle_mode(ts, HIMAX_HX8526_TS_IDLE_MODE_DISABLE);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int himax_hx8526_ts_information(struct himax_hx8526_ts_data *ts)
{
	int ret = 0;
#ifdef HIMAX_HX8526_ICX_TP_FIXED_MULTI_NUM
	ts->multi_num = HIMAX_HX8526_ICX_TP_FIXED_MULTI_NUM;
#else
	uint8_t data[12] = {0};

	data[0] = 0x8C;
	data[1] = 0x14;
	ret = himax_hx8526_ts_write(ts, 2, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8C 14\n", __func__);
		return ret;
	}
	msleep(10);

	data[0] = 0x8B;
	data[1] = 0x00;
	data[2] = 0x70;
	ret = himax_hx8526_ts_write(ts, 3, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8B 00 70\n", __func__);
		return ret;
	}
	msleep(10);

	ret = himax_hx8526_ts_read(ts, 0x5A, 12, data);
	if (ret < 0) {
		pr_err("[%s] failed to read coordinate information\n", __func__);
		return ret;
	}
	DPRINTK("HX_RX_NUM  : %02X\n", data[0]);
	DPRINTK("HX_TX_NUM  : %02X\n", data[1]);
	DPRINTK("HX_MAX_PT  : %02X\n", data[2] >> 4);
	ts->multi_num = data[2] >> 4;
	DPRINTK("HX_BT_NUM  : %02X\n", data[2] & 0x0F);
	DPRINTK("HX_XY_REVERSE : %02X\n", data[4] & 0x04);
	DPRINTK("HX_X_RES : %02X%02X\n", data[6], data[7]);
	DPRINTK("HX_Y_RES : %02X%02X\n", data[8], data[9]);
	ts->rx_num = data[0];
	ts->tx_num = data[1];
	ts->x_res = ((uint32_t)data[6] << 8) | data[7];
	ts->y_res = ((uint32_t)data[8] << 8) | data[9];
	ts->xy_reverse = (data[4] >> 2) & 0x01;

	data[0] = 0x8C;
	data[1] = 0x00;
	ret = himax_hx8526_ts_write(ts, 2, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8C 00\n", __func__);
		return ret;
	}
	msleep(10);

	data[0] = 0x8C;
	data[1] = 0x14;
	ret = himax_hx8526_ts_write(ts, 2, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8C 14\n", __func__);
		return ret;
	}
	msleep(10);

	data[0] = 0x8B;
	data[1] = 0x00;
	data[2] = 0x02;
	ret = himax_hx8526_ts_write(ts, 3, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8B 00 02\n", __func__);
		return ret;
	}
	msleep(10);

	ret = himax_hx8526_ts_read(ts, 0x5A, 10, data);
	if (ret < 0) {
		pr_err("[%s] failed to read edge information\n", __func__);
		return ret;
	}
	DPRINTK("HX_INT_IS_EDGE  : %02X\n", data[1] & 0x01);
	ts->int_is_edge = data[1] & 0x01;

	data[0] = 0x8C;
	data[1] = 0x00;
	ret = himax_hx8526_ts_write(ts, 2, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8C 00\n", __func__);
		return ret;
	}
	msleep(10);

	data[0] = 0x8C;
	data[1] = 0x14;
	ret = himax_hx8526_ts_write(ts, 2, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8C 14\n", __func__);
		return ret;
	}
	msleep(10);

	data[0] = 0x8B;
	data[1] = 0x00;
	data[2] = 0x02;
	ret = himax_hx8526_ts_write(ts, 3, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8B 00 02\n", __func__);
		return ret;
	}
	msleep(10);

	ret = himax_hx8526_ts_read(ts, 0x5A, 1, data);
	if (ret < 0) {
		pr_err("[%s] failed to read idle threshold\n", __func__);
		return ret;
	}
	DPRINTK("IDLE_MODE  : %02X\n", data[0]);
	ts->idle_mode = data[0];

	data[0] = 0x8C;
	data[1] = 0x00;
	ret = himax_hx8526_ts_write(ts, 2, data);
	if (ret < 0) {
		pr_err("[%s] failed to write 8C 00\n", __func__);
		return ret;
	}
	msleep(10);

	/* read Config version */
	ret = himax_hx8526_ts_read(ts, 0x39, 1, &data[0]);
	if (ret < 0) {
		pr_err("[%s] failed to read Config version %d\n", __func__, ret);
		return ret;
	}
	DPRINTK("%s: Config version = %02x\n", HIMAX_HX8526_ICX_NAME, data[0]);
#endif

	ts->coordinate_data_size = COORDINATE_DATA_SIZE(ts->multi_num);
	ts->finger_data_size     = FINGER_DATA_SIZE(ts->multi_num);
	ts->all_data_size        = ts->coordinate_data_size + ts->finger_data_size + ID_INFO_DATA_SIZE;
	ts->finger_data_offset   = ts->coordinate_data_size;
	ts->id_info_offset       = ts->finger_data_offset + ts->finger_data_size;

	if (ts->multi_num > MULTI_NUM_LIMIT) {
		ts->multi_num = MULTI_NUM_LIMIT;
	}

	return 0;
}

static int himax_hx8526_ts_poweron(struct himax_hx8526_ts_data *ts)
{
	int ret = 0;

	ret = himax_hx8526_ts_sleep_out(ts);
	if (ret < 0) {
		return ret;
	}

	ret = himax_hx8526_ts_sleep_in(ts);
	if (ret < 0) {
		return ret;
	}

	/* initialize TP parameters */
	ret = himax_hx8526_ts_initialize_param(ts);
	if (ret < 0) {
		pr_err("[%s] fail to initilize parameters\n", __func__);
		return ret;
	}

	/* read touch info. and settings from touch controller */
	ret = himax_hx8526_ts_information(ts);
	if (ret < 0) {
		pr_err("[%s] fail to read TS information\n", __func__);
		return ret;
	}

	ret = himax_hx8526_ts_sleep_out(ts);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int himax_hx8526_ts_reinitialize(struct himax_hx8526_ts_data *ts)
{
	int sleep_state;
	int ret = 0;

	/* reset */
	himax_hx8526_ts_reset(ts);
	msleep(20);
	himax_hx8526_ts_unreset(ts);

	sleep_state = ts->sleep_state;

	/* reinitialize */
	ret = himax_hx8526_ts_poweron(ts);
	if (ret < 0) {
		pr_err("[%s] power on failure\n", __func__);
	}

	if (sleep_state) {
		ret = himax_hx8526_ts_sleep_in(ts);
		if (ret < 0) {
			pr_err("[%s] sleep in failure\n", __func__);
		}
	}

	ts->send_event_cnt = 0;

	return ret;
}

static int himax_hx8526_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct himax_hx8526_ts_data *ts;
	struct ts_icx_platform_data *pdata;
	int i;
	int ret = 0;

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
	INIT_WORK(&ts->irq_work, himax_hx8526_ts_irq_work_func);
	INIT_DELAYED_WORK(&ts->keepalive_work, himax_hx8526_ts_keepalive_work_func);
	INIT_DELAYED_WORK(&ts->notify_release_work, himax_hx8526_ts_notify_release_work_func);
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

	for (i = 0; i < HIMAX_HX8526_ICX_TP_DMA_ALLOC_RETRY; i++) {
		ts->buf_virt = dma_alloc_coherent(NULL, MAX_DATA_SIZE, &(ts->buf_phys), 0);
		if (ts->buf_virt != NULL) {
			break;
		}
	}
	if (i >= HIMAX_HX8526_ICX_TP_DMA_ALLOC_RETRY) {
		ERROR("memory allocation error\n");
		goto err_dma_alloc_coherent_failed;
	}

	ret = himax_hx8526_ts_read_version(ts);
	if (ret < 0) {
		pr_err("[%s] fail to read TS version\n", __func__);
		goto err_detect_failed;
	}

	ret = himax_hx8526_ts_poweron(ts);
	if (ret < 0) {
		pr_err("[%s] power on failure\n", __func__);
		goto err_detect_failed;
	}

	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		ERROR("Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = HIMAX_HX8526_ICX_NAME;
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ts->input_dev->keybit);

	input_set_abs_params(ts->input_dev, ABS_X, pdata->min_x, pdata->max_x, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, pdata->min_y, pdata->max_y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, pdata->min_x, pdata->max_x, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, pdata->min_y, pdata->max_y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, HIMAX_HX8526_TS_FINGER_SIZE, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, HIMAX_HX8526_TS_FINGER_SIZE, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MINOR, 0, HIMAX_HX8526_TS_FINGER_SIZE, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, HIMAX_HX8526_TS_Z_MAX, 0, 0);

	input_set_events_per_packet(ts->input_dev,
								(7 * ts->multi_num) + 1 + 1); /* (events/touch=6) * point + BTN_TOUCH + SYNC */

	ret = input_register_device(ts->input_dev);
	if (ret) {
		ERROR("Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}

	himax_hx8526_icx_ts_data = ts; 

	mt_eint_set_sens(client->irq, MT_EDGE_SENSITIVE);
	mt_eint_registration(client->irq, pdata->irqflags, himax_hx8526_ts_irq_handler, 1);

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level   = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = himax_hx8526_ts_early_suspend;
	ts->early_suspend.resume  = himax_hx8526_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	device_create_file(&client->dev, &dev_attr_regv);
	device_create_file(&client->dev, &dev_attr_regp);
	device_create_file(&client->dev, &dev_attr_xrst);
	device_create_file(&client->dev, &dev_attr_sleep);
	device_create_file(&client->dev, &dev_attr_clear);
	device_create_file(&client->dev, &dev_attr_ignore_num);

	ts->last_touch_time = jiffies;
	ts->ignore_num = HIMAX_HX8526_TS_IGNORE_NUM_DEFAULT;

	himax_hx8526_ts_enable_irq(ts);
	queue_delayed_work(himax_hx8526_wq, &ts->keepalive_work, HIMAX_HX8526_TS_KEEPALIVE_PERIOD);

	himax_touch_proc_init();

	return 0;

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
err_detect_failed:
	dma_free_coherent(NULL, MAX_DATA_SIZE, ts->buf_virt, ts->buf_phys);
err_dma_alloc_coherent_failed:
err_power_failed:
	if (himax_hx8526_wq)
		destroy_workqueue(himax_hx8526_wq);
	kfree(ts);
err_alloc_data_failed:
err_check_functionality_failed:
	return ret;
}

static int himax_hx8526_ts_remove(struct i2c_client *client)
{
	struct himax_hx8526_ts_data *ts = i2c_get_clientdata(client);
	struct ts_icx_platform_data *pdata = client->dev.platform_data;

	mt_eint_mask(client->irq);

	unregister_early_suspend(&ts->early_suspend);
	mt_eint_registration(client->irq, pdata->irqflags, NULL, 0);
	input_unregister_device(ts->input_dev);
	if (ts->buf_virt) {
		dma_free_coherent(NULL, MAX_DATA_SIZE, ts->buf_virt, ts->buf_phys);
	}
	cancel_work_sync(&ts->irq_work);
	cancel_delayed_work_sync(&ts->keepalive_work);
	cancel_delayed_work_sync(&ts->notify_release_work);
	if (himax_hx8526_wq)
		destroy_workqueue(himax_hx8526_wq);

	himax_touch_proc_deinit();

	kfree(ts);

	return 0;
}

static void himax_hx8526_ts_shutdown(struct i2c_client *client)
{
	himax_hx8526_ts_remove(client);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void himax_hx8526_ts_early_suspend(struct early_suspend *h)
{
	struct  himax_hx8526_ts_data *ts;

	ts = container_of(h, struct  himax_hx8526_ts_data, early_suspend);

	mutex_lock(&ts->lock);

	mt_eint_mask(ts->client->irq);

	cancel_work_sync(&ts->irq_work);
	cancel_delayed_work_sync(&ts->keepalive_work);

	if (ts->power)
		ts->power(0);
	else
		himax_hx8526_ts_sleep(ts, 1);

	ts->is_suspend = 1;

	mutex_unlock(&ts->lock);
}

static void himax_hx8526_ts_late_resume(struct early_suspend *h)
{
	struct  himax_hx8526_ts_data *ts;

	ts = container_of(h, struct  himax_hx8526_ts_data, early_suspend);

	mutex_lock(&ts->lock);

	if (ts->power)
		ts->power(1);
	else
		himax_hx8526_ts_sleep(ts, 0);

	ts->is_suspend = 0;

	mt_eint_ack(ts->client->irq);
	mt_eint_unmask(ts->client->irq);

	queue_delayed_work(himax_hx8526_wq, &ts->keepalive_work, HIMAX_HX8526_TS_KEEPALIVE_PERIOD);

	mutex_unlock(&ts->lock);
}
#endif

static const struct i2c_device_id himax_hx8526_ts_id[] = {
	{ HIMAX_HX8526_ICX_NAME, 0 },
	{ }
};

static struct i2c_driver himax_hx8526_ts_driver = {
	.probe		= himax_hx8526_ts_probe,
	.remove		= himax_hx8526_ts_remove,
	.shutdown   = himax_hx8526_ts_shutdown,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= himax_hx8526_ts_suspend,
	.resume		= himax_hx8526_ts_resume,
#endif
	.id_table	= himax_hx8526_ts_id,
	.driver = {
		.name	= HIMAX_HX8526_ICX_NAME,
	},
};

static int __init himax_hx8526_ts_init(void)
{
	int ret = 0;

	himax_hx8526_wq = create_singlethread_workqueue("himax_hx8526_wq");
	if (!himax_hx8526_wq)
		return -ENOMEM;

	ret = i2c_add_driver(&himax_hx8526_ts_driver);
	if (ret < 0)
		destroy_workqueue(himax_hx8526_wq);

	return ret;
}

static void __exit himax_hx8526_ts_exit(void)
{
	i2c_del_driver(&himax_hx8526_ts_driver);
	if (himax_hx8526_wq)
		destroy_workqueue(himax_hx8526_wq);
}

module_init(himax_hx8526_ts_init);
module_exit(himax_hx8526_ts_exit);

MODULE_DESCRIPTION("Himax HX8526-E30 Touchscreen Driver");
MODULE_LICENSE("GPL");


/********************/
/*@ Debug functions */
/********************/

#define HIMAX_PROC_TOUCH_DIR	"himax_touch"
#define HIMAX_PROC_DEBUG_FILE	"debug"

#define HIMAX_FWUPDATE_NOT_EXEC	-1
#define HIMAX_FWUPDATE_OK		0
#define HIMAX_FWUPDATE_NG		1
#define HIMAX_FWUPDATE_NG_NOMEM	2
#define HIMAX_FWUPDATE_NG_FOPEN	3
#define HIMAX_FWUPDATE_NG_FREAD	4
#define HIMAX_FWUPDATE_NG_SUM	5
#define HIMAX_FWUPDATE_NG_I2C	6

static struct proc_dir_entry *himax_touch_proc_dir	= NULL;
static struct proc_dir_entry *himax_proc_debug_file = NULL;

static uint8_t		 himax_proc_send_flag			= 0;
static unsigned char himax_proc_debug_level_cmd		= 0;
static int			 himax_proc_set_param_result	= 0;
static int			 himax_proc_fw_update_complete	= HIMAX_FWUPDATE_NOT_EXEC;

static unsigned char *himax_debug_upgrade_fw;

#define	HIMAX_DEBUG_FW_SIZE			(32*1024)

static int himax_lock_flash(struct himax_hx8526_ts_data *ts, int enable)
{
	uint8_t cmd[5];

	cmd[0] = 0xAA;
	if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return 0;
	}

	/* lock sequence start */
	cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
	cmd[1] = 0x01;
	cmd[2] = 0x00;
	cmd[3] = 0x06;
	if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return 0;
	}

	cmd[0] = 0x44; /* HX_CMD_FLASH_SET_ADDRESS */
	cmd[1] = 0x03;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return 0;
	}

	cmd[0] = 0x45; /* HX_CMD_FLASH_WRITE_REGISTER */
	cmd[1] = 0x63;
	cmd[2] = 0x02;
	if (enable != 0) {
		cmd[3] = 0x70;
		cmd[4] = 0x03;
	}
	else {
		cmd[3] = 0x30;
		cmd[4] = 0x00;
	}
	if (himax_hx8526_ts_write(ts, 5, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return 0;
	}

	cmd[0] = 0x4A; /* HX_CMD_4A */
	if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return 0;
	}
	msleep(50);

	cmd[0] = 0xA9;
	if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return 0;
	}

	return 0;
	/* lock sequence stop */
}

static int himax_ManualMode(struct himax_hx8526_ts_data *ts, int enter)
{
	uint8_t cmd[2];

	cmd[0] = 0x42; /* HX_CMD_MANUALMODE */
	cmd[1] = enter;
	if (himax_hx8526_ts_write(ts, 2, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
	}
	return 0;
}

static int himax_FlashMode(struct himax_hx8526_ts_data *ts, int enter)
{
	uint8_t cmd[2];

	cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
	cmd[1] = enter;
	if (himax_hx8526_ts_write(ts, 2, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
	}
	return 0;
}

static int  himax_iref_number = 11;
static bool himax_iref_found  = false;

/* 1uA */
static unsigned char E_IrefTable_1[16][2] = {{0x20,0x0F},{0x20,0x1F},{0x20,0x2F},{0x20,0x3F},
											 {0x20,0x4F},{0x20,0x5F},{0x20,0x6F},{0x20,0x7F},
											 {0x20,0x8F},{0x20,0x9F},{0x20,0xAF},{0x20,0xBF},
											 {0x20,0xCF},{0x20,0xDF},{0x20,0xEF},{0x20,0xFF}};
/* 2uA */
static unsigned char E_IrefTable_2[16][2] = {{0xA0,0x0E},{0xA0,0x1E},{0xA0,0x2E},{0xA0,0x3E},
											 {0xA0,0x4E},{0xA0,0x5E},{0xA0,0x6E},{0xA0,0x7E},
											 {0xA0,0x8E},{0xA0,0x9E},{0xA0,0xAE},{0xA0,0xBE},
											 {0xA0,0xCE},{0xA0,0xDE},{0xA0,0xEE},{0xA0,0xFE}};
/* 3uA */
static unsigned char E_IrefTable_3[16][2] = {{0x20,0x0E},{0x20,0x1E},{0x20,0x2E},{0x20,0x3E},
											 {0x20,0x4E},{0x20,0x5E},{0x20,0x6E},{0x20,0x7E},
											 {0x20,0x8E},{0x20,0x9E},{0x20,0xAE},{0x20,0xBE},
											 {0x20,0xCE},{0x20,0xDE},{0x20,0xEE},{0x20,0xFE}};
/* 4uA */
static unsigned char E_IrefTable_4[16][2] = {{0xA0,0x0D},{0xA0,0x1D},{0xA0,0x2D},{0xA0,0x3D},
											 {0xA0,0x4D},{0xA0,0x5D},{0xA0,0x6D},{0xA0,0x7D},
											 {0xA0,0x8D},{0xA0,0x9D},{0xA0,0xAD},{0xA0,0xBD},
											 {0xA0,0xCD},{0xA0,0xDD},{0xA0,0xED},{0xA0,0xFD}};
/* 5uA */
static unsigned char E_IrefTable_5[16][2] = {{0x20,0x0D},{0x20,0x1D},{0x20,0x2D},{0x20,0x3D},
											 {0x20,0x4D},{0x20,0x5D},{0x20,0x6D},{0x20,0x7D},
											 {0x20,0x8D},{0x20,0x9D},{0x20,0xAD},{0x20,0xBD},
											 {0x20,0xCD},{0x20,0xDD},{0x20,0xED},{0x20,0xFD}};
/* 6uA */
static unsigned char E_IrefTable_6[16][2] = {{0xA0,0x0C},{0xA0,0x1C},{0xA0,0x2C},{0xA0,0x3C},
											 {0xA0,0x4C},{0xA0,0x5C},{0xA0,0x6C},{0xA0,0x7C},
											 {0xA0,0x8C},{0xA0,0x9C},{0xA0,0xAC},{0xA0,0xBC},
											 {0xA0,0xCC},{0xA0,0xDC},{0xA0,0xEC},{0xA0,0xFC}};
/* 7uA */
static unsigned char E_IrefTable_7[16][2] = {{0x20,0x0C},{0x20,0x1C},{0x20,0x2C},{0x20,0x3C},
											 {0x20,0x4C},{0x20,0x5C},{0x20,0x6C},{0x20,0x7C},
											 {0x20,0x8C},{0x20,0x9C},{0x20,0xAC},{0x20,0xBC},
											 {0x20,0xCC},{0x20,0xDC},{0x20,0xEC},{0x20,0xFC}};

static void himax_changeIref(struct himax_hx8526_ts_data *ts, int selected_iref)
{
	unsigned char temp_iref[16][2] = {{0x00,0x00},{0x00,0x00},{0x00,0x00},{0x00,0x00},
									  {0x00,0x00},{0x00,0x00},{0x00,0x00},{0x00,0x00},
									  {0x00,0x00},{0x00,0x00},{0x00,0x00},{0x00,0x00},
									  {0x00,0x00},{0x00,0x00},{0x00,0x00},{0x00,0x00}};
	uint8_t cmd[10];
	int i = 0;
	int j = 0;

	printk("%s: start to check iref,iref number = %d\n", __func__, selected_iref);

	cmd[0] = 0xAA;
	if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return;
	}

	for (i = 0; i < 16; i++) {
		for (j = 0; j < 2; j++) {
			if (selected_iref == 1) {
				temp_iref[i][j] = E_IrefTable_1[i][j];
			}
			else if (selected_iref == 2) {
				temp_iref[i][j] = E_IrefTable_2[i][j];
			}
			else if (selected_iref == 3) {
				temp_iref[i][j] = E_IrefTable_3[i][j];
			}
			else if (selected_iref == 4) {
				temp_iref[i][j] = E_IrefTable_4[i][j];
			}
			else if (selected_iref == 5) {
				temp_iref[i][j] = E_IrefTable_5[i][j];
			}
			else if (selected_iref == 6) {
				temp_iref[i][j] = E_IrefTable_6[i][j];
			}
			else if (selected_iref == 7) {
				temp_iref[i][j] = E_IrefTable_7[i][j];
			}
		}
	}

	if (!himax_iref_found) {
		/* Read Iref */
		cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
		cmd[1] = 0x01;
		cmd[2] = 0x00;
		cmd[3] = 0x0A;
		if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return;
		}

		cmd[0] = 0x44; /* HX_CMD_FLASH_SET_ADDRESS */
		cmd[1] = 0x00;
		cmd[2] = 0x00;
		cmd[3] = 0x00;
		if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return;
		}

		cmd[0] = 0x46;
		if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return;
		}

		if (himax_hx8526_ts_read(ts, 0x59, 4, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return;
		}

		/* find iref group , default is iref 3 */
		for (i = 0; i < 16; i++) {
			if ((cmd[0] == temp_iref[i][0]) && (cmd[1] == temp_iref[i][1])) {
				himax_iref_number = i;
				himax_iref_found = true;
				break;
			}
		}

		if (!himax_iref_found) {
			pr_err("%s: Can't find iref number!\n", __func__);
			return;
		}
		else {
			printk("%s: iref_number=%d, cmd[0]=0x%x, cmd[1]=0x%x\n", __func__,
																	 himax_iref_number,
																	 cmd[0],
																	 cmd[1]);
		}
	}

	msleep(5);

	/* iref write */
	cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
	cmd[1] = 0x01;
	cmd[2] = 0x00;
	cmd[3] = 0x06;
	if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return;
	}

	cmd[0] = 0x44; /* HX_CMD_FLASH_SET_ADDRESS */
	cmd[1] = 0x00;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return;
	}

	cmd[0] = 0x45; /* HX_CMD_FLASH_WRITE_REGISTER */
	cmd[1] = temp_iref[himax_iref_number][0];
	cmd[2] = temp_iref[himax_iref_number][1];
	cmd[3] = 0x17;
	cmd[4] = 0x28;
	if (himax_hx8526_ts_write(ts, 5, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return;
	}

	cmd[0] = 0x4A;
	if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return;
	}

	/* Read SFR to check the result */
	cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
	cmd[1] = 0x01;
	cmd[2] = 0x00;
	cmd[3] = 0x0A;
	if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return;
	}

	cmd[0] = 0x44; /* HX_CMD_FLASH_SET_ADDRESS */
	cmd[1] = 0x00;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return;
	}

	cmd[0] = 0x46;
	if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return;
	}

	if (himax_hx8526_ts_read(ts, 0x59, 4, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return;
	}

	printk("%s: cmd[0]=%d, cmd[1]=%d, temp_iref_1=%d, temp_iref_2=%d\n",
			__func__,
			cmd[0],
			cmd[1],
			temp_iref[himax_iref_number][0],
			temp_iref[himax_iref_number][1]);

	if ((cmd[0] != temp_iref[himax_iref_number][0])
	 || (cmd[1] != temp_iref[himax_iref_number][1])) {
		pr_err("%s: IREF Read Back is not match.\n", __func__);
		pr_err("%s: Iref [0]=%d,[1]=%d\n", __func__, cmd[0], cmd[1]);
	}
	else {
		printk("%s: IREF Pass\n", __func__);
	}

	cmd[0] = 0xA9;
	if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return;
	}
}

static uint8_t himax_calculateChecksum(struct himax_hx8526_ts_data *ts, bool change_iref)
{
	int iref_flag = 0;
	uint8_t cmd[10];

	memset(cmd, 0x00, sizeof(cmd));

	/* Sleep out */
	cmd[0] = 0x81; /* HX_CMD_TSSLPOUT */
	if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return HIMAX_FWUPDATE_NG_I2C;
	}
	msleep(120);

	while (true) {
		if (change_iref) {
			if (iref_flag == 0) {
				himax_changeIref(ts, 2); /* iref 2 */
			}
			else if (iref_flag == 1) {
				himax_changeIref(ts, 5); /* iref 5 */
			}
			else if (iref_flag == 2) {
				himax_changeIref(ts, 1); /* iref 1 */
			}
			else {
				goto himax_sum_check_fail;
			}
			iref_flag++;
		}

		cmd[0] = 0xED;
		cmd[1] = 0x00;
		cmd[2] = 0x04;
		cmd[3] = 0x0A;
		cmd[4] = 0x02;
		if (himax_hx8526_ts_write(ts, 5, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return HIMAX_FWUPDATE_NG_I2C;
		}

		/* Enable Flash */
		cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
		cmd[1] = 0x01;
		cmd[2] = 0x00;
		cmd[3] = 0x02;
		if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return HIMAX_FWUPDATE_NG_I2C;
		}

		cmd[0] = 0xD2;
		cmd[1] = 0x05;
		if (himax_hx8526_ts_write(ts, 2, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return HIMAX_FWUPDATE_NG_I2C;
		}

		cmd[0] = 0x53;
		cmd[1] = 0x01;
		if (himax_hx8526_ts_write(ts, 2, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return HIMAX_FWUPDATE_NG_I2C;
		}

		msleep(200);

		if (himax_hx8526_ts_read(ts, 0xAD, 4, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return HIMAX_FWUPDATE_NG_I2C;
		}

		printk("%s 0xAD[0,1,2,3] = %d,%d,%d,%d\n", __func__, cmd[0], cmd[1], cmd[2], cmd[3]);

		if ((cmd[0] == 0) && (cmd[1] == 0) && (cmd[2] == 0) && (cmd[3] == 0)) {
			himax_FlashMode(ts, 0);
			goto himax_sum_check_pass;
		}
		else {
			himax_FlashMode(ts, 0);
			goto himax_sum_check_fail;
		}

himax_sum_check_pass:
		if (change_iref) {
			if (iref_flag < 3) {
				continue;
			}
			else {
				return HIMAX_FWUPDATE_OK;
			}
		}
		else {
			return HIMAX_FWUPDATE_OK;
		}

himax_sum_check_fail:
		return HIMAX_FWUPDATE_NG_SUM;
	}

	return HIMAX_FWUPDATE_NG_SUM;
}

static int himax_debug_fts_ctpm_fw_upgrade_with_fs(
	struct himax_hx8526_ts_data *ts,
	unsigned char				*fw,
	int							len,
	bool						change_iref)
{
	unsigned char *ImageBuffer = fw;
	int fullFileLength = len;
	int i;
	uint8_t cmd[6], last_byte, prePage;
	int FileLength;
	uint8_t checksumResult = 0;

	FileLength = fullFileLength;

	cmd[0] = 0x81; /* HX_CMD_TSSLPOUT */
	if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return HIMAX_FWUPDATE_NG_I2C;
	}

	msleep(120);

	himax_lock_flash(ts, 0);

	cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
	cmd[1] = 0x05;
	cmd[2] = 0x00;
	cmd[3] = 0x02;
	if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return HIMAX_FWUPDATE_NG_I2C;
	}

	cmd[0] = 0x4F;
	if (himax_hx8526_ts_write(ts, 1, cmd) < 0) {
		pr_err("%s: i2c access fail!\n", __func__);
		return HIMAX_FWUPDATE_NG_I2C;
	}
	msleep(50);

	himax_ManualMode(ts, 1);
	himax_FlashMode(ts, 1);

	FileLength = (FileLength + 3) / 4;
	for (i = 0, prePage = 0; i < FileLength; i++) {
		last_byte = 0;

		cmd[0] = 0x44; /* HX_CMD_FLASH_SET_ADDRESS */
		cmd[1] = i & 0x1F;
		if ((cmd[1] == 0x1F) || (i == (FileLength - 1))) {
			last_byte = 1;
		}
		cmd[2] = (i >> 5) & 0x1F;
		cmd[3] = (i >> 10) & 0x1F;
		if (himax_hx8526_ts_write(ts, 4, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return HIMAX_FWUPDATE_NG_I2C;
		}

		if ((prePage != cmd[2]) || (i == 0)) {
			prePage = cmd[2];
			cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
			cmd[1] = 0x01;
			cmd[2] = 0x09;
			//cmd[3] = 0x02;
			if (himax_hx8526_ts_write(ts, 3, cmd) < 0) {
				pr_err("%s: i2c access fail!\n", __func__);
				return HIMAX_FWUPDATE_NG_I2C;
			}

			cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
			cmd[1] = 0x01;
			cmd[2] = 0x0D;
			//cmd[3] = 0x02;
			if (himax_hx8526_ts_write(ts, 3, cmd) < 0) {
				pr_err("%s: i2c access fail!\n", __func__);
				return HIMAX_FWUPDATE_NG_I2C;
			}

			cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
			cmd[1] = 0x01;
			cmd[2] = 0x09;
			//cmd[3] = 0x02;
			if (himax_hx8526_ts_write(ts, 3, cmd) < 0) {
				pr_err("%s: i2c access fail!\n", __func__);
				return HIMAX_FWUPDATE_NG_I2C;
			}
		}

		cmd[0] = 0x45; /* HX_CMD_FLASH_WRITE_REGISTER */
		memcpy(&cmd[1], &ImageBuffer[4*i], 4);
		if (himax_hx8526_ts_write(ts, 5, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return HIMAX_FWUPDATE_NG_I2C;
		}

		cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
		cmd[1] = 0x01;
		cmd[2] = 0x0D;
		//cmd[3] = 0x02;
		if (himax_hx8526_ts_write(ts, 3, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return HIMAX_FWUPDATE_NG_I2C;
		}

		cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
		cmd[1] = 0x01;
		cmd[2] = 0x09;
		//cmd[3] = 0x02;
		if (himax_hx8526_ts_write(ts, 3, cmd) < 0) {
			pr_err("%s: i2c access fail!\n", __func__);
			return HIMAX_FWUPDATE_NG_I2C;
		}

		if (last_byte == 1)	{
			cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
			cmd[1] = 0x01;
			cmd[2] = 0x01;
			//cmd[3] = 0x02;
			if (himax_hx8526_ts_write(ts, 3, cmd) < 0) {
				pr_err("%s: i2c access fail!\n", __func__);
				return HIMAX_FWUPDATE_NG_I2C;
			}

			cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
			cmd[1] = 0x01;
			cmd[2] = 0x05;
			//cmd[3] = 0x02;
			if (himax_hx8526_ts_write(ts, 3, cmd) < 0) {
				pr_err("%s: i2c access fail!\n", __func__);
				return HIMAX_FWUPDATE_NG_I2C;
			}

			cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
			cmd[1] = 0x01;
			cmd[2] = 0x01;
			//cmd[3] = 0x02;
			if (himax_hx8526_ts_write(ts, 3, cmd) < 0) {
				pr_err("%s: i2c access fail!\n", __func__);
				return HIMAX_FWUPDATE_NG_I2C;
			}

			cmd[0] = 0x43; /* HX_CMD_FLASH_ENABLE */
			cmd[1] = 0x01;
			cmd[2] = 0x00;
			//cmd[3] = 0x02;
			if (himax_hx8526_ts_write(ts, 3, cmd) < 0) {
				pr_err("%s: i2c access fail!\n", __func__);
				return HIMAX_FWUPDATE_NG_I2C;
			}

			msleep(10);
			if (i == (FileLength - 1)) {
				himax_FlashMode(ts, 0);
				himax_ManualMode(ts, 0);
				checksumResult = himax_calculateChecksum(ts, change_iref);
				//himax_ManualMode(ts, 0);
				himax_lock_flash(ts, 1);

				if (checksumResult == HIMAX_FWUPDATE_OK) { /* Success */
					return checksumResult;
				}
				else {				  /* Fail */
					pr_err("%s: checksumResult fail!\n", __func__);
					return checksumResult;
				}
			}
		}
	}

	return HIMAX_FWUPDATE_NG;
}

static int himax_debug_set_idle_mode(
	struct himax_hx8526_ts_data *ts,
	uint8_t						mode)
{
	uint8_t cmd[2];
	int ret;

	msleep(120);

	cmd[0] = 0x82;
	ret = himax_hx8526_ts_write(ts, 1, cmd);
	if (ret < 0) {
		pr_err("%s: sense off error!\n", __func__);
		return ret;
	}
	msleep(30);

	ret = himax_hx8526_ts_set_idle_mode(ts, mode);
	if (ret < 0) {
		pr_err("%s: failed to write IDLE threshold!\n", __func__);
		return ret;
	}
	msleep(120);

	cmd[0] = 0x83;
	ret = himax_hx8526_ts_write(ts, 1, cmd);
	if (ret < 0) {
		pr_err("%s: sense on error!\n", __func__);
		return ret;
	}

	return 0;
}

static ssize_t himax_debug_read(struct file *file, char *buf, size_t len, loff_t *pos)
{
	struct himax_hx8526_ts_data *ts = himax_hx8526_icx_ts_data;
	size_t ret = 0;

	if (!himax_proc_send_flag) {
		if (himax_proc_debug_level_cmd == 't') {
			switch (himax_proc_fw_update_complete) {
				case HIMAX_FWUPDATE_OK :
					ret += snprintf(buf, len, "FW Update Complete\n");
					break;
				case HIMAX_FWUPDATE_NG :
					ret += snprintf(buf, len, "FW Update Fail\n");
					break;
				case HIMAX_FWUPDATE_NG_NOMEM :
					ret += snprintf(buf, len, "FW Update Fail:NOMEM\n");
					break;
				case HIMAX_FWUPDATE_NG_FOPEN :
					ret += snprintf(buf, len, "FW Update Fail:FOPEN\n");
					break;
				case HIMAX_FWUPDATE_NG_FREAD :
					ret += snprintf(buf, len, "FW Update Fail:FREAD\n");
					break;
				case HIMAX_FWUPDATE_NG_SUM :
					ret += snprintf(buf, len, "FW Update Fail:SUM\n");
					break;
				case HIMAX_FWUPDATE_NG_I2C :
					ret += snprintf(buf, len, "FW Update Fail:I2C\n");
					break;
				default :
					ret += snprintf(buf, len, "FW Update Not Exec\n");
					break;
			}
		}
		else if (himax_proc_debug_level_cmd == 'v') {
			ret += snprintf(buf + ret, len, "FW_VER = ");
	        ret += snprintf(buf + ret, len, "0x%2.2X, %2.2X \n", ts->fw_ver_H, ts->fw_ver_L);

			ret += snprintf(buf + ret, len, "CONFIG_VER = ");
	        ret += snprintf(buf + ret, len, "0x%2.2X \n", ts->config_ver);
			ret += snprintf(buf + ret, len, "\n");
		}
		else if (himax_proc_debug_level_cmd == 'd') {
			ret += snprintf(buf + ret, len, "Himax Touch IC Information :\n");
			ret += snprintf(buf + ret, len, " RX Num : %d\n", ts->rx_num);
			ret += snprintf(buf + ret, len, " TX Num : %d\n", ts->tx_num);
			ret += snprintf(buf + ret, len, " X Resolution : %d\n", ts->x_res);
			ret += snprintf(buf + ret, len, " Y Resolution : %d\n", ts->y_res);
			ret += snprintf(buf + ret, len, " Max Point  : %d\n", ts->multi_num);
			ret += snprintf(buf + ret, len, " Int Is Edge: %d\n", ts->int_is_edge);
			ret += snprintf(buf + ret, len, " XY Reverse : %d\n", ts->xy_reverse);
			ret += snprintf(buf + ret, len, " Idle Mode  : 0x%2.2X\n", ts->idle_mode);
		}
		else if (himax_proc_debug_level_cmd == 'i') {
			if (himax_proc_set_param_result == 0) {
				ret += snprintf(buf, len, "Set IDLE mode OK\n");
			}
			else {
				ret += snprintf(buf, len, "Set IDLE mode NG\n");
			}
		}

		himax_proc_send_flag = 1;
	}
	else {
		himax_proc_send_flag = 0;
	}

	return ret;
}

static ssize_t himax_debug_write(struct file *file, const char *buff, size_t len, loff_t *pos)
{
	struct himax_hx8526_ts_data *ts = himax_hx8526_icx_ts_data;
	struct file* hx_filp = NULL;
	mm_segment_t oldfs;
	int result = 0;
	int size   = 0;
	char fileName[128];
	char buf[80] = {0};
	int  mode;

	if (len >= 80) {
		pr_err("%s: no command exceeds 80 chars.\n", __func__);
		return -EFAULT;
	}
	if (copy_from_user(buf, buff, len))	{
		return -EFAULT;
	}

	/* firmware version */
	if (buf[0] == 'v')	{
		himax_proc_debug_level_cmd = buf[0];

		himax_hx8526_ts_read_version(ts);
	}

	/* driver information */
	else if (buf[0] == 'd')	{
		himax_proc_debug_level_cmd = buf[0];

		himax_hx8526_ts_sleep_out(ts);
		himax_hx8526_ts_sleep_in(ts);
		himax_hx8526_ts_information(ts);
		himax_hx8526_ts_sleep_out(ts);
	}

	/* IDLE mode */
	else if (buf[0] == 'i') {
		result = kstrtol(&buf[2], 0, &mode);
		if (result == 0) {
			if (mode == 0) {
				mode = HIMAX_HX8526_TS_IDLE_MODE_DISABLE;
			}
			else {
				mode = HIMAX_HX8526_TS_IDLE_MODE_ENABLE;
			}
		}
		else {
			pr_err("%s: TP set idle mode error, line: %d\n", __func__, __LINE__);
			return -EINVAL;
		}

		himax_proc_debug_level_cmd = buf[0];
		himax_proc_set_param_result = himax_debug_set_idle_mode(ts, (uint8_t)mode);
	}

	/* FW update */
	else if (buf[0] == 't')	{

		himax_debug_upgrade_fw = kzalloc(HIMAX_DEBUG_FW_SIZE, GFP_KERNEL);
		if (himax_debug_upgrade_fw == NULL) {
			himax_proc_fw_update_complete = HIMAX_FWUPDATE_NG_NOMEM;
			len = -ENOMEM;
			goto himax_proc_firmware_upgrade_nomem;
		}

		mt_eint_mask(ts->client->irq);
		mutex_lock(&ts->lock);
		cancel_delayed_work_sync(&ts->keepalive_work);

		himax_proc_debug_level_cmd = buf[0];
		himax_proc_fw_update_complete = HIMAX_FWUPDATE_NG;

		memset(fileName, 0, 128);

		/* parse the file name */
		snprintf(fileName, len-2, "%s", &buf[2]);
		pr_err("%s: upgrade from file(%s) start!\n", __func__, fileName);

		/* open file */
		hx_filp = filp_open(fileName, O_RDONLY, 0);
		if (IS_ERR(hx_filp)) {
			pr_err("%s: open firmware file failed\n", __func__);
			himax_proc_fw_update_complete = HIMAX_FWUPDATE_NG_FOPEN;
			goto himax_proc_firmware_upgrade_open_fail;
		}

		oldfs = get_fs();
		set_fs(get_ds());

		/* read the latest firmware binary file */
		size = hx_filp->f_op->read(hx_filp,
								   himax_debug_upgrade_fw,
								   HIMAX_DEBUG_FW_SIZE,
								   &hx_filp->f_pos);
		if (size <= 0)	{
			pr_err("%s: read firmware file failed\n", __func__);
			himax_proc_fw_update_complete = HIMAX_FWUPDATE_NG_FREAD;
			goto himax_proc_firmware_upgrade_read_fail;
		}

		set_fs(oldfs);
		filp_close(hx_filp, NULL);

		printk("%s: upgrade start, len %d: %02X, %02X, %02X, %02X\n",
			   __func__, size, himax_debug_upgrade_fw[0],
							   himax_debug_upgrade_fw[1],
							   himax_debug_upgrade_fw[2],
							   himax_debug_upgrade_fw[3]);

		if (size > 0) {
			/* start to upgrade */
			himax_hx8526_ts_unreset(ts);

			result = himax_debug_fts_ctpm_fw_upgrade_with_fs(ts,
															 himax_debug_upgrade_fw,
															 size,
															 true);
			if (result == HIMAX_FWUPDATE_OK) {
				printk("%s: TP upgrade OK, line: %d\n", __func__, __LINE__);
			}
			else {
				printk("%s: TP upgrade error, line: %d\n", __func__, __LINE__);
			}
			himax_proc_fw_update_complete = result;

			goto himax_proc_firmware_upgrade_done;
		}
	}

	return len;

himax_proc_firmware_upgrade_read_fail:
	set_fs(oldfs);
	filp_close(hx_filp, NULL);

himax_proc_firmware_upgrade_done:
himax_proc_firmware_upgrade_open_fail:
	himax_hx8526_ts_unreset(ts);

	mutex_unlock(&ts->lock);
	mt_eint_unmask(ts->client->irq);
	queue_delayed_work(himax_hx8526_wq, &ts->keepalive_work, HIMAX_HX8526_TS_KEEPALIVE_PERIOD);

	kfree(himax_debug_upgrade_fw);

himax_proc_firmware_upgrade_nomem:

	return len;
}

static struct file_operations himax_proc_debug_ops =
{
	.owner = THIS_MODULE,
	.read  = himax_debug_read,
	.write = himax_debug_write,
};

static int himax_touch_proc_init(void)
{
	himax_touch_proc_dir = proc_mkdir(HIMAX_PROC_TOUCH_DIR, NULL);
	if (himax_touch_proc_dir == NULL) {
		pr_err(" %s: himax_touch_proc_dir file create failed!\n", __func__);
		return -ENOMEM;
	}

	himax_proc_debug_file = proc_create(HIMAX_PROC_DEBUG_FILE,
										(S_IWUSR|S_IRUGO),
										himax_touch_proc_dir,
										&himax_proc_debug_ops);
	if (himax_proc_debug_file == NULL) {
		pr_err(" %s: proc debug file create failed!\n", __func__);
		remove_proc_entry(HIMAX_PROC_TOUCH_DIR, NULL);
		himax_touch_proc_dir = NULL;

		return -ENOMEM;
	}

	return 0;
}

static void himax_touch_proc_deinit(void)
{
	if (himax_proc_debug_file) {
		remove_proc_entry(HIMAX_PROC_DEBUG_FILE, himax_touch_proc_dir);
		himax_proc_debug_file = NULL;
	}

	if (himax_touch_proc_dir) {
		remove_proc_entry(HIMAX_PROC_TOUCH_DIR,	 NULL);
		himax_touch_proc_dir = NULL;
	}
}
