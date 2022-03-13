/*
 * Copyright 2016 Sony Corporation
 * File Changed on 2016-10-17
 */
/*
 * MUSB OTG driver - support for Mentor's DMA controller
 *
 * Copyright 2005 Mentor Graphics Corporation
 * Copyright (C) 2005-2007 by Texas Instruments
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
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <asm/system.h>
#include <linux/musb11/musb11_core.h>
#include <linux/musb11/musb11_hsdma.h>

static int usb11_dma_controller_start(struct dma_controller *c)
{
	/* nothing to do */
	return 0;
}

static void usb11_dma_channel_release(struct dma_channel *channel);

static int usb11_dma_controller_stop(struct dma_controller *c)
{
	struct musb_dma_controller *controller = container_of(c,
							      struct musb_dma_controller,
							      controller);
	struct musb *musb = controller->private_data;
	struct dma_channel *channel;
	u8 bit;

	if (controller->used_channels != 0) {
		dev_err(musb->controller, "Stopping DMA controller while channel active\n");

		for (bit = 0; bit < MUSB_HSDMA_CHANNELS; bit++) {
			if (controller->used_channels & (1 << bit)) {
				channel = &controller->channel[bit].channel;
				usb11_dma_channel_release(channel);

				if (!controller->used_channels)
					break;
			}
		}
	}

	return 0;
}

static struct dma_channel *usb11_dma_channel_allocate(struct dma_controller *c,
						struct musb_hw_ep *hw_ep, u8 transmit)
{
	struct musb_dma_controller *controller = container_of(c,
							      struct musb_dma_controller,
							      controller);
	struct musb *musb = controller->private_data;
	struct musb_dma_channel *musb_channel = NULL;
	struct dma_channel *channel = NULL;
	u8 bit;

	for (bit = 1; bit < MUSB_HSDMA_CHANNELS; bit++) {
		if (!(controller->used_channels & (1 << bit))) {
			controller->used_channels |= (1 << bit);
			musb_channel = &(controller->channel[bit]);
			musb_channel->controller = controller;
			musb_channel->idx = bit;
			if (musb->is_host) {
				musb_channel->epnum = hw_ep->epnum;
			} else {
				if (transmit)
					/* dma irq will  use this member to get the hw ep. */
					musb_channel->epnum = hw_ep->ep_in.current_epnum;
				else	/* after mapping, hw ep num eques to the current num */
					musb_channel->epnum = hw_ep->ep_out.current_epnum;
			}
			musb_channel->transmit = transmit;
			channel = &(musb_channel->channel);
			channel->private_data = musb_channel;
			channel->status = MUSB_DMA_STATUS_FREE;
			channel->max_len = 0x100000;
			/* Tx => mode 1; Rx => mode 0 */
			channel->desired_mode = transmit;
			channel->actual_len = 0;
			break;
		}
	}

	return channel;
}

static void usb11_dma_channel_release(struct dma_channel *channel)
{
	struct musb_dma_channel *musb_channel = channel->private_data;

	channel->actual_len = 0;
	musb_channel->start_addr = 0;
	musb_channel->len = 0;

	musb_channel->controller->used_channels &= ~(1 << musb_channel->idx);

	channel->status = MUSB_DMA_STATUS_UNKNOWN;
}

