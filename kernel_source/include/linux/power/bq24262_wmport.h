/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* BQ24262 battery charger controller with WM-PORT.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef BQ24262_WMPORT_H_INCLUDED
#define BQ24262_WMPORT_H_INCLUDED

#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/notifier.h>
#include <linux/power_supply.h>
#include <linux/power/bq24262.h>
#include <linux/wakelock.h>
#include <linux/suspend.h>
#include <linux/earlysuspend.h>

#include "mach/bq24262_platform.h"

/* Handy printable strings. */
/*! Battery charger (USB-AC) names. */
extern const char *bq24262_wmport_bcdet_names[];

/* MUSB connect and disconnect functions. */
void mt_usb_connect(void);
void mt_usb_disconnect(void);

/* VBUS current remarkable values.
   current,  usb_state
     0..  2, SUSPEND
     2.. 99, UNCONFIG
   100..499, CONFIG_LOW
        500, CONFIG
*/
#define	BQ24262_WMPORT_VBUS_CURRENT_SUSPEND_MA		  (2)
#define	BQ24262_WMPORT_VBUS_CURRENT_UNCONFIG_MA		 (99) /*!< @note intentionally use 99 instead of 100 */
#define	BQ24262_WMPORT_VBUS_CURRENT_CONFIG_MA		(500)

/* power source work control flags in bit number.
   Lockings:
   (splock) Lock spin lock (bc->lock), when you change or read two or more flag
   bits and other (bc) members at once.
   (thm) only read/modify/write in thread main.
   
*/
#define	BQ24262_WMPORT_PPATH_WORK_ACTIVE	(0x0000)	/*!< (splock) Ready to run power path IRQ worker. */
#define	BQ24262_WMPORT_PPATH_WORK_RETRIGGER	(0x0001)	/*!< (splock) need retrigger(re-enter to work queue) power path IRQ worker. */
#define	BQ24262_WMPORT_USB_IRQ			(0x0002)	/*!< (not-locked) Assigned USB VBUS detection IRQ to function. */
#define	BQ24262_WMPORT_DCIN_IRQ			(0x0003)	/*!< (not-locked) Assigned DCIN detection IRQ to function. */
#define	BQ24262_WMPORT_THR_MAIN_START		(0x0004)	/*!< (thm) Main thread started. */
#define	BQ24262_WMPORT_THR_MAIN_WAKE		(0x0005)	/*!< (not-locked) Request wake main thread. */
#define	BQ24262_WMPORT_THR_MAIN_TERMINATE	(0x0006)	/*!< (splock) Request terminate main thread. */
#define	BQ24262_WMPORT_THR_MAIN_EXIT		(0x0007)	/*!< (splock) Exit main thread. */
#define	BQ24262_WMPORT_THR_MAIN_SYNC_REQ	(0x0008)	/*!< NOT USED. */
#define	BQ24262_WMPORT_THR_MAIN_SYNC_ACK	(0x0009)	/*!< NOT USED. */
#define	BQ24262_WMPORT_WAKE_SOURCE_CHANGED	(0x000a)	/*!< (splock) Feeding power to USB VBUS or DCIN wakeups (resumes) me. */
#define	BQ24262_WMPORT_CHARGER_IRQ_DISABLED	(0x000b)	/*!< disabled charger IRQ at suspend, no one set this bit, always cleared. */
#define	BQ24262_WMPORT_SUSPEND			(0x000c)	/*!< (thm) during from "called _suspend" to "called _resume"  */
#define	BQ24262_WMPORT_SUSPEND_IN_PROGRESS	(0x000d)	/*!< during form "notified PM_SUSPEND_PREPARE" to "notified PM_POST_SUSPEND" */
#define	BQ24262_WMPORT_THR_MAIN_RESUME_REQ	(0x000e)	/*!< (splock) Request resume to main thread. */
#define	BQ24262_WMPORT_THR_MAIN_RESUME_ACK	(0x000f)	/*!< (not-locked, atomic-bit-op) Main thread acknowledge resume. */
#define	BQ24262_WMPORT_THR_MAIN_SUSPEND_REQ	(0x0010)	/*!< (splock) Request suspend to main thread. */
#define	BQ24262_WMPORT_THR_MAIN_SUSPEND_ACK	(0x0011)	/*!< (not-locked, atomic-bit-op) Main thread acknowledge suspend. */
#define	BQ24262_WMPORT_THR_MAIN_SHUTDOWN_REQ	(0x0012)	/*!< (splock) Request shutdown to main thread, but not implies terminating thread. */
#define	BQ24262_WMPORT_THR_MAIN_SHUTDOWN_ACK	(0x0013)	/*!< (not-locked, atomic-bit-op) Main thread acknowledge shutdown.  */
/*! This flag turns only 0 to 1. */
#define	BQ24262_WMPORT_SHUTDOWN			(0x0014)	/*! Shutdown started. */
/*! This flag turns only 0 to 1. */
#define	BQ24262_WMPORT_SAFE_SUSPEND_CURRENT	(0x0015)	/*! (usb) Now safe to (should) stop drawing current from VBUS. */
#define	BQ24262_WMPORT_THR_MAIN_DIAG_REQ	(0x0016)	/*! Request diag operation to main thread. */
#define	BQ24262_WMPORT_THR_MAIN_DIAG_ACK	(0x0017)	/*! Main thread acknowledge diag operation. */
#define	BQ24262_WMPORT_CHARGER_WORK_ACTIVE	(0x0018)	/*!< (splock) Ready to run charger IRQ worker. */
#define	BQ24262_WMPORT_CHARGER_WORK_RETRIGGER	(0x0019)	/*!< (splock) need retrigger (re-enter to work queue) charger IRQ worker. */
#define	BQ24262_WMPORT_CHARGER_IRQ		(0x001a)	/*!< NOT USED. */
#define	BQ24262_WMPORT_CHARGER_IRQ_ENABLE	(0x001b)	/*!< (chg) Enabled charger IRQ. */
#define	BQ24262_WMPORT_USB_GADGET_ENABLE	(0x001c)	/*!< Enabled android USB gadget driver. */


