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
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "hdmi_rx_aud_task.h"
#include "hdmi_rx_ctrl.h"
#include "hdmi_rx_hw.h"
#include "hdmitable.h"
#include "hdmi_rx_hal.h"
#include "hdmiedid.h"
#include "internal_hdmi_drv.h"
#include <mach/mt_clkmgr.h>
#include "mt8590-afe.h"

#define RX_APLL_UNSTABLE_CNT_REACHED (50)  /*50 times, 20ms each time.*/

Audio_State_Type AState = ASTATE_AudioOff;
static  AUDIO_CAPS AudioCaps;
#define AUDIO_READY_TIMEOUT   1
BOOL hdmirxbypassmode = TRUE;
BOOL _fgHDMIRxAudTaskEnable = FALSE;
u8 _u1HDMIAFifoErrCnt = 0;
static u32 AudioCountingTimer = 0;
static BOOL s_fgNewHBRFound = FALSE;
HDMIRX_AUDIO_INFO audio_para;

extern void vHwSet_Hdmi_I2S_C_Status(unsigned char *prLChData, unsigned char *prRChData);
extern int hdmi_audiosetting(HDMIDRV_AUDIO_PARA *audio_para);

void hdmirx_settxaud(void)
{
	HDMIRX_SET_TXAUDIO_INFO tAudioPara;
	BYTE fs = u1HalHDMIRxAudFsGet()& M_Fs;
	BYTE audioflag  = ((fgHalHDMIRxAudioPkt() << B_CAP_AUDIO_ON_SFT)
			   | (fgHalHDMIRxHDAudio() << B_CAP_HBR_AUDIO_SFT)
			   | (fgHalHDMIRxDSDAudio() << B_CAP_DSD_AUDIO_SFT));
	tAudioPara.e_hdmi_aud_in = (u1HalHDMIRxAudioCHSTAT0() & 0x2) >> 1;
	tAudioPara.u1Aud_Input_Chan_Cnt = (u1HalHDMIRxAudioCHSTAT2() & 0xf0) >> 4;
	printk("audioflag =0x%x\n", audioflag);
	if (audioflag & B_DSDAUDIO) {
		tAudioPara.e_aud_code = AVD_DSD;
		tAudioPara.u1HdmiI2sMclk = MCLK_512FS;
		fs = B_Fs_44p1KHz;
	} else if (audioflag & B_HBRAUDIO){
	        tAudioPara.e_hdmi_aud_in = SV_SPDIF;
		tAudioPara.e_aud_code = AVD_LPCM;
		tAudioPara.e_iec_frame = IEC_768K;
		fs = B_Fs_192KHz;
	}
	else {
		tAudioPara.e_aud_code = AVD_LPCM;
		tAudioPara.u1HdmiI2sMclk = MCLK_128FS;
	}
	switch (fs) {
	case B_Fs_32KHz:
		tAudioPara.e_hdmi_fs = HDMI_FS_32K;
		break;
	case B_Fs_44p1KHz:
		tAudioPara.e_hdmi_fs = HDMI_FS_44K;
		break;
	case B_Fs_48KHz:
		tAudioPara.e_hdmi_fs = HDMI_FS_48K;
		break;
	case B_Fs_88p2KHz:
		tAudioPara.e_hdmi_fs = HDMI_FS_88K;
		break;
	case B_Fs_96KHz:
		tAudioPara.e_hdmi_fs = HDMI_FS_96K;
		break;
	case B_Fs_176p4KHz:
		tAudioPara.e_hdmi_fs = HDMI_FS_176K;
		break;
	case B_Fs_192KHz:
		tAudioPara.e_hdmi_fs = HDMI_FS_192K;
		break;
	case B_Fs_768KHz:
		tAudioPara.e_hdmi_fs = HDMI_FS_768K;
		break;
	default:
		tAudioPara.e_hdmi_fs = HDMI_FS_48K;
		break;
	}
	tAudioPara.e_I2sFmt = HDMI_I2S_24BIT;
	tAudioPara.bhdmi_LCh_status[0] = u1HalHDMIRxAudioCHSTAT0();
	tAudioPara.bhdmi_LCh_status[1] = u1HalHDMIRxAudioCHSTAT1();
	tAudioPara.bhdmi_LCh_status[2] = u1HalHDMIRxAudioCHSTAT2();
	tAudioPara.bhdmi_LCh_status[3] = u1HalHDMIRxAudioCHSTAT3();
	tAudioPara.bhdmi_LCh_status[4] = u1HalHDMIRxAudioCHSTAT4();
	tAudioPara.bhdmi_RCh_status[0] = u1HalHDMIRxAudioCHSTAT0();
	tAudioPara.bhdmi_RCh_status[1] = u1HalHDMIRxAudioCHSTAT1();
	tAudioPara.bhdmi_RCh_status[2] = u1HalHDMIRxAudioCHSTAT2();
	tAudioPara.bhdmi_RCh_status[3] = u1HalHDMIRxAudioCHSTAT3();
	tAudioPara.bhdmi_RCh_status[4] = u1HalHDMIRxAudioCHSTAT4();
	hdmi_audiosetting(&tAudioPara);
}

