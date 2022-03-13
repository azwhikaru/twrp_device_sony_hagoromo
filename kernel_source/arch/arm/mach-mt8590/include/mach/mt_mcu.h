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
#ifndef __MCU_H__
#define __MCU_H__

/*
 * Define hardware registers.
 */
#define MCU_BIU_CON             (MCU_BIU_BASE + 0x0)
#define MCU_BIU_PMCR            (MCU_BIU_BASE + 0x14)
#define MCU_BIU_CCR             (MCU_BIU_BASE + 0x40)
#define MCU_BIU_CCR_CON         (MCU_BIU_BASE + 0x44)
#define MCU_BIU_CCR_OVFL        (MCU_BIU_BASE + 0x48)
#define MCU_BIU_EVENT0_SEL      (MCU_BIU_BASE + 0x50)
#define MCU_BIU_EVENT0_CNT      (MCU_BIU_BASE + 0x54)
#define MCU_BIU_EVENT0_CON      (MCU_BIU_BASE + 0x58)
#define MCU_BIU_EVENT0_OVFL     (MCU_BIU_BASE + 0x5C)
#define MCU_BIU_EVENT1_SEL      (MCU_BIU_BASE + 0x60)
#define MCU_BIU_EVENT1_CNT      (MCU_BIU_BASE + 0x64)
#define MCU_BIU_EVENT1_CON      (MCU_BIU_BASE + 0x68)
#define MCU_BIU_EVENT1_OVFL     (MCU_BIU_BASE + 0x6C)

#define MCUSYS_CA7_CACHE_CONFIG         (MCUSYS_CFGREG_BASE + 0x000)
#define MCUSYS_CPU0_MEM_DELSEL          (MCUSYS_CFGREG_BASE + 0x004)
#define MCUSYS_CPU1_MEM_DELSEL          (MCUSYS_CFGREG_BASE + 0x008)
#define MCUSYS_CPU2_MEM_DELSEL          (MCUSYS_CFGREG_BASE + 0x00C)
#define MCUSYS_CPU3_MEM_DELSEL          (MCUSYS_CFGREG_BASE + 0x010)
#define MCUSYS_CACHE_MEM_DELSEL         (MCUSYS_CFGREG_BASE + 0x014)
#define MCUSYS_AXI_CONFIG               (MCUSYS_CFGREG_BASE + 0x020)
#define MCUSYS_MISC_CONFIG0             (MCUSYS_CFGREG_BASE + 0x024)
#define MCUSYS_MISC_CONFIG1             (MCUSYS_CFGREG_BASE + 0x028)
#define MCUSYS_CA7_CFG_DIS              (MCUSYS_CFGREG_BASE + 0x050)
#define MCUSYS_CA7_CLKEN_CTRL           (MCUSYS_CFGREG_BASE + 0x054)
#define MCUSYS_CA7_RST_CTRL             (MCUSYS_CFGREG_BASE + 0x058)
#define MCUSYS_CA7_MISC_CONFIG          (MCUSYS_CFGREG_BASE + 0x05C)
#define MCUSYS_ACLKEN_DIV               (MCUSYS_CFGREG_BASE + 0x060)
#define MCUSYS_PCLKEN_DIV               (MCUSYS_CFGREG_BASE + 0x064)
#define MCUSYS_MEM_PWR_CTRL             (MCUSYS_CFGREG_BASE + 0x068)
#define MCUSYS_ARMPLL_DIV_CTRL          (MCUSYS_CFGREG_BASE + 0x06C)
#define MCUSYS_RST_STATUS               (MCUSYS_CFGREG_BASE + 0x070)
#define MCUSYS_DBG_CTRL                 (MCUSYS_CFGREG_BASE + 0x080)
#define MCUSYS_DBG_FLAG                 (MCUSYS_CFGREG_BASE + 0x084)
#define MCUSYS_AP_BANK4_MAP_UPDATE      (MCUSYS_CFGREG_BASE + 0x090)
#define MCUSYS_RW_RSVD1                 (MCUSYS_CFGREG_BASE + 0x094)
#define MCUSYS_RO_RSVD                  (MCUSYS_CFGREG_BASE + 0x098)
#define MCUSYS_INT_POL_CTL0             (MCUSYS_CFGREG_BASE + 0x100)
#define MCUSYS_INT_POL_CTL1             (MCUSYS_CFGREG_BASE + 0x104)
#define MCUSYS_INT_POL_CTL2             (MCUSYS_CFGREG_BASE + 0x108)
#define MCUSYS_INT_POL_CTL3             (MCUSYS_CFGREG_BASE + 0x10C)
#define MCUSYS_INT_POL_CTL4             (MCUSYS_CFGREG_BASE + 0x110)
#define MCUSYS_INT_POL_CTL5             (MCUSYS_CFGREG_BASE + 0x114)
#define MCUSYS_INT_POL_CTL6             (MCUSYS_CFGREG_BASE + 0x118)
#define MCUSYS_AP_BANK4_MAP0            (MCUSYS_CFGREG_BASE + 0x200)
#define MCUSYS_AP_BANK4_MAP1            (MCUSYS_CFGREG_BASE + 0x204)
#define MCUSYS_BUS_SYNC_SEL             (MCUSYS_CFGREG_BASE + 0x208)
#define MCUSYS_CA7_IR_MON               (MCUSYS_CFGREG_BASE + 0x20C)
#define MCUSYS_DBG_CORE0_PC             (MCUSYS_CFGREG_BASE + 0x300)
#define MCUSYS_DBG_CORE0_FP             (MCUSYS_CFGREG_BASE + 0x304)
#define MCUSYS_DBG_CORE0_SP             (MCUSYS_CFGREG_BASE + 0x308)
#define MCUSYS_DBG_CORE1_PC             (MCUSYS_CFGREG_BASE + 0x310)
#define MCUSYS_DBG_CORE1_FP             (MCUSYS_CFGREG_BASE + 0x314)
#define MCUSYS_DBG_CORE1_SP             (MCUSYS_CFGREG_BASE + 0x318)
#define MCUSYS_DBG_CORE2_PC             (MCUSYS_CFGREG_BASE + 0x320)
#define MCUSYS_DBG_CORE2_FP             (MCUSYS_CFGREG_BASE + 0x324)
#define MCUSYS_DBG_CORE2_SP             (MCUSYS_CFGREG_BASE + 0x328)
#define MCUSYS_DBG_CORE3_PC             (MCUSYS_CFGREG_BASE + 0x330)
#define MCUSYS_DBG_CORE3_FP             (MCUSYS_CFGREG_BASE + 0x334)
#define MCUSYS_DBG_CORE3_SP             (MCUSYS_CFGREG_BASE + 0x338)
#define MCUSYS_DFD_CTRL                 (MCUSYS_CFGREG_BASE + 0x400)
#define MCUSYS_DFD_CNT_L                (MCUSYS_CFGREG_BASE + 0x404)
#define MCUSYS_DFD_CNT_H                (MCUSYS_CFGREG_BASE + 0x408)

/*
 * Define constants.
 */


/*
 * Define function prototypes.
 */
#endif  /*!__MCU_H__ */
