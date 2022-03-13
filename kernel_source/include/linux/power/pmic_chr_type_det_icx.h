/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/* BQ24262 battery charger controller with WM-PORT.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef PMIC_CHR_TYPE_DET_ICX_H_INCLUDED
#define PMIC_CHR_TYPE_DET_ICX_H_INCLUDED

/*! USB host and USB-AC Charger types.
    @note _APL_1R0, _APL_2R0 are not used, they are treated as _APL_0R5
*/
#define	ICX_CHARGER_UNKNOWN	(0)	/*!< Unknown (not yet detected) */
#define	ICX_CHARGER_STD		(1)	/*!< Standard Host Port. */
#define	ICX_CHARGER_CDP		(2)	/*!< Charging Host Port. */
#define	ICX_CHARGER_DCP		(3)	/*!< USB-AC Standard Charger. */
#define	ICX_CHARGER_APL_0R5	(4)	/*!< USB-AC apple 0.5A charger. */
#define	ICX_CHARGER_APL_1R0	(5)	/*!< USB-AC apple 1.0A charger. (not used) */
#define	ICX_CHARGER_APL_2R1	(6)	/*!< USB-AC apple 2.1A charger. (not used) */
#define	ICX_CHARGER_AC_S508U	(7)	/*!< USB-AC sony AC-S508U. */
#define	ICX_CHARGER_MISC_OPEN	(8)	/*!< Handmade USB-AC adaptor, DP and DM are open. */
#define	ICX_CHARGER_MISC_XXX	(9)	/*!< Handmade USB-AC adaptor, unknown. */
#define	ICX_CHARGER_NUMS	(10)	/*!< the number of charger types. */

void mt6323_bcdet_init(void);
void mt6323_bcdet_done(void);

unsigned mt6323_bcdet_type_detection(void);
bool mt6323_bcdet_is_contact_dpdm(void);


#endif /* PMIC_CHR_TYPE_DET_ICX_H_INCLUDED */