void SwitchAudioState(Audio_State_Type state)
{
	//HDMI_RX_AUDIO_INT_TYPE eIRQSrcSend;
	AState = state ;
	if (!_fgHDMIRxAudTaskEnable)
		return;
	printk("SwitchAudioState AState[%d] -> %s\n", AState, AStateStr[AState]) ;
	switch (AState) {
	case ASTATE_AudioOff:
		vNotifyAppState(HDMI_RX_AUD_STATUS_UNLOCK);
		SetAudioMute(TRUE) ;
		vAudioPacketOff(TRUE);
		break ;
	case ASTATE_WaitForReady:
		AssignAudioTimerTimeout(AUDIO_READY_TIMEOUT) ;
		break ;
	case ASTATE_AudioOn:
		vNotifyAppState(HDMI_RX_AUD_STATUS_LOCK);
		if (hdmirxbypassmode)
			hdmirx_settxaud();
		SetAudioMute(OFF) ;
		vAudioPacketOff(FALSE);
		break ;
	default:
		break;
	}
}

void AssignAudioTimerTimeout(u32 TimeOut)
{
	AudioCountingTimer = TimeOut ;
}

void SetAudioMute(BOOL bMute)
{
	if (bMute) {
		//Mute
		vHalHDMIRxSetAudMuteCH((UINT8)B_MUTE_AUDIO);
		printk("Audio Mute\n");
	} else {
		//Un-Mute
		vHalHDMIRxSetAudMuteCH((UINT8)~B_MUTE_AUDIO);
		printk("Audio Un-Mute.\n") ;
	}
}

void enable_aud_clk_rxaud(int en)
{
	if (en) {
		a1sys_start(AUD1PLL, NULL, 1);
		enable_pll(AUD2PLL, "AUDIO");
		enable_pll(HADDS2PLL, "AUDIO");
		enable_clock(MT_CG_AUDIO_A2SYS, "AUDIO");
	} else {
		disable_clock(MT_CG_AUDIO_A2SYS, "AUDIO");
		disable_pll(HADDS2PLL, "AUDIO");
		disable_pll(AUD2PLL, "AUDIO");
		a1sys_start(AUD1PLL, NULL, 0);
	}
}


void vEnableHdmiRxAudTask(bool on)
{
	_fgHDMIRxAudTaskEnable = on;
	if (_fgHDMIRxAudTaskEnable) {
	        enable_aud_clk_rxaud(1);
		vHalEnableRxAudClk();
		vHalHDMIRxEnableAudPktReceive();
		vHalHDMIRxAudResetAudio();
		SwitchAudioState(ASTATE_RequestAudio);
		printk("Enable HDMI Rx Audio Task.\n");
	} else {
		vHalHdmiRxAudBypass(FALSE, FALSE);
		hdmirxbypassmode = FALSE;
	        enable_aud_clk_rxaud(0);
		printk("Disable HDMI Rx Audio Task.\n");
	}
}

