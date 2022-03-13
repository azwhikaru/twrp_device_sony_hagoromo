/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf.c
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
#define TRACE_TAG "####### "
#define DEBUG_TAG "        "

#include "cxd3774gf_common.h"

/**************/
/*@ prototype */
/**************/

/* entry_routines */

static int  __init cxd3774gf_init(void);
static void __exit cxd3774gf_exit(void);

static int cxd3774gf_i2c_probe(
	      struct i2c_client    * client,
	const struct i2c_device_id * identify
);
static int cxd3774gf_i2c_remove(
	struct i2c_client * client
);
static void cxd3774gf_i2c_poweroff(
	struct i2c_client * client
);

static int cxd3774gf_codec_probe (struct snd_soc_codec * codec);
static int cxd3774gf_codec_remove(struct snd_soc_codec * codec);

/* power_management_routines */

static int cxd3774gf_i2c_suspend(struct device * device);
static int cxd3774gf_i2c_resume(struct device * device);

/* dai_ops */

static int cxd3774gf_icx_dai_startup(
	struct snd_pcm_substream * substream,
	struct snd_soc_dai       * dai
);
static int cxd3774gf_std_dai_startup(
	struct snd_pcm_substream * substream,
	struct snd_soc_dai       * dai
);

static void cxd3774gf_icx_dai_shutdown(
	struct snd_pcm_substream * substream,
	struct snd_soc_dai       * dai
);
static void cxd3774gf_std_dai_shutdown(
	struct snd_pcm_substream * substream,
	struct snd_soc_dai       * dai
);

static int cxd3774gf_icx_dai_set_fmt(
	struct snd_soc_dai * codec_dai,
	unsigned int         format
);
static int cxd3774gf_std_dai_set_fmt(
	struct snd_soc_dai * codec_dai,
	unsigned int         format
);

static int cxd3774gf_icx_dai_hw_params(
	struct snd_pcm_substream * substream,
	struct snd_pcm_hw_params * params,
	struct snd_soc_dai       * dai
);
static int cxd3774gf_std_dai_hw_params(
	struct snd_pcm_substream * substream,
	struct snd_pcm_hw_params * params,
	struct snd_soc_dai       * dai
);

static int cxd3774gf_icx_dai_mute(
	struct snd_soc_dai * dai,
	int                  mute
);
static int cxd3774gf_std_dai_mute(
	struct snd_soc_dai * dai,
	int                  mute
);

/* noise_cancel */

static int cxd3774gf_put_noise_cancel_mode_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_noise_cancel_mode_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_noise_cancel_status_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_noise_cancel_status_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_user_noise_cancel_gain_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_user_noise_cancel_gain_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_base_noise_cancel_gain_left_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_base_noise_cancel_gain_left_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_base_noise_cancel_gain_right_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_base_noise_cancel_gain_right_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_base_noise_cancel_gain_save_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_base_noise_cancel_gain_save_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);

/* sound_effect */

static int cxd3774gf_put_sound_effect_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_sound_effect_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);

/* volume_and_mute */

static int cxd3774gf_put_master_volume_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_master_volume_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_master_gain_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_master_gain_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_playback_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_playback_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_capture_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_capture_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_analog_playback_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_analog_playback_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_analog_stream_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_analog_stream_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_std_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_std_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_icx_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_icx_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);

/* device_control */

static int cxd3774gf_put_output_device_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_output_device_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_input_device_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_input_device_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_headphone_amp_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_headphone_amp_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_headphone_type_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_headphone_type_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_jack_mode_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_jack_mode_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_put_jack_status_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_jack_status_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);

/* debug_test */

static int cxd3774gf_put_debug_test_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);
static int cxd3774gf_get_debug_test_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
);

/**************/
/*@ variables */
/**************/

MODULE_AUTHOR("Sony Corporation");
MODULE_DESCRIPTION("CXD3774GF CODEC driver");
MODULE_LICENSE("GPL");

module_init(cxd3774gf_init);
module_exit(cxd3774gf_exit);

static const struct i2c_device_id cxd3774gf_id[] = {
	{CXD3774GF_DEVICE_NAME, 0},
	{}
};

static struct dev_pm_ops cxd3774gf_pm_ops = {
	.suspend  = cxd3774gf_i2c_suspend,
	.resume   = cxd3774gf_i2c_resume,
};

