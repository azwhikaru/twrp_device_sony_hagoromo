/*
 * Copyright 2016,2017 Sony Corporation
 * File changed on 2017-01-25
 */
/*
* Copyright (C) 2015 MediaTek Inc.
* Modification based on code covered by the below mentioned copyright
* and/or permission notice(s).
*/

/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */


#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/usb/audio.h>
#include <linux/usb/audio-v2.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/control.h>
#include <sound/tlv.h>

#include "usbaudio.h"
#include "card.h"
#include "proc.h"
#include "quirks.h"
#include "endpoint.h"
#include "pcm.h"
#include "helper.h"
#include "format.h"
#include "clock.h"
#include "stream.h"

/*
 * free a substream
 */
static void free_substream(struct snd_usb_substream *subs)
{
    struct audioformat *fp, *n;

    if (!subs->num_formats)
        return; /* not initialized */
    list_for_each_entry_safe(fp, n, &subs->fmt_list, list) {
        kfree(fp->rate_table);
        kfree(fp->chmap);
        kfree(fp);
    }
    kfree(subs->rate_list.list);
}


/*
 * free a usb stream instance
 */
static void snd_usb_audio_stream_free(struct snd_usb_stream *stream)
{
    free_substream(&stream->substream[0]);
    free_substream(&stream->substream[1]);
    list_del(&stream->list);
    kfree(stream);
}

static void snd_usb_audio_pcm_free(struct snd_pcm *pcm)
{
    struct snd_usb_stream *stream = pcm->private_data;
    if (stream) {
        stream->pcm = NULL;
        snd_usb_audio_stream_free(stream);
    }
}

/*
 * initialize the substream instance.
 */

static void snd_usb_init_substream(struct snd_usb_stream *as,
                   int stream,
                   struct audioformat *fp)
{
    struct snd_usb_substream *subs = &as->substream[stream];

    INIT_LIST_HEAD(&subs->fmt_list);
    spin_lock_init(&subs->lock);

    subs->stream = as;
    subs->direction = stream;
    subs->dev = as->chip->dev;
    subs->txfr_quirk = as->chip->txfr_quirk;
    subs->speed = snd_usb_get_speed(subs->dev);
    subs->pkt_offset_adj = 0;

    snd_usb_set_pcm_ops(as->pcm, stream);

    list_add_tail(&fp->list, &subs->fmt_list);
    subs->formats |= fp->formats;
    subs->num_formats++;
    subs->fmt_type = fp->fmt_type;
    subs->ep_num = fp->endpoint;
    if (fp->channels > subs->channels_max)
        subs->channels_max = fp->channels;
}

/* kctl callbacks for usb-audio channel maps */
static int usb_chmap_ctl_info(struct snd_kcontrol *kcontrol,
                  struct snd_ctl_elem_info *uinfo)
{
    struct snd_pcm_chmap *info = snd_kcontrol_chip(kcontrol);
    struct snd_usb_substream *subs = info->private_data;

    uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
    uinfo->count = subs->channels_max;
    uinfo->value.integer.min = 0;
    uinfo->value.integer.max = SNDRV_CHMAP_LAST;
    return 0;
}

/* check whether a duplicated entry exists in the audiofmt list */
static bool have_dup_chmap(struct snd_usb_substream *subs,
               struct audioformat *fp)
{
    struct list_head *p;

    for (p = fp->list.prev; p != &subs->fmt_list; p = p->prev) {
        struct audioformat *prev;
        prev = list_entry(p, struct audioformat, list);
        if (prev->chmap &&
            !memcmp(prev->chmap, fp->chmap, sizeof(*fp->chmap)))
            return true;
    }
    return false;
}

static int usb_chmap_ctl_tlv(struct snd_kcontrol *kcontrol, int op_flag,
                 unsigned int size, unsigned int __user *tlv)
{
    struct snd_pcm_chmap *info = snd_kcontrol_chip(kcontrol);
    struct snd_usb_substream *subs = info->private_data;
    struct audioformat *fp;
    unsigned int __user *dst;
    int count = 0;

    if (size < 8)
        return -ENOMEM;
    if (put_user(SNDRV_CTL_TLVT_CONTAINER, tlv))
        return -EFAULT;
    size -= 8;
    dst = tlv + 2;
    list_for_each_entry(fp, &subs->fmt_list, list) {
        int i, ch_bytes;

        if (!fp->chmap)
            continue;
        if (have_dup_chmap(subs, fp))
            continue;
        /* copy the entry */
        ch_bytes = fp->chmap->channels * 4;
        if (size < 8 + ch_bytes)
            return -ENOMEM;
        if (put_user(SNDRV_CTL_TLVT_CHMAP_FIXED, dst) ||
            put_user(ch_bytes, dst + 1))
            return -EFAULT;
        dst += 2;
        for (i = 0; i < fp->chmap->channels; i++, dst++) {
            if (put_user(fp->chmap->map[i], dst))
                return -EFAULT;
        }

        count += 8 + ch_bytes;
        size -= 8 + ch_bytes;
    }
    if (put_user(count, tlv + 1))
        return -EFAULT;
    return 0;
}

static int usb_chmap_ctl_get(struct snd_kcontrol *kcontrol,
                 struct snd_ctl_elem_value *ucontrol)
{
    struct snd_pcm_chmap *info = snd_kcontrol_chip(kcontrol);
    struct snd_usb_substream *subs = info->private_data;
    struct snd_pcm_chmap_elem *chmap = NULL;
    int i;

    memset(ucontrol->value.integer.value, 0,
           sizeof(ucontrol->value.integer.value));
    if (subs->cur_audiofmt)
        chmap = subs->cur_audiofmt->chmap;
    if (chmap) {
        for (i = 0; i < chmap->channels; i++)
            ucontrol->value.integer.value[i] = chmap->map[i];
    }
    return 0;
}

/* create a chmap kctl assigned to the given USB substream */
static int add_chmap(struct snd_pcm *pcm, int stream,
             struct snd_usb_substream *subs)
{
    struct audioformat *fp;
    struct snd_pcm_chmap *chmap;
    struct snd_kcontrol *kctl;
    int err;

    list_for_each_entry(fp, &subs->fmt_list, list)
        if (fp->chmap)
            goto ok;
    /* no chmap is found */
    return 0;

 ok:
    err = snd_pcm_add_chmap_ctls(pcm, stream, NULL, 0, 0, &chmap);
    if (err < 0)
        return err;

    /* override handlers */
    chmap->private_data = subs;
    kctl = chmap->kctl;
    kctl->info = usb_chmap_ctl_info;
    kctl->get = usb_chmap_ctl_get;
    kctl->tlv.c = usb_chmap_ctl_tlv;

    return 0;
}