void vGetInternalHDMIAudChannelStatus(UINT8 *u1AudChStatus)
{
	//L
	u1AudChStatus[0] = u1HalHDMIRxAudioCHSTAT0();
	u1AudChStatus[1] = u1HalHDMIRxAudioCHSTAT1();
	u1AudChStatus[2] = u1HalHDMIRxAudioCHSTAT2();
	u1AudChStatus[3] = u1HalHDMIRxAudioCHSTAT3();
	u1AudChStatus[4] = u1HalHDMIRxAudioCHSTAT4();
	//R
	u1AudChStatus[5] = u1HalHDMIRxAudioCHSTAT0();
	u1AudChStatus[6] = u1HalHDMIRxAudioCHSTAT1();
	u1AudChStatus[7] = u1HalHDMIRxAudioCHSTAT2();
	u1AudChStatus[8] = u1HalHDMIRxAudioCHSTAT3();
	u1AudChStatus[9] = u1HalHDMIRxAudioCHSTAT4();
}



BOOL GetAudioChannelStatus(RX_REG_AUDIO_CHSTS *RegChannelstatus, UINT8 audio_status)
{
	BYTE uc;
	if ((audio_status & T_AUDIO_MASK) == T_AUDIO_OFF) {
		// return false if no audio or one-bit audio.
		return FALSE ;
	}
	//SwitchHDMIRXBank(0); ??
	uc = u1HalHDMIRxAudioCHSTAT0();
	if ((audio_status & T_AUDIO_MASK) == T_AUDIO_HBR)
		RegChannelstatus->ISLPCM = 1 ;
	else if ((audio_status & T_AUDIO_MASK) == T_AUDIO_DSD)
		RegChannelstatus->ISLPCM = 0;
	else
		RegChannelstatus->ISLPCM = (uc & 0x02) >> 1;
	RegChannelstatus->CopyRight = (uc & 0x04) >> 2;
	RegChannelstatus->AdditionFormatInfo = (uc & 0x18) >> 3;
	RegChannelstatus->ChannelStatusMode = (uc & 0xE0) >> 5;
	RegChannelstatus->CategoryCode = u1HalHDMIRxAudioCHSTAT1();
	RegChannelstatus->SourceNumber = u1HalHDMIRxAudioCHSTAT2() & 0x0F;
	RegChannelstatus->ChannelNumber = (u1HalHDMIRxAudioCHSTAT2() & 0xf0) >> 4;
	uc = u1HalHDMIRxAudFsGet();
	if (fgHalHDMIRxHDAudio())
		RegChannelstatus->SamplingFreq = B_Fs_HBR;
	else
		RegChannelstatus->SamplingFreq = uc & 0x0F;
	RegChannelstatus->ClockAccuary = u1HalHDMIRxAudioCHSTAT3() & 0x03;
	RegChannelstatus->WorldLen = (u1HalHDMIRxAudioCHSTAT3() & 0xF0) >> 4;
	RegChannelstatus->OriginalSamplingFreq = (u1HalHDMIRxAudioCHSTAT4() & 0xF0) >> 4;
	return TRUE;
}

UINT32 u4HDMIRxAudErrorGet(void)
{
	UINT32 u4AudErr = vHalHDMIRxAudErrorGet();
	if (u4AudErr & HDMIRX_HBR_PACKET)
		printk("HDMI Rx: HDMIRX_HBR_PACKET.\n");
	if (u4AudErr & HDMIRX_DSD_PACKET)
		printk("HDMI Rx: HDMIRX_DSD_PACKET.\n");
	if (u4AudErr & HDMIRX_AFIFO_UNDERRUN)
		printk("HDMI Rx Audio Error : Audio Fifo Under-run.\n");
	if (u4AudErr & HDMIRX_AFIFO_OVERRUN)
		printk("HDMI Rx Audio Error : Audio Fifo Over-run.\n");
	if (u4AudErr & HDMIRX_AUD_FS_CHG)
		printk("HDMI Rx Audio Error : FS CHG Error.\n");
	if (u4AudErr & HDMIRX_TERC4_ERROR)
		printk("HDMI Rx Audio Error : TREC4 Packet Error.\n");
	if (u4AudErr & HDMIRX_HDCP_ERROR)
		printk("HDMI Rx Audio Error : HDCP Packet Error.\n");
	return u4AudErr;
}

