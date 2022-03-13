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
/*
 */
struct btcx_riscmem {
	unsigned int   size;
	__le32         *cpu;
	__le32         *jmp;
	dma_addr_t     dma;
};

struct btcx_skiplist {
	int start;
	int end;
};

int  btcx_riscmem_alloc(struct pci_dev *pci,
			struct btcx_riscmem *risc,
			unsigned int size);
void btcx_riscmem_free(struct pci_dev *pci,
		       struct btcx_riscmem *risc);

int btcx_screen_clips(int swidth, int sheight, struct v4l2_rect *win,
		      struct v4l2_clip *clips, unsigned int n);
int btcx_align(struct v4l2_rect *win, struct v4l2_clip *clips,
	       unsigned int n, int mask);
void btcx_sort_clips(struct v4l2_clip *clips, unsigned int nclips);
void btcx_calc_skips(int line, int width, int *maxy,
		     struct btcx_skiplist *skips, unsigned int *nskips,
		     const struct v4l2_clip *clips, unsigned int nclips);

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