/* convert from USB ChannelConfig bits to ALSA chmap element */
static struct snd_pcm_chmap_elem *convert_chmap(int channels, unsigned int bits,
                        int protocol)
{
    static unsigned int uac1_maps[] = {
        SNDRV_CHMAP_FL,     /* left front */
        SNDRV_CHMAP_FR,     /* right front */
        SNDRV_CHMAP_FC,     /* center front */
        SNDRV_CHMAP_LFE,    /* LFE */
        SNDRV_CHMAP_SL,     /* left surround */
        SNDRV_CHMAP_SR,     /* right surround */
        SNDRV_CHMAP_FLC,    /* left of center */
        SNDRV_CHMAP_FRC,    /* right of center */
        SNDRV_CHMAP_RC,     /* surround */
        SNDRV_CHMAP_SL,     /* side left */
        SNDRV_CHMAP_SR,     /* side right */
        SNDRV_CHMAP_TC,     /* top */
        0 /* terminator */
    };
    static unsigned int uac2_maps[] = {
        SNDRV_CHMAP_FL,     /* front left */
        SNDRV_CHMAP_FR,     /* front right */
        SNDRV_CHMAP_FC,     /* front center */
        SNDRV_CHMAP_LFE,    /* LFE */
        SNDRV_CHMAP_RL,     /* back left */
        SNDRV_CHMAP_RR,     /* back right */
        SNDRV_CHMAP_FLC,    /* front left of center */
        SNDRV_CHMAP_FRC,    /* front right of center */
        SNDRV_CHMAP_RC,     /* back center */
        SNDRV_CHMAP_SL,     /* side left */
        SNDRV_CHMAP_SR,     /* side right */
        SNDRV_CHMAP_TC,     /* top center */
        SNDRV_CHMAP_TFL,    /* top front left */
        SNDRV_CHMAP_TFC,    /* top front center */
        SNDRV_CHMAP_TFR,    /* top front right */
        SNDRV_CHMAP_TRL,    /* top back left */
        SNDRV_CHMAP_TRC,    /* top back center */
        SNDRV_CHMAP_TRR,    /* top back right */
        SNDRV_CHMAP_TFLC,   /* top front left of center */
        SNDRV_CHMAP_TFRC,   /* top front right of center */
        SNDRV_CHMAP_LLFE,   /* left LFE */
        SNDRV_CHMAP_RLFE,   /* right LFE */
        SNDRV_CHMAP_TSL,    /* top side left */
        SNDRV_CHMAP_TSR,    /* top side right */
        SNDRV_CHMAP_BC,     /* bottom center */
        SNDRV_CHMAP_BLC,    /* bottom left center */
        SNDRV_CHMAP_BRC,    /* bottom right center */
        0 /* terminator */
    };
    struct snd_pcm_chmap_elem *chmap;
    const unsigned int *maps;
    int c;

    if (!bits)
        return NULL;
    if (channels > ARRAY_SIZE(chmap->map))
        return NULL;

    chmap = kzalloc(sizeof(*chmap), GFP_KERNEL);
    if (!chmap)
        return NULL;

    maps = protocol == UAC_VERSION_2 ? uac2_maps : uac1_maps;
    chmap->channels = channels;
    c = 0;
    for (; bits && *maps; maps++, bits >>= 1) {
        if (bits & 1)
            chmap->map[c++] = *maps;
    }

    for (; c < channels; c++)
        chmap->map[c] = SNDRV_CHMAP_UNKNOWN;

    return chmap;
}

#ifdef MTK_ADD_DSD_DOP_SUPPORT
static void print_audio_format(struct audioformat *fp)
{
    printk("[USB]%s:%d audio format:%lld\n", __FUNCTION__, __LINE__, fp->formats);
    printk("[USB]foramt:0x%llx channel:%d fmt_type:%d frame_size:%d\n", 
            fp->formats, fp->channels, fp->fmt_type, fp->frame_size);
    printk("[USB]iface:%d altsetting:%d alt_inx:%d attr:%d\n", 
            fp->iface, fp->altsetting, fp->altset_idx, fp->attributes);
    printk("[USB]ep:%d ep_atrr:%d interval:%d maxp:%d\n", 
            fp->endpoint, fp->ep_attr, fp->datainterval, fp->maxpacksize);
    printk("[USB]rates:%d rate_min:%d rate_max:%d nr_rates:%d\n", 
            fp->rates, fp->rate_min, fp->rate_max, fp->nr_rates);
    printk("[USB]clock:%d dsd_dop:%d dsd_bitrev:%d\n", 
            fp->clock, fp->dsd_dop, fp->dsd_bitrev);
    
}

static unsigned int cap_capability(unsigned int rate_max, u32 usb_id, int type)
{
        switch (usb_id) {
			case USB_ID(0x054c,0x09d4): //MDR-1ADAC
				printk("[USB]%s:%d Detect MDR-1ADAC\n", __FUNCTION__, __LINE__);
				return	type == DSD_TYPE ? 176400 : 384000;

			case USB_ID(0x0644,0x8043): //TEAC UD-501
				printk("[USB]%s:%d Detect TEAC UD-501\n", __FUNCTION__, __LINE__);
				return	type == DSD_TYPE ? 176400 : 384000;
		}
		
		if (rate_max > 384000)
			return	type == DSD_TYPE ? 352800 : 384000;

		return rate_max;
}
#endif
/*
 * add this endpoint to the chip instance.
 * if a stream with the same endpoint already exists, append to it.
 * if not, create a new pcm stream.
 */

#if (defined(CONFIG_ARCH_MT8590_ICX))
int snd_usb_add_audio_stream_ex(struct snd_usb_audio *chip,
                 int stream,
                 struct audioformat *fp,
                 bool flag_head);

int snd_usb_add_audio_stream(struct snd_usb_audio *chip,
                 int stream,
                 struct audioformat *fp)
{
	return snd_usb_add_audio_stream_ex(chip, stream, fp, false);

}

int snd_usb_add_audio_stream_ex(struct snd_usb_audio *chip,
                 int stream,
                 struct audioformat *fp,
                 bool flag_head)
#else
int snd_usb_add_audio_stream(struct snd_usb_audio *chip,
                 int stream,
                 struct audioformat *fp)
#endif
{
    struct snd_usb_stream *as;
    struct snd_usb_substream *subs;
    struct snd_pcm *pcm;
    int err;

    list_for_each_entry(as, &chip->pcm_list, list) {
        if (as->fmt_type != fp->fmt_type)
            continue;
        subs = &as->substream[stream];
#if (defined(CONFIG_ARCH_MT8590_ICX))
        if (subs->ep_num == fp->endpoint) {
			if (flag_head)
	            list_add(&fp->list, &subs->fmt_list);
			else
            	list_add_tail(&fp->list, &subs->fmt_list);
#else
            list_add_tail(&fp->list, &subs->fmt_list);
#endif
            subs->num_formats++;
            subs->formats |= fp->formats;
            return 0;
        }
    }
    /* look for an empty stream */
    list_for_each_entry(as, &chip->pcm_list, list) {
        if (as->fmt_type != fp->fmt_type)
            continue;
        subs = &as->substream[stream];
        if (subs->ep_num)
            continue;
        err = snd_pcm_new_stream(as->pcm, stream, 1);
        if (err < 0)
            return err;
        snd_usb_init_substream(as, stream, fp);
        return add_chmap(as->pcm, stream, subs);
    }

    /* create a new pcm */
    as = kzalloc(sizeof(*as), GFP_KERNEL);
    if (!as)
        return -ENOMEM;

    as->pcm_index = chip->pcm_devs;
    as->chip = chip;
    as->fmt_type = fp->fmt_type;
    err = snd_pcm_new(chip->card, "USB Audio", chip->pcm_devs,
              stream == SNDRV_PCM_STREAM_PLAYBACK ? 1 : 0,
              stream == SNDRV_PCM_STREAM_PLAYBACK ? 0 : 1,
              &pcm);
    if (err < 0) {
        kfree(as);
        return err;
    }

    as->pcm = pcm;
    pcm->private_data = as;
    pcm->private_free = snd_usb_audio_pcm_free;
    pcm->info_flags = 0;
    if (chip->pcm_devs > 0)
        sprintf(pcm->name, "USB Audio #%d", chip->pcm_devs);
    else
        strcpy(pcm->name, "USB Audio");

    snd_usb_init_substream(as, stream, fp);

    list_add(&as->list, &chip->pcm_list);
    chip->pcm_devs++;

    snd_usb_proc_pcm_format_add(as);
    return add_chmap(pcm, stream, &as->substream[stream]);
}

