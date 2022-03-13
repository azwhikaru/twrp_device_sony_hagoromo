/*
* Copyright (C) 2011-2014 MediaTek Inc.
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
#include "osal_typedef.h"
#include "wmt_stp_exp.h"


/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG         "[WMT-STP-EXP]"

#define WMT_STP_EXP_INFO_FUNC(fmt, arg...)   printk(DFT_TAG "[I]%s: "  fmt, __FUNCTION__ ,##arg)
#define WMT_STP_EXP_WARN_FUNC(fmt, arg...)   printk(DFT_TAG "[W]%s: "  fmt, __FUNCTION__ ,##arg)
#define WMT_STP_EXP_ERR_FUNC(fmt, arg...)    printk(DFT_TAG "[E]%s(%d):ERROR! "   fmt, __FUNCTION__ , __LINE__, ##arg)

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
#ifdef MTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT
/*STP exp*/
MTK_WCN_STP_SEND_DATA mtk_wcn_stp_send_data_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_SEND_DATA mtk_wcn_stp_send_data_raw_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_PARSER_DATA mtk_wcn_stp_parser_data_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_RECV_DATA mtk_wcn_stp_receive_data_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_IS_RXQ_EMPTY mtk_wcn_stp_is_rxqueue_empty_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_IS_RDY mtk_wcn_stp_is_ready_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_SET_BLUEZ mtk_wcn_stp_set_bluez_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_REG_IF_TX mtk_wcn_stp_if_tx_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_REG_IF_RX mtk_wcn_stp_if_rx_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_REG_EVENT_CB mtk_wcn_stp_reg_event_cb_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_RGE_TX_EVENT_CB mtk_wcn_stp_reg_tx_event_cb_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_STP_COREDUMP_START_GET mtk_wcn_stp_coredump_start_get_f[CHIP_TYPE_MAX] = {NULL};

/*WMT exp*/
MTK_WCN_WMT_FUNC_CTRL mtk_wcn_wmt_func_on_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_WMT_FUNC_CTRL mtk_wcn_wmt_func_off_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_WMT_THERM_CTRL mtk_wcn_wmt_therm_ctrl_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_WMT_HWVER_GET mtk_wcn_wmt_hwver_get_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_WMT_DSNS_CTRL mtk_wcn_wmt_dsns_ctrl_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_WMT_MSGCB_REG mtk_wcn_wmt_msgcb_reg_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_WMT_MSGCB_UNREG mtk_wcn_wmt_msgcb_unreg_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_WMT_SDIO_OP_REG mtk_wcn_wmt_sdio_op_reg_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_WMT_SDIO_HOST_AWAKE mtk_wcn_wmt_sdio_host_awake_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_WMT_ASSERT mtk_wcn_wmt_assert_f[CHIP_TYPE_MAX] = {NULL};
MTK_WCN_WMT_IC_INFO_GET mtk_wcn_wmt_ic_info_get_f[CHIP_TYPE_MAX] = {NULL};

/*******************************************************************************
*                          F U N C T I O N S
********************************************************************************
*/

UINT32 mtk_wcn_stp_exp_cb_reg(P_MTK_WCN_STP_EXP_CB_INFO pStpExpCb, CHIP_TYPE_F chipType)
{
	if (chipType == CHIP_TYPE_SOC) {
		WMT_STP_EXP_INFO_FUNC("call SOC stp exp cb reg\n");
	} else if (chipType == CHIP_TYPE_COMBO) {
		WMT_STP_EXP_INFO_FUNC("call COMBO stp exp cb reg\n");
	} else {
		WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		return -1;
	}

	mtk_wcn_stp_send_data_f[chipType] = pStpExpCb->stp_send_data_cb;
	mtk_wcn_stp_send_data_raw_f[chipType] = pStpExpCb->stp_send_data_raw_cb;
	mtk_wcn_stp_parser_data_f[chipType] = pStpExpCb->stp_parser_data_cb;
	mtk_wcn_stp_receive_data_f[chipType] = pStpExpCb->stp_receive_data_cb;
	mtk_wcn_stp_is_rxqueue_empty_f[chipType] = pStpExpCb->stp_is_rxqueue_empty_cb;
	mtk_wcn_stp_is_ready_f[chipType] = pStpExpCb->stp_is_ready_cb;
	mtk_wcn_stp_set_bluez_f[chipType] = pStpExpCb->stp_set_bluez_cb;
	mtk_wcn_stp_if_tx_f[chipType] = pStpExpCb->stp_if_tx_cb;
	mtk_wcn_stp_if_rx_f[chipType] = pStpExpCb->stp_if_rx_cb;
	mtk_wcn_stp_reg_event_cb_f[chipType] = pStpExpCb->stp_reg_event_cb;
	mtk_wcn_stp_reg_tx_event_cb_f[chipType] = pStpExpCb->stp_reg_tx_event_cb;
	mtk_wcn_stp_coredump_start_get_f[chipType] = pStpExpCb->stp_coredump_start_get_cb;

	return 0;
}

