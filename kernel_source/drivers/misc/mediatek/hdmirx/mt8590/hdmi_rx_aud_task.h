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
#ifndef _HDMI_RX_AUD_TASK_H_
#define _HDMI_RX_AUD_TASK_H_


#include "typedef.h"
#include "video_in_if.h"


#define HDMIRX_INTR_STATE1 0x078
#define HDMIRX_AFIFO_UNDERRUN        ((unsigned)0x1 <<  0)   //Audio FIFO under-run. This interrupt bit is not set for all under-run conditions
#define HDMIRX_AFIFO_OVERRUN         ((unsigned)0x1 <<  1)   //Audio FIFO over-run.
#define HDMIRX_CTS_REUSED            ((unsigned)0x1 <<  2)
#define HDMIRX_CTS_DROPPED           ((unsigned)0x1 <<  3)
#define HDMIRX_TERC4_ERROR           ((unsigned)0x1 <<  5)    //TERC4 error. Set when number of TERC4 errors exceeds threshold
#define HDMIRX_HDCP_ERROR         ((unsigned)0x1 <<  6)  //HDCP error. Set when decryption fails.
#define HDMIRX_AUD_FS_CHG            ((unsigned)0x1 <<  8)   //Audio FS sample rate changed. Write 1 to clear
#define HDMIRX_SPDF_LR_ERR           ((unsigned)0x1 << 17)   //SPDIF left/right error
#define HDMIRX_AFIFO_PREOVERRUN      ((unsigned)0x1 << 22)   //Audio FIFO pre over-run
#define HDMIRX_AFIFO_PREUNDERRUN     ((unsigned)0x1 << 23)   //Audio FIFO pre under-run
#define HDMIRX_DSD_PACKET            ((unsigned)0x1 << 24)   //One Bit Audio(SACD) Packet Received.
#define HDMIRX_HBR_PACKET            ((unsigned)0x1 << 25)   //HBR Packet Received.
#define HDMIRX_AUD_CHSTS_CHG         ((unsigned)0x1 << 28)   //Audio Channel Status change


#define M_Fs 0xF
    #define B_Fs_UNKNOW    1
    #define B_Fs_44p1KHz    0
    #define B_Fs_48KHz  2
    #define B_Fs_32KHz  3
    #define B_Fs_88p2KHz    8
    #define B_Fs_96KHz  0xA
    #define B_Fs_176p4KHz   0xC
    #define B_Fs_192KHz 0xE
    #define B_Fs_768KHz 0x9 // 1001
    #define B_Fs_HBR 0x9 // 1001

#define B_CAP_AUDIO_ON  (1<<7)
#define B_CAP_HBR_AUDIO (1<<6)
#define B_CAP_DSD_AUDIO (1<<5)
#define B_LAYOUT        (1<<4)
#define B_MULTICH       (1<<4)
#define B_HBR_BY_SPDIF  (1<<3)

#define B_CAP_AUDIO_ON_SFT  7
#define B_CAP_HBR_AUDIO_SFT 6
#define B_CAP_DSD_AUDIO_SFT 5
#define B_MULTICH_SFT       4
#define B_HBR_BY_SPDIF_SFT  3

#define F_AUDIO_ON  (1<<7)
#define F_AUDIO_HBR (1<<6)
#define F_AUDIO_DSD (1<<5)
#define F_AUDIO_NLPCM (1<<4)
#define F_AUDIO_LAYOUT_1 (1<<3)
#define F_AUDIO_LAYOUT_0 (0<<3)

#define B_AUDIO_ON    (1<<7)
#define B_HBRAUDIO    (1<<6)
#define B_DSDAUDIO    (1<<5)
#define B_AUDIO_LAYOUT     (1<<4)
#define M_AUDIO_CH         0xF
#define B_AUDIO_SRC_VALID_3 (1<<3)
#define B_AUDIO_SRC_VALID_2 (1<<2)
#define B_AUDIO_SRC_VALID_1 (1<<1)
#define B_AUDIO_SRC_VALID_0 (1<<0)