static int parse_uac_endpoint_attributes(struct snd_usb_audio *chip,
                     struct usb_host_interface *alts,
                     int protocol, int iface_no)
{
    /* parsed with a v1 header here. that's ok as we only look at the
     * header first which is the same for both versions */
    struct uac_iso_endpoint_descriptor *csep;
    struct usb_interface_descriptor *altsd = get_iface_desc(alts);
    int attributes = 0;

    csep = snd_usb_find_desc(alts->endpoint[0].extra, alts->endpoint[0].extralen, NULL, USB_DT_CS_ENDPOINT);

    /* Creamware Noah has this descriptor after the 2nd endpoint */
    if (!csep && altsd->bNumEndpoints >= 2)
        csep = snd_usb_find_desc(alts->endpoint[1].extra, alts->endpoint[1].extralen, NULL, USB_DT_CS_ENDPOINT);

    /*
     * If we can't locate the USB_DT_CS_ENDPOINT descriptor in the extra
     * bytes after the first endpoint, go search the entire interface.
     * Some devices have it directly *before* the standard endpoint.
     */
    if (!csep)
        csep = snd_usb_find_desc(alts->extra, alts->extralen, NULL, USB_DT_CS_ENDPOINT);

    if (!csep || csep->bLength < 7 ||
        csep->bDescriptorSubtype != UAC_EP_GENERAL) {
        snd_printk(KERN_WARNING "%d:%u:%d : no or invalid"
               " class specific endpoint descriptor\n",
               chip->dev->devnum, iface_no,
               altsd->bAlternateSetting);
        return 0;
    }

    if (protocol == UAC_VERSION_1) {
        attributes = csep->bmAttributes;
    } else {
        struct uac2_iso_endpoint_descriptor *csep2 =
            (struct uac2_iso_endpoint_descriptor *) csep;

        attributes = csep->bmAttributes & UAC_EP_CS_ATTR_FILL_MAX;

        /* emulate the endpoint attributes of a v1 device */
        if (csep2->bmControls & UAC2_CONTROL_PITCH)
            attributes |= UAC_EP_CS_ATTR_PITCH_CONTROL;
    }

    return attributes;
}

/* find an input terminal descriptor (either UAC1 or UAC2) with the given
 * terminal id
 */
static void *
snd_usb_find_input_terminal_descriptor(struct usb_host_interface *ctrl_iface,
                           int terminal_id)
{
    struct uac2_input_terminal_descriptor *term = NULL;

    while ((term = snd_usb_find_csint_desc(ctrl_iface->extra,
                           ctrl_iface->extralen,
                           term, UAC_INPUT_TERMINAL))) {
        if (term->bTerminalID == terminal_id)
            return term;
    }

    return NULL;
}

static struct uac2_output_terminal_descriptor *
    snd_usb_find_output_terminal_descriptor(struct usb_host_interface *ctrl_iface,
                        int terminal_id)
{
    struct uac2_output_terminal_descriptor *term = NULL;

    while ((term = snd_usb_find_csint_desc(ctrl_iface->extra,
                           ctrl_iface->extralen,
                           term, UAC_OUTPUT_TERMINAL))) {
        if (term->bTerminalID == terminal_id)
            return term;
    }

    return NULL;
}

#ifdef MTK_ADD_DSD_DOP_SUPPORT

unsigned int snd_get_dsd_type(struct snd_usb_audio *chip,
                                    struct usb_host_interface *alts,
                                    struct audioformat *fp);
unsigned int snd_get_dop_type(struct usb_device *dev, 
                                    struct usb_host_interface *alts,
                                    struct audioformat *fp);

unsigned int snd_set_dsd_audio_fmt(struct usb_device *dev,
                                    struct audioformat *fp, unsigned int dsd_type);

unsigned int snd_set_dop_audio_fmt(struct usb_device *dev, 
                                    struct audioformat *fp, unsigned int dop_type);

unsigned int snd_force_create_dop_node(struct usb_device *dev, struct audioformat *fp);
unsigned int snd_force_create_dsd_node(struct usb_device *dev, struct audioformat *fp);



#endif