void getHDMIRxInputAudio(AUDIO_CAPS *pAudioCaps)
{
	BYTE uc ;
	if (!pAudioCaps)
		return ;
	uc = u1HalHDMIRxAudFsGet() ;
	pAudioCaps->SampleFreq = uc & M_Fs ;
	audio_para.hdmirxaudfs = pAudioCaps->SampleFreq;
	pAudioCaps->AudioFlag = ((fgHalHDMIRxAudioPkt() << B_CAP_AUDIO_ON_SFT)
				 | (fgHalHDMIRxHDAudio() << B_CAP_HBR_AUDIO_SFT)
				 | (fgHalHDMIRxDSDAudio() << B_CAP_DSD_AUDIO_SFT));
	if (pAudioCaps->AudioFlag & B_HBRAUDIO)
	{
		pAudioCaps->SampleFreq = B_Fs_HBR;
	        audio_para.hbraudio = TRUE;
	}
	else
	{
		audio_para.hbraudio = FALSE;
	}
	if (pAudioCaps->AudioFlag & B_DSDAUDIO)
	{
		pAudioCaps->SampleFreq = B_Fs_44p1KHz;
		audio_para.dsdaudio = TRUE;
	}
	else
	{
		audio_para.dsdaudio = FALSE;
	}
	// Check n@polling  0x1F070[17] & 0x1F078[24] & 0x1F078[25] 0 Twaudio T
	if (pAudioCaps->AudioFlag != 0)
		pAudioCaps->AudioFlag |= B_CAP_AUDIO_ON;
	pAudioCaps->AudioFlag |= (fgHalHDMIRxMultiPCM() << B_MULTICH_SFT);
	if ((pAudioCaps->AudioFlag & B_LAYOUT) == B_MULTICH) {
		// multi channel layout
		audio_para.pcmchinfo = 1;
		pAudioCaps->AudSrcEnable = u1HalHDMIRxAudValidCHGet()&M_AUDIO_CH;
	} else {
		// 2 channel layout
		audio_para.pcmchinfo = 0;
		pAudioCaps->AudSrcEnable = B_AUDIO_SRC_VALID_0;
	}
	bHalGetAudioInfoFrame(&(pAudioCaps->AudInf));
	GetAudioChannelStatus(&(pAudioCaps->AudChStat), pAudioCaps->AudioFlag);
}

void vSetHdmiRxAudioI2SFmt(void)
{
	vHalSetRxI2sLRInv(FALSE);
	vHalSetRxI2sAudFormat(FORMAT_I2S, LRCK_CYC_32);
}


