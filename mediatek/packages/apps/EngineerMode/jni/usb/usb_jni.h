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

#ifndef _OTG_TEST_H_
#define _OTG_TEST_H_

#include <cutils/xlog.h>

#define OTG_LOG(...) \
        do { \
            XLOGD(__VA_ARGS__); \
        } while (0)

/*macro for USB-IF for OTG driver*/
#define OTG_CMD_E_ENABLE_VBUS       0x00
#define OTG_CMD_E_ENABLE_SRP        0x01
#define OTG_CMD_E_START_DET_SRP     0x02
#define OTG_CMD_E_START_DET_VBUS    0x03
#define OTG_CMD_P_A_UUT             0x04
#define OTG_CMD_P_B_UUT             0x05
#define OTG_CMD_P_B_UUT_TD59        0x0d

/*test mode for USB host driver*/
#define HOST_CMD_TEST_SE0_NAK    	0x6
#define HOST_CMD_TEST_J          	0x7
#define HOST_CMD_TEST_K          	0x8
#define HOST_CMD_TEST_PACKET     	0x9
#define HOST_CMD_SUSPEND_RESUME  	0xa
#define HOST_CMD_GET_DESCRIPTOR  	0xb
#define HOST_CMD_SET_FEATURE     	0xc

#define HOST_CMD_ENV_INIT	     	0xe
#define HOST_CMD_ENV_EXIT	     	0xf

#define OTG_MSG_DEV_NOT_SUPPORT     0x01
#define OTG_MSG_DEV_NOT_CONNECT     0x02
#define OTG_MSG_HUB_NOT_SUPPORT     0x03

#define OTG_STOP_CMD    0x10
#define OTG_INIT_MSG    0x20

/*test number*/
#define ENABLE_VBUS     0x01
#define ENABLE_SRP      0x02
#define DETECT_SRP      0x03
#define DETECT_VBUS     0x04
#define A_UUT						0x05
#define B_UUT						0x06
#define TD_5_9						0x0e

#define TEST_SE0_NAK				0x07
#define TEST_J						0x08
#define TEST_K						0x09	
#define TEST_PACKET					0x0a
#define SUSPEND_RESUME				0x0b
#define GET_DESCRIPTOR				0x0c
#define SET_FEATURE					0x0d


#endif /* _OTG_TEST_H_ */
