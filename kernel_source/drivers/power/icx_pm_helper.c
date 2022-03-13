/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* ICX PM helper: ICX power management helper driver.
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

#include <asm/barrier.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <uapi/linux/stat.h>
#include <linux/hrtimer.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/wakelock.h>
#include <linux/suspend.h>
#include <linux/pm_runtime.h>
#include <linux/semaphore.h>
#include <mach/battery_common.h>
#include <mach/icx_pm_helper.h>

#include <mach/eint.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_core.h>
#include <linux/workqueue.h>
#include <linux/switch.h>

#include <cust_gpio_usage.h>
#include <cust_eint.h>

#define	STATIC_FUNC /* static */

#if (defined(__GNUC__))
#define	packed	__attribute__((__unused__))
#else
#define	packed
#endif

#if (defined(__GNUC__))
#define	may_unused	__attribute__((__unused__))
#else
#define	may_unused
#endif


/* Debug control flag bit masks.
   0x0001 1: Enable Light weight print, 0: Disable.
   0x0002 1: Enable Function entry/exit print, 0: Disable.
   0x0004 1: Enable Heavy weight print, 0: Disable.
*/

#define	DEBUG_FLAG_LIGHT	(0x00000001)	/* LI */
#define	DEBUG_FLAG_ENTRY	(0x00000002)	/* EN */
#define	DEBUG_FLAG_HEAVY	(0x00000004)	/* HV */

/*! Live debug control. */
static uint icx_pm_helper_debug=
	0x00000000
	| DEBUG_FLAG_LIGHT
	/* | DEBUG_FLAG_ENTRY */
	;

module_param_named(debug, icx_pm_helper_debug, uint, 0644);

#define	ICX_PM_HELPER_BOOT_X_UNKNOWN		(-1)

int	icx_pm_helper_boot_reason =	ICX_PM_HELPER_BOOT_REASON_UNKNOWN;
long	icx_pm_helper_boot_option =	ICX_PM_HELPER_BOOT_OPTION_UNKNOWN;

module_param_named(ICX_PM_HELPER_BOOT_REASON_KERNEL_PARAM, icx_pm_helper_boot_reason, int,  S_IWUSR | S_IRUGO);
module_param_named(ICX_PM_HELPER_BOOT_OPTION_KERNEL_PARAM, icx_pm_helper_boot_option, long, S_IWUSR | S_IRUGO);

EXPORT_SYMBOL_GPL(icx_pm_helper_boot_reason);
EXPORT_SYMBOL_GPL(icx_pm_helper_boot_option);

#ifdef CONFIG_USB_HOST_OC_DETECT
static const char *pm_usb_sw_name = "usb_overcurrent";
static struct switch_dev  oc_switch_usb;

enum switch_state {
  USB_OC_SWITCH_STATE_OVERCURRENT = 0,
  USB_OC_SWITCH_STATE_NORMAL      = 1
};

static struct delayed_work overcurrent_work;
static struct workqueue_struct * overcurrent_workqueue = NULL;

atomic_t oc_workq_flag;

#define OC_WORKQ_FLAG_ENABLED   1
#define OC_WORKQ_FLAG_DISABLED  0

#endif /* CONFIG_USB_HOST_OC_DETECT */

ulong icx_pm_helper_sysinfo = 0x0;
EXPORT_SYMBOL_GPL(icx_pm_helper_sysinfo);

module_param_named(ICX_PM_HELPER_SYSINFO_KERNEL_PARAM, icx_pm_helper_sysinfo, ulong, S_IWUSR | S_IRUGO );

ulong icx_pm_helper_modelid = 0x0;
EXPORT_SYMBOL_GPL(icx_pm_helper_modelid);

module_param_named(ICX_PM_HELPER_MODELID_KERNEL_PARAM, icx_pm_helper_modelid, ulong, S_IWUSR | S_IRUGO );

#if (!defined(CONFIG_ICX_PM_HELPER_RESUME_LOCK_MS))
#define CONFIG_ICX_PM_HELPER_RESUME_LOCK_MS	(20*1000)
#endif /* (!defined(CONFIG_ICX_PM_HELPER_RESUME_LOCK_MS)) */

/*! Debug:  Light weight printk.
    define: Do light weight printk.
    undef:  Do NOT light weight printk.
*/
#define	CONFIG_ICX_PM_HELPER_DEBUG_LIGHT

