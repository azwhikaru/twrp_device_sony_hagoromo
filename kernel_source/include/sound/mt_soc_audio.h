/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */
/*******************************************************************************

 *
 * Filename:
 * ---------
 *  mt_sco_stream_type.h
 *
 * Project:
 * --------
 *   Audio Driver Kernel Function
 *
 * Description:
 * ------------
 *   Audio stream type define
 *
 * Author:
 * -------
 * Chipeng Chang
 *
 *------------------------------------------------------------------------------
 * $Revision: #1 $
 * $Modtime:$
 * $Log:$
 *
 *
 *******************************************************************************/

#ifndef _MT_SOC_AUDIO_H_
#define _MT_SOC_AUDIO_H_

/* function to get if voice call exist */
bool get_voice_status(void);
int Get_Audio_Mode(void);
void audckbufEnable(bool enable);
/* function to notify vow irq */
void vow_irq_handler(void);

#endif
