/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * Rohm bu9873 rtc class driver
 *
 * Copyright 2015 Sony Corporation.
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_core.h>
#include <linux/rtc-mt.h>
#include <linux/rtc/bu9873.h>

#if (defined(CONFIG_ARCH_MT8590_ICX))
#if (defined(CONFIG_REGMON_DEBUG))
#include <mach/regmon.h>
#define REGMON_DEBUG	/* define shorter macro. */
#endif /* (defined(CONFIG_REGMON_DEBUG)) */
#endif /* (defined(CONFIG_ARCH_MT8590_ICX)) */

/*Debug Macro*/
/*#define TRACE_PRINT_ON*/
/*#define DEBUG_PRINT_ON*/
#ifdef TRACE_PRINT_ON
	#define print_trace(fmt, args...) printk("%s():[%d] " fmt "\n",  __FUNCTION__,__LINE__, ##args)
#else
	#define print_trace(fmt, args...)
#endif
#ifdef DEBUG_PRINT_ON
	#define print_debug(fmt, args...) printk("%s():[%d] " fmt,  __FUNCTION__,__LINE__, ##args)
#else
	#define print_debug(fmt, args...)
#endif
#define print_error(fmt, args...) printk(KERN_ERR "%s():[%d] " fmt,  __FUNCTION__,__LINE__, ##args)

#define MTK_I2C_READ_WRITE_LEN(read_len, write_len) \
	(((read_len) << 8) | ((write_len) << 0))

 struct bu9873_data {
	struct i2c_client *client;
	struct rtc_device *rtc;
	struct work_struct work;
	struct mutex mutex;
};

#if (defined(REGMON_DEBUG))
/* ICX platform feature. */
static regmon_reg_info_t bu9873_regmon_reg_info[] = {
	{"SECOND",	BU9873_REG_SECOND},
	{"MINUTE",	BU9873_REG_MINUTE},
	{"HOUR",	BU9873_REG_HOUR},
	{"DAY_OF_WEEK",	BU9873_REG_DAY_OF_WEEK},
	{"DAY",	BU9873_REG_DAY},
	{"MONTH",	BU9873_REG_MONTH},
	{"YEAR",	BU9873_REG_YEAR},
	{"TIME_TRIMMING",	BU9873_REG_TIME_TRIMMING},
	{"ALARM_A_MINUTE",	BU9873_REG_ALARM_A_MINUTE},
	{"ALARM_A_HOUR",	BU9873_REG_ALARM_A_HOUR},
	{"ALARM_A_DAY_OF_WEEK",	BU9873_REG_ALARM_A_DAY_OF_WEEK},
	{"ALARM_B_MINUTE",	BU9873_REG_ALARM_B_MINUTE},
	{"ALARM_B_HOUR",	BU9873_REG_ALARM_B_HOUR},
	{"ALARM_B_DAY_OF_WEEK",	BU9873_REG_ALARM_B_DAY_OF_WEEK},
	{"CONTROL_1",	BU9873_REG_CONTOROL_1},
	{"CONTROL_2",	BU9873_REG_CONTOROL_2},
};

/* define accessor proto types. */

static int bu9873_regmon_write_reg(
	 void	*private_data,
	unsigned int	address,
	unsigned int	value
);

static int bu9873_regmon_read_reg(
	void	*private_data,
	unsigned int	address,
	unsigned int	*value
);

static regmon_customer_info_t bu9873_customer_info =
{
	.name		= "bu9873",
	.reg_info	= bu9873_regmon_reg_info,
	.reg_info_count = sizeof(bu9873_regmon_reg_info)/sizeof(bu9873_regmon_reg_info[0]),
	.write_reg	= bu9873_regmon_write_reg,
	.read_reg	= bu9873_regmon_read_reg,
	.private_data	= NULL,
};
#endif /* (defined(REGMON_DEBUG)) */


