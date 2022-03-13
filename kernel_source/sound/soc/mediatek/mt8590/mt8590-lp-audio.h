/*
* Copyright (C) 2011-2015 MediaTek Inc.
*/
 /* but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _MT8590_LP_AUDIO_H_
#define _MT8590_LP_AUDIO_H_

extern struct snd_pcm_substream *lp_substream;
extern enum audio_irq_id lp_irq_id;
void lp_audio_isr(void);

#endif
