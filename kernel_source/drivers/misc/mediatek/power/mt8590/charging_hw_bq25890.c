#include <mach/charging.h>
#include <mach/upmu_common.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <mach/mt_boot.h>
#include <mach/battery_meter.h>
#include "bq25890.h"
#include <mach/mt_sleep.h>
/* ============================================================ // */
/* Define */
/* ============================================================ // */
#define STATUS_OK    0
#define STATUS_UNSUPPORTED    -1
#define GETARRAYNUM(array) (sizeof(array)/sizeof(array[0]))

/* ============================================================ // */
/* Global variable */
/* ============================================================ // */
#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
static CHARGER_TYPE g_charger_type = CHARGER_UNKNOWN;
#endif

kal_bool charging_type_det_done = KAL_TRUE;

/*BQ25890 REG06 VREG[5:0]*/
const unsigned int VBAT_CV_VTH[] = {
	3840000, 3856000, 3872000, 3888000,
	3904000, 3920000, 3936000, 3952000,
	3968000, 3984000, 4000000, 4016000,
	4032000, 4048000, 4064000, 4080000,
	4096000, 4112000, 4128000, 4144000,
	4160000, 4176000, 4192000, 4208000,
	4224000, 4240000, 4256000, 4272000,
	4288000, 4304000, 4320000, 4336000,
	4352000, 4368000, 4384000, 4400000,
	4416000, 4432000, 4448000, 4464000,
	4480000, 4496000, 4512000, 4528000,
	4544000, 4560000, 4576000, 4592000,
	4608000
};

/*BQ25890 REG04 ICHG[6:0]*/
const unsigned int CS_VTH[] = {
	0, 6400, 12800, 19200,
	25600, 32000, 38400, 44800,
	51200, 57600, 64000, 70400,
	76800, 83200, 89600, 96000,
	102400, 108800, 115200, 121600,
	128000, 134400, 140800, 147200,
	153600, 160000, 166400, 172800,
	179200, 185600, 192000, 198400,
	204800, 211200, 217600, 224000,
	230400, 236800, 243200, 249600,
	256000, 262400, 268800, 275200,
	281600, 288000, 294400, 300800,
	307200, 313600, 320000, 326400,
	332800, 339200, 345600, 352000,
	358400, 364800, 371200, 377600,
	384000, 390400, 396800, 403200,
	409600, 416000, 422400, 428800,
	435200, 441600, 448000, 454400,
	460800, 467200, 473600, 480000,
	486400, 492800, 499200, 505600
};

/*BQ25890 REG00 IINLIM[5:0]*/
const unsigned int INPUT_CS_VTH[] = {
	10000, 15000, 20000, 25000,
	30000, 35000, 40000, 45000,
	50000, 55000, 60000, 65000,
	70000, 75000, 80000, 85000,
	90000, 95000, 100000, 105000,
	110000, 115000, 120000, 125000,
	130000, 135000, 140000, 145000,
	150000, 155000, 160000, 165000,
	170000, 175000, 180000, 185000,
	190000, 195000, 200000, 200500,
	210000, 215000, 220000, 225000,
	230000, 235000, 240000, 245000,
	250000, 255000, 260000, 265000,
	270000, 275000, 280000, 285000,
	290000, 295000, 300000, 305000,
	310000, 315000, 320000, 325000
};

const unsigned int VCDT_HV_VTH[] = {
	BATTERY_VOLT_04_200000_V, BATTERY_VOLT_04_250000_V, BATTERY_VOLT_04_300000_V,
	BATTERY_VOLT_04_350000_V,
	BATTERY_VOLT_04_400000_V, BATTERY_VOLT_04_450000_V, BATTERY_VOLT_04_500000_V,
	BATTERY_VOLT_04_550000_V,
	BATTERY_VOLT_04_600000_V, BATTERY_VOLT_06_000000_V, BATTERY_VOLT_06_500000_V,
	BATTERY_VOLT_07_000000_V,
	BATTERY_VOLT_07_500000_V, BATTERY_VOLT_08_500000_V, BATTERY_VOLT_09_500000_V,
	BATTERY_VOLT_10_500000_V
};

/* ============================================================ // */
/* function prototype */
/* ============================================================ // */


/* ============================================================ // */
/* extern variable */
/* ============================================================ // */

/* ============================================================ // */
/* extern function */
/* ============================================================ // */
extern unsigned int upmu_get_reg_value(unsigned int reg);
extern bool mt_usb_is_device(void);
extern void Charger_Detect_Init(void);
extern void Charger_Detect_Release(void);
extern CHARGER_TYPE hw_charger_type_detection(void);
extern void mt_power_off(void);

