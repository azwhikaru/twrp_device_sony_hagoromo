#ifndef _TVE_CTRL_H_
#define _TVE_CTRL_H_

#include "tve_dev.h"

#define CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT 1

/* Maximum number of plane mixer */
#define TVE_1 1 						// Main display
#define TVE_2 0 						// Aux display
#define TVE_MAX 1
#define TVE_MAX_NS TVE_MAX 

//TVE config
#define VDAC_LOW_IMPEDANCE  0
#define VDAC_HIGH_IMPEDANCE  1

/* TVE mode */
#define TVE_MOD_OFF	    		0
#define TVE_MOD_COMPOSITE		1
#define TVE_MOD_S_VIDEO	    2
#define TVE_MAX_MOD			    3


/* TVE configuration return value */
typedef enum 
{
  TVE_SET_ERROR = 0,
  TVE_SET_OK,
} TVE_STATUS_T;

typedef  enum
{
    RES_480I=0,
    RES_576I, //1
    RES_480P, //2
    RES_576P
}   RESOLUTION_MODE_T;

/* TVE WSS/CGMS Aspect Ratio
   NOR: Normal, LB: LetterBox, C: Center, T: Top, LG: Larger than
   525-Line (CGMS) */
#define CGMS_NOR_4_3			0
#define CGMS_NOR_16_9		1
#define CGMS_LB_4_3    2
#define CGMS_LB_16_9			3

/* 625-Line (WSS)  */
#define WSS_LB_14_9_C		1
#define WSS_LB_14_9_T		2
#define WSS_LB_16_9_T		4
#define WSS_ANA_16_9			7
#define WSS_FULL_4_3			8
#define WSS_LB_16_9_C			  11
#define WSS_LB_LG_16_9_C		13
#define WSS_FULL_14_9_C			14

//480i
#define TVE_480I_CGMS_COPY_PERMITTED         (UINT32)(0)
#define TVE_480I_CGMS_RESERVED               (UINT32)(1)
#define TVE_480I_CGMS_ONE_COPY_ALLOWED       (UINT32)(2)
#define TVE_480I_CGMS_NO_COPY_PERMITTED      (UINT32)(3)

//480p
#define TVE_480P_CGMS_COPY_PERMITTED         (UINT32)(0)
#define TVE_480P_CGMS_NO_MORE_COPY           (UINT32)(1)
#define TVE_480P_CGMS_ONE_COPY_ALLOWED       (UINT32)(2)
#define TVE_480P_CGMS_NO_COPY_PERMITTED      (UINT32)(3)

//576i/p
#define TVE_576_CGMS_COPY_PERMITTED          (UINT32)(0)
#define TVE_576_CGMS_RESERVED                (UINT32)(1)
#define TVE_576_CGMS_ONE_COPY_ALLOWED        (UINT32)(2)
#define TVE_576_CGMS_NO_COPY_PERMITTED       (UINT32)(3)

/** Attributes for the cps common info inband command.
 */
typedef struct
{
    UINT32 u4InfoValid;
    UINT8 u1OriginalCgms;
    UINT8 u1Cgms;
    UINT8 u1Aps;
    UINT8 u1AnalogSrc;
    UINT8 u1ICT;
    UINT8 u1DOT;
    UINT8 u1CSS;
    UINT8 u1AACS;
    UINT8 u1EPN;
    UINT8 u1NotPassCnt; // must not pass content to a specific output device for wmdrm output control
    UINT8 u1DCICCI;
} IBC_CpsCommonInfoParamsDef;

typedef  enum
{
  ASPECT_RATIO_4_3 = 0,
  ASPECT_RATIO_14_9_LB_CENTRE,
  ASPECT_RATIO_14_9_LB_TOP,
  ASPECT_RATIO_16_9_LB_CENTRE,
  ASPECT_RATIO_16_9_LB_TOP, 
  ASPECT_RATIO_LARGE_16_9_LB_CENTRE,   
  ASPECT_RATIO_14_9_FULL_FORMAT,  
  ASPECT_RATIO_16_9_FULL_FORMAT,
  ASPECT_RATIO_MAX  
} TVE_ASPECT_RATIO_T;

typedef  enum
{
  MEDIA_TYPE_INVALID = 0, 
  MEDIA_TYPE_BD, 
  MEDIA_TYPE_BDRE, 
  MEDIA_TYPE_DVDCSS, 
  MEDIA_TYPE_DVDCPRM, 
  MEDIA_TYPE_VOD,// MEDIA_TYPE_BIVL
  MEDIA_TYPE_DME,
  MEDIA_TYPE_NUM,
} TVE_MEDIA_TYPE_T;


