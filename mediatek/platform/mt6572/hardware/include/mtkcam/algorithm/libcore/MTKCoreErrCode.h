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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _MTK_CORE_ERR_CODE_H_
#define _MTK_CORE_ERR_CODE_H_

/* error message data */
#define CORE_ERR_MESSAGE_DATA(n, CMD) \
    CMD(n, CORE_OK)\
    CMD(n, CORE_COMMON_ERR_INVALID_PARAMETER)\
    CMD(n, CORE_COMMON_ERR_NULL_BUFFER_POINTER)\
    CMD(n, CORE_COMMON_ERR_OUT_OF_MEMORY)\
    CMD(n, CORE_MOTION_ERR_LOW_TRUST_VALUE)\
    CMD(n, CORE_MOTION_ERR_LARGE_MOTION_VECTOR)\
    CMD(n, CORE_MOTION_ERR_VERTICAL_SHAKE)\
    CMD(n, CORE_WARP_ERR_INCORRECT_IMAGE_FORMAT)\
    CMD(n, CORE_CYLIND_PROJ_ERR_INCORRECT_IMAGE_FORMAT)\
    CMD(n, CORE_ERR_MAX)

/* macro to declare and get string */
#define CORE_ERRCODE_ENUM_DECLARE(n, a) a,
#define CORE_ERRCODE_ENUM_STRING(n, a) (a == n) ? #a :
#define CORE_GET_ERRCODE_NAME(n) \
    (0 == n) ? "CORE_ERR_UNKNOWN" : CORE_ERR_MESSAGE_DATA(n, CORE_ERRCODE_ENUM_STRING)""\

/* error code enum */
typedef enum CORE_ERRCODE_ENUM
{
    CORE_ERR_UNKNOWN = 0,
    CORE_ERR_MESSAGE_DATA(, CORE_ERRCODE_ENUM_DECLARE)
} CORE_ERRCODE_NUM;

#endif /* _MTK_CORE_ERR_CODE_H_ */

