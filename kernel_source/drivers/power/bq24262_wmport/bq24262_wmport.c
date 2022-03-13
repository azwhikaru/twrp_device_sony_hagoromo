/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* bq24262_wmport.c: BQ24262 battery charger with WM-PORT driver.
 *
 * Copyright 2015 Sony Corporation.
 * Author: Sony Corporation.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <mach/icx_pm_helper.h>
#include "bq24262_wmport_priv.h"

#if (defined(CONFIG_ARCH_MT8590_ICX))
#if (defined(CONFIG_REGMON_DEBUG))
#include <mach/regmon.h>
#define	REGMON_DEBUG	/* define shorter macro. */
#endif /* (defined(CONFIG_REGMON_DEBUG)) */
#endif /* (defined(CONFIG_ARCH_MT8590_ICX)) */

/* Limit VBUS current 100mA at shutdown. For Smart phone USB-DAC mode.  */
/* #define BQ24262_WMPORT_VBUS_100MA_AT_SHUTDOWN */

/* MTK API replacement */
int Enable_BATDRV_LOG = BAT_LOG_CRTI;

/* wait_for_xxx timeout, strictly use in this file. */
static const unsigned long	TIMEOUT_INFINITY=~0UL;

/*! Live debug control. */
uint bq24262_wmport_debug =
	   0
	   | DEBUG_FLAG_LIGHT
	/* | DEBUG_FLAG_IRQ */
	/* | DEBUG_FLAG_ENTRY */ 
	/* | DEBUG_FLAG_HEAVY */
	/* | DEBUG_FLAG_XX */
	/* | DEBUG_FLAG_THR_MAIN */
	/* | DEBUG_FLAG_CONFIG */
	/* | DEBUG_FLAG_IGNORE_BATTERY_TEMP */
	/* | DEBUG_FLAG_THR_VBUS */
	   | DEBUG_FLAG_WAKE_LOCK
	;

/* battery threshold for FS */
#define BTT_1_LOW		3350000
#define BTT_1_BLINK		3578000
#define BTT_1_GAUGE1	3707000
#define BTT_1_GAUGE2	3758000
#define BTT_1_GAUGE3	3830000
#define BTT_1_FULL		4100000

/* battery threshold for HM */
#define BTT_2_LOW		3350000
#define BTT_2_BLINK		3559000
#define BTT_2_GAUGE1	3714000
#define BTT_2_GAUGE2	3765000
#define BTT_2_GAUGE3	3828000
#define BTT_2_FULL		4100000

/* battery table structure */
typedef struct {
	long percent;
	long voltage; /* uV */
} battery_info_t;

/* battery table */
static battery_info_t battery_table[]={
	/* These are default value. These are rewritten when probe */
	{  0, 3350000},
	{  7, 3680000},
	{ 13, 3716000},
	{ 25, 3771000},
	{ 50, 3844000},
	{100, 4100000},
};

//#define BQ24262_WMPORT_CHG_OFFSET_UV 100000 /* 4.2V(charge V)-4.1V(100%) */
#define BQ24262_WMPORT_CHG_OFFSET_UV 0 /* no adjust at chargeing */
#define BQ24262_WMPORT_BATTERY_INIT_UV 5000000 /* 5.0V */

static const int battery_table_count=sizeof(battery_table)/sizeof(battery_info_t);

static int bq24262_wmport_debug_set(const char *val, const struct kernel_param *kp);
static int bq24262_wmport_debug_get(char *buffer, const struct kernel_param *kp);

/*! Lieve debug control functions. */
static const struct kernel_param_ops bq24262_wmport_debug_ops = {
	.set = bq24262_wmport_debug_set,
	.get = bq24262_wmport_debug_get,
};

module_param_cb(debug, &bq24262_wmport_debug_ops, &bq24262_wmport_debug, S_IWUSR | S_IRUGO);

uint	bq24262_wmport_i2c_write_ms = 0;
module_param_named(i2c_write_ms, bq24262_wmport_i2c_write_ms, uint, S_IWUSR | S_IRUGO);

uint	bq24262_wmport_i2c_clk_khz = 0;
module_param_named(i2c_clk_khz, bq24262_wmport_i2c_clk_khz, uint, S_IWUSR | S_IRUGO);

uint	bq24262_wmport_vbus_debounce_poll_ms = 8;
module_param_named(vbus_debounce_poll_ms, bq24262_wmport_vbus_debounce_poll_ms, uint, S_IWUSR | S_IRUGO);

uint	bq24262_wmport_vbus_debounce_poll_count = 5;
module_param_named(vbus_debounce_poll_count, bq24262_wmport_vbus_debounce_poll_count, uint, S_IWUSR | S_IRUGO);

uint	bq24262_wmport_dpdm_contact_poll_ms = 8;
module_param_named(dpdm_contact_poll_ms, bq24262_wmport_dpdm_contact_poll_ms, uint, S_IWUSR | S_IRUGO);

uint	bq24262_wmport_dpdm_contact_poll_ready = 5;
module_param_named(dpdm_contact_poll_ready, bq24262_wmport_dpdm_contact_poll_ready, uint, S_IWUSR | S_IRUGO);

uint	bq24262_wmport_vbus_max_power_init_ma = BQ24262_WMPORT_VBUS_CURRENT_CONFIG_MA;
module_param_named(vbus_max_power_init_ma, bq24262_wmport_vbus_max_power_init_ma, uint, S_IWUSR | S_IRUGO);

uint	bq24262_wmport_vbus_usb_pll_wait_ms = 150;
module_param_named(usb_pll_wait_ms, bq24262_wmport_vbus_usb_pll_wait_ms, uint, S_IWUSR | S_IRUGO);

#if (defined(REGMON_DEBUG))
/* ICX platform feature. */
static regmon_reg_info_t bq24262_wmport_regmon_reg_info[] = {
	{"STATUS",		BQ24262_STATUS_CONTROL},
	{"CONTROL",		BQ24262_CONTROL},
	{"BATTERY_VOLTAGE",	BQ24262_BATTERY_VOLTAGE},
	{"VENDOR",		BQ24262_VENDOR},
	{"BATTERY_CURRENT",	BQ24262_BATTERY_CURRENT},
	{"VIN_MINSYS",		BQ24262_VIN_MINSYS},
	{"SAFETY",		BQ24262_SAFETY},
};

/* define accessor proto types. */

static int bq24262_wmport_regmon_write_reg(
	void		*private_data,
	unsigned int	address,
	unsigned int	value
);

static int bq24262_wmport_regmon_read_reg(
	void		*private_data,
	unsigned int	address,
	unsigned int	*value
);

static regmon_customer_info_t bq24262_wmport_customer_info =
{
	.name           = "bq24262",
	.reg_info       = bq24262_wmport_regmon_reg_info,
	.reg_info_count = sizeof(bq24262_wmport_regmon_reg_info)/sizeof(bq24262_wmport_regmon_reg_info[0]),
	.write_reg      = bq24262_wmport_regmon_write_reg,
	.read_reg       = bq24262_wmport_regmon_read_reg,
	.private_data   = NULL,
};

#endif /* (defined(REGMON_DEBUG)) */

const char *bq24262_wmport_bcdet_names[]={
	[ICX_CHARGER_UNKNOWN] =		"Unkonwn",
	[ICX_CHARGER_STD] =		"STD",
	[ICX_CHARGER_CDP] =		"CDP",
	[ICX_CHARGER_DCP] =		"DCP",
	[ICX_CHARGER_APL_0R5] =		"APL0R5",
	[ICX_CHARGER_APL_1R0] =		"APL1R0",
	[ICX_CHARGER_APL_2R1] =		"APL2R1",
	[ICX_CHARGER_AC_S508U] =	"AC-S508U",
	[ICX_CHARGER_MISC_OPEN] =	"OPEN",
	[ICX_CHARGER_MISC_XXX] =	"XXX",
	[ICX_CHARGER_NUMS] =		"nums",
};

const char *bq24262_wmport_config_names[BQ24262_WMPORT_CONFIG_ALL]={
	[BQ24262_WMPORT_CONFIG_BOOT]="BOOT",
	[BQ24262_WMPORT_CONFIG_HIZ]="HIZ",
	[BQ24262_WMPORT_CONFIG_USB_ROLE_A]="ROLE_A", /* Do not change. */
	[BQ24262_WMPORT_CONFIG_USB_UNKNOWN]="UNKNOWM",
	[BQ24262_WMPORT_CONFIG_USB_UNCONFIG]="UNCONFIG",
	[BQ24262_WMPORT_CONFIG_USB_CONFIG]="CONFIG",
	[BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_500]="ACHAGER",
	[BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_900]="DCP",
	[BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_500]="CDP500",
	[BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_900]="CDP900",
	[BQ24262_WMPORT_CONFIG_DCIN]="DCIN",
	[BQ24262_WMPORT_CONFIG_NO_POWER]="NOP",
	[BQ24262_WMPORT_CONFIG_COOL_DOWN]="DOWN",
	[BQ24262_WMPORT_CONFIG_USB_RECOVERY]="RECOVERY",
	[BQ24262_WMPORT_CONFIG_USB_CONFIG_LOW]="LOWCONF",
	[BQ24262_WMPORT_CONFIG_USB_SUSPENDED]="SUSPENDED",
	[BQ24262_WMPORT_CONFIG_USB_SUS_DRAW]="SUS_DRAW",
};

/*! USB power source 500mA
*/
const struct bq24262_wmport_configs bq24262_wmport_configs_usb_in_500 = {
	.name =			 "USB500",
	.charger_ps_status =	 POWER_SUPPLY_STATUS_CHARGING,
	.notify_health =	true,
	.flags =		   BQ24262_WMPORT_CONFIGS_W_MAIN_SETS | BQ24262_WMPORT_CONFIGS_SYS_WAK_STAT
				 | BQ24262_WMPORT_CONFIGS_DRAWING | BQ24262_WMPORT_CONFIGS_CHARGING,
	.control =		 BQ24262_CONTROL_IN_LIMIT_500MA | BQ24262_CONTROL_EN_STAT | BQ24262_CONTROL_TE,
	.battery_voltage =	 BQ24262_BATTERY_VOLTAGE_MOD_FREQ_NO_CHANGE,
	.battery_current =	 BQ24262_BATTERY_CURRENT_ICHRG_IBATT_MA(700) | BQ24262_BATTERY_CURRENT_ITERM_IBATT_MA(50),
	.vin_minsys =		 BQ24262_VIN_MINSYS_VINDPM_PLUS(8) /* (4200mV*1.08)=4536mV (ANTICOLLAPSE) */,
};

/*! USB power source 900mA
*/
const struct bq24262_wmport_configs bq24262_wmport_configs_usb_in_900 = {
	.name =			 "USB900",
	.charger_ps_status =	 POWER_SUPPLY_STATUS_CHARGING,
	.notify_health =	true,
	.flags =		   BQ24262_WMPORT_CONFIGS_W_MAIN_SETS | BQ24262_WMPORT_CONFIGS_SYS_WAK_STAT
				 | BQ24262_WMPORT_CONFIGS_DRAWING | BQ24262_WMPORT_CONFIGS_CHARGING,
	.control =		 BQ24262_CONTROL_IN_LIMIT_900MA | BQ24262_CONTROL_EN_STAT | BQ24262_CONTROL_TE,
	.battery_voltage =	 BQ24262_BATTERY_VOLTAGE_MOD_FREQ_NO_CHANGE,
	.battery_current =	 BQ24262_BATTERY_CURRENT_ICHRG_IBATT_MA(700) | BQ24262_BATTERY_CURRENT_ITERM_IBATT_MA(50),
	.vin_minsys =		 BQ24262_VIN_MINSYS_VINDPM_PLUS(8) /* (4200mV*1.08)=4536mV (ANTICOLLAPSE) */,
};

/*! DCIN power source 900mA
*/
const struct bq24262_wmport_configs bq24262_wmport_configs_dc_in = {
	.name =			 "DCIN",
	.charger_ps_status =	 POWER_SUPPLY_STATUS_CHARGING,
	.notify_health =	true,
	.flags =		   BQ24262_WMPORT_CONFIGS_W_MAIN_SETS | BQ24262_WMPORT_CONFIGS_SYS_WAK_STAT | BQ24262_WMPORT_CONFIGS_FORCE_DCIN
				 | BQ24262_WMPORT_CONFIGS_DRAWING | BQ24262_WMPORT_CONFIGS_CHARGING,
	.control =		 BQ24262_CONTROL_IN_LIMIT_900MA | BQ24262_CONTROL_EN_STAT | BQ24262_CONTROL_TE,
	.battery_voltage =       BQ24262_BATTERY_VOLTAGE_MOD_FREQ_NO_CHANGE,
	.battery_current =	 BQ24262_BATTERY_CURRENT_ICHRG_IBATT_MA(700) | BQ24262_BATTERY_CURRENT_ITERM_IBATT_MA(50),
	.vin_minsys =		 BQ24262_VIN_MINSYS_VINDPM_PLUS(8) /* (4200mV*1.08)=4536mV (ANTICOLLAPSE) */,
};

/*! No power source or USB unconfigured.
*/
const struct bq24262_wmport_configs bq24262_wmport_configs_hiz = {
	.name =			 "HIZ",
	.charger_ps_status = 	 POWER_SUPPLY_STATUS_NOT_CHARGING,
	.notify_health =	false,
	.flags =		   BQ24262_WMPORT_CONFIGS_W_MAIN_SETS | BQ24262_WMPORT_CONFIGS_SYS_WAK_STAT | BQ24262_WMPORT_CONFIGS_SUSPEND
				 | 0x00 /* Not drawing, Not charging */,
	.control =		 BQ24262_CONTROL_IN_LIMIT_500MA | BQ24262_CONTROL_EN_STAT | BQ24262_CONTROL_TE,
	.battery_voltage =	 BQ24262_BATTERY_VOLTAGE_MOD_FREQ_NO_CHANGE,
	.battery_current =	 BQ24262_BATTERY_CURRENT_ICHRG_IBATT_MA(700) | BQ24262_BATTERY_CURRENT_ITERM_IBATT_MA(50),
	.vin_minsys =		 BQ24262_VIN_MINSYS_VINDPM_PLUS(8) /* (4200mV*1.08)=4536mV (ANTICOLLAPSE) */,
};

/*! Low VBUS current (Portable AMP / USB DAC device) mode.
    @note We set IN_LIMIT 100mA, but we don't draw current from VBUS when system is running.
          IN_LIMIT is refered at next "power up" sequence.
*/
const struct bq24262_wmport_configs bq24262_wmport_configs_low = {
	.name =			 "LOW",
	.charger_ps_status = 	 POWER_SUPPLY_STATUS_NOT_CHARGING,
	.notify_health =	false,
	.flags =		   BQ24262_WMPORT_CONFIGS_W_MAIN_SETS | BQ24262_WMPORT_CONFIGS_SYS_WAK_STAT | BQ24262_WMPORT_CONFIGS_SUSPEND
				 | 0x00 /* Not drawing, Not charging */,
	.control =		 BQ24262_CONTROL_IN_LIMIT_100MA | BQ24262_CONTROL_EN_STAT | BQ24262_CONTROL_TE,
	.battery_voltage =	 BQ24262_BATTERY_VOLTAGE_MOD_FREQ_NO_CHANGE,
	.battery_current =	 BQ24262_BATTERY_CURRENT_ICHRG_IBATT_MA(700) | BQ24262_BATTERY_CURRENT_ITERM_IBATT_MA(50),
	.vin_minsys =		 BQ24262_VIN_MINSYS_VINDPM_PLUS(8) /* (4200mV*1.08)=4536mV (ANTICOLLAPSE) */,
};

/*! Cool down.
*/
const struct bq24262_wmport_configs bq24262_wmport_configs_down = {
	.name =			 "DOWN",
	.charger_ps_status =	 POWER_SUPPLY_STATUS_NOT_CHARGING,
	.notify_health =	true,
	.flags =		   BQ24262_WMPORT_CONFIGS_W_MAIN_SETS | BQ24262_WMPORT_CONFIGS_SYS_WAK_STAT
				 | BQ24262_WMPORT_CONFIGS_DRAWING,
	.control =		 BQ24262_CONTROL_IN_LIMIT_500MA | BQ24262_CONTROL_EN_STAT | BQ24262_CONTROL_TE | BQ24262_CONTROL_CE_N,
	.battery_voltage =	 BQ24262_BATTERY_VOLTAGE_MOD_FREQ_NO_CHANGE,
	.battery_current =	 BQ24262_BATTERY_CURRENT_ICHRG_IBATT_MA(700) | BQ24262_BATTERY_CURRENT_ITERM_IBATT_MA(50),
	.vin_minsys =		 BQ24262_VIN_MINSYS_VINDPM_PLUS(8) /* (4200mV*1.08)=4536mV (ANTICOLLAPSE) */,
};

/*! configuration table.
    @note ICX platform doesn't support USB otg ROLE A.
*/
const struct bq24262_wmport_configs
	*bq24262_wmport_configs_table[BQ24262_WMPORT_CONFIG_ALL]={
	[BQ24262_WMPORT_CONFIG_BOOT] = 			NULL /* Do not change. */,
	[BQ24262_WMPORT_CONFIG_HIZ] = 			&bq24262_wmport_configs_hiz,
	[BQ24262_WMPORT_CONFIG_USB_ROLE_A] = 		&bq24262_wmport_configs_hiz,
	[BQ24262_WMPORT_CONFIG_USB_UNKNOWN] = 	&bq24262_wmport_configs_hiz,
	[BQ24262_WMPORT_CONFIG_USB_UNCONFIG] = 		&bq24262_wmport_configs_hiz,
	[BQ24262_WMPORT_CONFIG_USB_CONFIG] = 		&bq24262_wmport_configs_usb_in_500,
	[BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_500] = 	&bq24262_wmport_configs_usb_in_500,
	[BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_900] = 	&bq24262_wmport_configs_usb_in_900,
	[BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_500] =	&bq24262_wmport_configs_usb_in_500,
	[BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_900] =	&bq24262_wmport_configs_usb_in_900,
	[BQ24262_WMPORT_CONFIG_DCIN] = 			&bq24262_wmport_configs_dc_in,
	[BQ24262_WMPORT_CONFIG_NO_POWER] = 		&bq24262_wmport_configs_hiz,
	[BQ24262_WMPORT_CONFIG_COOL_DOWN] = 		&bq24262_wmport_configs_down,
	[BQ24262_WMPORT_CONFIG_USB_RECOVERY] = 		&bq24262_wmport_configs_usb_in_500,
	[BQ24262_WMPORT_CONFIG_USB_CONFIG_LOW] = 	&bq24262_wmport_configs_low,
	[BQ24262_WMPORT_CONFIG_USB_SUSPENDED] = 	&bq24262_wmport_configs_hiz,
	[BQ24262_WMPORT_CONFIG_USB_SUS_DRAW] = 		&bq24262_wmport_configs_down,
};

/* @todo will be renamed, remove prefix "bq24262_". */
static char bq24262_wmport_battery_name[]="battery";
static char bq24262_wmport_usb_name[]="usb";
static char bq24262_wmport_dc_name[]="dc";

static char bq24262_wmport_wake_lock_resume_name[]="bq24262_resume";
static char bq24262_wmport_wake_lock_charging_name[]="bq24262_charging";

/*! Statically keeped device information.
*/
struct bq24262_wmport_static_context	bq24262_wmport_static = {
	.bc =			NULL,	/*!< driver context used in interruptible context. */
	.arg =			NULL,
	.vbus_changed =		NULL,
	.bc_locked =		NULL,	/*!< driver context used in interrupt context. */
};
EXPORT_SYMBOL(bq24262_wmport_static);

#define	MTK_I2C_READ_WRITE_LEN(read_len, write_len) \
	(((read_len) << 8) | ((write_len) << 0))

/*! write BQ24262 I2C register
    @param bc driver context.
    @param reg_addr BQ24262 register address.
    @param val point to uint8_t to store register value.
    @return int < 0: Fail, Negative errno numner, \
                >=0: Success, MTK_I2C_READ_WRITE_LEN(read_bytes, write_bytes).
*/
int bq24262_wmport_i2c_read(struct bq24262_wmport_context *bc, uint8_t reg_addr, uint8_t *val)
{	struct i2c_client	*client;
	uint8_t			addr_read_buf[1];
	uint8_t			dummy;
	int			ret;
	unsigned		i2c_clk;

	PRINTK_FUNC_ENTRY("%s: Called. reg_addr=0x%.2x\n", __func__, reg_addr);
	if (val == NULL) {
		val = &dummy;
	}
	*val = 0xff;
	down(&(bc->charger_i2c_sem));
	client = bc->charger_client;
	client->addr &= I2C_MASK_FLAG;
	i2c_clk = bq24262_wmport_i2c_clk_khz;
	if ((i2c_clk < BQ24262_I2C_CLK_MIN_KHZ) || (i2c_clk > BQ24262_I2C_CLK_MAX_KHZ)) {
		client->timing = bc->platform_data->chg_i2c_timing;;
	} else {
		PRINTK_HV("%s: Override timing. i2c_clk=%u\n", __func__, i2c_clk);
		client->timing = i2c_clk;
	}
	/* @note clear I2C_HS_FLAG by masking with I2C_MASK_FLAG */
	client->ext_flag = ((client->ext_flag & I2C_MASK_FLAG) | I2C_WR_FLAG | I2C_DIRECTION_FLAG);
	addr_read_buf[0] = reg_addr;
	ret = i2c_master_send(client, addr_read_buf, MTK_I2C_READ_WRITE_LEN(1, 1));
	if (ret < 0) {
		pr_err("%s: ERROR: Can not read register. ret=%d, addr=0x%x, ext_flag=0x%x, reg_addr=0x%.2x\n", __func__,
			ret, client->addr, client->ext_flag, reg_addr
		);
		goto out;
	}
	*val = addr_read_buf[0];
out:
	client->ext_flag &= I2C_MASK_FLAG;
	up(&(bc->charger_i2c_sem));
	PRINTK_FUNC_ENTRY("%s: Read register. reg_addr=0x%.2x, *val=0x%.2x\n", __func__, reg_addr, *val);
	return ret;
}

/*! write BQ24262 I2C register
    @param bc driver context.
    @param reg_addr BQ24262 register address.
    @param val point to uint8_t to store register value.
    @return int < 0: Fail, Negative errno numner, \
                >=0: Success, MTK_I2C_READ_WRITE_LEN(read_bytes, write_bytes).
*/
int bq24262_wmport_i2c_write(struct bq24262_wmport_context *bc, uint8_t reg_addr, uint8_t val)
{	struct i2c_client	*client;
	uint8_t			addr_val_write_buf[2];
	int			ret;
	unsigned		i2c_clk;

	PRINTK_FUNC_ENTRY("%s: Called. reg_addr=0x%.2x, val=0x%.2x\n", __func__, reg_addr, val);
	down(&(bc->charger_i2c_sem));
	client = bc->charger_client;
	client->addr &= I2C_MASK_FLAG;
	i2c_clk = bq24262_wmport_i2c_clk_khz;
	if ((i2c_clk < BQ24262_I2C_CLK_MIN_KHZ) || (i2c_clk > BQ24262_I2C_CLK_MAX_KHZ)) {
		client->timing = bc->platform_data->chg_i2c_timing;;
	} else {
		PRINTK_HV("%s: Override timing. i2c_clk=%u\n", __func__, i2c_clk);
		client->timing = i2c_clk;
	}
	/* @note clear I2C_HS_FLAG by masking with I2C_MASK_FLAG */
	client->ext_flag = (((client->ext_flag) & I2C_MASK_FLAG) | I2C_DIRECTION_FLAG);
	addr_val_write_buf[0] = reg_addr;
	addr_val_write_buf[1] = val;
	ret = i2c_master_send(client, addr_val_write_buf, MTK_I2C_READ_WRITE_LEN(0, 2));
	if (ret < 0) {
		pr_err("%s: ERROR: Can not write register. ret=%d, addr=0x%x, ext_flag=0x%x, reg_addr=0x%.2x, val=0x%.2x\n",
			__func__, ret, client->addr, client->ext_flag, reg_addr, val
		);
	}
	if (bq24262_wmport_i2c_write_ms > 0) {
		msleep(bq24262_wmport_i2c_write_ms);
	}
	client->ext_flag &= I2C_MASK_FLAG;
	up(&(bc->charger_i2c_sem));
	return ret;
}

/*! regmon write accessor types.
*/
static int bq24262_wmport_regmon_write_reg(
	void		*private_data,
	unsigned int	address,
	unsigned int	value
	)
{	struct bq24262_wmport_context	*bc;
	uint8_t		reg_val;
	int		ret;
	int		result;

	result = 0;
	if (private_data == NULL) {
		pr_err("%s: ERROR: No device context.\n", __func__);
		return -ENODEV;
	}
	bc = private_data;
	reg_val = value;
	PRINTK_HV("%s: Write BQ24262. addr=0x%x, reg_val=0x%.2x\n", __func__, address, reg_val);
	ret = bq24262_wmport_i2c_write(bc, address, reg_val);
	if (ret < 0) {
		result = ret;
	}
	return result;
}

/*! regmon read accessor types.
*/
static int bq24262_wmport_regmon_read_reg(
	void		*private_data,
	unsigned int	address,
	unsigned int	*value
	)
{	struct bq24262_wmport_context	*bc;
	uint8_t				reg_val;
	int				ret;
	int				result;

	result = 0;
	if (private_data == NULL) {
		pr_err("%s: ERROR: No device context.\n", __func__);
		return -ENODEV;
	}
	bc = private_data;
	reg_val = 0xff;
	ret = bq24262_wmport_i2c_read(bc, address, &reg_val);
	if (ret < 0) {
		result = ret;
	}
	PRINTK_HV("%s: Read BQ24262. addr=0x%x, reg_val=0x%.2x\n", __func__, address, reg_val);
	*value = reg_val;
	return result;
}

/*! DEBUG: set debug control value.
*/
STATIC_FUNC int bq24262_wmport_debug_set(const char *val, const struct kernel_param *kp)
{	int				ret;
	long				v;
	long				prev_debug;

	if ((kp == NULL) || (val == NULL)) {
		return -ENOSYS;
	}

	v = 0;
	ret = strict_strtol(val, 0 /* any radix */, &v);
	if (ret != 0) {
		/* Can't convert. */
		return ret;
	}

	prev_debug = *((uint*)(kp->arg));
	*((uint*)(kp->arg))=(uint)v;
	/* @todo we will implement dynamic behaviour. */
	if (prev_debug != v) {
		PRINTK_LI("%s: Update debug switch. v=0x%.8lx\n", __func__, v);
	}
	return ret;
}

/*! DEBUG: get debug control value.
*/
STATIC_FUNC int bq24262_wmport_debug_get(char *buffer, const struct kernel_param *kp)
{	return param_get_uint(buffer, kp);
}

/*! check if recovery (firmware update) booting.
    @return bool ==true: Recovery boot, ==false: Normal boot.
*/
static bool bq24262_wmport_is_recovery_boot(void)
{	return (icx_pm_helper_boot_option == ICX_PM_HELPER_BOOT_OPTION_RECOVERY);
}

long bq24262_wmport_vbat_scale_to_uv(struct bq24262_wmport_context *bc, uint64_t raw_bin)
{	struct bq24262_wmport_platform_data	*pd;
	long					result;
	uint64_t				t;

	pd = bc->platform_data;
	t =         raw_bin * pd->battery_scale_to_uv_mul;
	do_div(t,  (pd->battery_scale_to_uv_div * pd->pmic_vbat_samples));
	t += pd->battery_scale_to_uv_offset;
	result = /* force_cast */(long)t;
	if (result < 0) {
		pr_err("%s: Unexpected negative voltage. raw_bin=0x%llx, result=%ld, mul=%ld, div=%ld, samples=%d, offset=%ld\n",
			__func__, (unsigned long long)raw_bin, result,
			pd->battery_scale_to_uv_mul, pd->battery_scale_to_uv_div,
			pd->pmic_vbat_samples, pd->battery_scale_to_uv_offset
		);
	}
	return result;
}

/*! Return VBAT voltage in uV.
    Or < 0 on failure.
*/
long bq24262_wmport_vbat_read(struct bq24262_wmport_context *bc)
{	struct bq24262_wmport_platform_data	*pd;
	uint32_t	raw;
	long		result;
	raw = 0;
	pd = bc->platform_data;
	if (test_bit(BQ24262_WMPORT_SUSPEND_IN_PROGRESS, &(bc->flags)) == 0) {
		/* "on" state. */
		/* ignore return value mV */ PMIC_IMM_GetOneChannelValueRaw(pd->pmic_vbat_channel, pd->pmic_vbat_samples, 1, &raw);
	} else {
		/* enter or exit "mem" state. */
		/* ignore return value mV */ PMIC_IMM_GetOneChannelValue_wo_wake_lock(pd->pmic_vbat_channel, pd->pmic_vbat_samples, 1, &raw);
	}
	result = bq24262_wmport_vbat_scale_to_uv(bc, raw);
	return result;
}

/*! Return NTC(thermal sensor) voltage in register raw value.
    Or 0 on failure.
*/
uint32_t bq24262_wmport_temp_read(struct bq24262_wmport_context *bc)
{	struct bq24262_wmport_platform_data	*pd;
	uint32_t				raw;

	raw = 0;
	pd = bc->platform_data;
	if (test_bit(BQ24262_WMPORT_SUSPEND_IN_PROGRESS, &(bc->flags)) == 0) {
		/* "on" state. */
		/* ignore voltate in mV */ PMIC_IMM_GetOneChannelValueRaw(pd->pmic_thr_sense_channel, pd->pmic_thr_sense_samples, 1, &raw);
	} else {
		/* enter or exit "mem" state. */
		/* ignore voltate in mV */ PMIC_IMM_GetOneChannelValue_wo_wake_lock(pd->pmic_thr_sense_channel, pd->pmic_thr_sense_samples, 1, &raw);
	}
	return raw;
}


/*! Read VBUS_DET and DCIN_DET
    @param driver context.
    @return unsigned following bits masks are ORed,\
     <ul>\
     <li>BQ24262_WMPORT_USB:  Power is sourced from USB VBUS.\
     <li>BQ24262_WMPORT_DCIN: Power is sourced from DCIN(DC jack).\
     </ul>
*/
unsigned bq24262_wmport_ppath_read_raw(struct bq24262_wmport_context *bc)
{	int		vbus;	/* 0: Powered, 1: Opened. */
	int		dcin;	/* 0: Powered, 1: Opened. */
	unsigned	result;
	struct bq24262_wmport_platform_data	*pd;

	result = 0;
	pd = bc->platform_data;
	vbus = mt_get_gpio_in(pd->vbus_xdet_gpio);
	dcin = mt_get_gpio_in(pd->dc_xdet_gpio);

	if (vbus == 0) {
		/* Power is sourced from VBUS. */
		result |= BQ24262_WMPORT_USB;
	}
	if (dcin == 0) {
		/* Power is sourced from DCIN. */
		result |= BQ24262_WMPORT_DCIN;
	}

	return result;
}

