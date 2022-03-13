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
#if defined(CONFIG_MTK_HDMI_SUPPORT)
#include <linux/string.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#include <linux/debugfs.h>

#include <mach/mt_typedefs.h>


#if defined(CONFIG_MTK_INTERNAL_HDMI_SUPPORT)
#include "internal_hdmi_drv.h"
#elif defined(MTK_INTERNAL_MHL_SUPPORT)
#include "inter_mhl_drv.h"
#else
#include "hdmi_drv.h"
#endif
//#include "hdmitx.h"
//#include "hdmitx_drv.h"

#if defined(MTK_INTERNAL_MHL_SUPPORT)
#include "mhl_dbg.h"
#endif

#include "hdmihdcp.h"
#include "hdmicec.h"

void DBG_Init(void);
void DBG_Deinit(void);

extern void hdmi_log_enable(int enable);
extern void hdmi_cable_fake_plug_in(void);
extern void hdmi_cable_fake_plug_out(void);
extern void hdmi_mmp_enable(int enable);
extern void hdmi_pattern(int enable);

extern void hdmi_drvlog_enable(unsigned short enable);
extern void hdmi_video_config_ext(HDMI_VIDEO_RESOLUTION res, HDMI_VIDEO_OUT_MODE mode);
#if CONFIG_MTK_HDMIRX_SUPPORT
extern UINT8   u1HalHDMIRxAudioCHSTAT0(void);
extern UINT8   u1HalHDMIRxAudioCHSTAT1(void);
extern UINT8   u1HalHDMIRxAudioCHSTAT2(void);
extern UINT8   u1HalHDMIRxAudioCHSTAT3(void);
extern UINT8   u1HalHDMIRxAudioCHSTAT4(void);
#endif
extern int hdmi_audiosetting(HDMIDRV_AUDIO_PARA* audio_para);


// ---------------------------------------------------------------------------
//  External variable declarations
// ---------------------------------------------------------------------------

//extern LCM_DRIVER *lcm_drv;
// ---------------------------------------------------------------------------
//  Debug Options
// ---------------------------------------------------------------------------


static char STR_HELP[] =
    "\n"
    "USAGE\n"
    "        echo [ACTION]... > hdmi\n"
    "\n"
    "ACTION\n"
    "        hdmitx:[on|off]\n"
    "             enable hdmi video output\n"
    "\n";

static char debug_buffer[3072];
extern size_t hdmi_cec_on;


extern void hdmi_log_enable(int enable);
extern void hdmi_enablehdcp(unsigned char u1hdcponoff);
extern unsigned int u4ReadNValue(void);
extern unsigned int u4ReadCtsValue(void);
extern unsigned int i4SharedInfo (unsigned int u4Index);
extern void UnMuteHDMIAudio(void);
extern void MuteHDMIAudio(void);
extern void vUnBlackHDMIOnly(void);
extern void vBlackHDMIOnly(void);
extern int hdmi_av_enable(int arg);
extern void CECMWSetLA(CEC_LA_ADDRESS* prLA);

extern HDMI_AV_INFO_T _stAvdAVInfo;
extern unsigned char _bflagvideomute;
extern unsigned char _bflagaudiomute;
extern unsigned char _bsvpvideomute;
extern unsigned char _bsvpaudiomute;
extern unsigned char _bHdcpOff;
extern unsigned char HDMI_AKSV[HDCP_AKSV_COUNT];
extern UINT8 _bHdcp_Bksv[5];

// TODO: this is a temp debug solution
//extern void hdmi_cable_fake_plug_in(void);
//extern int hdmi_drv_init(void);


extern void vShowHpdRsenStatus(void);
extern void vShowOutputVideoResolution(void);
extern void vShowDviOrHdmiMode(void);
extern void vShowDeepColor(void);
extern void vShowColorSpace(void);
void vShowHdmiAudioStatus(void);
extern void vShowInforFrame(void);
extern void vSend_AVMUTE(void);
extern void vSend_AVUNMUTE(void);
extern void vShowBstatus(void);

