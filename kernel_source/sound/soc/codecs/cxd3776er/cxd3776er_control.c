/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3776er_control.c
 *
 * CXD3776ER CODEC driver
 *
 * Copyright (c) 2015 Sony Corporation
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

#include "cxd3776er_common.h"

#define CODEC_RAM_NORMAL 0x08
#define CODEC_RAM_WRITE  0x48
#define CODEC_RAM_READ   0x88

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

static int change_output_path(
	int output_path,
	int output_select,
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
	int output_path,
	int output_select,
	int motion_feedback_active,
	int analog_playback_mute
);

static int adjust_tone_control(
	int output_path,
	int output_select,
	int sdin_mix,
	int motion_feedback_mode
);

static int adjust_device_gain(
	int input_device
);

static int report_tpm_status(
	int old_status,
	int new_status,
	int force
);

#ifdef ICX_ENABLE_AU2CLK

static int change_external_osc(
	struct cxd3776er_status * status
);

static int suitable_external_osc(
	int output_path,
	int output_select,
	int icx_playback_active,
	int std_playback_active,
	int capture_active,
	unsigned int sample_rate
);

#endif

static int check_active(
	struct cxd3776er_status * status
);

static int startup_chip(unsigned int rate);
static int shutdown_chip(void);
static int switch_cpclk(int value);
static int switch_dac1(int value);
static int switch_dac2(int value);
static int switch_smaster_power(int value);
static int switch_smaster1(int value);
static int switch_smaster2(int value);
static int switch_classh(int value);
static int switch_i2sout1(int value);
static int switch_i2sout2(int value);
static int switch_speaker(int value);
static int switch_speaker_power(int value);
static int switch_linein(int value);
static int switch_tuner(int value);
static int switch_dmic(int value);
static int get_tpm_status(int motion_feedback_active, int tpm_mode);
static int set_mic_bias_mode(int mode);
static int switch_dmfb_power(int value);
static int switch_tpm_power(int value);
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
static struct cxd3776er_status present = {

	.motion_feedback_mode           = MOTION_FEEDBACK_MODE_OFF,
	.motion_feedback_active         = FALSE,

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

	.output_path               = OUTPUT_PATH_NONE,
	.input_device                = INPUT_DEVICE_NONE,
	.output_select               = OUTPUT_SELECT_NONE,
	.sdin_mix              = SDIN_SEPARATE_1,
	.tpm_mode                   = TPM_MODE_OFF,
	.tpm_status                 = FALSE,

	.pcm_monitoring              = OFF,

	.sample_rate                 = 44100,
};

static struct cxd3776er_status back_up;

/**************************/
/*@ initialize / finalize */
/**************************/

int cxd3776er_core_initialize(struct mutex * mutex)
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

	cxd3776er_switch_smaster_mute(ON);
	cxd3776er_switch_class_h_mute(ON);
	msleep(50);
	digiamp_switch_shunt_mute(ON);

	/* dummy 1.8V/2.85V default on */
	cxd3776er_switch_logic_ldo(ON);
	cxd3776er_switch_330_power(ON);
	cxd3776er_switch_285_power(ON);
	digiamp_power_on();
	msleep(50);

	startup_chip(present.sample_rate);

	show_device_id();

	digiamp_initialize();

	/* dummy, dmfb driver is not loaded. */
	cxd3776er_dmfb_initialize();

	cxd3776er_tpm_initialize();
	/********************/

	cxd3776er_set_mix_timed_mute(&present,OFF);
	present.mix_timed_mute = OFF;
	cxd3776er_set_std_timed_mute(&present,OFF);
	present.std_timed_mute = OFF;
	cxd3776er_set_icx_timed_mute(&present,OFF);
	present.icx_timed_mute = OFF;

	cxd3776er_set_uncg_mute(&present,OFF);
	present.uncg_mute = OFF;

	cxd3776er_set_analog_stream_mute(&present,OFF);
	present.analog_stream_mute = OFF;

	cxd3776er_set_analog_playback_mute(&present,present.analog_playback_mute);

	cxd3776er_set_playback_mute(&present,OFF);
	present.playback_mute = OFF;

	cxd3776er_set_capture_mute(&present,OFF);
	present.capture_mute = OFF;

	change_output_path(
		present.output_path,
		OUTPUT_SELECT_1,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.output_select = OUTPUT_SELECT_1;

	change_input_device(
		present.input_device
	);

	/* dummy, dmfb driver is not loaded. */
	present.motion_feedback_active=cxd3776er_dmfb_judge(
		present.motion_feedback_mode,
		present.output_path,
		present.output_select,
		present.tpm_status,
		present.sdin_mix
	);

	present.tpm_status=cxd3776er_tpm_judge(
		present.tpm_mode,
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.sdin_mix
	);

	cxd3776er_set_master_volume(&present,present.master_volume);

	cxd3776er_set_master_gain(&present,30);
	present.master_gain=30;

	adjust_tone_control(
		present.output_path,
		present.output_select,
		present.sdin_mix,
		present.motion_feedback_mode
	);

	adjust_device_gain(present.input_device);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.analog_playback_mute
	);

	/********************/

	cxd3776er_set_output_path_mute(&present,OFF,TRUE);
	cxd3776er_set_input_device_mute(&present,OFF);

	core_active=TRUE;

	initialized=TRUE;

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_core_finalize(void)
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

	cxd3776er_set_output_path_mute(&present,ON,TRUE);
	cxd3776er_set_input_device_mute(&present,ON);

	/********************/

	cxd3776er_set_mix_timed_mute(&present,OFF);
	present.mix_timed_mute = OFF;
	cxd3776er_set_std_timed_mute(&present,OFF);
	present.std_timed_mute = OFF;
	cxd3776er_set_icx_timed_mute(&present,OFF);
	present.icx_timed_mute = OFF;

	cxd3776er_set_uncg_mute(&present,OFF);
	present.uncg_mute = OFF;

	cxd3776er_set_analog_stream_mute(&present,ON);
	present.analog_stream_mute=ON;

	cxd3776er_set_analog_playback_mute(&present,ON);
	present.analog_playback_mute=ON;

	cxd3776er_set_playback_mute(&present,ON);
	present.playback_mute=ON;

	cxd3776er_set_capture_mute(&present,ON);
	present.capture_mute=ON;

	change_output_path(
		OUTPUT_PATH_NONE,
	 	OUTPUT_SELECT_NONE,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.output_path=OUTPUT_PATH_NONE;
	present.output_select=OUTPUT_SELECT_NONE;

	change_input_device(
		INPUT_DEVICE_NONE
	);
	present.input_device=INPUT_DEVICE_NONE;

	present.motion_feedback_active=cxd3776er_dmfb_judge(
		MOTION_FEEDBACK_MODE_OFF,
		present.output_path,
		present.output_select,
		present.tpm_status,
		present.sdin_mix
	);

	present.tpm_status=cxd3776er_tpm_judge(
		TPM_MODE_OFF,
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.sdin_mix
	);
	present.motion_feedback_mode=MOTION_FEEDBACK_MODE_OFF;
	present.tpm_mode=TPM_MODE_OFF;

	cxd3776er_set_master_volume(&present,0);
	present.master_gain=0;

	cxd3776er_set_master_gain(&present,0);
	present.master_volume=0;

	adjust_tone_control(
		present.output_path,
		present.output_select,
		present.sdin_mix,
		present.motion_feedback_mode
	);

	adjust_device_gain(present.input_device);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.analog_playback_mute
	);

	/********************/

	cxd3776er_dmfb_shutdown();

	digiamp_shutdown();

	shutdown_chip();

	digiamp_power_off();
	cxd3776er_switch_285_power(OFF);
	cxd3776er_switch_330_power(OFF);
	msleep(20);
	cxd3776er_switch_logic_ldo(OFF);

	cxd3776er_switch_smaster_mute(ON);
	cxd3776er_switch_class_h_mute(ON);
	msleep(50);
	digiamp_switch_shunt_mute(OFF);

	mutex_unlock(global_mutex);

	return(0);
}

/*******************/
/*@ resume/suspend */
/*******************/

int cxd3776er_suspend(void)
{
	print_trace("%s()\n",__FUNCTION__);

	flush_delayed_work(&pcm_event_work);
	flush_timed_mute();

	mutex_lock(global_mutex);

	if(initialized)
		suspend_core();

	return(0);
}

int cxd3776er_resume(void)
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

