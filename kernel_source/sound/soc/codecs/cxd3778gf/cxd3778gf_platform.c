/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3778gf_platform.c
 *
 * CXD3778GF CODEC driver
 *
 * Copyright (c) 2013-2016 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* #define TRACE_PRINT_ON */
/* #define DEBUG_PRINT_ON */
#define TRACE_TAG "------- "
#define DEBUG_TAG "        "

#include "cxd3778gf_common.h"
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <../../../drivers/misc/mediatek/auxadc/mt_auxadc.h>
#define REGULATOR_180    (platform_data->regulator_180)
#define REGULATOR_285    (platform_data->regulator_285)
#define I2S0_DATA_OUT    (platform_data->port_i2s0_data_out)
#define I2S0_DATA_IN     (platform_data->port_i2s0_data_in)
#define I2S0_DATA_BCK    (platform_data->port_i2s0_data_bck)
#define I2S0_DATA_LRCK   (platform_data->port_i2s0_data_lrck)
#define I2S1_DATA_OUT    (platform_data->port_i2s1_data_out)
#define I2S1_DATA_IN     (platform_data->port_i2s1_data_in)
#define I2S1_DATA_BCK    (platform_data->port_i2s1_data_bck)
#define I2S1_DATA_LRCK   (platform_data->port_i2s1_data_lrck)
#define GPIO_ADA_LDO_EN  (platform_data->port_ada_ldo_en)
#define GPIO_ADA_XINT    (platform_data->port_ada_xint)
#define GPIO_SP_XSHTD    (platform_data->port_sp_xshtd)
#define GPIO_XPCM_DET    (platform_data->port_ada_xdet)
#define GPIO_ADA_XRST    (platform_data->port_ada_xrst)
#define GPIO_HP_XMUTE    (platform_data->port_hp_xmute)
#define GPIO_NC_CMPOUT   (platform_data->port_nc_cmpout)
#define GPIO_NC_DET      (platform_data->port_nc_det)
#define GPIO_HP_DET      (platform_data->port_hp_det)
#define GPIO_HP_SE_MUTE_ON   (platform_data->port_hp_se_mute_on)
#define GPIO_HP_SE_MUTE_OFF  (platform_data->port_hp_se_mute_off)
#define GPIO_HP_SE_MUTE_CP_ON  (platform_data->port_hp_se_mute_cp_on)
#define GPIO_HP_BTL_MUTE_ON  (platform_data->port_hp_btl_mute_on)
#define GPIO_HP_BTL_MUTE_OFF (platform_data->port_hp_btl_mute_off)
#define GPIO_HP_FM_SERIES_XMUTE (platform_data->port_hp_fm_series_xmute)
#define GPIO_ADA_FS480_EN  (platform_data->port_ada_fs480_en)
#define GPIO_ADA_FS441_EN  (platform_data->port_ada_fs441_en)
#define GPIO_HP_XMUTE4   (platform_data->port_hp_xmute4)
#define GPIO_AU_VL_EN  (platform_data->port_au_vl_en)
#define GPIO_AU_AVDD_EN  (platform_data->port_au_avdd_en)
#define GPIO_AU_BTL_5V_EN (platform_data->port_au_btl_5v_en)
#define GPIO_AU_BTL_5V_EN_2 (platform_data->port_au_btl_5v_en_2)
#define GPIO_AU_BTL_7V_EN (platform_data->port_au_btl_7v_en)
#define GPIO_AU_SE_POS_EN (platform_data->port_au_se_pos_en)
#define GPIO_AU_SE_NEG_EN (platform_data->port_au_se_neg_en)
#define GPIO_AU_SE_3V_EN (platform_data->port_au_se_3v_en)
#define EXT_CK1 (platform_data->port_ext_ck1)
#define GPIO_HP_BT_DET_L (platform_data->port_hp_btl_det_l)
#define GPIO_HP_BT_DET_R (platform_data->port_hp_btl_det_r)
#define BOARD_TYPE1 (platform_data->type1)
#define BOARD_TYPE2 (platform_data->type2)
#define CHK_RV(_msg) \
	if(rv<0){ \
		print_fail(_msg,rv); \
		return(-1); \
	}

static int initialized = FALSE;
static unsigned int board_type = TYPE_A;

/* temporary */
#define ICX_AD_BID3     (6)
#define ICX_AD_MAX      (0xfff)
#define ICX_AD_THRESHOLD_A      (0x2A1)
#define ICX_AD_THRESHOLD_B      (0x73B)
#define ICX_AD_THRESHOLD_C      (0xAB2)
#define ICX_AD_THRESHOLD_D      (0xF06)
int  board_set_flag = 0;

extern ulong icx_pm_helper_sysinfo;
#define GPIO_UPDATE_SYSINFO_MASK (0x00000020)

#define HP_DET_EINT 35  /* GPIO 53 */

static struct cxd3778gf_platform_data * platform_data = NULL;
static struct regulator * regulator_285 = NULL;
static struct regulator * regulator_180 = NULL;

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

