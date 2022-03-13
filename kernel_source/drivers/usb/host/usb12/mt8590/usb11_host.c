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
#include <linux/xlog.h>
#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>
#include <linux/musb11/musb11_core.h>
#include <linux/platform_device.h>
#include <linux/musb11/musb11_hsdma.h>
#include <cust_gpio_usage.h>
#include <linux/switch.h>
#include "usb11.h"
#include <mach/mt_spm_mtcmos.h>
#include <mach/icx_pm_helper.h>

#ifdef CONFIG_USB11_MTK_OTG
extern struct musb *mtk_musb11;

#define GPIO_VBUS_ON (GPIO_OTG_DRVVBUS1_PIN & ~(0x80000000))

#if defined(CONFIG_USB11_MTK_HDRC_GADGET)
	#if defined(MUSB11_ID_PIN_USE_EX_EINT)
		#define GPIO_OTG_INT (GPIO_OTG_IDDIG1_EINT_PIN  & ~(0x80000000))
		#define GPIO_OTG_INT_EINT IDDIG_EINT_PIN
	#else
		#ifdef P2701_PROJECT
			#define GPIO_OTG_INT GPIO44
		#else
			#define GPIO_OTG_INT GPIO238
		#endif
	#endif

	#if defined(MTK_USB11_VBUS_DETECT_SUPPORT)
		#define VBUS_DETECT_PIN_GPIO (GPIO_VBUS1_DETECT_PIN & ~(0x80000000))
		#define VBUS_DETECT_PIN_EINT CUST_EINT_USB_VBUS1_NUM
	#endif
#endif

#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))  
extern bool port1_qmu_stop;
#endif

static struct musb_fifo_cfg usb11_fifo_cfg_host[] = {
{ .hw_ep_num =  1, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  1, .style = MUSB_FIFO_RX,   .maxpacket = 1024, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  5, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	5, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  6, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	6, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_RX,   .maxpacket = 64,  .mode = MUSB_BUF_SINGLE},
};

u32 usb11_delay_time = 15;
module_param(usb11_delay_time,int,0644);
u32 usb11_delay_time1 = 55;
module_param(usb11_delay_time1,int,0644);

void mt_usb11_set_vbus(struct musb *musb, int is_on)
{
	DBG(0,"mt_usb11_set_vbus++,is_on=%d\r\n",is_on);
	if(is_on){
		#ifdef MTK_USB11_VBUS_DETECT_SUPPORT
		/* disable VBUS detect pin interrupt */
        mt_eint_mask(VBUS_DETECT_PIN_EINT);
        msleep(100);
		#endif
		
#ifdef CONFIG_USB_HOST_OC_DETECT
    overcurrent_start_processing();
#endif /* CONFIG_USB_HOST_OC_DETECT */
		mt_set_gpio_mode(GPIO_VBUS_ON,GPIO_MODE_00);
		mt_set_gpio_dir(GPIO_VBUS_ON, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_VBUS_ON,GPIO_OUT_ONE);
		printk("mt_usb11_set_vbus set vbus on and disable VBUS detect pin interrupt \n");
	} else {
#ifdef CONFIG_USB_HOST_OC_DETECT
    overcurrent_end_processing();
    mt_eint_mask(OC_DETECT_PIN_EINT); // This EINT mask is necessary after the overcurrent end processing.
#endif /* CONFIG_USB_HOST_OC_DETECT */
		mt_set_gpio_mode(GPIO_VBUS_ON,GPIO_MODE_00);
		mt_set_gpio_dir(GPIO_VBUS_ON, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_VBUS_ON,GPIO_OUT_ZERO);	
		#ifdef MTK_USB11_VBUS_DETECT_SUPPORT
		msleep(100);
        /* enable VBUS detect pin interrupt */
        mt_eint_unmask(VBUS_DETECT_PIN_EINT); 
		#endif
		
		printk("mt_usb11_set_vbus set vbus off and enable VBUS detect pin interrupt\n");
	}
}

int mt_usb11_get_vbus_status(struct musb *musb)
{
#if 1
	return true;
#else
	int	ret = 0;

	if ((musb11_readb(musb->mregs, MUSB11_DEVCTL)& MUSB11_DEVCTL_VBUS) != MUSB11_DEVCTL_VBUS) {
		ret = 1;
	} else {
		DBG(0, "VBUS error, devctl=%x, power=%d\n", musb_readb(musb->mregs,MUSB11_DEVCTL), musb->power);
	}
    printk("vbus ready = %d \n", ret);
	return ret;
#endif
}

void mt_usb11_init_drvvbus(void)
{
#ifdef P2_PROJECT
	mt_set_gpio_mode(GPIO_VBUS_ON,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_VBUS_ON, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_VBUS_ON,GPIO_OUT_ZERO); 
	
	#ifdef MTK_USB11_VBUS_DETECT_SUPPORT
		msleep(100);
		/* enable VBUS detect pin interrupt */
		mt_eint_unmask(VBUS_DETECT_PIN_EINT); 
	#endif	
#endif
}