int cxd3776er_startup(int icx_playback, int std_playback, int capture)
{
	print_trace("%s(%d,%d,%d)\n",__FUNCTION__,icx_playback,std_playback,capture);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	resume_core();
#if 0
	if(present.output_path==OUTPUT_SELECT_NONE_DAC){
		/* if mute is off, speaker is alrady used. */
		if(present.analog_playback_mute==ON){
			if( (icx_playback || std_playback)
			&& present.icx_playback_active==0
			&& present.std_playback_active==0)
				switch_speaker_power(ON);
		}
	}
#endif
	if(icx_playback)
		present.icx_playback_active++;

	if(std_playback)
		present.std_playback_active++;

	if(capture)
		present.capture_active++;

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_shutdown(int icx_playback, int std_playback, int capture)
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

#if 0
	if(present.output_path==OUTPUT_SELECT_NONE_SPEAKER){
		/* if mute is off, speaker is still used. */
		if(present.analog_playback_mute==ON){
			if( (icx_playback || std_playback)
			&& present.icx_playback_active==0
			&& present.std_playback_active==0)
				switch_speaker_power(OFF);
		}
	}
#endif
	if(!check_active(&present))
		suspend_core();

	mutex_unlock(global_mutex);

	return(0);
}

/******************/
/*@ sampling_rate */
/******************/

int cxd3776er_set_icx_playback_dai_rate(unsigned int rate)
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

int cxd3776er_set_std_playback_dai_rate(unsigned int rate)
{
	print_trace("%s(%u)\n",__FUNCTION__,rate);

	mutex_lock(global_mutex);

#ifdef ICX_ENABLE_AU2CLK
	change_external_osc(&present);
#endif

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_set_capture_dai_rate(unsigned int rate)
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
/*@ motion_feedback_mode              */
/*    MOTION_FEEDBACK_MODE_OFF      0 */
/*    MOTION_FEEDBACK_MODE_OFFICE   1 */
/*    MOTION_FEEDBACK_MODE_TRAIN    2 */
/*    MOTION_FEEDBACK_MODE_AIRPLANE 3 */
/***********************************/

int cxd3776er_put_motion_feedback_mode(int value)
{
	int switching;

	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>MOTION_FEEDBACK_MODE_MAX){
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
		back_up.motion_feedback_mode=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.motion_feedback_mode){
		mutex_unlock(global_mutex);
		return(0);
	}

	if((present.motion_feedback_mode==MOTION_FEEDBACK_MODE_OFF && value!=MOTION_FEEDBACK_MODE_OFF)
	|| (present.motion_feedback_mode!=MOTION_FEEDBACK_MODE_OFF && value==MOTION_FEEDBACK_MODE_OFF))
		switching=TRUE;
	else
		switching=FALSE;

	if(switching){
		cxd3776er_set_output_path_mute(&present,ON,TRUE);
	}

	/********************/

	present.motion_feedback_active=cxd3776er_dmfb_judge(
		value,
		present.output_path,
		present.output_select,
		present.tpm_status,
		present.sdin_mix
	);
	present.motion_feedback_mode=value;

	/********************/

	if(switching){
		present.pcm_monitoring=judge_pcm_monitoring(
			present.output_path,
			present.output_select,
			present.motion_feedback_active,
			present.analog_playback_mute
		);

		cxd3776er_set_master_volume(&present,present.master_volume);

		cxd3776er_set_output_path_mute(&present,OFF,TRUE);
	}

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_motion_feedback_mode(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.motion_feedback_mode;
	else
		*value=back_up.motion_feedback_mode;

	mutex_unlock(global_mutex);

	return(0);
}

/************************/
/*@ motion_feedback_status */
/************************/

int cxd3776er_get_motion_feedback_status(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	*value=present.motion_feedback_active;

	mutex_unlock(global_mutex);

	return(0);
}

/***************************/
/*@ user_motion_feedback_gain */
/*    0 - 30               */
/***************************/

int cxd3776er_put_user_motion_feedback_gain(int index)
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

	rv=cxd3776er_dmfb_set_user_gain(index);
	if(rv<0){
		back_trace();
		mutex_unlock(global_mutex);
		return(rv);
	}

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_user_motion_feedback_gain(int * index)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	*index=0;

	cxd3776er_dmfb_get_user_gain(index);

	mutex_unlock(global_mutex);

	return(0);
}

/***************************/
/*@ base_motion_feedback_gain */
/*    0 - 50               */
/***************************/

int cxd3776er_put_base_motion_feedback_gain(int left, int right)
{
	int rv;

	print_trace("%s(%d,%d)\n",__FUNCTION__,left,right);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	rv=cxd3776er_dmfb_set_base_gain(left,right);
	if(rv<0){
		back_trace();
		mutex_unlock(global_mutex);
		return(rv);
	}

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_base_motion_feedback_gain(int * left, int * right)
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

	cxd3776er_dmfb_get_base_gain(left,right);

	mutex_unlock(global_mutex);

	return(0);
}

/*******************************************/
/*@ exit_base_motion_feedback_gain_adjustment */
/*******************************************/

int cxd3776er_exit_base_motion_feedback_gain_adjustment(int save)
{
	int rv;

	print_trace("%s(%d)\n",__FUNCTION__,save);

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	rv=cxd3776er_dmfb_exit_base_gain_adjustment(save);
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

int cxd3776er_put_sound_effect(int value)
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

	cxd3776er_set_output_path_mute(&present,ON,TRUE);

	/********************/

	present.sound_effect=value;

	/********************/

	cxd3776er_set_master_volume(&present,present.master_volume);

	cxd3776er_set_output_path_mute(&present,OFF,TRUE);

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_sound_effect(int * value)
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

int cxd3776er_put_master_volume(int value)
{
	int delta;
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

	if(value>now)
		delta=1;
	else
		delta=-1;

	while(value!=now){

		now=now+delta;

		cxd3776er_set_master_volume(&present,now);
		present.master_volume=now;
	}

	/* cxd3776er_set_master_volume(&present,value); */
	/* present.master_volume=value; */

	/********************/

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_master_volume(int * value)
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

int cxd3776er_put_master_gain(int value)
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

	cxd3776er_set_master_gain(&present,value);
	present.master_gain=value;

	/********************/

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_master_gain(int * value)
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

int cxd3776er_put_playback_mute(int value)
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

	cxd3776er_set_playback_mute(&present,value);
	present.playback_mute=value;

	/********************/

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_playback_mute(int * value)
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

int cxd3776er_put_capture_mute(int value)
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

	cxd3776er_set_capture_mute(&present,value);
	present.capture_mute=value;

	/********************/

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_capture_mute(int * value)
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

int cxd3776er_put_analog_playback_mute(int value)
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
#if 0
	/********************/

	if(present.output_path==OUTPUT_SELECT_NONE_SPEAKER
	&& present.icx_playback_active==0
	&& present.std_playback_active==0){
		if(value==OFF)
			switch_speaker_power(ON);
	}

	cxd3776er_set_analog_playback_mute(&present,value);
	present.analog_playback_mute=value;

	if(present.output_path==OUTPUT_SELECT_NONE_SPEAKER
	&& present.icx_playback_active==0
	&& present.std_playback_active==0){
		if(value==ON)
			switch_speaker_power(OFF);
	}

	/********************/
#endif
	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.analog_playback_mute
	);

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_analog_playback_mute(int * value)
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

int cxd3776er_put_analog_stream_mute(int value)
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

	cxd3776er_set_analog_stream_mute(&present,value);
	present.analog_stream_mute=value;

	/********************/

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_analog_stream_mute(int * value)
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

int cxd3776er_put_timed_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	return(set_timed_mute(0,value));
}

int cxd3776er_get_timed_mute(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	return(get_timed_mute(0,value));
}

int cxd3776er_put_std_timed_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	return(set_timed_mute(1,value));
}

int cxd3776er_get_std_timed_mute(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	return(get_timed_mute(1,value));
}

int cxd3776er_put_icx_timed_mute(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);


	return(set_timed_mute(2,value));
}

