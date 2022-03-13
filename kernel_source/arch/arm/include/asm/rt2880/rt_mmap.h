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
#define HIFSYS_BASE			0xFA000000 //for PCIe/USB
#define ETHDMASYS_BASE			0xFB000000 //for I2S/PCM/GDMA/HSDMA/FE/GMAC

#define HIFSYS_PCI_BASE                 0xFA140000
#define HIFSYS_USB_HOST_BASE            0xFA1C0000
#define HIFSYS_USB_HOST2_BASE           0xFA240000

#define ETHDMASYS_SYSCTL_BASE           0xFB000000
#define ETHDMASYS_RBUS_MATRIXCTL_BASE   0xFB000400
#define ETHDMASYS_I2S_BASE              0xFB000A00
#define ETHDMASYS_PCM_BASE              0xFB002000
#define ETHDMASYS_GDMA_BASE             0xFB002800
#define ETHDMASYS_HS_DMA_BASE           0xFB007000
#define ETHDMASYS_FRAME_ENGINE_BASE     0xFB100000
#define ETHDMASYS_PPE_BASE		0xFB100C00
#define ETHDMASYS_ETH_SW_BASE		0xFB110000
#define ETHDMASYS_CRYPTO_ENGINE_BASE	0xFB240000

//for backward-compatible
#define RALINK_FRAME_ENGINE_BASE	ETHDMASYS_FRAME_ENGINE_BASE
#define RALINK_PPE_BASE                 ETHDMASYS_PPE_BASE
#define RALINK_SYSCTL_BASE		ETHDMASYS_SYSCTL_BASE
#define RALINK_ETH_SW_BASE		ETHDMASYS_ETH_SW_BASE
#define RALINK_GDMA_BASE      ETHDMASYS_GDMA_BASE
#define RALINK_HS_DMA_BASE    ETHDMASYS_HS_DMA_BASE
#define RALINK_11N_MAC_BASE		0	//unused for rt_rdm usage

//Reset Control Register
#define RSTCTL_SYS_RST			(1<<0)
#define RSTCTL_MCM_RST			(1<<2)
#define RSTCTL_HSDMA_RST		(1<<5)
#define RSTCTL_FE_RST			(1<<6)
#define RSTCTL_SPDIF_RST		(1<<7)
#define RSTCTL_TIMER_RST		(1<<8)
#define RSTCTL_CIRQ_RST			(1<<9)
#define RSTCTL_MC_RST			(1<<10)
#define RSTCTL_PCM_RST			(1<<11)
#define RSTCTL_GPIO_RST			(1<<13)
#define RSTCTL_GDMA_RST			(1<<14)
#define RSTCTL_NAND_RST			(1<<15)
#define RSTCTL_I2C_RST			(1<<16)
#define RSTCTL_I2S_RST			(1<<17)
#define RSTCTL_SPI_RST			(1<<18)
#define RSTCTL_UART0_RST		(1<<19)
#define RSTCTL_UART1_RST		(1<<20)
#define RSTCTL_UART2_RST		(1<<21)
#define RSTCTL_UPHY_RST			(1<<22)
#define RSTCTL_ETH_RST			(1<<23)
#define RSTCTL_PCIE0_RST		(1<<24)
#define RSTCTL_PCIE1_RST		(1<<25)
#define RSTCTL_PCIE2_RST		(1<<26)
#define RSTCTL_AUX_STCK_RST		(1<<28)
#define RSTCTL_CRYPT_RST		(1<<29)
#define RSTCTL_SDXC_RST			(1<<30)
#define RSTCTL_PWM_RST			(1<<31)

//for backward-compatible
#define RALINK_FE_RST			RSTCTL_FE_RST