/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
* Copyright (C) 2011-2015 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#include <sound/cxd3774gf.h>
#include <sound/cxd3776er.h>
#include <sound/cxd3778gf.h>
#include <sound/cxd90020.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE
/************************/
/*@ codec_platform_data */
/************************/

static struct cxd3774gf_platform_data cxd3774gf_platform_data = {
	.regulator_180     = "",
	.regulator_285     = "",
	.port_ada_ldo_en   =  -1,          /*                    */
	.port_ada_xint     = 113,          /* MSDC0_DAT5         */
	.port_sp_xshtd     =  -1,          /*                    */
	.port_xpcm_det     = 118,          /* MSDC0_DAT3         */
	.port_ada_xrst     = 112,          /* MSDC0_DAT6         */
	.port_hp_xmute     =  -1,          /*                    */
	.port_au_pwr2_en   = 102,          /* SPI2_MI            */
	.port_au_pwr3_en   = 103,          /* SPI2_MO            */
	.port_au_pwr5_en   = 117,          /* MSDC0_CLK          */
#ifdef ICX_ENABLE_NC
	.port_nc_cmpout    =  -1,          /* (EXT_XCS)          */
	.port_nc_det       =  -1,          /* (EXT_SCK)          */
	.port_hp_det       = 111,          /* MSDC0_DAT7         */
	.port_hp_xmute3    = 114,          /* MSDC0_DAT4         */
	.port_ada_osc_en   =  -1,          /*                    */
#else
	.port_nc_cmpout    =  -1,          /*                    */
	.port_nc_det       =  -1,          /*                    */
	.port_hp_det       = 111,          /* MCDC0_DAT7         */
	.port_hp_xmute3    = 114,          /* MSDC0_DAT4         */
	.port_ada_osc_en   =  -1,          /*                    */
#endif
#ifdef ICX_ENABLE_AU2CLK
	.port_hp_xmute4    =  -1,          /*                    */
	.port_mclk_441_en  =  38,          /* I2S2_DATA          */
#else
	.port_hp_xmute4    =  -1,          /*                    */
	.port_mclk_441_en  =  38,          /* I2S2_DATA          */
#endif
};

/***********************/
/*@ damp_platform_data */
/***********************/

static struct cxd90020_platform_data cxd90020_platform_data = {
        .port_tbsamp_xint   = 120,          /* MSDC0_DAT1         */
        .port_tbsamp_xrst   = 119,          /* MSDC0_DAT2         */
        .port_hp_xmute2     = 121,          /* MSDC0_DAT0         */
        .port_hp_mute4      =  -1,          /*                    */
#ifdef ICX_ENABLE_NC
        .port_vg_xon        =  -1,          /*                    */
        .port_vldo_xshunt   =  -1,          /*                    */
#else
        .port_vg_xon        =  -1,          /*                    */
        .port_vldo_xshunt   =  58,          /* SPL1_CLK           */
#endif
#ifdef ICX_ENABLE_AU2CLK
        .port_tbsamp_pwr_on =  -1,          /*                    */
#else
        .port_tbsamp_pwr_on =  -1,          /*                    */
#endif
};
#endif

#ifdef CONFIG_SND_SOC_ICX_AUDIO_STATIONARY
static struct cxd3776er_platform_data cxd3776er_platform_data = {
        .regulator_330     =  "",
        .regulator_285     =  "",
	.port_i2s0_data_out  = 49,
	.port_i2s0_data_in   = 72,
	.port_i2s0_data_lrck = 73,
	.port_i2s0_data_bck  = 74,
	.port_i2s1_data_out  = 33,
	.port_i2s1_data_in   = 34,
	.port_i2s1_data_bck  = 35,
	.port_i2s1_data_lrck = 36,
        .port_ada_ldo_en   =  -1,          /*                    */
        .port_ada_xint     = 242,          /* URTS2              */
        .port_sp_xshtd     =  -1,          /*                    */
        .port_ada_xrst     =  56,          /* SPI0_MO            */
        .port_mclk_441_en  =  -1,          /*                    */
};
#endif

