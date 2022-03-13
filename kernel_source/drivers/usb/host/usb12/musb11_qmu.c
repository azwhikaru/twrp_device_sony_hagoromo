/*
 * Copyright 2016 Sony Corporation
 * File Changed on 2016-10-17
 */
#ifdef MUSB11_QMU_SUPPORT

#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/stat.h>

#include <linux/musb11/musb11_core.h>
#include <linux/musb11/musb11_host.h>
#include <linux/musb11/musb11_hsdma.h>
#include <linux/musb11/mtk11_musb.h>
#include <linux/musb11/musb11_qmu.h>

//#ifdef CONFIG_OF
extern struct musb *mtk_musb11;
//#endif
#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
extern bool port1_qmu_stop;
#endif

void __iomem* usb11_qmu_base;
/* debug variable to check qmu_base issue */
void __iomem* usb11_qmu_base_2;

int musb11_qmu_init(struct musb *musb)
{
	/* set DMA channel 0 burst mode to boost QMU speed */
	musb11_writel(musb->mregs, 0x204 , musb11_readl(musb->mregs, 0x204) | 0x600 ) ;

#ifdef CONFIG_OF
	usb11_qmu_base = (void __iomem*)(mtk_musb11->mregs + MUSB11_QMUBASE);
	/* debug variable to check qmu_base issue */
	usb11_qmu_base_2 = (void __iomem*)(mtk_musb11->mregs + MUSB11_QMUBASE);
#else
	usb11_qmu_base = (void __iomem*)(USB1_BASE + MUSB11_QMUBASE);
	/* debug variable to check qmu_base issue */
	usb11_qmu_base_2 = (void __iomem*)(mtk_musb11->mregs + MUSB11_QMUBASE);
	musb11_writel((void __iomem*)(mtk_musb11->mregs + MUSB11_QISAR),0x30,0);
	musb11_writel((void __iomem*)(USB1_BASE + MUSB11_QISAR),0x30,0);
#endif
	mb();

	if(qmu11_init_gpd_pool(musb->controller)){
		QMU_ERR("[QMU]qmu_init_gpd_pool fail\n");
		return -1 ;
	}

    return 0;
}

void musb11_qmu_exit(struct musb *musb)
{
	qmu11_destroy_gpd_pool(musb->controller);
}

void musb11_disable_q_all(struct musb *musb)
{
    u32 ep_num;
	QMU_WARN("disable_q_all\n");

    for(ep_num = 1; ep_num <= MUSB11_RXQ_NUM; ep_num++){
        if(mtk_is_qmu11_enabled(ep_num, MUSB11_RXQ)){
            mtk11_disable_q(musb, ep_num, 1);
		}
    }
    for(ep_num = 1; ep_num <= MUSB11_TXQ_NUM; ep_num++){
        if(mtk_is_qmu11_enabled(ep_num, MUSB11_TXQ)){
            mtk11_disable_q(musb, ep_num, 0);
		}
    }
}

//jingao:add tmp
extern void mtk_qmu11_insert_task_ioc(u8 ep_num, u8 isRx, u8* buf, u32 length, u8 zlp,u8 ioc);

void musb11_kick_D_CmdQ(struct musb *musb, struct musb_request *request)
{
    int isRx;

    isRx = request->tx ? 0 : 1;

	/* enable qmu at musb_gadget_eanble */
#if 0
    if(!mtk_is_qmu11_enabled(request->epnum,isRx)){
		/* enable qmu */
        mtk_qmu_enable(musb, request->epnum, isRx);
    }
#endif

	if(request->request.number_of_packets ==0){
	/* note tx needed additional zlp field */
    mtk_qmu11_insert_task(request->epnum,
						isRx,
						(u8*)request->request.dma,
						request->request.length, ((request->request.zero==1)?1:0));
		//musb11_writel(musb->mregs,MUSB11_L1INTM, musb11_readl(musb->mregs,MUSB11_L1INTM)| MUSB11_QINT_STATUS);
		mtk_qmu11_resume(request->epnum, isRx);
	}else{	//jingao:add for sony tinycap device.
		int i =0; 
		u8 isioc =0;
		u8 * pBuffer = (uint8_t*)request->request.dma;
		uint32_t offset,dwlength;
				
		for(i =0;i<request->request.number_of_packets;i++){
			offset = request->request.iso_frame_desc[i].offset;
			dwlength = request->request.iso_frame_desc[i].length;
			isioc = (i ==(request->request.number_of_packets -1)) ? 1 : 0;			
			//printk("jingao:insert_gpd%d offset = %d,dwlength =%d \n",i,offset,dwlength);
			mtk_qmu11_insert_task_ioc(request->epnum,
								isRx,
								pBuffer+offset,
								dwlength, ((request->request.zero==1)?1:0),isioc);
	//musb_writel(musb->mregs,MUSB11_L1INTM, musb11_readl(musb->mregs,MUSB11_L1INTM)| MUSB11_QINT_STATUS);
	mtk_qmu11_resume(request->epnum, isRx);	
		}		
	}	
}

