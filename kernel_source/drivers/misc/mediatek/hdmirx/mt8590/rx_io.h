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
 /******************************************************************************
*[File]             rx_io.h
*[Version]          v0.1
*[Revision Date]    2009-04-18
*[Author]           Kenny Hsieh
*[Description]
*    source file for hdmi general control routine
*
*
******************************************************************************/
#ifndef _RX_IO_H_
#define _RX_IO_H_

#if 1//(DRV_SUPPORT_HDMI_RX)
#include "hdmi_rx_hw.h"
#include "hdmirx.h"
#include "typedef.h"


void Delay5MS(UINT32 count);
void vHalSetRxPort1HPDLevel(BOOL fgHighLevel);
void vHalSetRxPort2HPDLevel(BOOL fgHighLevel);
void vHalRxSwitchPortSelect(BYTE bPort);
void vHalSetSwitchRxPortHPDLevel(BOOL fgHighLevel);
BOOL fgRxPort1PWR5VStatus(void);

#define u1IO32Read1B(reg32) (*(volatile UINT8 *)(reg32))

#define u4IO32Read4B(reg32) (*(volatile UINT32 *)(reg32))

#define u1RegRead1B(reg32) 		u1IO32Read1B(reg32)
#define u4RegRead4B(reg32) 		u4IO32Read4B(reg32)

#define u2IO32Read2B(reg32) u2HdmiRxIO32Read2B(reg32)
#define vIO32Write1BMsk(reg32, val8, msk8) vHdmiRxIO32Write1BMsk(reg32, val8, msk8)
#define vIO32Write2BMsk(reg32, val16, msk16) vHdmiRxIO32Write2BMsk(reg32, val16, msk16)
#define vIO32Write4BMsk(reg32, val32, msk32) vHdmiRxIO32Write4BMsk(reg32, val32, msk32)

#define vIO32Write1B(reg32, val8) vIO32Write1BMsk(reg32,val8,0xff)
#define vIO32Write2B(reg32, val16) vIO32Write2BMsk(reg32,val16,0xffff)
#define vIO32Write4B(reg32,val32) (*(volatile UINT32 *)(reg32)=(val32))

#define u2RegRead2B(reg16) u2IO32Read2B(reg16)
#define vRegWrite1B(reg16, val8) vIO32Write1B(reg16,val8)
#define vRegWrite1BMsk(reg16, val8, msk8) vIO32Write1BMsk(reg16,val8,msk8)
#define vRegWrite2B(reg16,  val16) vIO32Write2B(reg16,val16)
#define vRegWrite2BMsk(reg16, val16, msk16) vIO32Write2BMsk(reg16,val16,msk16)

#define vRegWrite4B(reg32,val32)	vIO32Write4B(reg32,val32)
#define vRegWrite4BMsk(reg16, val32, msk32) vIO32Write4BMsk(reg16,val32,msk32)

#define	RegReadFld(reg16,fld) 	/*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	(((Fld_ac(fld)>=AC_FULLB0)&&(Fld_ac(fld)<=AC_FULLB3))?u1RegRead1B((reg16)+(Fld_ac(fld)-AC_FULLB0)):( \
	((Fld_ac(fld)>=AC_FULLW10)&&(Fld_ac(fld)<=AC_FULLW32))?u2RegRead2B((reg16)+(Fld_ac(fld)-AC_FULLW10)):( \
	(Fld_ac(fld)==AC_FULLDW)? u4RegRead4B(reg16):( \
	((Fld_ac(fld)>=AC_MSKB0)&&(Fld_ac(fld)<=AC_MSKB3))?(u1RegRead1B((reg16)+(Fld_ac(fld)-AC_MSKB0))&Fld2MskBX(fld,(Fld_ac(fld)-AC_MSKB0))):( \
	((Fld_ac(fld)>=AC_MSKW10)&&(Fld_ac(fld)<=AC_MSKW32))?(u2RegRead2B((reg16)+(Fld_ac(fld)-AC_MSKW10))&Fld2MskWX(fld,(Fld_ac(fld)-AC_MSKW10))):( \
	(Fld_ac(fld)==AC_MSKDW)?(u4RegRead4B(reg16)&Fld2Msk32(fld)):0 \
  )))))) /*lint -restore */