u32 usb11_sw_deboun_time = 400;
module_param(usb11_sw_deboun_time,int,0644);
struct switch_dev usb11_otg_state;
extern int ep_config_from_table_for_usb11_host(struct musb *musb);

#if defined(CONFIG_USB11_MTK_OTG) && !defined(MUSB11_PORT1_HOST) && defined(CONFIG_USB11_MTK_HDRC_GADGET)
static bool musb11_is_host(void)
{
	u8 devctl = 0;
    int iddig_state = 1;
    bool usb_is_host = 0;

    DBG(0,"will mask PMIC charger detection\n");
#ifndef FPGA_PLATFORM
    //pmic_chrdet_int_en(0);
#endif

    musb_platform_enable(mtk_musb11);
    
#if (!defined(CONFIG_USB11_MTK_HDRC_GADGET))
return true;
#endif

#ifdef MUSB11_ID_PIN_USE_EX_EINT
    iddig_state = mt_get_gpio_in(GPIO_OTG_INT);
	DBG(0,"iddig_state = %d\n", iddig_state);
#else
    iddig_state = 0 ;
    devctl = musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL);
    DBG(0, "devctl = %x before end session\n", devctl);
    devctl &= ~MUSB11_DEVCTL_SESSION;	// this will cause A-device change back to B-device after A-cable plug out
    musb11_writeb(mtk_musb11->mregs, MUSB11_DEVCTL, devctl);
    msleep(usb11_delay_time);

    devctl = musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL);
    DBG(0,"devctl = %x before set session\n",devctl);

    devctl |= MUSB11_DEVCTL_SESSION;
    musb11_writeb(mtk_musb11->mregs,MUSB11_DEVCTL,devctl);
    msleep(usb11_delay_time1);
    devctl = musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL);
    DBG(0,"devclt = %x\n",devctl);
#endif
    if ( devctl & MUSB11_DEVCTL_BDEVICE || iddig_state) {
        DBG(0,"will unmask PMIC charger detection\n");
#ifndef FPGA_PLATFORM
        //pmic_chrdet_int_en(1);
#endif
        usb_is_host = false;
    } else {
        usb_is_host = true;
    }
	DBG(0,"usb_is_host = %d\n", usb_is_host);
	return usb_is_host;
}
#endif

void musb11_session_restart(struct musb *musb)
{
	void __iomem	*mbase = musb->mregs;
	musb11_writeb(mbase, MUSB11_DEVCTL, (musb11_readb(mbase, MUSB11_DEVCTL) & (~MUSB11_DEVCTL_SESSION)));
	DBG(0,"[MUSB] stopped session for VBUSERROR interrupt\n");
	USB11PHY_SET8(0x6d, 0x3c);
	USB11PHY_SET8(0x6c, 0x10);
	USB11PHY_CLR8(0x6c, 0x2c);
	DBG(0,"[MUSB] force PHY to idle, 0x6d=%x, 0x6c=%x\n", USB11PHY_READ8(0x6d), USB11PHY_READ8(0x6c));
	mdelay(5);
	USB11PHY_CLR8(0x6d, 0x3c);
	USB11PHY_CLR8(0x6c, 0x3c);
	DBG(0,"[MUSB] let PHY resample VBUS, 0x6d=%x, 0x6c=%x\n", USB11PHY_READ8(0x6d), USB11PHY_READ8(0x6c));
	musb11_writeb(mbase, MUSB11_DEVCTL, (musb11_readb(mbase, MUSB11_DEVCTL) | MUSB11_DEVCTL_SESSION));
	DBG(0,"[MUSB] restart session\n");
}

void usb11_switch_int_to_device(struct musb *musb)
{
#if defined(CONFIG_USB11_MTK_OTG) && !defined(MUSB11_PORT1_HOST) && defined(CONFIG_USB11_MTK_HDRC_GADGET)
#ifdef MUSB11_ID_PIN_USE_EX_EINT
    mt_eint_set_polarity(IDDIG_EINT_PIN, MT_EINT_POL_POS);
	mt_eint_unmask(IDDIG_EINT_PIN);
#else
	 musb11_writel(musb->mregs,MUSB11_L1INTP,0);
	 musb11_writel(musb->mregs,MUSB11_L1INTM,MUSB11_IDDIG_INT_STATUS|musb11_readl(musb->mregs,MUSB11_L1INTM));
#endif
#endif
	 DBG(0,"usb11_switch_int_to_device is done\n");
}