/*! Read DCIN, VBUS detection status.
    @return unsigned ORed _DCIN, _USB, to use control BQ24262 and notify to user.
            *raw ORed  _DCIN, _USB, to use notify USB PHY driver.

    @note unsigned return value is ANDed with BQ24262's NOT(FAULT==FAULT_UVLO) status,
          this value is useful to control BQ24262 and to show status on
          User Interface.
          *raw is straight value read from GPIO, this value is useful to
          detect USB connection, and 1st step to wake power source thread.
          Read value VBUS and DCIN from GPIO are not chage at a same time
          with read value from BQ24262.STATUS_CONTROL.FAULT.
*/
unsigned bq24262_wmport_ppath_read(struct bq24262_wmport_context *bc, unsigned *raw)
{	unsigned	result;
	unsigned	dummy;

	if (!raw) {
		raw = &dummy;
	}

	*raw = result = bq24262_wmport_ppath_read_raw(bc);
	if (     (bc->charger_status.status & BQ24262_STATUS_CONTROL_STAT)
	      == BQ24262_STATUS_CONTROL_STAT_FAULT
	   ) {	/* Fault state. */
		if (    (bc->charger_status.status & BQ24262_STATUS_CONTROL_FAULT)
		     == BQ24262_STATUS_CONTROL_FAULT_UVLO
		) {
			/* IN(VBUS or DCIN) shows under volt lockout. */
			/* Clear DCIN and USB, it may doesn't tell the truth. */
			result &= ~(BQ24262_WMPORT_USB | BQ24262_WMPORT_DCIN);
		}
	}

	return result;
}

/*! Control FORCE_DCIN
    @param bc driver context.
    @param force_dcin BQ24262_WMPORT_FORCE_DCIN_AUTO_AUTO: Doesn't force draw current from DCIN\
                      BQ24262_WMPORT_FORCE_DCIN_FORCE: Force draw current from DCIN.
*/
void bq24262_wmport_force_dcin_write(struct bq24262_wmport_context *bc, unsigned force_dcin)
{	mt_set_gpio_out(bc->platform_data->force_dcin_gpio, force_dcin);
}

/*! Read FORCE_DCIN
    @param bc driver context.
    @return unsigned force_dcin _AUTO: Doesn't force draw current from DCIN\
                                _FORCE: Force draw current from DCIN.
*/
unsigned bq24262_wmport_force_dcin_read(struct bq24262_wmport_context *bc)
{	unsigned	in;
	in = mt_get_gpio_in(bc->platform_data->force_dcin_gpio);
	return in;
}

/*! Read CHG_XSTAT (Charger STAT pin)
    @param bc driver context.
    @return unsigned  0: STAT is LOW, it mostly shows charging, or sometimes FAULT.
                      1: STAT is HIGH, it shows not charging.
*/
unsigned bq24262_wmport_chg_xstat_read(struct bq24262_wmport_context *bc)
{	return (unsigned)(mt_get_gpio_in(bc->platform_data->chg_xstat_gpio));
}

/*! Write CHG_SUSPEND(CMIC SLG4R4801V)
    @param bc driver context.
    @param force_dcin BQ24262_WMPORT_CHG_SUSPEND_DRAW: Draw current from DCIN or VBUS.
                      BQ24262_WMPORT_CHG_SUSPEND_HIZ: Not draw from current from DCIN or VBUS.
*/
void bq24262_wmport_chg_suspend_write(struct bq24262_wmport_context *bc, unsigned chg_suspend)
{	mt_set_gpio_out(bc->platform_data->chg_suspend_gpio, chg_suspend);
}

/*! Read CHG_SUSPEND (CMIC SLG4R4801V SUSPEND pin)
    @param bc driver context.
    @return unsigned  _DRAW: Draw current from DCIN or VBUS.
                      _HIZ: Not draw from current from DCIN or VBUS.
*/
unsigned bq24262_wmport_chg_suspend_read(struct bq24262_wmport_context *bc)
{	return (unsigned)(mt_get_gpio_in(bc->platform_data->chg_suspend_gpio));
}


/*! Write SYS_WAK_STAT (CMIC SLG4R4801V)
    @param bc driver context.
    @param wak_stat BQ24262_WMPORT_SYS_WAK_STAT_BOOTING: booting, before latch PMIC.
                    BQ24262_WMPORT_SYS_WAK_STAT_READY: ready, after latch PMIC.
*/
void bq24262_wmport_sys_wak_stat_write(struct bq24262_wmport_context *bc, unsigned wak_stat)
{	mt_set_gpio_out(bc->platform_data->sys_wak_stat_gpio, wak_stat);
}

/*! Read SYS_WAK_STAT (CMIC SLG4R4801V)
    @param bc driver context.
    @return unsigned  _BOOTING: Booting, before latch PMIC.
                      _READY: Ready, after latch PMIC.
*/
unsigned bq24262_wmport_sys_wak_stat_read(struct bq24262_wmport_context *bc)
{	return (unsigned)(mt_get_gpio_in(bc->platform_data->sys_wak_stat_gpio));
}


/*! DEBUG: print charger registers and GPIO conditions.
*/
STATIC_FUNC void bq24262_wmport_configs_show_unlocked(struct bq24262_wmport_context *bc)
{	uint8_t		reg0;
	uint8_t		reg1;
	uint8_t		reg2;

	unsigned	suspend;
	unsigned	force_dcin;
	int		ret;

	force_dcin = bq24262_wmport_force_dcin_read(bc);
	suspend =    bq24262_wmport_chg_suspend_read(bc);
	PRINTK_LI("%s: FORCE_DCIN=%u, SUSPEND=%u\n", __func__, force_dcin, suspend);

	reg0 = 0xff;
	ret = bq24262_wmport_i2c_read(bc, BQ24262_CONTROL, &reg0);
	if (ret < 0) {
		PRINTK_LI("%s: Error access to CONTROL. ret=%d\n", __func__, ret);
	}
	reg1 = 0xff;
	ret = bq24262_wmport_i2c_read(bc, BQ24262_BATTERY_VOLTAGE, &reg1);
	if (ret < 0) {
		PRINTK_LI("%s: Error access to BATTERY_VOLTAGE. ret=%d\n", __func__, ret);
	}
	reg2 = 0xff;
	ret = bq24262_wmport_i2c_read(bc, BQ24262_BATTERY_CURRENT, &reg2);
	if (ret < 0) {
		PRINTK_LI("%s: Error access to BATTERY_CURRENT. ret=%d\n", __func__, ret);
	}
	PRINTK_LI("%s: CONTROL=0x%.2x, BATTERY_VOLTAGE=0x%.2x, BATTERY_CURRENT=0x%.2x\n",
		__func__,
		reg0, reg1, reg2
	);
	reg0 = 0xff;
	ret = bq24262_wmport_i2c_read(bc, BQ24262_VIN_MINSYS, &reg0);
	if (ret < 0) {
		PRINTK_LI("%s: Error access to VIN_MINSYS. ret=%d\n", __func__, ret);
	}
	reg1 = 0xff;
	ret = bq24262_wmport_i2c_read(bc, BQ24262_SAFETY, &reg1);
	if (ret < 0) {
		PRINTK_LI("%s: Error access to SAFETY. ret=%d\n", __func__, ret);
	}
	PRINTK_LI("%s: VIN_MINSYS=0x%.2x, SAFETY=0x%.2x\n", __func__,reg0, reg1);
}

/*! DEBUG: print charger registers and GPIO conditions with lock.
*/
STATIC_FUNC void bq24262_wmport_configs_show(struct bq24262_wmport_context *bc)
{	down(&(bc->charger_sem));
	bq24262_wmport_configs_show_unlocked(bc);
	up(&(bc->charger_sem));
}

/*! Test if charge settings are initialized or different from config.
    @param conf points configurations to compare with.
    @return int ==1: initialized or different from *conf.
                ==0: not initialized or same.
*/
STATIC_FUNC bool bq24262_wmport_is_initialized_settings_unlocked(struct bq24262_wmport_context *bc)
{	uint8_t		reg0;
	uint8_t		ichrg;
	uint8_t		iterm;
	int		ret;
	bool		result;
	
	result = false; /* not initialized. */
	reg0 = 0x00; /* safe value if failed read. */
	ret = bq24262_wmport_i2c_read(bc, BQ24262_BATTERY_CURRENT, &reg0);
	if (ret < 0) {
		/* read failed. */
		return true /* initialized. */;
	}
	ichrg = reg0 & BQ24262_BATTERY_CURRENT_ICHRG;
	if (ichrg > BQ24262_BATTERY_CURRENT_ICHRG_200MA) {
		/* Charge current is more than 700mA (200mA + 500mA) */
		result = true;
	}

	iterm = reg0 & BQ24262_BATTERY_CURRENT_ITERM;
	if (iterm > BQ24262_BATTERY_CURRENT_ITERM_50MA) {
		/* Charge termination current is more than 100mA (50mA + 50mA) */
		result = true;
	}
	if (result != 0) {
		/* Register is initialized. */
		PRINTK_LI("%s: Detect initialized. reg0=0x%.2x\n", __func__, reg0);
	}
	return result;
}


/*! Test if charge settings are initialized or different from config.
    @param conf points configurations to compare with.
    @return int ==1: initialized or different from *conf.
                ==0: not initialized or same.
*/
STATIC_FUNC bool bq24262_wmport_is_initialized_settings(struct bq24262_wmport_context *bc)
{	bool	result;

	down(&(bc->charger_sem));
	result = bq24262_wmport_is_initialized_settings_unlocked(bc);
	up(&(bc->charger_sem));
	return result;
}

/*! Refrect confing to charger_ps_status.
*/
void bq24262_wmport_configs_refrect_charger_ps_status(struct bq24262_wmport_context *bc,
	const struct bq24262_wmport_configs *conf,
	int charge
	)
{	if (conf == NULL) {
		return;
	}
	switch (charge) {
		case BQ24262_WMPORT_BATTERY_CHARGE_UP:
			/* Enable charging. */
			bc->charger_ps_status = conf->charger_ps_status;
			break;
		case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL:
			/* Disable charging on full. */
			bc->charger_ps_status = POWER_SUPPLY_STATUS_FULL;
			break;
		case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG:
			/* Disable charging on emergency. */
			bc->charger_ps_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			break;
		default:
			/* May not come here. */
			bc->charger_ps_status = conf->charger_ps_status;
			break;
	}
}

/*! Set SAFETY register.
    @note BQ24262 doesn't have safety timer, keep TMR_1, TMR_2 as default.
*/
STATIC_FUNC int bq24262_wmport_safety_setup(struct bq24262_wmport_context *bc)
{	int	result;
	int	ret;
	uint8_t	reg;

	result = 0;
	down(&(bc->charger_sem));
	reg = BQ24262_SAFETY_TS_EN;
	ret = bq24262_wmport_i2c_read(bc, BQ24262_SAFETY, &reg);
	if (ret < 0) {
		pr_err("%s: ERROR: Can not read SAFETY. ret=%d\n", __func__, ret);
		result = ret;
	}
	reg |= BQ24262_SAFETY_TS_EN;
	ret = bq24262_wmport_i2c_write(bc, BQ24262_SAFETY, reg);
	if (ret < 0) {
		result = ret;
	}
	up(&(bc->charger_sem));
	return result;
}