static int bu9873_i2c_write(
	struct i2c_client *client,
	unsigned int		address,
	unsigned char * value,
	int			size
)
{
	struct i2c_msg msg;
	int rv;
	u8 *buffer =NULL;
	u32 phyAddr = 0;
	struct bu9873_platform_data *platform_data = client->dev.platform_data;

	if(client==NULL){
		print_error("not initialized.\n");
		return(-ENODEV);
	}

	if(size>320){
		print_error("invalid size.\n");
		return(-EINVAL);
	}

	msg.addr  = client->addr;
	msg.flags = 0;
	msg.len = size+1;
	buffer = dma_alloc_coherent(0, size+1, &phyAddr, GFP_KERNEL);
	buffer[0]=((((unsigned char)address << 4) & BU9873_I2C_INTERNAL_ADDR_MASK) | BU9873_I2C_TRANS_FORMAT_WRITE);
	memcpy(buffer+1,value,size);

	msg.buf = (u8*)phyAddr;
	msg.ext_flag = I2C_DMA_FLAG;
	msg.timing = platform_data->rtc_i2c_timing;
	rv=i2c_transfer(client->adapter,&msg,1);
	if(rv<0) {
		print_error("i2c_transfer(): code %d error occurred.\n",rv);
		dma_free_coherent(0, size, buffer, phyAddr);
		return(rv);
	}

	if(rv!=1){
		print_error("count mismacth.\n");
		dma_free_coherent(0, size, buffer, phyAddr);
		return(-EIO);
	}

	dma_free_coherent(0, size+1, buffer, phyAddr);
	return(0);
}

static int bu9873_i2c_read(
	struct i2c_client *client,
	unsigned int address,
	unsigned char * value,
	int size
)
{
	unsigned char addr;
	int rv;
	struct bu9873_platform_data *platform_data = client->dev.platform_data;

	if(client==NULL){
		print_error("not initialized.\n");
		return(-ENODEV);
	}

	if(32 < size){
		print_error("invalid size.\n");
		return(-EINVAL);
	}

	addr=((((unsigned char)address << 4) & BU9873_I2C_INTERNAL_ADDR_MASK) | BU9873_I2C_TRANS_FORMAT_WRITE);

	client->addr &= I2C_MASK_FLAG;
	client->ext_flag = ((client->ext_flag & I2C_MASK_FLAG) | I2C_WR_FLAG | I2C_RS_FLAG | I2C_DIRECTION_FLAG);
	client->timing = platform_data->rtc_i2c_timing;
	value[0] = addr;
	rv = i2c_master_send(client, value, MTK_I2C_READ_WRITE_LEN(size, 1));
	if (rv < 0) {
		print_error("Can not read register. rv=%d, addr=0x%x, ext_flag=0x%x, addr=0x%.2x\n", 
		rv, client->addr, client->ext_flag, addr);
		goto out;
	}
out:
	client->ext_flag &= I2C_MASK_FLAG;

	return(rv);
}

static int bu9873_get_time(struct device *dev, struct rtc_time *tm)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bu9873_data *bu9873_data = i2c_get_clientdata(client);
	int ret=0;
	u8 buf[7]={0};
	unsigned int year, month, day, hour, minute, second;
	unsigned int week;

	print_trace();

	mutex_lock(&bu9873_data->mutex);

	ret = bu9873_i2c_read(client,BU9873_REG_SECOND,buf,8);
	if(ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		goto out;
	}
	second = buf[0];
	minute = buf[1];
	hour = buf[2];
	week = buf[3];
	day = buf[4];
	month = buf[5];
	year = buf[6];

	/* Write to rtc_time structure */
	tm->tm_sec = bcd2bin(second);
	tm->tm_min = bcd2bin(minute);
	tm->tm_hour = bcd2bin(hour);
	tm->tm_wday = bcd2bin(week);
	tm->tm_mday = bcd2bin(day);
	/* linux tm_mon range:0~11, while month range is 1~12 in RTC chip */
	tm->tm_mon = bcd2bin(month) - 1;

	tm->tm_year = bcd2bin(year) + BU9873_YEAR_OFFSET;
	ret = rtc_valid_tm(tm);

out:
	mutex_unlock(&bu9873_data->mutex);

	return ret;
}

