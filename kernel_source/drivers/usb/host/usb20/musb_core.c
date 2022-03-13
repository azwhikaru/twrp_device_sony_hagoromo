/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * MUSB OTG driver core code
 *
 * Copyright 2005 Mentor Graphics Corporation
 * Copyright (C) 2005-2006 by Texas Instruments
 * Copyright (C) 2006-2007 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Inventra (Multipoint) Dual-Role Controller Driver for Linux.
 *
 * This consists of a Host Controller Driver (HCD) and a peripheral
 * controller driver implementing the "Gadget" API; OTG support is
 * in the works.  These are normal Linux-USB controller drivers which
 * use IRQs and have no dedicated thread.
 *
 * This version of the driver has only been used with products from
 * Texas Instruments.  Those products integrate the Inventra logic
 * with other DMA, IRQ, and bus modules, as well as other logic that
 * needs to be reflected in this driver.
 *
 *
 * NOTE:  the original Mentor code here was pretty much a collection
 * of mechanisms that don't seem to have been fully integrated/working
 * for *any* Linux kernel version.  This version aims at Linux 2.6.now,
 * Key open issues include:
 *
 *  - Lack of host-side transaction scheduling, for all transfer types.
 *    The hardware doesn't do it; instead, software must.
 *
 *    This is not an issue for OTG devices that don't support external
 *    hubs, but for more "normal" USB hosts it's a user issue that the
 *    "multipoint" support doesn't scale in the expected ways.  That
 *    includes DaVinci EVM in a common non-OTG mode.
 *
 *      * Control and bulk use dedicated endpoints, and there's as
 *        yet no mechanism to either (a) reclaim the hardware when
 *        peripherals are NAKing, which gets complicated with bulk
 *        endpoints, or (b) use more than a single bulk endpoint in
 *        each direction.
 *
 *        RESULT:  one device may be perceived as blocking another one.
 *
 *      * Interrupt and isochronous will dynamically allocate endpoint
 *        hardware, but (a) there's no record keeping for bandwidth;
 *        (b) in the common case that few endpoints are available, there
 *        is no mechanism to reuse endpoints to talk to multiple devices.
 *
 *        RESULT:  At one extreme, bandwidth can be overcommitted in
 *        some hardware configurations, no faults will be reported.
 *        At the other extreme, the bandwidth capabilities which do
 *        exist tend to be severely undercommitted.  You can't yet hook
 *        up both a keyboard and a mouse to an external USB hub.
 */

/*
 * This gets many kinds of configuration information:
 *	- Kconfig for everything user-configurable
 *	- platform_device for addressing, irq, and platform_data
 *	- platform_data is mostly for board-specific informarion
 *	  (plus recentrly, SOC or family details)
 *
 * Most of the conditional compilation will (someday) vanish.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/kobject.h>
#include <linux/prefetch.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/idr.h>
#include <linux/dma-mapping.h>

#include <linux/musb/musb_core.h>
#include <linux/musb/musbhsdma.h>
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
/* @note CONFIG_CHARGER_BQ24262_WMPORT is defined, then CONFIG_ARCH_MT8590_ICX is also defined. */
#include <linux/power/bq24262_wmport.h>
#include <linux/workqueue.h>
#include <linux/switch.h>
static const char *usb0_suspend_state_sw_name = "usb0_suspend_state";
static struct switch_dev  usb0_suspend_state_sw;
static struct delayed_work usb0_suspend_state_work;
static struct workqueue_struct * usb0_suspend_state_workqueue = NULL;

static unsigned int usb0_gadget_intrusb = 0;

enum switch_state {
  USB0_GADGET_STATE_SWITCH_SUSPEND = 1,
  USB0_GADGET_STATE_SWITCH_NOT_SUSPEND = 0,
};
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */

#ifdef MUSB_QMU_SUPPORT
#include <linux/musb/musb_qmu.h>
int mtk_qmu_dbg_level = LOG_NOTICE;
u32 dma_burst_setting;
#ifdef QMU_TASKLET
int qmu_tasklet = 1;
void qmu_done_tasklet(unsigned long data){
	unsigned int qmu_val;
	unsigned int i;
	unsigned long flags;
	struct musb *musb = (struct musb *)data;

	spin_lock_irqsave(&musb->lock, flags);

	qmu_val = musb->qmu_done_intr;

	musb->qmu_done_intr = 0;

	for(i = 1; i<= MAX_QMU_EP; i++) {
		if (qmu_val & DQMU_M_RX_DONE(i)){
			qmu_done_rx(musb, i);
		}
		if (qmu_val & DQMU_M_TX_DONE(i)){
			qmu_done_tx(musb, i);
		}
	}
	spin_unlock_irqrestore(&musb->lock, flags);
}

#endif
#endif
DEFINE_SPINLOCK(usb_io_lock);
unsigned musb_debug = 0;
unsigned musb_uart_debug = 0;
struct musb *mtk_musb = NULL;
#if defined(CONFIG_USB_MTK_PORT0_CHARGE)
unsigned musb_speed = 0;
#else
unsigned musb_speed = 1;
#endif
bool mtk_usb_power = false;

struct timeval writeTime;
struct timeval interruptTime;

