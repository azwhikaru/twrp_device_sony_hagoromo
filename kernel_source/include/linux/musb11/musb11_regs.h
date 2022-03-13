/*
 * MUSB OTG driver register defines
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

#ifndef __MUSB11_REGS_H__
#define __MUSB11_REGS_H__

#define MUSB11_EP0_FIFOSIZE	64	/* This is non-configurable */

/*
 * MUSB Register bits
 */

/* POWER */
#define MUSB11_POWER_ISOUPDATE	0x80
#define MUSB11_POWER_SOFTCONN	0x40
#define MUSB11_POWER_HSENAB	0x20
#define MUSB11_POWER_HSMODE	0x10
#define MUSB11_POWER_RESET	0x08
#define MUSB11_POWER_RESUME	0x04
#define MUSB11_POWER_SUSPENDM	0x02
#define MUSB11_POWER_ENSUSPEND	0x01

/* INTRUSB */
#define MUSB11_INTR_SUSPEND	0x01
#define MUSB11_INTR_RESUME	0x02
#define MUSB11_INTR_RESET		0x04
#define MUSB11_INTR_BABBLE	0x04
#define MUSB11_INTR_SOF		0x08
#define MUSB11_INTR_CONNECT	0x10
#define MUSB11_INTR_DISCONNECT	0x20
#define MUSB11_INTR_SESSREQ	0x40
#define MUSB11_INTR_VBUSERROR	0x80	/* For SESSION end */

/* DEVCTL */
#define MUSB11_DEVCTL_BDEVICE	0x80
#define MUSB11_DEVCTL_FSDEV	0x40
#define MUSB11_DEVCTL_LSDEV	0x20
#define MUSB11_DEVCTL_VBUS	0x18
#define MUSB11_DEVCTL_VBUS_SHIFT	3
#define MUSB11_DEVCTL_HM		0x04
#define MUSB11_DEVCTL_HR		0x02
#define MUSB11_DEVCTL_SESSION	0x01

/* MUSB ULPI VBUSCONTROL */
#define MUSB11_ULPI_USE_EXTVBUS	0x01
#define MUSB11_ULPI_USE_EXTVBUSIND 0x02
/* ULPI_REG_CONTROL */
#define MUSB11_ULPI_REG_REQ	(1 << 0)
#define MUSB11_ULPI_REG_CMPLT	(1 << 1)
#define MUSB11_ULPI_RDN_WR	(1 << 2)

/* TESTMODE */
#define MUSB11_TEST_FORCE_HOST	0x80
#define MUSB11_TEST_FIFO_ACCESS	0x40
#define MUSB11_TEST_FORCE_FS	0x20
#define MUSB11_TEST_FORCE_HS	0x10
#define MUSB11_TEST_PACKET	0x08
#define MUSB11_TEST_K		0x04
#define MUSB11_TEST_J		0x02
#define MUSB11_TEST_SE0_NAK	0x01

/* Allocate for double-packet buffering (effectively doubles assigned _SIZE) */
#define MUSB11_FIFOSZ_DPB	0x10
/* Allocation size (8, 16, 32, ... 4096) */
#define MUSB11_FIFOSZ_SIZE	0x0f

/* CSR0 */
#define MUSB11_CSR0_FLUSHFIFO	0x0100
#define MUSB11_CSR0_TXPKTRDY	0x0002
#define MUSB11_CSR0_RXPKTRDY	0x0001

/* CSR0 in Peripheral mode */
#define MUSB11_CSR0_P_SVDSETUPEND	0x0080
#define MUSB11_CSR0_P_SVDRXPKTRDY	0x0040
#define MUSB11_CSR0_P_SENDSTALL	0x0020
#define MUSB11_CSR0_P_SETUPEND	0x0010
#define MUSB11_CSR0_P_DATAEND	0x0008
#define MUSB11_CSR0_P_SENTSTALL	0x0004

/* CSR0 in Host mode */
#define MUSB11_CSR0_H_DIS_PING		0x0800
#define MUSB11_CSR0_H_WR_DATATOGGLE	0x0400	/* Set to allow setting: */
#define MUSB11_CSR0_H_DATATOGGLE		0x0200	/* Data toggle control */
#define MUSB11_CSR0_H_NAKTIMEOUT		0x0080
#define MUSB11_CSR0_H_STATUSPKT		0x0040
#define MUSB11_CSR0_H_REQPKT		0x0020
#define MUSB11_CSR0_H_ERROR		0x0010
#define MUSB11_CSR0_H_SETUPPKT		0x0008
#define MUSB11_CSR0_H_RXSTALL		0x0004

