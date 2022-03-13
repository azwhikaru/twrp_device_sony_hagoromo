/*
 * Copyright 2016,2017 Sony Corporation
 * File changed on 2017-01-25
 */
/* ICX lm3630 driver.
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

#ifdef BUILD_LK
    #include <string.h>
    #include <platform/mt_pmic.h>
    #include <platform/mt_gpio.h>
    #include <platform/upmu_common.h>
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
    #include <platform/upmu_common.h>
#else
    #include <mach/mt_gpio.h>
    #include <mach/upmu_common.h>
    #include <mach/mt_pm_ldo.h>
    #include <linux/platform_data/icx_lm3630_bl.h>
    #include <linux/module.h>
    #include <linux/delay.h>
    #include <linux/earlysuspend.h>
    #include <linux/i2c.h>
    #include <linux/input.h>
    #include <linux/io.h>
    #include <linux/platform_device.h>
    #include <linux/slab.h>
    #include <linux/pwm.h>
    #include <linux/backlight.h>
    #include <linux/earlysuspend.h>
#endif

#if (defined(CONFIG_ARCH_MT8590_ICX))
#if (defined(CONFIG_REGMON_DEBUG))
#include <mach/regmon.h>
#define REGMON_DEBUG	/* define shorter macro. */
#endif /* (defined(CONFIG_REGMON_DEBUG)) */
#endif /* (defined(CONFIG_ARCH_MT8590_ICX)) */

#if (defined(REGMON_DEBUG))
/* ICX platform feature. */
static regmon_reg_info_t icx_lm3630_regmon_reg_info[] = {
	{"CONTROL",	LM3630_REG_CONTROL},
	{"CONFIGURATION",	LM3630_REG_CONFIGURATION},
	{"BOOST_CONTROOL",	LM3630_REG_BOOST_CONTROL},
	{"BRIGHTNESS_A",	LM3630_REG_BRIGHTNESS_A},
	{"BRIGHTNESS_B",	LM3630_REG_BRIGHTNESS_B},
	{"CURRENT_A",	LM3630_REG_CURRENT_A},
	{"ON_OFF_RAMP",	LM3630_REG_ON_OFF_RAMP},
	{"RUN_RAMP",	LM3630_REG_RUN_RAMP},
	{"INTERRUPT_STATUS",	LM3630_REG_INTERRUPT_STATUS},
	{"INTERRUPT_ENABLE",	LM3630_REG_INTERRUPT_ENABLE},
	{"FAULT_STATUS",	LM3630_REG_FAULT_STATUS},
	{"SOFTWARE_RESET",	LM3630_REG_SOFTWARE_RESET},
	{"PWM_OUT_LOW",	LM3630_REG_PWM_OUT_LOW},
	{"PWM_OUT_HIGH",	LM3630_REG_PWM_OUT_HIGH},
	{"REVISION",	LM3630_REG_REVISION},
	{"FILTER_STRENGTH",	LM3630_REG_FILTER_STRENGTH},
};

/* define accessor proto types. */

static int icx_lm3630_regmon_write_reg(
	 void	*private_data,
	unsigned int	address,
	unsigned int	value
);

static int icx_lm3630_regmon_read_reg(
	void	*private_data,
	unsigned int	address,
	unsigned int	*value
);

static regmon_customer_info_t icx_lm3630_customer_info =
{
	.name		= LM3630_NAME,
	.reg_info	= icx_lm3630_regmon_reg_info,
	.reg_info_count = sizeof(icx_lm3630_regmon_reg_info)/sizeof(icx_lm3630_regmon_reg_info[0]),
	.write_reg	= icx_lm3630_regmon_write_reg,
	.read_reg	= icx_lm3630_regmon_read_reg,
	.private_data	= NULL,
};
#else /* (defined(REGMON_DEBUG)) */
static struct lm3630_data *icx_lm3630_static_data = NULL;
#endif

extern ulong icx_pm_helper_sysinfo;
#define SYSINFO_LCD_MASK (0x00000007)
#define SYSINFO_LCD_OTM8018B (0x00)

#define MTK_I2C_READ_WRITE_LEN(read_len, write_len) \
	(((read_len) << 8) | ((write_len) << 0))

static void lm3630_early_suspend(struct early_suspend *h);
static void lm3630_late_resume(struct early_suspend *h);

struct lm3630_data {
	struct i2c_client *client;
	struct pwm_device *pwm;
	int blen_gpio;
	int en;
	struct early_suspend es_info;
	struct backlight_device *bl;
};