#define	BQ24262_WMPORT_CONFIGS_WRITE_RETRY	(10)
/*! Configure Chager.
    @param bc driver context.
    @param conf points configuration to setup.
    @param charge _UP: charge battery; _FULL|_EMG: don't charge battery but draw power from DCIN/USB.
    @param vindpm: _LO: VINDPM goes low, _HI: VINDPM goes high.
    @return int ==0: Success, <0: Failed, negative errno number.
*/
STATIC_FUNC int bq24262_wmport_configs_write(struct bq24262_wmport_context *bc,
	const struct bq24262_wmport_configs *conf,
	int charge, int vindpm
	)
{	int	result;
	struct bq24262_wmport_platform_data	*pd;
	int	ret;
	int	retry;
	uint8_t	reg;

	if (!conf) {
		/* Don't change. */
		return 0 /* Success. */;
	}

	pd = bc->platform_data;
	down(&(bc->charger_sem));
	PRINTK_FUNC_ENTRY("%s: Write config. name=%s, flags=0x%x, charge=%d\n",
		__func__,
		conf->name,
		conf->flags,
		charge
	);
	retry = 0;
	do	{
		result = 0;

		if ((conf->flags & BQ24262_WMPORT_CONFIGS_SYS_WAK_STAT) != 0) {
			/* Suspend charging. */
			unsigned	val;

			val = BQ24262_WMPORT_SYS_WAK_STAT_READY;
			PRINTK_XX("%s: Write SYS_WAK_STAT. val=%u\n", __func__, val);
			bq24262_wmport_sys_wak_stat_write(bc, val);
		}

		if ((conf->flags & BQ24262_WMPORT_CONFIGS_SUSPEND) != 0) {
			/* Suspend charging. */
			unsigned	val;

			val = BQ24262_WMPORT_CHG_SUSPEND_HIZ;
			PRINTK_XX("%s: Write SUSPEND. val=%u\n", __func__, val);
			bq24262_wmport_chg_suspend_write(bc, val);
		}

		if (conf->flags & BQ24262_WMPORT_CONFIGS_W_CONTROL) {
			/* Write CONTROL register. */
			/* Control VBUS input (if will disable input). */
			/* @note "1st to do" is "disable (be High-Z) VBUS input". Do Here!
				 "last to do" is "enable input".
			*/
			uint8_t	reg;

			reg = conf->control;
			switch (charge) {
				case BQ24262_WMPORT_BATTERY_CHARGE_UP:
					/* Enable charging. */
					/* Refer to configuration structure member control. */
					break;
				case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL:
				case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG:
					/* Disable charging. */
					reg |=  BQ24262_CONTROL_CE_N;
					break;
				default:
					pr_err("%s: ERROR: Unexpected charge value. charge=%d\n",
						__func__,
						charge
					);
					break;
			}
			if (test_bit(BQ24262_WMPORT_CHARGER_IRQ_ENABLE, &(bc->flags)) != 0) {
				/* IRQ enabled */
				reg |= BQ24262_CONTROL_EN_STAT;
			} else {
				/* IRQ disabled. */
				reg &= ~BQ24262_CONTROL_EN_STAT;
			}
			if (bc->shutdown_req) {
				/* Requested shutdown. */
#if (defined(BQ24262_WMPORT_VBUS_100MA_AT_SHUTDOWN))
				if (bc->usb_max_power < BQ24262_WMPORT_VBUS_CURRENT_CONFIG_MA) {
					/* Low VBUS current mode. */
					reg &= ~BQ24262_CONTROL_IN_LIMIT;
					reg |= BQ24262_CONTROL_IN_LIMIT_100MA;
				} else {
#else /* (defined(BQ24262_WMPORT_VBUS_100MA_AT_SHUTDOWN)) */
			/* always do */ {
#endif /* (defined(BQ24262_WMPORT_VBUS_100MA_AT_SHUTDOWN)) */
					/* Normal VBUS current mode. */
					reg &= ~BQ24262_CONTROL_IN_LIMIT;
					reg |= BQ24262_CONTROL_IN_LIMIT_500MA;
				}
			}
			if ((reg & BQ24262_CONTROL_HZ_MODE) != 0) {
				/* Disable input. */
				PRINTK_XX("%s: Write CONTROL. addr=0x%.2x, reg=0x%.2x\n", __func__, BQ24262_CONTROL, reg);
				ret = bq24262_wmport_i2c_write(bc, BQ24262_CONTROL, reg);
				if (ret < 0) {
					result = ret;
				}
			}
		}

		if (conf->flags & BQ24262_WMPORT_CONFIGS_W_BATTERY_CURRENT) {
			/* Write Battery Current. */

			reg = conf->battery_current;
			PRINTK_XX("%s: Write BATTERY_CURRENT. addr=0x%.2x, reg=0x%.2x\n", __func__, BQ24262_BATTERY_CURRENT, reg);
			ret = bq24262_wmport_i2c_write(bc, BQ24262_BATTERY_CURRENT, reg);
			if (ret < 0) {
				result = ret;
			}
		}

		if (conf->flags & BQ24262_WMPORT_CONFIGS_W_BATTERY_VOLTAGE) {
			/* Write Battery voltage. */

			switch (bc->fully_weakly) {
				case BQ24262_WMPORT_CHARGE_FULLY:
					/* Charge up to maximum capacity. */
					reg = conf->battery_voltage & ~BQ24262_BATTERY_VOLTAGE_VBREG;
					reg |= BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_UV(bc->platform_data->battery_fully_charge_uv);
					break;

				case BQ24262_WMPORT_CHARGE_WEAKLY:
					/* Charge up to long life capacity. */
					reg = conf->battery_voltage & ~BQ24262_BATTERY_VOLTAGE_VBREG;
					reg |= BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_UV(bc->platform_data->battery_weakly_charge_uv);
					break;
				default:
					/* May not come here, but choice more safe. */
					pr_err("%s: ERROR: Unexpected charge strength. fully_weakly=%u\n", __func__, bc->fully_weakly);
					reg = BQ24262_BATTERY_VOLTAGE_VBREG_VBATT_UV(bc->platform_data->battery_weakly_charge_uv);
					break;
			}
			PRINTK_XX("%s: Write BATTERY_VOLTAGE. addr=0x%.2x, reg=0x%.2x\n", __func__, BQ24262_BATTERY_VOLTAGE, reg);
			ret = bq24262_wmport_i2c_write(bc, BQ24262_BATTERY_VOLTAGE, reg);
			if (ret < 0) {
				result = ret;
			}
		}

		if (conf->flags & BQ24262_WMPORT_CONFIGS_W_VIN_MINSYS) {
			/* Write Battery Current. */

			reg = conf->vin_minsys;
			reg &= ~(BQ24262_VIN_MINSYS_VINDPM);
			switch (vindpm) {
				case BQ24262_WMPORT_VINDPM_LO:
					reg |= BQ24262_VIN_MINSYS_VINDPM_PLUS(pd->vindpm_lo);
					break;
				case BQ24262_WMPORT_VINDPM_HI:
					reg |= BQ24262_VIN_MINSYS_VINDPM_PLUS(pd->vindpm_hi);
					break;
				default:
					/* Revert override. */
					reg = conf->vin_minsys;
					break;
			}
			PRINTK_XX("%s: Write VIN_MINSYS. addr=0x%.2x, reg=0x%.2x\n", __func__, BQ24262_VIN_MINSYS, reg);
			ret = bq24262_wmport_i2c_write(bc, BQ24262_VIN_MINSYS, reg);
			if (ret < 0) {
				result = ret;
			}
		}

		if (conf->flags & BQ24262_WMPORT_CONFIGS_FORCE_DCIN) {
			/* Force DCIN. */
			PRINTK_XX("%s: Write FORCE_DCIN. out=%u\n", __func__, BQ24262_WMPORT_FORCE_DCIN_FORCE);
			bq24262_wmport_force_dcin_write(bc, BQ24262_WMPORT_FORCE_DCIN_FORCE);
		} else {
			/* Auto select USB or DCIN. */
			PRINTK_XX("%s: Write FORCE_DCIN. out=%u\n", __func__, BQ24262_WMPORT_FORCE_DCIN_AUTO);
			bq24262_wmport_force_dcin_write(bc, BQ24262_WMPORT_FORCE_DCIN_AUTO);
		}

		if (conf->flags & BQ24262_WMPORT_CONFIGS_W_CONTROL) {
			/* Write CONTROL register. */
			/* Control VBUS input (if will enable input). */
			/* @note "1st to do" is "disable (be High-Z) VBUS input".
				 "last to do" is "enable input". Do Here!
			*/
			reg = conf->control;
			switch (charge) {
				case BQ24262_WMPORT_BATTERY_CHARGE_UP:
					/* Enable charging. */
					/* Refer to configuration structure member control. */
					break;
				case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL:
				case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG:
					/* Disable charging. */
					reg |=  BQ24262_CONTROL_CE_N;
					break;
				default:
					pr_err("%s: ERROR: Unexpected charge value. charge=%d\n",
						__func__,
						charge
					);
					break;
			}
			if (test_bit(BQ24262_WMPORT_CHARGER_IRQ_ENABLE, &(bc->flags)) != 0) {
				/* IRQ enabled */
				reg |= BQ24262_CONTROL_EN_STAT;
			} else {
				/* IRQ disabled. */
				reg &= ~BQ24262_CONTROL_EN_STAT;
			}
			if (bc->shutdown_req) {
				/* Requested shutdown. */
#if (defined(BQ24262_WMPORT_VBUS_100MA_AT_SHUTDOWN))
				if (bc->usb_max_power < BQ24262_WMPORT_VBUS_CURRENT_CONFIG_MA) {
					/* Low VBUS current mode. */
					reg &= ~BQ24262_CONTROL_IN_LIMIT;
					reg |= BQ24262_CONTROL_IN_LIMIT_100MA;
				} else {
#else /* (defined(BQ24262_WMPORT_VBUS_100MA_AT_SHUTDOWN)) */
			/* always do */ {
#endif /* (defined(BQ24262_WMPORT_VBUS_100MA_AT_SHUTDOWN)) */
					/* Normal VBUS current mode. */
					reg &= ~BQ24262_CONTROL_IN_LIMIT;
					reg |= BQ24262_CONTROL_IN_LIMIT_500MA;
				}

			}
			if ((reg & BQ24262_CONTROL_HZ_MODE) == 0) {
				/* Enable input. */
				PRINTK_XX("%s: Write CONTROL. addr=0x%.2x, reg=0x%.2x\n", __func__, BQ24262_CONTROL, reg);
				ret = bq24262_wmport_i2c_write(bc, BQ24262_CONTROL, reg);
				if (ret < 0) {
					result = ret;
				}
			}
		}

		if ((conf->flags & BQ24262_WMPORT_CONFIGS_SUSPEND) == 0) {
			/* Resume charging. */
			unsigned	val;

			val = BQ24262_WMPORT_CHG_SUSPEND_DRAW;
			PRINTK_XX("%s: Write SUSPEND. val=%u\n", __func__, val);
			bq24262_wmport_chg_suspend_write(bc, val);
		}

		if ((conf->flags & BQ24262_WMPORT_CONFIGS_SYS_WAK_STAT) == 0) {
			/* Suspend charging. */
			unsigned	val;

			val = BQ24262_WMPORT_SYS_WAK_STAT_BOOTING;
			PRINTK_XX("%s: Write SYS_WAK_STAT. val=%u\n", __func__, val);
			bq24262_wmport_sys_wak_stat_write(bc, val);
		}


		if (bq24262_wmport_is_initialized_settings_unlocked(bc)) {
			/* Initialized charge settings. */
			/* Must retry again. */
			pr_err("%s: Unexpected initialization happen.\n",__func__);
			result = -EIO;
		}
		retry++;
	} while ((result != 0) && (retry < BQ24262_WMPORT_CONFIGS_WRITE_RETRY));
	if (retry >= BQ24262_WMPORT_CONFIGS_WRITE_RETRY) {
		pr_err("%s: ERROR: Can not write configs.\n", __func__);
	}
	up(&(bc->charger_sem));
	ret = bq24262_wmport_safety_setup(bc);
	if (ret < 0) {
		return ret;
	}
	return result;
}

/*! Check if charging for wake lock.
    @param battery_charge Battery charge current control number, one of _UP, _KEEP_EMG, _KEEP_FULL
    @param config Battery charge configuration number, one of _BOOT, _HIZ, ..., _COOL_DOWN
    @return bool ==false: Not charging and also will not be charging, ==true: Charging, or will be charging.
*/
STATIC_FUNC bool bq24262_wmport_is_charging_for_wake_lock(int battery_charge, int config)
{	const struct bq24262_wmport_configs	*config_ptr;

	switch (battery_charge) {
		case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL:
			/* Battery is charged full, and stop current from/to battery. */
			return false;
		default:
			/* do nothing. */
			/* @note if battery_charge is _KEEP_EMG now, it will turn into _UP state.
			         So _KEEP_EMG state means (will be) charging.
			*/
			break;
	}
	config_ptr = bq24262_wmport_configs_table[config];
	if (config_ptr == NULL) {
		/* config == _BOOT. */
		return false;
	}
	if ((config_ptr->flags & BQ24262_WMPORT_CONFIGS_DRAWING) == 0) {
		/* NOT drawing current from power source. */
		return false /* Will not be charging. */;
	}
	return true;
}

/*! Check if charging for timer.
    @param battery_charge Battery charge current control number, one of _UP, _KEEP_EMG, _KEEP_FULL
    @param config Battery charge configuration number, one of _BOOT, _HIZ, ..., _COOL_DOWN
    @return bool ==false: Not charging, ==true: Charging.
*/
STATIC_FUNC bool bq24262_wmport_is_charging_for_timer(int battery_charge, int config)
{	if (battery_charge == BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG) {
		return false;
	}
	if (config == BQ24262_WMPORT_CONFIG_COOL_DOWN) {
		return false;
	}
	return bq24262_wmport_is_charging_for_wake_lock(battery_charge, config);
}

/*! Check if drawing current from power source (VBUS or DCIN)
    @param battery_charge Battery charge current control number, one of _UP, _KEEP_EMG, _KEEP_FULL
    @param config Battery charge configuration number, one of _BOOT, _HIZ, ..., _COOL_DOWN
    @return bool ==false: Not drawing, ==true: Drawing.
*/
STATIC_FUNC bool bq24262_wmport_is_drawing(int config)
{	const struct bq24262_wmport_configs	*config_ptr;

	config_ptr = bq24262_wmport_configs_table[config];
	if (config_ptr == NULL) {
		/* config == _BOOT. */
		return false;
	}
	if ((config_ptr->flags & BQ24262_WMPORT_CONFIGS_DRAWING) == 0) {
		/* NOT drawing current from power source. */
		return false;
	}
	return true;
}

#if 0

/* Trivial capacity vs voltage curve fitting function. */
						/*   mV uV */
#define	BQ24262_WMPORT_BATTERY_UV_00P			(3350000LL)
#define	BQ24262_WMPORT_BATTERY_UV_00P_PROP		       (0L)
#define	BQ24262_WMPORT_BATTERY_UV_10P			(3550000LL)
#define	BQ24262_WMPORT_BATTERY_UV_10P_PROP		       (5L)
#define	BQ24262_WMPORT_BATTERY_UV_80P_PROP		      (80L)
#define	BQ24262_WMPORT_BATTERY_UV_100P			(4195000LL)
#define	BQ24262_WMPORT_BATTERY_UV_100P_PROP		     (100L)

#define	BQ24262_WMPORT_BATTERY_UV_80P_OFFSET		(-320000LL)
#define	BQ24262_WMPORT_BATTERY_UV_100P_OFFSET		  (-5000LL)

#define	BQ24262_WMPORT_BATTERY_UV_DISCHARGE_OFFSET	(  20000LL)

/*! Trivial capacity vs voltage fitting, (unlocked)
*/
static int bq24262_wmport_battery_uv_to_capacity_trivial_unlocked(struct bq24262_wmport_context *bc, long battery_uv_i)
{	long					battery_uv;
	long					 c80_uv;
	long					c100_uv;
	uint64_t				t;
	uint64_t				result;

	struct bq24262_wmport_platform_data	*pd;

	battery_uv = battery_uv_i;
	if (!bq24262_wmport_is_charging_for_timer(bc->battery_charge, bc->config)) {
		/* Discharging. */
		battery_uv += BQ24262_WMPORT_BATTERY_UV_DISCHARGE_OFFSET;
	}

	if (battery_uv < BQ24262_WMPORT_BATTERY_UV_00P) {
		return BQ24262_WMPORT_BATTERY_UV_00P_PROP /* percent */;
	}

	if (battery_uv > BQ24262_WMPORT_BATTERY_UV_100P) {
		return BQ24262_WMPORT_BATTERY_UV_100P_PROP /* percent */;
	}

	if (battery_uv < BQ24262_WMPORT_BATTERY_UV_10P) {
		t =        (battery_uv -                         BQ24262_WMPORT_BATTERY_UV_00P);
		t *=       (BQ24262_WMPORT_BATTERY_UV_10P_PROP - BQ24262_WMPORT_BATTERY_UV_00P_PROP);
		do_div(t , (BQ24262_WMPORT_BATTERY_UV_10P -      BQ24262_WMPORT_BATTERY_UV_00P));
		result = t + BQ24262_WMPORT_BATTERY_UV_00P_PROP;

		return  /* force_cast */(int)result;
	}
	pd = bc->platform_data;
	switch (bc->fully_weakly) {
		case BQ24262_WMPORT_CHARGE_FULLY:
			 c80_uv = pd->battery_fully_charge_uv + BQ24262_WMPORT_BATTERY_UV_80P_OFFSET;
			c100_uv = pd->battery_fully_charge_uv + BQ24262_WMPORT_BATTERY_UV_100P_OFFSET;
			break;
		case BQ24262_WMPORT_CHARGE_WEAKLY:
			 c80_uv = pd->battery_weakly_charge_uv + BQ24262_WMPORT_BATTERY_UV_80P_OFFSET;
			c100_uv = pd->battery_weakly_charge_uv + BQ24262_WMPORT_BATTERY_UV_100P_OFFSET;
			break;
		default:
			pr_err("%s: ERROR: Invalid fully or softy mode. fully_weakly=%d\n", __func__, bc->fully_weakly);
			 c80_uv = pd->battery_fully_charge_uv + BQ24262_WMPORT_BATTERY_UV_80P_OFFSET;
			c100_uv = pd->battery_fully_charge_uv + BQ24262_WMPORT_BATTERY_UV_100P_OFFSET;
			break;
	}

	if (battery_uv < c80_uv) {
		t =        (battery_uv                         - BQ24262_WMPORT_BATTERY_UV_10P);
		t *=       (BQ24262_WMPORT_BATTERY_UV_80P_PROP - BQ24262_WMPORT_BATTERY_UV_10P_PROP);
		do_div(t , (c80_uv -     BQ24262_WMPORT_BATTERY_UV_10P));
		result = t + BQ24262_WMPORT_BATTERY_UV_10P_PROP;

		return  /* force_cast */(int)result;
	}

	t =          (battery_uv - c80_uv);
	t *=         (BQ24262_WMPORT_BATTERY_UV_100P_PROP - BQ24262_WMPORT_BATTERY_UV_80P_PROP);
	do_div(t ,   (c100_uv    - c80_uv));
	result = t + BQ24262_WMPORT_BATTERY_UV_80P_PROP;

	if (result > BQ24262_WMPORT_BATTERY_UV_100P_PROP) {
		result = BQ24262_WMPORT_BATTERY_UV_100P_PROP;
	}
	return /* force_cast */(int)result;
}

#endif

static int bq24262_wmport_battery_uv_to_capacity_trivial_unlocked(struct bq24262_wmport_context * bc, long voltage)
{
	/* bool charging=false; */
	long ofs=0;
	long cap;
	int n;

	if(bq24262_wmport_is_charging_for_timer(bc->battery_charge, bc->config)){
		/* charging=true; */
		ofs=BQ24262_WMPORT_CHG_OFFSET_UV;
	}

	if(voltage>=(battery_table[battery_table_count-1].voltage+ofs)){
		/* pr_err("======== %s 100%%, %ld\n",charging?"charge":"dischg",voltage); */
		return 100;
	}

	for(n=battery_table_count-2;n>=0;n--){
		if(voltage>=(battery_table[n].voltage+ofs)){
			cap=(battery_table[n+1].voltage+ofs)-voltage;
			cap=cap*(battery_table[n+1].percent-battery_table[n].percent);
			/* cap=cap/((battery_table[n+1].voltage+ofs)-(battery_table[n].voltage+ofs)); */
			do_div(cap,((battery_table[n+1].voltage+ofs)-(battery_table[n].voltage+ofs)));
			cap=battery_table[n+1].percent-cap;
			/* pr_err("======== %s %ld%%, %ld\n",charging?"charge":"dischg",cap,voltage); */
			return cap;
		}
	}

	/* pr_err("======== %s 0%%, %ld\n",charging?"charge":"dischg",voltage); */
	return 0;
}

/*! Read interrupt factor status.
*/
STATIC_FUNC int bq24262_wmport_status_read(struct bq24262_wmport_context *bc, bool update_prev)
{	int				result;
	int				ret;
	struct bq24262_wmport_status	cst;
	int				battery_boost;
	long				battery_uv;
	int				battery_capacity;
	uint32_t			battery_temp_raw;
	int				battery_temp_state;
	int				vindpm_state;
	unsigned			power_source;
	unsigned			power_source_raw;

	struct bq24262_wmport_platform_data	*pd;

	result = 0;
	down(&(bc->charger_sem));
	ret = bq24262_wmport_i2c_read(bc, BQ24262_STATUS_CONTROL, &(cst.status));
	if (ret < 0) {
		pr_err("%s: ERROR: Can not read STATUS. ret=%d\n", __func__, ret);
		result = ret;
	}
	ret = bq24262_wmport_i2c_read(bc, BQ24262_VIN_MINSYS, &(cst.vin_minsys));
	if (ret < 0) {
		pr_err("%s: ERROR: Can not read VIN_MINSYS. ret=%d\n", __func__, ret);
		result = ret;
	}
	ret = bq24262_wmport_i2c_read(bc, BQ24262_SAFETY, &(cst.safety));
	if (ret < 0) {
		pr_err("%s: ERROR: Can not read SAFETY. ret=%d\n", __func__, ret);
		result = ret;
	}
	up(&(bc->charger_sem));
	power_source = bq24262_wmport_ppath_read(bc, &power_source_raw);
	if (test_bit(BQ24262_WMPORT_SUSPEND, &(bc->flags)) == 0) {
		/* "on" state. */
		battery_uv = bq24262_wmport_vbat_read(bc);
	} else {
		/* Called-_suspend .. Called-_resume. */
		/* Use snapshoted value. */
		battery_uv = bc->battery_uv_suspend;
	}

	/* battery voltage is down only without charging. (hysterisis) */
	if((cst.status&BQ24262_STATUS_CONTROL_STAT)!=BQ24262_STATUS_CONTROL_STAT_CHARGE_IN_PROGRESS){
		if(battery_uv>bc->battery_uv)
			battery_uv=bc->battery_uv;
	}
	/* PRINTK_LI("battery_uv,%ld\n",battery_uv); */

	pd = bc->platform_data;
	down(&(bc->status_sem));
	battery_capacity = bq24262_wmport_battery_uv_to_capacity_trivial_unlocked(bc, battery_uv);
	battery_boost = bc->battery_boost;
	switch (battery_boost) {
		case BQ24262_WMPORT_BATTERY_BOOST_CHARGE:
			/* Not full, charging or not charging. */
			switch (cst.status & BQ24262_STATUS_CONTROL_STAT) {
				case BQ24262_STATUS_CONTROL_STAT_CHARGE_DONE:
					/* Fully charged. */
					PRINTK_HV("%s: Enter full charge stop mode. status=0x%.2x, battery_uv=%ld\n", __func__, cst.status, battery_uv);
					battery_boost = BQ24262_WMPORT_BATTERY_BOOST_STOP;
					break;
				default:
					/* do nothing. */
					break;
			}
			break;
		case BQ24262_WMPORT_BATTERY_BOOST_STOP:
			/* Full stop mode, keep battery current zero. */
			if (battery_uv < (pd->battery_resume_charge_uv)) {
				/* battery voltage goes down. */
				PRINTK_HV("%s.VBAT: Exit full charge stop mode. status=0x%.2x, battery_uv=%ld\n", __func__, cst.status, battery_uv);
				battery_boost = BQ24262_WMPORT_BATTERY_BOOST_CHARGE;
			}
			if (power_source == 0) {
				/* No power. */
				PRINTK_HV("%s.NOP: Exit full charge stop mode. status=0x%.2x, battery_uv=%ld\n", __func__, cst.status, battery_uv);
				battery_boost = BQ24262_WMPORT_BATTERY_BOOST_CHARGE;
			}
			switch (cst.status & BQ24262_STATUS_CONTROL_STAT) {
				case BQ24262_STATUS_CONTROL_STAT_FAULT:
					/* Some fault happen. */
					switch (cst.status & BQ24262_STATUS_CONTROL_FAULT) {
						case BQ24262_STATUS_CONTROL_FAULT_UVLO:
						case BQ24262_STATUS_CONTROL_FAULT_TERMAL:
						case BQ24262_STATUS_CONTROL_FAULT_BATT_TEMP:
						case BQ24262_STATUS_CONTROL_FAULT_TIMER: /* may not happen. */
						case BQ24262_STATUS_CONTROL_FAULT_NO_BATT:
							PRINTK_HV("%s.FAULT: Exit full charge stop mode. status=0x%.2x, battery_uv=%ld\n", __func__, cst.status, battery_uv);
							battery_boost = BQ24262_WMPORT_BATTERY_BOOST_CHARGE;
							break;
						default:
							/* do nothing. */
							break;
					}
					break;
				default:
					/* do nothing. */
					break;
			}
			break;
		default:
			pr_err("%s: ERROR: Unexpected battery boost state. battery_boost=%d\n", __func__, battery_boost);
			battery_boost = BQ24262_WMPORT_BATTERY_BOOST_CHARGE;
			break;
	}
	if (test_bit(BQ24262_WMPORT_SUSPEND, &(bc->flags)) == 0) {
		/* "on" state. */
		battery_temp_raw = bq24262_wmport_temp_read(bc);
	} else {
		/* Called-_suspend .. Called-_resume. */
		/* Use snapshoted value. */
		battery_temp_raw = bc->battery_temp_raw_suspend;
	}
	battery_temp_state = bc->battery_temp_state;

	if ((DEBUG_IGNORE_BATTERY_TEMP_ENABLE == 0) || ((bq24262_wmport_debug&DEBUG_FLAG_IGNORE_BATTERY_TEMP) == 0)) {
		if (bc->config_ptr!=NULL && bc->config_ptr->notify_health){
			switch (battery_temp_state) {
				case BQ24262_WMPORT_BATTERY_TEMP_STATE_NORMAL:
					if (battery_temp_raw <= pd->battery_temp_hi_enter_raw) {
						pr_err("%s.NORMAL: Enter HOT state. battery_temp_raw=%lu(0x%lx)\n",
							__func__, (unsigned long)battery_temp_raw, (unsigned long)battery_temp_raw
						);
						battery_temp_state = BQ24262_WMPORT_BATTERY_TEMP_STATE_HOT;
					} else {
						if (battery_temp_raw >= pd->battery_temp_lo_enter_raw) {
							pr_err("%s.NORMAL: Enter COLD state. battery_temp_raw=%lu(0x%lx)\n",
								__func__, (unsigned long)battery_temp_raw, (unsigned long)battery_temp_raw
							);
							battery_temp_state = BQ24262_WMPORT_BATTERY_TEMP_STATE_COLD;
						}
					}
					break;
				case BQ24262_WMPORT_BATTERY_TEMP_STATE_COLD:
					if (battery_temp_raw <= pd->battery_temp_hi_enter_raw) {
						pr_err("%s.COLD: Enter HOT state. battery_temp_raw=%lu(0x%lx)\n",
							__func__, (unsigned long)battery_temp_raw, (unsigned long)battery_temp_raw
						);
						battery_temp_state = BQ24262_WMPORT_BATTERY_TEMP_STATE_HOT;
					} else {
						if (battery_temp_raw < pd->battery_temp_lo_exit_raw) {
							/* not error */ pr_err("%s.COLD: Enter NORMAL state. battery_temp_raw=%lu(0x%lx)\n",
								__func__, (unsigned long)battery_temp_raw, (unsigned long)battery_temp_raw
							);
							battery_temp_state = BQ24262_WMPORT_BATTERY_TEMP_STATE_NORMAL;
						}
					}
					break;
				case BQ24262_WMPORT_BATTERY_TEMP_STATE_HOT:
					if (battery_temp_raw >= pd->battery_temp_lo_enter_raw) {
						pr_err("%s.HOT: Enter COLD state. battery_temp_raw=%lu(0x%lx)\n",
							__func__, (unsigned long)battery_temp_raw, (unsigned long)battery_temp_raw
						);
						battery_temp_state = BQ24262_WMPORT_BATTERY_TEMP_STATE_COLD;
					} else {
						if (battery_temp_raw > pd->battery_temp_hi_exit_raw) {
							/* not error */ pr_err("%s.HOT: Enter NORMAL state. battery_temp_raw=%lu(0x%lx)\n",
								__func__, (unsigned long)battery_temp_raw, (unsigned long)battery_temp_raw
							);
							battery_temp_state = BQ24262_WMPORT_BATTERY_TEMP_STATE_NORMAL;
						}
					}
					break;
				default:
					/* May not come here. For safety, we change state HOT. */
					pr_err("%s.DEFAULT: Enter HOT state. battery_temp_raw=%lu(0x%lx)\n",
						__func__, (unsigned long)battery_temp_raw, (unsigned long)battery_temp_raw
					);
					battery_temp_state = BQ24262_WMPORT_BATTERY_TEMP_STATE_HOT;
					break;
			}
		}
		else{
			battery_temp_state = BQ24262_WMPORT_BATTERY_TEMP_STATE_NORMAL;
		}
	}
	vindpm_state = bc->vindpm_state;
	switch (vindpm_state) {
		case BQ24262_WMPORT_VINDPM_LO:
			/* VINDPM voltage low. */
			if (battery_uv >= pd->vindpm_hi_enter_battery_uv) {
				PRINTK_LI("%s: Enter VINDPM High mode. battery_uv=%ld\n", __func__, battery_uv);
				vindpm_state = BQ24262_WMPORT_VINDPM_HI;
			}
			break;
		case BQ24262_WMPORT_VINDPM_HI:
			/* VINDPM voltage high. */
			if (battery_uv <  pd->vindpm_lo_enter_battery_uv) {
				PRINTK_LI("%s: Enter VINDPM Low mode. battery_uv=%ld\n", __func__, battery_uv);
				vindpm_state = BQ24262_WMPORT_VINDPM_LO;
			}
			break;
		default:
			pr_err("%s.VINDPM.DEFAULT: Unexpected state. vindpm_state=%d\n", __func__, bc->vindpm_state);
			vindpm_state = BQ24262_WMPORT_VINDPM_HI;
			break;
	}
	if (update_prev) {
		bc->charger_status_prev =     bc->charger_status;	/* copy struct. */
		bc->battery_boost_prev =      bc->battery_boost;
		bc->battery_capacity_prev =   bc->battery_capacity;
		bc->battery_temp_state_prev = bc->battery_temp_state;
		bc->vindpm_state_prev =       bc->vindpm_state;
		bc->power_source_prev =       bc->power_source;
		bc->power_source_raw_prev =   bc->power_source_raw;
	}
	bc->charger_status =	    cst;	/* copy struct. */
	bc->power_source =	    power_source;
	bc->power_source_raw =      power_source_raw;
	bc->battery_boost =         battery_boost;
	bc->battery_uv =	    battery_uv;
	bc->battery_capacity =      battery_capacity;
	bc->battery_temp_raw =      battery_temp_raw;
	bc->battery_temp_state =    battery_temp_state;
	bc->vindpm_state =          vindpm_state;
	up(&(bc->status_sem));
	return result;
}

/*! Show charger status.
    @param bc driver context.
    @note Intentionally unlocked.
*/
STATIC_FUNC void bq24262_wmport_status_show(struct bq24262_wmport_context *bc)
{	PRINTK_LI("%s: Status. (hex)STA.VIN.SAF=%.2x.%.2x.%.2x\n",
		__func__,
		bc->charger_status.status,
		bc->charger_status.vin_minsys,
		bc->charger_status.safety
	);
}

/*! Refrect current condition to wake lock.
    @param bc driver context.
*/
STATIC_FUNC void bq24262_wmport_wake_lock_refrect(struct bq24262_wmport_context *bc)
{	int	result;
	int	continue_wake;

	continue_wake = 0;
	result = 1;
	down(&(bc->status_sem));
	down(&(bc->usb_sem));
	down(&(bc->wake_lock_sem));
	if ((bc->power_source) == 0) {
		/* No power source. */
		goto out;
	}
	if (bc->wake_lock_source & BQ24262_WMPORT_WAKE_LOCK_DCIN) {
		/* Wake lock when DCIN powered. */
		if (bc->power_source & BQ24262_WMPORT_DCIN) {
			/* DCIN powered. */
			continue_wake = 1;
			goto out;
		}
	}
	/* Here, no power sourced to DCIN. */
	if (bc->wake_lock_source & BQ24262_WMPORT_WAKE_LOCK_USB) {
		/* Wake lock when USB VBUS powered. */
		if ((bc->power_source & BQ24262_WMPORT_USB) != 0) {
			/* VBUS powered. */
			switch (bc->usb_bcdet) {
				case ICX_CHARGER_CDP:
				case ICX_CHARGER_DCP:
				case ICX_CHARGER_APL_0R5:
				case ICX_CHARGER_APL_1R0:
				case ICX_CHARGER_APL_2R1:
				case ICX_CHARGER_AC_S508U:
					/* USB-AC adaptors. */
					continue_wake = 1;
					goto out;

				case ICX_CHARGER_STD:
					/* Standard port. */
					switch (bc->usb_state) {
						case BQ24262_WMPORT_USB_SUSPENDED:
						case BQ24262_WMPORT_USB_UNCONFIG:
							/* Suspended, or Unconfigured */
							/* the voltage VBUS is rised enough but, do not draw. */
							goto out;
						case BQ24262_WMPORT_USB_CONFIG:
							/* Configured. */
							continue_wake = 1;
							goto out;
						case BQ24262_WMPORT_USB_CONFIG_LOW:
							/* Configured, but low current. */
							/* @note CONFIG_LOW doesn't draw current from VBUS,
							         but keep connection to the host, USB gadget or
							         services (in userland) may keep waking.
							*/
							continue_wake = 0;
							goto out;
						case BQ24262_WMPORT_USB_LBB_DRAW:
							/* Low battery boot, USB host sleeps. */
							/* @note When suspend requested, configure charger with "USB_SUSPENDED". */
							continue_wake = 0;
							goto out;
						case BQ24262_WMPORT_USB_UNKNOWN:
							/* Unknown USB state. */
							/* @note When suspend requested, configure charger with "USB_SUSPENDED". */
							continue_wake = 0;
							goto out;
						default:
							pr_err("%s: ERROR: Unknown USB state. usb_state=%d\n", __func__, bc->usb_state);
							continue_wake = 0;
							goto out;
					}
					break;
				case ICX_CHARGER_MISC_OPEN:
				case ICX_CHARGER_MISC_XXX:
					/* All handmade or broken adaptors. */
					continue_wake = 0;
					goto out;
				case ICX_CHARGER_UNKNOWN:
					/* Could not identify USB adaptor. */
					continue_wake = 0;
					goto out;
				default:
					pr_err("%s: ERROR: Unknown USB charger. usb_bcdet=%u\n", __func__, bc->usb_bcdet);
					continue_wake = 0;
					goto out;
			}
		}
	}
	if (bc->wake_lock_source & BQ24262_WMPORT_WAKE_LOCK_CHARGING) {
		/* Wake lock when charging. */
		continue_wake = (/* bool to */ int)bq24262_wmport_is_charging_for_wake_lock(bc->battery_charge, bc->config);
	}
out:
	if (continue_wake) {
		/* will lock wake. */
		if (test_and_set_bit(BQ24262_WMPORT_WAKE_LOCK_LOCKING, &(bc->wake_lock_flags)) == 0) {
			/* Now not locked. */
			PRINTK_WL("%s: wake_lock charging.\n", __func__);
			wake_lock(&(bc->wake_lock_charging));
		}
	} else {
		/* will unlock wake. */
		if (test_and_clear_bit(BQ24262_WMPORT_WAKE_LOCK_LOCKING, &(bc->wake_lock_flags))) {
			/* Now locked. */
			PRINTK_WL("%s: wake_unlock charging.\n", __func__);
			wake_unlock(&(bc->wake_lock_charging));
		}
	}
	up(&(bc->wake_lock_sem));
	up(&(bc->usb_sem));
	up(&(bc->status_sem));
}

/*! Disable charger interrupt, without semaphore lock.
    @param bc driver context.
*/
int bq24262_wmport_charger_irq_disable_unlocked(struct bq24262_wmport_context *bc)
{	uint8_t		reg;
	int		result;
	int		ret;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	result = 0;
	/* Set safe value, when read failed. */
	mt_eint_mask(bc->platform_data->chg_xstat_eint);
	clear_bit(BQ24262_WMPORT_CHARGER_IRQ_ENABLE, &(bc->flags));
	reg = BQ24262_CONTROL_IN_LIMIT_500MA | BQ24262_CONTROL_TE;
	ret = bq24262_wmport_i2c_read(bc, BQ24262_CONTROL, &reg);
	if (ret < 0) {
		pr_err("%s: ERROR: Can not read CONTROL. ret=%d\n", __func__, ret);
		result = ret;
	}
	reg &= ~(BQ24262_CONTROL_EN_STAT | BQ24262_CONTROL_RESET);
	ret = bq24262_wmport_i2c_write(bc, BQ24262_CONTROL, reg);
	if (ret < 0 ) {
		pr_err("%s: ERROR: Can not write CONTROL. ret=%d\n", __func__, ret);
		result = ret;
	}
	return result;
}


/*! Disable charger interrupt.
    @param bc driver context.
*/
int bq24262_wmport_charger_irq_disable(struct bq24262_wmport_context *bc)
{	int		result;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	down(&(bc->charger_sem));
	result = bq24262_wmport_charger_irq_disable_unlocked(bc);
	up(&(bc->charger_sem));
	return result;
}

/*! Enable charger interrupt. without semaphore lock.
    @param bc driver context.
*/
int bq24262_wmport_charger_irq_enable_unlocked(struct bq24262_wmport_context *bc)
{	uint8_t		reg;
	int		result;
	int		ret;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	result = 0;
	/* UNLOCKED charger_sem begin */
	mt_eint_unmask(bc->platform_data->chg_xstat_eint);
	/* Set safe value, when read failed. */
	set_bit(BQ24262_WMPORT_CHARGER_IRQ_ENABLE, &(bc->flags));
	reg = BQ24262_CONTROL_IN_LIMIT_500MA | BQ24262_CONTROL_EN_STAT | BQ24262_CONTROL_TE;
	ret = bq24262_wmport_i2c_read(bc, BQ24262_CONTROL, &reg);
	if (ret < 0) {
		pr_err("%s: ERROR: Can not read CONTROL. ret=%d\n", __func__, ret);
		result = ret;
	}
	reg &= ~(BQ24262_CONTROL_RESET);
	reg |= BQ24262_CONTROL_EN_STAT;
	ret = bq24262_wmport_i2c_write(bc, BQ24262_CONTROL, reg);
	if (ret < 0 ) {
		pr_err("%s: ERROR: Can not write CONTROL. ret=%d\n", __func__, ret);
		result = ret;
	}
	/* UNLOCKED charger_sem end */
	return result;
}


/*! Enable charger interrupt. with semaphore lock.
    @param bc driver context.
*/
int bq24262_wmport_charger_irq_enable(struct bq24262_wmport_context *bc)
{	int		ret;
	int		result;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	result = 0;
	down(&(bc->charger_sem));
	ret = bq24262_wmport_charger_irq_enable_unlocked(bc);
	if (ret != 0) {
		pr_err("%s: ERROR: Fail to enable charger IRQ.\n", __func__);
		result = ret;
	}
	up(&(bc->charger_sem));
	return result;
}

/*! Activate ICX platform charger IRQ worker.
    @param bc driver context.
*/

void bq24262_wmport_charger_work_activate(struct bq24262_wmport_context *bc)
{	unsigned long	flags;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	spin_lock_irqsave(&(bc->lock), flags);
	set_bit(BQ24262_WMPORT_CHARGER_WORK_ACTIVE, &(bc->flags));
	spin_unlock_irqrestore(&(bc->lock), flags);
}

/*! Shut ICX platform charger IRQ worker.
    @param bc driver context.
*/
void bq24262_wmport_charger_work_shut(struct bq24262_wmport_context *bc)
{	unsigned long	flags;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	spin_lock_irqsave(&(bc->lock), flags);
	clear_bit(BQ24262_WMPORT_CHARGER_WORK_ACTIVE, &(bc->flags));
	clear_bit(BQ24262_WMPORT_CHARGER_WORK_RETRIGGER, &(bc->flags));
	spin_unlock_irqrestore(&(bc->lock), flags);
}

/*! Cancel ICX platform charger IRQ worker.
    @param bc driver context.
*/
int bq24262_wmport_charger_work_cancel(struct bq24262_wmport_context *bc)
{
	return cancel_work_sync(&(bc->charger_work));
}


/*! Do ReTrigger ICX charger IRQ worker.
    @param bc driver context.
*/
void bq24262_wmport_charger_work_do_retrigger(struct bq24262_wmport_context *bc)
{	unsigned long	flags;

	PRINTK_IRQ_ENTRY("%s: Called.\n", __func__);
	spin_lock_irqsave(&(bc->lock), flags);
	if (test_bit(BQ24262_WMPORT_CHARGER_WORK_ACTIVE, &(bc->flags))) {
		if (test_and_clear_bit(BQ24262_WMPORT_CHARGER_WORK_RETRIGGER, &(bc->flags))) {
			PRINTK_LI("%s: Retrigger.\n", __func__);
			schedule_work(&(bc->charger_work));
		}
	} else {
		PRINTK_LI("%s: Try retrigger while inactivated.\n", __func__);
	}
	spin_unlock_irqrestore(&(bc->lock), flags);
	PRINTK_FUNC_ENTRY("%s: Exit.\n", __func__);
}

/*! Launch ICX platform charger IRQ worker.
    @param bc driver context.
*/
void bq24262_wmport_charger_work_launch(struct bq24262_wmport_context *bc)
{	unsigned long	flags;

	PRINTK_IRQ_ENTRY("%s: Called.\n", __func__);
	spin_lock_irqsave(&(bc->lock), flags);
	if (test_bit(BQ24262_WMPORT_CHARGER_WORK_ACTIVE, &(bc->flags))) {
		if (!schedule_work(&(bc->charger_work))) {
			/* already queued. */
			PRINTK_LI("%s: Already queued.\n", __func__);
			/* Retrigger power source work. */
			set_bit(BQ24262_WMPORT_CHARGER_WORK_RETRIGGER, &(bc->flags));
		}
	} else {
		PRINTK_LI("%s: Try launch while inactivated.\n", __func__);
	}
	spin_unlock_irqrestore(&(bc->lock), flags);
}

/*! ICX platform charger IRQ worker.
    @param work work_struct which container is bq24262_wmport_context.
*/
STATIC_FUNC void bq24262_wmport_charger_work(struct work_struct *work)
{
	struct bq24262_wmport_context *bc = container_of(work,
		struct bq24262_wmport_context, charger_work);

	struct bq24262_wmport_status	cst;
	int				ret;

	PRINTK_IRQ_ENTRY("%s: Called. bc=0x%p\n", __func__, bc);

	if (bq24262_wmport_debug&DEBUG_FLAG_IRQ) {
		ret = bq24262_wmport_i2c_read(bc, BQ24262_STATUS_CONTROL, &(cst.status));
		if (ret < 0) {
			pr_err("%s: Can not read STATUS. ret=%d\n", __func__, ret);
		}
		ret = bq24262_wmport_i2c_read(bc, BQ24262_VIN_MINSYS, &(cst.vin_minsys));
		if (ret < 0) {
			pr_err("%s: Can not read VIN_MINSYS. ret=%d\n", __func__, ret);
		}
		ret = bq24262_wmport_i2c_read(bc, BQ24262_SAFETY, &(cst.safety));
		if (ret < 0) {
			pr_err("%s: Can not read SAFETY. ret=%d\n", __func__, ret);
		}
		printk("%s: Charger IRQ status. STATUS=0x%.2x, VIN=0x%.2x, SAFETY=0x%.2x\n",
			__func__,
			cst.status, cst.vin_minsys, cst.safety
		);
	}
	bq24262_wmport_thr_main_event(bc);
	bq24262_wmport_charger_work_do_retrigger(bc);
}

/*! ICX platform charger IRQ handler
*/
void bq24262_wmport_charger_irq_handler(void) /* EINT_FUNC */
{	struct bq24262_wmport_static_context	*bcst;
	struct bq24262_wmport_context		*bc;
	unsigned long				flags;
	unsigned				xstat;
	unsigned				irq;
	unsigned				level;

	PRINTK_IRQ_ENTRY("%s: Called.\n", __func__);
	bcst = &bq24262_wmport_static;
	spin_lock_irqsave(&(bcst->lock), flags);
	bc = bcst->bc;
	if (!bc) {
		pr_err("%s: ERROR: Null points driver context.\n", __func__);
		/* Can't handle this interrupt any more. */
		goto out;
	}
	level = MT_POLARITY_LOW;
	xstat = bq24262_wmport_chg_xstat_read(bc);
	if (xstat == 0) {
		/* chg_xstat is low, wait turn high. */
		level = MT_POLARITY_HIGH;
	}
	/* chg_xstat is low, wait turn high. */
	irq = bc->platform_data->chg_xstat_eint;
	mt_eint_set_sens(irq, MT_LEVEL_SENSITIVE);
	mt_eint_set_polarity(irq, level);
	/* Do every thing in worker. */
	bq24262_wmport_charger_work_launch(bc);
out:
	spin_unlock_irqrestore(&(bcst->lock), flags);
	return;
}

/*! Request ICX platform charger IRQ handler.
    @param bc driver context.
*/
STATIC_FUNC int bq24262_wmport_charger_irq_request(struct bq24262_wmport_context *bc)
{	int	result;
	
	result = 0;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	bq24262_wmport_charger_irq_disable(bc);
	bq24262_wmport_charger_work_activate(bc);
	bq24262_wmport_charger_work_launch(bc);
	if (test_and_set_bit(BQ24262_WMPORT_CHARGER_IRQ_ENABLE, &(bc->flags)) == 0) {
		/* Not requested. */
		unsigned	irq;
		unsigned	level;

		irq = bc->platform_data->chg_xstat_eint;
		/* @note intentionally raise interrupt. */
		level = EINTF_TRIGGER_HIGH;
		if (bq24262_wmport_chg_xstat_read(bc) == 0) {
			/* vbus_xdet is high, intentionally raise EINT by sensing high level. */
			level = EINTF_TRIGGER_LOW;
		}
		PRINTK_LI("%s: Register CHARGER EINT. irq=%u, level=0x%x\n", __func__, irq, level);
		mt_eint_registration(irq, level, bq24262_wmport_charger_irq_handler, 1);
	}
	PRINTK_FUNC_ENTRY("%s: Exit.\n", __func__);
	return result;
}

/*! Free ICX platform power source IRQ handler.
    @param bc driver context.
*/
STATIC_FUNC void bq24262_wmport_charger_irq_free(struct bq24262_wmport_context *bc)
{	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	bq24262_wmport_charger_work_shut(bc);
	bq24262_wmport_charger_irq_disable(bc);
	bq24262_wmport_charger_work_cancel(bc);

	if (test_and_clear_bit(BQ24262_WMPORT_CHARGER_IRQ_ENABLE, &(bc->flags))) {
		/* Requested */
		unsigned	irq;
		irq = bc->platform_data->chg_xstat_eint;
		mt_eint_registration(irq, EINTF_TRIGGER_LOW, NULL, 0);
	}
}

/*! Prepare to suspend controller interrupt.
*/
int bq24262_wmport_charger_irq_suspend(struct bq24262_wmport_context *bc)
{	int		result;
	int		ret;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	result = 0;
	down(&(bc->status_sem));
	down(&(bc->charger_sem));
#if (defined(CONFIG_BQ24262_WMPORT_DISABLE_CHARGER_IRQ_SUSPEND))
	switch (bc->battery_charge) {
		case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL:
			/* "Full Stop" mode (Keep battery current 0A). */
			ret = bq24262_wmport_charger_irq_disable_unlocked(bc);
			if (ret != 0) {
				pr_err("%s: Fail to disable charger IRQ.\n", __func__);
				result = ret;
			} else {
				set_bit(BQ24262_WMPORT_CHARGER_IRQ_DISABLED, &(bc->flags));
			}
		default:
			ret = bq24262_wmport_charger_irq_enable_unlocked(bc);
			if (ret != 0) {
				pr_err("%s: Fail to enable charger IRQ.\n", __func__);
				result = ret;
			}
			break;
	}
#else /* (defined(CONFIG_BQ24262_WMPORT_DISABLE_CHARGER_IRQ_SUSPEND)) */
	ret = bq24262_wmport_charger_irq_enable_unlocked(bc);
	if (ret != 0) {
		pr_err("%s: Fail to enable charger IRQ.\n", __func__);
		result = ret;
	}
#endif /* (defined(CONFIG_BQ24262_WMPORT_DISABLE_CHARGER_IRQ_SUSPEND)) */
	up(&(bc->charger_sem));
	up(&(bc->status_sem));
	return result;
}

/*! Setup controller interrupt resume.
*/
int bq24262_wmport_charger_irq_resume(struct bq24262_wmport_context *bc)
{	int	result;
	int	ret;

	result = 0;
	down(&(bc->charger_sem));
	if (test_and_clear_bit(BQ24262_WMPORT_CHARGER_IRQ_DISABLED, &(bc->flags))) {
		/* Disabled IRQ at suspend, recover IRQ. */
		ret = bq24262_wmport_charger_irq_enable_unlocked(bc);
		if (ret != 0) {
			pr_err("%s: Fail to enable charger IRQ.\n", __func__);
			result = ret;
		}
	}
	up(&(bc->charger_sem));
	return result;
}

/*! Enable ICX platform GPIO IRQ.
    @param bc driver context.
*/
int bq24262_wmport_ppath_irq_enable(struct bq24262_wmport_context *bc)
{	unsigned long	flags;
	int		result;
	struct bq24262_wmport_platform_data	*pd;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	result = 0;
	spin_lock_irqsave(&(bc->lock), flags);
	if (test_bit(BQ24262_WMPORT_PPATH_WORK_ACTIVE, &(bc->flags))) {
		/* Activated. */
		pd = bc->platform_data;
		mt_eint_unmask(pd->dc_xdet_eint);
		mt_eint_unmask(pd->vbus_xdet_eint);
	} else {
		PRINTK_LI("%s: Try enable while inactivated.\n", __func__);
	}
	spin_unlock_irqrestore(&(bc->lock), flags);
	return result;
}

/*! Disable ICX platform power source IRQ by driver context.
    @param bc driver context.
*/
void bq24262_wmport_ppath_irq_disable(struct bq24262_wmport_context *bc)
{	unsigned long	flags;
	struct bq24262_wmport_platform_data	*pd;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	pd = bc->platform_data;
	spin_lock_irqsave(&(bc->lock), flags);
	mt_eint_mask(pd->vbus_xdet_eint);
	mt_eint_mask(pd->dc_xdet_eint);
	spin_unlock_irqrestore(&(bc->lock), flags);
}

int bq24262_wmport_ppath_irq_suspend(struct bq24262_wmport_context *bc)
{	int		result;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	result = 0;
	/* @note we may set Rising or Falling edge of irq signal
	         to detect power input change.
	*/
	return result;
}

int bq24262_wmport_ppath_irq_resume(struct bq24262_wmport_context *bc)
{	int		result;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	result = 0;
	/* @note we may set Rising or Falling edge of irq signal
	         to detect power input change.
	*/
	return result;
}

/*! Activate ICX platform power source IRQ and worker.
    @param bc driver context.
*/

void bq24262_wmport_ppath_work_activate(struct bq24262_wmport_context *bc)
{	unsigned long	flags;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	spin_lock_irqsave(&(bc->lock), flags);
	set_bit(BQ24262_WMPORT_PPATH_WORK_ACTIVE, &(bc->flags));
	spin_unlock_irqrestore(&(bc->lock), flags);
}

/*! Shut ICX platform power source IRQ and worker.
    @param bc driver context.
*/
void bq24262_wmport_ppath_work_shut(struct bq24262_wmport_context *bc)
{	unsigned long	flags;

	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	spin_lock_irqsave(&(bc->lock), flags);
	clear_bit(BQ24262_WMPORT_PPATH_WORK_ACTIVE, &(bc->flags));
	clear_bit(BQ24262_WMPORT_PPATH_WORK_RETRIGGER, &(bc->flags));
	spin_unlock_irqrestore(&(bc->lock), flags);
}

/*! Cancel ICX platform power source worker.
    @param bc driver context.
*/
int bq24262_wmport_ppath_work_cancel(struct bq24262_wmport_context *bc)
{
	PRINTK_FUNC_ENTRY("%s: Called.\n", __func__);
	return cancel_work_sync(&(bc->ppath_work));
}


/*! Do ReTrigger Power path worker.
    @param bc driver context.
*/
void bq24262_wmport_ppath_work_do_retrigger(struct bq24262_wmport_context *bc)
{	unsigned long	flags;

	spin_lock_irqsave(&(bc->lock), flags);
	if (test_bit(BQ24262_WMPORT_PPATH_WORK_ACTIVE, &(bc->flags))) {
		if (test_and_clear_bit(BQ24262_WMPORT_PPATH_WORK_RETRIGGER, &(bc->flags))) {
			PRINTK_LI("%s: Retrigger.\n", __func__);
			schedule_work(&(bc->ppath_work));
		}
	} else {
		PRINTK_LI("%s: Try retrigger while inactivated.\n", __func__);
	}
	spin_unlock_irqrestore(&(bc->lock), flags);
}

/*! Launch Power path event worker.
    @param bc driver context.
*/
void bq24262_wmport_ppath_work_launch(struct bq24262_wmport_context *bc)
{	unsigned long	flags;

	spin_lock_irqsave(&(bc->lock), flags);
	if (test_bit(BQ24262_WMPORT_PPATH_WORK_ACTIVE, &(bc->flags))) {
		if (!schedule_work(&(bc->ppath_work))) {
			/* already queued. */
			PRINTK_LI("%s: Already queued.\n", __func__);
			/* Retrigger power source work. */
			set_bit(BQ24262_WMPORT_PPATH_WORK_RETRIGGER, &(bc->flags));
		}
	} else {
		PRINTK_LI("%s: Try launch while inactivated.\n", __func__);
	}
	spin_unlock_irqrestore(&(bc->lock), flags);
}

/*! Power path event worker.
    @param work work_struct which container is bq24262_wmport_context.
*/
STATIC_FUNC void bq24262_wmport_ppath_work(struct work_struct *work)
{	unsigned	power_source;
	unsigned	power_source_raw;
	unsigned	diff;
	bool		event_thr_main;
	bool		event_thr_vbus;

	struct bq24262_wmport_context *bc = container_of(work,
		struct bq24262_wmport_context, ppath_work);

	event_thr_main = false;
	event_thr_vbus = false;
	power_source_raw = 0;
	power_source = bq24262_wmport_ppath_read(bc, &power_source_raw);
	PRINTK_IRQ_ENTRY("%s: Power source changed. ps=0x%x, ps_raw=0x%x\n",
		__func__,
		power_source, power_source_raw
	);

	/* Update status in locked context. */
	down(&(bc->status_sem));
	bc->power_source_irq_prev =     bc->power_source_irq;
	bc->power_source_raw_irq_prev = bc->power_source_raw_irq;
	bc->power_source_irq = power_source;
	bc->power_source_raw_irq = power_source_raw;
	up(&(bc->status_sem));
	/* In this context, we use status without lock,
	   Because power_source_*_irq_* are updated only here.
	*/
	diff = power_source_raw ^ bc->power_source_raw_irq_prev;
	if (diff != 0) {
		/* Changed power source. */
		event_thr_main = true;
	}
	if ((diff & BQ24262_WMPORT_USB) != 0) {
		event_thr_vbus = true;
	}
	diff = power_source ^ bc->power_source_irq_prev;
	if (diff) {
		event_thr_main = true;
	}
	if (event_thr_vbus) {
		PRINTK_TV("%s: Wake thread VBUS.\n", __func__);
		bq24262_wmport_thr_vbus_event(bc_to_vb(bc));
	}
	if (event_thr_main) {
		bq24262_wmport_thr_main_event(bc);
	}
	bq24262_wmport_ppath_work_do_retrigger(bc);
	bc->ppath_work_wake_count++;
}

/*! VBUS Power source Interrupt handler
    @param irq IRQ number.
    @param dev points bq24262_wmport_context.
    @brief interrupted when USB or DCIN connects happen.
*/
void bq24262_wmport_ppath_vbus_irq_handler(void) /* EINT_FUNC */ 
{	struct bq24262_wmport_static_context	*bcst;
	struct bq24262_wmport_context		*bc;
	unsigned long				flags;
	unsigned				raw;
	unsigned				irq;
	unsigned				level;

	PRINTK_IRQ_ENTRY("%s: Called.\n",__func__);
	bcst = &bq24262_wmport_static;
	spin_lock_irqsave(&(bcst->lock), flags);
	bc = bcst->bc;
	if (!bc) {
		pr_err("%s: ERROR: Null points driver context.\n", __func__);
		/* Can't handle this interrupt any more. */
		goto out;
	}

	level = MT_POLARITY_HIGH;
	raw = bq24262_wmport_ppath_read_raw(bc);
	if ((raw & BQ24262_WMPORT_USB) == 0) {
		/* No power at VBUS, wait turn low(powered). */
		level = MT_POLARITY_LOW;
	}

	irq = bc->platform_data->vbus_xdet_eint;	/* Low Active. */
	mt_eint_set_sens(irq, MT_LEVEL_SENSITIVE);
	mt_eint_set_polarity(irq, level);
	/* Do every thing in worker. */
	bq24262_wmport_ppath_work_launch(bc);
out:
	spin_unlock_irqrestore(&(bcst->lock), flags);
	return;
}

/*! DCIN Power source Interrupt handler
    @param irq IRQ number.
    @param dev points bq24262_wmport_context.
    @brief interrupted when USB or DCIN connects happen.
*/
void bq24262_wmport_ppath_dcin_irq_handler(void) /* EINT_FUNC */ 
{	struct bq24262_wmport_static_context	*bcst;
	struct bq24262_wmport_context		*bc;
	unsigned long				flags;
	unsigned				raw;
	unsigned				irq;
	unsigned				level;

	PRINTK_IRQ_ENTRY("%s: Called.\n",__func__);
	bcst = &bq24262_wmport_static;
	spin_lock_irqsave(&(bcst->lock), flags);
	bc = bcst->bc;
	if (!bc) {
		pr_err("%s: Null points driver context.\n", __func__);
		/* Can't handle this interrupt any more. */
		goto out;
	}

	level = MT_POLARITY_HIGH;
	raw = bq24262_wmport_ppath_read_raw(bc);
	if ((raw & BQ24262_WMPORT_DCIN) == 0) {
		/* No power at DCIN, wait turn low(powered). */
		level = MT_POLARITY_LOW;
	}
	irq = bc->platform_data->dc_xdet_eint; /* Low Active */
	mt_eint_set_sens(irq, MT_LEVEL_SENSITIVE);
	mt_eint_set_polarity(irq, level);

	/* Do every thing in worker. */
	bq24262_wmport_ppath_work_launch(bc);
out:
	spin_unlock_irqrestore(&(bcst->lock), flags);
	return;
}


/*! Request power path IRQ handler.
    @param bc driver context.
*/
STATIC_FUNC int bq24262_wmport_ppath_irq_request(struct bq24262_wmport_context *bc)
{	int					result;
	struct bq24262_wmport_platform_data	*pd;
	unsigned				raw;

	result = 0;
	bq24262_wmport_ppath_irq_disable(bc);
	bq24262_wmport_ppath_work_activate(bc);
	raw = bq24262_wmport_ppath_read_raw(bc);

	pd = bc->platform_data;
	if (test_and_set_bit(BQ24262_WMPORT_USB_IRQ, &(bc->flags)) == 0) {
		/* Not requested. */
		unsigned	irq;
		unsigned	level;
		
		/* @note intentionally raise interrupt. */
		level = EINTF_TRIGGER_LOW;
		if ((raw & BQ24262_WMPORT_USB) == 0) {
			/* vbus_xdet is high, intentionally raise EINT by sensing high level. */
			level = EINTF_TRIGGER_HIGH;
		}
		irq = pd->vbus_xdet_eint;
		PRINTK_LI("%s: Register VBUS EINT. irq=%u, level=0x%x\n", __func__, irq, level);
		mt_eint_registration(irq, level, bq24262_wmport_ppath_vbus_irq_handler, 1);
	}
	if (test_and_set_bit(BQ24262_WMPORT_DCIN_IRQ, &(bc->flags)) == 0) {
		/* Not requested. */
		unsigned	irq;
		unsigned	level;

		/* @note intentionally raise interrupt. */
		level = EINTF_TRIGGER_LOW;
		if ((raw & BQ24262_WMPORT_DCIN) == 0) {
			/* dcin_xdet is high, intentionally raise EINT by sensing high level. */
			level = EINTF_TRIGGER_HIGH;
		}
		irq = pd->dc_xdet_eint;
		PRINTK_LI("%s: Register DCIN EINT. irq=%u, level=0x%x\n", __func__, irq, level);
		mt_eint_registration(irq, level, bq24262_wmport_ppath_dcin_irq_handler, 1);
	}
	return result;
}

/*! Free power path IRQ handler.
    @param bc driver context.
*/
STATIC_FUNC void bq24262_wmport_ppath_irq_free(struct bq24262_wmport_context *bc)
{	struct bq24262_wmport_platform_data	*pd;

	bq24262_wmport_ppath_work_shut(bc);
	bq24262_wmport_ppath_irq_disable(bc);
	bq24262_wmport_ppath_work_cancel(bc);

	pd = bc->platform_data;
	if (test_and_clear_bit(BQ24262_WMPORT_USB_IRQ, &(bc->flags))) {
		/* Requested. */
		unsigned	irq;
		irq = pd->vbus_xdet_eint;
		mt_eint_registration(irq, EINTF_TRIGGER_LOW, NULL, 0);
	}
	if (test_and_clear_bit(BQ24262_WMPORT_DCIN_IRQ, &(bc->flags))) {
		/* Requested. */
		unsigned	irq;
		irq = pd->dc_xdet_eint;
		mt_eint_registration(irq, EINTF_TRIGGER_LOW, NULL, 0);
	}
}

/*! Time to sense again USB/DCIN power source condition after\
    update power source configuration in milli seconds.
*/
#define	BQ24262_WMPORT_THR_MAIN_TIMEOUT_CHANGED_PEND_MS		(200UL)
#define	BQ24262_WMPORT_THR_MAIN_TIMEOUT_STATUS_POLLING_MS	(500UL)
#define	BQ24262_WMPORT_THR_MAIN_TIMEOUT_STATUS_POLLING_REPEATS	(4)
#define	BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_LOW_POLLING_MS		(10*1000UL)
#define	BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_TEMP_NORM_POLLING_MS	(30*1000UL)
#define	BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_TEMP_LOHI_POLLING_MS	(60*1000UL)
#define	BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_DISCHARGE_POLLING_MS	(30*1000UL)

#define	BQ24262_WMPORT_THR_MAIN_TIMEOUT_UNKNOWN_POLLING_MS	( 1  * 1000UL)
#define	BQ24262_WMPORT_THR_MAIN_TIMEOUT_UNKNOWN_EXPIRE_MS	(50  * 1000UL)
#define	BQ24262_WMPORT_THR_MAIN_TIMEOUT_UNKNOWN_RUNNING_MS	(30  * 1000UL)

#define	BQ24262_WMPORT_DIAG_VBUS_DOWN_OFF_MS	(2 * 1000)

#define	BQ24262_WMPORT_BATTERY_CHARGED_ENOUGH_COUNT	(20)

/* Timeout milli seconds syncing with power supply thread at
   suspend, resume, and shutdown.
*/
#define	BQ24262_WMPORT_THR_MAIN_SYNC_SUSPEND_TIMEOUT_MS		(4000UL)
#define	BQ24262_WMPORT_THR_MAIN_SYNC_RESUME_TIMEOUT_MS		(4000UL)
#define	BQ24262_WMPORT_THR_MAIN_SYNC_SHUTDOWN_TIMEOUT_MS	(4000UL)

/*! wake lock time on resume */
#define BQ24262_WMPORT_RESUME_WAKE_LOCK_TURN_ON_TIME_MS	(5000 /* ms */)
#define BQ24262_WMPORT_RESUME_WAKE_LOCK_KEEP_TIME_MS	(2000 /* ms */)

/*! High frequency loop trace period. */
#define	BQ24262_WMPORT_THR_MAIN_TRACE_PERIOD_MS	(1000)
#define	BQ24262_WMPORT_THR_MAIN_TRACE_RATIO	(100)


/*! Test if on diag service.
    @param bc battery charger driver context.
    @pre "down bc->status_sem" or "in thread main"
    @return bool ==true:  On diag service.
                 ==false: Not on diag service (normal operation)
*/
STATIC_FUNC bool bq24262_wmport_diag_on_service(struct bq24262_wmport_context *bc)
{	switch (bc->diag_state) {
		case BQ24262_WMPORT_DIAG_ACTIVE:
		case BQ24262_WMPORT_DIAG_VBUS_DOWN:
		case BQ24262_WMPORT_DIAG_EXIT:
			return true;
		default:
			/* Do nothing. */
			break;
	}
	return false;
}


/*! Test if status says polling.
    @param bc battery charger driver context.
    @pre "down bc->status_sem" or "in thread main"
    @return bool ==true: Need polling, ==false: no need polling.
*/
STATIC_FUNC bool bq24262_wmport_status_is_needed_polling(struct bq24262_wmport_context *bc)
{	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_FAULT) {
		case BQ24262_STATUS_CONTROL_FAULT_TERMAL:
		case BQ24262_STATUS_CONTROL_FAULT_OVP:
			return true;
		default:
			/* do nothing. */
			break;
	}
	if ((((bc->power_source_raw)^(bc->power_source)) & BQ24262_WMPORT_USB) != 0) {
		/* May USB VBUS voltage goes low. */
		return true;
	}
	if (bq24262_wmport_force_dcin_read(bc) == BQ24262_WMPORT_FORCE_DCIN_AUTO) {
		if ((bc->power_source_raw & BQ24262_WMPORT_DCIN) != 0) {
			return true;
		}
	}
	return false /* doesn't need polling */;
}

/*! Test if charger status shows "should do slow polling".
    @param bc battery charger driver context.
    @pre "down bc->status_sem" or "in thread main"
*/
STATIC_FUNC bool bq24262_wmport_status_is_needed_slow_polling(struct bq24262_wmport_context *bc)
{	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_FAULT) {
		case BQ24262_STATUS_CONTROL_FAULT_TIMER:
		case BQ24262_STATUS_CONTROL_FAULT_NO_BATT:
			return true;
			break;
	}

	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_STAT) {
		case BQ24262_STATUS_CONTROL_STAT_CHARGE_DONE:
			return true;
		case BQ24262_STATUS_CONTROL_STAT_CHARGE_IN_PROGRESS:
			/* On charging watch battery temperature. */
			return true;
		default:
			/* Do nothing. */
			break;
	}
	switch (bc->battery_charge) {
		case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL:
			return true;
		default:
			/* Do nothing. */
			break;
	}
	return false /* doesn't need slow polling */;
}