void SetupAudio(void)
{
	static UINT32 s_u4APLLUnstableRstCnt = 0;
	getHDMIRxInputAudio(&AudioCaps) ;
	if (AudioCaps.AudioFlag & B_CAP_AUDIO_ON) {
		static BOOL fgHBRDetected = FALSE;
		/*hbr*/
		if (AudioCaps.AudioFlag & B_CAP_HBR_AUDIO) {
			vHalSetHDMIRxHBR(TRUE);
			if (!fgHBRDetected)
				vHalHDMIRxAudResetAudio();
			fgHBRDetected = TRUE;
			s_fgNewHBRFound = TRUE;
		} else {
			vHalSetHDMIRxHBR(FALSE);
			fgHBRDetected = FALSE;
			s_fgNewHBRFound = FALSE;
		}
		/*dsd*/
		if (AudioCaps.AudioFlag & B_CAP_DSD_AUDIO)
			vHalSetHDMIRxDSD(TRUE);
		else
			vHalSetHDMIRxDSD(FALSE);
		/*not HBR, and not DSD*/
		if ((AudioCaps.AudioFlag & (B_CAP_DSD_AUDIO | B_CAP_HBR_AUDIO)) == 0)
			vHalSetRxAudioFS(HW_FS);
		/*Check if rx_mclk is stable , if no reset audio*/
		if (!fgHalHDMIRxAPLLStatus()) {
			++s_u4APLLUnstableRstCnt;
			if (s_u4APLLUnstableRstCnt >= RX_APLL_UNSTABLE_CNT_REACHED) {
				printk("SetupAudio, RX MCLK is not stable.\n", s_u4APLLUnstableRstCnt);
				s_u4APLLUnstableRstCnt = 0;
				vHalHDMIRxAudResetAudio();
			}
		}
		/*Clean Aud error flag*/
		else {
			s_u4APLLUnstableRstCnt = 0;
			if (u4HDMIRxAudErrorGet() & HDMIRX_INT_STATUS_CHK) {
				/*Reset Audio Fifo if error occurs*/
				vHalHdmiRxAudResetAfifo();
				printk("SetupAudio, RX AFIFO Error , Reset AFIFO.\n");
			} else  if (AudioCaps.SampleFreq == B_Fs_UNKNOW)
				printk("Audio Sampling Rate is Unknow.\n");
			else {
				if (AudioCaps.AudioFlag & B_CAP_HBR_AUDIO) {
					printk("SetupAudio, B_CAP_HBR_AUDIO\n");
					vSetHdmiRxAudioI2SFmt();
					vHalSetRxI2sMclk(MCLK_128FS);   // 128xFs
					vHalHDMIRxSetAudValidCH(0xF); // 4 channels for HBR
					vHalHdmiRxAudBypass(hdmirxbypassmode, FALSE);
					if (hdmirxbypassmode) {
						_HdmiRxHBR();
						_HdmiRxHBRSPDIFBYPASS();
					}
				} else if (AudioCaps.AudioFlag & B_CAP_DSD_AUDIO) {
					printk("SetupAudio, B_CAP_DSD_AUDIO\n");
					vSetHdmiRxAudioI2SFmt();
					vHalSetRxI2sMclk(MCLK_512FS);
					vHalHDMIRxAudResetAudio();//mingchih add
					vHalHdmiRxAudBypass(hdmirxbypassmode, TRUE);
					if (hdmirxbypassmode)
						HalHdmiRxDSDBypass();
				} else {
					printk("SetupAudio, non-B_CAP_HBR_AUDIO and non-B_CAP_DSD_AUDIO\n");
					vSetHdmiRxAudioI2SFmt();
					vHalSetRxI2sMclk(MCLK_128FS);	// 128xFs
					vHalHDMIRxSetAudValidCH(AudioCaps.AudSrcEnable); // 4 channels for HBR
					vHalHdmiRxAudBypass(hdmirxbypassmode, FALSE);
					if (hdmirxbypassmode)
						HalHdmiRxI2SBypass();
				}
				SwitchAudioState(ASTATE_WaitForReady);
			}
		}
	} else {
		s_u4APLLUnstableRstCnt = 0;
		printk("Audio Off, clear Audio Error Count.\n");
	}
}


