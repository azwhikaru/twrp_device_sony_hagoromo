/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* bq24262_wmport_vubs.c: BQ24262 battery charger with WM-PORT driver, thread VBUS part.
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

#include "bq24262_wmport_priv.h"
#include <linux/power/pmic_chr_type_det_icx.h>

/* Timeout milli seconds syncing with power supply thread at
   suspend, resume, and shutdown.
*/
#define	BQ24262_WMPORT_THR_VBUS_SYNC_SUSPEND_TIMEOUT_MS		(4000UL)
#define	BQ24262_WMPORT_THR_VBUS_SYNC_RESUME_TIMEOUT_MS		(4000UL)
#define	BQ24262_WMPORT_THR_VBUS_SYNC_SHUTDOWN_TIMEOUT_MS	(4000UL)

/*! Test and clear "is evented to thread VBUS".
    @param vb thread VBUS context.
*/
STATIC_FUNC int bq24262_wmport_thr_vbus_is_waken(struct bq24262_wmport_vbus_context *vb)
{	if (kthread_should_stop()) {
		return 1 /* should stop. */;
	}
	return test_and_clear_bit(BQ24262_WMPORT_THR_VBUS_WAKE, &(vb->flags));
}

/*! Test and clear "Some sync request to thread VBUS."
    @param vb thread VBUS context.
*/
STATIC_FUNC int bq24262_wmport_thr_vbus_catch_sync_req(struct bq24262_wmport_vbus_context *vb)
{	if (test_and_clear_bit(BQ24262_WMPORT_THR_VBUS_RESUME_REQ, &(vb->flags))) {
		/* Requested resume this thread. */
		vb->resume_req = 1;
	}

	if (test_and_clear_bit(BQ24262_WMPORT_THR_VBUS_SUSPEND_REQ, &(vb->flags))) {
		/* Requested suspend this thread. */
		vb->suspend_req = 1;
	}

	if (test_and_clear_bit(BQ24262_WMPORT_THR_VBUS_SHUTDOWN_REQ, &(vb->flags))) {
		/* Requested shutdown this thread. */
		vb->shutdown_req = 1;
	}

	return 0 /* No error. */;
}

/*! Acknowledge "Some sync request to thread VBUS."
    @param vb thread VBUS context.
*/
STATIC_FUNC int bq24262_wmport_thr_vbus_sync_ack(struct bq24262_wmport_vbus_context *vb)
{
	if (vb->resume_req) {
		/* Requested sync resume process with this thread. */
		vb->resume_req = 0;
		set_bit(BQ24262_WMPORT_THR_VBUS_RESUME_ACK, &(vb->flags));
		wake_up_interruptible(&(vb->event_hs));
	}
	if (vb->suspend_req) {
		/* Requested sync suspend process with this thread. */
		vb->suspend_req = 0;
		set_bit(BQ24262_WMPORT_THR_VBUS_SUSPEND_ACK, &(vb->flags));
		wake_up_interruptible(&(vb->event_hs));
	}
	if (vb->shutdown_req) {
		/* Requested sync shutdown process with this thread. */
		vb->shutdown_req = 0;
		set_bit(BQ24262_WMPORT_THR_VBUS_SHUTDOWN_ACK, &(vb->flags));
		wake_up_interruptible(&(vb->event_hs));
	}
	return 0 /* No error. */;
}

/*! Update battery charger (USB-AC) detection status - thread VBUS main sub function.
*/
STATIC_FUNC void bq24262_wmport_thr_vbus_bcdet_update(struct bq24262_wmport_vbus_context *vb, unsigned bcdet)
{	unsigned long	flags;

	spin_lock_irqsave(&(vb->lock),flags);
	vb->usb_bcdet_splocked = bcdet;
	spin_unlock_irqrestore(&(vb->lock),flags);
	down(&(vb_to_bc(vb)->usb_sem));
	vb_to_bc(vb)->usb_bcdet = bcdet;
	vb_to_bc(vb)->usb_state = BQ24262_WMPORT_USB_SUSPENDED;
	up(&(vb_to_bc(vb)->usb_sem));
}