int cxd3776er_get_icx_timed_mute(int * value)
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
					cxd3776er_set_mix_timed_mute(&present,mute);
					present.mix_timed_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;

			case 1:
				if(present.std_timed_mute!=mute){
					cxd3776er_set_std_timed_mute(&present,mute);
					present.std_timed_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;

			case 2:
				if(present.icx_timed_mute!=mute){
					cxd3776er_set_icx_timed_mute(&present,mute);
					present.icx_timed_mute=mute;
					changed=TRUE;
					print_debug("timed mute %d = %s\n",port,mute?"ON":"OFF");
				}
				break;

			case 3:
				if(present.uncg_mute!=mute){
					cxd3776er_set_uncg_mute(&present,mute);
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
					cxd3776er_set_mix_timed_mute(&present,OFF);
					present.mix_timed_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;

			case 1:
				if(present.std_timed_mute!=OFF){
					cxd3776er_set_std_timed_mute(&present,OFF);
					present.std_timed_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;

			case 2:
				if(present.icx_timed_mute!=OFF){
					cxd3776er_set_icx_timed_mute(&present,OFF);
					present.icx_timed_mute=OFF;
					print_debug("timed mute %d = OFF\n",port);
				}
				break;

			case 3:
				if(present.uncg_mute!=OFF){
					cxd3776er_set_uncg_mute(&present,OFF);
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

int cxd3776er_put_input_device(int value)
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

	cxd3776er_set_input_device_mute(&present,ON);

	/********************/

	change_input_device(value);
	present.input_device=value;

	/********************/

	cxd3776er_set_master_gain(&present,present.master_gain);

	adjust_device_gain(present.input_device);

	cxd3776er_set_input_device_mute(&present,OFF);

	if(!check_active(&present))
		suspend_core();

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_input_device(int * value)
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
/*@ output_path               */
/*    OUTPUT_PATH_NONE      0 */
/*    OUTPUT_PATH_PWM       1 */
/*    OUTPUT_PATH_I2S       2 */
/*    OUTPUT_PATH_DAC       3 */
/*    OUTPUT_PATH_PWM_I2S   4 */
/*    OUTPUT_PATH_PWM_DAC   5 */
/*    OUTPUT_PATH_I2S_DAC   6 */
/*    OUTPUT_PATH_ALL       7 */
/********************************/

int cxd3776er_put_output_path(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>OUTPUT_PATH_MAX){
		print_error("invalid parameter, value = %d\n",value);
		return(-EINVAL);
	}

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(value!=OUTPUT_PATH_NONE)
		resume_core();

	if(value==present.output_path){
		mutex_unlock(global_mutex);
		return(0);
	}

	cxd3776er_set_output_path_mute(&present,ON,TRUE);

	/********************/

	change_output_path(
		value,
		present.output_select,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.output_path=value;

	/********************/

	present.motion_feedback_active=cxd3776er_dmfb_judge(
		present.motion_feedback_mode,
		present.output_path,
		present.output_select,
		present.tpm_status,
		present.sdin_mix
	);

	cxd3776er_set_master_volume(&present,present.master_volume);

	adjust_tone_control(
		present.output_path,
		present.output_select,
		present.sdin_mix,
		present.motion_feedback_mode
	);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.analog_playback_mute
	);

	cxd3776er_set_output_path_mute(&present,OFF,TRUE);

	if(!check_active(&present))
		suspend_core();

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_output_path(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	*value=present.output_path;

	mutex_unlock(global_mutex);

	return(0);
}

/*******************************/
/*@ output_select              */
/*    OUTPUT_SELECT_NONE     0 */
/*    OUTPUT_SELECT_1        1 */
/*    OUTPUT_SELECT_2        2 */
/*    OUTPUT_SELECT_BOTH     3 */
/*******************************/

int cxd3776er_put_output_select(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>OUTPUT_SELECT_MAX){
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
		back_up.output_select=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.output_select){
		mutex_unlock(global_mutex);
		return(0);
	}

	cxd3776er_set_output_path_mute(&present,ON,TRUE);

	/********************/

	change_output_path(
		present.output_path,
		value,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.output_select=value;

	/********************/

	present.motion_feedback_active=cxd3776er_dmfb_judge(
		present.motion_feedback_mode,
		present.output_path,
		present.output_select,
		present.tpm_status,
		present.sdin_mix
	);

	cxd3776er_set_master_volume(&present,present.master_volume);

	adjust_tone_control(
		present.output_path,
		present.output_select,
		present.sdin_mix,
		present.motion_feedback_mode
	);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.analog_playback_mute
	);

	cxd3776er_set_output_path_mute(&present,OFF,TRUE);

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_output_select(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.output_select;
	else
		*value=back_up.output_select;

	mutex_unlock(global_mutex);

	return(0);
}

/****************************/
/*@ sdin_mix            */
/*    SDIN_SEPARATE_1 0 */
/*    SDIN_SEPARATE_2 1 */
/*    SDIN_SEPARATE_3 2 */
/*    SDIN_MIX        3 */
/*    SDIN_INVERT     4 */
/****************************/

int cxd3776er_put_sdin_mix(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>SDIN_MIX_MAX){
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
		back_up.sdin_mix=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.sdin_mix){
		mutex_unlock(global_mutex);
		return(0);
	}

	cxd3776er_set_output_path_mute(&present,ON,TRUE);

	/********************/

	present.sdin_mix=value;

	/********************/

	present.motion_feedback_active=cxd3776er_dmfb_judge(
		present.motion_feedback_mode,
		present.output_path,
		present.output_select,
		present.tpm_status,
		present.sdin_mix
	);

	adjust_tone_control(
		present.output_path,
		present.output_select,
		present.sdin_mix,
		present.motion_feedback_mode
	);

	cxd3776er_set_master_volume(&present,present.master_volume);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.analog_playback_mute
	);

	cxd3776er_set_output_path_mute(&present,OFF,TRUE);

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_sdin_mix(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.sdin_mix;
	else
		*value=back_up.sdin_mix;

	mutex_unlock(global_mutex);

	return(0);
}

/****************************/
/*@ tpm_mode               */
/*    JACK_MODE_HEADPHONE 0 */
/*    JACK_MDOE_ANTENNA   1 */
/****************************/

int cxd3776er_put_tpm_mode(int value)
{
	int status;

	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value<0 || value>TPM_MODE_MAX){
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
		back_up.tpm_mode=value;
		mutex_unlock(global_mutex);
		return(0);
	}

	if(value==present.tpm_mode){
		mutex_unlock(global_mutex);
		return(0);
	}

	present.tpm_mode=value;

	present.tpm_status=cxd3776er_tpm_judge(
		present.tpm_mode,
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.sdin_mix
	);

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_tpm_mode(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	if(core_active)
		*value=present.tpm_mode;
	else
		*value=back_up.tpm_mode;

	mutex_unlock(global_mutex);

	return(0);
}

/****************/
/*@ tpm_status */
/****************/

int cxd3776er_get_tpm_status(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	if(!initialized){
		print_error("not initialized.\n");
		mutex_unlock(global_mutex);
		return(-EBUSY);
	}

	*value=present.tpm_status;

	mutex_unlock(global_mutex);

	return(0);
}

/************************/
/*@ register_dmfb_module */
/************************/

int cxd3776er_register_dmfb_module(struct cxd3776er_dmfb_interface * interface)
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
		interface->switch_dmfb_power       = switch_dmfb_power;
		interface->switch_ncmic_amp_power = switch_ncmic_amp_power;
		interface->modify_reg             = cxd3776er_register_modify;
		interface->write_reg              = cxd3776er_register_write;
		interface->read_reg               = cxd3776er_register_read;
		interface->write_buf              = cxd3776er_register_write_multiple;
		interface->read_buf               = cxd3776er_register_read_multiple;
		interface->global_mutex           = global_mutex;

		cxd3776er_dmfb_register_module(interface);

		rv=cxd3776er_dmfb_initialize();
		if(rv<0){
			back_trace();
			mutex_unlock(global_mutex);
			return(rv);
		}

		/********/

		present.motion_feedback_active=cxd3776er_dmfb_judge(
			present.motion_feedback_mode,
			present.output_path,
			present.output_select,
			present.tpm_status,
			present.sdin_mix
		);

		cxd3776er_set_master_volume(&present,present.master_volume);

		cxd3776er_set_master_gain(&present,present.master_gain);

		adjust_tone_control(
			present.output_path,
			present.output_select,
			present.sdin_mix,
			present.motion_feedback_mode
		);

		present.pcm_monitoring=judge_pcm_monitoring(
			present.output_path,
			present.output_select,
			present.motion_feedback_active,
			present.analog_playback_mute
		);

		/********/
	}

	else{
		cxd3776er_dmfb_shutdown();

		cxd3776er_dmfb_register_module(interface);

		/********/

		present.motion_feedback_active=cxd3776er_dmfb_judge(
			present.motion_feedback_mode,
			present.output_path,
			present.output_select,
			present.tpm_status,
			present.sdin_mix
		);

		cxd3776er_set_master_volume(&present,present.master_volume);

		cxd3776er_set_master_gain(&present,present.master_gain);

		adjust_tone_control(
			present.output_path,
			present.output_select,
			present.sdin_mix,
			present.motion_feedback_mode
		);

		present.pcm_monitoring=judge_pcm_monitoring(
			present.output_path,
			present.output_select,
			present.motion_feedback_active,
			present.analog_playback_mute
		);

		/********/
	}

	mutex_unlock(global_mutex);

	return(0);
}
EXPORT_SYMBOL(cxd3776er_register_dmfb_module);


int cxd3776er_register_tpm_module(struct cxd3776er_tpm_interface * interface)
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
		interface->switch_tpm_power       = switch_tpm_power;
		interface->switch_ncmic_amp_power = switch_ncmic_amp_power;
		interface->modify_reg             = cxd3776er_register_modify;
		interface->write_reg              = cxd3776er_register_write;
		interface->read_reg               = cxd3776er_register_read;
		interface->write_buf              = cxd3776er_register_write_multiple;
		interface->read_buf               = cxd3776er_register_read_multiple;
		interface->global_mutex           = global_mutex;

		cxd3776er_tpm_register_module(interface);

		rv=cxd3776er_tpm_initialize();
		if(rv<0){
			back_trace();
			mutex_unlock(global_mutex);
			return(rv);
		}

		present.tpm_status=cxd3776er_tpm_judge(
			present.tpm_mode,
			present.output_path,
			present.output_select,
			present.motion_feedback_active,
			present.sdin_mix
		);

	} else {
		cxd3776er_tpm_shutdown();
		cxd3776er_tpm_register_module(interface);
	}
	mutex_unlock(global_mutex);

	return(0);
}
EXPORT_SYMBOL(cxd3776er_register_tpm_module);
/**********************/
/*@ check_tpm_status */
/**********************/

