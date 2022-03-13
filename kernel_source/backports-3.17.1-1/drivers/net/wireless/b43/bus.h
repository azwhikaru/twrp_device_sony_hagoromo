/*
* Copyright (C) 2011-2015 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef B43_BUS_H_
#define B43_BUS_H_

enum b43_bus_type {
#ifdef CPTCFG_B43_BCMA
	B43_BUS_BCMA,
#endif
#ifdef CPTCFG_B43_SSB
	B43_BUS_SSB,
#endif
};

struct b43_bus_dev {
	enum b43_bus_type bus_type;
	union {
		struct bcma_device *bdev;
		struct ssb_device *sdev;
	};

	int (*bus_may_powerdown)(struct b43_bus_dev *dev);
	int (*bus_powerup)(struct b43_bus_dev *dev, bool dynamic_pctl);
	int (*device_is_enabled)(struct b43_bus_dev *dev);
	void (*device_enable)(struct b43_bus_dev *dev,
			      u32 core_specific_flags);
	void (*device_disable)(struct b43_bus_dev *dev,
			       u32 core_specific_flags);

	u16 (*read16)(struct b43_bus_dev *dev, u16 offset);
	u32 (*read32)(struct b43_bus_dev *dev, u16 offset);
	void (*write16)(struct b43_bus_dev *dev, u16 offset, u16 value);
	void (*write32)(struct b43_bus_dev *dev, u16 offset, u32 value);
	void (*block_read)(struct b43_bus_dev *dev, void *buffer,
			   size_t count, u16 offset, u8 reg_width);
	void (*block_write)(struct b43_bus_dev *dev, const void *buffer,
			    size_t count, u16 offset, u8 reg_width);

	struct device *dev;
	struct device *dma_dev;
	unsigned int irq;

	u16 board_vendor;
	u16 board_type;
	u16 board_rev;

	u16 chip_id;
	u8 chip_rev;
	u8 chip_pkg;

	struct ssb_sprom *bus_sprom;

	u16 core_id;
	u8 core_rev;
};

static inline bool b43_bus_host_is_pcmcia(struct b43_bus_dev *dev)
{
#ifdef CPTCFG_B43_SSB
	return (dev->bus_type == B43_BUS_SSB &&
		dev->sdev->bus->bustype == SSB_BUSTYPE_PCMCIA);
#else
	return false;
#endif
}
static inline bool b43_bus_host_is_sdio(struct b43_bus_dev *dev)
{
#ifdef CPTCFG_B43_SSB
	return (dev->bus_type == B43_BUS_SSB &&
		dev->sdev->bus->bustype == SSB_BUSTYPE_SDIO);
#else
	return false;
#endif
}

struct b43_bus_dev *b43_bus_dev_bcma_init(struct bcma_device *core);
struct b43_bus_dev *b43_bus_dev_ssb_init(struct ssb_device *sdev);

void *b43_bus_get_wldev(struct b43_bus_dev *dev);
void b43_bus_set_wldev(struct b43_bus_dev *dev, void *data);

#endif /* B43_BUS_H_ */