/* CSR0 bits to avoid zeroing (write zero clears, write 1 ignored) */
#define MUSB11_CSR0_P_WZC_BITS	\
	(MUSB11_CSR0_P_SENTSTALL)
#define MUSB11_CSR0_H_WZC_BITS	\
	(MUSB11_CSR0_H_NAKTIMEOUT | MUSB11_CSR0_H_RXSTALL \
	| MUSB11_CSR0_RXPKTRDY)

/* TxType/RxType */
#define MUSB11_TYPE_SPEED		0xc0
#define MUSB11_TYPE_SPEED_SHIFT	6
#define MUSB11_TYPE_PROTO		0x30	/* Implicitly zero for ep0 */
#define MUSB11_TYPE_PROTO_SHIFT	4
#define MUSB11_TYPE_REMOTE_END	0xf	/* Implicitly zero for ep0 */

/* CONFIGDATA */
#define MUSB11_CONFIGDATA_MPRXE		0x80	/* Auto bulk pkt combining */
#define MUSB11_CONFIGDATA_MPTXE		0x40	/* Auto bulk pkt splitting */
#define MUSB11_CONFIGDATA_BIGENDIAN	0x20
#define MUSB11_CONFIGDATA_HBRXE		0x10	/* HB-ISO for RX */
#define MUSB11_CONFIGDATA_HBTXE		0x08	/* HB-ISO for TX */
#define MUSB11_CONFIGDATA_DYNFIFO		0x04	/* Dynamic FIFO sizing */
#define MUSB11_CONFIGDATA_SOFTCONE	0x02	/* SoftConnect */
#define MUSB11_CONFIGDATA_UTMIDW		0x01	/* Data width 0/1 => 8/16bits */

/* TXCSR in Peripheral and Host mode */
#define MUSB11_TXCSR_AUTOSET		0x8000
#define MUSB11_TXCSR_DMAENAB		0x1000
#define MUSB11_TXCSR_FRCDATATOG		0x0800
#define MUSB11_TXCSR_DMAMODE		0x0400
#define MUSB11_TXCSR_CLRDATATOG		0x0040
#define MUSB11_TXCSR_FLUSHFIFO		0x0008
#define MUSB11_TXCSR_FIFONOTEMPTY		0x0002
#define MUSB11_TXCSR_TXPKTRDY		0x0001

/* TXCSR in Peripheral mode */
#define MUSB11_TXCSR_P_ISO		0x4000
#define MUSB11_TXCSR_P_INCOMPTX		0x0080
#define MUSB11_TXCSR_P_SENTSTALL		0x0020
#define MUSB11_TXCSR_P_SENDSTALL		0x0010
#define MUSB11_TXCSR_P_UNDERRUN		0x0004

/* TXCSR in Host mode */
#define MUSB11_TXCSR_H_WR_DATATOGGLE	0x0200
#define MUSB11_TXCSR_H_DATATOGGLE		0x0100
#define MUSB11_TXCSR_H_NAKTIMEOUT		0x0080
#define MUSB11_TXCSR_H_RXSTALL		0x0020
#define MUSB11_TXCSR_H_ERROR		0x0004

/* TXCSR bits to avoid zeroing (write zero clears, write 1 ignored) */
#define MUSB11_TXCSR_P_WZC_BITS	\
	(MUSB11_TXCSR_P_INCOMPTX | MUSB11_TXCSR_P_SENTSTALL \
	| MUSB11_TXCSR_P_UNDERRUN | MUSB11_TXCSR_FIFONOTEMPTY)
#define MUSB11_TXCSR_H_WZC_BITS	\
	(MUSB11_TXCSR_H_NAKTIMEOUT | MUSB11_TXCSR_H_RXSTALL \
	| MUSB11_TXCSR_H_ERROR | MUSB11_TXCSR_FIFONOTEMPTY)
/* RXCSR in Peripheral and Host mode */
#define MUSB11_RXCSR_AUTOCLEAR		0x8000
#define MUSB11_RXCSR_DMAENAB		0x2000
#define MUSB11_RXCSR_DISNYET		0x1000
#define MUSB11_RXCSR_PID_ERR		0x1000
#define MUSB11_RXCSR_DMAMODE		0x0800
#define MUSB11_RXCSR_INCOMPRX		0x0100
#define MUSB11_RXCSR_CLRDATATOG		0x0080
#define MUSB11_RXCSR_FLUSHFIFO		0x0010
#define MUSB11_RXCSR_DATAERROR		0x0008
#define MUSB11_RXCSR_FIFOFULL		0x0002
#define MUSB11_RXCSR_RXPKTRDY		0x0001

