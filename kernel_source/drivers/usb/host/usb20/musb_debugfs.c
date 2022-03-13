/*
 * Copyright 2016 Sony Corporation
 * File Changed on 2016-10-17
 */
/*
 * MUSB OTG driver debugfs support
 *
 * Copyright 2010 Nokia Corporation
 * Contact: Felipe Balbi <felipe.balbi@nokia.com>
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h>

#include <linux/musb/musb_core.h>
#include <linux/musb/musb_debug.h>

#define ICX_USB_TEST_FOR_USBHUB


struct musb_register_map {
	char *name;
	unsigned offset;
	unsigned size;
};

static const struct musb_register_map musb_regmap[] = {
	{"FAddr", 0x00, 8},
	{"Power", 0x01, 8},
	{"Frame", 0x0c, 16},
	{"Index", 0x0e, 8},
	{"Testmode", 0x0f, 8},
	{"TxMaxPp", 0x10, 16},
	{"TxCSRp", 0x12, 16},
	{"RxMaxPp", 0x14, 16},
	{"RxCSR", 0x16, 16},
	{"RxCount", 0x18, 16},
	{"ConfigData", 0x1f, 8},
	{"DevCtl", 0x60, 8},
	{"MISC", 0x61, 8},
	{"TxFIFOsz", 0x62, 8},
	{"RxFIFOsz", 0x63, 8},
	{"TxFIFOadd", 0x64, 16},
	{"RxFIFOadd", 0x66, 16},
	{"VControl", 0x68, 32},
	{"HWVers", 0x6C, 16},
	{"EPInfo", 0x78, 8},
	{"RAMInfo", 0x79, 8},
	{"LinkInfo", 0x7A, 8},
	{"VPLen", 0x7B, 8},
	{"HS_EOF1", 0x7C, 8},
	{"FS_EOF1", 0x7D, 8},
	{"LS_EOF1", 0x7E, 8},
	{"SOFT_RST", 0x7F, 8},
	{"DMA_CNTLch0", 0x204, 16},
	{"DMA_ADDRch0", 0x208, 32},
	{"DMA_COUNTch0", 0x20C, 32},
	{"DMA_CNTLch1", 0x214, 16},
	{"DMA_ADDRch1", 0x218, 32},
	{"DMA_COUNTch1", 0x21C, 32},
	{"DMA_CNTLch2", 0x224, 16},
	{"DMA_ADDRch2", 0x228, 32},
	{"DMA_COUNTch2", 0x22C, 32},
	{"DMA_CNTLch3", 0x234, 16},
	{"DMA_ADDRch3", 0x238, 32},
	{"DMA_COUNTch3", 0x23C, 32},
	{"DMA_CNTLch4", 0x244, 16},
	{"DMA_ADDRch4", 0x248, 32},
	{"DMA_COUNTch4", 0x24C, 32},
	{"DMA_CNTLch5", 0x254, 16},
	{"DMA_ADDRch5", 0x258, 32},
	{"DMA_COUNTch5", 0x25C, 32},
	{"DMA_CNTLch6", 0x264, 16},
	{"DMA_ADDRch6", 0x268, 32},
	{"DMA_COUNTch6", 0x26C, 32},
	{"DMA_CNTLch7", 0x274, 16},
	{"DMA_ADDRch7", 0x278, 32},
	{"DMA_COUNTch7", 0x27C, 32},
	{}			/* Terminating Entry */
};

static struct dentry *musb_debugfs_root;

