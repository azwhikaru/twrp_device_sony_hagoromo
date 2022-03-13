/*
 * Copyright 2016 Sony Corporation
 */

#ifndef _MT8590_MEMIF_H_
#define _MT8590_MEMIF_H_

struct rtc_multi_i2s_playback {
    enum afe_mem_interface memiftargetacc;
    enum audio_irq_id targetaccirqid;
};

extern struct rtc_multi_i2s_playback rtc_multi[];
extern int total_playback_num;

#endif // _MT8590_MEMIF_H_