int cxd3778gf_setup_platform(struct cxd3778gf_platform_data * data, unsigned int * type)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);
	platform_data = data;

	if( IMM_IsAdcInitReady() == 0 )
		printk(KERN_ERR "cxd3778gf_setup_platform() AUXADC is not ready\n");

	if( get_md_adc_val(5) < 223 ){
		*type = TYPE_A;
	}
	else if (3795 < get_md_adc_val(5)){
		*type = TYPE_Z;
	}
	else {
		printk(KERN_ERR "cxd3778gf_setup_platform() AUXADC value error\n");
		*type = TYPE_A;
	}

	board_type = *type;

	if (get_md_adc_val(ICX_AD_BID3) <= ICX_AD_THRESHOLD_A) {
		/* BOARD_ID_NO_PULLUP */
		board_set_flag = 0x01;
	} else if ((ICX_AD_THRESHOLD_A < get_md_adc_val(ICX_AD_BID3)) && (get_md_adc_val(ICX_AD_BID3) <= ICX_AD_THRESHOLD_B)) {
		/* BOARD_ID_22k_PULLUP */
		board_set_flag = 0x02;
	} else if ((ICX_AD_THRESHOLD_B < get_md_adc_val(ICX_AD_BID3)) && (get_md_adc_val(ICX_AD_BID3) <= ICX_AD_THRESHOLD_C)) {
		/* reserve */
		board_set_flag = 0x00;
        } else if ((ICX_AD_THRESHOLD_C < get_md_adc_val(ICX_AD_BID3)) && (get_md_adc_val(ICX_AD_BID3) <= ICX_AD_THRESHOLD_D)) {
		/* reserve */
		board_set_flag = 0x00;
	} else if ((ICX_AD_THRESHOLD_D < get_md_adc_val(ICX_AD_BID3)) && (get_md_adc_val(ICX_AD_BID3) <= ICX_AD_MAX)) {
		board_set_flag = 0x00;
	}

	rv = mt_set_gpio_mode(I2S0_DATA_OUT, GPIO_MODE_01); CHK_RV("I2S0_DATA_OUT: code %d error occured.\n");
	rv = mt_set_gpio_mode(I2S0_DATA_IN, GPIO_MODE_01); CHK_RV("I2S0_DATA_IN: code %d error occured.\n");
	rv = mt_set_gpio_mode(I2S0_DATA_BCK, GPIO_MODE_01); CHK_RV("I2S0_DATA_BCK: code %d error occured.\n");
	rv = mt_set_gpio_mode(I2S0_DATA_LRCK, GPIO_MODE_01); CHK_RV("I2S0_DATA_LRCK: code %d error occured.\n");
	rv = mt_set_gpio_mode(I2S1_DATA_OUT, GPIO_MODE_01); CHK_RV("I2S1_DATA_OUT: code %d error occured.\n");
	rv = mt_set_gpio_mode(I2S1_DATA_IN, GPIO_MODE_01); CHK_RV("I2S0_DATA_IN: code %d error occured.\n");
	rv = mt_set_gpio_mode(I2S1_DATA_BCK, GPIO_MODE_01); CHK_RV("I2S0_DATA_BCK: code %d error occured.\n");
	rv = mt_set_gpio_mode(I2S1_DATA_LRCK, GPIO_MODE_01); CHK_RV("I2S0_DATA_LRCK: code %d error occured.\n");

	rv = mt_set_gpio_mode(GPIO_AU_VL_EN, GPIO_MODE_GPIO); CHK_RV("GPIO_AU_VL_EN mode: code %d error occured.\n");
	rv = mt_set_gpio_dir(GPIO_AU_VL_EN, GPIO_DIR_OUT); CHK_RV("GPIO_AU_VL_EN dir: code %d error occured.\n");
	rv = mt_set_gpio_out(GPIO_AU_VL_EN, GPIO_OUT_ONE); CHK_RV("GPIO_AU_VL_EN out: code %d error occured.\n");
	msleep(1);

	rv = mt_set_gpio_mode(GPIO_AU_AVDD_EN, GPIO_MODE_GPIO); CHK_RV("GPIO_AU_AVDD_EN mode: code %d error occured.\n");
	rv = mt_set_gpio_dir(GPIO_AU_AVDD_EN, GPIO_DIR_OUT); CHK_RV("GPIO_AU_AVDD_EN dir: code %d error occured.\n");
	
	if (board_type == TYPE_Z){
		rv = mt_set_gpio_mode(GPIO_AU_SE_POS_EN, GPIO_MODE_GPIO); CHK_RV("GPIO_AU_SE_POS_EN mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_AU_SE_POS_EN, GPIO_DIR_OUT); CHK_RV("GPIO_AU_SE_POS_EN dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_AU_BTL_5V_EN, GPIO_MODE_GPIO); CHK_RV("GPIO_AU_BTL_5V_EN mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_AU_BTL_5V_EN, GPIO_DIR_OUT); CHK_RV("GPIO_AU_BTL_5V_EN dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_AU_BTL_5V_EN_2, GPIO_MODE_GPIO); CHK_RV("GPIO_AU_BTL_5V_EN_2 mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_AU_BTL_5V_EN_2, GPIO_DIR_OUT); CHK_RV("GPIO_AU_BTL_5V_EN_2 dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_AU_BTL_7V_EN, GPIO_MODE_GPIO); CHK_RV("GPIO_AU_BTL_7V_EN mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_AU_BTL_7V_EN, GPIO_DIR_OUT); CHK_RV("GPIO_AU_BTL_7V_EN dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_AU_SE_3V_EN, GPIO_MODE_GPIO); CHK_RV("GPIO_AU_SE_3V_EN mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_AU_SE_3V_EN, GPIO_DIR_OUT); CHK_RV("GPIO_AU_SE_3V_EN dir: code %d error occured.\n");
	}

	if (board_type == TYPE_A){
		/* temporary */
		rv = mt_set_gpio_mode(GPIO_AU_BTL_5V_EN_2, GPIO_MODE_GPIO); CHK_RV("GPIO_AU_BTL_5V_EN_2 mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_AU_BTL_5V_EN_2, GPIO_DIR_OUT); CHK_RV("GPIO_AU_BTL_5V_EN_2 dir: code %d error occured.\n");
	}

	rv = mt_set_gpio_mode(GPIO_ADA_LDO_EN, GPIO_MODE_GPIO); CHK_RV("GPIO_ADA_LDO_EN mode: code %d error occured.\n");
	rv = mt_set_gpio_dir(GPIO_ADA_LDO_EN, GPIO_DIR_OUT); CHK_RV("GPIO_ADA_LDO_EN dir: code %d error occured.\n");

	rv = mt_set_gpio_mode(GPIO_ADA_XRST, GPIO_MODE_GPIO); CHK_RV("GPIO_ADA_XRST mode: code %d error occured.\n");
	rv = mt_set_gpio_dir(GPIO_ADA_XRST, GPIO_DIR_OUT); CHK_RV("GPIO_ADA_XRST dir: code %d error occured.\n");

	if (board_type == TYPE_Z){
		rv = mt_set_gpio_mode(GPIO_AU_SE_NEG_EN, GPIO_MODE_GPIO); CHK_RV("GPIO_AU_SE_NEG_EN mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_AU_SE_NEG_EN, GPIO_DIR_OUT); CHK_RV("GPIO_AU_SE_NEG_EN dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_ADA_FS441_EN, GPIO_MODE_GPIO); CHK_RV("GPIO_ADA_FS441_EN mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_ADA_FS441_EN, GPIO_DIR_OUT); CHK_RV("GPIO_ADA_FS441_EN dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_ADA_FS480_EN, GPIO_MODE_GPIO); CHK_RV("GPIO_ADA_FS480_EN mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_ADA_FS480_EN, GPIO_DIR_OUT); CHK_RV("GPIO_ADA_FS480_EN dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(EXT_CK1, GPIO_MODE_01); CHK_RV("EXT_CK1 mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(EXT_CK1, GPIO_DIR_IN); CHK_RV("EXT_CK1 dir: code %d error occured.\n");
		rv = mt_set_gpio_pull_enable(EXT_CK1,GPIO_PULL_DISABLE); CHK_RV("EXT_CK1 pull disable: code %d error occured.\n");
	}

	if (board_type == TYPE_A){
		rv = mt_set_gpio_mode(GPIO_HP_XMUTE,GPIO_MODE_GPIO); CHK_RV("GPIO_HP_XMUTE mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_HP_XMUTE, GPIO_DIR_OUT); CHK_RV("GPIO_HP_XMUTE dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_HP_FM_SERIES_XMUTE, GPIO_MODE_GPIO); CHK_RV("GPIO_HP_FM_SERIES_XMUTE mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_HP_FM_SERIES_XMUTE, GPIO_DIR_OUT); CHK_RV("GPIO_HP_FM_SERIES_XMUTE dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(EXT_CK1, GPIO_MODE_GPIO); CHK_RV("EXT_CK1 mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(EXT_CK1, GPIO_DIR_IN); CHK_RV("EXT_CK1 dir: code %d error occured.\n");
		rv = mt_set_gpio_pull_enable(EXT_CK1,GPIO_PULL_ENABLE); CHK_RV("EXT_CK1 pull enable: code %d error occured.\n");
	}

	if (board_type == TYPE_Z){
		rv = mt_set_gpio_mode(GPIO_HP_XMUTE,GPIO_MODE_GPIO); CHK_RV("GPIO_HP_XMUTE mode: code %d error occured.\n"); 
		rv = mt_set_gpio_dir(GPIO_HP_XMUTE, GPIO_DIR_OUT); CHK_RV("GPIO_HP_XMUTE dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_HP_SE_MUTE_CP_ON,GPIO_MODE_GPIO); CHK_RV("GPIO_HP_SE_MUTE_CP_ON mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_HP_SE_MUTE_CP_ON, GPIO_DIR_OUT); CHK_RV("GPIO_HP_SE_MUTE_CP_ON dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_HP_BTL_MUTE_ON, GPIO_MODE_GPIO); CHK_RV("GPIO_HP_BTL_MUTE_ON, mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_HP_BTL_MUTE_ON, GPIO_DIR_OUT); CHK_RV("GPIO_HP_BTL_MUTE_ON, dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_HP_BTL_MUTE_OFF, GPIO_MODE_GPIO); CHK_RV("GPIO_HP_BTL_MUTE_OFF mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_HP_BTL_MUTE_OFF, GPIO_DIR_OUT); CHK_RV("GPIO_HP_BTL_MUTE_OFF dir: code %d error occured.\n");

		rv = mt_set_gpio_mode(GPIO_HP_BT_DET_L, GPIO_MODE_GPIO); CHK_RV("GPIO_HP_BT_DET_L mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_HP_BT_DET_L, GPIO_DIR_IN); CHK_RV("GPIO_HP_BT_DET_L dir: code %d error occured.\n");
		rv = mt_set_gpio_mode(GPIO_HP_BT_DET_R, GPIO_MODE_GPIO); CHK_RV("GPIO_HP_BT_DET_R mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_HP_BT_DET_R, GPIO_DIR_IN); CHK_RV("GPIO_HP_BT_DET_R dir: code %d error occured.\n");
		if((board_set_flag&BOARD_ID_NO_PULLUP_MASK) || (board_set_flag&BOARD_ID_PULLUP_MASK))
			rv = mt_set_gpio_pull_enable(GPIO_HP_BT_DET_R, GPIO_PULL_ENABLE);
		else
			rv = mt_set_gpio_pull_enable(GPIO_HP_BT_DET_R, GPIO_PULL_DISABLE);
	}

	if (board_type == TYPE_A){
		rv = mt_set_gpio_mode(GPIO_HP_BT_DET_L, GPIO_MODE_GPIO); CHK_RV("GPIO_HP_BT_DET_L mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_HP_BT_DET_L, GPIO_DIR_IN); CHK_RV("GPIO_HP_BT_DET_L dir: code %d error occured.\n");
		rv = mt_set_gpio_pull_enable(GPIO_HP_BT_DET_L, GPIO_PULL_ENABLE); CHK_RV("GPIO_HP_BT_DET_L pull enable: code %d error occured.\n");
	}

	rv = mt_set_gpio_mode(GPIO_HP_DET, GPIO_MODE_GPIO); CHK_RV("GPIO_HP_DET mode: code %d error occured.\n");
	rv = mt_set_gpio_dir(GPIO_HP_DET, GPIO_DIR_IN); CHK_RV("GPIO_HP_DET dir: code %d error occured.\n");

	if (board_type == TYPE_Z){
		rv = mt_set_gpio_pull_enable(GPIO_HP_DET, GPIO_PULL_DISABLE); CHK_RV("GPIO_HP_DET pull enable: code %d error occured.\n");
	}
	if (board_type == TYPE_A){
		rv = mt_set_gpio_pull_enable(GPIO_HP_DET, GPIO_PULL_ENABLE); CHK_RV("GPIO_HP_DET pull enbale: code %d error occured.\n");
		rv = mt_set_gpio_pull_select(GPIO_HP_DET, GPIO_PULL_DOWN); CHK_RV("GPIO_HP_DET select: code %d error occured.\n");
	}

	if (board_type == TYPE_Z){
		rv = mt_set_gpio_mode(GPIO_XPCM_DET, GPIO_MODE_GPIO); CHK_RV("GPIO_XPCM_DET mode: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_XPCM_DET, GPIO_DIR_IN); CHK_RV("GPIO_XPCM_DET dir: code %d error occured.\n");
		rv = mt_set_gpio_pull_enable(GPIO_XPCM_DET, GPIO_PULL_ENABLE); CHK_RV("GPIO_XPCM_DET pull enable: code %d error occured.\n");
		rv = mt_set_gpio_pull_select(GPIO_XPCM_DET, GPIO_PULL_DOWN); CHK_RV("GPIO_XPCM_DET pull down: code %d error occured.\n");
		rv = mt_set_gpio_mode(GPIO_ADA_XINT, GPIO_MODE_GPIO); CHK_RV("GPIO_ADA_XINT: code %d error occured.\n");
		rv = mt_set_gpio_dir(GPIO_ADA_XINT, GPIO_DIR_IN); CHK_RV("GPIO_ADA_XINT dir: code %d error occured.\n");
		rv = mt_set_gpio_pull_enable(GPIO_ADA_XINT, GPIO_PULL_ENABLE); CHK_RV("GPIO_ADA_XINT pull enable: code %d error occured.\n");
		rv = mt_set_gpio_pull_select(GPIO_ADA_XINT, GPIO_PULL_DOWN); CHK_RV("GPIO_ADA_XINT pull down: code %d error occured.\n");
	}
#if 0
	platform_data = data;

	regulator_180 = regulator_get(NULL, REGULATOR_180);
	if(IS_ERR(regulator_180)){
		print_fail("regulator_get(1.80): code %ld error occurred.\n",PTR_ERR(regulator_180));
		back_trace();
		return(PTR_ERR(regulator_180));
	}

	rv=regulator_set_voltage(regulator_180, 1800*1000, 1800*1000); CHK_RV("regulator_set_volatage(1.80): code %d error occured.\n");

	regulator_285 = regulator_get(NULL, REGULATOR_285);
	if(IS_ERR(regulator_285)){
		print_fail("regulator_get(2.85): code %ld error occurred.\n",PTR_ERR(regulator_285));
		back_trace();
		return(PTR_ERR(regulator_285));
	}

	rv=regulator_set_voltage(regulator_285, 2900*1000, 2900*1000); CHK_RV("regulator_set_volatage(2.85): code %d error occured.\n");

	rv=gpio_request(GPIO_ADA_LDO_EN, "ADA_LDO_EN"); CHK_RV("gpio_request(ADA_LDO_EN): code %d error occured.\n");
	rv=gpio_direction_output(GPIO_ADA_LDO_EN, 1);   CHK_RV("gpio_direction_outout(ADA_LDO_EN): code %d error occured.\n");

	rv=gpio_request(GPIO_ADA_XINT, "ADA_XINT"); CHK_RV("gpio_request(ADA_XINT): code %d error occured.\n");
	rv=gpio_direction_input(GPIO_ADA_XINT);     CHK_RV("gpio_direction_input(ADA_XINT): code %d error occured.\n");

	rv=gpio_request(GPIO_SP_XSHTD, "SP_XSHTD"); CHK_RV("gpio_request(SP_XSHTD): code %d error occured.\n");
	rv=gpio_direction_output(GPIO_SP_XSHTD, 0); CHK_RV("gpio_direction_output(SP_XSHTD): code %d error occured.\n");

	rv=gpio_request(GPIO_XPCM_DET, "XPCM_DET"); CHK_RV("gpio_request(XPCM_DET): code %d error occured.\n");
	rv=gpio_direction_input(GPIO_XPCM_DET);     CHK_RV("gpio_direction_input(XPCM_DET): code %d error occured.\n");

	rv=gpio_request(GPIO_ADA_XRST, "ADA_XRST"); CHK_RV("gpio_request(ADA_XRST): code %d error occured.\n");
	rv=gpio_direction_output(GPIO_ADA_XRST, 0); CHK_RV("gpio_direction_outout(ADA_XRST): code %d error occured.\n");

	rv=gpio_request(GPIO_HP_XMUTE, "HP_XMUTE"); CHK_RV("gpio_request(HP_XMUTE): code %d error occured.\n");
	rv=gpio_direction_output(GPIO_HP_XMUTE, 0); CHK_RV("gpio_direction_outout(HP_XMUTE): code %d error occured.\n");

	if(GPIO_NC_CMPOUT>=0){
		rv=gpio_request(GPIO_NC_CMPOUT, "NC_CMPOUT"); CHK_RV("gpio_request(NC_CMPOUT): code %d error occured.\n");
		rv=gpio_direction_input(GPIO_NC_CMPOUT);      CHK_RV("gpio_direction_input(NC_CMPOUT): code %d error occured.\n");
	}

	if(GPIO_NC_DET>=0){
		rv=gpio_request(GPIO_NC_DET, "NC_DET"); CHK_RV("gpio_request(NC_DET): code %d error occured.\n");
		rv=gpio_direction_input(GPIO_NC_DET);   CHK_RV("gpio_direction_input(NC_DET): code %d error occured.\n");
	}

	if(GPIO_HP_DET>=0){
		rv=gpio_request(GPIO_HP_DET, "HP_DET"); CHK_RV("gpio_request(HP_DET): code %d error occured.\n");
		rv=gpio_direction_input(GPIO_HP_DET);   CHK_RV("gpio_direction_input(HP_DET): code %d error occured.\n");
	}

	if(GPIO_HP_XMUTE3>=0){
		rv=gpio_request(GPIO_HP_XMUTE3, "HP_XMUTE3"); CHK_RV("gpio_request(HP_XMUTE3): code %d error occured.\n");
		rv=gpio_direction_output(GPIO_HP_XMUTE3, 0);  CHK_RV("gpio_direction_outout(HP_XMUTE3): code %d error occured.\n");
	}

	if(GPIO_ADA_OSC_EN>=0){
		rv=gpio_request(GPIO_ADA_OSC_EN, "ADA_OSC_EN"); CHK_RV("gpio_request(ADA_OSC_EN): code %d error occured.\n");
		rv=gpio_direction_output(GPIO_ADA_OSC_EN, 0);   CHK_RV("gpio_direction_outout(ADA_OSC_EN): code %d error occured.\n");
	}

	if(GPIO_HP_XMUTE4>=0){
		rv=gpio_request(GPIO_HP_XMUTE4, "HP_XMUTE4"); CHK_RV("gpio_request(HP_XMUTE4): code %d error occured.\n");
		rv=gpio_direction_output(GPIO_HP_XMUTE4, 0);  CHK_RV("gpio_direction_outout(HP_XMUTE4): code %d error occured.\n");
	}

	if(GPIO_MCLK_441_EN>=0){
		rv=gpio_request(GPIO_MCLK_441_EN, "MCLK_441_EN_EN"); CHK_RV("gpio_request(MCLK_441_EN): code %d error occured.\n");
		rv=gpio_direction_output(GPIO_MCLK_441_EN, 0);       CHK_RV("gpio_direction_outout(MCLK_441_EN): code %d error occured.\n");
	}

	initialized=TRUE;
#endif
	return(0);
}

int cxd3778gf_reset_platform(void)
{
	print_trace("%s()\n",__FUNCTION__);

	mt_set_gpio_out(GPIO_AU_VL_EN,GPIO_OUT_ZERO);

#if 0
	if(!initialized)
		return(0);

	if(GPIO_MCLK_441_EN>=0){
		gpio_direction_input(GPIO_MCLK_441_EN);
		gpio_free(GPIO_MCLK_441_EN);
	}

	if(GPIO_HP_XMUTE4>=0){
		gpio_direction_input(GPIO_HP_XMUTE4);
		gpio_free(GPIO_HP_XMUTE4);
	}

	if(GPIO_ADA_OSC_EN>=0){
		gpio_direction_input(GPIO_ADA_OSC_EN);
		gpio_free(GPIO_ADA_OSC_EN);
	}

	if(GPIO_HP_XMUTE3>=0){
		gpio_direction_input(GPIO_HP_XMUTE3);
		gpio_free(GPIO_HP_XMUTE3);
	}

	if(GPIO_HP_DET>=0){
		gpio_direction_input(GPIO_HP_DET);
		gpio_free(GPIO_HP_DET);
	}

	if(GPIO_NC_DET>=0){
		gpio_direction_input(GPIO_NC_DET);
		gpio_free(GPIO_NC_DET);
	}

	if(GPIO_NC_CMPOUT>=0){
		gpio_direction_input(GPIO_NC_CMPOUT);
		gpio_free(GPIO_NC_CMPOUT);
	}

	gpio_direction_input(GPIO_HP_XMUTE);
	gpio_free(GPIO_HP_XMUTE);

	gpio_direction_input(GPIO_ADA_XRST);
	gpio_free(GPIO_ADA_XRST);

	gpio_direction_input(GPIO_XPCM_DET);
	gpio_free(GPIO_XPCM_DET);

	gpio_direction_input(GPIO_SP_XSHTD);
	gpio_free(GPIO_SP_XSHTD);

	gpio_direction_input(GPIO_ADA_XINT);
	gpio_free(GPIO_ADA_XINT);

	gpio_direction_input(GPIO_ADA_LDO_EN);
	gpio_free(GPIO_ADA_LDO_EN);

	regulator_put(regulator_285);
	regulator_put(regulator_180);
#endif
	return(0);
}

int cxd3778gf_switch_180_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value)
		regulator_enable(regulator_180);
	else
		regulator_disable(regulator_180);
#endif
	return(0);
}

int cxd3778gf_switch_285_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value)
		regulator_enable(regulator_285);
	else
		regulator_disable(regulator_285);
#endif
	return(0);
}

int cxd3778gf_switch_hp3x_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		if (board_type == TYPE_Z){
			mt_set_gpio_out(GPIO_AU_BTL_7V_EN, GPIO_OUT_ONE);
		}
	}
	else {
		if (board_type == TYPE_Z){
			mt_set_gpio_out(GPIO_AU_BTL_7V_EN, GPIO_OUT_ZERO);
		}
	}
	return(0);
}

int cxd3778gf_switch_logic_ldo(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value)
		gpio_set_value(GPIO_ADA_LDO_EN,1);
	else
		gpio_set_value(GPIO_ADA_LDO_EN,0);
#endif
	return(0);
}

int cxd3778gf_switch_external_osc(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
#ifdef ICX_ENABLE_AU2CLK

	if(value==EXTERNAL_OSC_480){
		gpio_set_value(GPIO_MCLK_441_EN,0);
		/* msleep(10); */
		gpio_set_value(GPIO_ADA_OSC_EN,1);
		print_info("external OSC = 49.152MHz for 48KHz\n");
	}
	else if(value==EXTERNAL_OSC_441){
		gpio_set_value(GPIO_ADA_OSC_EN,0);
		/* msleep(10); */
		gpio_set_value(GPIO_MCLK_441_EN,1);
		print_info("external OSC = 45.1584MHz for 44.1KHz\n");
	}
	else if(value==EXTERNAL_OSC_OFF){
		gpio_set_value(GPIO_MCLK_441_EN,0);
		gpio_set_value(GPIO_ADA_OSC_EN,0);
		print_info("external OSC = OFF\n");
	}

#else

	if(value==EXTERNAL_OSC_480 || value==EXTERNAL_OSC_441){
		if(GPIO_ADA_OSC_EN>=0){
			gpio_set_value(GPIO_ADA_OSC_EN,1);
			print_info("external OSC = ON\n");
		}
	}
	else if(value==EXTERNAL_OSC_OFF){
		if(GPIO_ADA_OSC_EN>=0){
			gpio_set_value(GPIO_ADA_OSC_EN,0);
			print_info("external OSC = OFF\n");
		}
	}

#endif
#endif
	return(0);
}

int cxd3778gf_get_external_osc(void)
{
#if 0
	int osc;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

#ifdef ICX_ENABLE_AU2CLK

	osc=EXTERNAL_OSC_OFF;

	rv=gpio_get_value(GPIO_MCLK_441_EN);
	if(rv==1){
		osc=EXTERNAL_OSC_441;
	}
	else{
		rv=gpio_get_value(GPIO_ADA_OSC_EN);
		if(rv==1)
			osc=EXTERNAL_OSC_480;
		else
			osc=EXTERNAL_OSC_OFF;
	}

#else

	if(GPIO_ADA_OSC_EN>=0){
		rv=gpio_get_value(GPIO_ADA_OSC_EN);
		if(rv==1)
			osc=EXTERNAL_OSC_480;
		else
			osc=EXTERNAL_OSC_OFF;
	}
	else{
		osc=EXTERNAL_OSC_480;
	}

#endif

	return(osc);
#endif
	return(0);
}

int cxd3778gf_reset(void)
{
	print_trace("%s()\n",__FUNCTION__);

	if (board_type == TYPE_Z){
		mt_set_gpio_out(GPIO_AU_BTL_7V_EN, GPIO_OUT_ZERO);
		mt_set_gpio_out(GPIO_ADA_FS441_EN, GPIO_OUT_ZERO);
		mt_set_gpio_out(GPIO_ADA_FS480_EN, GPIO_OUT_ZERO);
		msleep(1);
		mt_set_gpio_mode(EXT_CK1, GPIO_MODE_GPIO);
		mt_set_gpio_pull_enable(EXT_CK1,GPIO_PULL_ENABLE);
	}

	mt_set_gpio_out(GPIO_AU_SE_NEG_EN, GPIO_OUT_ZERO);
	msleep(10);
	mt_set_gpio_out(GPIO_AU_SE_3V_EN, GPIO_OUT_ZERO);
	mt_set_gpio_out(GPIO_ADA_XRST, GPIO_OUT_ZERO);
	msleep(1);
	mt_set_gpio_out(GPIO_ADA_LDO_EN, GPIO_OUT_ZERO);
	msleep(1);

	if (board_type == TYPE_Z){
		mt_set_gpio_out(GPIO_AU_BTL_5V_EN, GPIO_OUT_ZERO);
		msleep(1);
	}
	/* temporary */
	mt_set_gpio_out(GPIO_AU_BTL_5V_EN_2, GPIO_OUT_ZERO);

	mt_set_gpio_out(GPIO_AU_SE_POS_EN, GPIO_OUT_ZERO);
	msleep(1);

	if(!hwPowerDown(MT6323_POWER_LDO_VIBR, "VIBR"))
		printk(KERN_ERR "MT6323_POWER_BUCK_VIBR OFF fail.\n");

	if(!hwPowerDown(MT6323_POWER_LDO_VMCH, "VMCH"))
		printk(KERN_ERR "MT6323_POWER_LDO_VMCH OFF fail.\n");

	if(board_set_flag & BOARD_ID_NO_PULLUP_MASK){
		if(board_type == TYPE_A && (icx_pm_helper_sysinfo & GPIO_UPDATE_SYSINFO_MASK)){
			/* LF1 board only */
			if(!hwPowerDown(MT6323_POWER_BUCK_VPA, "VPA"))
				printk(KERN_ERR "MT6323_POWER_BUCK_VPA OFF fail.\n");
		}
		if(board_type == TYPE_Z){
			/* LF1/LF2 board only */
			if(!hwPowerDown(MT6323_POWER_BUCK_VPA, "VPA"))
				printk(KERN_ERR "MT6323_POWER_BUCK_VPA OFF fail.\n");
		}
	}
	msleep(1);

	mt_set_gpio_out(GPIO_AU_AVDD_EN,GPIO_OUT_ZERO);

	return(0);
}

int cxd3778gf_unreset(void)
{
	print_trace("%s()\n",__FUNCTION__);

	mt_set_gpio_out(GPIO_AU_AVDD_EN,GPIO_OUT_ONE);
	msleep(1);

	if(!hwPowerOn(MT6323_POWER_LDO_VIBR, VOL_2800, "VIBR"))
		printk(KERN_ERR "MT6323_POWER_LDO_VIBR ON fail.\n");

	if(!hwPowerOn(MT6323_POWER_LDO_VMCH, VOL_3300, "VMCH"))
                printk(KERN_ERR "MT6323_POWER_LDO_VMCH ON fail.\n");


	if(board_set_flag & BOARD_ID_NO_PULLUP_MASK){
		/* LF board only */
		if(board_type == TYPE_A && (icx_pm_helper_sysinfo & GPIO_UPDATE_SYSINFO_MASK)) {
			if(!hwPowerOn(MT6323_POWER_BUCK_VPA, VOL_2200, "VPA"))
				printk(KERN_ERR "MT6323_POWER_BUCK_VPA 2.2V ON fail.\n");
		}
		if (board_type == TYPE_Z){
			if(!hwPowerOn(MT6323_POWER_BUCK_VPA, VOL_2500, "VPA"))
				printk(KERN_ERR "MT6323_POWER_BUCK_VPA 2.5V ON fail.\n");
		}
		msleep(1);
	}

        if (board_type == TYPE_Z){
                mt_set_gpio_out(GPIO_AU_SE_POS_EN, GPIO_OUT_ONE);

                mt_set_gpio_out(GPIO_AU_BTL_5V_EN, GPIO_OUT_ONE);

                mt_set_gpio_out(GPIO_AU_BTL_5V_EN_2, GPIO_OUT_ONE);

                /* power HP3x is off when boot-up */
                mt_set_gpio_out(GPIO_AU_BTL_7V_EN, GPIO_OUT_ZERO);

                msleep(1);

                mt_set_gpio_out(GPIO_AU_SE_3V_EN, GPIO_OUT_ONE);
        }

        if (board_type == TYPE_A){
                /* temporary */
                mt_set_gpio_out(GPIO_AU_BTL_5V_EN_2, GPIO_OUT_ONE);
                msleep(1);
        }

        mt_set_gpio_out(GPIO_ADA_LDO_EN, GPIO_OUT_ONE);
        msleep(10);

        mt_set_gpio_out(GPIO_ADA_XRST, GPIO_OUT_ONE);
        msleep(1);

        if (board_type == TYPE_Z){
                mt_set_gpio_out(GPIO_ADA_FS441_EN, GPIO_OUT_ONE);
		if ((board_set_flag & BOARD_ID_NO_PULLUP_MASK))
			mt_set_gpio_out(GPIO_ADA_FS480_EN, GPIO_OUT_ONE);
		else
			mt_set_gpio_out(GPIO_ADA_FS480_EN, GPIO_OUT_ZERO);
		msleep(1);
                mt_set_gpio_out(GPIO_AU_SE_NEG_EN, GPIO_OUT_ONE);
		mt_set_gpio_mode(EXT_CK1, GPIO_MODE_01);
		mt_set_gpio_pull_enable(EXT_CK1,GPIO_PULL_DISABLE);
        }
	return(0);
}

int cxd3778gf_switch_smaster_mute(int value, int amp)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value) {
		if (board_type == TYPE_Z){
			if(amp == HEADPHONE_AMP_SMASTER_SE || amp == HEADPHONE_AMP_SMASTER_BTL){
				mt_set_gpio_out(GPIO_HP_XMUTE, GPIO_OUT_ZERO);
				/* tentative */
//				msleep(3);
//				mt_set_gpio_out(GPIO_HP_SE_MUTE_CP_ON, GPIO_OUT_ZERO);
			}
			if(amp == HEADPHONE_AMP_SMASTER_BTL){
				mt_set_gpio_out(GPIO_HP_BTL_MUTE_ON, GPIO_OUT_ONE);
				msleep(3);
				mt_set_gpio_out(GPIO_HP_BTL_MUTE_ON, GPIO_OUT_ZERO);
			}
		}
		if (board_type == TYPE_A){
			mt_set_gpio_out(GPIO_HP_XMUTE, GPIO_OUT_ZERO);
                }
	}
        else {
                if (board_type == TYPE_Z){
			if(amp == HEADPHONE_AMP_SMASTER_SE || amp == HEADPHONE_AMP_SMASTER_BTL){
//				mt_set_gpio_out(GPIO_HP_SE_MUTE_CP_ON, GPIO_OUT_ONE);
				/* tentative */
//				msleep(3);
				mt_set_gpio_out(GPIO_HP_XMUTE, GPIO_OUT_ONE);
			}
			if(amp == HEADPHONE_AMP_SMASTER_BTL){
				mt_set_gpio_out(GPIO_HP_BTL_MUTE_OFF, GPIO_OUT_ONE);
				msleep(3);
				mt_set_gpio_out(GPIO_HP_BTL_MUTE_OFF, GPIO_OUT_ZERO);
			}
                }
                if (board_type == TYPE_A){
                        mt_set_gpio_out(GPIO_HP_XMUTE, GPIO_OUT_ONE);
                }
	}
#if 0
	if(value){
		gpio_set_value(GPIO_HP_XMUTE,0);
		if(GPIO_HP_XMUTE4>=0)
			gpio_set_value(GPIO_HP_XMUTE4,0);
	}
	else{
		gpio_set_value(GPIO_HP_XMUTE,1);
		if(GPIO_HP_XMUTE4>=0)
			gpio_set_value(GPIO_HP_XMUTE4,1);
	}
#endif
	return(0);
}

