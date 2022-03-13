#ifndef _TVE_DRV_H_
#define _TVE_DRV_H_

#define CVBS_DRV "/dev/cvbs"


typedef struct
{
    void *src_base_addr;
    void *src_phy_addr;
    int src_fmt;
    unsigned int  src_pitch;
    unsigned int  src_offset_x, src_offset_y;
    unsigned int  src_width, src_height;

    int next_buff_idx;
    int identity;
    int connected_type;
    unsigned int security;
    int fenceFd;
} tve_video_buffer_info;



typedef struct
{
    //  Input
    int ion_fd;
    // Output
    unsigned int index; //fence count
    int fence_fd;   //fence fd
} tve_buffer_info;

/** Attributes for the cps common info inband command.
 */
typedef struct
{
    unsigned int u4InfoValid;
    unsigned char u1OriginalCgms;
    unsigned char u1Cgms;
    unsigned char u1Aps;
    unsigned char u1AnalogSrc;
    unsigned char u1ICT;
    unsigned char u1DOT;
    unsigned char u1CSS;
    unsigned char u1AACS;
    unsigned char u1EPN;
    unsigned char u1NotPassCnt; // must not pass content to a specific output device for wmdrm output control
    unsigned char u1DCICCI;
} CpsCommonInfoParamsDef;

#define TVE_IO_MAGIC   'T'

#define TVE_IOW(num, dtype)     _IOW(TVE_IO_MAGIC, num, dtype)
#define TVE_IOR(num, dtype)     _IOR(TVE_IO_MAGIC, num, dtype)
#define TVE_IOWR(num, dtype)    _IOWR(TVE_IO_MAGIC, num, dtype)
#define TVE_IO(num)             _IO(TVE_IO_MAGIC, num)

#define CMD_TVE_SET_FORMAT      	TVE_IOW( 0, int)   
#define CMD_TVE_SET_LOG_ON      	TVE_IOW( 1, int)   
#define CMD_TVE_GET_STATUS      	TVE_IOW( 2, int)   
#define CMD_TVE_SET_COLORBAR    	TVE_IOW( 3, int)   
#define CMD_TVE_SET_BRIGHTNESS  	TVE_IOW( 4, int)   
#define CMD_TVE_GET_UNIFY       	TVE_IOW( 5, int) 
#define CMD_TVE_GET_DEVNAME     	TVE_IOW( 6, int)
#define CMD_TVE_SET_ENABLE      	TVE_IOW( 7, int)  //just for enable dac and encoder  
#define CMD_TVE_SET_TVE_POWER_ON   	TVE_IOW( 8, int)
#define CMD_TVE_SET_RESUME      	TVE_IOW( 9, int)
#define CMD_TVE_SET_SUSPEND     	TVE_IOW( 10, int)
#define CMD_TVE_GET_TVE_STATUS    	TVE_IOW( 11, int)
#define CMD_TVE_SET_DPI0_CB    		TVE_IOW( 12, int) 
#define CMD_TVE_SET_MV        		TVE_IOW( 13, int) 
#define CMD_TVE_GET_FORMAT     		TVE_IOW( 14, int)
#define CMD_TVE_SET_ASPECT     	TVE_IOW( 15, int)
#define CMD_TVE_SET_CPS     		TVE_IOW( 16, CpsCommonInfoParamsDef)
			
#define CMD_TVE_POST_VIDEO_BUFFER              TVE_IOW(20,  tve_video_buffer_info)
#define CMD_TVE_GET_DEV_INFO                   TVE_IOWR(35, mtk_dispif_info_t)
#define CMD_TVE_PREPARE_BUFFER                 TVE_IOW(36, tve_buffer_info)
#define CMD_TVE_IS_USER_MUTE              TVE_IOW(38,  int)

#define MTK_TVE_NO_FENCE_FD        ((int)(-1)) //((int)(~0U>>1))
#define MTK_TVE_NO_ION_FD        ((int)(-1))   //((int)(~0U>>1)

#endif