/*! Test if interrupt factor status tells "should do cool down".
    @param bc battery charger driver context.
    @pre "down bc->status_sem" or "in thread main"
*/
STATIC_FUNC bool bq24262_wmport_status_is_need_cool_down(struct bq24262_wmport_context *bc)
{	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_FAULT) {
		case BQ24262_STATUS_CONTROL_FAULT_BATT_TEMP:
			return true;
		case BQ24262_STATUS_CONTROL_FAULT_BATT_OVP:
			return true;
		default:
			/* Do nothing. */
			break;
	}
	switch (bc->charger_status.safety & BQ24262_SAFETY_TS_FAULT) {
		case BQ24262_SAFETY_TS_FAULT_TCOLD_HOT:
			return true;
		default:
			/* Do nothing. */
			break;
	}
	switch (bc->battery_temp_state) {
		case BQ24262_WMPORT_BATTERY_TEMP_STATE_COLD:
		case BQ24262_WMPORT_BATTERY_TEMP_STATE_HOT:
			return true;
		default:
			/* Do nothing. */
			break;
	}
	return false /* Normal operation. */;
}

/*! Test if charger status shows "should do slow polling".
    @param bc battery charger driver context.
    @pre "down bc->status_sem" or "in thread main"
*/
STATIC_FUNC unsigned long bq24262_wmport_status_charge_to_polling(struct bq24262_wmport_context *bc)
{	if (    (bc->battery_boost    !=  bc->battery_boost_prev)
	     || (bc->battery_capacity !=  bc->battery_capacity_prev)
	     || (bc->battery_temp_state != bc->battery_temp_state_prev)
	     || (bc->vindpm_state       != bc->vindpm_state_prev)
	     || (   (bc->charger_status.status ^ bc->charger_status_prev.status)
	          & (BQ24262_STATUS_CONTROL_STAT | BQ24262_STATUS_CONTROL_FAULT)
		)
	     || (   (bc->charger_status.safety ^ bc->charger_status_prev.safety)
	          & (BQ24262_SAFETY_TS_FAULT)
		)
	) {
		/*    "Battery boost(full stop) mode changed"
		   or "Battery capacity changed"
		   or "Battery temperature state changed"
		   or "Charger status updated"
		   or "Safety status updated"
		*/
		/* Check current condition now. */
		return BQ24262_WMPORT_THR_MAIN_TIMEOUT_STATUS_POLLING_MS;
	}
	if (bq24262_wmport_status_is_need_cool_down(bc)) {
		return BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_TEMP_LOHI_POLLING_MS;
	}
	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_FAULT) {
		case BQ24262_STATUS_CONTROL_FAULT_TERMAL:
		case BQ24262_STATUS_CONTROL_FAULT_OVP:
		case BQ24262_STATUS_CONTROL_FAULT_UVLO:
		case BQ24262_STATUS_CONTROL_FAULT_TIMER:
			return BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_DISCHARGE_POLLING_MS;
		default:
			/* Do nothing. */
			break;
	}

	switch (bc->battery_charge) {
		case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL:
			return BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_TEMP_NORM_POLLING_MS;
		default:
			/* Do nothing. */
			break;
	}

	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_FAULT) {
		case BQ24262_STATUS_CONTROL_FAULT_BATT_OVP:
			return BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_DISCHARGE_POLLING_MS;
		default:
			/* Do nothing. */
			break;
	}

	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_STAT) {
		case BQ24262_STATUS_CONTROL_STAT_READY:
			return BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_DISCHARGE_POLLING_MS;
		case BQ24262_STATUS_CONTROL_STAT_CHARGE_IN_PROGRESS:
			/* On charging watch battery temperature. */
			if (bc->battery_uv < bc->platform_data->battery_safe_uv) {
				return BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_LOW_POLLING_MS;
			}
			return BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_TEMP_NORM_POLLING_MS;
		case BQ24262_STATUS_CONTROL_STAT_CHARGE_DONE:
			return BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_TEMP_NORM_POLLING_MS;
		default:
			/* Do nothing. */
			break;
	}
	return BQ24262_WMPORT_THR_MAIN_TIMEOUT_BATTERY_TEMP_NORM_POLLING_MS;
}


/*! Test if status tells "stop charging".
    @param bc battery charger driver context.
    @pre "down bc->status_sem" or "in thread main"
    @return bool ==true:  Stop Charging except "no power source", "full",
                 ==false: Charging.
*/
STATIC_FUNC bool bq24262_wmport_status_is_stop_charging(struct bq24262_wmport_context *bc)
{	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_FAULT) {
		case BQ24262_STATUS_CONTROL_FAULT_TERMAL:
		case BQ24262_STATUS_CONTROL_FAULT_BATT_TEMP:
		case BQ24262_STATUS_CONTROL_FAULT_BATT_OVP:
		case BQ24262_STATUS_CONTROL_FAULT_NO_BATT:
		case BQ24262_STATUS_CONTROL_FAULT_UVLO:
			return true; /* stop charging. */
		default:
			/* do nothing. */
			break;
	}
	switch (bc->charger_status.safety & BQ24262_SAFETY_TS_FAULT) {
		case BQ24262_SAFETY_TS_FAULT_TCOLD_HOT:
			return true;
		default:
			/* Do nothing. */
			break;
	}
	switch (bc->battery_temp_state) {
		case BQ24262_WMPORT_BATTERY_TEMP_STATE_COLD:
		case BQ24262_WMPORT_BATTERY_TEMP_STATE_HOT:
			return true; /* stop charging. */
			break;
		default:
			/* do nothing. */
			break;
	}
	switch (bc->battery_charge) {
		case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG:
			return true; /* stop charging. */
		case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL:
			/* full stop mode, physically the current from/to battery is zero amps,
			   but it means "full charged".
			*/
			return false; /* "full". */
		default:
			/* Do nothig. */
			break;
	}
	return false /* "charging" or "no power source" or "full" */;
}

/*! Test if interrupt factor status tells "charged full".
    @param bc battery charger driver context.
    @pre "down bc->status_sem" or "in thread main"
    @return bool == true: fully charged,
                 == false: not fully charged.
*/
bool bq24262_wmport_status_is_charged_full(struct bq24262_wmport_context *bc)
{	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_STAT) {
		case BQ24262_STATUS_CONTROL_STAT_CHARGE_DONE:
			return true;
		case BQ24262_STATUS_CONTROL_STAT_READY:
			/* @todo ready may be following conditions.
			   + Stop boosting at full.
			   + Stop boosting at hot or cold temperature.
			*/
			switch (bc->battery_charge) {
				case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL:
					return true;
				case BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG:
					return false;
				default:
					break;
			}
		break;
	}
	return false /* NOT full. */;
}

/*! Test if interrupt factor status tells "battery connected(alive)".
    @param bc battery charger driver context.
    @pre "down bc->status_sem" or "in thread main"
*/
STATIC_FUNC bool bq24262_wmport_status_is_battery_online(struct bq24262_wmport_context *bc)
{	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_FAULT) {
		case BQ24262_STATUS_CONTROL_FAULT_NO_BATT:
			return false /* not alive, it may not happen. */;
			break;
	}
	return true /* alive. */;
}

/*! Test if interrupt factor status tells "dead/removed".
    @param bc battery charger driver context.
    @pre "down bc->status_sem" or "in thread main"
*/
STATIC_FUNC bool bq24262_wmport_status_is_battery_dead(struct bq24262_wmport_context *bc)
{	switch (bc->charger_status.status & BQ24262_STATUS_CONTROL_FAULT) {
		case BQ24262_STATUS_CONTROL_FAULT_TIMER: /* May not happen. */
		case BQ24262_STATUS_CONTROL_FAULT_NO_BATT:
			return true;
			break;
	}
	return false /* alive. */;
}

/*! "Battery thermal becomes out of safe range."?
    @param bc battery charger driver context.
    @pre "down bc->status_sem" or "in thread main"

    @return bool ==false: Not emergency, ==true: Emergency.
*/
STATIC_FUNC bool bq24262_wmport_emergency(struct bq24262_wmport_context *bc)
{	bool	ret;
	uint8_t	fault;

	ret = false;

	fault = (bc->charger_status.status) & BQ24262_STATUS_CONTROL_STAT_FAULT;
	switch (fault) {
		case BQ24262_STATUS_CONTROL_FAULT_OVP:
		case BQ24262_STATUS_CONTROL_FAULT_TERMAL:
		case BQ24262_STATUS_CONTROL_FAULT_BATT_TEMP:
		case BQ24262_STATUS_CONTROL_FAULT_TIMER:
		case BQ24262_STATUS_CONTROL_FAULT_BATT_OVP:
		case BQ24262_STATUS_CONTROL_FAULT_NO_BATT:
			ret = true;
			break;
		default:
			/* do nothing. */
			break;
	}
	switch (bc->charger_status.safety & BQ24262_SAFETY_TS_FAULT) {
		case BQ24262_SAFETY_TS_FAULT_TCOLD_HOT:
			return true;
		default:
			/* Do nothing. */
			break;
	}
	switch (bc->battery_temp_state) {
		case BQ24262_WMPORT_BATTERY_TEMP_STATE_COLD:
		case BQ24262_WMPORT_BATTERY_TEMP_STATE_HOT:
			ret = true;
			break;
		default:
			/* do nothing. */
			break;
	}
	return ret;
}

/*! the period to tick charge timer in seconds.  */
#define	BQ24262_WMPORT_CHARGE_TIMER_TICK_SECONDS	(1)

/*! get remaining time to expire charging timer.
    @param bc battery charger driver context.
    @return unsigned long remaining time to expire charging time in seconds.
*/

STATIC_FUNC unsigned long bq24262_wmport_charge_timer_remains(struct bq24262_wmport_context *bc)
{	time_t	dt;
	switch (bc->charge_timer_state) {
		case BQ24262_WMPORT_CHARGE_TIMER_WAIT_SYSTEM_RUNNING:
			/* Wait system running, tick every one seconds. */
			return BQ24262_WMPORT_CHARGE_TIMER_TICK_SECONDS; /* ticking. */

		case BQ24262_WMPORT_CHARGE_TIMER_NOT_CHARGING:
		case BQ24262_WMPORT_CHARGE_TIMER_EXPIRE:
			return TIMEOUT_INFINITY;

		case BQ24262_WMPORT_CHARGE_TIMER_CHARGING:
			dt = (bc->charge_timer_now_ts.tv_sec) - (bc->charge_timer_start_ts.tv_sec);
			if (dt <= (time_t)(bc->charge_timer_max_sec)) {
				/* timer is not expired. */
				return (time_t)(bc->charge_timer_max_sec) - dt;
			}
			/* timer expired. */
			return BQ24262_WMPORT_CHARGE_TIMER_TICK_SECONDS; /* ticking. */

		default:
			/* treat as not charging. */
			break;
	}
	return TIMEOUT_INFINITY;
}

