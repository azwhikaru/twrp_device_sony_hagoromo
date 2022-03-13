/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * MUSB OTG controller driver for Blackfin Processors
 *
 * Copyright 2006-2008 Analog Devices Inc.
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
#include <linux/usb/nop-usb-xceiv.h>
#endif
#include <linux/xlog.h>
#include <linux/switch.h>
#include <linux/i2c.h>

#include <mach/irqs.h>
#include <mach/eint.h>
#include <linux/musb11/musb11_core.h>
#include <linux/musb11/mtk11_musb.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/musb11/musb11_hsdma.h>
#include <cust_gpio_usage.h>
#include <mach/upmu_common.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_clkmgr.h>
#include <mach/emi_mpu.h>
#include "usb11.h"

#ifdef MUSB11_QMU_SUPPORT
#include <linux/musb11/musb11_qmu.h>
#endif
extern struct musb *mtk_musb11;
static DEFINE_SEMAPHORE(usb11_power_clock_lock);
//static bool platform_init_first = true;   //Mark by ALPS01262215
extern bool mtk_usb11_power;
#ifdef WITH_USB11_ENTER_SUSPEND
bool usb11_connect = false;
#endif
static u32 cable_mode = CABLE_MODE_NORMAL;
#ifdef MTK_UART_USB_SWITCH
u32 port_mode = PORT_MODE_USB;
u32 sw_tx = 0;
u32 sw_rx = 0;
u32 sw_uart_path = 0;
#endif

#if defined(CONFIG_USB11_MTK_OTG) && defined(CONFIG_USB11_MTK_HDRC_GADGET)
#define GPIO_OTG_INT (GPIO_OTG_IDDIG1_EINT_PIN  & ~(0x80000000))
#endif


#if defined(MTK_USB11_VBUS_DETECT_SUPPORT) && defined(CONFIG_USB11_MTK_HDRC_GADGET)
#define VBUS_DETECT_PIN_GPIO (GPIO_VBUS1_DETECT_PIN & ~(0x80000000))
#define VBUS_DETECT_PIN_EINT CUST_EINT_USB_VBUS1_NUM
#endif

/*EP Fifo Config*/
static struct musb_fifo_cfg __initdata usb11_fifo_cfg[] = {
{ .hw_ep_num =	1, .style = MUSB_FIFO_TX,	.maxpacket = 512, .ep_mode = EP_ISO,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =	1, .style = MUSB_FIFO_RX,	.maxpacket = 1024, .ep_mode = EP_ISO,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  5, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_INT,.mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	5, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_INT,.mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  6, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_INT, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	6, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_INT,.mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_ISO,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_ISO,.mode = MUSB_BUF_DOUBLE},
};

static struct timer_list musb11_idle_timer;

static void musb_do_idle(unsigned long _musb)
{
	struct musb	*musb = (void *)_musb;
	unsigned long	flags;
	u8	devctl;

	if (musb->is_active) {
		DBG(0, "%s active, igonre do_idle\n",
			otg_state_string(musb->xceiv->state));
		return;
	}

	spin_lock_irqsave(&musb->lock, flags);

	switch (musb->xceiv->state) {
	case OTG_STATE_B_PERIPHERAL:
	case OTG_STATE_A_WAIT_BCON:
		devctl = musb11_readb(musb->mregs, MUSB11_DEVCTL);
		if (devctl & MUSB11_DEVCTL_BDEVICE) {
			musb->xceiv->state = OTG_STATE_B_IDLE;
			MUSB_DEV_MODE(musb);
		} else {
			musb->xceiv->state = OTG_STATE_A_IDLE;
			MUSB_HST_MODE(musb);
		}
		break;
	case OTG_STATE_A_HOST:
		devctl = musb11_readb(musb->mregs, MUSB11_DEVCTL);
		if (devctl &  MUSB11_DEVCTL_BDEVICE)
			musb->xceiv->state = OTG_STATE_B_IDLE;
		else
			musb->xceiv->state = OTG_STATE_A_WAIT_BCON;
	default:
		break;
	}
	spin_unlock_irqrestore(&musb->lock, flags);

    DBG(0, "usb11_otg_state %s \n", otg_state_string(musb->xceiv->state));
}

static void mt_usb11_try_idle(struct musb *musb, unsigned long timeout)
{
	unsigned long		default_timeout = jiffies + msecs_to_jiffies(3);
	static unsigned long	last_timer;

	if (timeout == 0)
		timeout = default_timeout;

	/* Never idle if active, or when VBUS timeout is not set as host */
	if (musb->is_active || ((musb->a_wait_bcon == 0)
			&& (musb->xceiv->state == OTG_STATE_A_WAIT_BCON))) {
		DBG(2, "%s active, deleting timer\n",
			otg_state_string(musb->xceiv->state));
		del_timer(&musb11_idle_timer);
		last_timer = jiffies;
		return;
	}

	if (time_after(last_timer, timeout)) {
		if (!timer_pending(&musb11_idle_timer))
			last_timer = timeout;
		else {
			DBG(2, "Longer idle timer already pending, ignoring\n");
			return;
		}
	}
	last_timer = timeout;

	DBG(2, "%s inactive, for idle timer for %lu ms\n",
		otg_state_string(musb->xceiv->state),
		(unsigned long)jiffies_to_msecs(timeout - jiffies));
	mod_timer(&musb11_idle_timer, timeout);
}