void usb11_switch_int_to_host(struct musb *musb)
{
#if defined(CONFIG_USB11_MTK_OTG) && !defined(MUSB11_PORT1_HOST) && defined(CONFIG_USB11_MTK_HDRC_GADGET)
#ifdef MUSB11_ID_PIN_USE_EX_EINT
    mt_eint_set_polarity(IDDIG_EINT_PIN, MT_EINT_POL_NEG);
	mt_eint_unmask(IDDIG_EINT_PIN);
#else
	musb11_writel(musb->mregs,MUSB11_L1INTP,MUSB11_IDDIG_INT_STATUS);
	musb11_writel(musb->mregs,MUSB11_L1INTM,MUSB11_IDDIG_INT_STATUS|musb11_readl(musb->mregs,MUSB11_L1INTM));
#endif
#endif
	DBG(0,"usb11_switch_int_to_host is done\n");
}

void usb11_switch_int_to_host_and_mask(struct musb *musb)
{
#if defined(CONFIG_USB11_MTK_OTG) && !defined(MUSB11_PORT1_HOST) && defined(CONFIG_USB11_MTK_HDRC_GADGET)
#ifdef MUSB11_ID_PIN_USE_EX_EINT
    mt_eint_set_polarity(IDDIG_EINT_PIN, MT_EINT_POL_NEG);
	mt_eint_mask(IDDIG_EINT_PIN);
#else
	musb11_writel(musb->mregs,MUSB11_L1INTM,(~MUSB11_IDDIG_INT_STATUS)&musb11_readl(musb->mregs,MUSB11_L1INTM)); //mask before change polarity
	mb();
	musb11_writel(musb->mregs,MUSB11_L1INTP,MUSB11_IDDIG_INT_STATUS);
#endif
#endif
	DBG(0,"swtich_int_to_host_and_mask is done\n");
}

void musb11_do_infra_misc(bool enable)
{

#ifdef USB_OTG_LOW_POWER
	printk("musb_do_ifra_misc:enable = %d\n", enable);
	if(enable)
	{
		DBG(0,"disconnect the otg cable and close infra misc\n");
		spm_mtcmos_ctrl_ifrmiscsys(STA_POWER_DOWN);
	}
	else
	{
		DBG(0,"connect the otg cable and open infra misc\n");
		spm_mtcmos_ctrl_ifrmiscsys(STA_POWER_ON);
		mdelay(50);
		musb11_writel(mtk_musb11->mregs,MUSB11_L1INTM,MUSB11_TX_INT_STATUS | MUSB11_TRX_INT_STATUS | MUSB11_COM_INT_STATUS | MUSB11_DMA_INT_STATUS | MUSB11_QINT_STATUS);
		musb11_writel(mtk_musb11->mregs,MUSB11_L1INTS,0x100);
		musb11_writel(mtk_musb11->mregs,MUSB11_L1INTP,0x200);
		musb11_writel(mtk_musb11->mregs,MUSB_HSDMA_INTR,0xff | (0xff << MUSB11_DMA_INTR_UNMASK_SET_OFFSET));
		
	}
#else
#ifdef MUSB11_ID_PIN_USE_EX_EINT
	if(!enable)
	{
		DBG(0,"connect the otg cable and open infra misc\n");
		musb11_writel(mtk_musb11->mregs,MUSB11_L1INTM,MUSB11_TX_INT_STATUS | MUSB11_TRX_INT_STATUS | MUSB11_COM_INT_STATUS | MUSB11_DMA_INT_STATUS | MUSB11_QINT_STATUS);
		musb11_writel(mtk_musb11->mregs,MUSB11_L1INTS,0x100);
		musb11_writel(mtk_musb11->mregs,MUSB11_L1INTS,0x200);
		musb11_writel(mtk_musb11->mregs,MUSB_HSDMA_INTR,0xff | (0xff << MUSB11_DMA_INTR_UNMASK_SET_OFFSET));
		
	}
#else
	DBG(0,"do nothing\n");
#endif
#endif
}