/* PC port capability. */
#define	BQ24262_WMPORT_PC_PORT_STD	(0)	/*!< Standard port. */
#define	BQ24262_WMPORT_PC_PORT_CDP	(1)	/*!< Charge Downstream Port. */

#define	BQ24262_WMPORT_DIAG_OFF		(0)	/*!< Normal operation. */
#define	BQ24262_WMPORT_DIAG_ACTIVE	(1)	/*!< Diag operation. */
#define	BQ24262_WMPORT_DIAG_VBUS_DOWN	(2)	/*!< Diag will be expired by VBUS down. */
#define	BQ24262_WMPORT_DIAG_EXIT	(3)	/*!< Back to normal from diag operation. */


/*! Invalid fake capacity. */
#define	BQ24262_WMPORT_FAKE_CAPACITY_OFF	(-1)

/* Flags in power_supply_rate_limited.flags */
#define	POWER_SUPPLY_RL_INIT	(0x0000)	/*!< inited, only turn 0->1. */
#define	POWER_SUPPLY_RL_PEND	(0x0001)	/*!< pending notify "changed". */

/* OVP(power path switch) powered flags. */
#define	BQ24262_WMPORT_USB		(0x0001)	/*!< powered from USB  */
#define	BQ24262_WMPORT_DCIN		(0x0002)	/*!< powered from DCIN */
#define	BQ24262_WMPORT_DUMMY		(0x0004)	/*!< driver internal dummy flag. */

/* USB power source connection states.
*/
#define	BQ24262_WMPORT_USB_SUSPENDED	(USB_SUSPEND)		/*!< USB power source suspended (host goes sleep). */
#define	BQ24262_WMPORT_USB_UNCONFIG	(USB_UNCONFIGURED)	/*!< USB power source before configure. */
#define	BQ24262_WMPORT_USB_CONFIG	(USB_CONFIGURED)	/*!< USB power source after configure. */
#define	BQ24262_WMPORT_USB_CONFIG_LOW	(3)			/*!< USB power source after configure with low power. */
#define	BQ24262_WMPORT_USB_UNKNOWN	(4)			/*!< Unknown USB state. */
#define	BQ24262_WMPORT_USB_LBB_DRAW	(5)			/*!< Draw current at Low Battery Boot. */

