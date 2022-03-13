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
#if 1//defined(MTK_HDMI_SUPPORT)
#include <linux/string.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#include <linux/debugfs.h>

#include <mach/mt_typedefs.h>
#include <linux/vmalloc.h>
#include <mach/m4u.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/byteorder/generic.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include "hdmi_rx_task.h"
#include "hdmi_rx_ctrl.h"
#include "video_in_if.h"
#include "vsw_drv_if.h"
#include "dgi_if.h"
#include "vin_main.h"
#include "vin_hal.h"
#include "vin_drv_if.h"
#include "hdmi_rx_dvi.h"
#include "hdmi_rx_hal.h"
#include "edid_data.h"
#include "hdmi_rx_hw.h"


void DBG_Init(void);
void DBG_Deinit(void);
extern void vsw_log_enable(unsigned short enable);
extern void vEnableHdmiRxAudTask(bool on);
extern void vHalRxSwitchPortSelect(BYTE bPort);

extern HDMI_RESOLUTION_MODE_T _bVDITiming;
extern UINT8 _CrcResult[3][3];

// ---------------------------------------------------------------------------
//  External variable declarations
// ---------------------------------------------------------------------------

//extern LCM_DRIVER *lcm_drv;
// ---------------------------------------------------------------------------
//  Debug Options
// ---------------------------------------------------------------------------

unsigned char interediddata[256] =
{
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x4d,0xd9,0x01,0x52,0x01,0x01,0x01,0x01,
	0x01,0x11,0x01,0x03,0x80,0xa0,0x5a,0x78,0x0a,0x0d,0xc9,0xa0,0x57,0x47,0x98,0x27,
	0x12,0x48,0x4c,0x21,0x08,0x00,0x81,0x80,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x01,0x01,0x8c,0x0a,0xd0,0x8a,0x20,0xe0,0x2d,0x10,0x10,0x3e,
	0x96,0x00,0xc4,0x8e,0x21,0x00,0x00,0x18,0x01,0x1d,0x00,0x72,0x51,0xd0,0x1e,0x20,
	0x6e,0x28,0x55,0x00,0x40,0x84,0x63,0x00,0x00,0x1e,0x00,0x00,0x00,0xfc,0x00,0x53,
	0x4f,0x4e,0x59,0x20,0x54,0x56,0x20,0x58,0x56,0x0a,0x20,0x20,0x00,0x00,0x00,0xfe,
	0x00,0x30,0x3e,0x0e,0x46,0x0f,0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0x4b,
	0x02,0x03,0x4a,0xf4,0x5c,0x1f,0x03,0x04,0x12,0x13,0x05,0x14,0x20,0x07,0x16,0x10,
	0x15,0x11,0x02,0x06,0x01,0x0e,0x0f,0x1d,0x1e,0x0a,0x0b,0x19,0x1a,0x23,0x24,0x25,
	0x26,0x3b,0x09,0x7f,0x07,0x0f,0x7f,0x07,0x15,0x07,0x50,0x35,0x07,0x48,0x3e,0x1f,
	0xc0,0x4d,0x02,0x00,0x57,0x06,0x00,0x67,0x54,0x00,0x5f,0x54,0x01,0x83,0x5f,0x00,
	0x00,0x68,0x03,0x0c,0x00,0x21,0x00,0xb8,0x2d,0x00,0x8c,0x0a,0xd0,0x90,0x20,0x40,
	0x31,0x20,0x0c,0x40,0x55,0x00,0xc4,0x8e,0x21,0x00,0x00,0x18,0x01,0x1d,0x00,0xbc,
	0x52,0xd0,0x1e,0x20,0xb8,0x28,0x55,0x40,0x40,0x84,0x63,0x00,0x00,0x1e,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa3
};

static char STR_HELP[] =
    "\n"
    "USAGE\n"
    "        echo [ACTION]... > hdmirx\n"
    "\n"
    "ACTION\n"
    "        hdmirx:[on|off]\n"
    "             enable hdmirx video output\n"
    "\n";

// TODO: this is a temp debug solution