void mt_usb11_enable(struct musb *musb)
{
    unsigned long   flags;

    DBG(0, "%d, %d\n", mtk_usb11_power, musb->power);

    if (musb->power == true)
        return;

    flags = musb11_readl(mtk_musb11->mregs,MUSB11_L1INTM);

    // mask ID pin, so "open clock" and "set flag" won't be interrupted. ISR may call clock_disable.
    musb11_writel(mtk_musb11->mregs,MUSB11_L1INTM,(~MUSB11_IDDIG_INT_STATUS)&flags);

    /* Mark by ALPS01262215
    if (platform_init_first) {
        DBG(0,"usb init first\n\r");
        musb->is_host = true;
    } */

    if (!mtk_usb11_power) {
        if (down_interruptible(&usb11_power_clock_lock))
            xlog_printk(ANDROID_LOG_ERROR, "USB20", "%s: busy, Couldn't get usb11_power_clock_lock\n" \
                        , __func__);

#ifndef FPGA_PLATFORM
        enable_pll(UNIVPLL, "USB_PLL");
        DBG(0,"enable UPLL before connect\n");
#endif
        mdelay(10);
        usb11_phy_recover();
        mtk_usb11_power = true;

        up(&usb11_power_clock_lock);
    }
	musb->power = true;

    musb11_writel(mtk_musb11->mregs,MUSB11_L1INTM,flags);
}

void mt_usb11_disable(struct musb *musb)
{
	printk("mt_usb11_disable %s, %d, %d\n", __func__, mtk_usb11_power, musb->power);
    if (musb->power == false)
        return;

    /* Mark by ALPS01262215
    if (platform_init_first) {
        DBG(0,"usb init first\n\r");
        musb->is_host = false;
        platform_init_first = false;
    } */

    if (mtk_usb11_power) {
        if (down_interruptible(&usb11_power_clock_lock))
            xlog_printk(ANDROID_LOG_ERROR, "USB20", "%s: busy, Couldn't get usb11_power_clock_lock\n" \
                        , __func__);

        mtk_usb11_power = false;

        usb11_phy_savecurrent();

#ifndef FPGA_PLATFORM
        //disable_pll(UNIVPLL,"USB_PLL");
        DBG(0, "disable UPLL before disconnect\n");
#endif

        up(&usb11_power_clock_lock);
    }

    musb->power = false;
}

/* ================================ */
/* connect and disconnect functions */
/* ================================ */
#if 0 
bool mt_usb_is_device(void)
{
#if 0
	DBG(4,"called\n");
	if(!mtk_musb11){
		DBG(0,"mtk_musb is NULL\n");
		return false; // don't do charger detection when usb is not ready
	} else {
		DBG(4,"is_host=%d\n",mtk_musb11->is_host);
	}
  return !mtk_musb11->is_host;
#endif
	return true;
}
#endif
void mt_usb11_connect(void)
{
//#ifndef USB11_SW_WITCH_MODE
#if 1
#ifdef WITH_USB11_ENTER_SUSPEND
	usb11_connect = true;
#endif
	printk("[MUSB] USB is ready for connect\n");
    DBG(3, "is ready %d is_host %d power %d\n",mtk_musb11->is_ready,mtk_musb11->is_host , mtk_musb11->power);
    if (!mtk_musb11 || !mtk_musb11->is_ready || mtk_musb11->is_host || mtk_musb11->power)
        return;

    DBG(0,"cable_mode=%d\n",cable_mode);

	if(cable_mode != CABLE_MODE_NORMAL)
	{
        DBG(0,"musb11_sync_with_bat, USB_CONFIGURED\n");
        musb11_sync_with_bat(mtk_musb11,USB_CONFIGURED);
        mtk_musb11->power = true;
        return;
    }

#ifdef CONFIG_USB_CAN_SLEEP_CONNECTING
	if (usb_gadget_is_enabled()) {
		if (!wake_lock_active(&mtk_musb11->usb_lock))
			wake_lock(&mtk_musb11->usb_lock);
	}
#else
#ifndef WITH_USB11_ENTER_SUSPEND
	if (!wake_lock_active(&mtk_musb11->usb_lock))
		wake_lock(&mtk_musb11->usb_lock);
#endif
#endif

    musb11_start(mtk_musb11);
	printk("[MUSB] USB connect\n");
#else
    DBG(0,"connected!!!\n");
#endif
}

void mt_usb11_disconnect(void)
{
//#ifndef USB11_SW_WITCH_MODE
#if 1
#ifdef WITH_USB11_ENTER_SUSPEND
	usb11_connect = false;
#endif
	printk("[MUSB] USB is ready for disconnect\n");

	if (!mtk_musb11 || !mtk_musb11->is_ready || mtk_musb11->is_host || !mtk_musb11->power)
		return;

	musb11_stop(mtk_musb11);

	if (wake_lock_active(&mtk_musb11->usb_lock))
		wake_unlock(&mtk_musb11->usb_lock);

    DBG(0,"cable_mode=%d\n",cable_mode);

	if (cable_mode != CABLE_MODE_NORMAL) {
        DBG(0,"musb11_sync_with_bat, USB_SUSPEND\n");
		musb11_sync_with_bat(mtk_musb11,USB_SUSPEND);
		mtk_musb11->power = false;
	}

	printk("[MUSB] USB disconnect\n");
#else
    DBG(0,"disconnected!!!\n");
#endif
}