static struct i2c_driver cxd3774gf_i2c_driver = {
	.driver   = {
		.name  = CXD3774GF_DEVICE_NAME,
		.owner = THIS_MODULE,
		.pm    = &cxd3774gf_pm_ops,
	},
	.id_table = cxd3774gf_id,
	.probe    = cxd3774gf_i2c_probe,
	.remove   = cxd3774gf_i2c_remove,
	.shutdown = cxd3774gf_i2c_poweroff,
};

static struct snd_soc_codec_driver cxd3774gf_codec_driver = {
	.probe   = cxd3774gf_codec_probe,
	.remove  = cxd3774gf_codec_remove,
};

static struct snd_soc_dai_ops cxd3774gf_icx_dai_ops = {
	.startup      = cxd3774gf_icx_dai_startup,
	.shutdown     = cxd3774gf_icx_dai_shutdown,
	.set_fmt      = cxd3774gf_icx_dai_set_fmt,
	.hw_params    = cxd3774gf_icx_dai_hw_params,
	.digital_mute = cxd3774gf_icx_dai_mute,
};
static struct snd_soc_dai_ops cxd3774gf_std_dai_ops = {
	.startup      = cxd3774gf_std_dai_startup,
	.shutdown     = cxd3774gf_std_dai_shutdown,
	.set_fmt      = cxd3774gf_std_dai_set_fmt,
	.hw_params    = cxd3774gf_std_dai_hw_params,
	.digital_mute = cxd3774gf_std_dai_mute,
};

static struct snd_soc_dai_driver cxd3774gf_dai_driver[] = {
	{
		.name            = CXD3774GF_STD_DAI_NAME,
		.playback        = {
			.stream_name  = "FM Playback", /* same to omap-abe-dsp.c widget stream name */
			.channels_min = 2,
			.channels_max = 2,
			.rates        = SNDRV_PCM_RATE_44100,
			.formats      = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture         = {
			.stream_name  = "Capture",
			.channels_min = 2,
			.channels_max = 2,
			.rates        = SNDRV_PCM_RATE_44100,
			.formats      = SNDRV_PCM_FMTBIT_S16_LE,
		 },
		.ops             = &cxd3774gf_std_dai_ops,
		.symmetric_rates = 1,
	},
	{
		.name            = CXD3774GF_ICX_DAI_NAME,
		.playback        = {
			.stream_name  = "Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates        = 0
						  | SNDRV_PCM_RATE_5512
						  | SNDRV_PCM_RATE_11025
						  | SNDRV_PCM_RATE_22050
						  | SNDRV_PCM_RATE_44100
						  | SNDRV_PCM_RATE_88200
						  | SNDRV_PCM_RATE_176400
						  | SNDRV_PCM_RATE_8000
						  | SNDRV_PCM_RATE_16000
						  | SNDRV_PCM_RATE_32000
						  | SNDRV_PCM_RATE_48000
						  | SNDRV_PCM_RATE_64000
						  | SNDRV_PCM_RATE_96000
						  | SNDRV_PCM_RATE_192000
						  | 0,
			.formats      = 0
						  | SNDRV_PCM_FMTBIT_S16_LE
						  | SNDRV_PCM_FMTBIT_S32_LE
						  | 0,
		},
#if 0
		.capture         = {
			.stream_name  = "Capture",
			.channels_min = 2,
			.channels_max = 2,
			.rates        = 0
						  | SNDRV_PCM_RATE_5512
						  | SNDRV_PCM_RATE_11025
						  | SNDRV_PCM_RATE_22050
						  | SNDRV_PCM_RATE_44100
						  | SNDRV_PCM_RATE_88200
						  | SNDRV_PCM_RATE_176400
						  | SNDRV_PCM_RATE_8000
						  | SNDRV_PCM_RATE_16000
						  | SNDRV_PCM_RATE_32000
						  | SNDRV_PCM_RATE_48000
						  | SNDRV_PCM_RATE_64000
						  | SNDRV_PCM_RATE_96000
						  | SNDRV_PCM_RATE_192000
						  | 0,
			.formats      = 0
						  | SNDRV_PCM_FMTBIT_S16_LE
						  | SNDRV_PCM_FMTBIT_S32_LE
						  | 0,
		 },
#endif
		.ops             = &cxd3774gf_icx_dai_ops,
		.symmetric_rates = 1,
	}
};

#define DUMMY_REG   0
#define DUMMY_SFT   0
#define DUMMY_SFT_L 0
#define DUMMY_SFT_R 4
#define DUMMY_INV   0

static const char * noise_cancel_mode_value[] = {
	"off",
	"airplane",
	"train",
	"office",
};
static const struct soc_enum noise_cancel_mode_enum
	= SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(noise_cancel_mode_value), noise_cancel_mode_value);