#if (defined(MUSB11_PORT1_HOST))
void musb11_id_pin_work_host(void)
{	
		#ifdef MUSB11_ID_PIN_USE_EX_EINT
		u8 devctl = 0;
		#endif
		
		unsigned long flags;
		
		#ifdef P2_PROJECT
		musb_platform_set_vbus(mtk_musb11, 1);
		msleep(100);
		DBG(0, "P2_PROJECT\n");
		#endif
		
		spin_lock_irqsave(&mtk_musb11->lock, flags);
		musb11_generic_disable(mtk_musb11);
		spin_unlock_irqrestore(&mtk_musb11->lock, flags);

		down(&mtk_musb11->musb_lock);
		DBG(0, "work start, is_host=%d\n", mtk_musb11->is_host);
		if(mtk_musb11->in_ipo_off) {
			DBG(0, "do nothing due to in_ipo_off\n");
			goto out;
		}
		mtk_musb11 ->is_host = true;
		DBG(0,"musb is as %s\n",mtk_musb11->is_host?"host":"device");
	
		if (mtk_musb11->is_host) {
		#ifndef MTK_USB11_VBUS_DETECT_SUPPORT
			musb11_stop(mtk_musb11);
		#endif
			//setup fifo for host mode
			ep_config_from_table_for_usb11_host(mtk_musb11);
		#ifndef WITH_USB11_ENTER_SUSPEND
			wake_lock(&mtk_musb11->usb_lock);
		#endif
			/* for no VBUS sensing IP*/        
			/* wait VBUS ready */
			msleep(100);			
			musb11_start(mtk_musb11);
			MUSB_HST_MODE(mtk_musb11);
		#ifdef MUSB11_ID_PIN_USE_EX_EINT
				/* clear session*/
				devctl = musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL);
				musb11_writeb(mtk_musb11->mregs, MUSB11_DEVCTL, (devctl&(~MUSB11_DEVCTL_SESSION)));
				/* USB MAC OFF*/
				/* VBUSVALID=0, AVALID=0, BVALID=0, SESSEND=1, IDDIG=X */
				USB11PHY_SET8(0x6c, 0x10);
				USB11PHY_CLR8(0x6c, 0x2e);
				USB11PHY_SET8(0x6d, 0x3e);
				DBG(0,"force PHY to idle, 0x6d=%x, 0x6c=%x\n",USB11PHY_READ8(0x6d), USB11PHY_READ8(0x6c));
				/* wait */
				msleep(5);
				/* restart session */
				devctl = musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL);
				musb11_writeb(mtk_musb11->mregs, MUSB11_DEVCTL, (devctl| MUSB11_DEVCTL_SESSION));
				/* USB MAC ONand Host Mode*/
				/* VBUSVALID=1, AVALID=1, BVALID=1, SESSEND=0, IDDIG=0 */
				USB11PHY_CLR8(0x6c, 0x10);
				USB11PHY_SET8(0x6c, 0x2c);
				USB11PHY_SET8(0x6d, 0x3e);
				DBG(0,"force PHY to host mode, 0x6d=%x, 0x6c=%x devctl=%x\n",USB11PHY_READ8(0x6d), USB11PHY_READ8(0x6c),musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL));
		#endif
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD))	 
				DBG(0, "[HUB] PHY INIT hub before setting USB11PHY_SET8(0x24, 0x01) = %x USB11PHY_CLR8(0x1A, 0x80) = %x USB11PHY_READ8(0x18) = %x\n",USB11PHY_READ8(0x24),USB11PHY_READ8(0x1a),USB11PHY_READ8(0x18));
				USB11PHY_SET8(0x24, 0x01);
				USB11PHY_CLR8(0x1A, 0x80);
				USB11PHY_SET8(0x18, 0xF0);
				DBG(0, "[HUB] PHY INIT hub after setting USB11PHY_SET8(0x24, 0x01) = %x USB11PHY_CLR8(0x1A, 0x80) = %x USB11PHY_READ8(0x18) = %x\n",USB11PHY_READ8(0x24),USB11PHY_READ8(0x1a),USB11PHY_READ8(0x18));
		#endif
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))  
				DBG(0, "[HUB]forbid disconnect compare\n");
				USB11PHY_CLR8(0x19, 0x30);
				USB11PHY_SET8(0x19, 0x20);
				port1_qmu_stop = false;
				DBG(0, "[HUB]forbid disconnect compare: 0x19:0x%x\n", USB11PHY_READ8(0x19));
		#endif

		} 
	out:
		DBG(0, "work end, is_host=%d\n", mtk_musb11->is_host);
		up(&mtk_musb11->musb_lock);
}
#endif