static void process_dbg_cmd(char *cmd)
{
    char *opcode;
	char * oprand;

	sprintf(debug_buffer + strlen(debug_buffer), "cmd:%s\n", cmd);

	opcode = strsep(&cmd, " ");
	if(opcode == NULL)
		goto error;

	sprintf(debug_buffer + strlen(debug_buffer), "opcode:%s\n", opcode);

	if(0 == strncmp(opcode, "tgcstt", 6)) {
		vShowHpdRsenStatus();
		vShowOutputVideoResolution();
		vShowDviOrHdmiMode();
		vShowDeepColor();
		vShowColorSpace();
		vShowHdmiAudioStatus();
	} else if(0 == strncmp(opcode, "tgedid", 6)) {
		vShowEdidRawData();
		vShowEdidInformation();	
	} else if(0 == strncmp(opcode, "tgifrm", 6)) {
		vShowInforFrame();	
	} else if(0 == strncmp(opcode, "tgbstt", 6)) {	
		vShowBstatus();
	} else if(0 == strncmp(opcode, "tslog", 5)) {
		oprand = strsep(&cmd, " ");
		if(oprand == NULL)
			goto error;
		if(0 == strncmp(oprand, "d", 1))
			hdmi_drvlog_enable(0);	
		else if(0 == strncmp(oprand, "f", 1))
			hdmi_drvlog_enable(0xffff);
		else {
			int enableType;
			if(kstrtoint(oprand, 16, &enableType))
				goto error;
			hdmi_drvlog_enable(enableType);
		}
	} else if(0 == strncmp(opcode, "tshdcp", 6)) {
		oprand = strsep(&cmd, " ");
		if(oprand == NULL)
			goto error;
	
		if(0 == strncmp(oprand, "d", 1))
			vDisableHDCP(1);
		else if(0 == strncmp(oprand, "e", 1))
			vDisableHDCP(0);
	} else if(0 == strncmp(opcode, "tshhr", 5)) {
		oprand = strsep(&cmd, " ");
		if(oprand == NULL)
			goto error;
	
		if(0 == strncmp(oprand, "d", 1)) {
			vDisableHDCP(1);
			vEnable_hotplug_pord_int(0);
		} else if(0 == strncmp(oprand, "e", 1)) {
			vDisableHDCP(0);
			vEnable_hotplug_pord_int(1);
		}
	
	} else if(0 == strncmp(opcode, "tsavm", 5)) {
		oprand = strsep(&cmd, " ");
		if(oprand == NULL)
			goto error;
	
		if(0 == strncmp(oprand, "s", 1)) {
			vSend_AVMUTE();
		} else if(0 == strncmp(oprand, "c", 1)) {
			vSend_AVUNMUTE();
		}	
	} 

	if (0 == strncmp(opcode, "hdcpkeytype", 11))
	{
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		if(0 == strncmp(oprand, "1", 1))
		{	
			sprintf(debug_buffer + strlen(debug_buffer), "hdcp key type: EXTERNAL_KEY\n");
			vMoveHDCPInternalKey(EXTERNAL_KEY);
		}else if (0 == strncmp(oprand, "2", 1))
		{
			sprintf(debug_buffer + strlen(debug_buffer), "hdcp key type: INTERNAL_NOENCRYPT_KEY\n");
			vMoveHDCPInternalKey(INTERNAL_NOENCRYPT_KEY);
		}else if(0 == strncmp(oprand, "3", 1))
		{
			sprintf(debug_buffer + strlen(debug_buffer), "hdcp key type: INTERNAL_ENCRYPT_KEY\n");
			vMoveHDCPInternalKey(INTERNAL_ENCRYPT_KEY);
		}
	}
	else if (0 == strncmp(opcode, "enablehdcp", 10))
	{
		int enableType = 0;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		if(kstrtoint(oprand, 10, &enableType))
			goto error;
		hdmi_enablehdcp(enableType);
		sprintf(debug_buffer + strlen(debug_buffer), "enable hdcp %d\n", enableType);
	}
	else if (0 == strncmp(opcode, "aksv", 4))
	{
		sprintf(debug_buffer + strlen(debug_buffer), "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", HDMI_AKSV[0],HDMI_AKSV[1],HDMI_AKSV[2],HDMI_AKSV[3],HDMI_AKSV[4]);
	}
	else if (0 == strncmp(opcode, "res", 3))
	{
		int res = 0;
		int mode = 0;
		char * arg = NULL;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&res))
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&mode))
			goto error;

		hdmi_video_config_ext(res, mode);

		sprintf(debug_buffer + strlen(debug_buffer), "set resolution, res = %d, mode = %d\n", res, mode);
	}
	else if (0 == strncmp(opcode, "cecsetla", 8)) {
		CEC_LA_ADDRESS la;
		u8 ula;

		oprand = strsep(&cmd, " ");
		if(oprand == NULL)
			goto error;

		if(kstrtou8(oprand, 10, &ula))
			goto error;

		la.ui1_num = 1;
		la.aui1_la[0] = ula;

		sprintf(debug_buffer + strlen(debug_buffer), "la = %c\n", ula);

		CECMWSetLA(&la);
	} else if (0 == strncmp(opcode, "cecsend", 7)) {
		CEC_SEND_MSG_T msg;
		char * arg;
		int i = 0;
		
		oprand = strsep(&cmd, " ");
		if(oprand == NULL)
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&(msg.t_frame_info.ui1_init_addr)))
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&(msg.t_frame_info.ui1_dest_addr)))
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&(msg.t_frame_info.ui2_opcode)))
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&(msg.t_frame_info.z_operand_size)))
			goto error;

		if(msg.t_frame_info.z_operand_size > CEC_MAX_OPERAND_SIZE)
			goto error;

		for(i = 0; i< msg.t_frame_info.z_operand_size; i++){
			arg = strsep(&oprand, ",");
			if(arg == NULL)
				break;

			if(kstrtoint(arg, 10, (int *)&(msg.t_frame_info.aui1_operand[i])))
				goto error;
		}
		msg.pv_tag = NULL;
		msg.b_enqueue_ok = false;

		sprintf(debug_buffer + strlen(debug_buffer), "cec msg info, init addr:%d, dest addr:%d, opcode:%d, oprand size:%d\n", 
			msg.t_frame_info.ui1_init_addr,
			msg.t_frame_info.ui1_dest_addr,
			msg.t_frame_info.ui2_opcode,
			msg.t_frame_info.z_operand_size);

		hdmi_CECMWSend(&msg);
	}
	else if (0 == strncmp(opcode, "cecgetaddr", 10))
	{
		CEC_ADDRESS cec_addr;
		hdmi_NotifyApiCECAddress(&cec_addr);
		sprintf(debug_buffer + strlen(debug_buffer), "pa:%d, la:%d\n", cec_addr.ui2_pa, cec_addr.ui1_la);
	}
	else if (0 == strncmp(opcode, "cecenable", 9))
	{
		int enableType;
		oprand = strsep(&cmd, " ");
		if(oprand == NULL)
			goto error;

		if(kstrtoint(oprand, 10, &enableType))
			goto error;

		hdmi_CECMWSetEnableCEC(enableType);
		sprintf(debug_buffer + strlen(debug_buffer),"enable cec , enable type:%d\n", enableType);
	}
	else if (0 == strncmp(opcode, "cecstatus", 9))
	{
		if(hdmi_cec_on == 1)
			sprintf(debug_buffer + strlen(debug_buffer),"cec on\n");
		else
			sprintf(debug_buffer + strlen(debug_buffer),"cec off\n");
	}
	else if (0 == strncmp(opcode, "enablelog", 9))
	{
		int enableType;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		if(kstrtoint(oprand, 16, &enableType))
			goto error;
		
		hdmi_drvlog_enable(enableType);
		sprintf(debug_buffer + strlen(debug_buffer),"enable log , enable type:%d\n", enableType);
	}
	else if (0 == strncmp(opcode, "hdmitxlog", 9))
	{
		int enableType;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		if(kstrtoint(oprand, 16, &enableType))
			goto error;
		
		hdmi_log_enable(enableType);
		sprintf(debug_buffer + strlen(debug_buffer),"hdmitx log:%d\n", enableType);
	}
	else if (0 == strncmp(opcode, "mmp", 3))
	{
		int enableType;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		if(kstrtoint(oprand, 16, &enableType))
			goto error;
		
		hdmi_mmp_enable(enableType);
		sprintf(debug_buffer + strlen(debug_buffer),"hdmitx mmp:%d\n", enableType);
	}
	else if (0 == strncmp(opcode, "status", 6))
	{
		sprintf(debug_buffer + strlen(debug_buffer),"e_resolution : %d\n", _stAvdAVInfo.e_resolution);
		sprintf(debug_buffer + strlen(debug_buffer),"fgHdmiOutEnable : %d\n", _stAvdAVInfo.fgHdmiOutEnable);
		sprintf(debug_buffer + strlen(debug_buffer),"u2VerFreq : %d\n", _stAvdAVInfo.u2VerFreq);
		sprintf(debug_buffer + strlen(debug_buffer),"b_hotplug_state : %d\n", _stAvdAVInfo.b_hotplug_state);
		sprintf(debug_buffer + strlen(debug_buffer),"e_video_color_space : %d\n", _stAvdAVInfo.e_video_color_space);
		sprintf(debug_buffer + strlen(debug_buffer),"e_deep_color_bit : %d\n", _stAvdAVInfo.e_deep_color_bit);
		sprintf(debug_buffer + strlen(debug_buffer),"ui1_aud_out_ch_number : %d\n", _stAvdAVInfo.ui1_aud_out_ch_number);
		sprintf(debug_buffer + strlen(debug_buffer),"e_hdmi_fs : %d\n", _stAvdAVInfo.e_hdmi_fs);
		sprintf(debug_buffer + strlen(debug_buffer),"bMuteHdmiAudio : %d\n", _stAvdAVInfo.bMuteHdmiAudio);
		sprintf(debug_buffer + strlen(debug_buffer),"u1HdmiI2sMclk : %d\n", _stAvdAVInfo.u1HdmiI2sMclk);
		sprintf(debug_buffer + strlen(debug_buffer),"u1hdcponoff : %d\n", _stAvdAVInfo.u1hdcponoff);
		sprintf(debug_buffer + strlen(debug_buffer),"u1audiosoft : %d\n", _stAvdAVInfo.u1audiosoft);
		sprintf(debug_buffer + strlen(debug_buffer),"fgHdmiTmdsEnable : %d\n", _stAvdAVInfo.fgHdmiTmdsEnable);
		sprintf(debug_buffer + strlen(debug_buffer),"out_mode : %d\n", _stAvdAVInfo.out_mode);
		sprintf(debug_buffer + strlen(debug_buffer),"e_hdmi_aud_in : %d\n", _stAvdAVInfo.e_hdmi_aud_in);
		sprintf(debug_buffer + strlen(debug_buffer),"e_iec_frame : %d\n", _stAvdAVInfo.e_iec_frame);
		sprintf(debug_buffer + strlen(debug_buffer),"e_aud_code : %d\n", _stAvdAVInfo.e_aud_code);
		sprintf(debug_buffer + strlen(debug_buffer),"u1Aud_Input_Chan_Cnt : %d\n", _stAvdAVInfo.u1Aud_Input_Chan_Cnt);
		sprintf(debug_buffer + strlen(debug_buffer),"e_I2sFmt : %d\n", _stAvdAVInfo.e_I2sFmt);
		sprintf(debug_buffer + strlen(debug_buffer),"ACR N= %d, CTS = %d \n", u4ReadNValue(),u4ReadCtsValue());
		sprintf(debug_buffer + strlen(debug_buffer),"_bflagvideomute =%d \n", _bflagvideomute);
		sprintf(debug_buffer + strlen(debug_buffer),"_bflagaudiomute =%d \n", _bflagaudiomute);
		sprintf(debug_buffer + strlen(debug_buffer),"_bsvpvideomute =%d \n", _bsvpvideomute);
		sprintf(debug_buffer + strlen(debug_buffer),"_bsvpaudiomute =%d \n", _bsvpaudiomute);
		sprintf(debug_buffer + strlen(debug_buffer),"_bHdcpOff =%d \n", _bHdcpOff);
		sprintf(debug_buffer + strlen(debug_buffer),"Bksv:0x%x 0x%x 0x%x 0x%x 0x%x\n", _bHdcp_Bksv[0], _bHdcp_Bksv[1],_bHdcp_Bksv[2],_bHdcp_Bksv[3],_bHdcp_Bksv[4]);
		sprintf(debug_buffer + strlen(debug_buffer),"i4SharedInfo(SI_EDID_VSDB_EXIST) = %d \n", i4SharedInfo(SI_EDID_VSDB_EXIST));
		
	}
	else if(0 == strncmp(opcode, "mute", 9))
	{
		char * arg;
		int mute_type; //0:video;1 audio
		int mute_en;// 1: mute; 0: unmute
		
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, &mute_type))
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, &mute_en))
			goto error;

		if(mute_type == 0 && mute_en == 1)
		{
			vBlackHDMIOnly();
			sprintf(debug_buffer + strlen(debug_buffer),"mute video\n");
		}
		else if(mute_type == 0 && mute_en == 0)
		{
			vUnBlackHDMIOnly();
			sprintf(debug_buffer + strlen(debug_buffer),"unmute video\n");
		}
		else if(mute_type == 1 && mute_en == 1)
		{
			MuteHDMIAudio();
			sprintf(debug_buffer + strlen(debug_buffer),"mute audio\n");
		}
		else if(mute_type == 1 && mute_en == 0)
		{
			UnMuteHDMIAudio();
			sprintf(debug_buffer + strlen(debug_buffer),"unmute audio\n");
		}
	}
	else if(0 == strncmp(opcode, "avenable", 9))
	{
		char * arg;
		int enable;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, &enable))
			goto error;

		sprintf(debug_buffer + strlen(debug_buffer),"av enable , arg = %d\n", enable);
		hdmi_av_enable(enable);
	}
	else if(0 == strncmp(opcode, "aud", 3))
	{
		char * arg;
		HDMIDRV_AUDIO_PARA audio_setting;
		audio_setting.e_hdmi_aud_in = SV_I2S;
		audio_setting.e_iec_frame = IEC_48K;	 
		audio_setting.e_hdmi_fs = HDMI_FS_48K;
		audio_setting.e_aud_code = AVD_LPCM;
		audio_setting.u1Aud_Input_Chan_Cnt = AUD_INPUT_2_0;
		audio_setting.e_I2sFmt = HDMI_I2S_24BIT;
		audio_setting.u1HdmiI2sMclk = MCLK_128FS;
#if CONFIG_MTK_HDMIRX_SUPPORT
		audio_setting.bhdmi_LCh_status[0] = u1HalHDMIRxAudioCHSTAT0();;
		audio_setting.bhdmi_LCh_status[1] = u1HalHDMIRxAudioCHSTAT1();
		audio_setting.bhdmi_LCh_status[2] = u1HalHDMIRxAudioCHSTAT2();
		audio_setting.bhdmi_LCh_status[3] = u1HalHDMIRxAudioCHSTAT3();
		audio_setting.bhdmi_LCh_status[4] = u1HalHDMIRxAudioCHSTAT4();
		audio_setting.bhdmi_RCh_status[0] = u1HalHDMIRxAudioCHSTAT0();
		audio_setting.bhdmi_RCh_status[1] = u1HalHDMIRxAudioCHSTAT1();
		audio_setting.bhdmi_RCh_status[2] = u1HalHDMIRxAudioCHSTAT2();
		audio_setting.bhdmi_RCh_status[3] = u1HalHDMIRxAudioCHSTAT3();
		audio_setting.bhdmi_RCh_status[4] = u1HalHDMIRxAudioCHSTAT4();
#endif
		hdmi_audiosetting(&audio_setting);
	}
	return ;

	error:
		sprintf(debug_buffer + strlen(debug_buffer), "parse command error!\n");
}


