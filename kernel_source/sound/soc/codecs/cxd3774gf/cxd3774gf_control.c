/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf_control.c
 *
 * CXD3774GF CODEC driver
 *
 * Copyright (c) 2013,2014 Sony Corporation
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

/****************/
/*@ definitions */
/****************/

struct timed_mute_data {
	int                 port;
	int                 busy;
	unsigned long       timeout;
	struct delayed_work work;
};

/***************/
/*@ prototypes */
/***************/

static void do_resume_work(struct work_struct * work);

static int  set_timed_mute(int port, int value);
static int  get_timed_mute(int port, int * value);
static void do_timed_mute_work(struct work_struct * work);
static int  cancel_timed_mute(void);
static int  flush_timed_mute(void);

static void do_pcm_event_work(struct work_struct * work);

static int suspend_core(void);
static int resume_core(void);

static int change_output_device(
	int output_device,
	int headphone_amp,
	int analog_playback_mute,
	int icx_playback_active,
	int std_playback_active,
	int capture_active,
	unsigned int sample_rate
);

static int change_input_device(
	int input_device
);

static int change_sample_rate(
	unsigned int rate
);

static int judge_pcm_monitoring(
	int output_device,
	int headphone_amp,
	int noise_cancel_active,
	int analog_playback_mute
);

static int adjust_tone_control(
	int output_device,
	int headphone_amp,
	int headphone_type,
	int jack_status
);

static int adjust_device_gain(
	int input_device
);

static int report_jack_status(
	int old_status,
	int new_status,
	int force
);

#ifdef ICX_ENABLE_AU2CLK

static int change_external_osc(
	struct cxd3774gf_status * status
);

static int suitable_external_osc(
	int output_device,
	int headphone_amp,
	int icx_playback_active,
	int std_playback_active,
	int capture_active,
	unsigned int sample_rate
);

#endif

static int check_active(
	struct cxd3774gf_status * status
);

static int startup_chip(unsigned int rate);
static int shutdown_chip(void);
static int switch_cpclk(int value);
static int switch_dac(int value);
static int switch_smaster(int value);
static int switch_classh(int value);
static int switch_lineout(int value);
static int switch_speaker(int value);
static int switch_speaker_power(int value);
static int switch_linein(int value);
static int switch_tuner(int value);
static int switch_dmic(int value);
static int get_jack_status(int noise_cancel_active, int jack_mode);
static int set_mic_bias_mode(int mode);
static int switch_dnc_power(int value);
static int switch_ncmic_amp_power(int value);
static int show_device_id(void);

/**************/
/*@ variables */
/**************/

static int initialized = FALSE;
static struct mutex * global_mutex = NULL;
static int core_active = FALSE;
static struct work_struct  resume_work;
static struct delayed_work pcm_event_work;

static int (*repair_electrostatic_damage)(void) = NULL;

static struct mutex timed_mute_mutex;
struct timed_mute_data timed_mute[4]={
	{ 
		.port    = 0,
		.busy    = FALSE,
		.timeout = 0, 
	},
	{ 
		.port    = 1,
		.busy    = FALSE,
		.timeout = 0, 
	},
	{ 
		.port    = 2,
		.busy    = FALSE,
		.timeout = 0, 
	},
	{ 
		.port    = 3,
		.busy    = FALSE,
		.timeout = 0, 
	},
};

/* initial device status */
static struct cxd3774gf_status present = {

	.noise_cancel_mode           = NOISE_CANCEL_MODE_OFF,
	.noise_cancel_active         = FALSE,

	.sound_effect                = OFF,

	.playback_mute               = ON,
	.capture_mute                = ON,
	.master_volume               = 0,
	.master_gain                 = 0,

	.analog_playback_mute        = ON,
	.analog_stream_mute          = ON,
	.icx_playback_active         = 0,
	.std_playback_active         = 0,
	.capture_active              = 0,

	.mix_timed_mute              = OFF,
	.std_timed_mute              = OFF,
	.icx_timed_mute              = OFF,

	.uncg_mute                   = OFF,

	.output_device               = OUTPUT_DEVICE_NONE,
	.input_device                = INPUT_DEVICE_NONE,
	.headphone_amp               = HEADPHONE_AMP_NORMAL,
	.headphone_type              = NCHP_TYPE_BUNDLE,
	.jack_mode                   = JACK_MODE_HEADPHONE,
	.jack_status                 = JACK_STATUS_NONE,

	.pcm_monitoring              = OFF,

	.sample_rate                 = 44100,
};

static struct cxd3774gf_status back_up;

/**************************/
/*@ initialize / finalize */
/**************************/

