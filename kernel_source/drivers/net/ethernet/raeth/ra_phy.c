/*
 * Copyright 2016 Sony Corporation
 * File Changed on 2016-10-17
 */
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
#include <linux/kernel.h>
#include <linux/types.h>
#include <mach/mt_boot.h>
#include "raether.h"
#include "ra_phy.h"

void dump_phy_reg(int port_no, int from, int to, 
		int is_local, int page_no)
{
    u32 i=0;
    u32 temp=0;
    u32 r31=0;

    if(is_local==0) {

        printk("\n\nGlobal Register Page %d\n",page_no);
        printk("===============");
        r31 |= 0 << 15; //global
        r31 |= ((page_no&0x7) << 12); //page no
        mii_mgr_write(port_no, 31, r31); //select global page x
        for(i=16;i<32;i++) {
            if(i%8==0) {
                printk("\n");
            }
            mii_mgr_read(port_no,i, &temp);
            printk("%02d: %04X ",i, temp);
        }
    }else {
        printk("\n\nLocal Register Port %d Page %d\n",port_no, page_no);
        printk("===============");
        r31 |= 1 << 15; //local
        r31 |= ((page_no&0x7) << 12); //page no
        mii_mgr_write(port_no, 31, r31); //select local page x
        for(i=16;i<32;i++) {
            if(i%8==0) {
                printk("\n");
            }
            mii_mgr_read(port_no,i, &temp);
            printk("%02d: %04X ",i, temp);
        }
    }
    printk("\n");
}
static void default_phy_suspend(END_DEVICE *ei_local)
{
}
static void default_phy_resume(END_DEVICE *ei_local)
{
}

static void smsc8710a_phy_init(END_DEVICE *ei_local)
{

}

static enum mii_mode smsc8710a_phy_get_mii_type(END_DEVICE *ei_local)
{
    u32    val;
	mii_mgr_read(ei_local->phy_ops->addr, 18, &val);

	if(val & (1 << 14))
		return RMII;
	else
		return MII;
}

static void dm9162_phy_init(END_DEVICE *ei_local)
{

}

static enum mii_mode dm9162_phy_get_mii_type(END_DEVICE *ei_local)
{
    u32    val;
	mii_mgr_read(ei_local->phy_ops->addr, 24, &val);

	if(val & (1 << 11))
		return RMII;
	else
		return MII;
}

static void rtl8201_phy_init(END_DEVICE *ei_local)
{
	CHIP_SW_VER ver = mt_get_chip_sw_ver();
	u16 rmsr = 0;
	
	mii_mgr_write(ei_local->phy_ops->addr, 31, 7);//set to page7
	printk("chip ver (%d).\n", ver);
	rmsr = (ver >= CHIP_SW_VER_02 ) ? 0x1FFA : 0x1F3A;
	mii_mgr_write(ei_local->phy_ops->addr, 16, rmsr);//default is 1FFA
	mii_mgr_write(ei_local->phy_ops->addr, 31, 0);//set to page0
#ifdef REALTEK_ETH_CTL
	eee_contrl(0); /* enable:1 disable:0 */
#endif /* REALTEK_ETH_CTL */
}

