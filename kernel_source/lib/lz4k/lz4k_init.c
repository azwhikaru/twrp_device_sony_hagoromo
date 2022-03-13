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
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>

int lz4k_compress(const unsigned char *src, size_t src_len,
		  unsigned char *dst, size_t *dst_len, void *wrkmem);
int lz4k_decompress_safe(const unsigned char *src, size_t src_len,
			 unsigned char *dst, size_t *dst_len);


/* Set ZRAM hooks */
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 4, 69)) && defined(CONFIG_ZRAM)
extern void zram_set_hooks(void *compress_func, void *decompress_func);
#endif
static int __init lz4k_init(void)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 4, 69)) && defined(CONFIG_ZRAM)
	zram_set_hooks(&lz4k_compress, &lz4k_decompress_safe);
#endif
	return 0;
}

static void __exit lz4k_exit(void)
{
	printk(KERN_INFO "Bye LZ4K!\n");
}
module_init(lz4k_init);
module_exit(lz4k_exit);