/* ALPS00798316, Enable DMA RxMode1 */
#define MUSB11_EP_RXPKTCOUNT		0x0300
/* ALPS00798316, Enable DMA RxMode1 */

/* RXCSR in Peripheral mode */
#define MUSB11_RXCSR_P_ISO		0x4000
#define MUSB11_RXCSR_P_SENTSTALL		0x0040
#define MUSB11_RXCSR_P_SENDSTALL		0x0020
#define MUSB11_RXCSR_P_OVERRUN		0x0004

/* RXCSR in Host mode */
#define MUSB11_RXCSR_H_AUTOREQ		0x4000
#define MUSB11_RXCSR_H_WR_DATATOGGLE	0x0400
#define MUSB11_RXCSR_H_DATATOGGLE		0x0200
#define MUSB11_RXCSR_H_RXSTALL		0x0040
#define MUSB11_RXCSR_H_REQPKT		0x0020
#define MUSB11_RXCSR_H_ERROR		0x0004

/* RXCSR bits to avoid zeroing (write zero clears, write 1 ignored) */
#define MUSB11_RXCSR_P_WZC_BITS	\
	(MUSB11_RXCSR_P_SENTSTALL | MUSB11_RXCSR_P_OVERRUN \
	| MUSB11_RXCSR_RXPKTRDY)
#define MUSB11_RXCSR_H_WZC_BITS	\
	(MUSB11_RXCSR_H_RXSTALL | MUSB11_RXCSR_H_ERROR \
	| MUSB11_RXCSR_DATAERROR | MUSB11_RXCSR_RXPKTRDY)

/* HUBADDR */
#define MUSB11_HUBADDR_MULTI_TT		0x80

/*
 * Common USB registers
 */

#define MUSB11_FADDR		0x00	/* 8-bit */
#define MUSB11_POWER		0x01	/* 8-bit */

#define MUSB11_INTRTX		0x02	/* 16-bit */
#define MUSB11_INTRRX		0x04
#define MUSB11_INTRTXE		0x06
#define MUSB11_INTRRXE		0x08
#define MUSB11_INTRUSB		0x0A	/* 8 bit */
#define MUSB11_INTRUSBE		0x0B	/* 8 bit */
#define MUSB11_FRAME		0x0C
#define MUSB11_INDEX		0x0E	/* 8 bit */
#define MUSB11_TESTMODE		0x0F	/* 8 bit */

#define MUSB11_FIFO_OFFSET(epnum)	(0x20 + ((epnum) * 4))

/*
 * Additional Control Registers
 */

#define MUSB11_DEVCTL		0x60	/* 8 bit */

#define MUSB11_OPSTATE    0x620
#define MUSB11_OTG_IDLE 0


/* These are always controlled through the INDEX register */
#define MUSB11_TXFIFOSZ		0x62	/* 8-bit (see masks) */
#define MUSB11_RXFIFOSZ		0x63	/* 8-bit (see masks) */
#define MUSB11_TXFIFOADD		0x64	/* 16-bit offset shifted right 3 */
#define MUSB11_RXFIFOADD		0x66	/* 16-bit offset shifted right 3 */

/* REVISIT: vctrl/vstatus: optional vendor utmi+phy register at 0x68 */
#define MUSB11_HWVERS		0x6C	/* 8 bit */
#define MUSB11_ULPI_BUSCONTROL	0x70	/* 8 bit */
#define MUSB11_ULPI_INT_MASK	0x72	/* 8 bit */
#define MUSB11_ULPI_INT_SRC	0x73	/* 8 bit */
#define MUSB11_ULPI_REG_DATA	0x74	/* 8 bit */
#define MUSB11_ULPI_REG_ADDR	0x75	/* 8 bit */
#define MUSB11_ULPI_REG_CONTROL	0x76	/* 8 bit */
#define MUSB11_ULPI_RAW_DATA	0x77	/* 8 bit */

#define MUSB11_EPINFO		0x78	/* 8 bit */
#define MUSB11_RAMINFO		0x79	/* 8 bit */
#define MUSB11_LINKINFO		0x7a	/* 8 bit */
#define MUSB11_VPLEN		0x7b	/* 8 bit */
#define MUSB11_HS_EOF1		0x7c	/* 8 bit */
#define MUSB11_FS_EOF1		0x7d	/* 8 bit */
#define MUSB11_LS_EOF1		0x7e	/* 8 bit */

