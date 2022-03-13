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
#ifndef __DDP_DPFD_H__
#define __DDP_DPFD_H__

unsigned int ddp_bitblt_ioctl_wait_reequest( unsigned long ioctl_user_param  );
unsigned int ddp_bitblt_ioctl_inform_done( unsigned long ioctl_user_param  );

#define DDPKBITBLIT_CHNL_COUNT  (32)

#define DDPK_CH_HDMI_0   (0)
#define DDPK_CH_HDMI_1   (1)
#define DDPK_CH_HDMI_2   (2)
#define DDPK_CH_HDMI_3   (3)
#define DDPK_CH_WFD_0    (4)

#define DDPK_PRINTF               printk

//#define DDPK_SHOWFUNCTION( b_valid_function )     {  DDPK_PRINTF("[DDPK Func Call]: %s()%s\n\r", __FUNCTION__, b_valid_function?"":":Under Implement!" );  }
#define DDPK_SHOWFUNCTION( b_valid_function )

#define DDPK_MSG(fmt, arg...)                                   { DDPK_PRINTF("[DDPK MSG]: %s(): "fmt,__FUNCTION__, ##arg); }
#define DDPK_INFO(fmt, arg...)      if( b_ddp_show_info )       { DDPK_PRINTF("[DDPK INFO]: %s(): "fmt,__FUNCTION__, ##arg); }
#define DDPK_WARNING(fmt, arg...)   if( b_ddp_show_warning )    { DDPK_PRINTF("[DDPK W]: %s(): "fmt,__FUNCTION__, ##arg);   }
#define DDPK_ERROR(fmt, arg...)     if( b_ddp_show_error )      { DDPK_PRINTF("[DDPK E]: %s(): %s@%d: "fmt,__FUNCTION__, __FILE__,__LINE__, ##arg); }
#define DDPK_ISRINFO(fmt, arg...)   if( b_ddp_show_isrinfo )    { DDPK_PRINTF("[DDPK ISR]: %s(): "fmt,__FUNCTION__, ##arg);    }
//#define DDPK_ISRINFO(fmt, arg...) {;}
#define DDPK_POWERINFO(fmt, arg...) if( b_ddp_show_powerinfo )  { DDPK_PRINTF("[DDPK CG]: "fmt, ##arg);  }
#define DDPK_ZOOMINFO(fmt, arg...)  if( b_ddp_show_zoominfo )   { DDPK_PRINTF("[DDPK ZOOM]: "fmt, ##arg); }
#define DDPK_FPSINFO(fmt, arg...)   if( b_ddp_show_fpsinfo )    { DDPK_PRINTF("[DDPK FPS]: "fmt, ##arg); }
#define DDPK_BBINFO(fmt, arg...)    if( b_ddp_show_bbinfo )     { DDPK_PRINTF("[DDPK BB]: "fmt, ##arg); }
#define DDPK_ZSDINFO(fmt, arg...)   if( b_ddp_show_zsdinfo )    { DDPK_PRINTF("[DDPK ZSD]: "fmt, ##arg); }

#define DDPK_DEBUG(fmt, arg...)                                 { DDPK_PRINTF("[DDPK DEBUG]: "fmt, ##arg); }

enum DpColorFormat_K
{
    eY800_K,
    eYUY2_K,
    eUYVY_K,
    eYVYU_K,
    eVYUY_K,
    eYUV_444_1P_K,          // 5
    eYUV_422_I_K,
    eYUV_422_I_BLK_K,

    eNV21_K,
    eNV12_K,
    eNV12_BLK_K,            // 10
    eNV12_BLK_FCM_K,
    eYUV_420_2P_ISP_BLK_K,
    eYUV_420_2P_VDO_BLK_K,
    eYUV_422_2P_K,
    eYUV_444_2P_K,          // 15
    eYUV_420_2P_UYVY_K,
    eYUV_420_2P_VYUY_K,
    eYUV_420_2P_YUYV_K,
    eYUV_420_2P_YVYU_K,


    eYV16_K,                // 20
    eYV12_K,
    eYV21_K,
    eYUV_422_3P_K,
    eYUV_444_3P_K,
    eYUV_420_3P_YVU_K,      // 25
    eYUV_420_3P_K,

    eRGB565_K,
    eRGB888_K,
    eARGB8888_K,
    eABGR8888_K,            // 30

    eRGBA8888_K,
    eBGRA8888_K,

    ePARGB8888_K,
    eXARGB8888_K,

    eCompactRaw1_K,         // 35
    eMTKYUV_K,
    eNV16_K,
    eNV61_K,
};

typedef struct
{
	int srcX;
	int srcY;
	int srcW;
	int srcH;
	int srcWStride;
	int srcHStride;
	int srcAddr[3];
	int srcFormat;
	int srcBufferSize[3];
	int srcPlaneNum ;
	int srcMemType;

	int dstX;
	int dstY;
	int dstW;
	int dstWStride;
	int dstH;
	int dstHStride;
	int dstAddr[3];
	int dstFormat;
	int pitch;
	int dstBufferSize[3];
	int dstPlaneNum;
	int dstMemType;
	int orientation;

	int m_cropXStart;
	int m_cropYStart;
	int m_cropWidth;
	int m_cropHeight;

}DdpkBitbltConfig;

typedef struct
{
	int out_channel;
	int out_b_src_addr_dirty;
	int out_b_dst_addr_dirty;
	DdpkBitbltConfig out_config;
}DDPIOCTL_DdpkBitbltConfig;

typedef struct
{
	int in_channel;
	int in_ret_val;
}DDPIOCTL_DdpkBitbltInformDone;

typedef enum DPFD_MEMORY_ENUM
{
	DPFD_MEMORY_VA,
	DPFD_MEMORY_ION,
	DPFD_MEMORY_PHY,
	DPFD_MEMORY_MVA
} DPFD_MEMORY_ENUM;


void ddpk_testfunc_1( unsigned long channel);
void ddpk_testfunc_2( unsigned long channel);

int DDPK_Bitblt( int channel );
int DDPK_Bitblt_Config( int channel, DdpkBitbltConfig* pParam );

#endif /* __DDP_DPFD_H__ */