static int bu9873_set_time(struct device *dev, struct rtc_time *tm)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bu9873_data *bu9873_data = i2c_get_clientdata(client);
	u8 buf[7];
	u8 control_2;
	int ret =0;

	print_trace();

	mutex_lock(&bu9873_data->mutex);

	ret = bu9873_i2c_read(client,BU9873_REG_CONTOROL_2,&control_2,1);
	if (ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		goto out;
	}
	if(!(control_2 & BU9873_REG_CONTOROL_2_12B_24)){
		pr_warn(	"12B/24 flag not set\n");
		control_2 |= BU9873_REG_CONTOROL_2_12B_24;
	}
	ret = bu9873_i2c_write(client,BU9873_REG_CONTOROL_2,&control_2,1);
	if (ret < 0){
		print_error("bu9873_i2c_write error ret = %d \n",ret);
		goto out;
	}

	/* Extract time from rtc_time and load into bu9873*/
	buf[0] = bin2bcd(tm->tm_sec);
	buf[1] = bin2bcd(tm->tm_min);
	buf[2] = bin2bcd(tm->tm_hour);
	buf[3] = bin2bcd(tm->tm_wday);
	buf[4] = bin2bcd(tm->tm_mday); /* Date */
	/* linux tm_mon range:0~11, while month range is 1~12 in RTC chip */
	buf[5] = bin2bcd(tm->tm_mon + 1);
	if(tm->tm_year < BU9873_YEAR_MIN){
		tm->tm_year = BU9873_YEAR_MIN;
		ret = -EINVAL;
		goto out;
	}
	else if(BU9873_YEAR_MAX < tm->tm_year){
		tm->tm_year = BU9873_YEAR_MAX;
	}
	buf[6] = bin2bcd(tm->tm_year - BU9873_YEAR_OFFSET);

	ret = bu9873_i2c_write(client,BU9873_REG_SECOND,buf,7);
	if (ret < 0){
		print_error("bu9873_i2c_write error ret = %d \n",ret);
		goto out;
	}

out:
	mutex_unlock(&bu9873_data->mutex);

	return ret;
}

static int bu9873_read_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bu9873_data *bu9873_data = i2c_get_clientdata(client);
	int ret;
	u8 buf[3];
	u8 control_1;
	u8 mday=0;
	u8 rtc_mday=0;

	print_trace();

	mutex_lock(&bu9873_data->mutex);

	ret = bu9873_i2c_read(client,BU9873_REG_CONTOROL_1,&control_1,1);
	if (ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		goto out;
	}

	ret = bu9873_i2c_read(client,BU9873_REG_ALARM_A_MINUTE,buf,3);
	if (ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		goto out;
	}

	alarm->time.tm_sec = 0;
	alarm->time.tm_min = bcd2bin(buf[0] & 0x7F);
	alarm->time.tm_hour = bcd2bin(buf[1] & 0x7F);
	rtc_mday = buf[2];
	for(mday=0;mday<7;mday++){
		if(rtc_mday & 0x01){
			break;
		}
		rtc_mday = rtc_mday >> 1;
	}
	alarm->time.tm_mday = mday;
	alarm->time.tm_mon = -1;
	alarm->time.tm_year = -1;
	alarm->time.tm_wday = -1;
	alarm->time.tm_yday = -1;
	alarm->time.tm_isdst = -1;

	alarm->enabled = !!(control_1 & BU9873_REG_CONTOROL_1_AALE);

	ret = 0;
out:
	mutex_unlock(&bu9873_data->mutex);
	return ret;
}