int cxd3774gf_core_initialize(struct mutex * mutex)
{
	print_trace("%s()\n",__FUNCTION__);

	global_mutex=mutex;

	mutex_lock(global_mutex);

	INIT_WORK(&resume_work, do_resume_work);
	INIT_DELAYED_WORK(&pcm_event_work, do_pcm_event_work);

	mutex_init(&timed_mute_mutex);
	INIT_DELAYED_WORK(&timed_mute[0].work, do_timed_mute_work);
	INIT_DELAYED_WORK(&timed_mute[1].work, do_timed_mute_work);
	INIT_DELAYED_WORK(&timed_mute[2].work, do_timed_mute_work);
	INIT_DELAYED_WORK(&timed_mute[3].work, do_timed_mute_work);

	cxd3774gf_switch_smaster_mute(ON);
	cxd3774gf_switch_class_h_mute(ON);
	msleep(50);
	digiamp_switch_shunt_mute(ON);

	/* dummy 1.8V/2.85V default on */
	cxd3774gf_switch_logic_ldo(ON);
	cxd3774gf_switch_180_power(ON);
	cxd3774gf_switch_285_power(ON);
	digiamp_power_on();
	msleep(50);

	startup_chip(present.sample_rate);

	show_device_id();

	digiamp_initialize();

	/* dummy, dnc driver is not loaded. */
	cxd3774gf_dnc_initialize();

	/********************/

	cxd3774gf_set_mix_timed_mute(&present,OFF);
	present.mix_timed_mute = OFF;
	cxd3774gf_set_std_timed_mute(&present,OFF);
	present.std_timed_mute = OFF;
	cxd3774gf_set_icx_timed_mute(&present,OFF);
	present.icx_timed_mute = OFF;

	cxd3774gf_set_uncg_mute(&present,OFF);
	present.uncg_mute = OFF;

	cxd3774gf_set_analog_stream_mute(&present,OFF);
	present.analog_stream_mute = OFF;

	cxd3774gf_set_analog_playback_mute(&present,present.analog_playback_mute);

	cxd3774gf_set_playback_mute(&present,OFF);
	present.playback_mute = OFF;

	cxd3774gf_set_capture_mute(&present,OFF);
	present.capture_mute = OFF;

	change_output_device(
		present.output_device,
		HEADPHONE_AMP_SMASTER,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.headphone_amp = HEADPHONE_AMP_SMASTER;

	change_input_device(
		present.input_device
	);

	/* dummy, dnc driver is not loaded. */
	present.noise_cancel_active=cxd3774gf_dnc_judge(
		present.noise_cancel_mode,
		present.output_device,
		present.headphone_amp,
		present.jack_status,
		present.headphone_type
	);

	cxd3774gf_set_master_volume(&present,present.master_volume);

	cxd3774gf_set_master_gain(&present,30);
	present.master_gain=30;

	adjust_tone_control(
		present.output_device,
		present.headphone_amp,
		present.headphone_type,
		present.jack_status
	);

	adjust_device_gain(present.input_device);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_device,
		present.headphone_amp,
		present.noise_cancel_active,
		present.analog_playback_mute
	);

	/********************/

	cxd3774gf_set_output_device_mute(&present,OFF,TRUE);
	cxd3774gf_set_input_device_mute(&present,OFF);

	core_active=TRUE;

	initialized=TRUE;

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_core_finalize(void)
{
	print_trace("%s()\n",__FUNCTION__);

	cancel_work_sync(&resume_work);
	cancel_delayed_work_sync(&pcm_event_work);
	cancel_timed_mute();
	flush_work(&resume_work);
	flush_delayed_work(&pcm_event_work);
	flush_timed_mute();

	mutex_lock(global_mutex);

	if(!initialized){
		mutex_unlock(global_mutex);
		return(0);
	}

	initialized=FALSE;

	if(!core_active){
		mutex_unlock(global_mutex);
		return(0);
	}

	core_active=FALSE;

	cxd3774gf_set_output_device_mute(&present,ON,TRUE);
	cxd3774gf_set_input_device_mute(&present,ON);

	/********************/

	cxd3774gf_set_mix_timed_mute(&present,OFF);
	present.mix_timed_mute = OFF;
	cxd3774gf_set_std_timed_mute(&present,OFF);
	present.std_timed_mute = OFF;
	cxd3774gf_set_icx_timed_mute(&present,OFF);
	present.icx_timed_mute = OFF;

	cxd3774gf_set_uncg_mute(&present,OFF);
	present.uncg_mute = OFF;

	cxd3774gf_set_analog_stream_mute(&present,ON);
	present.analog_stream_mute=ON;

	cxd3774gf_set_analog_playback_mute(&present,ON);
	present.analog_playback_mute=ON;

	cxd3774gf_set_playback_mute(&present,ON);
	present.playback_mute=ON;

	cxd3774gf_set_capture_mute(&present,ON);
	present.capture_mute=ON;

	change_output_device(
		OUTPUT_DEVICE_NONE,
		HEADPHONE_AMP_NORMAL,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.output_device=OUTPUT_DEVICE_NONE;
	present.headphone_amp=HEADPHONE_AMP_NORMAL;

	change_input_device(
		INPUT_DEVICE_NONE
	);
	present.input_device=INPUT_DEVICE_NONE;

	present.noise_cancel_active=cxd3774gf_dnc_judge(
		NOISE_CANCEL_MODE_OFF,
		present.output_device,
		present.headphone_amp,
		present.jack_status,
		present.headphone_type
	);
	present.noise_cancel_mode=NOISE_CANCEL_MODE_OFF;

	cxd3774gf_set_master_volume(&present,0);
	present.master_gain=0;

	cxd3774gf_set_master_gain(&present,0);
	present.master_volume=0;

	adjust_tone_control(
		present.output_device,
		present.headphone_amp,
		present.headphone_type,
		present.jack_status
	);

	adjust_device_gain(present.input_device);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_device,
		present.headphone_amp,
		present.noise_cancel_active,
		present.analog_playback_mute
	);

	/********************/

	cxd3774gf_dnc_shutdown();

	digiamp_shutdown();

	shutdown_chip();

	digiamp_power_off();
	cxd3774gf_switch_285_power(OFF);
	cxd3774gf_switch_180_power(OFF);
	msleep(20);
	cxd3774gf_switch_logic_ldo(OFF);

	cxd3774gf_switch_smaster_mute(ON);
	cxd3774gf_switch_class_h_mute(ON);
	msleep(50);
	digiamp_switch_shunt_mute(OFF);

	mutex_unlock(global_mutex);

	return(0);
}

/*******************/
/*@ resume/suspend */
/*******************/

int cxd3774gf_suspend(void)
{
	print_trace("%s()\n",__FUNCTION__);

	flush_delayed_work(&pcm_event_work);
	flush_timed_mute();

	mutex_lock(global_mutex);

	if(initialized)
		suspend_core();

	return(0);
}

int cxd3774gf_resume(void)
{
	print_trace("%s()\n",__FUNCTION__);

	schedule_work(&resume_work);

	return(0);
}

static void do_resume_work(struct work_struct * work)
{
	print_trace("%s()\n",__FUNCTION__);

	if(initialized && check_active(&back_up))
		resume_core();

	mutex_unlock(global_mutex);

	return;
}

/*********************/
/*@ startup/shutdown */
/*********************/

int cxd3774gf_startup(int icx_playback, int std_playback, int capture)
{
	print_trace("%s(%d,%d,%d)\n",__FUNCTION__,icx_playback,std_playback,capture);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	resume_core();

	if(present.output_device==OUTPUT_DEVICE_SPEAKER){
		/* if mute is off, speaker is alrady used. */
		if(present.analog_playback_mute==ON){
			if( (icx_playback || std_playback)
			&& present.icx_playback_active==0
			&& present.std_playback_active==0)
				switch_speaker_power(ON);
		}
	}

	if(icx_playback)
		present.icx_playback_active++;

	if(std_playback)
		present.std_playback_active++;

	if(capture)
		present.capture_active++;

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_shutdown(int icx_playback, int std_playback, int capture)
{
	print_trace("%s(%d,%d,%d)\n",__FUNCTION__,icx_playback,std_playback,capture);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(icx_playback)
		present.icx_playback_active--;

	if(std_playback)
		present.std_playback_active--;

	if(capture)
		present.capture_active--;

#ifdef ICX_ENABLE_AU2CLK
	change_external_osc(&present);
#endif

	if(present.output_device==OUTPUT_DEVICE_SPEAKER){
		/* if mute is off, speaker is still used. */
		if(present.analog_playback_mute==ON){
			if( (icx_playback || std_playback)
			&& present.icx_playback_active==0
			&& present.std_playback_active==0)
				switch_speaker_power(OFF);
		}
	}

	if(!check_active(&present))
		suspend_core();

	mutex_unlock(global_mutex);

	return(0);
}

/******************/
/*@ sampling_rate */
/******************/

int cxd3774gf_set_icx_playback_dai_rate(unsigned int rate)
{
	print_trace("%s(%u)\n",__FUNCTION__,rate);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	change_sample_rate(rate);
	present.sample_rate=rate;

#ifdef ICX_ENABLE_AU2CLK
	change_external_osc(&present);
#endif

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_set_std_playback_dai_rate(unsigned int rate)
{
	print_trace("%s(%u)\n",__FUNCTION__,rate);

	mutex_lock(global_mutex);

#ifdef ICX_ENABLE_AU2CLK
	change_external_osc(&present);
#endif

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_set_capture_dai_rate(unsigned int rate)
{
	print_trace("%s(%u)\n",__FUNCTION__,rate);

	mutex_lock(global_mutex);

#ifdef ICX_ENABLE_AU2CLK
	change_external_osc(&present);
#endif

	mutex_unlock(global_mutex);

	return(0);
}

/***********************************/
/*@ noise_cancel_mode              */
/*    NOISE_CANCEL_MODE_OFF      0 */
/*    NOISE_CANCEL_MODE_OFFICE   1 */
/*    NOISE_CANCEL_MODE_TRAIN    2 */
/*    NOISE_CANCEL_MODE_AIRPLANE 3 */
/***********************************/

int cxd3774gf_put_noise_cancel_mode(int value)
{
	int switching;

	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>NOISE_CANCEL_MODE_MAX){
		print_error("invalid parameter, value = %d\n",value);
		return(-EINVAL);
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.noise_cancel_mode=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.noise_cancel_mode){
		mutex_unlock(global_mutex);
		return(0);
	}

	if((present.noise_cancel_mode==NOISE_CANCEL_MODE_OFF && value!=NOISE_CANCEL_MODE_OFF)
	|| (present.noise_cancel_mode!=NOISE_CANCEL_MODE_OFF && value==NOISE_CANCEL_MODE_OFF))
		switching=TRUE;
	else
		switching=FALSE;

	if(switching){
		cxd3774gf_set_output_device_mute(&present,ON,TRUE);
	}

	/********************/

	present.noise_cancel_active=cxd3774gf_dnc_judge(
		value,
		present.output_device,
		present.headphone_amp,
		present.jack_status,
		present.headphone_type
	);
	present.noise_cancel_mode=value;

	/********************/

	if(switching){
		present.pcm_monitoring=judge_pcm_monitoring(
			present.output_device,
			present.headphone_amp,
			present.noise_cancel_active,
			present.analog_playback_mute
		);

		cxd3774gf_set_master_volume(&present,present.master_volume);

		cxd3774gf_set_output_device_mute(&present,OFF,TRUE);
	}

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_noise_cancel_mode(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.noise_cancel_mode;
	else
		*value=back_up.noise_cancel_mode;

	mutex_unlock(global_mutex);

	return(0);
}

/************************/
/*@ noise_cancel_status */
/************************/

int cxd3774gf_get_noise_cancel_status(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	*value=present.noise_cancel_active;

	mutex_unlock(global_mutex);

	return(0);
}

/***************************/
/*@ user_noise_cancel_gain */
/*    0 - 30               */
/***************************/

int cxd3774gf_put_user_noise_cancel_gain(int index)
{
	int timeout;
	int rv;

	print_trace("%s(%d)\n",__FUNCTION__,index);

	rv=get_timed_mute(3,&timeout);
	if(rv<0)
		timeout=0;

	/* PDSWAMB30BUG-598 added "+1000" */
	if(timeout==0)
		set_timed_mute(3,500+1000);
	else
		set_timed_mute(3,150+1000);

	mutex_lock(global_mutex);

	if(timeout==0)
		msleep(350);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	rv=cxd3774gf_dnc_set_user_gain(index);
	if(rv<0){
		back_trace();
		mutex_unlock(global_mutex);
		return(rv);
	}

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_user_noise_cancel_gain(int * index)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	*index=0;

	cxd3774gf_dnc_get_user_gain(index);

	mutex_unlock(global_mutex);

	return(0);
}

/***************************/
/*@ base_noise_cancel_gain */
/*    0 - 50               */
/***************************/

int cxd3774gf_put_base_noise_cancel_gain(int left, int right)
{
	int rv;

	print_trace("%s(%d,%d)\n",__FUNCTION__,left,right);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	rv=cxd3774gf_dnc_set_base_gain(left,right);
	if(rv<0){
		back_trace();
		mutex_unlock(global_mutex);
		return(rv);
	}

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_base_noise_cancel_gain(int * left, int * right)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	*left=0;
	*right=0;

	cxd3774gf_dnc_get_base_gain(left,right);

	mutex_unlock(global_mutex);

	return(0);
}

/*******************************************/
/*@ exit_base_noise_cancel_gain_adjustment */
/*******************************************/

int cxd3774gf_exit_base_noise_cancel_gain_adjustment(int save)
{
	int rv;

	print_trace("%s(%d)\n",__FUNCTION__,save);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	rv=cxd3774gf_dnc_exit_base_gain_adjustment(save);
	if(rv<0){
		back_trace();
		mutex_unlock(global_mutex);
		return(rv);
	}

	mutex_unlock(global_mutex);

	return(0);
}

/*****************/
/*@ sound_effect */
/*    OFF 0      */
/*    ON  1      */
/*****************/

int cxd3774gf_put_sound_effect(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.sound_effect=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.sound_effect){
		mutex_unlock(global_mutex);
		return(0);
	}

	cxd3774gf_set_output_device_mute(&present,ON,TRUE);

	/********************/

	present.sound_effect=value;

	/********************/

	cxd3774gf_set_master_volume(&present,present.master_volume);

	cxd3774gf_set_output_device_mute(&present,OFF,TRUE);

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_sound_effect(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.sound_effect;
	else
		*value=back_up.sound_effect;

	mutex_unlock(global_mutex);

	return(0);
}

/******************/
/*@ master_volume */
/*    0 - 30      */
/******************/

int cxd3774gf_put_master_volume(int value)
{
	int now;

	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>MASTER_VOLUME_MAX){
		print_error("invalid parameter, value = %d\n",value);
		return(-EINVAL);
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.master_volume=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.master_volume){
		mutex_unlock(global_mutex);
		return(0);
	}

	/********************/

	now=present.master_volume;

	while(value!=now){

#ifdef ICX_ENABLE_VOL60STP
		if(value>now)
			now=minimum(now+3,value);
		else
			now=maximum(now-3,value);
#else
		if(value>now)
			now=minimum(now+1,value);
		else
			now=maximum(now-1,value);
#endif

		cxd3774gf_set_master_volume(&present,now);
		present.master_volume=now;
	}

	/* cxd3774gf_set_master_volume(&present,value); */
	/* present.master_volume=value; */

	/********************/

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_master_volume(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.master_volume;
	else
		*value=back_up.master_volume;

	mutex_unlock(global_mutex);

	return(0);
}

/****************/
/*@ master_gain */
/*    0 - 30    */
/****************/

int cxd3774gf_put_master_gain(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>MASTER_GAIN_MAX){
		print_error("invalid parameter, value = %d\n",value);
		return(-EINVAL);
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.master_gain=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.master_gain){
		mutex_unlock(global_mutex);
		return(0);
	}

	/********************/

	cxd3774gf_set_master_gain(&present,value);
	present.master_gain=value;

	/********************/

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_master_gain(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.master_gain;
	else
		*value=back_up.master_gain;

	mutex_unlock(global_mutex);

	return(0);
}

/******************/
/*@ playback_mute */
/*    OFF 0       */
/*    ON  1       */
/******************/

int cxd3774gf_put_playback_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.playback_mute=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.playback_mute){
		mutex_unlock(global_mutex);
		return(0);
	}

	/********************/

	cxd3774gf_set_playback_mute(&present,value);
	present.playback_mute=value;

	/********************/

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_playback_mute(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.playback_mute;
	else
		*value=back_up.playback_mute;

	mutex_unlock(global_mutex);

	return(0);
}

/*****************/
/*@ capture_mute */
/*    OFF 0      */
/*    ON  1      */
/*****************/

int cxd3774gf_put_capture_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.capture_mute=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.capture_mute){
		mutex_unlock(global_mutex);
		return(0);
	}

	/********************/

	cxd3774gf_set_capture_mute(&present,value);
	present.capture_mute=value;

	/********************/

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_capture_mute(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.capture_mute;
	else
		*value=back_up.capture_mute;

	mutex_unlock(global_mutex);

	return(0);
}

/*************************/
/*@ analog_playback_mute */
/*    OFF 0              */
/*    ON  1              */
/*************************/

int cxd3774gf_put_analog_playback_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.analog_playback_mute=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.analog_playback_mute){
		mutex_unlock(global_mutex);
		return(0);
	}

	/********************/

	if(present.output_device==OUTPUT_DEVICE_SPEAKER
	&& present.icx_playback_active==0
	&& present.std_playback_active==0){
		if(value==OFF)
			switch_speaker_power(ON);
	}

	cxd3774gf_set_analog_playback_mute(&present,value);
	present.analog_playback_mute=value;

	if(present.output_device==OUTPUT_DEVICE_SPEAKER
	&& present.icx_playback_active==0
	&& present.std_playback_active==0){
		if(value==ON)
			switch_speaker_power(OFF);
	}

	/********************/

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_device,
		present.headphone_amp,
		present.noise_cancel_active,
		present.analog_playback_mute
	);

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_analog_playback_mute(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.analog_playback_mute;
	else
		*value=back_up.analog_playback_mute;

	mutex_unlock(global_mutex);

	return(0);
}