#define  fgVideoIsNtsc(ucFmt)  ((ucFmt == RES_480I) || (ucFmt == RES_480P))
	
typedef struct _TVE_CONF_T
{
  UCHAR  ucTveEnable;
  UCHAR  ucMode;
  UCHAR  ucMainFmt;  
  UCHAR  ucFmt;
  UCHAR  ucTargetFmt;  
  UCHAR  ucVFreq;  
  UCHAR  ucImpedence;
  UCHAR  ucAspectRatio;
  UCHAR  ucCgmsaInfo;
  UCHAR  ucDACEnable;
  UCHAR  ucAps;  
  UCHAR  ucAnalogSrc;  
  UCHAR  ucVDacConfig;
  UCHAR  ucMediaType;
  BOOL    fgRCIRCD;  
  BOOL    fgBlackScn;  
  BOOL    fgEpn;  
  BOOL    fgNotPassCnt;  
  BOOL    fgDCICCI;  
  BOOL    fgICT;
  BOOL    fgDOT;
  BOOL    fgNtsc;
  BOOL    fgSDMode;
  BOOL    fgSetupEnable;
  BOOL    fgCavDownSample;
  BOOL    fgCSSDisc;
  BOOL    fgCSSTitle;
  BOOL    fgAACS;
  BOOL    fgSync714286;
  BOOL    fgOutputSD;
  UINT32  ui4AR1AR0;
  BOOL    fgIgoneCPS;
  BOOL    fgHDExist;
  UINT8   fgOSD5Enable;
  UCHAR	  ucVPSEnable;
} TVE_CONF_T;

typedef struct _TVE_VIDEO_ADJUST_T
{
  int i2BrightnessLevel;
  int i2ContrastLevel;
  int i2SaturationLevel;
  int i2HueLevel;
  int i2SharpnessLevel;
}  TVE_VIDEO_ADJUST_T;
#if 1//CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
typedef struct _TVE_STATUS_CONF_T
{
  UCHAR  ucTveEnable;
  UCHAR  ucMainFmt;  
  UCHAR  ucTargetFmt;  
  UCHAR  ucVFreq;  
  UCHAR  ucAspectRatio;
  UCHAR  ucCgmsaInfo;
  UCHAR  ucAps;  
  UCHAR  ucAnalogSrc;  
  UCHAR  ucMediaType;
  BOOL    fgEpn;  
  BOOL    fgNotPassCnt;  
  BOOL    fgDCICCI;  
  BOOL    fgICT;//analog output in SD
  BOOL    fgDOT;//analog output allow  
  BOOL    fgCSSTitle;
  BOOL    fgAACS;
  UCHAR  ucTveDacStatus;

}TVE_STATUS_CONF_T;
#endif

/* for IBC_CpsCommonInfoParamsDef.u4InfoValid */
#define IBC_CPS_INFO_CGMS_VALID       (UINT32)(1)
#define IBC_CPS_INFO_APS_VALID        (UINT32)(1 << 1)
#define IBC_CPS_INFO_ANALOG_SRC_VALID (UINT32)(1 << 2)
#define IBC_CPS_INFO_ICT_VALID        (UINT32)(1 << 3)
#define IBC_CPS_INFO_DOT_VALID        (UINT32)(1 << 4)
#define IBC_CPS_INFO_CSS_VALID        (UINT32)(1 << 5)
#define IBC_CPS_INFO_AACS_VALID       (UINT32)(1 << 6)
#define IBC_CPS_INFO_EPN_VALID        (UINT32)(1 << 7)
#define IBC_CPS_INFO_NPCNT_VALID        (UINT32)(1 << 8) //NotPassCnt
#define IBC_CPS_INFO_DCICCI_VALID        (UINT32)(1 << 9)
#define IBC_CPS_INFO_BIVL_DRM_VALID		(UINT32)(1 << 15)

/* for IBC Cps info CGMS type definition */
#define IBC_CPS_INFO_CGMS_COPY_PERMITTED      (UINT32)(0)
#define IBC_CPS_INFO_CGMS_NO_MORE_COPY        (UINT32)(1)
#define IBC_CPS_INFO_CGMS_ONE_COPY_ALLOWED    (UINT32)(2)
#define IBC_CPS_INFO_CGMS_NO_COPY_PERMITTED   (UINT32)(3)
#define IBC_CPS_INFO_CGMS_RESERVED            (UINT32)(4)