module_param_named(speed, musb_speed, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "USB speed configuration. default = 1, high speed");
module_param_named(debug, musb_debug, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug message level. Default = 0");
module_param_named(dbg_uart, musb_uart_debug, uint, S_IRUGO | S_IWUSR);

#define TA_WAIT_BCON(m) max_t(int, (m)->a_wait_bcon, OTG_TIME_A_WAIT_BCON)

#define DRIVER_AUTHOR "Mentor Graphics, Texas Instruments, Nokia"
#define DRIVER_DESC "Inventra Dual-Role USB Controller Driver"

#define MUSB_VERSION "6.0"

#define DRIVER_INFO DRIVER_DESC ", v" MUSB_VERSION

#define MUSB_DRIVER_NAME "musb-hdrc"
const char musb_driver_name[] = MUSB_DRIVER_NAME;

static DEFINE_IDA(musb_ida);

MODULE_DESCRIPTION(DRIVER_INFO);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" MUSB_DRIVER_NAME);

void dumpTime(writeFunc_enum func, int epnum)
{
#if 0
	struct timeval tv;
	int diffWrite = 0;
	int diffInterrupt = 0;

	if ((func == funcWriteb) || (func == funcWritew)) {
		do_gettimeofday(&tv);
		diffWrite = tv.tv_sec - writeTime.tv_sec;
		if (diffWrite > 10) {
			DBG(0, "Write Operation (%d) seconds\n", diffWrite);
			writeTime = tv;
		}
	}

	if (func == funcInterrupt) {
		do_gettimeofday(&tv);
		diffInterrupt = tv.tv_sec - interruptTime.tv_sec;
		if (diffInterrupt > 10) {
			DBG(0, "Interrupt Operation (%d) seconds\n", diffInterrupt);
			interruptTime = tv;
		}
	}
#endif
}

/*-------------------------------------------------------------------------*/

static inline struct musb *dev_to_musb(struct device *dev)
{
	return dev_get_drvdata(dev);
}

/*-------------------------------------------------------------------------*/

int musb_get_id(struct device *dev, gfp_t gfp_mask)
{
	int ret;
	int id;

	ret = ida_pre_get(&musb_ida, gfp_mask);
	if (!ret) {
		dev_err(dev, "failed to reserve resource for id\n");
		return -ENOMEM;
	}

	ret = ida_get_new(&musb_ida, &id);
	if (ret < 0) {
		dev_err(dev, "failed to allocate a new id\n");
		return ret;
	}

	return id;
}
EXPORT_SYMBOL_GPL(musb_get_id);

void musb_put_id(struct device *dev, int id)
{

	dev_dbg(dev, "removing id %d\n", id);
	ida_remove(&musb_ida, id);
}
EXPORT_SYMBOL_GPL(musb_put_id);

#if 0				/* #ifndef CONFIG_BLACKFIN */
static int musb_ulpi_read(struct usb_phy *phy, u32 offset)
{
	void __iomem *addr = phy->io_priv;
	int i = 0;
	u8 r;
	u8 power;
	int ret;

	pm_runtime_get_sync(phy->io_dev);

	/* Make sure the transceiver is not in low power mode */
	power = musb_readb(addr, MUSB_POWER);
	power &= ~MUSB_POWER_SUSPENDM;
	musb_writeb(addr, MUSB_POWER, power);

	/* REVISIT: musbhdrc_ulpi_an.pdf recommends setting the
	 * ULPICarKitControlDisableUTMI after clearing POWER_SUSPENDM.
	 */

	musb_writeb(addr, MUSB_ULPI_REG_ADDR, (u8) offset);
	musb_writeb(addr, MUSB_ULPI_REG_CONTROL, MUSB_ULPI_REG_REQ | MUSB_ULPI_RDN_WR);

	while (!(musb_readb(addr, MUSB_ULPI_REG_CONTROL)
		 & MUSB_ULPI_REG_CMPLT)) {
		i++;
		if (i == 10000) {
			ret = -ETIMEDOUT;
			goto out;
		}

	}
	r = musb_readb(addr, MUSB_ULPI_REG_CONTROL);
	r &= ~MUSB_ULPI_REG_CMPLT;
	musb_writeb(addr, MUSB_ULPI_REG_CONTROL, r);

	ret = musb_readb(addr, MUSB_ULPI_REG_DATA);

 out:
	pm_runtime_put(phy->io_dev);

	return ret;
}

static int musb_ulpi_write(struct usb_phy *phy, u32 offset, u32 data)
{
	void __iomem *addr = phy->io_priv;
	int i = 0;
	u8 r = 0;
	u8 power;
	int ret = 0;

	pm_runtime_get_sync(phy->io_dev);

	/* Make sure the transceiver is not in low power mode */
	power = musb_readb(addr, MUSB_POWER);
	power &= ~MUSB_POWER_SUSPENDM;
	musb_writeb(addr, MUSB_POWER, power);

	musb_writeb(addr, MUSB_ULPI_REG_ADDR, (u8) offset);
	musb_writeb(addr, MUSB_ULPI_REG_DATA, (u8) data);
	musb_writeb(addr, MUSB_ULPI_REG_CONTROL, MUSB_ULPI_REG_REQ);

	while (!(musb_readb(addr, MUSB_ULPI_REG_CONTROL)
		 & MUSB_ULPI_REG_CMPLT)) {
		i++;
		if (i == 10000) {
			ret = -ETIMEDOUT;
			goto out;
		}
	}

	r = musb_readb(addr, MUSB_ULPI_REG_CONTROL);
	r &= ~MUSB_ULPI_REG_CMPLT;
	musb_writeb(addr, MUSB_ULPI_REG_CONTROL, r);

 out:
	pm_runtime_put(phy->io_dev);

	return ret;
}
#else
#define musb_ulpi_read		NULL
#define musb_ulpi_write		NULL
#endif

static struct usb_phy_io_ops musb_ulpi_access = {
	.read = musb_ulpi_read,
	.write = musb_ulpi_write,
};

/*-------------------------------------------------------------------------*/



/*
 * Load an endpoint's FIFO
 */
void musb_write_fifo(struct musb_hw_ep *hw_ep, u16 len, const u8 *src)
{
	void __iomem *fifo;

	if (mtk_musb->is_host)
		fifo = hw_ep->fifo;
	else
		fifo = mtk_musb->mregs + MUSB_FIFO_OFFSET(hw_ep->ep_in.current_epnum);

	if (unlikely(len == 0))
		return;

	prefetch((u8 *) src);

	DBG(4, "%cX ep%d fifo %p count %d buf %p\n", 'T', hw_ep->epnum, fifo, len, src);

	/* we can't assume unaligned reads work */
	if (likely((0x01 & (unsigned long)src) == 0)) {
		u16 index = 0;

		/* best case is 32bit-aligned source address */
		if ((0x02 & (unsigned long)src) == 0) {
			if (len >= 4) {
				iowrite32_rep(fifo, src + index, len >> 2);
				index += len & ~0x03;
			}
			if (len & 0x02) {
				musb_writew(fifo, 0, *(u16 *) &src[index]);
				index += 2;
			}
		} else {
			if (len >= 2) {
				iowrite16_rep(fifo, src + index, len >> 1);
				index += len & ~0x01;
			}
		}
		if (len & 0x01)
			musb_writeb(fifo, 0, src[index]);
	} else {
		/* byte aligned */
		iowrite8_rep(fifo, src, len);
	}
}

/*
 * Unload an endpoint's FIFO
 */
void musb_read_fifo(struct musb_hw_ep *hw_ep, u16 len, u8 *dst)
{
	void __iomem *fifo;

	if (mtk_musb->is_host)
		fifo = hw_ep->fifo;
	else
		fifo = mtk_musb->mregs + MUSB_FIFO_OFFSET(hw_ep->ep_out.current_epnum);


	if (unlikely(len == 0))
		return;

	DBG(4, "%cX ep%d fifo %p count %d buf %p\n", 'R', hw_ep->epnum, fifo, len, dst);

	/* we can't assume unaligned writes work */
	if (likely((0x01 & (unsigned long)dst) == 0)) {
		u16 index = 0;

		/* best case is 32bit-aligned destination address */
		if ((0x02 & (unsigned long)dst) == 0) {
			if (len >= 4) {
				ioread32_rep(fifo, dst, len >> 2);
				index = len & ~0x03;
			}
			if (len & 0x02) {
				*(u16 *) &dst[index] = musb_readw(fifo, 0);
				index += 2;
			}
		} else {
			if (len >= 2) {
				ioread16_rep(fifo, dst, len >> 1);
				index = len & ~0x01;
			}
		}
		if (len & 0x01)
			dst[index] = musb_readb(fifo, 0);
	} else {
		/* byte aligned */
		ioread8_rep(fifo, dst, len);
	}
}



/*-------------------------------------------------------------------------*/

/* for high speed test mode; see USB 2.0 spec 7.1.20 */
static const u8 musb_test_packet[53] = {
	/* implicit SYNC then DATA0 to start */

	/* JKJKJKJK x9 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* JJKKJJKK x8 */
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	/* JJJJKKKK x8 */
	0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
	/* JJJJJJJKKKKKKK x8 */
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/* JJJJJJJK x8 */
	0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd,
	/* JKKKKKKK x10, JK */
	0xfc, 0x7e, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0x7e
	    /* implicit CRC16 then EOP to end */
};

void musb_load_testpacket(struct musb *musb)
{
	void __iomem *regs = musb->endpoints[0].regs;

	musb_ep_select(musb->mregs, 0);
	musb_write_fifo(musb->control_ep, sizeof(musb_test_packet), musb_test_packet);
	musb_writew(regs, MUSB_CSR0, MUSB_CSR0_TXPKTRDY);
}

/*-------------------------------------------------------------------------*/

/*
 * Handles OTG hnp timeouts, such as b_ase0_brst
 */
static void musb_otg_timer_func(unsigned long data)
{
	struct musb *musb = (struct musb *)data;
	unsigned long flags;

	spin_lock_irqsave(&musb->lock, flags);
	switch (musb->xceiv->state) {
	case OTG_STATE_B_WAIT_ACON:
		DBG(2, "HNP: b_wait_acon timeout; back to b_peripheral\n");
		musb_g_disconnect(musb);
		musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
		musb->is_active = 0;
		break;
	case OTG_STATE_A_SUSPEND:
	case OTG_STATE_A_WAIT_BCON:
		DBG(2, "HNP: %s timeout\n", otg_state_string(musb->xceiv->state));
		musb_platform_set_vbus(musb, 0);
		musb->xceiv->state = OTG_STATE_A_WAIT_VFALL;
		break;
	default:
		DBG(2, "HNP: Unhandled mode %s\n", otg_state_string(musb->xceiv->state));
	}
	musb->ignore_disconnect = 0;
	spin_unlock_irqrestore(&musb->lock, flags);
}

void musb_handle_disconnect(struct musb *musb)
{
	int epnum, i;
	struct urb *urb;
	struct musb_hw_ep *hw_ep;
	struct musb_qh *qh;
	struct usb_hcd *hcd = musb_to_hcd(musb);

	for (epnum = 0; epnum < musb->config->num_eps; epnum++) {
		hw_ep = musb->endpoints + epnum;
		for (i = 0; i < 2; i++) {
			if (hw_ep->in_qh == hw_ep->out_qh)
				i++;
			qh = (i == 0) ? hw_ep->in_qh : hw_ep->out_qh;

			if (qh && qh->hep) {
				qh->is_ready = 0;
				while ((urb = next_urb(qh))) {
					usb_hcd_unlink_urb_from_ep(hcd, urb);

					spin_unlock(&musb->lock);
					usb_hcd_giveback_urb(hcd, urb, 0);
					spin_lock(&musb->lock);
				}

				qh->hep->hcpriv = NULL;
				list_del(&qh->ring);
				kfree(qh);
				hw_ep->in_qh = hw_ep->out_qh = NULL;
			}
		}
	}
}

/*
 * Stops the HNP transition. Caller must take care of locking.
 */
void musb_hnp_stop(struct musb *musb)
{
	struct usb_hcd *hcd = musb_to_hcd(musb);
	void __iomem *mbase = musb->mregs;
	u8 reg;

	DBG(2, "HNP: stop from %s\n", otg_state_string(musb->xceiv->state));

	switch (musb->xceiv->state) {
	case OTG_STATE_A_PERIPHERAL:
		musb_g_disconnect(musb);
		DBG(2, "HNP: back to %s\n", otg_state_string(musb->xceiv->state));
		break;
	case OTG_STATE_B_HOST:
		DBG(2, "HNP: Disabling HR\n");
		hcd->self.is_b_host = 0;
		musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
		MUSB_DEV_MODE(musb);
		reg = musb_readb(mbase, MUSB_POWER);
		reg |= MUSB_POWER_SUSPENDM;
		musb_writeb(mbase, MUSB_POWER, reg);
		/* REVISIT: Start SESSION_REQUEST here? */
		break;
	default:
		DBG(2, "HNP: Stopping in unknown state %s\n", otg_state_string(musb->xceiv->state));
	}

	/*
	 * When returning to A state after HNP, avoid hub_port_rebounce(),
	 * which cause occasional OPT A "Did not receive reset after connect"
	 * errors.
	 */
	musb->port1_status &= ~(USB_PORT_STAT_C_CONNECTION << 16);
}

/*
 * Interrupt Service Routine to record USB "global" interrupts.
 * Since these do not happen often and signify things of
 * paramount importance, it seems OK to check them individually;
 * the order of the tests is specified in the manual
 *
 * @param musb instance pointer
 * @param int_usb register contents
 * @param devctl
 * @param power
 */
static int musb_sof_count = 0;

typedef void (*uac2_get_buffer_status)(
    u32 *status     
);
extern void musb_tx_data_directly(struct musb *musb, u8 epnum, void* data, uint32_t lengh);
#if (!defined(CONFIG_USB11_MTK_HDRC_GADGET)) 
uac2_get_buffer_status pf_get_uac2_buf_status = NULL;
void reg_uac2_buffer_status(void* pf)
{
	pf_get_uac2_buf_status = (uac2_get_buffer_status)pf;

}
#else
extern uac2_get_buffer_status pf_get_uac2_buf_status;
#endif
#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))  
extern bool port0_poll_need;
bool port0_qmu_stop = false;
#endif

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
static void usb0_suspend_state_workqueue_func(void)
{
  int ret;
  ret = 0;
  
  ret=schedule_delayed_work(&usb0_suspend_state_work, msecs_to_jiffies(0));
  
  if (!ret) {
    printk("usb0_suspend_state_workqueue_func return:%d!\n", ret);  		
  }
}

static void usb0_suspend_state_event_notify(void)
{
  switch (usb0_gadget_intrusb) {
    case MUSB_INTR_SUSPEND:
      switch_set_state(&usb0_suspend_state_sw, USB0_GADGET_STATE_SWITCH_SUSPEND); /* USB0 is suspend state */
      break;
    case MUSB_INTR_RESUME:
      switch_set_state(&usb0_suspend_state_sw, USB0_GADGET_STATE_SWITCH_NOT_SUSPEND); /* USB0 is Not suspend state */
      break;
    case MUSB_INTR_RESET:
      switch_set_state(&usb0_suspend_state_sw, USB0_GADGET_STATE_SWITCH_NOT_SUSPEND); /* USB0 is Not suspend state */
      break;
    default:
      printk("usb0 suspend event is issued by an unexpected interrupt!\n");
      switch_set_state(&usb0_suspend_state_sw, USB0_GADGET_STATE_SWITCH_NOT_SUSPEND); /* USB0 is Not suspend state */
      break;
  }
}
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */

static irqreturn_t musb_stage0_irq(struct musb *musb, u8 int_usb, u8 devctl)
{
	struct usb_otg *otg = musb->xceiv->otg;
	irqreturn_t handled = IRQ_NONE;

	DBG(2, "<== DevCtl=%02x, int_usb=0x%x\n", devctl, int_usb);

	USB_LOGGER(MUSB_STAGE0_IRQ, MUSB_STAGE0_IRQ, power, devctl, int_usb);

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
  usb0_gadget_intrusb = int_usb;
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */

	/* in host mode, the peripheral may issue remote wakeup.
	 * in peripheral mode, the host may resume the link.
	 * spurious RESUME irqs happen too, paired with SUSPEND.
	 */

	if ((int_usb & MUSB_INTR_RESET) && (musb->xceiv->state == OTG_STATE_A_HOST)) {
		DBG(0, "MUSB_INTR_RESET (%s)\n", otg_state_string(musb->xceiv->state));
		printk("<== DevCtl=%02x, int_usb=0x%x\n", devctl, int_usb);
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
		port0_poll_need = false;
		#endif
		#ifdef MUSB_QMU_SUPPORT
		musb_disable_q_all(musb);
		#endif
		handled = IRQ_HANDLED;
		int_usb &= ~MUSB_INTR_RESET;
		switch_int_to_host(musb);
		#if defined(USB_SW_WITCH_MODE)
		musb_session_restart(musb);
		#endif
		musb->xceiv->state = OTG_STATE_A_HOST;
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD))
		USBPHY_SET8(0x24,0x01);
		#endif
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
		USBPHY_CLR8(0x19, 0x30);
		USBPHY_SET8(0x19, 0x20);
		port0_poll_need = true;
		port0_qmu_stop = true;
		printk("port0 open polling thread and close hw detect%s %d\n",__func__,__LINE__);
		#endif
	}
	if (int_usb & MUSB_INTR_RESUME) {
		handled = IRQ_HANDLED;
		DBG(2, "RESUME (%s)\n", otg_state_string(musb->xceiv->state));

		if (devctl & MUSB_DEVCTL_HM) {
			void __iomem *mbase = musb->mregs;
			u8 power;

			switch (musb->xceiv->state) {
			case OTG_STATE_A_SUSPEND:
				/* remote wakeup?  later, GetPortStatus
				 * will stop RESUME signaling
				 */

				power = musb_readb(musb->mregs, MUSB_POWER);
				if (power & MUSB_POWER_SUSPENDM) {
					/* spurious */
					musb->int_usb &= ~MUSB_INTR_SUSPEND;
					DBG(2, "Spurious SUSPENDM\n");
					break;
				}

				power &= ~MUSB_POWER_SUSPENDM;
				musb_writeb(mbase, MUSB_POWER, power | MUSB_POWER_RESUME);

				musb->port1_status |= (USB_PORT_STAT_C_SUSPEND << 16)
				    | MUSB_PORT_STAT_RESUME;
				musb->rh_timer = jiffies + msecs_to_jiffies(20);

				musb->xceiv->state = OTG_STATE_A_HOST;
				musb->is_active = 1;
				usb_hcd_resume_root_hub(musb_to_hcd(musb));
				break;
			case OTG_STATE_B_WAIT_ACON:
				musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
				musb->is_active = 1;
				MUSB_DEV_MODE(musb);
				break;
			default:
				WARNING("bogus %s RESUME (%s)\n",
					"host", otg_state_string(musb->xceiv->state));
			}
		} else {
#if (defined(CONFIG_POWER_EXT)) /* other than Port0(CHG) model */
			switch (musb->xceiv->state) {
			case OTG_STATE_A_SUSPEND:
				/* possibly DISCONNECT is upcoming */
				musb->xceiv->state = OTG_STATE_A_HOST;
				usb_hcd_resume_root_hub(musb_to_hcd(musb));
				break;
			case OTG_STATE_B_WAIT_ACON:
			case OTG_STATE_B_PERIPHERAL:
				/* disconnect while suspended?  we may
				 * not get a disconnect irq...
				 */
				if ((devctl & MUSB_DEVCTL_VBUS)
				    != (3 << MUSB_DEVCTL_VBUS_SHIFT)
				    ) {
					musb->int_usb |= MUSB_INTR_DISCONNECT;
					musb->int_usb &= ~MUSB_INTR_SUSPEND;
					break;
				}
				musb_g_resume(musb);
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
        usb0_suspend_state_workqueue_func();
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */
				break;
			case OTG_STATE_B_IDLE:
				musb->int_usb &= ~MUSB_INTR_SUSPEND;
				break;
			default:
				WARNING("bogus %s RESUME (%s)\n",
					"peripheral", otg_state_string(musb->xceiv->state));
			}
#else /* Port0(CHG) model */
			if ((devctl & MUSB_DEVCTL_VBUS)
			    != (3 << MUSB_DEVCTL_VBUS_SHIFT)
			    ) {
				musb->int_usb |= MUSB_INTR_DISCONNECT;
				musb->int_usb &= ~MUSB_INTR_SUSPEND;
			} else {
			  musb_g_resume(musb);
			}
#endif /* (defined(CONFIG_POWER_EXT)) */
		}
	}

	/* see manual for the order of the tests */
	if (int_usb & MUSB_INTR_SESSREQ) {
		/* void __iomem *mbase = musb->mregs; */

		if ((devctl & MUSB_DEVCTL_VBUS) == MUSB_DEVCTL_VBUS
		    && (devctl & MUSB_DEVCTL_BDEVICE)) {
			DBG(2, "SessReq while on B state\n");
			return IRQ_HANDLED;
		}

		DBG(0, "SESSION_REQUEST (%s)\n", otg_state_string(musb->xceiv->state));

		/* IRQ arrives from ID pin sense or (later, if VBUS power
		 * is removed) SRP.  responses are time critical:
		 *  - turn on VBUS (with silicon-specific mechanism)
		 *  - go through A_WAIT_VRISE
		 *  - ... to A_WAIT_BCON.
		 * a_wait_vrise_tmout triggers VBUS_ERROR transitions
		 */

		/* do nothing when get SESSION_REQUEST */
		/* turn on VBUS in musb_id_pin_work() */
#if 0
		musb_writeb(mbase, MUSB_DEVCTL, MUSB_DEVCTL_SESSION);
		musb->ep0_stage = MUSB_EP0_START;
		musb->xceiv->state = OTG_STATE_A_IDLE;
		MUSB_HST_MODE(musb);
		musb_platform_set_vbus(musb, 1);
#endif

		handled = IRQ_HANDLED;
	}

	if (int_usb & MUSB_INTR_VBUSERROR) {
		int ignore = 0;

		DBG(0, "MUSB_INTR_VBUSERROR (%s)\n", otg_state_string(musb->xceiv->state));

		/* During connection as an A-Device, we may see a short
		 * current spikes causing voltage drop, because of cable
		 * and peripheral capacitance combined with vbus draw.
		 * (So: less common with truly self-powered devices, where
		 * vbus doesn't act like a power supply.)
		 *
		 * Such spikes are short; usually less than ~500 usec, max
		 * of ~2 msec.  That is, they're not sustained overcurrent
		 * errors, though they're reported using VBUSERROR irqs.
		 *
		 * Workarounds:  (a) hardware: use self powered devices.
		 * (b) software:  ignore non-repeated VBUS errors.
		 *
		 * REVISIT:  do delays from lots of DEBUG_KERNEL checks
		 * make trouble here, keeping VBUS < 4.4V ?
		 */
		DBG(0, "VBUSERROR\n");
		switch (musb->xceiv->state) {
		case OTG_STATE_A_HOST:
			/* recovery is dicey once we've gotten past the
			 * initial stages of enumeration, but if VBUS
			 * stayed ok at the other end of the link, and
			 * another reset is due (at least for high speed,
			 * to redo the chirp etc), it might work OK...
			 */
		case OTG_STATE_A_WAIT_BCON:
		case OTG_STATE_A_WAIT_VRISE:
			if (musb->vbuserr_retry) {
				musb->vbuserr_retry--;
				ignore = 1;
				/* workaround to let HW state matchine stop waiting for VBUS dropping and restart sampling VBUS.
				   add this because sometimes a short (~3ms) VBUS droop will cause HW state matching waiting forever for VBUS dropping below 0.2V
				 */
#if 1
				musb_session_restart(musb);
#else
				devctl |= MUSB_DEVCTL_SESSION;
				musb_writeb(mbase, MUSB_DEVCTL, devctl);
#endif

			} else {
				musb->port1_status |=
				    USB_PORT_STAT_OVERCURRENT | (USB_PORT_STAT_C_OVERCURRENT << 16);
			}
			break;
		default:
			break;
		}

		DBG(2, "VBUS_ERROR in %s (%02x, %s), retry #%d, port1 %08x\n",
		    otg_state_string(musb->xceiv->state), devctl, ({
								   char *s;
								   switch (devctl &
									   MUSB_DEVCTL_VBUS) {
case 0 << MUSB_DEVCTL_VBUS_SHIFT:
s = "<SessEnd"; break; case 1 << MUSB_DEVCTL_VBUS_SHIFT:
s = "<AValid"; break; case 2 << MUSB_DEVCTL_VBUS_SHIFT:
								   s = "<VBusValid"; break;
								   /* case 3 << MUSB_DEVCTL_VBUS_SHIFT: */
default:
								   s = "VALID"; break; };
								   s; }
		    ), VBUSERR_RETRY_COUNT - musb->vbuserr_retry, musb->port1_status);

		/* go through A_WAIT_VFALL then start a new session */
		if (!ignore) {
			if (is_switch_charger()) {
				DBG(0,
				    "too many VBUS error, restart power on sequence for switching charger!\n");
				schedule_delayed_work(&musb->id_pin_work, 400 * HZ / 1000);
			} else {
				musb_platform_set_vbus(musb, 0);
				DBG(0, "too many VBUS error, turn it off!\n");
			}
		}
		handled = IRQ_HANDLED;
	}

	if (int_usb & MUSB_INTR_SUSPEND) {
		DBG(0, "SUSPEND (%s) devctl %02x\n", otg_state_string(musb->xceiv->state), devctl);
		handled = IRQ_HANDLED;
		#if (defined(CONFIG_USB_MTK_PORT0_CHARGE))
		DBG(1, "announce suspend to the battery!\n");
		musb_sync_with_bat(musb, USB_SUSPEND);
		#endif

#if (defined(CONFIG_POWER_EXT)) /* other than Port0(CHG) model */
		switch (musb->xceiv->state) {
		case OTG_STATE_A_PERIPHERAL:
			/* We also come here if the cable is removed, since
			 * this silicon doesn't report ID-no-longer-grounded.
			 *
			 * We depend on T(a_wait_bcon) to shut us down, and
			 * hope users don't do anything dicey during this
			 * undesired detour through A_WAIT_BCON.
			 */
			musb_hnp_stop(musb);
			usb_hcd_resume_root_hub(musb_to_hcd(musb));
			musb_root_disconnect(musb);
			musb_platform_try_idle(musb, jiffies
					       + msecs_to_jiffies(musb->a_wait_bcon
								  ? : OTG_TIME_A_WAIT_BCON));

			break;
		case OTG_STATE_B_IDLE:
			if (!musb->is_active)
				break;
		case OTG_STATE_B_PERIPHERAL:
			musb_g_suspend(musb);
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
      usb0_suspend_state_workqueue_func();
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */
			musb->is_active = otg->gadget->b_hnp_enable;
			if (musb->is_active) {
				musb->xceiv->state = OTG_STATE_B_WAIT_ACON;
				DBG(2, "HNP: Setting timer for b_ase0_brst\n");
				mod_timer(&musb->otg_timer, jiffies
					  + msecs_to_jiffies(OTG_TIME_B_ASE0_BRST));
			}
			break;
		case OTG_STATE_A_WAIT_BCON:
			if (musb->a_wait_bcon != 0)
				musb_platform_try_idle(musb, jiffies
						       + msecs_to_jiffies(musb->a_wait_bcon));
			break;
		case OTG_STATE_A_HOST:
			musb->xceiv->state = OTG_STATE_A_SUSPEND;
			musb->is_active = otg->host->b_hnp_enable;
			break;
		case OTG_STATE_B_HOST:
			/* Transition to B_PERIPHERAL, see 6.8.2.6 p 44 */
			DBG(2, "REVISIT: SUSPEND as B_HOST\n");
			break;
		default:
			/* "should not happen" */
			musb->is_active = 0;
			break;
		}
#else /* Port0(CHG) model */
		if (!musb->is_active) {
			; /* do nothing */
		} else {
			musb_g_suspend(musb);
			musb->is_active = otg->gadget->b_hnp_enable;
			if (musb->is_active) {
				musb->xceiv->state = OTG_STATE_B_WAIT_ACON;
				DBG(2, "HNP: Setting timer for b_ase0_brst\n");
				mod_timer(&musb->otg_timer, jiffies
					  + msecs_to_jiffies(OTG_TIME_B_ASE0_BRST));
			}
		}
#endif /* (defined(CONFIG_POWER_EXT)) */
	}

	if (int_usb & MUSB_INTR_CONNECT) {
		struct usb_hcd *hcd = musb_to_hcd(musb);
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD))
		USBPHY_CLR8(0x24,0x01);		
		DBG(0, "[HUB]:port0 MUSB_INTR_CONNECT USBPHY_CLR8(0X24,0X01) \n");
		#endif
		DBG(0, "MUSB_INTR_CONNECT (%s)\n", otg_state_string(musb->xceiv->state));
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
		port0_qmu_stop = false;
		#endif

		handled = IRQ_HANDLED;
		musb->is_active = 1;

		musb->ep0_stage = MUSB_EP0_START;

