/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef DOWNLOAD_LEGACY_H
#define DOWNLOAD_LEGACY_H

#include "typedefs.h"

/**************************************************************************
 *  DEBUG CONTROL
 **************************************************************************/
#define DM_DBG_LOG                      0
#define DM_UNIT_TEST_NAND_ERASE         0
#define DM_CAL_CKSM_FROM_USB_BUFFER     1
#define DM_CAL_CKSM_FROM_NAND_FLASH     0           
#define DM_TIME_ANALYSIS                0

/**************************************************************************
 *  SIZE DEFINITION
 **************************************************************************/
/* only the starting 28 bytes of spare data is valid */
#define DM_SZ_SPARE_OFFSET      (64-28)
#define DM_SZ_PL_INFO_PKT       sizeof(DM_PL_INFO_PACKET)
#define DM_SZ_IMG_INFO_PKT      sizeof(DM_IMAGE_INFO_PACKET)
#define DM_SZ_EXT_IMG_INFO_PKT  sizeof(DM_EXT_IMAGE_INFO_PACKET)
#define DM_SZ_CHK_SUM_PKT       sizeof(DM_CHKSUM_PACKET)
#define DM_SZ_ERR_CODE_PKT      sizeof(DM_ERRCODE_PACKET)
#define DM_SZ_PATCH_CMD_PKT     sizeof(DM_PATCH_CMD_PACKET)
#define DM_SZ_LOCK_CMD_PKT      sizeof(SecurityBitCtrl_YuSu_T)
#define DM_SZ_PT_INFO_CMD_PKT   sizeof(DM_PARTITION_INFO_PACKET)
#define DM_BUF_MAX_SIZE         (COMMON_BUFFER_MAX_SIZE)
#define DM_CMD_MAX_SIZE         (128)

/**************************************************************************
*  CONSTANT DEFINITION
**************************************************************************/
/* Note : all the following constant definition                  */
/*        must be sych with tool side, once any one modifies it  */
/*        remember to inform any guy doing the same modification */

#define DM_PL_INFO_PKT_PATN             0x50494C50  /* Info Packet pattern */
                                                    /* = ascii value of "PLIP" */

#define DM_IMAGE_INFO_PKT_PATN          0x50474D49  /* Image Packet pattern */
                                                    /* = ascii value of "IMGP" */

#define DM_CHKSUM_PKT_PATN              0x4D534B43  /* Checksum Packet pattern */
                                                    /* = ascii value of "CKSM" */

#define DM_ERROR_PKT_PATN               0x50525245  /* Error Code Packet pattern */
                                                    /* = ascii value of "ERRP" */

#define DM_PCMD_INFO_PKT_PATN           0x444D4350  /* Patch Command Packet pattern */
                                                    /* = ascii value of "PCMD" */

#define DM_PARTITION_INFO_PKT_PATN      0x504E5450  /* Partition Info Packet pattern */
                                                    /* = ascii value of "PTNP" */
                                                        
/**************************************************************************
*  STRUCTURE DEFINITION
**************************************************************************/
/* Note : all the following structure and enumeration definition */
/*        must be sych with tool side, once any one modifies it  */
/*        remember to inform any guy doing the same modification */

typedef struct _DM_CHKSUM_PACKET
{
    UINT32 pattern;
    UINT32 chk_sum;                             /* check sum of whole image */
} DM_CHKSUM_PACKET;

typedef enum _DM_ERR_CODE
{
    DM_ERR_OK                       = 0,        /* success */
    DM_ERR_NOMEM                    = 1,        /* no memory */
    DM_ERR_NAND_RD_FAIL             = 2,        /* nand read fail */
    DM_ERR_NAND_WR_FAIL             = 3,        /* nand write fail */
    DM_ERR_NAND_ER_FAIL             = 4,
    DM_ERR_WRONG_SEQ                = 5,        /* protocol fail, wrong sequence */
    DM_ERR_WRONG_ADDR               = 6,
    DM_ERR_WRONG_PKT_SZ             = 7,
    DM_ERR_EXCEED_BOUNDARY          = 8,        /* boundary check fail */
    DM_ERR_NO_VALID_TABLE           = 9,
    DM_ERR_NO_SPACE_FOUND           = 10,
    DM_ERR_UNKNOWN                  = 0xFFFFFFFF
} DM_ERR_CODE;