static const char * output_value[] = {
	"off",
	"headphone",
	"line",
	"speaker",
	"fixedline",
};
static const struct soc_enum output_enum
	= SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(output_value), output_value);

static const char * input_value[] = {
	"off",
	"tuner",
	"mic",
	"line",
	"directmic",
};
static const struct soc_enum input_enum
	= SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(input_value), input_value);

static const char * headphone_amp_value[] = {
	"normal",
	"smaster",
};
static const struct soc_enum headphone_amp_enum
	= SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(headphone_amp_value), headphone_amp_value);

static const char * headphone_type_value[] = {
	"bundle",
	"model1",
};
static const struct soc_enum headphone_type_enum
	= SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(headphone_type_value), headphone_type_value);

static const char * jack_mode_value[] = {
	"headphone",
	"antenna",
};
static const struct soc_enum jack_mode_enum
	= SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_mode_value), jack_mode_value);

static const char * jack_status_value[] = {
	"none",
	"3pin",
	"5pin",
	"antenna",
};
static const struct soc_enum jack_status_enum
	= SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_status_value), jack_status_value);

static const struct snd_kcontrol_new cxd3774gf_snd_controls[] =
{
	/* noise_cancel */

	SOC_ENUM_EXT(
		"noise cancel mode",
		noise_cancel_mode_enum,
		cxd3774gf_get_noise_cancel_mode_control,
		cxd3774gf_put_noise_cancel_mode_control
	),
	SOC_SINGLE_EXT(
		"noise cancel status",
		DUMMY_REG,
		DUMMY_SFT,
		1,
		DUMMY_INV,
		cxd3774gf_get_noise_cancel_status_control,
		cxd3774gf_put_noise_cancel_status_control
	),
	SOC_SINGLE_EXT(
		"user noise cancel gain",
		DUMMY_REG,
		DUMMY_SFT,
		USER_NOISE_CANCEL_GAIN_INDEX_MAX,
		DUMMY_INV,
		cxd3774gf_get_user_noise_cancel_gain_control,
		cxd3774gf_put_user_noise_cancel_gain_control
	),
	SOC_SINGLE_EXT(
		"base noise cancel gain left",
		DUMMY_REG,
		DUMMY_SFT,
		BASE_NOISE_CANCEL_GAIN_INDEX_MAX,
		DUMMY_INV,
		cxd3774gf_get_base_noise_cancel_gain_left_control,
		cxd3774gf_put_base_noise_cancel_gain_left_control
	),
	SOC_SINGLE_EXT(
		"base noise cancel gain right",
		DUMMY_REG,
		DUMMY_SFT,
		BASE_NOISE_CANCEL_GAIN_INDEX_MAX,
		DUMMY_INV,
		cxd3774gf_get_base_noise_cancel_gain_right_control,
		cxd3774gf_put_base_noise_cancel_gain_right_control
	),
	SOC_SINGLE_EXT(
		"base noise cancel gain save",
		DUMMY_REG,
		DUMMY_SFT,
		1,
		DUMMY_INV,
		cxd3774gf_get_base_noise_cancel_gain_save_control,
		cxd3774gf_put_base_noise_cancel_gain_save_control
	),

	/* sound_effect */

	SOC_SINGLE_EXT(
		"sound effect",
		DUMMY_REG,
		DUMMY_SFT,
		1,
		DUMMY_INV,
		cxd3774gf_get_sound_effect_control,
		cxd3774gf_put_sound_effect_control
	),

	/* volume_and_mute */

	SOC_SINGLE_EXT(
		"master volume",
		DUMMY_REG,
		DUMMY_SFT,
		MASTER_VOLUME_MAX,
		DUMMY_INV,
		cxd3774gf_get_master_volume_control,
		cxd3774gf_put_master_volume_control
	),
	SOC_SINGLE_EXT(
		"master gain",
		DUMMY_REG,
		DUMMY_SFT,
		MASTER_GAIN_MAX,
		DUMMY_INV,
		cxd3774gf_get_master_gain_control,
		cxd3774gf_put_master_gain_control
	),
	SOC_SINGLE_EXT(
		"playback mute",
		DUMMY_REG,
		DUMMY_SFT,
		1,
		DUMMY_INV,
		cxd3774gf_get_playback_mute_control,
		cxd3774gf_put_playback_mute_control
	),
	SOC_SINGLE_EXT(
		"capture mute",
		DUMMY_REG,
		DUMMY_SFT,
		1,
		DUMMY_INV,
		cxd3774gf_get_capture_mute_control,
		cxd3774gf_put_capture_mute_control
	),
	SOC_SINGLE_EXT(
		"analog playback mute",
		DUMMY_REG,
		DUMMY_SFT,
		1,
		DUMMY_INV,
		cxd3774gf_get_analog_playback_mute_control,
		cxd3774gf_put_analog_playback_mute_control
	),
	SOC_SINGLE_EXT(
		"analog stream mute",
		DUMMY_REG,
		DUMMY_SFT,
		1,
		DUMMY_INV,
		cxd3774gf_get_analog_stream_mute_control,
		cxd3774gf_put_analog_stream_mute_control
	),
	SOC_SINGLE_EXT(
		"timed mute",
		DUMMY_REG,
		DUMMY_SFT,
		10000,
		DUMMY_INV,
		cxd3774gf_get_timed_mute_control,
		cxd3774gf_put_timed_mute_control
	),
	SOC_SINGLE_EXT(
		"std timed mute",
		DUMMY_REG,
		DUMMY_SFT,
		10000,
		DUMMY_INV,
		cxd3774gf_get_std_timed_mute_control,
		cxd3774gf_put_std_timed_mute_control
	),
	SOC_SINGLE_EXT(
		"icx timed mute",
		DUMMY_REG,
		DUMMY_SFT,
		10000,
		DUMMY_INV,
		cxd3774gf_get_icx_timed_mute_control,
		cxd3774gf_put_icx_timed_mute_control
	),

	/* device_control */

	SOC_ENUM_EXT(
		"output device",
		output_enum,
		cxd3774gf_get_output_device_control,
		cxd3774gf_put_output_device_control
	),
	SOC_ENUM_EXT(
		"analog input device",
		input_enum,
		cxd3774gf_get_input_device_control,
		cxd3774gf_put_input_device_control
	),
	SOC_ENUM_EXT(
		"headphone amp",
		headphone_amp_enum,
		cxd3774gf_get_headphone_amp_control,
		cxd3774gf_put_headphone_amp_control
	),
	SOC_ENUM_EXT(
		"noise cancel headphone type",
		headphone_type_enum,
		cxd3774gf_get_headphone_type_control,
		cxd3774gf_put_headphone_type_control
	),
	SOC_ENUM_EXT(
		"jack mode",
		jack_mode_enum,
		cxd3774gf_get_jack_mode_control,
		cxd3774gf_put_jack_mode_control
	),
	SOC_ENUM_EXT(
		"jack status",
		jack_status_enum,
		cxd3774gf_get_jack_status_control,
		cxd3774gf_put_jack_status_control
	),

	/* debug_test */

	SOC_SINGLE_EXT(
		"debug test",
		DUMMY_REG,
		DUMMY_SFT,
		100,
		DUMMY_INV,
		cxd3774gf_get_debug_test_control,
		cxd3774gf_put_debug_test_control
	),
};