int snd_usb_parse_audio_interface(struct snd_usb_audio *chip, int iface_no)
{
    struct usb_device *dev;
    struct usb_interface *iface;
    struct usb_host_interface *alts;
    struct usb_interface_descriptor *altsd;
    int i, altno, err, stream;
    unsigned int format = 0, num_channels = 0;
    struct audioformat *fp = NULL;
    int num, protocol, clock = 0;
    struct uac_format_type_i_continuous_descriptor *fmt;
    unsigned int chconfig;
    #ifdef MTK_ADD_DSD_DOP_SUPPORT
    struct usb_host_interface *first_valid_fp = NULL;
    struct snd_card *snd_card = chip->card;
    struct uac_format_type_i_ext_descriptor *ext_fmt;
    struct audioformat *dsd_fp = NULL;
    struct audioformat *dop_fp = NULL;
    struct audioformat **dsd_fp_array = NULL;
    unsigned int dsd_fp_num = 0;
    struct audioformat **dop_fp_array = NULL;
    unsigned int dop_fp_num = 0;
    unsigned int snd_usb_type;
    unsigned char enable_type_b = 0;
    #endif
    
    unsigned int s32_max = 0;
    bool flag_head = false;
    
    dev = chip->dev;

    /* parse the interface's altsettings */
    iface = usb_ifnum_to_if(dev, iface_no);
    num = iface->num_altsetting;

    #ifdef MTK_ADD_DSD_DOP_SUPPORT
    dsd_fp_array =  (struct audioformat *) kmalloc(sizeof(struct audioformat*)*num, GFP_KERNEL);
    dop_fp_array =  (struct audioformat *) kmalloc(sizeof(struct audioformat*)*num, GFP_KERNEL);
    #endif
    /*
     * Dallas DS4201 workaround: It presents 5 altsettings, but the last
     * one misses syncpipe, and does not produce any sound.
     */
    if (chip->usb_id == USB_ID(0x04fa, 0x4201))
        num = 4;

    for (i = 0; i < num; i++) {
        alts = &iface->altsetting[i];
        altsd = get_iface_desc(alts);
        protocol = altsd->bInterfaceProtocol;
        /* skip invalid one */
        if ((altsd->bInterfaceClass != USB_CLASS_AUDIO &&
             altsd->bInterfaceClass != USB_CLASS_VENDOR_SPEC) ||
            (altsd->bInterfaceSubClass != USB_SUBCLASS_AUDIOSTREAMING &&
             altsd->bInterfaceSubClass != USB_SUBCLASS_VENDOR_SPEC) ||
            altsd->bNumEndpoints < 1 ||
            le16_to_cpu(get_endpoint(alts, 0)->wMaxPacketSize) == 0)
            continue;

        /* must be isochronous */
        if ((get_endpoint(alts, 0)->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) !=
            USB_ENDPOINT_XFER_ISOC)
            continue;
        /* check direction */
        stream = (get_endpoint(alts, 0)->bEndpointAddress & USB_DIR_IN) ?
            SNDRV_PCM_STREAM_CAPTURE : SNDRV_PCM_STREAM_PLAYBACK;
        altno = altsd->bAlternateSetting;

        if (snd_usb_apply_interface_quirk(chip, iface_no, altno))
            continue;

        chconfig = 0;
        /* get audio formats */
        switch (protocol) {
        default:
            snd_printdd(KERN_WARNING "%d:%u:%d: unknown interface protocol %#02x, assuming v1\n",
                    dev->devnum, iface_no, altno, protocol);
            protocol = UAC_VERSION_1;
            /* fall through */

        case UAC_VERSION_1: {
            struct uac1_as_header_descriptor *as =
                snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL, UAC_AS_GENERAL);
            struct uac_input_terminal_descriptor *iterm;

            if (!as) {
                snd_printk(KERN_ERR "%d:%u:%d : UAC_AS_GENERAL descriptor not found\n",
                       dev->devnum, iface_no, altno);
                continue;
            }

            if (as->bLength < sizeof(*as)) {
                snd_printk(KERN_ERR "%d:%u:%d : invalid UAC_AS_GENERAL desc\n",
                       dev->devnum, iface_no, altno);
                continue;
            }

            format = le16_to_cpu(as->wFormatTag); /* remember the format value */

            iterm = snd_usb_find_input_terminal_descriptor(chip->ctrl_intf,
                                       as->bTerminalLink);
            if (iterm) {
                num_channels = iterm->bNrChannels;
                chconfig = le16_to_cpu(iterm->wChannelConfig);
            }

            break;
        }

        case UAC_VERSION_2: {
            struct uac2_input_terminal_descriptor *input_term;
            struct uac2_output_terminal_descriptor *output_term;
            struct uac2_as_header_descriptor *as =
                snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL, UAC_AS_GENERAL);

            if (!as) {
                snd_printk(KERN_ERR "%d:%u:%d : UAC_AS_GENERAL descriptor not found\n",
                       dev->devnum, iface_no, altno);
                continue;
            }

            if (as->bLength < sizeof(*as)) {
                snd_printk(KERN_ERR "%d:%u:%d : invalid UAC_AS_GENERAL desc\n",
                       dev->devnum, iface_no, altno);
                continue;
            }

            num_channels = as->bNrChannels;
            format = le32_to_cpu(as->bmFormats);

            /* lookup the terminal associated to this interface
             * to extract the clock */
            input_term = snd_usb_find_input_terminal_descriptor(chip->ctrl_intf,
                                        as->bTerminalLink);
            if (input_term) {
                clock = input_term->bCSourceID;
                chconfig = le32_to_cpu(input_term->bmChannelConfig);
                break;
            }

            output_term = snd_usb_find_output_terminal_descriptor(chip->ctrl_intf,
                                          as->bTerminalLink);
            if (output_term) {
                clock = output_term->bCSourceID;
                break;
            }

            snd_printk(KERN_ERR "%d:%u:%d : bogus bTerminalLink %d\n",
                   dev->devnum, iface_no, altno, as->bTerminalLink);
            continue;
        }
        }

        /* get format type */
        fmt = snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL, UAC_FORMAT_TYPE);
        if (!fmt) {
            snd_printk(KERN_ERR "%d:%u:%d : no UAC_FORMAT_TYPE desc\n",
                   dev->devnum, iface_no, altno);
            continue;
        }
        if (((protocol == UAC_VERSION_1) && (fmt->bLength < 8)) ||
            ((protocol == UAC_VERSION_2) && (fmt->bLength < 6))) {
            snd_printk(KERN_ERR "%d:%u:%d : invalid UAC_FORMAT_TYPE desc\n",
                   dev->devnum, iface_no, altno);
            continue;
        }

        /*
         * Blue Microphones workaround: The last altsetting is identical
         * with the previous one, except for a larger packet size, but
         * is actually a mislabeled two-channel setting; ignore it.
         */
        if (fmt->bNrChannels == 1 &&
            fmt->bSubframeSize == 2 &&
            altno == 2 && num == 3 &&
            fp && fp->altsetting == 1 && fp->channels == 1 &&
            fp->formats == SNDRV_PCM_FMTBIT_S16_LE &&
            protocol == UAC_VERSION_1 &&
            le16_to_cpu(get_endpoint(alts, 0)->wMaxPacketSize) ==
                            fp->maxpacksize * 2)
            continue;

        fp = kzalloc(sizeof(*fp), GFP_KERNEL);
        if (! fp) {
            snd_printk(KERN_ERR "cannot malloc\n");
            return -ENOMEM;
        }

        fp->iface = iface_no;
        fp->altsetting = altno;
        fp->altset_idx = i;
        fp->endpoint = get_endpoint(alts, 0)->bEndpointAddress;
        fp->ep_attr = get_endpoint(alts, 0)->bmAttributes;
        fp->datainterval = snd_usb_parse_datainterval(chip, alts);
        fp->maxpacksize = le16_to_cpu(get_endpoint(alts, 0)->wMaxPacketSize);
        fp->channels = num_channels;
        if (snd_usb_get_speed(dev) == USB_SPEED_HIGH)
            fp->maxpacksize = (((fp->maxpacksize >> 11) & 3) + 1)
                    * (fp->maxpacksize & 0x7ff);
        fp->attributes = parse_uac_endpoint_attributes(chip, alts, protocol, iface_no);
        fp->clock = clock;
        fp->chmap = convert_chmap(num_channels, chconfig, protocol);

        /* some quirks for attributes here */

        switch (chip->usb_id) {
        case USB_ID(0x0a92, 0x0053): /* AudioTrak Optoplay */
            /* Optoplay sets the sample rate attribute although
             * it seems not supporting it in fact.
             */
            fp->attributes &= ~UAC_EP_CS_ATTR_SAMPLE_RATE;
            break;
        case USB_ID(0x041e, 0x3020): /* Creative SB Audigy 2 NX */
        case USB_ID(0x0763, 0x2003): /* M-Audio Audiophile USB */
            /* doesn't set the sample rate attribute, but supports it */
            fp->attributes |= UAC_EP_CS_ATTR_SAMPLE_RATE;
            break;
        case USB_ID(0x0763, 0x2001):  /* M-Audio Quattro USB */
        case USB_ID(0x0763, 0x2012):  /* M-Audio Fast Track Pro USB */
        case USB_ID(0x047f, 0x0ca1): /* plantronics headset */
        case USB_ID(0x077d, 0x07af): /* Griffin iMic (note that there is
                        an older model 77d:223) */
        /*
         * plantronics headset and Griffin iMic have set adaptive-in
         * although it's really not...
         */
            fp->ep_attr &= ~USB_ENDPOINT_SYNCTYPE;
            if (stream == SNDRV_PCM_STREAM_PLAYBACK)
                fp->ep_attr |= USB_ENDPOINT_SYNC_ADAPTIVE;
            else
                fp->ep_attr |= USB_ENDPOINT_SYNC_SYNC;
            break;
        }

        usb_set_interface(chip->dev, iface_no, altno);

        /* ok, let's parse further... */
        if (snd_usb_parse_audio_format(chip, fp, format, fmt, stream, alts) < 0) {
            kfree(fp->rate_table);
            kfree(fp->chmap);
            kfree(fp);
            fp = NULL;
            continue;
        }
        
		fp->rate_max = cap_capability(fp->rate_max, chip->usb_id, DOP_TYPE);

        snd_printdd(KERN_INFO "%d:%u:%d: add audio endpoint %#x\n", dev->devnum, iface_no, altno, fp->endpoint);
        printk("[USB]%s:%d add stream\n", __FUNCTION__, __LINE__);
        
        ext_fmt = (struct uac_format_type_i_ext_descriptor*)fmt;
		flag_head = false;
		
        if (ext_fmt->bSubslotSize == 4)
        {
        	if (s32_max < ext_fmt->bBitResolution)
        	{
	            s32_max = ext_fmt->bBitResolution;
	            flag_head = true;
			}
        }
        printk("[USB]%s:%d \n ******* alt:%d ext_fmt->bSubslotSize:%d, bBitResolution:%d *******\n",
         	__FUNCTION__, __LINE__, altno, ext_fmt->bSubslotSize, ext_fmt->bBitResolution);

        
        err = snd_usb_add_audio_stream_ex(chip, stream, fp, flag_head);
        if (err < 0) {
            kfree(fp->rate_table);
            kfree(fp->chmap);
            kfree(fp);
            return err;
        }
 
        #ifdef MTK_ADD_DSD_DOP_SUPPORT
        if (!first_valid_fp)
            first_valid_fp = fp;
        #endif

        /* try to set the interface... */