static int musb_regdump_show(struct seq_file *s, void *unused)
{
	struct musb *musb = s->private;
	unsigned i;

	seq_puts(s, "MUSB (M)HDRC Register Dump\n");

	for (i = 0; i < ARRAY_SIZE(musb_regmap); i++) {
		switch (musb_regmap[i].size) {
		case 8:
			seq_printf(s, "%-12s: %02x\n", musb_regmap[i].name,
				   musb_readb(musb->mregs, musb_regmap[i].offset));
			break;
		case 16:
			seq_printf(s, "%-12s: %04x\n", musb_regmap[i].name,
				   musb_readw(musb->mregs, musb_regmap[i].offset));
			break;
		case 32:
			seq_printf(s, "%-12s: %08x\n", musb_regmap[i].name,
				   musb_readl(musb->mregs, musb_regmap[i].offset));
			break;
		}
	}

	return 0;
}

static int musb_regdump_open(struct inode *inode, struct file *file)
{
	return single_open(file, musb_regdump_show, inode->i_private);
}

static int musb_test_mode_show(struct seq_file *s, void *unused)
{
	struct musb *musb = s->private;
	unsigned test;

	test = musb_readb(musb->mregs, MUSB_TESTMODE);

	if (test & MUSB_TEST_FORCE_HOST)
		seq_puts(s, "force host\n");

	if (test & MUSB_TEST_FIFO_ACCESS)
		seq_puts(s, "fifo access\n");

	if (test & MUSB_TEST_FORCE_FS)
		seq_puts(s, "force full-speed\n");

	if (test & MUSB_TEST_FORCE_HS)
		seq_puts(s, "force high-speed\n");

	if (test & MUSB_TEST_PACKET)
		seq_puts(s, "test packet\n");

	if (test & MUSB_TEST_K)
		seq_puts(s, "test K\n");

	if (test & MUSB_TEST_J)
		seq_puts(s, "test J\n");

	if (test & MUSB_TEST_SE0_NAK)
		seq_puts(s, "test SE0 NAK\n");

	return 0;
}