/***********************/
/*@ analog_stream_mute */
/*    OFF 0            */
/*    ON  1            */
/***********************/

int cxd3774gf_put_analog_stream_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.analog_stream_mute=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.analog_stream_mute){
		mutex_unlock(global_mutex);
		return(0);
	}

	/********************/

	cxd3774gf_set_analog_stream_mute(&present,value);
	present.analog_stream_mute=value;

	/********************/

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_analog_stream_mute(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.analog_stream_mute;
	else
		*value=back_up.analog_stream_mute;

	mutex_unlock(global_mutex);

	return(0);
}

/***********************/
/*@ timed_mute         */
/*    OFF 0            */
/*    TIMEOUT  1- [ms] */
/***********************/

int cxd3774gf_put_timed_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	return(set_timed_mute(0,value));
}

int cxd3774gf_get_timed_mute(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	return(get_timed_mute(0,value));
}

int cxd3774gf_put_std_timed_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	return(set_timed_mute(1,value));
}

int cxd3774gf_get_std_timed_mute(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	return(get_timed_mute(1,value));
}

int cxd3774gf_put_icx_timed_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);


	return(set_timed_mute(2,value));
}

int cxd3774gf_get_icx_timed_mute(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	return(get_timed_mute(2,value));
}

static int set_timed_mute(int port, int value)
{
	unsigned long now;
	unsigned long delta;
	int mute;
	int changed;

	print_trace("%s(%d,%d)\n",__FUNCTION__,port,value);

	if(port<0 || port>3){
		print_error("port %d is invalid.\n",port);
		return(-1);
	}

	if(value==0)
		mute=OFF;
	else
		mute=ON;

	mutex_lock(&timed_mute_mutex);

	timed_mute[port].busy=TRUE;

	/* cancel_delayed_work_sync(&timed_mute[port].work); */
	flush_delayed_work(&timed_mute[port].work);

	timed_mute[port].busy=FALSE;

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	changed=FALSE;

	if(!core_active){

		switch(port){

			case 0:
				if(back_up.mix_timed_mute!=mute){
					back_up.mix_timed_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;

			case 1:
				if(back_up.std_timed_mute!=mute){
					back_up.std_timed_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;

			case 2:
				if(back_up.icx_timed_mute!=mute){
					back_up.icx_timed_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;

			case 3:
				if(back_up.uncg_mute!=mute){
					back_up.uncg_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;
		}
	}

	else{

		switch(port){

			case 0:
				if(present.mix_timed_mute!=mute){
					cxd3774gf_set_mix_timed_mute(&present,mute);
					present.mix_timed_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;

			case 1:
				if(present.std_timed_mute!=mute){
					cxd3774gf_set_std_timed_mute(&present,mute);
					present.std_timed_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;

			case 2:
				if(present.icx_timed_mute!=mute){
					cxd3774gf_set_icx_timed_mute(&present,mute);
					present.icx_timed_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;

			case 3:
				if(present.uncg_mute!=mute){
					cxd3774gf_set_uncg_mute(&present,mute);
					present.uncg_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;
		}
	}

	mutex_unlock(global_mutex);

	if(value==0){

		timed_mute[port].timeout=0;
		print_debug("timed_mute %d timeout = %lu\n",port,timed_mute[port].timeout);
	}

	else{

		delta=msecs_to_jiffies(value);
		now=jiffies;
		print_debug("now = %lu\n",now);

		print_debug("timed_mute %d timeout = %lu\n",port,timed_mute[port].timeout);

		if(changed || delta > timed_mute[port].timeout-now)
			timed_mute[port].timeout=now+delta;

		print_debug("timed_mute %d timeout = %lu\n",port,timed_mute[port].timeout);

		schedule_delayed_work(&timed_mute[port].work,timed_mute[port].timeout-now);

		print_debug("timed mute %d delayed = %lu\n",port,timed_mute[port].timeout-now);
	}

	mutex_unlock(&timed_mute_mutex);

	return(0);
}

static void do_timed_mute_work(struct work_struct * work)
{
	struct timed_mute_data * tmd;
	struct delayed_work * dw;
	int port;

	dw=container_of(work, struct delayed_work, work);
	tmd=container_of(dw, struct timed_mute_data, work);
	port=tmd->port;

	print_trace("%s(%d)\n",__FUNCTION__,port);

	if(tmd->busy){
		print_debug("timed mute %d = busy\n",port);
		return;
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return;
	}

	if(!core_active){

		switch(port){

			case 0:
				if(back_up.mix_timed_mute!=OFF){
					back_up.mix_timed_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;

			case 1:
				if(back_up.std_timed_mute!=OFF){
					back_up.std_timed_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;

			case 2:
				if(back_up.icx_timed_mute!=OFF){
					back_up.icx_timed_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;

			case 3:
				if(back_up.uncg_mute!=OFF){
					back_up.uncg_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;
		}
	}

	else{

		switch(port){

			case 0:
				if(present.mix_timed_mute!=OFF){
					cxd3774gf_set_mix_timed_mute(&present,OFF);
					present.mix_timed_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;

			case 1:
				if(present.std_timed_mute!=OFF){
					cxd3774gf_set_std_timed_mute(&present,OFF);
					present.std_timed_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;

			case 2:
				if(present.icx_timed_mute!=OFF){
					cxd3774gf_set_icx_timed_mute(&present,OFF);
					present.icx_timed_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;

			case 3:
				if(present.uncg_mute!=OFF){
					cxd3774gf_set_uncg_mute(&present,OFF);
					present.uncg_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;
		}
	}

	mutex_unlock(global_mutex);

	return;
}

static int get_timed_mute(int port, int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	if(port<0 || port>3){
		print_error("port %d is invalid.\n",port);
		return(-1);
	}

	mutex_lock(&timed_mute_mutex);
	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		mutex_unlock(&timed_mute_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		if(port==0){
			if(back_up.mix_timed_mute==OFF)
				*value=0;
			else
				*value=jiffies_to_msecs(timed_mute[port].timeout-jiffies);
		}
		else if(port==1){
			if(back_up.std_timed_mute==OFF)
				*value=0;
			else
				*value=jiffies_to_msecs(timed_mute[port].timeout-jiffies);
		}
		else if(port==2){
			if(back_up.icx_timed_mute==OFF)
				*value=0;
			else
				*value=jiffies_to_msecs(timed_mute[port].timeout-jiffies);
		}
		else{
			if(back_up.uncg_mute==OFF)
				*value=0;
			else
				*value=jiffies_to_msecs(timed_mute[port].timeout-jiffies);
		}
	}
	else{
		if(port==0){
			if(present.mix_timed_mute==OFF)
				*value=0;
			else
				*value=jiffies_to_msecs(timed_mute[port].timeout-jiffies);
		}
		else if(port==1){
			if(present.std_timed_mute==OFF)
				*value=0;
			else
				*value=jiffies_to_msecs(timed_mute[port].timeout-jiffies);
		}
		else if(port==2){
			if(present.icx_timed_mute==OFF)
				*value=0;
			else
				*value=jiffies_to_msecs(timed_mute[port].timeout-jiffies);
		}
		else{
			if(present.uncg_mute==OFF)
				*value=0;
			else
				*value=jiffies_to_msecs(timed_mute[port].timeout-jiffies);
		}
	}

	mutex_unlock(global_mutex);
	mutex_unlock(&timed_mute_mutex);

	return(0);
}

static int cancel_timed_mute(void)
{
	print_trace("%s()\n",__FUNCTION__);

	cancel_delayed_work(&timed_mute[0].work);
	cancel_delayed_work(&timed_mute[1].work);
	cancel_delayed_work(&timed_mute[2].work);
	cancel_delayed_work(&timed_mute[3].work);

	return(0);
}

static int flush_timed_mute(void)
{
	print_trace("%s()\n",__FUNCTION__);

	flush_delayed_work(&timed_mute[0].work);
	flush_delayed_work(&timed_mute[1].work);
	flush_delayed_work(&timed_mute[2].work);
	flush_delayed_work(&timed_mute[3].work);

	return(0);
}

/*******************************/
/*@ input_device               */
/*    INPUT_DEVICE_NONE      0 */
/*    INPUT_DEVICE_TUNER     1 */
/*    INPUT_DEVICE_MIC       2 */
/*    INPUT_DEVICE_LINE      3 */
/*    INPUT_DEVICE_DIRECTMIC 4 */
/*******************************/

int cxd3774gf_put_input_device(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>INPUT_DEVICE_MAX){
		print_error("invalid parameter, value = %d\n",value);
		return(-EINVAL);
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(value!=INPUT_DEVICE_NONE)
		resume_core();

	if(value==present.input_device){
		mutex_unlock(global_mutex);
		return(0);
	}

	cxd3774gf_set_input_device_mute(&present,ON);

	/********************/

	change_input_device(value);
	present.input_device=value;

	/********************/

	cxd3774gf_set_master_gain(&present,present.master_gain);

	adjust_device_gain(present.input_device);

	cxd3774gf_set_input_device_mute(&present,OFF);

	if(!check_active(&present))
		suspend_core();

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_input_device(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	*value=present.input_device;

	mutex_unlock(global_mutex);

	return(0);
}

/********************************/
/*@ output_device               */
/*    OUTPUT_DEVICE_NONE      0 */
/*    OUTPUT_DEVICE_HEADPHONE 1 */
/*    OUTPUT_DEVICE_LINE      2 */
/*    OUTPUT_DEVICE_SPEAKER   3 */
/*    OUTPUT_DEVICE_FIXEDLINE 4 */
/********************************/

int cxd3774gf_put_output_device(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>OUTPUT_DEVICE_MAX){
		print_error("invalid parameter, value = %d\n",value);
		return(-EINVAL);
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(value!=OUTPUT_DEVICE_NONE)
		resume_core();

	if(value==present.output_device){
		mutex_unlock(global_mutex);
		return(0);
	}

	cxd3774gf_set_output_device_mute(&present,ON,TRUE);

	/********************/

	change_output_device(
		value,
		present.headphone_amp,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.output_device=value;

	/********************/

	present.noise_cancel_active=cxd3774gf_dnc_judge(
		present.noise_cancel_mode,
		present.output_device,
		present.headphone_amp,
		present.jack_status,
		present.headphone_type
	);

	cxd3774gf_set_master_volume(&present,present.master_volume);

	adjust_tone_control(
		present.output_device,
		present.headphone_amp,
		present.headphone_type,
		present.jack_status
	);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_device,
		present.headphone_amp,
		present.noise_cancel_active,
		present.analog_playback_mute
	);

	cxd3774gf_set_output_device_mute(&present,OFF,TRUE);

	if(!check_active(&present))
		suspend_core();

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_output_device(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	*value=present.output_device;

	mutex_unlock(global_mutex);

	return(0);
}

/*******************************/
/*@ headphone_amp              */
/*    HEADPHONE_AMP_NORMAL   0 */
/*    HEADPHONE_AMP_SMASTER  1 */
/*******************************/

int cxd3774gf_put_headphone_amp(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>HEADPHONE_AMP_MAX){
		print_error("invalid parameter, value = %d\n",value);
		return(-EINVAL);
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.headphone_amp=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.headphone_amp){
		mutex_unlock(global_mutex);
		return(0);
	}

	if(present.output_device!=OUTPUT_DEVICE_HEADPHONE){
		present.headphone_amp=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	cxd3774gf_set_output_device_mute(&present,ON,TRUE);

	/********************/

	change_output_device(
		present.output_device,
		value,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.headphone_amp=value;

	/********************/

	present.noise_cancel_active=cxd3774gf_dnc_judge(
		present.noise_cancel_mode,
		present.output_device,
		present.headphone_amp,
		present.jack_status,
		present.headphone_type
	);

	cxd3774gf_set_master_volume(&present,present.master_volume);

	adjust_tone_control(
		present.output_device,
		present.headphone_amp,
		present.headphone_type,
		present.jack_status
	);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_device,
		present.headphone_amp,
		present.noise_cancel_active,
		present.analog_playback_mute
	);

	cxd3774gf_set_output_device_mute(&present,OFF,TRUE);

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_headphone_amp(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.headphone_amp;
	else
		*value=back_up.headphone_amp;

	mutex_unlock(global_mutex);

	return(0);
}

/****************************/
/*@ headphone_type          */
/*    NCHP_TYPE_NC_BUNDLE 0 */
/*    NCHP_TYPE_NC_MODEL1 1 */
/****************************/

int cxd3774gf_put_headphone_type(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>NCHP_TYPE_MAX){
		print_error("invalid parameter, value = %d\n",value);
		return(-EINVAL);
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.headphone_type=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.headphone_type){
		mutex_unlock(global_mutex);
		return(0);
	}

	cxd3774gf_set_output_device_mute(&present,ON,TRUE);

	/********************/

	present.headphone_type=value;

	/********************/

	present.noise_cancel_active=cxd3774gf_dnc_judge(
		present.noise_cancel_mode,
		present.output_device,
		present.headphone_amp,
		present.jack_status,
		present.headphone_type
	);

	adjust_tone_control(
		present.output_device,
		present.headphone_amp,
		present.headphone_type,
		present.jack_status
	);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_device,
		present.headphone_amp,
		present.noise_cancel_active,
		present.analog_playback_mute
	);

	cxd3774gf_set_output_device_mute(&present,OFF,TRUE);

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_headphone_type(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.headphone_type;
	else
		*value=back_up.headphone_type;

	mutex_unlock(global_mutex);

	return(0);
}

/****************************/
/*@ jack_mode               */
/*    JACK_MODE_HEADPHONE 0 */
/*    JACK_MDOE_ANTENNA   1 */
/****************************/

int cxd3774gf_put_jack_mode(int value)
{
	int status;

	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>JACK_MODE_MAX){
		print_error("invalid parameter, value = %d\n",value);
		return(-EINVAL);
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		back_up.jack_mode=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.jack_mode){
		mutex_unlock(global_mutex);
		return(0);
	}

	present.jack_mode=value;

	status=get_jack_status(present.noise_cancel_active,present.jack_mode); 

	report_jack_status(
		present.jack_status,
		status,
		FALSE
	);

	present.jack_status=status;

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_jack_mode(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.jack_mode;
	else
		*value=back_up.jack_mode;

	mutex_unlock(global_mutex);

	return(0);
}

/****************/
/*@ jack_status */
/****************/

int cxd3774gf_get_jack_status(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	*value=present.jack_status;

	mutex_unlock(global_mutex);

	return(0);
}

/************************/
/*@ register_dnc_module */
/************************/

int cxd3774gf_register_dnc_module(struct cxd3774gf_dnc_interface * interface)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(global_mutex==NULL){
		/* print_error("not initialized."); */
		return(-EBUSY);
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(interface!=NULL){

		interface->set_mic_bias_mode      = set_mic_bias_mode;
		interface->switch_dnc_power       = switch_dnc_power;
		interface->switch_ncmic_amp_power = switch_ncmic_amp_power;
		interface->modify_reg             = cxd3774gf_register_modify;
		interface->write_reg              = cxd3774gf_register_write;
		interface->read_reg               = cxd3774gf_register_read;
		interface->write_buf              = cxd3774gf_register_write_multiple;
		interface->read_buf               = cxd3774gf_register_read_multiple;
		interface->global_mutex           = global_mutex;

		cxd3774gf_dnc_register_module(interface);

		rv=cxd3774gf_dnc_initialize();
		if(rv<0){
			back_trace();
			mutex_unlock(global_mutex);
			return(rv);
		}

		/********/

		present.noise_cancel_active=cxd3774gf_dnc_judge(
			present.noise_cancel_mode,
			present.output_device,
			present.headphone_amp,
			present.jack_status,
			present.headphone_type
		);

		cxd3774gf_set_master_volume(&present,present.master_volume);

		cxd3774gf_set_master_gain(&present,present.master_gain);

		adjust_tone_control(
			present.output_device,
			present.headphone_amp,
			present.headphone_type,
			present.jack_status
		);

		present.pcm_monitoring=judge_pcm_monitoring(
			present.output_device,
			present.headphone_amp,
			present.noise_cancel_active,
			present.analog_playback_mute
		);

		/********/
	}

	else{
		cxd3774gf_dnc_shutdown();

		cxd3774gf_dnc_register_module(interface);

		/********/

		present.noise_cancel_active=cxd3774gf_dnc_judge(
			present.noise_cancel_mode,
			present.output_device,
			present.headphone_amp,
			present.jack_status,
			present.headphone_type
		);

		cxd3774gf_set_master_volume(&present,present.master_volume);

		cxd3774gf_set_master_gain(&present,present.master_gain);

		adjust_tone_control(
			present.output_device,
			present.headphone_amp,
			present.headphone_type,
			present.jack_status
		);

		present.pcm_monitoring=judge_pcm_monitoring(
			present.output_device,
			present.headphone_amp,
			present.noise_cancel_active,
			present.analog_playback_mute
		);

		/********/
	}

	mutex_unlock(global_mutex);

	return(0);
}
EXPORT_SYMBOL(cxd3774gf_register_dnc_module);

/**********************/
/*@ check_jack_status */
/**********************/

int cxd3774gf_check_jack_status(int force)
{
	int status;
	int backup;
	int rv;
	unsigned int ui;

	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(!core_active){
		mutex_unlock(global_mutex);
		return(0);
	}

	status=get_jack_status(present.noise_cancel_active,present.jack_mode);

	/* for electrostatic damege. */
	while(1){
		rv=cxd3774gf_register_read(CXD3774GF_DAC,&ui);
		if(rv!=0 || (ui&0x40)==0x40)
			break;

		print_info("detect electrostatic damages.\n");
		suspend_core();
		resume_core();
		if(repair_electrostatic_damage!=NULL)
			repair_electrostatic_damage();
		status=get_jack_status(present.noise_cancel_active,present.jack_mode);
	}

	report_jack_status(
		present.jack_status,
		status,
		force
	);

	backup=present.jack_status;
	present.jack_status=status;

	if(present.output_device==OUTPUT_DEVICE_HEADPHONE){

		if( (present.jack_status==JACK_STATUS_5PIN && backup==JACK_STATUS_3PIN)
		||  (present.jack_status==JACK_STATUS_3PIN && backup==JACK_STATUS_5PIN) ){

			cxd3774gf_set_output_device_mute(&present,ON,TRUE);

			adjust_tone_control(
				present.output_device,
				present.headphone_amp,
				present.headphone_type,
				present.jack_status
			);

			present.noise_cancel_active=cxd3774gf_dnc_judge(
				present.noise_cancel_mode,
				present.output_device,
				present.headphone_amp,
				present.jack_status,
				present.headphone_type
			);

			cxd3774gf_set_master_volume(&present,present.master_volume);

			present.pcm_monitoring=judge_pcm_monitoring(
				present.output_device,
				present.headphone_amp,
				present.noise_cancel_active,
				present.analog_playback_mute
			);

			cxd3774gf_set_output_device_mute(&present,OFF,TRUE);
		}
	}

	mutex_unlock(global_mutex);

	return(0);
}

/*********************/
/*@ handle_pcm_event */
/*********************/

int cxd3774gf_handle_pcm_event(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(present.pcm_monitoring==OFF)
		return(0);

	rv=cxd3774gf_get_xpcm_det_value();
	if(rv){
		/* mute off */
		schedule_delayed_work(&pcm_event_work, 0);
	}
	else{
		/* mute on */
		schedule_delayed_work(&pcm_event_work, msecs_to_jiffies(PWM_OUT_MUTE_DEALY_2));
	}

	return(0);
}

static void do_pcm_event_work(struct work_struct * work)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return;
	}

	if(!core_active){
		mutex_unlock(global_mutex);
		return;
	}

	if(present.pcm_monitoring==OFF){
		mutex_unlock(global_mutex);
		return;
	}

	if(present.output_device==OUTPUT_DEVICE_HEADPHONE && present.headphone_amp==HEADPHONE_AMP_SMASTER){
		rv=cxd3774gf_get_xpcm_det_value();
		if(rv)
			cxd3774gf_set_no_pcm_mute(OFF);
		else
			cxd3774gf_set_no_pcm_mute(ON);
	}
	else{
		cxd3774gf_set_no_pcm_mute(ON);
	}

	mutex_unlock(global_mutex);

	return;
}

/***********************/
/*@ apply_table_change */
/***********************/

int cxd3774gf_apply_table_change(int id)
{
	print_trace("%s(%d)\n",__FUNCTION__,id);

	if(!core_active)
		return(0);

	if(id==TABLE_ID_MASTER_VOLUME){
		cxd3774gf_set_master_volume(&present,present.master_volume);
	}
	else if(id==TABLE_ID_DEVICE_GAIN){
		adjust_device_gain(present.input_device);
	}
	else if(id==TABLE_ID_TONE_CONTROL){
		adjust_tone_control(
			present.output_device,
			present.headphone_amp,
			present.headphone_type,
			present.jack_status
		);
	}

	return(0);
}

/*************************/
/*@ electrostatic_damage */
/*************************/

void cxd3774gf_register_electrostatic_damage_repairer(int (*function)(void))
{
	print_trace("%s(0x%08X)\n",__FUNCTION__,(unsigned int)function);

	if(global_mutex==NULL)
		return;

	mutex_lock(global_mutex);

	repair_electrostatic_damage=function;

	mutex_unlock(global_mutex);

	return;
}
EXPORT_SYMBOL(cxd3774gf_register_electrostatic_damage_repairer);

/***************/
/*@ debug_test */
/***************/

int cxd3774gf_debug_test = 0;

int cxd3774gf_put_debug_test(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	mutex_lock(global_mutex);

	cxd3774gf_debug_test=value;

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3774gf_get_debug_test(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	*value=cxd3774gf_debug_test;

	mutex_unlock(global_mutex);

	return(0);
}

/********************/
/*@ common_routines */
/********************/

static int suspend_core(void)
{
	print_trace("%s()\n",__FUNCTION__);

	if(!core_active)
		return(0);

	core_active=FALSE;

	/**********/

	back_up=present;

	cxd3774gf_set_output_device_mute(&present,ON,TRUE);
	cxd3774gf_set_input_device_mute(&present,ON);

	cxd3774gf_set_mix_timed_mute(&present,ON);
	present.mix_timed_mute = ON;
	cxd3774gf_set_std_timed_mute(&present,ON);
	present.std_timed_mute = ON;
	cxd3774gf_set_icx_timed_mute(&present,ON);
	present.icx_timed_mute = ON;

	cxd3774gf_set_uncg_mute(&present,ON);
	present.uncg_mute = ON;

	cxd3774gf_set_analog_stream_mute(&present,ON);
	present.analog_stream_mute=ON;

	cxd3774gf_set_analog_playback_mute(&present,ON);
	present.analog_playback_mute=ON;

	cxd3774gf_set_playback_mute(&present,ON);
	present.playback_mute=ON;

	cxd3774gf_set_capture_mute(&present,ON);
	present.capture_mute=ON;

	change_output_device(
		OUTPUT_DEVICE_NONE,
		HEADPHONE_AMP_NORMAL,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.output_device=OUTPUT_DEVICE_NONE;
	present.headphone_amp=HEADPHONE_AMP_NORMAL;

	change_input_device(
		INPUT_DEVICE_NONE
	);
	present.input_device=INPUT_DEVICE_NONE;

	present.noise_cancel_active=cxd3774gf_dnc_judge(
		NOISE_CANCEL_MODE_OFF,
		present.output_device,
		present.headphone_amp,
		present.jack_status,
		present.headphone_type
	);
	present.noise_cancel_mode=NOISE_CANCEL_MODE_OFF;

	cxd3774gf_set_master_volume(&present,0);
	present.master_gain=0;

	cxd3774gf_set_master_gain(&present,0);
	present.master_volume=0;

	adjust_tone_control(
		present.output_device,
		present.headphone_amp,
		present.headphone_type,
		present.jack_status
	);

	adjust_device_gain(present.input_device);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_device,
		present.headphone_amp,
		present.noise_cancel_active,
		present.analog_playback_mute
	);

	/**********/

	cxd3774gf_dnc_cleanup();

	shutdown_chip();

	digiamp_switch_shunt_mute(OFF);

	return(0);
}

static int resume_core(void)
{
	print_trace("%s()\n",__FUNCTION__);

	if(core_active)
		return(0);

	digiamp_switch_shunt_mute(ON);

	startup_chip(present.sample_rate);

	cxd3774gf_dnc_prepare();

	/**********/

	present.sound_effect = back_up.sound_effect;
	/* present.jack_mode = back_up.jack_mode; */

	cxd3774gf_set_mix_timed_mute(&present,back_up.mix_timed_mute);
	present.mix_timed_mute = back_up.mix_timed_mute;
	cxd3774gf_set_std_timed_mute(&present,back_up.std_timed_mute);
	present.std_timed_mute = back_up.std_timed_mute;
	cxd3774gf_set_icx_timed_mute(&present,back_up.icx_timed_mute);
	present.icx_timed_mute = back_up.icx_timed_mute;

	cxd3774gf_set_uncg_mute(&present,back_up.uncg_mute);
	present.uncg_mute = back_up.uncg_mute;

	cxd3774gf_set_analog_stream_mute(&present,back_up.analog_stream_mute);
	present.analog_stream_mute=back_up.analog_stream_mute;

	cxd3774gf_set_analog_playback_mute(&present,back_up.analog_playback_mute);
	present.analog_playback_mute=back_up.analog_playback_mute;

	cxd3774gf_set_playback_mute(&present,back_up.playback_mute);
	present.playback_mute=back_up.playback_mute;

	cxd3774gf_set_capture_mute(&present,back_up.capture_mute);
	present.capture_mute=back_up.capture_mute;

	change_output_device(
		back_up.output_device,
		back_up.headphone_amp,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.output_device=back_up.output_device;
	present.headphone_amp=back_up.headphone_amp;

	change_input_device(
		back_up.input_device
	);
	present.input_device=back_up.input_device;

	present.noise_cancel_active=cxd3774gf_dnc_judge(
		back_up.noise_cancel_mode,
		present.output_device,
		present.headphone_amp,
		present.jack_status,
		present.headphone_type
	);
	present.noise_cancel_mode=back_up.noise_cancel_mode;

	cxd3774gf_set_master_volume(&present,back_up.master_volume);
	present.master_volume=back_up.master_volume;

	cxd3774gf_set_master_gain(&present,back_up.master_gain);
	present.master_gain=back_up.master_gain;

	adjust_tone_control(
		present.output_device,
		present.headphone_amp,
		present.headphone_type,
		present.jack_status
	);

	adjust_device_gain(present.input_device);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_device,
		present.headphone_amp,
		present.noise_cancel_active,
		present.analog_playback_mute
	);

	cxd3774gf_set_input_device_mute(&present,OFF);
	cxd3774gf_set_output_device_mute(&present,OFF,TRUE);

	/**********/

	core_active=TRUE;

	return(0);
}

static int change_output_device(
	int output_device,
	int headphone_amp,
	int analog_playback_mute,
	int icx_playback_active,
	int std_playback_active,
	int capture_active,
	unsigned int sample_rate
)
{
#ifdef ICX_ENABLE_AU2CLK
	int osc;
#endif

	print_trace(
		"%s(%d,%d,%d,%d,%d,%d,%u)\n",
		__FUNCTION__,
		output_device,
		headphone_amp,
		analog_playback_mute,
		icx_playback_active,
		std_playback_active,
		capture_active,
		sample_rate
	);

	/* for MCLK change */
	cxd3774gf_dnc_off();

#ifdef ICX_ENABLE_AU2CLK
	/* suitable external osc */
	osc=suitable_external_osc(
		output_device,
		headphone_amp,
		icx_playback_active,
		std_playback_active,
		capture_active,
		sample_rate
	);
#endif

	if(output_device==OUTPUT_DEVICE_HEADPHONE){

		if(headphone_amp==HEADPHONE_AMP_SMASTER){
			/* --- OFF --- */
			switch_speaker_power(OFF);
			switch_speaker(OFF);
			switch_lineout(OFF);
			switch_classh(OFF);
			switch_dac(OFF);
			switch_cpclk(OFF);
			/* --- CLK --- */
#ifdef ICX_ENABLE_AU2CLK
			cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x00,0x01);
			cxd3774gf_switch_external_osc(osc);
			cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x01,0x01);
#endif
			/* --- ON --- */
			switch_smaster(ON);
		}

		else{
			/* --- OFF --- */
			switch_speaker_power(OFF);
			switch_speaker(OFF);
			switch_lineout(OFF);
			switch_smaster(OFF);
			/* --- CLK --- */
#ifdef ICX_ENABLE_AU2CLK
			cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x00,0x01);
			cxd3774gf_switch_external_osc(osc);
			cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x01,0x01);
#endif
			/* --- ON --- */
			switch_cpclk(ON);
			switch_dac(ON);
			switch_classh(ON);
		}
	}

	else if(output_device==OUTPUT_DEVICE_SPEAKER){
		/* --- OFF --- */
		if(icx_playback_active==0 && std_playback_active==0 && analog_playback_mute==ON)
			switch_speaker_power(OFF);
		switch_lineout(OFF);
		switch_classh(OFF);
		switch_smaster(OFF);
		/* --- CLK --- */
#ifdef ICX_ENABLE_AU2CLK
		cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x00,0x01);
		cxd3774gf_switch_external_osc(osc);
		cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x01,0x01);
#endif
		/* --- ON --- */
		switch_cpclk(ON);
		switch_dac(ON);
		switch_speaker(ON);
		if(icx_playback_active>0 || std_playback_active>0 || analog_playback_mute==OFF)
			switch_speaker_power(ON);
	}

	else if(output_device==OUTPUT_DEVICE_LINE){
		/* --- OFF --- */
		switch_speaker_power(OFF);
		switch_speaker(OFF);
		switch_classh(OFF);
		switch_smaster(OFF);
		/* --- CLK --- */
#ifdef ICX_ENABLE_AU2CLK
		cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x00,0x01);
		cxd3774gf_switch_external_osc(osc);
		cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x01,0x01);
#endif
		/* --- ON --- */
		switch_cpclk(ON);
		switch_dac(ON);
		switch_lineout(ON);
	}

	else if(output_device==OUTPUT_DEVICE_FIXEDLINE){
		/* --- OFF --- */
		switch_speaker_power(OFF);
		switch_speaker(OFF);
		switch_classh(OFF);
		switch_smaster(OFF);
		/* --- CLK --- */
#ifdef ICX_ENABLE_AU2CLK
		cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x00,0x01);
		cxd3774gf_switch_external_osc(osc);
		cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x01,0x01);
#endif
		/* --- ON --- */
		switch_cpclk(ON);
		switch_dac(ON);
		switch_lineout(ON);
	}

	else{ /* NONE */
		/* --- OFF --- */
		switch_speaker_power(OFF);
		switch_speaker(OFF);
		switch_lineout(OFF);
		switch_classh(OFF);
		switch_smaster(OFF);
		switch_dac(OFF);
		switch_cpclk(OFF);
		/* --- CLK --- */
#ifdef ICX_ENABLE_AU2CLK
		cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x00,0x01);
		cxd3774gf_switch_external_osc(osc);
		cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x01,0x01);
#endif
		/* --- ON --- */
	}

	return(0);
}

static int change_input_device(int input_device)
{
	print_trace("%s(%d)\n",__FUNCTION__,input_device);

	switch(input_device){

		case INPUT_DEVICE_LINE:
			switch_tuner(OFF);
			switch_dmic(OFF);
			switch_linein(ON);
			break;

		case INPUT_DEVICE_MIC:
			switch_tuner(OFF);
			switch_linein(OFF);
			switch_dmic(ON);
			break;

		case INPUT_DEVICE_TUNER:
			switch_linein(OFF);
			switch_dmic(OFF);
			switch_tuner(ON);
			break;

		case INPUT_DEVICE_DIRECTMIC:
			switch_tuner(OFF);
			switch_linein(OFF);
			switch_dmic(ON);
			break;

		/* NONE */
		default:
			switch_tuner(OFF);
			switch_linein(OFF);
			switch_dmic(OFF);
			break;
	}

	return(0);
}

static int change_sample_rate(unsigned int rate)
{
	print_trace("%s(%u)\n",__FUNCTION__,rate);

	if(rate<=48000){
		cxd3774gf_register_modify(CXD3774GF_SRC,0x01,0x03);
	}
	else if(rate<=96000){
		cxd3774gf_register_modify(CXD3774GF_SRC,0x02,0x03);
	}
	else{
		cxd3774gf_register_modify(CXD3774GF_SRC,0x03,0x03);
	}

	return(0);
}

static int judge_pcm_monitoring(
	int output_device,
	int headphone_amp,
	int noise_cancel_active,
	int analog_playback_mute
)
{
	int judge;
	int rv;

	print_trace(
		"%s(%d,%d,%d,%d)\n",
		__FUNCTION__,
		output_device,
		headphone_amp,
		noise_cancel_active,
		analog_playback_mute
	);

	judge=ON;

	if(output_device!=OUTPUT_DEVICE_HEADPHONE)
		judge=OFF;

	if(headphone_amp!=HEADPHONE_AMP_SMASTER)
		judge=OFF;

	if(analog_playback_mute==OFF)
		judge=OFF;

	if(noise_cancel_active==ON)
		judge=OFF;

	if(judge==ON){

		print_debug("pcm monitoring = on\n");

		rv=cxd3774gf_get_xpcm_det_value();
		if(rv)
			cxd3774gf_set_no_pcm_mute(OFF);
		else
			cxd3774gf_set_no_pcm_mute(ON);
	}

	else{
		cancel_delayed_work(&pcm_event_work);

		print_debug("pcm monitoring = off\n");

		if(output_device==OUTPUT_DEVICE_HEADPHONE && headphone_amp==HEADPHONE_AMP_SMASTER)
			cxd3774gf_set_no_pcm_mute(OFF);
		else
			cxd3774gf_set_no_pcm_mute(ON);
	}

	return(judge);
}

static int adjust_tone_control(
	int output_device,
	int headphone_amp,
	int headphone_type,
	int jack_status
)
{
	int table;

	print_trace(
		"%s(%d,%d,%d,%d)\n",
		__FUNCTION__,
		output_device,
		headphone_amp,
		headphone_type,
		jack_status
	);

	switch(output_device){

		case OUTPUT_DEVICE_HEADPHONE:

			switch(jack_status){
				case JACK_STATUS_5PIN:
					if(headphone_amp==HEADPHONE_AMP_SMASTER){
						if(headphone_type==NCHP_TYPE_MODEL1)
							table=TONE_CONTROL_TABLE_SAMP_MODEL1_NCHP;
						else
							table=TONE_CONTROL_TABLE_SAMP_BUNDLE_NCHP;
					}
					else{
						if(headphone_type==NCHP_TYPE_MODEL1)
							table=TONE_CONTROL_TABLE_NAMP_MODEL1_NCHP;
						else
							table=TONE_CONTROL_TABLE_NAMP_BUNDLE_NCHP;
					}
					break;

				case JACK_STATUS_ANTENNA:
				case JACK_STATUS_3PIN:
				case JACK_STATUS_NONE:
				default:
					if(headphone_amp==HEADPHONE_AMP_SMASTER)
						table=TONE_CONTROL_TABLE_SAMP_GENERAL_HP;
					else
						table=TONE_CONTROL_TABLE_NAMP_GENERAL_HP;
					break;
			}
			break;

		case OUTPUT_DEVICE_SPEAKER:
		case OUTPUT_DEVICE_LINE:
		case OUTPUT_DEVICE_FIXEDLINE:
		case OUTPUT_DEVICE_NONE:
		default:
			table=TONE_CONTROL_TABLE_NO_HP;
			break;
	}

	cxd3774gf_register_modify(CXD3774GF_DEQ_CONTROL,0x00,0x80);

	cxd3774gf_register_write_multiple(
		CXD3774GF_DEQ_1_COEF_BASE,
		(unsigned char *)&cxd3774gf_tone_control_table[table].coefficient[0],
		sizeof(struct cxd3774gf_deq_coefficient)
	);
	cxd3774gf_register_write_multiple(
		CXD3774GF_DEQ_2_COEF_BASE,
		(unsigned char *)&cxd3774gf_tone_control_table[table].coefficient[1],
		sizeof(struct cxd3774gf_deq_coefficient)
	);
	cxd3774gf_register_write_multiple(
		CXD3774GF_DEQ_3_COEF_BASE,
		(unsigned char *)&cxd3774gf_tone_control_table[table].coefficient[2],
		sizeof(struct cxd3774gf_deq_coefficient)
	);
	cxd3774gf_register_write_multiple(
		CXD3774GF_DEQ_4_COEF_BASE,
		(unsigned char *)&cxd3774gf_tone_control_table[table].coefficient[3],
		sizeof(struct cxd3774gf_deq_coefficient)
	);
	cxd3774gf_register_write_multiple(
		CXD3774GF_DEQ_5_COEF_BASE,
		(unsigned char *)&cxd3774gf_tone_control_table[table].coefficient[4],
		sizeof(struct cxd3774gf_deq_coefficient)
	);

	cxd3774gf_register_modify(CXD3774GF_DEQ_CONTROL,0x80,0x80);

	print_debug("tone control: table=%d\n",table);

	return(0);
}

static int adjust_device_gain(int input_device)
{
	print_trace("%s(%d)\n",__FUNCTION__,input_device);

	cxd3774gf_register_write(CXD3774GF_PGA_1L,cxd3774gf_device_gain_table[input_device].pga);
	cxd3774gf_register_write(CXD3774GF_PGA_1R,cxd3774gf_device_gain_table[input_device].pga);
	cxd3774gf_register_write(CXD3774GF_ADC_1L,cxd3774gf_device_gain_table[input_device].adc);
	cxd3774gf_register_write(CXD3774GF_ADC_1R,cxd3774gf_device_gain_table[input_device].adc);

	return(0);
}

static int report_jack_status(
	int old_status,
	int new_status,
	int force
)
{
	/* print_trace("%s(%d,%d,%d)\n",__FUNCTION__,old_status,new_status,force); */

	if(new_status!=old_status){

		if(new_status==JACK_STATUS_5PIN){
			print_info("hp(5pin) detect.\n");
		}
		else if(new_status==JACK_STATUS_3PIN){
			print_info("hp(3pin) detect.\n");
		}
		else if(new_status==JACK_STATUS_ANTENNA){
			print_info("antenna detect.\n");
		}
		else{
			print_info("no hp.\n");
		}

		if((force || old_status==JACK_STATUS_NONE || old_status==JACK_STATUS_ANTENNA)
		&& (         new_status==JACK_STATUS_3PIN || new_status==JACK_STATUS_5PIN   )){
			/* for WiredAccessoryObserver.java */
			/* 2 = indicate headphone          */
			cxd3774gf_switch_set_headphone_value(2);
		}
		if((force || old_status==JACK_STATUS_3PIN || old_status==JACK_STATUS_5PIN   )
		&& (         new_status==JACK_STATUS_NONE || new_status==JACK_STATUS_ANTENNA)){
			/* for WiredAccessoryObserver.java */
			/* 0 = indicate no headphone       */
			cxd3774gf_switch_set_headphone_value(0);
		}
		if((force || old_status==JACK_STATUS_NONE) && (new_status!=JACK_STATUS_NONE)){
			cxd3774gf_switch_set_antenna_value(1);
		}
		if((force || old_status!=JACK_STATUS_NONE) && (new_status==JACK_STATUS_NONE)){
			cxd3774gf_switch_set_antenna_value(0);
		}
	}

	return(0);
}

#ifdef ICX_ENABLE_AU2CLK

static int change_external_osc(
	struct cxd3774gf_status * status
)
{
	int now;
	int req;

	print_trace("%s()\n",__FUNCTION__);

	now=cxd3774gf_get_external_osc();

	req=suitable_external_osc(
		status->output_device,
		status->headphone_amp,
		status->icx_playback_active,
		status->std_playback_active,
		status->capture_active,
		status->sample_rate
	);

	if(req==EXTERNAL_OSC_KEEP){
		print_debug("external OSC = KEEP\n");
		return(0);
	}

	if(req!=now){

		cxd3774gf_set_output_device_mute(status,ON,TRUE);

		if(status->output_device==OUTPUT_DEVICE_HEADPHONE
		&& status->headphone_amp==HEADPHONE_AMP_SMASTER){

			/* PCMCLKO=off */
			cxd3774gf_register_modify(CXD3774GF_DAC,0x80,0x80);

			/* disable digital amp */
			digiamp_disable(DAMP_CTL_LEVEL_RESET);
		}

		/* MCLK=off */
		cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x00,0x01);

		/* change master clock */
		cxd3774gf_switch_external_osc(req);

		/* MCLK=on */
		cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0x01,0x01);

		if(status->output_device==OUTPUT_DEVICE_HEADPHONE
		&& status->headphone_amp==HEADPHONE_AMP_SMASTER){

			/* enable digital amp */
			digiamp_enable(DAMP_CTL_LEVEL_RESET);

			/* PCMCLKO=on */
			cxd3774gf_register_modify(CXD3774GF_DAC,0x00,0x80);
		}

		cxd3774gf_set_output_device_mute(status,OFF,TRUE);
	}
	else{
		print_debug("external OSC = SAME\n");
	}

	return(0);
}

static int suitable_external_osc(
	int output_device,
	int headphone_amp,
	int icx_playback_active,
	int std_playback_active,
	int capture_active,
	unsigned int sample_rate
)
{
	int osc;

	print_trace(
		"%s(%d,%d,%d,%d,%d,%u)\n",
		__FUNCTION__,
		output_device,
		headphone_amp,
		icx_playback_active,
		std_playback_active,
		capture_active,
		sample_rate
	);

	if(capture_active>0){
		osc=EXTERNAL_OSC_441;
	}
	/* else if(output_device!=OUTPUT_DEVICE_HEADPHONE || headphone_amp!=HEADPHONE_AMP_SMASTER){ */
		/* osc=EXTERNAL_OSC_441; */
	/* } */
	else if(icx_playback_active>0){
		switch(sample_rate){
			case   8000:
			case  16000:
			case  32000:
			case  48000:
			case  64000:
			case  96000:
			case 192000:
				osc=EXTERNAL_OSC_480;
				break;
			case   5512:
			case  11025:
			case  22050:
			case  44100:
			case  88200:
			case 176400:
			default:
				osc=EXTERNAL_OSC_441;
				break;
		}
	}
	else if(std_playback_active>0){
		osc=EXTERNAL_OSC_441;
	}
	else{
		osc=EXTERNAL_OSC_KEEP;
	}

	return(osc);
}

#endif

static int check_active(struct cxd3774gf_status * status)
{
	int ret=FALSE;

	print_trace("%s()\n",__FUNCTION__);

	if(status->output_device!=OUTPUT_DEVICE_NONE)
		ret=TRUE;

	if(status->input_device!=INPUT_DEVICE_NONE)
		ret=TRUE;

	if(status->icx_playback_active>0)
		ret=TRUE;

	if(status->std_playback_active>0)
		ret=TRUE;

	if(status->capture_active>0)
		ret=TRUE;

	return(ret);
}

/*******************/
/*@ basic_routines */
/*******************/

static int startup_chip(unsigned int rate)
{
	print_trace("%s()\n",__FUNCTION__);

	/* external OSC on (only 1240/1265) */
#ifdef ICX_ENABLE_AU2CLK
	cxd3774gf_switch_external_osc(EXTERNAL_OSC_441);
#else
	cxd3774gf_switch_external_osc(EXTERNAL_OSC_480);
#endif

	/* unreset hardware */
	cxd3774gf_unreset();

	/* HPIBEN=0 */
	cxd3774gf_register_modify(CXD3774GF_TRIM_1,0x00,0x80);

	/* REG_TRM=4, BGR_TRM=5 */
	cxd3774gf_register_modify(CXD3774GF_TRIM_2,0x94,0xFC);

	/* PWMOUT_DS=8mA, PCMCLKO_DS=10mA */
	cxd3774gf_register_modify(CXD3774GF_IO_DRIVE,0x05,0x0F);

	/* REF=on, VCOM=on, MICDET=on, REGL=on */
	cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_2,0x00,0xF0);

	/* MCK=TCXO */
	cxd3774gf_register_modify(CXD3774GF_DAC,0x40,0x40);

	/* codec mode=high, I2S=slave, format=I2S, fs=1024 */
	cxd3774gf_register_modify(CXD3774GF_MODE_CONTROL,0xC2,0xFE);

	/* DSPC=on, DSPS1=on, DSPS2=on */
	cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x00,0xE0);

	/* DSPRAM_S, SRESET */
	cxd3774gf_register_modify(CXD3774GF_TEST_1,0x03,0x03);

	/* wait clear */
	msleep(20);

	/* SRC2IN=ADC, SRC2 mode=low, SRC1IN=ADC, SRC1 mode=XXX */
	if(rate<=48000){
		cxd3774gf_register_modify(CXD3774GF_SRC,0x99,0xFF);
	}
	else if(rate<=96000){
		cxd3774gf_register_modify(CXD3774GF_SRC,0x9A,0xFF);
	}
	else{
		cxd3774gf_register_modify(CXD3774GF_SRC,0x9B,0xFF);
	}

	/* mute_b=off, sdout2=on, sdout1=off, sdin2=on, sdin2=on */
	cxd3774gf_register_modify(CXD3774GF_SDO_CONTROL,0x0B,0x1F);

	/* DITHER */
	cxd3774gf_register_write(CXD3774GF_S_MASTER_DITHER_1,0x00);
	cxd3774gf_register_write(CXD3774GF_S_MASTER_DITHER_2,0x7F);
	cxd3774gf_register_write(CXD3774GF_S_MASTER_DITHER_3,0xB5);
	cxd3774gf_register_write(CXD3774GF_DITHER_MIC,0x8A);
	cxd3774gf_register_write(CXD3774GF_DITHER_LINE,0x8A);

	/* ARC=0dB fixed, HPF1=1.71Hz */
	cxd3774gf_register_modify(CXD3774GF_LINEIN_1,0x84,0xFC);

	/* CIC1IN=zero */
	cxd3774gf_register_modify(CXD3774GF_LINEIN_2,0x00,0xC0);

	/* CHGFREQ=564.5KHz, ADPTPWR=instantaneous value from nsin */
	cxd3774gf_register_modify(CXD3774GF_CHARGE_PUMP,0x46,0xF7);

	/* NS_OUT=RZ1 */
	cxd3774gf_register_modify(CXD3774GF_DAC,0x02,0x03);

	/* ANLGZC=off, analog zero cross off */
	cxd3774gf_register_modify(CXD3774GF_SOFT_RAMP_1,0x00,0x20);

	/* mic bias */
	set_mic_bias_mode(CXD3774GF_MIC_BIAS_MODE_NORMAL);

	return(0);
}

static int shutdown_chip(void)
{
	print_trace("%s()\n",__FUNCTION__);

	/* mute_b=on, sdout2=off, sdout1=off, sdin2=off, sdin2=off */
	cxd3774gf_register_modify(CXD3774GF_SDO_CONTROL,0x10,0x1F);

	/* all off */
	cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_3,0xFF,0xFF);

	/* all off */
	cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0xFF,0xFF);

	/* all off */
	cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_2,0xFF,0xFF);

	cxd3774gf_reset();

	/* external OSC off (only 1240/1265) */
	cxd3774gf_switch_external_osc(EXTERNAL_OSC_OFF);

	return(0);
}