typedef struct _DM_ERRCODE_PACKET
{
    UINT32 pattern;
    DM_ERR_CODE err_code;                       /* check sum of whole image */
} DM_ERRCODE_PACKET;

typedef struct _DM_FLASH_INFO
{
    UINT16 man_code;                            /* flash manufacture code */
    UINT16 dev_id;                              /* flash device id */
    UINT16 dev_code;                            /* flash device code */
    UINT16 reserve;
    UINT32 dev_size;                            /* flash device size */
    UINT32 page_size;                           /* flash device page size */
    UINT32 spare_size;                          /* flash device spare size */
                                                /* spare size = page size / 32 */
                                                /* but there may be exceptions */
} DM_FLASH_INFO;

typedef struct _DM_PL_INFO_PKT
{
    U32 pattern;                                /* preloader info packet */
    U32 version;                                /* spedicy preloader version */
    U32 chip;                                   /* which chip used */
    U32 reserved1;
    U32 reserved2;
    DM_FLASH_INFO flash_info;
} DM_PL_INFO_PACKET;

typedef enum
{
    DM_IMG_FORMAT_FAT = 0,
    DM_IMG_FORMAT_YAFFS2 = 1,
    DM_IMG_FORMAT_UNKNOWN = 0xFFFFFFFF
} DM_IMG_FORMAT;



/* no use */
#define IMAGE_PATCH_CMD_MAGIC       0x5613A8C3
typedef struct _DM_PATCH_CMD_PACKET
{
    UINT32 pattern;                             /* image info packet */
    U32 magic_number;
    U8 enable_patch_verify;                     /* 0 : no need to verify patch */
                                                /* 1 : need to verify patch */
} DM_PATCH_CMD_PACKET;


/**************************************************************************
*  PARTITION PART
*  : Tool team also refers to this data structure
*  : So, DO NOT modify the image type !!
**************************************************************************/
typedef enum
{
    DM_IMG_TYPE_LOGO                = 0,
    DM_IMG_TYPE_BOOTIMG             = 1,
    DM_IMG_TYPE_ANDROID             = 2,
    DM_IMG_TYPE_USRDATA             = 3,
    DM_IMG_TYPE_RECOVERY            = 4,
    DM_IMG_TYPE_UBOOT               = 5,
    DM_IMG_TYPE_AUTHEN_FILE         = 6,
    DM_IMG_TYPE_SECSTATIC           = 7,
    DM_IMG_TYPE_EU_FT_INFORM        = 8,
    DM_IMG_TYPE_FT_LOCK_INFORM      = 9,
    DM_IMG_TYPE_PT_TABLE_INFORM     = 10,

    //cust pack image
    DM_IMG_TYPE_CUST_IMAGE1         = 100,
    DM_IMG_TYPE_CUST_IMAGE2         = 101,  
    DM_IMG_TYPE_CUST_IMAGE3         = 102,  
    DM_IMG_TYPE_CUST_IMAGE4         = 103, 

    DM_IMG_TYPE_UNKNOWN             = 0xFFFFFFFF
} DM_IMG_TYPE;

typedef struct _DM_IMG_INFO
{
    DM_IMG_FORMAT img_format;                   /* image format */
    DM_IMG_TYPE img_type;                       /* image type */
    UINT32 img_size;                            /* image file size */
    UINT32 addr_off;                            /* image nand address offset */   
#ifdef FEATURE_DOWNLOAD_BOUNDARY_CHECK     
    UINT32 addr_boundary;                       /* image nand address boundary */   
#endif
    UINT32 pkt_size;                            /* packet size per transmition */    
} DM_IMG_INFO;