static int bu9873_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bu9873_data *bu9873_data = i2c_get_clientdata(client);
	u8 control_1;
	u8 control_2;
	int ret;
	u8 buf[3];

	print_trace();

	mutex_lock(&bu9873_data->mutex);

	buf[0] = bin2bcd(alarm->time.tm_min);
	buf[1] = bin2bcd(alarm->time.tm_hour);
	buf[2] = 0x01 << (alarm->time.tm_wday);

	/* clear alarm interrupt enable bit */
	ret = bu9873_i2c_read(client,BU9873_REG_CONTOROL_1,&control_1,1);
	if (ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		goto out;
	}

	control_1 &= ~(BU9873_REG_CONTOROL_1_AALE | BU9873_REG_CONTOROL_1_BALE);

	ret = bu9873_i2c_write(client,BU9873_REG_CONTOROL_1,&control_1,1);
	if (ret < 0){
		print_error("bu9873_i2c_write error ret = %d \n",ret);
		goto out;
	}

	/* clear alarm interrupt flag bit */
	ret = bu9873_i2c_read(client,BU9873_REG_CONTOROL_2,&control_2,1);
	if (ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		goto out;
	}

	control_2 &= ~(BU9873_REG_CONTOROL_2_AAFG | BU9873_REG_CONTOROL_2_BAFG | BU9873_REG_CONTOROL_2_CTFG);

	ret = bu9873_i2c_write(client,BU9873_REG_CONTOROL_2,&control_2,1);
	if (ret < 0){
		print_error("bu9873_i2c_write error ret = %d \n",ret);
		goto out;
	}

	ret = bu9873_i2c_write(client,BU9873_REG_ALARM_A_MINUTE,buf,3);
	if (ret < 0){
		print_error("bu9873_i2c_write error ret = %d \n",ret);
		goto out;
	}

	if (alarm->enabled) {
		control_1 |= BU9873_REG_CONTOROL_1_AALE;
		ret = bu9873_i2c_write(client,BU9873_REG_CONTOROL_1,&control_1,1);
		if (ret < 0){
			print_error("bu9873_i2c_write error ret = %d \n",ret);
			goto out;
		}
	}
out:
	mutex_unlock(&bu9873_data->mutex);
	return ret;
}

static int bu9873_update_alarm(struct i2c_client *client)
{
	struct bu9873_data *bu9873_data = i2c_get_clientdata(client);
	u8 control;
	int ret;

	print_trace();

	mutex_lock(&bu9873_data->mutex);

	ret = bu9873_i2c_read(client,BU9873_REG_CONTOROL_1,&control,1);
	if (ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		goto unlock;
	}

	if (bu9873_data->rtc->irq_data & (RTC_AF | RTC_UF)){
		/* enable alarm1 interrupt */
		control |= BU9873_REG_CONTOROL_1_AALE;
	}
	else {
		/* disable alarm1 interrupt */
		control &= ~(BU9873_REG_CONTOROL_1_AALE);
	}
	ret = bu9873_i2c_write(client, BU9873_REG_CONTOROL_1, &control,1);
	if (ret < 0){
		print_error("bu9873_i2c_write error ret = %d \n",ret);
		goto unlock;
	}

unlock:
	mutex_unlock(&bu9873_data->mutex);
	return ret;
}

static int bu9873_alarm_irq_enable(struct device *dev, unsigned int enabled)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bu9873_data *bu9873_data = i2c_get_clientdata(client);
	int ret;

	print_trace("enabled = %d",enabled);

	if (enabled)
		bu9873_data->rtc->irq_data |= RTC_AF;
	else
		bu9873_data->rtc->irq_data &= ~RTC_AF;

	ret = bu9873_update_alarm(client);
	if(ret < 0){
		print_error("bu9873_update_alarm error ret = %d \n",ret);
		return ret;
	}
	return ret;
}

static const struct rtc_class_ops bu9873_rtc_ops = {
	.read_time = bu9873_get_time,
	.set_time = bu9873_set_time,
	.read_alarm = bu9873_read_alarm,
	.set_alarm = bu9873_set_alarm,
	.alarm_irq_enable = bu9873_alarm_irq_enable,
};

static struct bu9873_data *bu9873_static_data = NULL;

static void bu9873_irq(void){
	struct bu9873_data *bu9873 = bu9873_static_data;

	print_trace();
	if(!bu9873){
		print_error("bu9873 is NULL pointer\n");
		goto out;
	}
	schedule_work(&bu9873->work);
out:
	return;
}