static int switch_cpclk(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* CPCLK=on */
		cxd3774gf_register_modify(CXD3774GF_CLASS_H_2,0x04,0x04);
		msleep(30);
	}
	else{
		/* CPCLK=off */
		cxd3774gf_register_modify(CXD3774GF_CLASS_H_2,0x00,0x04);
	}

	return(0);
}

static int switch_dac(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* DAC L/R=on */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_3,0x00,0xC0);
		msleep(10);
	}
	else{
		/* DAC L/R=off */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_3,0xC0,0xC0);
	}

	return(0);
}

static int switch_smaster(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){

		/* enable digital amp */
		digiamp_enable(DAMP_CTL_LEVEL_POWER);

		/* SMASTER=on */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x00,0x10);

		/* PCMCLKO=on */
		cxd3774gf_register_modify(CXD3774GF_DAC,0x00,0x80);

		/* mute=off, NSX2I=1, NSADJON=1 */
		cxd3774gf_register_modify(CXD3774GF_S_MASTER_CONTROL,0x60,0xE0);

		msleep(10);
	}
	else{
		/* mute=on, NSX2I=0, NSADJON=0 */
		cxd3774gf_register_modify(CXD3774GF_S_MASTER_CONTROL,0x80,0xE0);

		/* PCMCLKO=off */
		cxd3774gf_register_modify(CXD3774GF_DAC,0x80,0x80);

		/* SMASTER=off */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x10,0x10);

		/* disable digital amp */
		digiamp_disable(DAMP_CTL_LEVEL_POWER);
	}

	return(0);
}