// ---------------------------------------------------------------------------
//  Debug FileSystem Routines
// ---------------------------------------------------------------------------

struct dentry *hdmitx_dbgfs = NULL;


static ssize_t debug_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}

static ssize_t debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
	if(strlen(debug_buffer))
		return simple_read_from_buffer(ubuf, count, ppos, debug_buffer, strlen(debug_buffer));

    return simple_read_from_buffer(ubuf, count, ppos, STR_HELP, strlen(STR_HELP));
}



static ssize_t debug_write(struct file *file,
                           const char __user *ubuf, size_t count, loff_t *ppos)
{
	char cmd_buffer[128];
    const int debug_bufmax = sizeof(cmd_buffer) - 1;
    size_t ret;

	memset(cmd_buffer, 0, sizeof(cmd_buffer));
    ret = count;

    if (count > debug_bufmax)
    {
        count = debug_bufmax;
    }

    if (copy_from_user(&cmd_buffer, ubuf, count))
    {
        return -EFAULT;
    }

    cmd_buffer[count] = 0;
	memset(debug_buffer, 0, sizeof(debug_buffer));
    process_dbg_cmd(cmd_buffer);

    return ret;
}


static struct file_operations debug_fops =
{
    .read  = debug_read,
    .write = debug_write,
    .open  = debug_open,
};


void HDMI_DBG_Init(void)
{
    hdmitx_dbgfs = debugfs_create_file("hdmi",
                                       S_IFREG | S_IRUGO, NULL, (void *)0, &debug_fops);
}


void HDMI_DBG_Deinit(void)
{
    debugfs_remove(hdmitx_dbgfs);
}

#endif
