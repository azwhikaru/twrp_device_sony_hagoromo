/*
 * Copyright 2016 Sony Corporation
 * File Changed on 2016-10-17
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
#ifndef _MT_IDLE_H
#define _MT_IDLE_H

extern void enable_dpidle_by_bit(int id);
extern void disable_dpidle_by_bit(int id);

enum {
        IDLE_TYPE_SO = 0,
        IDLE_TYPE_DP = 1,
        IDLE_TYPE_SL = 2,
        IDLE_TYPE_RG = 3,
        NR_TYPES = 4,
};
extern int idle_switch[NR_TYPES];

#ifdef CONFIG_ARCH_MT8590_ICX
#define DPIDLE_CAN_ENTER_EXTRA_STATUS

extern void register_dpidle_key_status_func(int (*func)(void));
#endif

#ifdef __MT_IDLE_C__

/* idle task driver internal use only */

#if EN_PTP_OD
extern u32 ptp_data[3];
#endif

#ifdef SPM_SODI_ENABLED
extern u32 gSPM_SODI_EN;
extern bool gSpm_IsLcmVideoMode;
#endif

extern unsigned long localtimer_get_counter(void);
extern int localtimer_set_next_event(unsigned long evt);
#endif /* __MT_IDLE_C__ */


#endif