#if (defined(CONFIG_USB11_MTK_HDRC_GADGET))
bool usb_cable_connected(void)
{
#ifdef MTK_USB11_VBUS_DETECT_SUPPORT
	int i4_vbus_state = 0;
#endif

#ifdef CONFIG_USB11_MTK_OTG
	int iddig_state = 1;
	
	iddig_state = mt_get_gpio_in(GPIO_OTG_INT);
		DBG(0,"iddig_state = %d\n", iddig_state);
	
		if(!iddig_state)
			return false;
#endif

#ifdef MTK_USB11_VBUS_DETECT_SUPPORT
	i4_vbus_state = mt_get_gpio_in(VBUS_DETECT_PIN_GPIO);
	DBG(0,"i4_vbus_state = %d\n", i4_vbus_state);
	   
	if(1 == i4_vbus_state)
	{
		return true;
	}
	else
	{
	   return false;
	}
#endif
	return false;
}
#endif


#ifdef USB11_SW_WITCH_MODE
void mt_usb11_sw_connect(bool connect)
{
    DBG(0,"[MUSB] USB is connect=%d\n", connect);
    DBG(3, "is ready %d is_host %d power %d\n",mtk_musb11->is_ready,mtk_musb11->is_host , mtk_musb11->power);

#if (defined(CONFIG_USB11_MTK_HDRC_GADGET))
	if ((connect) && (usb_cable_connected())){
#else
	if (connect) {
#endif
        if(cable_mode != CABLE_MODE_NORMAL)
	    {
            DBG(0,"musb_sync_with_bat, USB_CONFIGURED\n");
            musb11_sync_with_bat(mtk_musb11,USB_CONFIGURED);
            mtk_musb11->power = true;
            return;
        }

		#ifdef ID_PIN_USE_EX_EINT
		musb11_do_infra_misc(false);
		#endif

		#ifndef WITH_USB_ENTER_SUSPEND
	    if (!wake_lock_active(&mtk_musb11->usb_lock))
		    wake_lock(&mtk_musb11->usb_lock);
        #endif
        musb11_start(mtk_musb11);
        DBG(0,"[MUSB] USB connect\n");
    } else {
        musb11_stop(mtk_musb11);
        if (wake_lock_active(&mtk_musb11->usb_lock))
            wake_unlock(&mtk_musb11->usb_lock);
		if (cable_mode != CABLE_MODE_NORMAL) {
            DBG(0,"musb_sync_with_bat, USB_SUSPEND\n");
		    musb11_sync_with_bat(mtk_musb11,USB_SUSPEND);
		    mtk_musb11->power = false;
	    }

		#ifdef ID_PIN_USE_EX_EINT
		musb11_do_infra_misc(true);
		#endif

	    DBG(0,"[MUSB] USB disconnect\n");
    }
}
#endif

void musb11_platform_reset(struct musb *musb)
{
	u16 swrst = 0;
	void __iomem	*mbase = musb->mregs;
	swrst = musb11_readw(mbase,MUSB11_SWRST);
	swrst |= (MUSB11_SWRST_DISUSBRESET | MUSB11_SWRST_SWRST);
	musb11_writew(mbase, MUSB11_SWRST,swrst);
}

#if 0
static void usb_check_connect(void)
{
#ifndef FPGA_PLATFORM
	if (usb_cable_connected())
        mt_usb_connect();
#endif
}
#endif

bool usb11_is_switch_charger(void)
{
#if 0
#ifdef SWITCH_CHARGER
	return true;
#else
	return false;
#endif
#endif
return false;

}

#if 0
void pmic_chrdet_int_en(int is_on)
{
#ifndef FPGA_PLATFORM
	upmu_interrupt_chrdet_int_en(is_on);
#endif
}
#endif

void musb11_sync_with_bat(struct musb *musb,int usb_state)
{
#ifdef CONFIG_MTK_USB11_SMART_BATTERY
#ifndef FPGA_PLATFORM
    DBG(0,"BATTERY_SetUSBState, state=%d\n",usb_state);
	BATTERY_SetUSBState(usb_state);
	wake_up_bat();
#endif
#endif
}

/*-------------------------------------------------------------------------*/
static irqreturn_t generic_interrupt_usb11(int irq, void *__hci)
{
	unsigned long	flags;
	irqreturn_t	retval = IRQ_NONE;
	struct musb	*musb = __hci;
	
	spin_lock_irqsave(&musb->lock, flags);

	
	/* musb_read_clear_generic_interrupt */
	musb->int_usb = musb11_readb(musb->mregs, MUSB11_INTRUSB) & musb11_readb(musb->mregs, MUSB11_INTRUSBE);
	musb->int_tx = musb11_readw(musb->mregs, MUSB11_INTRTX) & musb11_readw(musb->mregs, MUSB11_INTRTXE);
	musb->int_rx = musb11_readw(musb->mregs, MUSB11_INTRRX) & musb11_readw(musb->mregs, MUSB11_INTRRXE);
#ifdef MUSB11_QMU_SUPPORT
	musb->int_queue = musb11_readl(musb->mregs, MUSB11_QISAR);
#endif
	mb();
	musb11_writew(musb->mregs,MUSB11_INTRRX,musb->int_rx);
	musb11_writew(musb->mregs,MUSB11_INTRTX,musb->int_tx);
	musb11_writeb(musb->mregs,MUSB11_INTRUSB,musb->int_usb);
#ifdef MUSB11_QMU_SUPPORT
	if (musb->int_queue){
		musb11_writel(musb->mregs, MUSB11_QISAR, musb->int_queue);
		musb->int_queue &= ~(musb11_readl(musb->mregs, MUSB11_QIMR));
	}
#endif
	/* musb_read_clear_generic_interrupt */

#ifdef MUSB11_QMU_SUPPORT
	if (musb->int_usb || musb->int_tx || musb->int_rx || musb->int_queue)
		retval = musb11_interrupt(musb);
#else
	if (musb->int_usb || musb->int_tx || musb->int_rx)
		retval = musb11_interrupt(musb);
#endif

	spin_unlock_irqrestore(&musb->lock, flags);

	return retval;
}

static irqreturn_t mt_usb11_interrupt(int irq, void *dev_id)
{
    irqreturn_t tmp_status;
    irqreturn_t status = IRQ_NONE;
	struct musb	*musb = (struct musb*)dev_id;
	u32 usb_l1_ints;

	usb_l1_ints= musb11_readl(musb->mregs,MUSB11_L1INTS)&musb11_readl(mtk_musb11->mregs,MUSB11_L1INTM); //gang  REVISIT
	DBG(1,"usb interrupt assert %x %x  %x %x %x\n",usb_l1_ints,musb11_readl(mtk_musb11->mregs,MUSB11_L1INTM),musb11_readb(musb->mregs, MUSB11_INTRUSBE),musb11_readw(musb->mregs,MUSB11_INTRTX),musb11_readw(musb->mregs,MUSB11_INTRTXE));

	if ((usb_l1_ints & MUSB11_TX_INT_STATUS) || (usb_l1_ints & MUSB11_TRX_INT_STATUS) || (usb_l1_ints & MUSB11_COM_INT_STATUS)
#ifdef MUSB11_QMU_SUPPORT
			||(usb_l1_ints & MUSB11_QINT_STATUS)
#endif
	   ) {
		if((tmp_status = generic_interrupt_usb11(irq, musb)) != IRQ_NONE)
			status = tmp_status;
	}

	if (usb_l1_ints & MUSB11_DMA_INT_STATUS) {
		if((tmp_status = usb11_dma_controller_irq(irq, musb->dma_controller)) != IRQ_NONE)
			status = tmp_status;
	}
#ifdef 	CONFIG_USB11_MTK_OTG
	if(usb_l1_ints & MUSB11_IDDIG_INT_STATUS) {
		mt_usb11_iddig_int(musb);
		status = IRQ_HANDLED;
	}
#endif
    return status;

}

#if 0
/*--FOR INSTANT POWER ON USAGE--------------------------------------------------*/
static ssize_t mt_usb11_show_cmode(struct device* dev, struct device_attribute *attr, char *buf)
{
	if (!dev) {
		DBG(0,"dev is null!!\n");
		return 0;
	}
	return scnprintf(buf, PAGE_SIZE, "%d\n", cable_mode);
}

static ssize_t mt_usb11_store_cmode(struct device* dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int cmode;

	if (!dev) {
		DBG(0,"dev is null!!\n");
		return count;
	} else if (1 == sscanf(buf, "%d", &cmode)) {
		DBG(0, "cmode=%d, cable_mode=%d\n", cmode, cable_mode);
		if (cmode >= CABLE_MODE_MAX)
			cmode = CABLE_MODE_NORMAL;

		if (cable_mode != cmode) {
			if(mtk_musb11) {
				if(down_interruptible(&mtk_musb11->musb_lock))
					xlog_printk(ANDROID_LOG_ERROR, "USB20", "%s: busy, Couldn't get musb_lock\n", __func__);
			}
			if(cmode == CABLE_MODE_CHRG_ONLY) { // IPO shutdown, disable USB
				if(mtk_musb11) {
					mtk_musb11->in_ipo_off = true;
				}

			} else { // IPO bootup, enable USB
				if(mtk_musb11) {
					mtk_musb11->in_ipo_off = false;
				}
			}

			mt_usb11_disconnect();
			cable_mode = cmode;
			msleep(10);
			/* check that "if USB cable connected and than call mt_usb11_connect" */
			/* Then, the Bat_Thread won't be always wakeup while no USB/chatger cable and IPO mode */
			usb11_check_connect();

#ifdef CONFIG_USB11_MTK_OTG
			if(cmode == CABLE_MODE_CHRG_ONLY) {
				if(mtk_musb11 && mtk_musb11->is_host) { // shut down USB host for IPO
					if (wake_lock_active(&mtk_musb11->usb_lock))
                        wake_unlock(&mtk_musb11->usb_lock);
					musb_platform_set_vbus(mtk_musb11, 0);
					musb11_stop(mtk_musb11);
					MUSB_DEV_MODE(mtk_musb11);
					/* Think about IPO shutdown with A-cable, then switch to B-cable and IPO bootup. We need a point to clear session bit */
					musb11_writeb(mtk_musb11->mregs, MUSB11_DEVCTL, (~MUSB11_DEVCTL_SESSION)&musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL));
				}
				usb11_switch_int_to_host_and_mask(mtk_musb11); // mask ID pin interrupt even if A-cable is not plugged in
			} else {
				usb11_switch_int_to_host(mtk_musb11); // resotre ID pin interrupt
			}
#endif
			if(mtk_musb11) {
				up(&mtk_musb11->musb_lock);
			}
		}
	}
	return count;
}