void hdmirx_audio_task(void)
{
	AUDIO_CAPS CurAudioCaps ;
	switch (AState) {
	case ASTATE_RequestAudio:
		SetupAudio();
		break;
	case ASTATE_WaitForReady:
		if (AudioCountingTimer == 0) {
			if (fgHalHDMIRxAPLLStatus() && ((u4HDMIRxAudErrorGet()&HDMIRX_INT_STATUS_CHK) == 0x00)
			    && (AudioCaps.SampleFreq != B_Fs_UNKNOW)) {
				if (fgHalCheckIsAAC())
					printk("ASTATE_WaitForReady, Audio AutoMute.\n");
				else {
					SwitchAudioState(ASTATE_AudioOn);
					_u1HDMIAFifoErrCnt = 0;
				}
			} else {
				/*make sure the setting is correct according to the audio type*/
				getHDMIRxInputAudio(&AudioCaps) ;
				if (AudioCaps.AudioFlag & B_CAP_HBR_AUDIO) {
					vHalSetHDMIRxHBR(TRUE);
					vSetHdmiRxAudioI2SFmt();
					vHalSetRxI2sMclk(MCLK_128FS);   // 128xFs
					vHalHDMIRxSetAudValidCH(0xF); // 4 channels for HBR
					if (hdmirxbypassmode) {
						_HdmiRxHBR();
						_HdmiRxHBRSPDIFBYPASS();
					}

				} else {
					vHalSetHDMIRxHBR(FALSE);
					vSetHdmiRxAudioI2SFmt();
				}
				vHalSetHDMIRxDSD((AudioCaps.AudioFlag & B_CAP_DSD_AUDIO) ? TRUE : FALSE);
				if ((AudioCaps.AudioFlag & (B_CAP_DSD_AUDIO | B_CAP_HBR_AUDIO)) == 0) {
					//not HBR, and not DSD
					vHalSetRxAudioFS(HW_FS);
					vHalSetRxI2sMclk(MCLK_128FS);	// 128xFs
					vHalHDMIRxSetAudValidCH(AudioCaps.AudSrcEnable);
				}
			}//make sure end
			_u1HDMIAFifoErrCnt++;
			if (_u1HDMIAFifoErrCnt == 50) {
				if (!fgHalHDMIRxAPLLStatus()) {
					printk("ASTATE_WaitForReady, RX MCLK is not stable , Reset Audio.\n");
					vHalHDMIRxAudResetAudio();
				}
				_u1HDMIAFifoErrCnt = 0;
			} else {
				if (u4HDMIRxAudErrorGet()&HDMIRX_INT_STATUS_CHK) {
					// Reset Audio Fifo if error occurs
					if (_u1HDMIAFifoErrCnt % 2 == 0) {
						printk("ASTATE_WaitForReady, RX AFIFO Error , Reset AFIFO.\n");
						vHalHdmiRxAudResetAfifo();
					}
				}
			}
		} else
			AudioCountingTimer --;
		break;
	case ASTATE_AudioOn: {
		BOOL fgError = FALSE;
		getHDMIRxInputAudio(&CurAudioCaps) ;
		if (AudioCaps.AudioFlag != CurAudioCaps.AudioFlag) {
			printk("ASTATE_AudioOn : AudioFlag 0x%x(old) != 0x%x(new).\n", AudioCaps.AudioFlag , CurAudioCaps.AudioFlag);
			AudioCaps.AudioFlag = CurAudioCaps.AudioFlag;
			fgError = TRUE;
		}
		if (AudioCaps.AudSrcEnable != CurAudioCaps.AudSrcEnable) {
			printk("ASTATE_AudioOn : AudSrcEnable 0x%x(old) != 0x%x(new).\n", AudioCaps.AudSrcEnable , CurAudioCaps.AudSrcEnable);
			AudioCaps.AudSrcEnable = CurAudioCaps.AudSrcEnable;
			fgError = TRUE;
		}
		if (AudioCaps.SampleFreq != CurAudioCaps.SampleFreq) {
			printk("ASTATE_AudioOn : SampleFreq 0x%x(old) != 0x%x(new).\n", AudioCaps.SampleFreq , CurAudioCaps.SampleFreq);
			AudioCaps.SampleFreq  = CurAudioCaps.SampleFreq;
			fgError = TRUE;
		}
		if (CurAudioCaps.SampleFreq == B_Fs_UNKNOW) {
			printk("ASTATE_AudioOn : Sample frequency not indicated.\n");
			CurAudioCaps.SampleFreq = B_Fs_UNKNOW;
			fgError = TRUE;
		}
		if (fgError) {
			printk("Hdmi Rx  ResetAudio \n");
			vHalHDMIRxAudResetAudio();
			SwitchAudioState(ASTATE_AudioOff);
			SwitchAudioState(ASTATE_RequestAudio);
		} else {
			BOOL fgChange = FALSE;
			// Check afifo error or not
			//rx_mclk stable and already unmute, check afifo
			if (u4HDMIRxAudErrorGet()&HDMIRX_INT_STATUS_CHK) {
				// Reset Audio Fifo if error occurs
				if (_u1HDMIAFifoErrCnt == 0) {
					printk("ASTATE_AudioOn : Afifo error.\n");;
					SetAudioMute(ON);
					vHalHdmiRxAudResetAfifo();
				}
				_u1HDMIAFifoErrCnt++;
				if (_u1HDMIAFifoErrCnt > 5) {
					vAudioPacketOff(TRUE);
					vHalHDMIRxAudResetAudio();
					printk("ASTATE_AudioOn : AFIFO Error Hdmi Rx  ResetAudio \n");
					SwitchAudioState(ASTATE_AudioOff);
					SwitchAudioState(ASTATE_RequestAudio);
				}
			} else {
				if (_u1HDMIAFifoErrCnt != 0) {
					SetAudioMute(OFF);
					_u1HDMIAFifoErrCnt = 0;
				}
				if (s_fgNewHBRFound) {
					vHalHDMIRxAudResetMCLK();
					s_fgNewHBRFound = FALSE;
				}
			}
			if ((AudioCaps.AudInf.info.SpeakerPlacement != CurAudioCaps.AudInf.info.SpeakerPlacement) ||
			    (AudioCaps.AudInf.info.AudioCodingType != CurAudioCaps.AudInf.info.AudioCodingType) ||
			    (AudioCaps.AudInf.info.SampleFreq != CurAudioCaps.AudInf.info.SampleFreq)
			   ) {
				printk("ASTATE_AudioOn Audio : InforFrame Change\n");
				printk("AudioCaps.AudInf.info.SpeakerPlacement =0x%x\n", CurAudioCaps.AudInf.info.SpeakerPlacement);
				printk("AudioCaps.AudInf.info.AudioCodingType =0x%x\n", CurAudioCaps.AudInf.info.AudioCodingType);
				printk("AudioCaps.AudInf.info.SampleFreq =0x%x\n", CurAudioCaps.AudInf.info.SampleFreq);
				AudioCaps.AudInf.info.SpeakerPlacement = CurAudioCaps.AudInf.info.SpeakerPlacement;
				AudioCaps.AudInf.info.AudioCodingType = CurAudioCaps.AudInf.info.AudioCodingType;
				AudioCaps.AudInf.info.SampleFreq = CurAudioCaps.AudInf.info.SampleFreq;
			}
			if (CurAudioCaps.AudChStat.ISLPCM != AudioCaps.AudChStat.ISLPCM) {
				printk("ASTATE_AudioOn : ChStatBit1 0x%x(old) != 0x%x(new).\n", AudioCaps.AudChStat.ISLPCM,
				       CurAudioCaps.AudChStat.ISLPCM);
				AudioCaps.AudChStat.ISLPCM = CurAudioCaps.AudChStat.ISLPCM;
				fgChange = TRUE;
			}
			if ((CurAudioCaps.AudioFlag & B_HBRAUDIO) != (AudioCaps.AudioFlag & B_HBRAUDIO)) {
				printk("ASTATE_AudioOn :(CurAudioCaps.AudioFlag & B_HBRAUDIO) != (AudioCaps.AudioFlag & B_HBRAUDIO)\n");
				AudioCaps.AudioFlag = CurAudioCaps.AudioFlag;
				fgChange = TRUE;
			}
			if ((CurAudioCaps.AudioFlag & B_DSDAUDIO) != (AudioCaps.AudioFlag & B_DSDAUDIO)) {
				printk("[HDMI RX AUD] ASTATE_AudioOn :(CurAudioCaps.AudioFlag & B_DSDAUDIO) != (AudioCaps.AudioFlag & B_DSDAUDIO)\n");
				AudioCaps.AudioFlag = CurAudioCaps.AudioFlag;
				fgChange = TRUE;
			}
			if (fgChange) {
				getHDMIRxInputAudio(&AudioCaps);
			}
			if (AudioCountingTimer != 0)
				AudioCountingTimer -- ;
		}
	}
	break;
	default:
		break;
	}
}