static int switch_classh(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* HPOUT L/R=on */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_3,0x00,0x30);
		msleep(10);
	}
	else{
		/* HPOUT L/R=off */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_3,0x30,0x30);
	}

	return(0);
}

static int switch_lineout(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* lineout 1 L/R=on */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_3,0x00,0x0C);
		msleep(10);
	}
	else{
		/* lineout 1 L/R=off */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_3,0x0C,0x0C);
	}

	return(0);
}

static int switch_speaker(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* lineout 2 L=on */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_3,0x00,0x03);
		msleep(10);
	}
	else{
		/* lineout 2 L=off */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_3,0x03,0x03);
	}

	return(0);
}

static int switch_speaker_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		msleep(5);

		/* speaker on */
		cxd3774gf_switch_speaker_power(ON);

		msleep(50);
	}
	else{
		/* speaker off */
		cxd3774gf_switch_speaker_power(OFF);

		msleep(5);
	}

	return(0);
}

static int switch_linein(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* port=LINE1 */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_2,0x00,0x30);

		/* cic1in=LINE */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_3,0xC0,0xC0);

		/* BL DSP=on */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x00,0x02);

		/* linein L/R=on */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_2,0x00,0x03);
	}
	else{
		/* linein L/R=off */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_2,0x03,0x03);

		/* BL DSP=off */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x02,0x02);

		/* cic1in=zero */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_3,0x00,0xC0);

		/* port=LINE1 */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_2,0x00,0x30);
	}

	return(0);
}