static enum mii_mode rtl8201_phy_get_mii_type(END_DEVICE *ei_local)
{
		return RMII;
}
static void rtl8201_phy_suspend(END_DEVICE *ei_local)
{
	u32 phyAddr = ei_local->phy_ops->addr;
	struct sockaddr sa;
	char *macAddr = sa.sa_data;
	struct net_device *dev = ei_local->netdev;
	u32 val = 0;

	memcpy(sa.sa_data, dev->dev_addr, dev->addr_len);

	/* enable phy wol */

	mii_mgr_write(phyAddr, 4, 0x61);/* set speed at 10M */
	mii_mgr_write(phyAddr, 0, 0x3200);

	/* set mac address */
	mii_mgr_write(phyAddr, 31, 0x12);
	mii_mgr_write(phyAddr, 16, (macAddr[1] << 8) | (macAddr[0] << 0));
	mii_mgr_write(phyAddr, 17, (macAddr[3] << 8) | (macAddr[2] << 0));
	mii_mgr_write(phyAddr, 18, (macAddr[5] << 8) | (macAddr[4] << 0));
	printk("mac address:%x %x %x %x %x %x.\n",
		macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

	/* set max length */
	mii_mgr_write(phyAddr, 31, 0x11);
	mii_mgr_write(phyAddr, 17, 0x1FFF);

	/* enable magic packet event */
	mii_mgr_write(phyAddr, 16, 0x1000);

	/* set tx isolate */
	mii_mgr_write(phyAddr, 31, 0x7);
	mii_mgr_read(phyAddr, 20, &val);
	mii_mgr_write(phyAddr, 20, val | (1 << 15));

	/* set rx isolate */
	mii_mgr_write(phyAddr, 31, 0x17);
	mii_mgr_read(phyAddr, 19, &val);
	mii_mgr_write(phyAddr, 19, val | (1 << 15));

	mii_mgr_write(phyAddr, 31, 0);/* return page 0 */
}
static void rtl8201_phy_resume(END_DEVICE *ei_local)
{
	u32 phyAddr = ei_local->phy_ops->addr;
	u32 val = 0;

	/* unset rx isolate */
	mii_mgr_write(phyAddr, 31, 0x17);
	mii_mgr_read(phyAddr, 19, &val);
	mii_mgr_write(phyAddr, 19, val & (~(1 << 15)));

	/* unset tx isolate */
	mii_mgr_write(phyAddr, 31, 0x7);
	mii_mgr_read(phyAddr, 20, &val);
	mii_mgr_write(phyAddr, 20, val & (~(1 << 15)));

	mii_mgr_write(phyAddr, 31, 0x11);
	/* disable magic packet event */
	mii_mgr_write(phyAddr, 16, 0x0);

	/* unset max length and reset PMEB pin as high */
	mii_mgr_write(phyAddr, 17, 0x9FFF);

	mii_mgr_write(phyAddr, 31, 0);/* return page 0 */
}

static struct eth_phy_ops smsc8710a_phy_ops = {
    .phy_id = PHYID2_SMSC8710A,
    .init = smsc8710a_phy_init,
    .suspend = default_phy_suspend,
    .resume = default_phy_resume,
    .get_mii_type = smsc8710a_phy_get_mii_type,
};

static struct eth_phy_ops dm9162_phy_ops = {
    .phy_id = PHYID2_DM9162_XMII,
    .init = dm9162_phy_init,
    .suspend = default_phy_suspend,
    .resume = default_phy_resume,
	.get_mii_type = dm9162_phy_get_mii_type,
};

static struct eth_phy_ops rtl8201_phy_ops = {
    .phy_id = PHYID2_RTL8201FR,
    .init = rtl8201_phy_init,
    .suspend = rtl8201_phy_suspend,
	.resume = rtl8201_phy_resume,
	.get_mii_type = rtl8201_phy_get_mii_type,
};

int ra_detect_phy(END_DEVICE *ei_local)
{
    int     addr;
    u32     reg2;

    for (addr = 0; addr < 0x1f; addr++) {
		mii_mgr_read(addr, PHY_REG_IDENTFIR2, &reg2);
        printk("%s(%d) id=%d, vendor=0x%x\n", __func__, __LINE__, addr, reg2);

        if (reg2 == PHYID2_SMSC8710A) {
            printk("Ethernet: SMSC8710A PHY\n\r");
			ei_local->phy_ops = &smsc8710a_phy_ops;
            break;
        } else if (reg2 == PHYID2_DM9162_XMII) {
            printk("Ethernet: DM9162 PHY\n\r");
			ei_local->phy_ops = &dm9162_phy_ops;
            break;
        } else if (reg2 == PHYID2_RTL8201FR) {
            printk("Ethernet: Realtek PHY\n\r");
			ei_local->phy_ops = &rtl8201_phy_ops;
            break;
        }
    }
	if (addr == 0x1f) {
		printk("No Ethernet PHY!!!\n\r");
		ei_local->phy_ops = NULL;
		return -1;
	} else {
		ei_local->phy_ops->addr = addr;
	}
    return ei_local->phy_ops->addr;
}


