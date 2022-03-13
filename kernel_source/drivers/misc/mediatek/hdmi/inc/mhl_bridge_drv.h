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
typedef enum{
    MHLTX_CONNECT_NO_DEVICE,
    HDMITX_CONNECT_ACTIVE,
}MHLTX_CONNECT_STATE;

typedef struct
{
    int (*init)(void);
    int (*enter)(void);
    int (*exit)(void);
    void (*suspend)(void);
    void (*resume)(void);
    void  (*power_on)(void);
    void (*power_off)(void);
    MHLTX_CONNECT_STATE (*get_state)(void);
    void (*debug)(unsigned char *pcmdbuf);
    void (*enablehdcp)(unsigned char u1hdcponoff);
    int (*getedid)(unsigned char *pedidbuf);
    void (*mutehdmi)(unsigned char enable);
    unsigned char (*getppmodesupport)(void);
    void (*resolution)(unsigned char res,unsigned char cs);
} MHL_BRIDGE_DRIVER;

const MHL_BRIDGE_DRIVER* MHL_Bridge_GetDriver(void);
