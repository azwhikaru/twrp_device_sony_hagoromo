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
#ifndef _MT_PMIC_COMMON_H_
#define _MT_PMIC_COMMON_H_

#ifdef MTK_PMIC_MT6397
#include <mach/upmu_common_mt6397.h>
#else
#include <mach/upmu_common_mt6323.h>
#endif

#endif //_MT_PMIC_COMMON_H