#ifdef MTK_ADD_DSD_DOP_SUPPORT
        printk("[USB]%s:%d format:%020llu format:0x%016llx\n", __FUNCTION__, __LINE__,
            fp->formats, fp->formats);
        printk("[USB]%s:%d interface:%d alt:%d\n", __FUNCTION__, __LINE__,
                        iface_no, altno);

        snd_usb_type = snd_get_dsd_type(chip, alts, fp);
        printk("[USB]%s:%d dsd_type:0x%x\n", __FUNCTION__, __LINE__, snd_usb_type);
        if (snd_usb_type & (DSD_TYPE_B_88_2 | DSD_TYPE_B_176_4 | DSD_TYPE_B_352_8))
        {
			enable_type_b = 1;
			printk("[USB]%s:%d DETECT DSD_TYPE_B\n", __FUNCTION__, __LINE__);
		}
		
        if (snd_usb_type)
        {
            dsd_fp = kzalloc(sizeof(*fp), GFP_KERNEL);
            if (!dsd_fp){
                printk("[USB]%s:%d\n", __FUNCTION__, __LINE__);
                snd_printk(KERN_ERR "cannot malloc\n");
                return -ENOMEM;
            }

            memcpy(dsd_fp, fp, sizeof(*fp));
            if ((snd_usb_type & DSD_TYPE_A_88_2) || (snd_usb_type & DSD_TYPE_B_88_2))
            {
            	dsd_fp->rate_min = 88200;
            	dsd_fp->rate_max = 88200;
            }
            
            if ((snd_usb_type & DSD_TYPE_A_176_4) || (snd_usb_type & DSD_TYPE_B_176_4))
            {
                if (dsd_fp->rate_min != 88200) dsd_fp->rate_min = 176400;
               	dsd_fp->rate_max = 176400;
            }
            
            if ((snd_usb_type & DSD_TYPE_A_352_8) || (snd_usb_type & DSD_TYPE_B_352_8))
            {
                if ((dsd_fp->rate_min != 88200) && (dsd_fp->rate_min != 176400)) dsd_fp->rate_min = 352800;
               	dsd_fp->rate_max = 352800;
            }

            snd_set_dsd_audio_fmt(dev, dsd_fp, snd_usb_type);
            dsd_fp_array[dsd_fp_num++] = dsd_fp;
            printk("[USB]%s:%d add dsd fp:%d\n", __FUNCTION__, __LINE__, dsd_fp_num);
        }

        snd_usb_type = snd_get_dop_type(dev, alts, fp);
        printk("[USB]%s:%d dop type:0x%x\n", __FUNCTION__, __LINE__, snd_usb_type);
        if (snd_usb_type)
        {
            dop_fp = kzalloc(sizeof(*fp), GFP_KERNEL);
            if (!dop_fp){
                printk("[USB]%s:%d\n", __FUNCTION__, __LINE__);
                snd_printk(KERN_ERR "cannot malloc\n");
                return -ENOMEM;
            }

            memcpy(dop_fp, fp, sizeof(*fp));
            if (snd_usb_type & DOP_TYPE_88_2)
            {
            	dop_fp->rate_min = 88200;
            	dop_fp->rate_max = 88200;
            }
            
            if (snd_usb_type & DOP_TYPE_176_4)
            {
                if (dop_fp->rate_min != 88200) dop_fp->rate_min = 176400;
               	dop_fp->rate_max = 176400;
            }

            snd_set_dop_audio_fmt(dev, dop_fp, snd_usb_type);
            dop_fp_array[dop_fp_num++] = dop_fp;
            printk("[USB]%s:%d add dop fp:%d\n", __FUNCTION__, __LINE__, dop_fp_num);
        }
#endif
        snd_usb_init_pitch(chip, iface_no, alts, fp);
        snd_usb_init_sample_rate(chip, iface_no, alts, fp, fp->rate_max);
    }
    #ifdef MTK_ADD_DSD_DOP_SUPPORT

    if(dsd_fp_num > 0){
        printk("[USB]%s:%d dsd fp count:%d\n", __FUNCTION__, __LINE__,
                dsd_fp_num);
        for (i=0; i<dsd_fp_num; i++){
            if (enable_type_b && (dsd_fp_array[i]->dsd_dop_type & (DSD_TYPE_A_88_2 | DSD_TYPE_A_176_4 | DSD_TYPE_A_352_8)))
            {
            	printk("[USB]%s:%d skip stream\n", __FUNCTION__, __LINE__);
			}
			else
			{
            	printk("[USB]%s:%d add stream\n", __FUNCTION__, __LINE__);
            	err = snd_usb_add_audio_stream(chip, SNDRV_PCM_STREAM_PLAYBACK, dsd_fp_array[i]);
                if (err < 0) {
                    printk("[USB]%s:%d add dsd fp fail\n", __FUNCTION__, __LINE__);
                    kfree(dsd_fp_array[i]->rate_table);
                    kfree(dsd_fp_array[i]->chmap);
                    kfree(dsd_fp_array[i]);
                    return err;
                }
                snd_card->dsd_count++;
            }
        }
    }

    if((iface_no - chip->first_intf_num + 1) == chip->assoc_intf_num) {
        if(snd_card->dsd_count == 0){
            printk("[USB]%s:%d dsd fp count 0\n", __FUNCTION__, __LINE__);
            dsd_fp = kzalloc(sizeof(*fp), GFP_KERNEL);
            if (!dsd_fp){
                printk("[USB]%s:%d\n", __FUNCTION__, __LINE__);
                snd_printk(KERN_ERR "cannot malloc\n");
                return -ENOMEM;
            }

            if (first_valid_fp) {
                memcpy(dsd_fp, first_valid_fp, sizeof(*first_valid_fp));
            } else {
                printk("[USB]%s:%d no valid audio fp", __FUNCTION__, __LINE__);
                kfree(dsd_fp);
                return -ENODATA;
            }

            snd_force_create_dsd_node(dev, dsd_fp);
            //print_audio_format(dsd_fp);
            printk("[USB]%s:%d add stream\n", __FUNCTION__, __LINE__);
            err = snd_usb_add_audio_stream(chip, SNDRV_PCM_STREAM_PLAYBACK, dsd_fp);
            if (err < 0) {
                printk("[USB]%s:%d add dsd fp fail\n", __FUNCTION__, __LINE__);
                kfree(dsd_fp->rate_table);
                kfree(dsd_fp->chmap);
                kfree(dsd_fp);
                return err;
            }
            snd_card->dsd_count++;
        }
    }

    if(dop_fp_num > 0) {
        printk("[USB]%s:%d dop fp count:%d\n", __FUNCTION__, __LINE__,
                dop_fp_num);
        for (i=0; i<dop_fp_num; i++){
            printk("[USB]%s:%d add stream\n", __FUNCTION__, __LINE__);
            err = snd_usb_add_audio_stream(chip, SNDRV_PCM_STREAM_PLAYBACK, dop_fp_array[i]);
            if (err < 0) {
                printk("[USB]%s:%d add dop fp fail\n", __FUNCTION__, __LINE__);
                kfree(dop_fp_array[i]->rate_table);
                kfree(dop_fp_array[i]->chmap);
                kfree(dop_fp_array[i]);
                return err;
            }
            snd_card->dop_count++;
        }
    }
    printk("[USB]inface_no:%d assoc_intf_num:%d first_intf_num:%d\n",
            iface_no,
            chip->assoc_intf_num,
            chip->first_intf_num);
    if((iface_no - chip->first_intf_num + 1) == chip->assoc_intf_num){
        if(snd_card->dop_count == 0){
            printk("[USB]%s:%d dop fp count 0\n", __FUNCTION__, __LINE__);
            dop_fp = kzalloc(sizeof(*fp), GFP_KERNEL);
            if (!dop_fp){
                printk("[USB]%s:%d\n", __FUNCTION__, __LINE__);
                snd_printk(KERN_ERR "cannot malloc\n");
                return -ENOMEM;
            }

            if (first_valid_fp) {
                memcpy(dop_fp, first_valid_fp, sizeof(*first_valid_fp));
            } else {
                printk("[USB]%s:%d no valid audio fp", __FUNCTION__, __LINE__);
                kfree(dop_fp);
                return -ENODATA;
            }
 
            snd_force_create_dop_node(dev, dop_fp);
            printk("[USB]%s:%d add stream\n", __FUNCTION__, __LINE__);
            err = snd_usb_add_audio_stream(chip, SNDRV_PCM_STREAM_PLAYBACK, dop_fp);
            if (err < 0) {
                printk("[USB]%s:%d add dop_type fail\n", __FUNCTION__, __LINE__);
                kfree(dop_fp->rate_table);
                kfree(dop_fp->chmap);
                kfree(dop_fp);
                return err;
            }
            snd_card->dop_count++;
        }
    }

    #endif
    kfree(dsd_fp_array);
    dsd_fp_array = NULL;
    kfree(dop_fp_array);
    dop_fp_array = NULL;

    return 0;
}

