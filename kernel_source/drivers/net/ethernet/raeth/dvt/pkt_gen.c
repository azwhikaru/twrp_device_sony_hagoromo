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
//#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/moduleparam.h>

char *ifname="eth3";

static int32_t PktGenInitMod(void)
{

    struct net_dev *dev;
    struct sk_buff *skb;
    int i=0;

    unsigned char pkt[]={
	//0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // dest bcast mac
	0x00, 0x21, 0x86, 0xee, 0xe3, 0x95, // dest macA
	//0x00, 0x30, 0xdb, 0x02, 0x02, 0x01, // dest macB
	0x00, 0x0c, 0x43, 0x28, 0x80, 0x33, // src mac
	0x81, 0x00, // vlan tag
	//0x81, 0x10, // vlan tag
	//0x87, 0x39, // do not learn
	//0xc1, 0x03, // vlan tag SA=0, VID=2, LV=1
	0x00, 0x03, // pri=0, vlan=3
	0x08, 0x00, // eth type=ip
	0x45, 0x00, 0x00, 0x30, 0x12, 0x34, 0x40, 0x00, 0xff, 0x06,
	0x40, 0x74, 0x0a, 0x0a, 0x1e, 0x0a, 0x0a, 0x0a, 0x1e, 0x0b,
	0x00, 0x1e, 0x00, 0x28, 0x00, 0x1c, 0x81, 0x06, 0x00, 0x00,
	0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    skb = alloc_skb(256, GFP_ATOMIC);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    if((dev=dev_get_by_name(&init_net,ifname))){
#else
    if((dev=dev_get_by_name(ifname))){
#endif



	skb->dev=dev;
	skb_put(skb,sizeof(pkt));
	memcpy(skb->data, pkt, sizeof(pkt));

	printk("send pkt(len=%d) to %s\n", skb->len, skb->dev->name);


	for(i=0;i<sizeof(pkt);i++){
	    if(i%16==0) {
		printk("\n");
	    }
	    printk("%02X-",skb->data[i]);
	}

	dev_queue_xmit(skb);
    }else{
	printk("interface %s not found\n",ifname);
	return 1;
    }

    return 0;
}

static void PktGenCleanupMod(void)
{
}

module_init(PktGenInitMod);
module_exit(PktGenCleanupMod);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (ifname, "s");
#else
module_param (ifname, charp, 0);
#endif

MODULE_DESCRIPTION("Ralink PktGen Module");
MODULE_AUTHOR("Steven Liu");
MODULE_LICENSE("Proprietary");
MODULE_PARM_DESC (ifname, "interface name");

