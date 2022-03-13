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
#ifndef _WMT_TM_H
#define _WMT_TM_H
#if WMT_PLAT_ALPS

#define CONFIG_THERMAL_OPEN
#if  defined(CONFIG_THERMAL) &&  defined(CONFIG_THERMAL_OPEN)

struct wmt_thermal_ctrl_ops {
	INT32 (*query_temp)(VOID);
	INT32 (*set_temp)(INT32);
};

INT32 wmt_tm_init(struct wmt_thermal_ctrl_ops *ops);
INT32 wmt_tm_deinit(VOID);
INT32 wmt_tm_init_rt(VOID);
INT32 wmt_tm_deinit_rt(VOID);
#endif

#endif
#endif
