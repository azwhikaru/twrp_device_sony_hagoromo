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
#ifndef RTL8187_RFKILL_H
#define RTL8187_RFKILL_H

void rtl8187_rfkill_init(struct ieee80211_hw *hw);
void rtl8187_rfkill_poll(struct ieee80211_hw *hw);
void rtl8187_rfkill_exit(struct ieee80211_hw *hw);

#endif /* RTL8187_RFKILL_H */