int cxd3776er_check_tpm_status(int force)
{
#if 0
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

	status=get_tpm_status(present.motion_feedback_active,present.tpm_mode);

	/* for electrostatic damege. */
	while(1){
		rv=cxd3776er_register_read(CXD3776ER_DACOUT,&ui);
		if(rv!=0 || (ui&0x40)==0x40)
			break;

		print_info("detect electrostatic damages.\n");
		suspend_core();
		resume_core();
		if(repair_electrostatic_damage!=NULL)
			repair_electrostatic_damage();
		status=get_tpm_status(present.motion_feedback_active,present.tpm_mode);
	}

	report_tpm_status(
		present.tpm_status,
		status,
		force
	);

	backup=present.tpm_status;
	present.tpm_status=status;

	if(present.output_path==OUTPUT_SELECT_NONE_HEADPHONE){

		if( (present.tpm_status==JACK_STATUS_5PIN && backup==JACK_STATUS_3PIN)
		||  (present.tpm_status==JACK_STATUS_3PIN && backup==JACK_STATUS_5PIN) ){

			cxd3776er_set_output_path_mute(&present,ON,TRUE);

			adjust_tone_control(
				present.output_path,
				present.output_select,
				present.sdin_mix,
				present.tpm_status
			);

			present.motion_feedback_active=cxd3776er_dmfb_judge(
				present.motion_feedback_mode,
				present.output_path,
				present.output_select,
				present.tpm_status,
				present.sdin_mix
			);

			cxd3776er_set_master_volume(&present,present.master_volume);

			present.pcm_monitoring=judge_pcm_monitoring(
				present.output_path,
				present.output_select,
				present.motion_feedback_active,
				present.analog_playback_mute
			);

			cxd3776er_set_output_path_mute(&present,OFF,TRUE);
		}
	}

	mutex_unlock(global_mutex);
#endif
	return(0);
}

/*********************/
/*@ handle_pcm_event */
/*********************/

int cxd3776er_handle_pcm_event(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	if(present.pcm_monitoring==OFF)
		return(0);

	rv=cxd3776er_get_xpcm_det_value();
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
#if 0
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

	if(present.output_path==OUTPUT_SELECT_NONE_HEADPHONE && present.output_select==HEADPHONE_AMP_SMASTER){
		rv=cxd3776er_get_xpcm_det_value();
		if(rv)
			cxd3776er_set_no_pcm_mute(OFF);
		else
			cxd3776er_set_no_pcm_mute(ON);
	}
	else{
		cxd3776er_set_no_pcm_mute(ON);
	}

	mutex_unlock(global_mutex);
#endif
	return;
}

/***********************/
/*@ apply_table_change */
/***********************/

int cxd3776er_apply_table_change(int id)
{
	print_trace("%s(%d)\n",__FUNCTION__,id);

	if(!core_active)
		return(0);

	if(id==TABLE_ID_MASTER_VOLUME){
		cxd3776er_set_master_volume(&present,present.master_volume);
	}
	else if(id==TABLE_ID_DEVICE_GAIN){
		adjust_device_gain(present.input_device);
	}
	else if(id==TABLE_ID_TONE_CONTROL){
		adjust_tone_control(
			present.output_path,
			present.output_select,
			present.sdin_mix,
			present.motion_feedback_mode
		);
	}

	return(0);
}

/*************************/
/*@ electrostatic_damage */
/*************************/

void cxd3776er_register_electrostatic_damage_repairer(int (*function)(void))
{
	print_trace("%s(0x%08X)\n",__FUNCTION__,(unsigned int)function);

	if(global_mutex==NULL)
		return;

	mutex_lock(global_mutex);

	repair_electrostatic_damage=function;

	mutex_unlock(global_mutex);

	return;
}
EXPORT_SYMBOL(cxd3776er_register_electrostatic_damage_repairer);

/***************/
/*@ debug_test */
/***************/

int cxd3776er_debug_test = 0;

int cxd3776er_put_debug_test(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	mutex_lock(global_mutex);

	cxd3776er_debug_test=value;

	mutex_unlock(global_mutex);

	return(0);
}

int cxd3776er_get_debug_test(int * value)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	mutex_lock(global_mutex);

	*value=cxd3776er_debug_test;

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

	cxd3776er_set_output_path_mute(&present,ON,TRUE);
	cxd3776er_set_input_device_mute(&present,ON);

	cxd3776er_set_mix_timed_mute(&present,ON);
	present.mix_timed_mute = ON;
	cxd3776er_set_std_timed_mute(&present,ON);
	present.std_timed_mute = ON;
	cxd3776er_set_icx_timed_mute(&present,ON);
	present.icx_timed_mute = ON;

	cxd3776er_set_uncg_mute(&present,ON);
	present.uncg_mute = ON;

	cxd3776er_set_analog_stream_mute(&present,ON);
	present.analog_stream_mute=ON;

	cxd3776er_set_analog_playback_mute(&present,ON);
	present.analog_playback_mute=ON;

	cxd3776er_set_playback_mute(&present,ON);
	present.playback_mute=ON;

	cxd3776er_set_capture_mute(&present,ON);
	present.capture_mute=ON;

	change_output_path(
		OUTPUT_PATH_NONE,
		OUTPUT_SELECT_NONE,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.output_path=OUTPUT_PATH_NONE;
	present.output_select=OUTPUT_SELECT_NONE;

	change_input_device(
		INPUT_DEVICE_NONE
	);
	present.input_device=INPUT_DEVICE_NONE;

	present.motion_feedback_active=cxd3776er_dmfb_judge(
		MOTION_FEEDBACK_MODE_OFF,
		present.output_path,
		present.output_select,
		present.tpm_status,
		present.sdin_mix
	);
	present.motion_feedback_mode=MOTION_FEEDBACK_MODE_OFF;

	cxd3776er_set_master_volume(&present,0);
	present.master_gain=0;

	cxd3776er_set_master_gain(&present,0);
	present.master_volume=0;

	adjust_tone_control(
		present.output_path,
		present.output_select,
		present.sdin_mix,
		present.motion_feedback_mode
	);

	adjust_device_gain(present.input_device);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.analog_playback_mute
	);

	/**********/

	cxd3776er_dmfb_cleanup();

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

	cxd3776er_dmfb_prepare();

	/**********/

	present.sound_effect = back_up.sound_effect;
	/* present.tpm_mode = back_up.tpm_mode; */

	cxd3776er_set_mix_timed_mute(&present,back_up.mix_timed_mute);
	present.mix_timed_mute = back_up.mix_timed_mute;
	cxd3776er_set_std_timed_mute(&present,back_up.std_timed_mute);
	present.std_timed_mute = back_up.std_timed_mute;
	cxd3776er_set_icx_timed_mute(&present,back_up.icx_timed_mute);
	present.icx_timed_mute = back_up.icx_timed_mute;

	cxd3776er_set_uncg_mute(&present,back_up.uncg_mute);
	present.uncg_mute = back_up.uncg_mute;

	cxd3776er_set_analog_stream_mute(&present,back_up.analog_stream_mute);
	present.analog_stream_mute=back_up.analog_stream_mute;

	cxd3776er_set_analog_playback_mute(&present,back_up.analog_playback_mute);
	present.analog_playback_mute=back_up.analog_playback_mute;

	cxd3776er_set_playback_mute(&present,back_up.playback_mute);
	present.playback_mute=back_up.playback_mute;

	cxd3776er_set_capture_mute(&present,back_up.capture_mute);
	present.capture_mute=back_up.capture_mute;

	change_output_path(
		back_up.output_path,
		back_up.output_select,
		present.analog_playback_mute,
		present.icx_playback_active,
		present.std_playback_active,
		present.capture_active,
		present.sample_rate
	);
	present.output_path=back_up.output_path;
	present.output_select=back_up.output_select;

	change_input_device(
		back_up.input_device
	);
	present.input_device=back_up.input_device;

	present.motion_feedback_active=cxd3776er_dmfb_judge(
		back_up.motion_feedback_mode,
		present.output_path,
		present.output_select,
		present.tpm_status,
		present.sdin_mix
	);
	present.motion_feedback_mode=back_up.motion_feedback_mode;

	cxd3776er_set_master_volume(&present,back_up.master_volume);
	present.master_volume=back_up.master_volume;

	cxd3776er_set_master_gain(&present,back_up.master_gain);
	present.master_gain=back_up.master_gain;

	adjust_tone_control(
		present.output_path,
		present.output_select,
		present.sdin_mix,
		present.motion_feedback_mode
	);

	adjust_device_gain(present.input_device);

	present.pcm_monitoring=judge_pcm_monitoring(
		present.output_path,
		present.output_select,
		present.motion_feedback_active,
		present.analog_playback_mute
	);

	cxd3776er_set_input_device_mute(&present,OFF);
	cxd3776er_set_output_path_mute(&present,OFF,TRUE);

	/**********/

	core_active=TRUE;

