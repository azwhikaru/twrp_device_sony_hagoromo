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
#ifndef B43legacy_ILT_H_
#define B43legacy_ILT_H_

#define B43legacy_ILT_ROTOR_SIZE	53
extern const u32 b43legacy_ilt_rotor[B43legacy_ILT_ROTOR_SIZE];
#define B43legacy_ILT_RETARD_SIZE	53
extern const u32 b43legacy_ilt_retard[B43legacy_ILT_RETARD_SIZE];
#define B43legacy_ILT_FINEFREQA_SIZE	256
extern const u16 b43legacy_ilt_finefreqa[B43legacy_ILT_FINEFREQA_SIZE];
#define B43legacy_ILT_FINEFREQG_SIZE	256
extern const u16 b43legacy_ilt_finefreqg[B43legacy_ILT_FINEFREQG_SIZE];
#define B43legacy_ILT_NOISEA2_SIZE	8
extern const u16 b43legacy_ilt_noisea2[B43legacy_ILT_NOISEA2_SIZE];
#define B43legacy_ILT_NOISEA3_SIZE	8
extern const u16 b43legacy_ilt_noisea3[B43legacy_ILT_NOISEA3_SIZE];
#define B43legacy_ILT_NOISEG1_SIZE	8
extern const u16 b43legacy_ilt_noiseg1[B43legacy_ILT_NOISEG1_SIZE];
#define B43legacy_ILT_NOISEG2_SIZE	8
extern const u16 b43legacy_ilt_noiseg2[B43legacy_ILT_NOISEG2_SIZE];
#define B43legacy_ILT_NOISESCALEG_SIZE	27
extern const u16 b43legacy_ilt_noisescaleg1[B43legacy_ILT_NOISESCALEG_SIZE];
extern const u16 b43legacy_ilt_noisescaleg2[B43legacy_ILT_NOISESCALEG_SIZE];
extern const u16 b43legacy_ilt_noisescaleg3[B43legacy_ILT_NOISESCALEG_SIZE];
#define B43legacy_ILT_SIGMASQR_SIZE	53
extern const u16 b43legacy_ilt_sigmasqr1[B43legacy_ILT_SIGMASQR_SIZE];
extern const u16 b43legacy_ilt_sigmasqr2[B43legacy_ILT_SIGMASQR_SIZE];


void b43legacy_ilt_write(struct b43legacy_wldev *dev, u16 offset, u16 val);
void b43legacy_ilt_write32(struct b43legacy_wldev *dev, u16 offset,
			   u32 val);
u16 b43legacy_ilt_read(struct b43legacy_wldev *dev, u16 offset);

#endif /* B43legacy_ILT_H_ */
