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
/*! \file
    \brief  Declaration of library functions

    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/



#ifndef _MTK_WCN_CMB_HW_H_
#define _MTK_WCN_CMB_HW_H_


/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/



/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/



/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

typedef struct _PWR_SEQ_TIME_ {
	unsigned int rtcStableTime;
	unsigned int ldoStableTime;
	unsigned int rstStableTime;
	unsigned int offStableTime;
	unsigned int onStableTime;
} PWR_SEQ_TIME, *P_PWR_SEQ_TIME;


/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/



/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/





/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/



/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

extern int mtk_wcn_cmb_hw_pwr_off(void);
extern int mtk_wcn_cmb_hw_pwr_on(void);
extern int mtk_wcn_cmb_hw_rst(void);
extern int mtk_wcn_cmb_hw_init(P_PWR_SEQ_TIME pPwrSeqTime);
extern int mtk_wcn_cmb_hw_deinit(void);
extern int mtk_wcn_cmb_hw_state_show(void);


#endif				/* _MTK_WCN_CMB_HW_H_ */
