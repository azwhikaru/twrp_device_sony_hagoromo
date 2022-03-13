/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
#ifndef INCLUDED_SOUND_CORE_ICX_H
#define INCLUDED_SOUND_CORE_ICX_H

/* Sound core header for ICX platform.
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
 *
 */

#include <linux/semaphore.h>		/* semaphore */
#include <linux/switch.h>

/*! linux/switch (android connect/disconnect event notifier)
    event value.
*/
enum switch_state {
	SND_SWITCH_STATE_UNKNOWN = -1,
	SND_SWITCH_STATE_DISCONNECTED = 0,
	SND_SWITCH_STATE_CONNECTED = 1
};


#define	SND_SWITCH_FLAGS_INITIALIZED	(0x00)

struct snd_switch_dev {
	unsigned long		flags;
	struct semaphore	sem;
	struct switch_dev	sw;
};

#endif /* INCLUDED_SOUND_CORE_ICX_H */