/*! @note Avoid following error condition about BQ24262_WMPORT_USB_XXX macros.
 if (((USB_CONFIGURED) == BQ24262_WMPORT_USB_UNKNOWN) || ((USB_UNCONFIGURED) == BQ24262_WMPORT_USB_UNKNOWN) || ((USB_SUSPEND) == BQ24262_WMPORT_USB_UNKNOWN))
 error "Conflict constant value BQ24262_WMPORT_USB_UNKNOWN  for state."
 endif

 if (((USB_CONFIGURED) == BQ24262_WMPORT_USB_LBB_DRAW) || ((USB_UNCONFIGURED) == BQ24262_WMPORT_USB_LBB_DRAW) || ((USB_SUSPEND) == BQ24262_WMPORT_USB_LBB_DRAW))
 error "Conflict constant value BQ24262_WMPORT_USB_LBB_DRAW  for state."
 endif
*/

/* Power source configuration states. */
#define	BQ24262_WMPORT_CONFIG_BOOT		 (0) /*!< Unknown on boot process. */
#define	BQ24262_WMPORT_CONFIG_HIZ		 (1) /*!< power source off. */
#define	BQ24262_WMPORT_CONFIG_USB_ROLE_A	 (2) /*!< Mini A plugged.  */
#define	BQ24262_WMPORT_CONFIG_USB_UNKNOWN	 (3) /*!< Unknown. */
#define	BQ24262_WMPORT_CONFIG_USB_UNCONFIG	 (4) /*!< Unconfigured. */
#define	BQ24262_WMPORT_CONFIG_USB_CONFIG	 (5) /*!< Configured. */
#define	BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_500 (6) /*!< PS2 like (SONY) charger. */
#define	BQ24262_WMPORT_CONFIG_USB_AC_CHARGER_900 (7) /*!< DCP(Dedicated Charging Port) */
#define	BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_500 (8) /*!< CDP(Charging Down stream Port), before linked(BCS1.1). */
#define	BQ24262_WMPORT_CONFIG_USB_PC_CHARGER_900 (9) /*!< CDP(Charging Down stream Port), after linked(BCS1.1 and BCS1.2). */
#define	BQ24262_WMPORT_CONFIG_DCIN		(10) /*!< DCIN. AC-DC adapter */
#define	BQ24262_WMPORT_CONFIG_NO_POWER		(11) /*!< No power source. */
#define	BQ24262_WMPORT_CONFIG_COOL_DOWN		(12) /*!< Cool down charging circuit. */
#define	BQ24262_WMPORT_CONFIG_USB_RECOVERY	(13) /*!< Recovery boot mode. */
#define	BQ24262_WMPORT_CONFIG_USB_CONFIG_LOW	(14) /*!< Configured with low VBUS current. */
#define	BQ24262_WMPORT_CONFIG_USB_SUSPENDED	 (15) /*!< Suspended (host goes sleep). */
#define	BQ24262_WMPORT_CONFIG_USB_SUS_DRAW	 (16) /*!< Suspended (host goes sleep). */
#define	BQ24262_WMPORT_CONFIG_ALL		(17) /*!< The number of configurations. */

/*! Configuration values to set CHAGERUSB_ regster.
    @note On ICX platform, Disconnecting DCIN or Lost power from both 
          USB and DCIN initializes CHAGERUSB_ register values. So reconfigure
          CHAGERUSB_ registers at changing power state.
*/
struct bq24262_wmport_configs {
	const char	*name;			/*!< Configuration name.	*/
	unsigned	charger_ps_status;	/*!< Charge status.		*/
	const bool	notify_health;	/*!< Notify helth status	*/

	const uint16_t	flags;			/*!< Write Flags.		*/
	const uint8_t	control;		/*!< 0x01 CONTROL		*/
	const uint8_t	battery_voltage;	/*!< 0x02 BATTERY_VOLTAGE	*/
	const uint8_t	battery_current;	/*!< 0x04 BATTERY_CURRENT	*/
	const uint8_t	vin_minsys;		/*!< 0x05 VIN_MINSYS		*/
};

