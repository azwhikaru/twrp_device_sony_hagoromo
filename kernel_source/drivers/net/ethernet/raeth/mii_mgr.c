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
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netdevice.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <asm/rt2880/rt_mmap.h>
#endif

#include "ra_reg.h"
#include "raether.h"

#define PHY_CONTROL_0 		0x0004   
#define MDIO_PHY_CONTROL_0	(RALINK_ETH_SW_BASE + PHY_CONTROL_0)
#define enable_mdio(x)

u32 __mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	u32 volatile status = 0;
	u32 rc = 0;
	unsigned long volatile t_start = jiffies;
	u32 volatile data = 0;

	/* We enable mdio gpio purpose register, and disable it when exit. */
	enable_mdio(1);

	// make sure previous read operation is complete
	while (1) {
			// 0 : Read/write operation complete
		if(!( sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) 
		{
			break;
		}
		else if (time_after(jiffies, t_start + 5*HZ)) {
			enable_mdio(0);
			printk("\n MDIO Read operation is ongoing !!\n");
			return rc;
		}
	}

	data  = (0x01 << 16) | (0x02 << 18) | (phy_addr << 20) | (phy_register << 25);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	//printk("\n Set Command [0x%08X] = [0x%08X] to PHY !!\n",MDIO_PHY_CONTROL_0, data);


	// make sure read operation is complete
	t_start = jiffies;
	while (1) {
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			status = sysRegRead(MDIO_PHY_CONTROL_0);
			*read_data = (u32)(status & 0x0000FFFF);

			enable_mdio(0);
			return 1;
		}
		else if (time_after(jiffies, t_start+5*HZ)) {
			enable_mdio(0);
			printk("\n MDIO Read operation is ongoing and Time Out!!\n");
			return 0;
		}
	}
}

u32 __mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	unsigned long volatile t_start=jiffies;
	u32 volatile data;

	enable_mdio(1);

	// make sure previous write operation is complete
	while(1) {
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) 
		{
			break;
		}
		else if (time_after(jiffies, t_start + 5 * HZ)) {
			enable_mdio(0);
			printk("\n MDIO Write operation ongoing\n");
			return 0;
		}
	}

	data = (0x01 << 16)| (1<<18) | (phy_addr << 20) | (phy_register << 25) | write_data;
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data); //start operation

	t_start = jiffies;

	// make sure write operation is complete
	while (1) {
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) //0 : Read/write operation complete
		{
			enable_mdio(0);
			return 1;
		}
		else if (time_after(jiffies, t_start + 5 * HZ)) {
			enable_mdio(0);
			printk("\n MDIO Write operation Time Out\n");
			return 0;
		}
	}
}

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
#if defined (CONFIG_GE1_RGMII_FORCE_1000) || defined (CONFIG_GE1_TRGMII_FORCE_1200) || defined (CONFIG_P5_RGMII_TO_MT7530_MODE)
        u32 low_word;
        u32 high_word;
        u32 an_status = 0;
        
	if(phy_addr==31) 
	{
	        an_status = (*(unsigned long *)(ESW_PHY_POLLING) & (1<<31));
		if(an_status){
			*(unsigned long *)(ESW_PHY_POLLING) &= ~(1<<31);//(AN polling off)
		}
		//phase1: write page address phase
                if(__mii_mgr_write(phy_addr, 0x1f, ((phy_register >> 6) & 0x3FF))) {
                        //phase2: write address & read low word phase
                        if(__mii_mgr_read(phy_addr, (phy_register >> 2) & 0xF, &low_word)) {
                                //phase3: write address & read high word phase
                                if(__mii_mgr_read(phy_addr, (0x1 << 4), &high_word)) {
                                        *read_data = (high_word << 16) | (low_word & 0xFFFF);
					if(an_status){
						*(unsigned long *)(ESW_PHY_POLLING) |= (1<<31);//(AN polling on)
					}
					return 1;
                                }
                        }
                }
		if(an_status){
			*(unsigned long *)(ESW_PHY_POLLING) |= (1<<31);//(AN polling on)
		}
        } else 
#endif
	{
                if(__mii_mgr_read(phy_addr, phy_register, read_data)) {
                        return 1;
                }
        }

        return 0;
}