static int switch_tuner(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* port=LINE2 */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_2,0x20,0x30);

		/* cic1in=LINE */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_3,0xC0,0xC0);

		/* BL DSP=on */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x00,0x02);

		/* linein L/R=on */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_2,0x00,0x03);
	}
	else{
		/* linein L/R=off */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_2,0x03,0x03);

		/* BL DSP=off */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x02,0x02);

		/* cic1in=zero */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_3,0x00,0xC0);

		/* port=LINE1 */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_2,0x00,0x30);
	}

	return(0);
}

static int switch_dmic(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* cic1in=DMIC */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_3,0x80,0xC0);

		/* BL DSP=on, DMIC=on */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x00,0x03);

		/* ADC1_BOOST=on */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_2,0x04,0x04);
	}
	else{
		/* BL DSP=off, DMIC=off */
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x03,0x03);

		/* cic1in=zero */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_3,0x00,0xC0);

		/* ADC1_BOOST=off */
		cxd3774gf_register_modify(CXD3774GF_LINEIN_2,0x00,0x04);
	}

	return(0);
}

static int get_jack_status(int noise_cancel_active, int jack_mode)
{
#ifdef ICX_ENABLE_NC

	int status;
	int hp;
	int nc;

	/* print_trace("%s(%d,%d)\n",__FUNCTION__,noise_cancel_active,jack_mode); */

	if(noise_cancel_active == FALSE){
		set_mic_bias_mode(CXD3774GF_MIC_BIAS_MODE_MIC_DET);
		/* wait comparetor statble */
		msleep(80);
	}

	hp=cxd3774gf_get_hp_det_value();
	nc=cxd3774gf_get_mic_det_value();

	/* print_debug("hp=%d, nc=%d\n",hp,nc); */

	if(!hp){
		if(nc){
			status=JACK_STATUS_5PIN;
		}
		else{
			status=JACK_STATUS_3PIN;
		}
	}
	else{
		status=JACK_STATUS_NONE;
	}

	if(noise_cancel_active == FALSE){
		set_mic_bias_mode(CXD3774GF_MIC_BIAS_MODE_NORMAL);
	}

#else

	int status;
	int hp;

	/* print_trace("%s(%d,%d)\n",__FUNCTION__,noise_cancel_active,jack_mode); */

	hp=cxd3774gf_get_hp_det_value();

	if(!hp){
		status=JACK_STATUS_3PIN;
	}
	else{
		status=JACK_STATUS_NONE;
	}

#endif

	if(jack_mode==JACK_MODE_ANTENNA){
		if(status!=JACK_STATUS_NONE)
			status=JACK_STATUS_ANTENNA;
	}

	return(status);
}