#ifdef MUSB_QMU_SUPPORT
		musb_disable_q_all(musb);
#endif
		/* flush endpoints when transitioning from Device Mode */
		if (is_peripheral_active(musb)) {
			/* REVISIT HNP; just force disconnect */
		}
#if 0
		musb->intrtxe = musb->epmask;
		musb_writew(musb->mregs, MUSB_INTRTXE, musb->intrtxe);
		musb->intrrxe = musb->epmask & 0xfffe;
		musb_writew(musb->mregs, MUSB_INTRRXE, musb->intrrxe);
		musb_writeb(musb->mregs, MUSB_INTRUSBE, 0xf7);
#endif
		musb->port1_status &= ~(USB_PORT_STAT_LOW_SPEED
					| USB_PORT_STAT_HIGH_SPEED | USB_PORT_STAT_ENABLE);
		musb->port1_status |= USB_PORT_STAT_CONNECTION | (USB_PORT_STAT_C_CONNECTION << 16);

		/* high vs full speed is just a guess until after reset */
		if (devctl & MUSB_DEVCTL_LSDEV)
			musb->port1_status |= USB_PORT_STAT_LOW_SPEED;

		/* indicate new connection to OTG machine */
		switch (musb->xceiv->state) {
		case OTG_STATE_B_PERIPHERAL:
			if (int_usb & MUSB_INTR_SUSPEND) {
				DBG(2, "HNP: SUSPEND+CONNECT, now b_host\n");
				int_usb &= ~MUSB_INTR_SUSPEND;
				goto b_host;
			} else
				DBG(2, "CONNECT as b_peripheral???\n");
			break;
		case OTG_STATE_B_WAIT_ACON:
			DBG(2, "HNP: CONNECT, now b_host\n");
 b_host:
			musb->xceiv->state = OTG_STATE_B_HOST;
			hcd->self.is_b_host = 1;
			musb->ignore_disconnect = 0;
			del_timer(&musb->otg_timer);
			break;
		default:
			if ((devctl & MUSB_DEVCTL_VBUS)
			    == (3 << MUSB_DEVCTL_VBUS_SHIFT)) {
				musb->xceiv->state = OTG_STATE_A_HOST;
				hcd->self.is_b_host = 0;
			}
			break;
		}

		/* poke the root hub */
		MUSB_HST_MODE(musb);
		if (hcd->status_urb)
			usb_hcd_poll_rh_status(hcd);
		else
			usb_hcd_resume_root_hub(hcd);

		DBG(0, "CONNECT (%s) devctl %02x\n", otg_state_string(musb->xceiv->state), devctl);
	}

	if ((int_usb & MUSB_INTR_DISCONNECT) && !musb->ignore_disconnect) {
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD))   
		USBPHY_SET8(0x24,0x01);		
		DBG(0, "[HUB]:port0 MUSB_INTR_DISCONNECT USBPHY_SET8(0X24,0X01) \n");
		#endif
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
		USBPHY_CLR8(0x19, 0x30);
		USBPHY_SET8(0x19, 0x20);
		port0_poll_need = true;
		port0_qmu_stop = true;
		printk("port0 open polling thread and close hw detect %s %d\n",__func__,__LINE__);
		#endif
		DBG(0, "DISCONNECT (%s) as %s, devctl %02x\n",
		    otg_state_string(musb->xceiv->state), MUSB_MODE(musb), devctl);
		handled = IRQ_HANDLED;
		musb->is_active = 0;
