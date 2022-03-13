/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * mt8590-afe-regmon.c
 *
 * regmon debug driver
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



#ifdef CONFIG_REGMON_DEBUG
#include <linux/module.h>
#include <sound/soc.h>
#include <linux/debugfs.h>
#include <mach/upmu_hw.h>
#include "mt8590-dai.h"
#include "mt8590-afe.h"
#include "mt8590-private.h"
#include "mt8590-afe-reg.h"
#include <mach/mt_clkmgr.h>
#include <mach/regmon.h>

static regmon_reg_info_t afe_regmon_reg_info[] =
{
	{ "CLK_AUDDIV_0",  CLK_AUDDIV_0 },
	{ "CLK_AUDDIV_1",  CLK_AUDDIV_1 },
	{ "CLK_AUDDIV_2",  CLK_AUDDIV_2 },
	{ "CLK_AUDDIV_3",  CLK_AUDDIV_3 },
	{ "FPGA_CON",  FPGA_CON },
	{ "AUDIO_TOP_CON0", AUDIO_TOP_CON0 },
	{ "AUDIO_TOP_CON1", AUDIO_TOP_CON1 },
	{ "AUDIO_TOP_CON2", AUDIO_TOP_CON2 },
	{ "AUDIO_TOP_CON3", AUDIO_TOP_CON3 },
	{ "AUDIO_TOP_CON4", AUDIO_TOP_CON4 },
	{ "AFE_DAC_CON0",  AFE_DAC_CON0 },
	{ "AFE_DAC_CON1",  AFE_DAC_CON1 },
	{ "AFE_DAC_CON2",  AFE_DAC_CON2 },
	{ "AFE_DAC_CON3",  AFE_DAC_CON3 },
	{ "AFE_DAC_CON4",  AFE_DAC_CON4 },
	{ "AFE_DAC_CON5",  AFE_DAC_CON5 },
	{ "AFE_DAIBT_CON0", AFE_DAIBT_CON0 },
	{ "AFE_MRGIF_CON",  AFE_MRGIF_CON },
	{ "AFE_CONN0",  AFE_CONN0 },
	{ "AFE_CONN1",  AFE_CONN1 },
	{ "AFE_CONN2",  AFE_CONN2 },
	{ "AFE_CONN3",  AFE_CONN3 },
	{ "AFE_CONN4",  AFE_CONN4 },
	{ "AFE_CONN33",  AFE_CONN33 },
	{ "AFE_CONN34",  AFE_CONN34 },
	{ "AFE_CONN35",  AFE_CONN35 },
	{ "AFE_CONN40",  AFE_CONN40 },
	{ "AFE_CONN41",  AFE_CONN41 },
	{ "ASYS_TOP_CON", ASYS_TOP_CON },
	{ "ASYS_I2SIN1_CON",  ASYS_I2SIN1_CON },
	{ "ASYS_I2SIN2_CON",  ASYS_I2SIN1_CON + 4 },
	{ "ASYS_I2SIN3_CON",  ASYS_I2SIN1_CON + 8 },
	{ "ASYS_I2SIN4_CON",  ASYS_I2SIN1_CON + 12 },
	{ "ASYS_I2SIN5_CON",  ASYS_I2SIN1_CON + 16 },
	{ "ASYS_I2SIN6_CON",  ASYS_I2SIN1_CON + 20 },
	{ "ASYS_I2SO1_CON",  ASYS_I2SO1_CON },
	{ "ASYS_I2SO2_CON",  ASYS_I2SO1_CON + 4 },
	{ "ASYS_I2SO3_CON",  ASYS_I2SO1_CON + 8 },
	{ "ASYS_I2SO4_CON",  ASYS_I2SO1_CON + 12 },
	{ "ASYS_I2SO5_CON",  ASYS_I2SO1_CON + 16 },
	{ "ASYS_I2SO6_CON",  ASYS_I2SO1_CON + 20 },
	{ "AFE_MEMIF_HD_CON0", AFE_MEMIF_HD_CON0 },
	{ "AFE_MEMIF_HD_CON1", AFE_MEMIF_HD_CON1 },
	{ "MASM_TRAC_CON1", MASM_TRAC_CON1 },
	{ "AFE_DL1_BASE",  AFE_DL1_BASE },
	{ "AFE_DL1_CUR",  AFE_DL1_CUR },
	{ "AFE_DL1_END",  AFE_DL1_END },
	{ "AFE_DL2_BASE",  AFE_DL2_BASE },
	{ "AFE_DL2_CUR",  AFE_DL2_CUR },
	{ "AFE_DL2_END",  AFE_DL2_END },
	{ "AFE_DL3_BASE",  AFE_DL3_BASE },
	{ "AFE_DL3_CUR",  AFE_DL3_CUR },
	{ "AFE_DL3_END",  AFE_DL3_END },
	{ "AFE_DL4_BASE",  AFE_DL4_BASE },
	{ "AFE_DL4_CUR",  AFE_DL4_CUR },
	{ "AFE_DL4_END",  AFE_DL4_END },
	{ "AFE_DL5_BASE",  AFE_DL5_BASE },
	{ "AFE_DL5_CUR",  AFE_DL5_CUR },
	{ "AFE_DL5_END",  AFE_DL5_END },
	{ "AFE_DL6_BASE",  AFE_DL6_BASE },
	{ "AFE_DL6_CUR",  AFE_DL6_CUR },
	{ "AFE_DL6_END",  AFE_DL6_END },
	{ "AFE_ARB1_BASE",  AFE_ARB1_BASE },
	{ "AFE_ARB1_CUR",  AFE_ARB1_CUR },
	{ "AFE_ARB1_END",  AFE_ARB1_END },
	{ "AFE_DLMCH_BASE",  AFE_DLMCH_BASE },
	{ "AFE_DLMCH_CUR",  AFE_DLMCH_CUR },
	{ "AFE_DLMCH_END",  AFE_DLMCH_END },
	{ "AFE_DSDR_BASE",  AFE_DSDR_BASE },
	{ "AFE_DSDR_CUR",  AFE_DSDR_CUR },
	{ "AFE_DSDR_END",  AFE_DSDR_END },
	{ "AFE_AWB_BASE",  AFE_AWB_BASE },
	{ "AFE_AWB_CUR",  AFE_AWB_CUR },
	{ "AFE_AWB_END",  AFE_AWB_END },
	{ "AFE_AWB2_BASE",  AFE_AWB2_BASE },
	{ "AFE_AWB2_CUR",  AFE_AWB2_CUR },
	{ "AFE_AWB2_END",  AFE_AWB2_END },
	{ "AFE_DSDW_BASE",  AFE_DSDW_BASE },
	{ "AFE_DSDW_CUR",  AFE_DSDW_CUR },
	{ "AFE_DSDW_END",  AFE_DSDW_END },
	{ "AFE_VUL_BASE",  AFE_VUL_BASE },
	{ "AFE_VUL_CUR",  AFE_VUL_CUR },
	{ "AFE_VUL_END",  AFE_VUL_END },
	{ "AFE_UL2_BASE",  AFE_UL2_BASE },
	{ "AFE_UL2_CUR",  AFE_UL2_CUR },
	{ "AFE_UL2_END",  AFE_UL2_END },
	{ "AFE_UL3_BASE",  AFE_UL3_BASE },
	{ "AFE_UL3_CUR",  AFE_UL3_CUR },
	{ "AFE_UL3_END",  AFE_UL3_END },
	{ "AFE_UL4_BASE",  AFE_UL4_BASE },
	{ "AFE_UL4_CUR",  AFE_UL4_CUR },
	{ "AFE_UL4_END",  AFE_UL4_END },
	{ "AFE_UL5_BASE",  AFE_UL5_BASE },
	{ "AFE_UL5_CUR",  AFE_UL5_CUR },
	{ "AFE_UL5_END",  AFE_UL5_END },
	{ "AFE_UL6_BASE",  AFE_UL6_BASE },
	{ "AFE_UL6_CUR",  AFE_UL6_CUR },
	{ "AFE_UL6_END",  AFE_UL6_END },
	{ "AFE_DAI_BASE",  AFE_DAI_BASE },
	{ "AFE_DAI_CUR",  AFE_DAI_CUR },
	{ "AFE_DAI_END",  AFE_DAI_END },
	{ "AFE_DMA_BASE",  AFE_DMA_BASE },
	{ "AFE_DMA_CUR",  AFE_DMA_CUR },
	{ "AFE_DMA_END",  AFE_DMA_END },
	{ "AFE_DMA_WR_CUR", AFE_DMA_WR_CUR },
	{ "AFE_DMA_CON0",  AFE_DMA_CON0 },
	{ "AFE_DMA_THRESHOLD",  AFE_DMA_THRESHOLD },
	{ "AFE_DMA_INTR_SIZE",  AFE_DMA_INTR_SIZE },
	{ "AFE_DMA_NEXT_INTR",  AFE_DMA_NEXT_INTR },
	{ "AFE_DMA_NEXT_BASE",  AFE_DMA_NEXT_BASE },
	{ "AFE_DMA_NEXT_END",  AFE_DMA_NEXT_END },
	{ "AFE_MEMIF_MON0",  AFE_MEMIF_MON0 },
	{ "AFE_MEMIF_MON1",  AFE_MEMIF_MON1 },
	{ "AFE_MEMIF_MON2",  AFE_MEMIF_MON2 },
	{ "AFE_MEMIF_MON3",  AFE_MEMIF_MON3 },
	{ "AFE_MEMIF_MON4",  AFE_MEMIF_MON4 },
	{ "AFE_ADDA_DL_SRC2_CON0",  AFE_ADDA_DL_SRC2_CON0 },
	{ "AFE_ADDA_DL_SRC2_CON1",  AFE_ADDA_DL_SRC2_CON1 },
	{ "AFE_ADDA_UL_SRC_CON0",  AFE_ADDA_UL_SRC_CON0 },
	{ "AFE_ADDA_UL_SRC_CON1",  AFE_ADDA_UL_SRC_CON1 },
	{ "AFE_ADDA_TOP_CON0",  AFE_ADDA_TOP_CON0 },
	{ "AFE_ADDA_UL_DL_CON0",  AFE_ADDA_UL_DL_CON0 },
	{ "AFE_ADDA_SRC_DEBUG",  AFE_ADDA_SRC_DEBUG },
	{ "AFE_ADDA_SRC_DEBUG_MON0",  AFE_ADDA_SRC_DEBUG_MON0 },
	{ "AFE_ADDA_SRC_DEBUG_MON1",  AFE_ADDA_SRC_DEBUG_MON1 },
	{ "AFE_ADDA_NEWIF_CFG0",  AFE_ADDA_NEWIF_CFG0 },
	{ "AFE_ADDA_NEWIF_CFG1",  AFE_ADDA_NEWIF_CFG1 },
	{ "ASMI_TIMING_CON1", ASMI_TIMING_CON1  },
	{ "ASMO_TIMING_CON1", ASMO_TIMING_CON1  },
	{ "AFE_DL_SDM_CON0",  AFE_DL_SDM_CON0 },
	{ "AFE_UL_SRC_0", AFE_UL_SRC_0  },
	{ "AFE_UL_SRC_1", AFE_UL_SRC_1  },
	{ "AFE_VAGC_CON1", AFE_VAGC_CON1  },
	{ "AFE_VAGC_CON19", AFE_VAGC_CON19  },
	{ "AFE_FOC_CON", AFE_FOC_CON  },
	{ "AFE_MON_STEP", AFE_MON_STEP  },
	{ "AFE_SGEN_CON0", AFE_SGEN_CON0  },
	{ "AFE_TOP_CON0", AFE_TOP_CON0  },
	{ "AFE_VAGC_CON0",AFE_VAGC_CON0   },
	{ "AFE_ADDA_PREDIS_CON0", AFE_ADDA_PREDIS_CON0  },
	{ "AFE_ADDA_PREDIS_CON1", AFE_ADDA_PREDIS_CON1  },
	{ "AFE_AGC_MON0", AFE_AGC_MON0  },
	{ "AFE_VAD_MON0", AFE_VAD_MON0  },
	{ "AFE_MOD_PCM_BASE", AFE_MOD_PCM_BASE  },
	{ "AFE_MOD_PCM_END",  AFE_MOD_PCM_END },
	{ "AFE_MOD_PCM_CUR",  AFE_MOD_PCM_CUR },
	{ "AFE_SPDIF2_OUT_CON0", AFE_SPDIF2_OUT_CON0 },
	{ "AFE_SPDIF2_BASE", AFE_SPDIF2_BASE },
	{ "AFE_SPDIF2_CUR", AFE_SPDIF2_CUR },
	{ "AFE_SPDIF2_END", AFE_SPDIF2_END },
	{ "AFE_HDMI_OUT_CON0", AFE_HDMI_OUT_CON0 },
	{ "AFE_HDMI_OUT_BASE",AFE_HDMI_OUT_BASE  },
	{ "AFE_HDMI_OUT_CUR", AFE_HDMI_OUT_CUR },
	{ "AFE_HDMI_OUT_END", AFE_HDMI_OUT_END },
	{ "AFE_SPDIF_OUT_CON0", AFE_SPDIF_OUT_CON0 },
	{ "AFE_SPDIF_BASE", AFE_SPDIF_BASE },
	{ "AFE_SPDIF_CUR", AFE_SPDIF_CUR },
	{ "AFE_SPDIF_END", AFE_SPDIF_END },
	{ "AFE_HDMI_CONN0", AFE_HDMI_CONN0 },
	{ "AFE_8CH_I2S_OUT_CON", AFE_8CH_I2S_OUT_CON },
	{ "AFE_HDMI_CONN1", AFE_HDMI_CONN1 },
	{ "AFE_IRQ_CON", AFE_IRQ_CON },
	{ "AFE_IRQ_STATUS", AFE_IRQ_STATUS },
	{ "AFE_IRQ_CLR", AFE_IRQ_CLR },
	{ "AFE_IRQ_CNT1", AFE_IRQ_CNT1 },
	{ "AFE_IRQ_CNT2", AFE_IRQ_CNT2 },
	{ "AFE_IRQ_MON2", AFE_IRQ_MON2 },
	{ "AFE_IRQ_CNT5", AFE_IRQ_CNT5 },
	{ "AFE_IRQ1_CNT_MON", AFE_IRQ1_CNT_MON },
	{ "AFE_IRQ2_CNT_MON", AFE_IRQ2_CNT_MON },
	{ "AFE_IRQ1_EN_CNT_MON", AFE_IRQ1_EN_CNT_MON },
	{ "ASYS_IRQ1_CON", ASYS_IRQ1_CON },
	{ "ASYS_IRQ2_CON", ASYS_IRQ2_CON },
	{ "ASYS_IRQ3_CON", ASYS_IRQ3_CON },
	{ "ASYS_IRQ4_CON", ASYS_IRQ4_CON },
	{ "ASYS_IRQ5_CON", ASYS_IRQ5_CON },
	{ "ASYS_IRQ6_CON", ASYS_IRQ6_CON },
	{ "ASYS_IRQ7_CON", ASYS_IRQ7_CON },
	{ "ASYS_IRQ_CLR",  ASYS_IRQ_CLR},
	{ "ASYS_IRQ_STATUS", ASYS_IRQ_STATUS },
	{ "PWR2_ASM_CON2", PWR2_ASM_CON2 },
	{ "AFE_MEMIF_MINLEN0", AFE_MEMIF_MINLEN0 },
	{ "AFE_MEMIF_MAXLEN0", AFE_MEMIF_MAXLEN0 },
	{ "AFE_MEMIF_MAXLEN1", AFE_MEMIF_MAXLEN1 },
	{ "AFE_MEMIF_MAXLEN2", AFE_MEMIF_MAXLEN2 },
	{ "AFE_IEC_PREFETCH_SIZE",AFE_IEC_PREFETCH_SIZE  },
	{ "AFE_GAIN1_CON0", AFE_GAIN1_CON0 },
	{ "AFE_GAIN1_CON1", AFE_GAIN1_CON1 },
	{ "AFE_GAIN1_CON2", AFE_GAIN1_CON2 },
	{ "AFE_GAIN1_CON3", AFE_GAIN1_CON3 },
	{ "AFE_GAIN1_CONN", AFE_GAIN1_CONN },
	{ "AFE_GAIN1_CUR",  AFE_GAIN1_CUR},
	{ "AFE_GAIN2_CON0", AFE_GAIN2_CON0 },
	{ "AFE_IEC_CFG", AFE_IEC_CFG },
	{ "AFE_IEC2_CFG", AFE_IEC2_CFG },
	{ "PWR2_TOP_CON", PWR2_TOP_CON },
	{ "DSD1_FADER_CON0", DSD1_FADER_CON0 },
	{ "DSD2_FADER_CON0", DSD2_FADER_CON0 },
	{ "AFE_MPHONE_MULTI_CON0", AFE_MPHONE_MULTI_CON0 },
	{ "AFE_MPHONE_MULTI_CON1", AFE_MPHONE_MULTI_CON1 },
	{ "PWR1_ASM_CON1",PWR1_ASM_CON1  },
	{ "AFE_ASRC_NEW_CON0", AFE_ASRC_NEW_CON0 },
	{ "DSD_ENC_CON0",DSD_ENC_CON0  },
	{ "DSD_ENC_CON1",DSD_ENC_CON1  },
	{ "DSD_ENC_CON2",DSD_ENC_CON2  },
	{ "AFE_MEMIF_PBUF_SIZE",AFE_MEMIF_PBUF_SIZE  },
	{ "AFE_ULMCH_BASE", AFE_ULMCH_BASE },
	{ "AFE_ULMCH_END", AFE_ULMCH_END },
	{ "AFE_ULMCH_CUR", AFE_ULMCH_CUR },
	{ "AFE_LRCK_CNT", AFE_LRCK_CNT},
	{ "PCM_INTF_CON1",PCM_INTF_CON1 },
	{ "PCM_INTF_CON2", PCM_INTF_CON2},
	{ "DMIC_TOP_CON", DMIC_TOP_CON},
	{ "DMIC_ULCF_CON1", DMIC_ULCF_CON1},
	{ "DMIC2_TOP_CON", DMIC2_TOP_CON},
	{ "DMIC2_ULCF_CON1", DMIC2_ULCF_CON1}
};

static int afe_regmon_read_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int * value
)
{
	*value=afe_read(address);

	return(0);
}

static int afe_regmon_write_reg(
	void         * private_data,
	unsigned int   address,
	unsigned int   value
)
{
	afe_write(address,value);

	return(0);
}


static regmon_customer_info_t afe_customer_info =
{
	.name           = "afe_reg",
	.reg_info       = afe_regmon_reg_info,
	.reg_info_count = sizeof(afe_regmon_reg_info)/sizeof(regmon_reg_info_t),
	.write_reg      = afe_regmon_write_reg,
	.read_reg       = afe_regmon_read_reg,
	.private_data   = NULL,
};

static int initialized = FALSE;

int afe_regmon_initialize(void)
{
	regmon_add(&afe_customer_info);

	initialized=TRUE;

	return(0);
}

int afe_regmon_finalize(void)
{
	if(!initialized)
		return(0);

	regmon_del(&afe_customer_info);

	initialized=FALSE;

	return(0);
}

MODULE_AUTHOR("Sony Corporation");
MODULE_DESCRIPTION("REGMON CODEC driver");
MODULE_LICENSE("GPL");
#endif