/*! Tick charging timer.
    @param bc battery charger driver context.
    @param battery_charge one of _UP, _KEEP_EMG, _KEEP_FULL.
    @param config one of _CONFIG_BOOT .. _CONFIG_COOL_DOWN

    @pre "down bc->status_sem" or "in thread main"

    @return bool ==false: "not charging" or "continue charge", ==true: Timer expired.
*/
STATIC_FUNC bool bq24262_wmport_charge_timer_tick(struct bq24262_wmport_context *bc, int battery_charge, int config)
{	time_t	dt;

	/* getnstimeofday(&(bc->charge_timer_now_ts)); */
	/* ktime_get_ts(&(bc->charge_timer_now_ts)); */
	/* get_monotonic_boottime(&(bc->charge_timer_now_ts)); */
	getrawmonotonic(&(bc->charge_timer_now_ts));
	PRINTK_TM("%s: Get real time. now_ts=%llu.%09lu\n",
		__func__,
		(unsigned long long)(bc->charge_timer_now_ts.tv_sec),
		(unsigned long)(bc->charge_timer_now_ts.tv_nsec)
	);
	switch (bc->charge_timer_state) {
		case BQ24262_WMPORT_CHARGE_TIMER_WAIT_SYSTEM_RUNNING:
			/* wait system running. */
			if (system_state == SYSTEM_RUNNING) {
				/* will execute init.rc. */
				if (bq24262_wmport_is_charging_for_timer(battery_charge, config)) {
					/* charging. */
					bc->charge_timer_state = BQ24262_WMPORT_CHARGE_TIMER_CHARGING;
				} else {
					/* not charging. */
					bc->charge_timer_state = BQ24262_WMPORT_CHARGE_TIMER_NOT_CHARGING;
				}
				bc->charge_timer_start_ts = bc->charge_timer_now_ts; /* struct copy. */
			}
			return false; /* Temporal condition. */

		case BQ24262_WMPORT_CHARGE_TIMER_NOT_CHARGING:
			/* state not charging. */
			if (bq24262_wmport_is_charging_for_timer(battery_charge, config)) {
				/* charging. */
				bc->charge_timer_state = BQ24262_WMPORT_CHARGE_TIMER_CHARGING;
				bc->charge_timer_start_ts = bc->charge_timer_now_ts; /* struct copy. */
			}
			return false;

		case BQ24262_WMPORT_CHARGE_TIMER_CHARGING:
			/* state charging. */
			if (bq24262_wmport_is_charging_for_timer(battery_charge, config)) {
				/* charging. */
				dt = (bc->charge_timer_now_ts.tv_sec) - (bc->charge_timer_start_ts.tv_sec);
				if (dt >= (time_t)(bc->charge_timer_max_sec)) {
					/* charging too long time. */
					PRINTK_LI("%s: Expire charging timer. dt=%ld, max_sec=%ld\n",
						__func__,
						(long)dt, bc->charge_timer_max_sec
					);
					bc->charge_timer_state = BQ24262_WMPORT_CHARGE_TIMER_EXPIRE;
					/* keep start time. */
					return true;
				}
			} else {
				/* not charging. */
				bc->charge_timer_state = BQ24262_WMPORT_CHARGE_TIMER_NOT_CHARGING;
				bc->charge_timer_start_ts = bc->charge_timer_now_ts; /* struct copy. */
			}
			return false;

		case BQ24262_WMPORT_CHARGE_TIMER_EXPIRE:
			/* state expire(timeout charging). */
			if (!bq24262_wmport_is_drawing(config)) {
				/* Drawing no current from VBUS or DCIN. */
				PRINTK_LI("%s: Reset charging timer. dt=%ld\n", __func__,
					(long) ((bc->charge_timer_now_ts.tv_sec) - (bc->charge_timer_start_ts.tv_sec))
				);
				bc->charge_timer_state = BQ24262_WMPORT_CHARGE_TIMER_NOT_CHARGING;
				bc->charge_timer_start_ts = bc->charge_timer_now_ts; /* struct copy. */
				return false;
			}
			return true;

		default:
			pr_err("%s: Unknown state. charge_timer_state=%d\n", __func__, bc->charge_timer_state);
			/* for safety, stop charging. */
			bc->charge_timer_state = BQ24262_WMPORT_CHARGE_TIMER_EXPIRE;
			bc->charge_timer_start_ts = bc->charge_timer_now_ts; /* struct copy. */
			return true;
	}
	return false;
}


/*! unregister power supply rate limited notifier.
    @param psrl points array of power_supply_rate_limited
    @param n    the number of elements in array.
*/
static void power_supply_rl_array_unregister(struct power_supply_rate_limited *psrl, int n)
{	int	i;

	i = 0;
	while (i < n) {
		if (test_bit(POWER_SUPPLY_RL_INIT, &(psrl->flags))) {
			/* Registered power supply. */
			power_supply_unregister(&(psrl->ps));
		}
		psrl++;
		i++;
	}
}

/*! register power supply rate limited notifier.
    @param psrl points array of power_supply_rate_limited.
    @param n    the number of elements in array.
    @param now  current time in jiffies.
    @param pdev points platform_device passed to _probe().
    @return int ==0: Success, <0: Negative errno number.
*/
static int power_supply_rl_array_register(
	struct power_supply_rate_limited *psrl,
	int n,
	unsigned long now,
	struct device *dev
)
{	int	i;
	int	ret;
	int	result;

	result = 0;
	i = 0;
	while (i < n) {
		struct power_supply	*ps;

		spin_lock_init(&(psrl->state_lock));
		clear_bit(POWER_SUPPLY_RL_INIT, &(psrl->flags));
		clear_bit(POWER_SUPPLY_RL_PEND, &(psrl->flags));
		psrl->jiffies_changed = now;

		ps = &(psrl->ps);
		ret = power_supply_register(dev, ps);
		if (ret) {
			dev_err(dev, "Failed to register power_supply. i=%d(%s), ret=%d\n",
				i, ps->name, ret
			);
			result = ret;
		} else {
			/* Success register. */
			set_bit(POWER_SUPPLY_RL_INIT, &(psrl->flags));
		}
		psrl++;
		i++;
	}
	return result;
}


#define	BQ24262_WMPORT_POWER_SUPPLY_CHANGED_RATE_LIMIT_MS	(500)

/*! Trigger "changed", and will be "pending".
    @param psrl points one power_supply_rate_limited
*/
static void power_supply_rl_changed(struct power_supply_rate_limited *psrl)
{	unsigned long	flags;

	spin_lock_irqsave(&(psrl->state_lock), flags);
	set_bit(POWER_SUPPLY_RL_PEND, &(psrl->flags));
	spin_unlock_irqrestore(&(psrl->state_lock), flags);
}

/*! Test if is there pending notification.
    @param bc driver context.
    @param psrl points one power_supply_rate_limited
    @return bool ==false: Not pending, !=true: Pending.
    @note Don't use this fuction to notify "changed".
          If you want to notify "changed", use power_supply_rl_is_expired().
          Use this function to test if is there pending notification only.
          Remember that when this function returns "pending", but will
          be "not pending" by simultaneously calling power_supply_rl_is_expired().
*/
STATIC_FUNC bool power_supply_rl_is_pending(struct bq24262_wmport_context *bc, struct power_supply_rate_limited *psrl)
{	unsigned long	flags;
	int		ret;

	spin_lock_irqsave(&(psrl->state_lock), flags);
	ret = test_bit(POWER_SUPPLY_RL_PEND, &(psrl->flags));
	if (test_bit(POWER_SUPPLY_RL_SUSPEND_MASK, &(psrl->flags))) {
		if (test_bit(BQ24262_WMPORT_SUSPEND, &(bc->flags))) {
			/*     ("mask notify while suspend/resume.")
			   or  ("while supend/resume")
			*/
			ret = false;
		}
	}
	spin_unlock_irqrestore(&(psrl->state_lock), flags);
	return ret;
}

/*! Check and expire pending timer {Battery, USB, AC}.
    @param bc driver context.
    @param psrl points one power_supply_rate_limited
    @param now  current time in jiffies.
    @return unsigned ==0: Do not notify(not expired). ==1: Notify(expired).
    @note Because we want simultaneously notify {battery, usb, ac} "changed",
          we use standard time "now" in power supply main thread.
*/
STATIC_FUNC unsigned power_supply_rl_is_expired(struct bq24262_wmport_context *bc, struct power_supply_rate_limited *psrl)
{	unsigned	ret;
	unsigned long	expire;
	unsigned long	flags;
	unsigned long	now;

	ret = 0;
	spin_lock_irqsave(&(psrl->state_lock), flags);
	if (test_bit(POWER_SUPPLY_RL_PEND, &(psrl->flags))) {
		if (   (test_bit(POWER_SUPPLY_RL_SUSPEND_MASK, &(psrl->flags)) == 0)
		    || (test_bit(BQ24262_WMPORT_SUSPEND, &(bc->flags)) == 0)
		) {
			/*     (not "mask notify while suspend/resume.")
			   or  (not "while supend/resume")
			*/
			expire =  psrl->jiffies_changed + msecs_to_jiffies(BQ24262_WMPORT_POWER_SUPPLY_CHANGED_RATE_LIMIT_MS);
			now = bc->jiffies_now;

			if (   time_after(now, expire)
			    || time_before(now, psrl->jiffies_changed)
			) {
				/*    "spend enough time after last notified." */
				/* or "jiffies is round-tripped." */
				psrl->jiffies_changed = now;
				clear_bit(POWER_SUPPLY_RL_PEND, &(psrl->flags));
				ret = 1;
			}
		}
	}
	spin_unlock_irqrestore(&(psrl->state_lock), flags);
	return ret;
}
/*! Test if there are(is) pending notificatin "changed" in power supply rate limit array.
    @param bc driver context.
    @param psrl points array of power_supply_rate_limited
    @param n    the number of elements in array.
    @return bool ==false: there is no pending notification. ==true: there are(is) pending notification.
*/
STATIC_FUNC bool power_supply_rl_array_are_pending(struct bq24262_wmport_context *bc, struct power_supply_rate_limited *psrl, int n)
{	int		i;
	i = 0;
	while (i < n) {
		if (power_supply_rl_is_pending(bc, psrl)) {
			return true;
		}
		psrl++;
		i++;
	}
	return false;
}

/*! Test expired, updated pending state, and notify "chagned".
    @param bc driver context.
    @param psrl0 points array of power_supply_rate_limited
    @param n     the number of elements in array.
    @param now   current time in jiffies.
*/
STATIC_FUNC void power_supply_rl_array_raise_event(struct bq24262_wmport_context *bc, struct power_supply_rate_limited *psrl0, int n)
{	struct power_supply_rate_limited *psrl;
	unsigned	trig;
	int		i;

	psrl = psrl0;
	trig = 0;
	i = 0;
	while (i <  n) {
		trig |= (power_supply_rl_is_expired(bc, psrl)) << i;
		psrl++;
		i++;
	}
	if (trig) {
		PRINTK_HV("%s: Notify changed. trig=0x%x\n",
			__func__,
			trig
		);
	}
	psrl = psrl0;
	i = 0;
	while (i < n) {
		if ((trig & (1 << i)) != 0) {
			power_supply_changed(&(psrl->ps));
		}
		psrl++;
		i++;
	}
}

/*! Supported battery property */
static enum power_supply_property bq24262_wmport_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
};

/*! Supported VBUS(USB) power source property. */
static enum power_supply_property bq24262_wmport_usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

/*! Supported DCIN power source property. */
static enum power_supply_property bq24262_wmport_dc_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};


/*! Test and clear "is evented to ICX platform power source thread".
    @param bc battery charger context.
*/
static int bq24262_wmport_thr_main_is_waken(struct bq24262_wmport_context *bc)
{	if (kthread_should_stop()) {
		return 1 /* should stop. */;
	}
	return test_and_clear_bit(BQ24262_WMPORT_THR_MAIN_WAKE, &(bc->flags));
}


/*! Detect interrupt status change, and check if we need to notify battery object.
    @param bc battery charger context.
    @return bool ==false: Do not need notify to battery object, ==true: Need notify to battery object.
*/
STATIC_FUNC bool bq24262_wmport_thr_main_battery_status_notify(struct bq24262_wmport_context *bc)
{	uint8_t	status_diff;


	status_diff = bc->charger_status.status ^ bc->charger_status_prev.status;
	if ((status_diff & BQ24262_STATUS_CONTROL_STAT) != 0) {
		PRINTK_HV("%s: Status updated. status=0x%.2x, status_prev=0x%.2x\n",
			__func__,
			bc->charger_status.status,
			bc->charger_status_prev.status
		);
		return true;
	}
	if (bc->battery_capacity != bc->battery_capacity_notify_prev) {
		PRINTK_HV("%s: Capacity updated. capacity=%d, capacity_prev=%d\n",
			__func__,
			bc->battery_capacity,
			bc->battery_capacity_notify_prev
		);
		bc->battery_capacity_notify_prev = bc->battery_capacity;
		return true;
	}
	if (bc->battery_temp_state != bc->battery_temp_state_notify_prev) {
		PRINTK_HV("%s: Temp updated. temp_state=%d, temp_state_prev=%d\n",
			__func__,
			bc->battery_temp_state,
			bc->battery_temp_state_notify_prev
		);
		bc->battery_temp_state_notify_prev = bc->battery_temp_state;
		return true;
	}
	return false;
}

/*! Catch synchronize request.
    @param bc battery charger context.
    @return int ==0: Success, <0: Failed, negative errno number.
*/
STATIC_FUNC int bq24262_wmport_thr_main_catch_sync_req(struct bq24262_wmport_context *bc)
{
	if (test_and_clear_bit(BQ24262_WMPORT_THR_MAIN_RESUME_REQ, &(bc->flags))) {
		/* Requested resume this thread. */
		clear_bit(BQ24262_WMPORT_SUSPEND, &(bc->flags));
		bc->resume_req = 1;
	}

	if (test_and_clear_bit(BQ24262_WMPORT_THR_MAIN_SUSPEND_REQ, &(bc->flags))) {
		/* Requested suspend this thread. */
		bc->battery_uv_suspend = bq24262_wmport_vbat_read(bc);
		bc->battery_temp_raw_suspend = bq24262_wmport_temp_read(bc);
		set_bit(BQ24262_WMPORT_SUSPEND, &(bc->flags));
		bc->suspend_req = 1;
	}

	if (test_and_clear_bit(BQ24262_WMPORT_THR_MAIN_SHUTDOWN_REQ, &(bc->flags))) {
		/* Requested shutdown this thread. */
		bc->shutdown_req = 1;
	}

	if (test_and_clear_bit(BQ24262_WMPORT_THR_MAIN_DIAG_REQ, &(bc->flags))) {
		/* Requested diag this thread. */
		bc->diag_req = 1;
	}
	return 0 /* No error. */;
}

/*! Acknowledge synchronize request.
    @param bc battery charger context.
    @return int ==0: Success, <0: Failed, negative errno number.
*/
STATIC_FUNC int bq24262_wmport_thr_main_sync_ack(struct bq24262_wmport_context *bc)
{
	if (bc->resume_req) {
		/* Requested sync resume process with this thread. */
		bc->resume_req = 0;
		set_bit(BQ24262_WMPORT_THR_MAIN_RESUME_ACK, &(bc->flags));
		wake_up_interruptible(&(bc->event_hs));
	}
	if (bc->suspend_req) {
		/* Requested sync suspend process with this thread. */
		bc->suspend_req = 0;
		set_bit(BQ24262_WMPORT_THR_MAIN_SUSPEND_ACK, &(bc->flags));
		wake_up_interruptible(&(bc->event_hs));
	}
	if (bc->shutdown_req) {
		/* Requested sync shutdown process with this thread. */
		bc->shutdown_req = 0;
		set_bit(BQ24262_WMPORT_THR_MAIN_SHUTDOWN_ACK, &(bc->flags));
		set_bit(BQ24262_WMPORT_THR_MAIN_EXIT, &(bc->flags));
		wake_up_interruptible(&(bc->event_hs));
	}
	if (bc->diag_req) {
		/* Requested sync diag process with this thread. */
		bc->diag_req = 0;
		set_bit(BQ24262_WMPORT_THR_MAIN_DIAG_ACK, &(bc->flags));
		wake_up_interruptible(&(bc->event_diag));
	}
	return 0 /* No error. */;
}

/*! ICX platform power source thread.
    @param data as battery charger context.
    @note this thread setup BQ24262 configuration according to
          USB, DCIN, and USB charger type information from USB 
          OTG/gadget driver.
    @return int ==0: Success, !=0: Failed.
*/
int bq24262_wmport_thr_main_function(void *data)
{	struct bq24262_wmport_context	*bc;
	unsigned int			power_source_diff;
	unsigned			usb_bcdet;
	int				usb_state;
	long				battery_uv;
	int				new_config;
	int				charge;
	int				new_charge;
	int				vindpm_config;
	int				new_vindpm_config;
	int				ret;
	bool				unexpected_init;
	bool				do_config;

	unsigned long			flags;
	int				poll_count;
	unsigned long			timeout;

	const struct bq24262_wmport_configs *config_ptr;
	const struct bq24262_wmport_configs *new_config_ptr;

	bc = data;
	poll_count = 0;

	PRINTK_LI("%s: Thread main started.\n", __func__);
	set_bit(BQ24262_WMPORT_THR_MAIN_START ,&(bc->flags));

	/* Notify thread started. */
	wake_up_interruptible(&(bc->event_hs));
	while (test_bit(BQ24262_WMPORT_THR_MAIN_EXIT, &(bc->flags)) == 0) {
		bool	safe_suspend;
		bool	diag_on;

		/* Loop until "thread exited" flag turn into set(1). */
		timeout = TIMEOUT_INFINITY;
		down(&(bc->usb_sem));
		usb_state = bc->usb_state;
		safe_suspend = test_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags));
		up(&(bc->usb_sem));
		spin_lock_irqsave(&(bc->lock), flags);
		diag_on = bq24262_wmport_diag_on_service(bc);
		spin_unlock_irqrestore(&(bc->lock), flags);
		if (   (poll_count > 0)
		    || ((!bq24262_wmport_is_recovery_boot()) && (safe_suspend == false))
		    || (diag_on)
		    || (bq24262_wmport_status_is_needed_polling(bc))
		) {
			/*    "Under polling",
			   or ("Normal boot" && "Not safe to stop draw from USB VBUS.")
			   or "Service diag,"
			   or "need polling."
			*/
			/* Need status polling. */
			PRINTK_XP("%s: Polling. timeout=%lu, flags=0x%lx, diag_state=0x%d, config=%d, charge=%d\n",
				__func__,
				BQ24262_WMPORT_THR_MAIN_TIMEOUT_STATUS_POLLING_MS,
				bc->flags,
				bc->diag_state,
				bc->config,
				bc->battery_charge
			);
			timeout = min(timeout, BQ24262_WMPORT_THR_MAIN_TIMEOUT_STATUS_POLLING_MS);
		}

		if (power_supply_rl_array_are_pending(bc, &(bc->power_supply_rl[0]), BQ24262_WMPORT_PSRL_NUM)) {
			PRINTK_XP("%s.PSPEND: Polling. timeout=%lu\n",
				__func__,
				BQ24262_WMPORT_THR_MAIN_TIMEOUT_CHANGED_PEND_MS
			);
			timeout = min(timeout, BQ24262_WMPORT_THR_MAIN_TIMEOUT_CHANGED_PEND_MS);
		}
		switch (usb_state) {
			case BQ24262_WMPORT_USB_UNKNOWN:
			case BQ24262_WMPORT_USB_LBB_DRAW:
				/* After power on. */
				/* To stop drawing 500mA from suspended port,
				   check unconfigured, configured, or suspended.
				*/
				if (!bq24262_wmport_is_recovery_boot()) {
					/* normal boot. */
					PRINTK_XP("%s.BOOT: Polling. timeout=%lu, usb_state=%d\n",
						__func__,
						BQ24262_WMPORT_THR_MAIN_TIMEOUT_UNKNOWN_POLLING_MS,
						usb_state
					);
					timeout = min(timeout, BQ24262_WMPORT_THR_MAIN_TIMEOUT_UNKNOWN_POLLING_MS);
				} else {
					/* Recovery boot. */
					/* Do nothing. */
				}
				break;
			default:
				/* Do nothing. */
				break;
		}

		if (timeout == TIMEOUT_INFINITY) {
			/* We will wait until somthing happen. */
			/* So we are not busy, read status and check BQ24262 health. */
			bq24262_wmport_status_read(bc, false);
			timeout = bq24262_wmport_status_charge_to_polling(bc);
			PRINTK_XP("%s.INF: Polling. timeout=%lu\n", __func__, timeout);
		}
		/* always do */ {
			/* see charge timer. */
			unsigned long	remain;
			remain = bq24262_wmport_charge_timer_remains(bc);
			if (remain <= (((unsigned long)LONG_MAX) / 1000)) {
				timeout = min(timeout, (remain * 1000 /* scale seconds to milli seconds*/));
			}
		}
		/* Wait event with timeout. */
		ret = wait_event_interruptible_timeout(
			bc->event_main,
			bq24262_wmport_thr_main_is_waken(bc),
			msecs_to_jiffies(timeout)
		);
		if (poll_count <= 0) {
			/* End polling */
			poll_count = 0;
		} else {
			/* Continue polling. */
			poll_count--;
		}
		if (ret < 0) {
			/* returns error. */
			switch (ret) {
				case -ERESTARTSYS:
					/* Signaled, see __wait_event_interruptible_timeout().  */
					pr_err("%s: Wait event signaled. ret=%d.\n", __func__, ret);
					break;
				default:
					pr_err("%s: Unexpected wait with timeout error. ret=%d.\n", __func__, ret);
					set_bit(BQ24262_WMPORT_THR_MAIN_EXIT, &(bc->flags));
					break;
			}
		}
		/* shot jiffies. */
		bc->jiffies_now = jiffies;

		/* always do */ {
			unsigned long	now;

			now = bc->jiffies_now;
			if (time_after(now, bc->jiffies_thr_main_wake_trace + msecs_to_jiffies(BQ24262_WMPORT_THR_MAIN_TRACE_PERIOD_MS))) {
				unsigned long		jiffies_diff;
				unsigned long long	ms_diff;
				unsigned long		wake_count;
				unsigned long		wake_diff;

				jiffies_diff = now - bc->jiffies_thr_main_wake_trace;
				ms_diff = jiffies_to_msecs(jiffies_diff);
				wake_count = bc->thr_main_wake_count;
				wake_diff = wake_count - bc->thr_main_wake_count_trace;
				if (ms_diff <= (wake_diff * (unsigned long long)BQ24262_WMPORT_THR_MAIN_TRACE_RATIO)) {
					pr_err("%s: Thread main wakes High frequently. jiffies_diff=%lu, wake_diff=%lu\n",
						__func__,
						jiffies_diff, wake_diff
					);
				}
				bc->jiffies_thr_main_wake_trace = now;
				bc->thr_main_wake_count_trace = wake_count;
			}
		}

		if (kthread_should_stop()) {
			/* Some one stop me. */
			set_bit(BQ24262_WMPORT_THR_MAIN_EXIT, &(bc->flags));
		}

		bq24262_wmport_thr_main_catch_sync_req(bc);

		/* Let's do work. */
		do_config = false;
		bq24262_wmport_status_read(bc, true);
		if (bq24262_wmport_debug&DEBUG_FLAG_STATUS) {
			bq24262_wmport_status_show(bc);
		}
		down(&(bc->status_sem));
		down(&(bc->usb_sem));
		usb_bcdet = bc->usb_bcdet;
		usb_state = bc->usb_state;
		up(&(bc->usb_sem));
		new_config = bc->config;
		config_ptr = bc->config_ptr;
		battery_uv = bc->battery_uv;
		if (bc->battery_safe_count < BQ24262_WMPORT_BATTERY_CHARGED_ENOUGH_COUNT) {
			/* We didn't see enough times that the battery voltage is safe to work. */
			if (battery_uv >= bc->platform_data->battery_safe_absolutely_uv) {
				/* Absolutely battery is charged enough. */
				bc->battery_safe_count = BQ24262_WMPORT_BATTERY_CHARGED_ENOUGH_COUNT;
				PRINTK_LI("%s: Battery voltage is absolutely high enough. battery_uv=%ld\n",__func__, battery_uv);
			} else {
				/* Need watch battery voltage. */
				if (battery_uv >= bc->platform_data->battery_safe_uv) {
					/* Battery is charged, but low voltage. */
					bc->battery_safe_count++;
					if (bc->battery_safe_count >= BQ24262_WMPORT_BATTERY_CHARGED_ENOUGH_COUNT) {
						PRINTK_LI("%s: Battery voltage is high enough. battery_uv=%ld\n",__func__, battery_uv);
					}
				} else {
					bc->battery_safe_count = 0;
				}
			}
		}
		if (bc->fully_weakly != bc->fully_weakly_prev) {
			/* Update fully or softy charge mode. */
			bc->fully_weakly_prev = bc->fully_weakly;
			do_config = true;
		}
		charge = bc->battery_charge;
		new_charge = BQ24262_WMPORT_BATTERY_CHARGE_UP;
		switch (bc->battery_boost) {
			case BQ24262_WMPORT_BATTERY_BOOST_STOP:
				/* Boost stop (Full stop) mode. */
				new_charge = BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL;
			default:
				/* do nothing. */
				break;
		}
		if (bq24262_wmport_emergency(bc)) {
			new_charge = BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG;
		}
		up(&(bc->status_sem));

		PRINTK_TM("%s: Waken thread. ps=0x%x, ps_raw=0x%x, stat=0x%.2x, usb_state=%d\n",
			__func__,
			bc->power_source, bc->power_source_raw, bc->charger_status.status, usb_state
		);
		down(&(bc->usb_sem));
		if (   (test_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags)) == 0)
		    && (!bq24262_wmport_is_recovery_boot())
		) {
			/*     "Not safe to stop drawing current from USB VBUS."
			   and "Normal boot"
			*/
			unsigned long now;

			if (bc->battery_safe_count >= BQ24262_WMPORT_BATTERY_CHARGED_ENOUGH_COUNT) {
				/* Battery charge is enough. */
				PRINTK_LI("%s: Charged enough, safe to stop draw current from USB.\n",__func__);
				set_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags));
			}
			now = bc->jiffies_now;
			if (bc->jiffies_sys_running == 0) {
				if (system_state == SYSTEM_RUNNING) {
					/* Will start initrd. */
					bc->jiffies_sys_running = now;
					bc->jiffies_sys_running_expire =
						now
						+ msecs_to_jiffies(
							BQ24262_WMPORT_THR_MAIN_TIMEOUT_UNKNOWN_RUNNING_MS
						);
				}
			}
			if (   (time_after(now, bc->jiffies_probe_expire))
			    || (test_bit(BQ24262_WMPORT_SUSPEND,  &(bc->flags)))
			    || (test_bit(BQ24262_WMPORT_SHUTDOWN, &(bc->flags)))
			    || (    (system_state == SYSTEM_RUNNING)
			         && (time_after(now, bc->jiffies_sys_running_expire))
			       )
			) {
				/*    "Timeout, no more drawing current from USB port. "
				   or "Go suspend, stop drawing current in suspending."
				   or "Go shutdown, stop drawing current in power off."
				   or ( "start init, may android launcher ready." )
				*/
				PRINTK_LI("%s: Timeout, safe to stop draw current from USB.\n",__func__);
				switch (bc->usb_state) {
					case BQ24262_WMPORT_USB_LBB_DRAW:
					case BQ24262_WMPORT_USB_UNKNOWN:
						/* Temporal SUSPEND or unknown state, revert to SUSPEND state. */
						bc->usb_state = BQ24262_WMPORT_USB_SUSPENDED;
						break;
					default:
						/* do nothing. */
						break;
				}
				set_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags));
			}
		}
		switch (bc->power_source) {
			case 0:
				/* No power source. */
				/* Turn input to HI-Z. */
				new_config = BQ24262_WMPORT_CONFIG_NO_POWER;
				break;
			case BQ24262_WMPORT_USB:
				/* USB power sourced. */
				if (!bq24262_wmport_is_recovery_boot()) {
					/* Normal boot. */
					switch (usb_bcdet) {
						case ICX_CHARGER_CDP:
							new_config = BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_900;
							break;
						case ICX_CHARGER_DCP:
							new_config = BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_900;
							break;
						case ICX_CHARGER_APL_0R5:
						case ICX_CHARGER_APL_1R0:
						case ICX_CHARGER_APL_2R1:
							/* All apple adaptors. */
							new_config = BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_500;
							break;
						case ICX_CHARGER_AC_S508U:
							new_config = BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_500;
							break;
						case ICX_CHARGER_MISC_OPEN:
						case ICX_CHARGER_MISC_XXX:
							/* All handmade or broken adaptors. */
							new_config = BQ24262_WMPORT_CONFIG_NO_POWER;
							break;
						case ICX_CHARGER_UNKNOWN:
							/* Could not identify USB adaptor. */
							if (test_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags))) {
								new_config = BQ24262_WMPORT_CONFIG_USB_UNKNOWN;
							} else {
								new_config = BQ24262_WMPORT_CONFIG_USB_CONFIG;
							}
							break;
						case ICX_CHARGER_STD:
							/* Standard port. */
							switch (usb_state) {
								case BQ24262_WMPORT_USB_SUSPENDED:
									if (bc->usb_sus_mode == BQ24262_WMPORT_USB_SUS_DRAW)
										new_config = BQ24262_WMPORT_CONFIG_USB_SUS_DRAW;
									else
										new_config = BQ24262_WMPORT_CONFIG_USB_SUSPENDED;
									break;
								case BQ24262_WMPORT_USB_UNCONFIG:
									new_config = BQ24262_WMPORT_CONFIG_USB_UNCONFIG;
									break;
								case BQ24262_WMPORT_USB_CONFIG:
									new_config = BQ24262_WMPORT_CONFIG_USB_CONFIG;
									break;
								case BQ24262_WMPORT_USB_CONFIG_LOW:
									new_config = BQ24262_WMPORT_CONFIG_USB_CONFIG_LOW;
									break;
								case BQ24262_WMPORT_USB_LBB_DRAW:
									new_config = BQ24262_WMPORT_CONFIG_USB_CONFIG;
									break;
								case BQ24262_WMPORT_USB_UNKNOWN:
									if (test_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags))) {
										new_config = BQ24262_WMPORT_CONFIG_USB_UNKNOWN;
									} else {
										new_config = BQ24262_WMPORT_CONFIG_USB_CONFIG;
									}
									break;
							}
							if (bc->usb_test_mode){
								new_config = BQ24262_WMPORT_CONFIG_USB_SUSPENDED;
							}
							if (test_bit(BQ24262_WMPORT_USB_GADGET_ENABLE, &(bc->flags)) == 0) {
								/* Disabled gadget function. */
								new_config = BQ24262_WMPORT_CONFIG_USB_UNKNOWN;
							}
							break;
						default:
							pr_err("%s: Unexpected USB-AC charger. usb_bcdet=%u.\n", __func__, usb_bcdet);
							new_config = BQ24262_WMPORT_CONFIG_NO_POWER;
							break;
					}
				} else {
					/* Recovery Kernel, ignore suspendig, keep charging from USB port. */
					new_config = BQ24262_WMPORT_CONFIG_USB_RECOVERY;
				}
				break;
			case BQ24262_WMPORT_DCIN:
				/* DCIN power sourced. */
				new_config = BQ24262_WMPORT_CONFIG_DCIN;
				break;
			case BQ24262_WMPORT_USB | BQ24262_WMPORT_DCIN:
				/* USB and DCIN power sourced. */
				new_config = BQ24262_WMPORT_CONFIG_DCIN;
				break;
			default:
				/* Unexpected. */
				pr_err("%s: Unexpected power source condition. ps=0x%x\n",
					__func__,
					bc->power_source
				);
				break;
		}
		up(&(bc->usb_sem));

		if (bq24262_wmport_status_is_need_cool_down(bc)) {
			/* Need Cool down. */
			if (new_config != BQ24262_WMPORT_CONFIG_NO_POWER) {
				new_config = BQ24262_WMPORT_CONFIG_COOL_DOWN;
			}
		}

		power_source_diff = bc->power_source ^ bc->power_source_prev;
		if (power_source_diff) {
			/* Changed power source. */
			if (   ((bc->power_source) & ~(bc->power_source_prev))
			     & (BQ24262_WMPORT_USB | BQ24262_WMPORT_DCIN)
			   ) {/* Turn poer source on. */
				set_bit(BQ24262_WMPORT_WAKE_SOURCE_CHANGED, &(bc->flags));
			}
		}

		if (( power_source_diff & (BQ24262_WMPORT_USB | BQ24262_WMPORT_DUMMY)) != 0) {
			PRINTK_LI("%s: USB power source changed.\n",__func__);
			power_supply_rl_changed(&(bc->power_supply_rl[BQ24262_WMPORT_PSRL_USB]));
		}

		if (( power_source_diff & (BQ24262_WMPORT_DCIN | BQ24262_WMPORT_DUMMY)) != 0) {
			PRINTK_LI("%s: DCIN power source changed.\n", __func__);
			power_supply_rl_changed(&(bc->power_supply_rl[BQ24262_WMPORT_PSRL_DC]));
		}

		if (bc->resume_req) {
			/* Requested resume. */
			/* @note This thread is activated before _resume() functions are called.
			*/
			if (bc->power_source != 0) {
				/* USB or DCIN power is/are sourced. */
				if (test_and_clear_bit(BQ24262_WMPORT_WAKE_SOURCE_CHANGED, &(bc->flags))) {
					/* Detect power is(are) sourced to USB VBUS or DCIN while suspending. */
					/* Keep waking, while resume process. */
					PRINTK_WL("%s.S: Wake lock with timeout. ms=%d\n",
						__func__,
						BQ24262_WMPORT_RESUME_WAKE_LOCK_TURN_ON_TIME_MS
					);
					wake_lock_timeout(&(bc->wake_lock_resume),
						msecs_to_jiffies(BQ24262_WMPORT_RESUME_WAKE_LOCK_TURN_ON_TIME_MS)
					);
				} else {
					/* Keep sourceing power to USB VBUS or DCIN. */
					PRINTK_WL("%s.K: Wake lock with timeout. ms=%d\n",
						__func__,
						BQ24262_WMPORT_RESUME_WAKE_LOCK_KEEP_TIME_MS
					);
					wake_lock_timeout(&(bc->wake_lock_resume),
						msecs_to_jiffies(BQ24262_WMPORT_RESUME_WAKE_LOCK_KEEP_TIME_MS)
					);
				}
			}
		}

		spin_lock_irqsave(&(bc->lock), flags);
		switch (bc->diag_state) {
			case BQ24262_WMPORT_DIAG_ACTIVE:
			case BQ24262_WMPORT_DIAG_VBUS_DOWN:
				/* In diag mode, load diag values. */
				new_config = bc->diag_config;
				new_charge = bc->diag_battery_charge;
				break;
		}

		switch (bc->diag_state) {
			case BQ24262_WMPORT_DIAG_OFF:
				/* Not diag mode, do nothing. */
				break;
			case BQ24262_WMPORT_DIAG_ACTIVE:
				/* Diag in active. */
				if (bc->power_source == 0) {
					/* Lost power source, start diag off timer. */
					bc->diag_state = BQ24262_WMPORT_DIAG_VBUS_DOWN;
					bc->diag_jiffies_to_off = bc->jiffies_now
						+ msecs_to_jiffies(BQ24262_WMPORT_DIAG_VBUS_DOWN_OFF_MS);
				}
				break;
			case BQ24262_WMPORT_DIAG_VBUS_DOWN:
				/* VBUS down, diag will be exit. */
				if (bc->power_source != 0) {
					/* Sourced power, return diag state to active. */
					bc->diag_state = BQ24262_WMPORT_DIAG_ACTIVE;
				}
				if (time_after(bc->jiffies_now, bc->diag_jiffies_to_off)) {
					/* Diag mode expired. */
					/* In state _EXIT, we go back to normal operation. */
					bc->diag_state = BQ24262_WMPORT_DIAG_EXIT;
				}
				break;
			case BQ24262_WMPORT_DIAG_EXIT:
				/* Done diag mode, go back to normal operation. */
				PRINTK_LI("%s: DIAG: Exit diag mode.\n",__func__);
				bc->diag_state = BQ24262_WMPORT_DIAG_OFF;
				break;
			default:
				/* Unknown diag state, exit diag state. */
				PRINTK_LI("%s: DIAG: Unknown state back to OFF.\n",__func__);
				bc->diag_state = BQ24262_WMPORT_DIAG_OFF;
				break;
		}
		spin_unlock_irqrestore(&(bc->lock), flags);
		down(&(bc->status_sem));
		if (bq24262_wmport_charge_timer_tick(bc, new_charge, new_config)) {
			/* Charging too long. */
			new_charge = BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG;
		}
		vindpm_config = bc->vindpm_config;
		new_vindpm_config = bc->vindpm_state;
		up(&(bc->status_sem));
		new_config_ptr = bq24262_wmport_configs_table[new_config];
		unexpected_init = bq24262_wmport_is_initialized_settings(bc);
		do_config =    do_config
			    || unexpected_init
			    || (config_ptr != new_config_ptr)
			    || (charge != new_charge)
			    || (    (vindpm_config != new_vindpm_config) 
			         && (bq24262_wmport_is_drawing(new_config))
			       )
			    || (bc->shutdown_req)
		;
		if (do_config) {
			/* Need re-configure. */
			if ((unexpected_init) && (bq24262_wmport_debug&DEBUG_FLAG_KEEP_CONFIG)) {
				bq24262_wmport_configs_show(bc);
			}
			PRINTK_LI("%s: Config ps. new_config=%d(%s), new_charge=%d, new_vindpm=%d, ps=0x%x, ue_i=%d\n",
				__func__,
				new_config, bq24262_wmport_config_names[new_config],
				new_charge, new_vindpm_config,
				bc->power_source,
				unexpected_init
			);
			/* Update configuration. */
			bq24262_wmport_configs_write(bc, new_config_ptr, new_charge, new_vindpm_config);
			if (bq24262_wmport_debug&DEBUG_FLAG_CONFIG) {
				bq24262_wmport_configs_show(bc);
			}
			/* If power is sourced, activate GPADC.
			   @toto I may need to implement.
			*/
			/* To keep tracking with true conditions,
			   sense again, check what happens,
			   and determine which configuration is better.
			*/
			poll_count = BQ24262_WMPORT_THR_MAIN_TIMEOUT_STATUS_POLLING_REPEATS;
			/* Update saved current config. */
			down(&(bc->status_sem));
			/* Refrect configuration to charge status. */
			bq24262_wmport_configs_refrect_charger_ps_status(bc, new_config_ptr, new_charge);
			bc->config =             new_config;
			bc->config_ptr =         new_config_ptr;
			bc->battery_charge =     new_charge;
			bc->vindpm_config =      new_vindpm_config;
			up(&(bc->status_sem));
			/* @note should notify "changed" after update config.
			         The message receivers may refer config.
			*/
			if (   (config_ptr == NULL)
			    || (   ((config_ptr->flags) ^ (new_config_ptr->flags))
			         & (BQ24262_WMPORT_CONFIGS_SUSPEND)
			       )
			    || (   ((config_ptr->control) ^ (new_config_ptr->control))
			         & (BQ24262_CONTROL_HZ_MODE)
			       )
			    || (charge != new_charge)
			) {
				/* Charge drawing power source. */
				power_supply_rl_changed(&(bc->power_supply_rl[BQ24262_WMPORT_PSRL_USB]));
				power_supply_rl_changed(&(bc->power_supply_rl[BQ24262_WMPORT_PSRL_DC]));
			}
			power_supply_rl_changed(&(bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT]));
		} else {
			PRINTK_TM("%s: Keep power source config. new_config=%d, new_charge=%d\n",
				__func__,
				new_config,
				new_charge
			);
			if (bq24262_wmport_debug&DEBUG_FLAG_KEEP_CONFIG) {
				bq24262_wmport_configs_show(bc);
			}
			/* Update saved current config. */
			down(&(bc->status_sem));
			/* Refrect configuration to charge status. */
			bq24262_wmport_configs_refrect_charger_ps_status(bc, new_config_ptr, new_charge);
			bc->config =         new_config;
			/* Do not update config_ptr */
			/* Do not update battery_charge. */
			/* Do not update vindpm_config. */
			up(&(bc->status_sem));
		}

		bq24262_wmport_wake_lock_refrect(bc);

		if (bq24262_wmport_thr_main_battery_status_notify(bc)) {
			power_supply_rl_changed(&(bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT]));
		}

		power_source_diff = bc->power_source_raw ^ bc->power_source_usb_conn;
		if (power_source_diff & (BQ24262_WMPORT_USB | BQ24262_WMPORT_DUMMY)) {
			/* Updated VBUS power source status. */
			/* @todo Call connect/disconnect usb */
			bc->power_source_usb_conn = bc->power_source_raw;
		}
		power_supply_rl_array_raise_event(bc, &(bc->power_supply_rl[0]), BQ24262_WMPORT_PSRL_NUM);
		bq24262_wmport_thr_main_sync_ack(bc);

		if (test_bit(BQ24262_WMPORT_THR_MAIN_TERMINATE, &(bc->flags))) {
			/* Requested terminate this thread. */
			set_bit(BQ24262_WMPORT_THR_MAIN_EXIT, &(bc->flags));
		}
		bc->thr_main_wake_count++;
	}
	/* Notify thread exited. */
	wake_up_interruptible(&(bc->event_hs));
	PRINTK_LI(KERN_INFO "%s: Thread reaches end of function.\n",__func__);
	return 0;
}