#ifdef MUSB_QMU_SUPPORT
		musb_disable_q_all(musb);
#endif
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
		port0_qmu_stop = false;
		#endif

		switch (musb->xceiv->state) {
		case OTG_STATE_A_HOST:
		case OTG_STATE_A_SUSPEND:
			usb_hcd_resume_root_hub(musb_to_hcd(musb));
			/* musb_handle_disconnect(musb); */
			musb_root_disconnect(musb);
			if (musb->a_wait_bcon != 0)
				musb_platform_try_idle(musb, jiffies
						       + msecs_to_jiffies(musb->a_wait_bcon));
			break;
		case OTG_STATE_B_HOST:
			/* REVISIT this behaves for "real disconnect"
			 * cases; make sure the other transitions from
			 * from B_HOST act right too.  The B_HOST code
			 * in hnp_stop() is currently not used...
			 */
			musb_root_disconnect(musb);
			musb_to_hcd(musb)->self.is_b_host = 0;
			musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
			MUSB_DEV_MODE(musb);
			musb_g_disconnect(musb);
			break;
		case OTG_STATE_A_PERIPHERAL:
			musb_hnp_stop(musb);
			musb_root_disconnect(musb);
			/* FALLTHROUGH */
		case OTG_STATE_B_WAIT_ACON:
			/* FALLTHROUGH */
		case OTG_STATE_B_PERIPHERAL:
		case OTG_STATE_B_IDLE:
			musb_g_disconnect(musb);
			break;
		default:
			WARNING("unhandled DISCONNECT transition (%s)\n",
				otg_state_string(musb->xceiv->state));
			break;
		}
	}

	/* mentor saves a bit: bus reset and babble share the same irq.
	 * only host sees babble; only peripheral sees bus reset.
	 */
	if (int_usb & MUSB_INTR_RESET) {
		handled = IRQ_HANDLED;

		DBG(0, "MUSB_INTR_RESET (%s)\n", otg_state_string(musb->xceiv->state));

		if ((devctl & MUSB_DEVCTL_HM) != 0) {
			/*
			 * Looks like non-HS BABBLE can be ignored, but
			 * HS BABBLE is an error condition. For HS the solution
			 * is to avoid babble in the first place and fix what
			 * caused BABBLE. When HS BABBLE happens we can only
			 * stop the session.
			 */
			DBG(0, "Babble\n");
			if (devctl & (MUSB_DEVCTL_FSDEV | MUSB_DEVCTL_LSDEV))
				DBG(2, "BABBLE devctl: %02x\n", devctl);
			else {
				ERR("Stopping host session -- babble\n");
				/*musb_writeb(musb->mregs, MUSB_DEVCTL, 0);*/
			}
		} else {
			DBG(0, "BUS RESET as %s\n", otg_state_string(musb->xceiv->state));
#ifdef MUSB_QMU_SUPPORT
			musb_disable_q_all(musb);
#endif
			switch (musb->xceiv->state) {
			case OTG_STATE_A_SUSPEND:
				/* We need to ignore disconnect on suspend
				 * otherwise tusb 2.0 won't reconnect after a
				 * power cycle, which breaks otg compliance.
				 */
				musb->ignore_disconnect = 1;
				musb_g_reset(musb);
				/* FALLTHROUGH */
			case OTG_STATE_A_WAIT_BCON:	/* OPT TD.4.7-900ms */
				/* never use invalid T(a_wait_bcon) */
				DBG(2, "HNP: in %s, %d msec timeout\n",
				    otg_state_string(musb->xceiv->state), TA_WAIT_BCON(musb));
				mod_timer(&musb->otg_timer, jiffies
					  + msecs_to_jiffies(TA_WAIT_BCON(musb)));
				break;
			case OTG_STATE_A_PERIPHERAL:
				musb->ignore_disconnect = 0;
				del_timer(&musb->otg_timer);
				musb_g_reset(musb);
				break;
			case OTG_STATE_B_WAIT_ACON:
				DBG(2, "HNP: RESET (%s), to b_peripheral\n",
				    otg_state_string(musb->xceiv->state));
				musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
				musb_g_reset(musb);
				break;
			case OTG_STATE_B_IDLE:
				musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
				/* FALLTHROUGH */
			case OTG_STATE_B_PERIPHERAL:
				musb_g_reset(musb);
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
        usb0_suspend_state_workqueue_func();
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */
				break;
			default:
				DBG(0, "Unhandled BUS RESET as %s\n",
				    otg_state_string(musb->xceiv->state));
			}
		}
	}

	
	if (int_usb & MUSB_INTR_SOF) 
	{
		handled = IRQ_HANDLED;
		musb_sof_count++;
		if (musb->g.speed != USB_SPEED_HIGH) musb_sof_count = 8;
		if(musb_sof_count > 7){
			
			musb->g.in_epnum = 1;
			
			//printk("**********DEBUG Get One Sof, ep num %d, now frame %d\n", musb->g.in_epnum, musb->g.ops->get_frame(&(musb->g)));
			
		    if ((usb_endpoint_dir_in(musb->endpoints[musb->g.in_epnum].ep_in.desc)) &&
				(1 == usb_endpoint_num(musb->endpoints[musb->g.in_epnum].ep_in.desc)) &&
				(musb->endpoints[musb->g.in_epnum].ep_in.type == USB_ENDPOINT_XFER_ISOC))
		    {
				u32 tmp11 = 0;
				int cur_frame = 0;
				cur_frame = musb->g.ops->get_frame(&(musb->g));
				if(musb->endpoints[musb->g.in_epnum].start_frame != cur_frame)
				{
					//printk("start(%d),cur(%d)\n" ,musb->endpoints[musb->g.in_epnum].start_frame, cur_frame);
					musb->endpoints[musb->g.in_epnum].start_frame = cur_frame;
					if(NULL != pf_get_uac2_buf_status)
					{
						pf_get_uac2_buf_status(&tmp11);
					}else
					{
						DBG(0, "UAC2 buffer status pointer is NULL\n");
					}
					if (tmp11)
					{
						if (musb->g.speed == USB_SPEED_HIGH)
							musb_tx_data_directly(musb, musb->g.in_epnum, &tmp11, 4);
						else
							musb_tx_data_directly(musb, musb->g.in_epnum, &tmp11, 3);
					}
				}
			}
			musb_sof_count = 0;
		}
	}
	
#if 0
/* REVISIT ... this would be for multiplexing periodic endpoints, or
 * supporting transfer phasing to prevent exceeding ISO bandwidth
 * limits of a given frame or microframe.
 *
 * It's not needed for peripheral side, which dedicates endpoints;
 * though it _might_ use SOF irqs for other purposes.
 *
 * And it's not currently needed for host side, which also dedicates
 * endpoints, relies on TX/RX interval registers, and isn't claimed
 * to support ISO transfers yet.
 */
	if (int_usb & MUSB_INTR_SOF) {
		void __iomem *mbase = musb->mregs;
		struct musb_hw_ep *ep;
		u8 epnum;
		u16 frame;

		DBG(2, "START_OF_FRAME\n");
		handled = IRQ_HANDLED;

		/* start any periodic Tx transfers waiting for current frame */
		frame = musb_readw(mbase, MUSB_FRAME);
		ep = musb->endpoints;
		for (epnum = 1; (epnum < musb->nr_endpoints)
		     && (musb->epmask >= (1 << epnum)); epnum++, ep++) {
			/*
			 * FIXME handle framecounter wraps (12 bits)
			 * eliminate duplicated StartUrb logic
			 */
			if (ep->dwWaitFrame >= frame) {
				ep->dwWaitFrame = 0;
				pr_debug("SOF --> periodic TX%s on %d\n",
					 ep->tx_channel ? " DMA" : "", epnum);
				if (!ep->tx_channel)
					musb_h_tx_start(musb, epnum);
				else
					cppi_hostdma_start(musb, epnum);
			}
		}		/* end of for loop */
	}
#endif

	schedule_work(&musb->irq_work);

	return handled;
}

/*-------------------------------------------------------------------------*/

/*
* Program the HDRC to start (enable interrupts, dma, etc.).
*/
void musb_start(struct musb *musb)
{
	void __iomem *regs = musb->mregs;
	int vbusdet_retry = 5;

	u8 intrusbe;

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
	bq24262_wmport_usb_phy_off(false);
#endif

	DBG(0, "start, is_host=%d is_active=%d\n", musb->is_host, musb->is_active);
#ifdef MUSB_QMU_SUPPORT
	/* set DMA channel 0 burst mode to boost QMU speed */
	musb_writel(musb->mregs, 0x204, musb_readl(musb->mregs, 0x204) | 0x600) ;
#endif
	if (musb->is_active) {
		if (musb->is_host) {
			DBG(0, "we are host now, add more interrupt devctl=%x\n",
			    musb_readb(mtk_musb->mregs, MUSB_DEVCTL));
			musb->intrtxe = 0xffff;
			musb_writew(regs, MUSB_INTRTXE, musb->intrtxe);
			musb->intrrxe = 0xfffe;
			musb_writew(regs, MUSB_INTRRXE, musb->intrrxe);
			musb_writeb(regs, MUSB_INTRUSBE, 0xf7);
			return;
		}
	}
	musb_platform_enable(musb);
	musb_generic_disable(musb);

	intrusbe = musb_readb(regs, MUSB_INTRUSBE);
	if (musb->is_host) {
		musb->intrtxe = 0xffff;
		musb_writew(regs, MUSB_INTRTXE, musb->intrtxe);
		musb->intrrxe = 0xfffe;
		musb_writew(regs, MUSB_INTRRXE, musb->intrrxe);
		intrusbe = 0xf7;

		while (!musb_platform_get_vbus_status(musb)) {
			if (vbusdet_retry-- <= 1) {
				DBG(0, "VBUS detection fail!\n");
				break;
			}
		}

	} else if (!musb->is_host) {
		intrusbe |= MUSB_INTR_RESET;	/* device mode enable reset interrupt */
	}

	musb_writeb(regs, MUSB_INTRUSBE, intrusbe);

	/* remove babble: NOISE_STILL_SOF:1, BABBLE_CLR_EN:0 */
	intrusbe = musb_readb(regs, MUSB_ULPI_REG_DATA);
	intrusbe = intrusbe | 0x80;
	intrusbe = intrusbe & 0xbf;
	musb_writeb(regs, MUSB_ULPI_REG_DATA, intrusbe);

	if (musb_speed) {
		/* put into basic highspeed mode and start session */
		musb_writeb(regs, MUSB_POWER, MUSB_POWER_SOFTCONN | MUSB_POWER_HSENAB
			    /* ENSUSPEND wedges tusb */
			    | MUSB_POWER_ENSUSPEND);
	} else {
		/* put into basic fullspeed mode and start session */
		musb_writeb(regs, MUSB_POWER, MUSB_POWER_SOFTCONN
			    /* ENSUSPEND wedges tusb */
			    | MUSB_POWER_ENSUSPEND);
	}
	musb->is_active = 1;
}

