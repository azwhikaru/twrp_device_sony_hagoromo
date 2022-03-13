/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf_platform.c
 *
 * CXD3774GF CODEC driver
 *
 * Copyright (c) 2013 Sony Corporation
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

#include "cxd3774gf_common.h"
#include <mach/mt_gpio.h>

#define REGULATOR_180    (platform_data->regulator_180)
#define REGULATOR_285    (platform_data->regulator_285)
#define GPIO_ADA_LDO_EN  (platform_data->port_ada_ldo_en)
#define GPIO_ADA_XINT    (platform_data->port_ada_xint)
#define GPIO_SP_XSHTD    (platform_data->port_sp_xshtd)
#define GPIO_XPCM_DET    (platform_data->port_xpcm_det)
#define GPIO_ADA_XRST    (platform_data->port_ada_xrst)
#define GPIO_HP_XMUTE    (platform_data->port_hp_xmute)
#define GPIO_NC_CMPOUT   (platform_data->port_nc_cmpout)
#define GPIO_NC_DET      (platform_data->port_nc_det)
#define GPIO_HP_DET      (platform_data->port_hp_det)
#define GPIO_HP_XMUTE3   (platform_data->port_hp_xmute3)
#define GPIO_ADA_OSC_EN  (platform_data->port_ada_osc_en)
#define GPIO_HP_XMUTE4   (platform_data->port_hp_xmute4)
#define GPIO_MCLK_441_EN (platform_data->port_mclk_441_en)
#define PWR2_EN (platform_data->port_au_pwr2_en)
#define PWR3_EN (platform_data->port_au_pwr3_en)
#define PWR5_EN (platform_data->port_au_pwr5_en)

#define CHK_RV(_msg) \
	if(rv<0){ \
		print_fail(_msg,rv); \
		return(-1); \
	}

static int initialized = FALSE;
static struct cxd3774gf_platform_data * platform_data = NULL;
static struct regulator * regulator_285 = NULL;
static struct regulator * regulator_180 = NULL;

int cxd3774gf_setup_platform(struct cxd3774gf_platform_data * data)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);
	platform_data = data;

	mt_set_gpio_mode(PWR2_EN, GPIO_MODE_GPIO);
	mt_set_gpio_dir(PWR2_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(PWR2_EN,GPIO_OUT_ONE);
	msleep(1);

	mt_set_gpio_mode(PWR3_EN, GPIO_MODE_GPIO);
	mt_set_gpio_dir(PWR3_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(PWR3_EN,GPIO_OUT_ONE);
	msleep(1);

	mt_set_gpio_mode(PWR5_EN, GPIO_MODE_GPIO);
	mt_set_gpio_dir(PWR5_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(PWR5_EN,GPIO_OUT_ONE);
	msleep(10);


	mt_set_gpio_mode(GPIO_ADA_XRST, GPIO_MODE_GPIO);
	mt_set_gpio_dir(GPIO_ADA_XRST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_ADA_XRST,GPIO_OUT_ONE);
	msleep(1);
	mt_set_gpio_out(GPIO_ADA_XRST, GPIO_OUT_ZERO);
	msleep(1);
	mt_set_gpio_out(GPIO_ADA_XRST, GPIO_OUT_ONE);
	msleep(1);

	mt_set_gpio_mode(GPIO_MCLK_441_EN, GPIO_MODE_GPIO);
	mt_set_gpio_dir(GPIO_MCLK_441_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_MCLK_441_EN, GPIO_OUT_ONE);

	mt_set_gpio_mode(GPIO_HP_XMUTE3, GPIO_MODE_GPIO);
	mt_set_gpio_dir(GPIO_HP_XMUTE3, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_HP_XMUTE3, GPIO_OUT_ZERO);

	mt_set_gpio_mode(GPIO_HP_DET, GPIO_MODE_GPIO);
	mt_set_gpio_dir(GPIO_HP_DET, GPIO_DIR_IN);

	mt_set_gpio_mode(GPIO_NC_DET, GPIO_MODE_GPIO);
	mt_set_gpio_dir(GPIO_NC_DET, GPIO_DIR_IN);
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

int cxd3774gf_reset_platform(void)
{
	print_trace("%s()\n",__FUNCTION__);
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

int cxd3774gf_switch_180_power(int value)
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

int cxd3774gf_switch_285_power(int value)
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

int cxd3774gf_switch_logic_ldo(int value)
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

int cxd3774gf_switch_external_osc(int value)
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

int cxd3774gf_get_external_osc(void)
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

int cxd3774gf_reset(void)
{
	print_trace("%s()\n",__FUNCTION__);

//	gpio_set_value(GPIO_ADA_XRST,0);
	mt_set_gpio_out(GPIO_ADA_XRST, GPIO_OUT_ZERO);
	return(0);
}

int cxd3774gf_unreset(void)
{
	print_trace("%s()\n",__FUNCTION__);

//	gpio_set_value(GPIO_ADA_XRST,1);
	mt_set_gpio_out(GPIO_ADA_XRST, GPIO_OUT_ONE);
	return(0);
}

int cxd3774gf_switch_smaster_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
//		mt_set_gpio_out(GPIO_HP_XMUTE, GPIO_OUT_ZERO);
//		gpio_set_value(GPIO_HP_XMUTE,0);
//		if(GPIO_HP_XMUTE4>=0)
//			gpio_set_value(GPIO_HP_XMUTE4,0);
	}
	else{
//		mt_set_gpio_out(GPIO_HP_XMUTE, GPIO_OUT_ONE);
//		gpio_set_value(GPIO_HP_XMUTE,1);
//		if(GPIO_HP_XMUTE4>=0)
//			gpio_set_value(GPIO_HP_XMUTE4,1);
	}

	return(0);
}

int cxd3774gf_switch_class_h_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(GPIO_HP_XMUTE3>=0){
		if(value) {
//			gpio_set_value(GPIO_HP_XMUTE3,0);
			mt_set_gpio_out(GPIO_HP_XMUTE3, GPIO_OUT_ZERO);
		}else{
//			gpio_set_value(GPIO_HP_XMUTE3,1);
			mt_set_gpio_out(GPIO_HP_XMUTE3, GPIO_OUT_ONE);
		}
	}
	return(0);
}

int cxd3774gf_switch_speaker_power(int value)
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

int cxd3774gf_get_hp_det_value(void)
{
	int rv=0;

	print_trace("%s()\n",__FUNCTION__);

	if(GPIO_HP_DET>=0) {
		//rv=gpio_get_value(GPIO_HP_DET);
		rv = mt_get_gpio_in(GPIO_HP_DET);
	}

	if(GPIO_NC_DET>=0) {
		//rv=gpio_get_value(GPIO_NC_DET);
		rv = mt_get_gpio_in(GPIO_NC_DET);
	}

	return(rv);
}

int cxd3774gf_get_mic_det_value(void)
{
	int rv=0;
#if 0
	print_trace("%s()\n",__FUNCTION__);

	if(GPIO_NC_CMPOUT>=0)
		rv=gpio_get_value(GPIO_NC_CMPOUT);
#endif
	return(rv);
}

int cxd3774gf_get_xpcm_det_value(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

//	rv=gpio_get_value(GPIO_XPCM_DET);

	return(rv);
}

int cxd3774gf_get_xpcm_det_irq(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

//	rv=gpio_to_irq(GPIO_XPCM_DET);

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