#define	BQ24262_WMPORT_CONFIGS_W_CONTROL		(0x0001)	/*!< Write charger CONTROL(0x01) register. */
#define	BQ24262_WMPORT_CONFIGS_W_BATTERY_VOLTAGE	(0x0002)	/*!< Write charger BATTERY_VOLTAGE(0x02) register. */
#define	BQ24262_WMPORT_CONFIGS_W_BATTERY_CURRENT	(0x0004)	/*!< Write charger BATTERY_CURRENT(0x04) register. */
#define	BQ24262_WMPORT_CONFIGS_W_VIN_MINSYS		(0x0008)	/*!< Write charger VIN_MINSYS(0x05) register. */
#define	BQ24262_WMPORT_CONFIGS_FORCE_DCIN		(0x0010)	/*!< Set FORCE_DCIN, draw current from DCIN. */
#define	BQ24262_WMPORT_CONFIGS_SUSPEND			(0x0020)	/*!< Set CMIC.SUSPEND bit, stop draw current from VBUS and DCIN. */
#define	BQ24262_WMPORT_CONFIGS_SYS_WAK_STAT		(0x0040)	/*!< Set SYS_WAK_STAT, now need power to VSYS. */
#define	BQ24262_WMPORT_CONFIGS_DRAWING			(0x0080)	/*!< Draw current from VBUS or DCIN, used for driver state transition. */
#define	BQ24262_WMPORT_CONFIGS_CHARGING			(0x0100)	/*!< Charging battery, used for driver state transition. */

#define	BQ24262_WMPORT_CONFIGS_W_MAIN_SETS		\
	( BQ24262_WMPORT_CONFIGS_W_CONTROL		\
	| BQ24262_WMPORT_CONFIGS_W_BATTERY_VOLTAGE	\
	| BQ24262_WMPORT_CONFIGS_W_BATTERY_CURRENT	\
	| BQ24262_WMPORT_CONFIGS_W_VIN_MINSYS		\
	)

/* Power source configuration states, enable/disable charging. */
#define	BQ24262_WMPORT_BATTERY_CHARGE_UP	(0)	/*!< Charge battery. */
#define	BQ24262_WMPORT_BATTERY_CHARGE_KEEP_FULL	(1)	/*!< Keep battery charge(keep full). */
#define	BQ24262_WMPORT_BATTERY_CHARGE_KEEP_EMG	(2)	/*!< Keep battery charge(on emergency). */
#define	BQ24262_WMPORT_BATTERY_CHARGE_ALL	(3)	/*!< The number of states. */


/* wake_lock_source bit masks. */
#define	BQ24262_WMPORT_WAKE_LOCK_CHARGING	(0x0001)	/*!< enable wake_lock at charging. */
#define	BQ24262_WMPORT_WAKE_LOCK_USB		(0x0002)	/*!< enable wake_lock at VBUS powered */
#define	BQ24262_WMPORT_WAKE_LOCK_DCIN		(0x0004)	/*!< enable wake_lock at DCIN powered. */
#define	BQ24262_WMPORT_WAKE_LOCK_LOCKED		(0x0008)	/*!< wake_lock_charging locked. */
#define	BQ24262_WMPORT_WAKE_LOCK_FACTOR_ALL	\
	( 0 					\
	| BQ24262_WMPORT_WAKE_LOCK_CHARGING	\
	| BQ24262_WMPORT_WAKE_LOCK_USB		\
	| BQ24262_WMPORT_WAKE_LOCK_DCIN		\
	)

/* charge_timer_state */
#define	BQ24262_WMPORT_CHARGE_TIMER_WAIT_SYSTEM_RUNNING	(0)	/*!< Wait system running, ready to use RTC.	*/
#define	BQ24262_WMPORT_CHARGE_TIMER_NOT_CHARGING	(1)	/*!< Not charging.				*/
#define	BQ24262_WMPORT_CHARGE_TIMER_CHARGING		(2)	/*!< Charging.					*/
#define	BQ24262_WMPORT_CHARGE_TIMER_EXPIRE		(3)	/*!< Expired charging timer.			*/

#define	BQ24262_WMPORT_DYNAMIC_PERIOD_NOT_UPDATED	(-1L)	/*!< Discharging polling period while suspending is unknown. */

/* wake_lock_flags bit number. */
#define	BQ24262_WMPORT_WAKE_LOCK_LOCKING	(0x0000)

/*! Read status holder. */
struct bq24262_wmport_status {
	uint8_t	status;				/*!< 0x00 STATUS			*/
	uint8_t	vin_minsys;			/*!< 0x05 VIN-DPM/MINSYS		*/
	uint8_t	safety;				/*!< 0x06 SAFETY TIMER/NTC MONITOR	*/
};