/* Offsets to endpoint registers */
#define MUSB11_TXMAXP		0x00
#define MUSB11_TXCSR		0x02
#define MUSB11_CSR0		MUSB11_TXCSR	/* Re-used for EP0 */
#define MUSB11_RXMAXP		0x04
#define MUSB11_RXCSR		0x06
#define MUSB11_RXCOUNT		0x08
#define MUSB11_COUNT0		MUSB11_RXCOUNT	/* Re-used for EP0 */
#define MUSB11_TXTYPE		0x0A
#define MUSB11_TYPE0		MUSB11_TXTYPE	/* Re-used for EP0 */
#define MUSB11_TXINTERVAL		0x0B
#define MUSB11_NAKLIMIT0		MUSB11_TXINTERVAL	/* Re-used for EP0 */
#define MUSB11_RXTYPE		0x0C
#define MUSB11_RXINTERVAL		0x0D
#define MUSB11_FIFOSIZE		0x0F
#define MUSB11_CONFIGDATA		MUSB11_FIFOSIZE	/* Re-used for EP0 */

/* Offsets to endpoint registers in indexed model (using INDEX register) */
#define MUSB11_INDEXED_OFFSET(_epnum, _offset)	\
	(0x10 + (_offset))


#define MUSB11_TXCSR_MODE			0x2000

/* "bus control"/target registers, for host side multipoint (external hubs) */
#define MUSB11_TXFUNCADDR		0x0480
#define MUSB11_TXHUBADDR		0x0482

#define MUSB11_RXFUNCADDR		0x0484
#define MUSB11_RXHUBADDR		0x0486

/* Toggle registers */
#define MUSB11_RXTOG          0x0080
#define MUSB11_RXTOGEN        0x0082
#define MUSB11_TXTOG          0x0084
#define MUSB11_TXTOGEN        0x0086

#define MUSB11_BUSCTL_OFFSET(_epnum, _offset) \
	(0x80 + (8*(_epnum)) + (_offset))

/*
MTK Software reset reg
*/
#define MUSB11_SWRST 0x74
#define MUSB11_SWRST_PHY_RST         (1<<7)
#define MUSB11_SWRST_PHYSIG_GATE_HS  (1<<6)
#define MUSB11_SWRST_PHYSIG_GATE_EN  (1<<5)
#define MUSB11_SWRST_REDUCE_DLY      (1<<4)
#define MUSB11_SWRST_UNDO_SRPFIX     (1<<3)
#define MUSB11_SWRST_FRC_VBUSVALID   (1<<2)
#define MUSB11_SWRST_SWRST           (1<<1)
#define MUSB11_SWRST_DISUSBRESET     (1<<0)

#define MUSB11_L1INTS (0x00a0)	/* USB level 1 interrupt status register */
#define MUSB11_L1INTM (0x00a4)	/* USB level 1 interrupt mask register  */
#define MUSB11_L1INTP (0x00a8)	/* USB level 1 interrupt polarity register  */

#define MUSB11_DMA_INTR (USB1_BASE + 0x0200)
#define MUSB11_DMA_INTR_UNMASK_CLR_OFFSET (16)
#define MUSB11_DMA_INTR_UNMASK_SET_OFFSET (24)
#define MUSB11_USB_DMA_REALCOUNT(chan) (0x0280+0x10*(chan))


/* ====================== */
/* USB interrupt register */
/* ====================== */

/* word access */
#define MUSB11_TX_INT_STATUS        (1<<0)
#define MUSB11_TRX_INT_STATUS        (1<<1)
#define MUSB11_COM_INT_STATUS    (1<<2)
#define MUSB11_DMA_INT_STATUS       (1<<3)
#define MUSB11_PSR_INT_STATUS       (1<<4)
#define MUSB11_QINT_STATUS          (1<<5)
#define MUSB11_QHIF_INT_STATUS      (1<<6)
#define MUSB11_DPDM_INT_STATUS      (1<<7)
#define MUSB11_VBUSVALID_INT_STATUS (1<<8)
#define MUSB11_IDDIG_INT_STATUS     (1<<9)
#define MUSB11_DRVVBUS_INT_STATUS   (1<<10)

#define MUSB11_VBUSVALID_INT_POL    (1<<8)
#define MUSB11_IDDIG_INT_POL        (1<<9)
#define MUSB11_DRVVBUS_INT_POL      (1<<10)
/*
 * QMU Registers
 */
#ifdef MUSB11_QMU_SUPPORT
#define MUSB11_QMUBASE		(0x800)
#define MUSB11_QISAR		(0xc00)
#define MUSB11_QIMR			(0xc04)
#endif

static inline void musb11_write_txfifosz(void __iomem *mbase, u8 c_size)
{
	musb11_writeb(mbase, MUSB11_TXFIFOSZ, c_size);
}