/*! read bcdet Battery Charger(USB-AC adapter) detection result.
*/
STATIC_FUNC unsigned bq24262_wmport_thr_vbus_bcdet_read(struct bq24262_wmport_vbus_context *vb)
{	unsigned	result;
	unsigned long	flags;

	spin_lock_irqsave(&(vb->lock),flags);
	result = vb->usb_bcdet_splocked;
	spin_unlock_irqrestore(&(vb->lock),flags);
	return result;
}

/*! connect USB data line.
    @note Event to main thread in this function.
*/
STATIC_FUNC void bq24262_wmport_thr_vbus_usb_connect(struct bq24262_wmport_vbus_context *vb, unsigned bcdet)
{	bq24262_wmport_thr_vbus_bcdet_update(vb, bcdet);
	bq24262_wmport_thr_main_event(vb_to_bc(vb));
	switch (bcdet) {
		case ICX_CHARGER_CDP:
		case ICX_CHARGER_STD:
			/* There will be datas over DP and DM line. */
			/* @note I guess that musb_do_infra_misc(false) enable clock and power to USB block. */
			musb_do_infra_misc(false);
			/* @note if there is msleep, enter dpidle mode during this and disable usb clock and cannot recognize USB */
//			msleep(bq24262_wmport_vbus_usb_pll_wait_ms);
			/* Connect USB. */
			mt_usb_connect();
			break;
		default:
			/* do nothing. */
			break;
	}
}

/*! disconnect USB data line.
    @note Event to main thread in this function.
*/
STATIC_FUNC void bq24262_wmport_thr_vbus_usb_disconnect(struct bq24262_wmport_vbus_context *vb)
{	unsigned	bcdet;

	bcdet = bq24262_wmport_thr_vbus_bcdet_read(vb);
	bq24262_wmport_thr_vbus_bcdet_update(vb, ICX_CHARGER_UNKNOWN);
	bq24262_wmport_thr_main_event(vb_to_bc(vb));
	switch (bcdet) {
		case ICX_CHARGER_CDP:
		case ICX_CHARGER_STD:
			/* There are datas over DP and DM line. */
			/* Discoonect USB. */
			mt_usb_disconnect();
			/* @note if there is msleep, enter dpidle mode during this and disable usb clock and cannot recognize USB */
//			msleep(bq24262_wmport_vbus_usb_pll_wait_ms);
			/* @note I guess that musb_do_infra_misc(true) disable clock and power to USB block. */
			musb_do_infra_misc(true);
			break;
		default:
			/* do nothing. */
			break;
	}
}

#define	BQ24262_WMPORT_THR_VBUS_POLL_RUNNING_MS		(100)
#define	BQ24262_WMPORT_THR_VBUS_POLL_VBUS_MS		(200)
#define	BQ24262_WMPORT_THR_VBUS_POLL_VBUS_COUNT		(4)

