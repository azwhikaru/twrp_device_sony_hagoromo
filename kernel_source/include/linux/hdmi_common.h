#ifndef _HDMI_COMMON_H
#define _HDMI_COMMON_H

typedef enum {
	RES_480I = 0,
	RES_576I, //1
	RES_480P, //2
	RES_576P, //3
	RES_480P_1440,//4
	RES_576P_1440,//5
	RES_480P_2880,//6
	RES_576P_2880,//7
	RES_720P60HZ,//8
	RES_720P50HZ,//9
	RES_1080I60HZ,//10
	RES_1080I50HZ,//11
	RES_1080P60HZ,//12
	RES_1080P50HZ,//13
	RES_1080P30HZ,//14
	RES_1080P25HZ, //15
	RES_480I_2880,//16
	RES_576I_2880,//17
	RES_1080P24HZ, //18
	RES_1080P23_976HZ, //19, 1080P23.976hz
	RES_1080P29_97HZ, //20, 1080P29.97hz
	RES_3D_1080P23HZ, //21, 1080p47.952Hz
	RES_3D_1080P24HZ, //22, 1080p48hz
	RES_3D_720P60HZ, //23, 720p120hz
	RES_3D_720P50HZ, //24, 720p100hz
	RES_3D_720P30HZ, //25, 720p120hz
	RES_3D_720P25HZ, //26, 720p100hz
	RES_3D_576P50HZ, //27, 576p100hz
	RES_3D_480P60HZ, //28, 480p120hz
	RES_3D_1080I60HZ, //29, 1080i120hz
	RES_3D_1080I50HZ, //30, 1080i100hz
	RES_3D_1080I30HZ, //31, 1080i120hz
	RES_3D_1080I25HZ, //32, 1080i100hz
	RES_3D_576I25HZ, //33, 576i100hz
	RES_3D_480I30HZ, //34, 480i120hz
	RES_3D_576I50HZ, //35, 576i100hz
	RES_3D_480I60HZ, //36, 480i120hz
	RES_2D_480I60HZ, //37, 480i60hz
	RES_2D_576I50HZ, //38, 576i50hz
	RES_2D_640x480HZ,//39
	RES_PANEL_AUO_B089AW01, //40 //Total: 1344x625, Act: 1024x600, Frm: 60Hz, Clk: 50.4MHz
	RES_3D_720P60HZ_TB,  //41
	RES_3D_720P50HZ_TB,  //42
	RES_3D_1080I60HZ_SBS_HALF,//43
	RES_3D_1080I50HZ_SBS_HALF,//44
	RES_3D_1080P23HZ_TB, //45
	RES_3D_1080P24HZ_TB,//46

	RES_2160P_23_976HZ, //47
	RES_2160P_24HZ, //48
	RES_2160P_25HZ, //49
	RES_2160P_29_97HZ, //50
	RES_2160P_30HZ,  //51
	RES_2161P_24HZ,  //52


	RES_720P30HZ, // 53
	RES_720P25HZ, // 54
	RES_720P24HZ, // 55
	RES_720P23HZ, // 56
	//3D frame packet
	RES_3D_1080P60HZ,// 57
	RES_3D_1080P50HZ,//58
	RES_3D_1080P30HZ,//59
	RES_3D_1080P29HZ,//60
	RES_3D_1080P25HZ,//61
	RES_3D_720P24HZ, //62
	RES_3D_720P23HZ, //63
	//3D Top and bottom
	RES_3D_1080P60HZ_TB, //64
	RES_3D_1080P50HZ_TB, //65
	RES_3D_1080P30HZ_TB, //66
	RES_3D_1080P29HZ_TB,//67
	RES_3D_1080P25HZ_TB, //68
	RES_3D_1080I60HZ_TB, //69
	RES_3D_1080I50HZ_TB,//70
	RES_3D_1080I30HZ_TB,//71
	RES_3D_1080I25HZ_TB,//72
	RES_3D_720P30HZ_TB,//73
	RES_3D_720P25HZ_TB,//74
	RES_3D_720P24HZ_TB,//75
	RES_3D_720P23HZ_TB,//76
	RES_3D_576P50HZ_TB,//77
	RES_3D_576I25HZ_TB,//78
	RES_3D_576I50HZ_TB,//79
	RES_3D_480P60HZ_TB,//80
	RES_3D_480I30HZ_TB,//81
	RES_3D_480I60HZ_TB,//82
	//3D Side by Side half
	RES_3D_1080P60HZ_SBS_HALF,//83
	RES_3D_1080P50HZ_SBS_HALF,//84
	RES_3D_1080P30HZ_SBS_HALF,//85
	RES_3D_1080P29HZ_SBS_HALF,//86
	RES_3D_1080P25HZ_SBS_HALF,//87
	RES_3D_1080P24HZ_SBS_HALF,//88
	RES_3D_1080P23HZ_SBS_HALF,//89
	RES_3D_1080I30HZ_SBS_HALF,//90
	RES_3D_1080I25HZ_SBS_HALF,//91
	RES_3D_720P60HZ_SBS_HALF,//92
	RES_3D_720P50HZ_SBS_HALF,//93
	RES_3D_720P30HZ_SBS_HALF,//94
	RES_3D_720P25HZ_SBS_HALF,//95
	RES_3D_720P24HZ_SBS_HALF,//96
	RES_3D_720P23HZ_SBS_HALF,//97
	RES_3D_576P50HZ_SBS_HALF,//98
	RES_3D_576I25HZ_SBS_HALF,//99
	RES_3D_576I50HZ_SBS_HALF,//100
	RES_3D_480P60HZ_SBS_HALF,//101
	RES_3D_480I30HZ_SBS_HALF,//102
	RES_3D_480I60HZ_SBS_HALF,//103
	RES_4K2K23976HZ, //104
	RES_4K2K24HZ, //105
	RES_2160P_50HZ, //106
	RES_2160P_60HZ, //107
	RES_2161P_50HZ,  //108
	RES_2161P_60HZ,  //109
	RES_MODE_NUM,		   //110  dummy mode, used to determine the last mode
	RES_AUTO			   //111
} HDMI_RESOLUTION_MODE_T;