#define HDMIRX_PRINTF(fmt, arg...)  \
			do { \
				 temp_len = sprintf(buf,fmt,##arg);  \
				 buf += temp_len; \
				 len += temp_len; \
			}while (0)

#define REGISTER_WRITE32(u4Addr, u4Val)     (*((volatile unsigned int*)(u4Addr)) = (u4Val))
#define REGISTER_READ32(u4Addr)             (*((volatile unsigned int*)(u4Addr)))

static UINT32 reg_start, lenth, reg_index;

static ssize_t hdmirx_core_show_info(struct device *dev, 
	                                         struct device_attribute *attr, char *buf)
{
	struct attribute *pattr = &(attr->attr);

	UINT32 len = 0; 
	UINT32 temp_len = 0; 	

	HDMIRX_PRINTF("[hdmirx]%s,%d \n", __func__, __LINE__);
	HDMIRX_PRINTF("[hdmirx] %s \n",pattr->name);
	printk("read start address:0x%08x lenth:0x%x  %x\n",reg_start,lenth,reg_start+lenth);
	for (reg_index = reg_start; reg_index <(reg_start+lenth);)
	{
		printk("read:0x%08x = 0x%08x\n",reg_index,REGISTER_READ32(reg_index));
		HDMIRX_PRINTF("read:0x%08x = 0x%08x\n",reg_index,REGISTER_READ32(reg_index));
		reg_index += 0x4;
		if(reg_index >(reg_start+lenth))
			break;
	}
    return len;
}
			
static ssize_t hdmirx_core_store_info(struct device *dev, 
                                            struct device_attribute *attr,
		                                    const char *buf, size_t count)
{
    int val;
	UINT32 reg;
	UINT32 temp_len = 0; 
	UINT32 len = 0;
	
	struct attribute *pattr = &(attr->attr);	
	printk("input cmd %s \n",pattr->name);
    if (strcmp(pattr->name, "read") == 0)
	{  
		sscanf(buf, "%x %x", &reg_start , &lenth);
		return count;
	}
	if (strcmp(pattr->name, "write") == 0)
	{  
		sscanf(buf, "%x %x", &reg , &val);
		printk("write reg(0x%08x) =  val(0x%08x)\n", reg, val);
		REGISTER_WRITE32(reg, val);
		printk("read:0x%08x = 0x%08x\n",reg,REGISTER_READ32(reg));
		return count;
	}
  return count;
}


static DEVICE_ATTR(read, 0664, hdmirx_core_show_info, hdmirx_core_store_info);

static DEVICE_ATTR(write, 0664, hdmirx_core_show_info, hdmirx_core_store_info);

static struct device_attribute *hdmirx_attr_list[] = {
	&dev_attr_read,
	&dev_attr_write,
};

int mt_hdmirx_create_attr(struct device *dev) 
{
    int idx, err = 0;
    int num = (int)(sizeof(hdmirx_attr_list)/sizeof(hdmirx_attr_list[0]));
    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) {
        if ((err = device_create_file(dev, hdmirx_attr_list[idx])))
            break;
    }
    
    return err;
}
/*---------------------------------------------------------------------------*/
int mt_hdmirx_delete_attr(struct device *dev)
{
    int idx ,err = 0;
    int num = (int)(sizeof(hdmirx_attr_list)/sizeof(hdmirx_attr_list[0]));
    
    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) 
        device_remove_file(dev, hdmirx_attr_list[idx]);

    return err;
}