/* Pending "notify changed" index, includes {battery, usb, dc}.
   PSRL = power supply rate limited
*/
#define	BQ24262_WMPORT_PSRL_BAT		(0)	/*!< power supply battery. */
#define	BQ24262_WMPORT_PSRL_USB		(1)	/*!< power supply USB. */
#define	BQ24262_WMPORT_PSRL_DC		(2)	/*!< power supply DCIN. */
#define	BQ24262_WMPORT_PSRL_NUM		(3)	/*!< The number of power supplyes. */

/* Flags in power_supply_rate_limited.flags */
#define	POWER_SUPPLY_RL_INIT		(0x0000)	/*!< inited, only turn 0->1. */
#define	POWER_SUPPLY_RL_PEND		(0x0001)	/*!< pending notify "changed". */
#define	POWER_SUPPLY_RL_SUSPEND_MASK	(0x0002)	/*!< mask notify while suspend/resume. */

struct	power_supply_rate_limited {
	struct power_supply	ps;		/*!< Power supply state. */
	spinlock_t		state_lock;	/*!< locks {flags, jiffies_changed}. */
	unsigned long		flags;		/*!< Power supply rate limitted flags. */
	unsigned long		jiffies_changed;/*!< jiffies at notified. */
};

struct bq24262_wmport_diag_draw {
	int		config;		/*!< BQ24262_WMPORT_CONFIG_x */
	int		battery_charge;	/*!< _CHARGE_UP or _CHARGE_DOWN */
};

#define	BQ24262_WMPORT_CHARGE_FULLY	(0)	/*!< Charge battery up to maximum capacity. */
#define	BQ24262_WMPORT_CHARGE_WEAKLY	(1)	/*!< Charge battery up to long life capacity. */

#define	BQ24262_WMPORT_USB_SUS_NORMAL	(0)	/*!< USB suspend mode 0mA (normal). */
#define	BQ24262_WMPORT_USB_SUS_DRAW		(1)	/*!< USB suspend mode 500mA (early suspend). */

#define	BQ24262_WMPORT_BATTERY_CAPACITY_INVALID		(-1)

#define	BQ24262_WMPORT_BATTERY_TEMP_STATE_NORMAL	(0)	/*!< Battery temperature is normal. */
#define	BQ24262_WMPORT_BATTERY_TEMP_STATE_COLD		(1)	/*!< Bettery temperature is cold. */
#define	BQ24262_WMPORT_BATTERY_TEMP_STATE_HOT		(2)	/*!< Bettery temperature is hot. */

/* VINDPM state. */
#define	BQ24262_WMPORT_VINDPM_LO	(0)	/*!< VINDPM low state. */
#define	BQ24262_WMPORT_VINDPM_HI	(1)	/*!< VINDPM high state. */

/* Full stop mode state. */
#define	BQ24262_WMPORT_BATTERY_BOOST_CHARGE		(0)	/*!< Charge current to battery. */
#define	BQ24262_WMPORT_BATTERY_BOOST_STOP		(1)	/*!< Stop(Zero) current to/from battery. */

struct bq24262_wmport_context;

/* thread VBUS flag bit number.
*/
#define	BQ24262_WMPORT_THR_VBUS_START		(0x0000)	/*!< VBUS thread started. */
#define	BQ24262_WMPORT_THR_VBUS_WAKE		(0x0001)	/*!< Request wake VBUS thread. */
#define	BQ24262_WMPORT_THR_VBUS_TERMINATE	(0x0002)	/*!< Request terminate VBUS thread. */
#define	BQ24262_WMPORT_THR_VBUS_EXIT		(0x0003)	/*!< VBUS thread exited. */
#define	BQ24262_WMPORT_THR_VBUS_SYNC_REQ	(0x0004)	/*!< NOT USED. */
#define	BQ24262_WMPORT_THR_VBUS_SYNC_ACK	(0x0005)	/*!< NOT USED. */
#define	BQ24262_WMPORT_THR_VBUS_RESUME_REQ	(0x0006)	/*!< Request resume to VBUS thread. */
#define	BQ24262_WMPORT_THR_VBUS_RESUME_ACK	(0x0007)	/*!< VBUS thread acknowledge resume. */
#define	BQ24262_WMPORT_THR_VBUS_SUSPEND_REQ	(0x0008)	/*!< Request suspend to VBUS thread. */
#define	BQ24262_WMPORT_THR_VBUS_SUSPEND_ACK	(0x0009)	/*!< VBUS thread acknowledge suspend. */
#define	BQ24262_WMPORT_THR_VBUS_SHUTDOWN_REQ	(0x000a)	/*!< Request shutdown to VBUS thread. */
#define	BQ24262_WMPORT_THR_VBUS_SHUTDOWN_ACK	(0x000b)	/*!< VBUS thread acknowledge shutdown. */
#define	BQ24262_WMPORT_THR_VBUS_GADGET_READY	(0x000c)	/*!< Enabled android USB gadget driver. (only turn 0 to 1). */
#define	BQ24262_WMPORT_THR_VBUS_PHY_CHG			(0x000d)	/*!< VBUS PHY change */