DEVICE_ATTR(cmode,  0664, mt_usb11_show_cmode, mt_usb11_store_cmode);
#endif

#if 0
#ifdef MTK_UART_USB_SWITCH
static void uart_usb11_switch_dump_register(void)
{
usb11_enable_clock(true);
#ifdef FPGA_PLATFORM
	printk("[MUSB]addr: 0x6B, value: %x\n", USB11_PHY_Read_Register8(0x6B));
	printk("[MUSB]addr: 0x6E, value: %x\n", USB11_PHY_Read_Register8(0x6E));
	printk("[MUSB]addr: 0x22, value: %x\n", USB11_PHY_Read_Register8(0x22));
	printk("[MUSB]addr: 0x68, value: %x\n", USB11_PHY_Read_Register8(0x68));
	printk("[MUSB]addr: 0x6A, value: %x\n", USB11_PHY_Read_Register8(0x6A));
	printk("[MUSB]addr: 0x1A, value: %x\n", USB11_PHY_Read_Register8(0x1A));
#else
	printk("[MUSB]addr: 0x6B, value: %x\n", USB11PHY_READ8(0x6B));
	printk("[MUSB]addr: 0x6E, value: %x\n", USB11PHY_READ8(0x6E));
	printk("[MUSB]addr: 0x22, value: %x\n", USB11PHY_READ8(0x22));
	printk("[MUSB]addr: 0x68, value: %x\n", USB11PHY_READ8(0x68));
	printk("[MUSB]addr: 0x6A, value: %x\n", USB11PHY_READ8(0x6A));
	printk("[MUSB]addr: 0x1A, value: %x\n", USB11PHY_READ8(0x1A));
#endif
usb11_enable_clock(false);
	printk("[MUSB]addr: 0x11002090 (UART1), value: %x\n\n", DRV_Reg8(UART1_BASE + 0x90));
}