#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
static struct cxd3778gf_platform_data cxd3778gf_platform_data = {
        .regulator_180     = "",
        .regulator_285     = "",
	.port_i2s0_data_out  = 49,
	.port_i2s0_data_in   = 72,
	.port_i2s0_data_lrck = 73,
	.port_i2s0_data_bck  = 74,
	.port_i2s1_data_out  = 33,
	.port_i2s1_data_in   = 34,
	.port_i2s1_data_bck  = 35,
	.port_i2s1_data_lrck = 36,
        .port_ada_ldo_en   =  19,          /* PCM_SYNC           */
        .port_ada_xint     =  55,          /* SPI0_MI            */
        .port_se_xshtd     = 126,          /* I2S0_MCLK          */
        .port_ada_xdet     =  52,          /* I2S2_LRCK          */
        .port_ada_xrst     = 201,          /* SPDIF_IN0          */
        .port_au_vl_en     = 120,
	.port_hp_det       =  53,          /* SPI0_CSN           */
	.port_nc_det       =  -1,
        .port_au_avdd_en   =   8,          /* SPI1_MI            */
        .port_au_se_pos_en = 209,          /* EXT_CK2            */
        .port_au_se_neg_en =  48,          /* SPI1_MO            */
        .port_au_se_3v_en  = 199,          /* SPI1_CLK           */
        .port_hp_xmute4    =  -1,          /*                    */
	.port_ext_ck1      = 208,          /*                    */
        .port_hp_fm_series_xmute = 102, /* SPI2_MI            */
        .port_hp_xmute = 7, /* SPI1_CSN           */
        .port_au_btl_5v_en_2 = 9,
        .port_au_btl_5v_en = 188, /* I2S2_MCLK          */
        .port_au_btl_7v_en = 102, /* SPI2_MI            */
        .port_btl_xshtd = 37, /* I2S1_MCLK          */
        .port_hp_se_mute_on = 7, /* SPI1_CSN           */
        .port_hp_se_mute_off = 54, /* SPI0_CK            */
	.port_hp_se_mute_cp_on = 54, /* SPI0_CK            */
        .port_hp_btl_mute_on = 50, /* I2S2_BCK           */
        .port_hp_btl_mute_off = 56, /* SPI0_MO            */
        .port_hp_btl_det_l = 42, /* JTDO               */
        .port_hp_btl_det_r = 39, /* JTMS               */
        .port_ada_fs441_en = 117,          /* MCLK0_CLK          */
        .port_ada_fs480_en =  51,          /* I2S2_DATA_IN       */
};

#endif

static struct i2c_board_info __initdata mt_audio_i2c_devs_info[] = {
    {
        I2C_BOARD_INFO("pcm1795", (0x98>>1)),
    },
};

#ifdef CONFIG_SND_SOC_ICX_BOARD
static struct i2c_board_info __initdata icx_audio_i2c_devs_info[] = {
#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE
        {
                I2C_BOARD_INFO(CXD3774GF_DEVICE_NAME, 0x4E),
                .platform_data = &cxd3774gf_platform_data,
        },
        {
                I2C_BOARD_INFO(CXD90020_DEVICE_NAME,  0x4A),
                .platform_data = &cxd90020_platform_data,
        },
#endif
#ifdef CONFIG_SND_SOC_ICX_AUDIO_STATIONARY
        {
                I2C_BOARD_INFO(CXD3776ER_DEVICE_NAME, 0x1E),
                .platform_data = &cxd3776er_platform_data,
        },
#endif
#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
        {
                I2C_BOARD_INFO(CXD3778GF_DEVICE_NAME, 0x4E),
                .platform_data = &cxd3778gf_platform_data,
        },
#endif

};
#endif

static int __init mt_audio_i2c_devs_init(void)
{
    printk("pcm1795 %s()\n", __func__);    
    int err;

    err = i2c_register_board_info(0, mt_audio_i2c_devs_info, ARRAY_SIZE(mt_audio_i2c_devs_info));//1->bx8590p1_emmc 0->bx8590m2_emmc

    if (err) {
        printk("%s() call i2c_register_board_info() fail:%d\n", __func__, err);
    }

#ifdef CONFIG_SND_SOC_ICX_BOARD
#ifdef CONFIG_SND_SOC_ICX_AUDIO_MOBILE
    err = i2c_register_board_info(1, icx_audio_i2c_devs_info, ARRAY_SIZE(icx_audio_i2c_devs_info));
#elif CONFIG_SND_SOC_ICX_AUDIO_MOBILE_NEXT
    err = i2c_register_board_info(2, icx_audio_i2c_devs_info, ARRAY_SIZE(icx_audio_i2c_devs_info));
#else
    err = i2c_register_board_info(0, icx_audio_i2c_devs_info, ARRAY_SIZE(icx_audio_i2c_devs_info));
#endif
    if (err) {
        printk("%s() call icx i2c_register_board_info() fail:%d\n", __func__, err);
    }
#endif
    return 0;
}
arch_initcall(mt_audio_i2c_devs_init);
