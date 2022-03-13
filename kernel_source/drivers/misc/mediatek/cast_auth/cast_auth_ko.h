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
#ifndef _CAST_AUTH_KO_H_
#define _CAST_AUTH_KO_H_


/*-----------------------------------------------------------------------------
                    macros, typedefs, enums
 ----------------------------------------------------------------------------*/
    
typedef unsigned long UINT32;
typedef unsigned char UINT8;

#define ENC_RSA_KEY_PATH "/etc/cast_auth/cast_key_block.bin"

#define SHARED_MEM_SIZE 16*1024
#define SHARED_MEM_SIZE_OFFSET 8*1024

#define SUPPORT_INOUT_PARAMETER 16
#define SUPPORT_INOUT_PARAMETER_OFFSET 8

//other define
#define CAST_AUTHR_OK                           ((int)    0)
#define CIPHSV_IOCTL_CMD_CAST (0x0b<<4)
#define CAST_AUTH_IOCTL_CMD_DRM_SIGN_HASH         _IOWR('s',(CIPHSV_IOCTL_CMD_CAST + 0x01),AUDIO_CAST_DRM_SIGN_HASH_T)
#define CAST_AUTH_IOCTL_CMD_DRM_GENERATE_DEVICECERT_AND_KEY _IOWR('s',(CIPHSV_IOCTL_CMD_CAST + 0x02),AUDIO_CAST_DRM_GENERATE_DEVICECERT_AND_KEY_T)
#define CAST_AUTH_IOCTL_CMD_DRM_GET_UID                     _IOWR('s',(CIPHSV_IOCTL_CMD_CAST + 0x03),AUDIO_CAST_UID_T)
#define CAST_AUTH_KO_OP_SIGN_HASH                   0x01
#define CAST_AUTH_KO_OP_GENERATE_DEVICECERT_AND_KEY 0x02
#define CAST_AUTH_KO_OP_GET_UID                     0x03

#define UNIQUE_ID_LEN 20
#define WIFI_MAC_ADDRESS_LEN 6

typedef struct _AUDIO_CAST_DRM_SIGN_HASH_T
{
    const UINT8 * in_wrapped_device_key;
    UINT32   in_wrapped_device_key_len;
    const UINT8 * in_hash;
    UINT32   in_hash_len;
    UINT8 *  out_signature;
    UINT32   out_signature_len;
    int             result;
}AUDIO_CAST_DRM_SIGN_HASH_T;

typedef struct _AUDIO_CAST_DRM_GENERATE_DEVICECERT_AND_KEY_T
{
    const UINT8 * in_mac_address;
    UINT32    in_mac_address_len;
    UINT8 * in_out_device_certificate;
    UINT32   in_out_device_certificate_len;
    UINT8 *  out_wrapped_device_key;
    UINT32   in_out_wrapped_device_key_len;
    int      result;
}AUDIO_CAST_DRM_GENERATE_DEVICECERT_AND_KEY_T;

typedef struct _AUDIO_CAST_UID_T
{
    const unsigned char     * in_mac_address;
    unsigned long             in_mac_address_len;
    unsigned char           * out_uniqueID;
    unsigned long             out_uniqueID_len;
    int                       result;
}AUDIO_CAST_UID_T;


#define _user_
#define _kernel_

/*-----------------------------------------------------------------------------
                    export function
 ----------------------------------------------------------------------------*/
extern int cast_auth_ko_init(void);
extern int cast_auth_ko_uninit(void);

#endif //_CAST_AUTH_KO_H_