#ifdef MTK_ADD_DSD_DOP_SUPPORT
unsigned int snd_get_dsd_type(struct snd_usb_audio *chip,
                                    struct usb_host_interface *alts,
                                    struct audioformat *fp)
{
    struct uac_format_type_i_ext_descriptor *ext_fmt;
    struct usb_interface_descriptor *altsd;
    int stream, i, err;
    unsigned int dsd_type = 0;
    unsigned int check_dsd_type = 0;
    struct uac_format_type_i_continuous_descriptor *fmt;
    struct uac2_as_header_descriptor *as;
    unsigned int format;
    __le32 data;
    struct usb_device *dev = chip->dev;
    unsigned int rate;
   	unsigned int rate_max = cap_capability(fp->rate_max, chip->usb_id, DSD_TYPE);
	printk("rate_max = %d\n",rate_max);


    altsd = get_iface_desc(alts);
    stream = (get_endpoint(alts, 0)->bEndpointAddress & USB_DIR_IN) ?
            SNDRV_PCM_STREAM_CAPTURE : SNDRV_PCM_STREAM_PLAYBACK;
    fmt = snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL, UAC_FORMAT_TYPE);
    as = snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL, UAC_AS_GENERAL);
    format = le32_to_cpu(as->bmFormats);
    if ((altsd->bInterfaceProtocol == UAC_VERSION_2) &&
       (fmt->bFormatType != UAC_FORMAT_TYPE_III)    &&
       (stream == SNDRV_PCM_STREAM_PLAYBACK)){
            ext_fmt = (struct uac_format_type_i_ext_descriptor*)fmt;
            #if 1
            if (ext_fmt->bBitResolution != 32){
                printk("[USB]%s:%d bitResolution:%d continue\n", __FUNCTION__, __LINE__,
                        ext_fmt->bBitResolution);
                return 0;
            }
            #else
            printk("[USB]%s:%d bitResolution:%d bSubslotSize:%d, raw bit:%d\n", __FUNCTION__, __LINE__,
                        ext_fmt->bBitResolution,
                        ext_fmt->bSubslotSize,
                        (format & UAC2_FORMAT_TYPE_I_RAW_DATA));
            #endif

            //check RAW Data bit
            if (format & UAC2_FORMAT_TYPE_I_RAW_DATA){
                printk("[USB]KNOT DSD_B bit inter:%d alt:%d num:%d min:%d max:%d\n",
                            altsd->bInterfaceNumber,
                            altsd->bAlternateSetting,
                            fp->rates,
                            fp->rate_min,
                            fp->rate_max);
                for (i=0; i< fp->nr_rates; i++){
                    if ((fp->rate_table[i] == 88200) && (rate_max >= 88200)){
                        printk("[USB]%s:%d DSD TYPE B:88200\n", __FUNCTION__, __LINE__);
                        dsd_type |= DSD_TYPE_B_88_2;
                    }
                    if ((fp->rate_table[i] == 176400) && (rate_max >= 176400)){
                        printk("[USB]%s:%d DSD TYPE B:176400\n", __FUNCTION__, __LINE__);
                        dsd_type |= DSD_TYPE_B_176_4;
                    }
                    if ((fp->rate_table[i] == 352800) && (rate_max >= 352800)){
                        printk("[USB]%s:%d DSD TYPE B:352800\n", __FUNCTION__, __LINE__);
                        dsd_type |= DSD_TYPE_B_352_8;
                    }
                }
            }else {
                unsigned int i;
                if (! (fp->rates & SNDRV_PCM_RATE_CONTINUOUS)) {
                    printk("[USB]KNOT inter:%d alt:%d num:%d min:%d max:%d\n",
                            altsd->bInterfaceNumber,
                            altsd->bAlternateSetting,
                            fp->nr_rates,
                            fp->rate_min,
                            fp->rate_max);
                    for (i = 0; i < fp->nr_rates; i++){
                        printk("Rate:%d\n", fp->rate_table[i]);
                    }
                }else{
                    printk("[USB]CONTINUOUS inter:%d alt:%d\n",
                            altsd->bInterfaceNumber,
                            altsd->bAlternateSetting);
                }

                for (i = 0; i < fp->nr_rates; i++){
                    if ((fp->rate_table[i] == 88200) && (rate_max >= 88200))
                        check_dsd_type |= DSD_TYPE_A_88_2;
                    if ((fp->rate_table[i] == 176400) && (rate_max >= 176400))
                        check_dsd_type |= DSD_TYPE_A_176_4;
                    if ((fp->rate_table[i] == 352800) && (rate_max >= 352800))
                        check_dsd_type |= DSD_TYPE_A_352_8;
                }

                printk("[USB]%s:%d interface:%d alt:%d\n", __FUNCTION__, __LINE__,
                                            fp->iface, fp->altsetting);
                /*
                    In-band command value:
                        0: PCM
                        1: DSD
                    DSD64 -- 88200
                        Dont care the in-band command set PCM/DSD
                    DSD128 -- 176400
                    DSD256 -- 352800
                */

                /*dont do set interface here, move to snd_usb_parse_audio_interface
                    and before snd_get_dsd_type API
                */
                if (check_dsd_type & DSD_TYPE_A_88_2){
                    //set sample rate 88200
                    data = cpu_to_le32(88200);
                    err = snd_usb_ctl_msg(dev, usb_sndctrlpipe(dev, 0), UAC2_CS_CUR,
                            USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT,
                            UAC2_CS_CONTROL_SAM_FREQ << 8,
                            snd_usb_ctrl_intf(chip) | (fp->clock << 8),
                            &data, sizeof(data));
                    //send in-band command & value 0
	                usb_set_interface(chip->dev, fp->iface, 0);
                    err = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                            0, 0x41, 0, fp->iface, NULL, 0, 1000);
                    if (err < 0){//NG, dont support DSD64, goto check done
                        if (err == -32){//pipe stall
                            printk("[USB]%s:%d stall\n", __FUNCTION__, __LINE__);
                        }else{//echo the error msg
                            printk("[USB]%s error msg:%d\n", __FUNCTION__, err);
                        }
                        return dsd_type;
                    }else //OK, support DSD64,continue
                    {
                        dsd_type |= DSD_TYPE_A_88_2;
 					}
                }else
                    goto DSD_CHECK_DONE;
                

                if (check_dsd_type & DSD_TYPE_A_176_4){
                //set sample rate 176400
	                data = cpu_to_le32(176400);
	                err = snd_usb_ctl_msg(dev, usb_sndctrlpipe(dev, 0), UAC2_CS_CUR,
	                                  USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT,
	                                  UAC2_CS_CONTROL_SAM_FREQ << 8,
	                                  snd_usb_ctrl_intf(chip) | (fp->clock << 8),
	                                  &data, sizeof(data));
	                //send in-band command & value 1
	                err = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                                    0, 0x41, 1, fp->iface, NULL, 0, 1000);
                    if (err < 0){//NG, dont support DSD128, goto check done
                        if (err == -32){//pipe stall
                            printk("[USB]%s:%d stall\n", __FUNCTION__, __LINE__);
                        }else{//echo the error msg
                            printk("[USB]%s error msg:%d\n", __FUNCTION__, err);
                        }
                        goto DSD_CHECK_DONE;
                    }else //OK, support DSD176400,continue
                    {
                        dsd_type |= DSD_TYPE_A_176_4;
                    }
                }else
                    goto DSD_CHECK_DONE;

                if (check_dsd_type & DSD_TYPE_A_352_8){
                    //set sample rate 352800
                    data = cpu_to_le32(352800);
                    err = snd_usb_ctl_msg(dev, usb_sndctrlpipe(dev, 0), UAC2_CS_CUR,
                                      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT,
                                      UAC2_CS_CONTROL_SAM_FREQ << 8,
                                      snd_usb_ctrl_intf(chip) | (fp->clock << 8),
                                      &data, sizeof(data));
                    //send in-band command & value 1
                    err = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                                        0, 0x41, 1, fp->iface, NULL, 0, 1000);
                    if (err < 0){//NG, dont support DSD64, goto check done
                        if (err == -32){//pipe stall
                            printk("[USB]%s:%d stall\n", __FUNCTION__, __LINE__);
                        }else{//echo the error msg
                            printk("[USB]%s error msg:%d\n", __FUNCTION__, err);
                        }
                        goto DSD_CHECK_DONE;
                    }else //OK, support DSD256,continue
                    {
                        dsd_type |= DSD_TYPE_A_352_8;
                    }
                }

                //send in-band command, return the device to PCM mode
                DSD_CHECK_DONE:

                printk("[USB]KNOT DSD_A bit inter:%d alt:%d num:%d min:%d max:%d\n",
                            altsd->bInterfaceNumber,
                            altsd->bAlternateSetting,
                            fp->rates,
                            fp->rate_min,
                            fp->rate_max);

                err = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                                0, 0x41, 0, fp->iface, NULL, 0, 1000);  
        }
    }
    return dsd_type;
}