static int initialized=FALSE;

/*******************/
/*@ entry_routines */
/*******************/

static int __init cxd3774gf_init(void)
{
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	rv=i2c_add_driver(&cxd3774gf_i2c_driver);
	if(rv!=0) {
		print_fail("i2c_add_driver(): code %d error occurred.\n",rv);
		back_trace();
		return(rv);
	}

	return(0);
}

static void __exit cxd3774gf_exit(void)
{
	print_trace("%s()\n",__FUNCTION__);

	i2c_del_driver(&cxd3774gf_i2c_driver);

	return;
}

static int cxd3774gf_i2c_probe(
	      struct i2c_client    * client,
	const struct i2c_device_id * identify
)
{
	struct cxd3774gf_driver_data * driver_data;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	print_info("starting driver...\n");

#ifdef ICX_ENABLE_VOL60STP
	print_info("volume step : 60\n");
#else
	print_info("volume step : 30\n");
#endif

	driver_data=kzalloc(sizeof(struct cxd3774gf_driver_data),GFP_KERNEL);
	if(driver_data==NULL){
		print_fail("kzalloc(): no memory.\n");
		back_trace();
		return(-ENOMEM);
	}

	dev_set_drvdata(&client->dev,driver_data);
	driver_data->i2c=client;

	rv=snd_soc_register_codec(
		&client->dev,
		&cxd3774gf_codec_driver,
		cxd3774gf_dai_driver,
		ARRAY_SIZE(cxd3774gf_dai_driver)
	);
	if(rv<0){
		print_fail("snd_soc_register_codec(): code %d error occurred.\n",rv);
		back_trace();
		kfree(driver_data);
		return(rv);
	}

	return(0);
}

