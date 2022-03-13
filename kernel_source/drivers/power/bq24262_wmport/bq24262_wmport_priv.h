/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* bq24262_wmport_priv.h: BQ24262 battery charger with WM-PORT driver, private header.
 *
 * Copyright 2015 Sony Corporation.
 * Author: Sony Corporation.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


/*! Enables deb_dbg print out.
    define: Enable deb_dbg print out.
    undef:  Disable deb_dbg print out.
*/
/* #define	DEBUG */

#include <linux/kernel.h>
#include <linux/bitops.h>
#include <asm/div64.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/power/bq24262.h>
#include <linux/power/bq24262_wmport.h>
#include <linux/power/pmic_chr_type_det_icx.h>
#include <linux/wakelock.h>
#include <linux/suspend.h>
#include <linux/usb/otg.h>
#include <linux/ctype.h>
#include <linux/earlysuspend.h>

#include "mach/mt_gpio.h"
#include "mach/eint.h"
#include "mach/charging.h"
#include "mach/battery_common.h"	/* usb_state_enum, musb_do_infra_misc */
#include "mach/battery_meter_hal.h"	/* MT6323 GPADC function. */
#include "cust_battery_meter.h"		/* MT6323 GPADC battery channel. */
#include "mach/icx_pm_helper.h"
#include "mach/bq24262_platform.h"

extern void mt_usb_connect(void);
extern void mt_usb_disconnect(void);

#define BQWMPTAG "BQWMP: "

#define	DEVICE_NAME_TO_PROBE	"bq24262_wmport"

/*! function static modifier warapper
*/
#define	STATIC_FUNC	/* static */

/*! variable static modifier warapper
*/
#define	STATIC_DATA	/* static */

/* Debug control flag.
   0x0001 1: Enable Light weight print, 0: Disable.
   0x0002 1: Enable Function entry/exit print, 0: Disable.
   0x0004 1: Enable Printk IRQ entry, 0: Disable.
   0x0008 1: Enable Heavy weight print, 0: Disable.
   0x0010 1: Enable Printk GPADC reads. 0: Disable.
   0x0020 1: Enable Printk BATTERY reads. 0: Disable.
   0x0040 1: Enable Printk at Periodic event. 0: Disable.
   0x0080 1: Enable Printk frequently message in main thread. 0: Disable.
   0x0100 1: Enable Printk status. 0: Disable.
   0x0200 1: Enable Printk configuration registers. 0: Disable.
   0x0400 Reserved.
   0x0800 1: Ignore Battery temperature. 0: Watch Battery temperature.
   0x1000 1: Enable Printk configuration registers at no changes made. 0: Disable.
   0x2000 1: Enable Printk frequently message in VBUS thread, 0: Disable.
   0x4000 1: Enable Printk wake_lock message, 0: Disable.
*/

#define	DEBUG_FLAG_LIGHT		(0x0001)
#define	DEBUG_FLAG_ENTRY		(0x0002)
#define	DEBUG_FLAG_IRQ			(0x0004)
#define	DEBUG_FLAG_HEAVY		(0x0008)
#define	DEBUG_FLAG_GPADC_READS		(0x0010)
#define	DEBUG_FLAG_BATTERY_READS	(0x0020)
#define	DEBUG_FLAG_PERIODIC		(0x0040)
#define	DEBUG_FLAG_THR_MAIN		(0x0080)
#define	DEBUG_FLAG_STATUS		(0x0100)
#define	DEBUG_FLAG_CONFIG		(0x0200)
#define	DEBUG_FLAG_XXX_NOT_USED		(0x0400)	/* Reserved. */
#define	DEBUG_FLAG_IGNORE_BATTERY_TEMP	(0x0800)	/* this flag is disabled in normal driver, see DEBUG_IGNORE_BATTERY_TEMP_ENABLE.  */
#define	DEBUG_FLAG_KEEP_CONFIG		(0x1000)
#define	DEBUG_FLAG_THR_VBUS		(0x2000)
#define	DEBUG_FLAG_WAKE_LOCK		(0x4000)
#define	DEBUG_FLAG_XX			(0x80000000)

/* Debug: Ignore Battery temperature.
   Normal: (Release default) Define as (0)
   Debug:  Define as (1)
*/
#define	DEBUG_IGNORE_BATTERY_TEMP_ENABLE	(0)

/*! Live debug control. */
extern uint bq24262_wmport_debug;

extern uint	bq24262_wmport_i2c_write_ms;
extern uint	bq24262_wmport_i2c_clk_khz;