EXPORT_SYMBOL(mtk_wcn_stp_exp_cb_reg);

UINT32 mtk_wcn_stp_exp_cb_unreg(CHIP_TYPE_F chipType)
{
	if (chipType == CHIP_TYPE_SOC) {
		WMT_STP_EXP_INFO_FUNC("call SOC stp exp cb unreg\n");
	} else if (chipType == CHIP_TYPE_COMBO) {
		WMT_STP_EXP_INFO_FUNC("call COMBO stp exp cb unreg\n");
	} else {
		WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		return -1;
	}
		
	mtk_wcn_stp_send_data_f[chipType] = NULL;
	mtk_wcn_stp_send_data_raw_f[chipType] = NULL;
	mtk_wcn_stp_parser_data_f[chipType] = NULL;
	mtk_wcn_stp_receive_data_f[chipType] = NULL;
	mtk_wcn_stp_is_rxqueue_empty_f[chipType] = NULL;
	mtk_wcn_stp_is_ready_f[chipType] = NULL;
	mtk_wcn_stp_set_bluez_f[chipType] = NULL;
	mtk_wcn_stp_if_tx_f[chipType] = NULL;
	mtk_wcn_stp_if_rx_f[chipType] = NULL;
	mtk_wcn_stp_reg_event_cb_f[chipType] = NULL;
	mtk_wcn_stp_reg_tx_event_cb_f[chipType] = NULL;
	mtk_wcn_stp_coredump_start_get_f[chipType]= NULL;
	
	return 0;
}

EXPORT_SYMBOL(mtk_wcn_stp_exp_cb_unreg);