int cxd3778gf_switch_class_h_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if (board_type == TYPE_A){
		if(value) {
				mt_set_gpio_out(GPIO_HP_XMUTE, GPIO_OUT_ZERO);
		} else {
				mt_set_gpio_out(GPIO_HP_XMUTE, GPIO_OUT_ONE);
		}
	}
#if 0
	if(GPIO_HP_XMUTE3>=0){
		if(value)
			gpio_set_value(GPIO_HP_XMUTE3,0);
		else
			gpio_set_value(GPIO_HP_XMUTE3,1);
	}
#endif
	return(0);
}

int cxd3778gf_switch_speaker_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value)
		gpio_set_value(GPIO_SP_XSHTD,1);
	else
		gpio_set_value(GPIO_SP_XSHTD,0);
#endif
	return(0);
}

int cxd3778gf_get_hp_det_se_value(void)
{
	int rv=0;

	print_trace("%s()\n",__FUNCTION__);

	if(GPIO_HP_DET>=0) {
		//rv=gpio_get_value(GPIO_HP_DET);
		rv|=(1 ^ mt_get_gpio_in(GPIO_HP_DET));
	}
	if(GPIO_NC_DET>=0) {
		//rv=gpio_get_value(GPIO_NC_DET);
		rv|= (1 ^ mt_get_gpio_in(GPIO_NC_DET));
	}

	return(rv);
}