#if defined(CONFIG_USB11_MTK_OTG) && !defined(MUSB11_PORT1_HOST) && defined(CONFIG_USB11_MTK_HDRC_GADGET)
static void musb11_id_pin_work(struct work_struct *data)
{	
		#ifdef MUSB11_ID_PIN_USE_EX_EINT
		u8 devctl = 0;
		int iddig_state = 1;
		#endif
		unsigned long flags;
		
		printk("musb11_id_pin_work\n");
		#ifdef MUSB11_ID_PIN_USE_EX_EINT
		iddig_state = mt_get_gpio_in(GPIO_OTG_INT);
		if(!iddig_state)
			musb11_do_infra_misc(iddig_state);
		#endif

		#ifdef P2_PROJECT
		musb_platform_set_vbus(mtk_musb11, 1);
		msleep(100);
		DBG(0, "P2_PROJECT\n");
		#endif
		
		spin_lock_irqsave(&mtk_musb11->lock, flags);
		musb11_generic_disable(mtk_musb11);
		spin_unlock_irqrestore(&mtk_musb11->lock, flags);

		down(&mtk_musb11->musb_lock);
		DBG(0, "work start, is_host=%d\n", mtk_musb11->is_host);
		if(mtk_musb11->in_ipo_off) {
			DBG(0, "do nothing due to in_ipo_off\n");
			goto out;
		}
		mtk_musb11 ->is_host = musb11_is_host();
		DBG(0,"musb is as %s\n",mtk_musb11->is_host?"host":"device");
		switch_set_state((struct switch_dev *)&usb11_otg_state, mtk_musb11->is_host);
	
		if (mtk_musb11->is_host) {
		#ifndef MTK_USB11_VBUS_DETECT_SUPPORT
			musb11_stop(mtk_musb11);
		#endif
		
			//setup fifo for host mode
			ep_config_from_table_for_usb11_host(mtk_musb11);
		#ifndef WITH_USB11_ENTER_SUSPEND
			wake_lock(&mtk_musb11->usb_lock);
		#endif
			
			#ifndef P2_PROJECT
			musb_platform_set_vbus(mtk_musb11, 1);
			DBG(0, "not P2_PROJECT\n");
			#endif
			
			/* for no VBUS sensing IP*/        
			/* wait VBUS ready */
			msleep(100);			
		//#ifdef  MUSB11_ID_PIN_USE_EX_EINT
		
			musb11_start(mtk_musb11);
			MUSB_HST_MODE(mtk_musb11);
			usb11_switch_int_to_device(mtk_musb11);
		#ifdef MUSB11_ID_PIN_USE_EX_EINT
				/* clear session*/
				devctl = musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL);
				musb11_writeb(mtk_musb11->mregs, MUSB11_DEVCTL, (devctl&(~MUSB11_DEVCTL_SESSION)));
				/* USB MAC OFF*/
				/* VBUSVALID=0, AVALID=0, BVALID=0, SESSEND=1, IDDIG=X */
				USB11PHY_SET8(0x6c, 0x10);
				USB11PHY_CLR8(0x6c, 0x2e);
				USB11PHY_SET8(0x6d, 0x3e);
				DBG(0,"force PHY to idle, 0x6d=%x, 0x6c=%x\n",USB11PHY_READ8(0x6d), USB11PHY_READ8(0x6c));
				/* wait */
				msleep(5);
				/* restart session */
				devctl = musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL);
				musb11_writeb(mtk_musb11->mregs, MUSB11_DEVCTL, (devctl| MUSB11_DEVCTL_SESSION));
				/* USB MAC ONand Host Mode*/
				/* VBUSVALID=1, AVALID=1, BVALID=1, SESSEND=0, IDDIG=0 */
				USB11PHY_CLR8(0x6c, 0x10);
				USB11PHY_SET8(0x6c, 0x2c);
				USB11PHY_SET8(0x6d, 0x3e);
				DBG(0,"force PHY to host mode, 0x6d=%x, 0x6c=%x\n",USB11PHY_READ8(0x6d), USB11PHY_READ8(0x6c));
		#endif
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD))	 
				DBG(0, "[HUB] PHY INIT hub before setting USB11PHY_SET8(0x24, 0x01) = %x USB11PHY_CLR8(0x1A, 0x80) = %x USB11PHY_READ8(0x18) = %x\n",USB11PHY_READ8(0x24),USB11PHY_READ8(0x1a),USB11PHY_READ8(0x18));
				USB11PHY_SET8(0x24, 0x01);
				USB11PHY_CLR8(0x1A, 0x80);
				USB11PHY_SET8(0x18, 0xF0);
				DBG(0, "[HUB] PHY INIT hub after setting USB11PHY_SET8(0x24, 0x01) = %x USB11PHY_CLR8(0x1A, 0x80) = %x USB11PHY_READ8(0x18) = %x\n",USB11PHY_READ8(0x24),USB11PHY_READ8(0x1a),USB11PHY_READ8(0x18));
		#endif
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))  
				DBG(0, "[HUB]forbid disconnect compare\n");
				USB11PHY_CLR8(0x19, 0x30);
				USB11PHY_SET8(0x19, 0x20);
				port1_qmu_stop = false;
				DBG(0, "[HUB]forbid disconnect compare: 0x19:0x%x\n", USB11PHY_READ8(0x19));
		#endif

		}else {
			DBG(0,"devctl is %x\n",musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL));
			musb11_writeb(mtk_musb11->mregs,MUSB11_DEVCTL,0);
			if (wake_lock_active(&mtk_musb11->usb_lock))
				wake_unlock(&mtk_musb11->usb_lock);
			musb_platform_set_vbus(mtk_musb11, 0);
			/* for no VBUS sensing IP */
        #ifdef MUSB11_ID_PIN_USE_EX_EINT
			/* USB MAC OFF*/
			/* VBUSVALID=0, AVALID=0, BVALID=0, SESSEND=1, IDDIG=X */
			USB11PHY_SET8(0x6c, 0x10);
			USB11PHY_CLR8(0x6c, 0x2e);
			USB11PHY_SET8(0x6d, 0x3e);
			DBG(0,"force PHY to idle, 0x6d=%x, 0x6c=%x\n", USB11PHY_READ8(0x6d), USB11PHY_READ8(0x6c));
        #endif
			musb11_stop(mtk_musb11);
			//ALPS00849138
			mtk_musb11->xceiv->state =  OTG_STATE_B_IDLE;
			#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
			port1_qmu_stop = false;
			#endif
			MUSB_DEV_MODE(mtk_musb11);
			usb11_switch_int_to_host(mtk_musb11);

			#ifndef MTK_USB11_VBUS_DETECT_SUPPORT
			musb11_stop(mtk_musb11);
			#endif

			#ifdef MUSB11_ID_PIN_USE_EX_EINT
			if(iddig_state)
			musb11_do_infra_misc(iddig_state);
			#endif
		}
	out:
		DBG(0, "work end, is_host=%d\n", mtk_musb11->is_host);
		if (mtk_musb11->is_host)
			mtk_musb11->xceiv->state =  OTG_STATE_A_HOST;
		up(&mtk_musb11->musb_lock);
	

}
#endif

