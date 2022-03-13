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
#include "x_os.h"


extern INT32 mem_init(VOID *pv_addr, SIZE_T z_size, VOID *pv_ch2_addr, SIZE_T z_ch2_size);
extern INT32 os_thread_init(VOID);
extern INT32 os_timer_init(VOID);
extern INT32 os_sema_init(VOID);
extern INT32 msg_q_init(VOID);
extern INT32 ev_grp_init(VOID);
extern INT32 isr_init(VOID);


INT32
os_init(VOID *pv_addr, SIZE_T z_size, VOID *pv_ch2_addr, SIZE_T z_ch2_size)
{
    INT32 i4_i;

    i4_i = mem_init(pv_addr, z_size, pv_ch2_addr, z_ch2_size);
    if (i4_i != OSR_OK)
    {
        return i4_i;
    }
    i4_i = os_thread_init();
    if (i4_i != OSR_OK)
    {
        return i4_i;
    }
    i4_i = os_timer_init();
    if (i4_i != OSR_OK)
    {
        return i4_i;
    }
    i4_i = os_sema_init();
    if (i4_i != OSR_OK)
    {
        return i4_i;
    }
    i4_i = msg_q_init();
    if (i4_i != OSR_OK)
    {
        return i4_i;
    }
    i4_i = ev_grp_init();
    if (i4_i != OSR_OK)
    {
        return i4_i;
    }
    /*
    i4_i = isr_init();
    if (i4_i != OSR_OK)
    {
        return i4_i;
    }
    */
    return OSR_OK;
}