unsigned int snd_set_dsd_audio_fmt(struct usb_device *dev, 
                                    struct audioformat *fp, unsigned int dsd_type)
{
    int rates_count = 0;
    unsigned int rate_cur = 0;
    unsigned int rate_min = 0;
    unsigned int rate_max = 0;
    struct snd_pcm_chmap_elem *chmap;

    printk("[USB]%s:%d fp:0x%p chmap:0x%p size:%d\n", __FUNCTION__, __LINE__,
            fp, fp->chmap, sizeof(*chmap));
    if(fp->chmap){
        chmap = kzalloc(sizeof(*chmap), GFP_KERNEL);
        if (!chmap)
            printk("[USB]%s:%d allocate rate table error.\n", __FUNCTION__, __LINE__);
        memcpy((void*)chmap, (void*)fp->chmap, sizeof(*chmap));
        fp->chmap = chmap;
        printk("[USB]%s:%d fp:0x%p chmap:0x%p\n", __FUNCTION__, __LINE__,
            fp, fp->chmap);
    }

    fp->fmt_type = UAC_EXT_FORMAT_TYPE_I;
    fp->rate_table = kmalloc(sizeof(int) * 3, GFP_KERNEL);
    fp->formats = pcm_format_to_bits(SNDRV_PCM_FORMAT_DSD_U8);
    fp->rates = SNDRV_PCM_RATE_KNOT;
    fp->dsd_dop_type |= dsd_type;

    
    if (fp->dsd_dop_type & (DSD_TYPE_A_88_2 | DSD_TYPE_B_88_2)){
        fp->rates |= SNDRV_PCM_RATE_88200;
        rate_cur = 88200;
        
        if (rate_min == 0)
            rate_min = rate_cur;
        else if (rate_cur < rate_min)
            rate_min = rate_cur;

        if (rate_cur > rate_max)
            rate_max= rate_cur;

        fp->rate_table[rates_count++] = rate_cur;
    }
    if (fp->dsd_dop_type & (DSD_TYPE_A_176_4 | DSD_TYPE_B_176_4)){
        fp->rates |= SNDRV_PCM_RATE_176400;
        rate_cur = 176400;
        
        if (rate_min == 0)
            rate_min = rate_cur;
        else if (rate_cur < rate_min)
            rate_min = rate_cur;

        if (rate_cur > rate_max)
            rate_max= rate_cur;

        fp->rate_table[rates_count++] = rate_cur;
    }
    if (fp->dsd_dop_type & (DSD_TYPE_A_352_8 | DSD_TYPE_B_352_8)){
        fp->rates |= SNDRV_PCM_RATE_352800;
        rate_cur = 352800;
        
        if (rate_min == 0)
            rate_min = rate_cur;
        else if (rate_cur < rate_min)
            rate_min = rate_cur;

        if (rate_cur > rate_max)
            rate_max= rate_cur;

        fp->rate_table[rates_count++] = rate_cur;
    }
    
    fp->nr_rates = rates_count;
    printk("[USB]%s:%d dsd type:0x%x count:%d rate_min:%d rate_max:%d\n", __FUNCTION__, __LINE__, 
                fp->dsd_dop_type, fp->nr_rates, fp->rate_min, fp->rate_max);
    return 0;
}
unsigned int snd_get_dop_type(struct usb_device *dev, 
                                    struct usb_host_interface *alts,
                                    struct audioformat *fp)
{
    struct uac_format_type_i_ext_descriptor *ext_fmt;
    struct usb_interface_descriptor *altsd;
    int stream, i, err;
    unsigned int dop_type = 0;
    unsigned int format;
    struct uac2_as_header_descriptor *as;
    struct uac_format_type_i_continuous_descriptor *fmt;
    