extern uint	bq24262_wmport_vbus_debounce_poll_ms;
extern uint	bq24262_wmport_vbus_debounce_poll_count;

extern uint	bq24262_wmport_dpdm_contact_poll_ms;
extern uint	bq24262_wmport_dpdm_contact_poll_ready;
extern uint	bq24262_wmport_vbus_usb_pll_wait_ms;

/*! Debug: Do printk at function entry/exit.
    define: Do printk at function entry/exit.
    undef: Do NOT printk at function entry/exit.
*/
#define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_FUNC_ENTRY

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_FUNC_ENTRY))
#define	PRINTK_FUNC_ENTRY(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_ENTRY) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_FUNC_ENTRY(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Do printk at IRQ entry/exit.
    define: Do printk at IRQ entry/exit.
    undef: Do NOT printk at function entry/exit.
*/
#define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_IRQ_ENTRY

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_IRQ_ENTRY))
#define	PRINTK_IRQ_ENTRY(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_IRQ) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_IRQ_ENTRY(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Do light weight printk.
    define: Do light weight printk.
    undef:  Do not light weight printk.
*/
#define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_LIGHT_WEIGHT	(1)

#if (defined(CONFIG_ICX_MMC_TEST))
#define	PRINTK_LI(format, ...)
#else
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_LIGHT_WEIGHT))
#define	PRINTK_LI(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_LIGHT) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_LI(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif
#endif

/*! Debug: Do heavy weight printk.
    define: Do heavy weight printk.
    undef:  Do not heavy weight printk.
*/
#define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_HEAVY_WEIGHT	(1)

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_HEAVY_WEIGHT))
#define	PRINTK_HV(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_HEAVY) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_HV(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Do Printk GPADC reads.
    define: Do Printk GPADC reads.
    undef:  Do not Printk GPADC reads.
*/
#define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_GPADC_READS	(1)

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_GPADC_READS))
#define	PRINTK_GPADC_R(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_GPADC_READS) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_GPADC_R(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Do Printk BATTERY reads.
    define: Do Printk BATTERY reads.
    undef:  Do not Printk BATTERY reads.
*/
#define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_BATTERY_READS	(1)

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_BATTERY_READS))
#define	PRINTK_BATTERY_R(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_BATTERY_READS) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_BATTERY_R(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Do Printk at periodic events.
    define: Do Printk BATTERY reads.
    undef:  Do not Printk BATTERY reads.
*/
#define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_PERIODIC		(1)

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_PERIODIC))
#define	PRINTK_PERIODIC(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_PERIODIC) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_PERIODIC(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Do Printk at power thread activity.
    define: Do Printk power thread activity.
    undef:  Do not Printk power thread activity.
*/
#define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_THREAD_MAIN		(1)

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_THREAD_MAIN))
#define	PRINTK_TM(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_THR_MAIN) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_TM(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Do Printk at thread vbus activity.
    define: Do Printk thread vbus activity.
    undef:  Do not Printk power thread vbus.
*/
#define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_THREAD_VBUS		(1)

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_THREAD_VBUS))
#define	PRINTK_TV(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_THR_VBUS) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_TV(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Show wake lock operations.
           Do printk only under fixing issue.
           It produce messy message.
    define: Do Printk at wake lock operations.
    undef:  Do not Printk at wake lock operations.
*/
#define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_WL			(1) */

#if (defined(CONFIG_ICX_MMC_TEST))
#define	PRINTK_WL(format, ...)
#else
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_WL))
#define	PRINTK_WL(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_WAKE_LOCK) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_WL(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif
#endif


/*! Debug: Show flow marker.
           Do printk only under fixing issue.
           It produce messy message.
    define: Do Printk only under fixing issue.
    undef:  (Release Default) Do not Printk, do not produce messy message.
*/
/* #define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_XX			(1) */

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_XX))
#define	PRINTK_XX(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_XX) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_XX(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Debug: Show periodic polling marker.
           Do printk only under fixing issue.
           It produce messy message.
    define: Do Printk only under fixing issue.
    undef:  (Release Default) Do not Printk, do not produce messy message.