static void usb11_configure_channel(struct dma_channel *channel,
			      u16 packet_sz, u8 mode, dma_addr_t dma_addr, u32 len)
{
	struct musb_dma_channel *musb_channel = channel->private_data;
	struct musb_dma_controller *controller = musb_channel->controller;
	void __iomem *mbase = controller->base;
	u8 bchannel = musb_channel->idx;
	u16 csr = 0;

	DBG(4, "%p, pkt_sz %d, addr 0x%x, len %d, mode %d\n",
	    channel, packet_sz, dma_addr, len, mode);

	if (mode) {
		csr |= 1 << MUSB_HSDMA_MODE1_SHIFT;
		BUG_ON(len < packet_sz);
	}
	csr |= MUSB_HSDMA_BURSTMODE_INCR16 << MUSB_HSDMA_BURSTMODE_SHIFT;

	csr |= (musb_channel->epnum << MUSB_HSDMA_ENDPOINT_SHIFT)
	    | (1 << MUSB_HSDMA_ENABLE_SHIFT)
	    | (1 << MUSB_HSDMA_IRQENABLE_SHIFT)
	    | (musb_channel->transmit ? (1 << MUSB_HSDMA_TRANSMIT_SHIFT)
	       : 0);

	/* address/count */
	musb_write_hsdma_addr(mbase, bchannel, dma_addr);
	musb_write_hsdma_count(mbase, bchannel, len);

	/* control (this should start things) */
	musb11_writew(mbase, MUSB_HSDMA_CHANNEL_OFFSET(bchannel, MUSB_HSDMA_CONTROL), csr);
	DBG(5, "MUSB:DMA channel %d control reg is %x\n", bchannel, musb11_readw(mbase,
									       MUSB_HSDMA_CHANNEL_OFFSET
									       (bchannel,
										MUSB_HSDMA_CONTROL)));
}

static int usb11_dma_channel_program(struct dma_channel *channel,
			       u16 packet_sz, u8 mode, dma_addr_t dma_addr, u32 len)
{
	struct musb_dma_channel *musb_channel = channel->private_data;
	struct musb_dma_controller *controller = musb_channel->controller;
	struct musb *musb = controller->private_data;

	DBG(2, "ep%d-%s pkt_sz %d, dma_addr 0x%x length %d, mode %d\n",
	    musb_channel->epnum,
	    musb_channel->transmit ? "Tx" : "Rx", packet_sz, dma_addr, len, mode);

	BUG_ON(channel->status == MUSB_DMA_STATUS_UNKNOWN ||
	       channel->status == MUSB_DMA_STATUS_BUSY);

	/* Let targets check/tweak the arguments */
	if (musb->ops->adjust_channel_params) {
		int ret = musb->ops->adjust_channel_params(channel,
							   packet_sz, &mode, &dma_addr, &len);
		if (ret)
			return ret;
	}
#if 0
	/*
	 * The DMA engine in RTL1.8 and above cannot handle
	 * DMA addresses that are not aligned to a 4 byte boundary.
	 * It ends up masking the last two bits of the address
	 * programmed in DMA_ADDR.
	 *
	 * Fail such DMA transfers, so that the backup PIO mode
	 * can carry out the transfer
	 */
	if ((musb->hwvers >= MUSB_HWVERS_1800) && (dma_addr % 4))
		return false;
#endif
	channel->actual_len = 0;
	musb_channel->start_addr = dma_addr;
	musb_channel->len = len;
	musb_channel->max_packet_sz = packet_sz;
	channel->status = MUSB_DMA_STATUS_BUSY;

	usb11_configure_channel(channel, packet_sz, mode, dma_addr, len);

	return true;
}

static int usb11_dma_channel_abort(struct dma_channel *channel)
{
	struct musb_dma_channel *musb_channel = channel->private_data;
	void __iomem *mbase = musb_channel->controller->base;

	u8 bchannel = musb_channel->idx;
	int offset;
	u16 csr;

	if (channel->status == MUSB_DMA_STATUS_BUSY) {
		if (musb_channel->transmit) {
			offset = MUSB11_EP_OFFSET(musb_channel->epnum, MUSB11_TXCSR);

			/*
			 * The programming guide says that we must clear
			 * the DMAENAB bit before the DMAMODE bit...
			 */
			csr = musb11_readw(mbase, offset);
			csr &= ~(MUSB11_TXCSR_AUTOSET | MUSB11_TXCSR_DMAENAB);
			musb11_writew(mbase, offset, csr);
			csr &= ~MUSB11_TXCSR_DMAMODE;
			musb11_writew(mbase, offset, csr);
		} else {
			offset = MUSB11_EP_OFFSET(musb_channel->epnum, MUSB11_RXCSR);

			csr = musb11_readw(mbase, offset);
			csr &= ~(MUSB11_RXCSR_AUTOCLEAR | MUSB11_RXCSR_DMAENAB | MUSB11_RXCSR_DMAMODE);
			musb11_writew(mbase, offset, csr);
		}

		musb11_writew(mbase, MUSB_HSDMA_CHANNEL_OFFSET(bchannel, MUSB_HSDMA_CONTROL), 0);
		musb_write_hsdma_addr(mbase, bchannel, 0);
		musb_write_hsdma_count(mbase, bchannel, 0);
		channel->status = MUSB_DMA_STATUS_FREE;
	}

	return 0;
}

