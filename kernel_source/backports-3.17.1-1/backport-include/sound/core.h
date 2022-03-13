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
#ifndef _BACKPORT_SOUND_CORE_H
#define _BACKPORT_SOUND_CORE_H
#include_next <sound/core.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
#define snd_card_new LINUX_BACKPORT(snd_card_new)
static inline
int snd_card_new(struct device *parent, int idx, const char *xid,
		 struct module *module, int extra_size,
		 struct snd_card **card_ret)
{
	int ret;

	ret = snd_card_create(idx, xid, module, extra_size, card_ret);
	snd_card_set_dev(*card_ret, parent);
	return ret;
}
#endif

#endif /* _BACKPORT_SOUND_CORE_H */