/* ============================================================ // */
unsigned int charging_value_to_parameter(const unsigned int *parameter, const unsigned int array_size,
				       const unsigned int val)
{
	if (val < array_size)
		return parameter[val];

		battery_log(BAT_LOG_CRTI, "Can't find the parameter \r\n");
		return parameter[0];

}

unsigned int charging_parameter_to_value(const unsigned int *parameter, const unsigned int array_size,
				       const unsigned int val)
{
	unsigned int i;

	battery_log(BAT_LOG_FULL, "array_size = %d \r\n", array_size);

	for (i = 0; i < array_size; i++) {
		if (val == *(parameter + i))
			return i;
	}

	battery_log(BAT_LOG_CRTI, "NO register value match \r\n");
	/* TODO: ASSERT(0);    // not find the value */
	return 0;
}

static unsigned int bmt_find_closest_level(const unsigned int *pList, unsigned int number,
					 unsigned int level)
{
	unsigned int i;
	unsigned int max_value_in_last_element;

	if (pList[0] < pList[1])
		max_value_in_last_element = KAL_TRUE;
	else
		max_value_in_last_element = KAL_FALSE;

	if (max_value_in_last_element == KAL_TRUE) {
		for (i = (number - 1); i != 0; i--) {	/* max value in the last element */
			if (pList[i] <= level) {
				battery_log(2, "zzf_%d<=%d     i=%d\n", pList[i], level, i);
				return pList[i];
			}
		}

		battery_log(BAT_LOG_CRTI, "Can't find closest level \r\n");
		return pList[0];
		/* return CHARGE_CURRENT_0_00_MA; */
	} else {
		for (i = 0; i < number; i++) {	/* max value in the first element */
			if (pList[i] <= level)
				return pList[i];
		}

		battery_log(BAT_LOG_CRTI, "Can't find closest level \r\n");
		return pList[number - 1];
		/* return CHARGE_CURRENT_0_00_MA; */
	}
}

static unsigned int charging_hw_init(void *data)
{
	unsigned int status = STATUS_OK;

	bq25890_config_interface(bq25890_CON2, 0x1, 0x1, 4);	/* disable ico Algorithm -->bear:en */
	bq25890_config_interface(bq25890_CON2, 0x0, 0x1, 3);	/* disable HV DCP for gq25897 */
	bq25890_config_interface(bq25890_CON2, 0x0, 0x1, 2);	/* disbale MaxCharge for gq25897 */
	bq25890_config_interface(bq25890_CON2, 0x0, 0x1, 1);	/* disable DPDM detection */

	bq25890_config_interface(bq25890_CON7, 0x1, 0x3, 4);	/* enable  watch dog 40 secs 0x1 */
	bq25890_config_interface(bq25890_CON7, 0x1, 0x1, 3);	/* enable charging timer safety timer */
	bq25890_config_interface(bq25890_CON7, 0x2, 0x3, 1);	/* charging timer 12h */

	bq25890_config_interface(bq25890_CON2, 0x0, 0x1, 5);	/* boost freq 1.5MHz when OTG_CONFIG=1 */
	bq25890_config_interface(bq25890_CONA, 0xa, 0xF, 4);	/* boost voltagte 5.19V default */
	bq25890_config_interface(bq25890_CONA, 0x6, 0x7, 0);	/* boost current limit 2.15A */
#ifdef CONFIG_MTK_BIF_SUPPORT
	bq25890_config_interface(bq25890_CON8, 0x4, 0x7, 5);	/* enable ir_comp_resistance */
	bq25890_config_interface(bq25890_CON8, 0x7, 0x7, 2);	/* enable ir_comp_vdamp */
#else
	bq25890_config_interface(bq25890_CON8, 0x0, 0x7, 5);	/* disable ir_comp_resistance */
	bq25890_config_interface(bq25890_CON8, 0x0, 0x7, 2);	/* disable ir_comp_vdamp */
#endif
	bq25890_config_interface(bq25890_CON8, 0x3, 0x3, 0);	/* thermal 120 default */

	bq25890_config_interface(bq25890_CON9, 0x0, 0x1, 4);	/* JEITA_VSET: VREG-200mV */
	bq25890_config_interface(bq25890_CON7, 0x1, 0x1, 0);	/* JEITA_ISet : 20% x ICHG */

	bq25890_config_interface(bq25890_CON3, 0x5, 0x7, 1);	/* System min voltage default 3.5V */

	/*PreCC mode */
	bq25890_config_interface(bq25890_CON5, 0x1, 0xF, 4);	/* precharge current default 128mA */
	bq25890_config_interface(bq25890_CON6, 0x1, 0x1, 1);	/* precharge2cc voltage,BATLOWV, 3.0V */
	/*CC mode */
	bq25890_config_interface(bq25890_CON4, 0x08, 0x7F, 0);	/* ICHG (0x08)512mA --> (0x20)2.048mA */
	/*CV mode */
	bq25890_config_interface(bq25890_CON6, 0x20, 0x3F, 2);	/* VREG=CV 4.352V (default 4.208V) */
	bq25890_config_interface(bq25890_CON6, 0x0, 0x1, 0);	/* recharge voltage@VRECHG=CV-100MV */
	bq25890_config_interface(bq25890_CON7, 0x1, 0x1, 7);	/* disable ICHG termination detect */
	bq25890_config_interface(bq25890_CON5, 0x1, 0x7, 0);	/* termianation current default 128mA */
	/*Vbus current limit */
	bq25890_config_interface(bq25890_CON0, 0x3F, 0x3F, 0);	/* input current limit, IINLIM, 3.25A */
	bq25890_config_interface(bq25890_CON0, 0x01, 0x01, 6);	/* enable ilimit Pin */
	 /*DPM*/ bq25890_config_interface(bq25890_CON1, 0x6, 0xF, 0);	/* Vindpm offset  600MV */
	bq25890_config_interface(bq25890_COND, 0x1, 0x1, 7);	/* vindpm vth 0:relative 1:absolute */
	/* absolute VINDPM = 2.6 + code x 0.1 =4.5V;K2 24261 4.452V */
	bq25890_config_interface(bq25890_COND, 0x13, 0x7F, 0);

/*	upmu_set_rg_vcdt_hv_en(0);*/
	return status;
}