static int usb11_dma_channel_pause(struct dma_channel *channel)
{
	/*
	 * Probably nothing to be done here. This is needed
	 * only for certain DMA controllers which require
	 * the DMA channel to be paused to get correct DMA
	 * transfer residue
	 */
	return 0;
}

static int usb11_dma_channel_resume(struct dma_channel *channel)
{
	/* Probably nothing to be done here */
	return 0;
}

static int usb11_dma_channel_tx_status(struct dma_channel *channel)
{
	struct musb_dma_channel *musb_channel = channel->private_data;
	void __iomem *mbase = musb_channel->controller->base;

	u8 bchannel = musb_channel->idx;
	u32 addr, count, residue;

	/*
	 * Get the number of bytes left to be transferred over
	 * DMA
	 * The MUSB spec mentions "The DMA controller ADDR register
	 * will have been incremented as packets were unloaded from
	 * the fifo, the processor can determine the size of the
	 * transfer by comparing the current value of ADDR against
	 * the start address of the memory buffer
	 */
	/* residue = musb_read_hsdma_count(mbase, bchannel); */
	addr = musb_read_hsdma_addr(mbase, bchannel);
	count = addr - musb_channel->start_addr;
	residue = channel->prog_len - count;

	return residue;
}

static int usb11_dma_channel_check_residue(struct dma_channel *channel, u32 residue)
{
	int status;

	/* In cases where we know the transfer length and were expecting
	 * a DMA completion we could get into the DMA busy condition
	 * here if the next packet is short and the EP interrupt occurs
	 * before we recieve dma_completion interrupt for current transfer
	 * Wait for dma_completion. MUSB will interrupt us again for this
	 * short packet when we clear the DMA bits
	 */
	if (!residue) {
		/* Wait for DMA completion */
		status = -EINPROGRESS;
	} else if (residue == channel->prog_len) {
		/* Nothing transferred over DMA? */
		/* WARN_ON(1); */
		status = -EINVAL;
	} else {
		/* residue looks OK */
		status = 0;
	}
	return status;
}