static void bu9873_work(struct work_struct *work)
{
	struct bu9873_data *bu9873 = container_of(work, struct bu9873_data, work);
	struct i2c_client *client = bu9873->client;
	u8 stat, control;
	int ret;

	print_trace();
	printk("[RTC]bu9873 irq called!\n");

	mutex_lock(&bu9873->mutex);

	ret = bu9873_i2c_read(client,BU9873_REG_CONTOROL_2,&stat,1);
	if (ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		goto unlock;
	}

	if (stat & BU9873_REG_CONTOROL_2_AAFG) {
		ret = bu9873_i2c_read(client,BU9873_REG_CONTOROL_1,&control,1);
		if (ret < 0){
			print_error("bu9873_i2c_read error ret = %d \n",ret);
			goto unlock;
		}

		/* disable alarm1 interrupt */
		control &= ~(BU9873_REG_CONTOROL_1_AALE);
		ret = bu9873_i2c_write(client,BU9873_REG_CONTOROL_1,&control,1);
		if (ret < 0){
			print_error("bu9873_i2c_write error ret = %d \n",ret);
			goto unlock;
		}

		/* clear the alarm pend flag */
		stat &= ~BU9873_REG_CONTOROL_2_AAFG;
		ret = bu9873_i2c_write(client,BU9873_REG_CONTOROL_2,&stat,1);
		if (ret < 0){
			print_error("bu9873_i2c_write error ret = %d \n",ret);
			goto unlock;
		}
	}

unlock:
	mutex_unlock(&bu9873->mutex);
}

static int bu9873_reset_to_deftime(struct i2c_client *client)
{
	u8 buf[7];
	u8 control_2;
	int ret = 0;

	print_trace();

	ret = bu9873_i2c_read(client,BU9873_REG_CONTOROL_2,&control_2,1);
	if (ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		return ret;
	}
	if(!(control_2 & BU9873_REG_CONTOROL_2_12B_24)){
		pr_warn(	"12B/24 flag not set\n");
		control_2 |= BU9873_REG_CONTOROL_2_12B_24;
	}
	ret = bu9873_i2c_write(client,BU9873_REG_CONTOROL_2,&control_2,1);
	if (ret < 0){
		print_error("bu9873_i2c_write error ret = %d \n",ret);
		return ret;
	}
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = bin2bcd(RTC_DEFAULT_DOW);
	buf[4] = bin2bcd(RTC_DEFAULT_DOM);
	buf[5] = bin2bcd(RTC_DEFAULT_MTH);
	buf[6] = bin2bcd(RTC_DEFAULT_YEA - 1900 - 100);

	ret = bu9873_i2c_write(client,BU9873_REG_SECOND,buf,7);
	if (ret < 0){
		print_error("bu9873_i2c_write error ret = %d \n",ret);
		return ret;
	}
	return ret;
}

static int bu9873_check_rtc_status(struct i2c_client *client)
{
	int ret = 0;
	u8 stat;

	print_trace();

	ret = bu9873_i2c_read(client,BU9873_REG_CONTOROL_2,&stat,1);
	if (ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		return ret;
	}

	if (stat & BU9873_REG_CONTOROL_2_XSTEP){
		pr_warn(	"oscillator discontinuity flagged, "
				"time unreliable\n");
		ret = bu9873_reset_to_deftime(client);
		if (ret < 0){
			print_error("bu9873_reset_to_deftime error ret = %d \n",ret);
		}
	}

	ret = bu9873_i2c_read(client,BU9873_REG_CONTOROL_2,&stat,1);
	if (ret < 0){
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		return ret;
	}
	stat &= ~(BU9873_REG_CONTOROL_2_XSTEP | BU9873_REG_CONTOROL_2_CTFG | BU9873_REG_CONTOROL_2_AAFG | BU9873_REG_CONTOROL_2_BAFG);

	ret = bu9873_i2c_write(client,BU9873_REG_CONTOROL_2,&stat,1);
	if (ret < 0){
		print_error("bu9873_i2c_write error ret = %d \n",ret);
		return ret;
	}
	return ret;
}