static int cxd3774gf_i2c_remove(
	struct i2c_client * client
)
{
	struct cxd3774gf_driver_data * driver_data;

	print_trace("%s()\n",__FUNCTION__);

	driver_data=dev_get_drvdata(&client->dev);

	snd_soc_unregister_codec(&client->dev);

	kfree(driver_data);

	return(0);
}

static void cxd3774gf_i2c_poweroff(struct i2c_client * client)
{
	struct cxd3774gf_driver_data * driver_data;

	print_trace("%s()\n",__FUNCTION__);

	driver_data=dev_get_drvdata(&client->dev);

	/* snd_soc_unregister_codec(&client->dev); */

	cxd3774gf_codec_remove(driver_data->codec);

	kfree(driver_data);

	return;
}

static int cxd3774gf_codec_probe(struct snd_soc_codec * codec)
{
	struct cxd3774gf_driver_data * driver_data;
	struct cxd3774gf_platform_data * platform_data;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	driver_data=dev_get_drvdata(codec->dev);
	platform_data=dev_get_platdata(codec->dev);

	driver_data->codec=codec;

	mutex_init(&driver_data->mutex);
	wake_lock_init(&driver_data->wake_lock, WAKE_LOCK_SUSPEND, "CXD3774GF");

	rv=cxd3774gf_setup_platform(platform_data);
	if(rv<0){
		back_trace();
		cxd3774gf_codec_remove(codec);
		return(rv);
	}

	rv=cxd3774gf_register_initialize(driver_data->i2c);
	if(rv<0){
		back_trace();
		cxd3774gf_codec_remove(codec);
		return(rv);
	}

	rv=cxd3774gf_switch_initialize();

	if(rv<0){
		back_trace();
		cxd3774gf_codec_remove(codec);
		return(rv);
	}

	rv=cxd3774gf_table_initialize(&driver_data->mutex);
	if(rv<0){
		back_trace();
		cxd3774gf_codec_remove(codec);
		return(rv);
	}

	rv=cxd3774gf_volume_initialize();
	if(rv<0){
		back_trace();
		cxd3774gf_codec_remove(codec);
		return(rv);
	}

	rv=cxd3774gf_core_initialize(&driver_data->mutex);
	if(rv<0){
		back_trace();
		cxd3774gf_codec_remove(codec);
		return(rv);
	}
#if 0
	rv=cxd3774gf_interrupt_initialize();
	if(rv<0){
		back_trace();
		cxd3774gf_codec_remove(codec);
		return(rv);
	}
#endif
	rv=cxd3774gf_timer_initialize();
	if(rv<0){
		back_trace();
		cxd3774gf_codec_remove(codec);
		return(rv);
	}

	rv=cxd3774gf_regmon_initialize();
	if(rv<0){
		back_trace();
		cxd3774gf_codec_remove(codec);
		return(rv);
	}

	snd_soc_add_codec_controls(
		codec,
		cxd3774gf_snd_controls,
		ARRAY_SIZE(cxd3774gf_snd_controls)
	);

	initialized=TRUE;

	return(0);
}

static int cxd3774gf_codec_remove(struct snd_soc_codec * codec)
{
	struct cxd3774gf_driver_data * driver_data;
	struct cxd3774gf_platform_data * platform_data;

	print_trace("%s()\n",__FUNCTION__);

	initialized=FALSE;

	driver_data=dev_get_drvdata(codec->dev);
	platform_data=dev_get_platdata(codec->dev);

	cxd3774gf_regmon_finalize();
	cxd3774gf_timer_finalize();
	cxd3774gf_interrupt_finalize();
	cxd3774gf_core_finalize();
	cxd3774gf_volume_finalize();
	cxd3774gf_table_finalize();
	cxd3774gf_switch_finalize();
	cxd3774gf_register_finalize();
	cxd3774gf_reset_platform();

	wake_lock_destroy(&driver_data->wake_lock);

	return(0);
}

