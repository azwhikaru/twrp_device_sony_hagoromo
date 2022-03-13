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
#ifndef __BACKPORT_LNIUX_JIFFIES_H
#define __BACKPORT_LNIUX_JIFFIES_H
#include_next <linux/jiffies.h>

#ifndef time_is_before_jiffies
#define time_is_before_jiffies(a) time_after(jiffies, a)
#endif

#ifndef time_is_after_jiffies
#define time_is_after_jiffies(a) time_before(jiffies, a)
#endif

#ifndef time_is_before_eq_jiffies
#define time_is_before_eq_jiffies(a) time_after_eq(jiffies, a)
#endif

#ifndef time_is_after_eq_jiffies
#define time_is_after_eq_jiffies(a) time_before_eq(jiffies, a)
#endif

#endif /* __BACKPORT_LNIUX_JIFFIES_H */