/*! Issue event to power source thread.
    @param bc battery charger context.
*/
void bq24262_wmport_thr_main_event(struct bq24262_wmport_context *bc)
{	set_bit(BQ24262_WMPORT_THR_MAIN_WAKE, &(bc->flags));
	wake_up_interruptible(&(bc->event_main));
}

/*! Prepare power source thread to suspend.
    @param bc battery charger context.
    @return int ==0: Success (including timeout), <0: Failed, negative errno number.
*/
int bq24262_wmport_thr_main_suspend(struct bq24262_wmport_context *bc)
{	unsigned long	flags;
	int		ret;

	spin_lock_irqsave(&(bc->lock), flags);
	set_bit(BQ24262_WMPORT_THR_MAIN_SUSPEND_REQ, &(bc->flags));
	/* Clear bit to detect "cause of resume is connecting DCIN or VBUS". */
	clear_bit(BQ24262_WMPORT_WAKE_SOURCE_CHANGED, &(bc->flags));
	spin_unlock_irqrestore(&(bc->lock), flags);
	bq24262_wmport_thr_main_event(bc);
	/* Wait event thread synced. */
	ret = wait_event_interruptible_timeout(
		bc->event_hs,
		test_and_clear_bit(BQ24262_WMPORT_THR_MAIN_SUSPEND_ACK, &(bc->flags)),
		msecs_to_jiffies(BQ24262_WMPORT_THR_MAIN_SYNC_SUSPEND_TIMEOUT_MS)
	);
	if (ret <= 0) {
		pr_err("%s: Can not sync thread. ret=%d.\n",__func__, ret);
		return ret;
	} else {
		PRINTK_TM(KERN_INFO "%s: Sync with power thread. ret=%d.\n",__func__, ret);
	}
	return 0 /* Success. */;
}

/*! Prepare power source thread to shutdown.
    @param bc charger context.
    @return int ==0: Success (including timeout), <0: Failed, negative errno number.
*/
int bq24262_wmport_thr_main_shutdown(struct bq24262_wmport_context *bc)
{	unsigned long	flags;
	int		ret;

	spin_lock_irqsave(&(bc->lock), flags);
	set_bit(BQ24262_WMPORT_THR_MAIN_SHUTDOWN_REQ, &(bc->flags));
	/* Clear bit to detect "cause of resume is connecting DCIN or VBUS". */
	clear_bit(BQ24262_WMPORT_WAKE_SOURCE_CHANGED, &(bc->flags));
	spin_unlock_irqrestore(&(bc->lock), flags);
	bq24262_wmport_thr_main_event(bc);
	/* Wait event thread synced. */
	ret = wait_event_interruptible_timeout(
		bc->event_hs,
		test_and_clear_bit(
			BQ24262_WMPORT_THR_MAIN_SHUTDOWN_ACK,
			&(bc->flags)
		),
		msecs_to_jiffies(BQ24262_WMPORT_THR_MAIN_SYNC_SHUTDOWN_TIMEOUT_MS)
	);
	if (ret <= 0) {
		pr_err("%s: Can not sync thread. ret=%d.\n",__func__, ret);
		return ret;
	} else {
		PRINTK_TM(KERN_INFO "%s: Sync with power thread. ret=%d.\n",__func__, ret);
	}
	return 0 /* Success. */;
}


/*! Sync with ICX platform power source thread.
    @param bc battery charger driver context.
    @return int ==0: Success (including timeout), <0: Failed, negative errno number.
*/
int bq24262_wmport_thr_main_resume(struct bq24262_wmport_context *bc)
{	int		ret;
	unsigned long	flags;

	spin_lock_irqsave(&(bc->lock), flags);
	set_bit(BQ24262_WMPORT_THR_MAIN_RESUME_REQ, &(bc->flags));
	spin_unlock_irqrestore(&(bc->lock), flags);
	bq24262_wmport_thr_main_event(bc);
	/* Wait event thread synced. */
	ret = wait_event_interruptible_timeout(
		bc->event_hs,
		test_and_clear_bit(BQ24262_WMPORT_THR_MAIN_RESUME_ACK, &(bc->flags)),
		msecs_to_jiffies(BQ24262_WMPORT_THR_MAIN_SYNC_RESUME_TIMEOUT_MS)
	);
	if (ret <= 0) {
		pr_err("%s: Can not sync thread. ret=%d.\n",__func__, ret);
		return ret;
	}
	PRINTK_HV("%s: Sync with power source thread. ret=%d\n",__func__, ret);
	return 0 /* Success. */;
}


/*! Terminate ICX platform power source thread.
    @param bc battery charger context.
    @return int ==0: Success (including timeout), <0: Failed, negative errno number.
*/
int bq24262_wmport_thr_main_terminate(struct bq24262_wmport_context *bc)
{	int		ret;
	int		already_terminated;

	unsigned long	flags;

	spin_lock_irqsave(&(bc->lock), flags);
	already_terminated = test_bit(BQ24262_WMPORT_THR_MAIN_EXIT, &(bc->flags));
	set_bit(BQ24262_WMPORT_THR_MAIN_TERMINATE, &(bc->flags));
	spin_unlock_irqrestore(&(bc->lock), flags);
	if (already_terminated) {
		return 0 /* Success */;
	}
	bq24262_wmport_thr_main_event(bc);
	/* Wait event thread exited. */
	PRINTK_LI("%s: Terminate power source thread.\n", __func__);
	ret = wait_event_interruptible(
		bc->event_hs,
		test_bit(BQ24262_WMPORT_THR_MAIN_EXIT, &(bc->flags))
	);
	if (ret) {
		pr_err("%s: Can not exit thread. ret=%d.\n",__func__, ret);
		return ret;
	}
	PRINTK_LI(KERN_INFO "%s: Terminated power source thread.\n",__func__);
	return 0 /* Success. */;
}

/*! Prepare to start ICX platform power source thread.
    @param bc battery charger context.
    @return int ==0: Success, <0: Failed, negative errno number.
*/
int bq24262_wmport_thr_main_prepare_start(struct bq24262_wmport_context *bc)
{	bc->usb_bcdet =			ICX_CHARGER_UNKNOWN;
	bc->usb_state =			BQ24262_WMPORT_USB_UNKNOWN;
	bc->usb_max_power =		bq24262_wmport_vbus_max_power_init_ma;
	if (bq24262_wmport_vbus_max_power_init_ma < BQ24262_WMPORT_VBUS_CURRENT_CONFIG_MA) {
		/* limited VBUS current less than 500mA */
		/* Intentionally change flag safe to stop draw current VBUS.
		   Even if the battery goes low and not safe to run system,
		   we can't draw maximum current from VBUS.
		*/
		set_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags));
	}
	bc->config =			BQ24262_WMPORT_CONFIG_BOOT;
	bc->config_ptr =		NULL;
	bc->battery_boost =		BQ24262_WMPORT_BATTERY_BOOST_CHARGE;
	bc->battery_boost_prev =	BQ24262_WMPORT_BATTERY_BOOST_CHARGE;
	bc->battery_charge =		BQ24262_WMPORT_BATTERY_CHARGE_UP;
	bc->battery_temp_state =	BQ24262_WMPORT_BATTERY_TEMP_STATE_NORMAL;
	bc->vindpm_state =		BQ24262_WMPORT_VINDPM_HI;
	bc->vindpm_state_prev =		BQ24262_WMPORT_VINDPM_HI;
	bc->vindpm_config =		BQ24262_WMPORT_VINDPM_HI;
	bc->battery_safe_count =	0;
	bc->battery_uv=BQ24262_WMPORT_BATTERY_INIT_UV;
	bq24262_wmport_status_read(bc, false);
	bc->battery_capacity_notify_prev =	BQ24262_WMPORT_BATTERY_CAPACITY_INVALID;
	bc->battery_temp_state_notify_prev =	BQ24262_WMPORT_BATTERY_TEMP_STATE_NORMAL;
	PRINTK_LI("%s: ADC reads. battery_uv=%ld, battery_temp_raw=%lu(0x%lx)\n",
		__func__,
		bc->battery_uv,
		(unsigned long)bc->battery_temp_raw, (unsigned long)bc->battery_temp_raw
	);
	init_waitqueue_head(&(bc->event_main));
	init_waitqueue_head(&(bc->event_hs));
	init_waitqueue_head(&(bc->event_diag));
	bc->thr_main_wake_count = 0;
	bc->thr_main_wake_count_trace = 0;
	bc->resume_req = 0;
	bc->suspend_req = 0;
	bc->shutdown_req = 0;
	bc->diag_req = 0;
	bc->fully_weakly = BQ24262_WMPORT_CHARGE_FULLY;
	bc->fully_weakly_prev = BQ24262_WMPORT_CHARGE_FULLY;
	bc->usb_sus_mode = BQ24262_WMPORT_USB_SUS_NORMAL;
	bc->early_sus = false;
	bc->ulp_mode = false;
	bc->usb_test_mode = false;
	return 0 /* Success. */;
}


/*! Start ICX platform power source thread.
    @param bc battery charger context.
    @return int ==0: Success, <0: Failed, negative errno number.
*/
int bq24262_wmport_thr_main_start(struct bq24262_wmport_context *bc)
{	int			ret;
	struct task_struct	*thread;

	PRINTK_LI("%s: Start power supply thread.\n",__func__);
	thread = kthread_create(
		bq24262_wmport_thr_main_function,
		bc,
		bq24262_wmport_battery_name
	);
	bc->thr_main = thread;
	if (!thread) {
		pr_err("%s: Can not create thread.\n",__func__);
		ret = -ENOMEM;
		goto out;
	}
	ret = wake_up_process(thread);
	if (!ret) {
		pr_err("%s: Can not start thread. ret=%d.\n",__func__, ret);
		set_bit(BQ24262_WMPORT_THR_MAIN_EXIT, &(bc->flags));
		goto out;
	}

	ret = wait_event_interruptible(
		bc->event_hs,
		test_and_clear_bit(
			BQ24262_WMPORT_THR_MAIN_START,
			&(bc->flags)
		)
	);
	if (ret) {
		pr_err("%s: Can not start thread. ret=%d.\n",__func__, ret);
		goto out;
	}
	/* Started thread and waiting event. */
	return 0 /* Success. */;
out:
	set_bit(BQ24262_WMPORT_THR_MAIN_EXIT, &(bc->flags));
	bq24262_wmport_thr_main_terminate(bc);
	return ret;
}

/*! Is stopping drawing current from VBUS by suspend condition.
    @return int ==0: May drawing current, ==1: stop drawing.
    @note Assuming that we can read flags in atomic.
*/
static bool bq24262_wmport_is_stop_draw_by_suspend(int usb_state, unsigned usb_bcdet, bool safe_stop)
{	switch (usb_bcdet) {
		case ICX_CHARGER_CDP:
		case ICX_CHARGER_DCP:
		case ICX_CHARGER_APL_0R5:
		case ICX_CHARGER_APL_1R0:
		case ICX_CHARGER_APL_2R1:
		case ICX_CHARGER_AC_S508U:
			/* USB-AC adapters. */
			return false /* drawing (not stopped by suspend). */;
		case ICX_CHARGER_STD:
			/* Standard USB port. */
			switch (usb_state) {
				case BQ24262_WMPORT_USB_SUSPENDED:
				case BQ24262_WMPORT_USB_UNCONFIG:
					return true /* stopping. */;
				case BQ24262_WMPORT_USB_CONFIG_LOW:
					return true /* stopping. */;
				case BQ24262_WMPORT_USB_LBB_DRAW:
					return false /* drawing (not stopped by suspend). */;
				case BQ24262_WMPORT_USB_UNKNOWN:
					if (safe_stop) {
						return true /* stopping. */;
					}
					return false /* drawing (not stopped by suspend). */;
				default:
					return false /* drawing (not stopped by suspend). */;
			}
		case ICX_CHARGER_UNKNOWN:
			/* Not detected. */
			switch (usb_state) {
				case BQ24262_WMPORT_USB_UNKNOWN:
				case BQ24262_WMPORT_USB_SUSPENDED:
					if (safe_stop) {
						return true /* stopping. */;
						}
					return false /* drawing (not stopped by suspend). */;
				default:
					pr_err("%s.C_UNKNOWN: ERROR: Unexpected usb_state. usb_state=%u\n", __func__, usb_state);
					return true /* stopping. */;
			}
		case ICX_CHARGER_MISC_OPEN:
		case ICX_CHARGER_MISC_XXX:
			/* Handmade or broken adapters. */
			return true /* stopping. */;
		default:
			pr_err("%s: ERROR: Unexpected charger. usb_bcdet=%u\n", __func__, usb_bcdet);
			return true /* stopping. */;
	}
	/* not come here. */
	return false /* drawing (not sure). */;
}

#define bat_to_bq24262_wmport_context(x) container_of((x), \
			struct bq24262_wmport_context, power_supply_rl[BQ24262_WMPORT_PSRL_BAT].ps);

#define usb_to_bq24262_wmport_context(x) container_of((x), \
			struct bq24262_wmport_context, power_supply_rl[BQ24262_WMPORT_PSRL_USB].ps);

#define ac_to_bq24262_wmport_context(x) container_of((x), \
		struct bq24262_wmport_context, power_supply_rl[BQ24262_WMPORT_PSRL_DC].ps);

static int bq24262_wmport_dc_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	struct bq24262_wmport_context	*bc;

	bc = ac_to_bq24262_wmport_context(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		/* Read Power source conditon. */
		down(&(bc->status_sem));
		/* result: 0: Disconnected, 1: Connected. */
		val->intval = ((bc->power_source & BQ24262_WMPORT_DCIN) != 0);
		up(&(bc->status_sem));
		PRINTK_BATTERY_R("%s.ONLINE: return. intval=%d\n",__func__, val->intval);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int bq24262_wmport_usb_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	struct bq24262_wmport_context *bc;
	int		usb_state;
	unsigned	usb_bcdet;

	bc = usb_to_bq24262_wmport_context(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		/* Read Power source conditon. */
		down(&(bc->status_sem));
		down(&(bc->usb_sem));
		usb_state = bc->usb_state;
		usb_bcdet = bc->usb_bcdet;
		/* result: 0: Disconnected, 1: Connected. */
		/* USB power source is avaliable when source from
		   only USB VBUS.
		*/
		val->intval = (bc->power_source == BQ24262_WMPORT_USB);
		if (bq24262_wmport_is_stop_draw_by_suspend(
			usb_state, usb_bcdet,
			test_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags))
			)
		) {
			/* stop drawing from USB VBUS due to "suspend state". */
			val->intval = 0;
		}
		up(&(bc->usb_sem));
		up(&(bc->status_sem));
		PRINTK_BATTERY_R("%s.ONLINE: return. intval=%d\n",__func__, val->intval);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


#define	CHARGED_CAPACITY_FULL_OVERRIDE_FIX	 (100)


static int bq24262_wmport_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{	struct bq24262_wmport_context	*bc;
	union  power_supply_propval	val_dummy;
	int				usb_state;
	unsigned			usb_bcdet;

	if (!val) {
		/* Invalid pointer. */
		val_dummy.intval = POWER_SUPPLY_STATUS_UNKNOWN;
		val=&val_dummy;
	}

	bc = bat_to_bq24262_wmport_context(psy);

	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = bc->charger_ps_status;
			down(&(bc->status_sem));
			down(&(bc->usb_sem));
			if (bq24262_wmport_status_is_charged_full(bc)) {
				/* Charge done. */
				val->intval = POWER_SUPPLY_STATUS_FULL;
			}
			usb_state =    bc->usb_state;
			usb_bcdet =    bc->usb_bcdet;
			switch (bc->power_source) {
				case BQ24262_WMPORT_USB:
					/* USB feeds power. */
					if (bq24262_wmport_is_stop_draw_by_suspend(
						usb_state, usb_bcdet,
						test_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags))
					    )
					) {
						/* stop drawing from USB VBUS due to "suspend state". */
						val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
						PRINTK_BATTERY_R("%s.STATUS: Suspend. usb_state=%d, usb_bcdet=%d, current=%d\n",
							__func__,
							usb_state, usb_bcdet, test_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags))
						);
					}
					break;
				default:
					/* Do nothing. */
					break;
			}
			if (bq24262_wmport_status_is_stop_charging(bc)) {
				val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
				PRINTK_BATTERY_R("%s.STATUS: Stop charging.\n",
					__func__
				);
				/* @note refrected BQ24262_WMPORT_BATTERY_CHARGE_UP|KEEP in
				         bq24262_wmport_configs_refrect_charger_ps_status().
				*/
			}
			if (bc->fake_capacity >= CHARGED_CAPACITY_FULL_OVERRIDE_FIX) {
				val->intval = POWER_SUPPLY_STATUS_FULL;
			}
			up(&(bc->usb_sem));
			up(&(bc->status_sem));
			PRINTK_BATTERY_R(KERN_INFO "%s.STATUS: return. intval=%d\n",__func__, val->intval);
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			down(&(bc->status_sem));
			val->intval = (int)(bc->battery_uv);
			up(&(bc->status_sem));
			PRINTK_BATTERY_R("%s.VOLTAGE_NOW: return. intval=%d\n",__func__, val->intval);
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			val->intval = 0;
			down(&(bc->status_sem));
			val->intval = bc->battery_capacity;
			if (bq24262_wmport_status_is_charged_full(bc)) {
				val->intval = CHARGED_CAPACITY_FULL_OVERRIDE_FIX;
			} else {
				if (val->intval >= CHARGED_CAPACITY_FULL_OVERRIDE_FIX) {
					val->intval = CHARGED_CAPACITY_FULL_OVERRIDE_FIX - 1;
				}
			}
			if (bc->fake_capacity >= 0) {
				val->intval = bc->fake_capacity;
				PRINTK_LI("%s: Return fake_capacity. fake_capacity=%d\n", __func__, bc->fake_capacity);
			}
			up(&(bc->status_sem));
			PRINTK_BATTERY_R("%s.CAPACITY: return. intval=%d\n",__func__, val->intval);
			break;
		case POWER_SUPPLY_PROP_ONLINE:
			down(&(bc->status_sem));
			if (bq24262_wmport_status_is_battery_online(bc)) {
				val->intval = 1;
			} else {
				val->intval = 0;
			}
			up(&(bc->status_sem));
			PRINTK_BATTERY_R("%s.ONLINE: return. intval=%d\n",__func__, val->intval);
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			down(&(bc->status_sem));
			if (bq24262_wmport_status_is_battery_online(bc)) {
				val->intval = 1;
			} else {
				val->intval = 0;
			}
			up(&(bc->status_sem));
			PRINTK_BATTERY_R("%s.PRESENT: return. intval=%d\n",__func__, val->intval);
			break;
		case POWER_SUPPLY_PROP_HEALTH:
			val->intval = POWER_SUPPLY_HEALTH_GOOD;
			down(&(bc->status_sem));
			if (bq24262_wmport_status_is_battery_dead(bc)) {
				val->intval = POWER_SUPPLY_HEALTH_DEAD;
			} else {
				switch (bc->battery_temp_state) {
					case BQ24262_WMPORT_BATTERY_TEMP_STATE_COLD:
						val->intval =  POWER_SUPPLY_HEALTH_COLD;
						break;
					case BQ24262_WMPORT_BATTERY_TEMP_STATE_HOT:
						val->intval =  POWER_SUPPLY_HEALTH_OVERHEAT;
						break;
					default:
						/* do nothing. */
						break;
				}
				/* @todo read SAFETY and check hot or cold. */
			}
			up(&(bc->status_sem));
			PRINTK_BATTERY_R(KERN_INFO "%s.HEALTH: return. intval=%d\n",__func__, val->intval);
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static int bq24262_wmport_ma_to_usb_state(unsigned mA)
{	int	usb_state;

	usb_state = BQ24262_WMPORT_USB_SUSPENDED;
	if (mA > BQ24262_WMPORT_VBUS_CURRENT_SUSPEND_MA) {
		if (mA <= BQ24262_WMPORT_VBUS_CURRENT_UNCONFIG_MA) {
			/* Unconfigured current. */
			/* @note mA may be 0, 2, or 8. S usb_gadget_vbus_draw,
			   and musb_gadget_vbus_draw callers.
			*/
			usb_state = BQ24262_WMPORT_USB_UNCONFIG;
		} else {
			if (mA < BQ24262_WMPORT_VBUS_CURRENT_CONFIG_MA) {
				/* Configured current but not maximum. */
				usb_state = BQ24262_WMPORT_USB_CONFIG_LOW;
			} else {
				/* Configured current maximum. */
				usb_state = BQ24262_WMPORT_USB_CONFIG;
			}
		}
	}
	return usb_state;
}