#define	BQ24262_WMPORT_VB_STATE_INIT			(0)	/*!< initialize at boot state. */
#define	BQ24262_WMPORT_VB_STATE_INIT_WAIT_RUN		(1)	/*!< wait until system running (execute init.rc). */
#define	BQ24262_WMPORT_VB_STATE_INIT_DISCONNECT		(2)	/*!< 1st disconnect. */
#define	BQ24262_WMPORT_VB_STATE_CHARGER_DETECT		(3)	/*!< charger detect. */
#define	BQ24262_WMPORT_VB_STATE_CONNECT_DEBOUNCE	(4)	/*!< debounce at connect. */
#define	BQ24262_WMPORT_VB_STATE_CONTACT_DPDM		(5)	/*!< debounce at connect. */
#define	BQ24262_WMPORT_VB_STATE_USB_CONNECT		(6)	/*!< USB connect. */
#define	BQ24262_WMPORT_VB_STATE_USB_DISCONNECT		(7)	/*!< USB disconnect. */

/*! VBUS thread context	*/
struct bq24262_wmport_vbus_context {
	unsigned long		flags;			/*!< thread VBUS flags.		*/
	spinlock_t		lock;			/*!< =(vb-splock)		*/
	struct task_struct	*thr_vbus;		/*!< Thread VBUS task context.	*/
	int			state;			/*!< Thread VBUS state.		*/
	unsigned long		jiffies_now;		/*!< thread base jiffies time.	*/
	unsigned long		jiffies_remain;		/*!< timeout jiffies.		*/
	int			polling;		/*!< Polling count.		*/
	int			debounce_count;		/*!< Debounce count.		*/
	int			contact_ready;		/*!< Contact ready count.	*/
	char			resume_req;		/*!< resume requested.		*/
	char			suspend_req;		/*!< suspend requested.		*/
	char			shutdown_req;		/*!< shutdown requested.	*/
	unsigned		usb_bcdet_splocked;	/*!< (vb-splock) Battery Charger DETection status. */
	/* Thread wake event. */
	wait_queue_head_t	event_vbus;
	/* Thread start stop handshake event. */
	wait_queue_head_t	event_hs;
};

struct bq24262_wmport_context {
	struct device				*dev;			/*!< Linux driver core device conetxt. */
	struct bq24262_wmport_platform_data	*platform_data;		/*!< Pointer to copy of platform_data. */
	struct i2c_client			*charger_client;	/*!< BQ24262 charger I2C client context.	*/

	/* @note "icx_ps_" means "icx_power_source_" */		/*   v lock nest order. */
	spinlock_t				lock;		/*!<   =(splock) */
	unsigned long				flags;		/*!< (splock) _work_ .. _thr_main_		*/

	unsigned				usb_bcdet;	/*!< (usb) Battery Charger DETection status. */
	int					usb_state;	/*!< (usb) USB device state. */
	unsigned				usb_max_power;	/*!< (usb) bMaxPower in mA. */

								/*   v lock nest order. */
	struct semaphore		status_sem;		/*!< 1  =(sts) Device status semaphore. */
	struct semaphore		usb_sem;		/*!< 2  =(usb) USB status semaphore. */
	struct semaphore		wake_lock_sem;		/*!< 3  =(wlk) wake lock control semaphore. */
	struct semaphore 		charger_sem;		/*!< 4  =(chg) Charger Consecutive I2C Read/Write semaphore. */
	struct semaphore		charger_i2c_sem;	/*!< 5  Charger Single I2C Read/Write semaphore, nested inside charger_sem. */
	int				charger_ps_status;	/*!< (sts, thm) Power Supply Class charger status. */