static int lm3630_i2c_write_byte(
    struct i2c_client *client,
    unsigned char    address,
    unsigned char value
)
{
    struct i2c_msg msg;
    unsigned char buf[2];
    int rv;

    if(client==NULL){
        pr_err("not initialized.\n");
        return(-ENODEV);
    }

    buf[0]=(unsigned char)address;
    buf[1]=(unsigned char)value;

    msg.addr  = client->addr;
    msg.flags = 0;
    msg.len   = 2;
    msg.buf   = buf;
    msg.ext_flag = 0;
    msg.timing = 300;
    rv=i2c_transfer(client->adapter,&msg,1);
    if(rv<0) {
        pr_err("i2c_transfer(): code %d error occurred.\n",rv);
        return(rv);
    }

    if(rv!=1){
        pr_err("count mismacth.\n");
        return(-EIO);
    }

    return(0);
}

static int lm3630_i2c_read_byte(
    struct i2c_client *client,
    unsigned char    address,
    unsigned char *value
)
{
    uint8_t reg_val;
    int rv;

    if(client==NULL){
        pr_err("not initialized.\n");
        return(-ENODEV);
    }

	client->addr &= I2C_MASK_FLAG;
	client->ext_flag = ((client->ext_flag & I2C_MASK_FLAG) | I2C_WR_FLAG | I2C_DIRECTION_FLAG);
	client->timing = 300;
	reg_val = address;
	rv = i2c_master_send(client, &reg_val, MTK_I2C_READ_WRITE_LEN(1, 1));
	if (rv < 0) {
		pr_err("Can not read register. rv=%d, address=0x%x, ext_flag=0x%x, address=0x%.2x\n", 
		rv, client->addr, client->ext_flag, address);
		goto out;
	}
out:
	client->ext_flag &= I2C_MASK_FLAG;
	*value = reg_val;
    return(0);
}

static int lm3630_update_status(struct lm3630_data *ld)
{
	if (!ld->en) {
		mt_set_gpio_out(ld->blen_gpio| 0x80000000, GPIO_OUT_ONE);
		msleep(20);
		lm3630_i2c_write_byte(ld->client, LM3630_REG_FILTER_STRENGTH, 0x03);
		lm3630_i2c_write_byte(ld->client, LM3630_REG_CONTROL, 0x17);
		lm3630_i2c_write_byte(ld->client, LM3630_REG_CONFIGURATION, 0x1B);
		lm3630_i2c_write_byte(ld->client, LM3630_REG_BOOST_CONTROL, 0x00);
		lm3630_i2c_write_byte(ld->client, LM3630_REG_BRIGHTNESS_A, 0x00);
		lm3630_i2c_write_byte(ld->client, LM3630_REG_CURRENT_A, 0x17);
		usleep_range(20000,22000);
		lm3630_i2c_write_byte(ld->client, LM3630_REG_BRIGHTNESS_A, 0xFF);
		ld->en = 1;
	}
	return 0;
}
void icx_lm3630_enable(void){
#if (defined(REGMON_DEBUG))
	struct lm3630_data *ld = icx_lm3630_customer_info.private_data;
#else
	struct lm3630_data *ld = icx_lm3630_static_data;
#endif
	if(ld != NULL){
		pr_info("%s:in \n",__func__);
		lm3630_update_status(ld);
	}
}

static int lm3630_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	struct lm3630_data *ld;
	struct lm3630_bl_platform_data *pdata;

	pdata = client->dev.platform_data;
	if (!pdata) {
		pr_err("lm3030: no platform_data\n");
		return -EINVAL;
	}

	ld = kzalloc(sizeof(*ld), GFP_KERNEL);
	if (ld == NULL)
		return -ENOMEM;

	ld->es_info.suspend = lm3630_early_suspend;
	ld->es_info.resume = lm3630_late_resume;
	ld->es_info.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	register_early_suspend(&ld->es_info);
	ld->client = client;
	i2c_set_clientdata(client, ld);

	ld->blen_gpio = pdata->blen_gpio;

	mt_set_gpio_mode(ld->blen_gpio | 0x80000000,GPIO_MODE_GPIO);
	mt_set_gpio_pull_enable(ld->blen_gpio | 0x80000000 ,GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(ld->blen_gpio  | 0x80000000,GPIO_PULL_DOWN);
	mt_set_gpio_dir(ld->blen_gpio  | 0x80000000,GPIO_DIR_OUT);

	mt_set_gpio_mode(203 | 0x80000000,GPIO_MODE_02);
	mt_set_gpio_dir(203  | 0x80000000,GPIO_DIR_IN);

	if(mt_get_gpio_in(ld->blen_gpio  | 0x80000000) == GPIO_OUT_ZERO){ //in LK, not init lm3630.
		mt_set_gpio_out(ld->blen_gpio | 0x80000000,GPIO_OUT_ZERO);
	}
	else {
		ld->en = 1;	//in LK, already init lm3630.
		lm3630_i2c_write_byte(ld->client, LM3630_REG_FILTER_STRENGTH, 0x03);
	}
	lm3630_update_status(ld);

#if (defined(REGMON_DEBUG))
	icx_lm3630_customer_info.private_data = ld;
	regmon_add(&icx_lm3630_customer_info);
#else /* (defined(REGMON_DEBUG)) */
	icx_lm3630_static_data = ld;
#endif

	return 0;
}