static unsigned int charging_get_bif_vbat(void *data);

static unsigned int charging_dump_register(void *data)
{
	unsigned int status = STATUS_OK;

	battery_log(BAT_LOG_FULL, "charging_dump_register\r\n");
	bq25890_dump_register();

	/*unsigned int vbat;
	   charging_get_bif_vbat(&vbat);
	   battery_log(BAT_LOG_CRTI,"[BIF] vbat=%d mV\n", vbat); */


	return status;
}

static unsigned int charging_enable(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int enable = *(unsigned int *) (data);

	if (KAL_TRUE == enable) {
		/* bq25890_config_interface(bq25890_CON3, 0x1, 0x1, 4); //enable charging */
		bq25890_set_en_hiz(0x0);
		bq25890_chg_en(enable);
	} else {
		/* bq25890_config_interface(bq25890_CON3, 0x0, 0x1, 4); //enable charging */
		bq25890_chg_en(enable);
		/*bq25890_set_en_hiz(0x1);*/
#ifdef CONFIG_USB_IF
		bq25890_set_en_hiz(0x1);
#endif
	}

	return status;
}

static unsigned int charging_set_cv_voltage(void *data)
{
	unsigned int status;
	unsigned short int array_size;
	unsigned int set_cv_voltage;
	unsigned short int register_value;
	/*static kal_int16 pre_register_value; */

	array_size = GETARRAYNUM(VBAT_CV_VTH);
	status = STATUS_OK;
	/*pre_register_value = -1; */
	battery_log(BAT_LOG_CRTI, "charging_set_cv_voltage set_cv_voltage=%d\n",
		    *(unsigned int *) data);
	set_cv_voltage = bmt_find_closest_level(VBAT_CV_VTH, array_size, *(unsigned int *) data);
	register_value =
	    charging_parameter_to_value(VBAT_CV_VTH, GETARRAYNUM(VBAT_CV_VTH), set_cv_voltage);
	battery_log(BAT_LOG_FULL, "charging_set_cv_voltage register_value=0x%x\n", register_value);
	bq25890_set_vreg(register_value);

	return status;
}


static unsigned int charging_get_current(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int array_size;
	/*unsigned char reg_value; */
	unsigned int val;

	/*Get current level */
	array_size = GETARRAYNUM(CS_VTH);
	val = bq25890_get_ichg();
	*(unsigned int *) data = val;
	/* *(unsigned int *)data = charging_value_to_parameter(CS_VTH,array_size,val); */

	return status;
}