#ifdef USB11_SW_WITCH_MODE
void musb11_id_pin_sw_work(bool host_mode)
{	
		u8 devctl = 0;
		unsigned long flags;

		#ifdef ID_PIN_USE_EX_EINT
		if (host_mode)
			musb11_do_infra_misc(false);
		#endif

		#ifdef P2_PROJECT
		musb_platform_set_vbus(mtk_musb11, host_mode);
		msleep(100);
		DBG(0, "P2_PROJECT\n");
		#endif
		
		spin_lock_irqsave(&mtk_musb11->lock, flags);
		musb11_generic_disable(mtk_musb11);
		spin_unlock_irqrestore(&mtk_musb11->lock, flags);

		down(&mtk_musb11->musb_lock);
		DBG(0,"work start, is_host=%d\n", mtk_musb11->is_host);
		
		if(mtk_musb11->in_ipo_off) {
			DBG(0, "do nothing due to in_ipo_off\n");
			goto out;
		}
		mtk_musb11 ->is_host = host_mode;
        musb_platform_enable(mtk_musb11);
		DBG(0,"musb is as %s\n",mtk_musb11->is_host?"host":"device");
		switch_set_state((struct switch_dev *)&usb11_otg_state, mtk_musb11->is_host);
	
		if (mtk_musb11->is_host) {
			#ifndef MTK_USB11_VBUS_DETECT_SUPPORT
			musb11_stop(mtk_musb11);
			#endif
		
			//setup fifo for host mode
			ep_config_from_table_for_usb11_host(mtk_musb11);
			#ifndef WITH_USB11_ENTER_SUSPEND
			wake_lock(&mtk_musb11->usb_lock);
			#endif
			
			#ifndef P2_PROJECT
			musb_platform_set_vbus(mtk_musb11, 1);
			DBG(0, "not P2_PROJECT\n");
			#endif
			
			/* for no VBUS sensing IP*/        
			/* wait VBUS ready */
			msleep(100);					
			musb11_start(mtk_musb11);
			/* clear session*/
			devctl = musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL);
			musb11_writeb(mtk_musb11->mregs, MUSB11_DEVCTL, (devctl&(~MUSB11_DEVCTL_SESSION)));
			/* USB MAC OFF*/
			/* VBUSVALID=0, AVALID=0, BVALID=0, SESSEND=1, IDDIG=X */
			USB11PHY_SET8(0x6c, 0x10);
			USB11PHY_CLR8(0x6c, 0x2e);
			USB11PHY_SET8(0x6d, 0x3e);
			DBG(0,"force PHY to idle, 0x6d=%x, 0x6c=%x\n",USB11PHY_READ8(0x6d), USB11PHY_READ8(0x6c));
			/* wait */
			msleep(5);
			/* restart session */
			devctl = musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL);
			musb11_writeb(mtk_musb11->mregs, MUSB11_DEVCTL, (devctl| MUSB11_DEVCTL_SESSION));
			/* USB MAC ONand Host Mode*/
			/* VBUSVALID=1, AVALID=1, BVALID=1, SESSEND=0, IDDIG=0 */
			USB11PHY_CLR8(0x6c, 0x10);
			USB11PHY_SET8(0x6c, 0x2c);
			USB11PHY_SET8(0x6d, 0x3e);
			MUSB_HST_MODE(mtk_musb11);
			DBG(0,"force PHY to host mode, 0x6d=%x, 0x6c=%x\n",USB11PHY_READ8(0x6d), USB11PHY_READ8(0x6c));
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD))
			DBG(0, "[HUB] PHY INIT hub before setting USB11PHY_SET8(0x24, 0x01) = %x USB11PHY_CLR8(0x1A, 0x80) = %x USB11PHY_READ8(0x18) = %x\n",USB11PHY_READ8(0x24),USB11PHY_READ8(0x1a),USB11PHY_READ8(0x18));
			USB11PHY_SET8(0x24, 0x01);
			USB11PHY_CLR8(0x1A, 0x80);
			USB11PHY_SET8(0x18, 0xF0);
			DBG(0, "[HUB] PHY INIT hub after setting USB11PHY_SET8(0x24, 0x01) = %x USB11PHY_CLR8(0x1A, 0x80) = %x USB11PHY_READ8(0x18) = %x\n",USB11PHY_READ8(0x24),USB11PHY_READ8(0x1a),USB11PHY_READ8(0x18));
		#endif
		#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
			DBG(0, "[HUB]forbid disconnect compare\n");
			USB11PHY_CLR8(0x19, 0x30);
			USB11PHY_SET8(0x19, 0x20);
			port1_qmu_stop = false;
			DBG(0, "[HUB]forbid disconnect compare: 0x19:0x%x\n", USB11PHY_READ8(0x19));
		#endif
		}else {
			DBG(0,"devctl is %x\n",musb11_readb(mtk_musb11->mregs,MUSB11_DEVCTL));
			musb11_writeb(mtk_musb11->mregs,MUSB11_DEVCTL,0);
			if (wake_lock_active(&mtk_musb11->usb_lock))
				wake_unlock(&mtk_musb11->usb_lock);
			musb_platform_set_vbus(mtk_musb11, 0);
			/* for no VBUS sensing IP */
			/* USB MAC OFF*/
			/* VBUSVALID=0, AVALID=0, BVALID=0, SESSEND=1, IDDIG=X */
			USB11PHY_SET8(0x6c, 0x10);
			USB11PHY_CLR8(0x6c, 0x2e);
			USB11PHY_SET8(0x6d, 0x3e);
			DBG(0,"force PHY to idle, 0x6d=%x, 0x6c=%x\n", USB11PHY_READ8(0x6d), USB11PHY_READ8(0x6c));
			musb11_stop(mtk_musb11);
			//ALPS00849138
			mtk_musb11->xceiv->state =  OTG_STATE_B_IDLE;
			#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
			port1_qmu_stop = false;
			#endif
			MUSB_DEV_MODE(mtk_musb11);
			#ifndef MTK_USB11_VBUS_DETECT_SUPPORT
			musb11_stop(mtk_musb11);
			#endif
			#ifdef ID_PIN_USE_EX_EINT
			musb11_do_infra_misc(true);
			#endif
		}
	out:
		DBG(0, "work end, is_host=%d\n", mtk_musb11->is_host);
		up(&mtk_musb11->musb_lock);
	

}
#endif

