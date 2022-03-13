/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#ifndef _MT8590_DAI_H_
#define _MT8590_DAI_H_

#define MT8590_DAI_NONE              0
#define MT8590_DAI_I2S1_ID           1
#define MT8590_DAI_I2S2_ID           2
#define MT8590_DAI_I2S3_ID           3
#define MT8590_DAI_I2S4_ID           4
#define MT8590_DAI_I2S5_ID           5
#define MT8590_DAI_I2S6_ID           6
#define MT8590_DAI_IMMSND_ID         7
#define MT8590_DAI_I2SM_ID           8
#define MT8590_DAI_SPDIF_OUT_ID      9
#define MT8590_DAI_MULTI_IN_ID      10
#define MT8590_DAI_HDMI_OUT_I2S_ID  11
#define MT8590_DAI_HDMI_OUT_IEC_ID  12
#define MT8590_DAI_BTPCM_ID         13
#define MT8590_DAI_DSDENC_ID        14
#define MT8590_DAI_DMIC1_ID         15
#define MT8590_DAI_DMIC2_ID         16
#define MT8590_DAI_MRGIF_I2S_ID     17
#define MT8590_DAI_MRGIF_BT_ID      18
#define MT8590_DAI_DSDENC_RECORD_ID 19
#define MT8590_DAI_NUM              20

/*
 * Please define I2SM_NUM to one of
 * 1 (2ch MT8590_DAI_I2S1_ID),
 * 2 (4ch MT8590_DAI_I2S1+2_ID),
 * 3 (6ch MT8590_DAI_I2S1+2+3_ID),
 * 4 (8ch MT8590_DAI_I2S1+2+3+4_ID),
 * and 5 (10ch MT8590_DAI_I2S1+2+3+4+5_ID).
 */
#define I2SM_NUM  2
/*
 * Please define I2SM_AUXILIARY_STEREO_ID to one of
 * [MT8590_DAI_I2S1_ID+I2SM_NUM ... MT8590_DAI_I2S6_ID].
 */
#define I2SM_AUXILIARY_STEREO_ID  MT8590_DAI_I2S4_ID

#if (I2SM_AUXILIARY_STEREO_ID > MT8590_DAI_I2S6_ID) \
 || (I2SM_AUXILIARY_STEREO_ID < MT8590_DAI_I2S1_ID + I2SM_NUM)
#error Invalid I2SM_AUXILIARY_STEREO_ID. Please fix it.
#endif

#define DIV_ID_MCLK_TO_BCK  (0)
#define DIV_ID_BCK_TO_LRCK  (1)

#define SLAVE_USE_ASRC_MASK  (1U<<31)
#define SLAVE_USE_ASRC_YES   (1U<<31)
#define SLAVE_USE_ASRC_NO    (0U<<31)

#endif