*/
/* #define	CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_XP			(1) */

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT_DEBUG_XP))
#define	PRINTK_XP(format, ...) \
	do {\
		if (bq24262_wmport_debug&DEBUG_FLAG_XX) {\
			printk(BQWMPTAG format, ##__VA_ARGS__);\
		}\
	} while (0)
#else
#define	PRINTK_XP(format, ...) do {if (0) {printk(format, ##__VA_ARGS__);}} while (0)
#endif

/*! Statically keeped device information. */
extern struct bq24262_wmport_static_context	bq24262_wmport_static;

#define	MTK_I2C_READ_WRITE_LEN(read_len, write_len) \
	(((read_len) << 8) | ((write_len) << 0))

#define	vb_to_bc(vbus_context) (container_of((vbus_context), struct bq24262_wmport_context, vb))
#define	bc_to_vb(battery_charger_context) (&(bc->vb))

/* bq24262_wmport.c */
extern int bq24262_wmport_i2c_read(struct bq24262_wmport_context *bc, uint8_t reg_addr, uint8_t *val);
extern int bq24262_wmport_i2c_write(struct bq24262_wmport_context *bc, uint8_t reg_addr, uint8_t val);
extern long bq24262_wmport_vbat_read(struct bq24262_wmport_context *bc);
extern uint32_t bq24262_wmport_temp_read(struct bq24262_wmport_context *bc);
extern unsigned bq24262_wmport_ppath_read_raw(struct bq24262_wmport_context *bc);
extern unsigned bq24262_wmport_ppath_read(struct bq24262_wmport_context *bc, unsigned *raw);
extern void bq24262_wmport_force_dcin_write(struct bq24262_wmport_context *bc, unsigned force_dcin);
extern unsigned bq24262_wmport_force_dcin_read(struct bq24262_wmport_context *bc);
extern unsigned bq24262_wmport_chg_xstat_read(struct bq24262_wmport_context *bc);
extern void bq24262_wmport_chg_suspend_write(struct bq24262_wmport_context *bc, unsigned chg_suspend);
extern unsigned bq24262_wmport_chg_suspend_read(struct bq24262_wmport_context *bc);
extern void bq24262_wmport_sys_wak_stat_write(struct bq24262_wmport_context *bc, unsigned wak_stat);
extern unsigned bq24262_wmport_sys_wak_stat_read(struct bq24262_wmport_context *bc);

extern int bq24262_wmport_charger_irq_disable(struct bq24262_wmport_context *bc);
extern int bq24262_wmport_charger_irq_enable_unlocked(struct bq24262_wmport_context *bc);
extern int bq24262_wmport_charger_irq_enable(struct bq24262_wmport_context *bc);
extern void bq24262_wmport_charger_work_activate(struct bq24262_wmport_context *bc);
extern void bq24262_wmport_charger_work_shut(struct bq24262_wmport_context *bc);
extern int bq24262_wmport_charger_work_cancel(struct bq24262_wmport_context *bc);
extern void bq24262_wmport_charger_work_do_retrigger(struct bq24262_wmport_context *bc);
extern void bq24262_wmport_charger_work_launch(struct bq24262_wmport_context *bc);

int bq24262_wmport_ppath_irq_enable(struct bq24262_wmport_context *bc);
void bq24262_wmport_ppath_irq_disable(struct bq24262_wmport_context *bc);
void bq24262_wmport_ppath_work_activate(struct bq24262_wmport_context *bc);
void bq24262_wmport_ppath_work_shut(struct bq24262_wmport_context *bc);
int bq24262_wmport_ppath_work_cancel(struct bq24262_wmport_context *bc);
void bq24262_wmport_ppath_work_do_retrigger(struct bq24262_wmport_context *bc);
void bq24262_wmport_ppath_work_launch(struct bq24262_wmport_context *bc);

void bq24262_wmport_thr_main_event(struct bq24262_wmport_context *bc);

/* bq24262_wmport_vbus.c */
void bq24262_wmport_thr_vbus_gadget_ready(struct bq24262_wmport_vbus_context *vb);
void bq24262_wmport_thr_vbus_phy_off(struct bq24262_wmport_vbus_context *vb, int off);
void bq24262_wmport_thr_vbus_event(struct bq24262_wmport_vbus_context *vb);
int bq24262_wmport_thr_vbus_suspend(struct bq24262_wmport_vbus_context *vb);
int bq24262_wmport_thr_vbus_shutdown(struct bq24262_wmport_vbus_context *vb);
int bq24262_wmport_thr_vbus_resume(struct bq24262_wmport_vbus_context *vb);
int bq24262_wmport_thr_vbus_terminate(struct bq24262_wmport_vbus_context *vb);
int bq24262_wmport_thr_vbus_prepare_start(struct bq24262_wmport_vbus_context *vb);
int bq24262_wmport_thr_vbus_start(struct bq24262_wmport_vbus_context *vb);