/*! thread VBUS main.
    @param data points to thread VBUS context.
    @note this thread connect, disconnect, detect VBUS and DP, DM line.
*/
STATIC_FUNC int bq24262_wmport_thr_vbus_function(void *data)
{	struct bq24262_wmport_vbus_context	*vb;
	unsigned long				timeout_jiffies;
	const unsigned long			TIMEOUT_INFINITY=~0UL;
	unsigned				power_source_raw;
	unsigned				bcdet;

	vb = data;

	PRINTK_LI("%s: Thread vbus started.\n", __func__);
	set_bit(BQ24262_WMPORT_THR_VBUS_START ,&(vb->flags));

	/* Notify thread started. */
	wake_up_interruptible(&(vb->event_hs));
	while (test_bit(BQ24262_WMPORT_THR_VBUS_EXIT, &(vb->flags)) == 0) {
		/* Loop until "thread exited" flag turn into set(1). */
		timeout_jiffies = vb->jiffies_remain;
		if (vb->polling > 0) {
			timeout_jiffies = min(timeout_jiffies, msecs_to_jiffies(BQ24262_WMPORT_THR_VBUS_POLL_VBUS_MS));
			vb->polling--;
		}
		if (timeout_jiffies == TIMEOUT_INFINITY) {
			wait_event((vb->event_vbus),
				bq24262_wmport_thr_vbus_is_waken(vb)
			);
		} else {
			long	remain;
			if (timeout_jiffies != 0) {
				remain = wait_event_timeout((vb->event_vbus),
					bq24262_wmport_thr_vbus_is_waken(vb),
					timeout_jiffies
				);
				if (remain < 0) {
					pr_err("%s: Wait returns error. remain=%ld, state=%d\n",
						__func__, remain, vb->state
					);
					remain = TIMEOUT_INFINITY;
				}
				/* @note when polling > 0, jiffies_remain will be updated with zero mostly. */
				vb->jiffies_remain = remain;
			}
		}
		/* shot jiffies. */
		vb->jiffies_now = jiffies;

		if (kthread_should_stop()) {
			/* Some one stop me. */
			set_bit(BQ24262_WMPORT_THR_VBUS_EXIT, &(vb->flags));
		}

		bq24262_wmport_thr_vbus_catch_sync_req(vb);
		/* @todo read VCDT voltage, compare to enough to draw. */
		power_source_raw = bq24262_wmport_ppath_read_raw(vb_to_bc(vb));
		PRINTK_TV("%s: Thread waken. state=%d, jiffies_remain=%lu, polling=%d, ps_raw=0x%x\n",
			__func__,
			vb->state, vb->jiffies_remain, vb->polling, power_source_raw
		);
		switch (vb->state) {
			case BQ24262_WMPORT_VB_STATE_INIT:
				/* Start this thread. */
				clear_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags));
				vb->state = BQ24262_WMPORT_VB_STATE_INIT_WAIT_RUN;
				vb->jiffies_remain = msecs_to_jiffies(BQ24262_WMPORT_THR_VBUS_POLL_RUNNING_MS);
				vb->polling = 0;
				break;
			case BQ24262_WMPORT_VB_STATE_INIT_WAIT_RUN:
				/* Polling system be running (will call userland init). */
				clear_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags));
				if (system_state != SYSTEM_RUNNING) {
					/* Not running. */
					vb->state = BQ24262_WMPORT_VB_STATE_INIT_WAIT_RUN;
					if (vb->jiffies_remain == 0) {
						/* Timer expired. */
						/* Restart polling timer. */
						vb->jiffies_remain = msecs_to_jiffies(BQ24262_WMPORT_THR_VBUS_POLL_RUNNING_MS);
					}
				} else {
					/* Running. */
					PRINTK_LI("%s: Keep USB disconnected state at init.\n", __func__);
					vb->state = BQ24262_WMPORT_VB_STATE_INIT_DISCONNECT;
					vb->jiffies_remain = TIMEOUT_INFINITY;
					vb->polling = 0;
				}
				break;
			case BQ24262_WMPORT_VB_STATE_INIT_DISCONNECT:
				/* 1st disconnect state. */
				clear_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags));
				if (test_bit(BQ24262_WMPORT_THR_VBUS_GADGET_READY, &(vb->flags)) == 0) {
					/* Gadget driver(s) is(are) not ready. */
					/* Continue disconnect state. */
					vb->state = BQ24262_WMPORT_VB_STATE_INIT_DISCONNECT;
					vb->jiffies_remain = TIMEOUT_INFINITY;
					vb->polling = 0;
					break;
				}
				PRINTK_LI("VBUS gadget ready.\n");
				/* gadget driver(s) is(are) become ready, end 1st disconnect state. */
				if ((power_source_raw & BQ24262_WMPORT_USB) != 0) {
					/* VBUS is powered. */
					vb->state = BQ24262_WMPORT_VB_STATE_CONNECT_DEBOUNCE;
					vb->jiffies_remain = msecs_to_jiffies(bq24262_wmport_vbus_debounce_poll_ms);
					vb->polling = 0;
					vb->debounce_count = bq24262_wmport_vbus_debounce_poll_count;
				} else {
					/* VBUS is not powered. */
					vb->state = BQ24262_WMPORT_VB_STATE_USB_DISCONNECT;
					vb->jiffies_remain = 0;
					vb->polling = BQ24262_WMPORT_THR_VBUS_POLL_VBUS_COUNT;
				}
				break;
			case BQ24262_WMPORT_VB_STATE_CONNECT_DEBOUNCE:
				if(test_and_clear_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags))){
					PRINTK_LI("clear debounce count\n");
					vb->state = BQ24262_WMPORT_VB_STATE_CONNECT_DEBOUNCE;
					vb->jiffies_remain = msecs_to_jiffies(bq24262_wmport_vbus_debounce_poll_ms);
					vb->polling = 0;
					vb->debounce_count = bq24262_wmport_vbus_debounce_poll_count;
					break;
				}
				if ((power_source_raw & BQ24262_WMPORT_USB) == 0) {
					/* VBUS is not powered, unexpected disconnect. */
					vb->state = BQ24262_WMPORT_VB_STATE_USB_DISCONNECT;
					vb->jiffies_remain = 0;
					vb->polling = BQ24262_WMPORT_THR_VBUS_POLL_VBUS_COUNT;
					vb->debounce_count = 0;
					break;
				}
				if (vb->jiffies_remain > 0) {
					/* Remains debounce timer. */
					/* keep this state. */
					vb->polling = 0;
					break;
				}
				if (vb->debounce_count > 0) {
					/* Check VBUS line more. */
					vb->state = BQ24262_WMPORT_VB_STATE_CONNECT_DEBOUNCE;
					vb->jiffies_remain = msecs_to_jiffies(bq24262_wmport_vbus_debounce_poll_ms);
					vb->polling = 0;
					vb->debounce_count--;
					break;
				}
				/* Complete debounce. */
				vb->state = BQ24262_WMPORT_VB_STATE_CONTACT_DPDM;
				vb->jiffies_remain = msecs_to_jiffies(bq24262_wmport_dpdm_contact_poll_ms);
				vb->polling = 0;
				vb->debounce_count = 0;
				vb->contact_ready = 0;
				/* Switch DP and DM line to PMIC MT6323. */
				mt6323_bcdet_init();
				break;
			case BQ24262_WMPORT_VB_STATE_CONTACT_DPDM:
				if(test_and_clear_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags))){
					PRINTK_LI("return debounce state\n");
					vb->state = BQ24262_WMPORT_VB_STATE_CONNECT_DEBOUNCE;
					vb->jiffies_remain = msecs_to_jiffies(bq24262_wmport_vbus_debounce_poll_ms);
					vb->polling = 0;
					vb->debounce_count = bq24262_wmport_vbus_debounce_poll_count;
					mt6323_bcdet_done();
					break;
				}
				if ((power_source_raw & BQ24262_WMPORT_USB) == 0) {
					/* VBUS is not powered, unexpected disconnect. */
					vb->state = BQ24262_WMPORT_VB_STATE_USB_DISCONNECT;
					vb->jiffies_remain = 0;
					vb->polling = BQ24262_WMPORT_THR_VBUS_POLL_VBUS_COUNT;
					vb->debounce_count = 0;
					vb->contact_ready = 0;
					/* Switch DP and DM line to USB controller phy.  */
					mt6323_bcdet_done();
					break;
				}
				if (vb->jiffies_remain > 0) {
					/* Remains debounce timer. */
					/* keep this state. */
					vb->polling = 0;
					break;
				}
				/* Check VBUS line more. */
				if (mt6323_bcdet_is_contact_dpdm()) {
					vb->contact_ready++;
				} else {
					if (vb->contact_ready > 0) {
						PRINTK_LI("%s: Unstable contact. contact_ready=%d\n", __func__, vb->contact_ready);
					}
					vb->contact_ready = 0;
				}
				if (vb->contact_ready < bq24262_wmport_dpdm_contact_poll_ready) {
					vb->state = BQ24262_WMPORT_VB_STATE_CONTACT_DPDM;
					vb->jiffies_remain = msecs_to_jiffies(bq24262_wmport_dpdm_contact_poll_ms);
					vb->polling = 0;
					vb->debounce_count = 0;
					break;
				}
				/* Complete contact. */
				vb->state = BQ24262_WMPORT_VB_STATE_CHARGER_DETECT;
				vb->jiffies_remain = 0;
				vb->polling = 0;
				vb->debounce_count = 0;
				vb->contact_ready = 0;
				/* Switch DP and DM line to USB controller phy.  */
				mt6323_bcdet_done();
				break;

			case BQ24262_WMPORT_VB_STATE_CHARGER_DETECT:
				if(test_and_clear_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags))){
					PRINTK_LI("return debounce state\n");
					vb->state = BQ24262_WMPORT_VB_STATE_CONNECT_DEBOUNCE;
					vb->jiffies_remain = msecs_to_jiffies(bq24262_wmport_vbus_debounce_poll_ms);
					vb->polling = 0;
					vb->debounce_count = bq24262_wmport_vbus_debounce_poll_count;
					break;
				}
				if ((power_source_raw & BQ24262_WMPORT_USB) == 0) {
					/* VBUS is not powered, unexpected disconnect. */
					vb->state = BQ24262_WMPORT_VB_STATE_USB_DISCONNECT;
					vb->jiffies_remain = 0;
					vb->polling = BQ24262_WMPORT_THR_VBUS_POLL_VBUS_COUNT;
					break;
				}
				bcdet = mt6323_bcdet_type_detection();
				if(test_and_clear_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags))){
					PRINTK_LI("return debounce state\n");
					vb->state = BQ24262_WMPORT_VB_STATE_CONNECT_DEBOUNCE;
					vb->jiffies_remain = msecs_to_jiffies(bq24262_wmport_vbus_debounce_poll_ms);
					vb->polling = 0;
					vb->debounce_count = bq24262_wmport_vbus_debounce_poll_count;
					break;
				}
				PRINTK_LI("%s: Connect USB. bcdet=%u(%s)\n", __func__, bcdet, bq24262_wmport_bcdet_names[bcdet]);
				bq24262_wmport_thr_vbus_usb_connect(vb, bcdet);
				vb->state = BQ24262_WMPORT_VB_STATE_USB_CONNECT;
				vb->jiffies_remain = 0;
				vb->polling = BQ24262_WMPORT_THR_VBUS_POLL_VBUS_COUNT;
				break;
			case BQ24262_WMPORT_VB_STATE_USB_CONNECT:
				clear_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags));
				if ((power_source_raw & BQ24262_WMPORT_USB) != 0) {
					/* VBUS is powered. */
					/* Keep connect. */
					vb->state = BQ24262_WMPORT_VB_STATE_USB_CONNECT;
					vb->jiffies_remain = TIMEOUT_INFINITY;
					break;
				}
				/* VBUS is not powered, disconnect. */
				PRINTK_LI("%s: Disconnect USB.\n", __func__);
				bq24262_wmport_thr_vbus_usb_disconnect(vb);
				vb->state = BQ24262_WMPORT_VB_STATE_USB_DISCONNECT;
				vb->jiffies_remain = 0;
				vb->polling = BQ24262_WMPORT_THR_VBUS_POLL_VBUS_COUNT;
				break;
			case BQ24262_WMPORT_VB_STATE_USB_DISCONNECT:
				clear_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags));
				if ((power_source_raw & BQ24262_WMPORT_USB) == 0) {
					/* VBUS is not powered. */
					/* Keep disconnect. */
					vb->state = BQ24262_WMPORT_VB_STATE_USB_DISCONNECT;
					vb->jiffies_remain = TIMEOUT_INFINITY;
					break;
				}
				/* VBUS is powered, connect. */
				vb->state = BQ24262_WMPORT_VB_STATE_CONNECT_DEBOUNCE;
				vb->jiffies_remain = msecs_to_jiffies(bq24262_wmport_vbus_debounce_poll_ms);
				vb->polling = 0;
				vb->debounce_count = bq24262_wmport_vbus_debounce_poll_count;
				break;
			default:
				pr_err("%s: ERROR: Unexpected state. state=%d\n", __func__, vb->state);
				clear_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags));
				bq24262_wmport_thr_vbus_bcdet_update(vb, ICX_CHARGER_UNKNOWN);
				bq24262_wmport_thr_vbus_usb_disconnect(vb);
				vb->state = BQ24262_WMPORT_VB_STATE_USB_DISCONNECT;
				vb->jiffies_remain = 0;
				break;
		}

		bq24262_wmport_thr_vbus_sync_ack(vb);

		if (test_bit(BQ24262_WMPORT_THR_VBUS_TERMINATE, &(vb->flags))) {
			/* Requested terminate this thread. */
			set_bit(BQ24262_WMPORT_THR_VBUS_EXIT, &(vb->flags));
		}
	}
	switch (vb->state) {
		case BQ24262_WMPORT_VB_STATE_CONTACT_DPDM:
			/* Switch DP and DM line to USB controller phy.  */
			mt6323_bcdet_done();
			break;
		default:
			/* Do nothing. */
			break;
	}
	/* Notify thread exited. */
	wake_up_interruptible(&(vb->event_hs));
	PRINTK_LI("%s: Thread reaches end of function.\n",__func__);
	return 0;
}