void musb_generic_disable(struct musb *musb)
{
	void __iomem *mbase = musb->mregs;

	/* disable interrupts */
	musb_writeb(mbase, MUSB_INTRUSBE, 0);
	musb->intrtxe = 0;
	musb_writew(mbase, MUSB_INTRTXE, 0);
	musb->intrrxe = 0;
	musb_writew(mbase, MUSB_INTRRXE, 0);

	/* off */
	/* musb_writeb(mbase, MUSB_DEVCTL, 0); */

	/*  flush pending interrupts */
	musb_writew(musb->mregs, MUSB_INTRRX, 0xFFFF);
	musb_writew(musb->mregs, MUSB_INTRTX, 0xFFFF);
	musb_writeb(musb->mregs, MUSB_INTRUSB, 0xEF);
}

static void gadget_stop(struct musb *musb)
{
	u8 power;

	power = musb_readb(musb->mregs, MUSB_POWER);
	power &= ~MUSB_POWER_SOFTCONN;
	musb_writeb(musb->mregs, MUSB_POWER, power);

	/* notify gadget driver */
	if (musb->g.speed != USB_SPEED_UNKNOWN) {
		if (musb->gadget_driver && musb->gadget_driver->disconnect) {
			musb->gadget_driver->disconnect(&musb->g);
		}
		musb->g.speed = USB_SPEED_UNKNOWN;
	}
}

/*
 * Make the HDRC stop (disable interrupts, etc.);
 * reversible by musb_start
 * called on gadget driver unregister
 * with controller locked, irqs blocked
 * acts as a NOP unless some role activated the hardware
 */
void musb_stop(struct musb *musb)
{
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
	bq24262_wmport_usb_phy_off(true);
#endif

	/* stop IRQs, timers, ... */
	musb_generic_disable(musb);
	gadget_stop(musb);
	musb_platform_disable(musb);
	musb->is_active = 0;
	DBG(0, "HDRC disabled\n");

	/* FIXME
	 *  - mark host and/or peripheral drivers unusable/inactive
	 *  - disable DMA (and enable it in HdrcStart)
	 *  - make sure we can musb_start() after musb_stop(); with
	 *    OTG mode, gadget driver module rmmod/modprobe cycles that
	 *  - ...
	 */
	musb_platform_try_idle(musb, 0);
}

static void musb_shutdown(struct platform_device *pdev)
{
	struct musb *musb = dev_to_musb(&pdev->dev);
	unsigned long flags;
	DBG(0, "shut down\n");
	pm_runtime_get_sync(musb->controller);

	/* musb_gadget_cleanup(musb); */

	spin_lock_irqsave(&musb->lock, flags);
	musb_generic_disable(musb);
	musb_platform_disable(musb);
	spin_unlock_irqrestore(&musb->lock, flags);
#ifdef CONFIG_USB_MTK_HDRC_HCD
	if (musb->is_host)
		musb_platform_set_vbus(musb, 0);
#endif

	musb_writeb(musb->mregs, MUSB_DEVCTL, 0);
	musb_platform_exit(musb);

	pm_runtime_put(musb->controller);
	/* FIXME power down */
}

/*
 * configure a fifo; for non-shared endpoints, this may be called
 * once for a tx fifo and once for an rx fifo.
 *
 * returns negative errno or offset for next fifo.
 */
static int fifo_setup(struct musb *musb, struct musb_hw_ep *hw_ep,
		      const struct musb_fifo_cfg *cfg, u16 offset)
{
/* void __iomem    *mbase = musb->mregs; */
	int size = 0;
	u16 maxpacket = cfg->maxpacket;
	u16 c_off = offset >> 3;
	u8 c_size;

	/* expect hw_ep has already been zero-initialized */

	size = ffs(max(maxpacket, (u16) 8)) - 1;
	maxpacket = 1 << size;

	c_size = size - 3;
/*	if (cfg->mode == MUSB_BUF_DOUBLE) {
		if ((offset + (maxpacket << 1)) >(musb->fifo_size))
				return -EMSGSIZE;
		c_size |= MUSB_FIFOSZ_DPB;
	}else if ((offset + maxpacket) >(musb->fifo_size))
				return -EMSGSIZE;*/

	/* configure the FIFO */
/* musb_writeb(mbase, MUSB_INDEX, hw_ep->epnum); */

	switch (cfg->style) {
	case MUSB_FIFO_TX:
		DBG(0, "Tx ep %d fifo size is %d fifo address is %x\n", hw_ep->epnum, maxpacket,
		    c_off);
/* musb_write_txfifosz(mbase, c_size); */
/* musb_write_txfifoadd(mbase, c_off); */
		hw_ep->tx_double_buffered = !!(c_size & MUSB_FIFOSZ_DPB);
		hw_ep->max_packet_sz_tx = maxpacket;
		hw_ep->ep_in.fifo_size = maxpacket;
		hw_ep->ep_in.fifo_mode = cfg->mode;
		break;
	case MUSB_FIFO_RX:
		DBG(0, "Rx ep %d fifo size is %d fifo address is %x\n", hw_ep->epnum, maxpacket,
		    c_off);
/* musb_write_rxfifosz(mbase, c_size); */
/* musb_write_rxfifoadd(mbase, c_off); */
		hw_ep->rx_double_buffered = !!(c_size & MUSB_FIFOSZ_DPB);
		hw_ep->max_packet_sz_rx = maxpacket;
		hw_ep->ep_out.fifo_size = maxpacket;
		hw_ep->ep_out.fifo_mode = cfg->mode;
		break;
	case MUSB_FIFO_RXTX:
/* musb_write_txfifosz(mbase, c_size); */
/* musb_write_txfifoadd(mbase, c_off); */
		hw_ep->rx_double_buffered = !!(c_size & MUSB_FIFOSZ_DPB);
		hw_ep->max_packet_sz_rx = maxpacket;

/* musb_write_rxfifosz(mbase, c_size); */
/* musb_write_rxfifoadd(mbase, c_off); */
		hw_ep->tx_double_buffered = hw_ep->rx_double_buffered;
		hw_ep->max_packet_sz_tx = maxpacket;

		hw_ep->is_shared_fifo = true;
		hw_ep->ep_in.fifo_size = maxpacket;
		hw_ep->ep_out.fifo_size = maxpacket;
		hw_ep->ep_in.fifo_mode = cfg->mode;
		hw_ep->ep_out.fifo_mode = cfg->mode;
		break;
	}

	/* NOTE rx and tx endpoint irqs aren't managed separately,
	 * which happens to be ok
	 */

	hw_ep->ep_mode = cfg->ep_mode;	/* set the ep mode:ISO INT CONT or BULK */

	/* NOTE rx and tx endpoint irqs aren't managed separately,
	 * which happens to be ok
	 */
	musb->epmask |= (1 << hw_ep->epnum);

	return offset + (maxpacket << ((c_size & MUSB_FIFOSZ_DPB) ? 1 : 0));
}

static struct musb_fifo_cfg ep0_cfg = {
	.style = MUSB_FIFO_RXTX, .maxpacket = 64, .ep_mode = EP_CONT,
};

static int ep_config_from_table(struct musb *musb)
{
	const struct musb_fifo_cfg *cfg;
	unsigned i, n;
	int offset;
	struct musb_hw_ep *hw_ep = musb->endpoints;

	if (musb->fifo_cfg) {
		cfg = musb->fifo_cfg;
		n = musb->fifo_cfg_size;
	} else
		return -EINVAL;
	offset = fifo_setup(musb, hw_ep, &ep0_cfg, 0);
	/* assert(offset > 0) */

	/* NOTE:  for RTL versions >= 1.400 EPINFO and RAMINFO would
	 * be better than static musb->config->num_eps and DYN_FIFO_SIZE...
	 */

	for (i = 0; i < n; i++) {
		u8 epn = cfg->hw_ep_num;

		if (epn >= MUSB_C_NUM_EPS) {
			DBG(0, "%s: invalid ep %d\n", musb_driver_name, epn);
			return -EINVAL;
		}
		offset = fifo_setup(musb, hw_ep + epn, cfg++, offset);
		if (offset < 0) {
			DBG(0, "%s: mem overrun, ep %d\n", musb_driver_name, epn);
			return offset;
		}

		epn++;
		musb->nr_endpoints = max(epn, musb->nr_endpoints);
	}

	DBG(2, "%s: %d/%d max ep, %d/%d memory\n",
	    musb_driver_name,
	    n + 1, musb->config->num_eps * 2 - 1, offset, (1 << (musb->config->ram_bits + 2)));

	return 0;
}

static int fifo_setup_for_host(struct musb *musb, struct musb_hw_ep *hw_ep,
			       const struct musb_fifo_cfg *cfg, u16 offset)
{
	void __iomem *mbase = musb->mregs;
	int size = 0;
	u16 maxpacket = cfg->maxpacket;
	u16 c_off = offset >> 3;
	u8 c_size;

	/* expect hw_ep has already been zero-initialized */
	DBG(4, "++,hw_ep->epnum=%d\n", hw_ep->epnum);
	size = ffs(max(maxpacket, (u16) 8)) - 1;
	maxpacket = 1 << size;

	c_size = size - 3;
	if (cfg->mode == MUSB_BUF_DOUBLE) {
		if ((offset + (maxpacket << 1)) > (musb->fifo_size))
			return -EMSGSIZE;
		c_size |= MUSB_FIFOSZ_DPB;
	} else if ((offset + maxpacket) > (musb->fifo_size))
		return -EMSGSIZE;

	/* configure the FIFO */
	musb_writeb(mbase, MUSB_INDEX, hw_ep->epnum);

	switch (cfg->style) {
	case MUSB_FIFO_TX:
		DBG(4, "Tx ep %d fifo size is %d fifo address is %x\n", hw_ep->epnum, c_size,
		    c_off);
		musb_write_txfifosz(mbase, c_size);
		musb_write_txfifoadd(mbase, c_off);
		hw_ep->tx_double_buffered = !!(c_size & MUSB_FIFOSZ_DPB);
		hw_ep->max_packet_sz_tx = maxpacket;
		break;
	case MUSB_FIFO_RX:
		DBG(4, "Rx ep %d fifo size is %d fifo address is %x\n", hw_ep->epnum, c_size,
		    c_off);
		musb_write_rxfifosz(mbase, c_size);
		musb_write_rxfifoadd(mbase, c_off);
		hw_ep->rx_double_buffered = !!(c_size & MUSB_FIFOSZ_DPB);
		hw_ep->max_packet_sz_rx = maxpacket;
		break;
	case MUSB_FIFO_RXTX:
		musb_write_txfifosz(mbase, c_size);
		musb_write_txfifoadd(mbase, c_off);
		hw_ep->rx_double_buffered = !!(c_size & MUSB_FIFOSZ_DPB);
		hw_ep->max_packet_sz_rx = maxpacket;

		musb_write_rxfifosz(mbase, c_size);
		musb_write_rxfifoadd(mbase, c_off);
		hw_ep->tx_double_buffered = hw_ep->rx_double_buffered;
		hw_ep->max_packet_sz_tx = maxpacket;

		hw_ep->is_shared_fifo = true;
		break;
	}

	hw_ep->ep_mode = cfg->ep_mode;	/* set the ep mode:ISO INT CONT or BULK */

	/* NOTE rx and tx endpoint irqs aren't managed separately,
	 * which happens to be ok
	 */
	musb->epmask |= (1 << hw_ep->epnum);

	return offset + (maxpacket << ((c_size & MUSB_FIFOSZ_DPB) ? 1 : 0));
}

int ep_config_from_table_for_host(struct musb *musb)
{
	const struct musb_fifo_cfg *cfg;
	unsigned i, n;
	int offset;
	struct musb_hw_ep *hw_ep = musb->endpoints;
	if (musb->fifo_cfg_host) {
		cfg = musb->fifo_cfg_host;
		n = musb->fifo_cfg_host_size;
	} else {
		return -EINVAL;
	}
	offset = fifo_setup_for_host(musb, hw_ep, &ep0_cfg, 0);
	/* assert(offset > 0) */

	/* NOTE:  for RTL versions >= 1.400 EPINFO and RAMINFO would
	 * be better than static musb->config->num_eps and DYN_FIFO_SIZE...
	 */

	for (i = 0; i < n; i++) {
		u8 epn = cfg->hw_ep_num;

		if (epn >= musb->config->num_eps) {
			DBG(0, "%s: invalid ep %d\n", musb_driver_name, epn);
			return -EINVAL;
		}
		offset = fifo_setup_for_host(musb, hw_ep + epn, cfg++, offset);
		if (offset < 0) {
			DBG(0, "%s: mem overrun, ep %d\n", musb_driver_name, epn);
			return offset;
		}
		epn++;
		musb->nr_endpoints = max(epn, musb->nr_endpoints);
	}

	DBG(2, "%s: %d/%d max ep, %d/%d memory\n",
	    musb_driver_name,
	    n + 1, musb->config->num_eps * 2 - 1, offset, (1 << (musb->config->ram_bits + 2)));

	return 0;
}


