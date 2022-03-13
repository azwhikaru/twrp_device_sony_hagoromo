/*
 * MUSB OTG driver register I/O
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

#ifndef __MUSB11_LINUX_PLATFORM_ARCH_H__
#define __MUSB11_LINUX_PLATFORM_ARCH_H__

#include <linux/io.h>
#include <linux/spinlock.h>

extern bool mtk_usb11_power;
extern void usb11_enable_clock(bool enable);
extern spinlock_t usb11_io_lock;

static inline u16 musb11_readw(const void __iomem *addr, unsigned offset)
{
	u16 rc = 0;

	if (mtk_usb11_power) {
		rc = readw(addr + offset);
	} else {
		unsigned long flags = 0;
		spin_lock_irqsave(&usb11_io_lock, flags);
		usb11_enable_clock(true);
		DBG(0, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		rc = readw(addr + offset);
		usb11_enable_clock(false);
		spin_unlock_irqrestore(&usb11_io_lock, flags);
	}
	return rc;
}

static inline u32 musb11_readl(const void __iomem *addr, unsigned offset)
{
	u32 rc = 0;

	if (mtk_usb11_power) {
		rc = readl(addr + offset);
	} else {
		unsigned long flags = 0;
		spin_lock_irqsave(&usb11_io_lock, flags);
		usb11_enable_clock(true);
		DBG(0, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		rc = readl(addr + offset);
		usb11_enable_clock(false);
		spin_unlock_irqrestore(&usb11_io_lock, flags);
	}
	return rc;
}


static inline void musb11_writew(void __iomem *addr, unsigned offset, u16 data)
{
	if (mtk_usb11_power) {
		writew(data, addr + offset);
	} else {
		unsigned long flags = 0;
		spin_lock_irqsave(&usb11_io_lock, flags);
		usb11_enable_clock(true);
		DBG(0, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		writew(data, addr + offset);
		usb11_enable_clock(false);
		spin_unlock_irqrestore(&usb11_io_lock, flags);
	}
}

static inline void musb11_writel(void __iomem *addr, unsigned offset, u32 data)
{
	if (mtk_usb11_power) {
		writel(data, addr + offset);
	} else {
		unsigned long flags = 0;
		spin_lock_irqsave(&usb11_io_lock, flags);
		usb11_enable_clock(true);
		DBG(0, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		writel(data, addr + offset);
		usb11_enable_clock(false);
		spin_unlock_irqrestore(&usb11_io_lock, flags);
	}
}

static inline u8 musb11_readb(const void __iomem *addr, unsigned offset)
{
	u8 rc = 0;

	if (mtk_usb11_power) {
		rc = readb(addr + offset);
	} else {
		unsigned long flags = 0;
		spin_lock_irqsave(&usb11_io_lock, flags);
		usb11_enable_clock(true);
		DBG(0, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		rc = readb(addr + offset);
		usb11_enable_clock(false);
		spin_unlock_irqrestore(&usb11_io_lock, flags);
	}
	return rc;
}

static inline void musb11_writeb(void __iomem *addr, unsigned offset, u8 data)
{
	if (mtk_usb11_power) {
		writeb(data, addr + offset);
	} else {
		unsigned long flags = 0;
		spin_lock_irqsave(&usb11_io_lock, flags);
		usb11_enable_clock(true);
		DBG(0, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		writeb(data, addr + offset);
		usb11_enable_clock(false);
		spin_unlock_irqrestore(&usb11_io_lock, flags);
	}
}


#endif
