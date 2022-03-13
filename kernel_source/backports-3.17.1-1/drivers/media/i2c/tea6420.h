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
#ifndef __INCLUDED_TEA6420__
#define __INCLUDED_TEA6420__

/* input pins */
#define TEA6420_OUTPUT1 1
#define TEA6420_OUTPUT2 2
#define TEA6420_OUTPUT3 3
#define TEA6420_OUTPUT4 4

/* output pins */
#define TEA6420_INPUT1 1
#define TEA6420_INPUT2 2
#define TEA6420_INPUT3 3
#define TEA6420_INPUT4 4
#define TEA6420_INPUT5 5
#define TEA6420_INPUT6 6

/* gain on the output pins, ORed with the output pin */
#define TEA6420_GAIN0 0x00
#define TEA6420_GAIN2 0x20
#define TEA6420_GAIN4 0x40
#define TEA6420_GAIN6 0x60

#endif
