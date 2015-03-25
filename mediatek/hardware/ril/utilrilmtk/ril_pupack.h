/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


namespace android {

#define SOCKET_NAME_MTK_RILD    "rild-mtk-ut"
#define SOCKET_NAME_MTK_RILD_2    "rild-mtk-ut-2"

#define PROPERTY_RIL_UICC_TYPE  "gsm.ril.uicctype"
#define PROPERTY_RIL_UICC2_TYPE "gsm.ril.uicctype.2"

/* 
 * packRILcommandString
 * Used for packing standard AT command string to RIL command.
 * 
 * IN   *inStr      : AT command string with NULL terminate
 * IN   *prefix     : prefix of AT response
 * OUT  **outCmd    : RAW RIL command out. Caller is responsible to free this resource.
 * RETUURN          : Length of outCmd data.   
 */
size_t packRILCommand(char *inStr, char *prefix, char **outCmd);


/*
 * unpackRILResponse
 * Used for unpacking RIL response to AT command response.
 *
 * IN   *inRsp      : RAW RIL response from socket. 
 * IN   length      : Length of inRsp data.
 * RETURN           : AT response data string. Caller is responsible to free this resource
 */
char * unpackRILResponse(char *inRsp, size_t length);

/*
 * unpackRILResponse
 * Used for unpacking RIL response to AT command response.
 *
 * IN         *inRsp     : RAW RIL response from socket. 
 * IN         length     : Length of inRsp data.
 * OUT      *ril_errno : the returned error cause from RIL response
 * RETURN           : AT response data string. Caller is responsible to free this resource
 */
char * unpackRILResponseWithResult(char *inRsp, size_t length, int *ril_errno);


}