#if defined(MUSB11_ID_PIN_USE_EX_EINT) && defined(CONFIG_USB11_MTK_HDRC_GADGET)
#if (defined(CONFIG_USB11_MTK_OTG) && !defined(MUSB11_PORT1_HOST))
static void mt_usb11_ext_iddig_int(void)
{
#if 0
#ifndef USB11_SW_WITCH_MODE
    if (!mtk_musb11->is_ready) {
        /* dealy 5 sec if usb function is not ready */
        schedule_delayed_work(&mtk_musb11->id_pin_work,5000*HZ/1000);
    } else {
        schedule_delayed_work(&mtk_musb11->id_pin_work,usb11_sw_deboun_time*HZ/1000);
    }
#else
		DBG(0,"use software to switch\n");
#endif
#else
        schedule_delayed_work(&mtk_musb11->id_pin_work,usb11_sw_deboun_time*HZ/1000);
#endif
	DBG(0,"id pin interrupt assert\n");
}
#endif
#endif
void mt_usb11_iddig_int(struct musb *musb)
{
    u32 usb_l1_ploy = musb11_readl(musb->mregs,MUSB11_L1INTP);  
    DBG(0,"id pin interrupt assert,polarity=0x%x\n",usb_l1_ploy);
    if (usb_l1_ploy & MUSB11_IDDIG_INT_STATUS) {
        usb_l1_ploy &= (~MUSB11_IDDIG_INT_STATUS);
    } else {
        usb_l1_ploy |= MUSB11_IDDIG_INT_STATUS;
    }

    musb11_writel(musb->mregs,MUSB11_L1INTP,usb_l1_ploy);
    musb11_writel(musb->mregs,MUSB11_L1INTM,(~MUSB11_IDDIG_INT_STATUS)&musb11_readl(musb->mregs,MUSB11_L1INTM));

#if 0
    if (!mtk_musb11->is_ready) {
        /* dealy 5 sec if usb function is not ready */		
        schedule_delayed_work(&mtk_musb11->id_pin_work,5000*HZ/1000);
		
    } else {    	
        schedule_delayed_work(&mtk_musb11->id_pin_work,usb11_sw_deboun_time*HZ/1000);
	}
#else
        schedule_delayed_work(&mtk_musb11->id_pin_work,usb11_sw_deboun_time*HZ/1000);
#endif
}