/* call at USB test SE0/NACK */
void bq24262_wmport_usb_test_mode(bool on)
{
	struct bq24262_wmport_static_context * bcst;
	struct bq24262_wmport_context * bc;
	unsigned long flags_bcst;

	/* PRINTK_LI("%s\n", __func__); */

	bcst = &bq24262_wmport_static;

	down(&(bcst->sem));

	spin_lock_irqsave(&(bcst->lock), flags_bcst);
	bc = bcst->bc_locked;
	spin_unlock_irqrestore(&(bcst->lock), flags_bcst);

	if (!bc) {
		pr_err("%s: Null points driver context.\n", __func__);
		up(&(bcst->sem));
		return;
	}

	down(&(bc->status_sem));

	PRINTK_LI("%s: USB TEST MODE = %s\n", __func__, on?"on":"off");
	bc->usb_test_mode=on;

	up(&(bc->status_sem));

	bq24262_wmport_thr_main_event(bc);

	up(&(bcst->sem));

	return;
}


/* usb_phy to bq24262_wmport bridge: set_power (Set VBUS current) event handler.
*/
int bq24262_wmport_usb_set_power_event(unsigned mA)
{	struct bq24262_wmport_static_context	*bcst;
	struct bq24262_wmport_context		*bc;
	unsigned long				flags_bcst;
	int					usb_state;
	int					result;

	result = 0;
	bcst = &bq24262_wmport_static;
	down(&(bcst->sem));
	spin_lock_irqsave(&(bcst->lock), flags_bcst);
	bc = bcst->bc_locked;
	spin_unlock_irqrestore(&(bcst->lock), flags_bcst);
	if (!bc) {
		pr_err("%s: ERROR: Null points driver context.\n", __func__);
		/* Can't handle this event any more. */
		result = -ENODEV;
		goto out;
	}
	down(&(bc->usb_sem));
	usb_state = bq24262_wmport_ma_to_usb_state(mA);
	PRINTK_LI("%s: Set power. mA=%u, usb_state=%d\n", __func__, mA, usb_state);
	if (test_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags))) {
		/* Safe to stop VBUS current due to suspend. */
		bc->usb_state = usb_state;
	} else {
		/* Not safe to stop VBUS current due to suspend. */
		switch (usb_state) {
			case BQ24262_WMPORT_USB_SUSPENDED:
			case BQ24262_WMPORT_USB_UNCONFIG:
				bc->usb_state = BQ24262_WMPORT_USB_LBB_DRAW;
				break;
			default:
				/* @note default including _CONFIG_LOW,
				   _CONFIG_LOW doesn't draw current.
				   User selects _CONFIG_LOW intentionally.
				   User want that No or Less current draw from VBUS.
				*/
				bc->usb_state = usb_state;
				break;
		}
	}
	up(&(bc->usb_sem));
	bq24262_wmport_thr_main_event(bc);
out:
	up(&(bcst->sem));
	return result;
}

/*! android.c usb gadget to bq24262_wmport bridge: set android0/enable event handler.
*/
void bq24262_wmport_usb_set_gadget_enable_event(bool enable)
{	struct bq24262_wmport_static_context	*bcst;
	struct bq24262_wmport_context		*bc;
	unsigned long				flags;

	bcst = &bq24262_wmport_static;
	spin_lock_irqsave(&(bcst->lock), flags);
	bc = bcst->bc_locked;
	if (!bc) {
		pr_err("%s: Null points driver context.\n", __func__);
		/* Can't handle this interrupt any more. */
		goto out;
	}
	if (enable) {
		set_bit(  BQ24262_WMPORT_USB_GADGET_ENABLE, &(bc->flags));
		bq24262_wmport_thr_vbus_gadget_ready(bc_to_vb(bc));
	} else {
		clear_bit(BQ24262_WMPORT_USB_GADGET_ENABLE, &(bc->flags));
	}
	bq24262_wmport_thr_main_event(bc);
out:
	spin_unlock_irqrestore(&(bcst->lock), flags);
	return;
}

/* android.c usb gadget to bq24262_wmport bridge: set MaxPower event handler.
*/
int bq24262_wmport_usb_set_max_power_event(unsigned mA)
{	struct bq24262_wmport_static_context	*bcst;
	struct bq24262_wmport_context		*bc;
	unsigned long				flags_bcst;
	int					usb_state;
	int					result;

	result = 0;
	bcst = &bq24262_wmport_static;
	down(&(bcst->sem));
	spin_lock_irqsave(&(bcst->lock), flags_bcst);
	bc = bcst->bc_locked;
	spin_unlock_irqrestore(&(bcst->lock), flags_bcst);
	if (!bc) {
		pr_err("%s: ERROR: Null points driver context.\n", __func__);
		/* Can't handle this event any more. */
		result = -ENODEV;
		goto out;
	}
	down(&(bc->usb_sem));
	usb_state = bq24262_wmport_ma_to_usb_state(mA);
	bc->usb_max_power = mA;
	switch (usb_state) {
		default:
			/* low current. */
			/* limited VBUS current less than 500mA, user doesn't want draw current from VBUS. */
			/* Intentionally change flag safe to stop draw current VBUS.
			   Even if the battery goes low and not safe to run system,
			   we can't draw maximum current from VBUS.
			*/
			set_bit(BQ24262_WMPORT_SAFE_SUSPEND_CURRENT, &(bc->flags));
			break;
		case BQ24262_WMPORT_USB_CONFIG:
			break;
	}
	switch (bc->usb_state) {
		case BQ24262_WMPORT_USB_CONFIG:
		case BQ24262_WMPORT_USB_CONFIG_LOW:
			/* now configured state. */
			bc->usb_state = usb_state;
			break;
		default:
			/* do nothing. */
			break;
	}
	up(&(bc->usb_sem));
	bq24262_wmport_thr_main_event(bc);
out:
	up(&(bcst->sem));
	return result;
}

/* MUSB (usb20) to bq24262_wmport bridge.
   called from usb_cable_connected.
   @return bool ==false: "VBUS not connected" or "not connected host(CDP) port", \
                ==true: "connected to host(CDP) port."
*/
bool bq24262_wmport_usb_cable_for_charge_connected(void)
{	struct bq24262_wmport_static_context	*bcst;
	struct bq24262_wmport_context		*bc;
	struct bq24262_wmport_vbus_context	*vb;

	unsigned long				flags_bcst;
	unsigned long				flags;
	bool					result;

	result = false;
	bcst = &bq24262_wmport_static;
	/* @note intentionally DOES NOT down bcst->sem */
	spin_lock_irqsave(&(bcst->lock), flags_bcst);
	bc = bcst->bc_locked;
	if (!bc) {
		pr_err("%s: ERROR: Null points driver context.\n", __func__);
		/* Can't handle this event any more. */
		result = -ENODEV;
		goto out;
	}
	vb = bc_to_vb(bc);
	spin_lock_irqsave(&(vb->lock), flags);
	switch (vb->usb_bcdet_splocked) {
		case ICX_CHARGER_CDP:
		case ICX_CHARGER_STD:
			result = true;
			break;
		default:
			/* do nothing. */
			break;
	}
	spin_unlock_irqrestore(&(vb->lock), flags);
out:
	spin_unlock_irqrestore(&(bcst->lock), flags_bcst);
	return result;
}

int bq24262_wmport_usb_phy_off(int off)
{	struct bq24262_wmport_static_context	*bcst;
	struct bq24262_wmport_context		*bc;
	unsigned long				flags_bcst;
	int					result;

	result = 0;
	bcst = &bq24262_wmport_static;
	down(&(bcst->sem));
	spin_lock_irqsave(&(bcst->lock), flags_bcst);
	bc = bcst->bc_locked;
	spin_unlock_irqrestore(&(bcst->lock), flags_bcst);
	if (!bc) {
		pr_err("%s: ERROR: Null points driver context.\n", __func__);
		/* Can't handle this event any more. */
		result = -ENODEV;
		goto out;
	}
	/* down(&(bc->usb_sem)); */
	/* up(&(bc->usb_sem)); */
	bq24262_wmport_thr_vbus_phy_off(bc_to_vb(bc),off);
out:
	up(&(bcst->sem));
	return result;
}

STATIC_FUNC int bq24262_wmport_pm_notifier(struct notifier_block *nb, unsigned long val, void *ign)
{	struct bq24262_wmport_context *bc;

	PRINTK_LI("%s: Called. val=%lu\n", __func__, val);

	bc = container_of(nb, struct bq24262_wmport_context, pm_nb);

	switch (val) {
		case PM_SUSPEND_PREPARE:
			set_bit(BQ24262_WMPORT_SUSPEND_IN_PROGRESS, &(bc->flags));
			break;
		case PM_POST_SUSPEND:
			clear_bit(BQ24262_WMPORT_SUSPEND_IN_PROGRESS, &(bc->flags));
			break;
		default:
			return(NOTIFY_DONE);
	}
	return(NOTIFY_OK);
}

static const char *str_skip_space(const char *p, size_t n)
{	char		c;

	while (((c = *p) != 0) && (n > 0))  {
		if (((unsigned char)c) > ((unsigned char)' ')) {
			break;
		}
		p++;
		n--;
	}
	return p;
}

static const char *str_skip_alnum(const char *p)
{	char	c;

	while ((c=*p) != 0) {
		if (!isalnum(c)) {
			break;
		}
		p++;
	}
	return p;
}

static const char *str_skip_dot(const char *p, size_t count)
{	p = str_skip_space(p, count);
	if (*p == '.') {
		p++;
	}
	p = str_skip_space(p, count);
	return p;
}

static ssize_t bq24262_wmport_devattr_wake_lock_charging_set(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t count)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	const char		*p;
	long			val;
	int			status = count; /* Entire strings are parsed. */

	if (count == 0) {
		pr_err("%s: Zero length attr string.\n", __func__);
		return -EINVAL;
	}
	if (count >=PAGE_SIZE) {
		pr_err("%s: Too long attr string.\n", __func__);
		return -EINVAL;
	}

	val = 0;
	p = str_skip_space(buf, count);
	if ((buf + count) == p) {
		return -EINVAL;
	}
	count -= (p - buf); /* pointer subtract. */
	if (   ((*p >= '0') && (*p <= '9'))
	    || (*p == '-') || (*p == '+')
	) {
		/* Numeric request. */
		if (strict_strtol(p, 0 /* any radix */, &val) < 0) {
			pr_err("%s: Invalid numeric format. buf[20]=%20s\n", __func__, buf);
			return -EINVAL;
		}
		if ((val < 0) || ( val > BQ24262_WMPORT_WAKE_LOCK_FACTOR_ALL )) {
			pr_err("%s: Invalid value. val=0x%lx\n", __func__, val);
			return -EINVAL;
		}
	} else {
		/* Symbolic request. */
		while ((*p != 0) && (count > 0)) {
			switch (*p) {
				case 'c':
					val |= BQ24262_WMPORT_WAKE_LOCK_CHARGING;
					p++;
					count--;
					break;
				case 'u':
					val |= BQ24262_WMPORT_WAKE_LOCK_USB;
					p++;
					count--;
					break;
				case 'd':
					val |= BQ24262_WMPORT_WAKE_LOCK_DCIN;
					p++;
					count--;
					break;
				default:
					if (*((unsigned char*)p) >=' ') {
						pr_err("%s: Invalid wake lock factor symbol. *p=%c\n", __func__, *p);
						return -EINVAL;
					}
					/* Space. */
					p++;
					count--;
					break;
			}
		}
	}

	down(&(bc->wake_lock_sem));
	PRINTK_WL("%s: Changed wake lock source. val=0x%lx\n", __func__, val);
	bc->wake_lock_source = val;
	up(&(bc->wake_lock_sem));
	/* wake main thread to refrect wake lock settings. */
	bq24262_wmport_thr_main_event(bc);
	return status;
}

static ssize_t bq24262_wmport_devattr_wake_lock_charging_show(struct device *dev, struct device_attribute *attr,
				  char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	long				val;

	down(&(bc->wake_lock_sem));
	val = bc->wake_lock_source;
	up(&(bc->wake_lock_sem));
	return snprintf(buf, PAGE_SIZE, "0x%lx\n", val);
}

static DEVICE_ATTR(wake_lock_charging, S_IWUSR | S_IRUGO, bq24262_wmport_devattr_wake_lock_charging_show, bq24262_wmport_devattr_wake_lock_charging_set);


static ssize_t bq24262_wmport_devattr_fully_weakly_set(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t count)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	char			val;
	int			status = count; /* Entire strings are parsed. */
	const char		*p;

	if (count == 0) {
		pr_err("%s: Zero length attr string.\n", __func__);
		return -EINVAL;
	}
	if (count >=PAGE_SIZE) {
		pr_err("%s: Too long attr string.\n", __func__);
		return -EINVAL;
	}
	p = str_skip_space(buf, count);
	if ((buf + count) == p) {
		pr_err("%s: String contain no control character.\n", __func__);
		return -EINVAL;
	}
	switch (*p) {
		case 'f':
			val = BQ24262_WMPORT_CHARGE_FULLY;
			break;
		case 'w':
			val = BQ24262_WMPORT_CHARGE_WEAKLY;
			break;
		default:
			pr_err("%s: Invalid attr value. *p=%c\n", __func__, *p);
			return -EINVAL;
	}
	down(&(bc->status_sem));
	if ((bc->fully_weakly != val) && (val == BQ24262_WMPORT_CHARGE_FULLY)) {
		switch (bc->battery_boost) {
			case BQ24262_WMPORT_BATTERY_BOOST_STOP:
				/* Boost (Full stop) mode. */
				bc->battery_boost = BQ24262_WMPORT_BATTERY_BOOST_CHARGE;
			default:
				/* do nothing. */
				break;
		}
	}
	bc->fully_weakly = val;
	up(&(bc->status_sem));
	bq24262_wmport_thr_main_event(bc);
	return status;
}

static ssize_t bq24262_wmport_devattr_fully_weakly_show(struct device *dev, struct device_attribute *attr,
				  char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	const char			*val;

	down(&(bc->status_sem));
	switch (bc->fully_weakly) {
		case BQ24262_WMPORT_CHARGE_FULLY:
			val="fully";
			break;
		case BQ24262_WMPORT_CHARGE_WEAKLY:
			val="weakly";
			break;
		default:
			val="unknown";
			break;
	}
	up(&(bc->status_sem));
	return snprintf(buf, PAGE_SIZE, "%s\n", val);
}

static DEVICE_ATTR(fully_weakly, S_IWUSR | S_IRUGO, bq24262_wmport_devattr_fully_weakly_show, bq24262_wmport_devattr_fully_weakly_set);


static ssize_t bq24262_wmport_devattr_usb_sus_mode_set(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t count)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	char			val;
	int			status = count; /* Entire strings are parsed. */
	const char		*p;

	if (count == 0) {
		pr_err("%s: Zero length attr string.\n", __func__);
		return -EINVAL;
	}
	if (count >=PAGE_SIZE) {
		pr_err("%s: Too long attr string.\n", __func__);
		return -EINVAL;
	}
	p = str_skip_space(buf, count);
	if ((buf + count) == p) {
		pr_err("%s: String contain no control character.\n", __func__);
		return -EINVAL;
	}
	switch (*p) {
		case 'n':
			val = BQ24262_WMPORT_USB_SUS_NORMAL;
			break;
		case 'd':
			val = BQ24262_WMPORT_USB_SUS_DRAW;
			break;
		default:
			pr_err("%s: Invalid attr value. *p=%c\n", __func__, *p);
			return -EINVAL;
	}
	down(&(bc->status_sem));
	bc->usb_sus_mode = val;
	up(&(bc->status_sem));
	bq24262_wmport_thr_main_event(bc);
	return status;
}

static ssize_t bq24262_wmport_devattr_usb_sus_mode_show(struct device *dev, struct device_attribute *attr,
				  char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	const char			*val;

	down(&(bc->status_sem));
	switch (bc->usb_sus_mode) {
		case BQ24262_WMPORT_USB_SUS_NORMAL:
			val="normal";
			break;
		case BQ24262_WMPORT_USB_SUS_DRAW:
			val="draw";
			break;
		default:
			val="unknown";
			break;
	}
	up(&(bc->status_sem));
	return snprintf(buf, PAGE_SIZE, "%s\n", val);
}

static DEVICE_ATTR(usb_sus_mode, S_IWUSR | S_IRUGO, bq24262_wmport_devattr_usb_sus_mode_show, bq24262_wmport_devattr_usb_sus_mode_set);

/*! Device attr: Write Force DCIN gpio setting.
*/
static ssize_t bq24262_wmport_devattr_force_dcin_set(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t count)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	long			val;
	int			status = count; /* Entire strings are parsed. */

	if (strict_strtol(buf, 0 /* any radix */, &val) < 0) {
		return -EINVAL;
	}
	if ((val < 0) || ( val > 1 )) {
		return -EINVAL;
	}

	bq24262_wmport_force_dcin_write(bc, (unsigned)val);
	return status;
}

/*! Device attr: Read force DCIN gpio setting.
*/
static ssize_t bq24262_wmport_devattr_force_dcin_show(struct device *dev, struct device_attribute *attr,
				  char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	unsigned int val;

	val = bq24262_wmport_force_dcin_read(bc);
	return snprintf(buf, PAGE_SIZE, "%u\n", val);
}

static DEVICE_ATTR(force_dcin, S_IWUSR | S_IRUGO, bq24262_wmport_devattr_force_dcin_show, bq24262_wmport_devattr_force_dcin_set);

/*! Device attr: Read battery voltage in uv.
*/
static ssize_t bq24262_wmport_devattr_battery_uv_show(struct device *dev, struct device_attribute *attr, char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	long	val;

	down(&(bc->status_sem));
	val = bc->battery_uv;
	up(&(bc->status_sem));
	return snprintf(buf, PAGE_SIZE, "%ld\n", val);
}
static DEVICE_ATTR(battery_uv, S_IRUGO, bq24262_wmport_devattr_battery_uv_show, NULL);

/*! Device attr: Read battery voltage in mv.
    @note Implement this function for compatibility.
*/
static ssize_t bq24262_wmport_devattr_battery_mv_show(struct device *dev, struct device_attribute *attr, char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	long	val;

	down(&(bc->status_sem));
	val = (bc->battery_uv + 500) / 1000;
	up(&(bc->status_sem));
	return snprintf(buf, PAGE_SIZE, "%ld\n", val);
}
static DEVICE_ATTR(battery_mv, S_IRUGO, bq24262_wmport_devattr_battery_mv_show, NULL);


/*! Device attr: Read battery thermistor line voltage in raw.
*/
static ssize_t bq24262_wmport_devattr_battery_temp_raw_show(struct device *dev, struct device_attribute *attr, char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	unsigned long			val;

	down(&(bc->status_sem));
	val = (unsigned long)(bc->battery_temp_raw);
	up(&(bc->status_sem));
	return snprintf(buf, PAGE_SIZE, "%lu\n", val);
}
static DEVICE_ATTR(battery_temp_raw, S_IRUGO, bq24262_wmport_devattr_battery_temp_raw_show, NULL);

/*! Device attr: Read battery thermistor line voltage in mv.
*/
static ssize_t bq24262_wmport_devattr_battery_temp_mv_show(struct device *dev, struct device_attribute *attr, char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	unsigned long			val;

	down(&(bc->status_sem));
	val = (unsigned long)(bc->battery_temp_raw)*1800/32768/4;
	up(&(bc->status_sem));
	return snprintf(buf, PAGE_SIZE, "%lu\n", val);
}
static DEVICE_ATTR(battery_temp_mv, S_IRUGO, bq24262_wmport_devattr_battery_temp_mv_show, NULL);

/*! Device attr: Read power supply type.
*/
ssize_t bq24262_wmport_devattr_diag_charger_det_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	unsigned	usb_bcdet;
	ssize_t		len;

	if (!buf) {
		return -ENOMEM;
	}
	down(&(bc->usb_sem));
	usb_bcdet = bc->usb_bcdet;
	up(&(bc->usb_sem));

	if (usb_bcdet >= (sizeof(bq24262_wmport_bcdet_names)/sizeof(bq24262_wmport_bcdet_names[0]))) {
		/* Unexpected Battery Charger (USB-AC) detection result. */
		len = snprintf(buf, PAGE_SIZE, "0x%x\n", usb_bcdet);
	} else {
		len = snprintf(buf, PAGE_SIZE, "%s\n", bq24262_wmport_bcdet_names[usb_bcdet]);
	}
	return len;
}
static DEVICE_ATTR(diag_charger_det, S_IRUGO, bq24262_wmport_devattr_diag_charger_det_show, NULL);

static ssize_t bq24262_wmport_devattr_charge_timer_max_sec_set(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t count)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	long			val;
	int			status = count; /* Entire strings are parsed. */

	val = 0;
	if (strict_strtol(buf, 0 /* any radix */, &val) < 0) {
		return -EINVAL;
	}
	if (val < 0) {
		return -EINVAL;
	}

	down(&(bc->status_sem));
	bc->charge_timer_max_sec = val;
	up(&(bc->status_sem));
	bq24262_wmport_thr_main_event(bc);
	return status;
}

static ssize_t bq24262_wmport_devattr_charge_timer_max_sec_show(struct device *dev, struct device_attribute *attr,
				  char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	long val;

	down(&(bc->status_sem));
	val = bc->charge_timer_max_sec;
	up(&(bc->status_sem));
	return snprintf(buf, PAGE_SIZE, "%ld\n", val);
}

static DEVICE_ATTR(charge_timer_max_sec, S_IWUSR | S_IRUGO, bq24262_wmport_devattr_charge_timer_max_sec_show, bq24262_wmport_devattr_charge_timer_max_sec_set);

static ssize_t bq24262_wmport_devattr_suspend_dynamic_period_sec_show(struct device *dev, struct device_attribute *attr,
				  char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	long val;

	down(&(bc->status_sem));
	val = bc->dynamic_period;
	up(&(bc->status_sem));
	return snprintf(buf, PAGE_SIZE, "%ld\n", val);
}

static DEVICE_ATTR(suspend_dynamic_period_sec, S_IRUGO, bq24262_wmport_devattr_suspend_dynamic_period_sec_show, NULL);

static ssize_t bq24262_wmport_devattr_fake_capacity_set(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t count)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	long			val;
	int			status = count; /* Entire strings are parsed. */

	val = 0;
	if (strict_strtol(buf, 0 /* any radix */, &val) < 0) {
		return -EINVAL;
	}
	if (val < BQ24262_WMPORT_FAKE_CAPACITY_OFF) {
		return -EINVAL;
	}

	down(&(bc->status_sem));
	bc->fake_capacity = (/* force_cast */ int)val;
	up(&(bc->status_sem));
	power_supply_rl_changed(&(bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT]));
	bq24262_wmport_thr_main_event(bc);
	return status;
}

static ssize_t bq24262_wmport_devattr_fake_capacity_show(struct device *dev, struct device_attribute *attr,
				  char *buf)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	int	val;

	down(&(bc->status_sem));
	val = bc->fake_capacity;
	up(&(bc->status_sem));
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static DEVICE_ATTR(fake_capacity, S_IWUSR | S_IRUGO, bq24262_wmport_devattr_fake_capacity_show, bq24262_wmport_devattr_fake_capacity_set);


/*! DIAG: Set power source USB/DCIN current limit.
*/
int bq24262_wmport_diag_draw_set(struct bq24262_wmport_context *bc, const struct bq24262_wmport_diag_draw *draw)
{	unsigned long			flags;
	int				ret;

	if (!draw) {
		return -EINVAL;
	}

	spin_lock_irqsave(&(bc->lock), flags);
	if (draw->config >= 0) {
		PRINTK_LI(KERN_INFO "%s: DIAG: Enter diag mode.\n", __func__);
		bc->diag_state          = BQ24262_WMPORT_DIAG_ACTIVE;
		bc->diag_config         = draw->config;
		bc->diag_battery_charge = draw->battery_charge;
	} else {
		PRINTK_LI("%s: DIAG: Force exit diag mode.\n", __func__);
		bc->diag_state = BQ24262_WMPORT_DIAG_EXIT;
	}
	spin_unlock_irqrestore(&(bc->lock), flags);
	set_bit(BQ24262_WMPORT_THR_MAIN_DIAG_REQ, &(bc->flags));
	bq24262_wmport_thr_main_event(bc);
	PRINTK_LI(KERN_INFO "%s: DIAG: Sync power source thread.\n", __func__);
	ret = wait_event_interruptible(
		bc->event_diag,
		test_and_clear_bit(BQ24262_WMPORT_THR_MAIN_DIAG_ACK, &(bc->flags))
	);
	if (ret) {
		pr_err("%s: DIAG: Can not sync thread. ret=%d.\n",__func__, ret);
	}
	return ret /* Success. */;
}


/*! Set power source USB/DCIN current limit.
*/
void bq24262_wmport_diag_draw_get(struct bq24262_wmport_context *bc, struct bq24262_wmport_diag_draw *draw)
{	unsigned long			flags;

	spin_lock_irqsave(&(bc->lock), flags);
	draw->config =         bc->diag_config;
	draw->battery_charge = bc->diag_battery_charge;
	spin_unlock_irqrestore(&(bc->lock), flags);
	return;
}


static ssize_t bq24262_wmport_devattr_diag_draw_set(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t count)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	struct			bq24262_wmport_diag_draw draw;
	char			c;
	int			ret;

	if (count == 0) {
		pr_err("%s: Zero length command string.\n", __func__);
		return -EINVAL;
	}
	bq24262_wmport_diag_draw_get(bc, &draw);
	buf = str_skip_space(buf, count);

	c = toupper(*buf);
	switch (c) {
		case 'Q':
			/* Quit. */
			draw.config = -1;
			buf = str_skip_alnum(buf+1);
			break;
		case '.':
		case '\0':
			/* Skip. */
			break;
		case 'H':
			/* HIZ */
			draw.config = BQ24262_WMPORT_CONFIG_HIZ;
			buf = str_skip_alnum(buf+1);
			break;
		case 'R':
			/* ROLE_A */
			draw.config = BQ24262_WMPORT_CONFIG_USB_ROLE_A;
			buf = str_skip_alnum(buf+1);
			break;
		case 'S':
			/* SUSPENDED */
			draw.config = BQ24262_WMPORT_CONFIG_USB_SUSPENDED;
			buf = str_skip_alnum(buf+1);
			break;
		case 'W':
			/* SUS_DRAW */
			draw.config = BQ24262_WMPORT_CONFIG_USB_SUS_DRAW;
			buf = str_skip_alnum(buf+1);
			break;
		case 'X':
			/* UNKNOWN */
			draw.config = BQ24262_WMPORT_CONFIG_USB_UNKNOWN;
			buf = str_skip_alnum(buf+1);
			break;
		case 'U':
			/* UNCONFIG */
			draw.config = BQ24262_WMPORT_CONFIG_USB_UNCONFIG;
			buf = str_skip_alnum(buf+1);
			break;
		case 'L':
			/* CONFIG_LOW */
			draw.config = BQ24262_WMPORT_CONFIG_USB_CONFIG_LOW;
			buf = str_skip_alnum(buf+1);
			break;
		case 'C':
			/* CONFIG, CDP500, CDP900 */
			buf++;
			c = toupper(*buf);
			switch (c) {
				case 'O':
					/* CONFIG */
					draw.config = BQ24262_WMPORT_CONFIG_USB_CONFIG;
					buf = str_skip_alnum(buf+1);
					break;
				case '5':
					/* CDP500 */
					draw.config = BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_500;
					buf = str_skip_alnum(buf+1);
					break;
				case '9':
					/* CDP900 */
					draw.config = BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_900;
					buf = str_skip_alnum(buf+1);
					break;
				case 'D':
					buf++;
					c = toupper(*buf);
					switch (c) {
						case 'P':
							buf++;
							c = toupper(*buf);
							switch (c) {
								case '5':
									/* CDP500 */
									draw.config = BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_500;
									buf = str_skip_alnum(buf+1);
									break;
								case '9':
									/* CDP900 */
									draw.config = BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_900;
									buf = str_skip_alnum(buf+1);
									break;
								default:
									PRINTK_LI(KERN_INFO "%s: DIAG: Error, unknown config CDP%c.\n", __func__, c);
									return -EINVAL;
							}
							break;
						default:
							pr_err("%s: DIAG: Error, unknown config CD%c.\n", __func__, c);
							return -EINVAL;
					}
					break;
				default:
					pr_err("%s: DIAG: Error, unknown config C%c.\n", __func__, c);
					return -EINVAL;
			}
			break;
		case 'P':
			/* PS2LIKE */
			draw.config = BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_500;
			buf = str_skip_alnum(buf+1);
			break;
		case 'D':
			/* DCP, DCIN, DOWN */
			buf++;
			c = toupper(*buf);
			switch (c) {
				case 'C':
					buf++;
					c = toupper(*buf);
					switch (c) {
						case 'P':
							draw.config = BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_900;
							buf = str_skip_alnum(buf+1);
							break;
						case 'I':
							draw.config = BQ24262_WMPORT_CONFIG_DCIN;
							buf = str_skip_alnum(buf+1);
							break;
						default:
							PRINTK_LI(KERN_INFO "%s: DIAG: Error, unknown config DC%c.\n", __func__, c);
							return -EINVAL;
					}
					break;
				case 'O':
					draw.config = BQ24262_WMPORT_CONFIG_COOL_DOWN;
					buf = str_skip_alnum(buf+1);
					break;
				default:
					pr_err("%s: DIAG: Error, unknown config D%c.\n", __func__, c);
					return -EINVAL;
			}
			break;
		case '5':
			/* PS2LIKE */
			draw.config = BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_500;
			buf = str_skip_alnum(buf+1);
			break;
		case '9':
			/* DCP */
			draw.config = BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_900;
			buf = str_skip_alnum(buf+1);
			break;
		case 'N':
			/* No Power. */
			draw.config = BQ24262_WMPORT_CONFIG_NO_POWER;
			buf = str_skip_alnum(buf+1);
			break;
		default:
			pr_err("%s: DIAG: Error, unknown config %c.\n", __func__, c);
			return -EINVAL;
	}
	buf = str_skip_dot(buf, count);
	c = toupper(*buf);
	switch (c) {
		case '.':
		case '\0':
			break;
		case 'U':
			draw.battery_charge = BQ24262_WMPORT_BATTERY_CHARGE_UP;
			buf = str_skip_alnum(buf+1);
			break;
		case 'E':
			draw.battery_charge = BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG;
			buf = str_skip_alnum(buf+1);
			break;
		case 'F':
			draw.battery_charge = BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL;
			buf = str_skip_alnum(buf+1);
			break;
		default:
			pr_err("%s: DIAG: Error, unknown charge %c.\n", __func__, c);
			return -EINVAL;
	}
	ret = bq24262_wmport_diag_draw_set(bc, &draw);
	if (ret != 0) {
		return ret;
	}
	return count;
}

