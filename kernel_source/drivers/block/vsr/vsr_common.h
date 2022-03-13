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
#ifndef	__VSR_COMMON_H__
#define __VSR_COMMON_H__

#define	__CODER_INFO__						"[#WJing]"
#define	__CODE_TYPE__						"[DRV]"
#define	__PRINT_TYPE_DBG__					"[DBG]     "
#define	__PRINT_TYPE_ERR__					"[ERR]     "
#define	__PRINT_TYPE_LOG__					"[LOG]     "
#define	__PRINT_TYPE_WAN__					"[WAN]     "

#define	__PRINT__
#ifdef	__PRINT__
#define	PRINT(f...)	printk(f)
#else
#define	PRINT(f...)	do{}while(0)
#endif



#define	__DBG__
#ifdef	__DBG__
#define	DEBUG(f...)	PRINT(f)
#else
#define	DEBUG(f...)	do{}while(0)
#endif

#ifdef	__DBG__
#define	DBG(f...)	PRINT(__CODER_INFO__);PRINT(__CODE_TYPE__);PRINT(__PRINT_TYPE_DBG__);PRINT(f)
#else
#define	DBG(f...)	do{}while(0)
#endif



#define	__ERR__
#ifdef	__ERR__
#define	ERROR(f...)	PRINT(f)
#else
#define	ERROR(f...)	do{}while(0)
#endif

#ifdef	__ERR__
#define	ERR(f...)	PRINT(__CODER_INFO__);PRINT(__CODE_TYPE__);PRINT(__PRINT_TYPE_ERR__);PRINT(f)
#else
#define	ERR(f...)	do{}while(0)
#endif



#define	__LOG__
#ifdef	__LOG__
#define	LOGER(f...)	PRINT(f)
#else
#define	LOGER(f...)	do{}while(0)
#endif

#ifdef	__LOG__
#define	LOG(f...)	PRINT(__CODER_INFO__);PRINT(__CODE_TYPE__);PRINT(__PRINT_TYPE_LOG__);PRINT(f)
#else
#define	LOG(f...)	do{}while(0)
#endif



#define	__WAN__
#ifdef	__WAN__
#define	WANNING(f...)	PRINT(f)
#else
#define	WANNING(f...)	do{}while(0)
#endif

#ifdef	__WAN__
#define	WAN(f...)	PRINT(__CODER_INFO__);PRINT(__CODE_TYPE__);PRINT(__PRINT_TYPE_WAN__);PRINT(f)
#else
#define	WAN(f...)	do{}while(0)
#endif


#define VSR_ERR_EN_MASK			((u32)(1 << 0))
#define VSR_WAN_EN_MASK			((u32)(1 << 1))
#define VSR_DBG_EN_MASK			((u32)(1 << 2))
#define VSR_LOG_EN_MASK			((u32)(1 << 3))

static u32 vsr_log_en = 0x3;
static inline void set_vsr_log_en(u32 enable)
{
	vsr_log_en = enable;
}

static inline u32 get_vsr_log_en(void)
{
	return vsr_log_en;
}

#define	__INFO_HEAD_VSR__					"@VSR"

#define	__INFO_FORMATE__					"%s/func(%s/%d): "

#define	FUNC_INFO(__INFO_HEAD__)			DBG("%s: func(%s)!\n", __INFO_HEAD__, __func__);


#define	__VSR_DBG__
#ifdef	__VSR_DBG__
#define	VSR_DBG(...)													\
if (vsr_log_en & VSR_DBG_EN_MASK){										\
	do {																\
		DBG(__INFO_FORMATE__, __INFO_HEAD_VSR__, __func__, __LINE__);	\
		DEBUG(__VA_ARGS__);												\
		DEBUG("\n");													\
	}while(0);															\
}
#endif

#define	__VSR_ERR__
#ifdef	__VSR_ERR__
#define	VSR_ERR(...)													\
if (vsr_log_en & VSR_ERR_EN_MASK){										\
	do {																\
		ERR(__INFO_FORMATE__, __INFO_HEAD_VSR__, __func__, __LINE__);	\
		ERROR(__VA_ARGS__);												\
		ERROR("\n");													\
	}while(0);															\
}
#endif

#define	__VSR_LOG__
#ifdef	__VSR_LOG__
#define	VSR_LOG(...)													\
if (vsr_log_en & VSR_LOG_EN_MASK){										\
	do {																\
		LOG(__INFO_FORMATE__, __INFO_HEAD_VSR__, __func__, __LINE__);	\
		LOGER(__VA_ARGS__);												\
		LOGER("\n");													\
	}while(0);															\
}
#endif

#define	__VSR_WAN__
#ifdef	__VSR_WAN__
#define	VSR_WAN(...)													\
if (vsr_log_en & VSR_WAN_EN_MASK){										\
	do {																\
		WAN(__INFO_FORMATE__, __INFO_HEAD_VSR__, __func__, __LINE__);	\
		WANNING(__VA_ARGS__);											\
		WANNING("\n");													\
	}while(0);															\
}
#endif






#ifndef	assert
#define	assert(expr)	\
	if (!(expr)) {		\
		PRINT("Assertion failed! %s,%s,%s,line=%d\n", \
		#expr, __FILE__, __func__, __LINE__);\
	}
#endif

#define	CHECK_PRT_VOID(prt);		\
	if (!(prt)) {\
		VSR_ERR("%s is NULL!", #prt);	\
		return;						\
	}

#define	CHECK_PRT_INT(prt);		\
	if (!(prt)) {\
		VSR_ERR("%s is NULL!", #prt);	\
		return -1;						\
	}
	
#define	CHECK_PRT_CHAR(prt);		\
		if (!(prt)) {\
			VSR_ERR("%s is NULL!", #prt);	\
			return -1;						\
		}

#define	BAD_CMD						1
#define	ERR_OPTCODE					2
#define	NO_DATA						0
#define	SEND_CMD_ERR				4
#define	WAIT_XFER_DATA_ERR			5
#define	BAD_XFER_LOOP				6

#define	OPEN_USB_FOR_VSR			0x80000001
#define	CLOSE_USB_FOR_VSR			0x80000002
#define	SET_VSR_LOG_EN				0x80010001
#define	GET_VSR_LOG_EN				0x80010002

//#define	UNIT_TEST
//#define DATA_TEST
//#define	MODIFY_SECTOR_OFFSET
#define	ADD_SYNC_CMD
#endif	/*__VSR_COMMON_H__*/