bool bDQ = false;
static void process_dbg_opt(const char *opt)
{
    DGI_VIDEO_IN_INFO_T rInput;
    INPUT_DEVICE_INFO_T rVdoInInfo;
	HDMI_RX_STATUS_MODE rHDMIInmode;
	enum HDMI_SWITCH_NO rHDMIPortSwitch;
    u32 reg;
    unsigned int val;
    unsigned int vadr_regstart;
    unsigned int vadr_regend;
    unsigned int eDgiFMTMode;
    unsigned int eDgiInputMode;
    unsigned int res;
	unsigned int debugmode;
	unsigned char eSrcType;
	unsigned char eVideoinDramFmt;
	unsigned char eVideoinPutFmt;
	UINT32 temp_len = 0; 
	UINT32 len = 0;
	char *buf;
	buf = opt;
	VIDEO_IN_REQBUF requestBuffer;
	VIDEO_IN_BUFFER_INFO videoInBuffer;
	int i, temp_va = 0;
	void* obj[4];
    static int internal_buffer_init = 0;
    M4U_PORT_STRUCT portStruct;
    int tmpBufferSize;
	int videoinbuffersize;
	
    printk("[HDMIRX]%s\n", __func__);
    if (0 == strncmp(buf, "phyinit", 7))
    {
        _HdmiRxPHYInit();
    }
    else if (0 == strncmp(buf, "gtimingnoreset", 14))
    {
        _GetTimingInfomationNoreset();
    }
	else if(0 == strncmp(buf, "hdmirxtaskinit", 14))
	{
		hdmi_rx_internal_init();  
	}
	else if (0 == strncmp(buf, "hdmirxtmrisr", 12))
	{
		hdmi_rx_tmr_isr();
	}
	else if (0 == strncmp(buf, "setrepeatermode:", 16))
	{
		sscanf(buf+16, "%d,%d", (int*)(&rHDMIInmode),(int*)(&rHDMIPortSwitch));
		printk("rHDMIInmode = %d,rHDMIPortSwitch = %d\n",rHDMIInmode,rHDMIPortSwitch);
		hdmirx_internal_power_on();
		VdoIn_Switch(VIN_HDMI_1);
		vHdmiRepeaterMode(rHDMIInmode,rHDMIPortSwitch);
		vEnableHdmiRxAudTask(1);
	}
    else if (0 == strncmp(buf, "hdmirxcrc:", 10))
    {
		int crctime;
		sscanf(buf+10, "0x%x", &crctime);
	    _HdmiRxCrcCheck(crctime);
    }
    else if (0 == strncmp(buf, "bypassmode", 10))
    {
		//sscanf(buf+11, "%d/%d", &eDgiFMTMode,&eDgiInputMode);
		rInput.ePinType = YC444_36BIT;
		rInput.eInputMode = FMT_601_MODE;
		res = u4GetHdmiRxRes();
		printk("BYPASS MODE FMT = %d,eInputMode = %d,res = %d\n",eDgiFMTMode,eDgiInputMode,res);
		vset_dgi_in_mode(rInput,res);
    }
    else if (0 == strncmp(buf, "drammode:", 9))
    {
		//sscanf(buf+9, "%d/%d/%d/%d",&eSrcType,&res,&eVideoinDramFmt,&eVideoinPutFmt);
		
		sscanf(buf+9, "%d",&res);
		rVdoInInfo.eInputMode = FMT_601_MODE;
		rVdoInInfo.ePinType = YC444_36BIT;
		rVdoInInfo.eInputRes = (HDMI_RESOLUTION_MODE_T)res;
		rVdoInInfo.rVdoInWDramType.eVdoInAddrMode = VIN_LINEAR;
		rVdoInInfo.rVdoInWDramType.eVdoInSwapMode = VIN_SWAP_MODE_0;
		rVdoInInfo.rVdoInWDramType.eVdoInDramFmt = VIN_422;
		rVdoInInfo.eVdoInAR = SRC_ASP_16_9_FULL;
		rVdoInInfo.fgVgaIsCeaType = FALSE;
		rVdoInInfo.eDeviceId = VIN_HDMI_1;
		
		vin_hal_set_src_type(rVdoInInfo.eDeviceId); //internal Rx
			
		memset(&videoInBuffer, 0, sizeof(VIDEO_IN_BUFFER_INFO));
        
        if(fgVinSetCfgInfo(rVdoInInfo))
        {
			vVinSetPBType(VIN_PB_TYPE_START_CMD);
		}
    }
    else if (strncmp(buf, "w:",2) == 0)
	{
		sscanf(buf+2, "%x=%x", &reg , &val);
		printk("w:0x%08x=0x%08x\n", reg, val);
		HDMIRX_PRINTF("w:0x%08x=0x%08x\n", reg, val);
        *(volatile unsigned int*)(reg) = val;	
	}
    else if (strncmp(buf, "r:",2) == 0)
	{
		sscanf(buf+2, "%x/%x", &vadr_regstart , &vadr_regend);
		vadr_regend  &= 0x3ff;
		printk("r:0x%08x/0x%08x\n", vadr_regstart, vadr_regend);
		vadr_regend = vadr_regstart + vadr_regend;
		while (vadr_regstart <= vadr_regend)
		{
			 printk("0x%08x = 0x%08x\n",vadr_regstart,*(volatile unsigned int*)(vadr_regstart));
			 HDMIRX_PRINTF("0x%08x = 0x%08x\n",vadr_regstart,*(volatile unsigned int*)(vadr_regstart));
			 vadr_regstart += 4;
		}
	}
	else if(strncmp(buf, "rgcstt",6) == 0)
	{
		vShowHDMIRxStatus();
	}
	else if(strncmp(buf, "loadedid",8) == 0)
	{
		vLoadEdidFromCODE();
	}
	else if(strncmp(buf, "rgifrm",6) == 0)
	{
		vShowHDMIRxInfo();
	}
	else if(strncmp(buf, "rgbstt",6) == 0)
	{
		vShowRxHDCPBstatus();
	}
	else if(strncmp(buf, "rgpa",4) == 0)
	{
		vShowEdidPhyAddress();
	}
    else if(strncmp(buf, "rslog:", 6) == 0)
    {
        sscanf(buf+6, "0x%x", &debugmode);
        vsw_log_enable(debugmode);
    }
	else if(strncmp(buf, "showvinstatus",13) == 0)
	{
		vShowvideoinstatus();
	}
	else if (strncmp(buf, "requestbuf:", 11) == 0) {
		memset(&requestBuffer, 0, sizeof(VIDEO_IN_REQBUF));
		sscanf(buf+11, "%d", &requestBuffer.u4BufCount);
		printk("vadr_regstart: %d\n", requestBuffer.u4BufCount);
		vRequestBuffer(&requestBuffer);
	} 
	else if (strncmp(buf, "initbuf", 7) == 0) 
	{
		printk("VinBufQueue, running initbuf command\n");
		videoinbuffersize = 1920 *1080 ;
		memset(&videoInBuffer, 0, sizeof(VIDEO_IN_BUFFER_INFO));
		for (i = 0; i < 2; i++) {
			
			obj[i*2] = (unsigned int)vmalloc(videoinbuffersize);
			obj[i*2+1] = (unsigned int)vmalloc(videoinbuffersize);
			printk("VinBufQueue, kzalloc memory (%d), obj[%d]=0x%x, obj[%d]=0x%x\n", \
				i, i*2, obj[i*2], i*2+1, obj[i*2+1]);
			memset(obj[i*2], 0xfe, videoinbuffersize);
			memset(obj[i*2+1], 0xfe, videoinbuffersize);
			
			if (obj[i*2] == NULL || obj[i*2+1] == NULL) {
				printk("VinBufQueue, kzalloc memory failed, i(%d), obj[%d]=0x%x, obj[%d]=0x%x\n", \
					i, i*2, obj[i*2], i*2+1, obj[i*2+1]);
				return;
			}
			if (m4u_alloc_mva(BDP_WR_CHANNEL_VDI,
						obj[i*2],
						videoinbuffersize,
						0,
						0,
						&videoInBuffer.wch1m4uva[i*2]))
			{
				printk("m4u_alloc_mva for videoInBuffer.wch1mva fail\n");
				return;
			}
			
			if (m4u_alloc_mva(BDP_WR_CHANNEL_VDI,
						obj[i*2+1],
						videoinbuffersize,
						0,
						0,
						&videoInBuffer.wch1m4uva[i*2+1]))
			{
				printk("m4u_alloc_mva for videoInBuffer.wch1mva fail\n");
				return;
			}
			m4u_dma_cache_maint(BDP_WR_CHANNEL_VDI,
					(void const *)obj[i*2],
					videoinbuffersize,
					DMA_BIDIRECTIONAL);
			
			m4u_dma_cache_maint(BDP_WR_CHANNEL_VDI,
					(void const *)obj[i*2+1],
					videoinbuffersize,
					DMA_BIDIRECTIONAL);
			
			printk("i = %d @L%d\n",i,__LINE__);
			videoInBuffer.u4BufIndex = i;
			
			videoInBuffer.vAddrInfo.u4YAddr = videoInBuffer.wch1m4uva[i*2];
			videoInBuffer.vAddrInfo.u4CAddr = videoInBuffer.wch1m4uva[i*2+1];
			videoInBuffer.vAddrInfo.u4CBufferSize = 1920*1080;
			videoInBuffer.vAddrInfo.u4YBufferSize = 1920*1080;
			printk("videoInBuffer.u4BufIndex = %d @L%d\n",videoInBuffer.u4BufIndex,__LINE__);
			vInitBuffer(&videoInBuffer);
		}
		printk("VinBufQueue, end initbuf command\n");
	}
	else if (strncmp(buf, "startDQ:", 8) == 0) {
		sscanf(buf+8, "%d", &bDQ);
		printk("bDQ = %d\n",bDQ);
		
		if (bDQ == 0) {
			bDQ = false;
		} else {
			bDQ = true;
		}
		printk("VinBufQueue, bDQ: %d\n", bDQ);
	}
	else if(strncmp(buf, "streamon", 8) == 0) {
		vStreamOn();
	}
	else if(strncmp(buf, "streamoff", 9) == 0) {
		vStreamOff();
	}
	else if(strncmp(buf, "hdmirxhdcpmode:", 15) == 0)
	{
		int hdcpmode;
		sscanf(buf+15, "%d", &hdcpmode);
		printk(" hdcpmode = %d\n", hdcpmode);
		RxHdcpMode(hdcpmode);
	}
	else if(strncmp(buf, "audioreset", 10) == 0)
	{
		_HdmiRxAudioReset();
	}
	else if(strncmp(buf, "hbr", 3) == 0)
	{
		_HdmiRxHBR();
	}
	else if(strncmp(buf, "spdifhbrbypass", 14) == 0)
	{
		_HdmiRxHBRSPDIFBYPASS();
	}
	else if(strncmp(buf, "i2sbypass", 9) == 0)
	{
		_HdmiRxI2SBypass();
	}
	else if(strncmp(buf, "powerhdmirx", 11) == 0)
	{
		hdmirx_internal_power_on();
	}
	else if(strncmp(buf, "changeport", 10) == 0)
	{
		int portmode;
		sscanf(buf+10, "%d", &portmode);
		printk(" hdcpmode = %d\n", portmode);
		vHalRxSwitchPortSelect(1);
	} else if(strncmp(buf, "configinfo", 10) == 0) {
		VID_VDOIN_ALL_INFO_T VideoInCfgInfo;
		VideoInCfgInfo.eVddoInInfoType = VIN_GET_INFO_TYPE_ALL;
		video_in_config_info_get(&VideoInCfgInfo);;
	}
	else if(strncmp(buf, "gtime", 5) == 0)
	{
	UINT16 u2ActiveWidth, u2ActiveHigh, u2V2ACTIVELINES;
	UINT16 u2VFRONTPORCH, u2HFRONTPORCH, u2HsyncActWidth;
	UINT16 u2HTotal, u2VTotal;
	UINT32 u4Data,bTimingSearch1;
	UINT8 u1HResStable, u1VResStable;
	UINT16 wDviHclk, wDviHtotal,wDviWidth = 0;
	UINT8 bDviVclk;
	UINT8 u1InputPClkX, u1VSYNCPOL, u1HSYNCPOL;
	BOOL fgInterlace = 0;
	UINT32 u4DeepColor=0;
	u4Data = vRxReadReg(0xC00604c);
	u2ActiveWidth= (UINT16)((u4Data >> 16)&0xfff);

	u4Data = vRxReadReg(0xC006050);
	u2ActiveHigh = 	(UINT16)(u4Data & 0xfff);
	u2V2ACTIVELINES =  (UINT16)((u4Data >>16) & 0x7f);
	u2VFRONTPORCH =  (UINT16)((u4Data >>24) & 0x3f);

	u4Data = vRxReadReg(0xC006038);
	u2HTotal = (UINT16)((u4Data >>16) & 0x1fff);
	u1HResStable = (u4Data & 0x80000000)? 1:0;

	u4Data = vRxReadReg(0xC00603c);
	u2VTotal = (UINT16)(u4Data & 0x0fff);
	u1VResStable = (u4Data & 0x00008000)? 1:0;

	u4Data = vRxReadReg(0xC006008);
	if((u4Data & 0x30)== 0x00)
	{
		u1InputPClkX = 1; //no repeat
	}
	else if((u4Data & 0x30)== 0x10)
	{
		u1InputPClkX = 2; //repeat 1 times, 2x Pclk
		u2HTotal <<=  1;
		u2ActiveWidth <<= 1;
	}
	else if((u4Data & 0x30)== 0x30)
	{
		u1InputPClkX = 4; //repeat 2 times, 4x Pclk
		u2HTotal <<=  2;
		u2ActiveWidth <<= 2;
	}

	u4Data = vRxReadReg(0xC006054);
	u1VSYNCPOL = (u4Data >> 9)&0x01;
	u1HSYNCPOL = (u4Data >> 8)&0x01;


	u4Data = vRxReadReg(0xC006058);
	u2HFRONTPORCH = (u4Data >> 8)&0xff;
	u2HsyncActWidth = ((u4Data >> 24)&0xff)|((vRxReadReg(0xC00605c)&0x3)<<8);

	fgInterlace = fgHDMIinterlaced();
	wDviHclk = dwHDMILineFreq();
	bDviVclk = bHDMIRefreshRate();
	wDviHtotal =  wHDMIHTotal();
	u4DeepColor = (vRxReadReg(0xC006004)>>28)&0x03;

	HDMIRX_PRINTF("**************Timing Information Start**************************\n");
	if(u1HResStable)
		HDMIRX_PRINTF("H resolution Stable\n");
	else
		HDMIRX_PRINTF("H resolution unStable\n");

	if(u1VResStable)
		HDMIRX_PRINTF("V resolution Stable\n");
	else
		HDMIRX_PRINTF("V resolution unStable\n");

	HDMIRX_PRINTF("H Active = %d\n", u2ActiveWidth);
	HDMIRX_PRINTF("V Active = %d\n", u2ActiveHigh);
	if(fgInterlace)
		HDMIRX_PRINTF("I/P = Interlace\n");
	else
		HDMIRX_PRINTF("I/P = Progressive\n");

	HDMIRX_PRINTF("H Total = %d\n", u2HTotal);
	HDMIRX_PRINTF("V Total = %d\n", u2VTotal);
	HDMIRX_PRINTF("H Freq = %d hz\n", wDviHclk*100);
	HDMIRX_PRINTF("V Freq = %d hz\n", bDviVclk);

	HDMIRX_PRINTF("Vsync Front Porch = %d \n", u2VFRONTPORCH);
	HDMIRX_PRINTF("Vsync width + back porch = %d\n", u2V2ACTIVELINES);
	HDMIRX_PRINTF("Hsync Front Porch = %d \n", u2HFRONTPORCH);
	HDMIRX_PRINTF("Hsync Active width = %d \n", u2HsyncActWidth);

	if(u1VSYNCPOL)
		HDMIRX_PRINTF("VSYNC Polarity is Positive\n");
	else
		HDMIRX_PRINTF("VSYNC Polarity is Negative\n");

	if(u1HSYNCPOL)
		HDMIRX_PRINTF("HSYNC Polarity is Positive\n");
	else
		HDMIRX_PRINTF("HSYNC Polarity is Negative\n");

	if(u4DeepColor == 0)
	  HDMIRX_PRINTF("Rx detect 8 BIT \n");
	else if(u4DeepColor == 1)
	  HDMIRX_PRINTF("Rx detect 10 BIT \n");
	else if(u4DeepColor == 2)
	  HDMIRX_PRINTF("Rx detect 12 BIT \n");
	else if(u4DeepColor == 3)
	  HDMIRX_PRINTF("Rx detect 16 BIT \n");

	if(vRxReadReg(0xC006134)&0x01)
	HDMIRX_PRINTF("HDMI Mode \n");
	else
	HDMIRX_PRINTF("DVI Mode \n");


	HDMIRX_PRINTF("AUDIO PACKET layer =0x%x \n",(vRxReadReg(0xC006134)>>3)&0x01);

	HDMIRX_PRINTF("AUDIO PACKET valid =0x%x \n",(vRxReadReg(0xC006294)&0x0F));
		{
	HDMIRX_PRINTF("SPDIF channel status BYTE1~BYTE5 =0x%x,0x%x,0x%x,0x%x,0x%x \n",((vRxReadReg(0xC006128)>>16)&0xFF),
		((vRxReadReg(0xC006128)>>24)&0xFF),((vRxReadReg(0xC00612C))&0xFF),((vRxReadReg(0xC006130))&0xFF),((vRxReadReg(0xC006130)>>8)&0xFF));
		}
	HDMIRX_PRINTF("**************Timing Information End****************************\n");
	}
	else if(strncmp(buf, "printcrc:", 9) == 0)
	{
			//to overwrite this function
			UINT8 idx;
			UINT32 crccnt = 0;
			UINT8 result[3][3];
			UINT8 tmp[3];
			int ntry;
			
			sscanf(buf+9, "%d", &ntry);
		
			idx = 0;
			crccnt ++;
			result[0][0] = 0;
			result[0][1] = 0;
			result[0][2] = 0;
			result[1][0] = 0;
			result[1][1] = 0;
			result[1][2] = 0;
			result[2][0] = 0;
			result[2][1] = 0;
			result[2][2] = 0;
		
			_CrcResult[0][0] =0;
			_CrcResult[0][1] =0;
			_CrcResult[0][2] =0;
			_CrcResult[1][0] =0;
			_CrcResult[1][1] =0;
			_CrcResult[1][2] =0;
			_CrcResult[2][0] =0;
			_CrcResult[2][1] =0;
			_CrcResult[2][2] =0;
			tmp[0]=0;
			tmp[1]=0;
			tmp[2]=0;
			
			HDMIRX_PRINTF( "fgHDMICRC: %d \r\n", ntry);
		
			HDMIRX_CLR_BIT(REG_VID_CRC_CHK, 23);
			
			while (ntry > 0)
			{
				ntry--;
				//vUtDelay10ms(1); // NOTE: IT IS NECESSARY
				mdelay(10);
				
				if (idx > 2)
				{
					HDMIRX_PRINTF("CRC fail\n");
					_CrcResult[0][0] =result[0][0];
					_CrcResult[0][1] =result[0][1];
					_CrcResult[0][2] =result[0][2];
					_CrcResult[1][0] =result[1][0];
					_CrcResult[1][1] =result[1][1];
					_CrcResult[1][2] =result[1][2];
					_CrcResult[2][0] =tmp[0];
					_CrcResult[2][1] =tmp[1];
					_CrcResult[2][2] =tmp[2];
					HDMIRX_PRINTF( "FAIL: %d ,idx= %d\r\n", ntry,idx);
					HDMIRX_PRINTF("%x %x %x\r\n", result[0][0], result[0][1], result[0][2]);
					HDMIRX_PRINTF("%x %x %x\n", result[1][0], result[1][1], result[1][2]);
					return 0;
				}
				//vRegWrite1B(VID_CRC_CHK_2, 0x8c);// clr
				HDMIRX_WRITE32_MASK(REG_VID_CRC_CHK, (0x8c<<16), (0xff<<16));
				//PRINT_REG(0x22c68);
		
				while ((HDMIRX_READ32(REG_VID_CRC_CHK)&(1<<23)) != 0x0)
				{
					mdelay(1);
		
				}
				while (HDMIRX_READ32(REG_VID_CRC_CHK)&(0xff<<24) != 0x00)
				{
					mdelay(1);
				}
				while (HDMIRX_READ32(REG_VID_CRC_CHK)&(0xff)  != 0x00)
				{
					mdelay(1);
				}
				while (HDMIRX_READ32(REG_VID_CRC_CHK)&(0xff<<8) != 0x00)
				{
					
					mdelay(1);
				}
				if (( (HDMIRX_READ32(REG_VID_CRC_CHK))&(0x81<<16)) ==0x0)
				{
					//vRegWrite1B(VID_CRC_CHK_2, 0x0d);// start trigger
					HDMIRX_WRITE32_MASK(REG_VID_CRC_CHK, (0x0d<<16), (0xff<<16));
					//while (u1RegRead1B(VID_CRC_CHK_2)  != 0x8d)
					while (!(HDMIRX_READ32(REG_VID_CRC_CHK)&0x00800000))
					{
						mdelay(1);
					}
					
					//vRegWrite1B(VID_CRC_CHK_2, 0x0c);
					HDMIRX_WRITE32_MASK(REG_VID_CRC_CHK, (0x0c<<16), (0xff<<16));
					//PRINT_REG(0x22c68);
					if (((HDMIRX_READ32(REG_VID_CRC_CHK))&(0x00800000)))
					{
						//HDMI_LOG(HDMI_LOG_DEBUG, "CRC ready\r\n");
						tmp[0] = ((HDMIRX_READ32(REG_VID_CRC_CHK))>>24)&(0xff);
						tmp[1] = HDMIRX_READ32(REG_VID_CRC_CHK)&(0xff);
						tmp[2] = ((HDMIRX_READ32(REG_VID_CRC_CHK))>>8)&(0xff);
						
						HDMIRX_PRINTF( "[TMP]%x %x %x\n", tmp[0],	tmp[1],  tmp[2]);
						// vUtDelay10ms(2);
						// compare and update result if necessary
						if ((tmp[0] == result[0][0]) && (tmp[1] == result[0][1]) && (tmp[2] == result[0][2]))
						{
							continue;
						}
						if ((tmp[0] == result[1][0]) && (tmp[1] == result[1][1]) && (tmp[2] == result[1][2]))
						{
							continue;
						}
						//ASSERT((idx<3));
						if(idx>=3) {
							HDMIRX_PRINTF("#################################################\r\n");
							HDMIRX_PRINTF("############	FAIL CRC ########################\r\n");
							HDMIRX_PRINTF("#################################################\r\n");
							return 0;
						}
				   
						result[idx][0] = tmp[0];
						result[idx][1] = tmp[1];
						result[idx][2] = tmp[2];
						
						idx++;
						continue;
					}
					else
					{
						//PRINT_REG(0x22c68);
						printk("CRC is not ready\n");
						return 0;
					}
				}
				else
				{
					printk( "reset CRC fail");
					return 0;
				}
			}
		
			if(HDMIRX_READ32(REG_VID_CH_MAP)&(1<<10))
			{
				HDMIRX_PRINTF( "interlace signal\r\n");
			}
			else
			{
				HDMIRX_PRINTF( "progressive signal~\r\n");
			}
		
			HDMIRX_PRINTF( "idx = %d:\r\n",idx);
			if (idx == 1)
			{
				HDMIRX_PRINTF( "jiffies : %ld\n", jiffies);
				HDMIRX_PRINTF( "assume progressive signal\r\n");
				HDMIRX_PRINTF( "CRC result:\r\n");
				HDMIRX_PRINTF("%x %x %x\r\n", result[0][0], result[0][1], result[0][2]);
			}
			else if (idx == 2)
			{
				HDMIRX_PRINTF( "assume interlaced signal\n");
				HDMIRX_PRINTF( "CRC result:\n");
				HDMIRX_PRINTF( "%x %x %x\n", result[0][0], result[0][1], result[0][2]);
				HDMIRX_PRINTF("%x %x %x\n", result[1][0], result[1][1], result[1][2]);
			}
			else
			{
				HDMIRX_PRINTF( "############################abort(idx>=3)#####################\n");
			}
			_CrcResult[0][0] =result[0][0];
			_CrcResult[0][1] =result[0][1];
			_CrcResult[0][2] =result[0][2];
			_CrcResult[1][0] =result[1][0];
			_CrcResult[1][1] =result[1][1];
			_CrcResult[1][2] =result[1][2];
	}
	else if(strncmp(buf, "edid", 4) == 0)
	{
		vUserSetHDMIRxEDIDToDrv(interediddata);
	}else if(strncmp(buf, "hdcp", 4) == 0) {
	
		int testhdcp;
		sscanf(buf+4, "%d", &testhdcp);
		printk(" testhdcp = %d\n", testhdcp);
		vSetHDMIRXHDCP(testhdcp);
	}else if(strncmp(buf, "readedid:", 9) == 0) {
	
		int edidnum;
		sscanf(buf+4, "%d", &edidnum);
		printk(" edidnum = %d\n", edidnum);
		vDumpHdmiRxEdid(edidnum);
	}
    else
    {
        goto Error;
    }

    return;

Error:
    printk("[hdmirx] parse command error!\n\n%s", STR_HELP);
}