#define	RegReadFldAlign(reg16,fld) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	(((Fld_ac(fld)>=AC_FULLB0)&&(Fld_ac(fld)<=AC_FULLB3))?u1RegRead1B((reg16)+(Fld_ac(fld)-AC_FULLB0)):( \
	((Fld_ac(fld)>=AC_FULLW10)&&(Fld_ac(fld)<=AC_FULLW32))?u2RegRead2B((reg16)+(Fld_ac(fld)-AC_FULLW10)):( \
	(Fld_ac(fld)==AC_FULLDW)? u4RegRead4B(reg16):( \
	((Fld_ac(fld)>=AC_MSKB0)&&(Fld_ac(fld)<=AC_MSKB3))?((u1RegRead1B((reg16)+(Fld_ac(fld)-AC_MSKB0))&Fld2MskBX(fld,(Fld_ac(fld)-AC_MSKB0)))>>((Fld_shft(fld)-8*(Fld_ac(fld)-AC_MSKB0))&7)):( \
	((Fld_ac(fld)>=AC_MSKW10)&&(Fld_ac(fld)<=AC_MSKW32))?((u2RegRead2B((reg16)+(Fld_ac(fld)-AC_MSKW10))&Fld2MskWX(fld,(Fld_ac(fld)-AC_MSKW10)))>>((Fld_shft(fld)-8*(Fld_ac(fld)-AC_MSKW10))&15)):( \
	(Fld_ac(fld)==AC_MSKDW)?((u4RegRead4B(reg16)&Fld2Msk32(fld))>>Fld_shft(fld)):0 \
  ))))))  /*lint -restore */

#define	vRegWriteFld(reg16,val,fld)  /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	(((Fld_ac(fld)>=AC_FULLB0)&&(Fld_ac(fld)<=AC_FULLB3))? vRegWrite1B((reg16)+(Fld_ac(fld)-AC_FULLB0),(val)),0:( \
	((Fld_ac(fld)>=AC_FULLW10)&&(Fld_ac(fld)<=AC_FULLW32))?vRegWrite2B((reg16)+(Fld_ac(fld)-AC_FULLW10),(val)),0:( \
	(Fld_ac(fld)==AC_FULLDW)?vRegWrite4B((reg16),(val)),0:( \
	((Fld_ac(fld)>=AC_MSKB0)&&(Fld_ac(fld)<=AC_MSKB3))?vRegWrite1BMsk((reg16)+(Fld_ac(fld)-AC_MSKB0),(val),Fld2MskBX(fld,(Fld_ac(fld)-AC_MSKB0))),0:( \
	((Fld_ac(fld)>=AC_MSKW10)&&(Fld_ac(fld)<=AC_MSKW32))?vRegWrite2BMsk((reg16)+(Fld_ac(fld)-AC_MSKW10),(val),Fld2MskWX(fld,(Fld_ac(fld)-AC_MSKW10))),0:( \
	(Fld_ac(fld)==AC_MSKDW)?vRegWrite4BMsk((reg16),(val),Fld2Msk32(fld)),0:0\
	))))))  /*lint -restore */

#define	vRegWriteFldAlign(reg16,val,fld) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	(((Fld_ac(fld)>=AC_FULLB0)&&(Fld_ac(fld)<=AC_FULLB3))?vRegWrite1B((reg16)+(Fld_ac(fld)-AC_FULLB0),(val)),0:( \
	((Fld_ac(fld)>=AC_FULLW10)&&(Fld_ac(fld)<=AC_FULLW32))?vRegWrite2B((reg16)+(Fld_ac(fld)-AC_FULLW10),(val)),0:( \
	(Fld_ac(fld)==AC_FULLDW)?vRegWrite4B((reg16),(val)),0:( \
	((Fld_ac(fld)>=AC_MSKB0)&&(Fld_ac(fld)<=AC_MSKB3))?vRegWrite1BMsk((reg16)+(Fld_ac(fld)-AC_MSKB0),ValAlign2Fld((val),fld),Fld2MskBX(fld,(Fld_ac(fld)-AC_MSKB0))),0:( \
	((Fld_ac(fld)>=AC_MSKW10)&&(Fld_ac(fld)<=AC_MSKW32))?vRegWrite2BMsk((reg16)+(Fld_ac(fld)-AC_MSKW10),ValAlign2Fld((val),fld),Fld2MskWX(fld,(Fld_ac(fld)-AC_MSKW10))),0:( \
	(Fld_ac(fld)==AC_MSKDW)?vRegWrite4BMsk((reg16),((UINT32)(val)<<Fld_shft(fld)),Fld2Msk32(fld)),0:0\
	))))))  /*lint -restore */

#endif//DRV_SUPPORT_HDMI_RX

#endif