static ssize_t mt_usb11_show_portmode(struct device* dev, struct device_attribute *attr, char *buf)
{
	if (!dev) {
		printk("dev is null!!\n");
		return 0;
	}

	if (usb11_phy_check_in_uart_mode())
		port_mode = PORT_MODE_UART;
	else
		port_mode = PORT_MODE_USB;

	if (port_mode == PORT_MODE_USB) {
		printk("\nUSB Port mode -> USB\n");
	} else if (port_mode == PORT_MODE_UART) {
		printk("\nUSB Port mode -> UART\n");
	}
	uart_usb11_switch_dump_register();

	return scnprintf(buf, PAGE_SIZE, "%d\n", port_mode);
}

static ssize_t mt_usb11_store_portmode(struct device* dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int portmode;

	if (!dev) {
		printk("dev is null!!\n");
		return count;
	} else if (1 == sscanf(buf, "%d", &portmode)) {
		printk("\nUSB Port mode: current => %d (port_mode), change to => %d (portmode)\n", port_mode, portmode);
		if (portmode >= PORT_MODE_MAX)
			portmode = PORT_MODE_USB;

		if (port_mode != portmode) {
			if(portmode == PORT_MODE_USB) { // Changing to USB Mode
				printk("USB Port mode -> USB\n");
				usb11_phy_switch_to_usb();
			} else if(portmode == PORT_MODE_UART) { // Changing to UART Mode
				printk("USB Port mode -> UART\n");
				usb11_phy_switch_to_uart();
			}
			uart_usb11_switch_dump_register();
			port_mode = portmode;
		}
	}
	return count;
}

DEVICE_ATTR(portmode,  0664, mt_usb11_show_portmode, mt_usb11_store_portmode);


static ssize_t mt_usb11_show_tx(struct device* dev, struct device_attribute *attr, char *buf)
{
	UINT8 var;
	UINT8 var2;

	if (!dev) {
		printk("dev is null!!\n");
		return 0;
	}

#ifdef FPGA_PLATFORM
	var = USB11_PHY_Read_Register8(0x6E);
#else
	var = USB11PHY_READ8(0x6E);
#endif
	var2 = (var >> 3) & ~0xFE;
	printk("[MUSB]addr: 0x6E (TX), value: %x - %x\n", var, var2);

	sw_tx = var;

	return scnprintf(buf, PAGE_SIZE, "%x\n", var2);
}

static ssize_t mt_usb11_store_tx(struct device* dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int val;
	UINT8 var;
	UINT8 var2;

	if (!dev) {
		printk("dev is null!!\n");
		return count;
	} else if (1 == sscanf(buf, "%d", &val)) {
		printk("\n Write TX : %d\n", val);

#ifdef FPGA_PLATFORM
		var = USB11_PHY_Read_Register8(0x6E);
#else
		var = USB11PHY_READ8(0x6E);
#endif

		if (val == 0) {
			var2 = var & ~(1 << 3);
		} else {
			var2 = var | (1 << 3);
		}

#ifdef FPGA_PLATFORM
		USB11_PHY_Write_Register8(var2, 0x6E);
		var = USB11_PHY_Read_Register8(0x6E);
#else
		USB11PHY_WRITE8(0x6E, var2);
		var = USB11PHY_READ8(0x6E);
#endif

		var2 = (var >> 3) & ~0xFE;

		printk("[MUSB]addr: 0x6E TX [AFTER WRITE], value after: %x - %x\n", var, var2);
		sw_tx = var;
	}
	return count;
}
DEVICE_ATTR(tx,  0664, mt_usb11_show_tx, mt_usb11_store_tx);

static ssize_t mt_usb11_show_rx(struct device* dev, struct device_attribute *attr, char *buf)
{
	UINT8 var;
	UINT8 var2;

	if (!dev) {
		printk("dev is null!!\n");
		return 0;
	}

#ifdef FPGA_PLATFORM
	var = USB11_PHY_Read_Register8(0x77);
#else
	var = USB11PHY_READ8(0x77);
#endif
	var2 = (var >> 7) & ~0xFE;
	printk("[MUSB]addr: 0x77 (RX), value: %x - %x\n", var, var2);
	sw_rx = var;

	return scnprintf(buf, PAGE_SIZE, "%x\n", var2);
}

DEVICE_ATTR(rx,  0664, mt_usb11_show_rx, NULL);