static void process_dbg_cmd(char *cmd)
{
    char *tok;

    printk("[hdmirx] %s\n", cmd);

    while ((tok = strsep(&cmd, " ")) != NULL)
    {
        process_dbg_opt(tok);
    }
}

// ---------------------------------------------------------------------------
//  Debug FileSystem Routines
// ---------------------------------------------------------------------------

struct dentry *videoin_dbgfs = NULL;


static ssize_t debug_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}


static char debug_buffer[2048];

static ssize_t debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
    int n = 0;
    n = strlen(debug_buffer);
    debug_buffer[n++] = 0;

    return simple_read_from_buffer(ubuf, count, ppos, debug_buffer, n);
}


static ssize_t debug_write(struct file *file,
                           const char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(debug_buffer) - 1;
    size_t ret;

    ret = count;

    if (count > debug_bufmax)
    {
        count = debug_bufmax;
    }

    if (copy_from_user(&debug_buffer, ubuf, count))
    {
        return -EFAULT;
    }

    debug_buffer[count] = 0;

    process_dbg_cmd(debug_buffer);

    return ret;
}


static struct file_operations debug_fops =
{
    .read  = debug_read,
    .write = debug_write,
    .open  = debug_open,
};


void HDMIRX_DBG_Init(void)
{
    printk("[HDMIRX]%s\n", __func__);
    videoin_dbgfs = debugfs_create_file("hdmirx",
                                       S_IFREG | S_IRUGO, NULL, (void *)0, &debug_fops);
}


void HDMIRX_DBG_Deinit(void)
{
    debugfs_remove(videoin_dbgfs);
}

#endif