static int set_mic_bias_mode(int mode)
{
	/* print_trace("%s(%d)\n",__FUNCTION__,mode); */

	if(mode==CXD3774GF_MIC_BIAS_MODE_NC_ON){
		/* B=2V A=2V pull-none */
		cxd3774gf_register_modify(CXD3774GF_MIC_BIAS,0x2A,0x3F);
	}
	else if(mode==CXD3774GF_MIC_BIAS_MODE_MIC_DET){
		/* B=2V A=hiz pull-none */
		cxd3774gf_register_modify(CXD3774GF_MIC_BIAS,0x22,0x3F);
	}
	else if(mode==CXD3774GF_MIC_BIAS_MODE_NORMAL){
		/* B=2V A=hiz pull-up */
		cxd3774gf_register_modify(CXD3774GF_MIC_BIAS,0x20,0x3F);
	}
	else {
		/* B=hiz A=hiz pull-up */
		cxd3774gf_register_modify(CXD3774GF_MIC_BIAS,0x00,0x3F);
	}

	return(0);
}

static int switch_dnc_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value)
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x00,0x08);
	else
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_1,0x08,0x08);

	return(0);
}

static int switch_ncmic_amp_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value)
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_2,0x00,0x0C);
	else
		cxd3774gf_register_modify(CXD3774GF_POWER_CONTROL_2,0x0C,0x0C);

	return(0);
}

static int show_device_id(void)
{
	unsigned int val;
	int rv;

	rv=cxd3774gf_register_read(CXD3774GF_DEVICE_ID,&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	print_info("device ID = 0x%02X\n",val);

	rv=cxd3774gf_register_read(CXD3774GF_REVISION_NO,&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	print_info("revision = 0x%02X\n",val);

	return(0);
}