/******************************/
/*@ power_management_routines */
/******************************/

static int cxd3774gf_i2c_suspend(struct device * device)
{
	struct cxd3774gf_driver_data * driver_data;

	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

	driver_data=dev_get_drvdata(device);

	cxd3774gf_suspend();

	return(0);
}

static int cxd3774gf_i2c_resume(struct device * device)
{
	struct cxd3774gf_driver_data * driver_data;

	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

	driver_data=dev_get_drvdata(device);

	cxd3774gf_resume();

	return(0);
}

/************/
/*@ dai_ops */
/************/

static int cxd3774gf_icx_dai_startup(
	struct snd_pcm_substream * substream,
	struct snd_soc_dai       * dai
)
{
	struct cxd3774gf_driver_data * driver_data;
	int playback;
	int capture;

	print_trace("%s(%d)\n",__FUNCTION__,substream->stream);

	if(!initialized)
		return(0);

	driver_data=dev_get_drvdata(dai->dev);
	wake_lock_timeout(&driver_data->wake_lock, HZ * CXD3774GF_WAKE_LOCK_TIME);

	playback=FALSE;
	capture=FALSE;

	if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		playback=TRUE;
	}

	if(substream->stream == SNDRV_PCM_STREAM_CAPTURE){
		capture=TRUE;
	}

	cxd3774gf_startup(playback,FALSE,capture);

	return(0);
}

static int cxd3774gf_std_dai_startup(
	struct snd_pcm_substream * substream,
	struct snd_soc_dai       * dai
)
{
	struct cxd3774gf_driver_data * driver_data;
	int playback;
	int capture;

	print_trace("%s(%d)\n",__FUNCTION__,substream->stream);

	if(!initialized)
		return(0);

	driver_data=dev_get_drvdata(dai->dev);
	wake_lock_timeout(&driver_data->wake_lock, HZ * CXD3774GF_WAKE_LOCK_TIME);

	playback=FALSE;
	capture=FALSE;

	if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		playback=TRUE;
	}

	if(substream->stream == SNDRV_PCM_STREAM_CAPTURE){
		capture=TRUE;
	}

	cxd3774gf_startup(FALSE,playback,capture);

	return(0);
}

static void cxd3774gf_icx_dai_shutdown(
	struct snd_pcm_substream * substream,
	struct snd_soc_dai       * dai
)
{
	struct cxd3774gf_driver_data * driver_data;
	int playback;
	int capture;

	print_trace("%s(%d)\n",__FUNCTION__,substream->stream);

	if(!initialized)
		return;

	driver_data=dev_get_drvdata(dai->dev);
	wake_lock_timeout(&driver_data->wake_lock, HZ * CXD3774GF_WAKE_LOCK_TIME);

	playback=FALSE;
	capture=FALSE;

	if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		playback=TRUE;
	}

	if(substream->stream == SNDRV_PCM_STREAM_CAPTURE){
		capture=TRUE;
	}

	cxd3774gf_shutdown(playback,FALSE,capture);

	return;
}

static void cxd3774gf_std_dai_shutdown(
	struct snd_pcm_substream * substream,
	struct snd_soc_dai       * dai
)
{
	struct cxd3774gf_driver_data * driver_data;
	int playback;
	int capture;

	print_trace("%s(%d)\n",__FUNCTION__,substream->stream);

	if(!initialized)
		return;

	driver_data=dev_get_drvdata(dai->dev);
	wake_lock_timeout(&driver_data->wake_lock, HZ * CXD3774GF_WAKE_LOCK_TIME);

	playback=FALSE;
	capture=FALSE;

	if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		playback=TRUE;
	}

	if(substream->stream == SNDRV_PCM_STREAM_CAPTURE){
		capture=TRUE;
	}

	cxd3774gf_shutdown(FALSE,playback,capture);

	return;
}