/*! Issue event to thread VBUS.
    @param vb thread VBUS context.
*/
void bq24262_wmport_thr_vbus_event(struct bq24262_wmport_vbus_context *vb)
{	set_bit(BQ24262_WMPORT_THR_VBUS_WAKE, &(vb->flags));
	wake_up(&(vb->event_vbus));
}

/*! Prepare thread VBUS to suspend.
    @param vb thread VUBS context.
    @return int negative errno number in kernel.
*/
int bq24262_wmport_thr_vbus_suspend(struct bq24262_wmport_vbus_context *vb)
{	int		ret;

	set_bit(BQ24262_WMPORT_THR_VBUS_SUSPEND_REQ, &(vb->flags));
	bq24262_wmport_thr_vbus_event(vb);
	/* Wait event thread synced. */
	ret = wait_event_interruptible_timeout(
		vb->event_hs,
		test_and_clear_bit(BQ24262_WMPORT_THR_VBUS_SUSPEND_ACK, &(vb->flags)),
		msecs_to_jiffies(BQ24262_WMPORT_THR_VBUS_SYNC_SUSPEND_TIMEOUT_MS)
	);
	if (ret <= 0) {
		pr_err("%s: Can not sync thread. ret=%d.\n",__func__, ret);
		return ret;
	} else {
		PRINTK_TV(KERN_INFO "%s: Sync with thread VBUS. ret=%d.\n",__func__, ret);
	}
	return 0 /* Success. */;
}