int cxd3778gf_get_hp_det_btl_value(void)
{
        int rv=0;

        print_trace("%s()\n",__FUNCTION__);

	if (board_type == TYPE_Z){
		if(GPIO_HP_BT_DET_L>=0) {
		//rv=gpio_get_value(GPIO_NC_DET);
			rv|=(1 ^ mt_get_gpio_in(GPIO_HP_BT_DET_L));
		}
		if(GPIO_HP_BT_DET_R>=0) {
			rv|=(1 ^ mt_get_gpio_in(GPIO_HP_BT_DET_R))<<1;
			//rv=gpio_get_value(GPIO_NC_DET);
		}
	}

	return(rv);
}

int cxd3778gf_get_mic_det_value(void)
{
	int rv=0;
#if 0
	print_trace("%s()\n",__FUNCTION__);

	if(GPIO_NC_CMPOUT>=0)
		rv=gpio_get_value(GPIO_NC_CMPOUT);
#endif
	return(rv);
}

int cxd3778gf_get_xpcm_det_value(void)
{
	int rv=0;

	print_trace("%s()\n",__FUNCTION__);

//	rv=gpio_get_value(GPIO_XPCM_DET);

	return(rv);
}

int cxd3778gf_get_xpcm_det_irq(void)
{
	int rv=0;

	print_trace("%s()\n",__FUNCTION__);

//	rv=gpio_to_irq(GPIO_XPCM_DET);

	return(rv);
}