static ssize_t mt_usb11_show_uart_path(struct device* dev, struct device_attribute *attr, char *buf)
{
	UINT8 var;

	if (!dev) {
		printk("dev is null!!\n");
		return 0;
	}

	var = DRV_Reg8(UART1_BASE + 0x90);
	printk("[MUSB]addr: 0x11002090 (UART1), value: %x\n\n", DRV_Reg8(UART1_BASE + 0x90));
	sw_uart_path = var;

	return scnprintf(buf, PAGE_SIZE, "%x\n", var);
}

DEVICE_ATTR(uartpath,  0664, mt_usb11_show_uart_path, NULL);
#endif
#endif
#ifdef CONFIG_USB_CAN_SLEEP_CONNECTING
static ssize_t mt_usb11_show_usb11_wakelock(struct device* dev, struct device_attribute *attr, char *buf)
{
	UINT8 var;

	if (!dev) {
		printk("dev is null!!\n");
		return 0;
	}

	var = wake_lock_active(&mtk_musb11->usb_lock);

	return scnprintf(buf, PAGE_SIZE, "%x\n", var);
}

DEVICE_ATTR(usb11_wakelock,  0444, mt_usb11_show_usb11_wakelock, NULL);
#endif

#ifdef USB11_SW_WITCH_MODE
bool dev1_mode = false;
bool hos1_mode = false;
extern void musb11_id_pin_sw_work(bool host_mode);

static ssize_t mt_usb11_show_mode(struct device* dev, struct device_attribute *attr, char *buf)
{
    return 0;
}

static ssize_t mt_usb11_store_mode(struct device* dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
    unsigned int usb_mode;
    int ret = 0;

    if (!dev) {
        DBG(0,"dev is null!!\n");
        return count;
    }
    if (buf != NULL && count != 0) {
	    ret = sscanf(buf, "%d", &usb_mode);
	    DBG(0, "usb_mode=%d\n", usb_mode);
		if (ret != 1)
			return -1;
    }

	if (mtk_musb11->xceiv->state == OTG_STATE_B_PERIPHERAL)
	{
		dev1_mode = true;
		hos1_mode = false;
	}
	else if (mtk_musb11->xceiv->state == OTG_STATE_A_HOST)
	{
		dev1_mode = false;
		hos1_mode = true;
	}

	switch (usb_mode) {
		case 1:
			if (!dev1_mode && !hos1_mode) {
				 DBG(0, "switch to device mode\n");
				 mt_usb11_sw_connect(true);
				 dev1_mode = true;
				 hos1_mode = false;
            }
			break;
        case 2:
			if (!dev1_mode && !hos1_mode) {
                 DBG(0, "switch to host mode\n");
                 musb11_id_pin_sw_work(true);
				 dev1_mode = false;
				 hos1_mode = true;
            }
			break;
        case 0:
            if (dev1_mode){
                 DBG(0, "switch to dev->idle mode\n");
				 mt_usb11_sw_connect(false);
				 dev1_mode = false;
                 hos1_mode = false;
            }
			else if (hos1_mode){
                 DBG(0, "switch to host->idle mode\n");
                 musb11_id_pin_sw_work(false);
				 dev1_mode = false;
                 hos1_mode = false;
            }
           else
                DBG(0, "do nothing\n");
            break;
        default:
            break;
    }
	
	return count;
}

static DEVICE_ATTR(mode,  0664, mt_usb11_show_mode, mt_usb11_store_mode);
#endif

#ifdef FPGA_PLATFORM
static struct i2c_client *usb11_i2c_client = NULL;
static const struct i2c_device_id usb11_i2c_id[] = {{"mtk-usb11",0},{}};

static struct i2c_board_info __initdata usb11_i2c_dev = { I2C_BOARD_INFO("mtk-usb11", 0x60)};


void USB11_PHY_Write_Register8(UINT8 var,  UINT8 addr)
{
	char buffer[2];
	buffer[0] = addr;
	buffer[1] = var;
	i2c_master_send(usb11_i2c_client, buffer, 2);
}

UINT8 USB11_PHY_Read_Register8(UINT8 addr)
{
	UINT8 var;
	i2c_master_send(usb11_i2c_client, &addr, 1);
	i2c_master_recv(usb11_i2c_client, &var, 1);
	return var;
}

static int usb11_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	printk("[MUSB]usb11_i2c_probe, start\n");

	usb11_i2c_client = client;

	//disable usb mac suspend
	DRV_WriteReg8(USB_SIF_BASE + 0x86a, 0x00);

	USB11_PHY_Write_Register8(0x00, 0xFF);
	USB11_PHY_Write_Register8(0x55, 0x05);
	USB11_PHY_Write_Register8(0x84, 0x18);
	USB11_PHY_Write_Register8(0x10, 0xFF);
	USB11_PHY_Write_Register8(0x84, 0x0A);
	USB11_PHY_Write_Register8(0x40, 0xFF);
	USB11_PHY_Write_Register8(0x46, 0x38);
	USB11_PHY_Write_Register8(0x40, 0x42);
	USB11_PHY_Write_Register8(0xAB, 0x08);
	USB11_PHY_Write_Register8(0x0C, 0x09);
	USB11_PHY_Write_Register8(0x71, 0x0C);
	USB11_PHY_Write_Register8(0x4F, 0x0E);
	USB11_PHY_Write_Register8(0xE1, 0x10);
	USB11_PHY_Write_Register8(0x5F, 0x14);
	USB11_PHY_Write_Register8(0x60, 0xFF);
	USB11_PHY_Write_Register8(0x03, 0x14);
	USB11_PHY_Write_Register8(0x00, 0xFF);
	USB11_PHY_Write_Register8(0x04, 0x6A);
	USB11_PHY_Write_Register8(0x08, 0x68);


	printk("[MUSB]usb11_i2c_probe, end\n");
    return 0;

}