/*! Prepare thread VBUS to shutdown.
    @param vb charger context.
    @return int negative errno number in kernel.
*/
int bq24262_wmport_thr_vbus_shutdown(struct bq24262_wmport_vbus_context *vb)
{	int		ret;

	set_bit(BQ24262_WMPORT_THR_VBUS_SHUTDOWN_REQ, &(vb->flags));
	bq24262_wmport_thr_vbus_event(vb);
	/* Wait event thread synced. */
	ret = wait_event_interruptible_timeout(
		vb->event_hs,
		test_and_clear_bit(
			BQ24262_WMPORT_THR_VBUS_SHUTDOWN_ACK,
			&(vb->flags)
		),
		msecs_to_jiffies(BQ24262_WMPORT_THR_VBUS_SYNC_SHUTDOWN_TIMEOUT_MS)
	);
	if (ret <= 0) {
		pr_err("%s: Can not sync thread. ret=%d.\n",__func__, ret);
		return ret;
	} else {
		PRINTK_TV(KERN_INFO "%s: Sync with thread VBUS. ret=%d.\n",__func__, ret);
	}
	return 0 /* Success. */;
}


/*! Sync with thread VBUS at resume.
    @param vb thread VBUS context.
    @return int negative errno number in kernel.
*/
int bq24262_wmport_thr_vbus_resume(struct bq24262_wmport_vbus_context *vb)
{	int		ret;

	set_bit(BQ24262_WMPORT_THR_VBUS_RESUME_REQ, &(vb->flags));
	bq24262_wmport_thr_vbus_event(vb);
	/* Wait event thread synced. */
	ret = wait_event_interruptible_timeout(
		vb->event_hs,
		test_and_clear_bit(BQ24262_WMPORT_THR_VBUS_RESUME_ACK, &(vb->flags)),
		msecs_to_jiffies(BQ24262_WMPORT_THR_VBUS_SYNC_RESUME_TIMEOUT_MS)
	);
	if (ret <= 0) {
		pr_err("%s: Can not sync thread. ret=%d.\n",__func__, ret);
		return ret;
	}
	PRINTK_TV("%s: Sync with thread vbus. ret=%d\n",__func__, ret);
	return 0 /* Success. */;
}


