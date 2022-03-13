/*
 * Copyright 2016 Sony Corporation
 * File Changed on 2016-10-17
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

#ifndef _MT8590_AUDRTC_H_
#define _MT8590_AUDRTC_H_

#include "mt8590-afe.h"

#define TARGET_ACC_RESULT_OFFSET (24)

enum target_acc_result_e {
    TARGET_ACC_IDLE   = 0,
    TARGET_ACC_COMES  = 1,
    TARGET_ACC_OK     = 2,
    TARGET_ACC_FAILED = 0x80
};

extern volatile int target_acc_result;
extern volatile enum audio_irq_id target_acc_irq_id;

#endif