#pragma pack(4)
typedef struct _DM_IMAGE_INFO_PACKET
{
    UINT32 pattern;                             /* image info packet */
    struct _DM_IMG_INFO img_info;
} DM_IMAGE_INFO_PACKET;
#pragma pack()

typedef struct _DM_EXT_IMG_INFO
{
    DM_IMG_FORMAT img_format;                   /* image format */
    DM_IMG_TYPE img_type;                       /* image type */
    U32 img_size;                               /* image file size */
    U32 addr_off;                               /* image nand address offset */
#ifdef FEATURE_DOWNLOAD_BOUNDARY_CHECK    
    U32 addr_boundary;                          /* image nand address boundary */
#endif   
    U32 pkt_size;                               /* packet size per transmition */
} DM_EXT_IMG_INFO;

#pragma pack(4)
typedef struct _DM_EXT_IMAGE_INFO_PACKET
{
    U32 pattern;                                /* image info packet */
    struct _DM_EXT_IMG_INFO img_info;
} DM_EXT_IMAGE_INFO_PACKET;
#pragma pack()

/**************************************************************************
 *  Download Mode STRing
 **************************************************************************/
#define DM_STR_READY                "READY"     /* Ready Signal */
#define DM_STR_DOWNLOAD_REQ         "DOWNLOAD"  /* Download Request */
#define DM_STR_DOWNLOAD_ACK         "DAOLNWOD"  /* Download Ack Response */
#define DM_STR_START_REQ            "START"     /* start to send data after IMGP */
#define DM_STR_REBOOT               "REBOOT"    /* Reboot Command */
#define DM_STR_ATBOOT               "ATBOOT"    /* AutoBoot Command */
#define DM_STR_UPDATE               "UPDATE"    /* Update Command */
#define DM_STR_LOCKED_REQ           "LOCKED"    /* Respond lock notification */

#define DM_STR_VERIFY_PASS          "VERIFYOK" 
#define DM_STR_VERIFY_FAIL          "VERIFYFAIL" 

#define DM_STR_PLIP                 "PLIP"
#define DM_STR_IMGP                 "IMGP"
#define DM_STR_CKSM                 "CKSM"
#define DM_STR_ERRP                 "ERRP"
#define DM_STR_PCMD                 "PCMD"

#define DM_SZ_READY_STR             5
#define DM_SZ_DWN_REQ_STR           8
#define DM_SZ_DWN_ACK_STR           8
#define DM_SZ_START_STR             5
#define DM_SZ_REBOOT_STR            6
#define DM_SZ_ATBOOT_STR            6
#define DM_SZ_UPDATE_STR            6

#define DM_SZ_VERIFY_PASS_STR       9
#define DM_SZ_VERIFY_FAIL_STR       10

#define DM_SZ_IMG_HPBUF             10          /* max protocol buffer size */

/**************************************************************************
*  HANDSHAKE SYNC TIME
**************************************************************************/
#if (CFG_BOARD_ID == MT6516_EVB)
#define DM_WAIT_SYNCH_TIME          3000        /* in ms */
#else
#define DM_WAIT_SYNCH_TIME          2500        /* in ms */
#endif

/**************************************************************************
*  STRUCTURE DECLARATION
**************************************************************************/
typedef enum
{
    DM_PKT_DWNL                     = 0,        /* dwonload string packet */
    DM_PKT_REBT                     = 1,        /* reboot sting packet */
    DM_PKT_PLIP                     = 2,        /* preloader info packet */
    DM_PKT_IMGP                     = 3,        /* image info packet */
    DM_PKT_CKSM                     = 4,        /* checksum packet */
    DM_PKT_ERRP                     = 5,        /* error code packet */
    DM_PKT_DATA                     = 6,        /* image data packet */
    DM_PKT_AUTO                     = 7,        /* image data packet */
    DM_PKT_PCMD                     = 8,        /* patch command packet */
    DM_PKT_UPDT                     = 9,        /* update command packet */
    DM_PKT_UNKNOWN                  = 0xFFFFFFFF
} DM_PKT_TYPE;