return(0);
}

static int change_output_path(
	int output_path,
	int output_select,
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
		output_path,
		output_select,
		analog_playback_mute,
		icx_playback_active,
		std_playback_active,
		capture_active,
		sample_rate
	);

	/* for MCLK change */
	cxd3776er_dmfb_off();

#ifdef ICX_ENABLE_AU2CLK
	/* suitable external osc */
	osc=suitable_external_osc(
		output_path,
		output_select,
		icx_playback_active,
		std_playback_active,
		capture_active,
		sample_rate
	);
#endif

	if(output_path==OUTPUT_PATH_PWM){
		if(output_select!=OUTPUT_SELECT_NONE){
			/* --- OFF --- */
			switch_speaker_power(OFF);
			switch_speaker(OFF);
			switch_i2sout1(OFF);
			switch_i2sout2(OFF);
			switch_classh(OFF);
			switch_dac1(OFF);
			switch_dac2(OFF);
			switch_cpclk(OFF);
			/* --- CLK --- */
#ifdef ICX_ENABLE_AU2CLK
			cxd3776er_register_modify(CXD3776ER_MODE_MAIN_CLOCK,0x00,0x01);
			cxd3776er_switch_external_osc(osc);
			cxd3776er_register_modify(CXD3776ER_MODE_MAIN_CLOCK,0x01,0x01);
#endif
			/* --- ON --- */
			switch_smaster_power(ON);
			if(output_select==OUTPUT_SELECT_1){
				switch_smaster1(ON);
				switch_smaster2(OFF);
			}
			if(output_select==OUTPUT_SELECT_2){
				switch_smaster1(OFF);
				switch_smaster2(ON);
			}
			if(output_select==OUTPUT_SELECT_BOTH){
				switch_smaster1(ON);
				switch_smaster2(ON);
			}
		}
		else{
			/* --- OFF --- */
			switch_speaker_power(OFF);
			switch_speaker(OFF);
			switch_i2sout1(OFF);
			switch_i2sout2(OFF);
			switch_smaster1(OFF);
			switch_smaster2(OFF);
			switch_smaster_power(OFF);
			switch_dac1(OFF);
			switch_dac2(OFF);

			/* --- ON --- */
//			switch_cpclk(ON);
//			switch_dac(ON);
//			switch_classh(ON);
		}
	}

	else if(output_path==OUTPUT_PATH_DAC){
#if 0
		/* --- OFF --- */
		if(icx_playback_active==0 && std_playback_active==0 && analog_playback_mute==ON)
			switch_speaker_power(OFF);
#endif
		if(output_select!=OUTPUT_SELECT_NONE){
			switch_i2sout1(OFF);
			switch_i2sout2(OFF);
			switch_classh(OFF);
			switch_smaster1(OFF);
			switch_smaster2(OFF);
			switch_smaster_power(OFF);
			/* --- CLK --- */
#ifdef ICX_ENABLE_AU2CLK
			cxd3776er_register_modify(CXD3776ER_MODE_MAIN_CLOCK,0x00,0x01);
			cxd3776er_switch_external_osc(osc);
			cxd3776er_register_modify(CXD3776ER_MODE_MAIN_CLOCK,0x01,0x01);
#endif
			/* --- ON --- */
//			switch_cpclk(ON);
			if(output_select==OUTPUT_SELECT_1){
				switch_dac1(ON);
				switch_dac2(OFF);
			}
			if(output_select==OUTPUT_SELECT_2){
				switch_dac1(OFF);
				switch_dac2(ON);
			}
			if(output_select==OUTPUT_SELECT_BOTH){
				switch_dac1(ON);
				switch_dac2(ON);
			}

//			switch_speaker(ON);
#if 0
		if(icx_playback_active>0 || std_playback_active>0 || analog_playback_mute==OFF)
			switch_speaker_power(ON);
#endif
		}
	}

	else if(output_path==OUTPUT_PATH_I2S){
		if(output_select!=OUTPUT_SELECT_NONE){
			/* --- OFF --- */
			switch_speaker_power(OFF);
			switch_speaker(OFF);
			switch_classh(OFF);
			switch_smaster1(OFF);
			switch_smaster2(OFF);
			switch_smaster_power(OFF);
			switch_dac1(OFF);
			switch_dac2(OFF);
			/* --- CLK --- */
#ifdef ICX_ENABLE_AU2CLK
			cxd3776er_register_modify(CXD3776ER_MODE_MAIN_CLOCK,0x00,0x01);
			cxd3776er_switch_external_osc(osc);
			cxd3776er_register_modify(CXD3776ER_MODE_MAIN_CLOCK,0x01,0x01);
#endif
			/* --- ON --- */
			if(output_select==OUTPUT_SELECT_1){
				switch_i2sout1(ON);
				switch_i2sout2(OFF);
			}
			if(output_select==OUTPUT_SELECT_2){
				switch_i2sout1(OFF);
				switch_i2sout2(ON);
			}
			if(output_select==OUTPUT_SELECT_BOTH){
				switch_i2sout1(ON);
				switch_i2sout2(ON);
			}
		}
	}

	else if(output_path==OUTPUT_PATH_ALL){
		if(output_select!=OUTPUT_SELECT_NONE){

#ifdef ICX_ENABLE_AU2CLK
			cxd3776er_register_modify(CXD3776ER_MODE_MAIN_CLOCK,0x00,0x01);
			cxd3776er_switch_external_osc(osc);
			cxd3776er_register_modify(CXD3776ER_MODE_MAIN_CLOCK,0x01,0x01);
#endif
			switch_smaster_power(ON);

			if(output_select==OUTPUT_SELECT_1){
				switch_i2sout1(ON);
				switch_smaster1(ON);
				switch_dac1(ON);
				switch_i2sout2(OFF);
				switch_smaster2(OFF);
				switch_dac2(OFF);
			}
			if(output_select==OUTPUT_SELECT_2){
				switch_i2sout1(OFF);
				switch_smaster1(OFF);
				switch_dac1(OFF);
				switch_i2sout2(ON);
				switch_smaster2(ON);
				switch_dac2(ON);
			}
			if(output_select==OUTPUT_SELECT_BOTH){
				switch_i2sout1(ON);
				switch_smaster1(ON);
				switch_dac1(ON);
				switch_i2sout2(ON);
				switch_smaster2(ON);
				switch_dac2(ON);
			}
		}
	}

	else{ /* NONE */
		/* --- OFF --- */
		switch_speaker_power(OFF);
		switch_speaker(OFF);
		switch_i2sout1(OFF);
		switch_i2sout2(OFF);
		switch_classh(OFF);
		switch_smaster1(OFF);
		switch_smaster2(OFF);
		switch_smaster_power(OFF);
		switch_dac1(OFF);
		switch_dac2(OFF);
		switch_cpclk(OFF);
		/* --- CLK --- */
#ifdef ICX_ENABLE_AU2CLK
		cxd3776er_register_modify(CXD3776ER_MODE_MAIN_CLOCK,0x00,0x01);
		cxd3776er_switch_external_osc(osc);
		cxd3776er_register_modify(CXD3776ER_MODE_MAIN_CLOCK,0x01,0x01);
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
		cxd3776er_register_modify(CXD3776ER_SD1,0x01,0x03);
	}
	else if(rate<=96000){
		cxd3776er_register_modify(CXD3776ER_SD1,0x02,0x03);
	}
	else{
		cxd3776er_register_modify(CXD3776ER_SD1,0x03,0x03);
	}

	return(0);
}

