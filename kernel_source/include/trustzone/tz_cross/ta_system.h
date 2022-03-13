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
/** Commands for TA SYSTEM **/

#ifndef __TRUSTZONE_TA_SYSTEM__
#define __TRUSTZONE_TA_SYSTEM__

/* / Special handle for system connect. */
/* / NOTE: Handle manager guarantee normal handle will have bit31=0. */
#define MTEE_SESSION_HANDLE_SYSTEM 0xFFFF1234



/* Session Management */
#define TZCMD_SYS_INIT                0
#define TZCMD_SYS_SESSION_CREATE      1
#define TZCMD_SYS_SESSION_CLOSE       2
#define TZCMD_SYS_IRQ                 3
#define TZCMD_SYS_THREAD_CREATE       4


#endif				/* __TRUSTZONE_TA_SYSTEM__ */