irqreturn_t usb11_dma_controller_irq(int irq, void *private_data)
{
	struct musb_dma_controller *controller = private_data;
	struct musb *musb = controller->private_data;
	struct musb_dma_channel *musb_channel;
	struct dma_channel *channel;

	void __iomem *mbase = controller->base;

	irqreturn_t retval = IRQ_NONE;

	unsigned long flags;

	u8 bchannel;
	u8 int_hsdma;

	u32 addr, count;
	u16 csr;

	spin_lock_irqsave(&musb->lock, flags);

	/* musb_read_clear_dma_interrupt */
	int_hsdma = musb11_readb(musb->mregs, MUSB_HSDMA_INTR);
	mb();
	musb11_writeb(musb->mregs, MUSB_HSDMA_INTR, int_hsdma);
	/* musb_read_clear_dma_interrupt */

	if (!int_hsdma) {
		DBG(2, "spurious DMA irq\n");

		for (bchannel = 0; bchannel < MUSB_HSDMA_CHANNELS; bchannel++) {
			musb_channel = (struct musb_dma_channel *)
			    &(controller->channel[bchannel]);
			channel = &musb_channel->channel;
			if (channel->status == MUSB_DMA_STATUS_BUSY) {
				count = musb_read_hsdma_count(mbase, bchannel);

				if (count == 0)
					int_hsdma |= (1 << bchannel);
			}
		}

		DBG(2, "int_hsdma = 0x%x\n", int_hsdma);

		if (!int_hsdma)
			goto done;
	}

	for (bchannel = 0; bchannel < MUSB_HSDMA_CHANNELS; bchannel++) {
		if (int_hsdma & (1 << bchannel)) {
			musb_channel = (struct musb_dma_channel *)
			    &(controller->channel[bchannel]);
			channel = &musb_channel->channel;
			DBG(1, "MUSB:DMA channel %d interrupt\n", bchannel);

			csr = musb11_readw(mbase,
					 MUSB_HSDMA_CHANNEL_OFFSET(bchannel, MUSB_HSDMA_CONTROL));

			if (csr & (1 << MUSB_HSDMA_BUSERROR_SHIFT)) {
				musb_channel->channel.status = MUSB_DMA_STATUS_BUS_ABORT;
			} else {
				u8 devctl;

				addr = musb_read_hsdma_addr(mbase, bchannel);
				channel->actual_len = addr - musb_channel->start_addr;
				/* channel->actual_len = musb11_readl(mbase,MUSB11_USB_DMA_REALCOUNT(bchannel)); */

				DBG(2, "channel %d ch %p, 0x%x -> 0x%x (%zu / %d) %s\n", bchannel,
				    channel, musb_channel->start_addr,
				    addr, channel->actual_len,
				    musb_channel->len,
				    (channel->actual_len
				     < musb_channel->len) ? "=> reconfig 0" : "=> complete");

				devctl = musb11_readb(mbase, MUSB11_DEVCTL);

				channel->status = MUSB_DMA_STATUS_FREE;

				/* completed */
				if ((devctl & MUSB11_DEVCTL_HM)
				    && (musb_channel->transmit)
				    && ((channel->desired_mode == 0)
					|| (channel->actual_len &
					    (musb_channel->max_packet_sz - 1)))
				    ) {
					u8 epnum = musb_channel->epnum;
					int offset = MUSB11_EP_OFFSET(epnum,
								    MUSB11_TXCSR);
					u16 txcsr;

					/*
					 * The programming guide says that we
					 * must clear DMAENAB before DMAMODE.
					 */
					musb_ep_select(mbase, epnum);
					txcsr = musb11_readw(mbase, offset);
					txcsr &= ~(MUSB11_TXCSR_DMAENAB | MUSB11_TXCSR_AUTOSET);
					musb11_writew(mbase, offset, txcsr);
					/* Send out the packet */
					txcsr &= ~MUSB11_TXCSR_DMAMODE;
					txcsr |= MUSB11_TXCSR_TXPKTRDY;
					musb11_writew(mbase, offset, txcsr);
				}
				musb11_dma_completion(musb, musb_channel->epnum,
						    musb_channel->transmit);
			}
		}
	}
	DBG(4, "MUSB: DMA interrupt completino on ep\n");

	retval = IRQ_HANDLED;
 done:
	spin_unlock_irqrestore(&musb->lock, flags);
	return retval;
}

void usb11_dma_controller_destroy(struct dma_controller *c)
{
	struct musb_dma_controller *controller = container_of(c,
							      struct musb_dma_controller,
							      controller);

	if (!controller)
		return;

	if (controller->irq)
		free_irq(controller->irq, c);

	kfree(controller);
}

struct dma_controller *usb11_dma_controller_create(struct musb *musb, void __iomem *base)
{
	struct musb_dma_controller *controller;
	int irq = musb->dma_irq;

	if ((irq <= 0) && (irq != SHARE_IRQ)) {
		DBG(0, "No DMA interrupt line!\n");
		return NULL;
	}