/*static int usb_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {
    strcpy(info->type, "mtk-usb");
    return 0;
}*/

static int usb11_i2c_remove(struct i2c_client *client) {return 0;}


struct i2c_driver usb_i2c_driver = {
    .probe = usb11_i2c_probe,
    .remove = usb11_i2c_remove,
    /*.detect = usb_i2c_detect,*/
    .driver = {
    	.name = "mtk-usb11",
    },
    .id_table = usb11_i2c_id,
};

static int add_usb11_i2c_driver(void)
{
	i2c_register_board_info(2, &usb11_i2c_dev, 1);

	if (i2c_add_driver(&usb_i2c_driver)!=0) {
		printk("[MUSB]usb_i2c_driver initialization failed!!\n");
		return -1;
	} else {
		printk("[MUSB]usb_i2c_driver initialization succeed!!\n");
	}
	return 0;
}
#endif //End of FPGA_PLATFORM

#ifdef MTK_USB11_VBUS_DETECT_SUPPORT
static void switch_vbus_int_to_idle(void)
{
    mt_eint_set_polarity(VBUS_DETECT_PIN_EINT, MT_EINT_POL_NEG);
	mt_eint_unmask(VBUS_DETECT_PIN_EINT);

	DBG(0,"vbus switch interrupt for idle detect is done\n");
}

static void switch_vbus_int_to_device(void)
{
    mt_eint_set_polarity(VBUS_DETECT_PIN_EINT, MT_EINT_POL_POS);
	mt_eint_unmask(VBUS_DETECT_PIN_EINT);

	DBG(0,"vbus switch interrupt for device detect is done\n");
}