static int cxd3774gf_icx_dai_set_fmt(
	struct snd_soc_dai * dai,
	unsigned int         format
)
{
	print_trace("%s(%08X)\n",__FUNCTION__,format);

	if(!initialized)
		return(0);

	if((format&SND_SOC_DAIFMT_FORMAT_MASK)!=SND_SOC_DAIFMT_I2S){
		print_error("format 0x%X is unsupported.\n",(format&SND_SOC_DAIFMT_FORMAT_MASK));
		return(-EINVAL);
	}

	if((format&SND_SOC_DAIFMT_INV_MASK)!=SND_SOC_DAIFMT_NB_NF){
		print_error("polarity 0x%X is unsupported.\n",(format&SND_SOC_DAIFMT_INV_MASK));
		return(-EINVAL);
	}

	if((format&SND_SOC_DAIFMT_MASTER_MASK)!=SND_SOC_DAIFMT_CBS_CFS){
		print_error("mode 0x%X is unsupported.\n",(format&SND_SOC_DAIFMT_MASTER_MASK));
		return(-EINVAL);
	}

	return(0);
}

static int cxd3774gf_std_dai_set_fmt(
	struct snd_soc_dai * dai,
	unsigned int         format
)
{
	print_trace("%s(%08X)\n",__FUNCTION__,format);

	if(!initialized)
		return(0);

	if((format&SND_SOC_DAIFMT_FORMAT_MASK)!=SND_SOC_DAIFMT_I2S){
		print_error("format 0x%X is unsupported.\n",(format&SND_SOC_DAIFMT_FORMAT_MASK));
		return(-EINVAL);
	}

	if((format&SND_SOC_DAIFMT_INV_MASK)!=SND_SOC_DAIFMT_NB_NF){
		print_error("polarity 0x%X is unsupported.\n",(format&SND_SOC_DAIFMT_INV_MASK));
		return(-EINVAL);
	}

	if((format&SND_SOC_DAIFMT_MASTER_MASK)!=SND_SOC_DAIFMT_CBS_CFS){
		print_error("mode 0x%X is unsupported.\n",(format&SND_SOC_DAIFMT_MASTER_MASK));
		return(-EINVAL);
	}

	return(0);
}

static int cxd3774gf_icx_dai_hw_params(
	struct snd_pcm_substream * substream,
	struct snd_pcm_hw_params * params,
	struct snd_soc_dai       * dai
)
{
	unsigned int rate;

	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

	rate=params_rate(params);

	cxd3774gf_set_icx_playback_dai_rate(rate);

	return(0);
}