INT32 mtk_wcn_stp_send_data(const PUINT8 buffer, const UINT32 length, const UINT8 type, CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if(mtk_wcn_stp_send_data_f[chipType]) {
		ret = (*mtk_wcn_stp_send_data_f[chipType])(buffer,length,type);	
		//WMT_STP_EXP_INFO_FUNC("mtk_wcn_stp_send_data_f[%d] send data(%d)\n", chipType, ret);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_send_data_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_send_data_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
	
}

EXPORT_SYMBOL(mtk_wcn_stp_send_data);

INT32 mtk_wcn_stp_send_data_raw(const PUINT8 buffer, const UINT32 length, const UINT8 type, CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if(mtk_wcn_stp_send_data_raw_f[chipType]) {
		ret = (*mtk_wcn_stp_send_data_raw_f[chipType])(buffer,length,type);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_send_data_raw_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_send_data_raw_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_send_data_raw);

INT32 mtk_wcn_stp_parser_data(PUINT8 buffer, UINT32 length, CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if(mtk_wcn_stp_parser_data_f[chipType]) {
		ret = (*mtk_wcn_stp_parser_data_f[chipType])(buffer,length);	
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_parser_data_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_parser_data_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_parser_data);

INT32 mtk_wcn_stp_receive_data(PUINT8 buffer, UINT32 length, UINT8 type, CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if(mtk_wcn_stp_receive_data_f[chipType]) {
		ret = (*mtk_wcn_stp_receive_data_f[chipType])(buffer,length,type);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_receive_data_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_receive_data_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_receive_data);

MTK_WCN_BOOL mtk_wcn_stp_is_rxqueue_empty(UINT8 type, CHIP_TYPE_F chipType)
{
	MTK_WCN_BOOL ret = MTK_WCN_BOOL_FALSE;
	
	if(mtk_wcn_stp_is_rxqueue_empty_f[chipType]) {
		ret = (*mtk_wcn_stp_is_rxqueue_empty_f[chipType])(type);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_is_rxqueue_empty_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_is_rxqueue_empty_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_is_rxqueue_empty);

MTK_WCN_BOOL mtk_wcn_stp_is_ready(CHIP_TYPE_F chipType)
{
	MTK_WCN_BOOL ret = MTK_WCN_BOOL_FALSE;
	
	if(mtk_wcn_stp_is_ready_f[chipType]) {
		ret = (*mtk_wcn_stp_is_ready_f[chipType])();
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_is_ready_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_is_ready_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_is_ready);

void mtk_wcn_stp_set_bluez(MTK_WCN_BOOL flags, CHIP_TYPE_F chipType)
{
	
	if (mtk_wcn_stp_set_bluez_f[chipType]) {
		(*mtk_wcn_stp_set_bluez_f[chipType])(flags);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_set_bluez_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_set_bluez_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return;
}

EXPORT_SYMBOL(mtk_wcn_stp_set_bluez);

INT32 mtk_wcn_stp_register_if_tx(ENUM_STP_TX_IF_TYPE stp_if, MTK_WCN_STP_IF_TX func, CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if (mtk_wcn_stp_if_tx_f[chipType]) {
		ret = (*mtk_wcn_stp_if_tx_f[chipType])(stp_if,func);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_if_tx_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_if_tx_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_register_if_tx);

INT32 mtk_wcn_stp_register_if_rx(MTK_WCN_STP_IF_RX func, CHIP_TYPE_F chipType)
{
	INT32 ret = -1;

	if (mtk_wcn_stp_if_rx_f[chipType]) {
		ret = (*mtk_wcn_stp_if_rx_f[chipType])(func);	
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_if_rx_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_if_rx_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_register_if_rx);

INT32 mtk_wcn_stp_register_event_cb(INT32 type, MTK_WCN_STP_EVENT_CB func, CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if (mtk_wcn_stp_reg_event_cb_f[chipType]) {
		ret = (*mtk_wcn_stp_reg_event_cb_f[chipType])(type,func);	
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_reg_event_cb_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_reg_event_cb_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_register_event_cb);

INT32 mtk_wcn_stp_register_tx_event_cb(INT32 type, MTK_WCN_STP_EVENT_CB func, CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if (mtk_wcn_stp_reg_tx_event_cb_f[chipType]) {
		ret = (*mtk_wcn_stp_reg_tx_event_cb_f[chipType])(type,func);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_reg_tx_event_cb_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_reg_tx_event_cb_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_register_tx_event_cb);

INT32 mtk_wcn_stp_coredump_start_get(CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if (mtk_wcn_stp_coredump_start_get_f[chipType]) {
		ret = (*mtk_wcn_stp_coredump_start_get_f[chipType])();	
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_coredump_start_get_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_stp_coredump_start_get_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_coredump_start_get);

UINT32 mtk_wcn_wmt_exp_cb_reg(P_MTK_WCN_WMT_EXP_CB_INFO pWmtExpCb, CHIP_TYPE_F chipType)
{
	if (chipType == CHIP_TYPE_SOC) {
		WMT_STP_EXP_INFO_FUNC("call SOC wmt exp cb reg\n");
	} else if (chipType == CHIP_TYPE_COMBO) {
		WMT_STP_EXP_INFO_FUNC("call COMBO wmt exp cb reg\n");
	} else {
		WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		return -1;
	}

	mtk_wcn_wmt_func_on_f[chipType] = pWmtExpCb->wmt_func_on_cb;
	mtk_wcn_wmt_func_off_f[chipType] = pWmtExpCb->wmt_func_off_cb;
	mtk_wcn_wmt_therm_ctrl_f[chipType] = pWmtExpCb->wmt_therm_ctrl_cb;
	mtk_wcn_wmt_hwver_get_f[chipType] = pWmtExpCb->wmt_hwver_get_cb;
	mtk_wcn_wmt_dsns_ctrl_f[chipType] = pWmtExpCb->wmt_dsns_ctrl_cb;
	mtk_wcn_wmt_msgcb_reg_f[chipType] = pWmtExpCb->wmt_msgcb_reg_cb;
	mtk_wcn_wmt_msgcb_unreg_f[chipType] = pWmtExpCb->wmt_msgcb_unreg_cb;
	mtk_wcn_wmt_sdio_op_reg_f[chipType] = pWmtExpCb->wmt_sdio_op_reg_cb;
	mtk_wcn_wmt_sdio_host_awake_f[chipType] = pWmtExpCb->wmt_sdio_host_awake_cb;
	mtk_wcn_wmt_assert_f[chipType] = pWmtExpCb->wmt_assert_cb;
	mtk_wcn_wmt_ic_info_get_f[chipType] = pWmtExpCb->wmt_ic_info_get_cb;

	return 0;
}

EXPORT_SYMBOL(mtk_wcn_wmt_exp_cb_reg);

UINT32 mtk_wcn_wmt_exp_cb_unreg(CHIP_TYPE_F chipType)
{
	if (chipType == CHIP_TYPE_SOC) {
		WMT_STP_EXP_INFO_FUNC("call SOC wmt exp cb unreg\n");
	} else if (chipType == CHIP_TYPE_COMBO) {
		WMT_STP_EXP_INFO_FUNC("call COMBO wmt exp cb unreg\n");
	} else {
		WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		return -1;
	}

	mtk_wcn_wmt_func_on_f[chipType] = NULL;
	mtk_wcn_wmt_func_off_f[chipType] = NULL;
	mtk_wcn_wmt_therm_ctrl_f[chipType] = NULL;
	mtk_wcn_wmt_hwver_get_f[chipType] = NULL;
	mtk_wcn_wmt_dsns_ctrl_f[chipType] = NULL;
	mtk_wcn_wmt_msgcb_reg_f[chipType] = NULL;
	mtk_wcn_wmt_msgcb_unreg_f[chipType] = NULL;
	mtk_wcn_wmt_sdio_op_reg_f[chipType] = NULL;
	mtk_wcn_wmt_sdio_host_awake_f[chipType] = NULL;
	mtk_wcn_wmt_assert_f[chipType] = NULL;
	mtk_wcn_wmt_ic_info_get_f[chipType] = NULL;

	return 0;
}

EXPORT_SYMBOL(mtk_wcn_wmt_exp_cb_unreg);

MTK_WCN_BOOL mtk_wcn_wmt_func_off (ENUM_WMTDRV_TYPE_T type, CHIP_TYPE_F chipType)
{
	MTK_WCN_BOOL ret = MTK_WCN_BOOL_FALSE;
	
	if (mtk_wcn_wmt_func_off_f[chipType]) {
		ret = (*mtk_wcn_wmt_func_off_f[chipType])(type);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_func_off_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_func_off_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_wmt_func_off);

MTK_WCN_BOOL mtk_wcn_wmt_func_on (ENUM_WMTDRV_TYPE_T type, CHIP_TYPE_F chipType)
{
	MTK_WCN_BOOL ret = MTK_WCN_BOOL_FALSE;
	
	if (mtk_wcn_wmt_func_on_f[chipType]) {
		ret = (*mtk_wcn_wmt_func_on_f[chipType])(type);
		WMT_STP_EXP_INFO_FUNC("mtk_wcn_wmt_func_on_f type(%d)\n",type);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_func_on_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_func_on_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_wmt_func_on);

INT8 mtk_wcn_wmt_therm_ctrl (ENUM_WMTTHERM_TYPE_T eType, CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if (mtk_wcn_wmt_therm_ctrl_f[chipType]) {
		ret = (*mtk_wcn_wmt_therm_ctrl_f[chipType])(eType);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_therm_ctrl_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_therm_ctrl_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_wmt_therm_ctrl);

ENUM_WMTHWVER_TYPE_T mtk_wcn_wmt_hwver_get (CHIP_TYPE_F chipType)
{
	ENUM_WMTHWVER_TYPE_T ret = WMTHWVER_INVALID;

	if (mtk_wcn_wmt_hwver_get_f[chipType]) {
		ret = (*mtk_wcn_wmt_hwver_get_f[chipType])();
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_hwver_get_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_hwver_get_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_wmt_hwver_get);

MTK_WCN_BOOL mtk_wcn_wmt_dsns_ctrl (ENUM_WMTDSNS_TYPE_T eType, CHIP_TYPE_F chipType)
{
	MTK_WCN_BOOL ret = MTK_WCN_BOOL_FALSE;
	
	if (mtk_wcn_wmt_dsns_ctrl_f[chipType]) {
		ret = (*mtk_wcn_wmt_dsns_ctrl_f[chipType])(eType);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_dsns_ctrl_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_dsns_ctrl_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_wmt_dsns_ctrl);

INT32 mtk_wcn_wmt_msgcb_reg (ENUM_WMTDRV_TYPE_T eType,PF_WMT_CB pCb, CHIP_TYPE_F chipType)
{
	INT32 ret = 0;
	
	if (mtk_wcn_wmt_msgcb_reg_f[chipType]) {
		ret = (*mtk_wcn_wmt_msgcb_reg_f[chipType])(eType,pCb);	
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_msgcb_reg_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_msgcb_reg_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_wmt_msgcb_reg);

INT32 mtk_wcn_wmt_msgcb_unreg (ENUM_WMTDRV_TYPE_T eType, CHIP_TYPE_F chipType)
{
	INT32 ret = 0;
	
	if (mtk_wcn_wmt_msgcb_unreg_f[chipType]) {
		ret = (*mtk_wcn_wmt_msgcb_unreg_f[chipType])(eType);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_msgcb_unreg_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_msgcb_unreg_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_wmt_msgcb_unreg);

INT32 mtk_wcn_stp_wmt_sdio_op_reg (PF_WMT_SDIO_PSOP own_cb, CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if (mtk_wcn_wmt_sdio_op_reg_f[chipType]) {
		ret = (*mtk_wcn_wmt_sdio_op_reg_f[chipType])(own_cb);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_sdio_op_reg_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_sdio_op_reg_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_wmt_sdio_op_reg);

INT32 mtk_wcn_stp_wmt_sdio_host_awake(CHIP_TYPE_F chipType)
{
	INT32 ret = -1;
	
	if (mtk_wcn_wmt_sdio_host_awake_f[chipType]) {
		ret = (*mtk_wcn_wmt_sdio_host_awake_f[chipType])();	
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_sdio_host_awake_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_sdio_host_awake_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_stp_wmt_sdio_host_awake);

MTK_WCN_BOOL mtk_wcn_wmt_assert (ENUM_WMTDRV_TYPE_T type,UINT32 reason, CHIP_TYPE_F chipType)
{
	MTK_WCN_BOOL ret = MTK_WCN_BOOL_FALSE;
	
	if (mtk_wcn_wmt_assert_f[chipType]) {
		ret = (*mtk_wcn_wmt_assert_f[chipType])(type,reason);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_assert_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_assert_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret;
}

EXPORT_SYMBOL(mtk_wcn_wmt_assert);

UINT32
mtk_wcn_wmt_ic_info_get (ENUM_WMT_CHIPINFO_TYPE_T type, CHIP_TYPE_F chipType)
{
	UINT32 ret = 0;
	
	if (mtk_wcn_wmt_ic_info_get_f[chipType]) {
		ret = (*mtk_wcn_wmt_ic_info_get_f[chipType])(type);
	} else {
		if (chipType == CHIP_TYPE_SOC) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_ic_info_get_f SOC cb is null\n");
		} else if (chipType == CHIP_TYPE_COMBO) {
			WMT_STP_EXP_ERR_FUNC("mtk_wcn_wmt_ic_info_get_f COMBO cb is null\n");
		} else {
			WMT_STP_EXP_ERR_FUNC("CHIP_TYPE is error not SOC or COMBO\n");
		}
	}

	return ret; 
}
EXPORT_SYMBOL(mtk_wcn_wmt_ic_info_get);

#endif

