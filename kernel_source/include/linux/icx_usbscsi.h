/*
 * Copyright 2011,2012,2013,2014,2015,2016 Sony Corporation.
 */

/** @file icx_usbscsi.h
 ****************************************************
 *
 * @brief ioctl処理に関係するデータ構造の定義。
 *
 ****************************************************/
#ifndef ICX_USBSCSI_H
#define ICX_USBSCSI_H

struct scsi_to_msc_func {
    int (*devinfo)(char *, char *, u_int32_t *);
    int (*devprop)(char *, char *, u_int32_t *);
    int (*devreg)(char *, char *, u_int32_t *);
};

extern void msc_bind_usbscsi(struct scsi_to_msc_func *);

/* DEVINFO */
struct DEVINFO_data {
    u_int8_t vendor_identification[8];
    u_int8_t product_identification[16];
    u_int8_t product_revision[4];
    u_int8_t product_sub_revision[4];
    u_int8_t storage_size[4];
    u_int8_t serial_number[16];
    u_int8_t vendor_specific[32];
};

/* V_SMFMF */
struct V_SMFMF_data {
    u_int16_t smfmf_data_frame_info_count;
    u_int8_t  smfmf_data_frame_info[4064];  /* (4064 = 254 * 16) */
};

/* NVRAM */
struct NVRAM_INFO_data {
#define USBSCSI_IOC_NVRAM_CMD_READ_BDADR	1
#define USBSCSI_IOC_NVRAM_CMD_WRITE_BDADR	2
    u_int8_t cmd;
    u_int8_t data[256];
};

#define ICX_USBSCSI_DEVICE_NAME "/dev/icx_usbscsi"

#define USBSCSI_IOC_MAGIC  's'

/*--- Device Property Check Command ---*/
#define USBSCSI_IOC_SET_DEVINFO_NO		0
#define USBSCSI_IOC_GET_DEVINFO_NO		1
#define USBSCSI_IOC_SET_V_SMFMF_NO		2
#define USBSCSI_IOC_GET_V_SMFMF_NO		3
#define USBSCSI_IOC_SET_NVRAM_INFO_NO	4
#define USBSCSI_IOC_GET_NVRAM_INFO_NO	5

#define USBSCSI_IOC_SET_DEVINFO 	_IOW(USBSCSI_IOC_MAGIC, USBSCSI_IOC_SET_DEVINFO_NO, 	const struct DEVINFO_data *)  /* Set DEVINFO */
#define USBSCSI_IOC_GET_DEVINFO 	_IOR(USBSCSI_IOC_MAGIC, USBSCSI_IOC_GET_DEVINFO_NO, 	struct DEVINFO_data *)        /* Get DEVINFO */
#define USBSCSI_IOC_SET_V_SMFMF 	_IOW(USBSCSI_IOC_MAGIC, USBSCSI_IOC_SET_V_SMFMF_NO, 	const struct V_SMFMF_data *)  /* Set V_SMFMF */
#define USBSCSI_IOC_GET_V_SMFMF 	_IOR(USBSCSI_IOC_MAGIC, USBSCSI_IOC_GET_V_SMFMF_NO, 	struct V_SMFMF_data *)        /* Get V_SMFMF */
#define USBSCSI_IOC_SET_NVRAM_INFO 	_IOR(USBSCSI_IOC_MAGIC, USBSCSI_IOC_SET_NVRAM_INFO_NO, 	struct NVRAM_INFO_data *)     /* Set NVRAM */
#define USBSCSI_IOC_GET_NVRAM_INFO 	_IOR(USBSCSI_IOC_MAGIC, USBSCSI_IOC_GET_NVRAM_INFO_NO, 	struct NVRAM_INFO_data *)     /* Get NVRAM */

#define USBSCSI_IOC_MAXNR 5

#endif /* ICX_USBSCSI_H */