#if defined(CONFIG_USB11_MTK_OTG) && !defined(MUSB11_PORT1_HOST) && defined(CONFIG_USB11_MTK_HDRC_GADGET)
void static usb11_otg_int_init(void)
{
#if 0
//#if	(defined(MUSB11_ID_PIN_USE_EX_EINT) && defined(P1_PROJECT))
	mt_set_gpio_mode(GPIO_OTG_INT, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_OTG_INT, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_OTG_INT, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_OTG_INT, GPIO_PULL_UP);
	mt_eint_set_sens(6, MT_LEVEL_SENSITIVE);
	mt_eint_set_hw_debounce(6,64);
	mt_eint_registration(6, EINTF_TRIGGER_LOW, mt_usb11_ext_iddig_int, FALSE);
	printk("usb11_otg_int_init MUSB11_ID_PIN_USE_EX_EINT P1_PROJECT \n");
//#elif (defined(MUSB11_ID_PIN_USE_EX_EINT) && defined(MUSB11_PORT1_SUPPORT_OTG))
#endif
#if defined(MUSB11_ID_PIN_USE_EX_EINT)
	mt_set_gpio_mode(GPIO_OTG_INT, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_OTG_INT, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_OTG_INT, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_OTG_INT, GPIO_PULL_UP);
	mt_eint_set_sens(GPIO_OTG_INT_EINT, MT_LEVEL_SENSITIVE);
	mt_eint_set_hw_debounce(GPIO_OTG_INT_EINT,64);
	mt_eint_registration(GPIO_OTG_INT_EINT, EINTF_TRIGGER_LOW, mt_usb11_ext_iddig_int, FALSE);
	printk(" usb11_otg_int_init MUSB11_ID_PIN_USE_EX_EINT MUSB11_PORT1_SUPPORT_OTG \n");
#else
	u32 phy_id_pull = 0;
	mt_set_gpio_mode(GPIO_OTG_INT, GPIO_MODE_02);
   	mt_set_gpio_dir(GPIO_OTG_INT, GPIO_DIR_IN);
   	mt_set_gpio_pull_enable(GPIO_OTG_INT, GPIO_PULL_ENABLE);
   	mt_set_gpio_pull_select(GPIO_OTG_INT, GPIO_PULL_UP);

	phy_id_pull = __raw_readl((void __iomem *)USB11PHYDTM1);
	phy_id_pull |= ID_PULL_UP;
	__raw_writel(phy_id_pull,(void __iomem *)USB11PHYDTM1);

	musb11_writel(mtk_musb11->mregs,MUSB11_L1INTM,MUSB11_IDDIG_INT_STATUS|musb11_readl(mtk_musb11->mregs,MUSB11_L1INTM));	
	printk("usb11_otg_int_init inter MUSB11_PORT1_SUPPORT_OTG \n");

#endif	
}
#endif	

void mt_usb11_otg_init(struct musb *musb)
{
    /*init drrvbus*/
	mt_usb11_init_drvvbus();
	DBG(0, "mt_usb11_otg_init\n");

	/* EP table */
    musb->fifo_cfg_host = usb11_fifo_cfg_host;
    musb->fifo_cfg_host_size = ARRAY_SIZE(usb11_fifo_cfg_host);

    usb11_otg_state.name = "usb11_otg_state";
	usb11_otg_state.index = 0;
	usb11_otg_state.state = 0;

	if(switch_dev_register(&usb11_otg_state))
		printk("switch_dev_register fail\n");
	else
        printk("switch_dev register success\n");

#if defined(CONFIG_USB11_MTK_OTG) && !defined(MUSB11_PORT1_HOST) && defined(CONFIG_USB11_MTK_HDRC_GADGET)
	INIT_DELAYED_WORK(&musb->id_pin_work, musb11_id_pin_work);
	usb11_otg_int_init();
#endif
}
#else

/* for not define CONFIG_USB11_MTK_OTG */
void mt_usb11_otg_init(struct musb *musb){}
void mt_usb11_init_drvvbus(void){}
void mt_usb11_set_vbus(struct musb *musb, int is_on){}
int mt_usb11_get_vbus_status(struct musb *musb){return 1;}
void mt_usb11_iddig_int(struct musb *musb){}
void usb11_switch_int_to_device(struct musb *musb){}
void usb11_switch_int_to_host(struct musb *musb){}
void usb11_switch_int_to_host_and_mask(struct musb *musb){}
void musb11_session_restart(struct musb *musb){}

#endif