/*! Terminate ICX platform power source VBUS thread.
    @param vb thread VBUS context.
    @return int negative errno number in kernel.
*/
int bq24262_wmport_thr_vbus_terminate(struct bq24262_wmport_vbus_context *vb)
{	int		ret;
	int		already_terminated;

	already_terminated = test_bit(BQ24262_WMPORT_THR_VBUS_EXIT, &(vb->flags));
	set_bit(BQ24262_WMPORT_THR_VBUS_TERMINATE, &(vb->flags));
	if (already_terminated) {
		return 0 /* Success */;
	}
	bq24262_wmport_thr_vbus_event(vb);
	/* Wait event thread exited. */
	PRINTK_LI("%s: Terminate thread vbus.\n", __func__);
	ret = wait_event_interruptible(
		vb->event_hs,
		test_bit(BQ24262_WMPORT_THR_VBUS_EXIT, &(vb->flags))
	);
	if (ret) {
		pr_err("%s: Can not exit thread. ret=%d.\n",__func__, ret);
		return ret;
	}
	PRINTK_LI(KERN_INFO "%s: Terminated thread VBUS.\n",__func__);
	return 0 /* Success. */;
}

/*! Prepare to start thread VBUS.
    @param vb thread VBUS context.
    @return int ==0: Success, <0: Failed, negative errno number.
*/
int bq24262_wmport_thr_vbus_prepare_start(struct bq24262_wmport_vbus_context *vb)
{	unsigned long	jnow;

	jnow = jiffies;

	spin_lock_init(&(vb->lock));
	vb->state = BQ24262_WMPORT_VB_STATE_INIT;
	vb->jiffies_now = jnow;
	vb->jiffies_remain = 0;
	vb->polling = 0;
	vb->debounce_count = 0;
	vb->contact_ready = 0;
	vb->resume_req = 0;
	vb->suspend_req = 0;
	vb->shutdown_req = 0;
	vb->usb_bcdet_splocked = ICX_CHARGER_UNKNOWN;
	init_waitqueue_head(&(vb->event_vbus));
	init_waitqueue_head(&(vb->event_hs));

	return 0 /* Success. */;
}