	controller = kzalloc(sizeof(*controller), GFP_KERNEL);
	if (!controller)
		return NULL;

	controller->channel_count = MUSB_HSDMA_CHANNELS;
	controller->private_data = musb;
	controller->base = base;

	controller->controller.start = usb11_dma_controller_start;
	controller->controller.stop = usb11_dma_controller_stop;
	controller->controller.channel_alloc = usb11_dma_channel_allocate;
	controller->controller.channel_release = usb11_dma_channel_release;
	controller->controller.channel_program = usb11_dma_channel_program;
	controller->controller.channel_abort = usb11_dma_channel_abort;
	controller->controller.channel_pause = usb11_dma_channel_pause;
	controller->controller.channel_resume = usb11_dma_channel_resume;
	controller->controller.tx_status = usb11_dma_channel_tx_status;
	controller->controller.check_residue = usb11_dma_channel_check_residue;

	if (irq != SHARE_IRQ) {
		if (request_irq(irq, usb11_dma_controller_irq, 0,
				dev_name(musb->controller), &controller->controller)) {
			DBG(0, "request_irq %d failed!\n", irq);
			usb11_dma_controller_destroy(&controller->controller);

			return NULL;
		}
	}

	controller->irq = irq;

	return &controller->controller;
}

#define MUSB_HSDMA_REAL_COUNT 0x80

#define USB_HSDMA_CHANNEL_OFFSET(_bchannel, _offset)		\
		(MUSB_HSDMA_BASE + (_bchannel << 4) + _offset)

#define usb_read_hsdma_addr(mbase, bchannel)	\
	musb11_readl(mbase,	\
		   USB_HSDMA_CHANNEL_OFFSET(bchannel, MUSB_HSDMA_ADDRESS))

#define usb_read_hsdma_ctrl(mbase, bchannel)	\
	musb11_readb(mbase,	\
		   MUSB_HSDMA_CHANNEL_OFFSET(bchannel, MUSB_HSDMA_CONTROL))

#define usb_read_hsdma_count(mbase, bchannel)	\
	musb11_readl(mbase,	\
		   MUSB_HSDMA_CHANNEL_OFFSET(bchannel, MUSB_HSDMA_COUNT))

#define usb_read_hsdma_real_count(mbase, bchannel)	\
	musb11_readl(mbase,	\
		   MUSB_HSDMA_CHANNEL_OFFSET(bchannel, MUSB_HSDMA_REAL_COUNT))

u8 USB11_DMA_status(u8 *pbDMAen, u8 *pbDMAdir)
{
	void __iomem *base = (void *)USB_BASE;
	u8 bchannel;
	u8 bDMAen = 0;
	u8 bDMAdir = 0;


	for (bchannel = 0; bchannel < MUSB_HSDMA_CHANNELS; bchannel++) {
		bDMAen |= (usb_read_hsdma_ctrl(base, bchannel) & 0x01) << bchannel;
		bDMAdir |= ((usb_read_hsdma_ctrl(base, bchannel) & 0x02) >> 1) << bchannel;
	}
	if (pbDMAen)
		*pbDMAen = bDMAen;
	if (pbDMAdir)
		*pbDMAdir = bDMAdir;
	if (bDMAen > 0)
		return 1;
	else
		return 0;
}
EXPORT_SYMBOL(USB11_DMA_status);


u32 USB11_DMA_address(u32 *len, u8 bchannel)
{
	void __iomem *base = (void *)USB_BASE;
	u32 ret = 0;
	if (len) {
		*len =
		    usb_read_hsdma_count(base, bchannel) + usb_read_hsdma_real_count(base,
										     bchannel);
	}
	ret = usb_read_hsdma_addr(base, bchannel) - usb_read_hsdma_real_count(base, bchannel);
	return ret;
}
EXPORT_SYMBOL(USB11_DMA_address);
