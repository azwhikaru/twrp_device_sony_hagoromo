/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* ICX key resume helper: ICX key driver common header.
 * 
 *
 * Copyright 2015 Sony Corporation.
 * Author: Sony Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if defined(CONFIG_ENABLE_ICX_KEY)

#if (!defined(ICX_KEY_RESUME_HELPER_HEADER_INCLUDED))
#define ICX_KEY_RESUME_HELPER_HEADER_INCLUDED

#define NUM_KEY_AD_CH       2

#include <linux/types.h>
#include <mach/eint.h>

/*! Copy of status when the system resumed */
struct icx_key_spm_stat {
  uint32_t  key_ad_sta[NUM_KEY_AD_CH];  /*!< ad key status at resume. */
};

/* declared in mt_spm_sleep.c */
extern struct icx_key_spm_stat  icx_key_spm_stat;

#endif /* (!defined(ICX_KEY_RESUME_HELPER_HEADER_INCLUDED)) */

#endif /* CONFIG_ENABLE_ICX_KEY */