static int judge_pcm_monitoring(
	int output_path,
	int output_select,
	int motion_feedback_active,
	int analog_playback_mute
)
{
	int judge;
	int rv;

	print_trace(
		"%s(%d,%d,%d,%d)\n",
		__FUNCTION__,
		output_path,
		output_select,
		motion_feedback_active,
		analog_playback_mute
	);
#if 0
	judge=ON;

	if(output_path!=OUTPUT_SELECT_NONE_HEADPHONE)
		judge=OFF;

	if(output_select!=HEADPHONE_AMP_SMASTER)
		judge=OFF;

	if(analog_playback_mute==OFF)
		judge=OFF;

	if(motion_feedback_active==ON)
		judge=OFF;

	if(judge==ON){

		print_debug("pcm monitoring = on\n");

		rv=cxd3776er_get_xpcm_det_value();
		if(rv)
			cxd3776er_set_no_pcm_mute(OFF);
		else
			cxd3776er_set_no_pcm_mute(ON);
	}

	else{
		cancel_delayed_work(&pcm_event_work);

		print_debug("pcm monitoring = off\n");

		if(output_path==OUTPUT_SELECT_NONE_HEADPHONE && output_select==HEADPHONE_AMP_SMASTER)
			cxd3776er_set_no_pcm_mute(OFF);
		else
			cxd3776er_set_no_pcm_mute(ON);
	}
#endif
	return(judge);
}

static int adjust_tone_control(
	int output_path,
	int output_select,
	int sdin_mix,
	int motion_feedback_mode
)
{
	int table;
	int rv;
	int n;

	print_trace(
		"%s(%d,%d,%d,%d)\n",
		__FUNCTION__,
		output_path,
		output_select,
		sdin_mix,
		motion_feedback_mode
	);
#if 0
	switch(output_path){

		case OUTPUT_SELECT_NONE_HEADPHONE:

			switch(tpm_status){
				case JACK_STATUS_5PIN:
					if(output_select==HEADPHONE_AMP_SMASTER){
						if(sdin_mix==NCHP_TYPE_MODEL1)
							table=TONE_CONTROL_TABLE_SAMP_MODEL1_NCHP;
						else
							table=TONE_CONTROL_TABLE_SAMP_BUNDLE_NCHP;
					}
					else{
						if(sdin_mix==NCHP_TYPE_MODEL1)
							table=TONE_CONTROL_TABLE_NAMP_MODEL1_NCHP;
						else
							table=TONE_CONTROL_TABLE_NAMP_BUNDLE_NCHP;
					}
					break;

				case JACK_STATUS_ANTENNA:
				case JACK_STATUS_3PIN:
				case JACK_STATUS_NONE:
				default:
					if(output_select==HEADPHONE_AMP_SMASTER)
						table=TONE_CONTROL_TABLE_SAMP_GENERAL_HP;
					else
						table=TONE_CONTROL_TABLE_NAMP_GENERAL_HP;
					break;
			}
			break;

		case OUTPUT_SELECT_NONE_SPEAKER:
		case OUTPUT_SELECT_NONE_LINE:
		case OUTPUT_SELECT_NONE_FIXEDLINE:
		case OUTPUT_PATH_NONE:
		default:
			table=TONE_CONTROL_TABLE_NO_HP;
			break;
	}
#endif
/* DEQ need RAM */
	cxd3776er_set_playback_mute(&present,ON);

	/* set mode to write */
	rv=cxd3776er_register_modify(CXD3776ER_RAM_CONTROL_1,CODEC_RAM_WRITE,0xDF);
	if(rv<0){
		back_trace();
		return(-1);
	}

	/* write data */
	for(n=0; n<CODEC_RAM_SIZE/(CODEC_RAM_WORD_SIZE*8); n++){

		/* set start address */
		rv=cxd3776er_register_write(CXD3776ER_RAM_CONTROL_2,8*n);
		if(rv<0){
			back_trace();
			return(-1);
		}

		rv=cxd3776er_register_write_multiple(
			CXD3776ER_RAM_WRITE_BASE,
			((unsigned char *)&cxd3776er_tone_control_table[motion_feedback_mode])+CODEC_RAM_WORD_SIZE*8*n,
			CODEC_RAM_WORD_SIZE*8
		);
		if(rv<0){
			back_trace();
			return(-1);
		}
	}

	/* set mode to normal */
	rv=cxd3776er_register_modify(CXD3776ER_RAM_CONTROL_1,CODEC_RAM_NORMAL,0xDF);
	if(rv<0){
		back_trace();
		return(-1);
	}

	cxd3776er_set_playback_mute(&present,present.playback_mute);
#if 0
	/* set mode to read */
	rv=cxd3776er_register_modify(CXD3776ER_RAM_CONTROL_1,CODEC_RAM_READ,0xDF);
	if(rv<0){
		back_trace();
		return(-1);
	}

	for(n=0; n<CODEC_RAM_SIZE/(CODEC_RAM_WORD_SIZE*8); n++){

		/* set start address */
		rv=cxd3776er_register_write(CXD3776ER_RAM_CONTROL_2,8*n);
		if(rv<0){
			back_trace();
			return(-1);
		}

		rv=cxd3776er_register_read_multiple(
			CXD3776ER_RAM_READ_BASE,
			((unsigned char *)&cxd3776er_tone_control_table[motion_feedback_mode])+CODEC_RAM_WORD_SIZE*8*n,
			CODEC_RAM_WORD_SIZE*8
		);
		if(rv<0){
			back_trace();
			return(-1);
		}
	}

	/* set mode to normal */
	rv=cxd3776er_register_modify(CXD3776ER_RAM_CONTROL_1,CODEC_RAM_NORMAL,0xDF);
	if(rv<0){
		back_trace();
		return(-1);
	}

	cxd3776er_register_modify(CXD3776ER_DEQ_CONTROL,0x00,0x80);

	cxd3776er_register_write_multiple(
		CXD3776ER_DEQ_1_COEF_BASE,
		(unsigned char *)&cxd3776er_tone_control_table[table].coefficient[0],
		sizeof(struct cxd3776er_deq_coefficient)
	);
	cxd3776er_register_write_multiple(
		CXD3776ER_DEQ_2_COEF_BASE,
		(unsigned char *)&cxd3776er_tone_control_table[table].coefficient[1],
		sizeof(struct cxd3776er_deq_coefficient)
	);
	cxd3776er_register_write_multiple(
		CXD3776ER_DEQ_3_COEF_BASE,
		(unsigned char *)&cxd3776er_tone_control_table[table].coefficient[2],
		sizeof(struct cxd3776er_deq_coefficient)
	);
	cxd3776er_register_write_multiple(
		CXD3776ER_DEQ_4_COEF_BASE,
		(unsigned char *)&cxd3776er_tone_control_table[table].coefficient[3],
		sizeof(struct cxd3776er_deq_coefficient)
	);
	cxd3776er_register_write_multiple(
		CXD3776ER_DEQ_5_COEF_BASE,
		(unsigned char *)&cxd3776er_tone_control_table[table].coefficient[4],
		sizeof(struct cxd3776er_deq_coefficient)
	);

	cxd3776er_register_modify(CXD3776ER_DEQ_CONTROL,0x80,0x80);
#endif
	print_debug("tone control: table=%d\n",table);

	return(0);
}

static int adjust_device_gain(int input_device)
{
	print_trace("%s(%d)\n",__FUNCTION__,input_device);
#if 0
	cxd3776er_register_write(CXD3776ER_PGA_1L,cxd3776er_device_gain_table[input_device].pga);
	cxd3776er_register_write(CXD3776ER_PGA_1R,cxd3776er_device_gain_table[input_device].pga);
	cxd3776er_register_write(CXD3776ER_ADC_1L,cxd3776er_device_gain_table[input_device].adc);
	cxd3776er_register_write(CXD3776ER_ADC_1R,cxd3776er_device_gain_table[input_device].adc);

/* PGA gain */
	cxd3776er_register_write(CXD3776ER_AIN1,cxd3776er_device_gain_table[input_device].adc);
#endif
	return(0);
}