static int lm3630_remove(struct i2c_client *client)
{
	struct lm3630_data *ld = i2c_get_clientdata(client);
	unregister_early_suspend(&ld->es_info);
#if (defined(REGMON_DEBUG))
	icx_lm3630_customer_info.private_data = NULL;
	regmon_del(&icx_lm3630_customer_info);
#else /* (defined(REGMON_DEBUG)) */
	icx_lm3630_static_data = NULL;
#endif
	kfree(ld);
	return 0;
}

static void lm3630_early_suspend(struct early_suspend *h)
{
	struct lm3630_data *ld = container_of(h, struct lm3630_data, es_info);
	mt_set_gpio_out(ld->blen_gpio| 0x80000000, GPIO_OUT_ZERO);
	ld->en = 0;
	pr_info("%s:in \n",__func__);
	return;
}

static void lm3630_late_resume(struct early_suspend *h)
{
	struct lm3630_data *ld = container_of(h, struct lm3630_data, es_info);
	pr_info("%s:in \n",__func__);
	return;
}

/* define accessor proto types. */
static int icx_lm3630_regmon_write_reg(
	void	*private_data,
	unsigned int address,
	unsigned int value
	)
{
	struct lm3630_data *icx_lm3630_data;
	uint8_t reg_val;
	int ret;
	int result;

	result = 0;
	if (private_data == NULL) {
		pr_err("No device context.\n");
		return -ENODEV;
	}
	icx_lm3630_data = private_data;
	reg_val = value;
	ret = lm3630_i2c_write_byte(icx_lm3630_data->client, address, &reg_val);
	if (ret < 0) {
		pr_err("icx_lm3630_i2c_write error ret = %d \n",ret);
		result = ret;
	}
	return result;
}

static int icx_lm3630_regmon_read_reg(
	void *private_data,
	unsigned int address,
	unsigned int *value
	)
{
	struct lm3630_data *icx_lm3630_data;
	uint8_t reg_val;
	int ret;
	int result;

	result = 0;
	if (private_data == NULL) {
		pr_err("No device context.\n");
		return -ENODEV;
	}
	icx_lm3630_data = private_data;
	reg_val = 0xff;
	ret = lm3630_i2c_read_byte(icx_lm3630_data->client, address, &reg_val);
	if (ret < 0) {
		pr_err("icx_lm3630_i2c_read error ret = %d \n",ret);
		result = ret;
	}
	*value = reg_val;
	return result;
}


static const struct i2c_device_id lm3630_id[] = {
	{LM3630_NAME, 0 },
	{ }
};

static struct i2c_driver lm3630_driver = {
	.probe		= lm3630_probe,
	.remove		= lm3630_remove,
	.id_table	= lm3630_id,
	.driver = {
		.name	= LM3630_NAME,
	},
};

static int  lm3630_init(void)
{
	int rv=0;
	if((icx_pm_helper_sysinfo & SYSINFO_LCD_MASK) == SYSINFO_LCD_OTM8018B){
		rv = i2c_add_driver(&lm3630_driver);
	}
	return rv;
}

static void __exit lm3630_exit(void)
{
	if((icx_pm_helper_sysinfo & SYSINFO_LCD_MASK) == SYSINFO_LCD_OTM8018B){
		i2c_del_driver(&lm3630_driver);
	}
	return;
}

module_init(lm3630_init);
module_exit(lm3630_exit);

MODULE_DESCRIPTION("LM3630 backlight driver");
MODULE_LICENSE("GPL");