int cxd3778gf_set_se_cp_mute(void)
{
        int rv=0;

        print_trace("%s()\n",__FUNCTION__);

	mt_set_gpio_out(GPIO_HP_SE_MUTE_CP_ON, GPIO_OUT_ZERO);
//      rv=gpio_to_irq(GPIO_XPCM_DET);

        return(rv);
}

int cxd3778gf_set_se_cp_unmute(void)
{
        int rv=0;

        print_trace("%s()\n",__FUNCTION__);

        mt_set_gpio_out(GPIO_HP_SE_MUTE_CP_ON, GPIO_OUT_ONE);
//      rv=gpio_to_irq(GPIO_XPCM_DET);

        return(rv);
}


int cxd3778gf_set_441clk_enable(void)
{
        print_trace("%s()\n",__FUNCTION__);

	if (board_type == TYPE_Z){
		mt_set_gpio_out(GPIO_ADA_FS480_EN, GPIO_OUT_ZERO);
		mt_set_gpio_out(GPIO_ADA_FS441_EN, GPIO_OUT_ONE);
	}
//      rv=gpio_to_irq(GPIO_XPCM_DET);

        return(0);
}

int cxd3778gf_set_480clk_enable(void)
{
	print_trace("%s()\n",__FUNCTION__);

	if (board_type == TYPE_Z){
		mt_set_gpio_out(GPIO_ADA_FS441_EN, GPIO_OUT_ZERO);
		mt_set_gpio_out(GPIO_ADA_FS480_EN, GPIO_OUT_ONE);
	}

//      rv=gpio_to_irq(GPIO_XPCM_DET);

	return(0);
}