	struct bq24262_wmport_status	charger_status;		/*!< (sts, thm) */
	struct bq24262_wmport_status	charger_status_prev;	/*!< (sts, thm) */
	uint8_t				charger_vendor;		/*!< (unlocked) */
	int				battery_boost;		/*!< (sts, thm) Full stop control. */
	int				battery_boost_prev;	/*!< (sts, thm) Full stop control previous state. */
	int				battery_charge;			/*!< (sts, thm) keep or up battery charge. */
	long				battery_uv;			/*!< (sts, thm) battery voltage in micro volt. */
	long				battery_uv_suspend;		/*!< (sts, thm) battery voltage in micro volt at suspend. */
	int				battery_capacity;		/*!< (sts, thm) battery capacity. */
	int				battery_capacity_prev;		/*!< (sts, thm) previous battery capacity. */
	int				battery_capacity_notify_prev;	/*!< (thm) previous battery capacity at notify. */
	int				battery_safe_count;		/*!< (thm) checked battery charged enough N times. */
	uint32_t			battery_temp_raw;		/*!< (sts, thm) battery temperature(NTC pulldown) in register raw value. */
	uint32_t			battery_temp_raw_suspend;	/*!< (sts, thm) battery temperature(NTC pulldown) in register raw value in suspend. */
	int				battery_temp_state;		/*!< (sts, thm) battery boosting(charging) state by temperature. */
	int				battery_temp_state_prev;	/*!< (sts, thm) previous battery boosting(charging) state by temperature. */
	int				battery_temp_state_notify_prev;	/*!< (thm) previous battery boosting(charging) state by temperature at notify */
	int				vindpm_state;			/*!< (sts, thm) VINDPM state */
	int				vindpm_state_prev;		/*!< (sts, thm) VINDPM previous state. */
	int				vindpm_config;			/*!< (sts, thm) VINDPM config copied from state after configure. */

	struct power_supply_rate_limited	power_supply_rl[BQ24262_WMPORT_PSRL_NUM];

	unsigned long				wake_lock_source;	/*!< (wlk) wake_lock source. */
	unsigned long				wake_lock_flags;	/*!< (wlk) wake_lock current condition flags. */
	struct wake_lock			wake_lock_resume;	/*!< wake_lock at resume. */
	struct wake_lock			wake_lock_charging;	/*!< wake_lock on charging. */

	/* @note We may try queue a work_struct twice or more at a
	   same time. We assuming that WORK_STRUCT_PENDING_BIT in
	   delayed_work struct keeps none or one work in work_queue.
	*/

	struct work_struct	ppath_work;		/*!< Power Path interrupt work.			*/
	struct work_struct	charger_work;		/*!< Charger interrupt work.			*/
	struct task_struct	*thr_main;

	struct notifier_block			pm_nb;			/*!< pm notifier */
	struct notifier_block			gadget_nb;		/*!< usb gadget notifier */

	struct early_suspend es_info;	/* early suspend info */

	int					config;		/*!< (sts, thm) determined charger configuration number. */
	const struct bq24262_wmport_configs	*config_ptr;	/*!< (sts, thm) pointer to charger configuration. */

	unsigned int		power_source_irq;		/*!< (sts) power source bits, to notify power source thread. */
	unsigned int		power_source_raw_irq;		/*!< (sts) raw power source bits, to notify power source thread. */
	unsigned int		power_source_irq_prev;		/*!< (sts) previous state of power source bits, to notify power source thread. */
	unsigned int		power_source_raw_irq_prev;	/*!< (sts) previous state of raw power source bits, to notify power source thread. */

	unsigned int		power_source;			/*!< (sts, thm) power source bits at thread main. */
	unsigned int		power_source_raw;		/*!< (sts, thm) raw power source bits at thread main. */
	unsigned int		power_source_prev;		/*!< (sts, thm) previous state of power source bits in thread main context. */
	unsigned int		power_source_raw_prev;		/*!< (sts, thm) previous state of raw power source bits in thread main context. */
	unsigned int		power_source_usb_conn;		/*!< (thm) power source bits at connecting and disconnecting usb. */

	/* Thread wake event. */
	wait_queue_head_t	event_main;
	/* Thread start stop handshake event. */
	wait_queue_head_t	event_hs;
	/* Thread start stop handshake with diag event. */
	wait_queue_head_t	event_diag;