u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
#if defined (CONFIG_GE1_RGMII_FORCE_1000) || defined (CONFIG_GE1_TRGMII_FORCE_1200) || defined (CONFIG_P5_RGMII_TO_MT7530_MODE)
	u32 an_status = 0;
        
	if(phy_addr == 31) 
	{
		an_status = (*(unsigned long *)(ESW_PHY_POLLING) & (1<<31));
		if(an_status){
			*(unsigned long *)(ESW_PHY_POLLING) &= ~(1<<31);//(AN polling off)
		}
		//phase1: write page address phase
                if(__mii_mgr_write(phy_addr, 0x1f, (phy_register >> 6) & 0x3FF)) {
                        //phase2: write address & read low word phase
                        if(__mii_mgr_write(phy_addr, ((phy_register >> 2) & 0xF), write_data & 0xFFFF)) {
                                //phase3: write address & read high word phase
                                if(__mii_mgr_write(phy_addr, (0x1 << 4), write_data >> 16)) {
					if(an_status){
						*(unsigned long *)(ESW_PHY_POLLING) |= (1<<31);//(AN polling on)
					}
				    return 1;
                                }
                        }
                }
		if(an_status){
			*(unsigned long *)(ESW_PHY_POLLING) |= (1<<31);//(AN polling on)
		}
        } else 
#endif
	{
                if(__mii_mgr_write(phy_addr, phy_register, write_data)) {
                        return 1;
                }
        }

        return 0;
}

u32 mii_mgr_cl45_set_address(u32 port_num, u32 dev_addr, u32 reg_addr)
{
	u32 rc = 0;
	unsigned long volatile t_start = jiffies;
	u32 volatile data = 0;

	enable_mdio(1);

	while (1) {
		if(!( sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31)))
		{
			break;
		}
		else if (time_after(jiffies, t_start + 5*HZ)) {
			enable_mdio(0);
			printk("\n MDIO Read operation is ongoing !!\n");
			return rc;
		}
	}
	data = (dev_addr << 25) | (port_num << 20) | (0x00 << 18) | (0x00 << 16) | reg_addr;
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);

	t_start = jiffies;
	while (1) {
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) //0 : Read/write operation complete
		{
			enable_mdio(0);
			return 1;
		}
		else if (time_after(jiffies, t_start + 5 * HZ)) {
			enable_mdio(0);
			printk("\n MDIO Write operation Time Out\n");
			return 0;
		}
	}

}


u32 mii_mgr_read_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 *read_data)
{
	u32 volatile status = 0;
	u32 rc = 0;
	unsigned long volatile t_start = jiffies;
	u32 volatile data = 0;

        // set address first
	mii_mgr_cl45_set_address(port_num, dev_addr, reg_addr);
	//udelay(10);

	enable_mdio(1);

	while (1) {
		if(!( sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31)))
		{
			break;
		}
		else if (time_after(jiffies, t_start + 5*HZ)) {
			enable_mdio(0);
			printk("\n MDIO Read operation is ongoing !!\n");
			return rc;
		}
	}
	data = (dev_addr << 25) | (port_num << 20) | (0x03 << 18) | (0x00 << 16) | reg_addr;
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	t_start = jiffies;
	while (1) {
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			*read_data = (sysRegRead(MDIO_PHY_CONTROL_0) & 0x0000FFFF);
			enable_mdio(0);
			return 1;
		}
		else if (time_after(jiffies, t_start+5*HZ)) {
			enable_mdio(0);
			printk("\n Set Operation: MDIO Read operation is ongoing and Time Out!!\n");
			return 0;
		}
		status = sysRegRead(MDIO_PHY_CONTROL_0);
	}

}

u32 mii_mgr_write_cl45	(u32 port_num, u32 dev_addr, u32 reg_addr, u32 write_data)
{
	u32 rc = 0;
	unsigned long volatile t_start = jiffies;
	u32 volatile data = 0;

	// set address first
	mii_mgr_cl45_set_address(port_num, dev_addr, reg_addr);
	//udelay(10);

	enable_mdio(1);
	while (1) {
		if(!( sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31)))
		{
			break;
		}
		else if (time_after(jiffies, t_start + 5*HZ)) {
			enable_mdio(0);
			printk("\n MDIO Read operation is ongoing !!\n");
			return rc;
		}
	}

	data = (dev_addr << 25) | (port_num << 20) | (0x01 << 18) | (0x00 << 16) | write_data;
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);

	t_start = jiffies;

	while (1) {
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31)))
		{
			enable_mdio(0);
			return 1;
		}
		else if (time_after(jiffies, t_start + 5 * HZ)) {
			enable_mdio(0);
			printk("\n MDIO Write operation Time Out\n");
			return 0;
		}

	}
}

EXPORT_SYMBOL(mii_mgr_write);
EXPORT_SYMBOL(mii_mgr_read);