#if (defined(CONFIG_ICX_PM_HELPER_DEBUG_LIGHT))
#define	PRINTK_LI(format, ...) \
	do {\
		if (icx_pm_helper_debug&DEBUG_FLAG_LIGHT) {\
			printk(format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_LI(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug:  Do printk at function entry/exit.
    define: Do printk at function entry/exit.
    undef:  Do NOT printk at function entry/exit.
*/
#define	CONFIG_ICX_PM_HELPER_DEBUG_ENTRY

#if (defined(CONFIG_ICX_PM_HELPER_DEBUG_ENTRY))
#define	PRINTK_EN(format, ...) \
	do {\
		if (icx_pm_helper_debug&DEBUG_FLAG_ENTRY) {\
			printk(format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_EN(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug:  heavy weight printk.
    define: Do heavy weight printk.
    undef:  Do NOT heavy weight printk.
*/
/* #define	CONFIG_ICX_PM_HELPER_DEBUG_HEAVY */

#if (defined(CONFIG_ICX_PM_HELPER_DEBUG_HEAVY))
#define	PRINTK_HV(format, ...) \
	do {\
		if (icx_pm_helper_debug&DEBUG_FLAG_HEAVY) {\
			printk(format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_HV(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

const char this_driver_name[]="icx_pm_helper";

struct icx_pm_helper {
	struct device		*dev;			/*!< device pointer. */
	struct wake_lock	resume_wl;		/*!< wake_lock */
	struct notifier_block	pm_nb;			/*!< pm notifier */
	long			pm_state;		/*!< power management state. */
	long			resume_lock_ms;		/*!< resume wake lock time in milli seconds. */
	ktime_t			suspend_prepare_kt;	/*!< ktime at PM_SUSPEND_PREPARE */
	struct timespec		suspend_prepare_ts;	/*!< realtime at PM_SUSPEND_PREPARE */
	ktime_t			suspend_kt;		/*!< ktime at .suspend */
	struct timespec		suspend_ts;		/*!< real time at .suspend */
	ktime_t			post_suspend_kt;	/*!< ktime at PM_POST_SUSPEND */
	struct timespec		post_suspend_ts;	/*!< real time at PM_POST_SUSPEND */
	ktime_t			resume_kt;		/*!< ktime at .resume */
	struct timespec		resume_ts;		/*!< real time at .resume */
	int			boot_reset;		/*!< boot by reset. */
	int			boot_powerkey;		/*!< boot by power key. */
	int			boot_reboot;		/*!< boot by reboot. */
	int			boot_deadbat;		/*!< boot from deadbat. */
	int			boot_wdt;		/*!< boot by wdt. */
	int			boot_thermal;		/*!< boot by thermal. */
	long			boot_option;		/*!< boot option. */
};


/*! PM raise uevent at resume.
    @pre lock icx_pm_helper_global_sem.
*/
int icx_pm_helper_raise_resume_uevent(struct icx_pm_helper *pmh)
{	int		result;
	int		ret;
	struct device	*dev;

	enum resume_uevent_envp {
		RESUME_UEVENT_EVENT = 0,
		RESUME_UEVENT_NULL,
		RESUME_UEVENT_NUM,
	};

	const char event_resume[] =     "EVENT=resume";

#define	ALIGN_UP_N(a,n)	((((a)+(n)-1) / (n)) * (n))

	struct resume_uevent_envp_buf {
		char	event[ALIGN_UP_N((sizeof(event_resume)+sizeof(long)*3), sizeof(long))];
	};

	char					*envp[RESUME_UEVENT_NUM];
	struct resume_uevent_envp_buf		*buf;
	unsigned long				flags;
	struct icx_pm_helper_spm_stat		*sps;

	result = 0;
	dev = pmh->dev;
	if (dev == NULL) {
		/* Invalid device context. */
		pr_err("%s: Invalid device context.\n", __func__);
		result = -ENODEV;
		goto out;
	}

	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (!buf) {
		pr_err("%s: Not enough memory.\n",__func__);
		result = -ENOMEM;
		goto out;
	}

	envp[RESUME_UEVENT_EVENT] = &(buf->event[0]);
	envp[RESUME_UEVENT_NULL] =  NULL;

	snprintf(buf->event, sizeof(buf->event), event_resume);

	sps = &icx_pm_helper_spm_stat;
	spin_lock_irqsave(&(sps->lock), flags);
	sps->resume_count++;
	spin_unlock_irqrestore(&(sps->lock), flags);

	ret = kobject_uevent_env(&(dev->kobj), KOBJ_CHANGE, envp);
	if (ret != 0) {
		pr_err("%s: Fail to uevent.\n",__func__);
		result = ret;
		goto out_free;
	}

out_free:
	kfree(buf);
out:
	return result;
}


/*! PM notifier call back.
*/
int icx_pm_helper_notifier_call(struct notifier_block *nb, unsigned long event, may_unused void *ignore)
{	struct icx_pm_helper	*pmh;

	down(&icx_pm_helper_global_sem);
	pmh = container_of(nb, struct icx_pm_helper, pm_nb);
	pmh->pm_state = event;
	switch (event) {
		case PM_SUSPEND_PREPARE:
			getnstimeofday(&(pmh->suspend_prepare_ts));
			pmh->suspend_prepare_kt = ktime_get();
			break;
		case PM_POST_SUSPEND:
			getnstimeofday(&(pmh->post_suspend_ts));
			pmh->post_suspend_kt = ktime_get();
			icx_pm_helper_raise_resume_uevent(pmh);
			break;
		default:
			/* Do nothing. */
			break;
	}
	up(&icx_pm_helper_global_sem);
	return NOTIFY_OK;
}

/*! Template macro to define DEVICE_ATTR KT(ktime) show functions. */
#define	ICX_PM_HELPER_X_KT_SHOW(x_kt) \
STATIC_FUNC ssize_t icx_pm_helper_##x_kt##_show(struct device *dev, struct device_attribute *attr, char *buf) \
{	struct icx_pm_helper	*pmh;									\
	int			result;									\
	struct timespec		ts;									\
													\
	result = 0;											\
	down(&icx_pm_helper_global_sem);								\
	/*! @note dev_get_drvdata() returns the pointer passed to platform_set_drvdata(). */		\
	pmh = dev_get_drvdata(dev);									\
	if (pmh == NULL) {										\
		goto out;										\
	}												\
	/*! @note the suitable buffer size is shown in drivers/base/core.c. */				\
	/*! @note the ktime declared in s32, here treat ktime in unsigned long. */			\
	ts = ktime_to_timespec(pmh->x_kt);								\
	result = snprintf(buf, PAGE_SIZE, "%llu.%09lu\n",						\
		(unsigned long long)(ts.tv_sec),							\
		(unsigned long)     (ts.tv_nsec)							\
	);												\
out:													\
	up(&icx_pm_helper_global_sem);									\
	return result;											\
}

ICX_PM_HELPER_X_KT_SHOW(suspend_prepare_kt);
ICX_PM_HELPER_X_KT_SHOW(post_suspend_kt);
ICX_PM_HELPER_X_KT_SHOW(suspend_kt);
ICX_PM_HELPER_X_KT_SHOW(resume_kt);

/*! Template macro to define DEVICE_ATTR TS(timespec) show functions. */
#define	ICX_PM_HELPER_X_TS_SHOW(x_ts) \
STATIC_FUNC ssize_t icx_pm_helper_##x_ts##_show(struct device *dev, struct device_attribute *attr, char *buf) \
{	struct icx_pm_helper	*pmh;									\
	int			result;									\
													\
	result = 0;											\
	down(&icx_pm_helper_global_sem);								\
	/*! @note dev_get_drvdata() returns the pointer passed to platform_set_drvdata(). */		\
	pmh = dev_get_drvdata(dev);									\
	if (pmh == NULL) {										\
		goto out;										\
	}												\
	/*! @note the suitable buffer size is shown in drivers/base/core.c. */				\
	result = snprintf(buf, PAGE_SIZE, "%llu.%09lu\n",						\
		(unsigned long long)(pmh->x_ts.tv_sec),							\
		(unsigned long)     (pmh->x_ts.tv_nsec)							\
	);												\
out:													\
	up(&icx_pm_helper_global_sem);									\
	return result;											\
}

ICX_PM_HELPER_X_TS_SHOW(suspend_prepare_ts);
ICX_PM_HELPER_X_TS_SHOW(post_suspend_ts);
ICX_PM_HELPER_X_TS_SHOW(suspend_ts);
ICX_PM_HELPER_X_TS_SHOW(resume_ts);

/*! Template macro to define DEVICE_ATTR SPM show functions. */
#define	ICX_PM_HELPER_SPM_X_SHOW(x) \
STATIC_FUNC ssize_t icx_pm_helper_spm_##x##_show(struct device *dev, struct device_attribute *attr, char *buf) \
{	struct icx_pm_helper_spm_stat	*sps;								\
	int				result;								\
	unsigned long			val;								\
	unsigned long			flags;								\
													\
	down(&icx_pm_helper_global_sem);								\
	sps = &icx_pm_helper_spm_stat;									\
	spin_lock_irqsave(&(sps->lock), flags);								\
	val = sps->x;											\
	spin_unlock_irqrestore(&(sps->lock), flags);							\
	result = snprintf(buf, PAGE_SIZE, "0x%.8lx\n", val);						\
	up(&icx_pm_helper_global_sem);									\
	return result;											\
}

/*! Template macro to define DEVICE_ATTR SPM show functions. */
#define	ICX_PM_HELPER_SPM_X_SHOW_ULL(x) \
STATIC_FUNC ssize_t icx_pm_helper_spm_##x##_show(struct device *dev, struct device_attribute *attr, char *buf) \
{	struct icx_pm_helper_spm_stat	*sps;								\
	int				result;								\
	unsigned long long		val;								\
	unsigned long			flags;								\
													\
	down(&icx_pm_helper_global_sem);								\
	sps = &icx_pm_helper_spm_stat;									\
	spin_lock_irqsave(&(sps->lock), flags);								\
	val = sps->x;											\
	spin_unlock_irqrestore(&(sps->lock), flags);							\
	result = snprintf(buf, PAGE_SIZE, "%llu\n", val);						\
	up(&icx_pm_helper_global_sem);									\
	return result;											\
}

ICX_PM_HELPER_SPM_X_SHOW(debug_reg);
ICX_PM_HELPER_SPM_X_SHOW(r12);
ICX_PM_HELPER_SPM_X_SHOW(raw_sta);
ICX_PM_HELPER_SPM_X_SHOW(cpu_wake);
ICX_PM_HELPER_SPM_X_SHOW(timer_out);
ICX_PM_HELPER_SPM_X_SHOW(event_reg);
ICX_PM_HELPER_SPM_X_SHOW(isr);
ICX_PM_HELPER_SPM_X_SHOW(r13);
ICX_PM_HELPER_SPM_X_SHOW_ULL(resume_count);

STATIC_FUNC ssize_t icx_pm_helper_spm_eint_sta_show(struct device *dev, struct device_attribute *attr, char *buf)
{	ssize_t		ofs;
	int		ei;
	struct icx_pm_helper_spm_stat	*sps;
	uint32_t	eint_copy[(EINT_MAX_CHANNEL + 31) / 32];
	unsigned long	flags;

	sps = &icx_pm_helper_spm_stat;
	spin_lock_irqsave(&(sps->lock), flags);
	memcpy(eint_copy, sps->eint_sta, sizeof(eint_copy));
	spin_unlock_irqrestore(&(sps->lock), flags);

	ofs = 0;
	ei = 0;
	while ((ei < (sizeof(eint_copy)/sizeof(eint_copy[0]))) && (ofs < PAGE_SIZE)) {
		ofs += snprintf(buf+ofs, PAGE_SIZE-ofs, "0x%.8lx", (unsigned long)(eint_copy[ei]));
		if (ofs >= PAGE_SIZE) {
			break;
		}
		ei++;
		if (ei < (sizeof(eint_copy)/sizeof(eint_copy[0]))) {
			buf[ofs] = ',';
			ofs++;
		}
	}
	if ((((ssize_t)PAGE_SIZE)-ofs) >= 2) {
		ofs += snprintf(buf+ofs, PAGE_SIZE-ofs, "\n");
	}
	return ofs;
}

STATIC_FUNC ssize_t icx_pm_helper_resume_lock_ms_show(struct device *dev, struct device_attribute *attr, char *buf)
{	struct icx_pm_helper	*pmh;
	ssize_t			result;

	result = 0;
	down(&icx_pm_helper_global_sem);
	/*! @note dev_get_drvdata() returns the pointer passed to platform_set_drvdata(). */
	pmh = dev_get_drvdata(dev);
	if (pmh == NULL) {
		goto out;
	}
	/*! @note the suitable buffer size is shown in drivers/base/core.c. */
	result = snprintf(buf, PAGE_SIZE, "%lu\n",(unsigned long)(pmh->resume_lock_ms));
out:
	up(&icx_pm_helper_global_sem);
	return result;
}

STATIC_FUNC ssize_t icx_pm_helper_resume_lock_ms_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{	struct icx_pm_helper	*pmh;
	char			*end;
	long			val;
	ssize_t			result;

	result = (/* force_cast */ ssize_t)size;
	down(&icx_pm_helper_global_sem);
	/*! @note dev_get_drvdata() returns the pointer passed to platform_set_drvdata(). */
	pmh = dev_get_drvdata(dev);
	if (pmh == NULL) {
		result = -ENODEV;
		goto out;
	}
	end = NULL;
	val = simple_strtol(buf, &end, 0);
	if ((buf == end) || (end == NULL)) {
		pr_err("%s: Invalid string passed. buf=%.20s\n", __func__, buf);
		result = -EINVAL;
		goto out;
	}
	pmh->resume_lock_ms = val;
out:
	up(&icx_pm_helper_global_sem);
	/* Anyway we consume all written data. */
	return result;
}

STATIC_FUNC ssize_t icx_pm_helper_resume_lock_cancel_show(struct device *dev, struct device_attribute *attr, char *buf)
{	struct icx_pm_helper	*pmh;
	ssize_t			result;

	result = 0;
	down(&icx_pm_helper_global_sem);
	/*! @note dev_get_drvdata() returns the pointer passed to platform_set_drvdata(). */
	pmh = dev_get_drvdata(dev);
	if (pmh == NULL) {
		goto out;
	}
	/*! @note the suitable buffer size is shown in drivers/base/core.c. */
	/*! return alway zero. */
	result = snprintf(buf, PAGE_SIZE, "%lu\n", (unsigned long)0);
out:
	up(&icx_pm_helper_global_sem);
	return result;
}

#define	ICX_PM_HELPER_RESUME_LOCK_CANCEL_DO_NOTHING	(0)
#define	ICX_PM_HELPER_RESUME_LOCK_CANCEL_CANCEL		(1)

STATIC_FUNC ssize_t icx_pm_helper_resume_lock_cancel_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{	struct icx_pm_helper	*pmh;
	char			*end;
	long			val;
	ssize_t			result;

	result = (/* force_cast */ ssize_t)size;
	down(&icx_pm_helper_global_sem);
	/*! @note dev_get_drvdata() returns the pointer passed to platform_set_drvdata(). */
	pmh = dev_get_drvdata(dev);
	if (pmh == NULL) {
		result = -ENODEV;
		goto out;
	}
	end = NULL;
	val = simple_strtol(buf, &end, 0);
	if ((buf == end) || (end == NULL)) {
		pr_err("%s: Invalid string passed. buf=%.20s\n", __func__, buf);
		result = -EINVAL;
		goto out;
	}
	switch (val) {
		case ICX_PM_HELPER_RESUME_LOCK_CANCEL_CANCEL:
			/* Cancel wake lock timer. */
			/* Restart wake lock timer. */
			wake_lock_timeout(&(pmh->resume_wl), 0);
			break;
		case ICX_PM_HELPER_RESUME_LOCK_CANCEL_DO_NOTHING:
			/* do nothing. */
			break;
		default:
			result = -EINVAL;
			break;
	}
out:
	up(&icx_pm_helper_global_sem);
	/* Anyway we consume all written data. */
	return result;
}

STATIC_FUNC ssize_t icx_pm_helper_force_power_off_show(struct device *dev, struct device_attribute *attr, char *buf)
{	struct icx_pm_helper	*pmh;
	ssize_t			result;

	result = 0;
	down(&icx_pm_helper_global_sem);
	/*! @note dev_get_drvdata() returns the pointer passed to platform_set_drvdata(). */
	pmh = dev_get_drvdata(dev);
	if (pmh == NULL) {
		goto out;
	}
	/*! @note the suitable buffer size is shown in drivers/base/core.c. */
	/*! return alway zero. */
	result = snprintf(buf, PAGE_SIZE, "%lu\n", (unsigned long)0);
out:
	up(&icx_pm_helper_global_sem);
	return result;
}

#define	ICX_PM_HELPER_FORCE_POWER_OFF_DO_NOTHING	(0)
#define	ICX_PM_HELPER_FORCE_POWER_OFF_OFF		(1)

STATIC_FUNC ssize_t icx_pm_helper_force_power_off_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{	struct icx_pm_helper	*pmh;
	char			*end;
	long			val;
	ssize_t			result;

	result = (/* force_cast */ ssize_t)size;
	down(&icx_pm_helper_global_sem);
	/*! @note dev_get_drvdata() returns the pointer passed to platform_set_drvdata(). */
	pmh = dev_get_drvdata(dev);
	if (pmh == NULL) {
		result = -ENODEV;
		goto out;
	}
	end = NULL;
	val = simple_strtol(buf, &end, 0);
	if ((buf == end) || (end == NULL)) {
		pr_err("%s: Invalid string passed. buf=%.20s\n", __func__, buf);
		result = -EINVAL;
		goto out;
	}
	switch (val) {
		case ICX_PM_HELPER_FORCE_POWER_OFF_OFF:
			mt_power_off();
			break;
		case ICX_PM_HELPER_FORCE_POWER_OFF_DO_NOTHING:
			/* do nothing. */
			break;
		default:
			result = -EINVAL;
			break;
	}
out:
	up(&icx_pm_helper_global_sem);
	/* Anyway we consume all written data. */
	return result;
}

/*! Template macro to define DEVICE_ATTR icx_pm_helper.int_member show functions. */
#define	ICX_PM_HELPER_PMH_X_SHOW_INT(x) \
STATIC_FUNC ssize_t icx_pm_helper_##x##_show_int(struct device *dev, struct device_attribute *attr, char *buf) \
{	struct icx_pm_helper		*pmh;								\
	int				result;								\
	int				val;								\
													\
	down(&icx_pm_helper_global_sem);								\
	pmh = dev_get_drvdata(dev);									\
	if (pmh == NULL) {										\
		result = -ENODEV;									\
		goto out;										\
	}												\
	val = pmh->x;											\
	result = snprintf(buf, PAGE_SIZE, "%d\n", val);							\
out:													\
	up(&icx_pm_helper_global_sem);									\
	return result;											\
}

/*! Template macro to define DEVICE_ATTR icx_pm_helper.int_member show functions. */
#define	ICX_PM_HELPER_PMH_X_SHOW_LONG(x) \
STATIC_FUNC ssize_t icx_pm_helper_##x##_show_long(struct device *dev, struct device_attribute *attr, char *buf) \
{	struct icx_pm_helper		*pmh;								\
	int				result;								\
	long				val;								\
													\
	down(&icx_pm_helper_global_sem);								\
	pmh = dev_get_drvdata(dev);									\
	if (pmh == NULL) {										\
		result = -ENODEV;									\
		goto out;										\
	}												\
	val = pmh->x;											\
	result = snprintf(buf, PAGE_SIZE, "%ld\n", val);						\
out:													\
	up(&icx_pm_helper_global_sem);									\
	return result;											\
}


ICX_PM_HELPER_PMH_X_SHOW_INT(boot_reset);
ICX_PM_HELPER_PMH_X_SHOW_INT(boot_powerkey);
ICX_PM_HELPER_PMH_X_SHOW_INT(boot_reboot);
ICX_PM_HELPER_PMH_X_SHOW_INT(boot_deadbat);
ICX_PM_HELPER_PMH_X_SHOW_INT(boot_wdt);
ICX_PM_HELPER_PMH_X_SHOW_INT(boot_thermal);
ICX_PM_HELPER_PMH_X_SHOW_LONG(boot_option);

static DEVICE_ATTR(suspend_prepare_kt, S_IRUGO, icx_pm_helper_suspend_prepare_kt_show, NULL);
static DEVICE_ATTR(post_suspend_kt,    S_IRUGO, icx_pm_helper_post_suspend_kt_show,    NULL);
static DEVICE_ATTR(suspend_kt,         S_IRUGO, icx_pm_helper_suspend_kt_show,         NULL);
static DEVICE_ATTR(resume_kt,          S_IRUGO, icx_pm_helper_resume_kt_show,          NULL);

static DEVICE_ATTR(suspend_prepare_ts, S_IRUGO, icx_pm_helper_suspend_prepare_ts_show, NULL);
static DEVICE_ATTR(post_suspend_ts,    S_IRUGO, icx_pm_helper_post_suspend_ts_show,    NULL);
static DEVICE_ATTR(suspend_ts,         S_IRUGO, icx_pm_helper_suspend_ts_show,         NULL);
static DEVICE_ATTR(resume_ts,          S_IRUGO, icx_pm_helper_resume_ts_show,          NULL);

static DEVICE_ATTR(spm_debug_reg,      S_IRUGO, icx_pm_helper_spm_debug_reg_show,      NULL);
static DEVICE_ATTR(spm_r12,            S_IRUGO, icx_pm_helper_spm_r12_show,            NULL);
static DEVICE_ATTR(spm_raw_sta,        S_IRUGO, icx_pm_helper_spm_raw_sta_show,        NULL);
static DEVICE_ATTR(spm_cpu_wake,       S_IRUGO, icx_pm_helper_spm_cpu_wake_show,       NULL);
static DEVICE_ATTR(spm_timer_out,      S_IRUGO, icx_pm_helper_spm_timer_out_show,      NULL);
static DEVICE_ATTR(spm_event_reg,      S_IRUGO, icx_pm_helper_spm_event_reg_show,      NULL);
static DEVICE_ATTR(spm_isr,            S_IRUGO, icx_pm_helper_spm_isr_show,            NULL);
static DEVICE_ATTR(spm_r13,            S_IRUGO, icx_pm_helper_spm_r13_show,            NULL);
static DEVICE_ATTR(eint_sta,           S_IRUGO, icx_pm_helper_spm_eint_sta_show,       NULL);
static DEVICE_ATTR(resume_count,       S_IRUGO, icx_pm_helper_spm_resume_count_show,   NULL);

static DEVICE_ATTR(resume_lock_ms,     S_IWUSR | S_IRUGO, icx_pm_helper_resume_lock_ms_show,      icx_pm_helper_resume_lock_ms_store);
static DEVICE_ATTR(resume_lock_cancel, S_IWUSR | S_IRUGO, icx_pm_helper_resume_lock_cancel_show,  icx_pm_helper_resume_lock_cancel_store);
static DEVICE_ATTR(force_power_off,    S_IWUSR | S_IRUGO, icx_pm_helper_force_power_off_show, icx_pm_helper_force_power_off_store);

static DEVICE_ATTR(boot_reset,         S_IRUGO, icx_pm_helper_boot_reset_show_int,     NULL);
static DEVICE_ATTR(boot_powerkey,      S_IRUGO, icx_pm_helper_boot_powerkey_show_int,  NULL);
static DEVICE_ATTR(boot_reboot,        S_IRUGO, icx_pm_helper_boot_reboot_show_int,    NULL);
static DEVICE_ATTR(boot_deadbat,       S_IRUGO, icx_pm_helper_boot_deadbat_show_int,   NULL);
static DEVICE_ATTR(boot_wdt,           S_IRUGO, icx_pm_helper_boot_wdt_show_int,       NULL);
static DEVICE_ATTR(boot_thermal,       S_IRUGO, icx_pm_helper_boot_thermal_show_int,   NULL);
static DEVICE_ATTR(boot_option,        S_IRUGO, icx_pm_helper_boot_option_show_long,   NULL);

static struct attribute *icx_pm_helper_attributes[] = {
	&dev_attr_suspend_prepare_kt.attr,
	&dev_attr_post_suspend_kt.attr,
	&dev_attr_suspend_kt.attr,
	&dev_attr_resume_kt.attr,
	&dev_attr_suspend_prepare_ts.attr,
	&dev_attr_post_suspend_ts.attr,
	&dev_attr_suspend_ts.attr,
	&dev_attr_resume_ts.attr,
	&dev_attr_spm_debug_reg.attr,
	&dev_attr_spm_r12.attr,
	&dev_attr_spm_raw_sta.attr,
	&dev_attr_spm_cpu_wake.attr,
	&dev_attr_spm_timer_out.attr,
	&dev_attr_spm_event_reg.attr,
	&dev_attr_spm_isr.attr,
	&dev_attr_spm_r13.attr,
	&dev_attr_eint_sta.attr,
	&dev_attr_resume_count.attr,
	&dev_attr_resume_lock_ms.attr,
	&dev_attr_resume_lock_cancel.attr,
	&dev_attr_force_power_off.attr,
	&dev_attr_boot_reset.attr,
	&dev_attr_boot_powerkey.attr,
	&dev_attr_boot_reboot.attr,
	&dev_attr_boot_deadbat.attr,
	&dev_attr_boot_wdt.attr,
	&dev_attr_boot_thermal.attr,
	&dev_attr_boot_option.attr,
	NULL,
};

static const struct attribute_group icx_pm_helper_attr_group = {
	.attrs = icx_pm_helper_attributes,
};

static void icx_pm_helper_probe_boot_reasons(struct icx_pm_helper *pmh)
{
	int	reason;

	pmh->boot_reset =	ICX_PM_HELPER_BOOT_X_UNKNOWN;
	pmh->boot_powerkey =	ICX_PM_HELPER_BOOT_X_UNKNOWN;
	pmh->boot_reboot =	ICX_PM_HELPER_BOOT_X_UNKNOWN;
	pmh->boot_deadbat =	ICX_PM_HELPER_BOOT_X_UNKNOWN;
	pmh->boot_wdt =		ICX_PM_HELPER_BOOT_X_UNKNOWN;
	pmh->boot_thermal =	ICX_PM_HELPER_BOOT_X_UNKNOWN;
	pmh->boot_option =	icx_pm_helper_boot_option;

	reason = icx_pm_helper_boot_reason;
	if (reason != ICX_PM_HELPER_BOOT_REASON_UNKNOWN) {
		/* Boot loader set boot reason. */
		if (reason & ICX_PM_HELPER_BOOT_REASON_POWER_BUTTION) {
			pmh->boot_powerkey = 1;
		} else {
			pmh->boot_powerkey = 0;
		}
	}
	return;
}

#ifdef CONFIG_USB_HOST_OC_DETECT
static void overcurrent_workqueue_func(void)
{
  int ret;
  
  ret = 0;
  
  if (atomic_read(&oc_workq_flag) == OC_WORKQ_FLAG_ENABLED) {
    ret=schedule_delayed_work(&overcurrent_work, msecs_to_jiffies(0));
  }
  
  if (!ret)
  {
    printk("[oc]overcurrent_workqueue_func return:%d!\n", ret);  		
  }
  
}

static void overcurrent_irq_handler(void)
{
  overcurrent_workqueue_func();
}

void overcurrent_event_notify(void)
{
  if (mt_get_gpio_out(GPIO_OTG_DRVVBUS1_PIN) == GPIO_OUT_ONE) {
    if (mt_get_gpio_in(OC_DETECT_PIN_GPIO) == GPIO_OUT_ZERO) {
      switch_set_state(&oc_switch_usb, 1); /* over current condition */
      printk("OVER CURRENT happens!!>>\n");
      mt_eint_registration(OC_DETECT_PIN_EINT, EINTF_TRIGGER_RISING, overcurrent_irq_handler, 1);
      mt_eint_dis_debounce(OC_DETECT_PIN_EINT);
    } else {
      switch_set_state(&oc_switch_usb, 0); /* normal condition (over current is not happened) */
      printk("OVER CURRENT is NOT happens (NORMAL)>>\n");
      mt_eint_registration(OC_DETECT_PIN_EINT, EINTF_TRIGGER_FALLING, overcurrent_irq_handler, 1);
      mt_eint_dis_debounce(OC_DETECT_PIN_EINT);
    }
  }
  else {
    switch_set_state(&oc_switch_usb, 0); /* normal condition (over current is not happened) */
  }
}

static void overcurrent_work_callback(struct work_struct *work)
{
  overcurrent_event_notify();
}

void overcurrent_start_processing(void)
{
  mt_eint_registration(OC_DETECT_PIN_EINT, EINTF_TRIGGER_FALLING, overcurrent_irq_handler, 1);
  mt_eint_dis_debounce(OC_DETECT_PIN_EINT);
  atomic_set(&oc_workq_flag, OC_WORKQ_FLAG_ENABLED);
  overcurrent_event_notify();
}

void overcurrent_end_processing(void)
{
  mt_eint_mask(OC_DETECT_PIN_EINT);
  atomic_set(&oc_workq_flag, OC_WORKQ_FLAG_DISABLED);
  mt_eint_registration(OC_DETECT_PIN_EINT, EINTF_TRIGGER_FALLING, NULL, 0);
}

#endif /* CONFIG_USB_HOST_OC_DETECT */

/*! driver probe
*/
int icx_pm_helper_probe(struct platform_device *pdev)
{	struct icx_pm_helper	*pmh;
	int			result;
	int			ret;

#ifdef CONFIG_USB_HOST_OC_DETECT
  atomic_set(&oc_workq_flag, OC_WORKQ_FLAG_ENABLED);
  
  overcurrent_workqueue = create_singlethread_workqueue("overcurrent");
  INIT_DELAYED_WORK(&overcurrent_work, overcurrent_work_callback);
  mt_set_gpio_mode(OC_DETECT_PIN_GPIO, GPIO_MODE_00);
  mt_set_gpio_dir(OC_DETECT_PIN_GPIO, GPIO_DIR_IN);
  mt_set_gpio_pull_enable(OC_DETECT_PIN_GPIO, GPIO_PULL_DISABLE);
  
  oc_switch_usb.name = pm_usb_sw_name;
  oc_switch_usb.state = 0;
  ret = switch_dev_register(&oc_switch_usb);
#endif /* CONFIG_USB_HOST_OC_DETECT */

	down(&icx_pm_helper_global_sem);
	result = 0;
	PRINTK_LI("%s: Probe driver. name=%s\n", __func__, pdev->name);
	pmh = kzalloc(sizeof(*pmh), GFP_NOIO);
	if (!pmh) {
		/* Can't allocate driver context. */
		pr_err("%s: Can not allocate pm helper context.\n", __func__);
		result = -ENOMEM;
		goto out;
	}
	
	icx_pm_helper_probe_boot_reasons(pmh);
	
	pmh->dev = &(pdev->dev);
	wake_lock_init(&(pmh->resume_wl), WAKE_LOCK_SUSPEND, "");
	pmh->resume_lock_ms = CONFIG_ICX_PM_HELPER_RESUME_LOCK_MS;

	ret = sysfs_create_group(&pdev->dev.kobj, &icx_pm_helper_attr_group);
	if (ret != 0) {
		pr_err("%s: Can not create sysfs files. ret=%d\n", __func__, ret);
		result = ret;
		goto out_free;
	}

	platform_set_drvdata(pdev, pmh);
	pmh->pm_nb.notifier_call = icx_pm_helper_notifier_call;
	pmh->pm_nb.priority = 0;
	register_pm_notifier(&(pmh->pm_nb));
	up(&icx_pm_helper_global_sem);
	return result;

out_free:
	kfree(pmh);
out:
	up(&icx_pm_helper_global_sem);
	return result;
}

int icx_pm_helper_shutdown_remove(struct platform_device *pdev)
{	struct icx_pm_helper	*pmh;

#ifdef CONFIG_USB_HOST_OC_DETECT
  cancel_delayed_work_sync(&overcurrent_work);
  destroy_workqueue(overcurrent_workqueue);
#endif /* CONFIG_USB_HOST_OC_DETECT */

	down(&icx_pm_helper_global_sem);
	pmh = platform_get_drvdata(pdev);
	if (pmh == NULL) {
		/* Not registered pm helper context. */
		pr_err("%s: NULL points pm helper context.\n", __func__);
		goto out;
	}
	pmh->dev = NULL;
	/* registered drvdata. */
	unregister_pm_notifier(&(pmh->pm_nb));
	wake_lock_destroy(&(pmh->resume_wl));
	sysfs_remove_group(&pdev->dev.kobj, &icx_pm_helper_attr_group);
	platform_set_drvdata(pdev, NULL);
	kfree(pmh);
out:
	up(&icx_pm_helper_global_sem);
	return 0;
}

/*! driver shutdown
*/
void icx_pm_helper_shutdown(struct platform_device *pdev)
{	PRINTK_LI("%s: Shutdown driver. name=%s\n", __func__, pdev->name);
	icx_pm_helper_shutdown_remove(pdev);
}

/*! driver remove
*/
int icx_pm_helper_remove(struct platform_device *pdev)
{	PRINTK_LI("%s: Remove driver. name=%s\n", __func__, pdev->name);
	return icx_pm_helper_shutdown_remove(pdev);
}

#if (defined(CONFIG_PM))
/*! @note On SONY ICX platform, We always define CONFIG_PM.  */

/*! driver suspend
*/
int icx_pm_helper_susped(struct platform_device *pdev,  pm_message_t state)
{	struct icx_pm_helper	*pmh;

	PRINTK_EN("%s: Called. pdev=0x%p, state=%u\n", __func__, pdev, (unsigned)state.event);
	
#ifdef CONFIG_USB_HOST_OC_DETECT
  cancel_delayed_work_sync(&overcurrent_work);
  destroy_workqueue(overcurrent_workqueue);
#endif /* CONFIG_USB_HOST_OC_DETECT */
	
	down(&icx_pm_helper_global_sem);
	pmh = platform_get_drvdata(pdev);
	if (pmh == NULL) {
		/* Not registered pm helper context. */
		pr_err("%s: NULL points platform_device, continue resume.\n", __func__);
		goto out;
	}
	getnstimeofday(&(pmh->suspend_ts));
	pmh->suspend_kt = ktime_get();
out:
	up(&icx_pm_helper_global_sem);
	return 0;
}

/*! driver resume
*/
int icx_pm_helper_resume(struct platform_device *pdev)
{	struct icx_pm_helper	*pmh;
	int			result;
	int			lock_ms;
	
#ifdef CONFIG_USB_HOST_OC_DETECT  
  overcurrent_workqueue = create_singlethread_workqueue("overcurrent");
  INIT_DELAYED_WORK(&overcurrent_work, overcurrent_work_callback);
#endif /* CONFIG_USB_HOST_OC_DETECT */
	
	PRINTK_EN("%s: Called. pdev=0x%p\n", __func__, pdev);
	result = 0;
	down(&icx_pm_helper_global_sem);
	pmh = platform_get_drvdata(pdev);
	if (pmh == NULL) {
		/* Not registered pm helper context. */
		pr_err("%s: NULL points platform_device, continue resume.\n", __func__);
		goto out;
	}
	getnstimeofday(&(pmh->resume_ts));
	pmh->resume_kt = ktime_get();

	lock_ms = pmh->resume_lock_ms;
	if (lock_ms > 0) {
		PRINTK_LI("%s: Wakelock with timeout. lock_ms=%d\n", __func__, lock_ms);
		wake_lock_timeout(&(pmh->resume_wl), msecs_to_jiffies(lock_ms));
	}
out:
	up(&icx_pm_helper_global_sem);
	return result;
}
#else /* (defined(CONFIG_PM)) */
#define icx_pm_helper_susped	(NULL)
#define icx_pm_helper_resume	(NULL)
#endif /* (defined(CONFIG_PM)) */

static struct platform_driver icx_pm_helper_driver = {
	.probe		= icx_pm_helper_probe,
	.remove		= icx_pm_helper_remove,
	.shutdown	= icx_pm_helper_shutdown,
#if (defined(CONFIG_PM))
	.suspend	= icx_pm_helper_susped,
	.resume		= icx_pm_helper_resume,
#endif /* (defined(CONFIG_PM)) */
	.driver		= {
		.name	= this_driver_name,
		.owner  = THIS_MODULE,
	},
};

/*! module init procedure.
*/
static int __init icx_pm_helper_init(void)
{
	PRINTK_LI("%s: Register platform driver. name=%s\n", __func__, icx_pm_helper_driver.driver.name);
	platform_driver_register(&icx_pm_helper_driver);
	return 0;
}
module_init(icx_pm_helper_init);

/*! module exit procedure.
*/
static void __exit icx_pm_helper_exit(void)
{
	PRINTK_EN("%s: Called.\n", __func__);
	platform_driver_unregister(&icx_pm_helper_driver);
	return;
}
module_exit(icx_pm_helper_exit);

MODULE_ALIAS("platform:icx_pm_helper");

MODULE_AUTHOR("SONY");
MODULE_DESCRIPTION("ICX platform power management helper.");
MODULE_LICENSE("GPL");

#ifdef CONFIG_USB_HOST_OC_DETECT
EXPORT_SYMBOL(overcurrent_event_notify);
EXPORT_SYMBOL(overcurrent_end_processing);
EXPORT_SYMBOL(overcurrent_start_processing);
#endif /* CONFIG_USB_HOST_OC_DETECT */