static int bu9873_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct bu9873_data *bu9873;
	struct bu9873_platform_data *platform_data;
	int ret;

	print_trace();

	platform_data = client->dev.platform_data;
	if (platform_data == NULL) {
		dev_err(&client->dev, "%s fail\n",__func__);
		return -ENODEV;
	}

	bu9873 = kzalloc(sizeof(struct bu9873_data), GFP_KERNEL);
	if (!bu9873){
		print_error("kzalloc return NOMEM\n");
		return -ENOMEM;
	}

	bu9873->client = client;
	i2c_set_clientdata(client, bu9873);
	INIT_WORK(&bu9873->work, bu9873_work);
	mutex_init(&bu9873->mutex);

	ret = bu9873_check_rtc_status(client);
	if(ret < 0){
		print_error("bu9873_check_rtc_status error ret = %d \n",ret);
		kfree(bu9873);
		return ret;
	}

	device_init_wakeup(&client->dev, 1);
	bu9873->rtc = rtc_device_register(client->name,&client->dev,
					  &bu9873_rtc_ops, THIS_MODULE);
	if (IS_ERR(bu9873->rtc)) {
		dev_err(&client->dev, "unable to register the class device\n");
		ret = PTR_ERR(bu9873->rtc);
		kfree(bu9873);
		return ret;
	}

	bu9873_static_data = bu9873;
#if (defined(REGMON_DEBUG))
	bu9873_customer_info.private_data = bu9873;
	regmon_add(&bu9873_customer_info);
#endif /* (defined(REGMON_DEBUG)) */

	mt_set_gpio_mode(platform_data->rtc_intrb_gpio, GPIO_MODE_00);
	mt_set_gpio_dir(platform_data->rtc_intrb_gpio, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(platform_data->rtc_intrb_gpio, GPIO_PULL_DISABLE);
	mt_eint_registration(platform_data->rtc_intrb_eint, EINTF_TRIGGER_FALLING, bu9873_irq, 1);
	mt_eint_dis_debounce(platform_data->rtc_intrb_eint);
	mt_eint_unmask(platform_data->rtc_intrb_eint);

	return 0;
}
static int bu9873_remove(struct i2c_client *client)
{
	struct bu9873_data *bu9873_data = i2c_get_clientdata(client);

	print_trace();
	flush_scheduled_work();
	bu9873_static_data = NULL;
#if (defined(REGMON_DEBUG))
	bu9873_customer_info.private_data = NULL;
	regmon_del(&bu9873_customer_info);
#endif /* (defined(REGMON_DEBUG)) */
	i2c_set_clientdata(client, NULL);

	kfree(bu9873_data);
	return 0;
}

/* define accessor proto types. */
static int bu9873_regmon_write_reg(
	void	*private_data,
	unsigned int address,
	unsigned int value
	)
{
	struct bu9873_data *bu9873_data;
	uint8_t reg_val;
	int ret;
	int result;

	result = 0;
	if (private_data == NULL) {
		print_error("No device context.\n");
		return -ENODEV;
	}
	bu9873_data = private_data;
	reg_val = value;
	ret = bu9873_i2c_write(bu9873_data->client, address, &reg_val,1);
	if (ret < 0) {
		print_error("bu9873_i2c_write error ret = %d \n",ret);
		result = ret;
	}
	return result;
}

static int bu9873_regmon_read_reg(
	void *private_data,
	unsigned int address,
	unsigned int *value
	)
{
	struct bu9873_data *bu9873_data;
	uint8_t reg_val;
	int ret;
	int result;

	result = 0;
	if (private_data == NULL) {
		print_error("No device context.\n");
		return -ENODEV;
	}
	bu9873_data = private_data;
	reg_val = 0xff;
	ret = bu9873_i2c_read(bu9873_data->client, address, &reg_val,1);
	if (ret < 0) {
		print_error("bu9873_i2c_read error ret = %d \n",ret);
		result = ret;
	}
	*value = reg_val;
	return result;
}

static const struct i2c_device_id bu9873_id[] = {
	{BU9873_NAME, 0 },
	{ }
};

static struct i2c_driver bu9873_driver = {
	.driver = {
		.name	= BU9873_NAME,
	},
	.probe		= bu9873_probe,
	.remove		= bu9873_remove,
	.id_table	= bu9873_id,
};

static int  bu9873_init(void)
{
	return i2c_add_driver(&bu9873_driver);
}

static void __exit bu9873_exit(void)
{
	i2c_del_driver(&bu9873_driver);
}

module_init(bu9873_init);
module_exit(bu9873_exit);

MODULE_AUTHOR("Sony");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("bu9873 RTC driver");
MODULE_ALIAS("platform:rtc-bu9873");