static unsigned int charging_set_current(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int set_chr_current;
	unsigned int array_size;
	unsigned int register_value;
	unsigned int current_value = *(unsigned int *) data;

	array_size = GETARRAYNUM(CS_VTH);
	set_chr_current = bmt_find_closest_level(CS_VTH, array_size, current_value);
	register_value = charging_parameter_to_value(CS_VTH, array_size, set_chr_current);
	/* bq25890_config_interface(bq25890_CON4, register_value, 0x7F, 0); */
	bq25890_set_ichg(register_value);
	/*For USB_IF compliance test only when USB is in suspend(Ibus < 2.5mA) or unconfigured(Ibus < 70mA) states*/
#ifdef CONFIG_USBIF_COMPLIANCE
	if (current_value < CHARGE_CURRENT_100_00_MA)
		register_value = 0x7f;
	else
		register_value = 0x13;
	charging_set_vindpm(&register_value);
#endif

	return status;
}

static unsigned int charging_set_input_current(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int current_value = *(unsigned int *) data;
	unsigned int set_chr_current;
	unsigned int array_size;
	unsigned int register_value;

	/*if(current_value >= CHARGE_CURRENT_2500_00_MA)
	   {
	   register_value = 0x6;
	   }
	   else if(current_value == CHARGE_CURRENT_1000_00_MA)
	   {
	   register_value = 0x4;
	   }
	   else
	   { */
	array_size = GETARRAYNUM(INPUT_CS_VTH);
	set_chr_current = bmt_find_closest_level(INPUT_CS_VTH, array_size, current_value);
	register_value = charging_parameter_to_value(INPUT_CS_VTH, array_size, set_chr_current);
	/*} */

	/* bq25890_config_interface(bq25890_CON0, register_value, 0x3F, 0);//input  current */
	bq25890_set_iinlim(register_value);

	return status;
}

static unsigned int charging_get_charging_status(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned char reg_value;

	bq25890_read_interface(bq25890_CONB, &reg_value, 0x3, 3);	/* ICHG to BAT */

	if (reg_value == 0x3)	/* check if chrg done */
		*(unsigned int *) data = KAL_TRUE;
	else
		*(unsigned int *) data = KAL_FALSE;

	return status;
}

static unsigned int charging_reset_watch_dog_timer(void *data)
{
	unsigned int status = STATUS_OK;

	bq25890_config_interface(bq25890_CON3, 0x1, 0x1, 6);	/* reset watchdog timer */

	return status;
}

static unsigned int charging_set_hv_threshold(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int set_hv_voltage;
	unsigned int array_size;
	unsigned short int register_value;
	unsigned int voltage = *(unsigned int *) (data);

	array_size = GETARRAYNUM(VCDT_HV_VTH);
	set_hv_voltage = bmt_find_closest_level(VCDT_HV_VTH, array_size, voltage);
	register_value = charging_parameter_to_value(VCDT_HV_VTH, array_size, set_hv_voltage);
	upmu_set_rg_vcdt_hv_vth(register_value);

	return status;
}


static unsigned int charging_get_hv_status(void *data)
{
	unsigned int status = STATUS_OK;

	*(kal_bool *) (data) = upmu_get_rgs_vcdt_hv_det();
	return status;
}


static unsigned int charging_get_battery_status(void *data)
{
	unsigned int status = STATUS_OK;
#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
	*(kal_bool *) (data) = 0;
	battery_log(BAT_LOG_CRTI, "bat exist for evb\n");
#else
	upmu_set_baton_tdet_en(1);
	upmu_set_rg_baton_en(1);
	*(kal_bool*)(data) = upmu_get_rgs_baton_undet();
#endif
	return status;
}


static unsigned int charging_get_charger_det_status(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_MTK_FPGA)
	*(kal_bool *) (data) = 1;
	battery_log(BAT_LOG_CRTI, "chr exist for fpga\n");
#else
	*(kal_bool *) (data) = upmu_get_rgs_chrdet();
#endif
	return status;
}


kal_bool charging_type_detection_done(void)
{
	return charging_type_det_done;
}


static unsigned int charging_get_charger_type(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
	*(CHARGER_TYPE *) (data) = STANDARD_HOST;
#else
	charging_type_det_done = KAL_FALSE;
	*(CHARGER_TYPE *) (data) = hw_charger_type_detection();
	charging_type_det_done = KAL_TRUE;
	g_charger_type = *(CHARGER_TYPE *) (data);

#endif

	return status;
}