static int cxd3774gf_std_dai_hw_params(
	struct snd_pcm_substream * substream,
	struct snd_pcm_hw_params * params,
	struct snd_soc_dai       * dai
)
{
	unsigned int rate;

	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

	rate=params_rate(params);

	if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		cxd3774gf_set_std_playback_dai_rate(rate);

	if(substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		cxd3774gf_set_capture_dai_rate(rate);

	return(0);
}

static int cxd3774gf_icx_dai_mute(struct snd_soc_dai * dai, int mute)
{
	print_trace("%s(%d)\n",__FUNCTION__,mute);

	if(!initialized)
		return(0);

	return(0) ;
}

static int cxd3774gf_std_dai_mute(struct snd_soc_dai * dai, int mute)
{
	print_trace("%s(%d)\n",__FUNCTION__,mute);

	if(!initialized)
		return(0);

	return(0) ;
}

/****************/
/* noise_cancel */
/****************/

/*@ noise_cancel_mode */

static int cxd3774gf_put_noise_cancel_mode_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_noise_cancel_mode(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_noise_cancel_mode_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_noise_cancel_mode(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ noise_cancel_status */

static int cxd3774gf_put_noise_cancel_status_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	return(0);
}

static int cxd3774gf_get_noise_cancel_status_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_noise_cancel_status(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ user_noise_cancel_gain */

static int cxd3774gf_put_user_noise_cancel_gain_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_user_noise_cancel_gain(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_user_noise_cancel_gain_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_user_noise_cancel_gain(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ base_noise_cancel_gain_left */

static int cxd3774gf_put_base_noise_cancel_gain_left_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	struct soc_mixer_control * control;
	unsigned int vall;
	unsigned int valr;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	control = (struct soc_mixer_control *)kcontrol->private_value;

	vall=ucontrol->value.integer.value[0];
	if(vall>control->max)
		vall=control->max;

	valr=-1;

	rv=cxd3774gf_put_base_noise_cancel_gain(vall,valr);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_base_noise_cancel_gain_left_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int vall=0;
	unsigned int valr=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_base_noise_cancel_gain(&vall,&valr);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=vall;

	return(0);
}

/*@ base_noise_cancel_gain_right */

static int cxd3774gf_put_base_noise_cancel_gain_right_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	struct soc_mixer_control * control;
	unsigned int vall;
	unsigned int valr;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	control = (struct soc_mixer_control *)kcontrol->private_value;

	vall=-1;

	valr=ucontrol->value.integer.value[0];
	if(valr>control->max)
		valr=control->max;

	rv=cxd3774gf_put_base_noise_cancel_gain(vall,valr);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_base_noise_cancel_gain_right_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int vall=0;
	unsigned int valr=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_base_noise_cancel_gain(&vall,&valr);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=valr;

	return(0);
}

/*@ base_noise_cancel_gain_save */

static int cxd3774gf_put_base_noise_cancel_gain_save_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_exit_base_noise_cancel_gain_adjustment(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_base_noise_cancel_gain_save_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	/* print_trace("%s()\n",__FUNCTION__); */

	ucontrol->value.integer.value[0]=0;

	return(0);
}

/****************/
/* sound_effect */
/****************/

/*@ sound_effect */

static int cxd3774gf_put_sound_effect_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_sound_effect(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_sound_effect_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_sound_effect(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*******************/
/* volume_and_mute */
/*******************/

/*@ master_volume */

static int cxd3774gf_put_master_volume_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_master_volume(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_master_volume_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_master_volume(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ master_gain */

static int cxd3774gf_put_master_gain_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_master_gain(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_master_gain_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_master_gain(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ playback_mute */

static int cxd3774gf_put_playback_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_playback_mute(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_playback_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_playback_mute(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ capture_mute */

static int cxd3774gf_put_capture_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_capture_mute(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_capture_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_capture_mute(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ analog_playback_mute */

static int cxd3774gf_put_analog_playback_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_analog_playback_mute(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_analog_playback_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_analog_playback_mute(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ analog_stream_mute */

static int cxd3774gf_put_analog_stream_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_analog_stream_mute(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_analog_stream_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_analog_stream_mute(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ timed_mute */

static int cxd3774gf_put_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_timed_mute(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_timed_mute(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ std_timed_mute */

static int cxd3774gf_put_std_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_std_timed_mute(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_std_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_std_timed_mute(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ icx_timed_mute */

static int cxd3774gf_put_icx_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_icx_timed_mute(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_icx_timed_mute_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_icx_timed_mute(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/******************/
/* device_control */
/******************/

/*@ output_device */

static int cxd3774gf_put_output_device_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_output_device(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_output_device_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_output_device(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ input_device */

static int cxd3774gf_put_input_device_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_input_device(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_input_device_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_input_device(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ headphone_amp */

static int cxd3774gf_put_headphone_amp_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_headphone_amp(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_headphone_amp_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_headphone_amp(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ headphone_type */

static int cxd3774gf_put_headphone_type_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_headphone_type(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_headphone_type_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_headphone_type(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ jack_mode */

static int cxd3774gf_put_jack_mode_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_jack_mode(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_jack_mode_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_jack_mode(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/*@ jack_status */

static int cxd3774gf_put_jack_status_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	return(0);
}

static int cxd3774gf_get_jack_status_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;

	/* print_trace("%s()\n",__FUNCTION__); */

	cxd3774gf_get_jack_status(&val);

	ucontrol->value.integer.value[0]=val;

	return(0);
}

/**************/
/* debug_test */
/**************/

/*@ debug_test */

static int cxd3774gf_put_debug_test_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val;
	int rv;

	print_trace("%s()\n",__FUNCTION__);

	val=ucontrol->value.integer.value[0];

	rv=cxd3774gf_put_debug_test(val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	return(0);
}

static int cxd3774gf_get_debug_test_control(
	struct snd_kcontrol       * kcontrol,
	struct snd_ctl_elem_value * ucontrol
)
{
	unsigned int val=0;
	int rv;

	/* print_trace("%s()\n",__FUNCTION__); */

	rv=cxd3774gf_get_debug_test(&val);
	if(rv<0){
		back_trace();
		return(rv);
	}

	ucontrol->value.integer.value[0]=val;

	return(0);
}

