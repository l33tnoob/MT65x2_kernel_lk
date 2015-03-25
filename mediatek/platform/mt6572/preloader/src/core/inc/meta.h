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

#ifndef META_H
#define META_H

#define HSHK_TOKEN_SZ           (8)             /* handshake token size */
#define HSHK_COM_READY          "READY"         /* com ready for communication */
#define HSHK_DISCON_SZ          (10)
#define HSHK_DISCON             "DISCONNECT"    /* com disconnect for communication */

#define	META_STR_REQ            "METAMETA"      /* META request */

#define META_STR_ACK            "ATEMATEX"      /* META ack Response */

#define META_LOCK               "LOCK"          /* META lock */
#define META_FORBIDDEN_ACK      "METAFORB"

#define META_ADV_REQ            "ADVEMETA"
#define META_ADV_ACK            "ATEMEVDX"

#define FACTORY_STR_REQ         "FACTFACT"
#define FACTORY_STR_ACK         "TCAFTCAF"

#define ATE_STR_REQ             "FACTORYM"      /* ATE request */
#define ATE_STR_ACK             "MYROTCAF"      /* ATE ack response */

#define SWITCH_MD_REQ           "SWITCHMD"      /* switch MD request */
#define SWITCH_MD_ACK           "DMHCTIWS"      /* switch MD ack response */

#define ATCMD_PREFIX            "AT+"
#define ATCMD_NBOOT_REQ         ATCMD_PREFIX"NBOOT"    /* AT command to trigger normal boot */
#define ATCMD_OK                ATCMD_PREFIX"OK"
#define ATCMD_UNKNOWN           ATCMD_PREFIX"UNKONWN"

#define FB_STR_REQ              "FASTBOOT"
#define FB_STR_ACK              "TOOBTSAF"

typedef struct {
    unsigned int len;           /* the length of parameter */
    unsigned int ver;           /* the version of parameter */
} para_header_t;

typedef struct {
    para_header_t header;       /* the header of parameter */
    unsigned char usb_type;     /* 0: single interface device, 1: composite device */
    unsigned char usb_num;      /* usb com port number */
    unsigned char dummy1;
    unsigned char dummy2;
} para_v1_t;

typedef union {
    para_header_t header;
    para_v1_t     v1;
} para_t;

#endif /* META_H */

