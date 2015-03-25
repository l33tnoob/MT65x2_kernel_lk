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

#ifndef _MTK_CAMERA_INC_COMMON_CAMUTILS_CAMMISC_H_
#define _MTK_CAMERA_INC_COMMON_CAMUTILS_CAMMISC_H_
//


/******************************************************************************
*
*******************************************************************************/


namespace android {
namespace MtkCamUtils {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
*
*******************************************************************************/
template<int value> struct int2type { enum {v=value}; };


/******************************************************************************
  * @brief      make all directories in path.
  *
  * @details
  * @note
  *
  * @param[in]  path: a specified path to create.
  * @param[in]  mode: the argument specifies the permissions to use, like 0777 
  *                   (the same to that in mkdir).
  *
  * @return
  *  -  true if successful; otherwise false.
  *****************************************************************************/
bool
makePath(
    char const*const path, 
    uint_t const mode
);


/**
  * @brief save the buffer to the file 
  *
  * @details 
  * 
  * @note 
  * 
  * @param[in] fname: The file name 
  * @param[in] buf: The buffer want to save 
  * @param[in] size: The size want to save 
  *
  * @return
     * -   MTRUE indicates the operation is success 
     * -   MFALSE indicates the operation is fail 
  */ 
bool
saveBufToFile(
    char const*const fname, 
    uint8_t*const buf, 
    uint32_t const size
);


/**
  * @brief read the file to the buffer
  *
  * @details 
  * 
  * @note 
  * 
  * @param[in] fname: The input file name 
  * @param[out] buf: The output buffer 
  * @param[in] size: The buf size be read  
  * 
  *
  * @return
     * -   The read count 
  */ 
uint32_t
loadFileToBuf(
    char const*const fname, 
    uint8_t*const buf, 
    uint32_t size
);

/**
  * @brief set the thread policy & priority
  *
  * @details 
  * 
  * @note 
  * 
  * @param[in] policy: The policy of the thread 
  * @param[in] priority: The priority of the thread
  *
  * @return
     * -   MTRUE indicates the operation is success 
     * -   MFALSE indicates the operation is fail 
  */ 
bool
setThreadPriority(
    int policy, 
    int priority
);


/**
  * @brief get the thread policy & priority
  *
  * @details 
  * 
  * @note 
  * 
  * @param[out] policy: The policy of the thread 
  * @param[out] priority: The priority of the thread
  *
  * @return
     * -   MTRUE indicates the operation is success 
     * -   MFALSE indicates the operation is fail 
  */ 
bool
getThreadPriority(
    int& policy, 
    int& priority
);


/**
  * @dump android call stack
  *
  */ 
void
dumpCallStack(void);


/******************************************************************************
*
*******************************************************************************/
};  // namespace MtkCamUtils
};  // namespace android
#endif  //_MTK_CAMERA_INC_COMMON_CAMUTILS_CAMMISC_H_