irqreturn_t musb11_q_irq(struct musb *musb){

	irqreturn_t retval = IRQ_NONE;
	u32 wQmuVal = musb->int_queue;
#ifndef QMU_TASKLET
	int i;
#endif

	QMU_INFO("wQmuVal:%d\n", wQmuVal);
#ifdef QMU_TASKLET
	if (musb->qmu_done_intr != 0) {
		musb->qmu_done_intr = wQmuVal | musb->qmu_done_intr;
		QMU_WARN("Has not handle yet %x\n", musb->qmu_done_intr);
	} else {
		musb->qmu_done_intr = wQmuVal;
	}
	tasklet_schedule(&musb->qmu_done);
#else
	for(i = 1; i<= MUSB11_MAX_QMU_EP; i++) {
		if (wQmuVal & DQMU_M_RX_DONE(i)){
			#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
			if (!port1_qmu_stop) {
			#endif
			if (!musb->is_host){
				qmu11_done_rx(musb, i);
			}
			#ifdef MUSB11_QMU_SUPPORT_HOST
			else{				
				h_qmu11_done_rx(musb, i);			
			}		
			#endif	
			#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
			}
			else {
				QMU_WARN("QMU has already stopped!!\n");
				return retval;
			}
			#endif
		}
		if (wQmuVal & DQMU_M_TX_DONE(i)){
			#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
			if (!port1_qmu_stop) {
			#endif
			if (!musb->is_host){
				qmu11_done_tx(musb, i);
			}
			#ifdef MUSB11_QMU_SUPPORT_HOST
			else{				
				h_qmu11_done_tx(musb, i);					
				}
				#endif
			#if (defined(CONFIG_USB_MTK_DOUBLEHUB_THREAD) || defined(CONFIG_USB_MTK_DETECT))
			}
			else {
				QMU_WARN("QMU has already stopped!!\n");
				return retval;
			}
			#endif
		}
	}
#endif
	mtk_qmu11_irq_err(musb, wQmuVal);

	return retval;
}

void musb11_flush_qmu(u32 ep_num, u8 isRx)
{
	QMU_WARN("flush %s(%d)\n", isRx?"RQ":"TQ", ep_num);
	mtk_qmu11_stop(ep_num, isRx);
	qmu11_reset_gpd_pool(ep_num, isRx);
}

void musb11_restart_qmu(struct musb *musb, u32 ep_num, u8 isRx)
{
	QMU_WARN("restart %s(%d)\n", isRx?"RQ":"TQ", ep_num);
	usb11_flush_ep_csr(musb, ep_num, isRx);
	mtk_qmu11_enable(musb, ep_num, isRx);
}

bool musb11_is_qmu_stop(u32 ep_num, u8 isRx){
    void __iomem* base = usb11_qmu_base;

	/* debug variable to check qmu_base issue */
	if (usb11_qmu_base != usb11_qmu_base_2) {
		QMU_WARN("qmu_base != qmu_base_2");
		QMU_WARN("qmu_base = %p, qmu_base_2=%p",usb11_qmu_base, usb11_qmu_base_2);
	}

	if(!isRx){
		if(MGC_ReadQMU16(base, MGC_O_QMU_TQCSR(ep_num)) & DQMU_QUE_ACTIVE){
			return false;
		}else{
			return true;
		}
	} else {
		if(MGC_ReadQMU16(base, MGC_O_QMU_RQCSR(ep_num)) & DQMU_QUE_ACTIVE){
			return false;
		}else{
			return true;
		}
	}
}

void musb11_tx_zlp_qmu(struct musb *musb, u32 ep_num)
{
	/* sent ZLP through PIO */
	void __iomem        *epio = musb->endpoints[ep_num].regs;
	void __iomem		*mbase =  musb->mregs;
	unsigned long timeout = jiffies + HZ;
	int is_timeout = 1;
	u16			csr;

	QMU_WARN("TX ZLP direct sent\n");
	musb_ep_select(mbase, ep_num);

	/* disable dma for pio */
	csr = musb11_readw(epio, MUSB11_TXCSR);
	csr &= ~MUSB11_TXCSR_DMAENAB;
	musb11_writew(epio, MUSB11_TXCSR, csr);

	/* TXPKTRDY */
	csr = musb11_readw(epio, MUSB11_TXCSR);
	csr |= MUSB11_TXCSR_TXPKTRDY;
	musb11_writew(epio, MUSB11_TXCSR, csr);

	/* wait ZLP sent */
	while(time_before_eq(jiffies, timeout)){
		csr = musb11_readw(epio, MUSB11_TXCSR);
		if(!(csr & MUSB11_TXCSR_TXPKTRDY)){
			is_timeout = 0;
			break;
		}
	}

	/* re-enable dma for qmu */
	csr = musb11_readw(epio, MUSB11_TXCSR);
	csr |= MUSB11_TXCSR_DMAENAB;
	musb11_writew(epio, MUSB11_TXCSR, csr);

	if(is_timeout){
		QMU_ERR("TX ZLP sent fail???\n");
	}
	QMU_WARN("TX ZLP sent done\n");
}
#endif
