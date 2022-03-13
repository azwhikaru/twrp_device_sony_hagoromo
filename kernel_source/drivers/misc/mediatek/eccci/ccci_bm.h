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
#ifndef __CCCI_BM_H__
#define __CCCI_BM_H__

#include "ccci_core.h"

/*
 * tricky part, what's the relation between pool size of ccci_request and pool size of skb?
 */
#define SKB_POOL_SIZE_4K 256
#define SKB_POOL_SIZE_1_5K 256
#define SKB_POOL_SIZE_16 64
#define BM_POOL_SIZE (SKB_POOL_SIZE_4K+SKB_POOL_SIZE_1_5K+SKB_POOL_SIZE_16)
#define RELOAD_TH 3		/* reload pool if pool size dropped below 1/RELOAD_TH */

/*
 * the actually allocated skb's buffer is much bigger than what we request, so when we judge
 * which pool it belongs, the comparision is quite tricky...
 *
 * beaware, these macros are also used by CLDMA
 */
#define SKB_4K CCCI_MTU+16	/* user MTU+CCCI_H, for genral packet */
#define SKB_1_5K CCMNI_MTU+16	/* net MTU+CCCI_H, for network packet */
#define SKB_16 16		/* for struct ccci_header */
#define skb_size(x) ((x)->end - (x)->head)

struct sk_buff *ccci_alloc_skb(int size);
void ccci_free_skb(struct sk_buff *skb, DATA_POLICY policy);

struct ccci_request *ccci_alloc_req(DIRECTION dir, int size, char blk1, char blk2);
void ccci_free_req(struct ccci_request *req);
void ccci_dump_req(struct ccci_request *req);
void ccci_mem_dump(void *start_addr, int len);

#endif				/* __CCCI_BM_H__ */