int cxd3778gf_get_441clk_value(void)
{
        int rv=0;

        print_trace("%s()\n",__FUNCTION__);

        if (board_type == TYPE_Z){
                rv= mt_get_gpio_out(GPIO_ADA_FS441_EN);
        }

//      rv=gpio_to_irq(GPIO_XPCM_DET);

        return(rv);
}

int cxd3778gf_get_480clk_value(void)
{
        int rv=0;

        print_trace("%s()\n",__FUNCTION__);

        if (board_type == TYPE_Z){
                rv= mt_get_gpio_out(GPIO_ADA_FS480_EN);
        }

//      rv=gpio_to_irq(GPIO_XPCM_DET);

        return(rv);
}

int cxd3778gf_get_hp_det_irq(void)
{
	int rv=0;

	if(GPIO_HP_DET == 53)
		rv = HP_DET_EINT;

	return(rv);
}

/* for debug */

#if 0

/* if use, please modified PINMUX. */

void debug_gpio119(int value)
{
	static int init=0;
	int rv;

	if(init==0){
		rv=gpio_request(119,"DEBUG119");
		if(rv<0){
			printk(KERN_ERR "gpio_request(DEBUG119): code %d error occurred.\n",rv);
			printk(KERN_ERR "%s():\n",__FUNCTION__);
			return;
		}

		rv=gpio_direction_output(119,0);
		if(rv<0){
			printk(KERN_ERR "gpio_direction_output(DEBUG119): code %d error occurred.\n",rv);
			printk(KERN_ERR "%s():\n",__FUNCTION__);
			gpio_free(119);
			return;
		}

		init=1;
	}

	printk("GPIO119:%d\n",value?1:0);
	gpio_set_value(119,value?1:0);

	return;
}

#endif