#define T_AUDIO_MASK 0xF0
#define T_AUDIO_OFF 0
#define T_AUDIO_HBR (F_AUDIO_ON|F_AUDIO_HBR)
#define T_AUDIO_DSD (F_AUDIO_ON|F_AUDIO_DSD)
#define T_AUDIO_NLPCM (F_AUDIO_ON|F_AUDIO_NLPCM)
#define T_AUDIO_LPCM (F_AUDIO_ON)

#define B_TRI_I2S3 (1<<7)
#define B_TRI_I2S2 (1<<6)
#define B_TRI_I2S1 (1<<5)
#define B_TRI_I2S0 (1<<4)
#define B_MUTE_CH3  (1<<3)
#define B_MUTE_CH2  (1<<2)
#define B_MUTE_CH1  (1<<1)
#define B_MUTE_CH0  (1<<0)

#define B_TRI_ALL  (B_TRI_VIDEOIO|B_TRI_VIDEO|B_TRI_SPDIF|B_TRI_I2S3|B_TRI_I2S2|B_TRI_I2S1|B_TRI_I2S0)
#define B_TRI_AUDIO  (B_TRI_I2S3|B_TRI_I2S2|B_TRI_I2S1|B_TRI_I2S0)

#define B_MUTE_AUDIO  (B_MUTE_CH3|B_MUTE_CH2|B_MUTE_CH1|B_MUTE_CH0)

#define B_TRI_MASK  ~(B_TRI_VIDEOIO|B_TRI_VIDEO|B_TRI_SPDIF|B_TRI_I2S3|B_TRI_I2S2|B_TRI_I2S1|B_TRI_I2S0)
#define B_TRI_I2S  (B_TRI_I2S3|B_TRI_I2S2|B_TRI_I2S1|B_TRI_I2S0)


typedef enum
{

 HDMI_RX_AUDIO_ON =1 ,
 HDMI_RX_AUDIO_MUTE ,
 HDMI_RX_AUDIO_UNMUTE,
 HDMI_RX_AUDIO_UNLOCK,
 HDMI_RX_AUDIO_BIT_STREAM_CHANGE,
 HDMI_RX_AUDIO_INFORFRAME_CHANGE,
 HDMI_RX_PLUG_OUT,
 HDMI_RX_ACPPKT_CHG,
 HDMI_RX_AUDIO_CHG_PAUSE_STATUS,
}HDMI_RX_AUDIO_INT_TYPE;

typedef struct {
    BYTE AudioFlag ;
    BYTE AudSrcEnable ;
    BYTE SampleFreq ;
    BYTE ChStat[5] ;
    Audio_InfoFrame AudInf;
    RX_REG_AUDIO_CHSTS AudChStat;
} AUDIO_CAPS ;


static const CHAR *AStateStr[6] =
{
    "ASTATE_AudioOff",
    "ASTATE_RequestAudio",
    "ASTATE_ResetAudio",
    "ASTATE_WaitForReady",
    "ASTATE_AudioOn",
    "ASTATE_Reserved"
};

typedef struct
{
	unsigned char	e_hdmi_aud_in;
	unsigned char	e_iec_frame;
	unsigned char	e_hdmi_fs;
	unsigned char	e_aud_code;
	unsigned char	u1Aud_Input_Chan_Cnt;
	unsigned char	e_I2sFmt;
	unsigned char	u1HdmiI2sMclk;
	unsigned char	bhdmi_LCh_status[5];
	unsigned char	bhdmi_RCh_status[5];
}	HDMIRX_SET_TXAUDIO_INFO;

void SetAudioMute(BOOL bMute);
void AssignAudioTimerTimeout(u32 TimeOut);
extern HDMIRX_AUDIO_INFO audio_para;

extern void vAudioPacketOff(unsigned char bOn);
extern void _HdmiRxHBRSPDIFBYPASS(void);
extern void HalHdmiRxDSDBypass(void);
extern void HalHdmiRxI2SBypass(void);
extern void vEnableHdmiRxAudTask(bool on);
extern void hdmirx_audio_task(void);


void SwitchAudioState(Audio_State_Type state);

#endif