static ssize_t bq24262_wmport_devattr_diag_draw_get(struct device *dev,
	struct device_attribute *attr,
	char *buf
)
{	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);
	struct				bq24262_wmport_diag_draw draw;

	const char	*config_s;
	const char	*charge_s;

	char	config_tmp[sizeof(draw.config)*3+1+2];
	char	charge_tmp[sizeof(draw.battery_charge)*3+1+2];

	const char *charge_syms[]={
		[BQ24262_WMPORT_BATTERY_CHARGE_UP]="UP",
		[BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL]="FULL",
		[BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG]="EMERGENCY",
	};

	if (!buf) {
		pr_err("%s: Null points result buffer.\n", __func__);
		return -EINVAL;
	}

	bq24262_wmport_diag_draw_get(bc, &draw);

	if ((draw.config < 0) || (draw.config >= BQ24262_WMPORT_CONFIG_ALL)) {
		snprintf(config_tmp, sizeof(config_tmp),"0x%x", draw.config);
		config_s = config_tmp;
	} else {
		config_s = bq24262_wmport_config_names[draw.config];
	}

	if ((draw.battery_charge < 0) || (draw.battery_charge >= BQ24262_WMPORT_BATTERY_CHARGE_ALL)) {
		snprintf(charge_tmp, sizeof(charge_tmp), "0x%x", draw.battery_charge);
		charge_s = charge_tmp;
	} else {
		charge_s = charge_syms[draw.battery_charge];
	}
	return snprintf(buf, PAGE_SIZE, "%s.%s\n", config_s, charge_s);
}

static DEVICE_ATTR(diag_draw, S_IWUSR | S_IRUGO,
	bq24262_wmport_devattr_diag_draw_get,
	bq24262_wmport_devattr_diag_draw_set
	);


static struct attribute *bq24262_wmport_attributes[] = {
	&dev_attr_wake_lock_charging.attr,
	&dev_attr_fully_weakly.attr,
	&dev_attr_usb_sus_mode.attr,
	&dev_attr_force_dcin.attr,
	&dev_attr_battery_uv.attr,
	&dev_attr_battery_mv.attr,
	&dev_attr_battery_temp_raw.attr,
	&dev_attr_battery_temp_mv.attr,
	&dev_attr_charge_timer_max_sec.attr,
	&dev_attr_suspend_dynamic_period_sec.attr,
	&dev_attr_fake_capacity.attr,
	&dev_attr_diag_charger_det.attr,
	&dev_attr_diag_draw.attr,
	NULL,
};

static const struct attribute_group bq24262_wmport_attr_group = {
	.attrs = bq24262_wmport_attributes,
};

static char *bq24262_wmport_supplied_to[] = {
	bq24262_wmport_battery_name,
};


STATIC_FUNC void bq24262_wmport_gpio_configure(struct bq24262_wmport_context *bc)
{	unsigned long				gpio;
	unsigned int				irq;
	struct bq24262_wmport_platform_data	*pd;

	pd = bc->platform_data;

	/* power path DCIN */
	irq =  pd->dc_xdet_eint;
	gpio = pd->dc_xdet_gpio;
	PRINTK_XX("%s: Configure DC_XDET. dc_xdet=%lu:%u\n",
		__func__,
		BQ24262_WMPORT_GPIO_DECODE(gpio),irq
	);
	/* power path DCIN interrupt */
	mt_eint_mask(irq);
	/* power path DCIN gpio */
	mt_set_gpio_dir(gpio, 		GPIO_DIR_IN);
	mt_set_gpio_mode(gpio,		GPIO_MODE_00);
	mt_set_gpio_pull_select(gpio,	GPIO_PULL_UP);
	mt_set_gpio_pull_enable(gpio,	GPIO_PULL_ENABLE);

	/* power path VBUS */
	irq =  pd->vbus_xdet_eint;
	gpio = pd->vbus_xdet_gpio;
	PRINTK_XX("%s: Configure VBUS_XDET. vbus_xdet=%lu:%u\n",
		__func__,
		BQ24262_WMPORT_GPIO_DECODE(gpio),irq
	);
	/* power path VBUS interrupt */
	mt_eint_mask(irq);
	/* power path VBUS gpio */
	mt_set_gpio_dir(gpio, 		GPIO_DIR_IN);
	mt_set_gpio_mode(gpio,		GPIO_MODE_00);
	mt_set_gpio_pull_select(gpio,	GPIO_PULL_UP);
	mt_set_gpio_pull_enable(gpio,	GPIO_PULL_ENABLE);

	/* power path FORCE_DCIN  */
	gpio = pd->force_dcin_gpio;
	PRINTK_XX("%s: Configure FORCE_DCIN. force_dcin=%lu\n",
		__func__,
		BQ24262_WMPORT_GPIO_DECODE(gpio)
	);
	mt_set_gpio_out(gpio, 		BQ24262_WMPORT_FORCE_DCIN_AUTO);
	mt_set_gpio_dir(gpio, 		GPIO_DIR_OUT);
	mt_set_gpio_mode(gpio,		GPIO_MODE_00);
	mt_set_gpio_pull_select(gpio,	GPIO_PULL_DOWN);
	mt_set_gpio_pull_enable(gpio,	GPIO_PULL_DISABLE);

	/* charger(PMIC, CMIC) SYS_WAK_STAT */
	gpio = pd->sys_wak_stat_gpio;
	PRINTK_XX("%s: Configure SYS_WAK_STAT. sys_wak_stat=%lu\n",
		__func__,
		BQ24262_WMPORT_GPIO_DECODE(gpio)
	);
	mt_set_gpio_out(gpio,		BQ24262_WMPORT_SYS_WAK_STAT_READY);
	mt_set_gpio_dir(gpio, 		GPIO_DIR_OUT);
	mt_set_gpio_mode(gpio,		GPIO_MODE_00);
	mt_set_gpio_pull_select(gpio,	GPIO_PULL_UP);
	mt_set_gpio_pull_enable(gpio,	GPIO_PULL_DISABLE);

	/* charger(CMIC) suspend. */
	gpio = pd->chg_suspend_gpio;
	PRINTK_XX("%s: Configure CHG_SUSPEND. chg_suspend=%lu\n",
		__func__,
		BQ24262_WMPORT_GPIO_DECODE(gpio)
	);
	/* @note intentionally keep CHG_SUSPEND pin level. */
	mt_set_gpio_dir(gpio, 		GPIO_DIR_OUT);
	mt_set_gpio_mode(gpio,		GPIO_MODE_00);
	mt_set_gpio_pull_select(gpio,	GPIO_PULL_UP);
	mt_set_gpio_pull_enable(gpio,	GPIO_PULL_DISABLE);

	/* charger open drain /STAT */
	irq =  pd->chg_xstat_eint;
	gpio = pd->chg_xstat_gpio;
	PRINTK_XX("%s: Configure CHG_XSTAT. chg_xstat=%lu:%u\n",
		__func__,
		BQ24262_WMPORT_GPIO_DECODE(gpio),irq
	);
	/* charger open drain /STAT interrupt eint */
	if (irq != BQ24262_WMPORT_INVALID_EINT) {
		mt_eint_mask(irq);
	}
	/* charger open drain /STAT interrupt gpio */
	if (gpio != BQ24262_WMPORT_INVALID_GPIO) {
		mt_set_gpio_dir(gpio, 		GPIO_DIR_IN);
		mt_set_gpio_mode(gpio,		GPIO_MODE_00);
		mt_set_gpio_pull_select(gpio,	GPIO_PULL_UP);
		mt_set_gpio_pull_enable(gpio,	GPIO_PULL_ENABLE);
	}

	return;
}

static void bq24262_wmport_initialize_battery_table(void)
{
	switch (icx_pm_helper_modelid & 0xFFFF0000) {
		case 0x20000000:
		case 0x21000000:
			battery_table[0].voltage = BTT_1_LOW;
			battery_table[1].voltage = BTT_1_BLINK;
			battery_table[2].voltage = BTT_1_GAUGE1;
			battery_table[3].voltage = BTT_1_GAUGE2;
			battery_table[4].voltage = BTT_1_GAUGE3;
			battery_table[5].voltage = BTT_1_FULL;
			break;
		case 0x22000000:
			battery_table[0].voltage = BTT_2_LOW;
			battery_table[1].voltage = BTT_2_BLINK;
			battery_table[2].voltage = BTT_2_GAUGE1;
			battery_table[3].voltage = BTT_2_GAUGE2;
			battery_table[4].voltage = BTT_2_GAUGE3;
			battery_table[5].voltage = BTT_2_FULL;
			break;
		default:
			/* use default table */
			break;
	}
}

void bq24262_wmport_set_ulp_mode(bool ulp_mode)
{
	struct bq24262_wmport_static_context * bcst;
	struct bq24262_wmport_context * bc;
	unsigned long flags_bcst;

	/* PRINTK_LI("%s\n", __func__); */

	bcst = &bq24262_wmport_static;

	down(&(bcst->sem));

	spin_lock_irqsave(&(bcst->lock), flags_bcst);
	bc = bcst->bc_locked;
	spin_unlock_irqrestore(&(bcst->lock), flags_bcst);

	if (!bc) {
		pr_err("%s: Null points driver context.\n", __func__);
		up(&(bcst->sem));
		return;
	}

	down(&(bc->status_sem));

	bc->ulp_mode=ulp_mode;

	if(bc->early_sus && !bc->ulp_mode)
		bc->usb_sus_mode = BQ24262_WMPORT_USB_SUS_DRAW;
	else
		bc->usb_sus_mode = BQ24262_WMPORT_USB_SUS_NORMAL;

	up(&(bc->status_sem));

	bq24262_wmport_thr_main_event(bc);

	up(&(bcst->sem));

	return;
}

static void bq24262_wmport_early_suspend(struct early_suspend *h)
{
	struct bq24262_wmport_context *bc = container_of(h, struct bq24262_wmport_context, es_info);

	/* PRINTK_LI("%s\n", __func__); */

	down(&(bc->status_sem));
	bc->early_sus=true;
	if(bc->ulp_mode)
		bc->usb_sus_mode = BQ24262_WMPORT_USB_SUS_NORMAL;
	else
		bc->usb_sus_mode = BQ24262_WMPORT_USB_SUS_DRAW;
	up(&(bc->status_sem));
	bq24262_wmport_thr_main_event(bc);

	return;
}

static void bq24262_wmport_late_resume(struct early_suspend *h)
{
	struct bq24262_wmport_context *bc = container_of(h, struct bq24262_wmport_context, es_info);

	/* PRINTK_LI("%s\n", __func__); */

	down(&(bc->status_sem));
	bc->early_sus=false;
	bc->usb_sus_mode = BQ24262_WMPORT_USB_SUS_NORMAL;
	up(&(bc->status_sem));
	bq24262_wmport_thr_main_event(bc);

	return;
}

/*! probe device.
*/

STATIC_FUNC int bq24262_wmport_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{	struct i2c_adapter			*adapter = to_i2c_adapter(client->dev.parent);
	struct bq24262_wmport_platform_data	*pd = client->dev.platform_data;
	struct bq24262_wmport_context		*bc;
	struct bq24262_wmport_static_context	*bcst;
	unsigned long				now;
	unsigned long				flags;

	int					result;
	int					ret;

	result = 0;
	now = jiffies;
	bcst = &bq24262_wmport_static;

	bq24262_wmport_initialize_battery_table();

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
		pr_err("%s: I2C adapter is not functional.\n", __func__);
		return -EIO;
	}

	if (!pd) {
		dev_err(&(client->dev), "platform_data is NULL.\n");
		return -EINVAL;
	}

	bc = kzalloc(sizeof(*bc), GFP_KERNEL);
	if (!bc) {
		pr_err("%s: Can not allocate driver context.\n", __func__);
		return -ENOMEM;
	}
	PRINTK_LI("%s: Allocate charger context. bc=0x%p\n", __func__, bc);
	bc->dev = &(client->dev);
	bc->charger_client = client;
	spin_lock_init(&(bc->lock));
	sema_init(&(bc->status_sem), 1);
	sema_init(&(bc->usb_sem), 1);
	sema_init(&(bc->wake_lock_sem), 1);
	sema_init(&(bc->charger_sem), 1);
	sema_init(&(bc->charger_i2c_sem), 1);
	bc->platform_data = kmemdup(pd, sizeof(*pd), GFP_KERNEL);
	if (!(bc->platform_data)) {
		pr_err("%s: Can not duplicate platform data.\n", __func__);
		kfree(bc);
		return -ENOMEM;
	}
	bc->power_source_irq =          BQ24262_WMPORT_DUMMY;
	bc->power_source_raw_irq  =     BQ24262_WMPORT_DUMMY;
	bc->power_source_irq_prev =     BQ24262_WMPORT_DUMMY;
	bc->power_source_raw_irq_prev = BQ24262_WMPORT_DUMMY;
	bc->power_source_prev =         BQ24262_WMPORT_DUMMY;
	bc->power_source_raw_prev =     BQ24262_WMPORT_DUMMY;
	bc->power_source_usb_conn =     BQ24262_WMPORT_DUMMY;

	i2c_set_clientdata(client, bc);
	bq24262_wmport_gpio_configure(bc);

	ret = bq24262_wmport_i2c_read(bc, BQ24262_VENDOR, &(bc->charger_vendor));
	if (ret < 0) {
		pr_err("%s: Can not read VENDOR. ret=%d\n", __func__, ret);
		result = -ENODEV;
		goto out_free_mem;
	}

	PRINTK_LI("%s: Read BQ24262 VENDOR. vendor=0x%.2x\n", __func__, bc->charger_vendor);

	bc->charger_ps_status = POWER_SUPPLY_STATUS_DISCHARGING;
	bc->wake_lock_source =    BQ24262_WMPORT_WAKE_LOCK_CHARGING;
	bc->wake_lock_flags = 0;
	/* initialize wake lock */
	wake_lock_init(&(bc->wake_lock_resume), WAKE_LOCK_SUSPEND, bq24262_wmport_wake_lock_resume_name);
	wake_lock_init(&(bc->wake_lock_charging), WAKE_LOCK_SUSPEND, bq24262_wmport_wake_lock_charging_name);

	bc->jiffies_now = now;
	bc->jiffies_probe = now;
	bc->jiffies_thr_main_wake_trace = now;
	bc->jiffies_probe_expire = now
		+ msecs_to_jiffies(
		BQ24262_WMPORT_THR_MAIN_TIMEOUT_UNKNOWN_EXPIRE_MS
	);

	bc->jiffies_sys_running = 0;
	bc->jiffies_sys_running_expire = 0;

	bc->charge_timer_state = BQ24262_WMPORT_CHARGE_TIMER_WAIT_SYSTEM_RUNNING;
	bc->charge_timer_max_sec = pd->charge_time_max_sec;
	memset(&(bc->charge_timer_now_ts), 0,   sizeof(bc->charge_timer_now_ts));
	memset(&(bc->charge_timer_start_ts), 0, sizeof(bc->charge_timer_start_ts));
	bc->dynamic_period = -1L;

	bc->ppath_work_wake_count = 0;

	bc->fake_capacity = BQ24262_WMPORT_FAKE_CAPACITY_OFF;

	bc->diag_state = BQ24262_WMPORT_DIAG_OFF;

	/* @note Use same work handler function bq24262_wmport_ppath_work()
	         for power source DCIN and power source USB.
	*/
	INIT_WORK(&(bc->ppath_work), bq24262_wmport_ppath_work);
	INIT_WORK(&(bc->charger_work), bq24262_wmport_charger_work);
	if (bq24262_wmport_thr_main_prepare_start(bc) != 0) {
		pr_err("%s: Can not prepare to start power source thread.\n", __func__);
		goto out_free_mem;
	}
	if (bq24262_wmport_thr_vbus_prepare_start(bc_to_vb(bc)) != 0) {
		pr_err("%s: Can not prepare to thread vbus.\n", __func__);
		goto out_free_mem;
	}

	/* Register Power Management notifier. */

	bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT].ps.name =	 		bq24262_wmport_battery_name;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT].ps.supplied_to = 		NULL;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT].ps.num_supplicants =	0;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT].ps.type = 			POWER_SUPPLY_TYPE_BATTERY;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT].ps.properties =		bq24262_wmport_battery_props;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT].ps.num_properties =	ARRAY_SIZE(bq24262_wmport_battery_props);
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT].ps.get_property =		bq24262_wmport_battery_get_property;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT].ps.external_power_changed = NULL;
	set_bit(POWER_SUPPLY_RL_SUSPEND_MASK, &(bc->power_supply_rl[BQ24262_WMPORT_PSRL_BAT].flags));

	bc->power_supply_rl[BQ24262_WMPORT_PSRL_USB].ps.name = 			bq24262_wmport_usb_name;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_USB].ps.supplied_to =		bq24262_wmport_supplied_to;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_USB].ps.num_supplicants = 	ARRAY_SIZE(bq24262_wmport_supplied_to);
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_USB].ps.type =			POWER_SUPPLY_TYPE_USB;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_USB].ps.properties = 		bq24262_wmport_usb_props;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_USB].ps.num_properties =	ARRAY_SIZE(bq24262_wmport_usb_props);
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_USB].ps.get_property =		bq24262_wmport_usb_get_property;
	clear_bit(POWER_SUPPLY_RL_SUSPEND_MASK, &(bc->power_supply_rl[BQ24262_WMPORT_PSRL_USB].flags));

	bc->power_supply_rl[BQ24262_WMPORT_PSRL_DC].ps.name = 			bq24262_wmport_dc_name;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_DC].ps.supplied_to = 		bq24262_wmport_supplied_to;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_DC].ps.num_supplicants = 	ARRAY_SIZE(bq24262_wmport_supplied_to);
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_DC].ps.type = 			POWER_SUPPLY_TYPE_MAINS;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_DC].ps.properties = 		bq24262_wmport_dc_props;
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_DC].ps.num_properties = 	ARRAY_SIZE(bq24262_wmport_dc_props);
	bc->power_supply_rl[BQ24262_WMPORT_PSRL_DC].ps.get_property = 		bq24262_wmport_dc_get_property;
	clear_bit(POWER_SUPPLY_RL_SUSPEND_MASK, &(bc->power_supply_rl[BQ24262_WMPORT_PSRL_DC].flags));

	if (power_supply_rl_array_register(&(bc->power_supply_rl[0]), BQ24262_WMPORT_PSRL_NUM, now, bc->dev) != 0) {
		/* Can't register all power supply changed notifier. */
		goto out_unreg_power_supply;
	}

	/* Start thread main. */
	if (bq24262_wmport_thr_main_start(bc) != 0) {
		pr_err("%s: Can not start power source thread.\n", __func__);
		goto out_unreg_power_supply;
	}

	/* Initial configuration update. */
	bq24262_wmport_thr_main_event(bc);

	/* Start thread vbus. */
	if (bq24262_wmport_thr_vbus_start(bc_to_vb(bc)) != 0) {
		pr_err("%s: Can not start thread vbus.\n", __func__);
		goto out_terminate_thr_main;
	}

	down(&(bcst->sem));
	spin_lock_irqsave(&(bcst->lock), flags);
	if (bcst->bc == NULL) {
		/* 1st device. */
		bcst->bc = bc;
		bcst->bc_locked = bc;
	} else {
		/* 2nd, 3rd, ... or Nth device. */
		/* Why come here. */
		pr_err("%s: Unexpected two or more devices to probe.\n", __func__);
	}
	spin_unlock_irqrestore(&(bcst->lock), flags);
#if (defined(REGMON_DEBUG))
	bq24262_wmport_customer_info.private_data = bc;
	regmon_add(&bq24262_wmport_customer_info);
#endif /* (defined(REGMON_DEBUG)) */
	up(&(bcst->sem));

	PRINTK_XX("%s: Setup power path IRQ\n", __func__);
	bq24262_wmport_ppath_irq_request(bc);
	bq24262_wmport_ppath_irq_enable(bc);
	PRINTK_XX("%s: Setup charger IRQ.\n", __func__);
	bq24262_wmport_charger_irq_request(bc);
	bq24262_wmport_charger_irq_enable(bc);
	PRINTK_XX("%s: Register pm notifier.\n", __func__);
	/* register pm notifier */
	bc->pm_nb.notifier_call = bq24262_wmport_pm_notifier;
	bc->pm_nb.priority = 0;
	register_pm_notifier(&(bc->pm_nb));

	PRINTK_XX("%s: Create sysfs entry.\n", __func__);
	ret = sysfs_create_group(&(client->dev.kobj), &bq24262_wmport_attr_group);
	if (ret) {
		pr_err("%s: Could not create sysfs files. ret=%d\n", __func__, ret);
	}
	PRINTK_FUNC_ENTRY("%s: Exit.\n", __func__);

	bc->es_info.suspend = bq24262_wmport_early_suspend;
	bc->es_info.resume = bq24262_wmport_late_resume;
	/* bc->es_info.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN; */
	bc->es_info.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	register_early_suspend(&bc->es_info);

	return 0;

out_terminate_thr_main:
	bq24262_wmport_thr_main_terminate(bc);
out_unreg_power_supply:
	power_supply_rl_array_unregister(&(bc->power_supply_rl[0]), BQ24262_WMPORT_PSRL_NUM);
out_free_mem:
	i2c_set_clientdata(client, NULL);
	/* Free copied platform_data */
	kfree(bc->platform_data);
	kfree(bc);

	return ret;
}

static int bq24262_wmport_remove(struct i2c_client *client)
{	struct bq24262_wmport_static_context	*bcst;
	struct bq24262_wmport_context		*bc;
	unsigned long				flags;

	bcst = &bq24262_wmport_static;
	bc = i2c_get_clientdata(client);

	unregister_early_suspend(&bc->es_info);

	sysfs_remove_group(&(client->dev.kobj), &bq24262_wmport_attr_group);
	unregister_pm_notifier(&(bc->pm_nb));

	bq24262_wmport_charger_irq_free(bc);
	bq24262_wmport_ppath_irq_free(bc);

	flush_scheduled_work();
	bq24262_wmport_thr_vbus_terminate(bc_to_vb(bc));
	bq24262_wmport_thr_main_terminate(bc);
	power_supply_rl_array_unregister(&(bc->power_supply_rl[0]), BQ24262_WMPORT_PSRL_NUM);

	down(&(bcst->sem));
#if (defined(REGMON_DEBUG))
	bq24262_wmport_customer_info.private_data = NULL;
	regmon_del(&bq24262_wmport_customer_info);
#endif /* (defined(REGMON_DEBUG)) */
	spin_lock_irqsave(&(bcst->lock), flags);
	bcst->bc = NULL;
	bcst->bc_locked = NULL;
	spin_unlock_irqrestore(&(bcst->lock), flags);
	up(&(bcst->sem));

	wake_lock_destroy(&(bc->wake_lock_resume));
	wake_lock_destroy(&(bc->wake_lock_charging));
	i2c_set_clientdata(client, NULL);
	kfree(bc->platform_data);
	kfree(bc);
	return 0;
}

/* battery polling period in suspend,
   while charging poll NTC(thermal sensor), poll every 2min.
   charged enough, wake 1 times per 1 day.
   charged less, wake 1 times per 12 hour.
*/

#define	BQ24262_WMPORT_SPM_WAKE_DYNAMIC_PERIOD_CHARGING		(120)
#define	BQ24262_WMPORT_SPM_WAKE_DYNAMIC_PERIOD_ENOUGH		(24*3600)
#define	BQ24262_WMPORT_SPM_WAKE_DYNAMIC_PERIOD_ENOUGH_CAPACITY	(10)
#define	BQ24262_WMPORT_SPM_WAKE_DYNAMIC_PERIOD_LESS		(6*3600)

/*! Return seconds to resume.
    @note this function link to mt_spm_sleep.c
*/
int bq24262_wmport_get_dynamic_period(int first_use, int first_wakeup_time, int battery_capacity_level)
{	struct bq24262_wmport_static_context	*bcst;
	struct bq24262_wmport_context		*bc;
	int					result;
	long					battery_uv;
	int					battery_capacity;
	unsigned long				flags_bcst;

	result = BQ24262_WMPORT_SPM_WAKE_DYNAMIC_PERIOD_ENOUGH;
	bcst = &bq24262_wmport_static;
	down(&(bcst->sem));
	spin_lock_irqsave(&(bcst->lock), flags_bcst);
	bc = bcst->bc_locked;
	spin_unlock_irqrestore(&(bcst->lock), flags_bcst);
	if (!bc) {
		pr_err("%s: Null points driver context.\n", __func__);
		/* Can't handle this interrupt any more. */
		goto out;
	}
	down(&(bc->status_sem));
	battery_uv = bc->battery_uv;
	battery_capacity = bq24262_wmport_battery_uv_to_capacity_trivial_unlocked(bc, battery_uv);
	if (bq24262_wmport_is_charging_for_wake_lock(bc->battery_charge, bc->config)) {
		/* Now charging, but we DON'T expect suspend while charging.
		   We go to suspend, even if charging. we resume periodically very short interval.
		*/
		result = BQ24262_WMPORT_SPM_WAKE_DYNAMIC_PERIOD_CHARGING;
	} else {
		/* Not charging, watch discharging a little. */
		if (battery_capacity >= BQ24262_WMPORT_SPM_WAKE_DYNAMIC_PERIOD_ENOUGH_CAPACITY) {
			result = BQ24262_WMPORT_SPM_WAKE_DYNAMIC_PERIOD_ENOUGH;
		} else {
			result = BQ24262_WMPORT_SPM_WAKE_DYNAMIC_PERIOD_LESS;
		}
	}
	bc->dynamic_period = result;
	up(&(bc->status_sem));
out:
	up(&(bcst->sem));
	PRINTK_WL("%s: Request SPM timer. result=%d\n", __func__, result);
	return result;
}


#ifdef CONFIG_PM
static int bq24262_wmport_suspend(struct device *dev)
{
	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);


	PRINTK_LI("%s: Called.\n",__func__);

	bq24262_wmport_thr_vbus_suspend(bc_to_vb(bc));
	bq24262_wmport_thr_main_suspend(bc);

	bq24262_wmport_charger_irq_suspend(bc);
	bq24262_wmport_ppath_irq_suspend(bc);

	return 0;
}

static int bq24262_wmport_resume(struct device *dev)
{
	struct i2c_client		*client = to_i2c_client(dev);
	struct bq24262_wmport_context	*bc = i2c_get_clientdata(client);

	PRINTK_LI("%s: Called.\n",__func__);

	/* Take a look around, power source and configuration. */
	bq24262_wmport_thr_main_resume(bc);
	bq24262_wmport_thr_vbus_resume(bc_to_vb(bc));

	bq24262_wmport_ppath_irq_resume(bc);
	bq24262_wmport_charger_irq_resume(bc);

	return 0;
}

static void bq24262_wmport_shutdown(struct i2c_client *client)
{
	struct bq24262_wmport_static_context	*bcst = &bq24262_wmport_static;
	struct bq24262_wmport_context		*bc = i2c_get_clientdata(client);

	unsigned long	flags;

	PRINTK_LI("%s: Called.\n",__func__);

	bq24262_wmport_charger_irq_free(bc);
	bq24262_wmport_ppath_irq_free(bc);

	bq24262_wmport_thr_vbus_shutdown(bc_to_vb(bc));
	bq24262_wmport_thr_main_shutdown(bc);
	/* We keep power supplies are registerd, because it may call
	   power supply property functions at shutdown process.
	*/
	set_bit(BQ24262_WMPORT_SHUTDOWN, &(bc->flags));
	down(&(bcst->sem));
	spin_lock_irqsave(&(bcst->lock), flags);
	bcst->bc = NULL;
	bcst->bc_locked = NULL;
	spin_unlock_irqrestore(&(bcst->lock), flags);
	up(&(bcst->sem));
	/* Keep context memory. */

	/* HiZ and reset. */
	PRINTK_LI("reset BQ24262.\n");
	bq24262_wmport_chg_suspend_write(bc, BQ24262_WMPORT_CHG_SUSPEND_HIZ);
	bq24262_wmport_i2c_write(bc, BQ24262_CONTROL, BQ24262_CONTROL_RESET);
	return;
}

#else
#define bq24262_wmport_suspend	(NULL)
#define bq24262_wmport_resume	(NULL)
#define bq24262_wmport_shutdown	(NULL)
#endif /* CONFIG_PM */

/*! Device id table.
*/
static const struct i2c_device_id bq24262_wmport_id[] = {
	{ DEVICE_NAME_TO_PROBE, 0 },
	{ /* Terminator */ }
};

static const struct dev_pm_ops bq24262_wmport_pm_ops = {
	.suspend	= bq24262_wmport_suspend,
	.resume		= bq24262_wmport_resume,
};

static struct i2c_driver bq24262_wmport_driver = {
	.probe		= bq24262_wmport_probe,
	.remove		= bq24262_wmport_remove,
	.shutdown	= bq24262_wmport_shutdown,
	.id_table	= bq24262_wmport_id,
	.driver		= {
		.name	= DEVICE_NAME_TO_PROBE,
		.pm	= &bq24262_wmport_pm_ops,
	},
};

static int __init bq24262_wmport_init(void)
{
	struct bq24262_wmport_static_context	*bcst;

	PRINTK_LI("%s: Called.", __func__);

	bcst = &bq24262_wmport_static;
	sema_init(&(bcst->sem), 1);
	spin_lock_init(&(bcst->lock));

	return i2c_add_driver(&bq24262_wmport_driver);
}
module_init(bq24262_wmport_init);

static void __exit bq24262_wmport_exit(void)
{
	i2c_del_driver(&bq24262_wmport_driver);
}
module_exit(bq24262_wmport_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS("i2c:bq24262_wmport");
MODULE_AUTHOR("SONY");