/*
 * ep_config_from_hw - when MUSB_C_DYNFIFO_DEF is false
 * @param musb the controller
 */
static int ep_config_from_hw(struct musb *musb)
{
	u8 epnum = 0;
	struct musb_hw_ep *hw_ep;
	void __iomem *mbase = musb->mregs;
	int ret = 0;

	DBG(2, "<== static silicon ep config\n");

	/* FIXME pick up ep0 maxpacket size */

	for (epnum = 1; epnum < musb->config->num_eps; epnum++) {
		musb_ep_select(mbase, epnum);
		hw_ep = musb->endpoints + epnum;

		ret = musb_read_fifosize(musb, hw_ep, epnum);
		if (ret < 0)
			break;

		/* FIXME set up hw_ep->{rx,tx}_double_buffered */

		/* pick an RX/TX endpoint for bulk */
		if (hw_ep->max_packet_sz_tx < 512 || hw_ep->max_packet_sz_rx < 512)
			continue;

		/* REVISIT:  this algorithm is lazy, we should at least
		 * try to pick a double buffered endpoint.
		 */
		if (musb->bulk_ep)
			continue;
		musb->bulk_ep = hw_ep;
	}

	if (!musb->bulk_ep) {
		pr_debug("%s: missing bulk\n", musb_driver_name);
		return -EINVAL;
	}

	return 0;
}

enum { MUSB_CONTROLLER_MHDRC, MUSB_CONTROLLER_HDRC, };

/* Initialize MUSB (M)HDRC part of the USB hardware subsystem;
 * configure endpoints, or take their config from silicon
 */
static int musb_core_init(u16 musb_type, struct musb *musb)
{
	u8 reg;
	char *type;
	char aInfo[90], aRevision[32], aDate[12];
	void __iomem *mbase = musb->mregs;
	int status = 0;
	int i;

	/* log core options (read using indexed model) */
	reg = musb_read_configdata(mbase);

	strcpy(aInfo, (reg & MUSB_CONFIGDATA_UTMIDW) ? "UTMI-16" : "UTMI-8");
	if (reg & MUSB_CONFIGDATA_DYNFIFO) {
		strcat(aInfo, ", dyn FIFOs");
		musb->dyn_fifo = true;
	}
	if (reg & MUSB_CONFIGDATA_MPRXE) {
		strcat(aInfo, ", bulk combine");
		musb->bulk_combine = true;
	}
	if (reg & MUSB_CONFIGDATA_MPTXE) {
		strcat(aInfo, ", bulk split");
		musb->bulk_split = true;
	}
	if (reg & MUSB_CONFIGDATA_HBRXE) {
		strcat(aInfo, ", HB-ISO Rx");
		musb->hb_iso_rx = true;
	}
	if (reg & MUSB_CONFIGDATA_HBTXE) {
		strcat(aInfo, ", HB-ISO Tx");
		musb->hb_iso_tx = true;
	}
	if (reg & MUSB_CONFIGDATA_SOFTCONE)
		strcat(aInfo, ", SoftConn");

	DBG(0, "%s: ConfigData=0x%02x (%s)\n", musb_driver_name, reg, aInfo);

	aDate[0] = 0;
	if (MUSB_CONTROLLER_MHDRC == musb_type) {
		musb->is_multipoint = 1;
		type = "M";
	} else {
		musb->is_multipoint = 0;
		type = "";
#ifndef	CONFIG_USB_OTG_BLACKLIST_HUB
		pr_err("%s: kernel must blacklist external hubs\n", musb_driver_name);
#endif
	}

	/* log release info */
	musb->hwvers = musb_read_hwvers(mbase);
	snprintf(aRevision, 32, "%d.%d%s", MUSB_HWVERS_MAJOR(musb->hwvers),
		 MUSB_HWVERS_MINOR(musb->hwvers), (musb->hwvers & MUSB_HWVERS_RC) ? "RC" : "");
	pr_debug("%s: %sHDRC RTL version %s %s\n", musb_driver_name, type, aRevision, aDate);

	/* configure ep0 */
	musb_configure_ep0(musb);

	/* discover endpoint configuration */
	musb->nr_endpoints = 1;
	musb->epmask = 1;


	if (musb->dyn_fifo) {
		status = ep_config_from_table(musb);
		DBG(0, "ep_config_from_table %d\n", status);
	} else {
		status = ep_config_from_hw(musb);
		DBG(0, "ep_config_from_hw %d\n", status);
	}


	if (status < 0)
		return status;

	musb->ep_fifo_total_sz = musb->fifo_cfg_size;
	musb->ep_fifo = 0;
	DBG(0, "fifo_total_sz:%d\n", musb->ep_fifo_total_sz);
	/* finish init, and print endpoint config */
	for (i = 0; i < musb->nr_endpoints; i++) {
		struct musb_hw_ep *hw_ep = musb->endpoints + i;

		hw_ep->fifo = MUSB_FIFO_OFFSET(i) + mbase;
		hw_ep->regs = MUSB_EP_OFFSET(i, 0) + mbase;
#ifdef CONFIG_USB_MTK_HDRC_HCD
		hw_ep->rx_reinit = 1;
		hw_ep->tx_reinit = 1;
#endif
		if (hw_ep->max_packet_sz_tx) {
			DBG(0,
			    "%s: hw_ep %d%s, %smax %d\n",
			    musb_driver_name, i,
			    hw_ep->is_shared_fifo ? "shared" : "tx",
			    hw_ep->tx_double_buffered
			    ? "doublebuffer, " : "", hw_ep->max_packet_sz_tx);
		}
		if (hw_ep->max_packet_sz_rx && !hw_ep->is_shared_fifo) {
			DBG(0,
			    "%s: hw_ep %d%s, %smax %d\n",
			    musb_driver_name, i,
			    "rx",
			    hw_ep->rx_double_buffered
			    ? "doublebuffer, " : "", hw_ep->max_packet_sz_rx);
		}
		if (!(hw_ep->max_packet_sz_tx || hw_ep->max_packet_sz_rx))
			DBG(0, "hw_ep %d not configured\n", i);
	}
	DBG(0, "musb_core_init end\n");
	return 0;
}



/*
 * handle all the irqs defined by the HDRC core. for now we expect:  other
 * irq sources (phy, dma, etc) will be handled first, musb->int_* values
 * will be assigned, and the irq will already have been acked.
 *
 * called in irq context with spinlock held, irqs blocked
 */
irqreturn_t musb_interrupt(struct musb *musb)
{
	irqreturn_t retval = IRQ_NONE;
	u8 devctl;
	int ep_num;
	u32 reg;

	devctl = musb_readb(musb->mregs, MUSB_DEVCTL);
#ifdef MUSB_QMU_SUPPORT
//	DBG(1,"usb(%x) tx(%x) rx(%x) queue(%x)\n", musb->int_usb, musb->int_tx, musb->int_rx, musb->int_queue); //print interrupt status to see which function will be going.
#else
	DBG(1, "** IRQ %s usb%04x tx%04x rx%04x\n",
	    (devctl & MUSB_DEVCTL_HM) ? "host" : "peripheral",
	    musb->int_usb, musb->int_tx, musb->int_rx);
#endif

	USB_LOGGER(MUSB_INTERRUPT, MUSB_INTERRUPT,
		   (musb->is_host) ? "Host" : "Dev", musb->int_usb, musb->int_tx, musb->int_rx);

	dumpTime(funcInterrupt, 0);

	/* the core can interrupt us for multiple reasons; docs have
	 * a generic interrupt flowchart to follow
	 */
	if (musb->int_usb)
		retval |= musb_stage0_irq(musb, musb->int_usb, devctl);

	/* "stage 1" is handling endpoint irqs */

	/* handle endpoint 0 first */
	if (musb->int_tx & 1) {
		if (devctl & MUSB_DEVCTL_HM)
			retval |= musb_h_ep0_irq(musb);
		else
			retval |= musb_g_ep0_irq(musb);
	}

#ifdef MUSB_QMU_SUPPORT
	/* process generic queue interrupt*/
	if (musb->int_queue)
	{
		musb_q_irq(musb);
		retval = IRQ_HANDLED;
	}
#endif
	/* RX on endpoints 1-15 */
	reg = musb->int_rx >> 1;
	ep_num = 1;
	while (reg) {
		if (reg & 1) {
			/* musb_ep_select(musb->mregs, ep_num); */
			/* REVISIT just retval = ep->rx_irq(...) */
			retval = IRQ_HANDLED;
			if (devctl & MUSB_DEVCTL_HM) {
				musb_host_rx(musb, ep_num);
				}
			else {
				musb_g_rx(musb, ep_num);
				}
		}

		reg >>= 1;
		ep_num++;
	}

	/* TX on endpoints 1-15 */
	reg = musb->int_tx >> 1;
	ep_num = 1;
	while (reg) {
		if (reg & 1) {
			/* musb_ep_select(musb->mregs, ep_num); */
			/* REVISIT just retval |= ep->tx_irq(...) */
			retval = IRQ_HANDLED;
			if (devctl & MUSB_DEVCTL_HM)
				musb_host_tx(musb, ep_num);
			else
				musb_g_tx(musb, ep_num);
		}
		reg >>= 1;
		ep_num++;
	}

	return retval;
}
EXPORT_SYMBOL_GPL(musb_interrupt);

#ifndef CONFIG_MUSB_PIO_ONLY
static bool use_dma = 1;

/* "modprobe ... use_dma=0" etc */
module_param(use_dma, bool, 0);
MODULE_PARM_DESC(use_dma, "enable/disable use of DMA");

void musb_dma_completion(struct musb *musb, u8 epnum, u8 transmit)
{
	u8 devctl = musb_readb(musb->mregs, MUSB_DEVCTL);
	DBG(1, "musb_dma_completion %d   tx %d\n", epnum, transmit);

	/* called with controller lock already held */
	if (musb->xceiv->state >= OTG_STATE_A_IDLE)
		devctl |= MUSB_DEVCTL_HM;
	if (!epnum) {
		/* endpoint 0 */
		if (devctl & MUSB_DEVCTL_HM)
			musb_h_ep0_irq(musb);
		else
			musb_g_ep0_irq(musb);
	} else {
		/* endpoints 1..15 */
		if (transmit) {
			if (devctl & MUSB_DEVCTL_HM)
				musb_host_tx(musb, epnum);
			else
				musb_g_tx(musb, epnum);
		} else {
			/* receive */
			if (devctl & MUSB_DEVCTL_HM)
				musb_host_rx(musb, epnum);
			else
				musb_g_rx(musb, epnum);
		}
	}
}
EXPORT_SYMBOL_GPL(musb_dma_completion);

#else
#define use_dma			0
#endif

/*-------------------------------------------------------------------------*/
#ifdef CONFIG_SYSFS

static ssize_t musb_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct musb *musb = dev_to_musb(dev);
	unsigned long flags;
	int ret = -EINVAL;

	spin_lock_irqsave(&musb->lock, flags);
	ret = sprintf(buf, "%s\n", otg_state_string(musb->xceiv->state));
	spin_unlock_irqrestore(&musb->lock, flags);

	return ret;
}

static ssize_t
musb_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
	struct musb *musb = dev_to_musb(dev);
	unsigned long flags;
	int status;

	spin_lock_irqsave(&musb->lock, flags);
	if (sysfs_streq(buf, "host"))
		status = musb_platform_set_mode(musb, MUSB_HOST);
	else if (sysfs_streq(buf, "peripheral"))
		status = musb_platform_set_mode(musb, MUSB_PERIPHERAL);
	else if (sysfs_streq(buf, "otg"))
		status = musb_platform_set_mode(musb, MUSB_OTG);
	else
		status = -EINVAL;
	spin_unlock_irqrestore(&musb->lock, flags);

	return (status == 0) ? n : status;
}

