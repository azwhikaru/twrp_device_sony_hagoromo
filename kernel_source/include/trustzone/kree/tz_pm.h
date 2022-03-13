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
/*
 * Power management for TrustZone
 */

#ifndef __KREE_TZ_PM_H__
#define __KREE_TZ_PM_H__

#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT

void kree_pm_init(void);
void kree_pm_cpu_lowpower(volatile int *ppen_release, int logical_cpuid);
int kree_pm_cpu_dormant(int mode);
int kree_pm_device_ops(int state);
int kree_pm_cpu_dormant_workaround_wake(int workaround_wake);

#else

#define kree_pm_cpu_lowpower(ppen, cpuid)
#define kree_pm_cpu_dormant(mode)    1

#endif				/* CONFIG_MTK_IN_HOUSE_TEE_SUPPORT */

#endif				/* __KREE_TZ_PM_H__ */
