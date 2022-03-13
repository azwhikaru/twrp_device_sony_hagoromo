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
#ifndef __BACKPORT_PCMCIA_DEVICE_ID_H
#define __BACKPORT_PCMCIA_DEVICE_ID_H
#include_next <pcmcia/device_id.h>

#ifndef PCMCIA_DEVICE_MANF_CARD_PROD_ID3
#define PCMCIA_DEVICE_MANF_CARD_PROD_ID3(manf, card, v3, vh3) { \
	.match_flags = PCMCIA_DEV_ID_MATCH_MANF_ID| \
			PCMCIA_DEV_ID_MATCH_CARD_ID| \
			PCMCIA_DEV_ID_MATCH_PROD_ID3, \
	.manf_id = (manf), \
	.card_id = (card), \
	.prod_id = { NULL, NULL, (v3), NULL }, \
	.prod_id_hash = { 0, 0, (vh3), 0 }, }
#endif

#ifndef PCMCIA_DEVICE_PROD_ID3
#define PCMCIA_DEVICE_PROD_ID3(v3, vh3) { \
	.match_flags = PCMCIA_DEV_ID_MATCH_PROD_ID3, \
	.prod_id = { NULL, NULL, (v3), NULL },  \
	.prod_id_hash = { 0, 0, (vh3), 0 }, }
#endif

#endif /* __BACKPORT_PCMCIA_DEVICE_ID_H */