static const struct file_operations musb_regdump_fops = {
	.open = musb_regdump_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int musb_test_mode_open(struct inode *inode, struct file *file)
{
	return single_open(file, musb_test_mode_show, inode->i_private);
}

#ifdef ICX_USB_TEST_FOR_USBHUB
#define TEST_FORCE_ENABLE		(0x00040000 +0x00010000)
#endif

static ssize_t musb_test_mode_write(struct file *file,
				    const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct musb *musb = s->private;
	u8 test = 0;
	char buf[18];

#ifdef ICX_USB_TEST_FOR_USBHUB
	struct usb_hcd	  *hcd = NULL;
	struct usb_bus	  *bus = NULL;
	struct usb_device *root_hub  = NULL;
	struct usb_device *usb_child = NULL;

	int	ret;
	int	i = 0;
	const	int	 CONTROL_REQUEST_TIMEOUT = 1000;	/* ms */
	const	uint16_t HUB_PORT_1ST = 1;
	u8 	hub_test = 0;
#endif

	memset(buf, 0x00, sizeof(buf));

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

#ifdef ICX_USB_TEST_FOR_USBHUB
	if (!strncmp(buf, "hub J", 5)) {				/* J(HUB) */
		hub_test = 1;
	} else if (!strncmp(buf, "hub K", 5)) {			/* K(HUB) */
		hub_test = 2;
	} else if (!strncmp(buf, "hub SE0", 7)) {		/* SE0_NAK(HUB) */
		hub_test = 3;
	} else if (!strncmp(buf, "hub packet", 10)) {	/* PACKET(HUB)  */
		hub_test = 4;
	} else if (!strncmp(buf, "hub force", 9)) {		/* FORCE_ENABLE(HUB) */
		hub_test = 5;
	}

	if (hub_test > 0) {
		hcd = musb_to_hcd(musb);
		if (hcd == NULL) {
			pr_err("[dbg:%s(%d)] <error>\n", __func__, __LINE__);
			return 0;
		}

		usb_get_hcd(hcd);
		bus = hcd_to_bus(hcd);
		if (bus == NULL) {
			pr_err("[dbg:%s(%d)] <error>\n", __func__, __LINE__);
			goto out_put;
		}

		root_hub = bus->root_hub;
		if (root_hub == NULL) {
			pr_err("[dbg:%s(%d)] <error>\n", __func__, __LINE__);
			goto out_put;
		}

		/* Add root hub refrence */
		usb_get_dev(root_hub);
		usb_lock_device(root_hub);

		ret = usb_control_msg(root_hub,
				usb_sndctrlpipe(root_hub, 0),
				USB_REQ_SET_FEATURE, USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER,
				USB_PORT_FEAT_TEST, (TEST_FORCE_ENABLE << 8) | HUB_PORT_1ST,
				NULL, 0, CONTROL_REQUEST_TIMEOUT);
		if (ret < 0) {
			pr_err("[dbg:%s(%d)] <error>\n", __func__, __LINE__);
			goto out_put;
		}

		while (i < root_hub->maxchild) {
			usb_child = usb_hub_find_child(root_hub, 1);
			pr_info("[dbg:%s(%d)] hub_test:%d child:%p\n",
					__func__, __LINE__, hub_test, usb_child);
			if (usb_child != NULL) {
				usb_get_dev(usb_child);

				usb_lock_device(usb_child);
				ret = usb_control_msg(usb_child,
						usb_sndctrlpipe(usb_child, 0),
						USB_REQ_SET_FEATURE, USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER,
						USB_PORT_FEAT_TEST, (hub_test << 8) | HUB_PORT_1ST,
						NULL, 0, CONTROL_REQUEST_TIMEOUT);

				pr_info("[dbg:%s(%d)] test complete!!\n", __func__, __LINE__);

				usb_unlock_device(usb_child);
				usb_put_dev(usb_child);
				if (ret < 0) {
					pr_err("%s: Failed requesting control to child hub. ret=%d\n", __func__, ret);
					break;
				}
			}
			i++;
		}

out_put:
		/* remove root hub refrence */
		if (root_hub != NULL) {
			usb_unlock_device(root_hub);
			usb_put_dev(root_hub);
		}

		/* remove hcd refrence */
		usb_put_hcd(hcd);

		return count;
	}
#endif

	if (!strncmp(buf, "force host", 9))
		test = MUSB_TEST_FORCE_HOST;

	if (!strncmp(buf, "fifo access", 11))
		test = MUSB_TEST_FIFO_ACCESS;

	if (!strncmp(buf, "force full-speed", 15))
		test = MUSB_TEST_FORCE_FS;

	if (!strncmp(buf, "force high-speed", 15))
		test = MUSB_TEST_FORCE_HS;

	if (!strncmp(buf, "test packet", 10)) {
		test = MUSB_TEST_PACKET;
		musb_load_testpacket(musb);
	}

	if (!strncmp(buf, "test K", 6))
		test = MUSB_TEST_K;

	if (!strncmp(buf, "test J", 6))
		test = MUSB_TEST_J;

	if (!strncmp(buf, "test SE0 NAK", 12))
		test = MUSB_TEST_SE0_NAK;

	musb_writeb(musb->mregs, MUSB_TESTMODE, test);

	return count;
}

static const struct file_operations musb_test_mode_fops = {
	.open = musb_test_mode_open,
	.write = musb_test_mode_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int musb_init_debugfs(struct musb *musb)
{
	struct dentry *root;
	struct dentry *file;
	int ret;

	root = debugfs_create_dir("musb", NULL);
	if (!root) {
		ret = -ENOMEM;
		goto err0;
	}

	file = debugfs_create_file("regdump", S_IRUGO, root, musb, &musb_regdump_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

	file = debugfs_create_file("testmode", S_IRUGO | S_IWUSR, root, musb, &musb_test_mode_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

	musb_debugfs_root = root;

	return 0;

 err1:
	debugfs_remove_recursive(root);

 err0:
	return ret;
}

void /* __init_or_exit */ musb_exit_debugfs(struct musb *musb)
{
	debugfs_remove_recursive(musb_debugfs_root);
}
