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
#ifndef __BACKPORT_CHECKS
#define __BACKPORT_CHECKS

#if defined(CPTCFG_MAC80211) && defined(CPTCFG_MAC80211)
#error "You must not have mac80211 built into your kernel if you want to enable it"
#endif

#if defined(CPTCFG_CFG80211) && defined(CPTCFG_CFG80211)
#error "You must not have cfg80211 built into your kernel if you want to enable it"
#endif

#endif /* __BACKPORT_CHECKS */