/*! Wake thread VBUS by gadget ready.
*/
void bq24262_wmport_thr_vbus_gadget_ready(struct bq24262_wmport_vbus_context *vb)
{	PRINTK_TV("%s: Called.\n",__func__);
	set_bit(BQ24262_WMPORT_THR_VBUS_GADGET_READY, &(vb->flags));
	bq24262_wmport_thr_vbus_event(vb);
	return;
}

/*! Wake thread VBUS by PHY on/off.
*/
void bq24262_wmport_thr_vbus_phy_off(struct bq24262_wmport_vbus_context *vb, int off)
{	PRINTK_TV("%s: Called.\n",__func__);
	PRINTK_LI("PHY %s\n",off?"off":"on");
	set_bit(BQ24262_WMPORT_THR_VBUS_PHY_CHG, &(vb->flags));
	bq24262_wmport_thr_vbus_event(vb);
	return;
}

static char bq24262_wmport_thr_vbus_name[]="wmport_vbus";

/*! Start thread VBUS.
    @param vb thread vbus.
    @return int ==0: Success, <0: Failed, negative errno number.
*/
int bq24262_wmport_thr_vbus_start(struct bq24262_wmport_vbus_context *vb)
{	int			ret;
	struct task_struct	*thread;

	PRINTK_LI("%s: Start thread VBUS.\n",__func__);
	thread = kthread_create(
		bq24262_wmport_thr_vbus_function,
		vb,
		bq24262_wmport_thr_vbus_name
	);
	vb->thr_vbus = thread;
	if (!thread) {
		pr_err("%s: Can not create thread.\n",__func__);
		ret = -ENOMEM;
		goto out;
	}
	ret = wake_up_process(thread);
	if (!ret) {
		pr_err("%s: Can not start thread. ret=%d.\n",__func__, ret);
		set_bit(BQ24262_WMPORT_THR_VBUS_EXIT, &(vb->flags));
		goto out;
	}

	ret = wait_event_interruptible(
		vb->event_hs,
		test_and_clear_bit(
			BQ24262_WMPORT_THR_VBUS_START,
			&(vb->flags)
		)
	);
	if (ret) {
		pr_err("%s: Can not start thread. ret=%d.\n",__func__, ret);
		goto out;
	}
	/* Started thread and waiting event. */
	return 0 /* Success. */;
out:
	set_bit(BQ24262_WMPORT_THR_VBUS_EXIT, &(vb->flags));
	bq24262_wmport_thr_vbus_terminate(vb);
	return ret;
}