static int report_tpm_status(
	int old_status,
	int new_status,
	int force
)
{
	/* print_trace("%s(%d,%d,%d)\n",__FUNCTION__,old_status,new_status,force); */
#if 0
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
			//cxd3776er_switch_set_headphone_value(2);
		}
		if((force || old_status==JACK_STATUS_3PIN || old_status==JACK_STATUS_5PIN   )
		&& (         new_status==JACK_STATUS_NONE || new_status==JACK_STATUS_ANTENNA)){
			/* for WiredAccessoryObserver.java */
			/* 0 = indicate no headphone       */
			//cxd3776er_switch_set_headphone_value(0);
		}
		if((force || old_status==JACK_STATUS_NONE) && (new_status!=JACK_STATUS_NONE)){
			//cxd3776er_switch_set_antenna_value(1);
		}
		if((force || old_status!=JACK_STATUS_NONE) && (new_status==JACK_STATUS_NONE)){
			//cxd3776er_switch_set_antenna_value(0);
		}
	}
#endif
	return(0);
}

#ifdef ICX_ENABLE_AU2CLK

static int change_external_osc(
	struct cxd3776er_status * status
)
{
	int now;
	int req;

	print_trace("%s()\n",__FUNCTION__);

	now=cxd3776er_get_external_osc();

	req=suitable_external_osc(
		status->output_path,
		status->output_select,
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

		cxd3776er_set_output_path_mute(status,ON,TRUE);

		if(status->output_path==OUTPUT_SELECT_NONE_HEADPHONE
		&& status->output_select==HEADPHONE_AMP_SMASTER){

			/* PCMCLKO=off */
			cxd3776er_register_modify(CXD3776ER_MAIN_CLOCK,0x10,0x10);

			/* disable digital amp */
			digiamp_disable(DAMP_CTL_LEVEL_RESET);
		}

		/* MCLK=off */
		cxd3776er_register_modify(CXD3776ER_MAIN_CLOCK,0x00,0x01);

		/* change master clock */
		cxd3776er_switch_external_osc(req);

		/* MCLK=on */
		cxd3776er_register_modify(CXD3776ER_MAIN_CLOCK,0x01,0x01);

		if(status->output_path==OUTPUT_SELECT_NONE_HEADPHONE
		&& status->output_select==HEADPHONE_AMP_SMASTER){

			/* enable digital amp */
			digiamp_enable(DAMP_CTL_LEVEL_RESET);

			/* PCMCLKO=on */
			cxd3776er_register_modify(CXD3776ER_DAC,0x00,0x10);
		}

		cxd3776er_set_output_path_mute(status,OFF,TRUE);
	}
	else{
		print_debug("external OSC = SAME\n");
	}

	return(0);
}

static int suitable_external_osc(
	int output_path,
	int output_select,
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
		output_path,
		output_select,
		icx_playback_active,
		std_playback_active,
		capture_active,
		sample_rate
	);

	if(capture_active>0){
		osc=EXTERNAL_OSC_441;
	}
	else if(output_path!=OUTPUT_SELECT_NONE_HEADPHONE || output_select!=HEADPHONE_AMP_SMASTER){
		osc=EXTERNAL_OSC_441;
	}
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

static int check_active(struct cxd3776er_status * status)
{
	int ret=FALSE;

	print_trace("%s()\n",__FUNCTION__);

	if(status->output_path!=OUTPUT_PATH_NONE)
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
	cxd3776er_switch_external_osc(EXTERNAL_OSC_441);
#else
	cxd3776er_switch_external_osc(EXTERNAL_OSC_480);
#endif

	/* unreset hardware */
	cxd3776er_unreset();

	/* INV_DAC_CK:invert */
	cxd3776er_register_modify(CXD3776ER_DACOUT,0xC0,0xC0);

	cxd3776er_register_modify(CXD3776ER_TRIM_1,0x00,0x07);
	cxd3776er_register_modify(CXD3776ER_TRIM_2,0x03,0x03);

	/* DET_SPCLKERR */
	cxd3776er_register_modify(CXD3776ER_MISC,0x40,0x40);

	/* PWMOUT_DS=8mA, PCMCLKO_DS=10mA */
//	cxd3776er_register_modify(CXD3776ER_IO_DRIVE,0x05,0x0F);

	/* VCOM:ON, ADIN:OFF, DAC1:ON, DAC2:ON */
	cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0xBF,0xDF);

	/* FS256O=OFF, MCKO=OFF, DBLR=OFF, TCXO_SEL=0, MCK_DBL_SEL=0, MCK=ON */
	cxd3776er_register_modify(CXD3776ER_MAIN_CLOCK,0x21,0xFF);

	/* codec mode=high, I2S=slave, format=I2S, fs=1024 */
//	cxd3776er_register_modify(CXD3776ER_MODE_CONTROL,0xC2,0xFE);

	/* DSP:ON, SRC:ON;SMSTR:ON */
	cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x3E,0xDF);

	/* DSPRAM_S, SRESET */
	cxd3776er_register_modify(CXD3776ER_TEST,0x30,0x30);

	/* wait clear */
	msleep(20);

	/* SRC2IN=ADC, SRC2 mode=low, SRC1IN=ADC, SRC1 mode=XXX */
	if(rate<=48000){
		cxd3776er_register_modify(CXD3776ER_SD1,0x01,0xFF);
	}
	else if(rate<=96000){
		cxd3776er_register_modify(CXD3776ER_SD1,0x02,0xFF);
	}
	else{
		cxd3776er_register_modify(CXD3776ER_SD1,0x03,0xFF);
	}

	/* SD2_1_MODE = 10  (DMFB)  SD2_2_MODE  10  (CODEC) */
	cxd3776er_register_modify(CXD3776ER_SD2,0x2A,0xFF);
#if 0
	/* DITHER */
	cxd3776er_register_write(CXD3776ER_S_MASTER_DITHER_1,0x00);
	cxd3776er_register_write(CXD3776ER_S_MASTER_DITHER_2,0x7F);
	cxd3776er_register_write(CXD3776ER_S_MASTER_DITHER_3,0xB5);
	cxd3776er_register_write(CXD3776ER_DITHER_MIC,0x8A);
	cxd3776er_register_write(CXD3776ER_DITHER_LINE,0x8A);

	/* ARC=0dB fixed, HPF1=1.71Hz */
	cxd3776er_register_modify(CXD3776ER_LINEIN_1,0x84,0xFC);

	/* CIC1IN=zero */
	cxd3776er_register_modify(CXD3776ER_LINEIN_2,0x00,0xC0);

	/* CHGFREQ=564.5KHz, ADPTPWR=instantaneous value from nsin */
	cxd3776er_register_modify(CXD3776ER_CHARGE_PUMP,0x46,0xF7);

	/* NS_OUT=RZ1 */
	cxd3776er_register_modify(CXD3776ER_DAC,0x02,0x03);

	/* mic bias */
	set_mic_bias_mode(CXD3776ER_MIC_BIAS_MODE_NORMAL);
#endif
	cxd3776er_register_write(CXD3776ER_CODEC,0x00);

	return(0);
}

static int shutdown_chip(void)
{
	print_trace("%s()\n",__FUNCTION__);

	/* mute_b=on, sdout2=off, sdout1=off, sdin2=off, sdin2=off */
	cxd3776er_register_modify(CXD3776ER_SD2,0x00,0xFF);

	/* all off */
	cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0xFF,0xFF);

	/* all off */
	cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0xFF,0xFF);

	cxd3776er_reset();

	/* external OSC off (only 1240/1265) */
	cxd3776er_switch_external_osc(EXTERNAL_OSC_OFF);

	return(0);
}

static int switch_cpclk(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value){
		/* CPCLK=on */
		cxd3776er_register_modify(CXD3776ER_CLASS_H_2,0x04,0x04);
		msleep(30);
	}
	else{
		/* CPCLK=off */
		cxd3776er_register_modify(CXD3776ER_CLASS_H_2,0x00,0x04);
	}
#endif
	return(0);
}

static int switch_dac1(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* DAC L/R=on */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0x00,0x02);
		msleep(10);
	}
	else{
		/* DAC L/R=off */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0x02,0x02);
	}

	return(0);
}

static int switch_dac2(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* DAC L/R=on */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0x00,0x01);
		msleep(10);
	}
	else{
		/* DAC L/R=off */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0x01,0x01);
	}

	return(0);
}

static int switch_smaster_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){

		/* enable digital amp */
		digiamp_enable(DAMP_CTL_LEVEL_POWER);

		/* SMASTER=on */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x00,0x08);

		msleep(10);
	}
	else{
		/* mute=on, NSX2I=0, NSADJON=0 */
//              cxd3776er_register_modify(CXD3776ER_S_MASTER_CONTROL,0x80,0xE0);

		/* PCMCLKO=off */
//              cxd3776er_register_modify(CXD3776ER_DAC,0x80,0x80);

		/* SMASTER=off */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x08,0x08);

		/* disable digital amp */
		digiamp_disable(DAMP_CTL_LEVEL_POWER);
	}

	return(0);
}