static DEVICE_ATTR(mode, 0644, musb_mode_show, musb_mode_store);

static ssize_t
musb_vbus_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
	struct musb *musb = dev_to_musb(dev);
	unsigned long flags;
	unsigned long val;

	if (sscanf(buf, "%lu", &val) < 1) {
		DBG(0, "Invalid VBUS timeout ms value\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&musb->lock, flags);
	/* force T(a_wait_bcon) to be zero/unlimited *OR* valid */
	musb->a_wait_bcon = val ? max_t(int, val, OTG_TIME_A_WAIT_BCON) : 0;
	if (musb->xceiv->state == OTG_STATE_A_WAIT_BCON)
		musb->is_active = 0;
	musb_platform_try_idle(musb, jiffies + msecs_to_jiffies(val));
	spin_unlock_irqrestore(&musb->lock, flags);

	return n;
}

static ssize_t musb_vbus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct musb *musb = dev_to_musb(dev);
	unsigned long flags;
	unsigned long val;
	int vbus;

	spin_lock_irqsave(&musb->lock, flags);
	val = musb->a_wait_bcon;
	/* FIXME get_vbus_status() is normally #defined as false...
	 * and is effectively TUSB-specific.
	 */
	vbus = musb_platform_get_vbus_status(musb);
	spin_unlock_irqrestore(&musb->lock, flags);

	return sprintf(buf, "Vbus %s, timeout %lu msec\n", vbus ? "on" : "off", val);
}

static DEVICE_ATTR(vbus, 0644, musb_vbus_show, musb_vbus_store);

/* Gadget drivers can't know that a host is connected so they might want
 * to start SRP, but users can.  This allows userspace to trigger SRP.
 */
static ssize_t
musb_srp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
	struct musb *musb = dev_to_musb(dev);
	unsigned short srp;

	if (sscanf(buf, "%hu", &srp) != 1 || (srp != 1)) {
		DBG(0, "SRP: Value must be 1\n");
		return -EINVAL;
	}

	if (srp == 1)
		musb_g_wakeup(musb);

	return n;
}

static DEVICE_ATTR(srp, 0644, NULL, musb_srp_store);

//This host_strength is shared by musb (PORT1)
void print_vref_value(void)
{
	char vref_sel = (USBPHY_READ8(0x05) & 0x70) >> 4;
	int value = 0;

	switch (vref_sel)
	{
		case 0 :
			value = 700;
			break;
		case 1 :
			value = 720;
			break;
		case 2 :
			value = 740;
			break;
		case 3 :
			value = 760;
			break;
		case 4 :
			value = 770;
			break;
		case 5 :
			value = 780;
			break;
		case 6 :
			value = 800;
			break;
		case 7 :
			value = 820;
			break;
	}

	printk("usb host strength VREF_SEL(%d) = %dmV\n", vref_sel, value);
}

void set_vref(char vref_sel)
{
	USBPHY_CLR8(0x05, 0x70);
	USBPHY_SET8(0x05, vref_sel << 4);
}

ssize_t musb_show_host_strength(struct device* dev, struct device_attribute *attr, char *buf)
{
	print_vref_value();	
	return 0;
}

ssize_t musb_store_host_strength(struct device* dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned char vref_sel = 0;
	unsigned int value = 0;
    int ret = 0;

    if (!dev) {
        DBG(0,"dev is null!!\n");
        return count;
    }
    if (buf != NULL && count != 0) {
	    ret = sscanf(buf, "%1d", &value);
	    vref_sel = (char) value;
		if ((ret != 1) || (vref_sel > 7))
		{
			printk("Setting Value(%d) is not accepted\n",vref_sel);
			return -1;
		}
    }

	set_vref(vref_sel);
	print_vref_value();
	
	return count;
}

static DEVICE_ATTR(host_strength,  0664, musb_show_host_strength, musb_store_host_strength);

static struct attribute *musb_attributes[] = {
	&dev_attr_mode.attr,
	&dev_attr_vbus.attr,
	&dev_attr_srp.attr,
	&dev_attr_host_strength.attr,
	NULL
};

static const struct attribute_group musb_attr_group = {
	.attrs = musb_attributes,
};
#endif				/* sysfs */

/* Only used to provide driver mode change events */
static void musb_irq_work(struct work_struct *data)
{
	struct musb *musb = container_of(data, struct musb, irq_work);

	if (musb->xceiv->state != musb->xceiv_old_state) {
		musb->xceiv_old_state = musb->xceiv->state;
		sysfs_notify(&musb->controller->kobj, NULL, "mode");
	}
}

/* --------------------------------------------------------------------------
 * Init support
 */

static struct musb *allocate_instance(struct device *dev,
				      struct musb_hdrc_config *config, void __iomem *mbase)
{
	struct musb *musb;
	struct musb_hw_ep *ep;
	int epnum;
	struct usb_hcd *hcd;

	hcd = usb_create_hcd(&musb_hc_driver, dev, dev_name(dev));
	if (!hcd)
		return NULL;
	/* usbcore sets dev->driver_data to hcd, and sometimes uses that... */

	musb = hcd_to_musb(hcd);
	INIT_LIST_HEAD(&musb->control);
	INIT_LIST_HEAD(&musb->in_bulk);
	INIT_LIST_HEAD(&musb->out_bulk);

	hcd->uses_new_polling = 1;
	hcd->has_tt = 1;

	musb->vbuserr_retry = VBUSERR_RETRY_COUNT;
	musb->a_wait_bcon = OTG_TIME_A_WAIT_BCON;
	dev_set_drvdata(dev, musb);
	musb->mregs = mbase;
	musb->ctrl_base = mbase;
	musb->nIrq = -ENODEV;
	musb->config = config;
	musb->is_ready = false;
	musb->in_ipo_off = false;

	BUG_ON(musb->config->num_eps > MUSB_C_NUM_EPS);
	for (epnum = 0, ep = musb->endpoints; epnum < musb->config->num_eps; epnum++, ep++) {
		ep->musb = musb;
		ep->epnum = epnum;
	}

	musb->controller = dev;

	return musb;
}

static void musb_free(struct musb *musb)
{
	/* this has multiple entry modes. it handles fault cleanup after
	 * probe(), where things may be partially set up, as well as rmmod
	 * cleanup after everything's been de-activated.
	 */
#ifdef CONFIG_SYSFS
	sysfs_remove_group(&musb->controller->kobj, &musb_attr_group);
#endif

#ifdef MUSB_QMU_SUPPORT
	musb_gadget_cleanup(musb);
	musb_disable_q_all(musb);
	musb_qmu_exit(musb);
#endif
	if (musb->nIrq >= 0) {
		if (musb->irq_wake)
			disable_irq_wake(musb->nIrq);
		free_irq(musb->nIrq, musb);
	}
	if (is_dma_capable() && musb->dma_controller) {
		struct dma_controller *c = musb->dma_controller;

		(void)c->stop(c);
		dma_controller_destroy(c);
	}

	usb_put_hcd(musb_to_hcd(musb));
}

#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
/* SONY ICX-DMP platform feature. */
static int musb_phy_set_power(struct usb_phy *x, unsigned mA)
{	return bq24262_wmport_usb_set_power_event(mA);
}
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */

#if (defined(MUSB_PORT0_HOST))
extern void musb_id_pin_work_host(void);
#endif 

#ifdef ID_PIN_USE_EX_EINT
extern void musb_do_infra_misc(bool enable);
#endif
/*
 * Perform generic per-controller initialization.
 *
 * @dev: the controller (already clocked, etc)
 * @nIrq: IRQ number
 * @ctrl: virtual address of controller registers,
 *	not yet corrected for platform-specific offsets
 */
static int musb_init_controller(struct device *dev, int nIrq, void __iomem *ctrl)
{
	int status;
	struct musb *musb;
	struct musb_hdrc_platform_data *plat = dev->platform_data;
	struct usb_hcd *hcd;
	u8 u8_busperf3 = 0;
	/* The driver might handle more features than the board; OK.
	 * Fail when the board needs a feature that's not enabled.
	 */
	if (!plat) {
		DBG(0, "no platform_data?\n");
		status = -ENODEV;
		goto fail0;
	}

	/* allocate */
	musb = allocate_instance(dev, plat->config, ctrl);
	if (!musb) {
		status = -ENOMEM;
		goto fail0;
	}

	mtk_musb = musb;       
	sema_init(&musb->musb_lock, 1);
	pm_runtime_use_autosuspend(musb->controller);
	pm_runtime_set_autosuspend_delay(musb->controller, 200);
	pm_runtime_enable(musb->controller);

	spin_lock_init(&musb->lock);
	musb->board_set_power = plat->set_power;
	musb->min_power = plat->min_power;
	musb->ops = plat->platform_ops;
    
	/* The musb_platform_init() call:
	 *   - adjusts musb->mregs
	 *   - sets the musb->isr
	 *   - may initialize an integrated tranceiver
	 *   - initializes musb->xceiv, usually by otg_get_phy()
	 *   - stops powering VBUS
	 *
	 * There are various transceiver configurations.  Blackfin,
	 * DaVinci, TUSB60x0, and others integrate them.  OMAP3 uses
	 * external/discrete ones in various flavors (twl4030 family,
	 * isp1504, non-OTG, etc) mostly hooking up through ULPI.
	 */
	u8_busperf3= musb_readb(musb->mregs, 0x74);
	u8_busperf3 &= ~(0x40);
	u8_busperf3 |= 0x80;
	musb_writeb(musb->mregs, 0x74,u8_busperf3);
	status = musb_platform_init(musb);
#ifdef MUSB_QMU_SUPPORT
	musb_qmu_init(musb);
#ifdef QMU_TASKLET
	tasklet_init(&musb->qmu_done, qmu_done_tasklet, (unsigned long)musb);
#endif
#endif
	if (status < 0)
		goto fail1;

	if (!musb->isr) {
		status = -ENODEV;
		goto fail2;
	}

	if (!musb->xceiv->io_ops) {
		musb->xceiv->io_dev = musb->controller;
		musb->xceiv->io_priv = musb->mregs;
		musb->xceiv->io_ops = &musb_ulpi_access;
	}
#if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
/* SONY ICX-DMP platform feature. */
	musb->xceiv->set_power = musb_phy_set_power;
#endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */

	pm_runtime_get_sync(musb->controller);

#ifndef CONFIG_MUSB_PIO_ONLY
	if (use_dma && dev->dma_mask) {
		struct dma_controller *c;

		c = dma_controller_create(musb, musb->mregs);
		musb->dma_controller = c;
		if (c)
			(void)c->start(c);
	}
#endif
	/* ideally this would be abstracted in platform setup */
	if (!is_dma_capable() || !musb->dma_controller)
		dev->dma_mask = NULL;

	/* be sure interrupts are disabled before connecting ISR */
	musb_generic_disable(musb);
	musb_platform_disable(musb);

	/* setup musb parts of the core (especially endpoints) */
	status = musb_core_init(plat->config->multipoint
				? MUSB_CONTROLLER_MHDRC : MUSB_CONTROLLER_HDRC, musb);
	if (status < 0)
		goto fail3;

	setup_timer(&musb->otg_timer, musb_otg_timer_func, (unsigned long)musb);

	/* Init IRQ workqueue before request_irq */
	INIT_WORK(&musb->irq_work, musb_irq_work);

	/* attach to the IRQ */
	if (request_irq(musb->nIrq, musb->isr, IRQF_TRIGGER_LOW, dev_name(dev), musb)) {
		DBG(0, "request_irq %d failed!\n", musb->nIrq);
		status = -ENODEV;
		goto fail3;
	}
#if 0
	musb->nIrq = nIrq;
	/* FIXME this handles wakeup irqs wrong */
	if (enable_irq_wake(nIrq) == 0) {
		musb->irq_wake = 1;
		device_init_wakeup(dev, 1);
	} else {
		musb->irq_wake = 0;
	}
#endif
	/* host side needs more setup */
	hcd = musb_to_hcd(musb);
#if ((defined(CONFIG_USB11_MTK_HDRC_GADGET)) && (!defined(CONFIG_USB_MTK_PORT0_CHARGE)))
	status = usb_add_hcd(hcd, -1, 0);
	printk(KERN_ERR "musb_init_controller usb_add_hcd \n");
	if (status < 0) {
		DBG(2, "add_hcd failed, %d\n", status);
		goto fail3;
	}
#endif
	otg_set_host(musb->xceiv->otg, &hcd->self);
	hcd->self.otg_port = 1;
	musb->xceiv->otg->host = &hcd->self;
	hcd->power_budget = 2 * (plat->power ? : 250);

	/* program PHY to use external vBus if required */
	if (plat->extvbus) {
		u8 busctl = musb_read_ulpi_buscontrol(musb->mregs);
		busctl |= MUSB_ULPI_USE_EXTVBUS;
		musb_write_ulpi_buscontrol(musb->mregs, busctl);
	}

	if (musb->xceiv->otg->default_a) {
		MUSB_HST_MODE(musb);
		musb->xceiv->state = OTG_STATE_A_IDLE;
	} else {
		MUSB_DEV_MODE(musb);
		musb->xceiv->state = OTG_STATE_B_IDLE;
	}

#if ((!defined(CONFIG_USB11_MTK_HDRC_GADGET)) || (defined(CONFIG_USB_MTK_PORT0_CHARGE)))
	status = musb_gadget_setup(musb);
	if (status < 0)
	{
		printk("[long]%s[%d] ret %d\n", __FUNCTION__, __LINE__, status);
		goto fail3;
	}
 #endif
    
	musb_platform_disable(musb);

#if (defined(MUSB_PORT0_HOST))
	musb_id_pin_work_host();
	DBG(0, "musb_id_pin_work_host init \n");
#endif

	status = musb_init_debugfs(musb);
	if (status < 0)
	{
		printk("[long]%s[%d] ret %d\n", __FUNCTION__, __LINE__, status);
		goto fail4;
	}

#ifdef CONFIG_SYSFS
	status = sysfs_create_group(&musb->controller->kobj, &musb_attr_group);
	if (status)
	{
		printk("[long]%s[%d] ret %d\n", __FUNCTION__, __LINE__, status);
		goto fail5;
	}
#endif

	pm_runtime_put(musb->controller);
#if (defined(MUSB_PORT0_HOST))
#if ((defined(ID_PIN_USE_EX_EINT)) && (!defined(CONFIG_USB_MTK_PORT0_CHARGE)))
	musb_do_infra_misc(true);
#endif
#endif
	return 0;

#ifdef CONFIG_SYSFS
 fail5:
	musb_exit_debugfs(musb);
#endif

 fail4:
	musb_gadget_cleanup(musb);

 fail3:
	pm_runtime_put_sync(musb->controller);

 fail2:
	if (musb->irq_wake)
		device_init_wakeup(dev, 0);
	musb_platform_exit(musb);

 fail1:
	DBG(0, "musb_init_controller failed with status %d\n", status);

	musb_free(musb);

 fail0:

	return status;

}