static unsigned int charging_get_is_pcm_timer_trigger(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
	*(kal_bool *) (data) = KAL_FALSE;
#else
	if (slp_get_wake_reason() == WR_PCM_TIMER)
		*(kal_bool *) (data) = KAL_TRUE;
	else
		*(kal_bool *) (data) = KAL_FALSE;

	battery_log(BAT_LOG_FULL, "slp_get_wake_reason=%d\n", slp_get_wake_reason());
#endif

	return status;
}

static unsigned int charging_set_platform_reset(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
	battery_log(BAT_LOG_CRTI, "charging_set_platform_reset\n");
	kernel_restart("battery service reboot system");
#endif

	return status;
}

static unsigned int charging_get_platform_boot_mode(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
	*(unsigned int *) (data) = get_boot_mode();

	battery_log(BAT_LOG_CRTI, "get_boot_mode=%d\n", get_boot_mode());
#endif

	return status;
}

static unsigned int charging_set_power_off(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
	/*added dump_stack to see who the caller is */
	dump_stack();
	battery_log(BAT_LOG_CRTI, "charging_set_power_off\n");
	kernel_power_off();
#endif

	return status;
}

static unsigned int charging_set_vindpm(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int v = *(unsigned int *) data;

	bq25890_set_VINDPM(v);

	return status;
}

static unsigned int charging_set_vbus_ovp_en(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int e = *(unsigned int *) data;

	upmu_set_rg_vcdt_hv_en(e);

	return status;
}

static unsigned int charging_get_bif_vbat(void *data)
{
	unsigned int status = STATUS_OK;
#ifdef CONFIG_MTK_BIF_SUPPORT
	int vbat = 0;

	/* turn on VBIF28 regulator*/
	/*bif_init();*/

	/*change to HW control mode*/
	/*pmic_set_register_value(MT6351_PMIC_RG_VBIF28_ON_CTRL, 0);
	pmic_set_register_value(MT6351_PMIC_RG_VBIF28_EN, 1);*/
	if (bif_checked != 1 || bif_exist == 1) {
		bif_ADC_enable();

		vbat = bif_read16(MW3790_VBAT);
		*(unsigned int *) (data) = vbat;
	}
	/*turn off LDO and change SW control back to HW control */
	/*pmic_set_register_value(MT6351_PMIC_RG_VBIF28_EN, 0);
	pmic_set_register_value(MT6351_PMIC_RG_VBIF28_ON_CTRL, 1);*/
#else
	*(unsigned int *) (data) = 0;
#endif
	return status;
}

static unsigned int charging_get_bif_tbat(void *data)
{
	unsigned int status = STATUS_OK;
#ifdef CONFIG_MTK_BIF_SUPPORT
	int tbat = 0;
	int ret;
	int tried = 0;

	mdelay(50);

	if (bif_exist == 1) {
		do {
			bif_ADC_enable();
			ret = bif_read8(MW3790_TBAT, &tbat);
			tried++;
			mdelay(50);
			if (tried > 3)
				break;
		} while (ret != 1);

		if (tried <= 3)
			*(int *) (data) = tbat;
		else
			status =  STATUS_UNSUPPORTED;
	}
#endif
	return status;
}

static unsigned int (* const charging_func[CHARGING_CMD_NUMBER])(void *data)=
 {
 	 charging_hw_init
	,charging_dump_register  	
	,charging_enable
	,charging_set_cv_voltage
	,charging_get_current
	,charging_set_current
	,charging_set_input_current
	,charging_get_charging_status
	,charging_reset_watch_dog_timer
	,charging_set_hv_threshold
	,charging_get_hv_status
	,charging_get_battery_status
	,charging_get_charger_det_status
	,charging_get_charger_type
	,charging_get_is_pcm_timer_trigger
	,charging_set_platform_reset
	,charging_get_platform_boot_mode
	,charging_set_power_off
 };

/*
* FUNCTION
*        Internal_chr_control_handler
*
* DESCRIPTION
*         This function is called to set the charger hw
*
* CALLS
*
* PARAMETERS
*        None
*
* RETURNS
*
*
* GLOBALS AFFECTED
*       None
*/
signed int chr_control_interface(CHARGING_CTRL_CMD cmd, void *data)
{
	signed int status;

	if (cmd < CHARGING_CMD_NUMBER) {
		if (charging_func[cmd] != NULL)
			status = charging_func[cmd](data);
		else {
			battery_log(BAT_LOG_CRTI, "[chr_control_interface]cmd:%d not supported\n", cmd);
			status = STATUS_UNSUPPORTED;
		}
	} else
		status = STATUS_UNSUPPORTED;

	return status;
}