static int switch_smaster1(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* UnMute */
		cxd3776er_register_modify(CXD3776ER_PWMBUF,0x00,0x30);
		cxd3776er_register_modify(CXD3776ER_SMASTER_PWM_A,0x00,0x80);
		cxd3776er_register_modify(CXD3776ER_SMASTER_NS1_A,0x00,0x80);
		msleep(10);
	}
	else{
		/* Mute */
		cxd3776er_register_modify(CXD3776ER_SMASTER_PWM_A,0x80,0x80);
		cxd3776er_register_modify(CXD3776ER_SMASTER_NS1_A,0x80,0x80);
	}

	return(0);
}

static int switch_smaster2(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value){
		/* UnMute */
		cxd3776er_register_modify(CXD3776ER_PWMBUF,0x00,0x03);
		cxd3776er_register_modify(CXD3776ER_SMASTER_PWM_B,0x00,0x80);
		cxd3776er_register_modify(CXD3776ER_SMASTER_NS1_B,0x00,0x80);
		msleep(10);
	}
	else{
		/* Mute */
		cxd3776er_register_modify(CXD3776ER_SMASTER_PWM_B,0x80,0x80);
		cxd3776er_register_modify(CXD3776ER_SMASTER_NS1_B,0x80,0x80);
	}

	return(0);
}
static int switch_classh(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value){
		/* HPOUT L/R=on */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_3,0x00,0x30);
		msleep(10);
	}
	else{
		/* HPOUT L/R=off */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_3,0x30,0x30);
	}
#endif
	return(0);
}

static int switch_i2sout1(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
	if(value){
		/* lineout 1 L/R=on */
		cxd3776er_register_modify(CXD3776ER_SD2,0x02,0x03);
		msleep(10);
	}
	else{
		/* lineout 1 L/R=off */
		cxd3776er_register_modify(CXD3776ER_SD2,0x00,0x03);;
	}

	return(0);
}

static int switch_i2sout2(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
	if(value){
		/* lineout 1 L/R=on */
		cxd3776er_register_modify(CXD3776ER_SD2,0x08,0x0C);
		msleep(10);
	}
	else{
		/* lineout 1 L/R=off */
		cxd3776er_register_modify(CXD3776ER_SD2,0x00,0x0C);;
	}

	return(0);
}

static int switch_speaker(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value){
		/* lineout 2 L=on */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_3,0x00,0x03);
		msleep(10);
	}
	else{
		/* lineout 2 L=off */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_3,0x03,0x03);
	}
#endif
	return(0);
}

static int switch_speaker_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value){
		msleep(5);

		/* speaker on */
		cxd3776er_switch_speaker_power(ON);

		msleep(50);
	}
	else{
		/* speaker off */
		cxd3776er_switch_speaker_power(OFF);

		msleep(5);
	}
#endif
	return(0);
}

static int switch_linein(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value){
		/* port=LINE1 */
		cxd3776er_register_modify(CXD3776ER_LINEIN_2,0x00,0x30);

		/* cic1in=LINE */
		cxd3776er_register_modify(CXD3776ER_LINEIN_3,0xC0,0xC0);

		/* BL DSP=on */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x00,0x02);

		/* linein L/R=on */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0x00,0x03);
	}
	else{
		/* linein L/R=off */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0x03,0x03);

		/* BL DSP=off */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x02,0x02);

		/* cic1in=zero */
		cxd3776er_register_modify(CXD3776ER_LINEIN_3,0x00,0xC0);

		/* port=LINE1 */
		cxd3776er_register_modify(CXD3776ER_LINEIN_2,0x00,0x30);
	}
#endif
	return(0);
}

static int switch_tuner(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

#if 0
	if(value){
		/* port=LINE2 */
		cxd3776er_register_modify(CXD3776ER_LINEIN_2,0x20,0x30);

		/* cic1in=LINE */
		cxd3776er_register_modify(CXD3776ER_LINEIN_3,0xC0,0xC0);

		/* BL DSP=on */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x00,0x02);

		/* linein L/R=on */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0x00,0x03);
	}
	else{
		/* linein L/R=off */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0x03,0x03);

		/* BL DSP=off */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x02,0x02);

		/* cic1in=zero */
		cxd3776er_register_modify(CXD3776ER_LINEIN_3,0x00,0xC0);

		/* port=LINE1 */
		cxd3776er_register_modify(CXD3776ER_LINEIN_2,0x00,0x30);
	}
#endif
	return(0);
}

static int switch_dmic(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value){
		/* cic1in=DMIC */
		cxd3776er_register_modify(CXD3776ER_LINEIN_3,0x80,0xC0);

		/* BL DSP=on, DMIC=on */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x00,0x03);

		/* ADC1_BOOST=on */
		cxd3776er_register_modify(CXD3776ER_LINEIN_2,0x04,0x04);
	}
	else{
		/* BL DSP=off, DMIC=off */
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x03,0x03);

		/* cic1in=zero */
		cxd3776er_register_modify(CXD3776ER_LINEIN_3,0x00,0xC0);

		/* ADC1_BOOST=off */
		cxd3776er_register_modify(CXD3776ER_LINEIN_2,0x00,0x04);
	}
#endif
	return(0);
}

static int get_tpm_status(int motion_feedback_active, int tpm_mode)
{

	print_trace("%s(%d,%d)\n",__FUNCTION__,motion_feedback_active,tpm_mode);
#if 0
#ifdef ICX_ENABLE_NC

	int status;
	int hp;
	int nc;

	/* print_trace("%s(%d,%d)\n",__FUNCTION__,motion_feedback_active,tpm_mode); */

	if(motion_feedback_active == FALSE){
		set_mic_bias_mode(CXD3776ER_MIC_BIAS_MODE_MIC_DET);
		/* wait comparetor statble */
		msleep(80);
	}

	hp=cxd3776er_get_hp_det_value();
	nc=cxd3776er_get_mic_det_value();

	/* print_debug("hp=%d, nc=%d\n",hp,nc); */

	if(hp){
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

	if(motion_feedback_active == FALSE){
		set_mic_bias_mode(CXD3776ER_MIC_BIAS_MODE_NORMAL);
	}
#else

	int status;
	int hp;

	/* print_trace("%s(%d,%d)\n",__FUNCTION__,motion_feedback_active,tpm_mode); */

//	hp=cxd3776er_get_hp_det_value();

	if(hp){
		status=JACK_STATUS_3PIN;
	}
	else{
		status=JACK_STATUS_NONE;
	}

#endif

	if(tpm_mode==JACK_MODE_ANTENNA){
		if(status!=JACK_STATUS_NONE)
			status=JACK_STATUS_ANTENNA;
	}

	return(status);
#endif
return(0);
}

static int set_mic_bias_mode(int mode)
{
#if 0
	/* print_trace("%s(%d)\n",__FUNCTION__,mode); */

	if(mode==CXD3776ER_MIC_BIAS_MODE_NC_ON){
		/* B=2V A=2V pull-none */
		cxd3776er_register_modify(CXD3776ER_MIC_BIAS,0x2A,0x3F);
	}
	else if(mode==CXD3776ER_MIC_BIAS_MODE_MIC_DET){
		/* B=2V A=hiz pull-none */
		cxd3776er_register_modify(CXD3776ER_MIC_BIAS,0x22,0x3F);
	}
	else if(mode==CXD3776ER_MIC_BIAS_MODE_NORMAL){
		/* B=2V A=hiz pull-up */
		cxd3776er_register_modify(CXD3776ER_MIC_BIAS,0x20,0x3F);
	}
	else {
		/* B=hiz A=hiz pull-up */
		cxd3776er_register_modify(CXD3776ER_MIC_BIAS,0x00,0x3F);
	}
#endif
	return(0);
}

static int switch_dmfb_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value)
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x00,0x02);
	else
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x02,0x02);

	return(0);
}

static int switch_tpm_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);

	if(value)
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x00,0x04);
	else
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_1,0x04,0x04);

	return(0);
}

static int switch_ncmic_amp_power(int value)
{
	print_trace("%s(%d)\n",__FUNCTION__,value);
#if 0
	if(value)
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0x00,0x0C);
	else
		cxd3776er_register_modify(CXD3776ER_POWER_CONTROL_2,0x0C,0x0C);
#endif
	return(0);
}

static int show_device_id(void)
{
	unsigned int val;
	int rv;

	rv=cxd3776er_register_read(CXD3776ER_DEVICE_ID,&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	print_info("Device ID = 0x%02X\n",val);
	rv=cxd3776er_register_read(CXD3776ER_REVISION_NO,&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	print_info("revision = 0x%02X\n",val);
	return(0);
}

