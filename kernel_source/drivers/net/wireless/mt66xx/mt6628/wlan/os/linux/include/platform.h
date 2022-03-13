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
/* add platform.h to avoid extern in  c files */

#ifndef _GL_PLATFORM_H
#define _GL_PLATFORM_H

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "gl_typedef.h"

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
extern BOOLEAN fgIsUnderEarlierSuspend;

extern struct semaphore g_halt_sem;
extern int g_u4HaltFlag;

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
void wlanRegisterNotifier(void);
void wlanUnregisterNotifier(void);

#if defined(CONFIG_HAS_EARLYSUSPEND)
int glRegisterEarlySuspend(struct early_suspend *prDesc,
				  early_suspend_callback wlanSuspend,
				  late_resume_callback wlanResume);
int glUnregisterEarlySuspend(struct early_suspend *prDesc);
#endif

typedef int (*set_p2p_mode) (struct net_device *netdev, PARAM_CUSTOM_P2P_SET_STRUC_T p2pmode);
typedef void (*set_dbg_level) (unsigned char modules[DBG_MODULE_NUM]);

void register_set_p2p_mode_handler(set_p2p_mode handler);
void register_set_dbg_level_handler(set_dbg_level handler);

#endif					/* _GL_PLATFORM_H */