typedef HDMI_RESOLUTION_MODE_T HDMI_RESOLUTION_T ;

typedef  enum
{
    HDMI_VIDEO_720x480p_60Hz= RES_480P,  // 0
    HDMI_VIDEO_720x576p_50Hz = RES_576P,    // 1
    HDMI_VIDEO_1280x720p_60Hz = RES_720P60HZ,   // 2
    HDMI_VIDEO_1280x720p_50Hz = RES_720P50HZ,   // 3
    HDMI_VIDEO_1920x1080i_60Hz = RES_1080I60HZ,  // 4
    HDMI_VIDEO_1920x1080i_50Hz = RES_1080I50HZ,  // 5
    HDMI_VIDEO_1920x1080p_30Hz = RES_1080P30HZ,  // 6
    HDMI_VIDEO_1920x1080p_25Hz = RES_1080P25HZ,  // 7
    HDMI_VIDEO_1920x1080p_24Hz = RES_1080P24HZ,  // 8
    HDMI_VIDEO_1920x1080p_23Hz = RES_1080P23_976HZ,  // 9
    HDMI_VIDEO_1920x1080p_29Hz = RES_1080P29_97HZ,  // a
    HDMI_VIDEO_1920x1080p_60Hz = RES_1080P60HZ,  // b 
    HDMI_VIDEO_1920x1080p_50Hz = RES_1080P50HZ,  // c

    HDMI_VIDEO_1280x720p3d_60Hz = RES_3D_720P60HZ,   // d
    HDMI_VIDEO_1280x720p3d_50Hz = RES_3D_720P50HZ,   // e
    HDMI_VIDEO_1920x1080i3d_60Hz = RES_3D_1080I60HZ,  // f
    HDMI_VIDEO_1920x1080i3d_50Hz = RES_3D_1080I50HZ,  // 10
    HDMI_VIDEO_1920x1080p3d_24Hz = RES_3D_1080P24HZ,  // 11
    HDMI_VIDEO_1920x1080p3d_23Hz = RES_3D_1080P23HZ,  // 12

	HDMI_VIDEO_720x480i_60Hz = RES_480I,  // 13
    HDMI_VIDEO_720x576i_50Hz = RES_576I,    // 14   
	
    HDMI_VIDEO_RESOLUTION_NUM = RES_MODE_NUM
}   HDMI_VIDEO_RESOLUTION;


#endif