    altsd = get_iface_desc(alts);
    stream = (get_endpoint(alts, 0)->bEndpointAddress & USB_DIR_IN) ?
            SNDRV_PCM_STREAM_CAPTURE : SNDRV_PCM_STREAM_PLAYBACK;
    fmt = snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL, UAC_FORMAT_TYPE);
    as = snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL, UAC_AS_GENERAL);
    format = le32_to_cpu(as->bmFormats);

    if ((altsd->bInterfaceProtocol == UAC_VERSION_2) &&
       (fmt->bFormatType != UAC_FORMAT_TYPE_III) &&
       (stream == SNDRV_PCM_STREAM_PLAYBACK)){
            ext_fmt = (struct uac_format_type_i_ext_descriptor*)fmt;
            //Check and Create DOP device
            printk("[USB]%s:%d DOP bitResolution:%d bSubslotSize:%d RAW_DATA 0x:%x\n", __FUNCTION__, __LINE__,
                    ext_fmt->bBitResolution, ext_fmt->bSubslotSize, (format & UAC2_FORMAT_TYPE_I_RAW_DATA));
            if (ext_fmt->bBitResolution >= 24 &&
                ext_fmt->bSubslotSize == 4 &&
                format & UAC_FORMAT_TYPE_I_PCM){//support DOP Format
                for (i = 0; i < fp->nr_rates; i++){
                    if (fp->rate_table[i] == 176400){
                        printk("[USB]%s:%d DOP TYPE rate:88200\n", __FUNCTION__, __LINE__);
                        dop_type |= DOP_TYPE_88_2;
                    }
                    if (fp->rate_table[i] == 352800){
                        printk("[USB]%s:%d DOP TYPE rate:176400\n", __FUNCTION__, __LINE__);
                        dop_type |= DOP_TYPE_176_4;
                    }
                }
            }   
    }
    
    return dop_type;
}

unsigned int snd_set_dop_audio_fmt(struct usb_device *dev, 
                                    struct audioformat *fp, unsigned int dop_type)
{
    int rates_count = 0;
    unsigned int rate_cur = 0;
    unsigned int rate_min = 0;
    unsigned int rate_max = 0;
    struct snd_pcm_chmap_elem *chmap;

    printk("[USB]%s:%d fp:0x%p chmap:0x%p size:%d\n", __FUNCTION__, __LINE__,
            fp, fp->chmap, sizeof(*chmap));
    if(fp->chmap){
        chmap = kzalloc(sizeof(*chmap), GFP_KERNEL);
        if (!chmap)
            printk("[USB]%s:%d allocate rate table error.\n", __FUNCTION__, __LINE__);
        memcpy((void*)chmap, (void*)fp->chmap, sizeof(*chmap));
        fp->chmap = chmap;
        printk("[USB]%s:%d fp:0x%p chmap:0x%p\n", __FUNCTION__, __LINE__,
            fp, fp->chmap);
    }
    fp->rate_table = kmalloc(sizeof(int) * 3, GFP_KERNEL);
    if(!fp->rate_table)
        printk("[USB]%s:%d allocate rate table error.\n", __FUNCTION__, __LINE__);
    fp->fmt_type = UAC_EXT_FORMAT_TYPE_II;
    fp->formats = pcm_format_to_bits(SNDRV_PCM_FORMAT_DSD_U8);
    fp->rates = SNDRV_PCM_RATE_KNOT;
    fp->dsd_dop_type |= dop_type;
    fp->dsd_dop = true;
    
    if (fp->dsd_dop_type & DOP_TYPE_88_2){
        fp->rates |= SNDRV_PCM_RATE_88200;
        rate_cur = 88200;
        
        if (rate_min == 0)
            rate_min = rate_cur;
        else if (rate_cur < rate_min)
            rate_min = rate_cur;

        if (rate_cur > rate_max)
            rate_max= rate_cur;

        fp->rate_table[rates_count++] = rate_cur;
    }
    
    if (fp->dsd_dop_type & DOP_TYPE_176_4){
        fp->rates |= SNDRV_PCM_RATE_176400;
        rate_cur = 176400;
        
        if (rate_min == 0)
            rate_min = rate_cur;
        else if (rate_cur < rate_min)
            rate_min = rate_cur;

        if (rate_cur > rate_max)
            rate_max= rate_cur;

        fp->rate_table[rates_count++] = rate_cur;
    }

    if (fp->dsd_dop_type & DOP_TYPE_352_8){
        fp->rates |= SNDRV_PCM_RATE_352800;
        rate_cur = 352800;
        
        if (rate_min == 0)
            rate_min = rate_cur;
        else if (rate_cur < rate_min)
            rate_min = rate_cur;

        if (rate_cur > rate_max)
            rate_max= rate_cur;

        fp->rate_table[rates_count++] = rate_cur;
    }

    fp->nr_rates = rates_count;
    fp->rate_min = rate_min;
    fp->rate_max = rate_max;
    printk("[USB]%s:%d dop type:0x%x count:%d rate_min:%d rate_max:%d\n", __FUNCTION__, __LINE__, 
                fp->dsd_dop_type, fp->nr_rates, fp->rate_min, fp->rate_max);
    return 0;
}

unsigned int snd_force_create_dop_node(struct usb_device *dev, struct audioformat *fp)
{
    int rates_count = 0;
    unsigned int rate_cur = 0;
    unsigned int rate_min = 0;
    unsigned int rate_max = 0;

    //kfree(fp->rate_table);
    fp->fmt_type = UAC_EXT_FORMAT_TYPE_II;
    fp->rate_table = kmalloc(sizeof(int) * 1, GFP_KERNEL);
    fp->formats = pcm_format_to_bits(SNDRV_PCM_FORMAT_DSD_U8);
    fp->rates = SNDRV_PCM_RATE_KNOT;

    fp->rate_table[0] = 5512;
    fp->nr_rates = 1;
    fp->rate_min = 5512;
    fp->rate_max = 5512;
    printk("[USB]%s:%d count:%d rate_min:%d rate_max:%d\n", __FUNCTION__, __LINE__, 
                fp->nr_rates, fp->rate_min, fp->rate_max);
    return 0;
}

unsigned int snd_force_create_dsd_node(struct usb_device *dev, struct audioformat *fp)
{
    int rates_count = 0;
    unsigned int rate_cur = 0;
    unsigned int rate_min = 0;
    unsigned int rate_max = 0;
    //kfree(fp->rate_table);
    fp->fmt_type = UAC_EXT_FORMAT_TYPE_I;
    fp->rate_table = kmalloc(sizeof(int) * 1, GFP_KERNEL);
    fp->formats = pcm_format_to_bits(SNDRV_PCM_FORMAT_DSD_U8);
    fp->rates = SNDRV_PCM_RATE_KNOT;

    fp->rate_table[0] = 5512;
    fp->nr_rates = 1;
    fp->rate_min = 5512;
    fp->rate_max = 5512;
    printk("[USB]%s:%d count:%d rate_min:%d rate_max:%d\n", __FUNCTION__, __LINE__, 
                fp->nr_rates, fp->rate_min, fp->rate_max);
    return 0;
}

#endif