static inline void musb11_write_txfifoadd(void __iomem *mbase, u16 c_off)
{
	musb11_writew(mbase, MUSB11_TXFIFOADD, c_off);
}

static inline void musb11_write_rxfifosz(void __iomem *mbase, u8 c_size)
{
	musb11_writeb(mbase, MUSB11_RXFIFOSZ, c_size);
}

static inline void musb11_write_rxfifoadd(void __iomem *mbase, u16 c_off)
{
	musb11_writew(mbase, MUSB11_RXFIFOADD, c_off);
}

static inline void musb11_write_ulpi_buscontrol(void __iomem *mbase, u8 val)
{
	musb11_writeb(mbase, MUSB11_ULPI_BUSCONTROL, val);
}

static inline u8 musb11_read_txfifosz(void __iomem *mbase)
{
	return musb11_readb(mbase, MUSB11_TXFIFOSZ);
}

static inline u16 musb11_read_txfifoadd(void __iomem *mbase)
{
	return musb11_readw(mbase, MUSB11_TXFIFOADD);
}

static inline u8 musb11_read_rxfifosz(void __iomem *mbase)
{
	return musb11_readb(mbase, MUSB11_RXFIFOSZ);
}

static inline u16 musb11_read_rxfifoadd(void __iomem *mbase)
{
	return musb11_readw(mbase, MUSB11_RXFIFOADD);
}

static inline u8 musb11_read_ulpi_buscontrol(void __iomem *mbase)
{
	return musb11_readb(mbase, MUSB11_ULPI_BUSCONTROL);
}

static inline u8 musb11_read_configdata(void __iomem *mbase)
{
	musb11_writeb(mbase, MUSB11_INDEX, 0);
	return musb11_readb(mbase, 0x10 + MUSB11_CONFIGDATA);
}

static inline u16 musb11_read_hwvers(void __iomem *mbase)
{
	return musb11_readw(mbase, MUSB11_HWVERS);
}

static inline void musb11_write_rxfunaddr(void __iomem *mbase, u8 epnum, u8 qh_addr_reg)
{
	musb11_writew(mbase, MUSB11_RXFUNCADDR + 8 * epnum, qh_addr_reg);
}

static inline void musb11_write_rxhubaddr(void __iomem *mbase, u8 epnum, u8 qh_h_addr_reg)
{
	u16 rx_hub_port_addr = musb11_readw(mbase, 0x0486 + 8 * epnum);
	rx_hub_port_addr &= 0xff00;
	rx_hub_port_addr |= qh_h_addr_reg;
	musb11_writew(mbase, MUSB11_RXHUBADDR + 8 * epnum, rx_hub_port_addr);
}

static inline void musb11_write_rxhubport(void __iomem *mbase, u8 epnum, u8 qh_h_port_reg)
{
	u16 rx_hub_port_addr = musb11_readw(mbase, 0x0486 + 8 * epnum);
	u16 rx_port_addr = (u16) qh_h_port_reg;
	rx_hub_port_addr &= 0x00ff;
	rx_hub_port_addr |= (rx_port_addr << 8);
	musb11_writew(mbase, MUSB11_RXHUBADDR + 8 * epnum, rx_hub_port_addr);
}

static inline void musb11_write_txfunaddr(void __iomem *mbase, u8 epnum, u8 qh_addr_reg)
{
	musb11_writew(mbase, MUSB11_TXFUNCADDR + 8 * epnum, qh_addr_reg);
}

static inline void musb11_write_txhubaddr(void __iomem *mbase, u8 epnum, u8 qh_h_addr_reg)
{
	u16 tx_hub_port_addr = musb11_readw(mbase, 0x0482 + 8 * epnum);
	tx_hub_port_addr &= 0xff00;
	tx_hub_port_addr |= qh_h_addr_reg;
	musb11_writew(mbase, MUSB11_TXHUBADDR + 8 * epnum, tx_hub_port_addr);
}

static inline void musb11_write_txhubport(void __iomem *mbase, u8 epnum, u8 qh_h_port_reg)
{
	u16 tx_hub_port_addr = musb11_readw(mbase, 0x0482 + 8 * epnum);
	u16 tx_port_addr = (u16) qh_h_port_reg;
	tx_hub_port_addr &= 0x00ff;
	tx_hub_port_addr |= (tx_port_addr << 8);
	musb11_writew(mbase, MUSB11_TXHUBADDR + 8 * epnum, tx_hub_port_addr);
}

#endif				/* __MUSB11_REGS_H__ */