typedef enum
{
    DM_STATUS_NONE                  = 0,
    DM_STATUS_START                 = 1,        /* download mode statusstarted */
    DM_STATUS_SECT_ONGING           = 2,        /* one file section is onging */
    DM_STATUS_SECT_FINISHED         = 3,        /* one file section is finished */
    DM_STATUS_SECT_WAIT_NXT         = 4,        /* wait for next section */
    DM_STATUS_ERR_ONGOING           = 5,        /* error has occured, but still let section onging */
    DM_STATUS_ERR_FINISHED          = 6,        /* error has occured, and the section is finished  */
    DM_STATUS_REBOOT                = 7,        /* reboot state */
    DM_STATUS_AUTH_VERIFY           = 8,        /* verify authentication file */
    DM_STATUS_ATBOOT                = 9,        /* autoboot state */
    DM_STATUS_PATCH_CMD             = 10,       /* indicate if image patch needs to be verified */
    DM_STATUS_PATCH_VERIFY          = 11,       /* verify end user image patch */
    DM_STATUS_UPDATE                = 15,       /* update state */    
    DM_STATUS_UNKNOWN               = 0xFFFFFFFF
} DM_STATUS;

typedef struct
{
    DM_STATUS dm_status;                        /* current download mode status */
    DM_ERR_CODE dm_err;                         /* error code to return to tool */
    u32 block_size;                             /* flash device block size */
    u32 page_size;                              /* flash device page size */
    u32 spare_size;                             /* flash device spare size */
    struct _DM_IMG_INFO img_info;               /* current downloading image information */
    u32 part_range;                             /* cureent partition range/limit */
    //uint load_size;                           /* current received / loaded image size */
    u32 curr_off;                               /* current load image address offset (packet alignment) */
    u32 page_off;                               /* current load image address offset (page alignment) */
    u32 pkt_cnt;                                /* total packet count, count from 0 */
    u32 curr_cnt;                               /* current packet count, count form 0 */
    u8 *data_buf;                               /* image RX data buffer */

#if (DM_CAL_CKSM_FROM_USB_BUFFER || DM_DBG_LOG)
    u8 *rb_buf;                                 /* packet read back buffer */
#endif

#if DM_CAL_CKSM_FROM_USB_BUFFER
    u32 chk_sum;
#endif
} DM_CONTEXT;

/**************************************************************************
*  EXPORT FUNCTION PROTOTYPE
**************************************************************************/
extern void download_mode_detection (void);
extern void reset_dm_descriptor (void);
extern u32  get_part_range (DM_IMG_TYPE img_type);
extern u8   *get_part_name (DM_IMG_TYPE type);

extern void do_reboot (char mode);
extern bool listen_tool (uint listen_byte_cnt, uint wait_time);

#if DM_CAL_CKSM_FROM_USB_BUFFER
extern u32  cal_chksum_per_pkt (u8 * pkt_buf, u32 pktsz);
#endif

extern void handle_data (u32 pktsz, u8 * buf);
extern void handle_cksm (void);
extern u32  handle_imgp (u32 * pktsz);
extern u32  check_imgp (DM_IMAGE_INFO_PACKET * imgp, u32 * pktsz);
extern u32  save_imgp (DM_IMAGE_INFO_PACKET * imgp);
extern void handle_pl_info (void);
extern void handle_midle_state (u8 * buf);
extern u8   *prepare_data_buf (void);
extern DM_PKT_TYPE judge_pkt_type (const void *buf);

extern bool handshake_protocol (void);
extern void download_handler (void);

#endif /* DOWNLOAD_LEGACY_H */
