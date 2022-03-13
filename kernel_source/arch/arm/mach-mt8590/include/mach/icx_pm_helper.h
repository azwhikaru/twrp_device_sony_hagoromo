/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* ICX PM helper: ICX power management helper driver common header.
 * This is not hardware device driver. This helps userland
 * application to suspend, resume, on and off.
 *
 * Copyright 2015 Sony Corporation.
 * Author: Sony Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if (!defined(ICX_PM_HELPER_HEADER_INCLUDED))
#define ICX_PM_HELPER_HEADER_INCLUDED

#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>

#include "mach/eint.h"

/* boot reasons.
   @note Track this extesion with following files.
   + /bootable/bootloader/lk/platform/mt8590/include/platform/boot_mode.h
   + /bootable/bootloader/preloader/mt8590/src/drivers/inc/platform.h
*/
#define	ICX_PM_HELPER_BOOT_REASON_POWER_BUTTION	(0x00000001) /* Pressed power button. */
#define	ICX_PM_HELPER_BOOT_REASON_KERNEL_PARAM		icx_boot_reason
#define	ICX_PM_HELPER_BOOT_REASON_KERNEL_PARAM_STR	"icx_boot_reason"

#define	ICX_PM_HELPER_BOOT_OPTION_KERNEL_PARAM		icx_boot_option
#define	ICX_PM_HELPER_BOOT_OPTION_KERNEL_PARAM_STR	"icx_boot_option"

#define	ICX_PM_HELPER_BOOT_REASON_UNKNOWN	(-1)
#define	ICX_PM_HELPER_BOOT_OPTION_UNKNOWN	(-1L)
#define	ICX_PM_HELPER_BOOT_OPTION_NORMAL	( 0L)
#define	ICX_PM_HELPER_BOOT_OPTION_RECOVERY	( 1L)

extern int	icx_pm_helper_boot_reason;
extern long	icx_pm_helper_boot_option;

#define ICX_PM_HELPER_SYSINFO_KERNEL_PARAM      icx_sysinfo
#define ICX_PM_HELPER_SYSINFO_KERNEL_PARAM_STR  "icx_sysinfo"

#define ICX_PM_HELPER_MODELID_KERNEL_PARAM      icx_modelid
#define ICX_PM_HELPER_MODELID_KERNEL_PARAM_STR  "icx_modelid"

extern ulong icx_pm_helper_modelid;

/*! Copy of SPM and ISR status register. */
struct icx_pm_helper_spm_stat {
	spinlock_t	lock;		/*!< read/write spinlock. */
	uint32_t	debug_reg;	/*!< PCM_REG_DATA_INI PCM register data for debug?	*/
	uint32_t	r12;		/*!< PCM_REG12_DAT AWake reason bit maps.		*/
	uint32_t	raw_sta;	/*!< SLEEP_ISR_RAW_STA */
	uint32_t	cpu_wake;	/*!< SLEEP_CPU_WAKEUP_EVENT */
	uint32_t	timer_out;	/*!< PCM_TIMER_OUT */
	uint32_t	event_reg;	/*!< PCM_EVENT_REG_STA */
	uint32_t	isr;		/*!< SLEEP_ISR_STATUS */
	uint32_t	r13;		/*!< PCM_REG13_DATA */
	uint32_t	eint_sta[(EINT_MAX_CHANNEL + 31) / 32];	/*!< EINT status at resume. */
	unsigned long long	resume_count;	/*!< Resume counter. */
};


/* declared in icx_power.c */
extern struct icx_pm_helper_spm_stat	icx_pm_helper_spm_stat;
extern struct semaphore	icx_pm_helper_global_sem;

#ifdef CONFIG_USB_HOST_OC_DETECT
#define OC_DETECT_PIN_GPIO  (GPIO_USB_HOST1_OC_EINT_PIN & ~(0x80000000))
#define OC_DETECT_PIN_EINT  CUST_EINT_USB_OC_HOST1_NUM
extern void overcurrent_event_notify(void);
extern void overcurrent_end_processing(void);
extern void overcurrent_start_processing(void);
#endif /* CONFIG_USB_HOST_OC_DETECT */

#endif /* (!defined(ICX_PM_HELPER_HEADER_INCLUDED)) */
