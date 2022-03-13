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
/*-----------------------------------------------------------------------------
 *
 * Description:
 *
 *---------------------------------------------------------------------------*/

#ifndef X_ASSERT_H
#define X_ASSERT_H
//#define MEMORYZONE_FOUND_ALLOCATOR_BDP
//#define MEMORYZONE_THRSHD_BDP 6   /*512k memory*/

#include "x_typedef.h"

EXTERN void Assert(const CHAR* szExpress, const CHAR* szFile, INT32 i4Line);
#define NDEBUG
#ifndef NDEBUG

	#define ASSERT(x)		((x) ? (void)0 : Assert(#x, __FILE__, (INT32)__LINE__))
	#define VERIFY(x)		((x) ? (void)0 : Assert(#x, __FILE__, (INT32)__LINE__))

#else	// NDEBUG

	#define ASSERT(x)		((void)0)

    // Keep the same behavior as debug version
//	#define VERIFY(x)		((x) ? (void)0 : Assert(#x, __FILE__, (INT32)__LINE__))
	#define VERIFY(x)		((void)(x))

#endif	// NDEBUG

#endif	// X_ASSERT_H
