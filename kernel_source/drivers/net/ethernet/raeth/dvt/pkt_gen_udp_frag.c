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
#if 0
    unsigned char pkt_0[]={
//    0x00, 0x21, 0x86, 0xee, 0xe3, 0x95, // dest mac
    0x00, 0x21, 0x86, 0xee, 0xe3, 0x90, // dest mac
    0x00, 0x0c, 0x43, 0x28, 0x80, 0x33, // src mac
    0x08, 0x00, // type: ip
    0x45, 0x00, 0x00, 0x26, // ip: ..., total len (0x026 = 38)
//    0xa1, 0x78, 0x20, 0x00, // ip: id, frag, frag offset
    0xa1, 0x78, 0x40, 0x00, // ip: id, frag, frag offset
    0x40, 0x11, 0x63, 0x07, // ip: ttl, protocol, hdr checksum (0x6307)
    0x0a, 0x0a, 0x1e, 0x7b, // src ip (10.10.30.123)
//    0x0a, 0x0a, 0x1e, 0x03, // dst ip (10.10.30.3)
    0x0a, 0x0a, 0x1e, 0x05, // dst ip (10.10.30.5)
    0xca, 0x7b,  //udp src port
    0x13, 0x89,  //udp dst port
    0x00, 0x12,  //udp len (0x01c = 18) 
    0x2f, 0x96,  //udp checksum (0x2f96)
    0x01, 0x02, 0x03, 0x04, 0x05,  //payload (10 bytes)
    0x06, 0x07, 0x08, 0x09, 0x0a
    };
#endif

    unsigned char pkt_1[]={
//    0x00, 0x21, 0x86, 0xee, 0xe3, 0x95, // dest mac
    0x00, 0x21, 0x86, 0xee, 0xe3, 0x90, // dest mac
    0x00, 0x0c, 0x43, 0x28, 0x80, 0x33, // src mac
    0x08, 0x00, // type: ip
    0x45, 0x00, 0x00, 0x24, // ip: ..., total len (0x024 = 36)
    0xa1, 0x78, 0x20, 0x00, // ip: id, frag, frag offset
//    0xa1, 0x78, 0x40, 0x00, // ip: id, frag, frag offset
    0x40, 0x11, 0x63, 0x07, // ip: ttl, protocol, hdr checksum (0x6307)
    0x0a, 0x0a, 0x1e, 0x7b, // src ip (10.10.30.123)
//    0x0a, 0x0a, 0x1e, 0x03, // dst ip (10.10.30.3)
    0x0a, 0x0a, 0x1e, 0x05, // dst ip (10.10.30.5)
    0xca, 0x7b,  //udp src port
    0x13, 0x89,  //udp dst port
    0x00, 0x1a,  //udp len (0x01a = 26) 
    0x2f, 0x96,  //udp checksum (0x2f96)
    0x01, 0x02, 0x03, 0x04, 0x05,  //payload (8 bytes)
    0x06, 0x07, 0x08
    };
    
    unsigned char pkt_2[]={
//    0x00, 0x21, 0x86, 0xee, 0xe3, 0x95, // dest mac
    0x00, 0x21, 0x86, 0xee, 0xe3, 0x90, // dest mac
    0x00, 0x0c, 0x43, 0x28, 0x80, 0x33, // src mac
    0x08, 0x00, // type: ip
    0x45, 0x00, 0x00, 0x1e, // ip: ..., total len (0x01e = 30)
    0xa1, 0x78, 0x00, 0x02, // ip: id, frag, frag offset (16)
    0x40, 0x11, 0x63, 0x07, // ip: ttl, protocol, hdr checksum (0x6307)
    0x0a, 0x0a, 0x1e, 0x7b, // src ip (10.10.30.123)
//   0x0a, 0x0a, 0x1e, 0x03, // dst ip (10.10.30.3)
    0x0a, 0x0a, 0x1e, 0x05, // dst ip (10.10.30.5)
    0x11, 0x12, 0x13, 0x14, 0x15,  //payload (10 bytes)
    0x16, 0x17, 0x18, 0x19, 0x1a
    };

    struct net_dev *dev;
//    struct sk_buff *skb_0;
    struct sk_buff *skb_1;
    struct sk_buff *skb_2;
    int i=0;

//    skb_0 = alloc_skb(256, GFP_ATOMIC);
    skb_1 = alloc_skb(256, GFP_ATOMIC);
    skb_2 = alloc_skb(256, GFP_ATOMIC);

#if 0
/* send packet 0 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    if((dev=dev_get_by_name(&init_net,ifname))){
#else
    if((dev=dev_get_by_name(ifname))){
#endif

	skb_0->dev=dev;
	skb_put(skb_0,sizeof(pkt_0));
	memcpy(skb_0->data, pkt_0, sizeof(pkt_0));

	printk("send pkt(len=%d) to %s\n", skb_0->len, skb_0->dev->name);


	for(i=0;i<sizeof(pkt_0);i++){
	    if(i%16==0) {
		printk("\n");
	    }
	    printk("%02X-",skb_0->data[i]);
	}

	dev_queue_xmit(skb_0);
    }else{
	printk("interface %s not found\n",ifname);
	return 1;
    }
#endif

#if 1
/* send packet 1 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    if((dev=dev_get_by_name(&init_net,ifname))){
#else
    if((dev=dev_get_by_name(ifname))){
#endif

	skb_1->dev=dev;
	skb_put(skb_1,sizeof(pkt_1));
	memcpy(skb_1->data, pkt_1, sizeof(pkt_1));

	printk("send pkt(len=%d) to %s\n", skb_1->len, skb_1->dev->name);


	for(i=0;i<sizeof(pkt_1);i++){
	    if(i%16==0) {
		printk("\n");
	    }
	    printk("%02X-",skb_1->data[i]);
	}

	dev_queue_xmit(skb_1);
    }else{
	printk("interface %s not found\n",ifname);
	return 1;
    }
#endif

#if 1
/* send packet 2 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
        if((dev=dev_get_by_name(&init_net,ifname))){
#else
        if((dev=dev_get_by_name(ifname))){
#endif

	skb_2->dev=dev;
	skb_put(skb_2,sizeof(pkt_2));
	memcpy(skb_2->data, pkt_2, sizeof(pkt_2));

	printk("send pkt(len=%d) to %s\n", skb_2->len, skb_2->dev->name);


	for(i=0;i<sizeof(pkt_2);i++){
	    if(i%16==0) {
		printk("\n");
	    }
	    printk("%02X-",skb_2->data[i]);
	}

	dev_queue_xmit(skb_2);
    }else{
	printk("interface %s not found\n",ifname);
	return 1;
    }
#endif

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