	unsigned long		jiffies_now;			/*!< (thm) jiffies now */
	unsigned long		jiffies_probe;			/*!< (thm) jiffies at probe. */
	unsigned long		jiffies_probe_expire;		/*!< (thm) jiffies to expire boot state from probe. */
	unsigned long		jiffies_sys_running;		/*!< (thm) jiffies at start init. */
	unsigned long		jiffies_sys_running_expire;	/*!< (thm) jiffies to expire boot state from start init. */
	unsigned long		jiffies_thr_main_wake_trace;	/*!< (thm) jiffies to trace high frequently wake. */

	int			charge_timer_state;		/*!< (sts) charge timer status.		*/
	long			charge_timer_max_sec;		/*!< (sts) max time of charge timer in seconds. */
	struct timespec		charge_timer_now_ts;		/*!< (sts) Realtime at charge now.	*/
	struct timespec		charge_timer_start_ts;		/*!< (sts) Realtime at charge start.	*/
	long			dynamic_period;			/*!< (sts) Discharging polling period while suspending. */

	unsigned long		ppath_work_wake_count;		/*!< (thv) power path work wake count. */
	unsigned long		thr_main_wake_count;		/*!< (thm) thread main wake count. */
	unsigned long		thr_main_wake_count_trace;	/*!< (thm) thread main wake count trace. */
	char			resume_req;			/*!< (thm) Resume requested. */
	char			suspend_req;			/*!< (thm) Suspend requested. */
	char			shutdown_req;			/*!< (thm) Shutdown requested. */
	char			diag_req;			/*!< (thm) Diag requested. */
	char			fully_weakly;			/*!< (sts, thm) Full voltage control, FULLY: fully charge /  WEAKLY: softy charge. */
	char			fully_weakly_prev;		/*!< (sts, thm) previous status. */
	char			usb_sus_mode;			/*!< (sts, thm) USB susped mode.  */
	bool			early_sus;			/*!< (sts, thm) early susped state.  */
	bool			ulp_mode;			/*!< (sts, thm) ulp mode.  */
	bool			usb_test_mode;		/*!< (sts, thm) USB test mode.  */
	int			fake_capacity;			/*!< test purpose, fake capacity. */
	int			diag_state;			/*!< (splock) DIAG: */
	int			diag_config;			/*!< (splock) DIAG: */
	int			diag_battery_charge;		/*!< (splock) DIAG: */
	unsigned long		diag_jiffies_to_off;		/*!< (splock) DIAG: */
	struct bq24262_wmport_vbus_context	vb;		/*!< VBUS thread context. */
};


/*! Unknown power spply type.
    @note define _UNKNWON as _UPS. \
          ICX platform is small gadget in hand.
          We can't connect ICX platform to UPS.
          type _UNKNWON only appear at boot.
*/
#define	POWER_SUPPLY_TYPE_UNKNOWN	POWER_SUPPLY_TYPE_UPS

typedef void bq24262_wmport_vbus_changed_call_t(void *_twl, unsigned ps);

struct bq24262_wmport_static_context {
	/* If you want to lock "sem" and "lock", lock "sem", then lock "lock"
	*/
	struct semaphore			sem;		/*!< semaphore for interruptible context.	*/
	struct bq24262_wmport_context		*bc;		/*!< (sem)					*/
	void					*arg;		/*!< (sem) "Changed" call back argument.	*/
	bq24262_wmport_vbus_changed_call_t	*vbus_changed;	/*!< (sem) "Changed" call back.		*/
	spinlock_t				lock;		/*!< spinlock for non-interruptible context.	*/
	struct bq24262_wmport_context		*bc_locked;	/*!< (lock) spinlocked driver context pointer.		*/
};

void bq24262_wmport_usb_test_mode(bool on);
void bq24262_wmport_usb_state_event(int usb_state);
int bq24262_wmport_usb_set_power_event(unsigned mA);
void bq24262_wmport_usb_set_gadget_enable_event(bool enable);
int bq24262_wmport_usb_set_max_power_event(unsigned mA);
int bq24262_wmport_usb_phy_off(int off);
bool bq24262_wmport_usb_cable_for_charge_connected(void);
int bq24262_wmport_get_dynamic_period(int first_use, int first_wakeup_time, int battery_capacity_level);

#endif /* BQ24296_WMPORT_H_INCLUDED */