/*-------------------------------------------------------------------------*/

/* all implementations (PCI bridge to FPGA, VLYNQ, etc) should just
 * bridge to a platform device; this driver then suffices.
 */
static int musb_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int irq = 0;
	int status;
	/* struct resource      *iomem; */
	void __iomem *base;

  int ret;
  
  #if (defined(CONFIG_CHARGER_BQ24262_WMPORT))
  usb0_suspend_state_workqueue = create_singlethread_workqueue("usb0_g_state");
  INIT_DELAYED_WORK(&usb0_suspend_state_work, usb0_suspend_state_event_notify);
  usb0_suspend_state_sw.name = usb0_suspend_state_sw_name;
  usb0_suspend_state_sw.state = USB0_GADGET_STATE_SWITCH_SUSPEND;
  ret = switch_dev_register(&usb0_suspend_state_sw);
  
  if (ret != 0) {
    printk("%s: Can not create sysfs files. ret=%d\n", __func__, ret);
    return -ENODEV;
  }
  #endif /* (defined(CONFIG_CHARGER_BQ24262_WMPORT)) */

#if 1
	base = (void *)USB_BASE;
#else
	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iomem || irq <= 0)
		return -ENODEV;

	base = devm_ioremap_resource(dev, iomem);
	if (IS_ERR(base))
		return PTR_ERR(base);
#endif
	status = musb_init_controller(dev, irq, base);
	return status;
}

static int musb_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct musb *musb = dev_to_musb(dev);

	/* this gets called on rmmod.
	 *  - Host mode: host may still be active
	 *  - Peripheral mode: peripheral is deactivated (or never-activated)
	 *  - OTG mode: both roles are deactivated (or never-activated)
	 */
	musb_exit_debugfs(musb);
	musb_shutdown(pdev);

	musb_free(musb);
	device_init_wakeup(dev, 0);
#ifndef CONFIG_MUSB_PIO_ONLY
	dma_set_mask(dev, *dev->parent->dma_mask);
#endif
	return 0;
}

#ifdef CONFIG_PM

static void musb_save_context(struct musb *musb)
{
	int i;
	void __iomem *musb_base = musb->mregs;
	void __iomem *epio;

#ifdef MUSB_QMU_SUPPORT
	dma_burst_setting = musb_readl(musb->mregs, 0x204);
#endif

	musb->context.power = musb_readb(musb_base, MUSB_POWER);
	musb->context.intrusbe = musb_readb(musb_base, MUSB_INTRUSBE);
	musb->context.index = musb_readb(musb_base, MUSB_INDEX);
	musb->context.devctl = musb_readb(musb_base, MUSB_DEVCTL);

	musb->context.l1_int = musb_readl(musb_base, USB_L1INTM);

	for (i = 0; i < musb->config->num_eps; ++i) {
		struct musb_hw_ep *hw_ep;

		hw_ep = &musb->endpoints[i];
		if (!hw_ep)
			continue;

		epio = hw_ep->regs;
		if (!epio)
			continue;

		musb_writeb(musb_base, MUSB_INDEX, i);
		musb->context.index_regs[i].txmaxp = musb_readw(epio, MUSB_TXMAXP);
		musb->context.index_regs[i].txcsr = musb_readw(epio, MUSB_TXCSR);
		musb->context.index_regs[i].rxmaxp = musb_readw(epio, MUSB_RXMAXP);
		musb->context.index_regs[i].rxcsr = musb_readw(epio, MUSB_RXCSR);

		if (musb->dyn_fifo) {
			musb->context.index_regs[i].txfifoadd = musb_read_txfifoadd(musb_base);
			musb->context.index_regs[i].rxfifoadd = musb_read_rxfifoadd(musb_base);
			musb->context.index_regs[i].txfifosz = musb_read_txfifosz(musb_base);
			musb->context.index_regs[i].rxfifosz = musb_read_rxfifosz(musb_base);
		}
	}
}

static void musb_restore_context(struct musb *musb)
{
	int i;
	void __iomem *musb_base = musb->mregs;
	void __iomem *epio;
#ifdef MUSB_QMU_SUPPORT
	musb_writel(musb->mregs, 0x204 , dma_burst_setting);
#endif

	musb_writeb(musb_base, MUSB_POWER, musb->context.power);
	musb_writew(musb_base, MUSB_INTRTXE, musb->intrtxe);
	musb_writew(musb_base, MUSB_INTRRXE, musb->intrrxe);
	musb_writeb(musb_base, MUSB_INTRUSBE, musb->context.intrusbe);
	musb_writeb(musb_base, MUSB_DEVCTL, musb->context.devctl);

	for (i = 0; i < musb->config->num_eps; ++i) {
		struct musb_hw_ep *hw_ep;

		hw_ep = &musb->endpoints[i];
		if (!hw_ep)
			continue;

		epio = hw_ep->regs;
		if (!epio)
			continue;

		musb_writeb(musb_base, MUSB_INDEX, i);
		musb_writew(epio, MUSB_TXMAXP, musb->context.index_regs[i].txmaxp);
		musb_writew(epio, MUSB_TXCSR, musb->context.index_regs[i].txcsr);
		musb_writew(epio, MUSB_RXMAXP, musb->context.index_regs[i].rxmaxp);
		musb_writew(epio, MUSB_RXCSR, musb->context.index_regs[i].rxcsr);

		if (musb->dyn_fifo) {
			musb_write_txfifosz(musb_base, musb->context.index_regs[i].txfifosz);
			musb_write_rxfifosz(musb_base, musb->context.index_regs[i].rxfifosz);
			musb_write_txfifoadd(musb_base, musb->context.index_regs[i].txfifoadd);
			musb_write_rxfifoadd(musb_base, musb->context.index_regs[i].rxfifoadd);
		}
	}

	musb_writeb(musb_base, MUSB_INDEX, musb->context.index);

	/* Enable all interrupts at DMA
	 * Caution: The DMA Reg type is WRITE to SET or CLEAR
	 */
	musb_writel(musb->mregs, MUSB_HSDMA_INTR, 0xFF | (0xFF << DMA_INTR_UNMASK_SET_OFFSET));

	musb_writel(musb_base, USB_L1INTM, musb->context.l1_int);
}

static int musb_suspend_noirq(struct device *dev)
{
	struct musb *musb = dev_to_musb(dev);
	/*unsigned long flags; */

	/*No need spin lock in xxx_noirq() */
	/*spin_lock_irqsave(&musb->lock, flags); */
	/*Turn on USB clock, before reading a batch of regs */
	mtk_usb_power = true;
	#if !(defined(CONFIG_USB_MTK_PORT0_CHARGE))
	usb_enable_clock(true);
	#endif

	musb_save_context(musb);
	usb_phy_context_save();

	/*Turn off USB clock, after finishing reading regs */
	usb_enable_clock(false);
	mtk_usb_power = false;

	/*spin_unlock_irqrestore(&musb->lock, flags); */
	return 0;
}


static int musb_resume_noirq(struct device *dev)
{
	struct musb *musb = dev_to_musb(dev);

	/*Turn on USB clock, before writing a batch of regs */
	mtk_usb_power = true;
	usb_enable_clock(true);

	musb_restore_context(musb);
	usb_phy_context_restore();
	/*Turn off USB clock, after finishing writing regs */
	usb_enable_clock(false);
	mtk_usb_power = false;

	return 0;
}

#if 0
static int musb_runtime_suspend(struct device *dev)
{
	struct musb *musb = dev_to_musb(dev);

	musb_save_context(musb);

	return 0;
}

static int musb_runtime_resume(struct device *dev)
{
	struct musb *musb = dev_to_musb(dev);
	static int first = 1;

	/*
	 * When pm_runtime_get_sync called for the first time in driver
	 * init,  some of the structure is still not initialized which is
	 * used in restore function. But clock needs to be
	 * enabled before any register access, so
	 * pm_runtime_get_sync has to be called.
	 * Also context restore without save does not make
	 * any sense
	 */
	if (!first)
		musb_restore_context(musb);
	first = 0;

	return 0;
}
#endif

static const struct dev_pm_ops musb_dev_pm_ops = {
	.suspend_noirq		= musb_suspend_noirq,
	.resume_noirq	= musb_resume_noirq,
#if 0
	.runtime_suspend = musb_runtime_suspend,
	.runtime_resume = musb_runtime_resume,
#endif
};

#define MUSB_DEV_PM_OPS (&musb_dev_pm_ops)
#else
#define	MUSB_DEV_PM_OPS	NULL
#endif

static struct platform_driver musb_driver = {
	.driver = {
		.name		= (char *)musb_driver_name,
		.bus		= &platform_bus_type,
		.owner		= THIS_MODULE,
		.pm		= MUSB_DEV_PM_OPS,
	},
	.probe		= musb_probe,
	.remove		= musb_remove,
	.shutdown	= musb_shutdown,
};

/*-------------------------------------------------------------------------*/

static int __init musb_init(void)
{
	if (usb_disabled())
		return 0;

	return platform_driver_register(&musb_driver);
}
module_init(musb_init);

static void __exit musb_cleanup(void)
{
	platform_driver_unregister(&musb_driver);
}
module_exit(musb_cleanup);
#ifdef MUSB_QMU_SUPPORT
module_param(mtk_qmu_dbg_level, int, 0644);
#ifdef QMU_TASKLET
module_param(qmu_tasklet, int, 0644);
#endif
#endif