/* for IBC Cps info CSS flag definition */
#define IBC_CPS_INFO_CSS_EXIST        (UINT32)(1)
#define IBC_CPS_INFO_CSS_DISC         (UINT32)(1 << 1)
#define IBC_CPS_INFO_CSS_TITLE        (UINT32)(1 << 2)


/* must not pass content to a specific output device for wmdrm output control */
#define IBC_CPS_INFO_NOT_PASS_TO_DIGITAL_VID  (UINT32)(1)       /* must not pass content to digital video outputs */
#define IBC_CPS_INFO_NOT_PASS_TO_ANALOG_TV    (UINT32)(1 << 1)  /* must not pass content to analog television outputs */


#define TVE_LOW_IMP_TRIM					0x02
#define VDAC_HIGH_IMPEDANCE_TRIM	0x00

#define TVE_CVBS_NTSC_YScale			0x80//0x8c//0x7C
#define TVE_CVBS_PAL_YScale				0x80//0x8c//0x7C

#define TVE_CVBS_NTSC_UScale     	0x44
#define TVE_CVBS_NTSC_VScale     	0x60

#define TVE_CVBS_PAL_UScale      	0x68//0x45//0x44
#define TVE_CVBS_PAL_VScale      	0x68//0x52//0x60

#define MV_AGCLVL_NTSC_CVBS			(0x98 << 16)
#define MV_BPLVL_NTSC_CVBS			(0x85 << 8)

#define MV_AGCLVL_PAL						(0x9b << 16)
#define MV_BPLVL_PAL						(0x8a << 8)

#define MV_AGCLVL_NTSC		   		(0x98 << 16)
#define MV_BPLVL_NTSC						(0x85 << 8)

#define PAL_BURST 							0x3B
#define NTSC_BURST							0x38

#define NTSC_SCH								0x33
#define PAL_SCH 								0x2D

extern TVE_STATUS_T TVE_SetEnable(UCHAR ucEnable);
extern TVE_STATUS_T TVE_Reset(void);
extern TVE_STATUS_T TVE_DACPower(UCHAR ucEnable);
extern TVE_STATUS_T TVE_SetBrightness(UCHAR ucLevel);
extern TVE_STATUS_T TVE_SetContrast(UCHAR ucLevel);
extern TVE_STATUS_T TVE_SetHue(UCHAR ucLevel);
extern TVE_STATUS_T TVE_SetSaturation(UCHAR ucLevel);
extern void TVE_SetColorBar(UCHAR ucOn);
extern void TVE_HalColorBar(UCHAR ucOn);
extern void TVE_SetFmt(UCHAR ucFmt, UCHAR ucTargetFmt, UCHAR ucMode, UCHAR ucVDacConfig, UCHAR ucEnable);
extern void TVE_Init(void);
extern void TVE_SelSelfClkSrc(void);
extern UINT32 TVE_SetMacroVision(UCHAR ucAps);
extern void TVE_SetSyncTime1(UCHAR ucOn,UCHAR slf_run,UINT16 vsync, UINT16 hsync);
extern UINT32 TVE_SetMacroVision(UCHAR ucAps);
extern void  DPI_MIPI_clk_setting(unsigned int mipi_pll_clk_ref,unsigned int mipi_pll_clk_div1,unsigned int mipi_pll_clk_div2);
extern void TVE_SelClkFromDpiSrc(BOOL fgDpi0);
extern void vDPI0_576p(void);
extern void vDPI0_480p(void);
extern void TveSetFmt(int fmt);
extern void TveGetFmt(char* str);
extern UINT32 TVE_SetAspectRatio(TVE_ASPECT_RATIO_T ucAspectRatio);
extern UINT32 TVE_SetCps(IBC_CpsCommonInfoParamsDef *prCpsInfo );
extern void printf_cps(IBC_CpsCommonInfoParamsDef* prCpsInfo);
extern void TveSetDPI0Colorbar(unsigned short flag);
extern void TveEnable(unsigned short flag);
extern void TVE_Suspend(void);

void TVE_PowerOn();
void TVE_PowerOff();

extern void TVE_Resume(void);
extern void Tve_LogEnable(unsigned short enable);
#if CONFIG_ANALOG_VIDEO_CABLE_AUTODETECT_SUPPORT
extern void TVE_GetTveStatus(TVE_STATUS_CONF_T *TveStatus);
#endif
#endif
