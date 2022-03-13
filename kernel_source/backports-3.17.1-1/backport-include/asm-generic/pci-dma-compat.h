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
#ifndef __BACKPORT_ASM_GENERIC_PCI_DMA_COMPAT_H
#define __BACKPORT_ASM_GENERIC_PCI_DMA_COMPAT_H
#include_next <asm-generic/pci-dma-compat.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0)
#define pci_zalloc_consistent LINUX_BACKPORT(pci_zalloc_consistent)
static inline void *pci_zalloc_consistent(struct pci_dev *hwdev, size_t size,
					  dma_addr_t *dma_handle)
{
	void *ret = pci_alloc_consistent(hwdev, size, dma_handle);
	if (ret)
		memset(ret, 0, size);
	return ret;
}
#endif

#endif /* __BACKPORT_ASM_GENERIC_PCI_DMA_COMPAT_H */