static void musb_vbus_pin_detect_work(void)
{
    int i4_vbus_state = 0;
    
    i4_vbus_state = mt_get_gpio_in(VBUS_DETECT_PIN_GPIO);
	DBG(0,"i4_vbus_state = %d\n", i4_vbus_state);
    
    if(1 == i4_vbus_state)
    {
#ifdef CONFIG_USB11_MTK_OTG
//        mdelay(2000);
        printk("Open infra misc....\n");
        musb11_do_infra_misc(false);
#endif
        mt_usb11_connect();
        switch_vbus_int_to_idle();
    }
    else
    {
    	printk("Close infra misc....\n");
#ifdef CONFIG_USB11_MTK_OTG
        musb11_do_infra_misc(true);
#endif
        mt_usb11_disconnect();
        switch_vbus_int_to_device();
    }
}
#endif
static int mt_usb11_init(struct musb *musb)
{
	DBG(0,"mt_usb11_init \n");

    mtk_usb_nop_xceiv_register(1);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	musb->xceiv = mtk_usb_get_phy(USB_PHY_TYPE_USB2,1);
	printk("mt_usb11_init xceiv = 0x%p\n",musb->xceiv);
#else
    musb->xceiv = usb_get_transceiver();
    
#endif
    musb->nIrq = MT6582_USB1_IRQ_ID;
    musb->dma_irq= (int)SHARE_IRQ;
    musb->fifo_cfg = usb11_fifo_cfg;
    musb->fifo_cfg_size = ARRAY_SIZE(usb11_fifo_cfg);
    musb->dyn_fifo = true;
    musb->power = false;
    musb->is_host = false;
    musb->fifo_size = 8*1024;

    wake_lock_init(&musb->usb_lock, WAKE_LOCK_SUSPEND, "USB11 suspend lock");

#ifndef FPGA_PLATFORM
    //hwPowerOn(MT6323_POWER_LDO_VUSB, VOL_3300, "VUSB_LDO");
    hwPowerOn(MT65XX_POWER_LDO_VUSB, VOL_3300, "VUSB_LDO"); // don't need to power on PHY for every resume

    printk("%s, enable VBUS_LDO \n", __func__);
#endif

    mt_usb11_enable(musb);

    musb->isr = mt_usb11_interrupt;
    musb11_writel(musb->mregs,MUSB_HSDMA_INTR,0xff | (0xff << MUSB11_DMA_INTR_UNMASK_SET_OFFSET));
    DBG(0,"musb platform init %x\n",musb11_readl(musb->mregs,MUSB_HSDMA_INTR));
#ifdef MUSB11_QMU_SUPPORT
    musb11_writel(musb->mregs,MUSB11_L1INTM,MUSB11_TX_INT_STATUS | MUSB11_TRX_INT_STATUS | MUSB11_COM_INT_STATUS | MUSB11_DMA_INT_STATUS | MUSB11_QINT_STATUS);
#else	
    musb11_writel(musb->mregs,MUSB11_L1INTM,MUSB11_TX_INT_STATUS | MUSB11_TRX_INT_STATUS | MUSB11_COM_INT_STATUS | MUSB11_DMA_INT_STATUS);//gang
#endif

    setup_timer(&musb11_idle_timer, musb_do_idle, (unsigned long) musb);

#ifdef MTK_USB11_VBUS_DETECT_SUPPORT
    mt_set_gpio_mode(VBUS_DETECT_PIN_GPIO, GPIO_MODE_00);
    mt_set_gpio_dir(VBUS_DETECT_PIN_GPIO, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(VBUS_DETECT_PIN_GPIO, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(VBUS_DETECT_PIN_GPIO, GPIO_PULL_DOWN);
    mt_eint_set_sens(VBUS_DETECT_PIN_EINT, MT_LEVEL_SENSITIVE);
    mt_eint_set_hw_debounce(VBUS_DETECT_PIN_EINT,64);
    mt_eint_registration(VBUS_DETECT_PIN_EINT, EINTF_TRIGGER_HIGH, musb_vbus_pin_detect_work, true);
    DBG(0,"vbus_detect pin init done\n");
#endif

#ifdef CONFIG_USB11_MTK_OTG
	mt_usb11_otg_init(musb);
#endif

#if defined(USB11_SW_WITCH_MODE) && !defined(CONFIG_USB11_MTK_HDRC_GADGET)
	musb11_id_pin_sw_work(true);
	dev1_mode = false;
	hos1_mode = true;
	mtk_musb11->xceiv->state = OTG_STATE_A_HOST;
#endif

    return 0;
}

static int mt_usb11_exit(struct musb *musb)
{
    del_timer_sync(&musb11_idle_timer);
	return 0;
}

static const struct musb_platform_ops mt_usb11_ops = {
	.init		= mt_usb11_init,
	.exit		= mt_usb11_exit,
	/*.set_mode	= mt_usb_set_mode,*/
	.try_idle	= mt_usb11_try_idle,
	.enable		= mt_usb11_enable,
	.disable	= mt_usb11_disable,
	.set_vbus	= mt_usb11_set_vbus,
	.vbus_status = mt_usb11_get_vbus_status
};

static u64 mt_usb11_dmamask = DMA_BIT_MASK(32);

static int mt_usb11_probe(struct platform_device *pdev)
{
	struct musb_hdrc_platform_data	*pdata = pdev->dev.platform_data;
	struct platform_device		*musb;
	struct mt_usb_glue		*glue;
	int				ret = -ENOMEM;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
	int				musbid;
#endif
    glue = kzalloc(sizeof(*glue), GFP_KERNEL);
	if (!glue) {
		dev_err(&pdev->dev, "failed to allocate glue context\n");
		goto err0;
	}
#if 0
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
	/* get the musb id */
	musbid = musb11_get_id(&pdev->dev, GFP_KERNEL);
	if (musbid < 0) {
		dev_err(&pdev->dev, "failed to allocate musb id\n");
		ret = -ENOMEM;
		goto err1;
	}
#endif
#endif

#if 1
	musb = platform_device_alloc("musbfsh-hdrc", 1);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	musb = platform_device_alloc("musbfsh-hdrc", PLATFORM_DEVID_AUTO);
#else
	musb = platform_device_alloc("musbfsh-hdrc", musbid);
#endif
#endif

	if (!musb) {
		dev_err(&pdev->dev, "failed to allocate musb device\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
		goto err1;
#else
		goto err2;
#endif
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
	musb->id			= musbid;
#endif
	musb->dev.parent		= &pdev->dev;
	musb->dev.dma_mask		= &mt_usb11_dmamask;
	musb->dev.coherent_dma_mask	= mt_usb11_dmamask;

	glue->dev			= &pdev->dev;
	glue->musb			= musb;

	pdata->platform_ops		= &mt_usb11_ops;

	platform_set_drvdata(pdev, glue);

	ret = platform_device_add_resources(musb, pdev->resource,
			pdev->num_resources);
	if (ret) {
		dev_err(&pdev->dev, "failed to add resources\n");
		goto err3;
	}

	ret = platform_device_add_data(musb, pdata, sizeof(*pdata));
	if (ret) {
		dev_err(&pdev->dev, "failed to add platform_data\n");
		goto err3;
	}

	ret = platform_device_add(musb);

	if (ret) {
		dev_err(&pdev->dev, "failed to register musb device\n");
		goto err3;
	}

#ifdef USB11_SW_WITCH_MODE
	ret = device_create_file(&pdev->dev, &dev_attr_mode);
#endif
#if 0
    ret = device_create_file(&pdev->dev, &dev_attr_cmode);
#ifdef MTK_UART_USB_SWITCH
	ret = device_create_file(&pdev->dev,&dev_attr_portmode);
	ret = device_create_file(&pdev->dev,&dev_attr_tx);
	ret = device_create_file(&pdev->dev,&dev_attr_rx);
	ret = device_create_file(&pdev->dev,&dev_attr_uartpath);
#endif

    if (ret) {
		dev_err(&pdev->dev, "failed to create musb device\n");
		goto err3;
	}
#endif
#ifdef CONFIG_USB_CAN_SLEEP_CONNECTING
	ret = device_create_file(&pdev->dev,&dev_attr_usb11_wakelock);
#endif
	return 0;

err3:
	platform_device_put(musb);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
err2:
	//musb11_put_id(&pdev->dev, musbid);
#endif

err1:
	kfree(glue);

err0:
	return ret;
}

static int mt_usb11_remove(struct platform_device *pdev)
{
	struct mt_usb_glue		*glue = platform_get_drvdata(pdev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	platform_device_unregister(glue->musb);
#else
	//musb11_put_id(&pdev->dev, glue->musb->id);
	platform_device_del(glue->musb);
	platform_device_put(glue->musb);
#endif
	kfree(glue);

	return 0;
}

static struct platform_driver mt_usb_driver11 = {
	.remove		= mt_usb11_remove,
	.probe		= mt_usb11_probe,
	.driver		= {
		.name	= "mt11_usb",
	},
};

static int __init usb11_init(void)
{
	DBG(0,"usb11 init\n");

#ifdef FPGA_PLATFORM
    add_usb11_i2c_driver();
#endif

	return platform_driver_register(&mt_usb_driver11);
}
fs_initcall(usb11_init);

static void __exit usb11_exit(void)
{
	platform_driver_unregister(&mt_usb_driver11);
}
module_exit(usb11_exit)
